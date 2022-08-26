target/web4factory.jar:  ./src/main/java/institute/nautilus/web4factory/*.java  ./src/main/java/institute/nautilus/web4factory/*/*.java ./pom.xml src/main/resources/linux64/libfactory.so
	./mvnw package -Dmaven.test.skip=true

JAVA_HOME=/usr/lib/jvm/java-11-openjdk-amd64/

src/main/resources/linux64/libfactory.so: src/main/c/* src/main/java/institute/nautilus/web4factory/LibFactory.java
	javac -h src/main/c/ ./src/main/java/institute/nautilus/web4factory/LibFactory.java
	clang -c -fPIC -I${JAVA_HOME}/include -I$(JAVA_HOME)/include/linux ./src/main/c/libfactory.c -o ./target/libfactory.o
	set -x && sed "`nm target/libfactory.o --defined-only | grep ' T' | grep -v 'Java' | awk '{"xxd -c 2 -p /dev/urandom |uniq" |getline r; printf "s/\\\\([^[:alpha:]]\\\\)"$$3"(/\\\\1sub_"r"(/g;" }'`" ./src/main/c/libfactory.c > ./src/main/c/libfactory_enc.c
	clang -c -fPIC -I${JAVA_HOME}/include -I$(JAVA_HOME)/include/linux ./src/main/c/libfactory_enc.c -o ./target/libfactory.o
	mkdir ./src/main/resources/linux64 -p
	clang -shared -fPIC -g -o ./src/main/resources/linux64/libfactory.so ./target/libfactory.o -lc


.PHONY: run
run: target/web4factory.jar
	java -jar ./target/web4factory.jar

.PHONY: test
test:
	./mvnw test

