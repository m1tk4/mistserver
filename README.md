MistServer
==========

MistServer is an open source, public domain, full-featured, next-generation streaming media toolkit for OTT (internet streaming), designed to be ideal for developers and system integrators.

For full documentation, tutorials, guides and assistance, please look on our website at: https://mistserver.org


This Fork: m1tk4/mistserver
===========================
This fork of MistServer provides RPM packages and Docker container images for it (mainly for my own use but you're welcome to them!) using GitHub CI build system, GitHub hosting and container repositories. Check out the [Releases](https://github.com/m1tk4/mistserver/releases) and [Packages](https://github.com/m1tk4?tab=packages&repo_name=mistserver) sections to the 
right for links to individual RPMs and Docker container images.

The releases and docker images in this fork track `development` branch in https://github.com/DDVTECH/mistserver/tree/development.


Installing MistServer RPMs
==========================

1. Make sure CRB, EPEL, and RPMFusion repositories are enabled on your server:

    ```bash
    # Rocky9/Alma9:
    dnf -y install 'dnf-command(config-manager)'; \
    dnf config-manager --set-enabled crb; \
    dnf config-manager --set-enabled devel; \
    dnf -y install epel-release; \
    dnf -y install --nogpgcheck \
        https://dl.fedoraproject.org/pub/epel/epel-release-latest-$(rpm -E %rhel).noarch.rpm; \
    dnf -y install --nogpgcheck \
        https://mirrors.rpmfusion.org/free/el/rpmfusion-free-release-$(rpm -E %rhel).noarch.rpm \
        https://mirrors.rpmfusion.org/nonfree/el/rpmfusion-nonfree-release-$(rpm -E %rhel).noarch.rpm
    ```
2. Install `mistserver` RPM from https://github.com/m1tk4/mistserver/releases

3. If you need to be able to stream files other than MP4/TS install `mistserver-in-av` RPM.

4. Enable and start `mistserver` service:

    ```bash
    systemctl enable --now mistserver
    ```

5. Configure `firewalld` according to the outputs you are using in MistServer. Sample commands:

    ```bash
    firewall-cmd --permanent --add-port={4242,4200,8080,1935,5554}/tcp; \
    firewall-cmd --permanent --add-port={4242,8888,8889}/udp; \
    firewall-cmd --reload
    ```

Running MistServer Docker Containers
====================================

1. Install Docker on your system as per https://docs.docker.com/engine/install/

2. To pull the image and start the container, edit the following with your file paths and shared memory size and run:

   ```bash
   docker run \
     --detach \
     --mount type=bind,source=/PATH/TO/CONFIG.JSON,destination=/config.json \
     --mount type=bind,source=/FOLDER/WITH/VIDEOFILES,destination=/video \
     --shm-size=1G \
     --network=host \
     ghcr.io/m1tk4/mistserver-ubuntu-dev:latest
   ```

The shared memory size should take values like '4G' etc. On dedicated MistServer systems it is a good practice to use 95% of available RAM.

For more details on running MistServer in Docker Containers, see: 

 - https://docs.mistserver.org/mistserver/installation/docker/
 - https://docs.mistserver.org/mistserver/quickstart/installation#docker-specific-instructions



Usage
=====

Accessing port 4242 from a web browser will bring up a web interface capable of easily running most API commands for human-friendly configuration.

Full usage instructions and API specifications can be found in the manual: https://mistserver.org/guides/latest


Compile instructions
====================

Check out Visual Studio Code tasks.
