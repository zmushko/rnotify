#ifndef __LOG_H
#define __LOG_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <syslog.h>
#include <execinfo.h>

using namespace std;

#define WHERE	__FILE__ << ":" << __LINE__ << ":"
#define WHERE__	__FILE__, __LINE__,

class Logger
{
	private:
		Logger() : conf_verbose(0), conf_console(false), conf_pathfile("")
		{
		}

		~Logger()
		{
			m_fstream.close();
		}

		Logger(const Logger&);
		Logger& operator =(const Logger&);

		int	 conf_verbose;
		bool	 conf_console;
		string	 conf_pathfile;
		ofstream m_fstream;
		
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

		static Logger& Instance()
		{
			static Logger theSingleTone;
			return theSingleTone;
		}
		
		void setEnableConsole(bool flag)
		{
			conf_console = flag;
		}

		void setLoggerfilePath(string path)
		{
			conf_pathfile = path;
			m_fstream.open(path.c_str(), ios_base::app);
		}
		
		void setVerboseLevel(int level)
		{
			conf_verbose = level;
		}
		
		void printMessage(string message, int verbose)
		{
			if (verbose <= conf_verbose)
			{
				int safe_errno = errno;
				ostringstream out_msg;
				
				if (verbose != TRACE)
				{
					out_msg << verboseToString(verbose) << ':';
				}
				out_msg << message;

				if (verbose >= INFO && safe_errno)
				{
					out_msg << " errno:'";
		
					char error_buf[256] = {'\0', };
					if ((_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600) && !_GNU_SOURCE)
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
				errno = safe_errno;
				
				if (verbose >= ALL)
				{
					void* array[100];
					size_t size = backtrace (array, 100);
					char** strings = backtrace_symbols (array, size);
					if (strings)
					{
						out_msg << endl << "Obtained " << size << " stack frames:" << endl;

						for (size_t i = 0; i < size; ++i)
						{
							out_msg << strings[i] << endl;
						}
						delete strings;			
					}
				}

				if (verbose <= ERROR && !conf_console)
				{
					cerr << out_msg.str() << endl;
				}
				else if (conf_console)
				{
					cout << out_msg.str() << endl;
				}

				if (m_fstream.is_open())
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
						m_fstream << datetime << ' ';
					}

					m_fstream << out_msg.str() << endl;
					m_fstream.flush();
				}

				syslog(verboseToInt(verbose), "%s", out_msg.str().c_str());
			}
		}
		
		string printLegenda(string const& prefix)
		{
			ostringstream legenda;
			legenda << prefix << "0 - An unknown messages that should always be logged." << endl 
				<< prefix << "1 - An unhandleable errors that results in a program crash." << endl
				<< prefix << "2 - A handleables error condition." << endl
				<< prefix << "3 - A warnings." << endl
				<< prefix << "4 - Generic (useful) information about system operation." << endl
				<< prefix << "5 - Low-level information for developers." << endl
				<< prefix << "6 - Tracing information(omit data and time in log file)." << endl
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
			verbose = Logger::DEBUG;
		}

		template <class Type> Debug& operator <<(Type msg)
		{
			message << msg;
		}

		void operator <<(ostream& (*f)(ostream&))
		{
			Logger::Instance().printMessage(message.str(), verbose);
		}
		
		static string Legenda(string const& prefix)
		{
			return Logger::Instance().printLegenda(prefix);
		}
		
		static void Init(int verbose, bool console, string path)
		{
			Logger::Instance().setVerboseLevel(verbose);
			Logger::Instance().setEnableConsole(console);
			Logger::Instance().setLoggerfilePath(path);
		}
};

class Fatal : public Debug
{
	public:
		Fatal() : Debug()
		{
			verbose = Logger::FATAL;
		}
};

class Error : public Debug
{
	private:
		string error;
	public:
		Error() : Debug()
		{
			verbose = Logger::ERROR;
		}
};

class Info : public Debug
{
	public:
		Info() : Debug()
		{
			verbose = Logger::INFO;
		}
};

class Warning : public Debug
{
	public:
		Warning() : Debug()
		{
			verbose = Logger::WARN;
		}
};

class Trace : public Debug
{
	public:
		Trace() : Debug()
		{
			verbose = Logger::TRACE;
		}
};

class Exception
{
	private:
		string error;
	public:
		Exception(string e) : error(e)
		{
		}
		
		Exception(const char* file, int line, string e)
		{
			ostringstream message;
			message << file << ":" << line << ":" << e;
			error = message.str();
		}
		
		string What()
		{
			return error;
		}
};

#endif /* __LOG_H */
