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
		Demon(int count, char** values);
		~Demon();		
		
};


#endif /* __APP_H */

