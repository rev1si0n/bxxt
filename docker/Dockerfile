FROM debian:stretch-slim

ENV ANDROID_NDK_ROOT=/opt/android-ndk-r20
ENV NDK_DOWNLOAD_URL=https://dl.google.com/android/repository/android-ndk-r20-linux-x86_64.zip
RUN cd /tmp \
                        && apt update \
                        && apt upgrade -y \
                        && apt install -y wget curl unzip make file \
                        && wget ${NDK_DOWNLOAD_URL} -O ndk.zip \
                        && unzip -q ndk.zip -d /opt \
                        && rm ndk.zip
WORKDIR /src
CMD bash
