#include <iostream>
#include <exception>
#include <stdexcept>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "debug.h"
#include "config.h"
#include "demon.h"
#include "signal.h"

static volatile sig_atomic_t	g_SIGTERM	= 0;
static volatile sig_atomic_t	g_SIGINT	= 0;
static volatile sig_atomic_t	g_SIGCHLD	= 0;

static void sig_term_handler(int sig)
{
	(void)sig;
	g_SIGTERM = 1;
}

static void sig_chld_handler(int sig)
{
	(void)sig;
	g_SIGCHLD = 1;
}

Demon::Demon(int count, char** values)
{
	conf = new Config(count, values);
		
	if (conf->getNeedHelp())
	{
		printUsage();
		return;
	}

	info << "Start" << std::endl;

	setSigactions();
	initDemon();

}

Demon::~Demon()
{
	delete conf;
}

void Demon::setSigactions()
{
	struct sigaction sa;

	memset(&sa, 0, sizeof(struct sigaction));
	sa.sa_flags	= 0;
	sa.sa_handler	= sig_term_handler;
	sigemptyset(&sa.sa_mask);
	THROW_IF(sigaction(SIGTERM, &sa, NULL) == -1);

	memset(&sa, 0, sizeof(struct sigaction));
	sa.sa_flags	= 0;
	sa.sa_handler	= sig_term_handler;
	sigemptyset(&sa.sa_mask);
	THROW_IF(sigaction(SIGINT, &sa, NULL) == -1);

	memset(&sa, 0, sizeof(struct sigaction));
	sa.sa_flags = 0;
	sa.sa_handler = sig_chld_handler;
	sigemptyset(&sa.sa_mask);
	THROW_IF(sigaction(SIGCHLD, &sa, NULL) == -1);
}

void Demon::initDemon()
{
	pid_t pid   = fork();
	THROW_IF(pid == -1);
	if (pid)
	{
		exit(EXIT_SUCCESS);
	}
	setsid();
	signal(SIGHUP, SIG_IGN);

	pid = fork();
	THROW_IF(pid == -1);
	if (pid)
	{
		std::ofstream pid_file(conf->getPidfile().c_str());
		THROW_IF(!pid_file.is_open());
		pid_file << pid << std::endl;
		pid_file.close();
		exit(EXIT_SUCCESS);
	}

	int i;
	chdir("/");
	umask(0);
	for (i = 0; i < 64; i++)
	{
		close(i);
	}

}

void Demon::printUsage()
{
	std::cout << std::endl 
		<< "Usage: " << conf->getMyName() << " [-options] [-w path1 -w path2 ...]" << std::endl 
		<< "\twhere: path1, path2, ... - path to notified folders" << std::endl
		<< std::endl
		<< "Options:" << std::endl 
		<< "\t-v [verbose level 0-7] default: 2, the possible value for the verbose are:" << std::endl
		<< Debug::Legenda("\t   ") << std::endl
		<< "\t-p [path to pid file] default: /var/run" << std::endl
		<< "\t-l [path to log file] by default syslog only used" << std::endl
		<< "\t-s [path to executible scripts per notifications], default: /etc/rnotifyd " << std::endl
		<< "\t   each script should have the same name as name of notification for ex. IN_DELETE and be executable," << std::endl
		<< "\t   scripts itself will assigned to his appropriated events automatically, and receives full path of the item as first argument," << std::endl
		<< "\t   available scripts names is: IN_ACCESS, IN_ATTRIB, IN_CLOSE_WRITE, IN_CLOSE_NOWRITE, IN_CREATE" << std::endl
		<< "\t   IN_DELETE, IN_DELETE_SELF, IN_MODIFY, IN_MOVE_SELF, IN_MOVED_FROM, IN_MOVED_TO, IN_OPEN and" << std::endl
		<< "\t   IN_RENAME - is unique notification name will receives two arguments as `from` and `to`" << std::endl
		<< "\t-u suppress notifications after scripts above in case if script produce itself notification" << std::endl
		<< "\t-e [exclude] - regular expression to define patterns that excluded from watching, for ex: ^\\." << std::endl
		<< "\t-t [heartbeat] - timeout for IN_MOVED_FROM/IN_MOVED_TO when it will converted to IN_DELETE/IN_CREATE in miliseconds" << std::endl
		<< "\t-z skip files with zero length" << std::endl
		<< "\t-d no daemon mode (log will print to stdout)" << std::endl
		<< "\t-h this message" << std::endl
		<< std::endl;
}

