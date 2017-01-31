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

class Debug;

namespace Logging
{
	class Message
	{
		protected:
			ostringstream message;
			int verbose;
		
		public:
			string getMessage();
			int getVerbose();
	};
		
	class Logger
	{
		private:
			Logger();
			~Logger();
			Logger(const Logger&);
			Logger& operator =(const Logger&);

			int	 conf_verbose;
			bool	 conf_console;
			string	 conf_pathfile;
			ofstream m_fstream;			
			
			string verboseToString(int verbose);
			int verboseToInt(int verbose);

		public:	
			enum {UNKNOWN = 0, FATAL, ERROR, WARN, INFO, DEBUG, TRACE, ALL};

			static Logger& Instance();
			void enableConsole(bool flag);
			void enableFile(string path);
			void verboseLevel(int level);
			void printMessage(Message* message);
			static string printLegenda(string const& prefix);
	};

} // namespace Logger

class Debug : public Logging::Message
{
	public:
		Debug()
		{
			verbose = Logging::Logger::DEBUG;
		}

		template <class Type> Debug& operator <<(Type msg)
		{
			message << msg;
		}

		void operator <<(ostream& (*f)(ostream&))
		{
			Logging::Logger::Instance().printMessage(this);
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
