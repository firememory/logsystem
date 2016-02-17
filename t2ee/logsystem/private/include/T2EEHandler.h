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
	// IObject�ӿ�(����)
	virtual LPVOID inline Cast(const IID & riid)
		{	if(riid==IID_IRawHandler) return (IRawHandler*)this;
			if(riid==IID_IPushingHandler) return (IPushingHandler*)this;
			if(riid==IID_ITimerCaller) return (ITimerCaller*)this;
			return NULL;
		}
	// IHandler�ӿ�(����)
	virtual BOOL Configure();
	virtual BOOL AttachServer();
	virtual BOOL AttachBackstage();
	virtual VOID DetachServer();
	// IRawHandler�ӿ�
	virtual BPR ParseInfo(IConnect* pIConnect,IBusiness* pIBusiness,LPRAWPROTDEF pProtDef,LPPROTINFO pProtInfo);
	virtual BPR PreExecute(ISession* pISession,IConnect* pIConnect,IBusiness* pIBusiness);
	virtual BPR Execute(ISession* pISession,IConnect* pIConnect,IBusiness* pIBusiness);
	// IPushingHandler�ӿ�
	virtual BPR OnPushing(ISession* pISession,IConnect* pIConnect,IBusiness* pIBusinessHost,DWORD dwPushingLv,DWORD idPushingType,DWORD dwPushingOption,LPBYTE pSequence,DWORD cbSequence);
	// ITimerCaller�ӿ�
	virtual DWORD OnAlarm(DWORD_PTR dwAlarmID);
protected:
 	static RAWPROTDEF m_ProtDefs[];			// Э�鶨��
// 	IRPCHost* m_pIRPCHost;					// RPC�ն˷���
// 	BYTE m_cMySCClass;						// ���ԻỰ�����Ķ���
// 	BYTE m_cMyCCClass;						// �������������Ķ���
// 	LONG m_nTATDef;							// TAT�����
// 	LONG m_nThreadOffInTAT;					// ��TAT�е��߳�ƫ��
// 	LONG m_nThreadNumInTAT;					// ��TAT�е��̸߳���
//	BOOL m_bQueueByTransKey;				// ͨ������ID�Ŷ�
  
  CAlarmer<CT2EEHandler> m_Alarmer;
  CAlarmer<CT2EEHandler> m_LongAlarmer;

private:

  IAggregator*        m_pAggregator;
  HostContextImpl     m_HostContext;

  DynamicLibrary      m_dyncEngine;
};

#endif
