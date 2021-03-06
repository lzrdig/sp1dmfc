#include "stdafx.h"
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "poisp_types.h"
#include "poisp_defs.h"

#include "..\schrpois.h"


void CSchrPois::choose(struct global_type *pGlobals, struct flag_type *pFlags) {
/*************************************************/

/* Chose an input file without running the simulation */

	char extension[MAX_EXTENSION],access,message[MAX_MESSAGE];
	int error,ioerr_loc;
	struct layer_type *pLayer;
	struct mesh_type *pMesh;
	
/* If another simulation has alaready been run, free the memory allocated for the 
   layer and mesh arrays */
	
	if (!pFlags->noinputfile) {
		pLayer = pGlobals->pTopLayer;
		while (pLayer->pDownLayer != NULL) {
			pLayer = pLayer->pDownLayer;
			free(pLayer->pUpLayer);
		}
		free(pLayer);
		pMesh = pGlobals->pTopMesh;
		if (pMesh->pUpMesh !=NULL) free(pMesh->pUpMesh);
		while (pMesh->pDownMesh != NULL) {
			pMesh = pMesh->pDownMesh;
			free(pMesh->pUpMesh);
		}
		free(pMesh);
		pFlags->noinputfile = true;
	}

	strcpy(pGlobals->filename,"*");
	access='r';
	strcpy(extension,"");

	ioerr_loc = fileopen(4, pGlobals->filename, access, extension, pGlobals, "Choose an input file");

	if (ioerr_loc) {
		if (ioerr_loc == 26) {
			strcpy(message,"Sorry, That file is already open elsewhere.");
			alertboxWin(message);
		}
		else if (ioerr_loc == 29) 
		    /* user canceled open */
			;
		else{
		    strcpy(message,"Some sort of un-natural file error has occured.");
			alertboxWin(message);
		}
		return;
	}
	
	fileclose(4, pGlobals);

	error = input(pGlobals,pFlags);

	/* If the input read failed, stop.  Otherwise initialize the arrays. */
	if (error) 
		return;
	else {
		error = initialize(pGlobals,pFlags); 
		return;
	}
}


void CSchrPois::selrun(struct global_type *pGlobals, struct flag_type *pFlags) {
/************************************************/

	/* Select input file then run simulation */
		
	char field[20];
	int error;
	
	/* If another simulation has alaready been run, free the memory 
	allocated for the layer and mesh arrays */
	PostInfoSP(_T("selrun: Removing previous simulation data"));
	freeMemPrevSimul(pGlobals, pFlags);
	
	/*  Check to see if you have the right input file.
    If an error is found, stop the program. */	
    error = checkRightInputFile(pGlobals);
    if(error) return;
	
	/* Not used for now. The idea is to be able to run scripts. A line is missing that would
	   initialize the "field" variable */
	   strcpy(field, "");
	if (!strncmp(field,"#script",7)) {
		PostInfoSP(_T("Running a script"));
		//script(pGlobals,pFlags);
		return;
	}

	/*  To set up the simulation, call the subroutines input and initialize. */	
	PostInfoSP(_T("selrun: Getting the input (input)"));
	error = input(pGlobals,pFlags);

	/* If the input read failed, stop.  Otherwise initialize the arrays and run the simulation. */
	if (error) {	
		PostInfoSP(_T("selrun: Error getting the input (selrun)"));
		return;
	}
	else {
		PostInfoSP(_T("selrun: Initializing the simulation data (initialize)"));
		error = initialize(pGlobals,pFlags);
		if(!error)
		{
			PostInfoSP(_T("selrun: Starting the simulation (onedpoisson)"));
			error = onedpoisson(pGlobals,pFlags);
		}
		return;
	}
}


void CSchrPois::script(struct global_type *pGlobals, struct flag_type *pFlags) {
/*****************************************/

/* Run a script file, a file containing the names of input files to be run. */

	char info[MAX_INPUTLINE],shellfilename[MAX_EXTENSION],extension[MAX_EXTENSION],access;
	int infoLength,fieldlength,startsearch,error,ioerr_loc,sign_possible;

	
	strcpy(shellfilename,pGlobals->filename);
	access='r';
	strcpy(extension,"");
	sign_possible = true;

	ioerr_loc = fileopen(8, pGlobals->filename, access, extension, pGlobals, "Choose a script file");

	infoLength=getInput(8,info,pGlobals);	/*skip #script line */
	
	while (true) {
		infoLength=getInput(8,info,pGlobals);
			/* Check for end of file */
		if (infoLength == EOF)
			break;
	
		fieldlength = getop(pGlobals->filename,info,&startsearch,&sign_possible);
		printf("Now calculating=,%s,\n",pGlobals->filename);
		if (pGlobals->filename[0] == '#') 
			continue;

		error = input(pGlobals,pFlags);
		if (pFlags->stopthemusic) {
			printf("Error in input file\n");
			continue;
		}

		error = initialize(pGlobals,pFlags);
		if (error) {
			printf("Error in initializing simulation\n");
			continue;
		}
	onedpoisson(pGlobals,pFlags);	/*do the calculations */
	}
	fileclose(8, pGlobals);

}

void CSchrPois::freeMemPrevSimul(struct global_type *pGlobals, struct flag_type *pFlags)
{
	int i;
	struct layer_type *pLayer = NULL;


/* If another simulation has alaready been run, free the memory allocated for the 
   layer and mesh arrays */
	
	if (!pFlags->noinputfile) {
		pLayer = pGlobals->pTopLayer;
		while (pLayer->pDownLayer != NULL) {
			pLayer = pLayer->pDownLayer;
			free(pLayer->pUpLayer);
		}
		free(pLayer);
		pMesh = pGlobals->pTopMesh;
		if (pMesh->pUpMesh !=NULL) free(pMesh->pUpMesh);
		while (pMesh->pDownMesh != NULL) {
			pMesh = pMesh->pDownMesh;
			free(pMesh->pUpMesh);
		}
		free(pMesh);
		pFlags->noinputfile = true;
	}	
	if (pFlags->schMemAlloc) {
		free(pGlobals->pPotential);
		free(pGlobals->pMass);
		free(pGlobals->pSdy);
		free(pGlobals->pEval);
		free(pGlobals->pEvcharge);
		free(pGlobals->pEvderiv);
		pFlags->schMemAlloc = false;
	}	
	if (pFlags->schElecMemAlloc) {
		for (i=0; i<pGlobals->order; i++)
			free(pGlobals->pElecevec[i]);
		free(pGlobals->pElecevec);
		free(pGlobals->pEleceval);
		pFlags->schElecMemAlloc = false;
	}
	if (pFlags->schHhMemAlloc) {
		for (i=0; i<pGlobals->order; i++)
			free(pGlobals->pHhevec[i]);
		free(pGlobals->pHhevec);
		free(pGlobals->pHheval);
		pFlags->schHhMemAlloc = false;
	}
	if (pFlags->schLhMemAlloc) {
		for (i=0; i<pGlobals->order; i++)
			free(pGlobals->pLhevec[i]);
		free(pGlobals->pLhevec);
		free(pGlobals->pLheval);
		pFlags->schLhMemAlloc = false;
	}
}

int CSchrPois::checkRightInputFile(struct global_type *pGlobals)
{
	char message[MAX_MESSAGE];
	int ioerr_loc;

	PostInfoSP(_T("selrun: Loading input file"));	
	ioerr_loc = fileopen(4, pGlobals->filename, 'r', "", pGlobals, "selrun: Choose an input file");
	strcpy(inputFilePath, filePath);	// this is needed!!! It stores "input file" path inside the CSchrPois class
	if (ioerr_loc) {
		if (ioerr_loc == 26) {
			strcpy(message,"selrun: Sorry, That file is already open elsewhere.");
			alertboxWin(message);
		}
		else if (ioerr_loc == 29) 
		    /* user canceled open */
			;
		else{
		    strcpy(message,"selrun: Some sort of un-natural file error has occured.");
			alertboxWin(message);
		}
		PostInfoSP(_T("selrun: Input file failed to load or start"));

		return 1;
	}
	fileclose(4, pGlobals);
	PostInfoSP(_T("selrun: Input file loaded"));	

	return 0;
}
