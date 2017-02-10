#include "debug.h"

namespace Logging
{
	Logger::Logger() : conf_verbose(0)
	{
	}

	Logger::~Logger()
	{
		m_fstream.close();
	}
	
	Logger& Logger::Instance()
	{
		static Logger theSingleTone;
		return theSingleTone;
	}
			
	void Logger::enableConsole(bool flag)
	{
		conf_console = flag;
	}

	void Logger::enableFile(std::string const& path)
	{
		conf_pathfile = path;
	}
	
	void Logger::setVerboseLevel(int level)
	{
		conf_verbose = level;
	}

	int Logger::getVerboseLevel()
	{
		return conf_verbose;
	}
	
	bool Logger::isConsoleEnabled()
	{
		return conf_console;
	}
	
	bool Logger::isLogFileEnabled()
	{
		return m_fstream.is_open();
	}

	void Logger::printCerr(std::string const& message)
	{
		if (!conf_console)
		{
			std::cerr << message << std::endl;
		}
	}
	
	void Logger::printCout(std::string const& message)
	{
		if (conf_console)
		{
			std::cout << message << std::endl;
		}
	}
	
	void Logger::printFile(std::string const& message)
	{
		if (!m_fstream.is_open())
		{
			m_fstream.open(conf_pathfile.c_str(), std::ios_base::app);
		}	
		
		if (m_fstream.is_open())
		{
			char datetime[256] = {'\0', };
			time_t t = time(NULL);
			struct tm tms;
			struct tm* tmp = localtime_r(&t, &tms);
			if (tmp)
			{
				strftime(datetime, sizeof(datetime), "%b %e %Y %H:%M:%S", tmp);
			}
			
			m_mutex.lock();
			m_fstream << datetime << ' ' << message << std::endl;
			m_mutex.unlock();
		}
	}

	void Logger::printRaw(std::string const& message)
	{
		if (m_fstream.is_open())
		{
			m_mutex.lock();
			m_fstream << message << std::endl;
			m_mutex.unlock();
		}
	}

	std::string Logger::printLegenda(std::string const& prefix)
	{
		std::ostringstream legenda;
		legenda << prefix << "0 - An unknown messages that should always be logged." << std::endl 
			<< prefix << "1 - An unhandleable errors that results in a program crash." << std::endl
			<< prefix << "2 - A handleables error condition." << std::endl
			<< prefix << "3 - A warnings." << std::endl
			<< prefix << "4 - Generic (useful) information about system operation." << std::endl
			<< prefix << "5 - Low-level information for developers." << std::endl
			<< prefix << "6 - Tracing information(omit data and time in log file)." << std::endl
			<< prefix << "7 - Raw information for developers.";

		return legenda.str();
	}

	std::string Logger::verboseToString(int verbose)
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
	
	int Logger::verboseToInt(int verbose)
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

	void Message::Print(int verbose)
	{
		if (verbose <= Logging::Logger::Instance().getVerboseLevel())
		{
			int safe_errno = errno;
			std::ostringstream out_msg;
			if (verbose != Logging::Logger::TRACE)
			{
				out_msg << Logging::Logger::Instance().verboseToString(verbose) << ':';
			}
			out_msg << message.str();
			if (verbose == Logging::Logger::ERROR && safe_errno)
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

			if (verbose >= Logging::Logger::ALL)
			{
				void* array[100];
				size_t size = backtrace (array, 100);
				char** strings = backtrace_symbols (array, size);
				if (strings)
				{
					out_msg << std::endl << "Obtained " << size << " stack frames:" << std::endl;

					for (size_t i = 0; i < size; ++i)
					{
						out_msg << strings[i] << std::endl;
					}
					delete strings;			
				}
			}

			if (verbose <= Logging::Logger::ERROR)
			{
				Logging::Logger::Instance().printCerr(out_msg.str());
			}

			if (verbose != Logging::Logger::TRACE)
			{
				Logging::Logger::Instance().printFile(out_msg.str());
			}
			else
			{
				Logging::Logger::Instance().printRaw(out_msg.str());
			}
			
			if (!Logging::Logger::Instance().isLogFileEnabled() || 
				(verbose > Logging::Logger::UNKNOWN && verbose <= Logging::Logger::ERROR))
			{
				Logging::Logger::Instance().printCout(out_msg.str());
			}

			if ((!Logging::Logger::Instance().isConsoleEnabled() && !Logging::Logger::Instance().isLogFileEnabled()) || 
				verbose <= Logging::Logger::ERROR)
			{
				syslog(Logging::Logger::Instance().verboseToInt(verbose), "%s", out_msg.str().c_str());
			}

			message.str("");
		}
	}
} 

std::string Debug::Legenda(std::string const& prefix)
{
	return Logging::Logger::printLegenda(prefix);
}

void Debug::Init(int verbose, bool console, std::string const& path)
{
	Logging::Logger::Instance().setVerboseLevel(verbose);
	Logging::Logger::Instance().enableConsole(console);
	Logging::Logger::Instance().enableFile(path);
}


