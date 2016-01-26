#include "stdafx.h"

#include "T2EEHandler.h"

// 常用协议标识
#define PFS_COMM				PF_COMM_THREAD
#define PFS_CORE				PF_CORE_THREAD
#define PFS_SCHED				PF_SCHED_BUSINESS
#define PFS_BINDED				PF_BINDED_BUSINESS

// 扩充协议定义
RAWPROTDEF CT2EEHandler::m_ProtDefs[]=
{
	// ROSE协议
	{ ProtocolID_LSA		,0		,"日志汇聚"		,PFS_COMM|PF_DEFSIZE				,0								,0			,0			},
};

/*
// 会话上下文对象
class CMySC
{
public:
	CMySC() { TRACE("::::: Create MySC\r\n"); }
	virtual~CMySC() { TRACE("::::: Destroy MySC\r\n"); }
	DECLARE_DESTRUCT(CMySC);
	DECLARE_CONNECTOR(CMySC,LPVOID);
};


// 连接上下文对象
class CMyCC
{
public:
	CMyCC() { TRACE("::::: Create MyCC\r\n"); }
	virtual~CMyCC() { TRACE("::::: Destroy MyCC\r\n"); }
	DECLARE_DESTRUCT(CMyCC);
	DECLARE_CONNECTOR(CMyCC,LPVOID);
};
*/

CT2EEHandler::CT2EEHandler(IServer* pIServer,ICoreModule* pICoreModule)
	: CRawHandlerBase<IRawHandler,IPushingHandler,ITimerCaller>(pIServer,"日志汇聚器",pICoreModule)
// 	, m_cMySCClass(CNTX_INVALID)
// 	, m_cMyCCClass(CNTX_INVALID)
// 	, m_nTATDef(-1)
// 	, m_nThreadOffInTAT(-1)
// 	, m_nThreadNumInTAT(-1)
// 	, m_bQueueByTransKey(TRUE)
  , m_Alarmer(FALSE,NULL)
  , m_LongAlarmer(TRUE, NULL)
  //, m_HostContext(this, pIServer)
  , m_HostContext(NULL, pIServer)
  , m_dyncEngine(NULL)

  , m_pAggregator(NULL)
{

  m_HostContext.SetCT2EEHandler(this);
  const char_t* strEnginePath           = _STR("aggregator");
  const char_t* strEngineLib            = _STR("aggregator.dll");
  const char_t* strTaApi_CreateInstance  = _STR("Aggregator_CreateInstance");

  char_t strEngineDir[MAX_PATH] = {0};
  SNPRINTF(strEngineDir, sizeof(strEngineDir), _STR("%s/%s"), m_HostContext.GetSystemExeFullPath(), strEnginePath);
  if (RC_S_OK != m_dyncEngine.open(strEngineLib, strEngineDir)) { return; }

  typedef IAggregator* (*PFN_AGGREGATOR_CREATEINSTANCE)();

  PFN_AGGREGATOR_CREATEINSTANCE pfnAggregator_CreateInstance = 
    m_dyncEngine.LocateSymbol<PFN_AGGREGATOR_CREATEINSTANCE>(strTaApi_CreateInstance);
  if (NULL == pfnAggregator_CreateInstance) { return; }
  m_pAggregator = (pfnAggregator_CreateInstance)();
}

CT2EEHandler::~CT2EEHandler()
{
  if (m_pAggregator) { m_pAggregator->Release(); }
}

// IHandler接口
BOOL CT2EEHandler::Configure()
{
	// 获取依赖的服务
// 	LPVOID* apServices[]={(LPVOID*)&m_pIRPCHost};
// 	LPCSTR apszServices[]={"svc.RPCHost"};
// 	const IID* apidServices[]={&IID_IRPCHost};
// 	for(LONG i=0; i<ARRAYSIZE(apServices); i++)
// 	{	(*apServices[i])=m_pIServer->GetRegisteredService(apszServices[i],*apidServices[i],TRUE);
// 		if((*apServices[i])==NULL)
// 		{	m_pIServer->AddErrorLog("%s: 依赖服务%s,但是该服务不存在!",GetName(),apszServices[i]);
// 			return FALSE;
// 		}
// 	}
/*
	// 注册上下文类
	m_cMySCClass=m_pIServer->GetUserCntxClass(IServer::UCCATEGORY_SC,"MySC");
	if(m_cMySCClass==CNTX_INVALID) m_cMySCClass=m_pIServer->RegisterUserCntxClass(IServer::UCCATEGORY_SC,"MySC",sizeof(CMySC),&CMySC::__Destructor,&CMySC::__Connector);
	if(m_cMySCClass==CNTX_INVALID)
	{	m_pIServer->AddErrorLog("%s: 当前模块依赖或引用上下文%s,但上下文创建失败!",GetName(),"MySC");
		return FALSE;
	}
	m_cMyCCClass=m_pIServer->GetUserCntxClass(IServer::UCCATEGORY_CC,"MyCC");
	if(m_cMyCCClass==CNTX_INVALID) m_cMyCCClass=m_pIServer->RegisterUserCntxClass(IServer::UCCATEGORY_CC,"MyCC",sizeof(CMyCC),&CMyCC::__Destructor,&CMyCC::__Connector);
	if(m_cMyCCClass==CNTX_INVALID)
	{	m_pIServer->AddErrorLog("%s: 当前模块依赖或引用上下文%s,但上下文创建失败!",GetName(),"MyCC");
		return FALSE;
	}

	// 注册私有的TAT入口
	m_nTATDef=m_pIServer->GetRegisteredTAT("TestThreadPool");
	if(m_nTATDef<0) m_nTATDef=m_pIServer->RegisterTAT("TestThreadPool",20,"测试用的线程池");
	m_nThreadOffInTAT=m_pIServer->GetThreadOffInTAT(m_nTATDef);
	m_nThreadNumInTAT=m_pIServer->GetThreadNumInTAT(m_nTATDef);
*/
  // 
  if (NULL == m_pAggregator) { return FALSE;}

  if (RC_S_OK != m_pAggregator->Init(&m_HostContext)) {
    return FALSE;
  }

	// 添加一个定时器线程池并注册一个定时器ID
	if(!m_Alarmer.InitAlarmer(m_pIServer,this))
	{	m_pIServer->AddErrorLog("%s: 创建定时器失败!",GetName());
		return FALSE;
	}

  if(!m_LongAlarmer.InitAlarmer(m_pIServer,this))
  {	m_pIServer->AddErrorLog("%s: 创建定时器失败!",GetName());
  return FALSE;
  }

	// 创建一个列表,用于接收在线用户
	return TRUE;
}

BOOL CT2EEHandler::AttachServer()
{
	LPCSTR apszFamilys[]={"7BETA2","7X"};
	for(LONG j=0; j<(LONG)ARRAYSIZE(apszFamilys); j++)
	{	IProtocolFamily* pIProtocolFamily=m_pIServer->GetRegisteredProtocolFamily(apszFamilys[j]);
		if(pIProtocolFamily==NULL) continue;
		if(!pIProtocolFamily->RegisterProtocols(this,m_ProtDefs,ARRAYSIZE(m_ProtDefs),sizeof(m_ProtDefs[0]))) return FALSE;
	}

  // 
  if (RC_S_OK != m_pAggregator->Start()) {
    return FALSE;
  }

	return TRUE;
}

BOOL CT2EEHandler::AttachBackstage()
{
	// 设置定时器
  if (NULL == m_pAggregator) { return TRUE; }

	if(!m_Alarmer.SetAlarmer(0, m_pAggregator->GetTick(), TRUE))
		return FALSE;

  if(!m_LongAlarmer.SetAlarmer(0, m_pAggregator->GetLongTick(), TRUE))
    return FALSE;
  
  return TRUE;
}

VOID CT2EEHandler::DetachServer()
{
	// 取消定时器
	m_Alarmer.UnsetAlarmer();

  m_LongAlarmer.UnsetAlarmer();

  // 
  m_pAggregator->Stop();
}

// IRawHandler接口
BPR CT2EEHandler::ParseInfo(IConnect* pIConnect,IBusiness* pIBusiness,LPRAWPROTDEF pProtDef,LPPROTINFO pProtInfo)
{
	UNREFERENCED_PARAMETER(pIConnect);
	UNREFERENCED_PARAMETER(pProtDef);

	USING_RAW_REQCNTX();

	switch(pProtInfo->m_wCmdNo)
	{
	case ProtocolID_LSA:
		break;
  /*
	case TEST_ECHO:
		{	test_echo_ropt* roptp=(test_echo_ropt*)pRawReqCntx->m_pOption;
			CLIBVERIFY(roptp->m_cReserved==(BYTE)(~0));
			IProtocolFamily* pIProtocolFamily=pIConnect->GetProtocolFamily();
			CLIBVERIFY(pIProtocolFamily!=NULL);
			BPR nBPR=pIProtocolFamily->BusinessRenderEncoders(pIBusiness,NULL,sizeof(test_echo_aopt));
			if(nBPR!=BPR_SUCCESS) return nBPR;
			return BPR_SUCCESS;
		}
		break;
	case TEST_ENDORSEMENT:
		{	test_endorsement_ropt* roptp=(test_endorsement_ropt*)pRawReqCntx->m_pOption;
			CLIBVERIFY(roptp->m_cReserved==(BYTE)(~0));
			IProtocolFamily* pIProtocolFamily=pIConnect->GetProtocolFamily();
			CLIBVERIFY(pIProtocolFamily!=NULL);
			BPR nBPR=pIProtocolFamily->BusinessRenderEncoders(pIBusiness,NULL,sizeof(test_endorsement_aopt));
			if(nBPR!=BPR_SUCCESS) return nBPR;
			return BPR_SUCCESS;
		}
		break;
	case TEST_UPLOAD:
		{	TRACE("通过绑定线程<%d>访问线程池,算法:ConnectID MOD ThreadNum\r\n",GetCurrentThreadId());
			if(m_bQueueByTransKey)
			{	pProtInfo->m_dwBindedThreadNo=UINT_MAX;
				if(m_nThreadNumInTAT>0) pProtInfo->m_dwBindedThreadNo=m_nThreadOffInTAT+pIBusiness->GetTransKey()%m_nThreadNumInTAT;
			}
			else
			{	pProtInfo->m_dwBindedThreadNo=UINT_MAX;
				if(m_nThreadNumInTAT>0) pProtInfo->m_dwBindedThreadNo=m_nThreadOffInTAT+pIConnect->GetConnectID()%m_nThreadNumInTAT;
			}
			return BPR_SUCCESS;
		}
		break;
	case TEST_DOWNLOAD:
		break;
	case TEST_LONGTALK:
		{	test_longtalk_ropt* roptp=(test_longtalk_ropt*)pRawReqCntx->m_pOption;
			CLIBVERIFY(roptp->m_cReserved==(BYTE)(~0));
			IProtocolFamily* pIProtocolFamily=pIConnect->GetProtocolFamily();
			CLIBVERIFY(pIProtocolFamily!=NULL);
			BPR nBPR=pIProtocolFamily->BusinessRenderEncoders(pIBusiness,NULL,sizeof(test_longtalk_aopt));
			if(nBPR!=BPR_SUCCESS) return nBPR;
			return BPR_SUCCESS;
		}
		break;
  */
	default:
		break;
	}
	// 其他协议处理
	return BPR_SUCCESS;
}

BPR CT2EEHandler::PreExecute(ISession* pISession,IConnect* pIConnect,IBusiness* pIBusiness)
{
	UNREFERENCED_PARAMETER(pISession);
	UNREFERENCED_PARAMETER(pIConnect);
	USING_RAW_REQCNTX();

	// 得到当前协议号和调用者信息
	WORD wCmdNo=pIBusiness->GetCmdNo();
	IUser* pIUserCaller=pIBusiness->GetCaller();
	CLIBVERIFY(pIUserCaller!=NULL);

	// 其他协议处理
	switch(wCmdNo)
	{
  case ProtocolID_LSA:
    break;
  /*
	case TEST_LOGIN:
		{	test_login_req* reqp=(test_login_req*)pRawReqCntx->m_pBody;
			pIUserCaller->SetUID(reqp->m_szLoginID);
		}
		break;
	case TEST_ECHO:
		break;
	case TEST_ENDORSEMENT:
		{	test_endorsement_req* reqp=(test_endorsement_req*)pRawReqCntx->m_pBody;
			pIUserCaller->Zero();
			IP EthernetIp=IPConvert6ToLegal(reqp->m_EthernetIp);
			IP InternetIp=IPConvert6ToLegal(reqp->m_InternetIp);
			pIUserCaller->SetConnectInfo(reqp->m_szDevice,EthernetIp,InternetIp,reqp->m_Mac,reqp->m_szBuildName,reqp->m_wClientType,reqp->m_dwClientVer);
			SAFESTRING(reqp->m_szUID);
			pIUserCaller->SetUOrg("AgentUsers");
			pIUserCaller->SetUID(reqp->m_szUID);
		}
		break;
	case TEST_UPLOAD:
		{	test_upload_req* reqp=(test_upload_req*)pRawReqCntx->m_pBody;
			TCHAR szCmdDesc[32]={0};
			nsprintf(szCmdDesc,sizeof(szCmdDesc),"UPLOAD-%d",reqp->m_cTestType);
			pIBusiness->RenewCmdDesc(szCmdDesc);
		}
		break;
	case TEST_DOWNLOAD:
		break;
	case TEST_LONGTALK:
		break;
  */
  default:
    break;
	}
	return BPR_SUCCESS;
}

BPR CT2EEHandler::Execute(ISession* pISession,IConnect* pIConnect,IBusiness* pIBusiness)
{
	UNREFERENCED_PARAMETER(pISession);
	UNREFERENCED_PARAMETER(pIConnect);
	// 得到当前协议号和调用者信息
	WORD wCmdNo=pIBusiness->GetCmdNo();
	IUser* pIUserCaller=pIBusiness->GetCaller();
	CLIBVERIFY(pIUserCaller!=NULL);

	// 协议处理
	switch(wCmdNo)
	{
  case ProtocolID_LSA:
    {
      USING_RAW_REQANSCNTX();

      rc_t rc;
      NetHandlerImpl NetHandler(this, m_pIServer, pISession, pIConnect, pIBusiness, pRawAnsCntx);
      rc = m_pAggregator->NetProc(&NetHandler, pRawReqCntx->m_pBody, pRawReqCntx->m_cbBody);
      if (RC_S_OK == rc) { return BPR_SUCCESS; }
    }
    break;
  /*
	case TEST_LOGIN:
		{
			TRACE("TEST_LOGIN 线程ID: %d (客户端单线程顺序执行)\r\n",GetCurrentThreadId());
			
			if(pISession==NULL||pISession->IsObjectNull())
				return m_pIServer->ThrowProtocolError(pIConnect,pIBusiness,SESSION_ERR_CANOTOPEN,"打开SESSION失败");

			// 设置连接信息(登录前的相关信息)
			pISession->UpdateConnectInfo(pIUserCaller);

			// 准备处理相关数据
			USING_RAW_REQANSCNTX();
			test_login_req* reqp=(test_login_req*)pRawReqCntx->m_pBody;
			test_login_ans* ansp=(test_login_ans*)pRawAnsCntx->m_pBody;
			UNREFERENCED_PARAMETER(reqp);

			// 创建一个命名的上下文
			CMySC* pSC=CREATE_SUC_OBJECT(pISession,m_cMySCClass,CMySC);
			UNUSED_ALWAYS(pSC);
			CMyCC* pCC=CREATE_CUC_OBJECT(pIConnect,m_cMyCCClass,CMyCC);
			UNUSED_ALWAYS(pCC);
			TRACE("创建会话对象\r\n");

			// 进行验证并得到验证相关信息
			pIUserCaller->SetUserProfile(0,0,"AuthedUsers","UID",0,"认证用户",UAC_DEFAULT);
			m_pIServer->ManageLocalUser(pISession,pIConnect,pIBusiness);
			pIConnect->SetSoftQPC(-1);

			// 处理应答
			if(sizeof(*ansp)>pRawAnsCntx->m_cbBodyMax)
				return m_pIServer->ThrowProtocolError(pIConnect,pIBusiness,SYS_ERR_BUFFERTOOSMALL,"缓冲区太小。");
			memset(ansp,0,sizeof(*ansp));
			ansp->m_cReserved=0;
			COPYSTRARRAY(ansp->m_szUID,"UID");
			SetBusinessCompleted(pIBusiness,pRawAnsCntx,sizeof(*ansp));
			return BPR_SUCCESS;
		}
		break;
	case TEST_ECHO:
		{
			TRACE("TEST_ECHO 线程ID: %d (会阻塞通讯线程)\r\n",GetCurrentThreadId());
			USING_RAW_REQANSCNTX();
			test_echo_ropt* roptp=(test_echo_ropt*)pRawReqCntx->m_pOption;
			test_echo_req* reqp=(test_echo_req*)pRawReqCntx->m_pBody;
			test_echo_aopt* aoptp=(test_echo_aopt*)pRawAnsCntx->m_pOption;
			test_echo_ans* ansp=(test_echo_ans*)pRawAnsCntx->m_pBody;
			CLIBVERIFY(reqp->m_cReserved==(BYTE)(~0));
			if(aoptp==NULL||pRawAnsCntx->m_cbOption!=sizeof(*aoptp))
				return m_pIServer->ThrowProtocolError(pIConnect,pIBusiness,SYS_ERR_BADOPTBUFFER,"选项缓冲区错误。");
			DWORD cbInfo=pRawReqCntx->m_cbPackage-sizeof(*reqp);
			if(sizeof(*ansp)+cbInfo>pRawAnsCntx->m_cbBodyMax)
				return m_pIServer->ThrowProtocolError(pIConnect,pIBusiness,SYS_ERR_BUFFERTOOSMALL,"缓冲区太小。");
			aoptp->m_cReserved=roptp->m_cReserved;
			ansp->m_cReserved=roptp->m_cReserved;
			memcpy(ansp->m_szInfo,reqp->m_szInfo,cbInfo);
			SetBusinessCompleted(pIBusiness,pRawAnsCntx,sizeof(*ansp)+cbInfo);
			return BPR_SUCCESS;
		}
		break;
	case TEST_ENDORSEMENT:
		{
			TRACE("TEST_ENDORSEMENT 线程ID: %d (会阻塞通讯线程)\r\n",GetCurrentThreadId());
			USING_RAW_REQANSCNTX();
			test_endorsement_ropt* roptp=(test_endorsement_ropt*)pRawReqCntx->m_pOption;
			test_endorsement_req* reqp=(test_endorsement_req*)pRawReqCntx->m_pBody;
			test_endorsement_aopt* aoptp=(test_endorsement_aopt*)pRawAnsCntx->m_pOption;
			test_endorsement_ans* ansp=(test_endorsement_ans*)pRawAnsCntx->m_pBody;
			if(aoptp==NULL||pRawAnsCntx->m_cbOption!=sizeof(*aoptp))
				return m_pIServer->ThrowProtocolError(pIConnect,pIBusiness,SYS_ERR_BADOPTBUFFER,"选项缓冲区错误。");
			DWORD cbInfo=pRawReqCntx->m_cbPackage-sizeof(*reqp);
			if(sizeof(*ansp)+cbInfo>pRawAnsCntx->m_cbBodyMax)
				return m_pIServer->ThrowProtocolError(pIConnect,pIBusiness,SYS_ERR_BUFFERTOOSMALL,"缓冲区太小。");
			aoptp->m_cReserved=roptp->m_cReserved;
			memcpy(ansp->m_szInfo,reqp->m_szInfo,cbInfo);
			SetBusinessCompleted(pIBusiness,pRawAnsCntx,sizeof(*ansp)+cbInfo);
			return BPR_SUCCESS;
		}
		break;
	case TEST_UPLOAD:
		{
			USING_RAW_REQCNTX();
			test_upload_req* reqp=(test_upload_req*)pRawReqCntx->m_pBody;

			// 得到上传的大小并计算传送的文件信息
			DWORD cbInfo=pRawReqCntx->m_cbPackage-sizeof(*reqp);
			DWORD dwFragmentNo=FRAGMENT_MAX;
			if(reqp->m_nLength>0) dwFragmentNo=100*(reqp->m_nOffset+cbInfo)/reqp->m_nLength;
			TRACE("TEST_UPLOAD Trans,FragmentNo,CmdNo=%d,%d,%d 线程ID:%d Offset=%d\r\n",pIBusiness->GetTransKey(),pIBusiness->GetReadFragmentNo(),pIBusiness->GetReadCmdNo(),GetCurrentThreadId(),reqp->m_nOffset);

			// 得到文件名,打开文件并写入数据
			SAFESTRING(reqp->m_szAsFile);
			TClibStr strToWrite=TClibStr(m_pIServer->GetHomeDir())+"swap/"+reqp->m_szAsFile;
			CAutoFile<FILE> FileToWrite(fopen(strToWrite,(reqp->m_nOffset==0)?"w+b":"a+b"));
			if(FileToWrite.IsNull()) return m_pIServer->ThrowProtocolError(pIConnect,pIBusiness,SYS_ERR_UNKNOWN,"文件打开失败。");
			fseek(FileToWrite,reqp->m_nOffset,SEEK_SET);
			if(cbInfo>0) fwrite(reqp->m_szInfo,cbInfo,1,FileToWrite);

			// 测试: 异常中止
			if(reqp->m_cTestType==1)
				return m_pIServer->ThrowProtocolError(pIConnect,pIBusiness,SYS_ERR_UNKNOWN,"这个是由测试产生的错误。");

			// 测试: 没有完整逻辑,让客户端卡住死等
			if(reqp->m_cTestType==2) dwFragmentNo=pIBusiness->GetReadFragmentNo();

			// 测试: 返回对应的事务状态通知
			if(dwFragmentNo!=FRAGMENT_MAX&&(dwFragmentNo%10)==0)
			{	BYTE acSwap[256]={0};
				test_upload_ans* ansp=(test_upload_ans*)acSwap;
				ansp->m_cResult=0;
				BPR nBPR=m_pIServer->PushingFragment(pISession,pIConnect,pIBusiness,NULL,0,(LPBYTE)ansp,sizeof(*ansp),dwFragmentNo);
				if(nBPR!=BPR_SUCCESS) return nBPR;
			}
			if(dwFragmentNo!=FRAGMENT_MAX) return BPR_NORESULT;
			USING_RAW_ANSCNTX();
			test_upload_ans* ansp=(test_upload_ans*)pRawAnsCntx->m_pBody;
			ansp->m_cResult=(BYTE)(FileToWrite.IsNull()?FALSE:TRUE);
			SetBusinessCompleted(pIBusiness,pRawAnsCntx,sizeof(*ansp),pIBusiness->GetCmdNo(),dwFragmentNo);
			return BPR_SUCCESS;
		}
		break;
	case TEST_DOWNLOAD:
		{
			USING_RAW_REQANSCNTX();
			test_download_req* reqp=(test_download_req*)pRawReqCntx->m_pBody;
			test_download_ans* ansp=(test_download_ans*)pRawAnsCntx->m_pBody;

			TRACE("TEST_DOWNLOAD 线程ID: %d\r\n",GetCurrentThreadId());

			// 计算下载的块大小
			DWORD nBlock=max(pRawAnsCntx->m_cbBodyMax,sizeof(*ansp))-sizeof(*ansp);
			if(nBlock<=1024) return m_pIServer->ThrowProtocolError(pIConnect,pIBusiness,SYS_ERR_UNKNOWN,"缓冲区太小。");
			nBlock-=1024;

			// 打开文件
			TClibStr strToRead=TClibStr(m_pIServer->GetHomeDir())+"swap/"+reqp->m_szFile;
			CAutoFile<FILE> FileToRead(fopen(strToRead,"rb"));
			if(FileToRead.IsNull())
				return m_pIServer->ThrowProtocolError(pIConnect,pIBusiness,SYS_ERR_UNKNOWN,"文件打开失败。");
			LONG nFileLength=FFILELENGTH(FileToRead);

			// 逐步读取文件
			if(fseek(FileToRead,reqp->m_nOffset,SEEK_SET)!=0)
				return m_pIServer->ThrowProtocolError(pIConnect,pIBusiness,SYS_ERR_UNKNOWN,"文件偏移错误。");


			// 逐步读取文件并作为分片推送
			LONG nOffset=reqp->m_nOffset;
			for(;nOffset<nFileLength;)
			{	ansp->m_nOffset=nOffset;
				DWORD dwFragmentNo=100*nOffset/max(nFileLength,1);
				LONG nRead=(LONG)fread(ansp->m_szInfo,1,nBlock,FileToRead);
				if(nRead==0) break;
				TRACE("SendFragment: Size=%d FNO=%d\r\n",nRead,dwFragmentNo);
				BPR nBPR=m_pIServer->PushingFragment(pISession,pIConnect,pIBusiness,NULL,0,(LPBYTE)ansp,sizeof(*ansp)+nRead,dwFragmentNo);
				if(nBPR!=BPR_SUCCESS) return nBPR;
				nOffset+=nRead;
			}

			// 下载完成
			ansp->m_nOffset=nOffset;
			SetBusinessCompleted(pIBusiness,pRawAnsCntx,sizeof(*ansp));
			return BPR_SUCCESS;
		}
		break;

	case TEST_LONGTALK:
		{
			TRACE("TEST_LONGTALK 线程ID: %d (会阻塞通讯线程)\r\n",GetCurrentThreadId());
			USING_RAW_REQANSCNTX();
			test_longtalk_ropt* roptp=(test_longtalk_ropt*)pRawReqCntx->m_pOption;
			test_longtalk_req* reqp=(test_longtalk_req*)pRawReqCntx->m_pBody;
			test_longtalk_aopt* aoptp=(test_longtalk_aopt*)pRawAnsCntx->m_pOption;
			test_longtalk_ans* ansp=(test_longtalk_ans*)pRawAnsCntx->m_pBody;
			CLIBVERIFY(reqp->m_cReserved==(BYTE)(~0));
			if(aoptp==NULL||pRawAnsCntx->m_cbOption!=sizeof(*aoptp))
				return m_pIServer->ThrowProtocolError(pIConnect,pIBusiness,SYS_ERR_BADOPTBUFFER,"选项缓冲区错误。");
			DWORD cbInfo=pRawReqCntx->m_cbPackage-sizeof(*reqp);
			if(sizeof(*ansp)+cbInfo>pRawAnsCntx->m_cbBodyMax)
				return m_pIServer->ThrowProtocolError(pIConnect,pIBusiness,SYS_ERR_BUFFERTOOSMALL,"缓冲区太小。");
*//*
			if(pIBusiness->GetReadFragmentNo()<80)
			{	BYTE acSwap[2048]={0};
				test_hey_aopt aopt={0};
				test_hey_ans* ansp=(test_hey_ans*)acSwap;
				ansp->m_cReserved=0;
				sprintf(ansp->m_szSentence,"正准备发送:%d\r\n",pIBusiness->GetReadFragmentNo());
				BPR nBPR=SendS2CPushing(m_pIServer,pISession,pIConnect,pISession->GetCreator(),(LPBYTE)&aopt,sizeof(aopt),(LPBYTE)ansp,sizeof(*ansp),TEST_HEY,"HEY!");
				if(nBPR!=BPR_SUCCESS) return nBPR;
			}
*//*
			aoptp->m_cReserved=roptp->m_cReserved;
			ansp->m_cReserved=roptp->m_cReserved;
			memcpy(ansp->m_szInfo,reqp->m_szInfo,cbInfo);
			SetBusinessCompleted(pIBusiness,pRawAnsCntx,sizeof(*ansp)+cbInfo,wCmdNo,pIBusiness->GetReadFragmentNo());
			return BPR_SUCCESS;
		}
		break;
*/
  default:
    break;
	}

	return BPR_NOT_SUPPORT;
}

BPR CT2EEHandler::OnPushing(ISession* pISession,IConnect* pIConnect,IBusiness* pIBusinessHost,DWORD dwPushingLv,DWORD idPushingType,DWORD dwPushingOption,LPBYTE pSequence,DWORD cbSequence)
{
	UNREFERENCED_PARAMETER(pISession);
	UNREFERENCED_PARAMETER(pIConnect);
	UNREFERENCED_PARAMETER(pIBusinessHost);
	UNREFERENCED_PARAMETER(dwPushingLv);
	UNREFERENCED_PARAMETER(idPushingType);
	UNREFERENCED_PARAMETER(dwPushingOption);
	UNREFERENCED_PARAMETER(pSequence);
	UNREFERENCED_PARAMETER(cbSequence);
	TRACE("CT2EEHandler:OnPushing\r\n");
	return BPR_SUCCESS;
}

DWORD CT2EEHandler::OnAlarm(DWORD_PTR dwAlarmID)
{
  if (dwAlarmID == m_Alarmer.GetAlarmerId())  {
    m_pAggregator->TickProc();
  }

  if (dwAlarmID == m_LongAlarmer.GetAlarmerId())  {
    m_pAggregator->LongTickProc();
  }	
	return 0;
}

