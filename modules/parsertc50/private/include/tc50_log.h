/*

*/

#ifndef TC50_LOG_H_
#define TC50_LOG_H_

//////////////////////////////////////////////////////////////////////////
#include "aggregator_base.h"

#include "parse_ix.h"

#include "tc50_dict.h"

BEGIN_NAMESPACE_AGGREGATOR

//////////////////////////////////////////////////////////////////////////
// sql table;
const size_t kLogTypeNum    = 8;
static const char_t* kStrTables[kLogTypeNum] = {
  _STR("t_tc50_trade_req")
  , _STR("t_tc50_trade_ans")
  , _STR("t_tc50_logon_req")
  , _STR("t_tc50_logon_ans")
  , _STR("t_tc50_sc_req")
  , _STR("t_tc50_sc_ans")
  , _STR("t_tc50_log_info")
  , _STR("t_tc50_log_failed")
};

static const char_t* kStrTableFields[kLogTypeNum] = {

  "`parser`, `collector`, `file_id`, `file_pos_start`, `log_len`, `timestamp`, `log_date`, `log_type`, `log_time`, `ip`, `mac`, `thread_id`, `channel_id`, `trans_id`, `req_type`, `func_id`, `func_name`, `branch_id`, `branch_name`, `KHH`, `KHMC`, `ZHLB`, `ZJZH`, `GDDM`, `OP_WTFS`, `WTBH`, `WTFS`, `ZQDM`, `MMBZ`, `JYDW`, `WTSL`, `WTJG`, `WTRQ`, `WTSJ`, `XT_CHECKRISKFLAG`, `reserve_a`, `reserve_b`, `reserve_c`"

  , "`parser`, `collector`, `file_id`, `file_pos_start`, `log_len`, `timestamp`, `log_date`, `log_type`, `log_time`, `ip`, `mac`, `thread_id`, `channel_id`, `trans_id`, `req_type`, `func_id`, `func_name`, `branch_id`, `branch_name`, `timeA`, `timeB`, `queue`, `return_no`, `return_msg`, `WTBH`, `XT_CHECKRISKFLAG`, `RETINFO`, `reserve_d`, `reserve_e`, `reserve_f`"

  , "`parser`, `collector`, `file_id`, `file_pos_start`, `log_len`, `timestamp`, `log_date`, `log_type`, `log_time`, `ip`, `mac`, `thread_id`, `channel_id`, `trans_id`, `req_type`, `func_id`, `func_name`, `branch_id`, `branch_name`, `XT_GTLB`, `KHH`, `KHMC`, `ZHLB`, `ZJZH`, `SHGD`, `SZGD`, `XT_CLITYPE`, `XT_CLIVER`, `XT_VIPFLAG`, `XT_MACHINEINFO`, `reserve_a`, `reserve_b`, `reserve_c`"

  , "`parser`, `collector`, `file_id`, `file_pos_start`, `log_len`, `timestamp`, `log_date`, `log_type`, `log_time`, `ip`, `mac`, `thread_id`, `channel_id`, `trans_id`, `req_type`, `func_id`, `func_name`, `branch_id`, `branch_name`, `timeA`, `timeB`, `queue`, `return_no`, `return_msg`, `reserve_a`, `reserve_b`, `reserve_c`, `reserve_d`, `reserve_e`, `reserve_f`"

  , "`parser`, `collector`, `file_id`, `file_pos_start`, `log_len`, `timestamp`, `log_date`, `log_type`, `log_time`, `ip`, `mac`, `thread_id`, `channel_id`, `trans_id`, `req_type`, `func_id`, `func_name`, `branch_id`, `branch_name`, `XT_GTLB`, `CA_KHH`, `CA_KHMC`, `CA_VER`, `CA_AQJB`, `CA_TXMM`, `CA_ISVIPHOST`, `CA_JQTZM`, `CA_SLOTSN`, `CA_CID`, `CA_CERTREQ`, `CA_USERCERDN`, `CA_ZSQSRQ`, `CA_ZSJZRQ`, `CA_CERTSN`, `CA_CERTINFO`, `CA_MACHINENAME`, `CA_DLSJ`, `CA_LASTIP`, `CA_MAC`, `CA_CSCS`, `CA_RESV`, `reserve_a`, `reserve_b`, `reserve_c`"

  , "`parser`, `collector`, `file_id`, `file_pos_start`, `log_len`, `timestamp`, `log_date`, `log_type`, `log_time`, `ip`, `mac`, `thread_id`, `channel_id`, `trans_id`, `req_type`, `func_id`, `func_name`, `branch_id`, `branch_name`, `timeA`, `timeB`, `queue`, `return_no`, `return_msg`, `reserve_a`, `reserve_b`, `reserve_c`, `reserve_d`, `reserve_e`, `reserve_f`"

  , "`parser`, `collector`, `file_id`, `file_pos_start`, `log_len`, `timestamp`, `log_date`, `log_type`, `log_time`, `info`, `channel_id`, `ip`, `op_organization`, `op_account`, `reason`"

  , "`parser`, `collector`, `file_id`, `file_pos_start`, `log_len`, `timestamp`, `log_date`"

};

static const char_t* kStrCreateSQLFmt = "\
  CREATE TABLE IF NOT EXISTS %st_tc50_trade_req%s (\
  `parser` varchar(16) NOT NULL COMMENT 'lookup',\
  `collector` varchar(32) NOT NULL COMMENT 'lookup',\
  `file_id` int(10) NOT NULL COMMENT '文件编号',\
  `file_pos_start` bigint(20) NOT NULL COMMENT '文件位置',\
  `log_len` int(10) NOT NULL COMMENT '文件位置',\
  `timestamp` int(10) NOT NULL COMMENT '添加时间',\
  `log_date` smallint(5) NOT NULL,\
  `log_type` varchar(16) NOT NULL COMMENT 'lookup',\
  `log_time` int(10) NOT NULL,\
  `ip` bigint(20) NOT NULL,\
  `mac` bigint(20) NOT NULL,\
  `thread_id` smallint(5) NOT NULL,\
  `channel_id` int(10) NOT NULL,\
  `trans_id` int(10) NOT NULL,\
  `req_type` tinyint(3) NOT NULL,\
  `func_id` smallint(5) NOT NULL,\
  `func_name` varchar(32) NOT NULL COMMENT 'lookup',\
  `branch_id` int(10) NOT NULL,\
  `branch_name` varchar(64) NOT NULL COMMENT 'lookup',\
  `KHH` varchar(32) NOT NULL,\
  `KHMC` varchar(32) NOT NULL,\
  `ZHLB` int(10) NOT NULL,\
  `ZJZH` varchar(16) NOT NULL,\
  `GDDM` varchar(16) NOT NULL,\
  `OP_WTFS` int(10) NOT NULL,\
  `WTBH` varchar(32) NOT NULL,\
  `WTFS` int(10) NOT NULL,\
  `ZQDM` varchar(16) NOT NULL,\
  `MMBZ` int(10) NOT NULL,\
  `JYDW` int(10) NOT NULL,\
  `WTSL` int(10) NOT NULL,\
  `WTJG` DECIMAL(10,3) NOT NULL,\
  `WTRQ` int(10) NOT NULL,\
  `WTSJ` int(10) NOT NULL,\
  `XT_CHECKRISKFLAG` INT(10) NOT NULL,\
  `reserve_a` varchar(512) NOT NULL,\
  `reserve_b` varchar(512) NOT NULL,\
  `reserve_c` varchar(512) NOT NULL\
  ) ENGINE=%s DEFAULT CHARSET=utf8;\
  \
  CREATE TABLE IF NOT EXISTS %st_tc50_trade_ans%s (\
  `parser` varchar(16) NOT NULL COMMENT 'lookup',\
  `collector` varchar(32) NOT NULL COMMENT 'lookup',\
  `file_id` int(10) NOT NULL COMMENT '文件编号',\
  `file_pos_start` bigint(20) NOT NULL COMMENT '文件位置',\
  `log_len` int(10) NOT NULL COMMENT '文件位置',\
  `timestamp` int(10) NOT NULL COMMENT '添加时间',\
  `log_date` smallint(5) NOT NULL,\
  `log_type` varchar(16) NOT NULL COMMENT 'lookup',\
  `log_time` int(10) NOT NULL,\
  `ip` bigint(20) NOT NULL,\
  `mac` bigint(20) NOT NULL,\
  `thread_id` smallint(5) NOT NULL,\
  `channel_id` int(10) NOT NULL,\
  `trans_id` int(10) NOT NULL,\
  `req_type` tinyint(3) NOT NULL,\
  `func_id` smallint(5) NOT NULL,\
  `func_name` varchar(32) NOT NULL COMMENT 'lookup',\
  `branch_id` int(10) NOT NULL,\
  `branch_name` varchar(64) NOT NULL COMMENT 'lookup',\
  `timeA` int(10) NOT NULL,\
  `timeB` int(10) NOT NULL,\
  `queue` int(10) NOT NULL,\
  `return_no` int(10) NOT NULL,\
  `return_msg` varchar(512) NOT NULL,\
  `WTBH` varchar(32) NOT NULL,\
  `XT_CHECKRISKFLAG` INT(11) NOT NULL,\
  `RETINFO` VARCHAR(512) NOT NULL,\
  `reserve_d` varchar(512) NOT NULL,\
  `reserve_e` varchar(512) NOT NULL,\
  `reserve_f` varchar(512) NOT NULL\
  ) ENGINE=%s DEFAULT CHARSET=utf8;\
  \
  CREATE TABLE IF NOT EXISTS %st_tc50_logon_req%s (\
  `parser` varchar(16) NOT NULL COMMENT 'lookup',\
  `collector` varchar(32) NOT NULL COMMENT 'lookup',\
  `file_id` int(10) NOT NULL COMMENT '文件编号',\
  `file_pos_start` bigint(20) NOT NULL COMMENT '文件位置',\
  `log_len` int(10) NOT NULL COMMENT '文件位置',\
  `timestamp` int(10) NOT NULL COMMENT '添加时间',\
  `log_date` smallint(5) NOT NULL,\
  `log_type` varchar(16) NOT NULL COMMENT 'lookup',\
  `log_time` int(10) NOT NULL,\
  `ip` bigint(20) NOT NULL,\
  `mac` bigint(20) NOT NULL,\
  `thread_id` smallint(5) NOT NULL,\
  `channel_id` int(10) NOT NULL,\
  `trans_id` int(10) NOT NULL,\
  `req_type` tinyint(3) NOT NULL,\
  `func_id` smallint(5) NOT NULL,\
  `func_name` varchar(32) NOT NULL COMMENT 'lookup',\
  `branch_id` int(10) NOT NULL,\
  `branch_name` varchar(64) NOT NULL COMMENT 'lookup',\
  `XT_GTLB` char(2) NOT NULL,\
  `KHH` varchar(32) NOT NULL,\
  `KHMC` varchar(32) NOT NULL,\
  `ZHLB` int(10) NOT NULL,\
  `ZJZH` varchar(16) NOT NULL,\
  `SHGD` VARCHAR(16) NOT NULL,\
  `SZGD` VARCHAR(16) NOT NULL,\
  `XT_CLITYPE` int(10) NOT NULL,\
  `XT_CLIVER` varchar(16) NOT NULL,\
  `XT_VIPFLAG` int(10) NOT NULL,\
  `XT_MACHINEINFO` varchar(1024) NOT NULL,\
  `reserve_a` varchar(512) NOT NULL,\
  `reserve_b` varchar(512) NOT NULL,\
  `reserve_c` varchar(512) NOT NULL\
  ) ENGINE=%s DEFAULT CHARSET=utf8;\
  \
  CREATE TABLE IF NOT EXISTS %st_tc50_logon_ans%s (\
  `parser` varchar(16) NOT NULL COMMENT 'lookup',\
  `collector` varchar(32) NOT NULL COMMENT 'lookup',\
  `file_id` int(10) NOT NULL COMMENT '文件编号',\
  `file_pos_start` bigint(20) NOT NULL COMMENT '文件位置',\
  `log_len` int(10) NOT NULL COMMENT '文件位置',\
  `timestamp` int(10) NOT NULL COMMENT '添加时间',\
  `log_date` smallint(5) NOT NULL,\
  `log_type` varchar(16) NOT NULL COMMENT 'lookup',\
  `log_time` int(10) NOT NULL,\
  `ip` bigint(20) NOT NULL,\
  `mac` bigint(20) NOT NULL,\
  `thread_id` smallint(5) NOT NULL,\
  `channel_id` int(10) NOT NULL,\
  `trans_id` int(10) NOT NULL,\
  `req_type` tinyint(3) NOT NULL,\
  `func_id` smallint(5) NOT NULL,\
  `func_name` varchar(32) NOT NULL COMMENT 'lookup',\
  `branch_id` int(10) NOT NULL,\
  `branch_name` varchar(64) NOT NULL COMMENT 'lookup',\
  `timeA` int(10) NOT NULL,\
  `timeB` int(10) NOT NULL,\
  `queue` int(10) NOT NULL,\
  `return_no` int(10) NOT NULL,\
  `return_msg` varchar(512) NOT NULL,\
  `reserve_a` varchar(512) NOT NULL,\
  `reserve_b` varchar(512) NOT NULL,\
  `reserve_c` varchar(512) NOT NULL,\
  `reserve_d` varchar(512) NOT NULL,\
  `reserve_e` varchar(512) NOT NULL,\
  `reserve_f` varchar(512) NOT NULL\
  ) ENGINE=%s DEFAULT CHARSET=utf8;\
  \
  CREATE TABLE IF NOT EXISTS %st_tc50_sc_req%s (\
  `parser` varchar(16) NOT NULL COMMENT 'lookup',\
  `collector` varchar(32) NOT NULL COMMENT 'lookup',\
  `file_id` int(10) NOT NULL COMMENT '文件编号',\
  `file_pos_start` bigint(20) NOT NULL COMMENT '文件位置',\
  `log_len` int(10) NOT NULL COMMENT '文件位置',\
  `timestamp` int(10) NOT NULL COMMENT '添加时间',\
  `log_date` smallint(5) NOT NULL,\
  `log_type` varchar(16) NOT NULL COMMENT 'lookup',\
  `log_time` int(10) NOT NULL,\
  `ip` bigint(20) NOT NULL,\
  `mac` bigint(20) NOT NULL,\
  `thread_id` smallint(5) NOT NULL,\
  `channel_id` int(10) NOT NULL,\
  `trans_id` int(10) NOT NULL,\
  `req_type` tinyint(3) NOT NULL,\
  `func_id` smallint(5) NOT NULL,\
  `func_name` varchar(32) NOT NULL COMMENT 'lookup',\
  `branch_id` int(10) NOT NULL,\
  `branch_name` varchar(64) NOT NULL COMMENT 'lookup',\
  `XT_GTLB` char(2) NOT NULL,\
  `CA_KHH` varchar(64) NOT NULL,\
  `CA_KHMC` varchar(64) NOT NULL,\
  `CA_VER` varchar(64) NOT NULL,\
  `CA_AQJB` varchar(64) NOT NULL,\
  `CA_TXMM` varchar(64) NOT NULL,\
  `CA_ISVIPHOST` varchar(64) NOT NULL,\
  `CA_JQTZM` varchar(64) NOT NULL,\
  `CA_SLOTSN` varchar(64) NOT NULL,\
  `CA_CID` varchar(64) NOT NULL,\
  `CA_CERTREQ` varchar(64) NOT NULL,\
  `CA_USERCERDN` varchar(64) NOT NULL,\
  `CA_ZSQSRQ` varchar(64) NOT NULL,\
  `CA_ZSJZRQ` varchar(64) NOT NULL,\
  `CA_CERTSN` varchar(64) NOT NULL,\
  `CA_CERTINFO` varchar(64) NOT NULL,\
  `CA_MACHINENAME` varchar(64) NOT NULL,\
  `CA_DLSJ` varchar(64) NOT NULL,\
  `CA_LASTIP` varchar(64) NOT NULL,\
  `CA_MAC` varchar(64) NOT NULL,\
  `CA_CSCS` varchar(64) NOT NULL,\
  `CA_RESV` varchar(64) NOT NULL,\
  `reserve_a` varchar(512) NOT NULL,\
  `reserve_b` varchar(512) NOT NULL,\
  `reserve_c` varchar(512) NOT NULL\
  ) ENGINE=%s DEFAULT CHARSET=utf8;\
  \
  CREATE TABLE IF NOT EXISTS %st_tc50_sc_ans%s (\
  `parser` varchar(16) NOT NULL COMMENT 'lookup',\
  `collector` varchar(32) NOT NULL COMMENT 'lookup',\
  `file_id` int(10) NOT NULL COMMENT '文件编号',\
  `file_pos_start` bigint(20) NOT NULL COMMENT '文件位置',\
  `log_len` int(10) NOT NULL COMMENT '文件位置',\
  `timestamp` int(10) NOT NULL COMMENT '添加时间',\
  `log_date` smallint(5) NOT NULL,\
  `log_type` varchar(16) NOT NULL COMMENT 'lookup',\
  `log_time` int(10) NOT NULL,\
  `ip` bigint(20) NOT NULL,\
  `mac` bigint(20) NOT NULL,\
  `thread_id` smallint(5) NOT NULL,\
  `channel_id` int(10) NOT NULL,\
  `trans_id` int(10) NOT NULL,\
  `req_type` tinyint(3) NOT NULL,\
  `func_id` smallint(5) NOT NULL,\
  `func_name` varchar(32) NOT NULL COMMENT 'lookup',\
  `branch_id` int(10) NOT NULL,\
  `branch_name` varchar(64) NOT NULL COMMENT 'lookup',\
  `timeA` int(10) NOT NULL,\
  `timeB` int(10) NOT NULL,\
  `queue` int(10) NOT NULL,\
  `return_no` int(10) NOT NULL,\
  `return_msg` varchar(512) NOT NULL,\
  `reserve_a` varchar(512) NOT NULL,\
  `reserve_b` varchar(512) NOT NULL,\
  `reserve_c` varchar(512) NOT NULL,\
  `reserve_d` varchar(512) NOT NULL,\
  `reserve_e` varchar(512) NOT NULL,\
  `reserve_f` varchar(512) NOT NULL\
  ) ENGINE=%s DEFAULT CHARSET=utf8;\
  \
  CREATE TABLE IF NOT EXISTS %st_tc50_log_failed%s (\
  `parser` varchar(16) NOT NULL,\
  `collector` varchar(32) NOT NULL,\
  `file_id` int(10) NOT NULL,\
  `file_pos_start` bigint(20) NOT NULL,\
  `log_len` int(10) NOT NULL,\
  `timestamp` int(10) NOT NULL,\
  `log_date` smallint(5) NOT NULL\
  ) ENGINE=%s DEFAULT CHARSET=utf8;\
  \
  CREATE TABLE IF NOT EXISTS %st_tc50_log_info%s (\
  `parser` varchar(16) NOT NULL COMMENT 'lookup',\
  `collector` varchar(32) NOT NULL COMMENT 'lookup',\
  `file_id` int(10) NOT NULL COMMENT '文件编号',\
  `file_pos_start` bigint(20) NOT NULL COMMENT '文件位置',\
  `log_len` int(10) NOT NULL COMMENT '文件位置',\
  `timestamp` int(10) NOT NULL COMMENT '添加时间',\
  `log_date` smallint(5) NOT NULL COMMENT '解析者',\
  `log_type` varchar(16) NOT NULL COMMENT 'lookup',\
  `log_time` int(10) NOT NULL,\
  `info` varchar(64) NOT NULL,\
  `channel_id` int(10) NOT NULL,\
  `ip` bigint(20) NOT NULL,\
  `op_organization` varchar(64) NOT NULL,\
  `op_account` varchar(32) NOT NULL,\
  `reason` varchar(64) NOT NULL\
  ) ENGINE=%s DEFAULT CHARSET=utf8;\
";

static const uint32_t kMAX_FUNC_ID        = 64 * 1024;
static const uint32_t kLOG_UNFLAG         = 0;
static const uint32_t kLOG_FLAG           = 1;

static const uint32_t kConnInfoFuncID     = 0;
static const uint32_t kSystemInfoFuncID   = kMAX_FUNC_ID - 1;

//////////////////////////////////////////////////////////////////////////
class TC50Log {
public:
  /*
  typedef enum enumPACKECTRET {
    PRET_UNKNOW		    =	-1,				// 未知请求头
    PRET_SYSBASE	    =	200,			// 系统信息
    PRET_SYS_INFO	    =	PRET_SYSBASE+ 1,	// 系统信息
    PRET_SYS_ERROR	  =	PRET_SYSBASE+ 2,	// 系统错误
    PRET_CONNBASE	    =	800,			// 连接信息
    PRET_CONN_CON	    =	PRET_CONNBASE+ 1,	// 连接建立
    PRET_CONN_DIS	    =	PRET_CONNBASE+ 2,	// 连接断开
    PRET_BUSBASE	    =	1000,			// 业务信息
    PRET_REQ		      =	PRET_BUSBASE+ 12,	// 功能请求 9 为请求头节点数目
    PRET_ANS		      =	PRET_BUSBASE+ 15,	// 功能应答 12 为应答包头节点数目
    PRET_SUCESS		    =	PRET_BUSBASE+ 100,	// 成功处理
    PRET_FAILD		    =	PRET_BUSBASE+ 101,	// 调用失败
    PRET_POLERR		    =	PRET_BUSBASE+ 102,	// 协议异常
    PRET_DEAERR		    =	PRET_BUSBASE+ 103,	// 处理异常

    PRET_SKIP         = 2000,
    PRET_ERROR        = 99999
  } PACKECTRET_e;
  */

  typedef enum enumPACKECTRET {
    PRET_CONN_CON
    , PRET_CONN_DIS

    , PRET_REQ
    , PRET_SUCESS

    , PRET_SYS_INFO
    , PRET_SYS_ERROR

    , PRET_FAILD
    , PRET_POLERR
    , PRET_DEAERR

    , PRET_UNKNOW     = 2000
    , PRET_SKIP
  } PACKECTRET_e;

  typedef enum enumDICT {
    DICT_SIMPLE
    , DICT_COMMON
    , DICT_SCNTR
    , DICT_RZ
    , DICT_UNKNOW
  } DICT_e;

public:
  // we parse only one msg
  rc_t Parse(const char_t*, uint32_t*, TC50Dict*, IFileIteratorBase*, const char_t* strCollector, const char_t* strDir);
  void FixData();

public:
  bool_t bNormal;
  //
  PACKECTRET_e eLogType;

  DICT_e eDictType;

  // time
  uint32_t nDate;

  uint32_t nHour;
  uint32_t nMin;
  uint32_t nSec;
  uint32_t nMilSec;

  uint32_t nTime;

  // 功能请求
  char_t strLogType[16];

  // IP:
  char_t strIP[64];
  // MAC:
  char_t strMAC[32];
  // 线程:
  char_t strThreadID[32];
  // 通道ID:
  uint32_t nChannelID;
  //事务ID:
  uint32_t nTransID;

  //请求:(
  uint32_t nReqType;
  uint32_t nFuncID;
  char_t strFuncName[32];

  // 营业部:
  uint32_t nBranchID;
  //char_t strBranchID[16];
  char_t strBranchName[64];

  // 耗时A
  uint32_t nTimeA;
  // 耗时B
  uint32_t nTimeB;
  // 排队
  uint32_t nQueue;

  // req
  int32_t nReturnNO;

  // info
  union {
    char_t strSysInfo[512];
    char_t strSysError[512];
    char_t strReturnMsg[512];
    //char_t strDisconnect[512];
    //char_t strExcept[512];
  };


  //
  char_t strOP_Organization[64];
  char_t strOP_Account[32];
  char_t strReason[64];

  char_t strKHH[16];
  char_t strZJZH[16];

  uint32_t nZHLB;

  uint32_t nXT_CLITYPE;
  char_t strXT_CLIVER[8];

  char_t strGDDM[16];

  char_t strKHMC[16];
  char_t strSHGD[16];
  char_t strSZGD[16];
  char_t strXT_GTLB[2];

  uint32_t nXT_VIPFLAG;
  
  char_t strXT_MACHINEINFO[1024];

  char_t strWTBH[32];

  uint32_t nWTFS;
  uint32_t nOP_WTFS;

  char_t strZQDM[16];
  uint32_t nMMBZ;
  uint32_t nJYDW;
  //float_t nWTJG;
  char_t strWTJG[16];
  uint32_t nWTSL;
  uint32_t nWTRQ;
  uint32_t nWTSJ;

  // ca
  char_t strCA_KHMC[64];
  char_t strCA_KHH[64];
  char_t strCA_VER[64];
  char_t strCA_AQJB[64];
  char_t strCA_TXMM[64];
  char_t strCA_ISVIPHOST[64];
  char_t strCA_JQTZM[64];
  char_t strCA_SLOTSN[64];
  char_t strCA_CID[64];
  char_t strCA_CERTREQ[64];
  char_t strCA_USERCERDN[64];
  char_t strCA_ZSQSRQ[64];
  char_t strCA_ZSJZRQ[64];
  char_t strCA_CERTSN[64];
  char_t strCA_CERTINFO[64];
  char_t strCA_MACHINENAME[64];
  char_t strCA_DLSJ[64];
  char_t strCA_LASTIP[64];
  char_t strCA_MAC[64];
  char_t strCA_CSCS[64];
  char_t strCA_RESV[64];

  // add
  uint32_t nXT_CHECKRISKFLAG;
  char_t strXT_CHECKRISKFLAG[16];

  char_t strRETINFO[512];

  char_t strZJYE[16];
  uint32_t nZQSL;
  char_t strZQSL[16];
  uint32_t nKMSL;
  char_t strKMSL[16];

  char_t strReserve_a[512];
  char_t strReserve_b[512];
  char_t strReserve_c[512];
  char_t strReserve_d[512];
  char_t strReserve_e[512];
  char_t strReserve_f[512];

public:
  void SetFuncRule(const uint8_t*);
  uint32_t AllCount() { return nCountInfo + nCountError + nCountConnect + nCountDisConnect
     + nCountReq + nCountAns + nCountExcept;
  }
  void ResetCount();

  uint32_t nCountInfo;
  uint32_t nCountError;
  uint32_t nCountConnect;
  uint32_t nCountDisConnect;
  uint32_t nCountReq;
  uint32_t nCountAns;
  uint32_t nCountExcept;
  uint32_t nCountUnknow;

public:
  TC50Log(const uint8_t*);
  ~TC50Log();

private:
  void reset();
  void setLogType();
  rc_t setDict(TC50Dict* pTC50Dict, IFileIteratorBase* pIFileIterator
    , const char_t* strCollector, const char_t* strDir);

  void fixTime();
  uint64_t toTime();

private:
  AutoRelease<IParseIX*>      m_autoRelIParseIX;

private:
  bool_t didMathLogRule(uint32_t nFuncID) {
    ASSERT(m_pFuncRule);
    if (nFuncID < kMAX_FUNC_ID) { return kLOG_FLAG == m_pFuncRule[nFuncID] ? TRUE : FALSE; }
    return FALSE;
  }
  const uint8_t*              m_pFuncRule;

private:  
  DISALLOW_COPY_AND_ASSIGN(TC50Log);
}; // TC50Log

END_NAMESPACE_AGGREGATOR
#endif // TC50_LOG_H_

