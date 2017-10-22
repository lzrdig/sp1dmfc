// sp1dll.h : main header file for the sp1dll DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols
#include "schrpois.h"		// for CShcrPois class
//#include "extcode.h"		// CIN funcs from LabView


extern "C" __declspec(dllexport) bool ShowSP2Dlg();
//extern "C" __declspec(dllexport) void ShowSP1Dlg(LStrHandle matrlFileName, LStrHandle inputFileName, LStrHandle workingDir);
extern "C" __declspec(dllexport) CSchrPois * GetSPobjPtr();
extern "C" __declspec(dllexport) bool SetSPobjPtr(CSchrPois * spPtr);


// Csp1dllApp
// See sp1dll.cpp for the implementation of this class
//

class Csp1dllApp : public CWinApp
{
public:
	Csp1dllApp();

// Overrides
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
