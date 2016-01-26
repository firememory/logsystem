// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__057DDA31_5A8B_4065_9498_DA06E732A0B5__INCLUDED_)
#define AFX_STDAFX_H__057DDA31_5A8B_4065_9498_DA06E732A0B5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdisp.h>        // MFC Automation classes
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT
// »ù´¡Ö§³Ö¿â

#define FULL_SAFEVCRT
#define SAFEVCRT_DYNAMIC
#define SAFEVCRT_NOAUTOLINK
#include <SafeVCRT.h>

#define CLIBHLPR_NOAUTOLINK
#include <clibhlpr.h>

//#define ASE_FOR_ASFHM
#define ASE_FOR_TCFEM
#define ASELIB_STATIC
#include "ASE.h"

#include "dynamic_library.h"

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__057DDA31_5A8B_4065_9498_DA06E732A0B5__INCLUDED_)
