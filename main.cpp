#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

using namespace std;

#include "classes.h"

int main(int argc, char** argv)
{
	Config* conf = new Config(argc, argv);

	conf->printConfig();

	delete conf;

	return 0;
}

Config::Config(int count, char** values)
{
	verbose		= 0;
	pid		= "/var/run";
	no_demon	= false;
	path_to_scripts	= "/etc/rnotifyd";
	exclude		= "";
	heartbeat	= 250;
	skip_zero_file	= false;
	enable_supressor = false;

	const char* opts	= "v:p:dhw:x:e:t:zs";
	char opt		= 0;

	while (-1 != (opt = getopt(count, values, opts)))
	{
		switch (opt)
		{
			case 'v':
				verbose = atoi(optarg);
				break;
			case 'p':
				pid = optarg;
				break;
			case 'd':
				no_demon = true;
				break;
			case 'x':
				path_to_scripts = optarg;
				break;
			case 'e':
				exclude = optarg;
				break;
			case 't':
				heartbeat = atoi(optarg);
				break;
			case 'z':
				skip_zero_file = true;
				break;
			case 's':
				enable_supressor = true;
				break;
			case 'w':
				watch.push_back(optarg);
				break;
			case 'h':
				printUsage(values[0]);
				break;
			default:
				break;
		}
	}
}

Config::~Config()
{
}

void Config::printConfig()
{
	cout << endl << "Config:" << endl
		<< "pid=" << pid << endl
		<< "no_demon=" << no_demon << endl
		<< "verbose=" << verbose << endl;

	cout << endl << "Watched:" << endl;

	for (int i = 0; i < watch.size(); ++i)
	{
		cout << watch[i] << endl;
	}
}

void Config::printUsage(const char* name)
{
	cout << endl 
		<< "Usage: " << name << " [-options] [-w path1 -w path2 ...]" << endl 
		<< "\twhere: path1, path2, ... - path to notified folders" << endl
		<< endl
		<< "Options:" << endl 
		<< "\t-v [verbose level 0-2] default: 0 (only errors)" << endl
		<< "\t-p [path to pid file] default: /var/run" << endl
		<< "\t-x [path to executible scripts per notifications] - " << endl
		<< "\t   (each script should have the same name as name of notification for ex. IN_DELETE)," << endl
		<< "\t   each script will assigned to his event automatically, and receives full path of the item as argument," << endl
		<< "\t   availible scripts names is: IN_ACCESS, IN_ATTRIB, IN_CLOSE_WRITE, IN_CLOSE_NOWRITE, IN_CREATE" << endl
		<< "\t   IN_DELETE, IN_DELETE_SELF, IN_MODIFY, IN_MOVE_SELF, IN_MOVED_FROM, IN_MOVED_TO, IN_OPEN and" << endl
		<< "\t   IN_RENAME - is unique notification name will receives two arguments as `from` and `to`, default: /etc/rnotifyd" << endl
		<< "\t-s suppress notifications after scripts above in case if script produce itself notification" << endl
		<< "\t-e [exclude] - regular expression to define patterns that excluded from watching, for ex: ^\\." << endl
		<< "\t-t [heartbeat] - timeout for IN_MOVED_FROM/IN_MOVED_TO when it will converted to IN_DELETE/IN_CREATE in miliseconds" << endl
		<< "\t-z skip files with zero length" << endl
		<< "\t-h this message" << endl
		<< endl;
}

