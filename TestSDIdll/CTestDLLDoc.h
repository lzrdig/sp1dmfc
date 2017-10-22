// CTestDLLDoc.h : interface of the CTestDLLDoc class
//


#pragma once

class CTestDLLDoc : public CDocument
{
protected: // create from serialization only
	CTestDLLDoc();
	DECLARE_DYNCREATE(CTestDLLDoc)

// Attributes
public:

// Operations
public:

// Overrides
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);

// Implementation
public:
	virtual ~CTestDLLDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnOpenTestDialog();
};


