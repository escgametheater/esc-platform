#!/bin/bash

GAME_HOME="/Users/escuser/escgames"
GAME_EXPANDED="$GAME_HOME/.tmp_expanded"
BUCKET_CONFIG=$(</Users/escuser/Desktop/CONFIG/bucket_config.txt)
GAME_ARCHIVES="$GAME_HOME/.archives"
GAME_BUCKET="escgames$BUCKET_CONFIG:"

echo "$GAME_BUCKET"

s3sync sync -v $GAME_BUCKET $GAME_ARCHIVES

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

# unzip games downloaded in the last hour to the tmp directory
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

else
	echo "no new games"
fi 

# check for deleted games and remove from GAME_HOME if necessary
cd $GAME_HOME
for game in *; do
    GAME_NAME=${game%.*}
    MATCHES_FOUND=0
    for GAMEZIP in $GAME_ARCHIVES/*; do
	ZIP_NAME=`basename $GAMEZIP`
	NAME_TO_MATCH=$(echo $ZIP_NAME | sed -e "s/_[0-9]*\.zip$//")
	if [ "$NAME_TO_MATCH" == "$GAME_NAME" ]; then
	    ((MATCHES_FOUND++))
	fi
    done
    if [[ "$MATCHES_FOUND" -eq 0 ]]; then
	if [ -d "$GAME_HOME/${GAME_NAME}.app" ]; then
	    rm -rf $GAME_HOME/${GAME_NAME}.app
	    echo "DELETED $GAME_HOME/${GAME_NAME}.app"
	fi
    fi
done
