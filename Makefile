CC = g++ -std=c++11
CFLAGS = -g 

all : librnotify
	cd librnotify && make all 
	$(CC) ${CFLAGS} -c debug.cpp 
	$(CC) ${CFLAGS} -c config.cpp 
	$(CC) ${CFLAGS} -c demon.cpp 
	$(CC) ${CFLAGS} -o rnotifyd debug.o main.cpp config.o demon.o 

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
