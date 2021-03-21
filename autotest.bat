::Skyrmion.exe test/testcase1.txt > test/output1.txt 
::FC test/answer1 test/output1




@ECHO OFF
IF "%1" == "test" ( GOTO TEST )
IF "%1" == "cmp" ( GOTO CMP )
IF "%1" == "ans" ( GOTO ANS )
IF "%1" == "" ( GOTO DONE )

SET /A target=1
SET /A pass=0
Skyrmion.exe test\testcase%1.txt > test\output%1.out 2> test\log%1.log
IF %errorlevel%==0 ( ECHO test\testcase%1.txt ok & SET /A pass=pass+1 )
REM FC test\answer%1.ans test\output%1.out
GOTO DONE

:TEST
SET /A target=15
SET /A pass=0
FOR /l %%a IN (1 1 15) DO (
    REM COPY test\output%%a.out test\past%%a.out 
    ECHO Test test\testcase%%a.txt ...
    Skyrmion.exe test\testcase%%a.txt > test\output%%a.out 2> test\log%%a.log
    IF %errorlevel%==0 ( ECHO test\testcase%%a.txt ok & SET /A pass=pass+1 )
    ECHO Finish test\testcase%%a.txt
)
GOTO DONE

:CMP
FOR /l %%a IN (1 1 17) DO (
    Skyrmion.exe test\testcase%%a.txt > test\output%%a.out 
    FC test\answer%%a.ans test\output%%a.out
)
GOTO DONE

:ANS
FOR /l %%a IN (1 1 17) DO (
    @Skyrmion.exe test\testcase%%a.txt > test\answer%%a.ans 
)
GOTO DONE

:DONE
IF %pass%==%target% ( ECHO Accept ) ELSE ( ECHO Wrong )
