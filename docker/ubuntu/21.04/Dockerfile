FROM ubuntu:21.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update
RUN apt-get --no-install-recommends -yq install \
git cmake build-essential ragel ca-certificates \
libprotobuf-lite23 libz-dev libprotobuf-dev protobuf-compiler \
libcppunit-dev libicu-dev libcrypto++-dev libcgal-dev
RUN cd /usr/src/ && git clone --recursive https://github.com/fmi-alg/OsmGraphCreator && cd OsmGraphCreator && git checkout a66a70de2093ab574b78d44418aebd098a2fdf1b
RUN mkdir /usr/src/OsmGraphCreator/build && cd /usr/src/OsmGraphCreator/build && cmake -DCMAKE_BUILD_TYPE=Release ../
RUN cd /usr/src/OsmGraphCreator/build && make -j3
RUN cp /usr/src/OsmGraphCreator/build/creator/creator /usr/bin/creator
ENTRYPOINT [ "/bin/bash" ]
