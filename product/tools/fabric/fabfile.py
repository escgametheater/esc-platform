'''
Uses Fabric to distribute commands remotely to a set of computers
---------------------------------------------------------------------
Fabric:
http://fabfile.org

Fabric installation instructions:
http://docs.fabfile.org/en/1.4.3/installation.html

Fabric documentation:
http://docs.fabfile.org/en/1.4.3/index.html

Reminders:
 * Be sure add your python modules directory to the local path
 * Ensure that SSH keys are set and distributed accordingly
 * Use @parallel for parallelizing tasks. NOTE a password 
   challenge must be avoided for this.

=====================================================================
Imports fabfile_local.py by default to set the list of environments
Use e() method on the command line to pick the environment

example usage:
$ fab e:<env-name> <task-method-name>:<parameters>

The above example will invoke the e() method first using the supplied 
parameter followed by any subsequent methods thereafter

The deploy and rsync tasks are set to run in parallel mode when
using the -P flag with fab

Tested with Python 2.6 and 2.7.3
---------------------------------------------------------------------

Author: caleb.johnston@controlgroup.com

'''

import os, time, socket
from fabric.api import *
from fabric.contrib.project import rsync_project
from fabric.contrib import files

try:
    from fabfile_local import *
except ImportError, e:
    environments = {
            "dev": {
                "hosts": ["localhost"],
                },
            }
    print "You can customize environments with fabfile_local.py"

def e(name='staging'):
    print "Setting environment", name
    env.update(environments[name])
    env.environment = name
    env.user = 'escuser'
    env.password = 'ESCPassword1'
	
@parallel
def restart_app():
    stop_app()
    time.sleep(3)   # Wait a few seconds for program shutdown to complete
    start_app()
    
@parallel
def sleep_computer():
    sudo('pmset sleepnow')

@parallel
def shutdown_computer():
    env.warn_only = True
    sudo('shutdown -h now')

def restart_computer():
    time.sleep(5)
    env.warn_only = True
    sudo('shutdown -r now')


# def enable_maintanence_mode():
#     run('launchctl unload ~/Library/LaunchAgents/launchd-TechWall-live.plist')
#     run('rm ~/Library/LaunchAgents/launchd-TechWall-live.plist')


# def disable_maintanence_mode():
#     deploy_launchd_plists()
#     run('launchctl load ~/Library/LaunchAgents/launchd-TechWall-live.plist')
#     time.sleep(5)   # must wait until app is finished starting up...
#     run("osascript -e 'tell application \"TechWall-live\" to activate'")  # without this, the cursor is still drawn...


# def enable_live_mode():
#     disable_maintanence_mode()


# def disable_live_mode():
#     enable_maintanence_mode()
    

def start_app():
    run('open ~/EscPlatform.app')


def stop_app():
    run("osascript -e 'tell application \"EscPlatform\" to quit'")
	

# def get_logs():
#     get('/var/log/system.log', '~/Development/Brookfield/fabric/logs/bftw-syslog-' + env.host + '.log')
# 		
# def get_crash_dumps():
# 	env.warn_only = True
# 	local('rsync escuser@' + env.host + ':~/Library/Logs/DiagnosticReports/TechWall*.crash ~/Development/Brookfield/fabric/dumps')

# if files.exists('~/Library/Logs/DiagnosticReports/TechWall*.crash')
# get('~/Library/Logs/DiagnosticReports', '~/Development/Brookfield/fabric/logs/bftw-syslog-' + env.host + '.log')


# def deploy(local_path='~/Desktop/brookfield/content-schedule'):
#     put(local_path, '~/content')

def host_type():
    run('uname -s')


# def deploy_template(path='../TechWall/resources/template_location'):
#     rsync_project(
#             remote_dir="~/Development/Brookfield-TechWall/TechWall/xcode/build/Debug/",
#             local_dir='../TechWall/xcode/build/Debug/TechWall-live.app',
#             exclude=("*_local.py", "*.pyc",),
#             )

@parallel
def deploy_launchd_plists():
    rsync_project(
            remote_dir="~/Library/LaunchAgents/",
            local_dir='../TechWall/launchd/*.plist',
            exclude=("launchd-TechWall-stage.plist", "launchd-TechWall-stage.plist")
            )

def deploy_app(type='release'):
	if type=='release':
	    push("../../unity/EscPlatform/build/EscPlatform.app", "~/")
	elif type=='debug':
		push("../../unity/EscPlatform/build/EscPlatform.app", "~/")
	else: 
		print 'Deployment type ' + type + ' not understood. Must be "debug" or "release" kthnxbai'

# for pushing ...
def push(local_path, remote_path):
    rsync_project(
            remote_dir=remote_path,
            local_dir=local_path,
            )