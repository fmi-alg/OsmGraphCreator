FROM ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update
RUN apt-get --no-install-recommends -yq install git cmake build-essential ragel libprotobuf-lite17 libz-dev libcppunit-dev libicu-dev libprotobuf-dev protobuf-compiler libcrypto++-dev ca-certificates
RUN apt-get -yq install libcgal-dev
RUN cd /usr/src/ && git clone --recursive https://github.com/fmi-alg/OsmGraphCreator && cd OsmGraphCreator && git checkout f1dd5856754b0fa3a4c434598f4854cdf9b3a7f0
RUN mkdir /usr/src/OsmGraphCreator/build && cd /usr/src/OsmGraphCreator/build && cmake -DCMAKE_BUILD_TYPE=Release ../
RUN cd /usr/src/OsmGraphCreator/build && make -j3
