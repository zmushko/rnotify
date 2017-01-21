#ifndef __CONFIG_H
#define __CONFIG_H

#include <vector>

class Config 
{
	private:
		vector <char*> m_watch;
		int	m_verbose;
		string	m_pid;
		bool	m_no_demon;
		string	m_path_to_scripts;
		string	m_exclude;
		int	m_heartbeat;
		bool	m_skip_zero_file;
		bool	m_enable_supressor;
		int	m_mask;
		bool	m_in_rename;
	public:
			Config(int count, char** values);
			~Config();
		void	printUsage(const char* name);
	private:
		void	readMask();
};

#endif /* __CONFIG_H */


