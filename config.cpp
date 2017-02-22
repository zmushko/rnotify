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
	m_all(false),
	m_verbose(2), 
	m_pidfile_path("/var/run"),
	m_logfile_path(""),
	m_no_demon(false),
	m_path_to_scripts("/etc/rnotifyd"),
	m_exclude(),
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

bool Config::getAll()
{
	return m_all;
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

string Config::getPidfile()
{
	return m_pidfile_path + "/" + m_my_name + ".pid";
}

string Config::getLogfilePath()
{
	return m_logfile_path;
}

uint32_t Config::getMask()
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

char** Config::getWatch()
{
	return &m_watch[0];
}

void Config::readMask()
{
	if (m_all)
	{
		m_mask = IN_ALL_EVENTS;
		return;
	}

	if (m_path_to_scripts.empty())
	{
		m_mask = 0;
		return;
	}

	DIR* dp	= opendir(m_path_to_scripts.c_str());
	if (dp == NULL)
	{
		fatal << " Nothing to do in " << m_path_to_scripts.c_str() << "/. Please follow instructions for -s argument or use -a argument." << std::endl; 
		m_need_help = true;
		return;
	}

	long name_max = pathconf(m_path_to_scripts.c_str(), _PC_NAME_MAX);
	if (name_max == -1)
	{
		name_max = 255;
	}
	
	size_t dirent_sz = offsetof(struct dirent, d_name) + name_max;
	struct dirent* entry = new struct dirent[dirent_sz + 1];
	if (entry == NULL)
	{
		fatal << " Can't get entry from " << m_path_to_scripts.c_str() << "." << std::endl; 
		m_need_help = true;
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

		string script = m_path_to_scripts + "/" + result->d_name;

		if (!strcmp(result->d_name, "IN_ACCESS") && !access(script.c_str(), X_OK))
		{
			m_mask |= IN_ACCESS;
		}

		if (!strcmp(result->d_name, "IN_ATTRIB") && !access(script.c_str(), X_OK))
		{
			m_mask |= IN_ATTRIB;
		}

		if (!strcmp(result->d_name, "IN_CLOSE_WRITE") && !access(script.c_str(), X_OK))
		{
			m_mask |= IN_CLOSE_WRITE;
		}

		if (!strcmp(result->d_name, "IN_CLOSE_NOWRITE") && !access(script.c_str(), X_OK))
		{
			m_mask |= IN_CLOSE_NOWRITE;
		}

		if (!strcmp(result->d_name, "IN_CREATE") && !access(script.c_str(), X_OK))
		{
			m_mask |= IN_CREATE;
		}

		if (!strcmp(result->d_name, "IN_DELETE") && !access(script.c_str(), X_OK))
		{
			m_mask |= IN_DELETE;
		}

		if (!strcmp(result->d_name, "IN_DELETE_SELF") && !access(script.c_str(), X_OK))
		{
			m_mask |= IN_DELETE_SELF;
		}

		if (!strcmp(result->d_name, "IN_MODIFY") && !access(script.c_str(), X_OK))
		{
			m_mask |= IN_MODIFY;
		}

		if (!strcmp(result->d_name, "IN_MOVE_SELF") && !access(script.c_str(), X_OK))
		{
			m_mask |= IN_MOVE_SELF;
		}

		if (!strcmp(result->d_name, "IN_OPEN") && !access(script.c_str(), X_OK))
		{
			m_mask |= IN_OPEN;
		}

		if (!strcmp(result->d_name, "IN_RENAME") && !access(script.c_str(), X_OK))
		{
			m_catch_rename = true;
		}
	}

	if (!m_mask && !m_catch_rename)
	{
		m_need_help = true;
	}

	delete entry;
	closedir(dp);
}

void Config::readOpts(int count, char** values)
{
	const char* opts = "av:p:l:dhw:s:e:t:zu";
	char opt = 0;

	while (-1 != (opt = getopt(count, values, opts)))
	{
		switch (opt)
		{
			case 'a':
				m_all = true;
				m_no_demon = true;
				break;
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

	Debug::Init(m_verbose, m_no_demon, m_logfile_path);

	char** watch = getWatch();
	if (watch[0] == NULL)
	{
		fatal << " At least one watched directory should be present. Look at -w argument please." << std::endl; 
		m_need_help = true;
	}
}

