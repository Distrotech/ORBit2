#include <config.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>
#include <utime.h>
#include <errno.h>
#include <string.h>
#include <resolv.h>

#include <linc/linc-protocol.h>
#include <linc/linc-connection.h>

#undef DEBUG

#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_NETINET_TCP_H
#include <netinet/tcp.h>
#endif
#ifdef HAVE_SYS_UN_H
#include <sys/un.h>
#endif
#ifdef HAVE_LINUX_IRDA_H
#include <asm/types.h>
#include <linux/irda.h>
#endif

extern LINCProtocolInfo protocol_ents[];

/*
 * linc_protocol_all:
 *
 * Returns a list of protocols supported by linc.
 *
 * Note: the list is terminated by a #LINCProtocolInfo with a
 *       NULL name pointer.
 *
 * Return Value: an array of #LINCProtocolInfo structures.
 */
LINCProtocolInfo * const
linc_protocol_all (void)
{
	return protocol_ents;
}

/*
 * linc_protocol_find:
 * @name: name of the protocol.
 *
 * Find a protocol identified by @name.
 *
 * Return Value: a pointer to a valid #LINCProtocolInfo structure if 
 *               the protocol is supported by linc, NULL otherwise.
 */
LINCProtocolInfo * const
linc_protocol_find (const char *name)
{
	int i;

	for (i = 0; protocol_ents [i].name; i++) {
		if (!strcmp (name, protocol_ents [i].name))
			return &protocol_ents [i];
	}

	return NULL;
}

/*
 * linc_protocol_find_num:
 * @family: the family identifier of the protocol - i.e. AF_*
 *
 * Find a protocol identified by @family.
 *
 * Return Value: a pointer to a valid #LINCProtocolInfo structure if
 *               the protocol is supported by linc, NULL otherwise.
 */
LINCProtocolInfo * const
linc_protocol_find_num (const int family)
{
	int i;

	for (i = 0; protocol_ents [i].name; i++) {
		if (family == protocol_ents [i].family)
			return &protocol_ents [i];
	}

	return NULL;
}

static char linc_tmpdir [PATH_MAX] = "";

/*
 * make_local_tmpdir:
 * @dirname: directory name.
 *
 * Create a directory with the name in @dirname. Also, clear the
 * access and modification times of @dirname.
 *
 * If the directory already exists and is not owned by the current 
 * user, or is not solely readable by the current user, then linc
 * will error out.
 */
static void
make_local_tmpdir (const char *dirname)
{
	struct stat statbuf;
		
	if (mkdir (dirname, 0700) != 0) {
		int e = errno;
			
		switch (e) {
		case 0:
		case EEXIST:
			if (stat (dirname, &statbuf) != 0)
				g_error ("Can not stat %s\n", dirname);

			if (statbuf.st_uid != getuid ())
				g_error ("Owner of %s is not the current user\n", dirname);

			if((statbuf.st_mode & (S_IRWXG|S_IRWXO))
			   || !S_ISDIR (statbuf.st_mode))
				g_error ("Wrong permissions for %s\n", dirname);

			break;
				
		default:
			g_error("Unknown error on directory creation of %s (%s)\n",
				dirname, g_strerror (e));
		}
	}

	{ /* Hide some information ( apparently ) */
		struct utimbuf utb;
		memset (&utb, 0, sizeof (utb));
		utime (dirname, &utb);
	}
}

/*
 * linc_set_tmpdir:
 * @dir: directory name.
 *
 * Set the temporary directory used by linc to @dir. 
 *
 * This directory is used for the creation of UNIX sockets.
 */
void
linc_set_tmpdir (const char *dir)
{
	strncpy (linc_tmpdir, dir, PATH_MAX);

	linc_tmpdir [PATH_MAX - 1] = '\0';

	make_local_tmpdir (linc_tmpdir);
}

#ifdef HAVE_SOCKADDR_SA_LEN
#define LINC_SET_SOCKADDR_LEN(saddr, len)                     \
		((struct sockaddr *)(saddr))->sa_len = (len)
#else 
#define LINC_SET_SOCKADDR_LEN(saddr, len)
#endif

#ifdef AF_INET6
#define LINC_RESOLV_SET_IPV6     _res.options |= RES_USE_INET6
#define LINC_RESOLV_CLEAR_IPV6   _res.options &= ~RES_USE_INET6
#else
#define LINC_RESOLV_SET_IPV6
#define LINC_RESOLV_CLEAR_IPV6
#endif

/*
 * linc_protocol_get_sockaddr_ipv4:
 * @proto: the #LINCProtocolInfo structure for the IPv4 protocol.
 * @hostname: the hostname.
 * @portnum: the port number.
 * @saddr_len: location in which to store the returned structure's length.
 *
 * Allocates and fills a #sockaddr_in with with the IPv4 address 
 * information.
 *
 * Return Value: a pointer to a valid #sockaddr_in structure if the call 
 *               succeeds, NULL otherwise.
 */
#ifdef AF_INET
static struct sockaddr *
linc_protocol_get_sockaddr_ipv4 (const LINCProtocolInfo *proto,
				 const char             *hostname,
				 const char             *portnum,
				 socklen_t              *saddr_len)
{
	struct sockaddr_in *saddr;
	struct hostent     *host;

	g_assert (proto->family == AF_INET);
	g_assert (hostname);

	if (!portnum)
		portnum = "0";

	saddr = g_new0 (struct sockaddr_in, 1);

	*saddr_len = sizeof (struct sockaddr_in);

	LINC_SET_SOCKADDR_LEN (saddr, sizeof (struct sockaddr_in));

	saddr->sin_family = AF_INET;
	saddr->sin_port   = htons (atoi (portnum));

	if (!(_res.options & RES_INIT))
		res_init();

	LINC_RESOLV_CLEAR_IPV6;

	host = gethostbyname (hostname);
	if (!host) {
		g_free (saddr);
		return NULL;
	}

	memcpy (&saddr->sin_addr, host->h_addr_list[0], sizeof (struct in_addr));

	return (struct sockaddr *)saddr;
}
#endif /* AF_INET */

/*
 * linc_protocol_get_sockaddr_ipv6:
 * @proto: the #LINCProtocolInfo structure for the IPv6 protocol.
 * @hostname: the hostname.
 * @portnum: the port number
 * @saddr_len: location in which to store the returned structure's length.
 *
 * Allocates and fills a #sockaddr_in6 with with the IPv6 address 
 * information.
 *
 * NOTE: This function is untested.
 *
 * Return Value: a pointer to a valid #sockaddr_in6 structure if the call 
 *               succeeds, NULL otherwise.
 */
#ifdef AF_INET6
static struct sockaddr *
linc_protocol_get_sockaddr_ipv6 (const LINCProtocolInfo *proto,
				 const char             *hostname,
				 const char             *portnum,
				 socklen_t              *saddr_len)
{
	struct sockaddr_in6 *saddr;
	struct hostent      *host;

	g_assert (proto->family == AF_INET6);
	g_assert (hostname);

	if (!portnum)
		portnum = "0";

	saddr = g_new0 (struct sockaddr_in6, 1);

	*saddr_len = sizeof (struct sockaddr_in6);

	LINC_SET_SOCKADDR_LEN (saddr, sizeof (struct sockaddr_in6));

	saddr->sin6_family = AF_INET6;
	saddr->sin6_port = htons (atoi (portnum));

	if (!(_res.options & RES_INIT))
		res_init();

	LINC_RESOLV_SET_IPV6;

	host = gethostbyname (hostname);
	if (!host || host->h_addrtype != AF_INET6) {
		g_free (saddr);
		return NULL;
	}

	memcpy (&saddr->sin6_addr, host->h_addr_list[0], sizeof (struct in6_addr));

	return (struct sockaddr *)saddr;
}
#endif /* AF_INET6 */

/*
 * linc_protocol_get_sockaddr_unix:
 * @proto: the #LINCProtocolInfo structure for the UNIX sockets protocol.
 * @dummy: not used.
 * @path: the path name of the UNIX socket.
 * @saddr_len: location in which to store the returned structure's length.
 *
 * Allocates and fills a #sockaddr_un with with the UNIX socket address 
 * information.
 *
 * If @path is NULL, a new, unique path name will be generated.
 *
 * Return Value: a pointer to a valid #sockaddr_un structure if the call 
 *               succeeds, NULL otherwise.
 */
#ifdef AF_UNIX
static struct sockaddr *
linc_protocol_get_sockaddr_unix (const LINCProtocolInfo *proto,
				 const char             *dummy,
				 const char             *path,
				 socklen_t              *saddr_len)
{
	struct sockaddr_un *saddr;
	int                 pathlen;
	char                buf[64], *actual_path;

	g_assert (proto->family == AF_UNIX);

	if (!path) {
		struct timeval t;
		gettimeofday (&t, NULL);
		g_snprintf (buf, sizeof (buf),
			    "%s/linc-%x%x", linc_tmpdir,
			     rand(), (guint)(t.tv_sec^t.tv_usec));
#ifdef DEBUG
		if (g_file_test (buf, G_FILE_TEST_EXISTS))
			g_warning ("'%s' already exists !", buf);
#endif
		actual_path = buf;
		}
	else 
		actual_path = (char *)path;

	pathlen = strlen (actual_path);

	if (pathlen >= sizeof (saddr->sun_path))
		return NULL;

	saddr = g_new0 (struct sockaddr_un, 1);

	*saddr_len = sizeof (struct sockaddr_un) - sizeof (saddr->sun_path) + pathlen;

	LINC_SET_SOCKADDR_LEN (saddr, *saddr_len);

	saddr->sun_family =  AF_UNIX;
	strncpy (saddr->sun_path, actual_path, sizeof (saddr->sun_path) - 1);
	saddr->sun_path[sizeof (saddr->sun_path) - 1] = '\0';

	return (struct sockaddr *)saddr;
}
#endif /* AF_UNIX */

/*
 * linc_protocol_get_sockaddr_irda:
 * @proto:
 * @hostname:
 * @service:
 * @saddr_len:
 *
 * NOTE: This function is not implemented. We need to hack something
 *       together from irda_getaddrinfo.
 *
 * Return Value:
 */
#ifdef AF_IRDA
static struct sockaddr *
linc_protocol_get_sockaddr_irda (const LINCProtocolInfo *proto,
				 const char             *hostname,
				 const char             *service,
				 socklen_t              *saddr_len)
{
	g_assert (proto->family == AF_IRDA);

	return NULL;
}
#endif /* AF_IRDA */

/*
 * linc_protocol_get_sockaddr:
 * @proto: a #LINCProtocolInfo structure.
 * @hostname: protocol dependant host information.
 * @service: protocol dependant service information.
 * @saddr_len: location in which to store the returned structure's length.
 *
 * Allocates, fills in and returns the #sockaddr structure appropriate
 * for the supplied protocol, @proto.
 *
 * Return Value: a pointer to a valid #sockaddr structure if the call 
 *               succeeds, NULL otherwise.
 */
struct sockaddr *
linc_protocol_get_sockaddr (const LINCProtocolInfo *proto,
			    const char             *hostname,
			    const char             *service,
			    socklen_t              *saddr_len)
		   
{
	if (proto && proto->get_sockaddr)
		return proto->get_sockaddr (proto, hostname, service, saddr_len);

	return NULL;
}

int
linc_getaddrinfo (const char             *nodename,
		  const char             *servname,
		  const struct addrinfo  *hints,
		  struct addrinfo       **res)
{
  int i, rv = EAI_NONAME, tmprv;
  gboolean keep_going;

  for(keep_going = TRUE, i = 0; keep_going && protocol_ents[i].name; i++)
    {
      if(protocol_ents[i].getaddrinfo && (hints->ai_family == AF_UNSPEC || hints->ai_family == protocol_ents[i].family))
	{
	  char srvbuf[64];
	  gboolean fakeserv = FALSE;

	  if(!servname && (hints->ai_flags & AI_PASSIVE))
	    switch(hints->ai_family)
	      {
#if defined(AF_INET)
	      case AF_INET:
#endif
#if defined(AF_INET6)
	      case AF_INET6:
#endif
		strcpy(srvbuf, "0");
		fakeserv = TRUE;
		break;
#if defined(AF_UNIX)
	      case AF_UNIX: {
                struct timeval t;
		gettimeofday (&t, NULL);
		g_snprintf(srvbuf, sizeof(srvbuf),
			   "%s/linc-%x%x", linc_tmpdir,
			   rand(), (guint)(t.tv_sec^t.tv_usec));
#ifdef DEBUG
		if (g_file_test (srvbuf, G_FILE_TEST_EXISTS))
			g_warning ("'%s' already exists !", srvbuf);
#endif
		fakeserv = TRUE;
		break;
	      }
#endif
	      default:
		break;
	      }

	  tmprv = protocol_ents[i].getaddrinfo(nodename,
					       fakeserv?srvbuf:servname,
					       hints, res);
	  switch(tmprv)
	    {
	    case EAI_ADDRFAMILY:
	    case EAI_FAMILY:
	    case EAI_NONAME:
	      /* Maybe we have support for it in another routine */
	      break;

	    default:
	      rv = tmprv;
	      keep_going = FALSE;
	      break;
	    }
	}
    }

  return rv;
}

int
linc_getnameinfo (const struct sockaddr *sa,
		  socklen_t              sa_len,
		  char                  *host,
		  size_t                 hostlen,
		  char                  *serv,
		  size_t                 servlen,
		  int                    flags)
{
	int i, rv = -1;

	for (i = 0; rv == -1 && protocol_ents [i].name; i++) {
		if (protocol_ents [i].getnameinfo &&
		    protocol_ents [i].family == sa->sa_family)
			rv = protocol_ents [i].getnameinfo (
				sa, sa_len, host, hostlen,
				serv, servlen, flags);
	}

	return rv;
}

/* Routines for AF_UNIX */
#ifdef AF_UNIX
static void
af_unix_destroy(int fd, const char *host_info, const char *serv_info)
{
  unlink(serv_info);
}
#endif

/* Routines for AF_IRDA */
#ifdef AF_IRDA

#define MAX_IRDA_DEVICES 10
#define IRDA_NICKNAME_MAX (sizeof (((struct irda_device_info *)NULL)->info) + 1)

static int
irda_find_device (guint32  *addr,
		  char     *name,
		  gboolean  name_to_addr)
{
	struct irda_device_list *list;
	unsigned char           *buf;
	int                      len, i, retval, fd;

	retval = -1;

	fd = socket (AF_IRDA, SOCK_STREAM, 0);
	if (fd < 0)
		return -1;

	len = sizeof (struct irda_device_list) +
	      sizeof (struct irda_device_info) * MAX_IRDA_DEVICES;

	buf = g_alloca (len);
	list = (struct irda_device_list *)buf;
        
	if (getsockopt (fd, SOL_IRLMP, IRLMP_ENUMDEVICES, buf, &len))
		goto out;

	if (len < 1)
		goto out;

	for (i = 0; i < list->len && retval; i++) {
		if(name_to_addr) {
			if (!strcmp (list->dev[i].info, name)) {
				*addr = list->dev[i].daddr;
				retval = 0;
			}
		}
		else {
			if (list->dev[i].daddr == *addr) {
				strncpy (name, list->dev[i].info,
					 sizeof(list->dev[i].info));
				name[sizeof (list->dev[i].info)] = '\0';
				retval = 0;
			}
		}
	}

 out:
	close(fd);

	return retval;
}

#define IRDA_PREFIX      "IrDA-"
#define IRDA_PREFIX_LEN  5

static int
irda_getaddrinfo (const char             *nodename,
		  const char             *servname,
		  const struct addrinfo  *hints,
		  struct addrinfo       **res)
{
	struct sockaddr_irda  sai;
	struct addrinfo      *retval;
	char                  hnbuf[IRDA_NICKNAME_MAX + IRDA_PREFIX_LEN];
	int                   n;
	char                 *tptr;

	/* 
	 * For now, it *has* to start with IRDA_PREFIX to be in the IRDA
	 * hostname/address format we use 
	 */
	if (nodename && strcmp (nodename, IRDA_PREFIX))
		return EAI_NONAME;

	sai.sir_family   = AF_IRDA;
	sai.sir_lsap_sel = LSAP_ANY;

	if (servname)
		g_snprintf (sai.sir_name, sizeof (sai.sir_name), "%s", servname);
	else {
		struct timeval t;

		gettimeofday (&t, NULL);
		g_snprintf (sai.sir_name, sizeof (sai.sir_name), "IIOP%x%x",
		            rand(), (guint)(t.tv_sec^t.tv_usec));
	}

	if (nodename) {
		if (!strncmp (nodename + IRDA_PREFIX_LEN, "0x", 2)) {
			if (sscanf (nodename + strlen(IRDA_PREFIX "0x"),
				    "%u", &sai.sir_addr) != 1)
				return EAI_NONAME;

			/* It's a numeric address - we need to find the hostname */
			strcpy (hnbuf, IRDA_PREFIX);
			if (irda_find_device (&sai.sir_addr, 
					      hnbuf + IRDA_PREFIX_LEN,
					      FALSE))
				return EAI_NONAME;

			nodename = hnbuf;
		}
		else if (!(hints->ai_flags & AI_NUMERICHOST)) {
			/* It's a name - we need to find the address */
			if (irda_find_device (&sai.sir_addr, 
					      (char *)nodename + IRDA_PREFIX_LEN,
					      TRUE))
				return EAI_NONAME;
		}
		else
			return EAI_NONAME;
	}
	else
		/* AI_PASSIVE flag gets ignored, sort of */
		hnbuf[0] = 0;

	n = sizeof (struct addrinfo) + sizeof (struct sockaddr_irda);

	if (hints->ai_flags & AI_CANONNAME)
		n += strlen(hnbuf) + 1;

	retval = g_malloc0(n);

	tptr = (char *)retval;
	tptr += sizeof (struct addrinfo);
	retval->ai_addr = (struct sockaddr *)tptr;
	memcpy (retval->ai_addr, &sai, sizeof (struct sockaddr_irda));
	tptr += sizeof (struct sockaddr_irda);
	strcpy (tptr, hnbuf);
	retval->ai_family = AF_IRDA;
	retval->ai_socktype = SOCK_STREAM;
	retval->ai_protocol = 0;
	retval->ai_next = NULL;
	retval->ai_addrlen = sizeof(struct sockaddr_irda);

	*res = retval;

	return 0;
}

static int
irda_getnameinfo(const struct sockaddr *sa, socklen_t sa_len,
		 char *host, size_t hostlen, char *serv, size_t servlen,
		 int flags)
{
  struct sockaddr_irda *sai = (struct sockaddr_irda *)sa;
  gboolean got_host = FALSE;
  /* Here, we talk to the host specified, and ask it for its name */

  if(sa_len != sizeof(struct sockaddr_irda))
    return -1;

  /* It doesn't seem like the sir_lsap_sel is supposed to be taken into consideration when connecting... */
  if(!(flags & NI_NUMERICHOST))
    {
      char hostbuf[IRDA_NICKNAME_MAX];
      guint32 daddr;

      daddr = sai->sir_addr;
      if(!irda_find_device(&daddr, hostbuf, FALSE))
	{
	  g_snprintf(host, hostlen, "%s", hostbuf);
	  got_host = TRUE;
	}
    }
  if(!got_host)
    {
      if(flags & NI_NAMEREQD)
	return -1;

      g_snprintf(host, hostlen, IRDA_PREFIX "%#08x", sai->sir_addr);
    }

  g_snprintf(serv, servlen, "%s", sai->sir_name);

  return 0;
}
#endif

static int
sys_getaddrinfo (const char             *nodename,
		 const char             *servname,
		 const struct addrinfo  *hints,
		 struct addrinfo       **res)
{
	int retval;

	if (!nodename && !servname && hints)
		servname = "0";

	errno = 0;

	retval = getaddrinfo (nodename, servname, hints, res);
#ifdef DEBUG
	if (errno)
		perror ("In getaddrinfo ");
#endif

	return retval;
}

static int
sys_getnameinfo(const struct sockaddr *sa, socklen_t sa_len,
		char *host, size_t hostlen, char *serv, size_t servlen,
		int flags)
{
  int retval;
  char *fakehost = host;
  socklen_t fakelen = hostlen;

  switch(sa->sa_family)
    {
#if defined(AF_INET) || defined(AF_INET6)
#ifdef AF_INET
    case AF_INET:
      if(sa->sa_family == AF_INET && host)
	{
	  struct sockaddr_in *sa4 = (struct sockaddr_in *)sa;
	  if(sa4->sin_addr.s_addr == INADDR_ANY)
	    {
	      strcpy(host, "0.0.0.0");
	      fakehost = NULL;
	      fakelen = 0;
	    }
	}
#endif
#ifdef AF_INET6
    case AF_INET6:
#endif
      if(sa->sa_family == AF_INET6 && host)
	{
	  struct sockaddr_in6 *sa6 = (struct sockaddr_in6 *)sa;
	  struct in6_addr sa6addr = IN6ADDR_ANY_INIT;

	  if(!memcmp(&sa6->sin6_addr, &sa6addr, sizeof(sa6addr)))
	    {
	      strcpy(host, "::");
	      fakehost = NULL;
	      fakelen = 0;
	    }
	}
#endif
    default:
      retval = getnameinfo(sa, sa_len, fakehost, fakelen, serv, servlen, flags);
      break;
    }

  /* Yet another bad hack. Just give me my daggone FQDN */
  switch(sa->sa_family)
    {
#if defined(AF_INET) || defined(AF_INET6)
#ifdef AF_INET
    case AF_INET:
#endif
#ifdef AF_INET6
    case AF_INET6:
#endif
      if(host && (!strcmp(host, "0.0.0.0") || !strcmp(host, "::")))
	{
	  struct addrinfo *ai, hints;
	  gethostname(host, hostlen);
	  hints.ai_flags = AI_CANONNAME;
	  hints.ai_family = sa->sa_family;
	  hints.ai_protocol = 0;

	  if(!linc_getaddrinfo(host, NULL, &hints, &ai))
	    {
	      g_snprintf(host, hostlen, "%s", ai->ai_canonname);
	      freeaddrinfo(ai);
	    }
	}
      break;
#endif
    default:
      break;
    }

  return retval;
}

static void
tcp_setup(int fd, LINCConnectionOptions cnx_flags)
{
#ifdef TCP_NODELAY
  if(!(cnx_flags & LINC_CONNECTION_SSL))
    {
      struct protoent *proto;
      int on;
      proto = getprotobyname("tcp");
      if(!proto)
	return;
      on = 1;
      setsockopt(fd, proto->p_proto, TCP_NODELAY, &on, sizeof(on));
    }
#endif
}

static LINCProtocolInfo protocol_ents[] = {
#if defined(AF_INET)
	{ "IPv4", AF_INET,
	sizeof (struct sockaddr_in),
	IPPROTO_TCP, 0, 
	tcp_setup, NULL, 
	sys_getaddrinfo,
	sys_getnameinfo,
	linc_protocol_get_sockaddr_ipv4
	},
#endif
#if defined(AF_INET6)
	{ "IPv6", AF_INET6, 
	sizeof (struct sockaddr_in6),
	IPPROTO_TCP, 0, 
	tcp_setup, NULL, 
	sys_getaddrinfo, 
	sys_getnameinfo,
	linc_protocol_get_sockaddr_ipv6
	},
#endif
#ifdef AF_UNIX
	{ "UNIX", AF_UNIX,
	sizeof (struct sockaddr_un),
	0, LINC_PROTOCOL_SECURE|LINC_PROTOCOL_NEEDS_BIND,
	NULL, af_unix_destroy, 
	sys_getaddrinfo, 
	sys_getnameinfo,
	linc_protocol_get_sockaddr_unix
	},
#endif
#ifdef AF_IRDA
	{ "IrDA",AF_IRDA,
	sizeof (struct sockaddr_irda), 
	0, LINC_PROTOCOL_NEEDS_BIND,
	NULL, NULL,
	irda_getaddrinfo,
	irda_getnameinfo,
	linc_protocol_get_sockaddr_irda
	},
#endif
	{ NULL }
};
