@echo off

set ERROR=0
set TRISH=..\build\trish

%TRISH% empty.trish > nul
if %ERRORLEVEL% neq 0 set ERROR=1 && echo [test] fail: empty.trish

%TRISH% simple.trish > nul
if %ERRORLEVEL% neq 0 set ERROR=1 && echo [test] fail: simple.trish

%TRISH% echo.trish > temp.txt
if %ERRORLEVEL% neq 0 set ERROR=1 && echo [test] fail: echo.trish
fc temp.txt echo.txt > nul
if %ERRORLEVEL% neq 0 set ERROR=1 && echo [test] fail: echo.trish
del temp.txt

%TRISH% err-unclosed-single.trish > nul 2>&1
if %ERRORLEVEL% equ 0 set ERROR=1 && echo [test] fail: err-unclosed-single.trish

if %ERROR%==1 (
	exit /b 1
) else (
	echo [test] all tests passed.
	exit /b 0
)
