::Skyrmion.exe test/testcase1.txt > test/output1.txt 
::FC test/answer1 test/output1

@ECHO OFF
make

IF "%1" == "test" ( GOTO TEST )
IF "%1" == "cmp" ( GOTO CMP )
IF "%1" == "ans" ( GOTO ANS )
IF "%1" == "" ( GOTO DONE )
IF "%2" == "" ( GOTO DONE )

SET /A target=1
SET /A pass=0
Skyrmion.exe test\testcase%1.txt benchmark\setting%2.txt test\output%1.dat test\output%1.csv 2> test\log%1.log
IF %errorlevel%==0 ( ECHO test\testcase%1.txt ok & SET /A pass=pass+1 )
REM FC test\answer%1.ans test\output%1.out
GOTO DONE

:TEST
SET /A target=17
SET /A pass=0
FOR /l %%a IN (1 1 %target%) DO (
    ECHO Test test\testcase%%a.txt ...
    Skyrmion.exe test\testcase%%a.txt benchmark\setting%2.txt test\output%%a.dat test\output%%a.csv 2> test\log%%a.log && ( ECHO test\testcase%%a.txt ok & SET /A pass=pass+1 )
    ECHO Finish test\testcase%%a.txt
    ECHO:
)
GOTO DONE

:CMP
FOR /l %%a IN (1 1 %target%) DO (
    Skyrmion.exe test\testcase%%a.txt benchmark\setting%2.txt > test\output%%a.out 
    FC test\answer%%a.ans test\output%%a.out
)
GOTO DONE

:ANS
FOR /l %%a IN (1 1 %target%) DO (
    @Skyrmion.exe test\testcase%%a.txt benchmark\setting%2.txt > test\answer%%a.ans 
)
GOTO DONE

:DONE
IF %pass%==%target% ( ECHO Accept ) ELSE ( ECHO Wrong )
