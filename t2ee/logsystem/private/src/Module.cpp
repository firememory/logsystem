#include "stdafx.h"
#include "Module.h"

#if defined(WIN32) && defined(_DEBUG)
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CModule::CModule(IServer* pIServer)
	: CModuleBase<ICoreModule>(pIServer,"日志系统汇聚模块")
	, m_T2EEHandler(pIServer,This())
{
}

CModule::~CModule()
{
}

BOOL CModule::Start()
{
	// 注册服务,应用,协议控制器,协议簇,协议处理器
	if(!m_pIServer->RegisterHandler(&m_T2EEHandler)) return FALSE;
	return TRUE;
}

#include <config.h>

//EXTERN_C  SYMBOL_EXPORT
BOOL CoreModuleMain(IServer* pIServer) {
	ICoreModule* pICoreModule=new CModule(pIServer);
	if(!pIServer->RegisterModule(pICoreModule))
	{	pICoreModule->Final();
		pIServer->AddErrorLog("%s: 注册模块失败!\r\n",pICoreModule->GetName());
		return FALSE;
	}
	return TRUE;
}
