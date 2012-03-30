The RakNet lib needs to be copied to /usr/lib , the sh batch file will attempt to do this.

The cs files may need refreshing, if the batch file is used it will attempt to do that as well.

Everything can be compiled with the command gmcs *.cs -out:SwigTest.exe

The file can be ran with mono SwigTest.exe
