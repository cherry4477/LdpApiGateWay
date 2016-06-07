#!/bin/bash


sed -i 's/REDIS_SERVER_IP/'$REDIS_PORT_6379_TCP_ADDR'/g'  ./config/userQuery.conf 
sed -i 's/REDIS_SERVER_PORT/'$REDIS_PORT_6379_TCP_PORT'/g'  ./config/userQuery.conf 



export DATAHUB_IP=$DATAHUB_IP
export DATAHUB_PORT=$DATAHUB_PORT

./apigateway -c ./config/userQuery.conf -n query

#nohup ./main -c ./config/userQuery.conf -n query 2>&1 &
