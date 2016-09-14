#!/bin/bash

sed -i 's/REDIS_SERVER_IP/'$REDIS_PORT_6379_TCP_ADDR'/g'  ./config/userQuery.conf 
sed -i 's/REDIS_SERVER_PORT/'$REDIS_PORT_6379_TCP_PORT'/g'  ./config/userQuery.conf 


sed -i 's/THREAD_NUM/'$API_THREAD_NUM'/g'  ./config/userQuery.conf
sed -i 's/EXPOSE_PORT/'$API_PORT'/g'  ./config/userQuery.conf 

export DATAHUB_IP=$DATAHUB_IP
export DATAHUB_PORT=$DATAHUB_PORT
export ADMIN_AUTH=ZGF0YWh1YitkYXRhaHViQGFzaWFpbmZvLmNvbTo0NmM1ZmM4NDkxYjk2MzI0MDFhMjA3YzdhYjA0ZWIwYQ==

./apigateway -c ./config/userQuery.conf -n query

#nohup ./apigateway -c ./config/userQuery.conf -n query > /dev/null 2>&1 &