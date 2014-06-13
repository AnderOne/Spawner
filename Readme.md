������ ��������������� ���������� �������� "Spawner"
=======================================

������ ��������������� ��� ����� ������� ����������� ������������ [Cats](https://github.com/klenin/cats-judge).
������ ������������ ��������� ������ �������������. ����� ������������� �������� ������ `--legacy` ��� ���������� ��������� `SP_LEGACY`. ��������� �������� ����� ���������� � ������ �������������� ������. �������� ��-��������� `sp00`.
������� �� ������ �� ��� ����� �������� �������� ������ � ������ `--help` ��� `-h`.

sp99
---------------------------------------
����������� ������ sp ������ �� ������ ����� �������.
```
sp [�����] ���_���������� [���������_����������]
```
|�����              |���������� ��������� |                                                                           |
|-------------------|---------------------|---------------------------------------------------------------------------|
|  -ml:[n]          |  SP_MEMORY_LIMIT    |  ������������ ����� ����������� ������, ���������� �������� (� Mb).       |
|  -tl:[n]          |  SP_TIME_LIMIT      |  ������������ ����� ���������� �������� � ���������������� ������ (� ���).|
|  -d:[n]           |  SP_DEADLINE        |  ����� ����������� �������, ����������� �������� (� ���).                 |
|  -wl:[n]          |  SP_WRITE_LIMIT     |  ������������ ����� ������, ������� ����� ���� ������� ��������� (� Mb).  |
|  -u:[user@domain] |  SP_USER            |  ��� ������������ � �������: User[@Domain]                                |
|  -p:[password]    |  SP_PASSWORD        |  ������.                                                                  |
|  -runas:[0|1]     |  SP_RUNAS           |  ������������ ������ RunAs ��� ������� ��������.                          |
|  -s:[n]           |  SP_SECURITY_LEVEL  |  ������� ������������. ����� ��������� �������� 0 ��� 1.                  |
|  -hr:[0|1]        |  SP_HIDE_REPORT     |  �� ���������� �����.                                                     |
|  -ho:[0|1]        |  SP_HIDE_OUTPUT     |  �� ���������� �������� ����� (STDOUT) ����������.                        |
|  -sr:[file]       |  SP_REPORT_FILE     |  ��������� ����� � ����.                                                  |
|  -so:[file]       |  SP_OUTPUT_FILE     |  ��������� �������� ����� � ����.                                         |
|  -i:[file]        |  SP_INPUT_FILE      |  �������� ������� ����� �� �����.                                         |
                                                                                                                      |

 ����������: ��������� ��������� ������ ����������� �������� ���������� ���������.

                   
������ ����� ������.
```
[������ ������]
--------------- Spawner report ---------------
Application:            [Application]
Parameters:             [Parameters]
SecurityLevel:          [SecurityLevel]
CreateProcessMethod:    [CreateProcessMethod]
UserName:               [UserName]
UserTimeLimit:          [UserTimeLimit]
Deadline:               [Deadline]
MemoryLimit:            [MemoryLimit]
WriteLimit:             [WriteLimit]
----------------------------------------------
UserTime:               [UserTime]
PeakMemoryUsed:         [PeakMemoryUsed]
Written:                [Written]
TerminateReason:        [TerminateReason]
ExitStatus:             [ExitStatus]
----------------------------------------------
SpawnerError:           [SpawnerError]
```

* Application - ��� ����������
* Parameters - ��������� ����������
* SecurityLevel - ������� ������. ����� ���� 0 (��-���������) ��� 1.
    ������� 1 �������� ������ ��:
    * ������ �� �������, ���������� �� ������, ������������ ��� ���������� ����������
    * ������, ������, �������� ������ ������ 
    * ��������� ��������� ���������� ����� SystemParametersInfo 
    * ��������� ���������� ������ ����� ChangeDisplaySettings 
    * �������� ����� ������� ������ ��� ������������ ����� ����
    * ������������� USER-�������� (��������, HWND), ��������� �������� ���������� 
    * ������� � ����� ������� ������ (global atom table)

* CreateProcessMethod - ����� �������� ��������. ����� ����: "CreateProcess", "CreateProcessAsUser", "RunAs service"
        * "CreateProcess" - ������� ����������� �� �������� ������������
        * "CreateProcessAsUser" - ������� ����������� �� ��������� ������������. 
        * "RunAs service" - ������� ����������� ��� ������ ������� RunAs.
        
        ������ �������� �� ������� ������������ ������� ����� ������ ����������, � ������:
            SeTcbPrivilege (������ � ������ ������������ �������)
            SeAssignPrimaryTokenPrivilege (������ ������� ������ ��������)
            SeIncreaseQuotaPrivilege (���������� ����)
        ������ � ������� RunAs � ��������� ������������� ����� �������� �� ���������, 
        �� �� �� ������� ����������� ���������� � ����� ���� �������� �� �������� ������������, 
        ������ ��� ����� ������ ���� ������� ������ RunAs (SecondaryLogon).
 
* UserName - ��� ������������ ��� ������� ��� ������� �������� ������� � �������: User[@Domain]
* UserTimeLimit - ������������ ����� � ���. ���������� �������� � ���������������� ������ �� ��������� 
        �������� ������� �����������. �� ���������: "Infinity".
* Deadline - ����� � ���., ������� �������� ��������. �� ���������: "Infinity".
        ���������� �� TimeLimit ���, ��� ��� ���������� �����.
        ���� ������� ���������� ������������ ����/�����, ���������� � ��������� �������� ��� ������� �����������, 
        �� ������� ����� ���������� ������������� ����� �������� �� TimeLimit. 
        ��� �������������� ������ �������� ����� ���������� DeadLine.
* MemoryLimit - ������������ ����� ���������� ������ �������� � Mb. �� ���������: "Infinity"
* WriteLimit - ������������ ����� ����������, ������� ����� ���� ������� ��������� � Mb. �� ���������: "Infinity"

* UserTime - ����������� ����� ���������� �������� � ���. � ��������� �� 10e-3.
* PeakMemoryUsed - ������������ ������������� ����������� ������ ��������� � Mb.
* Written - ����� ����������, ������� ��� ������� ��������� � Mb.
* TerminateReason - ������� ���������� ��������. ����� ����: 
        * "ExitProcess" - ������� ���������� ���������
        * "MemoryLimitExceeded" - �������� ����� ������
        * "TimeLimitExceeded" - �������� ����� ������� ���������� (���� TimeLimit, ���� Deadline)
        * "WriteLimitExceeded" - �������� ����� ������
        * "AbormalExitProcess" - ������� ���������� � ����������� (������ ���������� ��. ����)
        * ���� ������� �� ��� ��������, �� ������� ���� ������������� �������� "<none>"
                
* ExitStatus - ������ ���������� ��������. ����� ��������� �������� ���� �������� ��������, ���� ����� �� 
        ��������� ��������:
        * "AccessViolation"
        * "ArrayBoundsExceeded"
        * "Breakpoint"
        * "Control_C_Exit"
        * "DatatypeMisalignment"
        * "FloatDenormalOperand"
        * "FloatInexactResult"
        * "FloatInvalidOperation"
        * "FloatMultipleFaults"
        * "FloatMultipleTraps"
        * "FloatOverflow"
        * "FloatStackCheck"
        * "FloatUnderflow"
        * "GuardPageViolation"
        * "IllegalInstruction"
        * "InPageError"
        * "InvalidDisposition"
        * "IntegerDivideByZero"
        * "IntegerOverflow"
        * "NoncontinuableException"
        * "PrivilegedInstruction"
        * "RegNatConsumption"
        * "SingleStep"
        * "StackOverflow"
* SpawnerError - ����� ������ ��� ������ spawner'�. 
        ���� ������ �� ���������, �� ���� ������������� �������� "<none>"


sp00
---------------------------------------------
|�����                 | ���������� ��������� |                         |
|----------------------|----------------------|-------------------------|
|tl                    | SP_TIME_LIMIT        |                         |
|ml                    | SP_MEMORY_LIMIT      |                         |
|s                     | SP_SECURITY_LEVEL    |                         |
|d                     | SP_DEADLINE          |                         |
|wl                    | SP_WRITE_LIMIT       |                         |
|-lr                   | SP_LOAD_RATIO        |                         |
|-y                    | SP_IDLE_TIME_LIMIT   |                         |
|-u                    | SP_USER              |                         |
|-p                    | SP_PASSWORD          |                         |
|-sr                   | SP_REPORT_FILE       |                         |
|-so, --out            | SP_OUTPUT_FILE       |                         |
|-e, -se, --err        | SP_ERROR_FILE        |                         |
|-i, --in              | SP_INPUT_FILE        |                         |
|--delegated           | SP_RUNAS             |                         |
|--debug               | SP_DEBUG             |                         |
|--cmd, --systempath   | SP_SYSTEM_PATH       |                         |
|-wd                   | SP_DIRECTORY         |                         |
|-ho                   | SP_HIDE_OUTPUT       |                         |
|-hr                   | SP_HIDE_REPORT       |                         |
|-sw                   | SP_SHOW_WINDOW       |                         |
|--session             |   --                 |                         |
|--separator           | SP_SEPARATOR         |                         |
|--program             | SP_PROGRAM           |                         |
pcms2
---------------------------------------------
|�����                                                  |                                                                                                                                                        |
|-------------------------------------------------------|--------------------------------------------------------------------------------------------------------------------------------------------------------|
|  -h                                                   | show this help                                                                                                                                         |
|  -t <time-limit>                                      | time limit, terminate after <time-limit> seconds, you can add "ms" (without quotes) after the number to specify time limit in milliseconds             |
|  -m <mem-limit>                                       | memory limit, terminate if working set of the process exceeds <mem-limit> bytes, you can add K or M to specify memory limit in kilo- or megabytes      |
|  -r <req-load>                                        | required load of the processor for this process not to be considered idle. You can add % sign to specify required load in percent, default is 0.05 = 5%|
|  -y <idle-limit>                                      | ildeness limit, terminate process if it did not load processor for at least <req-load> for <idleness-limit>                                            |
|  -d <directory>                                       | make <directory> home directory for process                                                                                                            |
|  -l <login-name>                                      | create process under <login-name>                                                                                                                      |
|  -p <password>                                        | logins user using <password>                                                                                                                           |
|  -i <file>                                            | redirects standart input stream to the <file>                                                                                                          |
|  -o <file>                                            | redirects standart output stream to the <file>                                                                                                         |
|  -e <file>                                            | redirects standart error stream to the <file>                                                                                                          |
|  -x                                                   | return exit code of the application                                                                                                                    |
|  -q                                                   | do not display any information on the screen                                                                                                           |
|  -w                                                   | display program window on the screen                                                                                                                   |
|  -1                                                   | use single CPU/CPU core                                                                                                                                |
|  -s <file>                                            | store statistics in then <file>                                                                                                                        |
|  -D var=value                                         | sets value of the environment variable, current environment is completly ignored in this case                                                          |
|Exteneded options:                                     |                                                                                                                                                        |
|  -Xacp,                                               |                                                                                                                                                        |
|  --allow-create-processes                             | allow the created process to create new processes                                                                                                      |
|  -Xtfce, --terminate-on-first-chance-exceptions       | do not ignore exceptions if they are marked as first-chance, required for some old compilers as Borland Delphi                                         |
|  -Xlegacy, -z                                         | try to be compatible with old invoke.dll                                                                                                               |

Examples:
```
  sp -t 10s -m 32M -i 10s a.exe
  sp -d "C:\My Directory" a.exe
  sp -l invoker -p password a.exe
  sp -i input.txt -o output.txt -e error.txt a.exe
```