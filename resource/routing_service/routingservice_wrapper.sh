#!/bin/bash

filename=$0
script_location=`cd "\`dirname "$filename"\`"; pwd`

cd $script_location

exec "$script_location/../common_service/commonservice_wrapper.sh" "$@" "-routing_service"