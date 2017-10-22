// SPgrph.cpp : implementation file
//

#include "stdafx.h"
#include "sp1dll.h"
#include "SPgrph.h"
#include ".\spgrph.h"
#include "poisp_types.h"
#include "poisp_defs.h"

extern CSchrPois theSP;

// CSPgrph dialog

IMPLEMENT_DYNAMIC(CGrphDlg, CResizableDialog)
CGrphDlg::CGrphDlg(CWnd* pParent /*=NULL*/)
	: CResizableDialog(CGrphDlg::IDD, pParent)
	, m_pParentWnd(NULL)
{
	ASSERT(pParent != NULL);

	m_pParentWnd = pParent;
	m_nID = CGrphDlg::IDD;
}

CGrphDlg::~CGrphDlg()
{
}

void CGrphDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_PLOTF, m_grphFile);
	DDX_Control(pDX, IDC_CHARTCTRL, m_ChartCtrl);
	DDX_Control(pDX, IDC_ECOND, m_ctrlEc);
	DDX_Control(pDX, IDC_EVAL, m_ctrlEv);
	DDX_Control(pDX, IDC_EFERMI, m_ctrlEf);
	DDX_Control(pDX, IDC_CHRGN, m_ctrlN);
	DDX_Control(pDX, IDC_CHRGP, m_ctrlP);
	DDX_Control(pDX, IDC_CHRGF, m_ctrlF);
	DDX_Control(pDX, IDC_EVCM, m_ctrlEfield);
	DDX_Control(pDX, IDC_EIGENS, m_cEigenvals);
	DDX_Control(pDX, IDC_WAVEFUNCS, m_ctrlWavefuncs);
	DDX_Control(pDX, IDC_OVRLAY, m_cOverlay1);
}

BOOL CGrphDlg::Create(LPCTSTR lpszTemplateName, CWnd* pParentWnd)
{
	return CDialog::Create(lpszTemplateName, pParentWnd);
}

BOOL CGrphDlg::Create(CSPdlg * pSPdlg)
{
	m_pSPdlg = pSPdlg;

	return CDialog::Create(m_nID, m_pParentWnd);
}

BEGIN_MESSAGE_MAP(CGrphDlg, CDialog)
	ON_WM_SIZE()
	ON_WM_GETMINMAXINFO()
	ON_BN_CLICKED(IDC_EXIT, OnOK)
	ON_BN_CLICKED(IDC_PLOT, OnPlot)
	ON_CBN_SELCHANGE(IDC_EIGENS, OnCbnSelchangeEigens)
	ON_LBN_SELCHANGE(IDC_WAVEFUNCS, OnLbnSelchangeWavefuncs)
	ON_BN_CLICKED(IDC_CLRGRAPH, &CGrphDlg::OnBnClickedClrgraph)
	ON_BN_CLICKED(IDC_ECOND, &CGrphDlg::OnBnClickedEcond)
	ON_BN_CLICKED(IDC_EFERMI, &CGrphDlg::OnBnClickedEfermi)
	ON_BN_CLICKED(IDC_EVAL, &CGrphDlg::OnBnClickedEval)
	ON_BN_CLICKED(IDC_OVRLAY, &CGrphDlg::OnBnClickedOvrlay)
	ON_BN_CLICKED(IDC_CHRGN, &CGrphDlg::OnBnClickedChrgn)
	ON_BN_CLICKED(IDC_CHRGP, &CGrphDlg::OnBnClickedChrgp)
	ON_BN_CLICKED(IDC_CHRGF, &CGrphDlg::OnBnClickedChrgf)
	ON_BN_CLICKED(IDC_ZOOM, &CGrphDlg::OnBnClickedZoom)
	ON_BN_CLICKED(IDC_PAN, &CGrphDlg::OnBnClickedPan)
	ON_BN_CLICKED(IDC_AUTOSCALE, &CGrphDlg::OnBnClickedAutoscale)
END_MESSAGE_MAP()


// CSPgrph message handlers


void CGrphDlg::OnClose()
{
	// Default OnClose is overwritten in order to make the dialog modeless
	m_pSPdlg->CloseGrphDlg();	// this sets the pointer to NULL. It should precede the destruction of the actual window.
	DestroyWindow();

//	CDialog::OnClose();
}

BOOL CGrphDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	CString str;
    CRect rect;
	int i;

	m_ctrlWavefuncs.ResetContent();
	m_ctrlWavefuncs.SetCheckStyle( BS_AUTOCHECKBOX );

	// preset layout - used by the Resizable class
	AddAnchor(IDC_PLOT, TOP_RIGHT);
	AddAnchor(IDC_EXIT, TOP_RIGHT);
	AddAnchor(IDC_PLOTF, BOTTOM_LEFT);
	AddAnchor(IDC_CHARTCTRL, TOP_LEFT);
	AddAnchor(IDC_CLRGRAPH, TOP_LEFT);

	CChartStandardAxis* pBottomAxis = m_ChartCtrl.CreateStandardAxis(CChartCtrl::BottomAxis);
	pBottomAxis->SetMinMax(0, 10);
	CChartStandardAxis* pLeftAxis =	m_ChartCtrl.CreateStandardAxis(CChartCtrl::LeftAxis);
	pLeftAxis->SetMinMax(0, 1);
	CChartStandardAxis* pRightAxis = m_ChartCtrl.CreateStandardAxis(CChartCtrl::RightAxis);
	pRightAxis->SetMinMax(-2, 2);
	pRightAxis->GetGrid()->SetVisible(false);

	for (i=0;i < m_cEigenvals.GetCount();i++)
	{
		m_cEigenvals.DeleteString( i );
	}
	for (i=0;i < m_ctrlWavefuncs.GetCount();i++)
	{
		m_ctrlWavefuncs.DeleteString( i );
	}

	if (theSP.pGlobals->numhheval > 0) {
		for(i=0;i<theSP.pGlobals->numhheval;i++){
			str.Format("HH Eigenvalue %d",i);
			m_cEigenvals.AddString(str);
			str.Format("Heavy Hole Psi %d",i);
			m_ctrlWavefuncs.AddString(str);
		}
	}
	if (theSP.pGlobals->numlheval > 0) {
		for(i=0;i<theSP.pGlobals->numlheval;i++){
			str.Format("LH Eigenvalue %d",i);
			m_cEigenvals.AddString(str);
			str.Format("Light Hole Psi %d",i);
			m_ctrlWavefuncs.AddString(str);
		}
	}
	if (theSP.pGlobals->numeleceval > 0) {
		for(i=0;i<theSP.pGlobals->numeleceval;i++){
			str.Format("EL Eigenvalue %d",i);
			m_cEigenvals.AddString(str);
			str.Format("Electron Psi %d",i);
			m_ctrlWavefuncs.AddString(str);
		}
	}

	return TRUE;  // return TRUE unless you set the focus to a control
}

void CGrphDlg::OnSize(UINT nType, int cx, int cy)
{
	CResizableDialog::OnSize(nType, cx, cy);

    CRect rect;
	CWnd * frmWnd = NULL;
	frmWnd = GetDlgItem(IDC_GRPHFRM);

	if(frmWnd != NULL)
	{
		frmWnd->GetClientRect(rect);
		//graph_wnd.MoveWindow(rect, false);
	}
}

void CGrphDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	lpMMI->ptMinTrackSize.x = 400;
	lpMMI->ptMinTrackSize.y = 400;

	//CResizableDialog::OnGetMinMaxInfo(lpMMI);
}

void CGrphDlg::PostNcDestroy()
{
	delete this;

	//CResizableDialog::PostNcDestroy();
}


void CGrphDlg::OnCancel()
{
	// Default OnCancel is overwritten in order to make the dialog modeless
	m_pSPdlg->CloseGrphDlg();
	DestroyWindow();

	//CResizableDialog::OnCancel();
}

void CGrphDlg::OnOK()
{
	// Default OnOK is overwritten in order to make the dialog modeless
	m_pSPdlg->CloseGrphDlg();
	DestroyWindow();

	//CResizableDialog::OnOK();
}

void CGrphDlg::OnPlot()
{
	//CFile chosenFile;
	//ULONGLONG fileSize;
	//CString chosenFileName = m_grphFile.GetPathName();
	//chosenFile.Open(chosenFileName.GetBuffer(),CFile::modeRead);
	//fileSize = chosenFile.GetLength();
	//while(chosenFile.GetPosition() != fileSize){
	//}

	if(m_cOverlay1.GetCheck() == 0)
	{
		UncheckEigenwavesBox();
		UncheckEigensBox();
		PlotPotentials(true);
	}
	else{PlotAllOverlayed();}
}

void CGrphDlg::OnCbnSelchangeEigens()
{
	if(m_cOverlay1.GetCheck() == 0)
	{
		UncheckEigenwavesBox();
		UncheckPotentialBoxes();
		PlotEigens(true);
	}
	else{PlotAllOverlayed();}
}

void CGrphDlg::OnLbnSelchangeWavefuncs()
{
	if(m_cOverlay1.GetCheck() == 0)
	{
		UncheckEigensBox();
		UncheckPotentialBoxes();
		PlotEigenWaves(true);
	}
	else{PlotAllOverlayed();}
}

void CGrphDlg::OnBnClickedClrgraph()
{
	int k = 0;

	m_ChartCtrl.RemoveAllSeries();

	if(abs(theSP.pGlobals->numeval) < 200){
		for(k = 0; k < m_cEigenvals.GetCount(); k++)
		{
			m_cEigenvals.SetCheck(k,0);
		}
		for(k = 0; k < m_ctrlWavefuncs.GetCount(); k++)
		{
			m_ctrlWavefuncs.SetCheck(k,0);
		}
		m_ctrlEc.SetCheck(0);
		m_ctrlEv.SetCheck(0);
		m_ctrlEf.SetCheck(0);
		m_ctrlN.SetCheck(0);
		m_ctrlP.SetCheck(0);
		m_ctrlF.SetCheck(0);
	}
}

void CGrphDlg::OnBnClickedEcond()
{
	OnPlot();
}

void CGrphDlg::OnBnClickedEfermi()
{
	OnPlot();
}

void CGrphDlg::OnBnClickedEval()
{
	OnPlot();
}

void CGrphDlg::OnBnClickedChrgn()
{
	OnPlot();
}

void CGrphDlg::OnBnClickedChrgp()
{
	OnPlot();
}

void CGrphDlg::OnBnClickedChrgf()
{
	OnPlot();
}

void CGrphDlg::OnBnClickedOvrlay()
{
	PlotAllOverlayed();
}

// Function to plot potential profile
int CGrphDlg::PlotPotentials(bool clrGraph)
{
	struct mesh_type *pMesh;
	int i, NumberPoints;
	int checkState = 0;
	CChartXYSerie* pSeries = NULL;
	CChartAxis* pBottomAxis = NULL;
	CChartAxis* pLeftAxis = NULL;
	CChartAxis* pRightAxis = NULL;
	CChartLineSerie* pLineSeries = NULL;
	//CChartPointsSerie* pPointsSeries = NULL;

	if(clrGraph == true){	m_ChartCtrl.RemoveAllSeries();	}

	pMesh = theSP.pGlobals->pStartSim;
	if((pMesh != NULL) && (abs(theSP.pGlobals->numeval) < 1000)){
		i = 0;
		do{
			i++;
		}while((pMesh = pMesh->pDownMesh) != theSP.pGlobals->pBottomMesh);
		NumberPoints  = i;
		double* XValues = new double[NumberPoints];
		double* Y1Values = new double[NumberPoints];
		double* Y2Values = new double[NumberPoints];
		double* Y3Values = new double[NumberPoints];
		double* Y4Values = new double[NumberPoints];
		double* Y5Values = new double[NumberPoints];
		double* Y6Values = new double[NumberPoints];
		double* Y7Values = new double[NumberPoints];
		double* Y8Values = new double[NumberPoints];
		pMesh = theSP.pGlobals->pStartSim;
		i = 0;
		do{
			XValues[i]  = pMesh->meshcoord;
			Y1Values[i] = pMesh->v + pMesh->offset;	// Ec
			Y2Values[i] = pMesh->v + pMesh->offset-pMesh->Eg;	// Ev
			Y3Values[i] = pMesh->Efn;	// Ef
			Y4Values[i] = (pMesh->pDownMesh->v-pMesh->v)*1.e8/pMesh->pDownMesh->dy;	// E (V/cm)
			Y5Values[i] = pMesh->n;	// n
			Y6Values[i] = pMesh->p;	// p
			Y7Values[i] = pMesh->fcharge;	// Nd-Na
			i++;
		}while((pMesh = pMesh->pDownMesh) != theSP.pGlobals->pBottomMesh);

		checkState = m_ctrlEc.GetCheck();
		if(checkState){
			pLineSeries = m_ChartCtrl.CreateLineSerie(false, false);
			pLineSeries->SetWidth(3);
			pSeries = pLineSeries;
			pSeries->SetPoints(XValues,Y1Values,NumberPoints);
		}

		checkState = m_ctrlEv.GetCheck();
		if(checkState){
			pLineSeries = m_ChartCtrl.CreateLineSerie(false, false);
			pLineSeries->SetWidth(3);
			pSeries = pLineSeries;
			pSeries->SetPoints(XValues,Y2Values,NumberPoints);
		}

		checkState = m_ctrlEf.GetCheck();
		if(checkState){
			pLineSeries = m_ChartCtrl.CreateLineSerie(false, false);
			pLineSeries->SetWidth(3);
			pSeries = pLineSeries;
			pSeries->SetPoints(XValues,Y3Values,NumberPoints);
		}

		checkState = m_ctrlEfield.GetCheck();
		if(checkState){
			pLineSeries = m_ChartCtrl.CreateLineSerie(false, false);
			pLineSeries->SetWidth(3);
			pSeries = pLineSeries;
			pSeries->SetPoints(XValues,Y4Values,NumberPoints);
		}

		checkState = m_ctrlN.GetCheck();
		if(checkState){
			pLineSeries = m_ChartCtrl.CreateLineSerie(false, true);
			pLineSeries->SetWidth(3);
			pSeries = pLineSeries;
			pSeries->SetPoints(XValues,Y5Values,NumberPoints);
		}

		checkState = m_ctrlP.GetCheck();
		if(checkState){
			pLineSeries = m_ChartCtrl.CreateLineSerie(false, true);
			pLineSeries->SetWidth(3);
			pSeries = pLineSeries;
			pSeries->SetPoints(XValues,Y6Values,NumberPoints);
		}

		checkState = m_ctrlF.GetCheck();
		if(checkState){
			pLineSeries = m_ChartCtrl.CreateLineSerie(false, true);
			pLineSeries->SetWidth(3);
			pSeries = pLineSeries;
			pSeries->SetPoints(XValues,Y7Values,NumberPoints);
		}

		pBottomAxis = m_ChartCtrl.GetAxis(CChartCtrl::BottomAxis);
		if ( ((CButton*)GetDlgItem(IDC_AUTOSCALE))->GetCheck() == 1)
			pBottomAxis->SetAutomatic(true);
		pLeftAxis = m_ChartCtrl.GetAxis(CChartCtrl::LeftAxis);
		if ( ((CButton*)GetDlgItem(IDC_ZOOM))->GetCheck() != 1)
			pLeftAxis->SetAutomatic(true);
		pRightAxis = m_ChartCtrl.GetAxis(CChartCtrl::RightAxis);
		pRightAxis->SetAutomatic(true);

		m_ChartCtrl.RefreshCtrl();
	}

	return 0;
}

// Plots the eigenvalues
int CGrphDlg::PlotEigens(bool clrGraph)
{
	struct mesh_type *pMesh;
	int i,j, k, NumberPoints;
	CString str;
	CString elStr, lhStr, hhStr, numStr;
	CChartXYSerie* pSeries = NULL;
	CChartAxis* pBottomAxis = NULL;
	CChartAxis* pRightAxis = NULL;
	CChartAxis* pLeftAxis = NULL;
	CChartLineSerie* pLineSeries = NULL;
	CChartPointsSerie* pPointsSeries = NULL;
	CString strEigens[3];

	elStr = "EL Eigenvalue";
	lhStr = "LH Eigenvalue";
	hhStr = "HH Eigenvalue";
	strEigens[0] = elStr;
	strEigens[1] = lhStr;
	strEigens[2] = hhStr;

	i = 0;
	j = 0;
	k = 0;
	if(abs(theSP.pGlobals->numeval) < 200){
		pMesh = theSP.pGlobals->pStartSim;
		do{	i++;}	while((pMesh = pMesh->pDownMesh) != theSP.pGlobals->pBottomMesh);
		NumberPoints  = i;
		double* XValues = new double[NumberPoints];
		double* YValues = new double[NumberPoints];

		if(clrGraph == true){
			m_ChartCtrl.RemoveAllSeries();
		}

		for(k = 0; k < m_cEigenvals.GetCount(); k++)
		{
			if(m_cEigenvals.GetCheck(k) == 1)
			{
				m_cEigenvals.GetText(k, str);
				m_cEigenvals.GetText(k, numStr);
				str.Delete(elStr.GetLength(),str.GetLength() - elStr.GetLength());
				numStr.Delete(0,elStr.GetLength());
				if(strcmp(elStr,str)==0){
					if (theSP.pGlobals->numeleceval > 0) {
						sscanf(numStr, "%d", &j);
						pMesh = theSP.pGlobals->pStartSim;
						for (i=0;i<NumberPoints;i++) {
							XValues[i] = pMesh->meshcoord;
							YValues[i] = theSP.pGlobals->pEleceval[j];
							pMesh = pMesh->pDownMesh;
						}
						pLineSeries = m_ChartCtrl.CreateLineSerie(false, false);
						pLineSeries->SetWidth(3);
						pSeries = pLineSeries;
						pSeries->SetPoints(XValues,YValues,NumberPoints);
					}
				}
				m_cEigenvals.GetText(k, str);
				m_cEigenvals.GetText(k, numStr);
				str.Delete(lhStr.GetLength(),str.GetLength() - lhStr.GetLength());
				numStr.Delete(0,lhStr.GetLength());
				if(strcmp(lhStr,str)==0){
					if (theSP.pGlobals->numlheval > 0) {
						sscanf(numStr, "%d", &j);
						pMesh = theSP.pGlobals->pStartSim;
						for (i=0;i<NumberPoints;i++) {
							XValues[i] = pMesh->meshcoord;
							YValues[i] = theSP.pGlobals->pLheval[j];
							pMesh = pMesh->pDownMesh;
						}
						pLineSeries = m_ChartCtrl.CreateLineSerie(false, false);
						pLineSeries->SetWidth(3);
						pSeries = pLineSeries;
						pSeries->SetPoints(XValues,YValues,NumberPoints);
					}
				}
				m_cEigenvals.GetText(k, str);
				m_cEigenvals.GetText(k, numStr);
				str.Delete(hhStr.GetLength(),str.GetLength() - hhStr.GetLength());
				numStr.Delete(0,hhStr.GetLength());
				if(strcmp(hhStr,str)==0){
					if (theSP.pGlobals->numhheval > 0) {
						sscanf(numStr, "%d", &j);
						pMesh = theSP.pGlobals->pStartSim;
						for (i=0;i<NumberPoints;i++) {
							XValues[i] = pMesh->meshcoord;
							YValues[i] = theSP.pGlobals->pHheval[j];
							pMesh = pMesh->pDownMesh;
						}
						pLineSeries = m_ChartCtrl.CreateLineSerie(false, false);
						pLineSeries->SetWidth(3);
						pSeries = pLineSeries;
						pSeries->SetPoints(XValues,YValues,NumberPoints);
					}
				}
			}
			pBottomAxis = m_ChartCtrl.GetAxis(CChartCtrl::BottomAxis);
			if ( ((CButton*)GetDlgItem(IDC_AUTOSCALE))->GetCheck() == 1)
				pBottomAxis->SetAutomatic(true);
			pLeftAxis = m_ChartCtrl.GetAxis(CChartCtrl::LeftAxis);
			if ( ((CButton*)GetDlgItem(IDC_ZOOM))->GetCheck() != 1)
				pLeftAxis->SetAutomatic(true);
			m_ChartCtrl.RefreshCtrl();
		}
	}

	return 0;
}

// Plot eigen wavefunctions
int CGrphDlg::PlotEigenWaves(bool clrGraph)
{
	struct mesh_type *pMesh;
	int i,j, k, NumberPoints;
	CString str;
	CString elStr, lhStr, hhStr, numStr;
	CChartXYSerie* pSeries = NULL;
	CChartAxis* pBottomAxis = NULL;
	CChartAxis* pRightAxis = NULL;
	CChartAxis* pLeftAxis = NULL;
	CChartLineSerie* pLineSeries = NULL;
	CChartPointsSerie* pPointsSeries = NULL;

	elStr = "Electron Psi";
	lhStr = "Light Hole Psi";
	hhStr = "Heavy Hole Psi";

	i = 0;
	j = 0;
	k = 0;
	if(abs(theSP.pGlobals->numeval) < 200)
	{
		pMesh = theSP.pGlobals->pSchStart;
		do{	i++;}	while((pMesh = pMesh->pDownMesh) != theSP.pGlobals->pSchStop->pDownMesh);
		NumberPoints  = i;
		double* XValues = new double[NumberPoints];
		double* YValues = new double[NumberPoints];

		if(clrGraph == true){
			m_ChartCtrl.RemoveAllSeries();
		}

		for(k = 0; k < m_ctrlWavefuncs.GetCount(); k++)
		{
			if(m_ctrlWavefuncs.GetCheck(k) == 1)
			{
				m_ctrlWavefuncs.GetText(k, str);
				m_ctrlWavefuncs.GetText(k, numStr);
				str.Delete(elStr.GetLength(),str.GetLength() - elStr.GetLength());
				numStr.Delete(0,elStr.GetLength());
				if(strcmp(elStr,str)==0){
					if (theSP.pGlobals->numeleceval > 0) {
						sscanf(numStr, "%d", &j);
						pMesh = theSP.pGlobals->pStartSim;
						for (i=0;i<NumberPoints;i++) {
							XValues[i] = pMesh->meshcoord;
							YValues[i] = theSP.pGlobals->pElecevec[i][j] + theSP.pGlobals->pEleceval[j];
							pMesh = pMesh->pDownMesh;
						}
						pLineSeries = m_ChartCtrl.CreateLineSerie(false, false);
						pLineSeries->SetWidth(3);
						pSeries = pLineSeries;
						pSeries->SetPoints(XValues,YValues,NumberPoints);
					}
				}
				m_ctrlWavefuncs.GetText(k, str);
				m_ctrlWavefuncs.GetText(k, numStr);
				str.Delete(lhStr.GetLength(),str.GetLength() - lhStr.GetLength());
				numStr.Delete(0,lhStr.GetLength());
				if(strcmp(lhStr,str)==0){
					if (theSP.pGlobals->numlheval > 0) {
						sscanf(numStr, "%d", &j);
						pMesh = theSP.pGlobals->pStartSim;
						for (i=0;i<NumberPoints;i++) {
							XValues[i] = pMesh->meshcoord + theSP.pGlobals->pSchStart->meshcoord;
							YValues[i] = theSP.pGlobals->pLhevec[i][j];
							pMesh = pMesh->pDownMesh;
						}
						pLineSeries = m_ChartCtrl.CreateLineSerie(false, false);
						pLineSeries->SetWidth(3);
						pSeries = pLineSeries;
						pSeries->SetPoints(XValues,YValues,NumberPoints);
					}
				}
				m_ctrlWavefuncs.GetText(k, str);
				m_ctrlWavefuncs.GetText(k, numStr);
				str.Delete(hhStr.GetLength(),str.GetLength() - hhStr.GetLength());
				numStr.Delete(0,hhStr.GetLength());
				if(strcmp(hhStr,str)==0){
					if (theSP.pGlobals->numhheval > 0) {
						sscanf(numStr, "%d", &j);
						pMesh = theSP.pGlobals->pStartSim;
						for (i=0;i<NumberPoints;i++) {
							XValues[i] = pMesh->meshcoord + theSP.pGlobals->pSchStart->meshcoord;
							YValues[i] = theSP.pGlobals->pHhevec[i][j];
							pMesh = pMesh->pDownMesh;
						}
						pLineSeries = m_ChartCtrl.CreateLineSerie(false, false);
						pLineSeries->SetWidth(3);
						pSeries = pLineSeries;
						pSeries->SetPoints(XValues,YValues,NumberPoints);
					}
				}
			}
			pBottomAxis = m_ChartCtrl.GetAxis(CChartCtrl::BottomAxis);
			if ( ((CButton*)GetDlgItem(IDC_AUTOSCALE))->GetCheck() == 1)
				pBottomAxis->SetAutomatic(true);
			pLeftAxis = m_ChartCtrl.GetAxis(CChartCtrl::LeftAxis);
			if ( ((CButton*)GetDlgItem(IDC_ZOOM))->GetCheck() != 1)
				pLeftAxis->SetAutomatic(true);
			m_ChartCtrl.RefreshCtrl();
		}
	}

	return 0;
}

// Unchecks the checkboxes for Potential plots
int CGrphDlg::UncheckPotentialBoxes(void)
{
	UpdateData(false);
	m_ctrlEc.SetCheck(0);
	m_ctrlEv.SetCheck(0);
	m_ctrlEf.SetCheck(0);
	m_ctrlEfield.SetCheck(0);
	m_ctrlN.SetCheck(0);
	m_ctrlP.SetCheck(0);
	m_ctrlF.SetCheck(0);
	UpdateData(true);

	return 0;
}

// Unchecks the boxes for the Eigenvalues in the eigenval listbox
int CGrphDlg::UncheckEigensBox(void)
{
	int k=0;

	for(k = 0; k < m_cEigenvals.GetCount(); k++)
	{
		m_cEigenvals.SetCheck(k,0);
	}

	return 0;
}

// Unchecks boxes for the eigenvectors in the Eigenwaves listbox
int CGrphDlg::UncheckEigenwavesBox(void)
{
	int k=0;

	for(k = 0; k < m_ctrlWavefuncs.GetCount(); k++)
	{
		m_ctrlWavefuncs.SetCheck(k,0);
	}

	return 0;
}

// Plots all checked plots
int CGrphDlg::PlotAllOverlayed(void)
{
	PlotPotentials(true);
	PlotEigens(false);
	PlotEigenWaves(false);

	return 0;
}



void CGrphDlg::OnBnClickedZoom()
{
	// TODO: Add your control notification handler code here
	if ( ((CButton*)GetDlgItem(IDC_ZOOM))->GetCheck() == 1)
		m_ChartCtrl.SetZoomEnabled(true);
	else
		m_ChartCtrl.SetZoomEnabled(false);	
}

void CGrphDlg::OnBnClickedPan()
{
	// TODO: Add your control notification handler code here
	if ( ((CButton*)GetDlgItem(IDC_PAN))->GetCheck() == 1)
		m_ChartCtrl.SetPanEnabled(true);
	else
		m_ChartCtrl.SetPanEnabled(false);	
}

void CGrphDlg::OnBnClickedAutoscale()
{
	CChartAxis* pBottomAxis = NULL;
	
	pBottomAxis = m_ChartCtrl.GetAxis(CChartCtrl::BottomAxis);
	if ( ((CButton*)GetDlgItem(IDC_AUTOSCALE))->GetCheck() == 1)
		pBottomAxis->SetAutomatic(true);
	else
		pBottomAxis->SetAutomatic(false);
}
