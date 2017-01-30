CC = g++
CFLAGS = -g -rdynamic

all : librnotify
	cd librnotify && make all 
	$(CC) ${CFLAGS} -c debug.cpp 
	$(CC) ${CFLAGS} -c config.cpp 
	$(CC) ${CFLAGS} -c demon.cpp 
	$(CC) ${CFLAGS} -o rnotifyd main.cpp config.o demon.o debug.o

librnotify : 
	git clone https://github.com/zmushko/librnotify.git

git : librnotify
	cd librnotify && git pull
	make all

clean :
	-rm *.o
	-rm rnotifyd

clean_all :
	-rm *.o
	-rm rnotifyd
	-rm -rf librnotify
