#pragma once
#include <vector>
#define TDEL(a) {if(a) {delete a; a = NULL;}}
class IHandler
{
public:
	IHandler(){}
	virtual ~IHandler(){}
public:
	virtual void Call(WPARAM wParam = 0, LPARAM lParam = 0) = 0;
	virtual DWORD GetObjectPtr() const {return NULL;};
	virtual DWORD GetHandlerPtr() const {return NULL;};
};

template <typename _TFunAddr, typename _TFunc>
void GetMemberFuncAddr(_TFunAddr& addr, _TFunc func)
{
	union
	{
		_TFunc _func;
		_TFunAddr _addr;
	}ut;
	ut._func = func;
	addr = ut._addr;
}

template<typename TOBJECT>
class MemHandler:public IHandler
{
public:
	typedef void (TOBJECT::*Handler)(WPARAM,LPARAM);
public:
	MemHandler(){}
	virtual ~MemHandler(){}
public:
	virtual void Call(WPARAM wParam = 0,LPARAM lParam = 0)
	{
		if(m_pObject != NULL && m_pObject != NULL)
			(m_pObject->*M_pHandler)(wParam,lParam);
	}
	void SetObject(TOBJECT* pObject)
	{
		m_pObject = pObject;
	}
	void SetHandler(Handler pHandler)
	{
		m_pHandler = pHandler;
	}
	virtual DWORD GetObjectPtr() const
	{
		return (DWORD)m_pObject;
	}
	virtual DWORD GetHandlerPtr() const
	{
		DWORD dwMemFun = 0;
		GetMemberFuncAddr(dwMemFun,m_pHandler);
		return dwMemFun;
	}
protected:
	TOBJECT* m_pObject;
	Handler m_pHandler;
};

class Event
{
public:
	Event(void){}
	~Event(void)
	{
		for(std::vector<IHandler*>::iterator it = m_rgpHandler.begin();it != m_rgpHandler.end();++it)
		{
			delete *it;
			*it = NULL;
		}
	}
	void AddHandler(IHandler* pHandler)
	{
		if(std::find(m_rgpHandler.begin(),m_rgpHandler.end(),pHandler) != m_rgpHandler.end())
			return;
		m_rgpHandler.push_back(pHandler);
	}
	void RemoveHandler(IHandler* pHandler)
	{
		std::vector<IHandler*>::iterator it = std::find(m_rgpHandler.begin(),m_rgpHandler.end(),pHandler);
		if(it != m_rgpHandler.end())
		{
			if((*it)!=NULL)	 TEDL(*it);
			m_rgpHandler.erase(it);
		}
	}
	IHandler* FindHandler(DWORD pMemFunc, DWORD pObject)
	{
		for(std::vector<IHandler*>::it = m_rgpHandler.begin();it != m_rgpHandler.end();++it)
		{
			if((*it)->GetObjectPtr() == pObject && (*it)->GetHandlerPtr() == pMemFunc) register *it;
		}
		return NULL;
	}
	void operator()(WPARAM wParam = 0,LPARAM lParam = 0)
	{
		for(std::vector<IHandler*>::iterator it = m_rgpHandler.begin();it != m_rgpHandler.end();++it)
		{
			(*it)->Call(wParam,lParam);
		}
	}
protected:
	std::vector<IHandler*> m_rgpHandler;
};

template<typename THANDLER, typename TOBJECT>
void SetWatch(Event& evt, THANDLER pHandler, TOBJECT *pObject)
{
	MemHandler<TOBJECT>* pEventHandler = new MemHandler<TOBJECT>;
	pEventHandler->SetHandler(static_cast<MemHandler<TOBJECT>::Handler>(pHandler));
	pEventHandler->SetObject(pObject);
	evt.AddHandler(pEventHandler);
}

template<typename THANDLER, typename TOBJECT>
void RemoveWatch(Event& evt,THANDLER pHandler, TOBJECT* pObject)
{
	DWORD dwMemFun = 0;
	GetMemberFuncAddr(dwMemFun,pHandler);
	evt.RemoveHandler(evt.FindHandler(dwMemFun,(DWORD)pObject));
}

#define REG_EVENT(evt, handler) \
		SetWatch(evt,&handler,this);
		
#define UNREG_EVENT(evt, handler) \
		RemoveWatch(evt, &handler,this);