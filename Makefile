all : librnotify
	cd librnotify && make all 
	g++ -g -o rnotifyd main.cpp 

librnotify : 
	git clone https://github.com/zmushko/librnotify.git

git : librnotify
	cd librnotify && git pull
	make all

