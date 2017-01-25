all : librnotify
	cd librnotify && make all 
	g++ -g -c config.cpp 
	g++ -g -c demon.cpp 
	g++ -g -o rnotifyd main.cpp config.o demon.o

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
