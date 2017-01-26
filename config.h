#ifndef __CONFIG_H
#define __CONFIG_H

#include <vector>
#include "debug.h"

class Config
{
	public:
		Config(int count, char** values);
		~Config();
		
		string	getMyName();
		bool	getNeedHelp();
		bool	getCatchRename();
		int	getMask();
		bool	getEnableSuppressor();
		bool	getSkipZeroFile();
		int	getHearbeat();
		string	getExclude();
		string	getPathToScripts();
		bool	getNoDemon();
		string	getPidfilePath();
		string	getLogfilePath();
		int	getVerbose();
		
	private:
		Debug	m_debug;
		vector	<char*> m_watch;
		int	m_verbose;
		string	m_pidfile_path;
		string	m_logfile_path;
		bool	m_no_demon;
		string	m_path_to_scripts;
		string	m_exclude;
		int	m_heartbeat;
		bool	m_skip_zero_file;
		bool	m_enable_supressor;
		int	m_mask;
		bool	m_catch_rename;
		bool	m_need_help;
		string	m_my_name;
		
		void	readOpts(int count, char** values);
		void	readMask();
};

#endif /* __CONFIG_H */
