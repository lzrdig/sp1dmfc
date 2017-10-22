// sp1dll.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "sp1dll.h"
#include "SPdlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//
//	Note!
//
//		If this DLL is dynamically linked against the MFC
//		DLLs, any functions exported from this DLL which
//		call into MFC must have the AFX_MANAGE_STATE macro
//		added at the very beginning of the function.
//
//		For example:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// normal function body here
//		}
//
//		It is very important that this macro appear in each
//		function, prior to any calls into MFC.  This means that
//		it must appear as the first statement within the 
//		function, even before any object variable declarations
//		as their constructors may generate calls into the MFC
//		DLL.
//
//		Please see MFC Technical Notes 33 and 58 for additional
//		details.
//


// Csp1dllApp

BEGIN_MESSAGE_MAP(Csp1dllApp, CWinApp)
END_MESSAGE_MAP()


// Csp1dllApp construction

Csp1dllApp::Csp1dllApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only Csp1dllApp object
Csp1dllApp theApp;

// The one and only CSchrPois object (global) that does the actual calculation
CSchrPois theSP;

// Global string for the notification info. Can be passed in and out of dll(?).
CString g_infoStr;

// Paths to material and input files passed in from the calling application. Can be modified from inside dll.
char g_matrlFname[MAX_FILENAME];
char g_inputFname[MAX_FILENAME];
char g_workDir[MAX_FILENAME];

// Pointer (global) to the dialog that will be shown
CSPdlg *p_spDlg;

// Csp1dllApp initialization

BOOL Csp1dllApp::InitInstance()
{
	CWinApp::InitInstance();

	return TRUE;
}


//void ShowSP1Dlg(LStrHandle matrlFileNameLV, LStrHandle inputFileNameLV, LStrHandle workingDirLV)
//{
//	AFX_MANAGE_STATE(AfxGetStaticModuleState());
//
//	// this is used if char* is being used to pass in strings (keep in mind that the size is fixed)
//	// the following crashes if LabView sends in a string of size smaller that g_matrlFname or g_inputFname
//	//strcpy(g_matrlFname, matrlFileName);
//	//strcpy(g_inputFname, inputFileName);
//
//	strncpy(g_matrlFname, (char *)LHStrBuf(matrlFileNameLV), LStrLen(*matrlFileNameLV));
//	strcat(g_matrlFname, "\0");
//	strncpy(g_inputFname, (char *)LHStrBuf(inputFileNameLV), LStrLen(*inputFileNameLV));
//	strcat(g_inputFname, "\0");
//
//	theSP.InitSPobj();
//
//	p_spDlg = new CSPdlg();
//	p_spDlg->Create(IDD_SPDIALOG,NULL);
//	p_spDlg->ShowWindow(SW_SHOW);
//	
//	// in order to not have to deal with fixed size strings passed between LabView and C do the following
//	int newsize = strlen(g_matrlFname);
//	if(NumericArrayResize(uB, 1L,(UHandle*)&matrlFileNameLV, newsize) == noErr)
//	{
//		MoveBlock(g_matrlFname, LStrBuf(*matrlFileNameLV), newsize);
//		LStrLen(*matrlFileNameLV) = newsize;
//	}
//	newsize = strlen(g_inputFname);
//	if(NumericArrayResize(uB, 1L,(UHandle*)&inputFileNameLV, newsize) == noErr)
//	{
//		MoveBlock(g_inputFname, LStrBuf(*inputFileNameLV), newsize);
//		LStrLen(*inputFileNameLV) = newsize;
//	}
//	newsize = strlen(g_workDir);
//	if(NumericArrayResize(uB, 1L,(UHandle*)&workingDirLV, newsize) == noErr)
//	{
//		MoveBlock(g_workDir, LStrBuf(*workingDirLV), newsize);
//		LStrLen(*workingDirLV) = newsize;
//	}
//}

bool ShowSP2Dlg()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	CSPdlg spDlg;

	theSP.InitSPobj();

	p_spDlg = new CSPdlg();
	p_spDlg->Create(IDD_SPDIALOG,NULL);
	p_spDlg->ShowWindow(SW_SHOW);

	return TRUE;
}

CSchrPois * GetSPobjPtr()
{
	return &theSP;
}


bool SetSPobjPtr(CSchrPois * spPtr)
{
	if(spPtr == NULL)
	{
		return false;
	}
	else
	{
		//free(theSP);	//?

		//&theSP = spPtr;

		return true;
	}
}
