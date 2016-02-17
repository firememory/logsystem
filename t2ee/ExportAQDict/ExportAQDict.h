// ExportAQDict.h : main header file for the EXPORTAQDICT application
//

#if !defined(AFX_EXPORTAQDICT_H__49F419AA_42CA_4661_8FC2_215328689340__INCLUDED_)
#define AFX_EXPORTAQDICT_H__49F419AA_42CA_4661_8FC2_215328689340__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CExportAQDictApp:
// See ExportAQDict.cpp for the implementation of this class
//

class CExportAQDictApp : public CWinApp
{
public:
	CExportAQDictApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CExportAQDictApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CExportAQDictApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EXPORTAQDICT_H__49F419AA_42CA_4661_8FC2_215328689340__INCLUDED_)
