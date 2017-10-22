#include "StdAfx.h"
#include ".\matrls.h"
#include "Poisson\poisp_types.h"

CMatrls::CMatrls(void)
{
	m_pMtrlListTop = NULL;
}

CMatrls::~CMatrls(void)
{
}

int CMatrls::LoadMtrls(char matrlfile[MAX_FILENAME])
{
/*This routine loads the materials file into a linked list, the first element of which */
/* is pointed to by m_pMtrlListTop */

	//char seminame[MAX_SEMINAME];
	char field[MAX_EXPRESS],access,extension[MAX_EXTENSION],message[MAX_MESSAGE];
	char materialLine[MAX_INPUTLINE];
	int semistart,infoLength,firstMaterial,ioerr_loc,fieldlength,matrlCount;
	struct mtrl_list *pMtrlList;

	firstMaterial = true;
	m_pMtrlListTop = (struct mtrl_list*)malloc(sizeof(struct mtrl_list));
	pMtrlList = m_pMtrlListTop;

/*	 open materials file	*/
	access='r';
	strcpy(extension,"");
	ioerr_loc = OpenMtrlFile(3, matrlfile, access, extension, "Load Materials file");
	if(ioerr_loc)
		return 1;

/* read data line, which will then be analyzed */
	infoLength=GetLineFromFile(3,materialLine);

	semistart = 0;
	fieldlength=fieldtrim(materialLine,&semistart,field,extension,caseInSens);
	if (strcmp(field,materialsversion)) {
	strcpy(message,"Wrong Version of the Materials File");
		alertboxWin(message);
		fclose(m_pFile[3]);
		return 1;
	}

/*  Now that we know the material file exists, and is the correct version, load the materials  
  	by loading the materials info into the materials structure.*/
	matrlCount = 0;
	while (infoLength != EOF) {
		infoLength=GetLineFromFile(3,materialLine);
  
  		if(materialLine[0] == '#')
  			continue;
  		
	/*  first field contains the semiconductor name*/
		semistart = 0;
		fieldlength = getSingField(field,materialLine,&semistart);
		/*lowercase(field);*/
		
		if (fieldlength != '\0') {			
			if (firstMaterial) {
				//if(pMtrlList->pMaterial == NULL)
				//{
				pMtrlList->pMaterial = (struct mtrl_type *)malloc(sizeof(struct mtrl_type)); /* pointer for first material layer */
				pMtrlList->pNext_mtrl = NULL;
				//}
				firstMaterial = false;
			}
			else {
				pMtrlList->pNext_mtrl = (struct mtrl_list *)malloc(sizeof(struct mtrl_list));
				pMtrlList->pNext_mtrl->pNext_mtrl = NULL;
				pMtrlList->pNext_mtrl->pMaterial = NULL;
				pMtrlList = pMtrlList->pNext_mtrl;
				pMtrlList->pMaterial = (struct mtrl_type *)malloc(sizeof(struct mtrl_type)); /* pointer for next material layer */
			}
			
			matrlCount++;
			strcpy(pMtrlList->name,field); /* Save semiconductor name */
			sprintf(message,"material #%i - %s",matrlCount,pMtrlList->name);
			fieldlength = getSingField(field,materialLine,&semistart);
			/*lowercase(field);*/
			strcpy(pMtrlList->type,field); /* save semiconductor type */
			fieldlength = getSingField(field,materialLine,&semistart);
			lowercase(field);
			strcpy(pMtrlList->family,field);	/* save family name */
			fieldlength = getSingField(field,materialLine,&semistart);
			if (fieldlength != '\0') {
				strcpy(pMtrlList->comment,field);	/* save material comment */
				while (fieldlength != '\0') {
					fieldlength = getSingField(field,materialLine,&semistart);
					strcat(pMtrlList->comment," ");	/* Add a space */
					strcat(pMtrlList->comment,field);	/* save the rest of material comment */
				}
			}
			else
				strcpy(pMtrlList->comment,"");
	
			infoLength=GetLineFromFile(3,materialLine); /* get next line (default dopings) */
			lowercase(materialLine);
			semistart = 0;
			fieldlength = getSingField(field,materialLine,&semistart);			
			pMtrlList->pMaterial->Nd=numconv(field);
			fieldlength = getSingField(field,materialLine,&semistart);			
			pMtrlList->pMaterial->Na=numconv(field);
			fieldlength = getSingField(field,materialLine,&semistart);			
			pMtrlList->pMaterial->Ndd=numconv(field);
			fieldlength = getSingField(field,materialLine,&semistart);			
			pMtrlList->pMaterial->Nda=numconv(field);
				
			infoLength=GetLineFromFile(3,materialLine); /* get next line (Eg info) */
			lowercase(materialLine);
			pMtrlList->pMaterial->Egline = ParameterInfo(materialLine);
			infoLength=GetLineFromFile(3,materialLine); /* get next line (Delta EC info) */
			lowercase(materialLine);
			pMtrlList->pMaterial->offsetline = ParameterInfo(materialLine);
			infoLength=GetLineFromFile(3,materialLine); /* get next line (Rel diel const info) */
			lowercase(materialLine);
			pMtrlList->pMaterial->erline = ParameterInfo(materialLine);
			infoLength=GetLineFromFile(3,materialLine); /* get next line (Ed info) */
			lowercase(materialLine);
			pMtrlList->pMaterial->Edline = ParameterInfo(materialLine);
			infoLength=GetLineFromFile(3,materialLine); /* get next line (Ea info) */
			lowercase(materialLine);
			pMtrlList->pMaterial->Ealine = ParameterInfo(materialLine);
			infoLength=GetLineFromFile(3,materialLine); /* get next line (Edd info) */
			lowercase(materialLine);
			pMtrlList->pMaterial->Eddline = ParameterInfo(materialLine);
			infoLength=GetLineFromFile(3,materialLine); /* get next line (Eda info) */
			lowercase(materialLine);
			pMtrlList->pMaterial->Edaline = ParameterInfo(materialLine);
			infoLength=GetLineFromFile(3,materialLine); /* get next line (elec eff mass info) */
			lowercase(materialLine);
			pMtrlList->pMaterial->eemline = ParameterInfo(materialLine);
			infoLength=GetLineFromFile(3,materialLine); /* get next line (Cond band degen info) */
			lowercase(materialLine);
			pMtrlList->pMaterial->valleyline = ParameterInfo(materialLine);
			infoLength=GetLineFromFile(3,materialLine); /* get next line (heavy hole eff mass info) */
			lowercase(materialLine);
			pMtrlList->pMaterial->mhhline = ParameterInfo(materialLine);
			infoLength=GetLineFromFile(3,materialLine); /* get next line (Light hole eff mass info) */
			lowercase(materialLine);
			pMtrlList->pMaterial->mlhline = ParameterInfo(materialLine);
			infoLength=GetLineFromFile(3,materialLine); /* get next line (elec mobil info) */
			lowercase(materialLine);
			pMtrlList->pMaterial->emobline = ParameterInfo(materialLine);
			infoLength=GetLineFromFile(3,materialLine); /* get next line (hole mobil info) */
			lowercase(materialLine);
			pMtrlList->pMaterial->hmobline = ParameterInfo(materialLine);
			infoLength=GetLineFromFile(3,materialLine); /* get next line (elec recomb time info) */
			lowercase(materialLine);
			pMtrlList->pMaterial->taunline = ParameterInfo(materialLine);
			infoLength=GetLineFromFile(3,materialLine); /* get next line (hole recomb time info) */
			lowercase(materialLine);
			pMtrlList->pMaterial->taupline = ParameterInfo(materialLine);
			infoLength=GetLineFromFile(3,materialLine); /* get next line (polariz info) */
			lowercase(materialLine);
			pMtrlList->pMaterial->polarizline = ParameterInfo(materialLine);
			infoLength=GetLineFromFile(3,materialLine); /* get next line (absorption info) */
			lowercase(materialLine);
			pMtrlList->pMaterial->alphaline = ParameterInfo(materialLine);
				
			
			infoLength=GetLineFromFile(3,materialLine);  /* skip the "end" line */
		} 
	}

	return 0;
}

int CMatrls::GetLineFromFile(int unit, char info[MAX_INPUTLINE]) {
/* Fuction to get a line of data from file*/

	int length,c,i,loc_error;
	
	for (i=0;i<MAX_INPUTLINE-1 && (c=getc(m_pFile[unit])) != EOF && c != '\n' && c!= '\r';i++)
		info[i] = c;
	info[i] = '\0';
	if (c == '\n' || c == '\r') {
		c=getc(m_pFile[unit]);
		if (c != '\n' && c != '\r')
			loc_error=ungetc(c,m_pFile[unit]);
	}
	else { 
		while ((c=getc(m_pFile[unit])) != EOF 
			&& c != '\n' && c!= '\r') {
			;
		}		
		if (c == '\n' || c == '\r') {
			c=getc(m_pFile[unit]);
			if (c != '\n' && c != '\r')
				loc_error=ungetc(c, m_pFile[unit]);
			
		}
	}
	info[i] = '\0';
	
	if (c == EOF && i == 0)
		length = EOF;
	else
		length = i;
		
	return length;
}	

int CMatrls::OpenMtrlFile(int unit, char fname[MAX_FILENAME], char access, char extension[MAX_EXTENSION], char caption[MAX_FILENAME]) {
/* Function to open a matrl file */

	int ioerr_loc = 0;
	char name2[MAX_FILENAME];
	char filePath[MAX_FILENAME];
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

	if ((fname[0] == '*')||(!strcmp(fname,""))) {
		// Display the Open dialog box.
		if (GetOpenFileName(&ofn)==TRUE)
		{
			cbBuf=GetFileTitle(ofn.lpstrFile, NULL, 0);
			ofn.lpstrFileTitle[cbBuf-5] = '\0'; // remove extension with the dot (isolate the name only)
			int sn = sizeof(fname);
			int flen = strlen(ofn.lpstrFile);
			fname = (char*)malloc(flen*sizeof(char));
			if(fname == NULL)
			{
				MessageBox(NULL, "Error in realloc (OpenMatrlFile)", "Error", MB_OK);
				return 1;
			}
			strcpy(fname, ofn.lpstrFileTitle);	// filename only; used by the SP routine
			strncpy(filePath, ofn.lpstrFile, strlen(ofn.lpstrFile)-4);	// store a copy of the actual path of the file without the extension
			filePath[strlen(ofn.lpstrFile)-4] = '\0';
			m_pFile[unit] = fopen(ofn.lpstrFile, &access);
			if (m_pFile[unit] == NULL) {
				ioerr_loc = errno;
				sprintf(msg,"File error: %i",ferror);
				MessageBox(NULL,msg,"Error", MB_OK);
			}
			else
				ioerr_loc = 0;

			strcpy(m_sMtrlFileName, ofn.lpstrFile);
		}
		else
			ioerr_loc = 29;
	}	
	else { 
		strcpy(name2,fname);
		strcat(name2,extension);
		strcat(name2,".txt");
		strcpy(filePath, fname);	// store a copy of the actual path of the file
		m_pFile[unit] = fopen(name2, &access);
		if (m_pFile[unit] == NULL) {
			ioerr_loc = errno;
			sprintf(msg,"File open error: %i",ferror);
			MessageBox(NULL,msg,"Error", MB_OK);
		}
		else
			ioerr_loc = 0;
	}

	return ioerr_loc;
}
double CMatrls::MatEval(struct mtrl_line_type * pCurrentInterval, double xval, double yval, double zval, 
				double wval, int *loc_badversion, double tmprt) {
	int rightrange;
	double functionvalue;
	char message[MAX_MESSAGE];
	struct global_type *p_dummyGlobals;

	p_dummyGlobals = new global_type;

	rightrange = false;        
  
/*  Get the parameter info that is stored in the materials structure */

/*  If the x value is in the specification range for this parameter, store the information*/
	while(pCurrentInterval != NULL) {
		if (xval <= pCurrentInterval->xmax  && xval >= pCurrentInterval->xmin &&
				yval <= pCurrentInterval->ymax  && yval >= pCurrentInterval->ymin &&
				zval <= pCurrentInterval->zmax  && zval >= pCurrentInterval->zmin &&
				wval <= pCurrentInterval->wmax  && wval >= pCurrentInterval->wmin ) {

			p_dummyGlobals->xval = xval;
			p_dummyGlobals->yval = yval;
			p_dummyGlobals->zval = zval;
			p_dummyGlobals->wval = wval;
			p_dummyGlobals->temperature = tmprt;
			p_dummyGlobals->y = 0;
			functionvalue = evaluate(pCurrentInterval->expression, p_dummyGlobals);	/*calculate the parameter*/
			rightrange = true;
			return functionvalue;
		}
		else
			pCurrentInterval = pCurrentInterval->pNextInterval;
	}
	
/*	if (xval == 2.0e0) {
		strcpy(message,"A ternary must have an x value declared");
		alertbox(message);
		*loc_badversion = true;
		return 0.0;
	}
	else {
	    strcpy(message,"Improper x value in a ternary definition");
		alertbox(message);
		*loc_badversion = true;
		return 0.0;
	}*/
	    
	strcpy(message,"Improper mole-fraction definition");
	alertboxWin(message);
	*loc_badversion = true;
	return 0.0;
}

struct mtrl_line_type * CMatrls::ParameterInfo(char materialLine[]) {

/**************************************************/

	int infostart,fieldlength,type,sign_possible;
	char field[MAX_EXPRESS];
	struct mtrl_line_type *pFirstInterval, *pCurrentInterval;
	
	pFirstInterval = (struct mtrl_line_type *) 
					malloc(sizeof(struct mtrl_line_type)); /* pointer for first interval of information */
	pCurrentInterval = pFirstInterval;
	pCurrentInterval->xmin = 998.0;
	pCurrentInterval->xmax = 1000.0;
	pCurrentInterval->ymin = 998.0;
	pCurrentInterval->ymax = 1000.0;
	pCurrentInterval->zmin = 998.0;
	pCurrentInterval->zmax = 1000.0;
	pCurrentInterval->wmin = 998.0;
	pCurrentInterval->wmax = 1000.0;
	pCurrentInterval->pNextInterval = NULL;
	infostart = 0;
	fieldlength = getSingField(field,materialLine,&infostart); /* ignore keyword */ 
	fieldlength = getSingField(field,materialLine,&infostart); /* get expression for this interval */
	infix_postfix(field); /* convert the expression into postfix format */
	val_append(field);  /* append val to x, y, z, or w */
	strcpy(pCurrentInterval->expression,field);
	
	fieldlength = getSingField(field,materialLine,&infostart); /* look for possible next field */
	sign_possible = true;
	
	while (fieldlength != '\0') {
		switch (field[0]) {
			case 'x':
				type = getop(field,materialLine,&infostart,&sign_possible);
				pCurrentInterval->xmin=numconv(field);
				type = getop(field,materialLine,&infostart,&sign_possible); /*ignore - sign */
				type = getop(field,materialLine,&infostart,&sign_possible);
				pCurrentInterval->xmax=numconv(field);
				break;
			case 'y':
				type = getop(field,materialLine,&infostart,&sign_possible);
				pCurrentInterval->ymin=numconv(field);
				type = getop(field,materialLine,&infostart,&sign_possible); /*ignore - sign */
				type = getop(field,materialLine,&infostart,&sign_possible);
				pCurrentInterval->ymax=numconv(field);
				break;
			case 'z':
				type = getop(field,materialLine,&infostart,&sign_possible);
				pCurrentInterval->zmin=numconv(field);
				type = getop(field,materialLine,&infostart,&sign_possible); /*ignore - sign */
				type = getop(field,materialLine,&infostart,&sign_possible);
				pCurrentInterval->zmax=numconv(field);
				break;
			case 'w':
				type = getop(field,materialLine,&infostart,&sign_possible);
				pCurrentInterval->wmin=numconv(field);
				type = getop(field,materialLine,&infostart,&sign_possible); /*ignore - sign */
				type = getop(field,materialLine,&infostart,&sign_possible);
				pCurrentInterval->wmax=numconv(field);
				break;
			default:
				pCurrentInterval->pNextInterval = (struct mtrl_line_type *) 
					malloc(sizeof(struct mtrl_line_type)); /* pointer for next interval information */
				pCurrentInterval = pCurrentInterval->pNextInterval;
				pCurrentInterval->xmin = 998.0;
				pCurrentInterval->xmax = 1000.0;
				pCurrentInterval->ymin = 998.0;
				pCurrentInterval->ymax = 1000.0;
				pCurrentInterval->zmin = 998.0;
				pCurrentInterval->zmax = 1000.0;
				pCurrentInterval->wmin = 998.0;
				pCurrentInterval->wmax = 1000.0;
				pCurrentInterval->pNextInterval = NULL;
				fieldlength = getSingField(field,materialLine,&infostart); /* get expression for this interval */
				infix_postfix(field); /* convert the expression into postfix format */
				val_append(field);
				strcpy(pCurrentInterval->expression,field);
		}
		fieldlength = getSingField(field,materialLine,&infostart); /* get next field */ 
	}
	return pFirstInterval;
}

void CMatrls::ListMtrls(char * txtMatrlsData) 
{
/*	this subroutine lists the materials contained in the materials file */
	
	char message[MAX_MESSAGE];
	char tmpStr[100];
	struct mtrl_list *pmaterial;
	
	pmaterial = m_pMtrlListTop;

/*  First, see if the materials file exists */
	if (m_pMtrlListTop == NULL) {
		strcpy(message,"Materials Not Loaded");
		alertboxWin(message);
		return;
	}
	  	
	do {
		sprintf(tmpStr, "%s\t%s\t%s\t%s\n",
			pmaterial->name,pmaterial->type,pmaterial->family,pmaterial->comment);
		strcat(txtMatrlsData, tmpStr);
	} while ((pmaterial = pmaterial->pNext_mtrl) != NULL);

	return;
}

void CMatrls::PopulateMtrlsTree(CTreeCtrl * pMtrlsTree, double tmprt, double xval, bool evaluate)
{
	char message[MAX_MESSAGE];
	char tmpStr[200];
	struct mtrl_list *pmaterial;
	int loc_badversion = false;
	int ioerr_loc = 0;
	double yval, zval, wval, funcvalue, xvalNew;
	struct mtrl_line_type *cur_pInterval;
	UINT *itemStates;
	int numItems = 0;
	int count = 0;

	yval = zval = wval = 999.0;
	xvalNew = xval;
	funcvalue = 0.0;

/*  First, see if the materials file exists */
	if (m_pMtrlListTop == NULL) {
		strcpy(message,"Materials Not Loaded");
		alertboxWin(message);
		return;
	}
	  	
	pmaterial = m_pMtrlListTop;

	ASSERT(pMtrlsTree != NULL);

	HTREEITEM curItem = pMtrlsTree->GetRootItem();
	while(curItem != NULL)
	{
		curItem = pMtrlsTree->GetNextItem(curItem, TVGN_NEXT);
		numItems++;
	}

	if(numItems != 0)
	{
		curItem = pMtrlsTree->GetRootItem();
		itemStates = (UINT*)malloc(sizeof(UINT)*numItems);
		while(curItem != NULL)
		{
			itemStates[count] = pMtrlsTree->GetItemState(curItem, TVIS_EXPANDED);
			curItem = pMtrlsTree->GetNextItem(curItem, TVGN_NEXT);
			count++;
		}
	}

	pMtrlsTree->DeleteAllItems();

	TVINSERTSTRUCT tvInsert;
	tvInsert.hParent = NULL;
	tvInsert.hInsertAfter = NULL;
	tvInsert.item.mask = TVIF_TEXT;
	
	HTREEITEM hMatrl;
	do {
		tvInsert.item.pszText = (LPSTR)malloc(sizeof(pmaterial->name));
		strcpy(tvInsert.item.pszText, pmaterial->name);
		hMatrl = pMtrlsTree->InsertItem(&tvInsert);
		if(numItems != 0 && count > 0) {
			pMtrlsTree->SetItemState(hMatrl, itemStates[numItems-count], TVIS_EXPANDED);
			count--;
		}
		sprintf(tmpStr, "%s", pmaterial->comment);
		pMtrlsTree->InsertItem(TVIF_TEXT,tmpStr, 0, 0, 0, 0, 0, hMatrl, NULL);
		sprintf(tmpStr, "family: %s", pmaterial->family);
		pMtrlsTree->InsertItem(TVIF_TEXT,tmpStr, 0, 0, 0, 0, 0, hMatrl, NULL);
		sprintf(tmpStr, "type: %s", pmaterial->type);
		pMtrlsTree->InsertItem(TVIF_TEXT,tmpStr, 0, 0, 0, 0, 0, hMatrl, NULL);
		if(evaluate) {
			if(pmaterial->pMaterial->alphaline->xmin > 1) xvalNew = 999.0;
			else xvalNew = xval;
			funcvalue = MatEval(pmaterial->pMaterial->alphaline, xvalNew, yval, zval, wval, &loc_badversion, tmprt);
			sprintf(tmpStr, "absorption (cm^-1) = %g", funcvalue);
		}
		else{
			cur_pInterval = pmaterial->pMaterial->alphaline;
			while((xvalNew > cur_pInterval->xmax) && (cur_pInterval != NULL))
			{
				cur_pInterval = cur_pInterval->pNextInterval;
			}
			if(cur_pInterval == NULL)
			{}
			else
			{
				sprintf(tmpStr, "absorption (cm^-1) = %s", cur_pInterval->expression);
			}
		}
		pMtrlsTree->InsertItem(TVIF_TEXT,tmpStr, 0, 0, 0, 0, 0, hMatrl, NULL);
		if(evaluate) {
			if(pmaterial->pMaterial->alphaline->xmin > 1) xvalNew = 999.0;
			else xvalNew = xval;
			funcvalue = MatEval(pmaterial->pMaterial->Ealine, xvalNew, yval, zval, wval, &loc_badversion, tmprt);
			sprintf(tmpStr, "acceptor ionization energy (eV) = %g", funcvalue);
		}
		else{sprintf(tmpStr, "acceptor ionization energy (eV) = %s", pmaterial->pMaterial->Ealine->expression);}
		pMtrlsTree->InsertItem(TVIF_TEXT,tmpStr, 0, 0, 0, 0, 0, hMatrl, NULL);
		if(evaluate) {
			if(pmaterial->pMaterial->alphaline->xmin > 1) xvalNew = 999.0;
			else xvalNew = xval;
			funcvalue = MatEval(pmaterial->pMaterial->Edaline, xvalNew, yval, zval, wval, &loc_badversion, tmprt);
			sprintf(tmpStr, "deep acceptor ionization energy (eV) = %g", funcvalue);
		}
		else{sprintf(tmpStr, "deep acceptor ionization energy (eV) = %s", pmaterial->pMaterial->Edaline->expression);}
		pMtrlsTree->InsertItem(TVIF_TEXT,tmpStr, 0, 0, 0, 0, 0, hMatrl, NULL);
		if(evaluate) {
			if(pmaterial->pMaterial->Eddline->xmin > 1) xvalNew = 999.0;
			else xvalNew = xval;
			funcvalue = MatEval(pmaterial->pMaterial->Eddline, xvalNew, yval, zval, wval, &loc_badversion, tmprt);
			sprintf(tmpStr, "deep donor ionization energy (eV) = %.2g", funcvalue);
		}
		else{sprintf(tmpStr, "deep donor ionization energy (eV) = %s", pmaterial->pMaterial->Eddline->expression);}
		pMtrlsTree->InsertItem(TVIF_TEXT,tmpStr, 0, 0, 0, 0, 0, hMatrl, NULL);
		if(evaluate) {
			if(pmaterial->pMaterial->alphaline->xmin > 1) xvalNew = 999.0;
			else xvalNew = xval;
			funcvalue = MatEval(pmaterial->pMaterial->Edline, xvalNew, yval, zval, wval, &loc_badversion, tmprt);
			sprintf(tmpStr, "donor ionization energy (eV) = %.2g", funcvalue);
		}
		else{sprintf(tmpStr, "donor ionization energy (eV) = %s", pmaterial->pMaterial->Edline->expression);}
		pMtrlsTree->InsertItem(TVIF_TEXT,tmpStr, 0, 0, 0, 0, 0, hMatrl, NULL);
		if(evaluate) {
			if(pmaterial->pMaterial->alphaline->xmin > 1) xvalNew = 999.0;
			else xvalNew = xval;
			funcvalue = MatEval(pmaterial->pMaterial->eemline, xvalNew, yval, zval, wval, &loc_badversion, tmprt);
			sprintf(tmpStr, "electron mass (m/m0) = %.2g", funcvalue);
		}
		else{
			cur_pInterval = pmaterial->pMaterial->eemline;
			while((xvalNew > cur_pInterval->xmax) && (cur_pInterval != NULL))
			{
				cur_pInterval = cur_pInterval->pNextInterval;
			}
			if(cur_pInterval == NULL)
			{}
			else
			{
				sprintf(tmpStr, "electron mass (m/m0) = %s", cur_pInterval->expression);
			}
		}
		pMtrlsTree->InsertItem(TVIF_TEXT,tmpStr, 0, 0, 0, 0, 0, hMatrl, NULL);
		if(evaluate) {
			if(pmaterial->pMaterial->alphaline->xmin > 1) xvalNew = 999.0;
			else xvalNew = xval;
			funcvalue = MatEval(pmaterial->pMaterial->Egline, xvalNew, yval, zval, wval, &loc_badversion, tmprt);
			sprintf(tmpStr, "bandgap energy (eV) = %.2g", funcvalue);
		}
		else{
			cur_pInterval = pmaterial->pMaterial->Egline;
			while((xvalNew > cur_pInterval->xmax) && (cur_pInterval != NULL))
			{
				cur_pInterval = cur_pInterval->pNextInterval;
			}
			if(cur_pInterval == NULL)
			{}
			else
			{
				sprintf(tmpStr, "bandgap energy (eV) = %s", cur_pInterval->expression);
			}
		}
		pMtrlsTree->InsertItem(TVIF_TEXT,tmpStr, 0, 0, 0, 0, 0, hMatrl, NULL);
		if(evaluate) {
			if(pmaterial->pMaterial->alphaline->xmin > 1) xvalNew = 999.0;
			else xvalNew = xval;
			funcvalue = MatEval(pmaterial->pMaterial->emobline, xvalNew, yval, zval, wval, &loc_badversion, tmprt);
			sprintf(tmpStr, "electron mobility (cm^2/V-s) = %.2g", funcvalue);
		}
		else{sprintf(tmpStr, "electron mobility (cm^2/V-s) = %s", pmaterial->pMaterial->emobline->expression);}
		pMtrlsTree->InsertItem(TVIF_TEXT,tmpStr, 0, 0, 0, 0, 0, hMatrl, NULL);
		if(evaluate) {
			if(pmaterial->pMaterial->alphaline->xmin > 1) xvalNew = 999.0;
			else xvalNew = xval;
			funcvalue = MatEval(pmaterial->pMaterial->erline, xvalNew, yval, zval, wval, &loc_badversion, tmprt);
			sprintf(tmpStr, "relative dielectric constant = %.2g", funcvalue);
		}
		else{sprintf(tmpStr, "relative dielectric constant = %s", pmaterial->pMaterial->erline->expression);}
		pMtrlsTree->InsertItem(TVIF_TEXT,tmpStr, 0, 0, 0, 0, 0, hMatrl, NULL);
		if(evaluate) {
			if(pmaterial->pMaterial->alphaline->xmin > 1) xvalNew = 999.0;
			else xvalNew = xval;
			funcvalue = MatEval(pmaterial->pMaterial->hmobline, xvalNew, yval, zval, wval, &loc_badversion, tmprt);
			sprintf(tmpStr, "hole mobility (cm^2/V-s) = %.2g", funcvalue);
		}
		else{sprintf(tmpStr, "hole mobility (cm^2/V-s) = %s", pmaterial->pMaterial->hmobline->expression);}
		pMtrlsTree->InsertItem(TVIF_TEXT,tmpStr, 0, 0, 0, 0, 0, hMatrl, NULL);
		if(evaluate) {
			if(pmaterial->pMaterial->alphaline->xmin > 1) xvalNew = 999.0;
			else xvalNew = xval;
			funcvalue = MatEval(pmaterial->pMaterial->mhhline, xvalNew, yval, zval, wval, &loc_badversion, tmprt);
			sprintf(tmpStr, "heavy hole mass (m/m0) = %.2g", funcvalue);
		}
		else{sprintf(tmpStr, "heavy hole mass (m/m0) = %s", pmaterial->pMaterial->mhhline->expression);}
		pMtrlsTree->InsertItem(TVIF_TEXT,tmpStr, 0, 0, 0, 0, 0, hMatrl, NULL);
		if(evaluate) {
			if(pmaterial->pMaterial->alphaline->xmin > 1) xvalNew = 999.0;
			else xvalNew = xval;
			funcvalue = MatEval(pmaterial->pMaterial->mlhline, xvalNew, yval, zval, wval, &loc_badversion, tmprt);
			sprintf(tmpStr, "light hole mass (m/m0) = %.2g", funcvalue);
		}
		else{sprintf(tmpStr, "light hole mass (m/m0) = %s", pmaterial->pMaterial->mlhline->expression);}
		pMtrlsTree->InsertItem(TVIF_TEXT,tmpStr, 0, 0, 0, 0, 0, hMatrl, NULL);
		sprintf(tmpStr, "acceptor concentration (cm^-3) = %.2E", pmaterial->pMaterial->Na);
		pMtrlsTree->InsertItem(TVIF_TEXT,tmpStr, 0, 0, 0, 0, 0, hMatrl, NULL);
		sprintf(tmpStr, "donor concentration (cm^-3) = %.2E", pmaterial->pMaterial->Nd);
		pMtrlsTree->InsertItem(TVIF_TEXT,tmpStr, 0, 0, 0, 0, 0, hMatrl, NULL);
		sprintf(tmpStr, "deep acceptor concentration (cm^-3) = %.2E", pmaterial->pMaterial->Nda);
		pMtrlsTree->InsertItem(TVIF_TEXT,tmpStr, 0, 0, 0, 0, 0, hMatrl, NULL);
		sprintf(tmpStr, "deep donor concentration (cm^-3) = %.2E", pmaterial->pMaterial->Ndd);
		pMtrlsTree->InsertItem(TVIF_TEXT,tmpStr, 0, 0, 0, 0, 0, hMatrl, NULL);
		if(evaluate) {
			if(pmaterial->pMaterial->alphaline->xmin > 1) xvalNew = 999.0;
			else xvalNew = xval;
			funcvalue = MatEval(pmaterial->pMaterial->offsetline, xvalNew, yval, zval, wval, &loc_badversion, tmprt);
			sprintf(tmpStr, "delta Ec (eV) = %.2g", funcvalue);
		}
		else{sprintf(tmpStr, "delta Ec (eV) = %s", pmaterial->pMaterial->offsetline->expression);}
		pMtrlsTree->InsertItem(TVIF_TEXT,tmpStr, 0, 0, 0, 0, 0, hMatrl, NULL);
		if(evaluate) {
			if(pmaterial->pMaterial->alphaline->xmin > 1) xvalNew = 999.0;
			else xvalNew = xval;
			funcvalue = MatEval(pmaterial->pMaterial->polarizline, xvalNew, yval, zval, wval, &loc_badversion, tmprt);
			sprintf(tmpStr, "polarization = %.2g", funcvalue);
		}
		else{sprintf(tmpStr, "polarization = %s", pmaterial->pMaterial->polarizline->expression);}
		pMtrlsTree->InsertItem(TVIF_TEXT,tmpStr, 0, 0, 0, 0, 0, hMatrl, NULL);
		if(evaluate) {
			if(pmaterial->pMaterial->alphaline->xmin > 1) xvalNew = 999.0;
			else xvalNew = xval;
			funcvalue = MatEval(pmaterial->pMaterial->taunline, xvalNew, yval, zval, wval, &loc_badversion, tmprt);
			sprintf(tmpStr, "electron lifetime (s) = %.2g", funcvalue);
		}
		else{sprintf(tmpStr, "electron lifetime (s) = %s", pmaterial->pMaterial->taunline->expression);}
		pMtrlsTree->InsertItem(TVIF_TEXT,tmpStr, 0, 0, 0, 0, 0, hMatrl, NULL);
		if(evaluate) {
			if(pmaterial->pMaterial->alphaline->xmin > 1) xvalNew = 999.0;
			else xvalNew = xval;
			funcvalue = MatEval(pmaterial->pMaterial->taupline, xvalNew, yval, zval, wval, &loc_badversion, tmprt);
			sprintf(tmpStr, "hole lifetime (s) = %.2g", funcvalue);
		}
		else{sprintf(tmpStr, "hole lifetime (s) = %s", pmaterial->pMaterial->taupline->expression);}
		pMtrlsTree->InsertItem(TVIF_TEXT,tmpStr, 0, 0, 0, 0, 0, hMatrl, NULL);
		if(evaluate) {
			if(pmaterial->pMaterial->alphaline->xmin > 1) xvalNew = 999.0;
			else xvalNew = xval;
			funcvalue = MatEval(pmaterial->pMaterial->valleyline, xvalNew, yval, zval, wval, &loc_badversion, tmprt);
			sprintf(tmpStr, "CB valley degeneracy = %.2g", funcvalue);
		}
		else{sprintf(tmpStr, "CB valley degeneracy = %s", pmaterial->pMaterial->valleyline->expression);}
		pMtrlsTree->InsertItem(TVIF_TEXT,tmpStr, 0, 0, 0, 0, 0, hMatrl, NULL);
	} while ((pmaterial = pmaterial->pNext_mtrl) != NULL);

	return;
}