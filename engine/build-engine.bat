REM Build script for engine
@ECHO off
SetLocal EnableDelayedExpansion

REM Listing files
SET "files="
FOR /R %%f IN (*.c) DO (
    SET "files=!files! %%f"
)

REM echo "Files: " %files%

SET "assembly=engine"
SET "compilerFlags=-g -shared -Wvarargs -Wall -Werror"
SET "includeFlags=-Iengine"
SET "linkerFlags=-luser32"
SET "defines=-D_DEBUG -DCAFF_EXPORT -D_CRT_SECURE_NO_WARNINGS"

REM "Building %assembly%"
clang %files% %compilerFlags% -o ../bin/%assembly%.dll %defines% %includeFlags% %linkerFlags%
