#pragma once


// CErgDlg dialog

class CErgDlg : public CDialog
{
	DECLARE_DYNAMIC(CErgDlg)

public:
	CErgDlg(CWnd* pParent = NULL, struct global_type *pGlobals = NULL, CString caption = "");   // standard constructor
	virtual ~CErgDlg();

// Dialog Data
	enum { IDD = IDD_ERGDLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CString m_ergLower;
	CString m_ergUpper;
	CString m_sCaption;
	struct global_type *m_pGlobals;

	afx_msg void OnBnClickedOk();
	virtual BOOL OnInitDialog();
};
