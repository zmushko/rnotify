#ifndef __APP_H
#define __APP_H

#include "config.h"

class Demon 
{
	private:
		Log*	log;
		Config*	m_conf;
		
		void	printUsage();
	public:
		App(int count, char** values);
		~App();		
		
};


#endif /* __APP_H */

