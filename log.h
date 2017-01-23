#ifndef __LOG_H
#define __LOG_H

#include <iostream>
#include <fstream>

using namespace std;

class Log
{
	private:
		bool	m_cout;
		bool	m_fout;
		int	m_verbose;
		
		string	m_line;
		string	m_path;
		
		ofstream	m_stream;
		
	public:	
		typedef enum {} end_t;
		end_t end; 
		
		Log();
		~Log();
		
		void setConsoleOut();
		void setFileOut();
		void setVerbose(int verbose);

		Log& operator <<(string);
		Log& operator <<(end_t);
};

#endif /* __LOG_H */

