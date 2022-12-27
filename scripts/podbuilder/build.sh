#!/bin/bash

# 
# Builds several version of MistServer for Linux using podman
#

# Show help and quit with errorlevel 1
help_and_bail() {
    cat << EOF
usage: $(basename $0) <buildVersion> [<buildVersion>...] | all | clean | distclean

Builds various versions of MistServer for different Linux platforms.

Parameters:

    buildVersion:   one of:
                        el9 - Rocky/Alma/RHEL 9

    clean:          delete podman build containers
    distclean:      delete podman build images and containers

EOF
    exit 1
} 

if [ "$1" == "" ]; then
    help_and_bail
fi

# Parse the command line params
while [ ! "$1" == "" ]
do
    case $1 in

        clean|distclean)
            make -C $(dirname $0) --eval "include *.make" $1
            shift
            ;;

        el9)
            make -C $(dirname $0) -f $1.make build
            shift
            ;;

        all)
            make -C $(dirname $0) --eval "include *.make" build
            shift
            ;;

        *)
            help_and_bail
            ;;

    esac
done

