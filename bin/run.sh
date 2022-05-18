#!/bin/bash

# export ROOTDIR=`git rev-parse --show-toplevel`
# export UID=101
# export GID=103
# export PORT
# export POSTGRESUSER=root
# export POSTGRESPASSWORD=password
# export ROOTDIR=`git rev-parse --show-toplevel`
# source ${ROOTDIR}/config/settings
# export PREFIX=${DOCKER_IMAGE_PREFIX}
export ROOTDIR=`git rev-parse --show-toplevel`
docker-compose -f ./config/docker/docker-compose.yml run -p 127.0.0.1:8081:8081 app bash