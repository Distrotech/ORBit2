#include "config.h"
#include <linc/linc-protocol.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>

#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_SYS_UN_H
#include <sys/un.h>
#endif
#ifdef HAVE_LINUX_IRDA_H
#include <asm/types.h>
#include <linux/irda.h>
#endif

static void af_unix_destroy(int fd, struct sockaddr *saddr);
static int irda_getaddrinfo(const char *nodename, const char *servname, const struct addrinfo *hints, struct addrinfo **res);
static int irda_getnameinfo(const struct sockaddr *sa, socklen_t sa_len,
			    char *host, size_t hostlen, char *serv, size_t servlen,
			    int flags);

static LINCProtocolInfo protocol_ents[] = {
#if defined(AF_INET) && defined(IPPROTO_TCP)
  {"IPv4", AF_INET, sizeof(struct sockaddr_in), IPPROTO_TCP, 0, NULL, getaddrinfo /* also covers IPv6 & UNIX */, getnameinfo},
#endif
#if defined(AF_INET6) && defined(IPPROTO_TCP)
  {"IPv6", AF_INET6, sizeof(struct sockaddr_in6), IPPROTO_TCP, 0},
#endif
#ifdef AF_UNIX
  {"UNIX", AF_UNIX, sizeof(struct sockaddr_un), 0, LINC_PROTOCOL_SECURE|LINC_PROTOCOL_NEEDS_BIND, af_unix_destroy},
#endif
#ifdef AF_IRDA
  {"IrDA", AF_IRDA, sizeof(struct sockaddr_irda), 0, LINC_PROTOCOL_NEEDS_BIND, NULL, irda_getaddrinfo, irda_getnameinfo},
#endif
  {NULL}
};

LINCProtocolInfo * const
linc_protocol_find(const char *name)
{
  int i;
  for(i = 0; protocol_ents[i].name; i++)
    {
      if(!strcmp(name, protocol_ents[i].name))
	return &protocol_ents[i];
    }

  return NULL;
}

LINCProtocolInfo * const
linc_protocol_find_num(const int family)
{
  int i;
  for(i = 0; protocol_ents[i].name; i++)
    {
      if(family == protocol_ents[i].family)
	return &protocol_ents[i];
    }

  return NULL;
}

static char linc_tmpdir[PATH_MAX] = "/tmp";

void
linc_set_tmpdir(const char *dir)
{
  const char *uname = g_get_user_name();
  g_snprintf(linc_tmpdir, sizeof(linc_tmpdir), dir, uname, uname, uname);
}

int
linc_getnameinfo(const struct sockaddr *sa, socklen_t sa_len,
		 char *host, size_t hostlen, char *serv, size_t servlen,
		 int flags)
{
  int i, rv = -1;

  for(i = 0; rv == -1 && protocol_ents[i].name; i++)
    {
      if(protocol_ents[i].getnameinfo && (protocol_ents[i].family == sa->sa_family))
	rv = protocol_ents[i].getnameinfo(sa, sa_len, host, hostlen, serv, servlen, flags);
    }

  return rv;
}

int
linc_getaddrinfo(const char *nodename, const char *servname, const struct addrinfo *hints, struct addrinfo **res)
{
  int i, rv = EAI_NONAME, tmprv;
  gboolean keep_going;

  for(keep_going = TRUE, i = 0; keep_going && protocol_ents[i].name; i++)
    {
      if(protocol_ents[i].getaddrinfo && (hints->ai_family == AF_UNSPEC || hints->ai_family == protocol_ents[i].family))
	{
	  tmprv = protocol_ents[i].getaddrinfo(nodename, servname, hints, res);
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
#if defined(AF_UNIX)
	      if(!rv
		 && !servname
		 && (hints->ai_flags & AI_PASSIVE)
		 && ((*res)->ai_family == AF_UNIX))
		{
		  struct sockaddr_un *sun = (struct sockaddr_un *)(*res)->ai_addr;
		  srand(time(NULL));
		  g_snprintf(sun->sun_path, sizeof(sun->sun_path),
			     "%s/linc-%x%x", linc_tmpdir, rand(), rand());
		}
#endif
	      break;
	    }
	}
    }

  return rv;
}

/* Routines for AF_UNIX */
#ifdef AF_UNIX
static void
af_unix_destroy(int fd, struct sockaddr *saddr)
{
  struct sockaddr_un *sau = (struct sockaddr_un *)saddr;

  unlink(sau->sun_path);
}
#endif

/* Routines for AF_IRDA */
#ifdef AF_IRDA

#define MAX_IRDA_DEVICES 10
#define IRDA_NICKNAME_MAX (sizeof(((struct irda_device_info *)NULL)->info) + 1)
static int
irda_find_device(guint32 *addr, char *name, gboolean name_to_addr)
{
  struct irda_device_list *list;
  unsigned char *buf;
  int len;
  int i;
  int retval = -1;
  int fd;

  fd = socket(AF_IRDA, SOCK_STREAM, 0);
  if(fd < 0)
    return -1;

  len = sizeof(struct irda_device_list) +
    sizeof(struct irda_device_info) * MAX_IRDA_DEVICES;

  buf = orbit_alloca(len);
  list = (struct irda_device_list *) buf;
        
  if (getsockopt(fd, SOL_IRLMP, IRLMP_ENUMDEVICES, buf, &len))
    goto out;

  if(len < 1)
    goto out;

  for(i = 0; i < list->len && retval; i++)
    {
      if(name_to_addr)
	{
	  if(!strcmp(list->dev[i].info, name))
	    {
	      *addr = list->dev[i].daddr;
	      retval = 0;
	    }
	}
      else
	{
	  if(list->dev[i].daddr == *addr)
	    {
	      strncpy(name, list->dev[i].info, sizeof(list->dev[i].info));
	      name[sizeof(list->dev[i].info)] = '\0';
	      retval = 0;
	    }
	}
    }

 out:
  close(fd);

  return retval;
}

#define IRDA_PREFIX "IrDA-"

static int
irda_getaddrinfo(const char *nodename, const char *servname, const struct addrinfo *hints, struct addrinfo **res)
{
  struct sockaddr_irda sai;
  struct addrinfo *retval;
  char hnbuf[IRDA_NICKNAME_MAX + strlen(IRDA_PREFIX)];
  int n;

  /* For now, it *has* to start with IRDA_PREFIX to be in the IRDA
     hostname/address format we use */
  if(nodename && strcmp(nodename, IRDA_PREFIX))
    return EAI_NONAME;

  sai.sir_family = AF_IRDA;
  sai.sir_lsap_sel = LSAP_ANY;
  if(servname)
    g_snprintf(sai.sir_name, sizeof(sai.sir_name), "%s", servname);
  else
    {
      srand(time(NULL));
      g_snprintf(sai.sir_name, sizeof(sai.sir_name), "IIOP%x%x",
		 rand(), rand());
    }

  if(nodename)
    {
      if(!strncmp(nodename + strlen(IRDA_PREFIX), "0x", 2))
	{
	  if(sscanf(nodename + strlen(IRDA_PREFIX "0x"), "%u", 
		    &sai.sir_addr) != 1)
	    return EAI_NONAME;

	  /* It's a numeric address - we need to find the hostname */
	  strcpy(hnbuf, IRDA_PREFIX);
	  if(irda_find_device(&sai.sir_addr, hnbuf + strlen(IRDA_PREFIX), FALSE))
	    return EAI_NONAME;
	  nodename = hnbuf;
	}
      else if(!(hints->ai_flags & AI_NUMERICHOST))
	{
	  /* It's a name - we need to find the address */
	  if(irda_find_device(&sai.sir_addr, (char *)nodename + strlen(IRDA_PREFIX), TRUE))
	    return EAI_NONAME;
	}
      else
	return EAI_NONAME;
    }
  /* else, AI_PASSIVE flag gets ignored */

  n = sizeof(struct addrinfo) + sizeof(struct sockaddr_irda);
  if(hints->ai_flags & AI_CANONNAME)
    n += strlen(hnbuf) + 1;
  retval = calloc(1, n);
  retval->ai_addr = (struct sockaddr *)(((guchar *)retval) + sizeof(struct addrinfo));
  memcpy(retval->ai_addr, &sai, sizeof(struct sockaddr_irda));
  strcpy(((guchar *)retval) + sizeof(struct addrinfo) + sizeof(struct sockaddr_irda), nodename);
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
