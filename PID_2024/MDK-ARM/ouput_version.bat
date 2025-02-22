@echo off
for %%F in (*.uvprojx) do (
    set "filename=%%~nF"
    goto :found
)
:found
set HEX_NAME=%filename%
set HEX_PATH=.\%filename%
if not exist ".\Output" (
    mkdir ".\Output"
)
set BASE_OUTPUT_PATH=.\Output
set VERSION_FILE_PATH=..\Core\Src\main.c
set SOFTWARE_VERSION="#define SOFTWARE_VERSION"
for /f "delims=" %%a in ('powershell -Command "Get-Date -Format 'yyMMdd_HHmm'"') do set "CURRENT_DATE=%%a"
set FROMELF_PATH=C:\Keil_v5\ARM\ARMCC\bin
for /f "tokens=3 delims= " %%i in ('findstr /C:%SOFTWARE_VERSION% %VERSION_FILE_PATH%') do set SW_VER=%%i
set SW_VER=%SW_VER:~1,-1%

rem Tạo thư mục cho phiên bản hiện tại
set VERSION_OUTPUT_PATH=%BASE_OUTPUT_PATH%\%SW_VER%
if not exist "%VERSION_OUTPUT_PATH%" (
    mkdir "%VERSION_OUTPUT_PATH%"
)

set output_file_name=%HEX_NAME%_%SW_VER%_%CURRENT_DATE%

echo "Output hex file: %VERSION_OUTPUT_PATH%\%output_file_name%.hex"
copy %HEX_PATH%\%HEX_NAME%.hex %VERSION_OUTPUT_PATH%\%output_file_name%.hex
%FROMELF_PATH%\fromelf --bin %HEX_PATH%\%HEX_NAME%.axf --output %VERSION_OUTPUT_PATH%\%output_file_name%.bin

rem Xóa các file cũ trong thư mục phiên bản hiện tại (tùy chọn)
for %%F in ("%VERSION_OUTPUT_PATH%\%HEX_NAME%_%SW_VER%_*.hex") do (
    if not "%%~nxF"=="%output_file_name%.hex" del "%%F"
)
for %%F in ("%VERSION_OUTPUT_PATH%\%HEX_NAME%_%SW_VER%_*.bin") do (
    if not "%%~nxF"=="%output_file_name%.bin" del "%%F"
)

exit