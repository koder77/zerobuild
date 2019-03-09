#include "global.h"


struct parsed_line parsed_line;
struct comp comp;

S2 sources_line = 0;
S2 includes_line = 0;
S2 objects_line = 0;

char *fgets_uni (char *str, int len, FILE *fptr)
{
    int ch, nextch;
    int i = 0, eol = FALSE;
    char *ret;

    ch = fgetc (fptr);
    if (feof (fptr))
    {
        return (NULL);
    }
    while (! feof (fptr) || i == len - 2)
    {
        switch (ch)
        {
            case '\r':
                /* check for '\r\n\' */

                nextch = fgetc (fptr);
                if (! feof (fptr))
                {
                    if (nextch != '\n')
                    {
                        ungetc (nextch, fptr);
                    }
                }
                str[i] = '\n';
                i++; eol = TRUE;
                break;

            case '\n':
                /* check for '\n\r\' */

                nextch = fgetc (fptr);
                if (! feof (fptr))
                {
                    if (nextch != '\r')
                    {
                        ungetc (nextch, fptr);
                    }
                }
                str[i] = '\n';
                i++; eol = TRUE;
                break;

            default:
				str[i] = ch;
				i++;

                break;
        }

        if (eol)
        {
            break;
        }

        ch = fgetc (fptr);
    }

    if (feof (fptr))
    {
//        str[i] = '\n';
//        i++;
        str[i] = '\0';
    }
    else
    {
        str[i] = '\0';
    }

    ret = str;
    return (ret);
}

void convtabs (U1 *str)
{
    S2 i, end;

    end = strlen (str) - 1;

    for (i = 0; i <= end; i++)
    {
        if (str[i] == '\t')
        {
            str[i] = ' ';
        }
    }
}

void remove_newline (U1 *str)
{
    S2 line_len;

    line_len = strlen (str) - 1;
    if (str[line_len] == '\n')
    {
        str[line_len] = '\0';
    }
}

S2 search_char (U1 *str, U1 search_char, S2 search_start)
{
    S2 i, str_len;

    str_len = strlen (str);
    if (search_start >= str_len || search_start < 0)
    {
        return (-1);        // index out of range
    }

    for (i = search_start; i < str_len; i++)
    {
        if (str[i] == search_char)
        {
            return (i);     // return position of char in string
        }
    }

    return (-1);    // char not found in string
}

S2 get_assign (U1 *str, U1 *ret, S2 ret_size, S2 start)
{
    S2 i, str_len, ret_ind = -1;
    U1 found_end = 0;
    U1 found_quote = 0;

    str_len = strlen (str);
    if (start >= str_len || start < 0)
    {
        return (-1);        // start out of range
    }

    i = start;
    while (found_end == 0)
    {
        if (str[i] != SPACE)
        {
            if (str[i] == QUOTE || str[i] == COMMA || str[i] == BLOCK_CLOSE)
            {
                if (str[i] == QUOTE)
                {
                    found_quote++;
                }
                else
                {
                    if (found_quote == 1)
                    {
                        if (str[i] != COMMA)
                        {
                            found_end = 1;
                        }
                    }
                    else
                    {
                        found_end = 1;
                    }
                }
            }
            else
            {
                if (ret_ind < ret_size - 2)
                {
                    ret_ind++;
                }
                else
                {
                    printf ("get_assign: ERROR return buffer overflow!\n");
                    return (1);
                }

                ret[ret_ind] = str[i];
            }

            if (found_quote == 2)
            {
                found_end = 1;
            }
        }
        else
        {
            if (ret_ind > -1)
            {
                // already found some chars, got string end
                if (found_quote == 0)
                {
                    found_end = 1;
                }
                else
                {
                    if (ret_ind < ret_size - 2)
                    {
                        ret_ind++;
                    }
                    else
                    {
                        printf ("get_assign: ERROR return buffer overflow!\n");
                        return (1);
                    }

                    ret[ret_ind] = str[i];
                }
            }
        }

        if (i < str_len)
        {
            i++;
        }
        else
        {
            found_end = 1;
        }
    }
    ret_ind++;
    ret[ret_ind] = '\0';

    return (0);
}

get_assign_in_quote (U1 *str, U1 *ret, S2 ret_size, S2 start)
{
	S2 i, str_len, ret_ind = -1;
    U1 found_end = 0;
    U1 found_quote = 0;

    str_len = strlen (str);
    if (start >= str_len || start < 0)
    {
        return (-1);        // start out of range
    }

    i = start;
    while (found_end == 0)
    {
		if (str[i] == QUOTE)
		{
			if (found_quote == 0)
			{
				found_quote = 1;

				if (i < str_len)
				{
					i++;
				}
				else
				{
					return (-1);
				}
			}
			else
			{
				found_end = 1;
			}
		}

		if (! found_end)
		{
			if (ret_ind < ret_size - 2)
			{
				ret_ind++;
			}
			else
			{
				printf ("get_assign: ERROR return buffer overflow!\n");
				return (1);
			}

			ret[ret_ind] = str[i];

			if (i < str_len)
			{
				i++;
			}
			else
			{
				return (-1);
			}
		}
    }
	ret_ind++;
    ret[ret_ind] = '\0';
	return (0);
}
S2 parse_line (U1 *line)
{
    S2 line_len;
    S2 block_start = -1, block_end = -1, pos, search_start;
    char *ret;

    U1 buf[4096];

    U1 line_end = 0;

    line_len = strlen (line);


    // check if line begins with a #

    if (line[0] == '#')
    {
        return (0);     // line is a comment
    }



    // not a comment, here we go:

    // check for type block

    search_start = 0;
    pos = search_char (line, BLOCK_OPEN, search_start);
    if (pos >= 0)
    {
        block_start = pos;
        pos = search_char (line, BLOCK_CLOSE, pos + 1);
        if (pos == -1)
        {
            // syntax error, no block close found

            printf ("syntax ERROR no block close found!\n");
            return (1);
        }
        else
        {
            if (pos != line_len - 1)
            {
                printf ("syntax ERROR block close not at end of line!\n");
                return (1);
            }
            else
            {
                block_end = pos;
            }
        }

        if (block_start >= 0 && block_end >= 0)
        {
            // found type block, do parse block

            ret = strstr (line, TYPE_EXECUTABLE);
            if (ret != 0)
            {
                if (parsed_line.type == LIBRARY)
                {
                    printf ("syntax ERROR: type must be one of executable OR library!\n");
                    return (1);
                }

                parsed_line.type = EXECUTABLE;
            }

            ret = strstr (line, TYPE_LIBRARY);
            if (ret != 0)
            {
                // pos = ret - line;

                if (parsed_line.type == EXECUTABLE)
                {
                    printf ("syntax ERROR: type must be one of executable OR library!\n");
                    return (1);
                }

                parsed_line.type = LIBRARY;
            }


            ret = strstr (line, STATIC);
            if (ret != 0)
            {
                if (parsed_line.buildtype == BUILD_SHARED)
                {
                    printf ("syntax ERROR: buildtype must be one of static OR shared!\n");
                    return (1);
                }

                parsed_line.buildtype = BUILD_STATIC;
            }

            ret = strstr (line, SHARED);
            if (ret != 0)
            {
                // pos = ret - line;

                if (parsed_line.buildtype == BUILD_STATIC)
                {
                    printf ("syntax ERROR: buildtype must be one of static OR shared!\n");
                    return (1);
                }

                parsed_line.buildtype = BUILD_SHARED;
            }

            ret = strstr (line, NAME);
            if (ret != 0)
            {
                pos = (U1 *) ret - line;

                search_start = pos + 1;
                pos = search_char (line, ASSIGN, search_start);
                if (pos == -1)
                {
                    printf ("syntax ERROR: no assign found for name!\n");
                    return (1);
                }

                if (get_assign (line, parsed_line.name, 4096, pos + 1) != 0)
                {
                    printf ("syntax ERROR: can't get assign string!\n");
                    return (1);
                }

                printf ("name = '%s'\n", parsed_line.name);
            }
        }
    }
    else
    {
        pos = search_char (line, ASSIGN, search_start);
        if (pos != -1)
        {
            if (sources_line >= 1)
            {
                // new assign, not a sources line anymore so switch flag off
                sources_line = 0;
            }

            if (includes_line >= 1)
            {
                // // new assign, not a includes line anymore so switch flag off
                includes_line = 0;
            }
        }


        // check assign

        ret = strstr (line, SOURCES);
        if (ret != 0 || sources_line >= 1)
        {
            printf ("sources parser running...\n");

            sources_line++;               // is source code line

            if (sources_line == 1)
            {
                pos = (U1 *) ret - line;

                search_start = pos + 1;
                pos = search_char (line, ASSIGN, search_start);
                if (pos == -1)
                {
                    printf ("syntax ERROR: no assign found for sources!\n");
                    return (1);
                }
            }
            else
            {
                pos = -1;
            }

            // sources = foo.c, bar.c, test.c


            while (line_end == 0)
            {
                if (parsed_line.sources_ind < MAXSOURCES)
                {
                    parsed_line.sources_ind++;
                }

                if (get_assign (line, parsed_line.sources[parsed_line.sources_ind], 4096, pos + 1) != 0)
                {
                    if (sources_line == 1)
                    {
                        printf ("syntax ERROR: can't get assign string!\n");
                        return (1);
                    }
                }

                pos = search_char (line, COMMA, pos + 1);
                if (pos == -1)
                {
                    line_end = 1;
                }

                if (parsed_line.sources[parsed_line.sources_ind][0] == '\0')
                {
                    parsed_line.sources_ind--;
                    sources_line = 0;
                }
                else
                {
                    printf ("source: '%s'\n", parsed_line.sources[parsed_line.sources_ind]);
                }
            }
        }

        ret = strstr (line, INCLUDES);
        if (ret != 0 || includes_line >= 1)
        {
            printf ("includes parser running...\n");

            includes_line++;

            if (includes_line == 1)
            {
                pos = (U1 *) ret - line;

                search_start = pos + 1;
                pos = search_char (line, ASSIGN, search_start);
                if (pos == -1)
                {
                    printf ("syntax ERROR: no assign found for includes!\n");
                    return (1);
                }
            }
            else
            {
                pos = -1;
            }

            // sources = foo.c, bar.c, test.c


            while (line_end == 0)
            {
                if (parsed_line.includes_ind < MAXINCLUDES)
                {
                    parsed_line.includes_ind++;
                }

                if (get_assign (line, parsed_line.includes[parsed_line.includes_ind], 4096, pos + 1) != 0)
                {
                    if (includes_line == 1)
                    {
                        printf ("syntax ERROR: can't get assign string!\n");
                        return (1);
                    }
                }

                pos = search_char (line, COMMA, pos + 1);
                if (pos == -1)
                {
                    line_end = 1;
                }

                if (parsed_line.includes[parsed_line.includes_ind][0] == '\0')
                {
                    parsed_line.includes_ind--;
                    includes_line = 0;
                }
                else
                {
                    printf ("include: '%s'\n", parsed_line.includes[parsed_line.includes_ind]);
                }
            }
        }

        ret = strstr (line, OBJECTS);
        if (ret != 0 || objects_line >= 1)
        {
            printf ("objects parser running...\n");

            objects_line++;

            if (objects_line == 1)
            {
                pos = (U1 *) ret - line;

                search_start = pos + 1;
                pos = search_char (line, ASSIGN, search_start);
                if (pos == -1)
                {
                    printf ("syntax ERROR: no assign found for objects!\n");
                    return (1);
                }
            }
            else
            {
                pos = -1;
            }

            // objects = foo.o, bar.o, test.o


            while (line_end == 0)
            {
                if (parsed_line.objects_ind < MAXOBJECTS)
                {
                    parsed_line.objects_ind++;
                }

                if (get_assign (line, parsed_line.objects[parsed_line.objects_ind], 4096, pos + 1) != 0)
                {
                    if (objects_line == 1)
                    {
                        printf ("syntax ERROR: can't get assign string!\n");
                        return (1);
                    }
                }

                pos = search_char (line, COMMA, pos + 1);
                if (pos == -1)
                {
                    line_end = 1;
                }

                if (parsed_line.objects[parsed_line.objects_ind][0] == '\0')
                {
                    parsed_line.objects_ind--;
                    objects_line = 0;
                }
                else
                {
                    printf ("object: '%s'\n", parsed_line.objects[parsed_line.objects_ind]);
                }
            }
        }


        ret = strstr (line, CFLAGS);
        if (ret != 0)
        {
            pos = (U1 *) ret - line;

            search_start = pos + 1;
            pos = search_char (line, ASSIGN, search_start);
            if (pos == -1)
            {
                printf ("syntax ERROR: no assign found for cflags!\n");
                return (1);
            }

            if (get_assign_in_quote (line, parsed_line.cflags, 4096, pos + 1) != 0)
            {
                printf ("syntax ERROR: can't get assign string!\n");
                return (1);
            }

            printf ("cflags = '%s'\n", parsed_line.cflags);
        }

        ret = strstr (line, CPLUSPLUSFLAGS);
        if (ret != 0)
        {
            pos = (U1 *) ret - line;

            search_start = pos + 1;
            pos = search_char (line, ASSIGN, search_start);
            if (pos == -1)
            {
                printf ("syntax ERROR: no assign found for c++flags!\n");
                return (1);
            }

            if (get_assign_in_quote (line, parsed_line.cplusplusflags, 4096, pos + 1) != 0)
            {
                printf ("syntax ERROR: can't get assign string!\n");
                return (1);
            }

            printf ("c++flags = '%s'\n", parsed_line.cplusplusflags);
        }

        ret = strstr (line, CCOMPILER);
        if (ret != 0)
        {
            pos = (U1 *) ret - line;

            search_start = pos + 1;
            pos = search_char (line, ASSIGN, search_start);
            if (pos == -1)
            {
                printf ("syntax ERROR: no assign found for ccompiler!\n");
                return (1);
            }

            if (get_assign (line, comp.c, 4096, pos + 1) != 0)
            {
                printf ("syntax ERROR: can't get assign string!\n");
                return (1);
            }

            printf ("ccompiler = '%s'\n", comp.c);
        }

        ret = strstr (line, CPLUSPLUSCOMPILER);
        if (ret != 0)
        {
            pos = (U1 *) ret - line;

            search_start = pos + 1;
            pos = search_char (line, ASSIGN, search_start);
            if (pos == -1)
            {
                printf ("syntax ERROR: no assign found for c++compiler!\n");
                return (1);
            }

            if (get_assign (line, comp.cplusplus, 4096, pos + 1) != 0)
            {
                printf ("syntax ERROR: can't get assign string!\n");
                return (1);
            }

            printf ("c++compiler = '%s'\n", comp.cplusplus);
        }

        ret = strstr (line, ARCHIVER);
        if (ret != 0)
        {
            pos = (U1 *) ret - line;

            search_start = pos + 1;
            pos = search_char (line, ASSIGN, search_start);
            if (pos == -1)
            {
                printf ("syntax ERROR: no assign found for archiver!\n");
                return (1);
            }

            if (get_assign (line, comp.archiver, 4096, pos + 1) != 0)
            {
                printf ("syntax ERROR: can't get assign string!\n");
                return (1);
            }

            printf ("archiver = '%s'\n", comp.archiver);
        }

        ret = strstr (line, AFLAGS);
        if (ret != 0)
        {
            pos = (U1 *) ret - line;

            search_start = pos + 1;
            pos = search_char (line, ASSIGN, search_start);
            if (pos == -1)
            {
                printf ("syntax ERROR: no assign found for aflags!\n");
                return (1);
            }

            if (get_assign (line, parsed_line.aflags, 4096, pos + 1) != 0)
            {
                printf ("syntax ERROR: can't get assign string!\n");
                return (1);
            }

            printf ("aflags = '%s'\n", parsed_line.aflags);
        }

        ret = strstr (line, LFLAGS);
        if (ret != 0)
        {
            pos = (U1 *) ret - line;

            search_start = pos + 1;
            pos = search_char (line, ASSIGN, search_start);
            if (pos == -1)
            {
                printf ("syntax ERROR: no assign found for lflags!\n");
                return (1);
            }

            if (get_assign_in_quote (line, parsed_line.lflags, 4096, pos + 1) != 0)
            {
                printf ("syntax ERROR: can't get assign string!\n");
                return (1);
            }

            printf ("lflags = '%s'\n", parsed_line.lflags);
        }
    }
    return (0);
}


S2 parse_makefile (U1 *makefile)
{
    FILE *fptr;
    U1 buf[MAXSTRLEN + 1];
    U1 *read;
    U1 ok = TRUE;

    if ((fptr = fopen (makefile, "r")) == NULL)
    {
        printf ("ERROR: can't open makefile: '%s'\n", makefile);
        return (1);
    }

    while (ok)
    {
        read = fgets_uni (buf, MAXSTRLEN, fptr);
        if (read != NULL)
        {
            remove_newline (buf);
            convtabs (buf);     // tabs to spaces
            if (parse_line (buf) != 0)
            {
                printf ("ERROR parsing makefile '%s'\n", makefile);
                return (1);
            }
        }
        else
        {
            ok = FALSE;
        }
    }
    fclose (fptr);
    return (0);
}


int main (int ac, char *av[])
{
    int ret;
    int force_build_all = 0;

    parsed_line.sources_ind = -1;  // undefined index
    parsed_line.includes_ind = -1;
    parsed_line.objects_ind = -1;

    parsed_line.type = NOTDEF;
    parsed_line.buildtype = NOTDEF;

    strcpy (comp.c, "");
    strcpy (comp.cplusplus, "");
    strcpy (comp.archiver, "");
    strcpy (parsed_line.cplusplusflags, "");
    strcpy (parsed_line.cflags, "");
    strcpy (parsed_line.aflags, "");
    strcpy (parsed_line.lflags, "");

    printf ("zerobuild V1.0.1\n");

	printf ("DEBUG: ac: %i\n", ac);

	if (ac == 1)
    {
        ret = parse_makefile ("zerobuild.txt");
    }
    else
    {
        if (ac > 3)
        {
            printf ("zerobuild <makefile> <force>\n");
            exit (1);
        }

        if (ac == 2)
        {
			printf ("DEBUG: av[1]: ''%s'\n", av[1]);
			if (strcmp (av[1], "force") == 0)
            {
                printf ("FORCE FULL BUILD!\n");
                force_build_all = 1;
				ret = parse_makefile ("zerobuild.txt");
            }
			else
			{
            	ret = parse_makefile (av[1]);
			}
        }
        if (ac == 3)
        {
            if (strcmp (av[2], "force") == 0)
            {
                printf ("FORCE FULL BUILD!\n");
                force_build_all = 1;
            }
            else
            {
                // unknown flag!!!
                printf ("zerobuild <makefile> <force>\n");
                exit (1);
            }
            ret = parse_makefile (av[1]);
        }
    }

    if (ret == 1)
    {
        exit (1);
    }

    ret = make (force_build_all);
    if (ret == 1)
    {
        exit (1);
    }

    exit (0);       // no error
}
