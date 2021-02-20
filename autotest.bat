::Skyrmion.exe test/testcase1.txt > test/output1.txt 
::FC test/answer1 test/output1

@ECHO OFF
IF "%1" == "test" ( GOTO TEST )
IF "%1" == "ans" ( GOTO ANS ) ELSE ( GOTO DONE )

:TEST
FOR /l %%a IN (1 1 1) DO (
    Skyrmion.exe test\testcase%%a.txt > test\output%%a.out 
    FC test\answer%%a.ans test\output%%a.out
)
GOTO DONE

:ANS
FOR /l %%a IN (1 1 1) DO (
    @Skyrmion.exe test\testcase%%a.txt > test\answer%%a.ans 
)
GOTO DONE

:DONE