#pragma once

#include "ResizableDialog.h"

class CSPdlg;

// CInfoDlg dialog

class CInfoDlg : public CResizableDialog
{
	DECLARE_DYNAMIC(CInfoDlg)

public:
	CInfoDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CInfoDlg();

// Dialog Data
	enum { IDD = IDD_INFOWND };

	virtual BOOL Create(LPCTSTR lpszTemplateName, CWnd* pParentWnd = NULL);
	BOOL Create(CSPdlg * pSPdlg);

	afx_msg void OnCancel();
	afx_msg void OnClose();
	afx_msg void OnSize(UINT nType, int cx, int cy);

	virtual BOOL OnInitDialog();

	CEdit m_cInfoBox;
	CRect m_dInfoWndSize;
	CRect m_dInfoBoxSize;
	CString m_cInfoStr;
	CSPdlg * m_pSPdlg;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

	CWnd *m_pParentWnd;
	int m_nID;
public:
	afx_msg void OnBnClickedClear();
};
