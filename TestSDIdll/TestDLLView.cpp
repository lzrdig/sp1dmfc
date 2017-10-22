// TestDLLView.cpp : implementation of the CTestDLLlView class
//

#include "stdafx.h"
#include "TestSDIdll.h"

#include "CTestDLLDoc.h"
#include "TestDLLView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CTestDLLlView

IMPLEMENT_DYNCREATE(CTestDLLlView, CView)

BEGIN_MESSAGE_MAP(CTestDLLlView, CView)
END_MESSAGE_MAP()

// CTestDLLlView construction/destruction

CTestDLLlView::CTestDLLlView()
{
	// TODO: add construction code here

}

CTestDLLlView::~CTestDLLlView()
{
}

BOOL CTestDLLlView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

// CTestDLLlView drawing

void CTestDLLlView::OnDraw(CDC* /*pDC*/)
{
	CTestDLLDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: add draw code for native data here
}


// CTestDLLlView diagnostics

#ifdef _DEBUG
void CTestDLLlView::AssertValid() const
{
	CView::AssertValid();
}

void CTestDLLlView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CTestDLLDoc* CTestDLLlView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CTestDLLDoc)));
	return (CTestDLLDoc*)m_pDocument;
}
#endif //_DEBUG


// CTestDLLlView message handlers
