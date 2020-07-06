#!/bin/bash

EJABBERDCTL='/Applications/ejabberd/bin/ejabberdctl '

# check ejabberd status
$EJABBERDCTL status
if [ $? != 0 ]; then
    ps -ef | grep "beam.smp" | grep -v grep | awk '{print $2}' | xargs kill -9
    $EJABBERDCTL start
else
    exit 0
fi    
    
