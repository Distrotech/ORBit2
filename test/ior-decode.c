#include "config.h"
#include <stdio.h>
#include <orbit/orbit.h>
#include <ctype.h>

static void
print_objkey (IOP_ObjectKey_info *oki)
{
	int i;
	GString *str = g_string_sized_new (oki->object_key._length * 2 + 8);

	for (i = 0; i < oki->object_key._length; i++)
		g_string_printfa (str, "%02x", oki->object_key._buffer [i]);

	printf ("(%d) '%s'", oki->object_key._length, str->str);

	g_string_free (str, TRUE);
}

static void
print_components(GSList *components)
{
  GSList *ltmp;
  for(ltmp = components; ltmp; ltmp = ltmp->next)
    {
      IOP_Component_info *c = ltmp->data;

      switch(c->component_type)
	{
	case IOP_TAG_COMPLETE_OBJECT_KEY:
	  {
	    IOP_TAG_COMPLETE_OBJECT_KEY_info *coki = ltmp->data;
	    printf("    IOP_TAG_COMPLETE_OBJECT_KEY: object_key ");
	    print_objkey(coki->oki);
	    printf("\n");
	  }
	  break;
	case IOP_TAG_SSL_SEC_TRANS:
	  {
	    IOP_TAG_SSL_SEC_TRANS_info *sst = ltmp->data;
	    printf("    IOP_TAG_SSL_SEC_TRANS: %d:%d port %d\n",
		   sst->target_supports, sst->target_requires,
		   sst->port);
	  }
	  break;
	case IOP_TAG_GENERIC_SSL_SEC_TRANS:
	  {
	    IOP_TAG_GENERIC_SSL_SEC_TRANS_info *sst = ltmp->data;
	    printf("    IOP_TAG_GENERIC_SSL_SEC_TRANS: service %s\n",
		   sst->service);
	  }
	  break;
	default:
	  printf("    Unknown component %#x\n",
		 c->component_type);
	  break;
	}
      printf("\n");
    }
}

static void
print_iiop_version(GIOPVersion ver)
{
  switch(ver)
    {
    case GIOP_1_0:
      printf("GIOP 1.0");
      break;
    case GIOP_1_1:
      printf("GIOP 1.1");
      break;
    case GIOP_1_2:
      printf("GIOP 1.2");
      break;
    default:
      g_assert_not_reached();
      break;
    }
}
int main(int argc, char *argv[])
{
  CORBA_ORB orb;
  CORBA_Object obj;
  CORBA_Environment ev;
  GSList *ltmp;

  CORBA_exception_init(&ev);

  if(argc != 2)
    {
      fprintf(stderr, "Usage: ior-decode <IOR>\n");
      return 1;
    }

  orb = CORBA_ORB_init(&argc, argv, "orbit-local-orb", &ev);

  obj = CORBA_ORB_string_to_object(orb, argv[1], &ev);
  if(ev._major)
    {
      fprintf(stderr, "Couldn't do string_to_object: %s\n",
	      CORBA_exception_id(&ev));
      return 2;
    }
  printf("Object ID: %s\n", obj->type_id);
  for(ltmp = obj->profile_list; ltmp; ltmp = ltmp->next)
    {
      IOP_Profile_info *pi = ltmp->data;
      switch(pi->profile_type)
	{
	case IOP_TAG_INTERNET_IOP:
	  {
	    IOP_TAG_INTERNET_IOP_info *iiop = ltmp->data;

	    printf("IOP_TAG_INTERNET_IOP: ");
	    print_iiop_version(iiop->iiop_version);
	    printf(" %s:%d\n",
		   iiop->host, iiop->port);
	    printf("    object_key ");
	    print_objkey(iiop->oki);
	    printf("\n");
	    print_components(iiop->components);
	  }
	  break;
	case IOP_TAG_GENERIC_IOP:
	  {
	    IOP_TAG_GENERIC_IOP_info *giop = ltmp->data;
	    printf("IOP_TAG_GENERIC_IOP: ");
	    print_iiop_version(giop->iiop_version);
	    printf("[%s] %s:%s\n",
		   giop->proto,
		   giop->host, giop->service);
	    print_components(giop->components);
	  }
	  break;
	case IOP_TAG_MULTIPLE_COMPONENTS:
	  {
	    IOP_TAG_MULTIPLE_COMPONENTS_info *mci = ltmp->data;
	    printf("IOP_TAG_MULTIPLE_COMPONENTS:\n");
	    print_components(mci->components);
	  }
	  break;
	case IOP_TAG_ORBIT_SPECIFIC:
	  {
	    IOP_TAG_ORBIT_SPECIFIC_info *osi = ltmp->data;
	    printf("IOP_TAG_ORBIT_SPECIFIC: usock %s IPv6 port %d\n",
		   osi->unix_sock_path, osi->ipv6_port);
	    printf("    object_key ");
	    print_objkey(osi->oki);
	    printf("\n");
	  }
	  break;
	default:
	  printf("Unknown profile type %#x\n", pi->profile_type);
	  break;
	}
      printf("\n");
    }
  
  return 0;
}
