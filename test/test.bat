@echo off

set ERROR=0
set TRISH=..\build\trish

%TRISH% empty.trish
if %ERRORLEVEL% neq 0 set ERROR=1 && echo fail: empty.trish

if %ERROR%==1 exit /b 1 else exit /b 0
