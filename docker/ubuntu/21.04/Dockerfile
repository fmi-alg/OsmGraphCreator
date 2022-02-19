FROM ubuntu:21.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update
RUN apt-get --no-install-recommends -yq install \
	git cmake build-essential ragel ca-certificates \
	libprotobuf-lite23 libz-dev libprotobuf-dev protobuf-compiler \
	libcppunit-dev libicu-dev libcrypto++-dev libcgal-dev

RUN cd /usr/src/ \
	&& git clone --recursive https://github.com/fmi-alg/OsmGraphCreator \
	&& cd OsmGraphCreator \
	&& git checkout 7d962b31e9b2d684bf100b8fdf0e148c91c037b2

RUN mkdir /usr/src/OsmGraphCreator/build \
	&& cd /usr/src/OsmGraphCreator/build \
	&& cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INTERPROCEDURAL_OPTIMIZATION=TRUE -DCONFIG_CREATOR_COPY_TAGS=ON ../

RUN cd /usr/src/OsmGraphCreator/build \
	&& make -j3

RUN cp /usr/src/OsmGraphCreator/build/creator/creator /usr/bin/creator
ENTRYPOINT [ "/bin/bash" ]
