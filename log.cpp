#include "log.h"

Log::Log():
	m_cout(false),
	m_fout(false),
	m_verbose(0)
{
}

Log::~Log()
{
}

void Log::setConsoleOut()
{
	m_cout = true;
}

void Log::setFileOut()
{
	m_fout = true;
}

void Log::setVerbose(int verbose)
{
	m_verbose = verbose;
}

Log& Log::operator <<(string str)
{
	cerr << str;

	return *this;
}

Log& Log::operator <<(end_t e)
{
	cerr << endl;

	return *this;
}

