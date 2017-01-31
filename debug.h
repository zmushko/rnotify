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
			void Print(int verbose);
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
			
		public:	
			enum {UNKNOWN = 0, FATAL, ERROR, WARN, INFO, DEBUG, TRACE, ALL};

			static Logger& Instance();
			void enableConsole(bool flag);
			void enableFile(std::string const& path);
			void setVerboseLevel(int level);
			int getVerboseLevel();
			void printCerr(std::string const& message);
			void printCout(std::string const& message);
			void printFile(std::string const& message);
			void printRaw(std::string const& message);
			static std::string printLegenda(std::string const& prefix);
			std::string verboseToString(int verbose);
			int verboseToInt(int verbose);
	};

} // namespace Logger

class Debug : public Logging::Message
{
	public:
		static std::string Legenda(std::string const& prefix)
		{
			return Logging::Logger::printLegenda(prefix);
		}
		
		static void Init(int verbose, bool console, std::string path)
		{
			Logging::Logger::Instance().setVerboseLevel(verbose);
			Logging::Logger::Instance().enableConsole(console);
			Logging::Logger::Instance().enableFile(path);
		}

		template <class Type> Debug& operator <<(Type msg)
		{
			message << msg;
		}
		
		void operator <<(std::ostream& (*f)(std::ostream&))
		{
			Print(Logging::Logger::DEBUG);
		}
};

class Error : public Logging::Message
{
	public:
		template <class Type> Error& operator <<(Type msg)
		{
			message << msg;
		}
		
		void operator <<(std::ostream& (*f)(std::ostream&))
		{
			Print(Logging::Logger::ERROR);
		}
};

class Info : public Logging::Message
{
	public:
		template <class Type> Info& operator <<(Type msg)
		{
			message << msg;
		}
		
		void operator <<(std::ostream& (*f)(std::ostream&))
		{
			Print(Logging::Logger::INFO);
		}
};

class Warning : public Logging::Message
{
	public:
		template <class Type> Warning& operator <<(Type msg)
		{
			message << msg;
		}
		
		void operator <<(std::ostream& (*f)(std::ostream&))
		{
			Print(Logging::Logger::WARN);
		}
};

class Trace : public Logging::Message
{
	public:
		template <class Type> Trace& operator <<(Type msg)
		{
			message << msg;
		}
		
		void operator <<(std::ostream& (*f)(std::ostream&))
		{
			Print(Logging::Logger::TRACE);
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
