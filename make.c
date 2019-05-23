#include "global.h"

extern struct parsed_line parsed_line;
extern struct comp comp;

// protos
S4 run_process (U1 *name);
char *fgets_uni (char *str, int len, FILE *fptr);

S2 make_timestamp_dir ()
{
#if OS_LINUX || OS_WINDOWS
    if (run_process("mkdir .timest") != 0)
    {
        printf ("ERROR creating directory .timest\n");
        return (1);
    }
    else
    {
        return (0);
    }
#endif
}

S2 check_timestamp_dir ()
{
    struct stat st;

    if (stat (".timest", &st) != 0)
    {
        return (make_timestamp_dir ());
    }

    return (0);
}

S2 check_timestamp (U1 *filename)
{
    FILE *file_last;
    U1 *read;
    U1 buf[MAXSTRLEN];
    time_t timest_curr, timest_last;

    S4 str_len;

    U1 objfilename[MAXSTRLEN];
    FILE *objfile;

    struct stat st;

    U1 filename_last[MAXSTRLEN];


    // try to open object file
    strcpy (objfilename, filename);
    str_len = strlen (objfilename);
    objfilename[str_len - 1] = 'o';

    if ((objfile = fopen (objfilename, "r")) == NULL)
    {
        return (1);

        // build object file
    }

    // generate timestamp of source file
    if (stat (filename, &st) != 0)
    {
        printf ("check_timestamp: ERROR %s\n", filename);
		fclose (objfile);
        return (-1);
    }

    timest_curr = st.st_mtime;

    // read last timestamp of modification
    strcpy (filename_last, ".timest/");
    strcat (filename_last, filename);
    strcat (filename_last, ".ts");

    if ((file_last = fopen (filename_last, "r+")) == NULL)
    {
        printf ("ERROR: can't open last timestamp: '%s'\n", filename_last);

        if ((file_last = fopen (filename_last, "w")) == NULL)
        {
            printf ("ERROR: can't create last timestamp: '%s'\n", filename_last);
			fclose (objfile);
			return (-1);
        }

        if (fprintf (file_last, "%li\n", timest_curr) < 0)
        {
            printf ("ERROR: can't update timestamp: '%s'\n", filename_last);
            fclose (file_last);
            return (-1);
        }

        fclose (file_last);
		fclose (objfile);
        return (1);         // code for build
    }

    read = fgets_uni (buf, MAXSTRLEN, file_last);
    if (read == NULL)
    {
        printf ("ERROR: can't read last timestamp: '%s'\n", filename_last);

        fseek (file_last, SEEK_SET, 0L);

        if (fprintf (file_last, "%li\n", timest_curr) < 0)
        {
            printf ("ERROR: can't update timestamp: '%s'\n", filename_last);
            fclose (file_last);
			fclose (objfile);
            return (-1);
        }

        fclose (file_last);
		fclose (objfile);
        return (1);         // code for build
    }

    timest_last = atol (buf);

    if (timest_curr != timest_last)
    {
        // timestamps differ,
        // update last timestamp

        fseek (file_last, SEEK_SET, 0L);
        if (fprintf (file_last, "%li\n", timest_curr) < 0)
        {
            printf ("ERROR: can't update timestamp: '%s'\n", filename_last);
			fclose (file_last);
			fclose (objfile);
            return (-1);
        }

        fclose (file_last);

        return (1);     // code for build
    }
    else
    {
		fclose (objfile);
        return (0);     // code for don't build as file not changed
    }
}



S2 make (S2 force_build_all)
{
    S2 i, inc, str_len, error = 0;
    U1 run_shell[MAXSTRLEN];

    S2 ret;
    // S2 force_build_all = 0;

    if (check_timestamp_dir () != 0)
    {
        error = 1;
        printf ("ERROR: can't access .timest/ directory!");
        goto make_end;
    }

    for (i = 0; i <= parsed_line.sources_ind; i++)
    {
        ret = check_timestamp(parsed_line.sources[i]);
        // if (ret == -1)
        // {
           //  error = 1;
           // goto make_end;
       // }

        if (force_build_all == 0)
        {
            if (ret == 0) continue;
        }

        str_len = strlen (parsed_line.sources[i]);

        if (str_len >= 4)
        {
            if (parsed_line.sources[i][str_len - 1] == 'p' && parsed_line.sources[i][str_len - 2] == 'p' && parsed_line.sources[i][str_len - 3] == 'c' && parsed_line.sources[i][str_len - 4] == '.')
            {
                // .cpp = C++ compiler

                strcpy (run_shell, comp.cplusplus);
            }
            else
            {
                if (parsed_line.sources[i][str_len - 1] == 'h' && parsed_line.sources[i][str_len - 2] == '.')
                {
                    // global header file changed:
                    // force build all

                    force_build_all = 1;
                    continue;
                }
                else
                {
                    strcpy (run_shell, comp.c);
                }
            }
        }

        for (inc = 0; inc <= parsed_line.includes_ind; inc++)
        {
            strcat (run_shell, " -I");
            strcat (run_shell, parsed_line.includes[inc]);
            strcat (run_shell, " ");
        }

        strcat (run_shell, parsed_line.cflags);
        strcat (run_shell, " -c -o ");

        strcat (run_shell, parsed_line.sources[i]);

        str_len = strlen (run_shell);

        run_shell[str_len - 1] = 'o';   // change suffix to object file

        strcat (run_shell, " ");
        strcat (run_shell, parsed_line.sources[i]);

        // build

        printf ("'%s'\n", run_shell);

        if (run_process(run_shell) != 0)
        {
            printf ("build ERROR!\n");
            error = 1;
        }
    }

    // link

    if (remove (parsed_line.name) != 0)
    {
        printf ("build ERROR!\n");
        error = 1;
    }

    if (parsed_line.type == LIBRARY)
    {
        strcpy (run_shell, comp.c);
        strcat (run_shell, " -o out.o ");

        for (i = 0; i <= parsed_line.sources_ind; i++)
        {
            str_len = strlen (parsed_line.sources[i]);
            if (parsed_line.sources[i][str_len - 1] != 'h')
            {
                strcat (run_shell, parsed_line.sources[i]);
                str_len = strlen (run_shell);
                run_shell[str_len - 1] = 'o';   // change suffix to object file

                if (i <= parsed_line.sources_ind - 1) strcat (run_shell, " ");
            }
        }

        if (parsed_line.objects_ind > -1)
        {
            strcat (run_shell, " ");
            for (i = 0; i <= parsed_line.objects_ind; i++)
            {
                strcat (run_shell, parsed_line.objects[i]);

                if (i <= parsed_line.objects_ind - 1) strcat (run_shell, " ");
            }
        }

        if (strcmp (parsed_line.lflags, "") != 0)
        {
            strcat (run_shell, " ");
            strcat (run_shell, parsed_line.lflags);
        }

        printf ("'%s'\n", run_shell);

        if (run_process(run_shell) != 0)
        {
            printf ("build ERROR!\n");
            error = 1;
        }


        strcpy (run_shell, comp.archiver);
        strcat (run_shell, " ");
        strcat (run_shell, parsed_line.aflags);
        strcat (run_shell, " ");
        strcat (run_shell, parsed_line.name);
        strcat (run_shell, " out.o");

        printf ("'%s'\n", run_shell);

        if (run_process(run_shell) != 0)
        {
            printf ("build ERROR!\n");
            error = 1;
        }

        // ranlib

#if OS_LINUX
        strcpy (run_shell, "ranlib ");
        strcat (run_shell, parsed_line.name);

        printf ("'%s'\n", run_shell);

        if (run_process(run_shell) != 0)
        {
            printf ("build ERROR!\n");
            error = 1;
        }
#endif
    }
    else
    {
        strcpy (run_shell, comp.c);
        strcat (run_shell, " -o ");
        strcat (run_shell, parsed_line.name);
        strcat (run_shell, " ");

        for (i = 0; i <= parsed_line.sources_ind; i++)
        {
            str_len = strlen (parsed_line.sources[i]);
            if (parsed_line.sources[i][str_len - 1] != 'h')
            {
                strcat (run_shell, parsed_line.sources[i]);
                str_len = strlen (run_shell);
                run_shell[str_len - 1] = 'o';   // change suffix to object file

                if (i <= parsed_line.sources_ind - 1) strcat (run_shell, " ");
            }
        }

        if (parsed_line.objects_ind > -1)
        {
            strcat (run_shell, " ");
            for (i = 0; i <= parsed_line.objects_ind; i++)
            {
                strcat (run_shell, parsed_line.objects[i]);

                if (i <= parsed_line.objects_ind - 1) strcat (run_shell, " ");
            }
        }

        if (strcmp (parsed_line.lflags, "") != 0)
        {
            strcat (run_shell, " ");
            strcat (run_shell, parsed_line.lflags);
        }

        printf ("'%s'\n", run_shell);

        if (run_process(run_shell) != 0)
        {
            printf ("build ERROR!\n");
            error = 1;
        }
    }

 make_end:
    if (error == 0)
    {
        printf ("build: successfull\n");
        return (0);
    }
    else
    {
        printf ("build: failed ERROR!\n");
        return (1);
    }
}
