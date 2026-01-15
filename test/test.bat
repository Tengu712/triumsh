@echo off

set ERROR=0
set TRISH=..\build\trish

%TRISH% empty.trish > nul 2>&1
if %ERRORLEVEL% neq 0 set ERROR=1 && echo fail: empty.trish

%TRISH% simple.trish > nul 2>&1
if %ERRORLEVEL% neq 0 set ERROR=1 && echo fail: simple.trish

if %ERROR%==1 (
	exit /b 1
) else (
	echo all tests passed.
	exit /b 0
)
