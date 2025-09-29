FROM debian:11

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get -y update

RUN apt-get install -y \
        build-essential \
        libhidapi-dev \
        qtbase5-dev \
        file \
        wget \
        zip

ENV LINUXDEPLOYQT=linuxdeployqt-continuous-x86_64.AppImage

WORKDIR /opt

RUN wget --progress=dot:mega \
      https://github.com/probonopd/linuxdeployqt/releases/download/continuous/$LINUXDEPLOYQT \
 && chmod -v a+x $LINUXDEPLOYQT \
 && ./$LINUXDEPLOYQT --appimage-extract \
 && mv -v squashfs-root linuxdeployqt
