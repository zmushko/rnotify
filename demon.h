#ifndef __DEMON_H
#define __DEMON_H

#include "debug.h"
#include "config.h"

class Demon 
{
	private:
		Debug	debug;
		Trace	trace;

		Config*	conf;
		void	printUsage();
	public:
		Demon(int count, char** values);
		~Demon();		
		
};


#endif /* __DEMON_H */
