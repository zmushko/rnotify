#ifndef __DEMON_H
#define __DEMON_H

#include "debug.h"
#include "config.h"

class Demon 
{
	private:
		Error	error;
		Debug	debug;
		Info	info;

		Config*	conf;
		void	printUsage();
		void	initDemon();
		void	stopDemon();
		void	setSigactions();
		void	runObserver();
		void	spawnChild(const char* path, const char* name);

	public:
		Demon(int count, char** values);
		~Demon();		
};


#endif /* __DEMON_H */
