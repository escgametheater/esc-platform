#!/bin/bash

BUCKET_CONFIG=$(</Users/escuser/Desktop/CONFIG/media_config.txt)
MEDIA_HOME="/Users/escuser/esclauncher/slideshow"
MEDIA_BUCKET="escmedia$BUCKET_CONFIG:"

# check for slideshow directory and create if necessary
if [ ! -d $MEDIA_HOME ]; then
	mkdir /Users/escuser/esclauncher/slideshow
fi

s3sync sync -v $MEDIA_BUCKET $MEDIA_HOME
