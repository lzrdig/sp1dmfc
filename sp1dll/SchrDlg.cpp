// SchrDlg.cpp : implementation file
//

#include "stdafx.h"
#include "sp1dll.h"
#include "SchrDlg.h"
#include ".\schrdlg.h"
#include ".\poisson\poisp_types.h"


// CSchrDlg dialog

IMPLEMENT_DYNAMIC(CSchrDlg, CDialog)
CSchrDlg::CSchrDlg(CWnd* pParent /*=NULL*/, struct global_type *pGlobals)
	: CDialog(CSchrDlg::IDD, pParent)
	//, m_schrStart(0)
	//, m_schrStop(0)
{
	m_pGlobals = pGlobals;
}

CSchrDlg::~CSchrDlg()
{
}

void CSchrDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_SCHRSTART, m_schrStart);
	DDX_Text(pDX, IDC_SCHRSTOP, m_schrStop);
}


BEGIN_MESSAGE_MAP(CSchrDlg, CDialog)
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
END_MESSAGE_MAP()


// CSchrDlg message handlers

void CSchrDlg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	UpdateData(true);
	m_pGlobals->realschstart = m_schrStart;
	m_pGlobals->realschstop = m_schrStop;

	OnCancel();
}
