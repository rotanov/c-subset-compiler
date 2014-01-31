call vcvars32.bat
ml /c /Cp /coff /Fl /Fr src.asm
link src.obj /DEFAULTLIB:libcmt /SUBSYSTEM:console /out:src.exe /entry:mainCRTStartup