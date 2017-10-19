/*
 * License: GPL
 * Copyright (c) 2010,2012,2013 Davide Madrisan <davide.madrisan@gmail.com>
 *
 * A Nagios plugin to check how long the system has been running
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"

#include <assert.h>
#include <fcntl.h>
#if HAVE_GETOPT_H
#include <getopt.h>
#else
#include <compat_getopt.h>
#endif

#if HAVE_KSTAT_H
#include <kstat.h>
#endif

#if HAVE_LIBPERFSTAT
#include <sys/protosw.h>
#include <libperfstat.h>
#endif

#include <stdio.h>
#include <stdlib.h>

#if HAVE_SYS_SYSINFO_H
#include <sys/sysinfo.h>
#endif

#if TIME_WITH_SYS_TIME
#include <sys/time.h>
#include <time.h>
#else
#if HAVE_SYS_TIME_H
#include <sys/time.h>
#else
#include <time.h>
#endif
#endif

#include <unistd.h>

#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif

#if HAVE_SYS_SYSCTL_H
#include <sys/sysctl.h>
#endif

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#include "nputils.h"

static const char *program_name = "check_update";
static const char *program_version = PACKAGE_VERSION;
static const char *program_copyright =
  "Copyright (C) 2010,2012-2013 Davide Madrisan <" PACKAGE_BUGREPORT ">";

#define BUFSIZE 127
static char buf[BUFSIZE + 1];
static char result_line[BUFSIZE + 1], perfdata_line[BUFSIZE + 1];

double uptime (void);
char *sprint_uptime (double);

static void __attribute__ ((__noreturn__)) print_version (void)
{
  printf ("%s, version %s\n", program_name, program_version);
  printf ("%s\n", program_copyright);
  fputs ("\
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>\n\n\
This is free software; you are free to change and redistribute it.\n\
There is NO WARRANTY, to the extent permitted by law.\n", stdout);

  exit (STATE_OK);
}

static struct option const longopts[] = {
  {(char *) "critical", required_argument, NULL, 'c'},
  {(char *) "warning", required_argument, NULL, 'w'},
  {(char *) "help", no_argument, NULL, 'h'},
  {(char *) "version", no_argument, NULL, 'V'},
  {NULL, 0, NULL, 0}
};

static void __attribute__ ((__noreturn__)) usage (FILE * out)
{
  fprintf (out,
	   "%s, version %s - check how long the system has been running.\n",
	   program_name, program_version);
  fprintf (out, "%s\n\n", program_copyright);
  fprintf (out, "Usage: %s [OPTION]\n\n", program_name);
  fputs ("\
Options:\n\
  -w, --warning [@]start:end]   warning threshold\n\
  -c, --critical [@]start:end]   critical threshold\n\
  -h, --help            display this help and exit\n\
  -v, --version         output version information and exit\n\n", out);

  fputs ("\
Where:\n\
  1. start <= end\n\
  2. start and \":\" is not required if start=0\n\
  3. if range is of format \"start:\" and end is not specified, assume end is infinity\n\
  4. to specify negative infinity, use \"~\"\n\
  5. alert is raised if metric is outside start and end range (inclusive of endpoints)\n\
  6. if range starts with \"@\", then alert if inside this range (inclusive of endpoints)\n\n", out);
  fprintf (out, "Examples:\n  %s\n  %s --warning 30: --critical 15:\n",
	   program_name, program_name);

  exit (out == stderr ? STATE_UNKNOWN : STATE_OK);
}

/* assume uptime never be zero seconds in practice */
#define UPTIME_RET_FAIL  0

double
uptime ()
{
#if defined(HAVE_STRUCT_SYSINFO_WITH_UPTIME)	/* Linux */

  struct sysinfo info;

  if (0 != sysinfo (&info))
    {
      perror ("cannot get the system uptime");
      return UPTIME_RET_FAIL;
    }

  return info.uptime;

#elif defined(HAVE_FUNCTION_SYSCTL_KERN_BOOTTIME)	/* FreeBSD */

  int mib[] = { CTL_KERN, KERN_BOOTTIME };
  struct timeval system_uptime;
  size_t len = sizeof (system_uptime);

  if (0 != sysctl (mib, 2, &system_uptime, &len, NULL, 0))
    return UPTIME_RET_FAIL;

  return (time (NULL) - system_uptime.tv_sec);

#elif defined(HAVE_KSTAT_H)	/* Solaris */

  kstat_ctl_t *kc;
  kstat_t *ksp;
  kstat_named_t *knp;

  time_t now;

  if (NULL == (kc = kstat_open ()))
    return UPTIME_RET_FAIL;

  if (NULL !=
      (ksp = kstat_lookup (kc, (char *) "unix", 0, (char *) "system_misc")))
    {
      if (-1 != kstat_read (kc, ksp, 0))
	{
	  if (NULL != (knp = kstat_data_lookup (ksp, (char *) "boot_time")))
	    {
	      time (&now);
	      kstat_close (kc);
	      return (difftime (now, (time_t) knp->value.ul));
	    }
	}
    }

  kstat_close (kc);
  return UPTIME_RET_FAIL;

#elif defined(HAVE_LIBPERFSTAT)	/* AIX */

  long herz = 0;
  perfstat_cpu_total_t ps_cpu_total;

  herz = sysconf (_SC_CLK_TCK);
  /* make sure we do not divide by 0 */
  assert (herz);

  if (-1 ==
      perfstat_cpu_total (NULL, &ps_cpu_total, sizeof (ps_cpu_total), 1))
    return UPTIME_RET_FAIL;

  return ((double) ps_cpu_total.lbolt / herz);

#else

  return UPTIME_RET_FAIL;

#endif
}

char *
sprint_uptime (double uptime_secs)
{
  int upminutes, uphours, updays;
  int pos = 0;

  updays = (int) uptime_secs / (60 * 60 * 24);
  if (updays)
    pos +=
      snprintf (buf, BUFSIZE, "%d day%s ", updays, (updays != 1) ? "s" : "");
  upminutes = (int) uptime_secs / 60;
  uphours = upminutes / 60;
  uphours = uphours % 24;
  upminutes = upminutes % 60;

  if (uphours)
    {
      pos +=
	snprintf (buf + pos, BUFSIZE - pos, "%d hour%s %d min", uphours,
		  (uphours != 1) ? "s" : "", upminutes);
    }
  else
    pos += snprintf (buf + pos, BUFSIZE - pos, "%d min", upminutes);

  return buf;
}

int
main (int argc, char **argv)
{
  int c, uptime_mins, status;
  char *critical = NULL, *warning = NULL;
  double uptime_secs;
  thresholds *my_threshold = NULL;

  while ((c = getopt_long (argc, argv, "c:w:hV", longopts, NULL)) != -1)
    {
      switch (c)
	{
	default:
	  usage (stderr);
	  break;
	case 'c':
	  critical = optarg;
	  break;
	case 'w':
	  warning = optarg;
	  break;
	case 'h':
	  usage (stdout);
	  break;
	case 'V':
	  print_version ();
	  break;
	}
    }

  status = set_thresholds (&my_threshold, warning, critical);
  if (status == NP_RANGE_UNPARSEABLE)
    usage (stderr);

  if (UPTIME_RET_FAIL != (uptime_secs = uptime ()))
    {
      uptime_mins = (int) uptime_secs / 60;
      status = get_status (uptime_mins, my_threshold);
      free (my_threshold);

      switch (status)
	{
	case STATE_CRITICAL:
	  c = snprintf (result_line, BUFSIZE, "UPTIME CRITICAL:");
	  break;
	case STATE_WARNING:
	  c = snprintf (result_line, BUFSIZE, "UPTIME WARNING:");
	  break;
	case STATE_OK:
	  c = snprintf (result_line, BUFSIZE, "UPTIME OK:");
	  break;
	}

      snprintf (result_line + c, BUFSIZE - c, " %s",
		sprint_uptime (uptime_secs));
      snprintf (perfdata_line, BUFSIZE, "uptime=%d", uptime_mins);

      printf ("%s|%s\n", result_line, perfdata_line);
    }
  else
    {
      c = snprintf (result_line, BUFSIZE,
		    "UPTIME UNKNOWN: can't get system uptime counter");
      status = STATE_UNKNOWN;
    }

  return status;
}
