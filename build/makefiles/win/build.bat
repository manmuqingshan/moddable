@echo off
REM
REM Copyright (c) 2016-2017  Moddable Tech, Inc.
REM
REM   This file is part of the Moddable SDK Tools.
REM 
REM   The Moddable SDK Tools is free software: you can redistribute it and/or modify
REM   it under the terms of the GNU General Public License as published by
REM   the Free Software Foundation, either version 3 of the License, or
REM   (at your option) any later version.
REM 
REM   The Moddable SDK Tools is distributed in the hope that it will be useful,
REM   but WITHOUT ANY WARRANTY; without even the implied warranty of
REM   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
REM   GNU General Public License for more details.
REM 
REM   You should have received a copy of the GNU General Public License
REM   along with the Moddable SDK Tools.  If not, see <http://www.gnu.org/licenses/>.
REM
REM

IF "%MODDABLE%"=="" (
    echo MODDABLE environment variable not set
    EXIT /B 1
)

IF NOT EXIST %MODDABLE%\xs (
    echo MODDABLE environment variable is set to an incorrect path: %MODDABLE%
    EXIT /B 1
)

SET BUILD_DIR=%MODDABLE%\build
SET XS_DIR=%MODDABLE%\xs

IF "%1%"=="clean" (
    echo Cleaning...
    rmdir /S /Q %BUILD_DIR%\tmp
    rmdir /S /Q %BUILD_DIR%\bin
    echo Done.
    EXIT /B 0
)

@echo on

nmake GOAL=debug BUILD_DIR=%BUILD_DIR% XS_DIR=%XS_DIR% /c /f %XS_DIR%\makefiles\win\xsc.mak /s
nmake GOAL=debug BUILD_DIR=%BUILD_DIR% XS_DIR=%XS_DIR% /c /f %XS_DIR%\makefiles\win\xsid.mak /s
nmake GOAL=debug BUILD_DIR=%BUILD_DIR% XS_DIR=%XS_DIR% /c /f %XS_DIR%\makefiles\win\xsl.mak /s

nmake GOAL=debug BUILD_DIR=%BUILD_DIR% XS_DIR=%XS_DIR% /c /f serial2xsbug.mak /s
nmake GOAL=debug BUILD_DIR=%BUILD_DIR% XS_DIR=%XS_DIR% /c /f tools.mak /s

nmake GOAL=release BUILD_DIR=%BUILD_DIR% XS_DIR=%XS_DIR% /c /f %XS_DIR%\makefiles\win\xsc.mak /s
nmake GOAL=release BUILD_DIR=%BUILD_DIR% XS_DIR=%XS_DIR% /c /f %XS_DIR%\makefiles\win\xsid.mak /s
nmake GOAL=release BUILD_DIR=%BUILD_DIR% XS_DIR=%XS_DIR% /c /f %XS_DIR%\makefiles\win\xsl.mak /s

nmake GOAL=release BUILD_DIR=%BUILD_DIR% XS_DIR=%XS_DIR% /c /f serial2xsbug.mak /s
nmake GOAL=release BUILD_DIR=%BUILD_DIR% XS_DIR=%XS_DIR% /c /f tools.mak /s

call %MODDABLE%\build\bin\win\debug\mcconfig -d -m -p x-win %MODDABLE%\tools\xsbug\manifest.json
call %MODDABLE%\build\bin\win\debug\mcconfig -d -m -p x-win %MODDABLE%\tools\mcsim\manifest.json

call %MODDABLE%\build\bin\win\release\mcconfig -m -p x-win %MODDABLE%\tools\xsbug\manifest.json
call %MODDABLE%\build\bin\win\release\mcconfig -m -p x-win %MODDABLE%\tools\mcsim\manifest.json
