REM Build script for client
@ECHO OFF
SetLocal EnableDelayedExpansion

REM Get a list of all the .c files.
SET cFilenames=
FOR /R %%f in (*.c) do (
    SET cFilenames=!cFilenames! %%f
)

REM echo "Files:" %cFilenames%

SET assembly=client
SET compilerFlags=-g 
REM -Wall -Werror
SET includeFlags=-Isrc -I../engine
SET linkerFlags=-L../bin/ -lengine
SET defines=-D_DEBUG

ECHO "Building %assembly%%..."
clang %cFilenames% %compilerFlags% -o ../bin/%assembly%.exe %defines% %includeFlags% %linkerFlags%