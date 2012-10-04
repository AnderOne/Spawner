#include <process.h>
#include <iostream>
#include <windows.h>
#include <winbase.h>

// Initializing winapi process with pipes and options
void CProcess::createProcess()
{
    ZeroMemory(&si, sizeof(si));

    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput = std_input.ReadPipe();
    si.hStdOutput = std_output.WritePipe();
    si.hStdError = std_error.WritePipe();

    si.lpDesktop = "";

    if (!options.show_gui)
    {
        si.dwFlags |= STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_HIDE;
    }

    // Extracting program name and generating cmd line
    char *cmd;
    const char *wd = (options.working_directory != "")?options.working_directory.c_str():NULL;
    string command_line;
    size_t  index_win = application.find_last_of('\\'), 
            index_nix = application.find_last_of('/');

    if (index_win != string::npos)
        command_line = application.substr(index_win + 1);
    else if (index_nix != string::npos)
        command_line = application.substr(index_nix + 1);
    else
        command_line = application;

    command_line = command_line + " " + (options.string_arguments==""?options.get_arguments():options.string_arguments);
    cmd = new char [command_line.size()+1];
    strcpy(cmd, command_line.c_str());
    
    if (options.silent_errors)
        SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);

    // check if program exists or smth like this
    // if not exists try to execute full cmd
    if ( !CreateProcess(application.c_str(),
        cmd,
        NULL, NULL,
        TRUE,
        PROCESS_CREATION_FLAGS,
        NULL, wd,
        &si, &process_info) )
    {
        if ( !CreateProcess(NULL,
        cmd,
        NULL, NULL,
        TRUE,
        PROCESS_CREATION_FLAGS,
        NULL, wd,
        &si, &process_info) )
        {
            DWORD le = GetLastError();
            delete[] cmd;
            throw("!!!");
        }
    }
    delete[] cmd;
}

// Initalizing jobobject objects

void CProcess::setRestrictions()
{
    /* implement restriction value check */
    hJob = CreateJobObject(NULL, NULL);
    DWORD le = GetLastError();

    // Memory and time limit
    JOBOBJECT_EXTENDED_LIMIT_INFORMATION joeli;
    memset(&joeli, 0, sizeof(joeli));
    joeli.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_DIE_ON_UNHANDLED_EXCEPTION;

    if (restrictions.get_restriction(restriction_memory_limit) != restriction_no_limit)
    {   
        joeli.JobMemoryLimit = restrictions.get_restriction(restriction_memory_limit);
        joeli.ProcessMemoryLimit = restrictions.get_restriction(restriction_memory_limit);
        joeli.BasicLimitInformation.LimitFlags |=
            JOB_OBJECT_LIMIT_PROCESS_MEMORY | JOB_OBJECT_LIMIT_JOB_MEMORY;
    }

    SetInformationJobObject(hJob, JobObjectExtendedLimitInformation, &joeli, sizeof(joeli));

    // Security limit
    if (restrictions.get_restriction(restriction_security_limit) != restriction_no_limit)
    {
        JOBOBJECT_BASIC_UI_RESTRICTIONS buir;
#ifdef JOB_OBJECT_UILIMIT_ALL
        buir.UIRestrictionsClass = JOB_OBJECT_UILIMIT_ALL;
#else
        buir.UIRestrictionsClass = 0x000000FF;
#endif
        SetInformationJobObject(hJob, JobObjectBasicUIRestrictions, &buir, sizeof(buir));
    }

    // Assigning created process to job object
    AssignProcessToJobObject(hJob, process_info.hProcess);

    hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 1, 1);

    JOBOBJECT_ASSOCIATE_COMPLETION_PORT joacp; 
    joacp.CompletionKey = (PVOID)COMPLETION_KEY; 
    joacp.CompletionPort = hIOCP; 
    SetInformationJobObject(hJob, JobObjectAssociateCompletionPortInformation, &joacp, sizeof(joacp));
}

void CProcess::wait()
{
    DWORD waitTime = INFINITE;
    if (restrictions.get_restriction(restriction_user_time_limit) != restriction_no_limit)
    {
        waitTime = restrictions.get_restriction(restriction_user_time_limit);
        WaitForSingleObject(process_info.hProcess, waitTime); // TODO test this
        //PostQueuedCompletionStatus(hIOCP, JOB_OBJECT_MSG_PROCESS_USER_TIME_LIMIT, COMPLETION_KEY, NULL);
        //and then terminate job object!!!
    }
    DWORD dwNumBytes, dwKey;
    LPOVERLAPPED completedOverlapped;  
    static CHAR buf[1024];
    //set msg
    int message = 0;
    do
    {     
        GetQueuedCompletionStatus(hIOCP, &dwNumBytes, &dwKey, &completedOverlapped, INFINITE);

        switch (dwNumBytes)
        {
        case JOB_OBJECT_MSG_NEW_PROCESS:
            break;
        case JOB_OBJECT_MSG_END_OF_PROCESS_TIME:
            message++;
            TerminateJobObject(hJob, 0);
            terminate_reason = terminate_reason_time_limit;
            process_status = process_finished_terminated;
            break;
        case JOB_OBJECT_MSG_PROCESS_WRITE_LIMIT:  
            message++;
            TerminateJobObject(hJob, 0);
            terminate_reason = terminate_reason_write_limit;
            process_status = process_finished_terminated;
            break;
        case JOB_OBJECT_MSG_EXIT_PROCESS:
            message++;
            //*message = TM_EXIT_PROCESS;
            break;
        case JOB_OBJECT_MSG_ABNORMAL_EXIT_PROCESS:
            message++;
            process_status = process_finished_abnormally;
            //*message = TM_ABNORMAL_EXIT_PROCESS;
            break;
        case JOB_OBJECT_MSG_PROCESS_MEMORY_LIMIT:
            message++;
            //*message = TM_MEMORY_LIMIT_EXCEEDED;
            TerminateJobObject(hJob, 0);
            terminate_reason = terminate_reason_memory_limit;
            process_status = process_finished_terminated;
            break;
        case JOB_OBJECT_MSG_JOB_MEMORY_LIMIT:
            message++;
            //*message = TM_MEMORY_LIMIT_EXCEEDED;
            TerminateJobObject(hJob, 0);
            terminate_reason = terminate_reason_memory_limit;
            process_status = process_finished_terminated;
            break;
        case JOB_OBJECT_MSG_PROCESS_USER_TIME_LIMIT:
            message++;
            //*message = TM_MEMORY_LIMIT_EXCEEDED;
            TerminateJobObject(hJob, 0);
            terminate_reason = terminate_reason_user_time_limit;
            process_status = process_finished_terminated;
            break;
        };
        //cout << dwNumBytes;

    } while (!message);
    GetQueuedCompletionStatus(hIOCP, &dwNumBytes, &dwKey, &completedOverlapped, INFINITE);;
    std_output.wait_for_pipe(100);
    std_error.wait_for_pipe(100);
    WaitForSingleObject(process_info.hProcess, 10000);// TODO: get rid of this
    running = false;
}

// Finalization
void CProcess::finish()
{
    get_report();
    std_output.finish();
    std_error.finish();
    CloseHandleSafe(hIOCP);
    CloseHandleSafe(hJob);
    CloseHandleSafe(process_info.hProcess);
    CloseHandleSafe(process_info.hThread);
    CloseHandleSafe(check);
}

CProcess::CProcess(const string &file):application(file), process_status(process_not_started), terminate_reason(terminate_reason_not_terminated),
    std_input(STD_INPUT), std_output(STD_OUTPUT), std_error(STD_ERROR), running(false)
{
	//getting arguments from list
	//working dir, etc
}

void CProcess::SetArguments()
{
	//is this required?..
	//after-constructor argument changing
}

int CProcess::Run()
{
    // deprecated
    createProcess();
    setRestrictions();
    running = true;

    DWORD w = ResumeThread(process_info.hThread);

    std_output.bufferize();
    std_error.bufferize();

    check = CreateThread(NULL, 0, check_limits, this, 0, NULL);
    // create thread, waiting for completition
    wait();
    int exit_code = get_exit_code();
    finish();

	return exit_code;
}

void CProcess::RunAsync()
{
    createProcess();
    setRestrictions();    
    running = true;

    DWORD w = ResumeThread(process_info.hThread);

    std_output.bufferize();
    std_error.bufferize();

    check = CreateThread(NULL, 0, check_limits, this, 0, NULL);
    completition = CreateThread(NULL, 0, process_completition, this, 0, NULL);
    //WaitForSingleObject(completition, 100); // TODO fix this
    //create in another thread waiting function
}
CProcess::~CProcess()
{
	//kills process if it is running
}
thread_return_t CProcess::process_completition(thread_param_t param)
{
    CProcess *self = (CProcess *)param;
    self->wait();
    return 0;
}

thread_return_t CProcess::check_limits(thread_param_t param)
{
    CProcess *self = (CProcess *)param;
    DWORD t;
    JOBOBJECT_BASIC_AND_IO_ACCOUNTING_INFORMATION bai;

    if (self->restrictions.get_restriction(restriction_processor_time_limit) == restriction_no_limit &&
        self->restrictions.get_restriction(restriction_user_time_limit) == restriction_no_limit &&
        self->restrictions.get_restriction(restriction_write_limit) == restriction_no_limit)
        return 0;

    t = GetTickCount();
    while (1)
    {
        BOOL rs = QueryInformationJobObject(self->hJob, JobObjectBasicAndIoAccountingInformation, &bai, sizeof(bai), NULL);
        if (!rs)
            break;
        //bai.BasicInfo.ThisPeriodTotalKernelTime

        if (self->restrictions.get_restriction(restriction_write_limit) != restriction_no_limit && 
            bai.IoInfo.WriteTransferCount > (1024 * 1024) * self->restrictions.get_restriction(restriction_write_limit))
        {
            PostQueuedCompletionStatus(self->hIOCP, JOB_OBJECT_MSG_PROCESS_WRITE_LIMIT, COMPLETION_KEY, NULL);
            break;
        }

        if (self->restrictions.get_restriction(restriction_processor_time_limit) != restriction_no_limit && 
            (DOUBLE)bai.BasicInfo.TotalUserTime.QuadPart > SECOND_COEFF * self->restrictions.get_restriction(restriction_processor_time_limit))
        {
            PostQueuedCompletionStatus(self->hIOCP, JOB_OBJECT_MSG_END_OF_PROCESS_TIME, COMPLETION_KEY, NULL);
            break;
        }
        if (self->restrictions.get_restriction(restriction_user_time_limit) != restriction_no_limit && 
            (GetTickCount() - t) > self->restrictions.get_restriction(restriction_user_time_limit))
        {
            PostQueuedCompletionStatus(self->hIOCP, JOB_OBJECT_MSG_PROCESS_USER_TIME_LIMIT, COMPLETION_KEY, NULL);//freezed
            break;
        }
        Sleep(1);
    }
    return 0;
}

unsigned long CProcess::get_exit_code()
{
    DWORD dwExitCode = 0;
    if (!GetExitCodeProcess(process_info.hProcess, &dwExitCode))
        throw "!!!";
    return dwExitCode;
}

void CProcess::suspend()
{
    if (get_process_status() != process_still_active)
        return;
    dumpThreads(true);
    process_status = process_suspended;
    //SuspendThread(process_info.hThread);
}

void CProcess::resume()
{
    if (get_process_status() != process_suspended)
        return;
    while (!threads.empty())
    {
        handle_t handle = threads.front();
        threads.pop_front();
        ResumeThread(handle);
        CloseHandle(handle);
    }
    process_status = process_still_active;
    get_process_status();
}

void CProcess::dumpThreads(bool suspend)
{
    //if process is active and started!!!
    if (!is_running())
        return;
    //while (threads.empty())
    //{
    //CloseHandle(threads.begin()
    //}
    threads.clear();
    HANDLE h = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (h != INVALID_HANDLE_VALUE)
    {
        THREADENTRY32 te;
        te.dwSize = sizeof(te);
        if (Thread32First(h, &te)) 
        {
            do {
                if (te.dwSize >= FIELD_OFFSET(THREADENTRY32, th32OwnerProcessID) +
                    sizeof(te.th32OwnerProcessID) && te.th32OwnerProcessID == process_info.dwProcessId) 
                {
                    handle_t handle = OpenThread(THREAD_ALL_ACCESS, FALSE, te.th32ThreadID);
                    if (suspend)
                        SuspendThread(handle);
                    //may be close here??
                    threads.push_back(handle);
                    /*printf("Process 0x%04x Thread 0x%04x\n",
                    te.th32OwnerProcessID, te.th32ThreadID);*/
                }
                te.dwSize = sizeof(te);
            } while (Thread32Next(h, &te));
        }
        CloseHandle(h);
    }
}

process_status_t CProcess::get_process_status()
{
    // renew process status
    //cout << process_status << endl;
    // ************************* DIRTY HACK *************************//
    if (terminate_reason != terminate_reason_not_terminated)
        process_status = process_finished_terminated;
    // ************************* END OF HACK ************************//
    if (process_status & process_finished || process_status == process_suspended)
        return process_status;
    unsigned long exitcode = get_exit_code();
    if (exitcode == exit_code_still_active)
        process_status = process_still_active;
    else
        process_status = process_finished_abnormally;
    if (exitcode == 0)
        process_status = process_finished_normal;
    return process_status;
}

istringstream & CProcess::stdoutput()
{
    return std_output.stream();
}

istringstream & CProcess::stderror()
{
    return std_error.stream();
}

bool CProcess::is_running()
{
    if (running)
        return process_is_active;
    return (bool)(get_process_status() & process_is_active);
}

exception_t CProcess::get_exception()
{
    if (get_process_status() == process_finished_abnormally)
        return (exception_t)get_exit_code();
    else return exception_no_exception;
}

CReport CProcess::get_report()
{
    JOBOBJECT_BASIC_AND_IO_ACCOUNTING_INFORMATION bai;
    if (hJob == INVALID_HANDLE_VALUE)
        return report;
    if (!QueryInformationJobObject(hJob, JobObjectBasicAndIoAccountingInformation, &bai, sizeof(bai), NULL))
    {
        //throw GetWin32Error("QueryInformationJobObject");
    }

    report.processor_time = bai.BasicInfo.TotalUserTime.QuadPart;
    report.write_transfer_count = bai.IoInfo.WriteTransferCount;

    JOBOBJECT_EXTENDED_LIMIT_INFORMATION xli;
    if (!QueryInformationJobObject(hJob, JobObjectExtendedLimitInformation, &xli, sizeof(xli), NULL))
    {
        //throw GetWin32Error("QueryInformationJobObject");
    }

    report.peak_memory_used = xli.PeakJobMemoryUsed;

    report.application_name = application;

    report.process_status = get_process_status();
    report.exception = get_exception();
    report.terminate_reason = get_terminate_reason();
    report.exit_code = get_exit_code();

    report.options = options;
    report.restrictions = restrictions;

    return report;
}

terminate_reason_t CProcess::get_terminate_reason()
{
    return terminate_reason;
}

void CProcess::set_restrictions(const CRestrictions &Restrictions)
{
    // TODO m.b. test restrictions here
    restrictions = Restrictions;
}

void CProcess::set_options(const COptions &Options)
{
    options = Options;
}

void CProcess::Finish()
{
    finish();
}

bool CProcess::Wait(const unsigned long &ms_time)
{
    return WaitForSingleObject(completition, ms_time) != 0;
}