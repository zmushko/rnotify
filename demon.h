#ifndef __APP_H
#define __APP_H

#include "log.h"
#include "config.h"

class Demon 
{
	private:
		Error	error;
		Debug	debug;	

		Config*	conf;
	
		void	printUsage();
	public:
		Demon(int count, char** values);
		~Demon();		
		
};


#endif /* __APP_H */

