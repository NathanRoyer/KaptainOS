@echo off
echo This will erase the USB drive and upload KaptainOS.bin to it.
set /p DL="Enter drive letter: "
%~dp0\dd if=%~dp0\KaptainOS.bin od=\\.\%DL%: bs=512
echo Done uploading KaptainOS to the drive %DL%:
pause