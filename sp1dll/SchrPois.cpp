#include "StdAfx.h"
#include ".\schrpois.h"
#include "spdlg.h"
#include ".\poisson\poisp_types.h"
#include "InfoDlg.h"


extern CSchrPois theSP;	//exporting the class out of dll



CSchrPois::CSchrPois(void)
{
}

CSchrPois::~CSchrPois(void)
{
}

void CSchrPois::ShowSPdlg(void)
{
	CSPdlg spMainCtrl;

	HINSTANCE hPrevHandle = AfxGetResourceHandle();
	CWinApp *theApp = AfxGetApp();
    AfxSetResourceHandle( theApp->m_hInstance );
	spMainCtrl.DoModal();
    AfxSetResourceHandle( hPrevHandle ); // Never forget to reset previous handle	
}


int CSchrPois::InitSPobj(void)
{
	pGlobals = (struct global_type *) malloc(sizeof(struct global_type));
	pFlags = (struct flag_type *) malloc(sizeof(struct flag_type));
	
	pFlags->noinputfile = true;
	pFlags->nomtrlfile = true;
	pFlags->schMemAlloc = false;
	pFlags->badversion = false;
	pFlags->schElecMemAlloc = false; /* Start with the Schrodinger memory flag unset */
	pFlags->schHhMemAlloc = false; /* Start with the Schrodinger memory flag unset */
	pFlags->schLhMemAlloc = false; /* Start with the Schrodinger memory flag unset */
	
	strcpy(pGlobals->materialsversion,"beta6");		/* Materials file Default version*/
	  
	strcpy(pGlobals->matrlfile,"materials");		/* Default materials file is the file 'materials'*/
	  
	pGlobals->pTopMtrl = (struct mtrl_list *) malloc(sizeof(struct mtrl_list));
	pGlobals->pTopMtrl->pNext_mtrl = NULL;
	pGlobals->pTopLayer = NULL;
	
	return 0;
}

int CSchrPois::LoadMatrlFile(char * fname)
{
	int ioerr = 0;

	strcpy(pGlobals->matrlfile, fname);

	ioerr = theSP.loadMaterials(pGlobals, pFlags);

	return ioerr;
}

int CSchrPois::PostInfo(CEdit * editWndPtr, CString info)
{
	/*Utility function to post message to any Edit box, whose pointer is passed in*/
	CString curStr;

	if(editWndPtr != NULL)
	{
		editWndPtr->GetWindowText(curStr);
		//info = curStr + '\n' + info;
		curStr += info + char(13) + char(10);
		editWndPtr->SetWindowText(curStr);
	}

	return 0;
}

int CSchrPois::PostInfoSP(CString info)
{
	/*Class function to post message to the Edit box window that has been initialzed.
	If m_pInfoDlg not present no message is posted nor saved.*/
	CString curStr;

	if(m_pInfoDlg != NULL)
	{
		m_pInfoDlg->m_cInfoBox.GetWindowText(curStr);
		//info = curStr + '\n' + info;
		curStr += info + char(13) + char(10);
		m_pInfoDlg->m_cInfoBox.SetWindowText(curStr);
	}

	return 0;
}

int CSchrPois::SetInfoboxPtr(CInfoDlg * pInfobox)
{
	int ierr = 0;

	m_pInfoDlg = pInfobox;

	return ierr;
}

int CSchrPois::ReleaseInfoboxPtr()
{
	int ierr = 0;

	m_pInfoDlg = NULL;

	return ierr;
}

int CSchrPois::ReleaseGrphdlgPtr()
{
	int ierr = 0;

	m_pGrphDlg = NULL;

	return ierr;
}

int CSchrPois::ReleaseMtrldlgPtr()
{
	int ierr = 0;

	m_pMtrlDlg = NULL;

	return ierr;
}

int CSchrPois::ReleaseViewStrctDlgPtr()
{
	int ierr = 0;

	m_pStrctDlg = NULL;

	return ierr;
}