// StructDlg.cpp : implementation file
//

#include "stdafx.h"
#include "sp1dll.h"
#include "poisp_types.h"
#include "StructDlg.h"
#include "spdlg.h"

extern CSchrPois theSP;

// CStructDlg dialog

IMPLEMENT_DYNAMIC(CStructDlg, CResizableDialog)

CStructDlg::CStructDlg(CWnd* pParent /*=NULL*/)
	: CResizableDialog(CStructDlg::IDD, pParent)
	, m_pParentWnd(NULL)
	, m_cStructInfoStr(_T(""))
{
	ASSERT(pParent != NULL);

	m_pParentWnd = pParent;
	m_nID = CStructDlg::IDD;
}

CStructDlg::~CStructDlg()
{
}

void CStructDlg::DoDataExchange(CDataExchange* pDX)
{
	CResizableDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STRCTLIST, m_cStrctTree);
	DDX_Control(pDX, IDC_INPUTFILE, m_cInputFile);
	DDX_Control(pDX, IDC_STRUCTBOX, m_cStructInfoBox);
	DDX_Text(pDX, IDC_STRUCTBOX, m_cStructInfoStr);
}

BOOL CStructDlg::Create(LPCTSTR lpszTemplateName, CWnd* pParentWnd)
{
	// TODO: Add your specialized code here and/or call the base class

	return CDialog::Create(lpszTemplateName, pParentWnd);
}

BOOL CStructDlg::Create(CSPdlg *pSPdlg)
{
	m_pSPdlg = pSPdlg;

	return CDialog::Create(m_nID, m_pParentWnd);
}

BEGIN_MESSAGE_MAP(CStructDlg, CResizableDialog)
	ON_WM_CLOSE()
	ON_WM_CREATE()
	ON_WM_NCDESTROY()
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_LOADSTRCT1, &CStructDlg::OnBnClickedLoadstrct1)
END_MESSAGE_MAP()


// CStructDlg message handlers

void CStructDlg::OnClose()
{
	m_pSPdlg->CloseStrctDlg();
	DestroyWindow();

	//CResizableDialog::OnClose();
}

int CStructDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CResizableDialog::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here

	return 0;
}

void CStructDlg::OnNcDestroy()
{
	CResizableDialog::OnNcDestroy();

	// TODO: Add your message handler code here
}

void CStructDlg::OnSize(UINT nType, int cx, int cy)
{
	CResizableDialog::OnSize(nType, cx, cy);

	ArrangeLayout();
}

void CStructDlg::PostNcDestroy()
{
	delete this;

	//CResizableDialog::PostNcDestroy();
}

BOOL CStructDlg::OnInitDialog()
{
	CResizableDialog::OnInitDialog();

	// preset layout
	AddAnchor(IDC_STRCTLIST, TOP_LEFT, TOP_RIGHT);
	AddAnchor(IDOK, BOTTOM_RIGHT);
	AddAnchor(IDCANCEL, BOTTOM_RIGHT);
	AddAnchor(IDC_LOADSTRCT1, TOP_RIGHT);
	AddAnchor(IDC_STRUCTBOX, TOP_LEFT, BOTTOM_RIGHT);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CStructDlg::LoadStruct()
{
	int error;
	CString s_fname, s_fext, s_matfname;

	if(theSP.pGlobals->pTopLayer != NULL)
	{
		PopulateStrctTree();
		PopulateStrctInfoBox(theSP.pGlobals);

		if(s_fext.IsEmpty())
			s_fext = _T("txt");
		strcpy(theSP.inputFilePath, theSP.pGlobals->filenamePath);
		s_fname = theSP.inputFilePath;
		s_fname = s_fname + "." + s_fext;
		m_cInputFile.SetPathName(s_fname);
		m_pSPdlg->m_inputFile.SetPathName(s_fname);
	}
	else{
		//PostInfoSP(_T("LoadStruct: Getting the input (input)"));
		s_fname = m_cInputFile.GetPathName();
		s_fext = m_cInputFile.GetFileExt();
		s_fname.Delete(s_fname.GetLength() - 4,4);
		strcpy((theSP.pGlobals)->filename, s_fname.GetBuffer());

		error = theSP.input(theSP.pGlobals,theSP.pFlags);
		if(theSP.pFlags->nomtrlfile == false)
			error = theSP.initialize(theSP.pGlobals,theSP.pFlags);

		if(error){ 
			MessageBox("Layer Structure not Loaded. No input file specified");
		}
		else{
			PopulateStrctTree();
			PopulateStrctInfoBox(theSP.pGlobals);

			if(s_fext.IsEmpty())
				s_fext = _T("txt");
			strcpy(theSP.inputFilePath, theSP.pGlobals->filenamePath);
			s_fname = theSP.inputFilePath;
			s_fname = s_fname + "." + s_fext;
			m_cInputFile.SetPathName(s_fname);
			m_pSPdlg->m_inputFile.SetPathName(s_fname);
		}
	}
}
void CStructDlg::OnBnClickedLoadstrct1()
{
	LoadStruct();
}

void CStructDlg::PopulateStrctTree(void)
{
	struct layer_type *pCurLayer;
	HTREEITEM hMatrl;
	TVINSERTSTRUCT tvInsert;
	char tmpStr[200];
	struct mesh_type *pCurMeshPt;
	int numMeshPts = 0, i = 0;

	pCurLayer = theSP.pGlobals->pTopLayer;

	m_cStrctTree.DeleteAllItems();

	tvInsert.hParent = NULL;
	tvInsert.hInsertAfter = NULL;
	tvInsert.item.mask = TVIF_TEXT;

	tvInsert.item.pszText = (LPSTR)malloc(sizeof(pCurLayer->contactLayer));
	strcpy(tvInsert.item.pszText, pCurLayer->contactLayer);
	hMatrl = m_cStrctTree.InsertItem(&tvInsert);
	pCurMeshPt = theSP.pGlobals->pTopMesh;
	do {
		numMeshPts = (int) (pCurLayer->tLayer/pCurLayer->dyLayer+0.5e0);
		tvInsert.item.pszText = (LPSTR)malloc(sizeof(pCurLayer->nameLayer));
		strcpy(tvInsert.item.pszText, pCurLayer->nameLayer);
		hMatrl = m_cStrctTree.InsertItem(&tvInsert);
		sprintf(tmpStr, "layer thickness (A) = %.1f", pCurLayer->tLayer);
		m_cStrctTree.InsertItem(TVIF_TEXT,tmpStr, 0, 0, 0, 0, 0, hMatrl, NULL);
		sprintf(tmpStr, "mesh step size (A) = %.1f", pCurLayer->dyLayer);
		m_cStrctTree.InsertItem(TVIF_TEXT,tmpStr, 0, 0, 0, 0, 0, hMatrl, NULL);
		if(theSP.pFlags->nomtrlfile == false)
		{
			sprintf(tmpStr, "band gap = %.2f (eV)", pCurMeshPt->Eg);
			m_cStrctTree.InsertItem(TVIF_TEXT,tmpStr, 0, 0, 0, 0, 0, hMatrl, NULL);
			sprintf(tmpStr, "electron effective mass = %.2f", pCurMeshPt->eem);
			m_cStrctTree.InsertItem(TVIF_TEXT,tmpStr, 0, 0, 0, 0, 0, hMatrl, NULL);
			sprintf(tmpStr, "heavy hole effective mass = %.2f", pCurMeshPt->ehm);
			m_cStrctTree.InsertItem(TVIF_TEXT,tmpStr, 0, 0, 0, 0, 0, hMatrl, NULL);
			sprintf(tmpStr, "light hole effective mass = %.2f", pCurMeshPt->elhm);
			m_cStrctTree.InsertItem(TVIF_TEXT,tmpStr, 0, 0, 0, 0, 0, hMatrl, NULL);
			sprintf(tmpStr, "electron mobility = %.2f", pCurMeshPt->emob);
			m_cStrctTree.InsertItem(TVIF_TEXT,tmpStr, 0, 0, 0, 0, 0, hMatrl, NULL);
			sprintf(tmpStr, "hole mobility = %.2f", pCurMeshPt->hmob);
			m_cStrctTree.InsertItem(TVIF_TEXT,tmpStr, 0, 0, 0, 0, 0, hMatrl, NULL);
			sprintf(tmpStr, "acceptor ionization level = %.2f (eV)", pCurMeshPt->Ea);
			m_cStrctTree.InsertItem(TVIF_TEXT,tmpStr, 0, 0, 0, 0, 0, hMatrl, NULL);
			sprintf(tmpStr, "deep acceptor ionization level = %.2f (eV)", pCurMeshPt->Eda);
			m_cStrctTree.InsertItem(TVIF_TEXT,tmpStr, 0, 0, 0, 0, 0, hMatrl, NULL);
			sprintf(tmpStr, "donor ionization level = %.2f (eV)", pCurMeshPt->Ed);
			m_cStrctTree.InsertItem(TVIF_TEXT,tmpStr, 0, 0, 0, 0, 0, hMatrl, NULL);
			sprintf(tmpStr, "deep donor ionization level = %.2f (eV)", pCurMeshPt->Edd);
			m_cStrctTree.InsertItem(TVIF_TEXT,tmpStr, 0, 0, 0, 0, 0, hMatrl, NULL);
			sprintf(tmpStr, "acceptor concentration = %.2f", pCurMeshPt->Na);
			m_cStrctTree.InsertItem(TVIF_TEXT,tmpStr, 0, 0, 0, 0, 0, hMatrl, NULL);
			sprintf(tmpStr, "deep acceptor concentration = %.2f", pCurMeshPt->Nda);
			m_cStrctTree.InsertItem(TVIF_TEXT,tmpStr, 0, 0, 0, 0, 0, hMatrl, NULL);
			sprintf(tmpStr, "donor concentration = %.2f", pCurMeshPt->Nd);
			m_cStrctTree.InsertItem(TVIF_TEXT,tmpStr, 0, 0, 0, 0, 0, hMatrl, NULL);
			sprintf(tmpStr, "deep donor concentration = %.2f", pCurMeshPt->Ndd);
			m_cStrctTree.InsertItem(TVIF_TEXT,tmpStr, 0, 0, 0, 0, 0, hMatrl, NULL);
			sprintf(tmpStr, "absorption = %.2f", pCurMeshPt->alpha);
			m_cStrctTree.InsertItem(TVIF_TEXT,tmpStr, 0, 0, 0, 0, 0, hMatrl, NULL);
			sprintf(tmpStr, "polarization = %.2f", pCurMeshPt->polariz);
			m_cStrctTree.InsertItem(TVIF_TEXT,tmpStr, 0, 0, 0, 0, 0, hMatrl, NULL);
			sprintf(tmpStr, "hole effective mass = %.2f", pCurMeshPt->ehm);
			m_cStrctTree.InsertItem(TVIF_TEXT,tmpStr, 0, 0, 0, 0, 0, hMatrl, NULL);
			sprintf(tmpStr, "hole effective mass = %.2f", pCurMeshPt->ehm);
			m_cStrctTree.InsertItem(TVIF_TEXT,tmpStr, 0, 0, 0, 0, 0, hMatrl, NULL);
			sprintf(tmpStr, "hole effective mass = %.2f", pCurMeshPt->ehm);
			m_cStrctTree.InsertItem(TVIF_TEXT,tmpStr, 0, 0, 0, 0, 0, hMatrl, NULL);
			if(pCurMeshPt->pDownMesh == theSP.pGlobals->pBottomMesh)
			{		continue;		}
			else
			{
				for(i=0;i<numMeshPts;i++)
				{		pCurMeshPt = pCurMeshPt->pDownMesh;			}
			}
		}
		else{
			sprintf(tmpStr, "acceptor ionization level (eV) = %s", pCurLayer->EaLayer);
			m_cStrctTree.InsertItem(TVIF_TEXT,tmpStr, 0, 0, 0, 0, 0, hMatrl, NULL);
			sprintf(tmpStr, "sheet acceptor ionization level (eV) = %s", pCurLayer->EasLayer);
			m_cStrctTree.InsertItem(TVIF_TEXT,tmpStr, 0, 0, 0, 0, 0, hMatrl, NULL);
			sprintf(tmpStr, "deep acceptor ionization level (eV) = %s", pCurLayer->EdaLayer);
			m_cStrctTree.InsertItem(TVIF_TEXT,tmpStr, 0, 0, 0, 0, 0, hMatrl, NULL);
			sprintf(tmpStr, "deep donor ionization level (eV) = %s", pCurLayer->EddLayer);
			m_cStrctTree.InsertItem(TVIF_TEXT,tmpStr, 0, 0, 0, 0, 0, hMatrl, NULL);
			sprintf(tmpStr, "donor ionization level (eV) = %s", pCurLayer->EdLayer);
			m_cStrctTree.InsertItem(TVIF_TEXT,tmpStr, 0, 0, 0, 0, 0, hMatrl, NULL);
			sprintf(tmpStr, "sheet donor ionization level (eV) = %s", pCurLayer->EdsLayer);
			m_cStrctTree.InsertItem(TVIF_TEXT,tmpStr, 0, 0, 0, 0, 0, hMatrl, NULL);
			sprintf(tmpStr, "electron effective mass = %s", pCurLayer->eemLayer);
			m_cStrctTree.InsertItem(TVIF_TEXT,tmpStr, 0, 0, 0, 0, 0, hMatrl, NULL);
			sprintf(tmpStr, "Fermi leve (eV) = %s", pCurLayer->EfLayer);
			m_cStrctTree.InsertItem(TVIF_TEXT,tmpStr, 0, 0, 0, 0, 0, hMatrl, NULL);
			sprintf(tmpStr, "Band gap (eV) = %s", pCurLayer->EgLayer);
			m_cStrctTree.InsertItem(TVIF_TEXT,tmpStr, 0, 0, 0, 0, 0, hMatrl, NULL);
			sprintf(tmpStr, "electron mobility (1/cm) = %s", pCurLayer->emobLayer);
			m_cStrctTree.InsertItem(TVIF_TEXT,tmpStr, 0, 0, 0, 0, 0, hMatrl, NULL);
			sprintf(tmpStr, "dielectric constant = %s", pCurLayer->erLayer);
			m_cStrctTree.InsertItem(TVIF_TEXT,tmpStr, 0, 0, 0, 0, 0, hMatrl, NULL);
			sprintf(tmpStr, "fully ionized flag = %d", pCurLayer->fullyionLayer);
			m_cStrctTree.InsertItem(TVIF_TEXT,tmpStr, 0, 0, 0, 0, 0, hMatrl, NULL);
			sprintf(tmpStr, "hole mobility (1/cm) = %s", pCurLayer->hmobLayer);
			m_cStrctTree.InsertItem(TVIF_TEXT,tmpStr, 0, 0, 0, 0, 0, hMatrl, NULL);
			sprintf(tmpStr, "heavy hole effective mass = %s", pCurLayer->mhhLayer);
			m_cStrctTree.InsertItem(TVIF_TEXT,tmpStr, 0, 0, 0, 0, 0, hMatrl, NULL);
			sprintf(tmpStr, "light hole effective mass = %s", pCurLayer->mlhLayer);
			m_cStrctTree.InsertItem(TVIF_TEXT,tmpStr, 0, 0, 0, 0, 0, hMatrl, NULL);
			sprintf(tmpStr, "acceptor concentration = %s", pCurLayer->NaLayer);
			m_cStrctTree.InsertItem(TVIF_TEXT,tmpStr, 0, 0, 0, 0, 0, hMatrl, NULL);
			sprintf(tmpStr, "sheet acceptor concentration = %s", pCurLayer->NasLayer);
			m_cStrctTree.InsertItem(TVIF_TEXT,tmpStr, 0, 0, 0, 0, 0, hMatrl, NULL);
			sprintf(tmpStr, "deep acceptor concentration = %s", pCurLayer->NdaLayer);
			m_cStrctTree.InsertItem(TVIF_TEXT,tmpStr, 0, 0, 0, 0, 0, hMatrl, NULL);
			sprintf(tmpStr, "deep donor concentration = %s", pCurLayer->NddLayer);
			m_cStrctTree.InsertItem(TVIF_TEXT,tmpStr, 0, 0, 0, 0, 0, hMatrl, NULL);
			sprintf(tmpStr, "donor concentration = %s", pCurLayer->NdLayer);
			m_cStrctTree.InsertItem(TVIF_TEXT,tmpStr, 0, 0, 0, 0, 0, hMatrl, NULL);
			sprintf(tmpStr, "sheet donor concentration = %s", pCurLayer->NdsLayer);
			m_cStrctTree.InsertItem(TVIF_TEXT,tmpStr, 0, 0, 0, 0, 0, hMatrl, NULL);
			sprintf(tmpStr, "no electrons flag = %d", pCurLayer->noelecLayer);
			m_cStrctTree.InsertItem(TVIF_TEXT,tmpStr, 0, 0, 0, 0, 0, hMatrl, NULL);
			sprintf(tmpStr, "no electrons flag = %d", pCurLayer->noholesLayer);
			m_cStrctTree.InsertItem(TVIF_TEXT,tmpStr, 0, 0, 0, 0, 0, hMatrl, NULL);
			sprintf(tmpStr, "sheet acceptor concentration = %.1f", pCurLayer->NasLayer);
			m_cStrctTree.InsertItem(TVIF_TEXT,tmpStr, 0, 0, 0, 0, 0, hMatrl, NULL);
			sprintf(tmpStr, "sheet acceptor concentration = %.1f", pCurLayer->NasLayer);
			m_cStrctTree.InsertItem(TVIF_TEXT,tmpStr, 0, 0, 0, 0, 0, hMatrl, NULL);
		}
	}while ((pCurLayer = pCurLayer->pDownLayer) != NULL);
}

int CStructDlg::PopulateStrctInfoBox(struct global_type *pGlobals)
{
	char outdata[20000], outsubdata[100];
	struct layer_type *pLayer;
	struct dummy_layer_type *pDefaultLayer;
	char dummyname[MAX_SEMINAME],dummyname2[MAX_SEMINAME];
	char simfamily[MAX_SEMINAME];
	struct mtrl_list *pMtrlList; 
	int loc_badversion = false;
	CString tmp;

	pDefaultLayer = (struct dummy_layer_type *) malloc(sizeof(struct dummy_layer_type)); /* allocate default layer information */

	if (!strncmp(pGlobals->surfcontact,"slope",5)) {
		strcpy(outdata,"surface            ");
		strcat(outdata,pGlobals->surfcontact);
		strcat(outdata,"=0\n");
		strcpy(outdata,pGlobals->surfcontact);
		tmp = outdata;
		m_cStructInfoStr = tmp + char(13) + char(10);
	}
	else {
		strcpy(outdata,"surface            ");
		strcat(outdata,pGlobals->surfcontact);
		strcat(outdata,"\n");
		strcpy(outdata,pGlobals->surfcontact);
		tmp = outdata;
		m_cStructInfoStr = tmp + char(13) + char(10);
	}
	
	pLayer = pGlobals->pTopLayer;
	if(pLayer->dyLayer != NULL) {
		do {
			/* First, see if the needed material is in the materials data structure*/
			strcpy(dummyname,pLayer->nameLayer);
			lowercase(dummyname);
			pMtrlList = pGlobals->pTopMtrl;
			if (pMtrlList->pNext_mtrl == NULL) {
				pMtrlList = NULL;
			}
			if(pMtrlList != NULL) {
				strcpy(dummyname2,pMtrlList->name);
				lowercase(dummyname2);
				while(strcmp(dummyname2,dummyname)) {
					if (pMtrlList->pNext_mtrl == NULL) {
						pMtrlList = NULL;
						break;
					}
					else {
						pMtrlList=pMtrlList->pNext_mtrl;
						strcpy(dummyname2,pMtrlList->name);
						lowercase(dummyname2);
					}
				}
			}
			if(pLayer != NULL) {
				strcpy(outdata,"-----------------------------------------------------------------------------------------\n");
				tmp = outdata;
				m_cStructInfoStr += tmp + char(13) + char(10);
				sprintf(outsubdata, "%5.1f",pLayer->tLayer);
				strcpy(outdata,outsubdata);
				strcat(outdata," Ang\t");
				strcat(outdata,pLayer->nameLayer);
				strcat(outdata,"\tEg = ");
				if(!strcmp(pLayer->EgLayer, "")) {
					if(pMtrlList != NULL) {
						pGlobals->xval = 999.0; /* default value for unused xval */
						pGlobals->yval = 999.0; /* default value for unused yval */
						pGlobals->zval = 999.0; /* default value for unused zval */
						pGlobals->wval = 999.0; /* default value for unused wval */
						if (pLayer->xvalLayer[0] != '\0') pGlobals->xval = evaluate(pLayer->xvalLayer, pGlobals);
						if (pLayer->yvalLayer[0] != '\0') pGlobals->yval = evaluate(pLayer->yvalLayer, pGlobals);
						if (pLayer->zvalLayer[0] != '\0') pGlobals->zval = evaluate(pLayer->zvalLayer, pGlobals);
						if (pLayer->wvalLayer[0] != '\0') pGlobals->wval = evaluate(pLayer->wvalLayer, pGlobals);
						int oldcheckfamily = theSP.pFlags->checkfamily;
						theSP.pFlags->checkfamily = false;
						int ioerr_loc = theSP.semisearch(pLayer->nameLayer,pGlobals->xval,pGlobals->yval,pGlobals->zval,pGlobals->wval,
											pDefaultLayer,simfamily,pGlobals,theSP.pFlags);	/* get information from materials file*/
						theSP.pFlags->checkfamily = oldcheckfamily;
						sprintf(outsubdata, "%5.1f", pDefaultLayer->EgLayer);
						strcat(outdata,outsubdata);
					}
				}
				else {
					strcat(outdata, pLayer->EgLayer);
				}
				strcat(outdata,"\tns = ");
				sprintf(outsubdata, "%10.3e",pLayer->nsLayer);
				strcat(outdata,outsubdata);
				strcat(outdata," cm-2,\tps= ");
				sprintf(outsubdata, "%10.3e",pLayer->psLayer);
				strcat(outdata,outsubdata);
				strcat(outdata," cm-2\n");
				tmp = outdata;
				m_cStructInfoStr += tmp + char(13) + char(10);
			}
		} while((pLayer=pLayer->pDownLayer) != NULL);
		strcpy(outdata,"-----------------------------------------------------------------------------------------\n");
		tmp = outdata;
		m_cStructInfoStr += tmp + char(13) + char(10);
		
		if (!strncmp(pGlobals->subcontact,"slope",5)) {
			strcpy(outdata,"substrate            ");
			strcat(outdata,pGlobals->subcontact);
			strcat(outdata,"=0\n");
			tmp = outdata;
			m_cStructInfoStr += tmp + char(13) + char(10) + char(13) + char(10);
		}
		else {
			strcpy(outdata,"substrate            ");
			strcat(outdata,pGlobals->subcontact);
			strcat(outdata,"\n");
			tmp = outdata;
			m_cStructInfoStr += tmp + char(13) + char(10) + char(13) + char(10);
		}
		
		//m_cStructInfoStr += char(13) + char(10);

		strcpy(outdata,"Temperature = ");
		sprintf(outsubdata, "%5.1f",pGlobals->temperature);
		strcat(outdata,outsubdata);
		strcat(outdata," K\n");
		tmp = outdata;
		m_cStructInfoStr += tmp + char(13) + char(10);
	}

	UpdateData(false);

	free(pDefaultLayer);

	return 0;
}
