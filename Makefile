all : librnotify
	cd librnotify && make all 
	g++ -g -c log.cpp 
	g++ -g -c config.cpp 
	g++ -g -c app.cpp 
	g++ -g -o rnotifyd main.cpp log.o config.o app.o

librnotify : 
	git clone https://github.com/zmushko/librnotify.git

git : librnotify
	cd librnotify && git pull
	make all

