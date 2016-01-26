// ExportAQDictDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ExportAQDict.h"
#include "ExportAQDictDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CExportAQDictDlg dialog

CExportAQDictDlg::CExportAQDictDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CExportAQDictDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CExportAQDictDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
  BZERO_ARR(m_cbDicData);
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CExportAQDictDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CExportAQDictDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CExportAQDictDlg, CDialog)
	//{{AFX_MSG_MAP(CExportAQDictDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_OPEN_DICT1, OnOpenDict1)
	ON_BN_CLICKED(IDC_OPEN_DICT2, OnOpenDict2)
	ON_BN_CLICKED(IDC_OPEN_DICT3, OnOpenDict3)
	ON_BN_CLICKED(IDC_CHECK1_C, OnCheck1C)
	ON_BN_CLICKED(IDC_CHECK1_S, OnCheck1S)
	ON_BN_CLICKED(IDC_CHECK1_SC, OnCheck1Sc)
	ON_BN_CLICKED(IDC_CHECK2_C, OnCheck2C)
	ON_BN_CLICKED(IDC_CHECK2_S, OnCheck2S)
	ON_BN_CLICKED(IDC_CHECK2_SC, OnCheck2Sc)
	ON_BN_CLICKED(IDC_CHECK3_C, OnCheck3C)
	ON_BN_CLICKED(IDC_CHECK3_S, OnCheck3S)
	ON_BN_CLICKED(IDC_CHECK3_SC, OnCheck3Sc)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CExportAQDictDlg message handlers

BOOL CExportAQDictDlg::OnInitDialog()
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
  BZERO_ARR(m_cbDicData);
	GetDlgItem(IDC_DICT_NAME)->SetWindowText("Default");
	
	TCHAR exeFullPath[MAX_PATH]; // MAX_PATH
  GetModuleFileName(NULL,exeFullPath,MAX_PATH);
  m_csLocalPath = exeFullPath;
  m_csLocalPath = m_csLocalPath.Left(m_csLocalPath.ReverseFind('\\'));
	SetWindowText("dictexplorer " STR_VERSION);
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CExportAQDictDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CExportAQDictDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

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

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CExportAQDictDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CExportAQDictDlg::OnOK() {

  // get file name
  CString csFileName;
  GetDlgItem(IDC_DICT_NAME)->GetWindowText(csFileName);
  if (0 == csFileName.GetLength()) {

    CString csInfo("File Name Illegal!");
    MessageBox(csInfo, MB_OK);
    return;
  }

  csFileName = m_csLocalPath + "/" + csFileName + ".sto";

  FILE* outFile = fopen(csFileName, "wb+");
  if (NULL == outFile) {
    CString csInfo("Write File Failed!" + csFileName);
    MessageBox(csInfo, MB_OK);
    return;
  }

  // simple
  uint32_t nDictIdx;
  if (TRUE == ((CButton*)GetDlgItem(IDC_CHECK1_S))->GetCheck()) { nDictIdx = 0; }
  else if (TRUE == ((CButton*)GetDlgItem(IDC_CHECK2_S))->GetCheck()) { nDictIdx = 1; }
  else if (TRUE == ((CButton*)GetDlgItem(IDC_CHECK3_S))->GetCheck()) { nDictIdx = 2; }
  else { 
    CString csInfo("CheckBox Choose Failed!" + csFileName);
    MessageBox(csInfo, MB_OK);
    fclose(outFile);
    return;
  }
  
  LPBYTE lpBuffer = m_cbDicData[nDictIdx];
  uint32_t nDictLen = *((uint32_t*)lpBuffer);
  if (1 != fwrite(lpBuffer, DICT_LEN_LEN + nDictLen, 1, outFile)) {
    
    CString csInfo("Write simple Failed!" + csFileName);
    MessageBox(csInfo, MB_OK);
    fclose(outFile);
    return;
  }
  
  // common
  if (TRUE == ((CButton*)GetDlgItem(IDC_CHECK1_C))->GetCheck()) { nDictIdx = 0; }
  else if (TRUE == ((CButton*)GetDlgItem(IDC_CHECK2_C))->GetCheck()) { nDictIdx = 1; }
  else if (TRUE == ((CButton*)GetDlgItem(IDC_CHECK3_C))->GetCheck()) { nDictIdx = 2; }
  else {
    CString csInfo("CheckBox Choose Failed!" + csFileName);
    MessageBox(csInfo, MB_OK);
    fclose(outFile);
    return;
  }

  uint32_t nDictPos = DICT_LEN_LEN + *((uint32_t*)m_cbDicData[nDictIdx]);

  lpBuffer = m_cbDicData[nDictIdx] + nDictPos;
  nDictLen = *((uint32_t*)lpBuffer);  
  if (1 != fwrite(lpBuffer, DICT_LEN_LEN + nDictLen, 1, outFile)) {
    
    CString csInfo("Write Comm Failed!" + csFileName);
    MessageBox(csInfo, MB_OK);
    fclose(outFile);
    return;
  }

  // sc
  CWtDict2 dictSC;
  BYTE cDict[128*1024]= {0};
  DWORD dwLen = sizeof(cDict) - DICT_LEN_LEN;
  lpBuffer = cDict;
	
  if(!INIT_SCNTR_DICT(&dictSC)
    || 0 == dictSC.ExportCommonAndGetUsed(cDict + DICT_LEN_LEN, dwLen, dwLen)
    || 0 == (nDictLen = dwLen)
    || 0 == (*((uint32_t*)cDict) = nDictLen)
    || 1 != fwrite(lpBuffer, DICT_LEN_LEN + nDictLen, 1, outFile)
  ) {

    CString csInfo("Write SC Failed!" + csFileName);
    MessageBox(csInfo, MB_OK);
    fclose(outFile);
    return;
  }

  fclose(outFile);

  {
    CString csInfo("Write Dict File OK!" + csFileName);
    MessageBox(csInfo, MB_OK);
  }
}
//////////////////////////////////////////////////////////////////////////
LONG CALLBACK DataOut(LPSTR lpszCode,SHORT nDataType,LPVOID lpData,
                      DWORD cbData,DWORD_PTR dwIoParam,DWORD dwReserved) {
                          UNREFERENCED_PARAMETER( lpszCode);
                          UNREFERENCED_PARAMETER( nDataType);
                          UNREFERENCED_PARAMETER( lpData);
                          UNREFERENCED_PARAMETER( cbData);
                          UNREFERENCED_PARAMETER( dwIoParam);
                          UNREFERENCED_PARAMETER( dwReserved);
                          return 0;
}

//////////////////////////////////////////////////////////////////////////
bool_t CExportAQDictDlg::loadDict(uint32_t idx, const CString& csPath) {

  if (idx >= DICT_NUM) { return FALSE; }

  typedef BOOL	(*WT_INITINST)(LPFEMPARAM);
  typedef VOID	(*WT_EXITINST)();
typedef DWORD	(*WT_GETDICTS)(DWORD,DWORD,LPBYTE);

  int nStart = 0;
  int nLastDir = 0;
  while (-1 != (nStart = csPath.Find('\\', nStart))) { nLastDir = nStart; ++nStart; }
  if (0 == nLastDir) { return FALSE; }
  
  CString csDir = csPath.Left(nLastDir) + "\\";
  CString csName = csPath.Right(csPath.GetLength() - nLastDir - 1);

  DynamicLibrary dync_lib(csName, csDir);
  if (FALSE == dync_lib.isValid()) {

    CString csInfo("LoadLibrary Failed!" + csPath);
    MessageBox(csInfo, MB_OK);
    return FALSE;
  }

  // get func
  WT_INITINST pfn_InitInst = (WT_INITINST)dync_lib.LocateSymbol("FEM_InitInst");  
  WT_EXITINST pfn_ExitInst = (WT_EXITINST)dync_lib.LocateSymbol("FEM_ExitInst");  
  WT_GETDICTS pfn_GetDicts = (WT_GETDICTS)dync_lib.LocateSymbol("FEM_GetDicts");

  if (NULL == pfn_InitInst || NULL == pfn_ExitInst || NULL == pfn_GetDicts) {

    CString csInfo("GetProcAddress Failed! FEM_InitInst, FEM_ExitInst, FEM_GetDicts");
    MessageBox(csInfo, MB_OK);
    return FALSE;
  }

  nStart = 0;
  int nLastDot = 0;
  while (-1 != (nStart = csName.Find('.', nStart))) { nLastDot = nStart; ++nStart; }
  if (0 == nLastDot) { return FALSE; }

  CString csTc50(csDir + "..\\..\\");
  CString csTc50Ini(csTc50 + "configs\\tdxtc50.ini");

  CString strININame(csDir + csName.Left(nLastDot) + ".ini");

  // 注意: 此处获取第一个营业部作为默认营业部以简化处理
  // 当配置多个方案,分营业部的时候,可能还是无法
  // 区分普通和信用加载的字典
  int nBranchNum;
  nBranchNum = GetPrivateProfileInt("REAL_BRANCH", "BRANCH_NUM", 0, csTc50Ini);
  //char szKey[32] = {0}, szLine[1024] = {0}, szBranchID[32] = {0};
  char szKey[32] = {0}, szLine[1024] = {0};
  DWORD dwBranchID = 1;
  for (int i=0; i<nBranchNum; i++) {

    CString	strValue = "";
    _snprintf(szKey, sizeof(szKey), "BRANCH_%04d", i+1);
    GetPrivateProfileString("REAL_BRANCH", szKey, "", szLine, sizeof(szLine), csTc50Ini);
    /*
    GetStr(szLine, szBranchID, sizeof(szBranchID), 1, ',');
    strValue = szBranchID;
    strValue.TrimLeft('R');
    _snprintf(szBranchID, sizeof(szBranchID), "%s", strValue);
    dwBranchID = (DWORD)atol(szBranchID);
    */
    dwBranchID = 'R' == szLine[0] ? (DWORD)atol(szLine + 1) : (DWORD)atol(szLine);
    break;
	}

  // 参数设置
  FEMBRANCHINFO stFemBranchInfo;
  stFemBranchInfo.m_dwBranchID = dwBranchID;
  stFemBranchInfo.m_dwMaxThreadNum = 100;
  stFemBranchInfo.m_dwMinThreadNum = 1;
  _snprintf( stFemBranchInfo.m_szBranchSection, sizeof(stFemBranchInfo.m_szBranchSection), "%s", "STAGE_默认方案" );
  FEMPARAM FEMPARAMTemp;
  memset( &FEMPARAMTemp, 0, sizeof(FEMPARAM) );
  
  FEMPARAMTemp.m_pfnDataIO = DataOut;
  _snprintf( FEMPARAMTemp.m_szOptionFile, sizeof(FEMPARAMTemp.m_szOptionFile), "%s", strININame );
  _snprintf( FEMPARAMTemp.m_szPublicSection, sizeof(FEMPARAMTemp.m_szPublicSection), "%s", "PUBLIC" );
  FEMPARAMTemp.m_dwMaxAnsBufLen = 64*1024;
  FEMPARAMTemp.m_dwBranchNum = nBranchNum;
	FEMPARAMTemp.m_lpBranchInfos = &stFemBranchInfo;
  
  bool_t bRet = pfn_InitInst(&FEMPARAMTemp);
  if (FALSE == bRet) {
    CString csInfo("Init Failed!" + csPath);
    MessageBox(csInfo, MB_OK);
    return FALSE;
  }

  uint32_t nDiceLen = 0;
  uint32_t nFreeLen = ALL_DICT_DATA_LEN - DICT_LEN_LEN;
  LPBYTE lpBuffer = m_cbDicData[idx] + DICT_LEN_LEN;
  nDiceLen = pfn_GetDicts(DICTTYPE_SIMPLE, nFreeLen, lpBuffer);
  if (0 == nDiceLen) {
    CString csInfo("Get Dict Failed! Simple");
    MessageBox(csInfo, MB_OK);
  }
  *((uint32_t *)(lpBuffer - DICT_LEN_LEN)) = nDiceLen;

  nFreeLen = ALL_DICT_DATA_LEN - 2 * DICT_LEN_LEN - nDiceLen;
  lpBuffer = m_cbDicData[idx] + DICT_LEN_LEN + nDiceLen + DICT_LEN_LEN;
  nDiceLen = pfn_GetDicts(DICTTYPE_FULL, nFreeLen, lpBuffer);
  if (0 == nDiceLen) {
    CString csInfo("Get Dict Failed! Full");
    MessageBox(csInfo, MB_OK);
  }
  *((uint32_t *)(lpBuffer - DICT_LEN_LEN)) = nDiceLen;

  pfn_ExitInst();
  return TRUE;
}

CString CExportAQDictDlg::getOpenFilePath() {

  CFileDialog dlg(TRUE,//TRUE是创建打开文件对话框，FALSE则创建的是保存文件对话框
    ".txt",//默认的打开文件的类型
    NULL,//默认打开的文件名
    OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,//打开只读文件
    "(*.dll)|*.dll|");//所有可以打开的文件类型
  
  if(dlg.DoModal() !=IDOK) { return ""; }
  return dlg.GetPathName();////////取出文件路径  
}

void CExportAQDictDlg::OnOpenDict1() {
  
  CString csFileName = getOpenFilePath();
  if (TRUE == loadDict(0, csFileName)) {
    GetDlgItem(IDC_CHECK1_S)->EnableWindow(TRUE);
    GetDlgItem(IDC_CHECK1_C)->EnableWindow(TRUE);
    GetDlgItem(IDC_DICT_NAME1)->SetWindowText(csFileName);
  }
  else {
    GetDlgItem(IDC_CHECK1_S)->EnableWindow(FALSE);
    GetDlgItem(IDC_CHECK1_C)->EnableWindow(FALSE);
    GetDlgItem(IDC_DICT_NAME1)->SetWindowText("");
  }
}

void CExportAQDictDlg::OnOpenDict2() {

  CString csFileName = getOpenFilePath();
  if (TRUE == loadDict(1, csFileName)) {
    GetDlgItem(IDC_CHECK2_S)->EnableWindow(TRUE);
    GetDlgItem(IDC_CHECK2_C)->EnableWindow(TRUE);
    GetDlgItem(IDC_DICT_NAME2)->SetWindowText(csFileName);
  }
  else {
    GetDlgItem(IDC_CHECK2_S)->EnableWindow(FALSE);
    GetDlgItem(IDC_CHECK2_C)->EnableWindow(FALSE);
    GetDlgItem(IDC_DICT_NAME2)->SetWindowText("");
  }
}

void CExportAQDictDlg::OnOpenDict3() {

  CString csFileName = getOpenFilePath();
  if (TRUE == loadDict(2, csFileName)) {
    GetDlgItem(IDC_CHECK3_S)->EnableWindow(TRUE);
    GetDlgItem(IDC_CHECK3_C)->EnableWindow(TRUE);
    GetDlgItem(IDC_DICT_NAME3)->SetWindowText(csFileName);
  }
  else {
    GetDlgItem(IDC_CHECK3_S)->EnableWindow(FALSE);
    GetDlgItem(IDC_CHECK3_C)->EnableWindow(FALSE);
    GetDlgItem(IDC_DICT_NAME3)->SetWindowText("");
  }
}

//////////////////////////////////////////////////////////////////////////
void CExportAQDictDlg::OnCheck1C() {
  ((CButton*)GetDlgItem(IDC_CHECK2_C))->SetCheck(FALSE);
  ((CButton*)GetDlgItem(IDC_CHECK3_C))->SetCheck(FALSE);
}

void CExportAQDictDlg::OnCheck1S() {
  ((CButton*)GetDlgItem(IDC_CHECK2_S))->SetCheck(FALSE);
  ((CButton*)GetDlgItem(IDC_CHECK3_S))->SetCheck(FALSE);
}

void CExportAQDictDlg::OnCheck1Sc() {
  ((CButton*)GetDlgItem(IDC_CHECK2_SC))->SetCheck(FALSE);
  ((CButton*)GetDlgItem(IDC_CHECK3_SC))->SetCheck(FALSE);
}

void CExportAQDictDlg::OnCheck2C() {
  ((CButton*)GetDlgItem(IDC_CHECK1_C))->SetCheck(FALSE);
  ((CButton*)GetDlgItem(IDC_CHECK3_C))->SetCheck(FALSE);
}

void CExportAQDictDlg::OnCheck2S() {
  ((CButton*)GetDlgItem(IDC_CHECK1_S))->SetCheck(FALSE);
  ((CButton*)GetDlgItem(IDC_CHECK3_S))->SetCheck(FALSE);
}

void CExportAQDictDlg::OnCheck2Sc() {
  ((CButton*)GetDlgItem(IDC_CHECK1_SC))->SetCheck(FALSE);
  ((CButton*)GetDlgItem(IDC_CHECK3_SC))->SetCheck(FALSE);
}

void CExportAQDictDlg::OnCheck3C() {
  ((CButton*)GetDlgItem(IDC_CHECK1_C))->SetCheck(FALSE);
  ((CButton*)GetDlgItem(IDC_CHECK2_C))->SetCheck(FALSE);
}

void CExportAQDictDlg::OnCheck3S() {
  ((CButton*)GetDlgItem(IDC_CHECK1_S))->SetCheck(FALSE);
  ((CButton*)GetDlgItem(IDC_CHECK2_S))->SetCheck(FALSE);
}

void CExportAQDictDlg::OnCheck3Sc() {
  ((CButton*)GetDlgItem(IDC_CHECK1_SC))->SetCheck(FALSE);
  ((CButton*)GetDlgItem(IDC_CHECK2_SC))->SetCheck(FALSE);
}
