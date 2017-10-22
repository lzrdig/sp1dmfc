// TestSDIdll.h : main header file for the TestSDIdll application
//
#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols


// CTestDLLApp:
// See TestSDIdll.cpp for the implementation of this class
//

class CTestDLLApp : public CWinApp
{
public:
	CTestDLLApp();


// Overrides
public:
	virtual BOOL InitInstance();

// Implementation
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CTestDLLApp theApp;