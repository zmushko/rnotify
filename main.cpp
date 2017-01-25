#include <iostream>
#include <exception>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <stddef.h>
#include <linux/inotify.h>

using namespace std;

#include "log.h"
#include "config.h"
#include "demon.h"

int main(int argc, char** argv)
{
	Debug	debug;
	
	try
	{
		Demon demon(argc, argv);
		debug << "Hello" << " world!!!" << debug.end;
	}
	catch(exception const& e)
	{
		cout << "ERROR exception: " << e.what() << endl;
	}
	
	return 0;
}

/*
	char** w = &watch[0];
	for (int i = 0; w[i]; ++i)
	{
		cout << "W:" << w[i] << endl;
	}
*/
