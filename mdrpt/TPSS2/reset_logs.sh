#!/usr/bin/ksh

cat /dev/null > ./var/bnmlog/mdrapi_node.bnmlog
cat /dev/null > ./var/bnmlog/mdrmgr.bnmlog
cat /dev/null > ./var/log/console.out
cat /dev/null > ./var/log/mdrapi_node.log
cat /dev/null > ./var/log/mdrmgr.log
rm -f ./data/nodmq/nodmq.db3

