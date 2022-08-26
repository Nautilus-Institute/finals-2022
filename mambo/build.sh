
gcc  -o mamboserver -Os -msse4.1 -Wall mambo.c -lssl -lcrypto
strip mamboserver
#clang  -Wall -o mamboclient -DCLIENT -g -Os -msse4.1 -fsanitize=address -fno-omit-frame-pointer mamboclient.c mambo.c -lssl -lcrypto
#clang  -Wall -o mamboserver -g -Os -msse4.1 -fsanitize=address -fno-omit-frame-pointer mambo.c -lssl -lcrypto
gcc  -o mamboclient -msse4.1 -Wall -DCLIENT mamboclient.c  mambo.c  -lssl -lcrypto
