#pragma once

#include "ResizableDialog.h"
#include "afxcmn.h"
#include "browsectrl.h"
#include "afxwin.h"

class CSPdlg;

// CStructDlg dialog

class CStructDlg : public CResizableDialog
{
	DECLARE_DYNAMIC(CStructDlg)

public:
	CStructDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CStructDlg();

// Dialog Data
	enum { IDD = IDD_STRCTDLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

	CWnd *m_pParentWnd;
	int m_nID;
public:
	virtual BOOL Create(LPCTSTR lpszTemplateName, CWnd* pParentWnd = NULL);
	BOOL Create(CSPdlg *pSPdlg);
	afx_msg void OnClose();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void LoadStruct();
	afx_msg void OnNcDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);

	CSPdlg * m_pSPdlg;
	CTreeCtrl m_cStrctTree;
protected:
	virtual void PostNcDestroy();
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedLoadstrct1();
	CBrowseCtrl m_cInputFile;
	void PopulateStrctTree(void);
	CEdit m_cStructInfoBox;
	CString m_cStructInfoStr;
	int PopulateStrctInfoBox(struct global_type *pGlobals);
};
