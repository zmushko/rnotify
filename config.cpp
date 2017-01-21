#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <stddef.h>
#include <linux/inotify.h>

using namespace std;

#include "config.h"

Config::Config(int count, char** values) : 
	m_verbose(0), 
	m_pid("/var/run"),
	m_no_demon(false),
	m_path_to_scripts("/etc/rnotifyd"),
	m_exclude(""),
	m_heartbeat(250),
	m_skip_zero_file(false),
	m_enable_supressor(false),
	m_mask(0),
	m_in_rename(false)
{
	const char* opts	= "v:p:dhw:s:e:t:zu";
	char opt		= 0;

	while (-1 != (opt = getopt(count, values, opts)))
	{
		switch (opt)
		{
			case 'v':
				m_verbose = atoi(optarg);
				break;
			case 'p':
				m_pid = optarg;
				break;
			case 'd':
				m_no_demon = true;
				break;
			case 's':
				m_path_to_scripts = optarg;
				break;
			case 'e':
				m_exclude = optarg;
				break;
			case 't':
				m_heartbeat = atoi(optarg);
				break;
			case 'z':
				m_skip_zero_file = true;
				break;
			case 'u':
				m_enable_supressor = true;
				break;
			case 'w':
				m_watch.push_back(strdup(optarg));
				break;
			case 'h':
				printUsage(values[0]);
				break;
			default:
				break;
		}
	}
	m_watch.push_back(NULL);
	
	readMask();
}

Config::~Config()
{
}

/*
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
*/

void Config::readMask()
{
	if (!m_path_to_scripts.length())
	{
		m_mask = 0;
		return;
	}

	DIR* dp	= opendir(m_path_to_scripts.c_str());
	if (dp == NULL)
	{
		return;
	}

	size_t dirent_sz	= offsetof(struct dirent, d_name) + pathconf(m_path_to_scripts.c_str(), _PC_NAME_MAX);
	struct dirent* entry	= new struct dirent[dirent_sz + 1];
	if (entry == NULL)
	{
		closedir(dp);
		return;
	}

	char** lst = NULL;
	for (;;)
	{
		struct dirent* result = NULL;
		if (0 != readdir_r(dp, entry, &result)
			|| result == NULL)
		{
			break;
		}

                if (!strcmp(entry->d_name, ".")
			|| !strcmp(entry->d_name, ".."))
		{
                        continue;
		}

		if (!strcmp(result->d_name, "IN_ACCESS"))
		{
			m_mask |= IN_ACCESS;
		}

		if (!strcmp(result->d_name, "IN_ATTRIB"))
		{
			m_mask |= IN_ATTRIB;
		}

		if (!strcmp(result->d_name, "IN_CLOSE_WRITE"))
		{
			m_mask |= IN_CLOSE_WRITE;
		}

		if (!strcmp(result->d_name, "IN_CLOSE_NOWRITE"))
		{
			m_mask |= IN_CLOSE_NOWRITE;
		}

		if (!strcmp(result->d_name, "IN_CREATE"))
		{
			m_mask |= IN_CREATE;
		}

		if (!strcmp(result->d_name, "IN_DELETE"))
		{
			m_mask |= IN_DELETE;
		}

		if (!strcmp(result->d_name, "IN_DELETE_SELF"))
		{
			m_mask |= IN_DELETE_SELF;
		}

		if (!strcmp(result->d_name, "IN_MODIFY"))
		{
			m_mask |= IN_MODIFY;
		}

		if (!strcmp(result->d_name, "IN_MOVE_SELF"))
		{
			m_mask |= IN_MOVE_SELF;
		}

		if (!strcmp(result->d_name, "IN_MOVED_FROM"))
		{
			m_mask |= IN_MOVED_FROM;
		}

		if (!strcmp(result->d_name, "IN_MOVED_TO"))
		{
			m_mask |= IN_MOVED_TO;
		}

		if (!strcmp(result->d_name, "IN_OPEN"))
		{
			m_mask |= IN_OPEN;
		}

		if (!strcmp(result->d_name, "IN_RENAME"))
		{
			m_in_rename = true;
		}
	}

	delete entry;
	closedir(dp);
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
