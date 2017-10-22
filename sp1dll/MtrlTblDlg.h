#pragma once
#include "afxcmn.h"


// CMtrlTblDlg dialog

class CMtrlTblDlg : public CDialog
{
	DECLARE_DYNAMIC(CMtrlTblDlg)

public:
	CMtrlTblDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CMtrlTblDlg();

	virtual BOOL Create(LPCTSTR lpszTemplateName, CWnd* pParentWnd = NULL);
	BOOL Create(CMtrlDlg * pMtrlDlg);

// Dialog Data
	enum { IDD = IDD_MTRLTBL };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CListCtrl m_mtrlListTbl;
	CWnd *m_pParentWnd;
	CMtrlDlg * m_pMtrlDlg;
	int m_nID;
	void ModifyHeaderItems();
	void FillMtrlTbl();
	virtual BOOL OnInitDialog();
	CString m_sTmprt;
	CString m_sXcomp;
	bool m_bEval;
	afx_msg void OnEvalMtrls();
};
