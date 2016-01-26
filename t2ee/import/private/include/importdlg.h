// importDlg.h : header file
//

#pragma once
#include "afxcmn.h"

#include "object.h"
USING_NAMESPACE;

#include "hash_tables.h"
#include "std_list"
#include "lock.h"
#include "thread.h"
#include "waitable_event.h"
#include "atomic_count.h"
#include "file_util.h"
#include "memory_pool.h"
USING_NAMESPACE_BASE;

#include "database.h"
USING_NAMESPACE_DATABASE;

#include "tc50_log.h"
#include "tc50_dict.h"
#include "afxwin.h"
USING_NAMESPACE_AGGREGATOR;

template<> inline
size_t stdext::hash_value< CString >(const CString & s) {
  return stdext::hash_value((LPCTSTR)s);
}

// CimportDlg dialog
class CimportDlg : public CDialog, public Thread
{
// Construction
public:
	CimportDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_IMPORT_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();

  afx_msg void OnBnClickedOk();
  afx_msg void OnBnClickedCancel();
  afx_msg void OnBnClickedButtonLog();
  afx_msg void OnBnClickedButtonDict();
  afx_msg void OnBnClickedButtonData();
  afx_msg void OnBnClickedButtonLoad();
  afx_msg void OnBnClickedButtonStop();
  afx_msg void OnBnClickedButtonCt();
  afx_msg void OnBnClickedButtonRefresh();
  afx_msg void OnBnClickedButtonSavecfg();

	DECLARE_MESSAGE_MAP()

private:
  CString getOpenFilePath();
  CString getOpenPath(const CString&);

private:
  CString m_csDB;
  CString m_csDict;
  CString m_csPath;
  CString m_csLog;
  DWORD m_dwThreadNum;
  DWORD m_dwTableName;
  CString m_csLogFlag;
  CListCtrl m_lcLogFile;
  CString m_csStaticPath;
  CString m_csSep;
  CString m_csLogFilter;
  CString m_csDBEng;
  BOOL m_bDBRemote;
  BOOL m_bDelTempFile;
  CString m_csTimeRun;
  CString m_csFilterTime;
  DWORD m_dwNetRate;

  CString m_csLogInclude;
  CString m_csLogExclude;

  CButton m_cbImport;

private:
  BOOL checkSystem();
  BOOL checkSystemFileSystem();
  BOOL checkSystemDataBase();

  void startImport();
  void stopImport();

  void loadFileQueue();

public:
  enum { kLogTypeNum = 8 };

private:
  ThreadGroup<CimportDlg>     m_thdgrpParser;

  void ThreadMain(void*);

  bool_t parseLog(AutoRelease<IConnection*>&, AutoReleaseMemoryBase&, AutoReleaseMemoryBase*, TC50Log&, uint32_t idx);
  bool_t parseLogProc(AutoReleaseFile*, AutoReleaseMemoryBase*, uint32_t* BufPos, TC50Log*, const uint8_t*, uint32_t*, file_size_t);
  rc_t writeLog(AutoReleaseFile*, AutoReleaseMemoryBase*, uint32_t* BufPos, file_size_t, uint32_t, const char_t*, const TC50Log*);

  typedef int32_t (*PFN_WRITE_LOG)(char_t*, uint32_t, file_size_t, uint32_t, const char_t*, const TC50Log*);


  PFN_WRITE_LOG m_pfn_wl[kLogTypeNum];

  
  static int32_t writeLog_trade_req(char_t*, uint32_t, file_size_t, uint32_t, const char_t*, const TC50Log*);

  static int32_t writeLog_logon_req(char_t*, uint32_t, file_size_t, uint32_t, const char_t*, const TC50Log*);
  
  static int32_t writeLog_sc_req(char_t*, uint32_t, file_size_t, uint32_t, const char_t*, const TC50Log*);

  static int32_t writeLog_ans(char_t*, uint32_t, file_size_t, uint32_t, const char_t*, const TC50Log*);

  static int32_t writeLog_info(char_t*, uint32_t, file_size_t, uint32_t, const char_t*, const TC50Log*);
  static int32_t writeLog_failed(char_t*, uint32_t, file_size_t, uint32_t, const char_t*, const TC50Log*);

private:
  struct FileNode {
    CString     name;
    uint32_t    order;
    file_size_t size;

    void Copy(const FileNode& other) {
      name    = other.name;
      order   = other.order;
      size    = other.size;
    }
  };
  std::queue<FileNode>    m_queueFile;
  Lock                    m_lockFileQueue;

  bool_t addListItem(const FileNode&);

  uint32_t                m_nMaxFileSize;
private:
  volatile bool_t         m_bStart;
  //WaitableEvent           m_evStop;
  /*m_evStop.Signal();*/atomic_count            m_acThreaNum;

private:
  TC50Dict                m_dictTC50;

  void parseLogFilter(const char_t*, uint32_t);
  uint8_t                 m_arrLogRule[kMAX_FUNC_ID];

  void SetLogRule(uint32_t nFuncID, uint32_t v) {
    if (nFuncID >= kMAX_FUNC_ID) { return; }
    m_arrLogRule[nFuncID] = (uint8_t)v;
  }

  bool_t didMathLogRule(uint32_t nFuncID) {
    if (nFuncID < kMAX_FUNC_ID) { return kLOG_FLAG == m_arrLogRule[nFuncID] ? TRUE : FALSE; }
    return FALSE;
  }

  // config
private:
  void initConfig();
  AutoReleaseFile         m_autoRelFileProcess;
  Lock                    m_lockFileProcess;
  hash_set<CString>       m_setFileProcess;

  bool_t loadConfigFile();
  bool_t saveConfigFile();

  AutoReleaseFile         m_autoRelFileConfig;

private:
  uint32_t                m_nEnableTimeStart;
  uint32_t                m_nEnableTimeEnd;

private:
  static uint32_t ipToInt(const char_t*);
  static uint64_t macToInt(const char_t*);  
};

#define STR_VERSION           "v1.0.0.1031"
