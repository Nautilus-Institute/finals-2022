
SRC := main.c \
	cookies.c \
	re.c \
	hash.c \
	dns_resolve.c \
	base32.c

HEADERS := common.h \
		   re.h \
		   hash.h \
		   dns_resolve.h \
		   base32.h

CFLAGS := -fno-stack-protector -Wno-format-security -Wno-unused-result -lssl -lcrypto

all: httpd_arm \
	httpd_arm_debug

httpd_arm_debug: ${SRC} ${HEADERS}
	# arm-linux-gnueabihf-gcc -g -DVULN ${CFLAGS} ${SRC} -o httpd_arm_debug
	cp *.c ../rootfs/
	cp *.h ../rootfs/
	sudo chroot ../rootfs/ ./qemu-arm-static -L /lib /usr/bin/gcc -g -DVULN -DDEBUG ${CFLAGS} ${SRC} -o httpd_arm_debug
	rm ../rootfs/*.c
	rm ../rootfs/*.h
	mv ../rootfs/httpd_arm_debug .
	sudo chown $$USER:$$USER httpd_arm_debug

httpd_arm: ${SRC} ${HEADERS}
	# arm-linux-gnueabihf-gcc -O2 -DVULN ${CFLAGS} ${SRC} -o httpd_arm
	cp *.c ../rootfs/
	cp *.h ../rootfs/
	sudo chroot ../rootfs/ ./qemu-arm-static -L /lib /usr/bin/gcc -O2 -DVULN ${CFLAGS} ${SRC} -o httpd_arm
	rm ../rootfs/*.c
	rm ../rootfs/*.h
	mv ../rootfs/httpd_arm .
	sudo chown $$USER:$$USER httpd_arm
	arm-linux-gnueabihf-strip --strip-all httpd_arm

run: httpd_arm
	cp httpd_arm ../rootfs/
	cp cert.* ../rootfs/
	cp -r static ../rootfs/
	# sudo chroot rootfs/ ./qemu-arm-static -strace -L /lib ./httpd_arm -p 8888
	sudo socat tcp-l:8888,reuseaddr,fork exec:"chroot ../rootfs/ ./qemu-arm-static -L /lib ./httpd_arm -s"

run_debug: httpd_arm_debug
	cp httpd_arm_debug ../rootfs/
	cp cert.* ../rootfs/
	cp -r static ../rootfs/
	# sudo chroot rootfs/ ./qemu-arm-static -strace -L /lib ./httpd_arm -p 8888
	sudo socat tcp-l:8888,reuseaddr,fork exec:"chroot ../rootfs/ ./qemu-arm-static -strace -L /lib ./httpd_arm_debug -s"

docker: httpd_arm
	docker build . -t router-pi

bundle: httpd_arm
	mkdir -p bundle/service
	cp httpd_arm bundle/service/
	cp cert.crt bundle/service/
	cp cert.key bundle/service/
	cp -r static bundle/service/
	cp -r rootfs/* bundle/

clean:
	rm -rf httpd_arm_debug \
		httpd_arm \
		bundle/

