#include <iostream>
#include <exception>
#include <stdexcept>

using namespace std;

#include "config.h"
#include "demon.h"

Demon::Demon(int count, char** values)
{
	conf = new Config(count, values);
	
	if (conf->getNeedHelp())
	{
		printUsage();
		return;
	}

	//Error("wwwwwwwwwwww");

	//throw Error("wwwwwwwwwwww");
}

Demon::~Demon()
{
	delete conf;
}

void Demon::printUsage()
{
	cout << endl 
		<< "Usage: " << conf->getMyName() << " [-options] [-w path1 -w path2 ...]" << endl 
		<< "\twhere: path1, path2, ... - path to notified folders" << endl
		<< endl
		<< "Options:" << endl 
		<< "\t-v [verbose level 0-7] default: 0, the possible value for the verbose are:" << endl
		<< Debug::Legenda("\t   ") << endl
		<< "\t-p [path to pid file] default: /var/run" << endl
		<< "\t-l [path to log file] by default syslog only used" << endl
		<< "\t-s [path to executible scripts per notifications], default: /etc/rnotifyd " << endl
		<< "\t   each script should have the same name as name of notification for ex. IN_DELETE and be executable," << endl
		<< "\t   scripts itself will assigned to his appropriated events automatically, and receives full path of the item as first argument," << endl
		<< "\t   available scripts names is: IN_ACCESS, IN_ATTRIB, IN_CLOSE_WRITE, IN_CLOSE_NOWRITE, IN_CREATE" << endl
		<< "\t   IN_DELETE, IN_DELETE_SELF, IN_MODIFY, IN_MOVE_SELF, IN_MOVED_FROM, IN_MOVED_TO, IN_OPEN and" << endl
		<< "\t   IN_RENAME - is unique notification name will receives two arguments as `from` and `to`" << endl
		<< "\t-u suppress notifications after scripts above in case if script produce itself notification" << endl
		<< "\t-e [exclude] - regular expression to define patterns that excluded from watching, for ex: ^\\." << endl
		<< "\t-t [heartbeat] - timeout for IN_MOVED_FROM/IN_MOVED_TO when it will converted to IN_DELETE/IN_CREATE in miliseconds" << endl
		<< "\t-z skip files with zero length" << endl
		<< "\t-d no daemon mode (log will print to stdout)" << endl
		<< "\t-h this message" << endl
		<< endl;
}

