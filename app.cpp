#include <iostream>

using namespace std;

#include "config.h"
#include "app.h"

App::App(Config* conf)
{
	m_conf = conf;
}

App::~App()
{
}

void App::run()
{
	if (m_conf->getNeedHelp())
	{
		printUsage();
		return;
	}
}

void App::printUsage()
{
	cout << endl 
		<< "Usage: " << m_conf->getMyName() << " [-options] [-w path1 -w path2 ...]" << endl 
		<< "\twhere: path1, path2, ... - path to notified folders" << endl
		<< endl
		<< "Options:" << endl 
		<< "\t-v [verbose level 0-2] default: 0 (only errors)" << endl
		<< "\t-p [path to pid file] default: /var/run" << endl
		<< "\t-s [path to executible scripts per notifications] - " << endl
		<< "\t   (each script should have the same name as name of notification for ex. IN_DELETE and be executable)," << endl
		<< "\t   each script will assigned to his event automatically, and receives full path of the item as argument," << endl
		<< "\t   availible scripts names is: IN_ACCESS, IN_ATTRIB, IN_CLOSE_WRITE, IN_CLOSE_NOWRITE, IN_CREATE" << endl
		<< "\t   IN_DELETE, IN_DELETE_SELF, IN_MODIFY, IN_MOVE_SELF, IN_MOVED_FROM, IN_MOVED_TO, IN_OPEN and" << endl
		<< "\t   IN_RENAME - is unique notification name will receives two arguments as `from` and `to`, default: /etc/rnotifyd" << endl
		<< "\t-u suppress notifications after scripts above in case if script produce itself notification" << endl
		<< "\t-e [exclude] - regular expression to define patterns that excluded from watching, for ex: ^\\." << endl
		<< "\t-t [heartbeat] - timeout for IN_MOVED_FROM/IN_MOVED_TO when it will converted to IN_DELETE/IN_CREATE in miliseconds" << endl
		<< "\t-z skip files with zero length" << endl
		<< "\t-h this message" << endl
		<< endl;
}

