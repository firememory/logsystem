#ifndef _STDAFX__H__
#define _STDAFX__H__
#if _MSC_VER > 1000
#pragma once
#endif

#pragma pack(8)

#define VC_EXTRALEAN
#ifndef WINVER
	#define WINVER 0X0501
#endif
#ifndef WIN32_WINNT
	#define WIN32_WINNT 0x0501
#endif
#if !defined(_WIN64) && !defined(_USE_32BIT_TIME_T)
	#define USE_32BIT_TIME_T
#endif

#include <afxwin.h>
#include <afxext.h>

#ifndef AFX_NO_OLE_SUPPORT
#include <afxole.h>
#include <afxodlgs.h>
#include <afxdisp.h>
#endif

#include <afxdtctl.h>
#ifndef AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>
#endif

#pragma pack()

#define SAFEVCRT_NOAUTOLINK
#define DYNA_SAFEVCRT
#include <SafeVCRT.h>

#undef CLIBVERIFY
#define CLIBVERIFY

#undef CLIBASSERT
#define CLIBASSERT


#define WTCOMMLIB_NOAUTOLINK
#define ASE_FOR_TSMOD
#define ASE_WITH_IX_CLASS
#define ASE_WITH_IX_WEAK
#include "ASE.h"

#include <CmnServices.h>
#include <SmsServices.h>
#include <ProcCallSvcs.h>
#include <SvcEasyUse.inl>

//{{AFX_INSERT_LOCATION}}
#endif
