#!/usr/bin/env python

'''
Updates all Unity game plist files in the specified directory to 
include CFBundleURLSchemes which enables the Game Launcher to 
launch a new game using launch services.
---------------------------------------------------------------------

Reminders:
 * This file requires chmod +x to be executable
 * Unity game app packages should be placed in a 
   directory named "escgames" within the user home directory
 * Script writes output to esclauncher/Info.plist

Tested with Python versions 2.7.5, 2.7.6
---------------------------------------------------------------------

Authors: alex@19seventynine.com, caleb.johnston@controlgroup.com, andy.so@controlgroup.com

'''

import os
import os.path
import subprocess
import glob
import sys
import plistlib
import shutil
import distutils
import traceback
import datetime

from distutils import dir_util

bundleNames = {}

from os.path import expanduser

# collect a list of game file names
home = expanduser('~')
gamesPath = os.path.join(home, 'escgames/*.app')
games = glob.glob(gamesPath)
updateLauncherFile = False

# unregister list of games from launch services with lsregister kill
subprocess.call(['/usr/bin/lsregister', '-kill'])

# Update each games info.plist with URL scheme
for game in games:
    # get the path of the game's .plist
    info_plist_path = os.path.join(game, 'Contents/Info.plist')

    # get the .plist
    try:
        pl = plistlib.readPlist(info_plist_path)
    except IOError as e:
        error = "  Error loading plist file at: " + info_plist_path
        print error.format(e.errno, e.strerror)

    # if CFBundleDisplayName key does not exist, create it from the original bundle name
    if 'CFBundleDisplayName' in pl:
        pl['CFBundleDisplayName'] = pl['CFBundleDisplayName']
    else:
        pl['CFBundleDisplayName'] = pl['CFBundleName']

    # get the NaturalName attribute and strip spaces and lowercase
    _name = pl['CFBundleDisplayName']
    _newName = _name.replace(" ", "")
    _newName = _newName.lower()

    # rename the CFBundle attributes
    pl['CFBundleName'] = _newName
    pl['CFBundleExecutable'] = _newName

    # rename the game's executable file
    executablePath = os.path.join(game, 'Contents/MacOS/')
    for filename in os.listdir(executablePath):
        path = os.path.join(executablePath, filename)
        target = os.path.join(executablePath, _newName)
        os.rename(path, target)

    # create CFBundleURLTypes dictionary
    url_settings = [{'CFBundleURLSchemes': [_newName]}]
    pl['CFBundleURLTypes'] = url_settings

    # save the .plist
    plistlib.writePlist(pl, info_plist_path)

    # add this game's display name and its ID to dictionary in memory
    bundleNames[_name] = _newName

    subprocess.call(['/usr/bin/lsregister', '-f', game])

    updateLauncherFile = True

# update the esc launcher info.plist
# set filepath for the launcher .plist
gamesListPath = os.path.join(home, 'esclauncher/Info.plist')

# create the dict element for the launcher .plist
launcherPL = dict(
    CFBundleName = "escGames",
    games = bundleNames,
)

# write the launcher .plist to file
plistlib.writePlist(launcherPL, gamesListPath)

# remove the DOCTYPE node because Unity will crumble when trying to parse it...
lines = open(gamesListPath).readlines()
output_lines = []
output_lines.extend(lines[0])
output_lines.extend(lines[2:])
open(gamesListPath, 'w').writelines(output_lines)
print str(datetime.datetime.now()) + " : updated esc launch services"

