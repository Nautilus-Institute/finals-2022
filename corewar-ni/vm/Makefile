SRC := vm.c \
	   cpu.c \
	   vis.c

all: vm_debug vm vm_vis

vm_debug: ${SRC}
	gcc -O0 -g -DDEBUG -DVIS ${SRC} -lseccomp -o vm_debug

vm: ${SRC}
	gcc -O2 ${SRC} -lseccomp -o vm
	strip --strip-all vm

vm_vis: ${SRC}
	gcc -O2 ${SRC} -g -DVIS -o vm_vis

clean:
	rm -f vm vm_debug vm_vis
