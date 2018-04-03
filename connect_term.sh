#! /bin/bash
port=`ls /dev/cu*|grep serial`
miniterm.py $port 74880

