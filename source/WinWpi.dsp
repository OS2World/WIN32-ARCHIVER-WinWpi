# Microsoft Developer Studio Project File - Name="WinWpi" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=WinWpi - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "WinWpi.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "WinWpi.mak" CFG="WinWpi - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "WinWpi - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "WinWpi - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "WinWpi - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "U:\cbockem\Winwpi" /I "U:\cbockem\Winwpi\libbz2" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "BUILDING_WINWPI" /YX /FD /c
# SUBTRACT CPP /X
# ADD BASE RSC /l 0x807 /d "NDEBUG"
# ADD RSC /l 0x807 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "WinWpi - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I ".." /I "U:\cbockem\Winwpi" /I "U:\cbockem\Winwpi\libbz2" /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "BUILDING_WINWPI" /YX /FD /GZ /c
# SUBTRACT CPP /X
# ADD BASE RSC /l 0x807 /d "_DEBUG"
# ADD RSC /l 0x807 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "WinWpi - Win32 Release"
# Name "WinWpi - Win32 Debug"
# Begin Source File

SOURCE=.\libbz2\blocksort.c
# End Source File
# Begin Source File

SOURCE=.\base\bs_base.cpp
# End Source File
# Begin Source File

SOURCE=.\base\bs_base.h
# End Source File
# Begin Source File

SOURCE=.\base\bs_list.h
# End Source File
# Begin Source File

SOURCE=.\base\bs_string.cpp
# End Source File
# Begin Source File

SOURCE=.\base\bs_string.h
# End Source File
# Begin Source File

SOURCE=.\libbz2\bzlib.c
# End Source File
# Begin Source File

SOURCE=.\libbz2\bzlib.h
# End Source File
# Begin Source File

SOURCE=.\libbz2\bzlib2.c
# End Source File
# Begin Source File

SOURCE=.\libbz2\bzlib_private.h
# End Source File
# Begin Source File

SOURCE=.\libbz2\compress.c
# End Source File
# Begin Source File

SOURCE=.\libbz2\crctable.c
# End Source File
# Begin Source File

SOURCE=.\libbz2\decompress.c
# End Source File
# Begin Source File

SOURCE=.\libbz2\huffman.c
# End Source File
# Begin Source File

SOURCE=.\helpers\linklist.c
# End Source File
# Begin Source File

SOURCE=.\helpers\linklist.h
# End Source File
# Begin Source File

SOURCE=.\os2.h
# End Source File
# Begin Source File

SOURCE=.\libbz2\randtable.c
# End Source File
# Begin Source File

SOURCE=.\setup.h
# End Source File
# Begin Source File

SOURCE=.\helpers\stringh.c
# End Source File
# Begin Source File

SOURCE=.\helpers\stringh.h
# End Source File
# Begin Source File

SOURCE=.\wiarchive\wiarchive.cpp
# End Source File
# Begin Source File

SOURCE=.\wiarchive\wiarchive.h
# End Source File
# Begin Source File

SOURCE=.\wiarchive\wicomp.cpp
# End Source File
# Begin Source File

SOURCE=.\wiarchive\widecomp.cpp
# End Source File
# Begin Source File

SOURCE=.\WinWpi.cpp
# End Source File
# Begin Source File

SOURCE=.\helpers\xstring.c
# End Source File
# Begin Source File

SOURCE=.\helpers\xstring.h
# End Source File
# End Target
# End Project
