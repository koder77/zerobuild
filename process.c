#include "global.h"

#if OS_WINDOWS
    #include <windows.h>
    #include <errno.h>
#endif

/* LINUX ------------------------------------------------------------ */

#if OS_LINUX

S4 run_process (U1 *name)
{
    S4 ret;

    ret = system (name);
    return (ret);
}

#endif

/* WINDOWS ---------------------------------------------------------- */

#if OS_WINDOWS

S4 run_process (U1 *name)
{
    S4 ret;
    S4 exit_code;
    
    STARTUPINFO start_info;
    PROCESS_INFORMATION process_info;

	GetStartupInfo (&start_info);

    ret = CreateProcess (NULL, name, NULL, NULL, FALSE, 0, NULL, NULL, &start_info, &process_info);
    if (ret == 0)
    {
        return (-1);
    }
    // Wait until child process exits.
    WaitForSingleObject( process_info.hProcess, INFINITE );

    GetExitCodeProcess(process_info.hProcess, &exit_code);
    return (exit_code);
}

#endif
