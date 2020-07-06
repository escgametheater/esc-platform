import os, time, sys, shutil, subprocess
from datetime import tzinfo, timedelta, datetime

# location where new tech wall content will reside
mediasymlink = "/volume1/TechWallData/media"
# location where 
syncdir = "/volume1/TechWallData/syncdir"
# location where the usb drive will be mounted
mountdir = "/volumeUSB1"
# location where the usb drive content partition will be mounted
mountmediadir = "/volumeUSB1/usbshare"
# addresses for all application servers 
hosts = [ 
	"10.138.234.180", 
	"10.138.234.181",
	"10.138.234.182", 
	"10.138.234.183",
	"10.138.234.184", 
	"10.138.234.185",
	"10.138.234.186", 
	"10.138.234.187",
	"10.138.234.188",
	"10.138.234.189",
	"10.138.234.190",
	"10.138.234.191",
	 ]
	
# logging routine
def log(message):
	print 'sync.py // ' + datetime.now().strftime('%d-%b-%Y %H:%M:%S') + ' -- ' + message

# function to copy all content from usb connected drive, then to propogate content to all hosts
def copy_from_usb(): 
    if os.path.exists(mediasymlink) and os.path.getctime(mediasymlink) > os.path.getctime(mountdir):
        log('syncdir is newer')
    else: 
		if os.path.exists(mountmediadir):
		    log('mountdir is newer. beginning to copy files')
		    if os.path.exists(syncdir + ".bak"):
		        shutil.rmtree(syncdir + ".bak")

		    if os.path.exists(syncdir):
		        shutil.move(syncdir, syncdir + ".bak")
		    shutil.copytree(mountmediadir, syncdir)
		    if os.path.exists(mediasymlink):
		        os.remove(mediasymlink)
		    os.symlink(syncdir, mediasymlink)
		    copy_to_hosts()
        # else:
            # log('no usb drive mounted')

# function to propagate content from usb connected drive to all hosts defined above
def copy_to_hosts():
    for host in hosts:
		logfile = open('/var/services/homes/admin/log/py-scp.log', "w")
		params = ["/usr/syno/bin/scp", "-i", "/var/services/homes/admin/.ssh/id_dsa", "-r", "/volume1/TechWallData/media/assets", "/volume1/TechWallData/media/compositions", "/volume1/TechWallData/media/content-schedule", "/volume1/TechWallData/media/templates", "brookfield@" + host + ":~/content"]
		proc = subprocess.Popen(params, stdout=logfile)
		ret_code = proc.wait()
		logfile.flush()
		print 'return code = ' + str(ret_code)
		print 'log output: ', logfile

# obtain process ID for currently running python script
pid = str(os.getpid())
pidfile = "/tmp/sync.pid"

# if there is a process running currently then exit; otherwise perform copy operations
if os.path.isfile(pidfile):
	log('pid already exists (at ' + pidfile + '), exiting')
	sys.exit()
else:
	file(pidfile, 'w').write(pid)
	copy_from_usb()
	os.unlink(pidfile)
# copy_to_hosts()
