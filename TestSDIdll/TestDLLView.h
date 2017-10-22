// TestDLLView.h : interface of the CTestDLLlView class
//


#pragma once


class CTestDLLlView : public CView
{
protected: // create from serialization only
	CTestDLLlView();
	DECLARE_DYNCREATE(CTestDLLlView)

// Attributes
public:
	CTestDLLDoc* GetDocument() const;

// Operations
public:

// Overrides
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:

// Implementation
public:
	virtual ~CTestDLLlView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in TestDLLView.cpp
inline CTestDLLDoc* CTestDLLlView::GetDocument() const
   { return reinterpret_cast<CTestDLLDoc*>(m_pDocument); }
#endif

