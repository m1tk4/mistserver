FROM ubuntu:25.04 AS mist_build

ADD . /src/

RUN <<EOF 

apt-get update 

ln -fs /usr/share/zoneinfo/America/New_York /etc/localtime
DEBIAN_FRONTEND=noninteractive apt-get install -y tzdata locales
dpkg-reconfigure --frontend noninteractive tzdata
localedef -i en_US -c -f UTF-8 -A /usr/share/locale/locale.alias en_US.UTF-8

apt-get install -y git meson gcc g++
EOF

RUN <<EOF 
mkdir /build
mkdif /out
cd /build 
meson setup -Dprefix=/out /src 
ninja install
cp -P /build/subprojects/*/lib*.so* /out/lib/x86_64-linux-gnu/ 
strip /out/bin/* /out/lib/x86_64-linux-gnu/*.so*
EOF


FROM ubuntu:25.04
LABEL org.opencontainers.image.authors="Dimitri Tarassenko <mitka@mitka.us>"

COPY --from=mist_build /out/ /usr/
#RUN <<EOF
#    apt-get update 
#    ln -fs /usr/share/zoneinfo/America/New_York /etc/localtime
#    DEBIAN_FRONTEND=noninteractive apt-get install -y tzdata locales
#    dpkg-reconfigure --frontend noninteractive tzdata
#    localedef -i en_US -c -f UTF-8 -A /usr/share/locale/locale.alias en_US.UTF-8
#    apt clean
#EOF
ENV LANG=en_US.utf8
EXPOSE 4242 8080 1935 5554
ENTRYPOINT ["MistController"]