// ErgDlg.cpp : implementation file
//

#include "stdafx.h"
#include "sp1dll.h"
#include "ErgDlg.h"
#include ".\poisson\poisp_types.h"
#include ".\ergdlg.h"


// CErgDlg dialog

IMPLEMENT_DYNAMIC(CErgDlg, CDialog)
CErgDlg::CErgDlg(CWnd* pParent /*=NULL*/, struct global_type *pGlobals, CString caption)
	: CDialog(CErgDlg::IDD, pParent)
{
	m_pGlobals = pGlobals;
	m_sCaption = caption;
}

CErgDlg::~CErgDlg()
{
}

void CErgDlg::DoDataExchange(CDataExchange* pDX)
{
	DDX_Text(pDX, IDC_ERGLOWER, m_ergLower);
	DDX_Text(pDX, IDC_ERGUPPER, m_ergUpper);

	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CErgDlg, CDialog)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
END_MESSAGE_MAP()


// CErgDlg message handlers

void CErgDlg::OnBnClickedOk()
{
	UpdateData(true);
	m_pGlobals->LB = atof(m_ergLower);
	m_pGlobals->UB = atof(m_ergUpper);

	OnOK();
}

BOOL CErgDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	SetWindowText(m_sCaption);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
