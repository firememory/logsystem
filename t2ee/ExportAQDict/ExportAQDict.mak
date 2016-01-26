# Microsoft Developer Studio Generated NMAKE File, Based on ExportAQDict.dsp
!IF "$(CFG)" == ""
CFG=ExportAQDict - Win32 Debug
!MESSAGE No configuration specified. Defaulting to ExportAQDict - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "ExportAQDict - Win32 Release" && "$(CFG)" != "ExportAQDict - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ExportAQDict.mak" CFG="ExportAQDict - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ExportAQDict - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "ExportAQDict - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "ExportAQDict - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\ExportAQDict.exe"


CLEAN :
	-@erase "$(INTDIR)\dynamic_library.obj"
	-@erase "$(INTDIR)\ExportAQDict.obj"
	-@erase "$(INTDIR)\ExportAQDict.pch"
	-@erase "$(INTDIR)\ExportAQDict.res"
	-@erase "$(INTDIR)\ExportAQDictDlg.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\ExportAQDict.exe"
	-@erase "$(OUTDIR)\ExportAQDict.map"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MD /W4 /WX /GX /O2 /I "e:\DevKits\T2EE_SDK\safevcrt\include" /I "e:\DevKits\T2EE_SDK\wtcommlib\include" /I "e:\DevKits\T2EE_SDK\clibhlpr\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Fp"$(INTDIR)\ExportAQDict.pch" /Yu"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC=rc.exe
RSC_PROJ=/l 0x804 /fo"$(INTDIR)\ExportAQDict.res" /d "NDEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\ExportAQDict.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=/nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\ExportAQDict.pdb" /map:"$(INTDIR)\ExportAQDict.map" /machine:I386 /out:"$(OUTDIR)\ExportAQDict.exe" /libpath:"e:\DevKits\T2EE_SDK\clibhlpr\lib" /libpath:"e:\DevKits\T2EE_SDK\wtcommlib\lib" /libpath:"e:\DevKits\T2EE_SDK\safevcrt\lib" 
LINK32_OBJS= \
	"$(INTDIR)\dynamic_library.obj" \
	"$(INTDIR)\ExportAQDict.obj" \
	"$(INTDIR)\ExportAQDictDlg.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\ExportAQDict.res"

"$(OUTDIR)\ExportAQDict.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "ExportAQDict - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\ExportAQDict.exe"


CLEAN :
	-@erase "$(INTDIR)\dynamic_library.obj"
	-@erase "$(INTDIR)\ExportAQDict.obj"
	-@erase "$(INTDIR)\ExportAQDict.pch"
	-@erase "$(INTDIR)\ExportAQDict.res"
	-@erase "$(INTDIR)\ExportAQDictDlg.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\ExportAQDict.exe"
	-@erase "$(OUTDIR)\ExportAQDict.ilk"
	-@erase "$(OUTDIR)\ExportAQDict.map"
	-@erase "$(OUTDIR)\ExportAQDict.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MDd /W4 /Gm /GX /ZI /Od /I "E:\myjob\TS\T2EE_SDK\safevcrt\include" /I "E:\myjob\TS\T2EE_SDK-20130629\wtcommlib\include" /I "E:\myjob\TS\T2EE_SDK\clibhlpr\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Fp"$(INTDIR)\ExportAQDict.pch" /Yu"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC=rc.exe
RSC_PROJ=/l 0x804 /fo"$(INTDIR)\ExportAQDict.res" /d "_DEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\ExportAQDict.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=/nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\ExportAQDict.pdb" /map:"$(INTDIR)\ExportAQDict.map" /debug /machine:I386 /out:"$(OUTDIR)\ExportAQDict.exe" /pdbtype:sept /libpath:"E:\myjob\TS\T2EE_SDK\clibhlpr\lib" /libpath:"E:\myjob\TS\T2EE_SDK-20130629\wtcommlib\lib" /libpath:"E:\myjob\TS\T2EE_SDK\safevcrt\lib" 
LINK32_OBJS= \
	"$(INTDIR)\dynamic_library.obj" \
	"$(INTDIR)\ExportAQDict.obj" \
	"$(INTDIR)\ExportAQDictDlg.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\ExportAQDict.res"

"$(OUTDIR)\ExportAQDict.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("ExportAQDict.dep")
!INCLUDE "ExportAQDict.dep"
!ELSE 
!MESSAGE Warning: cannot find "ExportAQDict.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "ExportAQDict - Win32 Release" || "$(CFG)" == "ExportAQDict - Win32 Debug"
SOURCE=.\dynamic_library.cpp

"$(INTDIR)\dynamic_library.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\ExportAQDict.pch"


SOURCE=.\ExportAQDict.cpp

"$(INTDIR)\ExportAQDict.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\ExportAQDict.pch"


SOURCE=.\ExportAQDict.rc

"$(INTDIR)\ExportAQDict.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


SOURCE=.\ExportAQDictDlg.cpp

"$(INTDIR)\ExportAQDictDlg.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\ExportAQDict.pch"


SOURCE=.\StdAfx.cpp

!IF  "$(CFG)" == "ExportAQDict - Win32 Release"

CPP_SWITCHES=/nologo /MD /W4 /WX /GX /O2 /I "e:\DevKits\T2EE_SDK\safevcrt\include" /I "e:\DevKits\T2EE_SDK\wtcommlib\include" /I "e:\DevKits\T2EE_SDK\clibhlpr\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Fp"$(INTDIR)\ExportAQDict.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\ExportAQDict.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "ExportAQDict - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W4 /Gm /GX /ZI /Od /I "E:\myjob\TS\T2EE_SDK\safevcrt\include" /I "E:\myjob\TS\T2EE_SDK-20130629\wtcommlib\include" /I "E:\myjob\TS\T2EE_SDK\clibhlpr\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Fp"$(INTDIR)\ExportAQDict.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\ExportAQDict.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 


!ENDIF 

