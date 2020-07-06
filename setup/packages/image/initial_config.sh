#!/bin/bash

# set hostname, localhostname, and computer name
echo "quit" | scutil --set HostName "esc-game-server.local"
echo "quit" | scutil --set LocalHostName "esc-game-server"
echo "quit" | scutil --set ComputerName "esc-game-server"

# set power management
pmset -a sleep 0 disksleep 0 womp 1 darkwakes 0 autorestart 1 displaysleep 0

# remove LaunchDaemon plist for initial config
rm /Library/LaunchDaemons/com.escgame.initialconfig.plist

rm $0
