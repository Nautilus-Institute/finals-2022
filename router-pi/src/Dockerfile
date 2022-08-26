FROM alpine:3.14

COPY rootfs/lib/ld-linux-armhf.so.3 /lib/
COPY rootfs/lib/arm-linux-gnueabihf /lib/arm-linux-gnueabihf

RUN mkdir /service
ADD httpd_arm /service/
ADD cert.crt /service/
ADD cert.key /service/
ADD static /service/static

ENTRYPOINT ["/service/httpd_arm", "-s"]
