// CTestDLLDoc.cpp : implementation of the CTestDLLDoc class
//

#include "stdafx.h"
#include "TestSDIdll.h"

#include "CTestDLLDoc.h"
#include ".\ctestdlldoc.h"
#include "..\sp1dll\sp1dll.h"
#include "..\sp1dll\SchrPois.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


//CSchrPois objSP;


// CTestDLLDoc

IMPLEMENT_DYNCREATE(CTestDLLDoc, CDocument)

BEGIN_MESSAGE_MAP(CTestDLLDoc, CDocument)
	ON_COMMAND(ID_OPENTESTDIALOG, OnOpenTestDialog)
END_MESSAGE_MAP()


// CTestDLLDoc construction/destruction

CTestDLLDoc::CTestDLLDoc()
{
	// TODO: add one-time construction code here

}

CTestDLLDoc::~CTestDLLDoc()
{
}

BOOL CTestDLLDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}




// CTestDLLDoc serialization

void CTestDLLDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}


// CTestDLLDoc diagnostics

#ifdef _DEBUG
void CTestDLLDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CTestDLLDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CTestDLLDoc commands

void CTestDLLDoc::OnOpenTestDialog()
{
	// TODO: Add your command handler code here
	CSchrPois *p_objSP;

	//uChar dum[10];
	//LStrHandle lvh_str1;
	//LStrPtr lv_strPtr;
	//LStr lv_str;

	p_objSP = GetSPobjPtr();

	//strcpy((char*)dum,_T("test12345"));

	//lv_str.cnt = strlen((char *)dum);
	//strcpy((char*)lv_str.str,(char *)dum);
	//lv_strPtr = &lv_str;
	//lvh_str1 = &(lv_strPtr);

	//ShowSP1Dlg(lvh_str1,lvh_str1,lvh_str1);
	ShowSP2Dlg();

	//lv_str.cnt = strlen((char *)dum);
}
