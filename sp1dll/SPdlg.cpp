// SPdlg.cpp : implementation file
//

#include "stdafx.h"
#include "sp1dll.h"
#include "poisp_types.h"
#include "SchrPois.h"
#include "SPdlg.h"
#include "StructDlg.h"


extern CSchrPois theSP;
extern char g_matrlFname[MAX_FILENAME];
extern char g_inputFname[MAX_FILENAME];
extern char g_workDir[MAX_FILENAME];

#define WM_BROWSECTRL_NOTIFY	WM_APP + 100	// used by CBrowseCtrl objects to send notifications to parent app/dlg

// CSPdlg dialog

IMPLEMENT_DYNAMIC(CSPdlg, CDialog)
CSPdlg::CSPdlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSPdlg::IDD, pParent)
{
	m_pInfoDlg = NULL;
	m_pGrphDlg = NULL;
	m_pMtrlDlg = NULL;
	m_pStrctDlg = NULL;
}

CSPdlg::~CSPdlg()
{
}

void CSPdlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_INPUTF, m_inputFile);
	DDX_Control(pDX, IDC_MATRLF, m_mtrlFile);
	DDX_Control(pDX, IDC_WORKDIR, m_workDir);
	DDX_Control(pDX, IDC_MTRLFILEID, m_mtrLoadInd);
}


BEGIN_MESSAGE_MAP(CSPdlg, CDialog)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_BN_CLICKED(IDC_RUNSP, OnRunSP)
	ON_BN_CLICKED(IDC_LOADMATRLS, OnLoadMatrls)
	ON_BN_CLICKED(IDC_SHOWINFO, OnShowInfo)
	ON_BN_CLICKED(IDC_VIEWOUTPUT, OnViewOutput)
	ON_BN_CLICKED(IDC_VIEWMATRLS, OnViewMatrls)
	ON_MESSAGE(WM_BROWSECTRL_NOTIFY, OnFileCtrlNotify)
	ON_BN_CLICKED(IDC_CLRPATHS, &CSPdlg::OnClrGrphPaths)
	ON_BN_CLICKED(IDC_VIEWSTRUCT, &CSPdlg::OnViewStruct)
END_MESSAGE_MAP()


// CSPdlg message handlers
BOOL CSPdlg::OnInitDialog()
{
	//char curDir[MAX_FILENAME];
	DWORD bufLen = 0;
	DWORD curStyle;

	CDialog::OnInitDialog();

	//m_mtrLoadInd.Attach(IDC_MTRLFILEID, this); // CCT

	GetCurrentDirectory(MAX_FILENAME, g_workDir);

	curStyle = m_inputFile.GetButtonStyle();
	curStyle |= BC_CTL_ALLOWEDIT;
	m_inputFile.SetButtonStyle(curStyle);
	m_inputFile.SetPathName(g_inputFname);

	curStyle = m_mtrlFile.GetButtonStyle();
	curStyle |= BC_CTL_ALLOWEDIT;
	m_mtrlFile.SetButtonStyle(curStyle);
	m_mtrlFile.SetPathName(g_matrlFname);

	curStyle = m_workDir.GetButtonStyle();
	curStyle |= BC_CTL_ALLOWEDIT | BC_CTL_FOLDERSONLY;
	m_workDir.SetButtonStyle(curStyle);
	m_workDir.SetPathName(g_workDir);

	return TRUE;  // return TRUE unless you set the focus to a control
}

void CSPdlg::OnCancel()
{
	strcpy(g_matrlFname, m_mtrlFile.GetPathName());
	strcpy(g_inputFname, m_inputFile.GetPathName());
	strcpy(g_workDir, m_workDir.GetPathName());

	CDialog::OnCancel();
}

LRESULT CSPdlg::OnFileCtrlNotify(WPARAM wParam, LPARAM lParam)
{
	// The user just activated and closed the file dialog.
	CBrowseCtrl* pCtrl = (CBrowseCtrl*)lParam; // pointer to the CBrowseCtrl object.
	ASSERT((pCtrl == &m_inputFile)||(pCtrl == &m_mtrlFile)||(pCtrl == &m_workDir));

	if ((int)wParam == IDOK)
	{
		// ***** The file dialog was closed by the "OK" button *****
		// TODO: Add your control notification handler code here
	}
	else
	{
		// ***** The file dialog was closed by the "Cancel" button *****		
		// TODO: Add your control notification handler code here
	}

	pCtrl = NULL; // Appeases VC6 warning level 4.

	return (LRESULT)0;
}


void CSPdlg::OnLoadMatrls()
{
	CString s_fname, s_fext;
	int ioerr = 0;

	s_fname = m_mtrlFile.GetPathName();
	s_fext = m_mtrlFile.GetFileExt();
	s_fname.Delete(s_fname.GetLength() - 4,4);

	ioerr = theSP.LoadMatrlFile(s_fname.GetBuffer());

	if(!ioerr)
	{
		s_fname = theSP.filePath;
		if(s_fext.IsEmpty())	s_fext = _T("txt");
		s_fname = s_fname + "." + s_fext;
		m_mtrlFile.SetPathName(s_fname);

		m_mtrLoadInd.SetWindowText(_T("Materials loaded"));
		m_mtrLoadInd.SetState(true);
		//m_mtrLoadInd.SetColor(BLACK, GREEN);

		if(m_pInfoDlg != NULL)
		{
			theSP.PostInfo(&(m_pInfoDlg->m_cInfoBox), _T("Materials have been loaded"));
			//theSP.PostInfo(&(m_pInfoDlg->m_cInfoBox), _T("The number of materials is: "));
		}
	}
	else
	{
		m_mtrLoadInd.SetWindowText(_T("Materials not loaded"));
		m_mtrLoadInd.SetState(false);

		if(m_pInfoDlg != NULL)
		{
			theSP.PostInfo(&(m_pInfoDlg->m_cInfoBox), _T("Failed to load materials file."));
		}
	}
}



void CSPdlg::OnRunSP()
{
	CString s_fname, s_fext, s_matfname;
	int ioerr = 0;

	s_fname = m_inputFile.GetPathName();
	s_fext = m_inputFile.GetFileExt();
	s_fname.Delete(s_fname.GetLength() - 4,4);
	strcpy((theSP.pGlobals)->filename, s_fname.GetBuffer());

	if(m_mtrLoadInd.GetState() == false)
	{
		ioerr = theSP.LoadMatrlFile(m_mtrlFile.GetPathName().GetBuffer());

		if(!ioerr)
		{
			s_matfname = theSP.filePath;
			s_matfname = s_matfname + "." + "txt";
			m_mtrlFile.SetPathName(s_matfname);

			//update material load state indicator
			m_mtrLoadInd.SetWindowText(_T("Materials loaded"));
			m_mtrLoadInd.SetState(true);

			// Run 1D SP calculation
			theSP.selrun(theSP.pGlobals, theSP.pFlags);

			if(s_fext.IsEmpty())
				s_fext = _T("txt");
			s_fname = theSP.inputFilePath;
			s_fname = s_fname + "." + s_fext;
			m_inputFile.SetPathName(s_fname);
		}
		else
		{
			m_mtrLoadInd.SetWindowText(_T("Materials not loaded"));
			m_mtrLoadInd.SetState(false);

			if(m_pInfoDlg != NULL)
			{
				theSP.PostInfo(&(m_pInfoDlg->m_cInfoBox), 
					_T("Could not start 1D Schrodinger-Poisson calculation."));
				theSP.PostInfo(&(m_pInfoDlg->m_cInfoBox), 
					_T("It seems that the materials file has not been loaded."));
			}
		}
	}
	else
	{
		theSP.selrun(theSP.pGlobals, theSP.pFlags);

		theSP.PostInfoSP(_T("Completed 1D Schrodinger-Poisson calculation."));

		if(s_fext.IsEmpty())
			s_fext = _T("txt");
		s_fname = theSP.inputFilePath;
		s_fname = s_fname + "." + s_fext;
		m_inputFile.SetPathName(s_fname);
	}
}

void CSPdlg::OnShowInfo()
{
	if (m_pInfoDlg == NULL)
	{
		m_pInfoDlg = new CInfoDlg(GetDesktopWindow());
		if (m_pInfoDlg->Create(this) == TRUE)
		{
			//m_pInfoDlg->m_cInfoStr = m_sInfoStr;
			m_pInfoDlg->m_cInfoBox.SetWindowText(m_sInfoStr);
			theSP.SetInfoboxPtr(m_pInfoDlg);
		}
	}
	else
	{
		m_pInfoDlg->ShowWindow(SW_SHOW);
		m_pInfoDlg->SetActiveWindow();
	}

}

void CSPdlg::OnViewOutput()
{
	if (m_pGrphDlg == NULL)
	{
		m_pGrphDlg = new CGrphDlg(GetDesktopWindow());

		m_pGrphDlg->Create(this);
	}
	else
	{
		m_pGrphDlg->ShowWindow(SW_SHOW);
		m_pGrphDlg->SetActiveWindow();
	}
}

void CSPdlg::OnViewMatrls()
{
	CString curFile;

	curFile = m_mtrlFile.GetPathName();

	if (m_pMtrlDlg == NULL)
	{
		m_pMtrlDlg = new CMtrlDlg(GetDesktopWindow());
		m_pMtrlDlg->Create(this);
		m_pMtrlDlg->m_mtrlFile.SetPathName(curFile.GetBuffer());
		m_pMtrlDlg->OnReloadMtrls();
		m_mtrlFile.SetPathName(m_pMtrlDlg->m_mtrlFile.GetPathName());
	}
	else
	{
		m_pMtrlDlg->ShowWindow(SW_SHOW);
		m_pMtrlDlg->SetActiveWindow();
	}
}

void CSPdlg::OnViewStruct()
{
	if (m_pStrctDlg == NULL)
	{
		m_pStrctDlg = new CStructDlg(GetDesktopWindow());
		m_pStrctDlg->Create(this);
	}
	else
	{
		m_pStrctDlg->ShowWindow(SW_SHOW);
		m_pStrctDlg->SetActiveWindow();
	}
}

int CSPdlg::CloseMtrlDlg(void)
{
	m_pMtrlDlg = NULL;	// don't delete m_pGrphDlg; !
	theSP.ReleaseMtrldlgPtr();

	return 0;
}

int CSPdlg::CloseGrphDlg(void)
{
	m_pGrphDlg = NULL;	// don't delete m_pGrphDlg; !
	theSP.ReleaseGrphdlgPtr();

	return 0;
}

int CSPdlg::CloseInfoDlg(void)
{
	m_pInfoDlg = NULL;	// don't delete m_pInfoDlg; !
	theSP.ReleaseInfoboxPtr();

	return 0;
}


int CSPdlg::CloseStrctDlg(void)
{
	m_pStrctDlg = NULL;	// don't delete m_pInfoDlg; !
	theSP.ReleaseViewStrctDlgPtr();

	return 0;
}


void CSPdlg::OnClrGrphPaths()
{
	m_inputFile.SetPathName("");
	//m_mtrlFile.SetPathName("");
	//theSP.PostInfoSP(_T("Removing previous simulation data"));
}

