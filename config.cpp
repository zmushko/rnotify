#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <stddef.h>
#include <linux/inotify.h>

using namespace std;

#include "debug.h"
#include "config.h"

Config::Config(int count, char** values) : 
	m_verbose(0), 
	m_pidfile_path("/var/run"),
	m_logfile_path(""),
	m_no_demon(false),
	m_path_to_scripts("/etc/rnotifyd"),
	m_exclude(""),
	m_heartbeat(250),
	m_skip_zero_file(false),
	m_enable_supressor(false),
	m_mask(0),
	m_catch_rename(false),
	m_need_help(false),
	m_my_name(values[0])
{	
	readOpts(count, values);
	readMask();
}

Config::~Config()
{
	vector <char*> :: iterator itr;
	for (itr = m_watch.begin(); itr != m_watch.end(); ++itr)
	{
		delete *itr;
	}
}

string Config::getMyName()
{
	return m_my_name;
}

bool Config::getNeedHelp()
{
	return m_need_help;
}

bool Config::getCatchRename()
{
	return m_catch_rename;
}

bool Config::getEnableSuppressor()
{
	return m_enable_supressor;
}

bool Config::getSkipZeroFile()
{
	return m_skip_zero_file;
}

bool Config::getNoDemon()
{
	return m_no_demon;
}

string Config::getExclude()
{
	return m_exclude;
}

string Config::getPathToScripts()
{
	return m_path_to_scripts;
}

string Config::getPidfilePath()
{
	return m_pidfile_path;
}

string Config::getLogfilePath()
{
	return m_logfile_path;
}

int Config::getMask()
{
	return m_mask;
}

int Config::getHearbeat()
{
	return m_heartbeat;
}

int Config::getVerbose()
{
	return m_verbose;
}


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
			m_catch_rename = true;
		}
	}

	delete entry;
	closedir(dp);
}

void Config::readOpts(int count, char** values)
{
	const char* opts	= "v:p:l:dhw:s:e:t:zu";
	char opt		= 0;

	while (-1 != (opt = getopt(count, values, opts)))
	{
		switch (opt)
		{
			case 'v':
				m_verbose = atoi(optarg);
				break;
			case 'p':
				m_pidfile_path = optarg;
				break;
			case 'l':
				m_logfile_path = optarg;
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
				m_need_help = true;
				break;
			default:
				break;
		}
	}
	m_watch.push_back(NULL);

	m_debug.Init(m_verbose, m_no_demon, m_logfile_path);
}
