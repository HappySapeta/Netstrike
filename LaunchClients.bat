@echo off

cd "Binaries\windows-x86_64\Debug-Client\Game"

set /A COUNT=(%1)-1
set DELAY=4

for /L %%i in (1,1,%COUNT%) do (
    start "NS - BOT" "Game.exe" bot %2 %3
    timeout /t %DELAY% /nobreak > nul
)

start "NS - PLAYER" "Game.exe" player %2 %3