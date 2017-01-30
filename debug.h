#ifndef __DEBUG_H
#define __DEBUG_H

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

namespace Logging
{
	class LoggerStream
	{
		protected:
			string verboseToString(int verbose);
			int verboseToInt(int verbose);

		public:
			virtual void Print(int verbose, string message) = 0;
	};
	
	class LoggerConsole : public LoggerStream
	{
		public:
			void Print(int verbose, string message);
	};

	class LoggerFile : public LoggerStream
	{
		private:
			ofstream file;
		
		public:
			LoggerFile();
			~LoggerFile();
			void Print(int verbose, string message);
	};
	
	class LoggerSyslog : public LoggerStream
	{
		public:
			void Print(int verbose, string message);
	};
		
	class Logger
	{
		private:
			Logger();
			~Logger();
			Logger(const Logger&);
			Logger& operator =(const Logger&);

			int	 conf_verbose;
			
			LoggerConsole	console;
			LoggerFile	file;
			LoggerSyslog	sysLog;
			
		public:	
			enum {UNKNOWN = 0, FATAL, ERROR, WARN, INFO, DEBUG, TRACE, ALL};

			static Logger& Instance();
			void enableConsole(bool flag);
			void enableFile(string path);
			void verboseLevel(int level);
			void printMessage(int verbose, string message);
			static string printLegenda(string const& prefix);
	};

} // namespace Logger

class Debug
{
	protected: 
		ostringstream message;
		int verbose;
	public:
		Debug() : message("")
		{
			verbose = Logging::Logger::DEBUG;
		}

		template <class Type> Debug& operator <<(Type msg)
		{
			message << msg;
		}

		void operator <<(ostream& (*f)(ostream&))
		{
			Logging::Logger::Instance().printMessage(verbose, message.str());
		}
		
		static string Legenda(string const& prefix)
		{
			return Logging::Logger::printLegenda(prefix);
		}
		
		static void Init(int verbose, bool console, string path)
		{
			Logging::Logger::Instance().verboseLevel(verbose);
			Logging::Logger::Instance().enableConsole(console);
			Logging::Logger::Instance().enableFile(path);
		}
};

class Fatal : public Debug
{
	public:
		Fatal() : Debug()
		{
			verbose = Logging::Logger::FATAL;
		}
};

class Error : public Debug
{
	private:
		string error;
	public:
		Error() : Debug()
		{
			verbose = Logging::Logger::ERROR;
		}
};

class Info : public Debug
{
	public:
		Info() : Debug()
		{
			verbose = Logging::Logger::INFO;
		}
};

class Warning : public Debug
{
	public:
		Warning() : Debug()
		{
			verbose = Logging::Logger::WARN;
		}
};

class Trace : public Debug
{
	public:
		Trace() : Debug()
		{
			verbose = Logging::Logger::TRACE;
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

#endif /* __DEBUG_H */
