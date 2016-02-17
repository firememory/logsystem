#ifndef _T2EE_HANDLER__H__
#define _T2EE_HANDLER__H__
#if _MSC_VER > 1000
#pragma once
#endif

#include "host_context_impl.h"

#include "aggregator_locl.h"
USING_NAMESPACE_AGGREGATOR;
USING_NAMESPACE_LOGSYSTEM;

#include "dynamic_library.h"
USING_NAMESPACE_BASE;


class CT2EEHandler : public CRawHandlerBase<IRawHandler,IPushingHandler,ITimerCaller>
{
public:
	CT2EEHandler(IServer* pIServer,ICoreModule* pICoreModule);
	virtual~CT2EEHandler();
	// IObject接口(重载)
	virtual LPVOID inline Cast(const IID & riid)
		{	if(riid==IID_IRawHandler) return (IRawHandler*)this;
			if(riid==IID_IPushingHandler) return (IPushingHandler*)this;
			if(riid==IID_ITimerCaller) return (ITimerCaller*)this;
			return NULL;
		}
	// IHandler接口(重载)
	virtual BOOL Configure();
	virtual BOOL AttachServer();
	virtual BOOL AttachBackstage();
	virtual VOID DetachServer();
	// IRawHandler接口
	virtual BPR ParseInfo(IConnect* pIConnect,IBusiness* pIBusiness,LPRAWPROTDEF pProtDef,LPPROTINFO pProtInfo);
	virtual BPR PreExecute(ISession* pISession,IConnect* pIConnect,IBusiness* pIBusiness);
	virtual BPR Execute(ISession* pISession,IConnect* pIConnect,IBusiness* pIBusiness);
	// IPushingHandler接口
	virtual BPR OnPushing(ISession* pISession,IConnect* pIConnect,IBusiness* pIBusinessHost,DWORD dwPushingLv,DWORD idPushingType,DWORD dwPushingOption,LPBYTE pSequence,DWORD cbSequence);
	// ITimerCaller接口
	virtual DWORD OnAlarm(DWORD_PTR dwAlarmID);
protected:
 	static RAWPROTDEF m_ProtDefs[];			// 协议定义
// 	IRPCHost* m_pIRPCHost;					// RPC终端服务
// 	BYTE m_cMySCClass;						// 测试会话上下文对象
// 	BYTE m_cMyCCClass;						// 测试连接上下文对象
// 	LONG m_nTATDef;							// TAT定义号
// 	LONG m_nThreadOffInTAT;					// 在TAT中的线程偏移
// 	LONG m_nThreadNumInTAT;					// 在TAT中的线程个数
//	BOOL m_bQueueByTransKey;				// 通过事务ID排队
  
  CAlarmer<CT2EEHandler> m_Alarmer;
  CAlarmer<CT2EEHandler> m_LongAlarmer;

private:

  IAggregator*        m_pAggregator;
  HostContextImpl     m_HostContext;

  DynamicLibrary      m_dyncEngine;
};

#endif
