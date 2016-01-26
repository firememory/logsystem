#ifndef _MODULE__H__
#define _MODULE__H__
#if _MSC_VER > 1000
#pragma once
#endif
// #include "TestSvc.h"
// #include "TestServ.h"
#include "T2EEHandler.h"

class CModule : public CModuleBase<ICoreModule>
{
public:
	CModule(IServer* pIServer);
	virtual~CModule();
	// ICoreModuleΩ”ø⁄(÷ÿ‘ÿ)
	virtual BOOL Start();
protected:
// 	CTestSvc		m_TestSvc;
// 	CTestServ		m_TestServ;
	CT2EEHandler	m_T2EEHandler;
};

#endif
