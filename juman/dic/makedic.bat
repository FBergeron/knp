echo off

..\makemat
if errorlevel 1 goto error

for %%f in (*.dic) do ..\makeint %%f
if errorlevel 1 goto error

copy /b *.int jumandic.txt
if errorlevel 1 goto error

del *.int
if errorlevel 1 goto error

..\dicsort jumandic.txt > jumandic.dat
if errorlevel 1 goto error

del jumandic.txt
if errorlevel 1 goto error

..\makepat
if errorlevel 1 goto error

echo JUMANの辞書が作成されました。
goto end

:error
echo 辞書の作成に失敗しました。
goto end

:end
