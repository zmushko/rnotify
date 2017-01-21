#ifndef __APP_H
#define __APP_H

#include "config.h"

class App 
{
	private:
		Config*	m_conf;
		void	printUsage();
	public:
			App(Config* conf);
			~App();
		void	run();
};


#endif /* __APP_H */

