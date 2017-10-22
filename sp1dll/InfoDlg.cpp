// InfoDlg.cpp : implementation file
//

#include "stdafx.h"
#include "sp1dll.h"
#include "InfoDlg.h"
#include ".\infodlg.h"
#include "spdlg.h"


// CInfoDlg dialog

IMPLEMENT_DYNAMIC(CInfoDlg, CResizableDialog)
CInfoDlg::CInfoDlg(CWnd* pParent /*=NULL*/)
	: CResizableDialog(CInfoDlg::IDD, pParent)
	, m_pParentWnd(NULL)
	, m_cInfoStr(_T(""))
{
	ASSERT(pParent != NULL);

	m_pParentWnd = pParent;
	m_nID = CInfoDlg::IDD;
}

CInfoDlg::~CInfoDlg()
{
}

void CInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_INFOBOX, m_cInfoBox);
	DDX_Text(pDX, IDC_INFOBOX, m_cInfoStr);
}


BEGIN_MESSAGE_MAP(CInfoDlg, CDialog)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_WM_CLOSE()
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_CLEAR, &CInfoDlg::OnBnClickedClear)
END_MESSAGE_MAP()


// CInfoDlg message handlers

BOOL CInfoDlg::Create(LPCTSTR lpszTemplateName, CWnd* pParentWnd)
{
	// TODO: Add your specialized code here and/or call the base class

	return CDialog::Create(lpszTemplateName, pParentWnd);
}

BOOL CInfoDlg::Create(CSPdlg *pSPdlg)
{
	m_pSPdlg = pSPdlg;

	return CDialog::Create(m_nID, m_pParentWnd);
}

void CInfoDlg::OnCancel()
{
	// TODO: Add your control notification handler code here
	DestroyWindow();
	m_pSPdlg->CloseInfoDlg();

	//CResizableDialog::OnCancel(); if you leave it in then it causes a crash
}

void CInfoDlg::OnClose()
{
	// TODO: Add your message handler code here and/or call default
	DestroyWindow();
	m_pSPdlg->CloseInfoDlg();

	//CResizableDialog::OnClose();
}


BOOL CInfoDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	GetWindowRect(m_dInfoWndSize);
	m_cInfoBox.GetWindowRect(m_dInfoBoxSize);

	// preset layout
	AddAnchor(IDC_INFOBOX, TOP_LEFT, BOTTOM_RIGHT);
	AddAnchor(IDC_CLEAR, TOP_LEFT);
	AddAnchor(IDCANCEL, TOP_RIGHT);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CInfoDlg::OnSize(UINT nType, int cx, int cy)
{
	CResizableDialog::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here
}

void CInfoDlg::OnBnClickedClear()
{
	m_cInfoBox.SetWindowText("");
}
