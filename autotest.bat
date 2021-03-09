::Skyrmion.exe test/testcase1.txt > test/output1.txt 
::FC test/answer1 test/output1

@ECHO OFF
IF "%1" == "test" ( GOTO TEST )
IF "%1" == "cmp" ( GOTO CMP )
IF "%1" == "ans" ( GOTO ANS )
IF "%1" == "" ( GOTO DONE )

Skyrmion.exe test\testcase%1.txt > test\output%1.out 
REM FC test\answer%1.ans test\output%1.out
GOTO DONE

:TEST
FOR /l %%a IN (1 1 8) DO (
    REM COPY test\output%%a.out test\past%%a.out 
    Skyrmion.exe test\testcase%%a.txt > test\output%%a.out 
)
GOTO DONE

:CMP
FOR /l %%a IN (1 1 8) DO (
    Skyrmion.exe test\testcase%%a.txt > test\output%%a.out 
    FC test\answer%%a.ans test\output%%a.out
)
GOTO DONE

:ANS
FOR /l %%a IN (1 1 8) DO (
    @Skyrmion.exe test\testcase%%a.txt > test\answer%%a.ans 
)
GOTO DONE

:DONE