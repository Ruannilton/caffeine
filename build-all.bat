@ECHO OFF
REM Build Everything

ECHO "Building everything..."


PUSHD engine
CALL build-engine.bat
POPD
IF %ERRORLEVEL% NEQ 0 (echo Error:%ERRORLEVEL% && exit)

PUSHD client
CALL build-client.bat
POPD
IF %ERRORLEVEL% NEQ 0 (echo Error:%ERRORLEVEL% && exit)

ECHO "All assemblies built successfully."