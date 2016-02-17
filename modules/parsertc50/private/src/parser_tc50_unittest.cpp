/*

*/
//#define ENABLE_DETECT_MEMORY_LEAK
#include "test.h"

#ifdef DO_UNITTEST

#include "parse_ix.h"
#include "tc50_log.h"
#include "tc50_dict.h"
USING_NAMESPACE_AGGREGATOR

#include "object.h"
USING_NAMESPACE

#include "memory_pool.h"
#include "file_util.h"
#include "dynamic_library.h"
USING_NAMESPACE_BASE

#include "regex.hpp"

// ǿ��1�ֽڶ���
#pragma pack(1) {

// ������ȫ�ֳ�ʼ��
typedef struct tagFEMBRANCHINFO
{	DWORD   m_dwBranchID;								// Ӫҵ��ID
DWORD	m_dwMinThreadNum;							// ��Ӫҵ����Ӧ����С�߳���
DWORD	m_dwMaxThreadNum;							// ��Ӫҵ����Ӧ������߳���
CHAR	m_szBranchSection[MAX_PATH];				// Ӫҵ�����ý�
} FEMBRANCHINFO,*LPFEMBRANCHINFO;
typedef struct tagFEMPARAM
{	DWORD				m_dwReserved1;					// �Ƿ����첽��ʽ����
LPVOID				m_pfnDataIO;					// ����IO�ص�����
DWORD_PTR			m_dwIoParam;					// ����IO�ص�����
DWORD				m_dwMaxAnsBufLen;				// ���Ӧ�𻺳�������
DWORD				m_dwBranchNum;					// Ӫҵ����
CHAR				m_szOptionFile[MAX_PATH];		// �����ļ�
CHAR				m_szPublicSection[MAX_PATH];	// �������ý�
LPFEMBRANCHINFO		m_lpBranchInfos;				// Ӫҵ������
BYTE				m_szReserved2[64];				// �����ֽ�,ȫ��
} FEMPARAM,*LPFEMPARAM;

#pragma pack() {

// ȡ�������ֵ�
enum enumDICTTYPE
{	DICTTYPE_SIMPLE,									// ���ֵ�,��Ҫ���ڵ�¼����
DICTTYPE_FULL,										// �����ֵ�,��Ҫ����ҵ����
};

typedef BOOL	(*WT_INITINST)(LPFEMPARAM);
typedef VOID	(*WT_EXITINST)();
typedef DWORD	(*WT_GETDICTS)(DWORD,DWORD,LPBYTE);


LONG CALLBACK DataOut(LPSTR lpszCode,SHORT nDataType,LPVOID lpData,
                      DWORD cbData,DWORD_PTR dwIoParam,DWORD dwReserved) {
{
  UNREFERENCED_PARAMETER( lpszCode);
  UNREFERENCED_PARAMETER( nDataType);
  UNREFERENCED_PARAMETER( lpData);
  UNREFERENCED_PARAMETER( cbData);
  UNREFERENCED_PARAMETER( dwIoParam);
  UNREFERENCED_PARAMETER( dwReserved);
  return 0;
}

TEST_BEGIN(module, export_dict) {

  // 
  DynamicLibrary dync_lib("JzxpWin.dll", "z:/logtest");
  ASSERT_EQ(TRUE, dync_lib.isValid());

  WT_INITINST pfn_InitInst = dync_lib.LocateSymbol<WT_INITINST>("FEM_InitInst");
  ASSERT_NE(NULL, pfn_InitInst);

  WT_EXITINST pfn_ExitInst = dync_lib.LocateSymbol<WT_EXITINST>("FEM_ExitInst");
  ASSERT_NE(NULL, pfn_ExitInst);

  WT_GETDICTS pfn_GetDicts = dync_lib.LocateSymbol<WT_GETDICTS>("FEM_GetDicts");
  ASSERT_NE(NULL, pfn_GetDicts);

  // ���ó�ʼ������
  FEMPARAM FEMParam;
  BZERO(&FEMParam, sizeof(FEMParam));
  FEMBRANCHINFO info;
  BZERO(&info, sizeof(info));

  FEMParam.m_dwReserved1= 0;
  FEMParam.m_pfnDataIO= DataOut;
  FEMParam.m_dwIoParam= 0;
  FEMParam.m_dwMaxAnsBufLen= 128*1024;
  FEMParam.m_dwBranchNum= 0;
  //COPYSTRARRAY( FEMParam.m_szOptionFile, m_szModuleCfgFilePath);
  STRCPY( FEMParam.m_szOptionFile, "z:\\logtest\\jzapixp.ini");
  STRCPY( FEMParam.m_szPublicSection, "Public");
  FEMParam.m_lpBranchInfos= &info;

  bool_t bRet = pfn_InitInst(&FEMParam);
  ASSERT_EQ(TRUE, bRet);


  // get
  uint8_t byteDict[128*1024]= { 0};
  uint32_t dwDict= sizeof( byteDict);
  dwDict = pfn_GetDicts(DICTTYPE_SIMPLE, dwDict, byteDict);

  AutoReleaseFile autoRelFile(OpenFile("z:/test.dict", strAttrOpenWrite));
  ASSERT_NE(NULL, autoRelFile);

  const uint8_t* pWriteData = byteDict;
  file_size_t nWriteLen = dwDict;
  ASSERT_EQ(RC_S_OK, WriteFile(autoRelFile, 0, pWriteData, &nWriteLen));

  pfn_ExitInst();

} TEST_END

TEST_BEGIN(module, wtcom_req) {

  AutoReleaseFile autoRelFile(OpenFile("z:/logtest/JzxpWin.dll_27_20130717160850.sto", strAttrOpenRead));
  ASSERT_NE(NULL, autoRelFile);

  uint8_t arrFile[96*1024] = {0};
  file_size_t nReadSize = 96*1024;
  ReadFile(arrFile, &nReadSize, autoRelFile, 0);
  ASSERT_LT(0, nReadSize);

  uint32_t nDictSize = *((uint32_t*)arrFile);
  nReadSize = nReadSize - nDictSize - 8;
  IDictIX* pIDictIX = IDictIX::CreateInstance(kStrWtDict2, arrFile + 8 + nDictSize, (uint32_t)nReadSize);
//     nReadSize = nDictSize;
//     IDictIX* pIDictIX = IDictIX::CreateInstance(kStrWtDict2, arrFile + 4, (uint32_t)nReadSize);
  ASSERT_NE(NULL, pIDictIX);
  ASSERT_EQ(TRUE, pIDictIX->isValid());

  IParseIX* pIParseIX = IParseIX::CreateInstance(kStrParseWtComm, pIDictIX);
  ASSERT_NE(NULL, pIDictIX);
  ASSERT_EQ(TRUE, pIParseIX->isValid());

  const char_t* strData = "1045960|************||8|1045960|12|0|||||||||S7S1B1qKQsA=|||||1||0|10.245.7.230;JERIES-LAPTOP;Jeries;2047;Intel(R)Core(TM)i5CPUM460@2.53GHz*4;00020655;Windows7 Professional Service Pack 1 (Build 7601);0.0.0.0,0.0.0.0,0.0.0.0,10.245.7.230;58946BC96421,58946BC96421,58946BC96420,F0DEF13977C4;5VJAQJX5;05/20/2011;D523FD57-CA05-4A26-A00E-837564D5EB33|||0|||||||\r\n";

  // TDX_ID_FUNC_REQ_KHJY
  uint32_t nLen = (uint32_t)STRLEN(strData);
  rc_t rc = pIParseIX->Parse(100, strData, nLen);

  uint32_t return_no = 0x000ff;
  pIParseIX->GetReturnNo(&return_no);

  char_t errMsg[128] = {0};
  uint32_t nMsgLen = 128;
  pIParseIX->GetErrmsg(errMsg, nMsgLen);

  uint32_t nVal = 0;
  // TDX_ID_ZHLB
  rc = pIParseIX->get_data(125, &nVal);

  char_t arrVal[1024] = {0};
  char_t* pVal = arrVal;
  nLen = 1024;

  // TDX_ID_KHH
  rc = pIParseIX->get_data(120, pVal, sizeof(arrVal));

  // TDX_ID_XT_CLIVER
  rc = pIParseIX->get_data(1235, pVal, sizeof(arrVal));

  // TDX_ID_XT_MACADDR
  rc = pIParseIX->get_data(1204, pVal, sizeof(arrVal));

} TEST_END

TEST_BEGIN(module, wtcom_ans) {

  AutoReleaseFile autoRelFile(OpenFile("z:/logtest/JzxpWin.dll_27_20130717160850.sto", strAttrOpenRead));
  ASSERT_NE(NULL, autoRelFile);

  uint8_t arrFile[96*1024] = {0};
  file_size_t nReadSize = 96*1024;
  ReadFile(arrFile, &nReadSize, autoRelFile, 0);
  ASSERT_LT(0, nReadSize);

  
//  IDictIX* pIDictIX = IDictIX::CreateInstance(kStrWtDict2, arrFile, (uint32_t)nReadSize);
   uint32_t nDictSize = *((uint32_t*)arrFile);
   nReadSize = nReadSize - nDictSize - 8;
   IDictIX* pIDictIX = IDictIX::CreateInstance(kStrWtDict2, arrFile + 8 + nDictSize, (uint32_t)nReadSize);
//   nReadSize = nDictSize;
//   IDictIX* pIDictIX = IDictIX::CreateInstance(kStrWtDict2, arrFile + 4, (uint32_t)nReadSize);
  ASSERT_NE(NULL, pIDictIX);
  ASSERT_EQ(TRUE, pIDictIX->isValid());

  IParseIX* pIParseIX = IParseIX::CreateInstance(kStrParseWtComm, pIDictIX);
  ASSERT_NE(NULL, pIDictIX);
  ASSERT_EQ(TRUE, pIParseIX->isValid());

  const char_t* strData = "123|1000,100000000100,���\s[100000000100],|3|||\r\n"
    "100000000100|���\s[100000000100]|1|1|1|0|3|����|1000||1|\r\n"
    "10000000010000|\r\n"
    "10000000010001|\r\n";

  // TDX_ID_FUNC_REQ_KHJY
  uint32_t nLen = (uint32_t)STRLEN(strData);
  rc_t rc = pIParseIX->Parse(100, strData, nLen);

  uint32_t return_no = 0x000ff;
  pIParseIX->GetReturnNo(&return_no);

  char_t errMsg[128] = {0};
  uint32_t nMsgLen = 128;
  pIParseIX->GetErrmsg(errMsg, nMsgLen);

  uint32_t nVal = 0;
  // TDX_ID_XT_ERRCODE
  rc = pIParseIX->get_data(1200, &nVal);

  char_t arrVal[1024] = {0};
  char_t* pVal = arrVal;
  nLen = 1024;

  // TDX_ID_KHH
  rc = pIParseIX->get_data(120, pVal, sizeof(arrVal));

  // TDX_ID_KHMC
  rc = pIParseIX->get_data(122, pVal, sizeof(arrVal));

} TEST_END

TEST_BEGIN(module, parser_tc50) {

  AutoReleaseFile autoRelFile(OpenFile("z:/20130413.log", strAttrOpenRead));
  ASSERT_NE(NULL, autoRelFile);

  char_t arrFile[96*1024] = {0};
  file_size_t nReadSize = 96*1024;
  ReadFile((uint8_t*)arrFile, &nReadSize, autoRelFile, 0);
  ASSERT_LT(0, nReadSize);

  //RegEx regexAns(0, "^\\([0-9]{2}\\)\\1", 0);
  //RegEx regexAns(0, "^[0-9]{2}:");
  // RegEx regexAns(0, "a\\(b*\\)c\\1d", 0);
  //RegEx regexAns(0, "a\\(b.\\)c\\1d", 0);
  //RegEx regexAns(0, "^[0-9]{2}:[0-9]{2}:[0-9]{2}\\.[0-9]{3} ");
  //RegEx regexReq(0, "^[0-9]{2}:[0-9]{2}:[0-9]{2}\\.[0-9]{3} [[:xdigit:]]{4} ");
  /*
  10:19:09.976 �������� IP:10.245.7.230 MAC:F0DEF13977C4 �߳�:00000F8C ͨ��ID:1 ����ID:4 ����:(1-4504)ͳһ��֤(*) Ӫҵ��:(0001)����֤ȯ(*) {
  */

  const char_t* strRegExReq = "^"
    "[0-9]{2}:[0-9]{2}:[0-9]{2}\\.[0-9]{3} "
    "�������� "
    "IP:[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3} "
    "MAC:[[:xdigit:]]{12} "
    "�߳�:[[:xdigit:]]{8} "
    "ͨ��ID:[0-9]+ "
    "����ID:[0-9]+ "
    "����:\\([0-9]+-[0-9]+\\)[^\\w\\s]+\\ "
    "Ӫҵ��:\\([0-9]+\\)[^\\w\\s]+\\"
    "\r\n"
    ;

  const char_t* strRegExAns = "^"
    "[0-9]{2}:[0-9]{2}:[0-9]{2}\\.[0-9]{3} "
    "[^\\w\\s]{8} "
    "IP:[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3} "
    "MAC:[[:xdigit:]]{12} "
    "�߳�:[[:xdigit:]]{8} "
    "ͨ��ID:[0-9]+ "
    "����ID:[0-9]+ "
    "����:\\([0-9]+-[0-9]+\\)[^\\w\\s]+\\ "
    "Ӫҵ��:\\([0-9]+\\)[^\\w\\s]+\\ "
    "��ʱA:[0-9]+ "
    "��ʱB:[0-9]+ "
    "�Ŷ�:[0-9]+"
    "\r\n"
    ;

  RegEx regexAns(0, strRegExAns);
  RegEx regexReq(0, strRegExReq);
  bool_t bMatch;
  bMatch = regexAns.isMatchOnly("abbcbbd");
  bMatch = regexAns.isMatchOnly("10:19:09.976 ");
  bMatch = regexReq.isMatchOnly("10:19:09.976 �������� ");

  const char_t* strLogText = "10:19:09.976 �������� IP:10.245.7.230 MAC:F0DEF13977C4 �߳�:00000F8C ͨ��ID:1 ����ID:4 ����:(1-4504)ͳһ��֤(*) Ӫҵ��:(0001)����֤ȯ(*)\r\n";
  bMatch = regexReq.isMatchOnly(strLogText);


  bMatch = regexReq.isMatchOnly("a10:19:09.976 �������� IP:10.245.7.230 MAC:F0DEF13977C4 �߳�:00000F8C ͨ��ID:1 ����ID:4 ����:(1-4504)ͳһ��֤(*) Ӫҵ��:(0001)����֤ȯ(*)");

  bMatch = regexAns.isMatchOnly("17:20:52.492 �ɹ����� IP:10.245.7.230 MAC:F0DEF13977C4 �߳�:000006E4 ͨ��ID:12 ����ID:1065 ����:(0-908)��ѯ������Ϣ(*) Ӫҵ��:(0070)���Ű���·֤ȯӪҵ�� ��ʱA:0 ��ʱB:0 �Ŷ�:0\r\n");


  const int kScanfCount = 15;
  const int kScanfSize = 1024;

  typedef enum HEAD_TYPE_E_{
    HT_RECV_TIME = 0
    , HT_PACKET_TYPE
    , HT_IP
    , HT_MAC
    , HT_THEAD_ID
    , HT_CHANNEL_ID
    , HT_TRANS_ID
    , HT_REQ_TYPE
    , HT_FUNC_ID
    , HT_FUNC_NAME
    , HT_BRANCH_ID
    , HT_BRANCH_NAME

    , HT_TIMEA
    , HT_TIMEB
    , HT_QUEUE

  } HEAD_TYPE_e;

  bMatch = regexReq.isMatchOnly(arrFile + 330);
  bMatch = regexReq.isMatchOnly(arrFile);

  int nScaned = 0;

  uint32_t nHour = 0, nMin = 0, nSec = 0, nMilSec = 0, nChannelID = 0, nTransID = 0, nReqType = 0, nFuncID = 0, nBranchID = 0;
  char_t strReqMsg[32]={0}, strIP[64]={0}, strMAC[32]={0}, strThreadID[16]={0}, strFuncName[64]={0}, strBranchName[64]={0};
  const char_t* strLogText2 = "10:19:09.976 ��������";

  nScaned = SSCANF(strLogText2, 
    "%d:%d:%d.%d %s IP:%s MAC:%s �߳�:%s ͨ��ID:%d ����ID:%d ����:(%d-%d)%s Ӫҵ��:(%d)%s", 
    &nHour, &nMin, &nSec, &nMilSec
    , strReqMsg, sizeof(strReqMsg)
    , strIP, sizeof(strIP)
    , strMAC, sizeof(strMAC)
    , strThreadID, sizeof(strThreadID)
    , &nChannelID, &nTransID, &nReqType, &nFuncID
    , strFuncName, sizeof(strFuncName)
    , &nBranchID
    , strBranchName, sizeof(strBranchName)
    );

  char_t szHSubValue[kScanfCount][kScanfSize]= {0};
  nScaned = SSCANF((const char_t*)arrFile, 
    "%s %s IP:%s MAC:%s �߳�:%s ͨ��ID:%s ����ID:%s ����:(%[^()-]-%[^()-])%s Ӫҵ��:(%[^()])%s", 
    szHSubValue[ HT_RECV_TIME],		// ʱ��
    szHSubValue[ HT_PACKET_TYPE],		// ������
    szHSubValue[ HT_IP],		// IP
    szHSubValue[ HT_MAC],		// MAC
    szHSubValue[ HT_THEAD_ID],		// �߳�
    szHSubValue[ HT_THEAD_ID],		// ͨ��ID
    szHSubValue[ HT_TRANS_ID],		// ����ID
    szHSubValue[ HT_REQ_TYPE],		// ��������
    szHSubValue[ HT_FUNC_ID],		// �����ܺ�
    szHSubValue[ HT_FUNC_NAME],		// ��������
    szHSubValue[ HT_BRANCH_ID],		// Ӫҵ��ID
    szHSubValue[ HT_BRANCH_NAME]		// Ӫҵ������
  );

  if (kScanfCount != nScaned) {

    return;
  }

} TEST_END


class FileIteratorImpl : public IFileIteratorBase {
public:
  class MemoryNodeImpl : public IMemoryNode {
  public:
    MemoryNodeImpl(uint8_t* d, uint32_t l) {
      : d(d) {
      , l(l) {
    {}

    void Release() {}
    void AddRef() {}

  public:
    uint8_t* data() const { return d; }
    uint32_t len() const { return l; }

  private:
    uint8_t* d;
    uint32_t l;
  }; // IMemoryNode



  FileIteratorImpl(char_t* data, uint32_t len) {
    : memNode((uint8_t *)data, len) {
  {}

  void Release() {}

public:
  rc_t file_iterate(file_iterator_callback_t, void* /*context*/) { return RC_S_FAILED; }

public:
  IMemoryNode* GetFileData(file_id_t, file_size_t* pos) {

    if (0 == (*pos)) { return &memNode; }
    return NULL;
  }

private:
  MemoryNodeImpl memNode;
}; // FileIteratorImpl

TEST_BEGIN(module, parser_tc50_log) {

  TC50Dict dict;
  TC50Log  log;

  const char_t* strCollector = "fafd";
  const char_t* strDir = "z:/fds";

  dict.AddDict(strCollector, strDir, 3, micro_time::time());

  AutoReleaseFile autoRelFile(OpenFile("z:/20130413.log", strAttrOpenRead));
  ASSERT_NE(NULL, autoRelFile);

  char_t arrFile[96*1024] = {0};
  file_size_t nReadSize = 96*1024;
  ReadFile((uint8_t*)arrFile, &nReadSize, autoRelFile, 0);
  ASSERT_LT(0, nReadSize);

  AutoReleaseFile autoRelDictFile(OpenFile("z:/JzxpWin.dll_27_20130717160850.sto", strAttrOpenRead));
  ASSERT_NE(NULL, autoRelDictFile);

  char_t arrDictFile[96*1024] = {0};
  file_size_t nDictReadSize = 96*1024;
  ReadFile((uint8_t*)arrDictFile, &nDictReadSize, autoRelDictFile, 0);
  ASSERT_LT(0, nDictReadSize);

  uint32_t nLogSize = (uint32_t)nReadSize;
  FileIteratorImpl fileIteratorImpl(arrDictFile, (uint32_t)nDictReadSize);
  uint32_t nLogPos = 0;
  rc_t rc; 
  do {
    nLogSize = (uint32_t)nReadSize - nLogPos;
    rc = log.Parse(arrFile + nLogPos, &nLogSize, &dict, &fileIteratorImpl, strCollector, strDir);
    nLogPos += nLogSize;

    TRACE(log.strSysInfo);
    TRACE("%d%d%d.%d, %s, %s", log.nHour, log.nMin, log.nSec, log.nMilSec, log.strFuncName, log.strIP);
  } while (RC_S_OK == rc);

} TEST_END

EXTERN_C SYMBOL_EXPORT
TEST_MAIN_MOCK_BEGIN(module, unittest) {

  TEST_EXEC(module, parser_tc50_log);
  TEST_EXEC(module, parser_tc50);
  //TEST_EXEC(module, export_dict);
  TEST_EXEC(module, wtcom_req);
  TEST_EXEC(module, wtcom_ans);

} TEST_MAIN_MOCK_END

#endif // DO_UNITTEST
