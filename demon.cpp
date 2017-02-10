#include <iostream>
#include <exception>
#include <stdexcept>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/ioctl.h>

#include "sys/inotify.h"

#include "debug.h"
#include "config.h"
#include "demon.h"
#include "signal.h"
#include "rnotify.h"

static volatile sig_atomic_t	g_SIGTERM	= 0;
static volatile sig_atomic_t	g_SIGINT	= 0;
static volatile sig_atomic_t	g_SIGCHLD	= 0;

namespace DemonNs
{
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

	class pipeReader
	{
		private:
			int fd;

		public:
			pipeReader(int fd);
			void operator >>(std::string& respond);
	};

	pipeReader::pipeReader(int fd) : fd(fd)
	{
	}

	void pipeReader::operator >>(std::string& rval)
	{
		rval.clear();

		int status = 0;
		char buf[512] = {'\0', };

		while ((status = ::read(fd, buf, sizeof(buf))))
		{
			if (-1 == status)
			{
				if (errno == EINTR
					|| errno == EAGAIN)
				{
					errno = 0;
					continue;
				}
				_THROW_IF(true);
			}
			rval += std::string(buf);
		}
	}
}

Demon::Demon(int count, char** values)
{
	conf = new Config(count, values);
		
	if (conf->getNeedHelp())
	{
		printUsage();
		return;
	}

	setSigactions();
	initDemon();
	stopDemon();
	runObserver();
	stopDemon();
}

Demon::~Demon()
{
	delete conf;
}

void Demon::spawnChild(const char* path, const char* name)
{
	if (path == NULL || name == NULL)
	{
		debug << __FUNCTION__ << "(): arguments can not be NULL" << std::endl;
		return;
	}

	int fd1[2] = {0, };
	int fd2[2] = {0, };
	
	_THROW_IF(-1 == pipe(fd1));
	_THROW_IF(-1 == pipe(fd2));

	pid_t pid = fork();

	_THROW_IF(-1 == pid);
	
	if (pid)
	{
		info << "Child " << pid << " " << name << " " << path << std::endl;
		
		close(fd1[1]);
		close(fd2[1]);

		std::string err_respond;
		DemonNs::pipeReader p_err(fd2[0]);
		p_err >> err_respond;
		if (err_respond.length())
		{
			info << "Child " << pid << " E:" << err_respond << "." << std::endl;
		}

		std::string out_respond;
		DemonNs::pipeReader p_out(fd1[0]);
		p_out >> out_respond;
		if (out_respond.length())
		{
			info << "Child " << pid << " " << out_respond << "." << std::endl;
		}

		int status = 0;
		if (-1 != waitpid(pid, &status, 0))
		{
			if (!WIFEXITED(status))
			{
				if (WIFSIGNALED(status))
				{
					int term_stat = WTERMSIG(status);
					#ifdef WCOREDUMP
						int core_stat = WCOREDUMP(status);
						error << "Child " << pid << " terminated by signal " << term_stat << (core_stat ? ", Core dumped" : "") << std::endl;
					#else
						error << "Child " << pid << " terminated by signal " << term_stat << std::endl;
					#endif
				}
				else
				{
					error << "Child " << pid << " abnormally terminated" << std::endl;
				}
			}
			else
			{
				info << "Child " << pid << " status " << WEXITSTATUS(status) << std::endl;
			}
		}

		close(fd1[0]);
		close(fd2[0]);		
	}
	else
	{
		close(fd1[0]);
		close(fd2[0]);
		
		dup2(fd1[1], STDOUT_FILENO);
		dup2(fd2[1], STDERR_FILENO);

		std::string path_name = conf->getPathToScripts() + "/" + name;
		execlp(path_name.c_str(), name, path, NULL);
		
		exit(errno);
	}
}

void Demon::runObserver()
{
	char** path = conf->getWatch();

	Notify* ntf = initNotify(path, conf->getMask());
	_THROW_IF(ntf == NULL);

	for (;;)
	{
		if (g_SIGTERM || g_SIGINT)
		{
			break;
		}

		char* np	= NULL;
		uint32_t mask	= 0;
		uint32_t cookie	= 0;
		int r = waitNotify(ntf, &np, &mask, conf->getHearbeat() * 1e3, &cookie);
		if (r < 0)
		{
			if (errno == EINTR ||
				errno == EACCES )
			{
				error << " *** Ignored *** " << np << " " << std::endl;
				errno = 0;
				continue;
			}
			else if (errno == ENOSPC)
			{
				Fatal fatal;
				fatal << " Limit of /proc/sys/fs/inotify/max_user_watches was reached, please increase it and restart me." << std::endl;
			}
			else
			{
				error << "Error in waitNotify(" << np << ")" << std::endl;
			}			
			break;
		}

		const char* name = NULL;
		if (mask & IN_CLOSE_WRITE)
		{
			name = "IN_CLOSE_WRITE";
		}
				
		if (mask & IN_CLOSE_NOWRITE)
		{
			name = "IN_CLOSE_NOWRITE";
		}
				
		if (mask & IN_MODIFY)
		{
			name = "IN_MODIFY";
		}
		
		if (mask & IN_CREATE)
		{
			name = "IN_CREATE";
		}
		
		if (mask & IN_DELETE)
		{
			name = "IN_DELETE";
		}
		
		if (mask & IN_MOVED_FROM)
		{
			name = "IN_MOVED_FROM";
		}
		
		if (mask & IN_MOVED_TO)
		{
			name = "IN_MOVED_TO";
		}
		
		if (mask & IN_DELETE_SELF)
		{
			name = "IN_DELETE_SELF";
		}
		
		if (mask & IN_MOVE_SELF)
		{
			name = "IN_MOVE_SELF";
		}

		if (np && name)
		{
			spawnChild(np, name);
		}

		free(np);
	}

	freeNotify(ntf);
}

void Demon::setSigactions()
{
	struct sigaction sa;

	memset(&sa, 0, sizeof(struct sigaction));
	sa.sa_flags	= 0;
	sa.sa_handler	= DemonNs::sig_term_handler;
	sigemptyset(&sa.sa_mask);
	_THROW_IF(sigaction(SIGTERM, &sa, NULL) == -1);

	memset(&sa, 0, sizeof(struct sigaction));
	sa.sa_flags	= 0;
	sa.sa_handler	= DemonNs::sig_term_handler;
	sigemptyset(&sa.sa_mask);
	_THROW_IF(sigaction(SIGINT, &sa, NULL) == -1);

	memset(&sa, 0, sizeof(struct sigaction));
	sa.sa_flags = 0;
	sa.sa_handler = DemonNs::sig_chld_handler;
	sigemptyset(&sa.sa_mask);
	_THROW_IF(sigaction(SIGCHLD, &sa, NULL) == -1);
}

void Demon::initDemon()
{
	if (conf->getNoDemon())
	{
		return;
	}

	pid_t pid   = fork();
	_THROW_IF(pid == -1);
	if (pid)
	{
		exit(EXIT_SUCCESS);
	}
	setsid();
	signal(SIGHUP, SIG_IGN);

	pid = fork();
	_THROW_IF(pid == -1);
	if (pid)
	{
		std::ofstream pid_file(conf->getPidfile().c_str());
		if (!pid_file.is_open())
		{
			error << " Can't open pid file " << conf->getPidfile() << std::endl;
		}
		pid_file << pid << std::endl;
		pid_file.close();
		exit(EXIT_SUCCESS);
	}

	int i;
	chdir("/");
	umask(0);
	close(0);
	close(1);
	for (i = 3; i < 64; i++)
	{
		close(i);
	}
}

void Demon::stopDemon()
{
	const char* pid_file = conf->getPidfile().c_str();
	if (!access(pid_file, F_OK))
	{
		unlink(pid_file);
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
		<< "\t-l [path to log file] by default syslog only used, for big data you should always prefer own log file" << std::endl
		<< "\t-s [path to executible scripts per notifications], default: /etc/rnotifyd " << std::endl
		<< "\t   each script should have the same name as name of notification for ex. IN_DELETE and be executable," << std::endl
		<< "\t   scripts itself will assigned to his appropriated events automatically, and receives full path of the item as first argument," << std::endl
		<< "\t   available scripts names is: IN_ACCESS, IN_ATTRIB, IN_CLOSE_WRITE, IN_CLOSE_NOWRITE, IN_CREATE" << std::endl
		<< "\t   IN_DELETE, IN_DELETE_SELF, IN_MODIFY, IN_MOVE_SELF, IN_MOVED_FROM, IN_MOVED_TO, IN_OPEN and" << std::endl
		<< "\t   IN_RENAME - is unique notification name will receives two arguments as `from` and `to`" << std::endl
		<< "\t   IN_MOVED_FROM, IN_MOVED_TO - is notification has second argument 'cookie' that pairing them both" << std::endl
		<< "\t-u suppress notifications after scripts above in case if script produce itself notification" << std::endl
		<< "\t-e [exclude] - regular expression to define patterns that excluded from watching, for ex: ^\\." << std::endl
		<< "\t-t [heartbeat] - timeout for IN_MOVED_FROM/IN_MOVED_TO when it will converted to IN_DELETE/IN_CREATE in miliseconds" << std::endl
		<< "\t-z skip files with zero length" << std::endl
		<< "\t-d no daemon mode (log will print to stdout if no enabled -l option)" << std::endl
		<< "\t-h this message" << std::endl
		<< std::endl;
}

