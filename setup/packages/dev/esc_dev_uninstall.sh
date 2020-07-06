#!/bin/bash

/Applications/ejabberd/uninstall.app/Contents/MacOs/osx-intel --mode unattended
sleep 8

if [ -f /Users/$USER/.erlang.cookie ]
    then
    sudo rm /Users/$USER/.erlang.cookie
fi

if [ -f /Library/LaunchAgents/com.escgame.startejabberd.plist ]
    then
    sudo rm /Library/LaunchAgents/com.escgame.startejabberd.plist
fi

if [ -f /Users/Shared/keep_ejabberd_alive.sh ]
    then
    sudo rm /Users/Shared/keep_ejabberd_alive.sh
fi

if [ -d /Applications/ejabberd ]
    then
    sudo rm -rf /Applications/ejabberd  
fi
