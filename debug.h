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

#define THROW_IF(A) do{if(A)throw Exception(__FUNCTION__, __FILE__, __LINE__);}while(0)

namespace Logging
{
	class Message
	{
		protected:
			std::ostringstream message;
			std::string str_errno;
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
		static std::string Legenda(std::string const& prefix);
		
		static void Init(int verbose, bool console, std::string const& path);

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
		void addWhere(const char* file, int line)
		{
			error = std::string(file) + std::string(":") + std::to_string(line);
		}

	public:
		Exception(const char* e) : error(e)
		{
		}
		
		Exception(const char* file, int line)
		{
			addWhere(file, line);
		}

		Exception(const char* file, int line, std::string e)
		{
			addWhere(file, line);
			error += std::string(":") + e;
		}
		
		Exception(const char* function, const char* file, int line)
		{
			addWhere(file, line);
			error += std::string(" Exception in ") + std::string(function) + std::string("()");
		}
		
		std::string What()
		{
			return error;
		}
};

#endif /* __DEBUG_H */

