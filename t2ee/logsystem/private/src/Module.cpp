#include "stdafx.h"
#include "Module.h"

#if defined(WIN32) && defined(_DEBUG)
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CModule::CModule(IServer* pIServer)
	: CModuleBase<ICoreModule>(pIServer,"��־ϵͳ���ģ��")
	, m_T2EEHandler(pIServer,This())
{
}

CModule::~CModule()
{
}

BOOL CModule::Start()
{
	// ע�����,Ӧ��,Э�������,Э���,Э�鴦����
	if(!m_pIServer->RegisterHandler(&m_T2EEHandler)) return FALSE;
	return TRUE;
}

#include <config.h>

//EXTERN_C  SYMBOL_EXPORT
BOOL CoreModuleMain(IServer* pIServer) {
	ICoreModule* pICoreModule=new CModule(pIServer);
	if(!pIServer->RegisterModule(pICoreModule))
	{	pICoreModule->Final();
		pIServer->AddErrorLog("%s: ע��ģ��ʧ��!\r\n",pICoreModule->GetName());
		return FALSE;
	}
	return TRUE;
}
