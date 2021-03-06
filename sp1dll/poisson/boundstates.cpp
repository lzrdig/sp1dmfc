#include "stdafx.h"

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

#include "poisp_types.h"
#include "poisp_defs.h"

#include "..\schrpois.h"
#include "..\sp1dll.h"
#include "..\schrdlg.h"
#include "..\ergdlg.h"


/**************************************************/
int CSchrPois::boundstates(struct global_type *pGlobals, struct flag_type *pFlags) {
  
/* this subroutine finds the electron and hole bound states of a previously calculated structure */
	
	char ans,statesname[MAX_FILENAME],charDummy[MAX_EXTENSION],extension[MAX_EXTENSION];
	char access,info[MAX_INPUTLINE],message[MAX_MESSAGE];
	int fieldlength,fixBounds,i,j,count,startsearch;
	double nthird,pthird,*pOldn,*pOldderiv,tem;
	struct mesh_type *pMesh;
	int ioerr_loc = 0;
	
	fixBounds = true;  /* Upper and lower bounds of subband search are fixed. */
	count=0;
	ans='\0';
	
/*	first check to see that there is an input file */
	if (pFlags->noinputfile) {
		strcpy(message,"You Must First Choose an Input File");
		alertboxWin(message);
		return 1;
	}
  
/*  if the inputfile is a multiple voltage run, ask for the proper .out file to use 
	if(pFlags->multiruns) {
		strcpy(message,"Select the .out file that you wish to use...");
		alertbox(message);
	  pGlobals->filename= '*'
	  access = 'notappend'
	  status = 'old'
	  action = 'readwrite'
	  extension = ' '
	  call fileopen(4, pGlobals->filename, access, status, action, extension,ioerr)
	  inquire (4,name=statesname)
	  statesname(mytrim(statesname)-3:mytrim(statesname)) = '    '
	  if (trim(pGlobals->filename) .NE. statesname(1:mytrim(pGlobals->filename))) then
	    message = 'OK smarty pants!  The .out file must match the input file.'
	    call alertbox(message)
	    return
	  end if
	}
	else {
  
/*  Next, check to see if an output file exists for the choosen input file  
!  If not, ask the user to run the poisson program first

	  if (comp .eq. 'mac') then
	    name2 = trim(pGlobals->filename)//'.out'
	  else
	    extension = '_O'
	    name2= pGlobals->filename
	    call getfilename(name2,extension)
	  end if
	  inquire(file=trim(name2),exist=pFlags->outfileexists)
	  if (.NOT. pFlags->outfileexists) then
	    message = 'You must First Run the Poisson Solver'
	    call alertbox(message)
	    return
	  end if
	  open (4,file=trim(name2),status='old')
	end if
	
	write (*,*) ' ' */
	access='r';
	strcpy(extension,"_Out");
	ioerr_loc = fileopen(4, pGlobals->filename, access, extension, pGlobals, "");
	if(ioerr_loc != 0) return 1;
	
/* if the input file defines a pFlags->schrodinger range, see if the user wants it changed */
	CSchrDlg schrDlg(NULL, pGlobals);
	schrDlg.m_schrStart = pGlobals->pSchStart->meshcoord;
	schrDlg.m_schrStop = pGlobals->pSchStop->meshcoord;
	if((schrDlg.DoModal() == IDCANCEL) || (!pFlags->schrodinger))
	{
		pFlags->schMemAlloc = false;
		pGlobals->pSchStop = ymesh(pGlobals->realschstop,pGlobals);
		pGlobals->pSchStart = ymesh(pGlobals->realschstart,pGlobals);
	}

/* set the pFlags->schrodinger flag so that the proper output files are written */
	pFlags->schrodinger = true; 

/* Find the pGlobals->order of the Schrodinger matrices*/
	pGlobals->order =1;
	pMesh = pGlobals->pSchStart;
	while ((pMesh = pMesh->pDownMesh) != pGlobals->pSchStop) 
		pGlobals->order = pGlobals->order +1;
	pGlobals->order = pGlobals->order + 1;

	if (pGlobals->order > 0 && !pFlags->schMemAlloc) { 
		pFlags->schrodinger = true;
		pGlobals->pPotential = (double *) malloc(pGlobals->order*sizeof(double));
		if (pGlobals->pPotential == NULL) {
			strcpy(message,"Can't allocate memory for Schrodinger array.  Increase Memory allocation.");
			alertboxWin(message);
			pFlags->stopthemusic = true;
			return 1;
		}
		pGlobals->pMass = (double *) malloc(pGlobals->order*sizeof(double));
		if (pGlobals->pMass == NULL) {
			strcpy(message,"Can't allocate memory for Schrodinger array.  Increase Memory allocation.");
			alertboxWin(message);
			pFlags->stopthemusic = true;
			return 1;
		}
		pGlobals->pSdy = (double *) malloc(pGlobals->order*sizeof(double));
		if (pGlobals->pSdy == NULL) {
			strcpy(message,"Can't allocate memory for Schrodinger array.  Increase Memory allocation.");
			alertboxWin(message);
			pFlags->stopthemusic = true;
			return 1;
		}
		pGlobals->pEval = (double *) malloc(pGlobals->order*sizeof(double));
		if (pGlobals->pEval == NULL) {
			strcpy(message,"Can't allocate memory for Schrodinger array.  Increase Memory allocation.");
			alertboxWin(message);
			pFlags->stopthemusic = true;
			return 1;
		}
		pGlobals->pEvcharge = (double *) malloc(pGlobals->order*sizeof(double));
		if (pGlobals->pEvcharge == NULL) {
			strcpy(message,"Can't allocate memory for Schrodinger array.  Increase Memory allocation.");
			alertboxWin(message);
			pFlags->stopthemusic = true;
			return 1;
		}
		pGlobals->pEvderiv = (double *) malloc(pGlobals->order*sizeof(double));
		if (pGlobals->pEvderiv == NULL) {
			strcpy(message,"Can't allocate memory for Schrodinger array.  Increase Memory allocation.");
			alertboxWin(message);
			pFlags->stopthemusic = true;
			return 1;
		}
	}
	else if (pGlobals->order <= 0) {
		strcpy(message,"Schrodinger interval must be at least one mesh point.");
		alertboxWin(message);
		pFlags->stopthemusic = true;
		return 1;
	}
	
/* Check for a flat Fermi Level in the Schrodinger region */
	tem = 1.e-5*((int)(1.e5*pGlobals->pSchStart->Efn+0.5e0));
	pMesh = pGlobals->pSchStart;
	while ((pMesh = pMesh->pDownMesh) != pGlobals->pSchStop) {
		if (1.e-5*((int) (1.e5*pMesh->Efn+0.5e0)) != tem) {
			strcpy(message,"The Fermi level must be constant in the Schrodinger region");
			alertboxWin(message);
			pFlags->stopthemusic = true;
			return 1;
		}
	}

	if (pFlags->Asterix) getInput(4,info,pGlobals);  /* Skip the heading */
	
	fieldlength = getInput(4,info,pGlobals);  /* Skip labels */
	pMesh = pGlobals->pStartSim;
	do {  
		fieldlength = getInput(4,info,pGlobals);
		startsearch = 0;
		fieldlength = getSingField(charDummy,info,&startsearch); /* Skip y coordinate*/
		fieldlength = getSingField(charDummy,info,&startsearch); /* This is the Conduction band */
		pMesh->v = numconv(charDummy) - pMesh->offset; /* convert to potential */
    } while ((pMesh = pMesh->pDownMesh) != pGlobals->pBottomMesh);
	
	pMesh = pGlobals->pBottomMesh;  
	startsearch = 0;
	fieldlength = getInput(4,info,pGlobals); /*Get the last line */
	fieldlength = getSingField(charDummy,info,&startsearch); /* Skip y coordinate*/
	fieldlength = getSingField(charDummy,info,&startsearch); /* This is the Conduction band */
	pMesh->v = numconv(charDummy) - pMesh->offset; /* convert to potential */
	
	if (!strncmp(pGlobals->subcontact,"slope",5)) pMesh->pDownMesh->v = pMesh->v;
	
  
/*	Find the eigenvalue search range.  The default range finds the lowest and highest  
	points in the potential and uses the lowest point as the lower bound, and 30meV  
	below the highest point as the upper bound.  The user can also specify the bounds. */
	

/* Allocate workspace memory to be used in the Schrodinger solver */
	pOldn = (double *) malloc(pGlobals->order*sizeof(double));
	if (pOldn == NULL) {
		strcpy(message,"Can't allocate memory for Schrodinger array (pOldn).  Increase Memory allocation.");
		alertboxWin(message);
		return 1;
	}
	pOldderiv = (double *) malloc(pGlobals->order*sizeof(double));
	if (pOldderiv == NULL) {
		strcpy(message,"Can't allocate memory for Schrodinger array (pOldderiv).  Increase Memory allocation.");
		alertboxWin(message);
		return 1;
	}
	
	/*  do electrons first */

	pMesh = pGlobals->pSchStart;
	for (i=0; i<pGlobals->order; i++) {
		pOldn[i] = pMesh->n;
		pOldderiv[i] = pMesh->deriv;
		pGlobals->pMass[i] = pMesh->eem;
		if (pFlags->exchange && (pOldn[i] > 1.0e0)) {
			nthird = pow(pMesh->n,(1.e0/3.e0));
			pGlobals->pPotential[i] = pMesh->v+pMesh->offset-1.256e-20*nthird/pMesh->esemi*(1.0e0+0.0594e0/
 				(nthird*pMesh->aBohrElec)*log(1.0e0+13.0274e0*pMesh->aBohrElec*nthird));
		}
		else
			pGlobals->pPotential[i] = pMesh->v+pMesh->offset;
	  
		pGlobals->pSdy[i] = pMesh->dy;
		pMesh = pMesh->pDownMesh;
	}
	pGlobals->LB = pGlobals->pPotential[0];
	pGlobals->UB = pGlobals->pPotential[0];
	
	for(i=1; i<pGlobals->order; i++) {
		if (pGlobals->pPotential[i] < pGlobals->LB) 	/* look for lowest point in band diagram */
			pGlobals->LB = pGlobals->pPotential[i];
	}
	
	if (pGlobals->pPotential[pGlobals->order-1] < pGlobals->UB) pGlobals->UB = pGlobals->pPotential[pGlobals->order-1];	/* find least upper bound  */

	
/*	 See if the user wants to change the energy range to search */
	CErgDlg ergDlg(NULL, pGlobals, "Energy search range for electrons");
	CString tmp;
	tmp.Format("%.2f", pGlobals->LB); 
	ergDlg.m_ergLower = tmp;
	tmp.Format("%.2f", pGlobals->UB); 
	ergDlg.m_ergUpper = tmp;
	ergDlg.DoModal();
	pGlobals->LB = atof(ergDlg.m_ergLower);
	pGlobals->UB = atof(ergDlg.m_ergUpper);

/*	Calculate the sheet charge for selected energy range*/	
	PostInfoSP("Calculating sheet charge for electrons");
	ioerr_loc = schcharge('e',fixBounds,pGlobals,pFlags); 
	if (ioerr_loc) {
		strcpy(message,"Error in electron Schrodinger solution.");
		alertboxWin(message);
		return 1;		/* stop the calculation if error */
	}

/* save the electron eigenvalues and eigenvectors */
	if (pGlobals->numeval > 0) {
		pGlobals->pEleceval = (double *) malloc(pGlobals->numeval*sizeof(double));
		pGlobals->pElecevec = (double **) malloc(pGlobals->order*sizeof(double *));
		if (pGlobals->pElecevec == NULL) {
			strcpy(message,"Can't allocate memory for electron wavefunctions.  Increase Memory allocation.");
			alertboxWin(message);
			return 1;
		}	
		for (i=0; i<pGlobals->order; i++) {
			pGlobals->pElecevec[i] = (double *) malloc(pGlobals->numeval*sizeof(double));
			if (pGlobals->pElecevec[i] == NULL) {
				strcpy(message,"Can't allocate memory for electron wavefunctions.  Increase Memory allocation.");
				alertboxWin(message);
				return 1;
			}	
		}
		pGlobals->numeleceval = pGlobals->numeval;
		for (i=0;i<pGlobals->numeleceval;i++) {
			pGlobals->pEleceval[i] = pGlobals->pEval[i];
			for (j=0; j<pGlobals->order; j++) 
				pGlobals->pElecevec[j][i] = pGlobals->pEigenv[j][i];
		}	
		for (i=0; i<pGlobals->order; i++)
			free(pGlobals->pEigenv[i]);
		free(pGlobals->pEigenv);
	}
	  
/*  Now do the holes */
	pMesh = pGlobals->pSchStart;
	for (i=0; i<pGlobals->order; i++) {
		pGlobals->pMass[i] = pMesh->ehm;
		if (pFlags->exchange && (pMesh->p > 1.0e0)) {
			pthird = pow(pMesh->p,(1.e0/3.e0));
			pGlobals->pPotential[i] = -(pMesh->v+pMesh->offset-pMesh->Eg-1.256e-20*pthird/pMesh->esemi
				*(1.0e0+0.034e0/(pMesh->aBohrHhole*pthird)*log(1.0e0+18.376e0*pMesh->aBohrHhole*pthird)));
		}
		else
			pGlobals->pPotential[i] = -(pMesh->v+pMesh->offset-pMesh->Eg);
		pMesh->p = 1.0e-10;
		pMesh = pMesh->pDownMesh;
	}
	pGlobals->LB = pGlobals->pPotential[0];
	pGlobals->UB = pGlobals->pPotential[0];
	
	for(i=1; i<pGlobals->order; i++) {
		if (pGlobals->pPotential[i] < pGlobals->LB) 	/* look for lowest point in band diagram */
			pGlobals->LB = pGlobals->pPotential[i];
	}
	
	if (pGlobals->pPotential[pGlobals->order-1] < pGlobals->UB) pGlobals->UB = pGlobals->pPotential[pGlobals->order-1];	/* find least upper bound  */
	
/*	 See if the user wants to change the energy range to search */
	ergDlg.m_sCaption = "Energy search range for holes";
	tmp.Format("%.2f", pGlobals->LB); 
	ergDlg.m_ergLower = tmp;
	tmp.Format("%.2f", pGlobals->UB); 
	ergDlg.m_ergUpper = tmp;
	ergDlg.DoModal();
	pGlobals->LB = atof(ergDlg.m_ergLower);
	pGlobals->UB = atof(ergDlg.m_ergUpper);

	pGlobals->numhheval = 0;	
	pGlobals->numeval = 0;
	
	ioerr_loc = schcharge('h', fixBounds,pGlobals,pFlags); 
	if (ioerr_loc) {
		strcpy(message,"Error in heavy hole Schrodinger solution.");
		alertboxWin(message);
		return 1;		/* stop the calculation if error */
	}

/* save the heavy hole eigenvalues eigenvectors */
	if (pGlobals->numeval > 0) {
		pGlobals->pHheval = (double *) malloc(pGlobals->numeval*sizeof(double));
		pGlobals->pHhevec = (double **) malloc(pGlobals->order*sizeof(double *));
		if (pGlobals->pHhevec == NULL) {
			strcpy(message,"Can't allocate memory for heavy hole wavefunctions.  Increase Memory allocation.");
			alertboxWin(message);
			return 1;
		}	
		for (i=0; i<pGlobals->order; i++) {
			pGlobals->pHhevec[i] = (double *) malloc(pGlobals->numeval*sizeof(double));
			if (pGlobals->pHhevec[i] == NULL) {
				strcpy(message,"Can't allocate memory for heavy hole wavefunctions.  Increase Memory allocation.");
				alertboxWin(message);
				return 1;
			}	
		}
		pGlobals->numhheval = pGlobals->numeval;
		for (i=0;i<pGlobals->numhheval;i++) {
			pGlobals->pHheval[i] = -pGlobals->pEval[i];
			for (j=0; j<pGlobals->order; j++) 
				pGlobals->pHhevec[j][i] = pGlobals->pEigenv[j][i];
		}	
		for (i=0; i<pGlobals->order; i++)
			free(pGlobals->pEigenv[i]);
		free(pGlobals->pEigenv);
	}
	

/*  Now do the light holes */
	pMesh = pGlobals->pSchStart;
	for (i=0; i<pGlobals->order; i++) {
		pGlobals->pMass[i] = pMesh->elhm;
	}
	pGlobals->numlheval = 0;	
	pGlobals->numeval = 0;
	
	ioerr_loc = schcharge('l', fixBounds,pGlobals,pFlags); 
	if (ioerr_loc) {
		strcpy(message,"Error in light hole Schrodinger solution.");
		alertboxWin(message);
		return 1;		/* stop the calculation if error */
	}

/* save the light hole eigenvalues eigenvectors */
	if (pGlobals->numeval > 0) {
		pGlobals->pLheval = (double *) malloc(pGlobals->numeval*sizeof(double));
		pGlobals->pLhevec = (double **) malloc(pGlobals->order*sizeof(double *));
		if (pGlobals->pLhevec == NULL) {
			strcpy(message,"Can't allocate memory for light hole wavefunctions.  Increase Memory allocation.");
			alertboxWin(message);
			return 1;
		}	
		for (i=0; i<pGlobals->order; i++) {
			pGlobals->pLhevec[i] = (double *) malloc(pGlobals->numeval*sizeof(double));
			if (pGlobals->pLhevec[i] == NULL) {
				strcpy(message,"Can't allocate memory for light hole wavefunctions.  Increase Memory allocation.");
				alertboxWin(message);
				return 1;
			}	
		}
		pGlobals->numlheval = pGlobals->numeval;
		for (i=0;i<pGlobals->numlheval;i++) {
			pGlobals->pLheval[i] = -pGlobals->pEval[i];
			for (j=0; j<pGlobals->order; j++) 
				pGlobals->pLhevec[j][i] = pGlobals->pEigenv[j][i];
		}	
		for (i=0; i<pGlobals->order; i++)
			free(pGlobals->pEigenv[i]);
		free(pGlobals->pEigenv);
	}
	
	/* Free up the workspace memory */
	free(pOldn);
	free(pOldderiv);
	
	if (!pFlags->schrodinger) {
		pFlags->schrodinger = true;
	}

	strcpy(extension,"_St");
	if (!pFlags->multiruns)		strcpy(statesname,pGlobals->filename);
	sprintf(message, "Writing bound states to files beginning with %s%s", statesname, extension);
	PostInfoSP(message);
	ioerr_loc = output(statesname,extension,count,pGlobals,pFlags);	/* output the bound states data */
		
	return 0;
}
	