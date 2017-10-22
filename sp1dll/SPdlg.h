#pragma once
#include "resource.h"
#include "stdafx.h"
#include "browsectrl.h"
#include "colorBtn.h"
#include "infodlg.h"
#include "spgrph.h"
#include "mtrldlg.h"
#include "StructDlg.h"


// CSPdlg dialog

class CSPdlg : public CDialog
{
	DECLARE_DYNAMIC(CSPdlg)

public:
	CSPdlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSPdlg();

// Dialog Data
	enum { IDD = IDD_SPDIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CBrowseCtrl m_inputFile;
	CBrowseCtrl m_mtrlFile;
	CBrowseCtrl m_workDir;
	CColorButton m_mtrLoadInd;
	CString m_sInfoStr;
	CInfoDlg * m_pInfoDlg;
	CMtrlDlg * m_pMtrlDlg;
	CGrphDlg * m_pGrphDlg;
	CStructDlg * m_pStrctDlg;

	virtual BOOL OnInitDialog();

	afx_msg void OnLoadMatrls();
	afx_msg void OnRunSP();
	afx_msg void OnCancel();
	afx_msg void OnViewOutput();
	afx_msg void OnViewMatrls();
	afx_msg void OnShowInfo();
	afx_msg LRESULT OnFileCtrlNotify(WPARAM wParam, LPARAM lParam);
	afx_msg void OnClrGrphPaths();
	afx_msg void OnViewStruct();

	int CloseInfoDlg(void);
	int CloseGrphDlg(void);
	int CloseMtrlDlg(void);
	int CloseStrctDlg(void);
};
