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

# Prepare to build Mesos
RUN mkdir -p /mesos
RUN mkdir -p /tmp
RUN mkdir -p /usr/share/java/
RUN wget http://search.maven.org/remotecontent?filepath=com/google/protobuf/protobuf-java/2.5.0/protobuf-java-2.5.0.jar -O protobuf.jar
RUN mv protobuf.jar /usr/share/java/

WORKDIR /mesos

# Clone Mesos (master branch)
RUN git clone git://git.apache.org/mesos.git /mesos
RUN git checkout $version
RUN git log -n 1

# Bootstrap
RUN ./bootstrap

# Configure
RUN mkdir build && cd build && ../configure --disable-optimize --without-included-zookeeper

# Build Mesos
RUN cd build && make -j 2 install

# Install python eggs
RUN easy_install /mesos/build/src/python/dist/mesos.interface-*.egg
RUN easy_install /mesos/build/src/python/dist/mesos.native-*.egg
