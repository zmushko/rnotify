#ifndef __LOG_H
#define __LOG_H

#include <iostream>
#include <fstream>

using namespace std;

class Log
{
	private:
		Log() : log_verbose(0), log_console(false), log_file("")
		{
		}

		~Log()
		{
		}

		Log(const Log&);
		Log& operator =(const Log&);

		int	 log_verbose;
		bool	 log_console;
		string	 log_file;
		ofstream log_stream;
				
	public:	
		enum {UNKNOWN = 0, FATAL, ERROR, WARN, INFO, DEBUG, TRACE};

		static Log& Instance()
		{
			static Log theSingleTone;
			return theSingleTone;
		}
		
		void setFlagConsole(bool flag)
		{
			log_console = flag;
		}

		void setFilePath(string path)
		{
			log_file = path;
		}
		
		void setVerboseLevel(int level)
		{
			log_verbose = level;
		}
		
		void printMessage(string message, int verbose)
		{
			cout << verbose << ": " << log_verbose << endl;
			if (verbose <= log_verbose)
			{
				cout << verbose << ": " << message << endl;
			}
		}
};

class Debug
{
	protected: 
		string message;
		int verbose;
	public:
		Debug() : message("")
		{
			verbose = Log::ERROR;
		}
		
		Debug& operator <<(string str)
		{
			message += str;
		}
		
		typedef enum {} end_t;
		end_t end; 

		void operator <<(end_t e)
		{
			Log& log = Log::Instance(); 
			log.printMessage(message, verbose);
		}
};

class Error : public Debug
{
	public:
		Error() : Debug()
		{
			verbose = Log::DEBUG;
		}
};

#endif /* __LOG_H */
