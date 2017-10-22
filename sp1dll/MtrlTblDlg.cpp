// MtrlTblDlg.cpp : implementation file
//

#include "stdafx.h"
#include "sp1dll.h"
#include "MtrlTblDlg.h"
#include "poisp_types.h"
#include "matrls.h"
#include "mtrldlg.h"


// CMtrlTblDlg dialog

IMPLEMENT_DYNAMIC(CMtrlTblDlg, CDialog)

CMtrlTblDlg::CMtrlTblDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMtrlTblDlg::IDD, pParent)
	, m_pParentWnd(NULL)
	, m_sTmprt("300.0")
	, m_sXcomp(_T("0.1"))
	, m_bEval(false)
{

	ASSERT(pParent != NULL);

	m_pParentWnd = pParent;
	m_nID = CMtrlTblDlg::IDD;
}

CMtrlTblDlg::~CMtrlTblDlg()
{
}

void CMtrlTblDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MTRLLIST, m_mtrlListTbl);
	DDX_Text(pDX, IDC_TMPRT, m_sTmprt);
	DDX_Text(pDX, IDC_XCOMP, m_sXcomp);
	DDX_Check(pDX, IDC_EVALFLAG, (int&)m_bEval);
}


BEGIN_MESSAGE_MAP(CMtrlTblDlg, CDialog)
	ON_BN_CLICKED(IDC_EVALFLAG, &CMtrlTblDlg::OnEvalMtrls)
END_MESSAGE_MAP()


// CMtrlTblDlg message handlers

BOOL CMtrlTblDlg::Create(LPCTSTR lpszTemplateName, CWnd* pParentWnd)
{
	// TODO: Add your specialized code here and/or call the base class

	return CDialog::Create(lpszTemplateName, pParentWnd);
}

BOOL CMtrlTblDlg::Create(CMtrlDlg *pMtrldlg)
{
	m_pMtrlDlg = pMtrldlg;

	return CDialog::Create(m_nID, m_pParentWnd);
}
BOOL CMtrlTblDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	FillMtrlTbl();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CMtrlTblDlg::ModifyHeaderItems()
{
	//HD_ITEM curItem;

	// retrieve embedded header control
	CHeaderCtrl* pHdrCtrl= NULL;
	pHdrCtrl= m_mtrlListTbl.GetHeaderCtrl();

	//pHdrCtrl->SetImageList(m_pImageHdrSmall);

	//// add bmaps to each header item
	//memset(&curItem, 0, sizeof(curItem));
	//pHdrCtrl->GetItem(0, &curItem);
	//curItem.mask= HDI_IMAGE | HDI_FORMAT;
	//curItem.iImage= 0;
	//curItem.fmt= HDF_LEFT | HDF_IMAGE | HDF_STRING;
	//pHdrCtrl->SetItem(0, &curItem);

	//pHdrCtrl->GetItem(1, &curItem);
	//curItem.mask= HDI_IMAGE | HDI_FORMAT;
	//curItem.iImage= 1;
	//curItem.fmt= HDF_LEFT | HDF_IMAGE | HDF_STRING;
	//pHdrCtrl->SetItem(1, &curItem);
}

void CMtrlTblDlg::FillMtrlTbl()
{
	CRect           rect;
	int             iIcon, iItem, iSubItem;
	LV_ITEM         lvitem;
	CString         strItem1= _T("Material");
	CString         strItem2= _T("Family");
	CString         strItem3= _T("Type");
	CString         strItem4= _T("Bandgap (eV)");
	CString         strItem5= _T("Electron effective mass");
	CString         strItem6= _T("Heavy hole effective mass");
	CString         strItem7= _T("Light hole effective mass");
	CString         strIconDesc[4];
	struct			mtrl_list *pmaterial;
	char			tmpStr[100];
	bool			evaluate = false;
	double			xval = 0.0, xvalNew, yval, zval, wval;
	double			funcvalue, tmprt = 0.0;
	int				loc_badversion = 0;
	struct mtrl_line_type *cur_pInterval;

	iIcon = iItem = iSubItem = 0;

	strIconDesc[0]= _T("GaAs");

	UpdateData(true);

	strcpy(tmpStr, m_sTmprt.GetBuffer());
	tmprt = atof(tmpStr);
	strcpy(tmpStr, m_sXcomp.GetBuffer());
	xval = atof(tmpStr);
	evaluate = m_bEval;

	UpdateData(false);

	yval = zval = wval = 999.0;
	xvalNew = xval;
	funcvalue = 0.0;

	int scrollPos = m_mtrlListTbl.GetScrollPos(SB_HORZ);

	int nCols = m_mtrlListTbl.GetHeaderCtrl()->GetItemCount();
	int iCol = 0;
	for(iCol = 0; iCol < nCols; iCol++)
		m_mtrlListTbl.DeleteColumn(0);
	//if(nCols > 0) return;
	m_mtrlListTbl.DeleteAllItems();

	//m_mtrlListTbl.SetImageList(m_pImageList, LVSIL_NORMAL);
	//m_mtrlListTbl.SetImageList(m_pImageListSmall, LVSIL_SMALL);

	// insert columns (REPORT mode) and modify the new header items
	m_mtrlListTbl.GetWindowRect(&rect);
	m_mtrlListTbl.InsertColumn(0, strItem1, LVCFMT_LEFT, rect.Width() * 1/4, -1);
	m_mtrlListTbl.InsertColumn(1, strItem2, LVCFMT_LEFT, rect.Width() * 1/4, -1);
	m_mtrlListTbl.InsertColumn(2, strItem3, LVCFMT_LEFT, rect.Width() * 1/4, -1);
	m_mtrlListTbl.InsertColumn(3, strItem4, LVCFMT_LEFT, rect.Width() * 1/4, -1);
	m_mtrlListTbl.InsertColumn(4, strItem5, LVCFMT_LEFT, rect.Width() * 1/4, -1);
	m_mtrlListTbl.InsertColumn(5, strItem6, LVCFMT_LEFT, rect.Width() * 1/4, -1);
	m_mtrlListTbl.InsertColumn(6, strItem7, LVCFMT_LEFT, rect.Width() * 1/4, -1);
	//ModifyHeaderItems();

	lvitem.mask = LVIF_TEXT;
	lvitem.iItem = iItem;
	lvitem.iSubItem = iSubItem;
	lvitem.pszText = (LPSTR)malloc(sizeof(char)*256);

	pmaterial = m_pMtrlDlg->m_mtrlObj.m_pMtrlListTop;

	do {
		strcpy(lvitem.pszText, pmaterial->name);
		m_mtrlListTbl.InsertItem(&lvitem);
		sprintf(tmpStr, "%s", pmaterial->family);
		strcpy(lvitem.pszText, tmpStr);
		(lvitem.iSubItem)++;
		m_mtrlListTbl.SetItem(&lvitem);
		sprintf(tmpStr, "%s", pmaterial->type);
		strcpy(lvitem.pszText, tmpStr);
		(lvitem.iSubItem)++;
		m_mtrlListTbl.SetItem(&lvitem);
		if(evaluate) {
			if(pmaterial->pMaterial->alphaline->xmin > 1) xvalNew = 999.0;
			else xvalNew = xval;
			funcvalue = m_pMtrlDlg->m_mtrlObj.MatEval(pmaterial->pMaterial->Egline, xvalNew, yval, zval, wval, &loc_badversion, tmprt);
			sprintf(tmpStr, "%.3g", funcvalue);
		}
		else{
			cur_pInterval = pmaterial->pMaterial->Egline;
			while((xvalNew > cur_pInterval->xmax) && (cur_pInterval != NULL))
			{
				cur_pInterval = cur_pInterval->pNextInterval;
			}
			if(cur_pInterval == NULL)
			{}
			else
			{
				sprintf(tmpStr, "%s", cur_pInterval->expression);
			}
		}
		strcpy(lvitem.pszText, tmpStr);
		(lvitem.iSubItem)++;
		m_mtrlListTbl.SetItem(&lvitem);
		if(evaluate) {
			if(pmaterial->pMaterial->eemline->xmin > 1) xvalNew = 999.0;
			else xvalNew = xval;
			funcvalue = m_pMtrlDlg->m_mtrlObj.MatEval(pmaterial->pMaterial->eemline, xvalNew, yval, zval, wval, &loc_badversion, tmprt);
			sprintf(tmpStr, "%.2g", funcvalue);
		}
		else{sprintf(tmpStr, "%s", pmaterial->pMaterial->eemline->expression);}
		strcpy(lvitem.pszText, tmpStr);
		(lvitem.iSubItem)++;
		m_mtrlListTbl.SetItem(&lvitem);
		if(evaluate) {
			if(pmaterial->pMaterial->eemline->xmin > 1) xvalNew = 999.0;
			else xvalNew = xval;
			funcvalue = m_pMtrlDlg->m_mtrlObj.MatEval(pmaterial->pMaterial->mhhline, xvalNew, yval, zval, wval, &loc_badversion, tmprt);
			sprintf(tmpStr, "%.2g", funcvalue);
		}
		else{sprintf(tmpStr, "%s", pmaterial->pMaterial->mhhline->expression);}
		strcpy(lvitem.pszText, tmpStr);
		(lvitem.iSubItem)++;
		m_mtrlListTbl.SetItem(&lvitem);
		if(evaluate) {
			if(pmaterial->pMaterial->eemline->xmin > 1) xvalNew = 999.0;
			else xvalNew = xval;
			funcvalue = m_pMtrlDlg->m_mtrlObj.MatEval(pmaterial->pMaterial->mlhline, xvalNew, yval, zval, wval, &loc_badversion, tmprt);
			sprintf(tmpStr, "%.2g", funcvalue);
		}
		else{sprintf(tmpStr, "%s", pmaterial->pMaterial->mlhline->expression);}
		strcpy(lvitem.pszText, tmpStr);
		(lvitem.iSubItem)++;
		m_mtrlListTbl.SetItem(&lvitem);
		//if(evaluate) {
		//	if(pmaterial->pMaterial->alphaline->xmin > 1) xvalNew = 999.0;
		//	else xvalNew = xval;
		//	funcvalue = m_pMtrlDlg->m_mtrlObj.MatEval(pmaterial->pMaterial->Edline, xvalNew, yval, zval, wval, &loc_badversion, tmprt);
		//	sprintf(tmpStr, "donor ionization energy (eV) = %.2g", funcvalue);
		//}
		//else{sprintf(tmpStr, "donor ionization energy (eV) = %s", pmaterial->pMaterial->Edline->expression);}
		//pMtrlsTree->InsertItem(TVIF_TEXT,tmpStr, 0, 0, 0, 0, 0, hMatrl, NULL);
		//if(evaluate) {
		//	if(pmaterial->pMaterial->alphaline->xmin > 1) xvalNew = 999.0;
		//	else xvalNew = xval;
		//	funcvalue = MatEval(pmaterial->pMaterial->alphaline, xvalNew, yval, zval, wval, &loc_badversion, tmprt);
		//	sprintf(tmpStr, "absorption (cm^-1) = %g", funcvalue);
		//}
		//else{
		//	cur_pInterval = pmaterial->pMaterial->alphaline;
		//	while((xvalNew > cur_pInterval->xmax) && (cur_pInterval != NULL))
		//	{
		//		cur_pInterval = cur_pInterval->pNextInterval;
		//	}
		//	if(cur_pInterval == NULL)
		//	{}
		//	else
		//	{
		//		sprintf(tmpStr, "absorption (cm^-1) = %s", cur_pInterval->expression);
		//	}
		//}
		//pMtrlsTree->InsertItem(TVIF_TEXT,tmpStr, 0, 0, 0, 0, 0, hMatrl, NULL);
		//if(evaluate) {
		//	if(pmaterial->pMaterial->alphaline->xmin > 1) xvalNew = 999.0;
		//	else xvalNew = xval;
		//	funcvalue = MatEval(pmaterial->pMaterial->Ealine, xvalNew, yval, zval, wval, &loc_badversion, tmprt);
		//	sprintf(tmpStr, "acceptor ionization energy (eV) = %g", funcvalue);
		//}
		//else{sprintf(tmpStr, "acceptor ionization energy (eV) = %s", pmaterial->pMaterial->Ealine->expression);}
		//pMtrlsTree->InsertItem(TVIF_TEXT,tmpStr, 0, 0, 0, 0, 0, hMatrl, NULL);
		//if(evaluate) {
		//	if(pmaterial->pMaterial->alphaline->xmin > 1) xvalNew = 999.0;
		//	else xvalNew = xval;
		//	funcvalue = MatEval(pmaterial->pMaterial->Edaline, xvalNew, yval, zval, wval, &loc_badversion, tmprt);
		//	sprintf(tmpStr, "deep acceptor ionization energy (eV) = %g", funcvalue);
		//}
		//else{sprintf(tmpStr, "deep acceptor ionization energy (eV) = %s", pmaterial->pMaterial->Edaline->expression);}
		//pMtrlsTree->InsertItem(TVIF_TEXT,tmpStr, 0, 0, 0, 0, 0, hMatrl, NULL);
		//if(evaluate) {
		//	if(pmaterial->pMaterial->alphaline->xmin > 1) xvalNew = 999.0;
		//	else xvalNew = xval;
		//	funcvalue = MatEval(pmaterial->pMaterial->Eddline, xvalNew, yval, zval, wval, &loc_badversion, tmprt);
		//	sprintf(tmpStr, "deep donor ionization energy (eV) = %.2g", funcvalue);
		//}
		//else{sprintf(tmpStr, "deep donor ionization energy (eV) = %s", pmaterial->pMaterial->Eddline->expression);}
		//pMtrlsTree->InsertItem(TVIF_TEXT,tmpStr, 0, 0, 0, 0, 0, hMatrl, NULL);
		//else{
		//	cur_pInterval = pmaterial->pMaterial->eemline;
		//	while((xvalNew > cur_pInterval->xmax) && (cur_pInterval != NULL))
		//	{
		//		cur_pInterval = cur_pInterval->pNextInterval;
		//	}
		//	if(cur_pInterval == NULL)
		//	{}
		//	else
		//	{
		//		sprintf(tmpStr, "electron mass (m/m0) = %s", cur_pInterval->expression);
		//	}
		//}
		//pMtrlsTree->InsertItem(TVIF_TEXT,tmpStr, 0, 0, 0, 0, 0, hMatrl, NULL);
		//if(evaluate) {
		//	if(pmaterial->pMaterial->alphaline->xmin > 1) xvalNew = 999.0;
		//	else xvalNew = xval;
		//	funcvalue = MatEval(pmaterial->pMaterial->emobline, xvalNew, yval, zval, wval, &loc_badversion, tmprt);
		//	sprintf(tmpStr, "electron mobility (cm^2/V-s) = %.2g", funcvalue);
		//}
		//else{sprintf(tmpStr, "electron mobility (cm^2/V-s) = %s", pmaterial->pMaterial->emobline->expression);}
		//pMtrlsTree->InsertItem(TVIF_TEXT,tmpStr, 0, 0, 0, 0, 0, hMatrl, NULL);
		//if(evaluate) {
		//	if(pmaterial->pMaterial->alphaline->xmin > 1) xvalNew = 999.0;
		//	else xvalNew = xval;
		//	funcvalue = MatEval(pmaterial->pMaterial->erline, xvalNew, yval, zval, wval, &loc_badversion, tmprt);
		//	sprintf(tmpStr, "relative dielectric constant = %.2g", funcvalue);
		//}
		//else{sprintf(tmpStr, "relative dielectric constant = %s", pmaterial->pMaterial->erline->expression);}
		//pMtrlsTree->InsertItem(TVIF_TEXT,tmpStr, 0, 0, 0, 0, 0, hMatrl, NULL);
		//if(evaluate) {
		//	if(pmaterial->pMaterial->alphaline->xmin > 1) xvalNew = 999.0;
		//	else xvalNew = xval;
		//	funcvalue = MatEval(pmaterial->pMaterial->hmobline, xvalNew, yval, zval, wval, &loc_badversion, tmprt);
		//	sprintf(tmpStr, "hole mobility (cm^2/V-s) = %.2g", funcvalue);
		//}
		//else{sprintf(tmpStr, "hole mobility (cm^2/V-s) = %s", pmaterial->pMaterial->hmobline->expression);}
		//pMtrlsTree->InsertItem(TVIF_TEXT,tmpStr, 0, 0, 0, 0, 0, hMatrl, NULL);
		//if(evaluate) {
		//	if(pmaterial->pMaterial->alphaline->xmin > 1) xvalNew = 999.0;
		//	else xvalNew = xval;
		//	funcvalue = MatEval(pmaterial->pMaterial->mhhline, xvalNew, yval, zval, wval, &loc_badversion, tmprt);
		//	sprintf(tmpStr, "heavy hole mass (m/m0) = %.2g", funcvalue);
		//}
		//else{sprintf(tmpStr, "heavy hole mass (m/m0) = %s", pmaterial->pMaterial->mhhline->expression);}
		//pMtrlsTree->InsertItem(TVIF_TEXT,tmpStr, 0, 0, 0, 0, 0, hMatrl, NULL);
		//if(evaluate) {
		//	if(pmaterial->pMaterial->alphaline->xmin > 1) xvalNew = 999.0;
		//	else xvalNew = xval;
		//	funcvalue = MatEval(pmaterial->pMaterial->mlhline, xvalNew, yval, zval, wval, &loc_badversion, tmprt);
		//	sprintf(tmpStr, "light hole mass (m/m0) = %.2g", funcvalue);
		//}
		//else{sprintf(tmpStr, "light hole mass (m/m0) = %s", pmaterial->pMaterial->mlhline->expression);}
		//pMtrlsTree->InsertItem(TVIF_TEXT,tmpStr, 0, 0, 0, 0, 0, hMatrl, NULL);
		//sprintf(tmpStr, "acceptor concentration (cm^-3) = %.2E", pmaterial->pMaterial->Na);
		//pMtrlsTree->InsertItem(TVIF_TEXT,tmpStr, 0, 0, 0, 0, 0, hMatrl, NULL);
		//sprintf(tmpStr, "donor concentration (cm^-3) = %.2E", pmaterial->pMaterial->Nd);
		//pMtrlsTree->InsertItem(TVIF_TEXT,tmpStr, 0, 0, 0, 0, 0, hMatrl, NULL);
		//sprintf(tmpStr, "deep acceptor concentration (cm^-3) = %.2E", pmaterial->pMaterial->Nda);
		//pMtrlsTree->InsertItem(TVIF_TEXT,tmpStr, 0, 0, 0, 0, 0, hMatrl, NULL);
		//sprintf(tmpStr, "deep donor concentration (cm^-3) = %.2E", pmaterial->pMaterial->Ndd);
		//pMtrlsTree->InsertItem(TVIF_TEXT,tmpStr, 0, 0, 0, 0, 0, hMatrl, NULL);
		//if(evaluate) {
		//	if(pmaterial->pMaterial->alphaline->xmin > 1) xvalNew = 999.0;
		//	else xvalNew = xval;
		//	funcvalue = MatEval(pmaterial->pMaterial->offsetline, xvalNew, yval, zval, wval, &loc_badversion, tmprt);
		//	sprintf(tmpStr, "delta Ec (eV) = %.2g", funcvalue);
		//}
		//else{sprintf(tmpStr, "delta Ec (eV) = %s", pmaterial->pMaterial->offsetline->expression);}
		//pMtrlsTree->InsertItem(TVIF_TEXT,tmpStr, 0, 0, 0, 0, 0, hMatrl, NULL);
		//if(evaluate) {
		//	if(pmaterial->pMaterial->alphaline->xmin > 1) xvalNew = 999.0;
		//	else xvalNew = xval;
		//	funcvalue = MatEval(pmaterial->pMaterial->polarizline, xvalNew, yval, zval, wval, &loc_badversion, tmprt);
		//	sprintf(tmpStr, "polarization = %.2g", funcvalue);
		//}
		//else{sprintf(tmpStr, "polarization = %s", pmaterial->pMaterial->polarizline->expression);}
		//pMtrlsTree->InsertItem(TVIF_TEXT,tmpStr, 0, 0, 0, 0, 0, hMatrl, NULL);
		//if(evaluate) {
		//	if(pmaterial->pMaterial->alphaline->xmin > 1) xvalNew = 999.0;
		//	else xvalNew = xval;
		//	funcvalue = MatEval(pmaterial->pMaterial->taunline, xvalNew, yval, zval, wval, &loc_badversion, tmprt);
		//	sprintf(tmpStr, "electron lifetime (s) = %.2g", funcvalue);
		//}
		//else{sprintf(tmpStr, "electron lifetime (s) = %s", pmaterial->pMaterial->taunline->expression);}
		//pMtrlsTree->InsertItem(TVIF_TEXT,tmpStr, 0, 0, 0, 0, 0, hMatrl, NULL);
		//if(evaluate) {
		//	if(pmaterial->pMaterial->alphaline->xmin > 1) xvalNew = 999.0;
		//	else xvalNew = xval;
		//	funcvalue = MatEval(pmaterial->pMaterial->taupline, xvalNew, yval, zval, wval, &loc_badversion, tmprt);
		//	sprintf(tmpStr, "hole lifetime (s) = %.2g", funcvalue);
		//}
		//else{sprintf(tmpStr, "hole lifetime (s) = %s", pmaterial->pMaterial->taupline->expression);}
		//pMtrlsTree->InsertItem(TVIF_TEXT,tmpStr, 0, 0, 0, 0, 0, hMatrl, NULL);
		//if(evaluate) {
		//	if(pmaterial->pMaterial->alphaline->xmin > 1) xvalNew = 999.0;
		//	else xvalNew = xval;
		//	funcvalue = MatEval(pmaterial->pMaterial->valleyline, xvalNew, yval, zval, wval, &loc_badversion, tmprt);
		//	sprintf(tmpStr, "CB valley degeneracy = %.2g", funcvalue);
		//}
		//else{sprintf(tmpStr, "CB valley degeneracy = %s", pmaterial->pMaterial->valleyline->expression);}
		//pMtrlsTree->InsertItem(TVIF_TEXT,tmpStr, 0, 0, 0, 0, 0, hMatrl, NULL);

		lvitem.iItem++;
		lvitem.iSubItem = 0;
	} while ((pmaterial = pmaterial->pNext_mtrl) != NULL);

	m_mtrlListTbl.UpdateData(true);

	//m_mtrlListTbl.SetScrollPos(SB_HORZ, scrollPos);
}

void CMtrlTblDlg::OnEvalMtrls()
{
	//UpdateData(false);

	//if(m_bEval == true) {
	//	m_bEval = false;}
	//else {
	//	m_bEval = true;}

	//UpdateData(true);

	FillMtrlTbl();
}
