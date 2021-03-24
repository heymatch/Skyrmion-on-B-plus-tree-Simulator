@ECHO OFF
MKDIR obj
make
autotest %1 %2