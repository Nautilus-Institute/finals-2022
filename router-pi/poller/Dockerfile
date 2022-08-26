FROM ubuntu:22.04

RUN apt-get update -y
RUN apt-get install -y python3 python3-pip python3-dev git libssl-dev libffi-dev build-essential
RUN python3 -m pip install --upgrade requests
RUN adduser --no-create-home --disabled-password --gecos "" user

ADD poll.py /poll.py
ADD cert.crt /cert.crt
USER user

CMD ["/poller.py"]
