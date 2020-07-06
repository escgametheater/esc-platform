#!/bin/bash

# set ARD to allow only cguser and enable access
sudo /System/Library/CoreServices/RemoteManagement/ARDAgent.app/Contents/Resources/kickstart -configure -allowAccessFor -specifiedUsers
sudo /System/Library/CoreServices/RemoteManagement/ARDAgent.app/Contents/Resources/kickstart -activate -configure -users cguser -access -on -privs -all

# set Remote Login to allow only cguser and enable access
if [ $(sudo dscl . list /Groups | grep 'access_ssh') == "com.apple.access_ssh-disabled" ]; then
    sudo dscl . change /Groups/com.apple.access_ssh-disabled RecordName com.apple.access_ssh-disabled com.apple.access_ssh
fi
sudo dscl . append /Groups/com.apple.access_ssh GroupMembership cguser
sudo systemsetup -setremotelogin on
