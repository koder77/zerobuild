#include "global.h"

#if OS_WINDOWS
    #include <windows.h>
	#include <stdlib.h>
    #include <errno.h>
	
	#define WEXITSTATUS(status) (((status) &0xff00) >> 8)
#endif

/* LINUX ------------------------------------------------------------ */

#if OS_LINUX || OS_WINDOWS

S4 run_process (U1 *name)
{
    S4 ret;

    ret = system (name);
	if (ret == -1)
	{
		perror ("run_shell:\n");
	}
	else
	{
		ret = WEXITSTATUS (ret);
	}

    return (ret);
}

#endif

/* WINDOWS ---------------------------------------------------------- */
/*
#if OS_WINDOWS

S8 run_process (U1 *name)
{
    S4 ret;
    LPDWORD *exit_code = NULL;

    STARTUPINFO start_info;
    PROCESS_INFORMATION process_info;

	GetStartupInfo (&start_info);

    ret = CreateProcess (NULL, name, NULL, NULL, FALSE, 0, NULL, NULL, &start_info, &process_info);
    if (ret == 0)
    {
        return (-1);
    }
    
    WaitForSingleObject( process_info.hProcess, INFINITE );

    GetExitCodeProcess(process_info.hProcess, exit_code);
    return ((S8) exit_code);
}

#endif
*/

