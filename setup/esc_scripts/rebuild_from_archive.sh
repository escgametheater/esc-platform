GAME_HOME="/Users/escuser/escgames"
GAME_EXPANDED="$GAME_HOME/.tmp_expanded"
GAME_ARCHIVES="$GAME_HOME/.archives"

# unload and kill launcher
launchctl unload /Library/LaunchAgents/com.escgame.keeplauncheralive.plist
ps aux |grep -v grep |grep -i escLauncher | awk '{print $2;}' | xargs kill -9

# check for necessary directories and create if necessary, cd to GAME_EXPANDED
if [ ! -d $GAME_ARCHIVES ]; then
	mkdir $GAME_HOME/.tmp_expanded
fi

if [ -d $GAME_EXPANDED ]; then
	cd $GAME_EXPANDED
else
	mkdir $GAME_HOME/.tmp_expanded
	cd $GAME_EXPANDED
fi

# unzip all games from archives
find $GAME_ARCHIVES/ -name '*.zip' -exec sh -c 'unzip -o {}' \;

# move newly expanded games to escgames or delete then move if existing
if [ "$(ls -A $GAME_EXPANDED)" ]; then 
	for game in *; do
	    GAME=$game
	    if [ -d $GAME_HOME/$GAME ]; then
		rm -rf $GAME_HOME/$GAME
		mv -fv $GAME $GAME_HOME/$GAME
	    else	
		mv -fv $GAME $GAME_HOME/$GAME
	    fi
	done
fi

# relaunch launcher
launchctl load /Library/LaunchAgents/com.escgame.keeplauncheralive.plist