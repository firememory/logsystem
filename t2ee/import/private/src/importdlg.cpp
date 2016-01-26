// importDlg.cpp : implementation file
//

#include "stdafx.h"
#include "import.h"
#include "importDlg.h"
#include "regex.hpp"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////////
// CimportDlg dialog
const uint32_t kMaxThreadNum  = 16;
const char_t* kStrThreadName  = _STR("Import");
const uint32_t knColumnCount  = 4;
const uint32_t kMinThreadMemorySize = 1 * 1024 * 1024;
const uint32_t kWriteFileMemorySize = 1 * 1024 * 1024;
const uint32_t kPerMSize            = 1 * 1024 * 1024;

const char_t* kStrProcessFileName     = _STR("process.txt");
const char_t* kStrConfigFileName      = _STR("import.xml");

static const char_t* kStrEnd         = _STR("\r\n");
static const uint32_t kStrEndLen     = 2 * sizeof(char_t);

const char_t* kStrSepFields   = "|";
const char_t* kStrParser      = "ParserTC50";
#define kStrCollector         "import"

const char_t* kStrErrorDBCheckTable     = _T("表校验失败，");
const char_t* kStrNoErrorDBCreateTable  = _T("建表成功！");
const char_t* kStrErrorDBCreateTable    = _T("建表失败，检查数据库配置！");
const char_t* kStrErrorDBConfig         = _T("数据库连接失败，检查数据库配置！");
const char_t* kStrErrorDBInstance       = _T("数据库加载失败， 检查libmysql.dll！");

const char_t* kStrErrorFileQueue        = _T("日志队列为空！");
const char_t* kStrErrorTC50Dict         = _T("TC50字典文件载入失败！");

const char_t* kStrErrorTempPath         = _T("临时目录不可用！");

const char_t* kStrErrorTimeConfig       = _T("运行时间配置错误！");

const char_t* kStrErrorLoadConfigFile   = _T("载入配置文件失败！");
const char_t* kStrErrorWriteConfigFile  = _T("写入配置文件失败！");


const char_t* kStrErrorThreadRunning    = _T("解析线程运行中！");
//////////////////////////////////////////////////////////////////////////

CimportDlg::CimportDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CimportDlg::IDD, pParent)
  , m_csDB(_T("<Server Driver=\"MYSQLCI\" Source=\"127.0.0.1,3306\" DBName=\"tls_his\" DBUsr=\"root\" DBPwd=\"\" dbcharset=\"gbk\" Compress=\"yes\" />"))
  , m_csDict(_T(""))
  , m_csPath(_T(""))
  , m_csLog(_T(""))
  , m_dwThreadNum(1)
  , m_dwTableName(13)
  , m_csLogFlag(_T("1"))
  , m_lcLogFile()
  , m_csStaticPath(_T(""))
  , m_csSep(_T("|"))
  , m_csLogFilter(_T("^20[0-9]{6}\\.log$"))
  , m_csDBEng(_T("BDE"))
  , m_bDBRemote(FALSE)
  , m_bDelTempFile(TRUE)
  , m_csTimeRun(_T("170000-50000"))
  , m_csFilterTime(_T(""))
  , m_dwNetRate(0)
  , m_csLogInclude(_T(""))
  , m_csLogExclude(_T("0,65534"))

  //////////////////////////////////////////////////////////////////////////
  , Thread(kStrThreadName)
  , m_thdgrpParser()
  , m_queueFile()
  , m_lockFileQueue()
  , m_nMaxFileSize(0)
  , m_bStart(FALSE)
  //, m_evStop(FALSE, FALSE)
  , m_acThreaNum(0)
  

  , m_autoRelFileProcess(NULL)
  , m_setFileProcess()

  , m_autoRelFileConfig(NULL)
  , m_nEnableTimeStart(170000)
  , m_nEnableTimeEnd(50000)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CimportDlg::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);
  DDX_Text(pDX, IDC_EDIT_DB, m_csDB);
  DDX_Text(pDX, IDC_EDIT_DICT, m_csDict);
  DDX_Text(pDX, IDC_EDIT_DATA, m_csPath);
  DDX_Text(pDX, IDC_EDIT_LOG, m_csLog);
  DDX_Text(pDX, IDC_EDIT_TN, m_dwThreadNum);
  DDV_MinMaxUInt(pDX, m_dwThreadNum, 1, kMaxThreadNum);
  DDX_Text(pDX, IDC_EDIT_TNB, m_dwTableName);
  DDV_MinMaxUInt(pDX, m_dwTableName, 0, 99);
  DDX_Text(pDX, IDC_EDIT_LOGFALG, m_csLogFlag);
  DDX_Control(pDX, IDC_LIST_FILE, m_lcLogFile);
  DDX_Text(pDX, IDC_STATIC_PATH, m_csStaticPath);
  DDX_Text(pDX, IDC_EDIT_SEP, m_csSep);
  DDX_Text(pDX, IDC_EDIT_FILTER, m_csLogFilter);
  DDX_Text(pDX, IDC_EDIT_DBENG, m_csDBEng);
  DDX_Check(pDX, IDC_CHECK_DBLOC, m_bDBRemote);
  DDX_Check(pDX, IDC_CHECK_DELTEMPFILE, m_bDelTempFile);
  DDX_Text(pDX, IDC_EDIT_TIMERUN, m_csTimeRun);
  DDX_Text(pDX, IDC_EDIT_FILTER_TIME, m_csFilterTime);
  DDX_Text(pDX, IDC_EDIT_NETRATE, m_dwNetRate);
  DDX_Control(pDX, IDC_BUTTON_LOAD, m_cbImport);
}

BEGIN_MESSAGE_MAP(CimportDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
  ON_BN_CLICKED(IDOK, &CimportDlg::OnBnClickedOk)
  ON_BN_CLICKED(IDCANCEL, &CimportDlg::OnBnClickedCancel)
  ON_BN_CLICKED(IDC_BUTTON_LOG, &CimportDlg::OnBnClickedButtonLog)
  ON_BN_CLICKED(IDC_BUTTON_DICT, &CimportDlg::OnBnClickedButtonDict)
  ON_BN_CLICKED(IDC_BUTTON_DATA, &CimportDlg::OnBnClickedButtonData)
  ON_BN_CLICKED(IDC_BUTTON_LOAD, &CimportDlg::OnBnClickedButtonLoad)
  ON_BN_CLICKED(IDC_BUTTON_STOP, &CimportDlg::OnBnClickedButtonStop)
  ON_BN_CLICKED(IDC_BUTTON_CT, &CimportDlg::OnBnClickedButtonCt)
  ON_BN_CLICKED(IDC_BUTTON_REFRESH, &CimportDlg::OnBnClickedButtonRefresh)
  ON_BN_CLICKED(IDC_BUTTON_SAVECFG, &CimportDlg::OnBnClickedButtonSavecfg)
END_MESSAGE_MAP()


// CimportDlg message handlers
typedef enum LISTCTRL_e_ {
  LC_NO
  , LC_STATUS
  , LC_NAME
  , LC_SIZE

  , LC_PROCESS_SIZE
  , LC_PROCESS_LOGCOUNT
  , LC_PROCESS_TIME
} LISTCTRL_e;

BOOL CimportDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
  m_lcLogFile.SetExtendedStyle(LVS_EX_GRIDLINES|LVS_EX_FULLROWSELECT);

  const DWORD dwColLen = 40;
  const DWORD dwDoubleColLen = 80;
  m_lcLogFile.InsertColumn(LC_NO, "序号", LVCFMT_LEFT, dwColLen);
  m_lcLogFile.InsertColumn(LC_STATUS, "状态", LVCFMT_LEFT, dwColLen);
  m_lcLogFile.InsertColumn(LC_NAME, "文件", LVCFMT_LEFT, dwDoubleColLen);
  m_lcLogFile.InsertColumn(LC_SIZE, "大小", LVCFMT_LEFT, dwDoubleColLen);

  m_lcLogFile.InsertColumn(LC_PROCESS_SIZE, "位置", LVCFMT_LEFT, dwDoubleColLen);
  m_lcLogFile.InsertColumn(LC_PROCESS_LOGCOUNT, "数量", LVCFMT_LEFT, dwDoubleColLen);
  m_lcLogFile.InsertColumn(LC_PROCESS_TIME, "耗时(ms)", LVCFMT_LEFT, dwDoubleColLen);

  MEMSET(m_arrLogRule, kLOG_FLAG, sizeof(m_arrLogRule));

  m_pfn_wl[0] = writeLog_trade_req;
  m_pfn_wl[1] = writeLog_ans;

  m_pfn_wl[2] = writeLog_logon_req;
  m_pfn_wl[3] = writeLog_ans;

  m_pfn_wl[4] = writeLog_sc_req;
  m_pfn_wl[5] = writeLog_ans;

  m_pfn_wl[6] = writeLog_info;
  m_pfn_wl[7] = writeLog_failed;

  SetWindowText(kStrCollector " " STR_VERSION);

  // config
  initConfig();
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CimportDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CimportDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CimportDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

CString CimportDlg::getOpenFilePath() {

  CFileDialog dlg(TRUE,//TRUE是创建打开文件对话框，FALSE则创建的是保存文件对话框
    _T(".sto"),//默认的打开文件的类型
    NULL,//默认打开的文件名
    OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,//打开只读文件
    _T("(*.sto)|*.sto|"));//所有可以打开的文件类型

  if(dlg.DoModal() !=IDOK) { return ""; }
  return dlg.GetPathName();////////取出文件路径  
}

CString CimportDlg::getOpenPath(const CString& csTitle) {

  BROWSEINFO bi;

  char Buffer[MAX_PATH] = "";

  //初始化入口参数bi开始
  bi.hwndOwner = NULL;
  bi.pidlRoot =NULL;//初始化制定的root目录很不容易
  bi.pszDisplayName = Buffer;//此参数如为NULL则不能显示对话框
  bi.lpszTitle = csTitle;
  bi.ulFlags = BIF_EDITBOX;//带编辑框的风格
  bi.lpfn = NULL;
  bi.lParam = 0;
  bi.iImage=IDR_MAINFRAME;
  //初始化入口参数bi结束
  LPITEMIDLIST pIDList = SHBrowseForFolder(&bi);//调用显示选择对话框
  if(pIDList) {

    SHGetPathFromIDList(pIDList, Buffer);
    //取得文件夹路径到Buffer里
    //CString m_cSisDes = Buffer;//将路径保存在一个CString对象里
  }

  // free memory used
  IMalloc * imalloc = 0;
  if (SUCCEEDED(SHGetMalloc(&imalloc))) {
    imalloc->Free (pIDList);
    imalloc->Release();
  }

  return CString(Buffer);
}

void CimportDlg::OnBnClickedOk()
{
  // TODO: 在此添加控件通知处理程序代码
  //OnOK();
}

void CimportDlg::OnBnClickedCancel()
{
  // TODO: 在此添加控件通知处理程序代码
  if (m_acThreaNum) { MessageBox(kStrErrorThreadRunning); return; }
  if (FALSE == m_bStart) { OnCancel(); }
}

bool_t CimportDlg::addListItem(const FileNode& fileNode) {

  //
  bool_t bRet = TRUE;
  CString strText;
  strText.Format(_T("%u"), fileNode.order);

  // Insert the item, select every other item.
  m_lcLogFile.InsertItem(
    LVIF_TEXT|LVIF_STATE, fileNode.order, strText, 
    //(i%2)==0 ? LVIS_SELECTED : 0, LVIS_SELECTED,
    0,LVIS_SELECTED,
    0, 0);

  bool_t bSkipFilter = TRUE;
  if (m_csFilterTime.GetLength()) {
    uint32_t nFileTimeStart = 0;
    uint32_t nFileTimeEnd = 0;
    
    const uint32_t kScanfCount = 2;
    int nScaned = SSCANF(m_csFilterTime, m_csFilterTime.GetLength()
      , "%u-%u"
      , &nFileTimeStart
      , &nFileTimeEnd
      );

    if (kScanfCount == nScaned && nFileTimeStart && nFileTimeEnd) {

      bSkipFilter = FALSE;

      uint32_t nDate = ATOI(fileNode.name);

      CString csMatch(m_csLog + "\\" + fileNode.name);
      if (nDate < nFileTimeStart || nDate > nFileTimeEnd) {
        m_lcLogFile.SetItemText(fileNode.order, LC_STATUS, _T("skip by time"));
        bRet = FALSE;
      }
      else {
        m_lcLogFile.SetItemText(fileNode.order, LC_STATUS, _T("wait by time"));
      }
    }
  }

  if (TRUE == bSkipFilter) {

    //const char_t* kStrStatus = _T("wait");
    CString csMatch(m_csLog + "\\" + fileNode.name);
    if (m_setFileProcess.end() != m_setFileProcess.find(csMatch)) {
      m_lcLogFile.SetItemText(fileNode.order, LC_STATUS, _T("skip by processed"));
      bRet = FALSE;
    }
    else {
      m_lcLogFile.SetItemText(fileNode.order, LC_STATUS, _T("wait"));
    }
  }

  char_t szFileSize[64] = {0};
  I64TOA(fileNode.size, szFileSize, sizeof(szFileSize), 10);

  m_lcLogFile.SetItemText(fileNode.order, LC_NAME, fileNode.name);
  m_lcLogFile.SetItemText(fileNode.order, LC_SIZE, szFileSize);

  m_lcLogFile.SetItemText(fileNode.order, LC_PROCESS_SIZE, _T("0"));
  m_lcLogFile.SetItemText(fileNode.order, LC_PROCESS_LOGCOUNT, _T("0"));
  m_lcLogFile.SetItemText(fileNode.order, LC_PROCESS_TIME, _T("0"));

  return bRet;

  /*
  // Insert 10 items in the list view control.
  for (int i=0;i < 30;i++)
  {
    strText.Format(_T("item %d"), i);

    // Insert the item, select every other item.
    m_lcLogFile.InsertItem(
      LVIF_TEXT|LVIF_STATE, i, strText, 
      //(i%2)==0 ? LVIS_SELECTED : 0, LVIS_SELECTED,
      0,LVIS_SELECTED,
      0, 0);

    // Initialize the text of the subitems.
    for (int j=1;j < nColumnCount;j++)
    {
      strText.Format(_T("sub-item %d %d"), i, j);
      m_lcLogFile.SetItemText(i, j, strText);
    }
  }
  */
}

void CimportDlg::OnBnClickedButtonLog()
{
  // TODO: 在此添加控件通知处理程序代码
  if (TRUE == m_bStart) { return; }

  UpdateData(TRUE);
  CString csLog = getOpenPath(_T("日志路径"));
  int nLen = csLog.GetLength();
  if (0 == nLen) { return; }
  if ('\\' == csLog[nLen - 1]) { m_csLog = CString(csLog, nLen - 1); }
  else { m_csLog = csLog; }

  loadFileQueue();
}

void CimportDlg::loadFileQueue()
{
  // clear
  // push
  AutoLock autoLock(m_lockFileQueue);
  while (!m_queueFile.empty()) { m_queueFile.pop(); }

  m_lcLogFile.DeleteAllItems();
  m_nMaxFileSize = 1;


  RegEx regexFileName(0, m_csLogFilter);

  CString strText;
  uint32_t nOrder = 0;

  WIN32_FIND_DATA find_file_data;
  FileNode fileNode;
  CString csDir(m_csLog + "\\*");
  //__try 
  {

    HANDLE find_handle = FindFirstFile(csDir, &find_file_data);

    if (find_handle != INVALID_HANDLE_VALUE) {
      do {
        // Don't count current or parent directories.
        if ((STRCMP(find_file_data.cFileName, _STR("..")) == 0) ||
          (STRCMP(find_file_data.cFileName, _STR(".")) == 0))
          continue;

        if (find_file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) { continue; }
        /*
        long result = CompareFileTime(&find_file_data.ftCreationTime,  // NOLINT
        &comparison_filetime);
        // File was created after or on comparison time
        if ((result == 1) || (result == 0))
        ++file_count;
        */

        if (FALSE == regexFileName.didCompile()
          || FALSE == regexFileName.isMatchOnly(find_file_data.cFileName)
          ) { continue; }

        LARGE_INTEGER size;
        size.HighPart   = find_file_data.nFileSizeHigh;
        size.LowPart    = find_file_data.nFileSizeLow;

        if (0 == size.QuadPart) { continue; }

        if (size.QuadPart > kPerMSize * m_nMaxFileSize) { m_nMaxFileSize = (uint32_t)(size.QuadPart / kPerMSize); }

        // is we need file
        fileNode.name   = find_file_data.cFileName;
        fileNode.order  = nOrder;
        fileNode.size = size.QuadPart;
        ++nOrder;

        // add list
        if (TRUE == addListItem(fileNode)) {
          // push
          m_queueFile.push(fileNode);
        }

      } while (FindNextFile(find_handle,  &find_file_data));

      FindClose(find_handle);
    }
  }
  //    __except(EXCEPTION_EXECUTE_HANDLER) {
  // 
  //     return TRUE;
  //   }

  m_csStaticPath.Format(_T("转换目录可用空间大于 线程数 * 最大文件 = %u *%u(M)"), m_dwThreadNum, m_nMaxFileSize);

  if (0 == m_csDict.GetLength()) { m_csDict = m_csLog + "\\default.sto"; }
  if (0 == m_csPath.GetLength()) { m_csPath = m_csLog + "\\o"; }
  UpdateData(FALSE);
}

void CimportDlg::OnBnClickedButtonRefresh()
{
  // TODO: 在此添加控件通知处理程序代码
  if (TRUE == m_bStart) { return; }
  UpdateData(TRUE);
  loadFileQueue();
}


void CimportDlg::OnBnClickedButtonSavecfg()
{
  // TODO: 在此添加控件通知处理程序代码
  UpdateData(TRUE);

  size_t nRead = SSCANF(m_csTimeRun, m_csTimeRun.GetLength()
    , "%u-%u"
    , &m_nEnableTimeStart
    , &m_nEnableTimeEnd
    );

  if (2 != nRead || m_nEnableTimeStart == m_nEnableTimeEnd
    || m_nEnableTimeStart >= 240000
    || m_nEnableTimeEnd >= 240000
    ) { MessageBox(kStrErrorTimeConfig); return; }

  if (FALSE == saveConfigFile()) { MessageBox(kStrErrorWriteConfigFile);  return; }
}


void CimportDlg::OnBnClickedButtonDict()
{
  // TODO: 在此添加控件通知处理程序代码
  m_csDict = getOpenFilePath();
  UpdateData(FALSE);
}

void CimportDlg::OnBnClickedButtonData()
{
  // TODO: 在此添加控件通知处理程序代码
  CString csPath = getOpenPath(_T("转换目录"));
  if (0 == csPath.GetLength()) { return; }
  m_csPath = csPath;
  UpdateData(FALSE);
}

void CimportDlg::OnBnClickedButtonLoad()
{
  // TODO: 在此添加控件通知处理程序代码
  if (TRUE == m_bStart) { return; }
  if (FALSE == UpdateData(TRUE)) { return; }

  size_t nRead = SSCANF(m_csTimeRun, m_csTimeRun.GetLength()
    , "%u-%u"
    , &m_nEnableTimeStart
    , &m_nEnableTimeEnd
    );

  if (2 != nRead || m_nEnableTimeStart == m_nEnableTimeEnd
    || m_nEnableTimeStart >= 240000
    || m_nEnableTimeEnd >= 240000
    ) { MessageBox(kStrErrorTimeConfig); return; }

  if (FALSE == checkSystem()) { return; }

  kStrSepFields = m_csSep;

  // write config file
  if (FALSE == saveConfigFile()) { MessageBox(kStrErrorWriteConfigFile);  return; }

  startImport();

  // disable this button
  m_cbImport.EnableWindow(FALSE);
}

void CimportDlg::OnBnClickedButtonStop()
{
  // TODO: 在此添加控件通知处理程序代码
  if (FALSE == m_bStart) { return; }
  stopImport();
}

void CimportDlg::OnBnClickedButtonCt()
{
  // TODO: 在此添加控件通知处理程序代码
  UpdateData(TRUE);
  AutoRelease<IConnection*> autoRelConn(IConnection::CreateInstance(_STR("MYSQLCI")));
  if (NULL == autoRelConn) { MessageBox(kStrErrorDBInstance); return; }

  db_conn_str_t dbConn;
  if (RC_S_OK != db_parse_cs(&dbConn, m_csDB) || RC_S_OK != autoRelConn->connect(&dbConn)) {
    MessageBox(kStrErrorDBConfig); return;
  }

  char_t strSQL[16 * 1024] = {0};
  
  char_t strHisDBName[128] = "`";

  char_t strHisTableNameEx[128] = {0};
  SNPRINTF(strHisTableNameEx, sizeof(strHisTableNameEx), sizeof(strHisTableNameEx), "_%02u`", m_dwTableName);

  SNPRINTF(strSQL, sizeof(strSQL), sizeof(strSQL), kStrCreateSQLFmt
    , strHisDBName, strHisTableNameEx, m_csDBEng
    , strHisDBName, strHisTableNameEx, m_csDBEng
    , strHisDBName, strHisTableNameEx, m_csDBEng
    , strHisDBName, strHisTableNameEx, m_csDBEng
    , strHisDBName, strHisTableNameEx, m_csDBEng
    , strHisDBName, strHisTableNameEx, m_csDBEng
    , strHisDBName, strHisTableNameEx, m_csDBEng
    , strHisDBName, strHisTableNameEx, m_csDBEng
    );

  if (RC_S_OK != autoRelConn->execute(strSQL, FALSE)) { MessageBox(kStrErrorDBCreateTable); return; }

  MessageBox(kStrNoErrorDBCreateTable);
}

//////////////////////////////////////////////////////////////////////////
BOOL CimportDlg::checkSystem() {

  m_dwThreadNum = m_dwThreadNum % kMaxThreadNum;

  if (FALSE == checkSystemFileSystem()) { return FALSE; }

  if (FALSE == checkSystemDataBase()) { return FALSE; }

  return TRUE;  
}

BOOL CimportDlg::checkSystemDataBase() {

  // database
  AutoRelease<IConnection*> autoRelConn(IConnection::CreateInstance(_STR("MYSQLCI")));
  if (NULL == autoRelConn) { MessageBox(kStrErrorDBInstance); return FALSE; }

  db_conn_str_t dbConn;
  if (RC_S_OK != db_parse_cs(&dbConn, m_csDB)|| RC_S_OK != autoRelConn->connect(&dbConn)) {
    MessageBox(kStrErrorDBConfig);  return FALSE;
  }

  CString csSQL;
  for (uint32_t idx = 0; idx < kLogTypeNum; ++idx) {

    csSQL.Format("SELECT %s FROM %s_%02u LIMIT 1;"
      , kStrTableFields[0]
    , kStrTables[0], m_dwTableName
      );

    if (RC_S_OK != autoRelConn->execute(csSQL, FALSE)) {
      MessageBox(CString(kStrErrorDBCheckTable) + kStrTables[idx]);  return FALSE;
    }
  }

  return TRUE;
}

BOOL CimportDlg::checkSystemFileSystem() {

  AutoLock autoLock(m_lockFileQueue);
  if (m_queueFile.empty()) { MessageBox(kStrErrorFileQueue); return FALSE; }

  // dict
  if (RC_S_OK != m_dictTC50.LoadDefaultDict(m_csDict)) { MessageBox(kStrErrorTC50Dict); return FALSE; }

  // test
  CString csTestFilePath(m_csPath);
  csTestFilePath.Format("%s/__test__%p", m_csPath, &csTestFilePath);
  AutoReleaseFile autoRelFile(OpenFile(csTestFilePath, strAttrOpenWrite));
  if (NULL == autoRelFile) { MessageBox(kStrErrorTempPath);  return FALSE; }

  autoRelFile.Release();
  Delete(csTestFilePath, TRUE);

  return TRUE;
}

void CimportDlg::startImport() {
  
  m_bStart = TRUE;

  for (uint32_t idx = 0; idx < m_dwThreadNum; ++idx) {

    if (NULL == m_thdgrpParser.create_thread(this, &idx)) { continue; }
    ++m_acThreaNum;
  }
}

void CimportDlg::stopImport() {

  if (TRUE == m_bStart) {
    m_bStart = FALSE;
    //m_evStop.Wait();
  }

  // cancel
  AutoLock autoLock(m_lockFileQueue);
  while (!m_queueFile.empty()) {

    const FileNode& fileNode = m_queueFile.front();

    m_lcLogFile.SetItemText(fileNode.order, LC_STATUS, _T("cancel"));

    m_queueFile.pop();
  }
  
  //m_thdgrpParser.join_all();
}

void CimportDlg::ThreadMain(void* context) {

  if (NULL == context) { return; }
  uint32_t idx = (*(uint32_t*)context);

  db_conn_str_t dbConn;
  if (RC_S_OK != db_parse_cs(&dbConn, m_csDB)) { return; }

  AutoRelease<IConnection*> autoRelConn(IConnection::CreateInstance(dbConn.db_driver));
  if (NULL == autoRelConn || RC_S_OK != autoRelConn->connect(&dbConn)) { return; }

  TC50Log tc50Log(m_arrLogRule);
  AutoReleaseMemoryBase autoRelReadBuf((uint8_t*)malloc(kMinThreadMemorySize));
  if (NULL == autoRelReadBuf) { return; }

  AutoReleaseMemoryBase autoRelWriteBuf[kLogTypeNum] = {
    (uint8_t*)malloc(kWriteFileMemorySize)
    , (uint8_t*)malloc(kWriteFileMemorySize)
    , (uint8_t*)malloc(kWriteFileMemorySize)
    , (uint8_t*)malloc(kWriteFileMemorySize)
    , (uint8_t*)malloc(kWriteFileMemorySize)
    , (uint8_t*)malloc(kWriteFileMemorySize)
    , (uint8_t*)malloc(kWriteFileMemorySize)
    , (uint8_t*)malloc(kWriteFileMemorySize)
  };

  // run
  
  uint32_t knSleepTime = 300;
  while (TRUE == m_bStart) {

    uint32_t now = DayTime::to_string(DayTime::now());
    if (m_nEnableTimeStart > m_nEnableTimeEnd) {
      if (now < m_nEnableTimeStart && now > m_nEnableTimeEnd) { Thread::sleep(knSleepTime); continue; }
    }
    else {
      if (now < m_nEnableTimeStart || now > m_nEnableTimeEnd) { Thread::sleep(knSleepTime); continue; }
    }

    if (FALSE == parseLog(autoRelConn, autoRelReadBuf, autoRelWriteBuf, tc50Log, idx)) { goto end; }
  }

end:
  if (0 == --m_acThreaNum) {
    /*m_evStop.Signal();*/ m_bStart = FALSE;
    m_cbImport.EnableWindow(TRUE);
  }
}

bool_t CimportDlg::parseLog(AutoRelease<IConnection*>& autoRelConn
                            , AutoReleaseMemoryBase& autoRelReadBuf
                            , AutoReleaseMemoryBase* autoRelWriteBuf
                            , TC50Log& tc50Log
                            , uint32_t thd_idx) {

  FileNode fileNode;
  uint32_t BufPos[kLogTypeNum] = {0};
  AutoReleaseFile autoRelFileOut[kLogTypeNum] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
  UNUSED_PARAM(thd_idx);
  
  // get file
  {
    AutoLock autoLock(m_lockFileQueue);
    if (m_queueFile.empty()) { return FALSE; }

    fileNode.Copy(m_queueFile.front());
    m_queueFile.pop();

    // update 
    m_lcLogFile.SetItemText(fileNode.order, LC_STATUS, _T("processing"));
  }


  uint64_t nProcLogTime = micro_time::time();
  // open file
  ASSERT(fileNode.size);
  CString csPath(m_csLog + "\\" + fileNode.name);
  AutoReleaseFile autoRelFile(OpenFile(csPath, strAttrOpenRead));
  if (NULL == autoRelFile) { goto end; }

  uint32_t nDate = ATOI(fileNode.name);
  uint32_t nDataTime = DayTime::to_localtime(nDate / 10000, (nDate / 100) % 100, nDate % 100, 0, 0, 0);
  static const uint32_t knTiem19900101 = DayTime::to_localtime(1990, 1, 1, 0, 0, 0);
  nDataTime = nDataTime - knTiem19900101;
  const uint32_t nOneDaySec = 24 * 60 * 60;
  tc50Log.nDate = nDataTime / nOneDaySec;

  // temp file
  for (uint32_t idx = 0; idx < kLogTypeNum; ++idx) {

    char_t strFileName[MAX_PATH] = {0};
    SNPRINTF(strFileName, sizeof(strFileName), sizeof(strFileName), _STR("%s/%s_%u"), m_csPath, kStrTables[idx], nDate);
    autoRelFileOut[idx].Set(OpenFile(strFileName, strAttrOpenWrite));
    if (NULL == autoRelFileOut[idx]) { return TRUE; }
    if (NULL == autoRelWriteBuf[idx]) { return TRUE; }
  }

  tc50Log.ResetCount();
  file_size_t nFileParserPos = 0;
  file_size_t nFilePos = 0;
  uint32_t nBufPos = 0;
  do 
  {
    file_size_t nReadSize = kMinThreadMemorySize - nBufPos;
    ReadFile(autoRelReadBuf + nBufPos, &nReadSize, autoRelFile);
    
    if (0 == nReadSize) {
      break; }
    nFilePos += nReadSize;

    // parser
    nBufPos += (uint32_t)nReadSize;
    uint32_t nParsePos = nBufPos;
    if (TRUE != parseLogProc(autoRelFileOut
      , autoRelWriteBuf, BufPos, &tc50Log, autoRelReadBuf, &nParsePos, nFileParserPos))
    {
      m_lcLogFile.SetItemText(fileNode.order, LC_STATUS, _T("Parse File Failed"));
      return TRUE;
    }

    // move
    nFileParserPos += nParsePos;
    nBufPos -= nParsePos;
    MEMMOVE(autoRelReadBuf, kMinThreadMemorySize, autoRelReadBuf + nParsePos, nBufPos);

  } while(nFilePos < fileNode.size);

end:

  bool_t bOK = TRUE;
  // exec load data
  //const char_t* strStatus = _T("ok");
  CString strStatus(_T("ok"));

  DWORD dwTableName = (nDate / 10000) % 100;

  CString csSQL;
  CString csFilePath;
  CString csTableName;
  for (uint32_t idx = 0; idx < kLogTypeNum; ++idx) {

    // finish file
    if (BufPos[idx]) {
      file_size_t nWriteSize = BufPos[idx];
      rc_t rc = WriteFile(autoRelFileOut[idx], autoRelWriteBuf[idx], &nWriteSize);
      autoRelFileOut[idx].Release();

      if (RC_S_OK != rc) { bOK = FALSE; strStatus = _T("Write Temp File Failed"); break; }
      BufPos[idx] = 0;
    }
    else {
      autoRelFileOut[idx].Release();
    }

    csFilePath.Format(_T("%s\\%s_%u"), m_csPath, kStrTables[idx], nDate);

    if (TRUE == bOK) {
      csTableName.Format(_T("%s_%02u"), kStrTables[idx], dwTableName);

      csFilePath.Replace('\\', '/');
      const char_t* strDBLoadDataEx = TRUE == m_bDBRemote ? "LOCAL" : NULL_STR;

      csSQL.Format("LOAD DATA LOW_PRIORITY %s INFILE "
        "'%s' "
        "REPLACE INTO TABLE `%s` "
        "CHARACTER SET gbk "
        "FIELDS TERMINATED BY '%s' "
        "OPTIONALLY ENCLOSED BY '\\t' "
        "ESCAPED BY '\\n' "
        "LINES TERMINATED BY '\\r\\n' "
        "(%s)"
        ";"

        , strDBLoadDataEx
        , csFilePath
        , csTableName
        , kStrSepFields
        , kStrTableFields[idx]
      );
      if (RC_S_OK != autoRelConn->execute(csSQL, FALSE)) {
        bOK = FALSE;
        uint32_t nLastErrorNo = 0;
        const char_t* strLastError = autoRelConn->get_last_error(&nLastErrorNo);
        if (strLastError) {
          strStatus.Format("Load Data Failed.%u,%s", nLastErrorNo, strLastError);
        }
        else {
          strStatus.Format("Load Data Failed.%u,%s", nLastErrorNo, "UnKnown Error");
        }

        if (TRUE == m_bDelTempFile) { Delete(csFilePath, FALSE); }
        break;
      }
    }

    if (TRUE == m_bDelTempFile) { Delete(csFilePath, FALSE); }
  }
  if (TRUE == bOK) { autoRelConn->commit(); }
  else { autoRelConn->rollback(); }

  nProcLogTime = micro_time::time() - nProcLogTime;
  nProcLogTime = micro_time::to_millisecond(nProcLogTime);

  // write config
  if (TRUE == bOK && m_autoRelFileProcess) {
    AutoLock autoLock(m_lockFileProcess);
    //CString csData(m_csLog + "\\" + fileNode.name + kStrEnd);
    CString csData;
    csData.Format("%s\\%s,%I64u,%u,%u,%u\r\n"
      , m_csLog, fileNode.name
      , nFilePos
      , tc50Log.nCountReq + tc50Log.nCountAns, tc50Log.AllCount()
      , nProcLogTime
      /*, kStrEnd*/
      );
    file_size_t nWriteSize = csData.GetLength();
    if (nWriteSize) {

      hash_set<CString>::_Pairib pair = m_setFileProcess.insert(m_csLog + "\\" + fileNode.name);
      if (m_setFileProcess.end() != pair.first) {
        WriteFile(m_autoRelFileProcess, (const uint8_t*)((const char_t*)csData), &nWriteSize);
        FlushFile(m_autoRelFileProcess);
      }
    }
  }

  // update list
  CString csCount;
  csCount.Format("%u/%u", tc50Log.nCountReq + tc50Log.nCountAns, tc50Log.AllCount());
  m_lcLogFile.SetItemText(fileNode.order, LC_STATUS, strStatus);

  char_t szFilePos[64] = {0};
  char_t szProctime[64] = {0};
  I64TOA(nFilePos, szFilePos, sizeof(szFilePos), 10);
  I64TOA(nProcLogTime, szProctime, sizeof(szProctime), 10);
  m_lcLogFile.SetItemText(fileNode.order, LC_PROCESS_SIZE, szFilePos);
  m_lcLogFile.SetItemText(fileNode.order, LC_PROCESS_LOGCOUNT, csCount);
  m_lcLogFile.SetItemText(fileNode.order, LC_PROCESS_TIME, szProctime);

  return TRUE;
}

static const uint32_t kMAX_COPY_BUF_SIZE  = 8 * 1024;
bool_t CimportDlg::parseLogProc(AutoReleaseFile* autoRelFile, AutoReleaseMemoryBase* autoRelBuf
                                , uint32_t* BufPos
                                , TC50Log* pTC50Log, const uint8_t* data, uint32_t* len
                                , file_size_t nFilePos) {

  ASSERT(autoRelFile && autoRelBuf && BufPos && pTC50Log && data && len && (*len));

  IFileIteratorBase* pIFileIterator = (IFileIteratorBase*)((uintptr_t)0xbeadbeef);
  const char_t* strCollector        = NULL_STR;
  const char_t* strFileDir          = NULL_STR;
  TC50Dict* pTC50Dict               = &m_dictTC50;

  const char_t* strLogData          = (const char_t*)data;
  uint32_t nLogLen = (*len);
  uint32_t nLogSize = 0;
  uint32_t nLogPos = 0;

  do {
    
    nLogSize = nLogLen - nLogPos;
    if (kMinThreadMemorySize == nLogLen && nLogSize < 64 * 1024) { break; }
    
    if (RC_S_OK != pTC50Log->Parse(strLogData + nLogPos, &nLogSize
      , pTC50Dict, pIFileIterator, strCollector, strFileDir)) {

        // is 0x00 
        // save null data log
        size_t nNullLogLen = STRLEN(strLogData + nLogPos);
        if (nLogSize <= nNullLogLen) { break; }

        if (nNullLogLen < kMAX_COPY_BUF_SIZE) {

          char_t strNullLog[kMAX_COPY_BUF_SIZE + 8] = {0};
          size_t nNullLogSize = SNPRINTF(strNullLog, kMAX_COPY_BUF_SIZE, kMAX_COPY_BUF_SIZE
            , "%s%s", strLogData + nLogPos, kStrEnd);

          pTC50Log->Parse(strNullLog, &nNullLogSize, pTC50Dict, pIFileIterator, strCollector, strFileDir);
        }

        nLogSize = (uint32_t)nNullLogLen + 1;

        // skip all zero
        do 
        {
          uint32_t ch = *(strLogData + nLogPos + nLogSize);
          if (0x00 == ch || 0x20 == ch || '\r' == ch || '\n' == ch) { ++nLogSize; }
          else { break; }
        } while(nLogPos + nLogSize < nLogLen);

    }

    // new log
    if (TC50Log::PRET_UNKNOW == pTC50Log->eLogType || TRUE == didMathLogRule(pTC50Log->nFuncID)) {
      pTC50Log->FixData();
      if (RC_S_OK != writeLog(autoRelFile, autoRelBuf, BufPos, nFilePos + nLogPos, nLogSize, m_csLogFlag, pTC50Log)) {
        return FALSE;
      }
    }

    // update 
    nLogPos += nLogSize;
  } while (nLogPos < nLogLen);

  (*len) = nLogPos;

  return TRUE;
}

rc_t CimportDlg::writeLog(AutoReleaseFile* autoRelFile, AutoReleaseMemoryBase* autoRelBuf
                            , uint32_t* BufPos
                            , file_size_t nFilePos
                            , uint32_t nLogLen
                            , const char_t* strCollector
                            , const TC50Log* pTC50Log) {

  ASSERT(autoRelFile && autoRelBuf && BufPos && pTC50Log);

  size_t nLogType = kLogTypeNum;

  // switch file pos
  if (TC50Log::PRET_REQ == pTC50Log->eLogType) {

    if (TC50Log::DICT_SIMPLE == pTC50Log->eDictType)      { nLogType = 2; }
    else if (TC50Log::DICT_COMMON == pTC50Log->eDictType) { nLogType = 0; }
    else if (TC50Log::DICT_SCNTR == pTC50Log->eDictType)  { nLogType = 4; }
  }
  else if (TC50Log::PRET_SUCESS == pTC50Log->eLogType
    ||TC50Log::PRET_DEAERR == pTC50Log->eLogType
    || TC50Log::PRET_POLERR == pTC50Log->eLogType
    || TC50Log::PRET_FAILD == pTC50Log->eLogType
  ) {
    if (TC50Log::DICT_SIMPLE == pTC50Log->eDictType)      { nLogType = 3; }
    else if (TC50Log::DICT_COMMON == pTC50Log->eDictType) { nLogType = 1; }
    else if (TC50Log::DICT_SCNTR == pTC50Log->eDictType)  { nLogType = 5; }
  }
  
  else if (TC50Log::PRET_SYS_INFO == pTC50Log->eLogType
    || TC50Log::PRET_SYS_ERROR == pTC50Log->eLogType
    || TC50Log::PRET_CONN_CON == pTC50Log->eLogType
    || TC50Log::PRET_CONN_DIS == pTC50Log->eLogType
  ) {
    nLogType = 6;
  }
  else if (TC50Log::PRET_UNKNOW == pTC50Log->eLogType) { nLogType = 7; }

  if (kLogTypeNum == nLogType) { return RC_S_OK; }

  const size_t nMaxLogSize = 16 * 1024;
  if (kWriteFileMemorySize < BufPos[nLogType] + nMaxLogSize) {

    // write file
    ASSERT(autoRelFile[nLogType]);
    file_size_t nWriteSize = BufPos[nLogType];
    if (RC_S_OK != WriteFile(autoRelFile[nLogType], autoRelBuf[nLogType], &nWriteSize)) {
      return RC_S_FAILED;
    }
    BufPos[nLogType] = 0x00;
  }

  // write file
  int32_t nWriteLen = (m_pfn_wl[nLogType])(
    (char_t*)(autoRelBuf[nLogType] + BufPos[nLogType])
    , kWriteFileMemorySize - BufPos[nLogType]
    , nFilePos
    , nLogLen
    , strCollector
    , pTC50Log);

  if (0 > nWriteLen) { nWriteLen = 0; }

  // fix data
  /*
  {
    // skip \n
    const char_t* strData = (char_t*)(autoRelBuf[nLogType] + BufPos[nLogType]);
    //char_t* strFind = STRSTR(strData, nWriteLen - 2, "\r\n");
    char_t* strFind = STRCHR(strData, nWriteLen - 2, '\n');
    while (strFind) {
      //*(strFind) = '\\'; *(strFind + 1) = 'r';
      //strFind = STRSTR(strFind, nWriteLen - 2 - (strFind - strData), "\r\n");
      *(strFind) = ' ';
      strFind = STRCHR(strData, nWriteLen - 2, '\n');
    }

    // skip \t
    strFind = STRCHR(strData, nWriteLen - 2, '\t');
    while (strFind) {
      *(strFind) = ' ';
      strFind = STRCHR(strFind, nWriteLen - 2 - (strFind - strData), '\t');
    }
  }
  */

  BufPos[nLogType] += nWriteLen;

  // write not normal log
  if (FALSE == pTC50Log->bNormal && TC50Log::PRET_UNKNOW != pTC50Log->eLogType) {

    nLogType = 7;
    if (kWriteFileMemorySize < BufPos[nLogType] + nMaxLogSize) {

      // write file
      ASSERT(autoRelFile[nLogType]);
      file_size_t nWriteSize = BufPos[nLogType];
      if (RC_S_OK != WriteFile(autoRelFile[nLogType], autoRelBuf[nLogType], &nWriteSize)) {
        return RC_S_FAILED;
      }
      BufPos[nLogType] = 0x00;
    }

    nWriteLen = (m_pfn_wl[nLogType])(
      (char_t*)(autoRelBuf[nLogType] + BufPos[nLogType])
      , kWriteFileMemorySize - BufPos[nLogType]
      , nFilePos
      , nLogLen
      , strCollector
      , pTC50Log);

    BufPos[nLogType] += nWriteLen;
  }
  return RC_S_OK;
}

int32_t CimportDlg::writeLog_trade_req(char_t* pBuf, uint32_t nLen, file_size_t nFilePos, uint32_t nLogLen, const char_t* strCollectorFlag, const TC50Log* pTC50Log) {

  uint32_t now = DayTime::now();
  uint32_t nIP = ipToInt(pTC50Log->strIP);
  uint64_t nMAC = macToInt(pTC50Log->strMAC);
  uint32_t nThreadID = ATOI(pTC50Log->strThreadID);

  return SNPRINTF(pBuf, nLen, nLen
    , "%s%s"
    "%s_%s%s"
    "%u%s"
    "%I64u%s"
    "%u%s"
    "%u%s"
    "%u%s"

    "%s%s"
    "%02u%02u%02u%03u%s"
    "%u%s"
    "%I64u%s"

    "%u%s"
    "%u%s"
    "%u%s"
    "%u%s"
    "%u%s"
    "%s%s"
    "%u%s"
    "%s%s"

    "%s%s"
    "%s%s"

    "%u%s"
    "%s%s"
    "%s%s"
    "%u%s"

    "%s%s"
    "%u%s"
    "%s%s"
    "%u%s"
    "%u%s"

    "%u%s"
    "%s%s"
    "%u%s"
    "%u%s"

    "%u%s"

    "%s%s"
    "%s%s"
    "%s"
    "\r\n"

    , kStrParser, kStrSepFields
    , kStrCollector, strCollectorFlag, kStrSepFields
    , 0, kStrSepFields
    , nFilePos, kStrSepFields
    , nLogLen, kStrSepFields
    , now, kStrSepFields
    , pTC50Log->nDate, kStrSepFields

    , pTC50Log->strLogType, kStrSepFields
    , pTC50Log->nHour, pTC50Log->nMin, pTC50Log->nSec, pTC50Log->nMilSec, kStrSepFields
    , nIP, kStrSepFields
    , nMAC, kStrSepFields

    , nThreadID, kStrSepFields
    , pTC50Log->nChannelID, kStrSepFields
    , pTC50Log->nTransID, kStrSepFields
    , pTC50Log->nReqType, kStrSepFields
    , pTC50Log->nFuncID, kStrSepFields
    , pTC50Log->strFuncName, kStrSepFields
    , pTC50Log->nBranchID, kStrSepFields
    , pTC50Log->strBranchName, kStrSepFields

    , pTC50Log->strKHH    , kStrSepFields
    , pTC50Log->strKHMC   , kStrSepFields

    , pTC50Log->nZHLB     , kStrSepFields
    , pTC50Log->strZJZH   , kStrSepFields    
    , pTC50Log->strGDDM   , kStrSepFields    
    , pTC50Log->nOP_WTFS  , kStrSepFields

    , pTC50Log->strWTBH   , kStrSepFields
    , pTC50Log->nWTFS     , kStrSepFields
    , pTC50Log->strZQDM   , kStrSepFields
    , pTC50Log->nMMBZ     , kStrSepFields
    , pTC50Log->nJYDW     , kStrSepFields

    , pTC50Log->nWTSL     , kStrSepFields
    , pTC50Log->strWTJG   , kStrSepFields
    , pTC50Log->nWTRQ     , kStrSepFields
    , pTC50Log->nWTSJ     , kStrSepFields

    , pTC50Log->nXT_CHECKRISKFLAG , kStrSepFields

    // reserve
    , pTC50Log->strReserve_a , kStrSepFields
    , pTC50Log->strReserve_b , kStrSepFields
    , pTC50Log->strReserve_c
    );
}

int32_t CimportDlg::writeLog_logon_req(char_t* pBuf, uint32_t nLen, file_size_t nFilePos, uint32_t nLogLen, const char_t* strCollectorFlag, const TC50Log* pTC50Log) {

  uint32_t now = DayTime::now();
  uint32_t nIP = ipToInt(pTC50Log->strIP);
  uint64_t nMAC = macToInt(pTC50Log->strMAC);
  uint32_t nThreadID = ATOI(pTC50Log->strThreadID);

  return SNPRINTF(pBuf, nLen, nLen
    , "%s%s"
    "%s_%s%s"
    "%u%s"
    "%I64u%s"
    "%u%s"
    "%u%s"
    "%u%s"

    "%s%s"
    "%02u%02u%02u%03u%s"
    "%u%s"
    "%I64u%s"

    "%u%s"
    "%u%s"
    "%u%s"
    "%u%s"
    "%u%s"
    "%s%s"
    "%u%s"
    "%s%s"

    "%s%s"
    "%s%s"
    "%s%s"

    "%u%s"
    "%s%s"
    "%s%s"
    "%s%s"

    "%u%s"
    "%s%s"
    "%u%s"
    "%s%s"

    "%s%s"
    "%s%s"
    "%s"
    "\r\n"

    , kStrParser, kStrSepFields
    , kStrCollector, strCollectorFlag, kStrSepFields
    , 0, kStrSepFields
    , nFilePos, kStrSepFields
    , nLogLen, kStrSepFields
    , now, kStrSepFields
    , pTC50Log->nDate, kStrSepFields

    , pTC50Log->strLogType, kStrSepFields
    , pTC50Log->nHour, pTC50Log->nMin, pTC50Log->nSec, pTC50Log->nMilSec, kStrSepFields
    , nIP, kStrSepFields
    , nMAC, kStrSepFields

    , nThreadID, kStrSepFields
    , pTC50Log->nChannelID, kStrSepFields
    , pTC50Log->nTransID, kStrSepFields
    , pTC50Log->nReqType, kStrSepFields
    , pTC50Log->nFuncID, kStrSepFields
    , pTC50Log->strFuncName, kStrSepFields
    , pTC50Log->nBranchID, kStrSepFields
    , pTC50Log->strBranchName, kStrSepFields

    , pTC50Log->strXT_GTLB , kStrSepFields
    , pTC50Log->strKHH     , kStrSepFields
    , pTC50Log->strKHMC    , kStrSepFields

    , pTC50Log->nZHLB      , kStrSepFields
    , pTC50Log->strZJZH    , kStrSepFields
    , pTC50Log->strSHGD   , kStrSepFields
    , pTC50Log->strSZGD   , kStrSepFields

    , pTC50Log->nXT_CLITYPE, kStrSepFields
    , pTC50Log->strXT_CLIVER, kStrSepFields
    , pTC50Log->nXT_VIPFLAG, kStrSepFields
    , pTC50Log->strXT_MACHINEINFO, kStrSepFields

    // reserve
    , pTC50Log->strReserve_a, kStrSepFields
    , pTC50Log->strReserve_b, kStrSepFields
    , pTC50Log->strReserve_c
    );
}

int32_t CimportDlg::writeLog_ans(char_t* pBuf, uint32_t nLen, file_size_t nFilePos, uint32_t nLogLen, const char_t* strCollectorFlag, const TC50Log* pTC50Log) {

  uint32_t now = DayTime::now();
  uint32_t nIP = ipToInt(pTC50Log->strIP);
  uint64_t nMAC = macToInt(pTC50Log->strMAC);
  uint32_t nThreadID = ATOI(pTC50Log->strThreadID);

  return SNPRINTF(pBuf, nLen, nLen
    , "%s%s"
    "%s_%s%s"
    "%u%s"
    "%I64u%s"
    "%u%s"
    "%u%s"
    "%u%s"

    "%s%s"
    "%02u%02u%02u%03u%s"
    "%u%s"
    "%I64u%s"

    "%u%s"
    "%u%s"
    "%u%s"
    "%u%s"
    "%u%s"
    "%s%s"
    "%u%s"
    "%s%s"

    "%u%s"
    "%u%s"
    "%u%s"
    "%d%s"
    "%s%s"

    "%s%s"
    "%s%s"
    "%s%s"

    "%s%s"
    "%s%s"
    "%s"

    "\r\n"

    , kStrParser, kStrSepFields
    , kStrCollector, strCollectorFlag, kStrSepFields
    , 0, kStrSepFields
    , nFilePos, kStrSepFields
    , nLogLen, kStrSepFields
    , now, kStrSepFields
    , pTC50Log->nDate, kStrSepFields

    , pTC50Log->strLogType, kStrSepFields
    , pTC50Log->nHour, pTC50Log->nMin, pTC50Log->nSec, pTC50Log->nMilSec, kStrSepFields
    , nIP, kStrSepFields
    , nMAC, kStrSepFields

    , nThreadID, kStrSepFields
    , pTC50Log->nChannelID, kStrSepFields
    , pTC50Log->nTransID, kStrSepFields
    , pTC50Log->nReqType, kStrSepFields
    , pTC50Log->nFuncID, kStrSepFields
    , pTC50Log->strFuncName, kStrSepFields
    , pTC50Log->nBranchID, kStrSepFields
    , pTC50Log->strBranchName, kStrSepFields

    , pTC50Log->nTimeA        , kStrSepFields
    , pTC50Log->nTimeB        , kStrSepFields
    , pTC50Log->nQueue        , kStrSepFields
    , pTC50Log->nReturnNO     , kStrSepFields
    , pTC50Log->strReturnMsg  , kStrSepFields

    , pTC50Log->strWTBH       , kStrSepFields
    , pTC50Log->strXT_CHECKRISKFLAG , kStrSepFields
    , pTC50Log->strRETINFO     , kStrSepFields

    // reserve
    , pTC50Log->strZJYE       , kStrSepFields
    , pTC50Log->strZQSL         , kStrSepFields
    , pTC50Log->strKMSL
    );
}
int32_t CimportDlg::writeLog_sc_req(char_t* pBuf, uint32_t nLen, file_size_t nFilePos, uint32_t nLogLen, const char_t* strCollectorFlag, const TC50Log* pTC50Log) {

  uint32_t now = DayTime::now();
  uint32_t nIP = ipToInt(pTC50Log->strIP);
  uint64_t nMAC = macToInt(pTC50Log->strMAC);
  uint32_t nThreadID = ATOI(pTC50Log->strThreadID);

  return SNPRINTF(pBuf, nLen, nLen
    , "%s%s"
    "%s_%s%s"
    "%u%s"
    "%I64u%s"
    "%u%s"
    "%u%s"
    "%u%s"

    "%s%s"
    "%02u%02u%02u%03u%s"
    "%u%s"
    "%I64u%s"

    "%u%s"
    "%u%s"
    "%u%s"
    "%u%s"
    "%u%s"
    "%s%s"
    "%u%s"
    "%s%s"

    "%s%s"
    "%s%s"
    "%s%s"
    "%s%s"
    "%s%s"
    "%s%s"
    "%s%s"
    "%s%s"
    "%s%s"
    "%s%s"

    "%s%s"
    "%s%s"
    "%s%s"
    "%s%s"
    "%s%s"
    "%s%s"
    "%s%s"

    "%s%s"
    "%s%s"
    "%s%s"
    "%s%s"
    "%s%s"

    "%s%s"
    "%s%s"
    "%s"
    "\r\n"

    , kStrParser, kStrSepFields
    , kStrCollector, strCollectorFlag, kStrSepFields
    , 0, kStrSepFields
    , nFilePos, kStrSepFields
    , nLogLen, kStrSepFields
    , now, kStrSepFields
    , pTC50Log->nDate, kStrSepFields

    , pTC50Log->strLogType, kStrSepFields
    , pTC50Log->nHour, pTC50Log->nMin, pTC50Log->nSec, pTC50Log->nMilSec, kStrSepFields
    , nIP, kStrSepFields
    , nMAC, kStrSepFields

    , nThreadID, kStrSepFields
    , pTC50Log->nChannelID, kStrSepFields
    , pTC50Log->nTransID, kStrSepFields
    , pTC50Log->nReqType, kStrSepFields
    , pTC50Log->nFuncID, kStrSepFields
    , pTC50Log->strFuncName, kStrSepFields
    , pTC50Log->nBranchID, kStrSepFields
    , pTC50Log->strBranchName, kStrSepFields

    , pTC50Log->strXT_GTLB       , kStrSepFields
    , pTC50Log->strCA_KHH        , kStrSepFields
    , pTC50Log->strCA_KHMC       , kStrSepFields
    , pTC50Log->strCA_VER        , kStrSepFields
    , pTC50Log->strCA_AQJB       , kStrSepFields
    , pTC50Log->strCA_TXMM       , kStrSepFields
    , pTC50Log->strCA_ISVIPHOST  , kStrSepFields
    , pTC50Log->strCA_JQTZM      , kStrSepFields
    , pTC50Log->strCA_SLOTSN     , kStrSepFields
    , pTC50Log->strCA_CID        , kStrSepFields

    , pTC50Log->strCA_CERTREQ    , kStrSepFields
    , pTC50Log->strCA_USERCERDN  , kStrSepFields
    , pTC50Log->strCA_ZSQSRQ     , kStrSepFields
    , pTC50Log->strCA_ZSJZRQ     , kStrSepFields
    , pTC50Log->strCA_CERTSN     , kStrSepFields
    , pTC50Log->strCA_CERTINFO   , kStrSepFields
    , pTC50Log->strCA_MACHINENAME, kStrSepFields

    , pTC50Log->strCA_DLSJ       , kStrSepFields
    , pTC50Log->strCA_LASTIP     , kStrSepFields
    , pTC50Log->strCA_MAC        , kStrSepFields
    , pTC50Log->strCA_CSCS       , kStrSepFields
    , pTC50Log->strCA_RESV       , kStrSepFields

    // reserve
    , pTC50Log->strReserve_a, kStrSepFields
    , pTC50Log->strReserve_b, kStrSepFields
    , pTC50Log->strReserve_c
    );
}

int32_t CimportDlg::writeLog_info(char_t* pBuf, uint32_t nLen, file_size_t nFilePos, uint32_t nLogLen, const char_t* strCollectorFlag, const TC50Log* pTC50Log) {

  uint32_t now = DayTime::now();
  uint32_t nIP = ipToInt(pTC50Log->strIP);

  return SNPRINTF(pBuf, nLen, nLen
    , "%s%s"
    "%s_%s%s"
    "%u%s"
    "%I64u%s"
    "%u%s"
    "%u%s"
    "%u%s"

    "%s%s"
    "%02u%02u%02u%03u%s"
    "%s%s"
    "%u%s"

    "%u%s"
    "%s%s"
    "%s%s"
    "%s"
    "\r\n"

    , kStrParser, kStrSepFields
    , kStrCollector, strCollectorFlag, kStrSepFields
    , 0, kStrSepFields
    , nFilePos, kStrSepFields
    , nLogLen, kStrSepFields
    , now, kStrSepFields
    , pTC50Log->nDate, kStrSepFields

    , pTC50Log->strLogType, kStrSepFields
    , pTC50Log->nHour, pTC50Log->nMin, pTC50Log->nSec, pTC50Log->nMilSec, kStrSepFields
    , pTC50Log->strSysInfo, kStrSepFields
    , pTC50Log->nChannelID, kStrSepFields

    , nIP, kStrSepFields
    , pTC50Log->strOP_Organization, kStrSepFields
    , pTC50Log->strOP_Account, kStrSepFields
    , pTC50Log->strReason
    );
}

int32_t CimportDlg::writeLog_failed(char_t* pBuf, uint32_t nLen, file_size_t nFilePos, uint32_t nLogLen, const char_t* strCollectorFlag, const TC50Log* pTC50Log) {

  uint32_t now = DayTime::now();

  return SNPRINTF(pBuf, nLen, nLen
    , "%s%s"
      "%s_%s%s"
      "%u%s"
      "%I64u%s"
      "%u%s"
      "%u%s"
      "%u"
      "\r\n"
    
    , kStrParser, kStrSepFields
    , kStrCollector, strCollectorFlag, kStrSepFields
    , 0, kStrSepFields
    , nFilePos, kStrSepFields
    , nLogLen, kStrSepFields
    , now, kStrSepFields
    , pTC50Log->nDate
    );
}

void CimportDlg::initConfig() {

  m_autoRelFileProcess.Set(OpenFile(kStrProcessFileName, strAttrOpenReadWrite));
  if (NULL == m_autoRelFileProcess) {
    m_autoRelFileProcess.Set(OpenFile(kStrProcessFileName, strAttrOpenWrite));
  }

  const uint32_t kMaxConfigSize = 64 * 1024;
  char_t strFileData[kMaxConfigSize] = {0};
  file_size_t nReadSize = kMaxConfigSize;
  if (RC_S_OK == ReadFile((uint8_t*)strFileData, &nReadSize, m_autoRelFileProcess, 0x00)
    && nReadSize
    ) {

      const char_t* strData = strFileData;
      const char_t* strFind = NULL;
      while (NULL != (strFind = STRSTR(strData, kMaxConfigSize - (strData - strFind), kStrEnd))) {

        CString csFile;
        // find ,
        const char_t* strFindSep = STRCHR(strData, STRLEN(strData), ',');
        if (strFindSep) {
          csFile = CString(strData, (uint32_t)(strFindSep - strData));
        }
        else {
          csFile = CString(strData, (uint32_t)(strFind - strData));
        }
        
        // add to set
        m_setFileProcess.insert(csFile);
        strData = strFind + kStrEndLen;
      }
  }

  m_autoRelFileConfig.Set(OpenFile(kStrConfigFileName, strAttrOpenReadWrite));
  if (NULL == m_autoRelFileConfig) {
    m_autoRelFileConfig.Set(OpenFile(kStrConfigFileName, strAttrOpenWrite));
  }

  loadConfigFile();
}

//////////////////////////////////////////////////////////////////////////
/*
<?xml version="1.0" encoding="utf-8"?>
<Import>
  <file logpath="" filter="^20[0-9]{6}\.log$" dict="" temppath="" deltemp="Y" />
  <database>
	  <Server name="maindb" Driver="MYSQLCI" Source="127.0.0.1,3306" DBName="tls" DBUsr="root" DBPwd="" BaseConn="2" MaxConn="64" KeepAlive="3600" dbcharset="gbk" Compress="yes" />
  </database>  
  <runtime threadnum="1" logflag="1" tableex="13" storeeng="BDE" sepstr="|" dbremote="Y" />
  <timerun enable="170000-50000" />
</Import>	
*/
static const char_t* kStrConfigFileWriteFMT = 
"<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n"
"<Import>\r\n"
"  <file logpath=\"%s\" filtertime=\"%s\" dict=\"%s\" temppath=\"%s\" deltemp=\"%s\" />\r\n"
"  <database>\r\n"
"	  %s\r\n"
"  </database>\r\n"
"  <runtime threadnum=\"%u\" logflag=\"%s\" tableex=\"%u\" storeeng=\"%s\" sepstr=\"%s\" dbremote=\"%s\" />\r\n"
"  <timerun enable=\"%u-%u\" />\r\n"
"  <net sendrate=\"%u\" />\r\n"
"  <tc50log include=\"%s\" exclude=\"%s\" />\r\n"
"</Import>\r\n"
;

static const char_t* kStrConfigFileReadFMT = 
"<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n"
"<Import>\r\n"
"  <file logpath=%s filtertime=%s dict=%s temppath=%s deltemp=%s />\r\n"
"  <database>\r\n"
"	  %[^\r\n]"
"  </database>\r\n"
"  <runtime threadnum=\"%u\" logflag=%s tableex=\"%u\" storeeng=%s sepstr=%s dbremote=%s />\r\n"
"  <timerun enable=\"%u-%u\" />\r\n"
"  <net sendrate=\"%u\" />\r\n"
"  <tc50log include=%s exclude=%s />\r\n"
"</Import>\r\n"
;

bool_t CimportDlg::loadConfigFile() {

  if (NULL == m_autoRelFileConfig) { return FALSE; }

  const uint32_t kMaxConfigSize = 1024;
  char_t strFileData[kMaxConfigSize] = {0};
  file_size_t nReadSize = kMaxConfigSize;
  if (RC_S_OK != ReadFile((uint8_t*)strFileData, &nReadSize, m_autoRelFileConfig, 0x00)
    || 0 == nReadSize
    ) { return FALSE; }


  char_t strLog[512] = {0};

  char_t strFilterTime[32] = {0};
  /*char_t strFilter[128] = {0};*/
  char_t strDict[512] = {0};
  char_t strPath[512] = {0};
  char_t strDelTemp[32] = {0};

  char_t strDB[1024] = {0};
  char_t strLogFlag[32] = {0};

  char_t strDBEng[32] = {0};
  char_t strSep[32] = {0};
  char_t strDBRemote[32] = {0};

  char_t strLogInclude[16*1024] = {0};
  char_t strLogExclude[16*1024] = {0};

  const uint32_t kScanfCount = 15;
  const uint32_t kNewScanfCount = 17;
  int nScaned = SSCANF(strFileData, (uint32_t)nReadSize
    , kStrConfigFileReadFMT

    , strLog, sizeof(strLog), strFilterTime, sizeof(strFilterTime), /*strFilter, sizeof(strFilter), */strDict, sizeof(strDict)
    , strPath, sizeof(strPath), strDelTemp, sizeof(strDelTemp)

    , strDB, sizeof(strDB)

    , &m_dwThreadNum, strLogFlag, sizeof(strLogFlag), &m_dwTableName
    , strDBEng, sizeof(strDBEng), strSep, sizeof(strSep), strDBRemote, sizeof(strDBRemote)

    , &m_nEnableTimeStart, &m_nEnableTimeEnd
    , &m_dwNetRate
    , strLogInclude, sizeof(strLogInclude)
    , strLogExclude, sizeof(strLogExclude)
    );

  if (kScanfCount != nScaned && kNewScanfCount != nScaned) { return FALSE; }

  strLog[STRLEN(strLog) - 1] = 0;
  m_csLog = strLog + 1;
/*  m_csLogFilter = strFilter;*/

  strDict[STRLEN(strDict) - 1] = 0;
  m_csDict = strDict  +1;

  strPath[STRLEN(strPath) - 1] = 0;
  m_csPath = strPath + 1;

  m_bDelTempFile = 'Y' == strDelTemp[1] ? TRUE : FALSE;
  
  //strDB[STRLEN(strDB) - 1] = 0;
  m_csDB = strDB;

  strLogFlag[STRLEN(strLogFlag) - 1] = 0;
  m_csLogFlag = strLogFlag + 1;

  strDBEng[STRLEN(strDBEng) - 1] = 0;
  m_csDBEng = strDBEng + 1;

  strSep[STRLEN(strSep) - 1] = 0;
  m_csSep = strSep + 1;

  m_bDBRemote = 'Y' == strDBRemote[1] ? TRUE : FALSE;

  if (m_nEnableTimeStart >= 240000) {
    MessageBox(kStrErrorTimeConfig);
    m_nEnableTimeStart = 170000;
  }

  if (m_nEnableTimeEnd >= 240000) {
    MessageBox(kStrErrorTimeConfig);
    m_nEnableTimeEnd = 50000;
  }

  m_csTimeRun.Format("%u-%u", m_nEnableTimeStart, m_nEnableTimeEnd);

  strFilterTime[STRLEN(strFilterTime) - 1] = 0;
  m_csFilterTime = strFilterTime + 1;

  // log filter
  if (kNewScanfCount == nScaned) {

    strLogInclude[STRLEN(strLogInclude) - 1] = 0;
    m_csLogInclude = strLogInclude + 1;

    strLogExclude[STRLEN(strLogExclude) - 1] = 0;
    m_csLogExclude = strLogExclude + 1;
  }

  const char_t* strIncludeLog = m_csLogInclude.GetBuffer(0);
  if (*strIncludeLog) {

    // clear logrule
    MEMSET(m_arrLogRule, kLOG_UNFLAG, sizeof(m_arrLogRule));

    // include
    parseLogFilter(strIncludeLog, kLOG_FLAG);
  }
  else {

    // fill logrule
    MEMSET(m_arrLogRule, kLOG_FLAG, sizeof(m_arrLogRule));

    const char_t* strExcludeLog = m_csLogExclude.GetBuffer(0);
    if (*strExcludeLog) { parseLogFilter(strExcludeLog, kLOG_UNFLAG); }
  }

  loadFileQueue();
  return TRUE;
}

void CimportDlg::parseLogFilter(const char_t* strLogFilter, uint32_t v) {

  const char_t* strFind = NULL;
  do {

    strFind = STRCHR(strLogFilter, STRLEN(strLogFilter), ',');
    if (NULL == strFind)  {
      SetLogRule(ATOI(strLogFilter), v); break;
    }
    else {
      SetLogRule(ATOI(strLogFilter), v);
      strLogFilter = strFind + 1;
    }

  } while (*strLogFilter);
}

bool_t CimportDlg::saveConfigFile() {

  if (NULL == m_autoRelFileConfig) { return FALSE; }

  m_dwNetRate = 0;

  char_t strConfigFile[1024] = {0};
  file_size_t nWriteSize;
  size_t nLen = SNPRINTF(strConfigFile, sizeof(strConfigFile), sizeof(strConfigFile)
    , kStrConfigFileWriteFMT

    , m_csLog, m_csFilterTime, /*m_csLogFilter, */m_csDict, m_csPath, TRUE == m_bDelTempFile ? "Y" : "N"
    , m_csDB
    , m_dwThreadNum, m_csLogFlag, m_dwTableName, m_csDBEng, m_csSep, TRUE == m_bDBRemote ? "Y" : "N"
    , m_nEnableTimeStart, m_nEnableTimeEnd
    , m_dwNetRate
    , m_csLogInclude, m_csLogExclude
    );

  if (0 == nLen) { return FALSE; }

  nWriteSize = nLen;
  //
  if (RC_S_OK != WriteFile(m_autoRelFileConfig, 0x00, (const uint8_t*)strConfigFile, &nWriteSize)) { return FALSE; }
  FlushFile(m_autoRelFileConfig);

  loadFileQueue();
  return TRUE;
}

// like 192.168.2.2
uint32_t CimportDlg::ipToInt(const char_t* strIP) {

  uint32_t a = 0, b = 0, c = 0, d = 0;

  const uint32_t kScanfCount = 4;
  int nScaned = SSCANF(strIP, STRLEN(strIP), _STR("%u.%u.%u.%u"), &a, &b, &c, &d);

  if (kScanfCount != nScaned) { return 0; }

  return a << 24 | b << 16 | c << 8 | d;
}

// like 8C89A5772B3D
uint64_t CimportDlg::macToInt(const char_t* strMAC) {

  uint64_t val;

  const uint32_t kScanfCount = 1;
  int nScaned = SSCANF(strMAC, STRLEN(strMAC), "%I64X",&val);

  return kScanfCount != nScaned ? 0 : val;
}
