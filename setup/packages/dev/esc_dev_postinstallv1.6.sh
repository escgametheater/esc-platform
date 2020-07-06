#!/bin/bash

# change permissions for copied assets
sudo chown -R $USER /Users/Shared/ejabberd* /Users/Shared/keep_ejabberd_alive.sh

# install and configure ejabberd
su $USER -c "/Users/Shared/ejabberd-13.12.app/Contents/MacOS/osx-intel --mode unattended --installer-language en --prefix /Applications/ejabberd --admin escdeveloper --adminpw 'H8gn42x@' --cluster 0"

# rename config file
su $USER -c "mv /Applications/ejabberd/conf/ejabberd.cfg /Applications/ejabberd/conf/ejabberd.cfg.default"

# replace config file
su $USER -c "mv /Users/Shared/ejabberd.cfg.template /Applications/ejabberd/conf/ejabberd.cfg"

# create a backup and replace all occurences within file 
sed -i.template -e "s/REPLACE_HOSTNAME/$HOSTNAME/g" /Applications/ejabberd/conf/ejabberd.cfg 

# start ejabberd
su $USER -c "/Applications/ejabberd/bin/ejabberdctl start"

# disable Display Sleep and Screensaver
sudo pmset -a displaysleep 0
su $USER -c "defaults write com.apple.screensaver idleTime 0"

# clean-up
rm -rf /Users/Shared/ejabberd-13.12.app
