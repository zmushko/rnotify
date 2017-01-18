#ifndef __CLASSES_H
#define __CLASSES_H

#include <vector>

class Config 
{
	protected:
		int verbose;
		string pid;
		bool no_demon;
		vector <char*> watch;
		string path_to_scripts;
		string exclude;
		int heartbeat;
		bool skip_zero_file;
		bool enable_supressor;
	public:
		Config(int count, char** values);
		~Config();
		void printUsage(const char* name);
		void printConfig();
};

class App : public Config 
{
	public:
		App(int count, char** values);
		~App();
};

#endif /* __CLASSES_H */

