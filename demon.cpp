#include <iostream>
#include <exception>
#include <stdexcept>
#include <list>
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

	class pendingEvents
	{
	private:

		struct Event
		{
			std::string path;
			uint32_t mask;
			uint32_t cookie;
			struct timespec mtime;
		};
		std::list<Event*> events;

	public:
		
		pendingEvents()
		{
		}

		~pendingEvents()
		{
			std::list<Event*> :: iterator i;
			for (i = events.begin(); i != events.end(); ++i)
			{
				delete *i;
				//std::cout << "<<<<< " << (*i)->path << std::endl; 
			}
		}

		void Push(std::string const& path, uint32_t mask, uint32_t cookie)
		{
			Event* event = new Event;

			event->path = path;
			event->mask = mask;
			event->cookie = cookie;

			if (!::access(path.c_str(), F_OK))
			{
				struct stat st;
				memset(&st, 0, sizeof(st));
				_THROW_IF(-1 == ::stat(path.c_str(), &st));

				event->mtime.tv_sec = st.st_mtim.tv_sec;
				event->mtime.tv_nsec = st.st_mtim.tv_nsec;
			}
			
			events.push_back(event);
		}

		bool pickPair(std::string& path, uint32_t mask, uint32_t cookie)
		{
			std::list<Event*> :: iterator i;
			for (i = events.begin(); i != events.end(); ++i)
			{
				if ((((*i)->mask & IN_MOVED_TO && mask & IN_MOVED_FROM) ||
					((*i)->mask & IN_MOVED_FROM && mask & IN_MOVED_TO)) &&
					cookie == (*i)->cookie)
				{
					path = (*i)->path;
					Event* e = *i;
					i = events.erase(i);
					delete e;

					return true;
				}
			}

			return false;
		}

		bool detectFlood(std::string const& path, uint32_t mask, int heartbeat)
		{
			if (-1 == ::access(path.c_str(), F_OK))
			{
				return false;
			}

			std::list<Event*> :: iterator i;
			for (i = events.begin(); i != events.end(); ++i)
			{
				if ((*i)->mask & IN_MODIFY && (*i)->path == path)
				{
					struct stat st;
					memset(&st, 0, sizeof(st));
					_THROW_IF(-1 == ::stat((*i)->path.c_str(), &st));

					long sec = st.st_mtim.tv_sec - (*i)->mtime.tv_sec;
					long nsec = (*i)->mtime.tv_nsec ? 1E9L - (*i)->mtime.tv_nsec + st.st_mtim.tv_nsec : st.st_mtim.tv_nsec;
					long delta = sec * 1E6L + nsec / 1E3L;

					if (delta <= heartbeat * 1E6L)
					{
						(*i)->mtime.tv_sec = st.st_mtim.tv_sec;
						(*i)->mtime.tv_nsec = st.st_mtim.tv_nsec;
						return true;
					}
					else
					{
						Event* e = *i;
						i = events.erase(i);
						delete e;

						return false;
					}
				}
			}
			
			Push(path, mask, 0);

			return false;
		}
		
		bool pickExpired(char** path, uint32_t* mask, int heartbeat)
		{
			struct timespec t = {0, 0};
			_THROW_IF(clock_gettime(CLOCK_REALTIME, &t));

			std::list<Event*> :: iterator i;
			for (i = events.begin(); i != events.end(); ++i)
			{
				long sec = t.tv_sec - (*i)->mtime.tv_sec;
				long nsec = (*i)->mtime.tv_nsec ? 1E9L - (*i)->mtime.tv_nsec + t.tv_nsec : t.tv_nsec;
				long delta = sec * 1E6L + nsec / 1E3L;

std::cout << "delta=" << delta << " heartbeat=" << heartbeat << "000000" << std::endl;

				if (delta > heartbeat * 1E6L)
				{
					*path = strdup((*i)->path.c_str());
					_THROW_IF(*path == NULL);
					*mask |= (*i)->mask;
					
					Event* e = *i;
					i = events.erase(i);
					delete e;

					return true;
				}
			}

			return false;
		}
	};
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
	runObserver();
	stopDemon();
}

Demon::~Demon()
{
	delete conf;
}

void Demon::spawnChild(const char* path, const char* name, const char* pair)
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
		execlp(path_name.c_str(), name, path, pair, NULL);
		
		exit(errno);
	}
}

void Demon::runObserver()
{
	char** path = conf->getWatch();

	Notify* ntf = initNotify(path, conf->getMask(), conf->getExclude().empty() ? NULL : conf->getExclude().c_str());
	if (ntf == NULL)
	{
		if (errno == ENOENT)
		{
			fatal << " Nothing to do since even one notified folder does not found, please check -w argument" << std::endl;
			return;
		}
		else
		{
			_THROW_IF(true);
		}
	}

	DemonNs::pendingEvents pending; 

	for (;;)
	{
		if (g_SIGTERM || g_SIGINT)
		{
			break;
		}

		char* np = NULL;
		uint32_t mask = 0;
		uint32_t cookie	= 0;
		//if (false == pending.pickExpired(&np, &mask, conf->getHearbeat()))
		//{
			int r = waitNotify(ntf, &np, &mask, conf->getHearbeat() * 1E3L, &cookie);
			if (r < 0)
			{
				if (errno == EACCES)
				{
					error << " *** Ignored *** " << np << " " << std::endl;
					errno = 0;
					continue;
				}
				else if (errno == EINTR)
				{
					warning << "Interrupted system call" << std::endl;
					errno = 0;
					continue;
				}
				else if (errno == ENOSPC)
				{
					fatal << " Limit of /proc/sys/fs/inotify/max_user_watches was reached, please increase it and restart me." << std::endl;
				}
				else
				{
					error << "Error in waitNotify(" << np << ")" << std::endl;
				}			
				break;
			}
		//}

		//if (np == NULL)
		//{
		//	continue;
		//}

		std::string pair;

		const char* name = NULL;
		if (mask & IN_ACCESS)
		{
			name = "IN_ACCESS";
		}

		if (mask & IN_ATTRIB)
		{
			name = "IN_ATTRIB";
		}

		if (mask & IN_CLOSE_WRITE)
		{
			name = "IN_CLOSE_WRITE";
		}

		if (mask & IN_CLOSE_NOWRITE)
		{
			name = "IN_CLOSE_NOWRITE";
		}
				
		if (mask & IN_CREATE)
		{
			name = "IN_CREATE";
		}
		
		if (mask & IN_DELETE)
		{
			name = "IN_DELETE";
		}
		
		if (mask & IN_DELETE_SELF)
		{
			name = "IN_DELETE_SELF";
		}

		if (mask & IN_MODIFY)
		{
			if (false == pending.detectFlood(np, mask, conf->getHearbeat()))
			{
				name = "IN_MODIFY";
			}
		}

		if (mask & IN_MOVE_SELF)
		{
			name = "IN_MOVE_SELF";
		}
		
		if (mask & IN_MOVED_FROM)
		{
			if (pending.pickPair(pair, mask, cookie))
			{
				name = "IN_RENAME";
			}
			else
			{
				pending.Push(np, mask, cookie);
			}
		}
		
		if (mask & IN_MOVED_TO)
		{
			if (pending.pickPair(pair, mask, cookie))
			{
				name = "IN_RENAME";
			}
			else
			{
				pending.Push(np, mask, cookie);
			}
		}
		
		if (mask & IN_OPEN)
		{
			name = "IN_OPEN";
		}
		
		if (np && name)
		{
			if (!conf->getAll())
			{
				spawnChild(np, name, pair.empty() ? NULL : pair.c_str());
			}
			else
			{
				if (!pair.empty())
				{
					std::cout << "IN_RENAME_FROM:" << pair << "\n" << "IN_RENAME_TO:" << np << std::endl;
				}
				else
				{
					std::cout << name << ":" << np << std::endl;
				}
			}
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
		<< "\t-a mode that will print notifications to stdout (this will skip -s option and enable -d option)" << std::endl
		<< "\t   output format: <notification>:<cookie>:<path>" << std::endl
		<< "\t-v [verbose level 0-7] default: 2, the possible value for the verbose are:" << std::endl
		<< Debug::Legenda("\t   ") << std::endl
		<< "\t-p [path to pid file] default: /var/run" << std::endl
		<< "\t-l [path to log file] by default syslog only used, for big data you should always prefer own log file" << std::endl
		<< "\t-s [path to executible scripts per notifications], default: /etc/rnotifyd " << std::endl
		<< "\t   each script should have the same name as name of notification for ex. IN_DELETE and be executable," << std::endl
		<< "\t   scripts itself will assigned to his appropriated events automatically, and receives full path of the item as first argument," << std::endl
		<< "\t   available scripts names is: IN_ACCESS, IN_ATTRIB, IN_CLOSE_WRITE, IN_CLOSE_NOWRITE, IN_CREATE" << std::endl
		<< "\t   IN_DELETE, IN_DELETE_SELF, IN_MODIFY, IN_MOVE_SELF, IN_OPEN and" << std::endl
		<< "\t   IN_RENAME - is unique notification name will receives two arguments as `from` and `to`" << std::endl
		<< "\t-u suppress notifications after scripts above in case if script produce itself notification" << std::endl
		<< "\t-e [exclude] - regular expression to define patterns that excluded from watching, for ex: ^\\." << std::endl
		<< "\t-t [heartbeat] - timeout for IN_MOVED_FROM/IN_MOVED_TO when it will converted to IN_DELETE/IN_CREATE in miliseconds" << std::endl
		<< "\t   this parameter also used to delay the flood of the IN_MODIFY when final notification " << std::endl
		<< "\t   will produced not earlier than timeout expired after last one" << std::endl
		<< "\t-z skip files with zero length" << std::endl
		<< "\t-d no daemon mode (log will print to stdout if no enabled -l option)" << std::endl
		<< "\t-h this message" << std::endl
		<< std::endl;
}

