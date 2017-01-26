#ifndef __APP_H
#define __APP_H

#include "debug.h"
#include "config.h"

class Demon 
{
	private:
		Debug	debug;	
		Config*	conf;
		void	printUsage();
	public:
		Demon(int count, char** values);
		~Demon();		
		
};


#endif /* __APP_H */

