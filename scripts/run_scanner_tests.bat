@echo OFF
cd lexer
echo Testing scanner: > scanner_test.log
echo. >> scanner_test.log
for %%V in (*.c) do (
echo -----------------------------------------------
echo testing: "%%~nV"
..\..\bin\Compiler.debug.exe -l %%~nV.c > %%~nV.out
cmp -c %%~nV.out %%~nV.ans
if ERRORLEVEL 1 (echo ******* FAILED *******) else (echo ******* PASSED *******)
if ERRORLEVEL 1 (
echo test "%%~nV":    FAILED: >>scanner_test.log & cmp -c %%~nV.out %%~nV.ans >> scanner_test.log) else (echo test "%%~nV":    PASSED >>scanner_test.log)
) 
cd ..\
pause

rem type %%V.out.txt