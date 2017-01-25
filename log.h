#ifndef __LOG_H
#define __LOG_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <syslog.h>

using namespace std;

#define WHERE __FILE__ << ":" << __LINE__

class Log
{
	private:
		Log() : log_verbose(0), log_console(false), log_file("")
		{
		}

		~Log()
		{
			log_fstream.close();
		}

		Log(const Log&);
		Log& operator =(const Log&);

		int	 log_verbose;
		bool	 log_console;
		string	 log_file;
		ofstream log_fstream;
		
		string verboseToString(int verbose)
		{
			if	(verbose == UNKNOWN)	return "UNKNOWN";
			else if (verbose == FATAL)	return "FATAL";
			else if (verbose == ERROR)	return "ERROR";
			else if (verbose == WARN)	return "WARN";
			else if (verbose == INFO)	return "INFO";
			else if (verbose == DEBUG)	return "DEBUG";
			else if (verbose == TRACE)	return "TRACE";
			else if (verbose == ALL)	return "ALL";
		}

		int verboseToInt(int verbose)
		{
			if	(verbose == UNKNOWN)	return LOG_ALERT;
			else if (verbose == FATAL)	return LOG_CRIT;
			else if (verbose == ERROR)	return LOG_ERR;
			else if (verbose == WARN)	return LOG_WARNING;
			else if (verbose == INFO)	return LOG_INFO;
			else if (verbose == DEBUG)	return LOG_DEBUG;
			else if (verbose == TRACE)	return LOG_DEBUG;
			else if (verbose == ALL)	return LOG_NOTICE;
		}

	public:	
		enum {UNKNOWN = 0, FATAL, ERROR, WARN, INFO, DEBUG, TRACE, ALL};

		static Log& Instance()
		{
			static Log theSingleTone;
			return theSingleTone;
		}
		
		void setEnableConsole(bool flag)
		{
			log_console = flag;
		}

		void setLogfilePath(string path)
		{
			log_file = path;
			log_fstream.open(path.c_str());
		}
		
		void setVerboseLevel(int level)
		{
			log_verbose = level;
		}
		
		void printMessage(string message, int verbose, int safe_errno)
		{
			if (verbose <= log_verbose)
			{
				ostringstream out_msg;
				
				if (verbose != TRACE)
				{
					out_msg << verboseToString(verbose) << ' ';
				}
				out_msg << message;

				if (verbose == ERROR || verbose == FATAL || verbose == UNKNOWN)
				{
					if (safe_errno)
					{
						char error_buf[256] = {'\0', };
						out_msg << " errno:'";
						
						if ((_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600) && ! _GNU_SOURCE)
						{
							strerror_r(safe_errno, error_buf, 256);
							out_msg << error_buf;
						}
						else
						{
							out_msg << strerror_r(safe_errno, error_buf, 256);
						}
						out_msg << "'";
					}
					(log_console ? cout : cerr) << out_msg.str() << endl;
				}
				else if (log_console)
				{
					cout << out_msg.str() << endl;
				}

				if (log_fstream.is_open())
				{
					if (verbose != TRACE)
					{
						char datetime[256] = {'\0', };
						time_t t = time(NULL);
						struct tm tms;
						struct tm* tmp = localtime_r(&t, &tms);
						if (tmp)
						{
							strftime(datetime, sizeof(datetime), "%b %e %Y %H:%M:%S", tmp);
						}
						log_fstream << datetime << ' ';
					}
					log_fstream << out_msg.str() << endl;
					log_fstream.flush();
				}
				syslog(verboseToInt(verbose), "%s", out_msg.str().c_str());
			}
		}
		
		const string printLegenda(string const& prefix)
		{
			ostringstream legenda;
			legenda << prefix << "0 - An unknown messages that should always be logged." << endl 
				<< prefix << "1 - An unhandleable errors that results in a program crash." << endl
				<< prefix << "2 - A handleables error condition." << endl
				<< prefix << "3 - A warnings." << endl
				<< prefix << "4 - Generic (useful) information about system operation." << endl
				<< prefix << "5 - Low-level information for developers." << endl
				<< prefix << "6 - Tracing information for developers." << endl
				<< prefix << "7 - Raw information for developers.";

			return legenda.str();
		}
};

class Debug
{
	protected: 
		ostringstream message;
		int verbose;
	public:
		Debug() : message("")
		{
			verbose = Log::DEBUG;
		}
		
		template <class Type> Debug& operator <<(Type msg)
		{
			message << msg;
		}

		void operator <<(ostream& (*f)(ostream&))
		{
			int safe_errno = errno;
			Log& log = Log::Instance(); 
			log.printMessage(message.str(), verbose, safe_errno);
			errno = safe_errno;
		}
};

class Error : public Debug
{
	public:
		Error() : Debug()
		{
			verbose = Log::ERROR;
		}
};

class Info : public Debug
{
	public:
		Info() : Debug()
		{
			verbose = Log::INFO;
		}
};

class Warning : public Debug
{
	public:
		Warning() : Debug()
		{
			verbose = Log::WARN;
		}
};

class Trace : public Debug
{
	public:
		Trace() : Debug()
		{
			verbose = Log::TRACE;
		}
};

#endif /* __LOG_H */
