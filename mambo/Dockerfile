FROM ubuntu:22.04

RUN apt-get update && \
    apt-get upgrade -y && \
        apt-get install -y libssl3

ADD mamboclient /opt/mamboclient

CMD /opt/mamboclient