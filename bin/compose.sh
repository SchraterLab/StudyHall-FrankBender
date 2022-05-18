#!/bin/bash

PORT=$1

if [ -z "$1" ]
then
    PORT=8081
fi

ROOTDIR=`git rev-parse --show-toplevel`
source ${ROOTDIR}/config/settings
export ROOTDIR
export UID=$(id -u)
export GID=$(id -g)
export PORT
export DEP_DIR="${DEP_DIR}"
echo "DEP DIR IS"
echo "${DEP_DIR}"
export SRC_DIR=/env
export POSTGRESUSER=root
export POSTGRESPASSWORD=password
export PREFIX=${DOCKER_IMAGE_PREFIX}
echo "PREFIX IS"
echo "${PREFIX}"
# docker run --rm -p 127.0.0.1:$PORT:8081 -v "${ROOTDIR}:/home/user/repo" -it ${DOCKER_IMAGE_PREFIX}/env
docker-compose -f docker-compose.yml build app
