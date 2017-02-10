#ifndef __CONFIG_H
#define __CONFIG_H

#include <vector>
#include "debug.h"

class Config
{
	public:
		Config(int count, char** values);
		~Config();
		
		bool getNeedHelp();
		bool getCatchRename();
		bool getEnableSuppressor();
		bool getSkipZeroFile();
		int	getHearbeat();
		bool getNoDemon();
		int	getVerbose();
		char** getWatch();
		uint32_t getMask();
		std::string	getPidfile();
		std::string	getLogfilePath();
		std::string	getExclude();
		std::string	getPathToScripts();
		std::string	getMyName();
		
	private:
		Fatal fatal;
		Debug debug;
		int	m_verbose;
		bool m_no_demon;
		int	m_heartbeat;
		bool m_skip_zero_file;
		bool m_enable_supressor;
		int	m_mask;
		bool m_catch_rename;
		bool m_need_help;
		std::string	m_my_name;
		std::string	m_pidfile_path;
		std::string	m_logfile_path;
		std::string	m_path_to_scripts;
		std::string	m_exclude;
		std::vector<char*> m_watch;
		
		void readOpts(int count, char** values);
		void readMask();
};

#endif /* __CONFIG_H */
