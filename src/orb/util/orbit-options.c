#include <glib.h>
#include <stdlib.h>
#include <string.h>

#include "orbit-options.h"

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
ORBit_option_set (ORBit_option *option,
		  const gchar  *val)
{
	g_assert (option != NULL);

	if (!option->arg)
		return;

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
		gchar **str_arg = (char **)option->arg;

		if (*str_arg)
			g_free (*str_arg);

		*str_arg = g_strdup(val);
		}
		break;
	default:
		g_assert_not_reached ();
		break;
	}
}

/*
 * ORBit_option_parse:
 * @argc: main's @argc param.
 * @argv: main's @argv param.
 * @option_list: list of #ORBit_options to parse from @argv.
 *
 * Parse @argv looking for options contained in @option_list and
 * setting values as appropriate. Also strip these options from
 * @argv and adjust @argc.
 */
void 
ORBit_option_parse (int           *argc,
		    char         **argv,
		    ORBit_option  *option_list)
{
	ORBit_option *option = NULL;
        gboolean     *erase;
	gint          i, j, numargs;
	gchar         name [1024];
	gchar        *tmpstr;

	erase = g_new0 (gboolean, *argc);

	for (i = 1, numargs = *argc; i < *argc; i++) {
		if (argv [i][0] != '-') {
                        if (!option)
                                continue;
                        
			erase [i] = TRUE;
			numargs--;

			if (!option->arg) {
				option = NULL;
				continue;
			}

			ORBit_option_set (option, argv [i]);

			option = NULL;
			continue;
                }
		else if (option && option->type != ORBIT_OPTION_NONE)
			g_warning ("Option %s requires an argument\n",
				   option->name);

                tmpstr = argv [i];
                while (*tmpstr && *tmpstr == '-')
			tmpstr++;

                strncpy (name, tmpstr, sizeof (name) - 1);
                name [sizeof (name) - 1] = '\0';

                tmpstr = strchr (name, '=');
                if (tmpstr)
                        *tmpstr++ = '\0';

                for (option = option_list; option->name; option++)
			if (!strcmp (name, option->name))
				break;

		if (!option->name) {
			option = NULL;
			continue;
		}

		erase [i] = 1;
		numargs--;

		if (option->type == ORBIT_OPTION_NONE || tmpstr) {
			ORBit_option_set (option, tmpstr);
			option = NULL;
		}
	}

        for (i = j = 1; i < *argc; i++) {
		if (erase [i] == 1)
                        continue;

		if (j < numargs)
			argv [j++] = argv [i];
 		else
			argv [j++] = '\0';
        }

        *argc = numargs;

        g_free (erase);
}
