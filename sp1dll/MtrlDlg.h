#pragma once

#include "ResizableDialog.h"
#include "matrls.h"
#include "browsectrl.h"
#include "mtrltbldlg.h"
#include "spdlg.h"

//class CSPdlg;

// CMtrlDlg dialog

class CMtrlDlg : public CResizableDialog
{
	DECLARE_DYNAMIC(CMtrlDlg)

public:
	CMtrlDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CMtrlDlg();

// Dialog Data
	enum { IDD = IDD_MTRLDLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

	virtual void PostNcDestroy();
	virtual void OnOK();

	CWnd *m_pParentWnd;
	int m_nID;
public:
	virtual BOOL Create(LPCTSTR lpszTemplateName, CWnd* pParentWnd = NULL);
	BOOL Create(CSPdlg *pSPdlg);

	virtual BOOL OnInitDialog();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnClose();
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	afx_msg void OnReloadMtrls();
	afx_msg void OnMtrlEval();

	CSPdlg * m_pSPdlg;
	CMtrlTblDlg * m_pMtrlTblDlg;
	CBrowseCtrl m_mtrlFile;
	CTreeCtrl m_mtrlTree;
	CMatrls m_mtrlObj;
	CString m_sFileID;
	double m_nTmprt;
	double m_nAlloyComp;
protected:
	virtual void OnCancel();
public:
	afx_msg void OnEnChangeAlloycomp();
	afx_msg void OnViewMtrlTbl();
};
