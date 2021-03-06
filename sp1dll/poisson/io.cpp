#include "stdafx.h"

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

#include "poisp_types.h"
#include "poisp_defs.h"

#include "..\schrpois.h"
#include "..\SPgrph.h"


int CSchrPois::input(struct global_type *pGlobals, struct flag_type *pFlags) {
/*****************************************/

/*  This routine reads the input file */

	int dosefound,Rpfound,drpfound,imptypefound;
	int sourcenumber,fieldlength,infoLength,ioerr_loc,startsearch;
	char info[MAX_INPUTLINE],keyword[MAX_KEYWORD], message[MAX_MESSAGE],expression[MAX_EXPRESS];
	//char extension[MAX_EXTENSION];
	char access,*charPt;
	double tempconv; 

	PostInfoSP(_T("input: Setting defaults"));
	dosefound = false;				/* pGlobals->dose not found */
	Rpfound	= false;				/* range not found */
	drpfound = false;				/* pGlobals->dRp not found */
	imptypefound = false;			/* pFlags->implant type not found */

/*  Populate pGlobals with default values */
	setdefaults(pGlobals, pFlags);

/*	We first need to find dy for the structure (needed for graded layers). Also, any additional simulation definitions are read at this point */
	PostInfoSP(_T("input: Getting the additional simulation parameters (getdy)"));
	ioerr_loc = getdy(pGlobals);

/*  Now open the input file and read the contents for real */
	PostInfoSP(_T("input: Reading input file and populating the structure"));
	access='r';
	ioerr_loc = fileopen(4, pGlobals->filename, 'r', "", pGlobals, "");	  

	/* loop until all the data has been read */
	while(true) {
	 
		 /* read data line, which will then be analyzed */
		infoLength=getInput(4,info,pGlobals);

		 /* Look for end of file */
		if (infoLength == EOF)
			break;
		
	   /* extract the first field */
		startsearch = 0;
		fieldlength=fieldtrim(info,&startsearch,keyword,expression,caseInSens);


		if (fieldlength != 0) {
			if (!strncmp(keyword,"#",1))   
	      		continue; /*skip comments */

			/* Check to see if the first field contains 'surface', or 'substrate',
    		which signals the beginning of the layer specifications */
			else if (!strcmp(keyword,"surface")) {
				PostInfoSP(_T("input: Reading each layer specifications"));
	    		readlayers(info, pGlobals, pFlags);		/* read the layer specifications */
	    		if (pFlags->badversion || pFlags->stopthemusic) 
	      			return 1;
			}

			else if (keyword[0] == 'v') {
	    		keyword[0] = ' ';
	    		sourcenumber=atoi(keyword);
	    		if (strlen(expression) != 0) {	
	    			startsearch = 0;
					fieldlength = getSingField(keyword,info,&startsearch); /*ignore keyword */
					fieldlength = getSingField(keyword,info,&startsearch); 
	        		pGlobals->vbias[sourcenumber]=strtod(keyword,&charPt); /* get the source voltage */
	    		}
	    		else {
	        		strcpy(message,"A voltage source must have a voltage declared");
	        		alertboxWin(message);
					pFlags->stopthemusic = true;
					return 0;
	   			}
				fieldlength = getSingField(keyword,info,&startsearch);	/* get the stop voltage */
	    		if (fieldlength != '\0') { 
	        		pFlags->multiruns = true;
	        		pGlobals->vstop=strtod(keyword,&charPt);
					fieldlength = getSingField(keyword,info,&startsearch);	/* get the voltage increment */
	        		if (fieldlength != 0) 
		  				pGlobals->vincrement=strtod(keyword,&charPt);
					else {
		  				strcpy(message,"You must give a voltage increment for multiple voltage runs");
		  				alertboxWin(message);
		  				pFlags->stopthemusic = true;
		  				return 1;
		  			}
				}
	    	}
			else if (!strncmp(keyword,"cv",2)) {  	/* read CV parameter info */
	    		pFlags->cv = true;
	    		startsearch = 2;
		  		fieldlength=fieldtrim(info,&startsearch,keyword,expression,caseInSens);
	    		if (fieldlength != 0) {
	    			if(!strncmp(keyword,"only",4)) 
						pFlags->cvonly = true;
					else{
		  				if(!strncmp(keyword,"voltage",7)) {
		  					pGlobals->cvvoltage=strtod(expression,&charPt); /* get CV voltage */ 
     					}
     				}
     			}
     		}
	    	else if (!strncmp(keyword,"implant",7)) {  	/* read ion implant parameters */
	    		pGlobals->impnumber++;		/* increment the implant counter */
	      		fieldlength = fieldtrim(info,&startsearch,keyword,expression,caseInSens);	/* get the pFlags->implant type */
	      		pGlobals->imptype[pGlobals->impnumber-1] = keyword[0];
	    		if (pGlobals->imptype[pGlobals->impnumber-1] == 'a' || pGlobals->imptype[pGlobals->impnumber-1] == 'd') 
					imptypefound = true;

	    		while (fieldlength != 0){
	        		//fieldlength = fieldtrim(info,&startsearch,keyword,expression,caseInSens);	/* get the rest of the pFlags->implant info */
					if (!strncmp(keyword,"dose",4)) {
	          			pGlobals->dose[pGlobals->impnumber-1] = numconv(expression);
		  				dosefound = true;
		  			}
					else if (!strncmp(keyword,"rp",2)) {
	          			pGlobals->Rp[pGlobals->impnumber-1] = numconv(expression);
		  				Rpfound = true;
		  			}
					else if (!strncmp(keyword,"drp",3)) {
	        			pGlobals->dRp[pGlobals->impnumber-1] = numconv(expression);
		  				drpfound = true;
		  			}
	        		fieldlength = fieldtrim(info,&startsearch,keyword,expression,caseInSens);	/* get the rest of the pFlags->implant info */
				}

	    		if (dosefound && Rpfound && drpfound && imptypefound) {
	        		pFlags->implant = true;
					if (pGlobals->impnumber > 10) {
		  				strcpy(message,"Only 10 implants may be specified");
		  				alertboxWin(message);
		  				pFlags->stopthemusic = true;
		  				return 1;
		  			}
					pGlobals->dose[pGlobals->impnumber-1] = pGlobals->dose[pGlobals->impnumber-1]/(2.506628
     					*pGlobals->dRp[pGlobals->impnumber-1]*1.0e-8);	/* pGlobals->dose scale factors: pGlobals->dose/(sqrt(2pi)*pGlobals->dRp) */
	      		}
	    		if (!imptypefound) {
	        		strcpy(message,"An implant type must be given for Ion Implant");
	       			alertboxWin(message);
					pGlobals->impnumber = pGlobals->impnumber-1;
				}
	    		else if (!dosefound) {
	        		strcpy(message,"A Dose must be given for Ion Implant");
	       			alertboxWin(message);
					pGlobals->impnumber = pGlobals->impnumber-1;
				}
	    		else if (!Rpfound) {
	        		strcpy(message,"An Ion Range must be given for Ion Implant");
	       			alertboxWin(message);
					pGlobals->impnumber = pGlobals->impnumber-1;
				}
	    		else if (!drpfound) {
	        		strcpy(message,"A Range Std. Dev. must be given for Ion Implant");
	       			alertboxWin(message);
					pGlobals->impnumber = pGlobals->impnumber-1;
				}
	      
				/* now reset all pFlags->implant flags to be ready for another pFlags->implant spec. */
	    		dosefound = false;
	    		imptypefound = false;
	    		Rpfound = false;
	    		drpfound = false;
			}

			else if (!strncmp(keyword,"etch",4)) {  	/* An etch simulation is requested */
				pFlags->etched = true;
			
				fieldlength = getSingField(expression,info,&startsearch); /* get etch depth */
	        	pGlobals->etchdepth = numconv(expression);

				fieldlength = getSingField(expression,info,&startsearch); /* get etch increment */
	        	pGlobals->etchinc = numconv(expression);
			}

			else if (!strncmp(keyword,"etchonly",8)){   /* Only sheet resistance is requested */
	    		pFlags->etchonly = true;
			}
			else if (!strncmp(keyword,"temp",4)) {
	    		pGlobals->temperature = strtod(expression,&charPt);
				if (*charPt == 'k')   
	        		 /* pGlobals->temperature is given in kelven */
		  			tempconv = 0.0e0;	/* conversion factor */

				else if (*charPt == 'c')   
					/* pGlobals->temperature is given in celsius */
		  			tempconv = 273.15e0;

				else  
	           		 /* pGlobals->temperature is assumed to be kelvin (default) */
	          		tempconv = 0.0e0;
		
				pGlobals->temperature = pGlobals->temperature + tempconv;	/* pGlobals->temperature in K */
			}

	    	else if (!strncmp(keyword,"schrodingerstart",16)) {  
	         /* read in the starting point of the pFlags->schrodinger mesh */
				pFlags->schstartflag = true;
	    		pGlobals->realschstart = numconv(expression);
			}

			else if (!strncmp(keyword,"schrodingerstop",15)) {
				pFlags->schstopflag = true;
	    		pGlobals->realschstop = numconv(expression);
			}

			else if (!strncmp(keyword,"converg",7)) {  /* Change convergence criteria */
	        	pGlobals->converg = strtod(expression,&charPt);
			}

			else if (!strncmp(keyword,"cvconverg",9)) {  /* Change convergence criteria */
	        	pGlobals->CVconverg = strtod(expression,&charPt);
			}

			else if (!strncmp(keyword,"maxiterat",9)) {  /* Change convergence criteria */
	        	pGlobals->maxcount = (int)strtod(expression,&charPt);
			}

			else if (!strncmp(keyword,"crossover",9)) {  /* Change point where schrodinger solver kicks in */
	        		pGlobals->crossover = strtod(expression,&charPt);
			}

			else if (!strncmp(keyword,"exchange",8)){   /* Use pFlags->exchange potential */
	        	pFlags->exchange = true;
			}
			else if (!strncmp(keyword,"stop",4)) {  /* Stop  */
				fieldlength = getSingField(expression,info,&startsearch);
				fieldlength = getSingField(expression,info,&startsearch);
	        	if (!strncmp(expression,"will",4)) 
					pFlags->stopDanger = true;		/* Stop on Will Robinson error */
			}

			else if (!strncmp(keyword,"ignore",6))  
	        	pFlags->ignoreDanger = true;		/* Ignore  Will Robinson error */

			else if (!strncmp(keyword,"dy",2)) 	/* do nothing, dy is already found */
				;
			
			else if (!strncmp(keyword,"fullyionized",12))  /* already found */
				;

			else if (!strncmp(keyword,"restart",7))  /* already found */
				;

			else if (!strncmp(keyword,"as",2))  /* already found */
				;

			else if (!strncmp(keyword,"read",4))  /* already found */
				;

			else if (!strncmp(keyword,"no",2))  /* already found */
				;

			else if (!strncmp(keyword,"find",4))  /* already found */
				;

			else {
				strcpy(message,"Please correct input file.  ");
				strcat(message,"Sorry, I dont know what \"");
				strcat(message,keyword);
				strcat(message,"\" means.");
				alertboxWin(message);
				pFlags->stopthemusic = true;
				return 1;
			}
		}
	}

	fileclose(4, pGlobals);


	/* If no layers were found, stop the calculation */

	if (pGlobals->pTopLayer == NULL && !pFlags->badversion) {
		strcpy(message,"Please correct input file. There must be at least one material layer.");
		alertboxWin(message);
	}

	return 0;
}

int CSchrPois::getdy(struct global_type *pGlobals)
{
	int fieldlength,infoLength,ioerr_loc,startsearch;
	char info[MAX_INPUTLINE],keyword[MAX_KEYWORD],extension[MAX_EXTENSION],	expression[MAX_EXPRESS];
	char access;
	//char message[MAX_MESSAGE];
	//char *charPt;
	//double tempconv; 

	access='r';
	strcpy(extension,"");
	ioerr_loc = fileopen(4, pGlobals->filename, access, extension, pGlobals, "");

	strcpy(pGlobals->filenamePath, filePath);

	PostInfoSP(_T("input: Getting the value of step dy from input file (getdy)"));

	while(true) { 
		/* read data line, which will then be analyzed */
		infoLength=getInput(4,info,pGlobals);

		/* Check for end of file */
		if (infoLength == EOF)
			break;
		
		/* extract the first field */
		startsearch = 0;
		fieldlength=fieldtrim(info,&startsearch,keyword,expression,caseInSens);

		if (!strcmp(keyword,"no")) {  /* Restrictions requested */
			if (!strncmp(expression,"el",2))
	    		pFlags->defaultNoElec = true;	/* no electrons allowed */
			else if (!strncmp(expression,"ho",2)) 
	    		pFlags->defaultNoHoles = true;	/* no holes allowed */
	    	else if (!strncmp(expression,"st",2)) 
	    		pFlags->noStatus = true;		/* no status file will be output */
	    	else if (!strncmp(expression,"cl",2)) 
	    		pFlags->cleanFiles = false;	/* do not delete old output files */
		}

		if (!strcmp(keyword,"fullyionized"))
			pFlags->defFullyIonized = true;
	  
		if (!strncmp(keyword,"aster",5)) 
			pFlags->Asterix = true;	/* put * in first line of output files */

		if (!strncmp(keyword,"restart",7)) 
			pFlags->restart = true;

		if (!strncmp(keyword,"find",4)) 
			pFlags->findBst = true;	/* find all bound states automatically */

		if (!strncmp(keyword,"read",4)) { 
			if (!strncmp(expression,"param",5)) 
				pFlags->readParam = true;		/* no electrons allowed */
		}
	
		if (!strncmp(keyword,"dy",2)) {
			pGlobals->defaultdy = numconv(expression);
		}
	}
	fileclose(4, pGlobals);

	return ioerr_loc;
}



int CSchrPois::output(char name[MAX_FILENAME], char extension[MAX_EXTENSION], int count, struct global_type *pGlobals, struct flag_type *pFlags) {
 
/*  This routine creates the output files. */

	double y;		/* the real y coordinate */
	int ioerr_loc,j,i;
	char access;
	char extension2[MAX_EXTENSION],*outdata,outsubdata[20];
	char msg[MAX_MESSAGE];
	struct mesh_type *pMesh;
	
	pGlobals->numeval = pGlobals->numeleceval + pGlobals->numhheval + pGlobals->numlheval;
	outdata =  (char *) malloc((pGlobals->numeval*18+115)*sizeof(char));
	access='w';
	strcpy(extension2,extension);
	strcat(extension2,"_Out");
	ioerr_loc = fileopen(4, pGlobals->filename, access, extension2, pGlobals, "");
	if(ioerr_loc != 0) return 1;
  
/*  File_out contains the conduction and valence bands, hole and electron concentrations,  
	and the eigenvalues found */
	
	y=0.0e0;
	if (pFlags->Asterix) 
		fprintf(pGlobals->pFile[4],"*\n");
		
	if (pFlags->schrodinger) {
		sprintf(msg, "Writing Schrodinger calculation result to: %s%s.txt", pGlobals->filename, extension2);
		PostInfoSP(msg);
		strcpy(outdata,"Y (ang)\tEc (eV)\tEv (eV)\tE (V/cm)\tEf (eV)\tn (cm-3)\tp (cm-3)\tNd - Na (cm-3)");
		if (pGlobals->numeleceval > 0) {
			for (j=0;j<pGlobals->numeleceval;j++) {
				strcat(outdata,"\tel eval ");
				sprintf(outsubdata, "%d", j+1);
				strcat(outdata,outsubdata);
				strcat(outdata, " (eV)");
			}
		}
		if (pGlobals->numhheval > 0) {
			for (j=0;j<pGlobals->numhheval;j++) {
				strcat(outdata,"\thh eval ");
				sprintf(outsubdata, "%d", j+1);
				strcat(outdata,outsubdata);
				strcat(outdata, " (eV)");
			}
		}
		if (pGlobals->numeleceval > 0) {
			for (j=0;j<pGlobals->numlheval;j++) {
				strcat(outdata,"\tlh eval ");
				sprintf(outsubdata, "%d", j+1);
				strcat(outdata,outsubdata);
				strcat(outdata, " (eV)");
			}
		}
		strcat(outdata,"\n");
	}
	else 
		strcpy(outdata,"Y (ang)\tEc (eV)\tEv (eV)\tE (V/cm)\tEf (eV)\tn (cm-3)\tp (cm-3)\tNd - Na (cm-3)\n");
		
	fprintf(pGlobals->pFile[4],outdata);
	

	/*if (trim(pGlobals->surfcontact) .EQ. 'schottky') then
	do 100 i=-5,-1
	    if (pFlags->multiruns) then
	      write (4,7) i*dy(layer(1)),tab,-1.0*pGlobals->vmulti,tab,
     &			  -1.0*pGlobals->vmulti,tab,0.0e0,tab,
     &			  -1.0*pGlobals->vmulti,tab,1.0e-4,tab,1.0e-4,tab,1.0e-4
	    else
	      write (4,7) i*dy(layer(1)),tab,-1.0*pGlobals->vbias(pGlobals->surfvoltage),
     &		tab,-1.0*pGlobals->vbias(pGlobals->surfvoltage),tab,0.0e0,tab,
     &		-1.0*pGlobals->vbias(pGlobals->surfvoltage),tab,1.0e-4,tab,
     &		1.0e-4,tab,1.0e-4
	    end if
100	  continue
	end if*/
	
	/*if (pFlags->etched) then
	  do 101 i=0,startsim-1
	    y = y + dy(layer(i))
	    write (4,7) y,tab,-1.0*pGlobals->vbias(pGlobals->surfvoltage),tab,
     &		  -1.0*pGlobals->vbias(pGlobals->surfvoltage),tab,0.0e0,tab,
     &		  -1.0*pGlobals->vbias(pGlobals->surfvoltage),tab,1.0e-4,tab,1.0e-4,
     &		  tab,1.0e-4
101	  continue
     	end if*/
	/*v(size+1) = v(size) */
	
	pMesh = pGlobals->pStartSim;
	do { 		
		sprintf(outdata, "%13.6e\t%15.8e\t%15.8e\t%15.8e\t%15.8e\t%11.4e\t%11.4e\t%11.4e", 
			pMesh->meshcoord,pMesh->v+pMesh->offset, pMesh->v+pMesh->offset-pMesh->Eg,
			(pMesh->pDownMesh->v-pMesh->v)*1.e8/pMesh->pDownMesh->dy,pMesh->Efn,
			pMesh->n, pMesh->p, pMesh->fcharge);
			
		if ( pFlags->schrodinger && ((pMesh == pGlobals->pSchStart) || (pMesh == pGlobals->pSchStop))) {
			if (pGlobals->numeleceval > 0) {
				for (j=0;j<pGlobals->numeleceval;j++) {
					sprintf(outsubdata, "\t%11.4e", pGlobals->pEleceval[j]);
					strcat(outdata,outsubdata);
				}
			}
			if (pGlobals->numhheval > 0) {
				for (j=0;j<pGlobals->numhheval;j++) {
					sprintf(outsubdata, "\t%11.4e", pGlobals->pHheval[j]);
					strcat(outdata,outsubdata);
				}
			}
			if (pGlobals->numeleceval > 0) {
				for (j=0;j<pGlobals->numlheval;j++) {
					sprintf(outsubdata, "\t%11.4e", pGlobals->pLheval[j]);
					strcat(outdata,outsubdata);
				}
			}
		}
		strcat(outdata,"\n");
		fprintf(pGlobals->pFile[4],outdata);
		 
    } while ((pMesh = pMesh->pDownMesh) != pGlobals->pBottomMesh);
		
	sprintf(outdata, "%13.6e\t%15.8e\t%15.8e\t%15.8e\t%15.8e\t%11.4e\t%11.4e\t%11.4e", 
			pMesh->meshcoord,pMesh->v+pMesh->offset, pMesh->v+pMesh->offset-pMesh->Eg,
			(pMesh->v-pMesh->pUpMesh->v)*1.e8/pMesh->dy,pMesh->Efn,
			pMesh->n, pMesh->p, pMesh->fcharge);
			
	if ( pFlags->schrodinger && ((pMesh == pGlobals->pSchStart) || (pMesh == pGlobals->pSchStop))) {
		if (pGlobals->numeleceval > 0) {
			for (j=0;j<pGlobals->numeleceval;j++) {
				sprintf(outsubdata, "\t%11.4e", pGlobals->pEleceval[j]);
				strcat(outdata,outsubdata);
			}
		}
		if (pGlobals->numhheval > 0) {
			for (j=0;j<pGlobals->numhheval;j++) {
				sprintf(outsubdata, "\t%11.4e", pGlobals->pHheval[j]);
				strcat(outdata,outsubdata);
			}
		}
		if (pGlobals->numeleceval > 0) {
			for (j=0;j<pGlobals->numlheval;j++) {
				sprintf(outsubdata, "\t%11.4e", pGlobals->pLheval[j]);
				strcat(outdata,outsubdata);
			}
		}
	}
	strcat(outdata,"\n");
	fprintf(pGlobals->pFile[4],outdata);

     
     /*	if (trim(pGlobals->subcontact) .EQ. 'schottky') then
	  do 102 i=size+1,size+5
	    y = meshcoord(size) + (i-size)*dy(layer(size))
	    write (4,7) y,tab,-1.0*pGlobals->vbias(pGlobals->subvoltage),tab,
     &		-1.0*pGlobals->vbias(pGlobals->subvoltage),tab,0.0e0,tab,
     &		-1.0*pGlobals->vbias(pGlobals->subvoltage),tab,1.0e-4,tab,1.0e-4,tab,1.0e-4
102	  continue
	end if
7	format(e13.6,4(a1,e15.8),3(a1,e11.4),150(a1,f8.4))*/ 

	fileclose(4, pGlobals);
	  
/* If a pFlags->schrodinger calculation was performed, output the eigenvectors */
	
	if (pFlags->schrodinger) 
	{	
		access='w';
		strcpy(extension2,extension);
		strcat(extension2,"_Wave");
		ioerr_loc = fileopen(4, pGlobals->filename, access, extension2, pGlobals, "");
		if(ioerr_loc != 0) return 1;
  
/*  File .wave contains electron, heavy hole, and light hole eigenvectors */
	  
		sprintf(msg, "Writing eigenvectors to the file: %s%s.txt", pGlobals->filename, extension2);
		PostInfoSP(msg);
		if (pFlags->Asterix) 
			fprintf(pGlobals->pFile[4],"*\n");
		
		strcpy(outdata,"Y (ang)");
		if (pGlobals->numeleceval > 0) {
			for (j=0;j<pGlobals->numeleceval;j++) {
				strcat(outdata,"\tel Psi ");
				sprintf(outsubdata, "%d", j+1);
				strcat(outdata,outsubdata);
			}
		}
		if (pGlobals->numhheval > 0) {
			for (j=0;j<pGlobals->numhheval;j++) {
				strcat(outdata,"\thh Psi ");
				sprintf(outsubdata, "%d", j+1);
				strcat(outdata,outsubdata);
			}
		}
		if (pGlobals->numeleceval > 0) {
			for (j=0;j<pGlobals->numlheval;j++) {
				strcat(outdata,"\tlh Psi ");
				sprintf(outsubdata, "%d", j+1);
				strcat(outdata,outsubdata);
			}
		}
		strcat(outdata,"\n");
		fprintf(pGlobals->pFile[4],outdata);

		pMesh = pGlobals->pSchStart;
		i=0;
		do {
			sprintf(outdata, "%13.6e",pMesh->meshcoord);
			
			if (pGlobals->numeleceval > 0) {
				for (j=0;j<pGlobals->numeleceval;j++) {
					sprintf(outsubdata, "\t%11.4e", pGlobals->pElecevec[i][j]);
					strcat(outdata,outsubdata);
				}
			}
			if (pGlobals->numhheval > 0) {
				for (j=0;j<pGlobals->numhheval;j++) {
					sprintf(outsubdata, "\t%11.4e", pGlobals->pHhevec[i][j]);
					strcat(outdata,outsubdata);
				}
			}
			if (pGlobals->numeleceval > 0) {
				for (j=0;j<pGlobals->numlheval;j++) {
					sprintf(outsubdata, "\t%11.4e", pGlobals->pLhevec[i][j]);
					strcat(outdata,outsubdata);
				}
			}
			i++;
		
			strcat(outdata,"\n");
			fprintf(pGlobals->pFile[4],outdata);
		 
    	} while ((pMesh = pMesh->pDownMesh) != pGlobals->pSchStop->pDownMesh);

		fileclose(4, pGlobals);

/*
// Debug of wavefunc plotting
		CWnd* pWnd = CWnd::FindWindow(NULL, _T("Schrodinger-Poisson Plots"));
		if(pWnd != NULL)
		{
			CChartXYSerie* pSeries = NULL;
			CChartAxis* pBottomAxis = NULL;
			CChartAxis* pLeftAxis = NULL;
			CChartLineSerie* pLineSeries = NULL;
			int NumberPoints = 0;
			CChartCtrl* p_ChartCtrl;
			CGrphDlg* m_pGrphDlg;

			m_pGrphDlg = (CGrphDlg*)pWnd;
			p_ChartCtrl = &(m_pGrphDlg->m_ChartCtrl);

			i = 0;
			pMesh = pGlobals->pSchStart;
			do{	i++;}	while((pMesh = pMesh->pDownMesh) != pGlobals->pSchStop->pDownMesh);
			NumberPoints  = i;
			double* XValues = new double[NumberPoints];
			double* YValues = new double[NumberPoints];

			pMesh = pGlobals->pStartSim;
			if(pGlobals->numeleceval > 0)
			{
				//FILE* pFile;
				for (i=0;i<NumberPoints;i++) 
				{
					XValues[i] = pMesh->meshcoord;
					YValues[i] = pGlobals->pElecevec[i][0];
					pMesh = pMesh->pDownMesh;
					//sprintf(outsubdata, "%11.4e\t%11.4e", XValues[i], YValues[i]);
					//strcat(outsubdata,"\n");
					//pFile = fopen("Test2.txt","a+");
					//fprintf(pFile, outsubdata);
					//fclose(pFile);
				}
				pLineSeries = p_ChartCtrl->CreateLineSerie(false, false);
				pLineSeries->SetWidth(3);
				pSeries = pLineSeries;
				pSeries->SetPoints(XValues,YValues,NumberPoints);
				pBottomAxis = p_ChartCtrl->GetAxis(CChartCtrl::BottomAxis);
				pBottomAxis->SetAutomatic(true);
				pLeftAxis = p_ChartCtrl->GetAxis(CChartCtrl::LeftAxis);
				pLeftAxis->SetAutomatic(true);
				p_ChartCtrl->RefreshCtrl();
			}
		}
// End of Debug of wavefunc plotting
*/

	}
	//free(outdata);	// check to see if outdata pointer is released. Otherwise, it crashes!!!
	  
	/*close(4,status='delete')	/* delete any existing .wave file if a Sch cal. was not done */
		
	ioerr_loc = writestatus(name,extension,count,pGlobals,pFlags);
	if(ioerr_loc != 0) return 1;
	
	return 0;
}

int CSchrPois::writestatus(char name[MAX_FILENAME], char extension[MAX_EXTENSION], int count, struct global_type *pGlobals, struct flag_type *pFlags) {
  
/*  This routine creates the _status file. */

	double sheetres;	/* Sheet resistance of the structure */
	double nsTotal,psTotal;
	int ioerr_loc,i;
	char access,extension2[MAX_EXTENSION],outdata[400],outsubdata[20];
	char msg[MAX_MESSAGE];
	struct layer_type *pLayer;
		
	if (!pFlags->noStatus) {
		sheetcharge(&nsTotal,&psTotal,&sheetres,pGlobals);
	  
		access='w';
		strcpy(extension2,extension);
		strcat(extension2,"_Status");
		ioerr_loc = fileopen(4, name, access, extension2, pGlobals, "");
		if(ioerr_loc != 0) return 1;
	  
		sprintf(msg, "Writing Status file: %s%s.txt", name, extension2);
		PostInfoSP(msg);

/*  File _status  contains info on the execution and the final solution */
	
		sprintf(outdata,"Number of iterations to converge = %i\n",count);
		fprintf(pGlobals->pFile[4],outdata);

		sprintf(outdata,"Final correction to bands = %10.3e eV\n",pGlobals->ConvgError);
		fprintf(pGlobals->pFile[4],outdata);

		sprintf(outdata,"Maximum error in poisson equation= %10.3e\n",pGlobals->poierr);
		fprintf(pGlobals->pFile[4],outdata);
		
		if (pGlobals->poierr < 1.0e-3) 
			fprintf(pGlobals->pFile[4],"Don't worry, be happy! The convergence is good!\n");
		else
			fprintf(pGlobals->pFile[4],"Worry, be sad.  The convergence is not good.\n");
	
		if (pFlags->multiruns) {
			fprintf(pGlobals->pFile[4],"Multi-run  %10.3e, %10.3e, %10.3e\n",pGlobals->vbias[pGlobals->surfvoltage],pGlobals->vstop,pGlobals->vincrement);
		}
	
	  /*if (pFlags->etched) then
	    write (4,8) 'Etched ',pGlobals->etchdepth,pGlobals->etchinc
8	    format (a7,2(i10,1x))
	  end if */
	
		fprintf(pGlobals->pFile[4],"\n");
		fprintf(pGlobals->pFile[4],"Structure Sheet Resistance = %10.3e Ohms/square\n",sheetres);

		fprintf(pGlobals->pFile[4],"\n");
		fprintf(pGlobals->pFile[4],"Layer sheet concentrations\n");
		fprintf(pGlobals->pFile[4],"\n");
		
		if (!strncmp(pGlobals->surfcontact,"slope",5)) {
			strcpy(outdata,"surface            ");
			strcat(outdata,pGlobals->surfcontact);
			strcat(outdata,"=0\n");
			fprintf(pGlobals->pFile[4],outdata);
		}
		else {
			strcpy(outdata,"surface            ");
			strcat(outdata,pGlobals->surfcontact);
			strcat(outdata,"\n");
			fprintf(pGlobals->pFile[4],outdata);
		}
		
		pLayer = pGlobals->pTopLayer;
		do {
			strcpy(outdata,"-----------------------------------------------------------------------------------------\n");
			fprintf(pGlobals->pFile[4],outdata);
			sprintf(outdata, "%5.1f",pLayer->tLayer);
			strcat(outdata," Ang\t");
			strcat(outdata,pLayer->nameLayer);
			strcat(outdata,"\tns= ");
			sprintf(outsubdata, "%10.3e",pLayer->nsLayer);
			strcat(outdata,outsubdata);
			strcat(outdata," cm-2,\tps= ");
			sprintf(outsubdata, "%10.3e",pLayer->psLayer);
			strcat(outdata,outsubdata);
			strcat(outdata," cm-2\n");
			fprintf(pGlobals->pFile[4],outdata);
		} while((pLayer=pLayer->pDownLayer) != NULL);
		strcpy(outdata,"-----------------------------------------------------------------------------------------\n");
		fprintf(pGlobals->pFile[4],outdata);
		
		if (!strncmp(pGlobals->subcontact,"slope",5)) {
			strcpy(outdata,"substrate            ");
			strcat(outdata,pGlobals->subcontact);
			strcat(outdata,"=0\n");
			fprintf(pGlobals->pFile[4],outdata);
		}
		else {
			strcpy(outdata,"substrate            ");
			strcat(outdata,pGlobals->subcontact);
			strcat(outdata,"\n");
			fprintf(pGlobals->pFile[4],outdata);
		}
		
		fprintf(pGlobals->pFile[4],"\n");
		strcpy(outdata,"Temperature = ");
		sprintf(outsubdata, "%5.1f",pGlobals->temperature);
		strcat(outdata,outsubdata);
		strcat(outdata," K\n");
		fprintf(pGlobals->pFile[4],outdata);
		fprintf(pGlobals->pFile[4],"\n");
		
		if (pFlags->schrodinger) {
			strcpy(outdata,"Schrodinger solution from ");
			sprintf(outsubdata, "%10.3e",pGlobals->pSchStart->meshcoord);
			strcat(outdata,outsubdata);
			strcat(outdata," Ang to ");
			sprintf(outsubdata, "%10.3e",pGlobals->pSchStop->meshcoord);
			strcat(outdata,outsubdata);
			strcat(outdata," Ang.\n");
			fprintf(pGlobals->pFile[4],outdata);
			
			fprintf(pGlobals->pFile[4],"\n");
			fprintf(pGlobals->pFile[4],"The following subband energies were found (E-Ef):\n");
			fprintf(pGlobals->pFile[4],"\n");
			for (i=0;i<pGlobals->numeleceval;i++) 
				fprintf(pGlobals->pFile[4],"Electron eigenvalue %i = %13.6e eV\n",i+1,pGlobals->pEleceval[i]);
			for (i=0;i<pGlobals->numhheval;i++) 
				fprintf(pGlobals->pFile[4],"Heavy hole eigenvalue %i = %13.6e eV\n",i+1,pGlobals->pHheval[i]);
			for (i=0;i<pGlobals->numlheval;i++) 
				fprintf(pGlobals->pFile[4],"Light hole eigenvalue %i = %13.6e eV\n",i+1,pGlobals->pLheval[i]);
		}

		fileclose(4, pGlobals);
	}
	
	return 0;
}

int CSchrPois::CVout(char * pCVdata[], int multicountmax, struct global_type *pGlobals) {
/**********************************************************/
  
/*  This routine creates the _CV file. */
	int ioerr_loc, multicount;
	char outdata[88];
	char access;
	char extension2[MAX_EXTENSION];

	access='w';
	strcpy(extension2,"_CV");
	ioerr_loc = fileopen(4, pGlobals->filename, access, extension2, pGlobals, "");
	if(ioerr_loc != 0) return 1;
	
	strcpy(outdata,"V\tC (Fcm-2)\tInitcharge (cm-2)\tFinalcharge (cm-2)\tns Total (cm-2)\tps Total (cm-2)\n");
	fprintf(pGlobals->pFile[4],outdata);
	
	for (multicount=0; multicount<multicountmax; multicount++) fprintf(pGlobals->pFile[4],pCVdata[multicount]);
	fileclose (4, pGlobals);
	
	return 0;
}
int CSchrPois::getInput(int unit, char info[MAX_INPUTLINE], struct global_type *pGlobals) {
/***********************************************/

/* Fuction to get a line of data from a file*/

	int length,c,i,loc_error;
	
	for (i=0;i<MAX_INPUTLINE-1 && (c=getc(pGlobals->pFile[unit])) != EOF && c != '\n' && c!= '\r';i++)
		info[i] = c;
	info[i] = '\0';
	if (c == '\n' || c == '\r') {
		c=getc(pGlobals->pFile[unit]);
		if (c != '\n' && c != '\r')
			loc_error=ungetc(c,pGlobals->pFile[unit]);
	}
	else { 
		while ((c=getc(pGlobals->pFile[unit])) != EOF 
			&& c != '\n' && c!= '\r') {
			;
		}		
		if (c == '\n' || c == '\r') {
			c=getc(pGlobals->pFile[unit]);
			if (c != '\n' && c != '\r')
				loc_error=ungetc(c,pGlobals->pFile[unit]);
			
		}
	}
	info[i] = '\0';
	
	if (c == EOF && i == 0)
		length = EOF;
	else
		length = i;
		
	return length;
}	

void CSchrPois::fileclose(int unit, struct global_type *pGlobals) {
/****************************************/

/* Function to close a file */

	fclose(pGlobals->pFile[unit]);
}

int CSchrPois::fileopen(int unit, char name[MAX_FILENAME], char access, char extension[MAX_EXTENSION], struct global_type *pGlobals, char caption[MAX_FILENAME]) {
/*******************************************************************************************************************/

/* Function to open a file. No reading of its content takes place */

	int ioerr_loc = 0;
	char name2[MAX_FILENAME];
	char msg[120];
	OPENFILENAME ofn;
	char szFile[260];       // buffer for file name
	char szFileTitle[100];	// buffer for file name without extension
	HWND hwnd = NULL;       // owner window
	WORD cbBuf = 0;


	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFile = szFile;	// Set lpstrFile[0] to '\0' so that GetOpenFileName does not use the contents of szFile to initialize itself.
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = "All\0*.*\0Text\0*.TXT\0\0";
	ofn.nFilterIndex = 2;
	ofn.lpstrFileTitle = szFileTitle;
	ofn.lpstrFileTitle[0] = '\0';
	ofn.nMaxFileTitle = sizeof(szFileTitle);
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	ofn.lpstrDefExt = "TXT";
	ofn.lpstrTitle = caption;

	if ((name[0] == '*')||(!strcmp(name,""))) {
		// Display the Open dialog box.
		if (GetOpenFileName(&ofn)==TRUE)
		{
			cbBuf=GetFileTitle(ofn.lpstrFile, NULL, 0);
			ofn.lpstrFileTitle[cbBuf-5] = '\0'; // remove extension with the dot (isolate the name only)
			strcpy(name, ofn.lpstrFileTitle);	// filename only; used by the SP routine
			strncpy(filePath, ofn.lpstrFile, strlen(ofn.lpstrFile)-4);	// store a copy of the actual path of the file without the extension
			filePath[strlen(ofn.lpstrFile)-4] = '\0';
			pGlobals->pFile[unit] = fopen(ofn.lpstrFile, &access);
			if (pGlobals->pFile[unit] == NULL) {
				ioerr_loc = errno;
				sprintf(msg,"File error: %i",ferror);
				MessageBox(NULL,msg,"Error", MB_OK);
			}
			else
				ioerr_loc = 0;
		}
		else
			ioerr_loc = 29;
	}	
	else { 
		strcpy(name2,name);
		strcat(name2,extension);
		strcat(name2,".txt");
		strcpy(filePath, name);	// store a copy of the actual path of the file
		pGlobals->pFile[unit] = fopen(name2, &access);
		if (pGlobals->pFile[unit] == NULL) {
			ioerr_loc = errno;
			sprintf(msg,"File open error: %i",ferror);
			MessageBox(NULL,msg,"Error", MB_OK);
		}
		else
			ioerr_loc = 0;
	}

	return ioerr_loc;
}


int CSchrPois::setdefaults(struct global_type *pGlobals, struct flag_type *pFlags)
{
	int ioerr_loc = 0;

/* Set defaults */
	strcpy(pGlobals->surfcontact,"schottky"); /* default surface contact is a schottky barrier */
	strcpy(pGlobals->subcontact,"slope");	/* default substrate boundary condition is slope=0 */
	pGlobals->surfvoltage = 1;		/* surface schottky is connected to voltage source 1 */
	pGlobals->subvoltage = 1;		/* default substrate connected to voltage source 1 */
	pGlobals->vbias[1] = 0.0e0;		/* default voltage of source 1 is 0 */
	pGlobals->cvvoltage = 10e-3;	/* default pFlags->cv voltage is 10mV */
	pGlobals->defaultdy = 10.0e0;	/* Set default dy to 10Ang */
	pGlobals->temperature = 300;	/* Set default pGlobals->temperature to 300K */
	pGlobals->pSchStart = NULL;		/* To be on the safe side, set schroding start and stop pointers to Null */
	pGlobals->pSchStop = NULL;
	pGlobals->etchdepth = 0.0e0;
	pGlobals->etchinc = 0.0e0;
	pGlobals->impnumber = 0;		/* initialize implant counter */
	pGlobals->maxcount = 100;		/* default max interations = 100 */
	pGlobals->converg = 1.0e-5;		/* default convergence criteria */
	pGlobals->CVconverg = 1.0e-10;	/* default convergence criteria for CV sims */
	pGlobals->crossover = 1.0e-3;	/* default pGlobals->crossover is 1 meV */

	pFlags->schstartflag = false;	/* used to be sure that both schstart and schstop are set */
	pFlags->schstopflag = false;
	pFlags->defFullyIonized = false;/* Set default to calculate dopant ionization */
	pFlags->schrodinger = false;	/* Default is no pFlags->schrodinger solution */
	pFlags->badversion = false;		/* materials file version flag */
	pFlags->stopthemusic = false;	/* halt flag */
	pFlags->multiruns = false;		/* default to only one voltage run */
	pFlags->usesurfbar = false;		/* use default surface barrier unless specified */
	pFlags->usesubbar = false;		/* use default substrate barrier unless specified */
	pFlags->cv = false;				/* default is no CV */
	pFlags->cvonly = false;			/* default is not only pFlags->cv output */
	pFlags->implant = false;		/* default is no pFlags->implant */
	pFlags->etched = false;			/* default is no etch simulation */
	pFlags->defaultNoHoles = false;	/* default is to calculate the holes */
	pFlags->defaultNoElec = false;	/* default is to calculate the electrons */
	pFlags->defFullyIonized = false;/* default is calculate dopant ionization */
	pFlags->DangerWill = false;
	pFlags->ignoreDanger = false;	/* default is to act on "Danger Will Rob." error */
	pFlags->printDanger = true;		/* print the danger message on the first time */
	pFlags->stopDanger = false;		/* default is to not stop on "Danger Will Rob." error */
	pFlags->Asterix = false;		/* default is no * in first line of files */
	pFlags->etchonly = false;		/* default is output for all etch depths */
	pFlags->checkfamily = true;
	pFlags->firstlayer = true;
	pFlags->exchange = false;		/* pFlags->exchange potential flag */
	pFlags->cleanFiles = true;		/* default is to delete old files */
	pFlags->restart = false;		/* default is to start new simulations from scratch */
	pFlags->readParam = false;		/* default is to not read doping table file */
	pFlags->findBst = false;		/* default is to not find all bound states automatically */
	pFlags->noStatus = false;		/* default is to generate the status output file */

	return ioerr_loc;
}