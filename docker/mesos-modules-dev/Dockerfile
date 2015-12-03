FROM ubuntu:15.10

ARG version=0.25.0
# Install Dependencies

RUN apt-get update -q --fix-missing
RUN apt-get -q -y install \
  build-essential                         \
  automake \
  autoconf                                \
  libtool \
  cmake                                   \
  wget                                    \
  git-core                                \
  make                                    \
  openjdk-7-jdk                           \
  unzip                                   \
  python-dev \
  python-boto \
  libcurl4-nss-dev \
  libsasl2-2 libsasl2-dev libsasl2-modules sasl2-bin \
  libapr1-dev \
  libsvn-dev    \
  libgoogle-glog-dev \
    maven \
    zlib1g-dev \
  --no-install-recommends

# Install the picojson headers
RUN wget https://raw.githubusercontent.com/kazuho/picojson/v1.3.0/picojson.h -O /usr/local/include/picojson.h

# Prepare to build Mesos
RUN mkdir -p /mesos
RUN mkdir -p /tmp

WORKDIR /mesos

# Clone Mesos (master branch)
RUN git clone git://git.apache.org/mesos.git /mesos
RUN git checkout $version
RUN git log -n 1

# Bootstrap
RUN ./bootstrap

# Configure
RUN mkdir build && cd build && ../configure --disable-java --disable-optimize --without-included-zookeeper

# Build Mesos
RUN cd build && make -j 2 install