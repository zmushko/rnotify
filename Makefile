all : librnotify
	cd librnotify && make all 
	g++ -g -c config.cpp 
	g++ -g -o rnotifyd main.cpp config.o 

librnotify : 
	git clone https://github.com/zmushko/librnotify.git

git : librnotify
	cd librnotify && git pull
	make all

