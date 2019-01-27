#
# Regular cron jobs for the quickpad package
#
0 4	* * *	root	[ -x /usr/bin/quickpad_maintenance ] && /usr/bin/quickpad_maintenance
