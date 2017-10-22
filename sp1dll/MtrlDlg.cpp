// MtrlDlg.cpp : implementation file
//

#include "stdafx.h"
#include "sp1dll.h"
#include "MtrlDlg.h"
#include ".\mtrldlg.h"
#include "poisp_types.h"

extern char g_workDir[MAX_FILENAME];
extern char g_matrlFname[MAX_FILENAME];
extern CSchrPois theSP;

// CMtrlDlg dialog

IMPLEMENT_DYNAMIC(CMtrlDlg, CResizableDialog)
CMtrlDlg::CMtrlDlg(CWnd* pParent /*=NULL*/)
	: CResizableDialog(CMtrlDlg::IDD, pParent)
	, m_pParentWnd(NULL)
	, m_sFileID(_T("beta6"))
	, m_nTmprt(0.0)
	, m_nAlloyComp(0.1)
{
	ASSERT(pParent != NULL);

	m_pParentWnd = pParent;
	m_nID = CMtrlDlg::IDD;

	m_pMtrlTblDlg = NULL;
}

CMtrlDlg::~CMtrlDlg()
{
}

void CMtrlDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_MTRLFILE, m_mtrlFile);
	DDX_Control(pDX, IDC_MTRLIST, m_mtrlTree);
	DDX_Text(pDX, IDC_MTRLFILEID, m_sFileID);
	DDX_Text(pDX, IDC_TMPRT, m_nTmprt);
	DDX_Text(pDX, IDC_ALLOYCOMP, m_nAlloyComp);
}

BOOL CMtrlDlg::Create(LPCTSTR lpszTemplateName, CWnd* pParentWnd)
{
	// TODO: Add your specialized code here and/or call the base class

	return CDialog::Create(lpszTemplateName, pParentWnd);
}

BOOL CMtrlDlg::Create(CSPdlg *pSPdlg)
{
	m_pSPdlg = pSPdlg;

	return CDialog::Create(m_nID, m_pParentWnd);
}


BEGIN_MESSAGE_MAP(CMtrlDlg, CDialog)
	ON_WM_SIZE()
	ON_WM_CLOSE()
	ON_WM_CREATE()
	ON_WM_GETMINMAXINFO()
	ON_BN_CLICKED(IDC_RELOADMTRL, OnReloadMtrls)
	ON_BN_CLICKED(IDC_MTRLEVAL, OnMtrlEval)
	ON_BN_CLICKED(IDC_EXIT, OnOK)
	ON_EN_CHANGE(IDC_ALLOYCOMP, &CMtrlDlg::OnEnChangeAlloycomp)
	ON_BN_CLICKED(IDC_VIEWMTRLTBL, &CMtrlDlg::OnViewMtrlTbl)
END_MESSAGE_MAP()


// CMtrlDlg message handlers


int CMtrlDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CResizableDialog::OnCreate(lpCreateStruct) == -1)
		return -1;

	return 0;
}

void CMtrlDlg::OnSize(UINT nType, int cx, int cy)
{
	CResizableDialog::OnSize(nType, cx, cy);

	ArrangeLayout();
}

void CMtrlDlg::OnClose()
{
	m_pSPdlg->m_mtrlFile.SetPathName(m_mtrlFile.GetPathName().GetBuffer());
	m_pSPdlg->CloseMtrlDlg();
	DestroyWindow();

	//CDialog::OnClose();
}

BOOL CMtrlDlg::OnInitDialog()
{
	DWORD curStyle;

	CResizableDialog::OnInitDialog();

	// preset layout
	AddAnchor(IDC_MTRLIST, TOP_LEFT, BOTTOM_RIGHT);
	AddAnchor(IDC_MTRLFILE, TOP_LEFT);
	AddAnchor(IDC_EXIT, TOP_RIGHT);
	AddAnchor(IDC_MTRLEVAL, TOP_RIGHT);
	AddAnchor(IDC_ENVPARAMS, TOP_LEFT);
	AddAnchor(IDC_TMPRT, TOP_LEFT);
	AddAnchor(IDC_ALLOYCOMP, TOP_LEFT);
	AddAnchor(IDC_TMPRTSPIN, TOP_LEFT);
	AddAnchor(IDC_ALLOYCOMPSPIN, TOP_LEFT);
	AddAnchor(IDC_TMPRTLBL, TOP_LEFT);
	AddAnchor(IDC_ALLOYCOMPLBL, TOP_LEFT);
	AddAnchor(IDC_RELOADMTRL, TOP_RIGHT);

	curStyle = m_mtrlFile.GetButtonStyle();
	curStyle |= BC_CTL_ALLOWEDIT;
	m_mtrlFile.SetButtonStyle(curStyle);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CMtrlDlg::PostNcDestroy()
{
	// TODO: Add your specialized code here and/or call the base class
	delete this;

	//CResizableDialog::PostNcDestroy();
}

void CMtrlDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	// TODO: Add your message handler code here and/or call default
	lpMMI->ptMinTrackSize.x = 500;
	lpMMI->ptMinTrackSize.y = 400;

	//CResizableDialog::OnGetMinMaxInfo(lpMMI);
}

void CMtrlDlg::OnReloadMtrls()
{
	//char access,extension[MAX_EXTENSION];
	int loadResult = 1;

	CString curMtrlFilePath = m_pSPdlg->m_mtrlFile.GetPathName();
	CString curMtrlFileName = "";
	CString s_fext;

	strcpy(m_mtrlObj.materialsversion, m_sFileID.GetBuffer());

	if((curMtrlFilePath != m_mtrlFile.GetPathName())||(curMtrlFilePath == ""))
	{
		loadResult = theSP.LoadMatrlFile("");
		s_fext = "txt";
	}
	else
	{
		curMtrlFileName = m_mtrlFile.GetPathName();
		curMtrlFileName.Delete(curMtrlFileName.GetLength()-4,4);
		strcpy(m_mtrlObj.m_sMtrlFileName, curMtrlFilePath.GetBuffer());
		loadResult = theSP.LoadMatrlFile(curMtrlFileName.GetBuffer());
		s_fext = m_pSPdlg->m_mtrlFile.GetFileExt();
	}
	if(!loadResult){
		curMtrlFileName = theSP.filePath;
		m_mtrlObj.LoadMtrls(curMtrlFileName.GetBuffer());
		m_mtrlObj.PopulateMtrlsTree(&m_mtrlTree, m_nTmprt, m_nAlloyComp, false);
		m_nTmprt = 300.0;
		m_nAlloyComp = 0.1;
		UpdateData(false);

		m_mtrlFile.SetPathName(curMtrlFileName + "." + s_fext);
		strcpy(g_matrlFname,m_mtrlObj.m_sMtrlFileName);
		m_pSPdlg->m_mtrLoadInd.SetState(true);
		m_pSPdlg->m_mtrLoadInd.SetWindowText(_T("Materials loaded"));
	}
	else{
		m_pSPdlg->m_mtrLoadInd.SetState(false);
		m_pSPdlg->m_mtrLoadInd.SetWindowText(_T("Materials not loaded"));
	}
}

void CMtrlDlg::OnMtrlEval()
{
	//char message[MAX_MESSAGE];
	bool evaluate;
		
	if(!(m_pSPdlg->m_mtrLoadInd.GetState() & 0x0003))
	{
		evaluate = true;

		UpdateData(true);

		m_mtrlObj.PopulateMtrlsTree(&m_mtrlTree, m_nTmprt, m_nAlloyComp, evaluate);
	}
}

void CMtrlDlg::OnOK()
{
	m_pSPdlg->m_mtrlFile.SetPathName(m_mtrlFile.GetPathName().GetBuffer());
	m_pSPdlg->CloseMtrlDlg();
	DestroyWindow();

	//CResizableDialog::OnOK();
}

void CMtrlDlg::OnCancel()
{
	// Default OnOK is overwritten in order to make the dialog modeless
	m_pSPdlg->m_mtrlFile.SetPathName(m_mtrlFile.GetPathName().GetBuffer());
	m_pSPdlg->CloseMtrlDlg();
	DestroyWindow();

	//CResizableDialog::OnCancel();
}

void CMtrlDlg::OnEnChangeAlloycomp()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CResizableDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	bool evaluate = false;

	//if(!(m_pSPdlg->m_mtrLoadInd.GetState() & 0x0003)) {	evaluate = true; }
	//else { evaluate = false; }

	UpdateData(true);

	m_mtrlObj.PopulateMtrlsTree(&m_mtrlTree, m_nTmprt, m_nAlloyComp, evaluate);
}

void CMtrlDlg::OnViewMtrlTbl()
{
	if (m_pMtrlTblDlg == NULL)
	{
		m_pMtrlTblDlg = new CMtrlTblDlg(GetDesktopWindow());
		if (m_pMtrlTblDlg->Create(this) == TRUE)
		{
		}
	}
	else
	{
		m_pMtrlTblDlg->ShowWindow(SW_SHOW);
		m_pMtrlTblDlg->SetActiveWindow();
	}
}
