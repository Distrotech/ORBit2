#include <config.h>
#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "orbit-options.h"

#undef DEBUG

/*
 * ORBit_option_set:
 * @option: an #ORBit_option describing the option.
 * @val: a pointer to the option value string.
 *
 * Sets @option's arg member member to the appropiate form
 * of @val.
 *
 * If the option is an string option the string will be
 * duplicated, if the option if a 'none' option it will
 * be treated as a boolean option of value TRUE.
 */
static void
ORBit_option_set (const ORBit_option *option,
		  const gchar        *val)
{
	g_assert (option != NULL);

	if (!option->arg)
		return;

#ifdef DEBUG
	fprintf (stderr, "Setting option %s to %s\n", option->name, 
	                                              val ? val : "(none)" );
#endif

	switch (option->type) {
	case ORBIT_OPTION_NONE:
		*(gboolean *)option->arg = TRUE;
		break;
	case ORBIT_OPTION_BOOLEAN:
		*(gboolean *)option->arg = (gboolean)atoi (val);
		break;
	case ORBIT_OPTION_INT:
		*(gint *)option->arg = atoi (val);	
		break;
	case ORBIT_OPTION_STRING: {
		gchar **str_arg = (char **) option->arg;

		if (*str_arg)
			g_free (*str_arg);

		*str_arg = g_strdup (val);
		break;
	}
	case ORBIT_OPTION_KEY_VALUE: {
		fprintf (stderr, "");
		GSList **list = (GSList**) option->arg;

		/* split string into tuple */ 
		gchar **str_vec=g_strsplit (val, "=", 2);
		
		if (str_vec==NULL 
		    || str_vec [0] == NULL
		    || str_vec [1] == NULL)
		{
			g_warning ("key=value pair expected: %s", val);
			if (str_vec!=NULL) g_strfreev (str_vec);
			break;
		}
		g_assert (str_vec[0] != NULL);
		g_assert (str_vec[1] != NULL);
		
		*list = g_slist_append (*list, str_vec);
		break;		
	}
	default:
		g_assert_not_reached ();
		break;
	}
}

/*
 * ORBit_option_rc_parse:
 * @rcfile: the path of the orbitrc file.
 * @option_list: the #ORBit_options to parse from the file.
 *
 * Parses @rcfile for any of the options in @option_list. The syntax
 * of rcfile is simple : 'option=value'.
 *
 * Note: leading or trailing whitespace is allowed for both the option 
 *       and its value.
 */
static void
ORBit_option_rc_parse (const gchar         *rcfile,
		       const ORBit_option  *option_list)
{
	gchar  line [1024];
	FILE  *fh;

	fh = fopen (rcfile, "r");
	if (!fh)
		return;

#ifdef DEBUG
	fprintf (stderr, "Parsing file %s for options\n", rcfile);
#endif

	while (fgets (line, sizeof (line), fh)) {
		const ORBit_option  *option = NULL;
		gchar              **strvec;
		gchar               *key;
		gchar               *value;

		if (line [0] == '#')
			continue;

		strvec = g_strsplit (line, "=", 3);

		if (!strvec || !strvec[0] || !strvec[1])
			continue;

		key = g_strchomp (g_strchug (strvec[0]));

                for (option = option_list; option->name; option++)
			if (!strcmp (key, option->name))
				break;

		if (!option->name) {
			option = NULL;
			continue;
		}

		value = g_strchomp (g_strchug (strvec[1]));

		ORBit_option_set (option, value);

		g_strfreev (strvec);
	}

	fclose (fh);
}

/*
 * ORBit_option_command_line_parse:
 * @argc: main's @argc param.
 * @argv: main's @argv param.
 * @option_list: list of #ORBit_options to parse from @argv.
 *
 * Parse @argv looking for options contained in @option_list and
 * setting values as appropriate. Also strip these options from
 * @argv and adjust @argc.
 */
static void 
ORBit_option_command_line_parse (int                 *argc,
				 char               **argv,
				 const ORBit_option  *option_list)
{
	/* this must be declared outside of loop, as it holds
	 * iteration state */
	ORBit_option *option = NULL;
	
	gboolean *erase;
	gint      i, j, numargs;
	gchar     name [1024];
	gchar    *tmpstr;

#ifdef DEBUG
	fprintf (stderr, "Parsing command line for options\n");
#endif

	if (!argc || !argv)
		return;

	erase = g_new0 (gboolean, *argc);

	for (i = 1, numargs = *argc; i < *argc; i++) {

		/* this is true if previous option name comes with
		 * argument value, and this value is given as extra
		 * string on @argv list */
		if (argv [i][0] != '-') {
			if (!option)
				continue;

			/* mark argv-element as consumed  */
			erase [i] = TRUE;
			numargs--;

			/* skip argument if target location is
			 * specified as NULL */
			if (!option->arg) {
				option = NULL;
				continue;
			}

			/* set argument value */ 
			ORBit_option_set (option, argv [i]);

			/* reset state variable and continue with new
			 * loop */
			option = NULL;
			continue;
                }
		/* if this arguments starts with '-' the user did not
		 * specify option value as expected - go on with usual
		 * processing */ 
		else if (option && option->type != ORBIT_OPTION_NONE)
			g_warning ("Option %s requires an argument\n",
				   option->name);

		/* skip characters until '-' or end of string is found  */
                tmpstr = argv [i];
                while (*tmpstr && *tmpstr == '-')
			tmpstr++;
		
		/* copy argument string into temporary buffer */
                strncpy (name, tmpstr, sizeof (name) - 1);
		name [sizeof (name) - 1] = '\0';

		/* search for '=' in argument string and replace by
		 * '\0' as string seperator */
                tmpstr = strchr (name, '=');
                if (tmpstr)
                        *tmpstr++ = '\0';

		/* find corresponding option declaration */
                for (option = option_list; option->name; option++)
			if (!strcmp (name, option->name))
				break;

		/* if string is not known as ORBit2 option skip the
		 * argument */ 
		if (!option->name) {
			option = NULL;
			continue;
		}
		
		/* mark argument as consumed */ 
		erase [i] = TRUE;
		numargs--;

		/* call ORBit_option_set, if argument is expteced and
		 * if option value was attached with '=' to option
		 * name */ 
		if (option->type != ORBIT_OPTION_NONE && tmpstr) {
			ORBit_option_set (option, tmpstr);
			option = NULL;
		}
	}

	/* erase all consumed arguments from @argv list */
        for (i = j = 1; i < *argc; i++) {
		if (erase [i])
                        continue;

		if (j < numargs)
			argv [j++] = argv [i];
 		else
			argv [j++] = "";
        }

        *argc = numargs;

	/* clean up local resources */ 
        g_free (erase);
}

static gboolean no_sysrc  = FALSE;
static gboolean no_userrc = FALSE;

static ORBit_option orbit_sysrc_options [] = {
        {"ORBNoSystemRC", ORBIT_OPTION_NONE, &no_sysrc},
        {"ORBNoUserRC",   ORBIT_OPTION_NONE, &no_userrc},
	{NULL,            0,                 NULL}
};

/*
 * ORBit_option_parse:
 * @argc: main's @argc param.
 * @argv: main's @argv param.
 * @option_list: list of #ORBit_options.
 *
 * First parses the command line - @argv - to check for orbitrc related
 * options, then parses the relevant orbitrc files and finally parse the
 * command line for all other ORB related options.
 *
 * All ORBit options are stripped from @argv and @argc is adjusted.
 *
 * Note: Command line arguments override orbitrc options and ~/.orbitrc
 *       overrides ${sysconfdir}/orbitrc.
 */
void 
ORBit_option_parse (int                 *argc,
		    char               **argv,
		    const ORBit_option  *option_list)
{
	ORBit_option_command_line_parse (argc, argv, orbit_sysrc_options);

	if (!no_sysrc)
		ORBit_option_rc_parse (ORBIT_SYSTEM_RCFILE, option_list);

	if (!no_userrc) {
		gchar *rcfile;

		rcfile = g_strdup_printf ("%s/%s", g_get_home_dir (), ORBIT_USER_RCFILE);

		ORBit_option_rc_parse (rcfile, option_list);

		g_free (rcfile);
	}

	ORBit_option_command_line_parse (argc, argv, option_list);
}
