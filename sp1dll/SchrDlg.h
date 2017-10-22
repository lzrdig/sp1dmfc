#pragma once


// CSchrDlg dialog

class CSchrDlg : public CDialog
{
	DECLARE_DYNAMIC(CSchrDlg)

public:
	CSchrDlg(CWnd* pParent = NULL, struct global_type *pGlobals = NULL);   // standard constructor
	virtual ~CSchrDlg();

// Dialog Data
	enum { IDD = IDD_SCHRDLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	double m_schrStart;
	double m_schrStop;
	struct global_type *m_pGlobals;

	afx_msg void OnBnClickedCancel();
};
