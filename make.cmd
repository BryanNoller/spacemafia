SET PATH=%PATH%;"C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\MSBuild\Current\Bin"
MSBuild.exe -nologo -p:Configuration=Release -p:Platform=x86 -t:venom venom.sln
pause
