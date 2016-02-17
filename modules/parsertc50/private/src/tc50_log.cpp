/*


*/

#include "tc50_log.h"

#include "code_util.h"
#include "time_util.h"

BEGIN_NAMESPACE_AGGREGATOR

USING_BASE(DayTime);
USING_BASE(ToBase16_WT);

//////////////////////////////////////////////////////////////////////////
static const char_t* kStrEnd         = _STR("\r\n");
static const uint32_t kStrEndLen     = 2 * sizeof(char_t);

static const char_t kSepCharTC50     = _CHAR('|');
static const uint32_t knLogTypeCount = 9;
const char_t* kStrLogType[] = {
  "连接建立"
  , "连接断开"

  , "功能请求"
  , "成功处理"

  , "系统信息"
  , "系统错误"

  , "调用失败"
  , "协议异常"
  , "处理异常"
};

void TC50Log::setLogType() {

  for (size_t idx = 0; idx < knLogTypeCount; ++idx) {

    if (0 == STRCMP(strLogType, kStrLogType[idx])) {
      eLogType = (PACKECTRET_e)idx;
      break;
    }
  }
}


uint64_t TC50Log::toTime() {

  return DayTime::to_localtime(nDate / 10000
    , (nDate / 100) % 100
    , nDate % 100
    , nHour
    , nMin
    , nSec
    );
}

rc_t TC50Log::setDict(TC50Dict* pTC50Dict, IFileIteratorBase* pIFileIterator
                 , const char_t* strCollector, const char_t* strDir) {

  ASSERT(pTC50Dict);
  ASSERT(pIFileIterator);
  ASSERT(strCollector);
  ASSERT(strDir);

  uint64_t timeLog = toTime();
  IDictIX* pIDictIX;

  if (1 == nReqType) {
    pIDictIX = pTC50Dict->FindNearDict_Scntr(pIFileIterator, strCollector, strDir, timeLog);
    if (NULL == pIDictIX || FALSE == pIDictIX->isValid()) { 
      pIDictIX = pTC50Dict->GetDefDict_Scntr();
    }
    eDictType = DICT_SCNTR;
  }
  else {
    // Simple
    if (TDX_ID_FUNC_REQ_ZHYCL   == nFuncID 
      || TDX_ID_FUNC_REQ_DLYCL  == nFuncID 
      || TDX_ID_FUNC_REQ_JCKHJY == nFuncID 
      || TDX_ID_FUNC_REQ_KHJY   == nFuncID
      ) {
        // 
        pIDictIX = pTC50Dict->FindNearDict_Simple(pIFileIterator, strCollector, strDir, timeLog);
        if (NULL == pIDictIX || FALSE == pIDictIX->isValid()) { 
          pIDictIX = pTC50Dict->GetDefDict_Simple();
        }
        eDictType = DICT_SIMPLE;
    }
    // common
    else {
      // 
      pIDictIX = pTC50Dict->FindNearDict_Common(pIFileIterator, strCollector, strDir, timeLog);
      if (NULL == pIDictIX || FALSE == pIDictIX->isValid()) { 
        pIDictIX = pTC50Dict->GetDefDict_Common();
      }
      eDictType = DICT_COMMON;
    }
  }

  if (NULL == pIDictIX || FALSE == pIDictIX->isValid() ) { return RC_S_NULL_VALUE; }
  return m_autoRelIParseIX->SetDict(pIDictIX);
}

//////////////////////////////////////////////////////////////////////////
const size_t knLogHeadLen = 13;
const size_t knMinLogLen = 86;
const uint32_t kScanfLenLogType = 22;
//09:05:06.1 功能请求  IP:2.8.0.1 MAC:D 线程:C 通道ID:1 事务ID:5 请求:(0-1)F 营业部:(1)F

//////////////////////////////////////////////////////////////////////////
#if 0
static bool_t IsGBKCode(const char_t* src, size_t len) {

  ASSERT(src && len);
  uint8_t ch1 = *src;

  if (ch1 >= 0x81 && ch1 <= 0xFE) {

    if (len < 2) { return TRUE; }

    uint8_t ch2 = *(src + 1);
    return (ch2 >= 0x40 && ch2 <= 0xFE) ? TRUE : FALSE;
  }

  return FALSE;
}
#endif // 0

static void strncpy_utf8(char *dest, size_t size, const char *source, size_t slen, size_t len) {

  size_t count = min(size, len);
  uint8_t ch1;

  while (count && slen && 0 != (ch1 = *dest/*++*/ = *source++)) {   /* copy string */

    if (ch1 >= 0x81 && ch1 <= 0xFE) {

      if (slen < 2) { break; } // unknow hz
      uint8_t ch2 = *(source);
      if (ch2 >= 0x40 && ch2 <= 0xFE) {
        // hz
        *(++dest) = *source++;
        ++dest; count -= 3; slen -= 2;
      } else { break; } // unknow hz
    } else {
      ++dest; --count; --slen;
    }
  }

  if (count) { *(++dest) = '\0'; }
}

static void truncate_utf8(char *dest, size_t size, size_t len) {

  size_t count = min(size, len);
  uint8_t ch1;

  while (count && 0 != (ch1 = *dest)) {

    if (ch1 >= 0x81 && ch1 <= 0xFE) {

      if (size < 2) { break; } // unknow hz
      uint8_t ch2 = *(dest + 1);
      if (ch2 >= 0x40 && ch2 <= 0xFE) {

        // hz
        dest += 2;
        count -= 3;
        size -= 2;
      } else { break; } // unknow hz
    } else {

      // for csv 
      if ('\r' == ch1 || '\n' == ch1 || '\t' == ch1 || '|' == ch1 || '"' == ch1 || '\'' == ch1) { *dest = '*'; }

      ++dest; --count; --size;
    }
    /*
    int value = (*dest) & 0xFF;
    if (0xA0 < value) {
    if (1 == cntHZ) {
    // back one 
    if (1 == count) { --dest; break; }
    --count; cntHZ = 0;
    }
    else { cntHZ = 1; }
    }
    else {
    ASSERT(0 == cntHZ);
    // back one 
    if (1 == cntHZ) { --dest; break; }
    }

    ++dest; --count;
    */
  }

  *dest = '\0';
}

static const char_t* isTC50Log(const char_t* strLog, size_t len) {
  
  if (len < knMinLogLen) { return NULL; }

  uint32_t nHour;
  uint32_t nMin;
  uint32_t nSec;
  uint32_t nMilSec;
  char_t strLogType[32];

  for (size_t idx = 0; idx < knLogTypeCount; ++idx) {

    const char_t* strFindHead = STRSTR(strLog, len, kStrLogType[idx]);

    if (NULL == strFindHead) { continue; }
    // save this log
    //ASSERT((strFindHead - data) >= knLogHeadLen);
    if (strFindHead - strLog < knLogHeadLen) { continue; }

    const char_t* strNewLogHead = strFindHead - knLogHeadLen;
    if (len - (strNewLogHead - strLog) < kScanfLenLogType) { continue; }

    const uint32_t kScanfCount = 5;
    int nScaned = SSCANF(strNewLogHead, len - (strNewLogHead - strLog)
      , "%d:%d:%d.%d %s "
      , &nHour, &nMin, &nSec, &nMilSec
      , strLogType, sizeof(strLogType)
      );

    if (kScanfCount != nScaned) { continue; }
    return strNewLogHead;
  }
  return NULL;
}

static const char_t* findFirstNumber(const char_t* strLog, size_t len) {
  
  const char_t* strNumber = strLog;
  for (size_t idx = 0; idx < len; ++idx, ++strNumber) {
    if (*(strNumber) >= '0' && *(strNumber) <= '9') { return strNumber; }
  }
  return NULL;
}

rc_t TC50Log::Parse(const char_t* data, uint32_t* len, TC50Dict* pTC50Dict, IFileIteratorBase* pIFileIterator
                    , const char_t* strCollector, const char_t* strDir) {

  if (NULL == data || NULL == len || (*len) == 0
    || NULL == pTC50Dict || NULL == pIFileIterator || NULL == strCollector || NULL == strDir) { return RC_S_NULL_VALUE; }

  uint32_t nLogLen = (*len);
  const char_t* strLog = data;
  const char_t* strPackBegin;
  const char_t* strPackEnd = strLog;

  reset();

  const char_t* strFindEnd = NULL;

  // scanf time and log_type
  {
restart:
    // goto next char
    while ((0x00 == (*strLog) ||  0x20 == (*strLog) || '\r' == (*strLog) || '\n' == (*strLog))) {

      bNormal = FALSE;
      ++strLog;

      uint32_t nFreeLogSize = (uint32_t)(strLog - data);

      // just over
      if (nFreeLogSize == nLogLen) { eLogType = PRET_SKIP; (*len) = nLogLen; return RC_S_OK; }
    }    

    strFindEnd = STRSTR(strLog, (*len) - (strLog - data), kStrEnd);
    if (NULL == strFindEnd) { return RC_S_FAILED; } // not enough data
    strFindEnd += kStrEndLen;
    
    strLog = findFirstNumber(strLog, strFindEnd - strLog);
    if (NULL == strLog) { eLogType = PRET_SKIP; (*len) = (uint32_t)(strFindEnd - data); return RC_S_OK; }

    const uint32_t kScanfCount = 5;
    int nScaned = 0;
    if ((*len) - (strLog - data) > kScanfLenLogType) { 

      nScaned = SSCANF(strLog, kScanfLenLogType
        , "%d:%d:%d.%d %s "
        , &nHour, &nMin, &nSec, &nMilSec
        , strLogType, sizeof(strLogType)
      );
    }

    if (kScanfCount != nScaned) {

      // find |
      const char_t strTooLong[] = "(**内容太长被省略**)\r\n";
      const uint32_t nStrTooLongLen = sizeof(strTooLong) - 1;
      if (kSepCharTC50 == (*(strFindEnd - 1 - kStrEndLen))
        || 0 == STRNCMP(strTooLong, strLog, nStrTooLongLen)
      ) {
        eLogType = PRET_SKIP;
      }
      else {
        // this line is unknow
        goto log_save;
      }

      // save packet data
      strPackEnd = strFindEnd;
      ++nCountUnknow;
    }
    else {

      fixTime();
      // calc time
      nTime = nHour * 10000000 + nMin * 100000 + nSec * 1000 + nMilSec;

      setLogType();
      const uint32_t kLogTypeLen = 8;
      strLog = STRSTR(strLog, (*len) - (strLog - data), strLogType) + kLogTypeLen + sizeof(char_t); // ' '

      if (PRET_UNKNOW == eLogType) {
        // 
        const char_t* strNewLog = isTC50Log(strLog, (*len) - (strLog - data));
        if (strNewLog) {
          strLog = strNewLog; goto restart; }
      }

      //
    }
  }

  switch (eLogType) {

    // 功能请求    
    case PRET_REQ: {

      // 
      //uint32_t nBranchID = 0;
      const uint32_t kScanfCount = 10;
      int nScaned = SSCANF(strLog, (*len) - (strLog - data)
        , "IP:%[0-9.] MAC:%[0-9A-Fa-f] 线程:%[0-9A-Fa-f] 通道ID:%d 事务ID:%d 请求:(%d-%d)%s 营业部:(%d)%s"
        , strIP, sizeof(strIP)
        , strMAC, sizeof(strMAC)
        , strThreadID, sizeof(strThreadID)
        , &nChannelID, &nTransID, &nReqType, &nFuncID
        , strFuncName, sizeof(strFuncName)
        //, strBranchID, sizeof(strBranchID)
        , &nBranchID
        , strBranchName, sizeof(strBranchName)
        );

      if (nScaned != kScanfCount) {

        // skip branchID
        nBranchID = 0;
        nScaned = SSCANF(strLog, (*len) - (strLog - data)
          , "IP:%[0-9.] MAC:%[0-9A-Fa-f] 线程:%[0-9A-Fa-f] 通道ID:%d 事务ID:%d 请求:(%d-%d)%s 营业部:%s"
          , strIP, sizeof(strIP)
          , strMAC, sizeof(strMAC)
          , strThreadID, sizeof(strThreadID)
          , &nChannelID, &nTransID, &nReqType, &nFuncID
          , strFuncName, sizeof(strFuncName)
          //, strBranchID, sizeof(strBranchID)
          , strBranchName, sizeof(strBranchName)
          );

        if (nScaned != kScanfCount - 1) {
          goto log_save;
        }
      }

      //SNPRINTF(strBranchID, sizeof(strBranchID), sizeof(strBranchID), "%04d", nBranchID);
      
      // goto req data
      ASSERT(strFindEnd);
      strPackBegin = strFindEnd;
      strFindEnd = STRSTR(strPackBegin, (*len) - (strPackBegin - data), kStrEnd);
      if (NULL == strFindEnd) {
        //(*len) = (uint32_t)(strPackBegin - data);
        //pTC50Log->eLogType = PRET_UNKNOW;
        // not engont data
        return RC_S_NOTFOUND;
      }

      if (kSepCharTC50 != *(strFindEnd - 1)) {
        // data error.
        strPackEnd = strPackBegin;
      }
      else {
        strPackEnd = strFindEnd + kStrEndLen;
      }

      // dict
      if (TRUE == didMathLogRule(nFuncID)
        && RC_S_OK == setDict(pTC50Dict, pIFileIterator, strCollector, strDir)
      ) {
        

        // save
        char_t* strDataEnd = (char_t*)strPackBegin + (uint32_t)(strFindEnd + kStrEndLen - strPackBegin);
        ASSERT(strDataEnd);
        char_t chEnd = *strDataEnd; *strDataEnd = 0x00;

        if (RC_S_OK == m_autoRelIParseIX->Parse((uint16_t)nFuncID, strPackBegin, (uint32_t)(strFindEnd + kStrEndLen - strPackBegin))) {

#define GET_STR_DATA(x) m_autoRelIParseIX->get_data(TDX_ID_ ## x, str ## x, sizeof(str ## x))
#define GET_STR_DATA_EX(x, y) m_autoRelIParseIX->get_data(TDX_ID_ ## y, str ## x, sizeof(str ## x))
        /*
#define GET_STR_DATA(x) {\
          m_autoRelIParseIX->get_data(TDX_ID_ ## x, str ## x, sizeof(str ## x));\
          if (m_strFill && m_nStrFillLen && 0 == STRLEN(str ## x)) {\
            STRNCPY(str ## x, sizeof(str ## x), m_strFill, m_nStrFillLen);\
          }\
        }\
*/

#define GET_INT_DATA(x) m_autoRelIParseIX->get_data(TDX_ID_ ## x, &n ## x)
#define GET_INT_DATA_EX(x, y) m_autoRelIParseIX->get_data(TDX_ID_ ## y, &n ## x)

        if (DICT_COMMON == eDictType || DICT_SIMPLE == eDictType) {

          if (TDX_ID_FUNC_REQ_ZHYCL == nFuncID || TDX_ID_FUNC_REQ_DLYCL == nFuncID || TDX_ID_FUNC_REQ_JCKHJY == nFuncID) {
            // 
            GET_STR_DATA_EX(KHH, XT_LOGINID);
          }
          else {
            // KHH 
            GET_STR_DATA(KHH);
          }

          GET_STR_DATA(ZJZH);
          GET_INT_DATA(ZHLB);

          GET_STR_DATA(GDDM);

          if (RC_S_OK != GET_STR_DATA(KHMC)) {
            GET_STR_DATA_EX(KHMC, SFFS);
          }

          GET_STR_DATA(SHGD);
          GET_STR_DATA(SZGD);
          GET_STR_DATA(XT_GTLB);

          //GET_STR_DATA(XT_CLITYPE);
          GET_INT_DATA(XT_CLITYPE);
          GET_STR_DATA(XT_CLIVER);

          GET_INT_DATA(XT_VIPFLAG);
          GET_STR_DATA(XT_MACHINEINFO);

          GET_STR_DATA(WTBH);

          if (RC_S_OK != GET_INT_DATA(WTFS)) {
            GET_INT_DATA_EX(WTFS, SHFS);
          }

          GET_INT_DATA(OP_WTFS);

          if (RC_S_OK != GET_STR_DATA(ZQDM)) { 
            if (RC_S_OK != GET_STR_DATA_EX(ZQDM, CPDM)) {
              GET_STR_DATA_EX(ZQDM, KFSJJ_JJDM);
            }
          }

          if (RC_S_OK != GET_INT_DATA(MMBZ)) {
            if (RC_S_OK != GET_INT_DATA_EX(MMBZ, ETFMMBS)) {
              if (RC_S_OK != GET_INT_DATA_EX(MMBZ, KFSJJ_MMBZ)) {
                GET_INT_DATA_EX(MMBZ, YZZZ_ZZFX);
              }
            }
          }

          GET_INT_DATA(JYDW);

          if (RC_S_OK != GET_STR_DATA(WTJG)) {

            if (RC_S_OK != GET_STR_DATA_EX(WTJG, KFSJJ_JYJE)) {
              GET_STR_DATA_EX(WTJG, YZZZ_ZZJE);
            }
          }

          if (RC_S_OK != GET_INT_DATA(WTSL)) {
            if (RC_S_OK != GET_INT_DATA_EX(WTSL, ETFRGSL)) {
              GET_INT_DATA_EX(WTSL, KFSJJ_JJFE);
            }
          }

          GET_INT_DATA(WTRQ);
          GET_INT_DATA(WTSJ);

          //
          GET_INT_DATA(XT_CHECKRISKFLAG);

          //
          if (RC_S_OK != GET_STR_DATA_EX(Reserve_a, ETFDM)) {

            if (RC_S_OK != GET_STR_DATA_EX(Reserve_a, KFSJJ_JJGSDM)) {
              GET_STR_DATA_EX(Reserve_a, YZZZ_YHDM);
            }
          }

          if (RC_S_OK != GET_STR_DATA_EX(Reserve_b, KFSJJ_JJZH)) {
            GET_STR_DATA_EX(Reserve_b, YZZZ_YHZH);
          }

          GET_STR_DATA_EX(Reserve_c, KFSJJ_ZHDM);

          if (TDX_ID_FUNC_REQ_ZHYCL == nFuncID || TDX_ID_FUNC_REQ_DLYCL == nFuncID || TDX_ID_FUNC_REQ_JCKHJY == nFuncID || TDX_ID_FUNC_REQ_KHJY == nFuncID) {
            GET_STR_DATA_EX(Reserve_a, XT_AUTH_MODE);
          }
        }
        else if (DICT_SCNTR == eDictType) {

          GET_STR_DATA(CA_KHMC);
          GET_STR_DATA(CA_KHH);
          GET_STR_DATA(CA_VER);
          GET_STR_DATA(CA_AQJB);
          GET_STR_DATA(CA_TXMM);
          GET_STR_DATA(CA_ISVIPHOST);
          GET_STR_DATA(CA_JQTZM);
          GET_STR_DATA(CA_SLOTSN);
          GET_STR_DATA(CA_CID);
          GET_STR_DATA(CA_CERTREQ);
          GET_STR_DATA(CA_USERCERDN);
          GET_STR_DATA(CA_ZSQSRQ);
          GET_STR_DATA(CA_ZSJZRQ);
          GET_STR_DATA(CA_CERTSN);
          GET_STR_DATA(CA_CERTINFO);
          GET_STR_DATA(CA_MACHINENAME);
          GET_STR_DATA(CA_DLSJ);
          GET_STR_DATA(CA_LASTIP);
          GET_STR_DATA(CA_MAC);
          GET_STR_DATA(CA_CSCS);
          GET_STR_DATA(CA_RESV);
        }
        }

        // restore
        ASSERT(strDataEnd);
        *strDataEnd = chEnd;
      }

      ++nCountReq;
      break;
    }
    // 应答
    case PRET_SUCESS:
    case PRET_FAILD:
    case PRET_POLERR:
    case PRET_DEAERR: {
    //else if (TRUE == m_regexAns.isMatchOnly(strLog)) {

      //uint32_t nBranchID = 0;
      const uint32_t kScanfCount = 13;
      int nScaned = SSCANF(strLog, (*len) - (strLog - data)
        , "IP:%[0-9.] MAC:%[0-9A-Fa-f] 线程:%[0-9A-Fa-f] 通道ID:%d 事务ID:%d 请求:(%d-%d)%s 营业部:(%d)%s 耗时A:%d 耗时B:%d 排队:%d"
        , strIP, sizeof(strIP)
        , strMAC, sizeof(strMAC)
        , strThreadID, sizeof(strThreadID)
        , &nChannelID, &nTransID, &nReqType, &nFuncID
        , strFuncName, sizeof(strFuncName)
        //, strBranchID, sizeof(strBranchID)
        , &nBranchID
        , strBranchName, sizeof(strBranchName)
        , &nTimeA, &nTimeB, &nQueue
        );

      // goto req data
      if (nScaned != kScanfCount) {

        // skip branchID
        nBranchID = 0;
        nScaned = SSCANF(strLog, (*len) - (strLog - data)
          , "IP:%[0-9.] MAC:%[0-9A-Fa-f] 线程:%[0-9A-Fa-f] 通道ID:%d 事务ID:%d 请求:(%d-%d)%s 营业部:%s 耗时A:%d 耗时B:%d 排队:%d"
          , strIP, sizeof(strIP)
          , strMAC, sizeof(strMAC)
          , strThreadID, sizeof(strThreadID)
          , &nChannelID, &nTransID, &nReqType, &nFuncID
          , strFuncName, sizeof(strFuncName)
          //, strBranchID, sizeof(strBranchID)
          , strBranchName, sizeof(strBranchName)
          , &nTimeA, &nTimeB, &nQueue
          );

        // goto req data
        if (nScaned < 9) {
          goto log_save;
        }
 
      }

      ASSERT(strFindEnd);
      strPackBegin = strFindEnd;

      if (TRUE == didMathLogRule(nFuncID)
        && RC_S_OK == setDict(pTC50Dict, pIFileIterator, strCollector, strDir)
        ) {

        // 
        ASSERT(m_autoRelIParseIX);
        ASSERT(TRUE == m_autoRelIParseIX->isValid());

        if (PRET_POLERR == eLogType || PRET_DEAERR == eLogType) {

          if (STRNCMP("(**", strPackBegin, 3)) { (*len) = (uint32_t)(strPackBegin - data); return RC_S_OK; }
          strFindEnd = STRSTR(strPackBegin, (*len) - (strPackBegin - data), kStrEnd);
          if (NULL == strFindEnd) { (*len) = (uint32_t)(strPackBegin - data); return RC_S_FAILED; }
          strPackEnd = strFindEnd + kStrEndLen;

          ++nCountExcept;
          break;
        }

        bool_t bRealResponseData = FALSE;
        // skip nexe line
        strPackEnd = strPackBegin;
        while ((uint32_t)(strPackEnd - data) < (*len)
          && NULL != (strFindEnd = STRSTR(strPackEnd, (*len) - (strPackEnd - data), kStrEnd))) {

            if (kSepCharTC50 != (*(strFindEnd - 1))) { break; }
            strPackEnd = strFindEnd + kStrEndLen;
            bRealResponseData = TRUE;
        }

        strFindEnd = TRUE == bRealResponseData ? strPackEnd : strFindEnd;

        const size_t kMAX_RETURN_MSG_LEN = 500;
#ifdef USE_WTCOM_PARSER_ANS
        if (strPackBegin != strPackEnd) {

          // save
          char_t* strDataEnd = (char_t*)strPackBegin + (uint32_t)(strPackEnd - strPackBegin);
          ASSERT(strDataEnd);
          char_t chEnd = *strDataEnd; *strDataEnd = 0x00;

          if (RC_S_OK == m_autoRelIParseIX->Parse((uint16_t)(nFuncID + 1), strPackBegin, (uint32_t)(strPackEnd - strPackBegin))) {

            m_autoRelIParseIX->GetReturnNo(&nReturnNO);
            //nReturnNO = ATOI(strPackBegin);
            if (0 != nReturnNO) {

              m_autoRelIParseIX->GetErrmsg(strReturnMsg, sizeof(strReturnMsg));
              truncate_utf8(strReturnMsg, sizeof(strReturnMsg) - 1, kMAX_RETURN_MSG_LEN);
            }
            else {

              if (TDX_ID_FUNC_REQ_JCKHJY == nFuncID) {

                char_t strCookies[512] = {0};
                if (RC_S_OK == m_autoRelIParseIX->GetCookies(strCookies, sizeof(strCookies))) {
                  //
                  const uint32_t kCookiesNamePos = 2;
                  const char_t* strFindName = strCookies;
                  size_t nCookiesLen = STRNLEN(strCookies, sizeof(strCookies));
                  size_t nCookiesNameLen = nCookiesLen;
                  uint32_t nFindCount = 0;
                  while (NULL != (strFindName = STRCHR(strFindName, nCookiesNameLen, ','))) {

                    ++strFindName;
                    nCookiesNameLen = nCookiesLen - (strFindName - strCookies);

                    if (kCookiesNamePos == ++nFindCount) {

                      // find next , or |
                      const char_t* strFindNameEnd = STRCHR(strFindName, nCookiesLen, ',');
                      if (strFindNameEnd) {
                        strncpy_utf8(strRETINFO, sizeof(strRETINFO)
                          , strFindName, strFindNameEnd - strFindName, kMAX_RETURN_MSG_LEN);
                      }
                      else {
                        strFindNameEnd = STRCHR(strFindName, nCookiesLen, kSepCharTC50);
                        if (strFindNameEnd) {
                          strncpy_utf8(strRETINFO, sizeof(strRETINFO)
                            , strFindName, strFindNameEnd - strFindName, kMAX_RETURN_MSG_LEN);
                        }
                      }
                      break;
                    }
                  } // while
                }
              }
              else {
                if (m_autoRelIParseIX->GetLineCount() && RC_S_OK == m_autoRelIParseIX->Next()) {

                  GET_STR_DATA(WTBH);

                  GET_INT_DATA(XT_CHECKRISKFLAG);
                  GET_STR_DATA(RETINFO);

                  GET_STR_DATA(ZJYE);
                  GET_INT_DATA(ZQSL);
                  GET_INT_DATA(KMSL);
                }
              }
            } //
          }

          // restore
          ASSERT(strDataEnd);
          *strDataEnd = chEnd;
        }

#else
        {
          nReturnNO = ATOI(strPackBegin);
          if (0 == nReturnNO) {
            ++nCountAns;

            // next line
            const char_t* strFindData = STRSTR(strPackBegin, strPackEnd - strPackBegin, kStrEnd);
            if (NULL == strFindData) { break; }

            uint16_t nPos = 0;

            // TDX_ID_WTBH
            if (RC_S_OK == m_autoRelIParseIX->HaveFuncID((uint16_t)(nFuncID + 1), TDX_ID_WTBH, &nPos)) {

              const char_t* strFindSepEnd = strFindData;
              uint16_t pos_idx;
              for (pos_idx = 0; pos_idx < nPos; ++pos_idx) {
                strFindSepEnd = STRCHR(strFindSepEnd, strPackEnd - strFindSepEnd, kSepCharTC50);
                if (NULL == strFindSepEnd) { break; }
                ++strFindSepEnd;
              }

              if (strFindSepEnd && pos_idx == nPos) { // fix number
                uint32_t nWTBH = ATOI(strFindSepEnd);
                ITOA(nWTBH, strWTBH, sizeof(strWTBH), 10);
              }
            }

            // TDX_ID_XT_CHECKRISKFLAG
            if (RC_S_OK == m_autoRelIParseIX->HaveFuncID((uint16_t)(nFuncID + 1), TDX_ID_XT_CHECKRISKFLAG, &nPos)) {

              const char_t* strFindSepEnd = strFindData;
              uint16_t pos_idx;
              for (pos_idx = 0; pos_idx < nPos; ++pos_idx) {
                strFindSepEnd = STRCHR(strFindSepEnd, strPackEnd - strFindSepEnd, kSepCharTC50);
                if (NULL == strFindSepEnd) { break; }
                ++strFindSepEnd;
              }

              if (strFindSepEnd && pos_idx == nPos) { // fix number
                nXT_CHECKRISKFLAG = ATOI(strFindSepEnd);
              }
            }

            // TDX_ID_RETINFO
            if (RC_S_OK == m_autoRelIParseIX->HaveFuncID((uint16_t)(nFuncID + 1), TDX_ID_RETINFO, &nPos)) {

              const char_t* strFindSepEnd = strFindData;
              uint16_t pos_idx;
              for (pos_idx = 0; pos_idx < nPos; ++pos_idx) {
                strFindSepEnd = STRCHR(strFindSepEnd, strPackEnd - strFindSepEnd, kSepCharTC50);
                if (NULL == strFindSepEnd) { break; }
                ++strFindSepEnd;
              }
              if (strFindSepEnd && pos_idx == nPos) {

                size_t nCopySize;
                const char_t* strFindSep = STRCHR(strFindSepEnd, strPackEnd - strFindSepEnd, kSepCharTC50);
                nCopySize = strFindSep ? strFindSep - strFindSepEnd : strPackEnd - strFindSepEnd;
                if (nCopySize) {
                  if (nCopySize > kMAX_RETURN_MSG_LEN) { nCopySize = kMAX_RETURN_MSG_LEN; }
                  strncpy_utf8(strRETINFO, sizeof(strRETINFO), strFindSepEnd, nCopySize, kMAX_RETURN_MSG_LEN);
                }
              }
            }
            /*
              if (strFindData)
              // next line
              const char_t* strFindData = STRSTR(strPackBegin, (*len) - (strPackEnd - data), kStrEnd);
              if (strFindData) {
                strFindData += kStrEndLen;
                const char_t* strFindSepEnd = STRCHR(strFindData, (*len) - (strFindData - data), kSepCharTC50);
                //if (strFindSepEnd) { STRNCPY(strWTBH, sizeof(strWTBH), strFindData, strFindSepEnd - strFindData); }
                if (strFindSepEnd) {
                  uint32_t nWTBH = ATOI(strFindData); ITOA(nWTBH, strWTBH, sizeof(strWTBH), 10);
                }
              }
            }
            */
            break;
          }

          // return msg
          {
            const char_t* strFindSep = STRCHR(strPackBegin, (*len) - (strPackBegin - data), kSepCharTC50);
            const char_t* strFindSepEnd;
            if (strFindSep) {

              strFindSep++;
              strFindSepEnd = STRCHR(strFindSep, (*len) - (strFindSep - data), kSepCharTC50);
              if (strFindSepEnd) {

                // 
                size_t nCopySize = strFindSepEnd - strFindSep;
                if (nCopySize > kMAX_RETURN_MSG_LEN) { nCopySize = kMAX_RETURN_MSG_LEN; }
                strncpy_utf8(strReturnMsg, sizeof(strReturnMsg), strFindSep, nCopySize, kMAX_RETURN_MSG_LEN);
              }
            }
          }
        }
#endif // USE_WTCOM_PARSER_ANS
      }
      else {
        strPackEnd = strPackBegin;
      }
      // save packet data
      ++nCountAns;
      break;
    }

    //系统信息
    case PRET_SYS_INFO: {
      ASSERT(strFindEnd);
      strPackEnd = strFindEnd;
      if (FALSE == didMathLogRule(kSystemInfoFuncID)) { break; }

      STRNCPY(strSysInfo, sizeof(strSysInfo), strLog, (strPackEnd - strLog) - kStrEndLen);
      // save packet data
      ++nCountInfo;
      break;
    }

    // 系统错误
    case PRET_SYS_ERROR: {

      ASSERT(strFindEnd);
      strPackEnd = strFindEnd;
      if (FALSE == didMathLogRule(kSystemInfoFuncID)) { break; }

      STRNCPY(strSysError, sizeof(strSysError), strLog, (strPackEnd - strLog) - kStrEndLen);
      // save packet data      
      ++nCountError;
      break;
    }

    // 连接建立
    case PRET_CONN_CON: {

      ASSERT(strFindEnd);
      strPackEnd = strFindEnd;
      if (FALSE == didMathLogRule(kConnInfoFuncID)) { break; }

      const uint32_t kScanfCount = 2;
      int nScaned = SSCANF(strLog, (*len) - (strLog - data)
        , "通道ID:%d 连接信息:%[0-9.]\r\n"
        , &nChannelID
        , strIP, sizeof(strIP)
        );

      if (nScaned < kScanfCount) { goto log_save; }

      // save packet data
      ++nCountConnect;
      break;
    }

    // 连接断开
    case PRET_CONN_DIS: {

      ASSERT(strFindEnd);
      strPackEnd = strFindEnd;
      if (FALSE == didMathLogRule(kConnInfoFuncID)) { break; }

      const uint32_t kScanfCount = 5;
      int nScaned = SSCANF(strLog, (*len) - (strLog - data)
        , "通道ID:%d 连接信息:%[0-9.] 操作机构:%s 操作帐号:%s 原因:%s"
        , &nChannelID
        , strIP, sizeof(strIP)
        , strOP_Organization, sizeof(strOP_Organization)
        , strOP_Account, sizeof(strOP_Account)
        , strReason, sizeof(strReason)
        );

      if (kScanfCount != nScaned) {
        strOP_Organization[0] = 0;
        strOP_Account[0] = 0;
        /*int nScaned = */SSCANF(strLog, (*len) - (strLog - data)
          , "通道ID:%d 连接信息:%[0-9.] 操作机构: 操作帐号: 原因:%s"
          , &nChannelID
          , strIP, sizeof(strIP)
          , strReason, sizeof(strReason)
          );
      }

      ++nCountDisConnect;
      break;
    }//

    // only one line
    default: {

      ASSERT(strFindEnd);
      strPackEnd = strFindEnd;
    }

  } // swtich

  // save packet data
  (*len) = (uint32_t)(strPackEnd - data);
  return RC_S_OK;

log_save:
  bNormal = FALSE;

  const char_t* strNewLog = isTC50Log(strLog, (*len) - (strLog - data));
  if (strNewLog) {
    strLog = strNewLog; goto restart; }

  (*len) = (uint32_t)(strFindEnd - data);
  eLogType = PRET_UNKNOW;
  return RC_S_OK;

}

//////////////////////////////////////////////////////////////////////////
TC50Log::TC50Log(const uint8_t* pFuncRule) 
  : m_autoRelIParseIX(IParseIX::CreateInstance(kStrParseWtComm, NULL))

  // count
  , nCountInfo(0)
  , nCountError(0)
  , nCountConnect(0)
  , nCountDisConnect(0)
  , nCountReq(0)
  , nCountAns(0)
  , nCountExcept(0)
  , nCountUnknow(0)

  , m_pFuncRule(pFuncRule)
{
  reset();
}

TC50Log::~TC50Log() {}

void TC50Log::SetFuncRule(const uint8_t* pFuncRule) { m_pFuncRule = pFuncRule; }

//////////////////////////////////////////////////////////////////////////
void TC50Log::reset() {

  bNormal = TRUE;

  nHour = nMin = nSec = nMilSec = 0;

#define BZERO_STR(x) BZERO_ARR(str##x);

  BZERO_STR(LogType);
  BZERO_STR(IP);
  BZERO_STR(MAC);
  BZERO_STR(ThreadID);

  nChannelID = nTransID = nReqType = nFuncID = 0;

  BZERO_STR(FuncName);

  nBranchID = 0;
  //BZERO_STR(BranchID);
  BZERO_STR(BranchName);

  nTimeA = nTimeB = nQueue = nReturnNO = 0;

  //
  BZERO_STR(ReturnMsg);

  BZERO_STR(OP_Organization);
  BZERO_STR(OP_Account);
  BZERO_STR(Reason);

  BZERO_STR(KHH);
  BZERO_STR(ZJZH);

  nZHLB = 0;

  nXT_CLITYPE = 0;
  //BZERO_STR(XT_CLITYPE);
  BZERO_STR(XT_CLIVER);

  nXT_VIPFLAG = 0;

  BZERO_STR(GDDM);
  BZERO_STR(KHMC);
  BZERO_STR(SHGD);
  BZERO_STR(SZGD);
  BZERO_STR(XT_GTLB);

  BZERO_STR(XT_MACHINEINFO);
  BZERO_STR(WTBH);

  nWTFS = 0;
  nOP_WTFS = 0;
  BZERO_STR(ZQDM);

  nMMBZ = 0;
  //nWTJG = 0;
  BZERO_STR(WTJG);

  nJYDW = 0;
  nWTSL = 0;
  nWTRQ = 0;
  nWTSJ = 0;

  BZERO_STR(CA_KHMC);
  BZERO_STR(CA_KHH);
  BZERO_STR(CA_VER);
  BZERO_STR(CA_AQJB);
  BZERO_STR(CA_TXMM);
  BZERO_STR(CA_ISVIPHOST);
  BZERO_STR(CA_JQTZM);
  BZERO_STR(CA_SLOTSN);
  BZERO_STR(CA_CID);
  BZERO_STR(CA_CERTREQ);
  BZERO_STR(CA_USERCERDN);
  BZERO_STR(CA_ZSQSRQ);
  BZERO_STR(CA_ZSJZRQ);
  BZERO_STR(CA_CERTSN);
  BZERO_STR(CA_CERTINFO);
  BZERO_STR(CA_MACHINENAME);
  BZERO_STR(CA_DLSJ);
  BZERO_STR(CA_LASTIP);
  BZERO_STR(CA_MAC);
  BZERO_STR(CA_CSCS);
  BZERO_STR(CA_RESV);

  // 
  nXT_CHECKRISKFLAG = 0;
  BZERO_STR(RETINFO);

  BZERO_STR(ZJYE);
  nZQSL = 0;
  nKMSL = 0;

  BZERO_STR(ZQSL);
  BZERO_STR(KMSL);
  BZERO_STR(XT_CHECKRISKFLAG);

  BZERO_STR(Reserve_a);
  BZERO_STR(Reserve_b);
  BZERO_STR(Reserve_c);
  BZERO_STR(Reserve_d);
  BZERO_STR(Reserve_e);
  BZERO_STR(Reserve_f);

  eLogType = PRET_UNKNOW;
  eDictType = DICT_UNKNOW;
}

void TC50Log::ResetCount() {

  nCountInfo = 0;
  nCountError = 0;
  nCountConnect = 0;
  nCountDisConnect = 0;
  nCountReq = 0;
  nCountAns = 0;
  nCountExcept = 0;
  nCountUnknow = 0;
}

void TC50Log::FixData() {

  // IP
  char_t* strFind = STRCHR(strIP, STRLEN(strIP), '.');
  char_t* strFindLast = NULL;
  while (strFind) { strFindLast = strFind + 1; strFind = STRCHR(strFindLast, STRLEN(strFindLast), '.'); }

  if (strFindLast && 3 < STRLEN(strFindLast)) {
    *(strFindLast + 3) = 0;
  }

  // MAC
  strMAC[12] = 0;
  strThreadID[16] = 0;

//#define FIX_STR(x) x[sizeof(x) - 1] = 0;

#define FIX_STR(x) truncate_utf8(str##x, sizeof(str##x) - 1, sizeof(str##x))

  FIX_STR(FuncName);
  FIX_STR(BranchName);
  FIX_STR(ReturnMsg);

  FIX_STR(OP_Organization);
  FIX_STR(OP_Account);
  FIX_STR(Reason);

  FIX_STR(KHH);
  FIX_STR(ZJZH);

  FIX_STR(XT_CLIVER);

  FIX_STR(GDDM);

  FIX_STR(KHMC);
  FIX_STR(SHGD);
  FIX_STR(SZGD);
  FIX_STR(XT_GTLB);

  FIX_STR(XT_MACHINEINFO);

  FIX_STR(WTBH);

  FIX_STR(ZQDM);

  FIX_STR(WTJG);

  FIX_STR(CA_KHMC);
  FIX_STR(CA_KHMC);
  FIX_STR(CA_KHH);
  FIX_STR(CA_VER);
  FIX_STR(CA_AQJB);
  FIX_STR(CA_TXMM);
  FIX_STR(CA_ISVIPHOST);
  FIX_STR(CA_JQTZM);
  FIX_STR(CA_SLOTSN);
  FIX_STR(CA_CID);
  FIX_STR(CA_CERTREQ);
  FIX_STR(CA_USERCERDN);
  FIX_STR(CA_ZSQSRQ);
  FIX_STR(CA_ZSJZRQ);
  FIX_STR(CA_CERTSN);
  FIX_STR(CA_CERTINFO);
  FIX_STR(CA_MACHINENAME);
  FIX_STR(CA_DLSJ);
  FIX_STR(CA_LASTIP);
  FIX_STR(CA_MAC);
  FIX_STR(CA_CSCS);
  FIX_STR(CA_RESV);

  // 
  FIX_STR(RETINFO);
  strXT_CHECKRISKFLAG[0] = 0x00;
  ITOA(nXT_CHECKRISKFLAG, strXT_CHECKRISKFLAG, sizeof(strXT_CHECKRISKFLAG), 10);

  FIX_STR(ZJYE);
  strZQSL[0] = 0x00;
  ITOA(nZQSL, strZQSL, sizeof(strZQSL), 10);
  strKMSL[0] = 0x00;
  ITOA(nKMSL, strKMSL, sizeof(strKMSL), 10);

  FIX_STR(Reserve_a);
  FIX_STR(Reserve_b);
  FIX_STR(Reserve_c);
  FIX_STR(Reserve_d);
  FIX_STR(Reserve_e);
  FIX_STR(Reserve_f);
}

void TC50Log::fixTime() {

  if (nHour > 23) { nHour = (nHour % 100) % 23; }
  if (nMin > 59)  { nMin  = (nMin % 100) % 59; }
  if (nSec > 59)  { nSec  = (nSec % 100) % 59; }
  if (nMilSec > 999)  { nMilSec =(nMilSec % 1000) %  999; }
}

//////////////////////////////////////////////////////////////////////////
END_NAMESPACE_AGGREGATOR
