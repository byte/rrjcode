#!/bin/sh

REMOTE_IP="192.168.0.8"
REMOTE_USER="byte"
evolution --force-shutdown
ssh $REMOTE_IP evolution --force-shutdown
rsync -e ssh -vzra ~/.evolution/ $REMOTE_USER@$REMOTE_IP:~/.evolution/
# rsync -e ssh -vzra ~/evolution/ $REMOTE_USER@$REMOTE_IP:~/evolution/ # for <2.0 only
rsync -e ssh -vzra ~/.gconf/apps/evolution/ $REMOTE_USER@$REMOTE_IP:~/.gconf/apps/evolution/
