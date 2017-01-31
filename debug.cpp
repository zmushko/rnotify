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

	void Logger::enableFile(std::string path)
	{
		m_fstream.open(path.c_str(), std::ios_base::app);
	}
	
	void Logger::verboseLevel(int level)
	{
		conf_verbose = level;
	}

	void Logger::printMessage(Message* message)
	{
		int verbose = message->getVerbose();

		if (verbose <= conf_verbose)
		{
			m_mutex.lock();
			int safe_errno = errno;
			std::ostringstream out_msg;
			
			if (verbose != TRACE)
			{
				out_msg << verboseToString(verbose) << ':';
			}
			out_msg << message->getMessage();

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
					out_msg << std::endl << "Obtained " << size << " stack frames:" << std::endl;

					for (size_t i = 0; i < size; ++i)
					{
						out_msg << strings[i] << std::endl;
					}
					delete strings;			
				}
			}

			if (verbose <= ERROR && !conf_console)
			{
				std::cerr << out_msg.str() << std::endl;
			}
			else if (conf_console)
			{
				std::cout << out_msg.str() << std::endl;
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

				m_fstream << out_msg.str() << std::endl;
				m_fstream.flush();
			}

			syslog(verboseToInt(verbose), "%s", out_msg.str().c_str());
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
		if	(verbose == Logger::UNKNOWN)	return "UNKNOWN";
		else if (verbose == Logger::FATAL)	return "FATAL";
		else if (verbose == Logger::ERROR)	return "ERROR";
		else if (verbose == Logger::WARN)	return "WARN";
		else if (verbose == Logger::INFO)	return "INFO";
		else if (verbose == Logger::DEBUG)	return "DEBUG";
		else if (verbose == Logger::TRACE)	return "TRACE";
		else if (verbose == Logger::ALL)	return "ALL";
	}
	
	int Logger::verboseToInt(int verbose)
	{
		if	(verbose == Logger::UNKNOWN)	return LOG_ALERT;
		else if (verbose == Logger::FATAL)	return LOG_CRIT;
		else if (verbose == Logger::ERROR)	return LOG_ERR;
		else if (verbose == Logger::WARN)	return LOG_WARNING;
		else if (verbose == Logger::INFO)	return LOG_INFO;
		else if (verbose == Logger::DEBUG)	return LOG_DEBUG;
		else if (verbose == Logger::TRACE)	return LOG_DEBUG;
		else if (verbose == Logger::ALL)	return LOG_NOTICE;
	}
	
	int Message::getVerbose()
	{
		return verbose;
	}

	std::string Message::getMessage()
	{
		return message.str();
	}

} 

