
SRC := vm.cpp \
	   cpu.cpp \
	   parser.cpp

all: vm_ad vm_ad_debug

vm_ad: ${SRC}
	g++ -O2 ${SRC} -DAD -lcrypto -o vm_ad
	strip --strip-all vm_ad

vm_ad_debug: ${SRC}
	g++ -O0 -g -DAD -DDEBUG ${SRC} -lcrypto -o vm_ad_debug

run_ad: vm_ad
	cp vm_ad ../bundle/vm
	socat tcp-l:8888,reuseaddr,fork exec:"../bundle/wrapper.py"

clean:
	rm -f vm_ad vm_ad_debug
