MistServer
==========

MistServer is an open source, public domain, full-featured, next-generation streaming media toolkit for OTT (internet streaming), designed to be ideal for developers and system integrators.

For full documentation, tutorials, guides and assistance, please look on our website at: https://mistserver.org


This Fork: m1tk4/mistserver
===========================
This fork of MistServer provides RPM packages for it (mainly for my own use but you're welcome to them!) using GitHub CI build system and GitHub Hosting. Check out the "Releases" section to the 
right for links to individual RPMs.

The releases in this fork track `development` branch in https://github.com/DDVTECH/mistserver/tree/development

Getting MistServer onto your system
===================================

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

Usage
=====

Accessing port 4242 from a web browser will bring up a web interface capable of easily running most API commands for human-friendly configuration.

Full usage instructions and API specifications can be found in the manual: https://mistserver.org/guides/latest
