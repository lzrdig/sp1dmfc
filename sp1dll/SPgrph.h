#pragma once

#include "ResizableDialog.h"
#include "ChartCtrl.h"
#include "ColourPicker.h"
#include "ChartLineSerie.h"
#include "ChartPointsSerie.h"
#include "browsectrl.h"
#include "spdlg.h"
#include "afxwin.h"

class CSPdlg;

// CGrphDlg dialog

class CGrphDlg : public CResizableDialog
{
	DECLARE_DYNAMIC(CGrphDlg)

public:
	CGrphDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CGrphDlg();

// Dialog Data
	enum { IDD = IDD_GRAPHDLG };


// Funcs
	BOOL Create(CSPdlg * pSPdlg);
	virtual BOOL Create(LPCTSTR lpszTemplateName, CWnd* pParentWnd = NULL);
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void PostNcDestroy();
	virtual void OnCancel();
	virtual void OnOK();

	DECLARE_MESSAGE_MAP()

	CWnd *m_pParentWnd;
	int m_nID;
public:
	afx_msg void OnPlot();
	afx_msg void OnCbnSelchangeEigens();
	afx_msg void OnLbnSelchangeWavefuncs();
	afx_msg void OnBnClickedClrgraph();
	afx_msg void OnBnClickedEcond();
	afx_msg void OnBnClickedEfermi();
	afx_msg void OnBnClickedEval();
	afx_msg void OnBnClickedChrgn();
	afx_msg void OnBnClickedOvrlay();
	afx_msg void OnBnClickedChrgp();
	afx_msg void OnBnClickedChrgf();
	afx_msg void OnBnClickedZoom();
	afx_msg void OnBnClickedPan();
	afx_msg void OnBnClickedAutoscale();

	CBrowseCtrl m_grphFile;
	CSPdlg * m_pSPdlg;
	CChartCtrl m_ChartCtrl;
	CButton m_ctrlEc;
	CButton m_ctrlEv;
	CButton m_ctrlEf;
	CButton m_ctrlN;
	CButton m_ctrlP;
	CButton m_ctrlF;
	CButton m_ctrlEfield;
	CCheckListBox m_cEigenvals;
	CCheckListBox m_ctrlWavefuncs;
	CButton m_cOverlay1;

	// Function to plot potential profile
	int PlotPotentials(bool clrGraph);
	// Plots the eigenvalues
	int PlotEigens(bool clrGraph);
	// Plot eigen wavefunctions
	int PlotEigenWaves(bool clrGraph);
	// Unchecks the checkboxes for Potential plots
	int UncheckPotentialBoxes(void);
	// Unchecks the boxes for the Eigenvalues in the eigenval listbox
	int UncheckEigensBox(void);
	// Unchecks boxes for the eigenvectors in the Eigenwaves listbox
	int UncheckEigenwavesBox(void);
	// Plots all checked plots
	int PlotAllOverlayed(void);
};
