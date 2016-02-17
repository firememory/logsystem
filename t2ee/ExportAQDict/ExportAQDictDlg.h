// ExportAQDictDlg.h : header file
//

#if !defined(AFX_EXPORTAQDICTDLG_H__6E5BD6C3_472E_486C_9754_B53E90B121C0__INCLUDED_)
#define AFX_EXPORTAQDICTDLG_H__6E5BD6C3_472E_486C_9754_B53E90B121C0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CExportAQDictDlg dialog

class CExportAQDictDlg : public CDialog
{
// Construction
public:
	CExportAQDictDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CExportAQDictDlg)
	enum { IDD = IDD_EXPORTAQDICT_DIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CExportAQDictDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CExportAQDictDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	virtual void OnOK();
	afx_msg void OnOpenDict1();
	afx_msg void OnOpenDict2();
	afx_msg void OnOpenDict3();
	afx_msg void OnCheck1C();
	afx_msg void OnCheck1S();
	afx_msg void OnCheck1Sc();
	afx_msg void OnCheck2C();
	afx_msg void OnCheck2S();
	afx_msg void OnCheck2Sc();
	afx_msg void OnCheck3C();
	afx_msg void OnCheck3S();
	afx_msg void OnCheck3Sc();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()


private:

  enum { DICT_NUM = 3, DICT_LEN_LEN = 4, ALL_DICT_DATA_LEN = 256*1024 };
	BYTE  m_cbDicData[DICT_NUM][ALL_DICT_DATA_LEN];
  
private:
  bool_t loadDict(uint32_t idx, const CString& csPath);
  CString getOpenFilePath();
  
private:
  CString m_csLocalPath;
};


#define STR_VERSION           "v1.0.0.827"

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EXPORTAQDICTDLG_H__6E5BD6C3_472E_486C_9754_B53E90B121C0__INCLUDED_)
