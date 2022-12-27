#
#   This Dockerfile is only used in the build process on Github
#

FROM docker.io/rockylinux/rockylinux:9

# Repositories
RUN dnf -y install 'dnf-command(config-manager)'; \
    dnf config-manager --set-enabled crb; \
    dnf config-manager --set-enabled devel; \
    dnf -y install epel-release; \
    dnf -y install --nogpgcheck https://dl.fedoraproject.org/pub/epel/epel-release-latest-$(rpm -E %rhel).noarch.rpm; \
    dnf -y install --nogpgcheck https://mirrors.rpmfusion.org/free/el/rpmfusion-free-release-$(rpm -E %rhel).noarch.rpm https://mirrors.rpmfusion.org/nonfree/el/rpmfusion-nonfree-release-$(rpm -E %rhel).noarch.rpm

# Things we need to build
RUN dnf -y install \
        rpm-build cmake \
        doxygen mc libsrtp libsrtp-devel \
        srt-libs srt-devel \
        ffmpeg ffmpeg-libs ffmpeg-devel \
        meson \
    ;

RUN dnf -y install git
RUN dnf -y install nasm gcc-c++ 

# Fetch and build mbedtls - note we are forcing -fPIC so we could build shared library for MistServer components later
RUN git clone --branch dtls_srtp_support https://github.com/livepeer/mbedtls.git /home/mbedtls
RUN cd /home/mbedtls; \
    cmake "-DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=true" "-DCMAKE_INSTALL_PREFIX=/usr" "-DCMAKE_INSTALL_LIBDIR=lib64" -S . -B /home/build.mbed; \
    cmake --build /home/build.mbed ; \
    cmake --install /home/build.mbed

# Fetch and build RIST
RUN git clone -b v0.2.7 https://code.videolan.org/rist/librist.git /home/rist; \
    dnf -y install cjson-devel libcmocka-devel; \
    meson setup -Dprefix=/usr -Dlibdir=lib64 -Dbuiltin_mbedtls=true -Ddefault_library=static /home/rist /home/build.rist; \
    meson compile -C /home/build.rist; \
    meson install -C /home/build.rist

# Ensure the termination happens on container stop, cgroup, starting init
WORKDIR /home/src/scripts/podbuilder
CMD make -fel9.make ctr-build

# For Debugging
#CMD ["/bin/bash"]

