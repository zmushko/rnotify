#ifndef __DEBUG_H
#define __DEBUG_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <mutex>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <syslog.h>
#include <execinfo.h>

#define WHERE	__FILE__ << ":" << __LINE__ << ":"
#define EXEPTION	__FILE__, __LINE__,

namespace Logging
{
	class Message
	{
		protected:
			std::ostringstream message;
			int verbose;
		
		public:
			std::string getMessage() const;
			int getVerbose() const;
	};
		
	class Logger
	{
		private:
			Logger();
			~Logger();
			Logger(const Logger&);
			Logger& operator =(const Logger&);

			int	conf_verbose;
			bool	conf_console;
			std::string	conf_pathfile;
			std::ofstream	m_fstream;
			std::mutex	m_mutex;
			
			std::string verboseToString(int verbose);
			int verboseToInt(int verbose);

		public:	
			enum {UNKNOWN = 0, FATAL, ERROR, WARN, INFO, DEBUG, TRACE, ALL};

			static Logger& Instance();
			void enableConsole(bool flag);
			void enableFile(std::string const& path);
			void verboseLevel(int level);
			void printMessage(const Message& message);
			static std::string printLegenda(std::string const& prefix);
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

		void operator <<(std::ostream& (*f)(std::ostream&))
		{
			Logging::Logger::Instance().printMessage(*this);
		}
		
		static std::string Legenda(std::string const& prefix)
		{
			return Logging::Logger::printLegenda(prefix);
		}
		
		static void Init(int verbose, bool console, std::string path)
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
		std::string error;
	public:
		Exception(std::string e) : error(e)
		{
		}
		
		Exception(const char* file, int line, std::string e)
		{
			std::ostringstream message;
			message << file << ":" << line << ":" << e;
			error = message.str();
		}
		
		std::string What()
		{
			return error;
		}
};

#endif /* __DEBUG_H */
