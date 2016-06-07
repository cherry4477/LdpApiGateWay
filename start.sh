#!/bin/bash

export DATAHUB_IP=10.1.235.98
export DATAHUB_PORT=443
./apigateway -c ./config/userQuery.conf -n query

#nohup ./main -c ./config/userQuery.conf -n query 2>&1 &
