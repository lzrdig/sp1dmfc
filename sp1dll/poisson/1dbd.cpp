#include "stdafx.h"

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

#include "poisp_types.h"
#include "poisp_defs.h"

#include "..\schrpois.h"


int count;

/******************************************************************************/
/*  this is the main subroutine for the 1D poisson/pFlags->schrodinger solver */
/******************************************************************************/
int CSchrPois::onedpoisson(struct global_type *pGlobals, struct flag_type *pFlags) {
	char message[MAX_MESSAGE];
	char extension[MAX_EXTENSION];
	int cvcount;
	int error;
	//char voltconv[MAX_VOLTSOURCES];
	//char **pCVdata;
	//char multirunname[MAX_FILENAME];	/* pGlobals->filename for pFlags->multiruns*/
	//double initcharge;	/* charge of the initial run*/
	//double etchposition;
	//double nsTotal,sheetres;
	//double psTotal;
	//int multicount,multicountmax;

	PostInfoSP(_T("1D_Poisson: Starting 1D poisson solver"));

	/*	first check to see that there is an input file */
	if (pFlags->noinputfile) {
		strcpy(message,"You Must First Choose an Input File");
		alertboxWin(message);
		return 1;
	}
	
	/*if (pFlags->cleanFiles) cleanup();  /*Delete out of date output files */
	
	pFlags->firstcv = true;		/* set first iteration flag for CV runs */
	pFlags->firstetch = true;	/* set first iteration flag for etch runs */
	cvcount = 1;				/* set default cvcount */
	pGlobals->pStartSim = pGlobals->pTopMesh;

	if (pFlags->multiruns) 
		error = multiruns(pGlobals, pFlags);
	else if (pFlags->etched)
	{ // then
//		etchposition = 0.0;	//etchposition =0.0d0
//		do
//		{	//do 200
//			pGlobals->etchdepth =+ pGlobals->etchinc;	   //pGlobals->etchdepth,pGlobals->etchinc
//
//			/*  The first thing is to find where, in terms of mesh points, 
//			the current surface falls. */
//			pGlobals->pStartSim = ymesh(etchposition, pGlobals);
//
//			///*  Next find the effects of the etch on boundaries of the pFlags->schrodinger mesh */
//			//if ((pGlobals->pStartSim > schstart) && (pGlobals->pStartSim <= schstop)){
//			//	schstart = pGlobals->pStartSim;	/* top of the pFlags->schrodinger mesh has been pFlags->etched */
//			//}
//			//else if (pGlobals->pStartSim > schstop)	{
//			//	pFlags->schrodinger = false;	/* pFlags->schrodinger mesh has been pFlags->etched through */
//			//}
//
//			//i = pGlobals->pStartSim;
//			//if (pFlags->usesurfbar){ 
//			//	v(i) = pGlobals->surfbar-pGlobals->vbias(pGlobals->surfvoltage)-offset(layer(i));}
//			//else{
//			//	v(i) = bar(layer(i))-pGlobals->vbias(pGlobals->surfvoltage)-offset(layer(i));	/* surface voltage, including quasipotential */
//			//}
//			//solve (pGlobals->converg);	/* Solve Poi/Sch for this etch position */
//			//if (pFlags->findBst) 
//			//	boundstates();	/* find all bound states if requested */
//
//	/*	    write(voltconv,11) etchposition
//			voltconv = adjustl(voltconv)
//			if (comp .eq. 'mac') then
//			  extension = '.E'//trim(voltconv)
//			else
//			  extension = trim(voltconv)
//			end if
//		    	    
//			if (pFlags->etchonly .AND. pFlags->firstetch) then
//			  call output(pGlobals->filename,extension)	/*Output the first band diagram */
//	/*	      call etchout(pGlobals->filename,etchposition)
//			else if (pFlags->etchonly) then
//			  call etchout(pGlobals->filename,etchposition)	/* Output only the sheet resistance */
//	/*	    else  
//			  call output(pGlobals->filename,extension)		/* output results */
//	/*	      call etchout(pGlobals->filename,etchposition)
//			end if
//
//			write (*,*) 'Calculation '//trim(voltconv)//'Completed'
//			write (*,*) ' '
//			if (pFlags->DangerWill .AND. pFlags->ignoreDanger) then
//			  message = "Final structure has a confinement problem in the Schrodinger mesh"
//			  call alertbox(message)
//			end if
//200	  continue */
//		} while(;)
	}
	else {
		PostInfoSP("1D_Poisson: starting solve routine");
		error = solve(pGlobals->converg,pGlobals, pFlags);			/* Normal run, nothing fancy */
		
		sprintf(message, "1D_Poisson: Writing output to files beginning with %s", pGlobals->filename);
		PostInfoSP(message);
		/* output results, normal run */
		output(pGlobals->filename, "",count,pGlobals,pFlags);	
		if (error) {
			sprintf(message, "Writing errored solver output to the file %s%s", pGlobals->filename,extension);
			PostInfoSP(message);
			return 1;
		}
		PostInfoSP("1D_Poisson: Completed solve routine");


		if (pFlags->findBst){
			PostInfoSP(_T("1D_Poisson: Calculating bound states (onedpoisson)"));
			error = boundstates(pGlobals,pFlags);			/* find all bound states if requested */
			if(!error)
				PostInfoSP(_T("1D_Poisson: boundstates calculated"));
		}
		strcpy(message,"1D_Poisson: Calculation Completed");
		alertboxWin(message);
		if (pFlags->DangerWill && pFlags->ignoreDanger) {
			strcpy(message,"1D_Poisson: Final structure has a confinement problem in the Schrodinger mesh");
			alertboxWin(message);
		}
	}

	return 0;
}

int CSchrPois::multiruns(struct global_type *pGlobals, struct flag_type *pFlags)
{
	char message[MAX_MESSAGE], voltconv[MAX_VOLTSOURCES];
	char extension[MAX_EXTENSION], **pCVdata;
	//char multirunname[MAX_FILENAME];	/* pGlobals->filename for pFlags->multiruns*/
	double initcharge;	/* charge of the initial run*/
	//double etchposition;
	double nsTotal,psTotal,sheetres;
	//int cvcount;
	int multicount,multicountmax,error;
	
	PostInfoSP(_T("1D_Poisson: Multirun"));

	multicountmax  = (int) (fabs((pGlobals->vstop-pGlobals->vbias[pGlobals->surfvoltage])/pGlobals->vincrement) + 0.5e0) +1;
	pCVdata = (char **) malloc(multicountmax*sizeof(char *));
	for (multicount=0; multicount<multicountmax; multicount++) {
		pGlobals->vmulti = pGlobals->vbias[pGlobals->surfvoltage] + pGlobals->vincrement*(double) (multicount);
		pGlobals->pTopMesh->Efn = -pGlobals->vmulti;
		if (pFlags->usesurfbar)  
			pGlobals->pTopMesh->v = pGlobals->surfbar-pGlobals->vmulti-pGlobals->pTopMesh->offset;	/* surface voltage, including quasipotential */
		
		sprintf(voltconv,"%-10.2f", pGlobals->vmulti);
		if ( !pFlags->cv ) {
			error = solve(pGlobals->converg,pGlobals,pFlags);		/* solve Poissons and Schrodingers eq. */
			if (error) return 1;
			if (pFlags->findBst) error = boundstates(pGlobals,pFlags);	/* find all bound states if requested */
			if (error) return 1;
			error = writestatus(pGlobals->filename,extension,count,pGlobals,pFlags);	/* write status file used by cleanup */
			if (error) return 1;
			strcpy(extension,"_V");
			strcat(extension,voltconv);
			output(pGlobals->filename,extension,count,pGlobals,pFlags);		/* output results */
		}
		else {
			/* Capacitance vs voltage scan */
			error = solve(pGlobals->CVconverg,pGlobals,pFlags);		/* solve Poissons and Schrodingers eq. */
			if (error) return 1;
			if (pFlags->findBst) error = boundstates(pGlobals,pFlags);	/* find all bound states if requested */
			if (pFlags->firstcv && pFlags->cvonly) {
				strcpy(extension,"");
				output(pGlobals->filename, extension,count,pGlobals,pFlags);
			}
			else if (!pFlags->cvonly) {
				strcpy(extension,"_V");
				strcat(extension,voltconv);
				output(pGlobals->filename,extension,count,pGlobals,pFlags);		/* output results */
			}
			initcharge = cvcharge(pGlobals->pTopMesh);		/* find the charge for this voltage */
			printf("\n");
			pGlobals->pTopMesh->v = pGlobals->pTopMesh->v + pGlobals->cvvoltage;
			error = solve(pGlobals->CVconverg,pGlobals,pFlags);		/* Solve Poi/Sch for pFlags->cv voltage */

			pGlobals->capacitance = (cvcharge(pGlobals->pTopMesh)-initcharge)/pGlobals->cvvoltage;
			sheetcharge(&nsTotal, &psTotal, &sheetres, pGlobals);
			pCVdata[multicount] = (char *) malloc(sizeof(char[71]));		
			sprintf(pCVdata[multicount], "%10.3e\t%11.4e\t%11.4e\t%11.4e\t%11.4e\t%11.4e\n", 
				pGlobals->vmulti,pGlobals->capacitance*q,initcharge,cvcharge(pGlobals->pTopMesh),nsTotal,psTotal);
			if (error) return 1;
		}
		strcpy(message,"Calculation ");
		strcat(message,voltconv);
		strcat(message,"V Completed");
		PostInfoSP(message);
		//printf("%s",message);
		//printf("\n");
	}
	error = CVout(pCVdata,multicountmax,pGlobals);
	for (multicount=0; multicount<multicountmax; multicount++) free(pCVdata[multicount]);
	free (pCVdata);
	
	return 0;
}

/***********************************/
int CSchrPois::solve(double loc_converg, struct global_type *pGlobals, struct flag_type *pFlags) {

/***********************************/

	int outputen;	/*output enable */
	char number[10],message[MAX_MESSAGE];

	if (strncmp(pGlobals->surfcontact,"slope",5)){ 
		pGlobals->pStartMesh = pGlobals->pStartSim->pDownMesh; /* For schottky and ohmic BCs, don't calc the surface mesh*/
		PostInfoSP(_T("Finding mobile charge at the surface for slope contact"));
		mobilecharge(pGlobals->pStartSim,pGlobals->kT);	/* Find the mobile and fixed charge for the point that's not calculated*/
		PostInfoSP(_T("Finding fixed charge at the surface for slope contact"));
		fixedcharge(pGlobals->pStartSim,pGlobals->kT);
	}
	else
		pGlobals->pStartMesh = pGlobals->pStartSim; /* For slope BC, calc the surf mesh*/	

	if (strncmp(pGlobals->subcontact,"slope",5)){ 
		pGlobals->pStopMesh = pGlobals->pBottomMesh->pUpMesh; /* For schottky and ohmic BCs, don't calc the substrate mesh*/
		PostInfoSP(_T("Finding mobile charge at the substrate for slope contact"));
		mobilecharge(pGlobals->pBottomMesh,pGlobals->kT);	/* Find the mobile and fixed charge for the point that's not calculated*/
		PostInfoSP(_T("Finding fixed charge at the substrate for slope contact"));
		fixedcharge(pGlobals->pBottomMesh,pGlobals->kT);
	}
	else
	  pGlobals->pStopMesh = pGlobals->pBottomMesh; /* For slope BC, calc the substrate mesh*/

	if (!pFlags->schrodinger) {
		PostInfoSP(_T("Schrodinger enabled"));
		outputen = true;
	}
	else{
		PostInfoSP(_T("Schrodinger disabled"));
		outputen = false;
	}
		
	pFlags->schex = false;		/* reset pFlags->schrodinger execute flag */

	if (pFlags->restart && pFlags->schrodinger) {
		pFlags->schex = true;
		outputen = true;	/*On a pFlags->restart, jump right into a pFlags->schrodinger calculation */
	}
	
	/*  Now we are ready to start the calculation */
	for (count=1; count<pGlobals->maxcount; count++) {		/* do up to specified max iterations */

		/*  When the convergence error falls below the pGlobals->crossover value, the pFlags->schrodinger solver kicks in, if requested */
		if (pGlobals->ConvgError < pGlobals->crossover && pFlags->schrodinger && !pFlags->schex && count > 1) {
			sprintf(message,"Convergence below %10.3e value. Schrodinger solver turned on.", pGlobals->crossover);			
			PostInfoSP(message);
			pFlags->schex = true;
			outputen = true;
		}

		PostInfoSP(_T("Finding electron and hole concentration, and fixed charge"));
		charge(pGlobals,pFlags);		/* find electron and hole concentration, and fixed charge */
		PostInfoSP(_T("Adjusting band diagram"));
		adjust(pGlobals,pFlags);		/* adjust band diagram */

		sprintf(message,"%-4i: convergence error = %10.3e eV", count, pGlobals->ConvgError);
		PostInfoSP(message);

		/*  check for convergence, which occurs when the max correction to the conduction */
		/*  band falls below the requested convergence.  At least two iterations must be done. */
		if ((pGlobals->ConvgError < loc_converg) && outputen && (count > 2)) {
			sprintf(message,"%-4i: convergence error = %10.3e eV", count, pGlobals->ConvgError);
			PostInfoSP(message);
			return 0;
		}
	}

	/*  if not converged after max iterations, print error and stop. */
	strcpy(message,"I canna hold her much longer cap'n, she's breakin up!\n");
	alertboxWin(message);
	sprintf(number,"%4i",pGlobals->maxcount);
	strcpy(message,"Execution stopped after ");
	strcat(message,number);
	strcat(message," iterations.\n");
	alertboxWin (message);

	PostInfoSP(message);

	/*  report the nuumber of iterations and convergence error. */
	sprintf(message,"total steps = %-4i: convergence error = %10.3e eV", count, pGlobals->ConvgError);
	PostInfoSP(message);

	return 1;
}
	