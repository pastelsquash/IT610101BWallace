# OVERVIEW AND SYSTEM SPECIFICATIONS

###1. Project Purpose and Notes
This document is intended to aid in recreating a fully-functional final project for
DJ Kehoe's IT610-101 class. The documentation will cover the setup and configuration
of three servers (named Front-End, Backend, and Nagios), as well as provide example
configuration files using git.

Note that the configuration files provided through git <b>won't necessarily work out of the box!</b>
IP addresses, e-mails, names, and pertinent admin information provided within these files are specific
to the project delivered at the end of the class, and will not work "as-is" outside of this implementation.

Where outside instructions were used, links have been provided. Credit goes where credit is due.

Most credit is due to me.

I am the best.

###2. Recommended Computer Specifications - AWS

All instructions provided in this document assume an Ubuntu 64-bit 16.04 environment - no guarantees
are provided that this will work in other environments. All servers used in the final deliverable were AWS
virtual machines of the "t2.micro" EC2 category. This document does not cover how to generate AWS instances, and does not
necessarily require them in order to be properly recreated.

As t2.micro VMs, each machine has the following specs:

- CPU: single core
- Memory: 1GB
- Storage: EBS only (recommended storage for non-AWS: 20GB+)
- Network Performance: Low to Moderate

Outside of these basic requirements, any Ubuntu 64-bit 16.04 or later environment will likely work.

###3. How to Use this Document

If you can successfully read this document, then congratulations - you're on your first step to properly using it.
Anyone capable of vaulting such a high hurdle will surely be capable of following these three simple usage rules:

- I provide a lot of commands. When commands are provided, it's assumed you're in either the project git directory, or the previous working directory.
If you're not sure where you need to be, think and look ahead a bit - you'll figure it out.
- When a guide is linked to, I will assume you can follow that guide just as well as you follow this one. Some guides require you to
(gasp!) put in <b>your own</b> IP Addressing info and other unique credentials. This is one of those guides. I link to some of those guides.
- This guide specifically follows the command-line portions of configuration. This is about 90% of the work, but not all of it. Specifically,
if you have trouble getting the three servers to talk to each other, it's likely a problem with your firewalls (either in AWS or your local network).
There are guides that will help you fix this. I'm sure you can find them.

With pleasantries out of the way, it's time to get to work.

# SETTING UP THE FRONT-END SERVER

###1. Installing and setting up Git

Your first order of business is to get Git up-and-running on the device that will serve as your front-end server.
The following commands will help you do just that:

	sudo apt-get update
	sudo apt-get install git

From your home folder (which you can reach from anywhere with "cd ~"), execute the following commands:

	mkdir projectgit
	cd projectgit
	git clone https://github.com/pastelsquash/IT610101BWallace.git
	sudo mv IT610101BWallace/* .

This will make a folder named projectgit in your home directory. It includes a lot of important files necessary to set up the front-end, plus
a ton you won't need. If you're reading this document, chances are this isn't news.

###2. Installing Necessary Dependencies
#### APACHE-------

Apache helps your front-end become visible to the outside world through a webpage. This is basically all the front-end server is - a web server.
Getting Apache up-and-running is the first step to getting the webpage up:

	sudo apt-get install apache2
	sudo ufw allow in "Apache Full"

Once you've done this, try to connect to your server: mine's at http://ec2-18-217-25-112.us-east-2.compute.amazonaws.com in a local web browser
	If this fails, open up the necessary ports (80 and 443) in AWS, or on your local network.

To make sure Apache boots with the system boots, try this command:

	sudo /usr/sbin/update-rc.d apache2 defaults

#### MYSQL-------

The base project I'm building from included a full LAMP stack, so we're gonna go ahead and install the M here. Why?
Good question: we'll be setting up a default Bootstrap page, and Bootstrap needs an entire LAMP. Setup MySQL with this command:

	sudo apt-get install mysql-server
		(set a password of your choice when prompted; don't leave it blank!)

####	PHP-------

The P in LAMP means PHP. PHP is pretty easy to install:

	sudo apt-get install php libapache2-mod-php php-mcrypt php-mysql

With this, all the necessary dependencies for the front-end server have been installed. Now it's time to actually configure stuff.

###3. Configuring SSL and Bootstrap
####	SSL------

For SSL, we're using a "snake oil" cert which I've helpfully provided within my project git. You're welcome.
Install it, and SSL, by doing this:

	sudo a2enmod ssl
	sudo a2ensite default-ssl
	sudo service apache2 restart

	sudo mkdir /etc/apache2/cert
	sudo cp ./apache.crt /etc/apache2/cert/apache.crt
	sudo cp ./apache.key /etc/apache2/cert/apache.key

(Those last two commands assume you're in my projectgit directory. If you're not - or if you've moved stuff around - go ahead and fix
the commands as needed.)

####	BOOTSTRAP------

Bootstrap's configuration marks the first step of this documentation where things can go terribly wrong. Hooray!
See if this stuff works:

	sudo apt-get install node.js
	sudo apt-get install npm
	sudo npm install -g grunt-cli

Did it work? It did, you say - perfectly, even? Great! That's sure not the experience I had the first time, you lucky dog, you.

Now we need to move into my project git's bootstrapproject/bootstrap directory and install npm. Try these commands:

	cd bootstrapproject
	cd bootstrap
	sudo npm install
	sudo service apache2 restart

	cd ../..  (move back to projectgit folder)

At this point, we have bootstrap mostly set up, and npm good to go. Now we just need to get the .conf files straightened out.

####	CONF FILES-------

Now, here's where things start getting tricky. I've provided some example config files which can be used throughout this project;
and some of these files, such as apache.conf, can be ported directly to your implementation likely without much trouble. Emphasis
on "likely".

Move the apache.conf file in projectgit to your apache home folder:

	sudo cp ./apache2.conf /etc/apache2/apache2.conf

Now, that said, one line in my apache.conf file needs to be fixed. Check the "<Directory /home/ben/projectgit/..." line;
confirm path to projectgit is correct. (Note: your username is likely not "ben", which is why this needs fixing.)

Now that you're past that part, you just need to move a couple more config files to their new homes:

	sudo cp ./default-ssl.conf /etc/apache2/sites-available/default-ssl.conf
	sudo cp ./000-default.conf /etc/apache2/sites-enabled/000-default.conf

Note that both of these config files have the same problem as apache.conf. Check their DocumentRoot directives, and modify them to match
the route to <b>your</b> bootstrap folder, not mine.

Now that that's over with,restart the apache service:

	sudo service apache2 restart

...And try to connect to your default webpage again. You ought to see something new. If you do, congratulations - the front-end server
is fully-functional! (We'll configure its Nagios agent later.) If not... start diagnosing. You probably did something wrong. You wrong-doer.

#SETTING UP THE BACK-END SERVER

###1. Installing MySQL

Remember that this is a different server than the front-end machine; this documentation will guide you through setting it up as a separate
machine.

All the back-end server is is a MySQL server. It's really not hard to set up, so consider this a freebie - the third server will be much harder.
Let's start by making sure MySQL is isntalled and able to communicate on the network:

	sudo apt-get update
	sudo apt-get install mysql-server
		(choose desired password; again, don't leave it blank!)
	sudo iptables -I INPUT -p tcp --dport 3306 -m state --state NEW,ESTABLISHED -j ACCEPT
	sudo iptables -I OUTPUT -p tcp --sport 3306 -m state --state ESTABLISHED -j ACCEPT
	sudo /usr/sbin/update-rc.d mysql defaults

###2. Make an Example Database and Table

Now that MySQL is installed, you're gonna want to make a table:

	mysql -u root -p
		(enter password)

You're now signed in to MySQL's command-line interface. You need to make a table now, and fill it some values -
any values. The exact values are unimportant, so I'll leave it up to your creativity. Make some happy little clouds.

If you're not SQL savvy, try this guide: https://www.w3schools.com/sql/sql_create_db.asp

If you're not feeling especially Bob Ross, you can always just copy my values like the lazy-ass you are.

		My values in database test_database:
			test_table(student_id, student_name, student_major)
				1,Ben Wallace, IT Admin
				2,DJ Kehoe, Computer Biologies
				3,John Cena, Art of Invisibility
				4,Joseph Stalin,Sharing
				5,William Shatner,The Art of the Deal

###3. Cron Job

The only other thing our back-end server needs (other than a Nagios agent) is a backup script.
But first, let's make a directory for our backups to go into:

	move to home folder (cd ~)
	mkdir mysql
	cd mysql

Copy the following into a file and name it "backupscript":

	#!/bin/bash

	user="root"
	password="<password you set at initialization>"
	host=""
	db_name="<name of database you made in pt. 2>"

	backup_path="/home/<your username>/mysql" #<---make sure directory exists!
	date=$(date +"%d-%b-%Y")

	umask 177

	mysqldump --user=$user --password=$password --host=$host $db_name > $backup_path/$db_name-$date.sql
	find $backup_path/* -mtime +7 -exec rm {} \;

Now we need to make the backupscript executable and order root's crontab to run it:

	chmod 755 ./backupscript
	sudo crontab -e
		#Add this line to the bottom of the file:
			45 23 * * * /home/<your username>/mysql/backupscript


(Crontab notes: The first two numbers are the minute of hour and hour of day to do each daily backup;
    set these to whatever you choose. I chose the minute after I was done writing the line, because I'm not
		a patient man.)

Congratulations - the back-end server is (mostly) set up now. We'll get its Nagios agent set up later.

#SETTING UP THE NAGIOS SERVER
###1. Installing and setting up Git

Again, this server is supposed to be a distinct machine from the other two. Just figured I'd reiterate that.

The Nagios server is easily the most configuration-intensive one, so put your man pants on and get to work:

	sudo apt-get update
	sudo apt-get install git

We're gonna need the project git again, so go ahead and make space for it like you did with the front-end server:

	mkdir projectgit
	cd projectgit
	git clone https://github.com/pastelsquash/IT610101BWallace.git
	sudo mv IT610101BWallace/* .

So far, so good. Now for the hard part...

###2. Installing Necessary Dependencies
####APACHE-----

So first thing's first; Nagios needs Apache up-and-running to make its GUI accessible from anywhere. So let's set Apache up first:

	sudo apt-get install apache2
	sudo ufw allow in "Apache Full"

Try to connect to your default webpage in a local web browser.
If this fails, open up the necessary ports (80 and 443) in AWS or your local network.

Now we make Apache run at startup:

	sudo /usr/sbin/update-rc.d apache2 defaults

####MYSQL-------

Just like Bootstrap for the front-end server, Nagios needs a full LAMP stack. Here's how you install MySQL:

	sudo apt-get install mysql-server
		(set a password of your choice)

####PHP-------

...and here's what we'll need from PHP:

	sudo apt-get install php libapache2-mod-php php-mcrypt php-mysql

###2. Setting up Nagios Server

Instructions are hard. Luckily, sometimes you don't need to give instructions, because someone else already has.

Such is the case with installing the Nagios core for this server. You can go ahead and follow the tutorial in the link below;
note that you can skip the part about downloading and unzipping Nagios though, as Nagios and its plugins are already in the project git.

	Nagios tutorial (already unzipped)

	https://www.howtoforge.com/tutorial/ubuntu-nagios/

	(Note: you only need to install the Nagios core on this machine. You can follow its instructions on setting up agents later.)

Did you survive that tutorial? Great! Now don't forget to make Nagios run at startup:

	sudo /usr/sbin/update-rc.d nagios defaults

Now that the default Nagios core is taken care of, we need to make some configuration changes - a lot of them. Here comes the hard part:

###3. Nagios Config Files

The previous tutorial helped you install the Nagios core. What it didn't do is help you configure it.

I've provided every file I modified on the Nagios core within my project git, under <your home>/projectgit/AWS/nagios/.
The config files come from various folders spread all across your Nagios server. Here are where they'll be on your machine:

	/usr/local/nagios/etc/servers/apache_host.cfg #The front-end server's tracker file
	/usr/local/nagios/etc/servers/mysql_server.cfg #The back-end server's tracker file
	/usr/local/nagios/etc/objects/commands.cfg #All commands Nagios can use to query server health
	/usr/local/nagios/etc/objects/contacts.cfg #How Nagios tracks admins, specifically how to e-mail/warn them
	/etc/ssmtp/ssmtp.conf #The config file for ssmtp, which we'll set up in a moment
	/usr/local/nagios/etc/nagios.cfg # Nagios's core config file, didn't change much here
	/usr/local/nagios/etc/objects/localhost.cfg #How the Nagios server tracks its own health

You're free to move my config files as they are over yours in the same locations; be warned, however, that many of these files contain
IPs, e-mail addresses, or other me-specific info that you'll need to look over and set to your own values. Just scan over the files
and make changes as needed: if at any point you're confused as to what a config file is doing, google it - you'll get plenty of help.

The only thing you'll need to do, other than moving and altering these files, is to make Nagios aware of two non-standard health checks -
check_eth and check_uptime. Here's how to do check_eth:

	cd ~/projectgit
	cp ./check_eth /usr/local/nagios/libexec/check_eth
	cd /usr/local/nagios/libexec
	sudo chmod 755 check_eth
	sudo chmod 755 check_uptime

Did that all work for you? Great. Now you can check your glorious work - but first you'll need to restart Nagios and Apache:

	sudo service apache2 restart
	sudo service nagios restart

(If nagios isn't registered as a service yet, refer to the guide posted earlier - it'll show you how. If nagios is STILL
not showing up as a service, this is probably why: https://serverfault.com/questions/774498/failed-to-start-nagios-service-unit-nagios-service-failed-to-load-no-such-file)

Now navigate to your http://your.ip.here/nagios page; you'll need to sign in with the username "nagiosadmin" and the password you provided when configuring nagios.

if things work, then congratulations - you're most of the way done! If they don't work, then start diagnosin'.

###4. Setting up E-mail Notifications

There's a lot of ways to set up Nagios to send e-mail notifications. Most of them are wrong.

This guide isn't wrong:

	https://access.redhat.com/documentation/en-us/red_hat_gluster_storage/3/html/console_administration_guide/configuring_nagios_to_send_mail_notifications

That guide will show you how to set up the contacts.cfg file - which, if you've followed along with this guide, is something you've probably already done with your own info.
However, this guide might help check your work and ensure you understand what's going on - because from here on out, nothing's gonna work unless this part's done right.

With that out of the way, we now need to actually configure mail functionality. We'll use ssmtp:

	sudo apt-get install ssmtp

Now, i did provide the ssmtp.conf file for my implementation - you can fix it up and use if if you like.
That said, I'd recommend you instead follow the setup instructions from this guide: https://help.ubuntu.com/community/EmailAlerts

I've already modified the provided commands.cfg file to use ssmtp instead of /usr/bin/mail. This is a step not shown in any guide, so make sure you're aware of it - it may
help when diagnosing problems. Using vim, you can search through the commands.cfg file for the two instances of ssmtp to see what I did here.
(This isn't something you have to act on unless something's gone wrong.)

If You can see the Nagios webpage GUI, and you can receive e-mail alerts, then you're done setting up the Nagios server. That said, you've probably noticed that Nagios is failing
to contact every machine other than localhost: that's because the Nagios agents aren't set up yet on the front-end and back-end. That's our last step in finalizing the implementation.

#SETTING UP THE NAGIOS AGENTS

###1. Install agent (Front-End and Back-End)

As you can proably guess, this entire section must be done twice - once for the front-end server, and once for the back-end. Luckily, it's not terribly difficult either time.

The following guide will get you through the entire process:

	https://www.howtoforge.com/tutorial/ubuntu-nagios/

	(Note: you only need to follow the instructions on how to install Nagios agents. The core you're directing them to is the address of your Nagios server.)

Hopefully you on't have much trouble here. That said, things can break, especially when it comes to getting the three servers to communicate. The next part of the guide will go over
what might be going wrong

###3. Update Security Groups

There's a few things that get in the way of your Nagios agents on the back-end and front-end communicating with the Nagios server. Here are some diagnostic suggestions
if you're using AWS.

First, remove all outbound rules from each of your instance's security groups. Outbound rules are a whitelist, and keeping track of all the traffic and all the ports you'll be using is dumb.

Next, Make sure you configure implicit inbound rules for each instance. You'll need SSH for all three, HTTP/HTTPS for the front-end and Nagios server, and ICMP Echo Request and ICMP Echo Reply on all three as well.
(You can theoretically configure a single security group and put all three in it, but this technically gives your MySQL server unnecessary inbound ports.)

If you're still having trouble, I found it easier to put each device in its own security group. For some reason, it seemed to help. Why? I'm not sure; AWS is weird that way. But if you can't think of what the issue
could be any other way, chances are this could help.

#CONCLUSION

...So, what? you expected a parade? Confetti, perhaps?

You're done. Get movin'. You're not my problem anymore.

Shoo.