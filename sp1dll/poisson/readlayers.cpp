#include "stdafx.h"
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "poisp_types.h"
#include "poisp_defs.h"

#include "..\schrpois.h"


#define MAXLOOP 100


struct layer_stack_type {
	struct layer_type *pStartLayer;
	int loopcount;
};
struct layer_stack_type layerStack[MAXLOOP];
int layerStackPointer=0;


/**************************************************/
void CSchrPois::readlayers(char info[], struct global_type *pGlobals, struct flag_type *pFlags) 
{
	int nlayers;					/* layer number*/
	int layertdefined;				/* set if the layer thickness has been defined*/
	int sheetfound,loopfound,type;
	int startsearch;
	char keyword[MAX_EXPRESS],seminame[MAX_SEMINAME],message[MAX_MESSAGE],expression[MAX_EXPRESS];
	int infoLength,fieldlength;
	struct layer_type *pLayer;
	struct layer_stack_type layerStack_loc;

  
/*  Layers start at the surface, and the layer info is stored into the layer list*/  


/*  the first thing to do is to get surface boundary condition   
/*  information, which was passed in the info string.  
/*  There are three possible boundary conditions, Schottky, ohmic, and slope=0  
/*  the type of the  boundary condition is stored for later use.*/  

	startsearch = 0;
	fieldlength = fieldtrim(info,&startsearch,keyword,expression,caseInSens);
	if (!strcmp(keyword,"surface")) {
		while(fieldlength != 0) {
			fieldlength = fieldtrim(info,&startsearch,keyword,expression,caseInSens); /* seperate out the BC keyword*/
			if (!strcmp(keyword,"schottky")) {
				strcpy(pGlobals->surfcontact,"schottky");
				if (strlen(expression) != 0) {
					pGlobals->surfbar=numconv(expression);	/* define a barrier height*/
					pFlags->usesurfbar = true;
				}
				fieldlength = fieldtrim(info,&startsearch,keyword,expression,caseInSens);
				if (keyword[0] == 'v') {	  
					/* surface voltage*/
					keyword[0] = ' ';
	        		pGlobals->surfvoltage=(int)numconv(keyword);
	        	}
				else {
					strcpy(message,"You must declare a voltage source number for a Schottky");
					alertboxWin(message);
					pFlags->stopthemusic = true;
				}
			}
			else if (!strcmp(keyword,"ohmic")) { 
				strcpy(pGlobals->surfcontact,"ohmic");
				fieldlength = fieldtrim(info,&startsearch,keyword,expression,caseInSens);
				if (keyword[0] == 'v') {	  
					/* surface voltage*/
					keyword[0] = ' ';
	        		pGlobals->surfvoltage=(int)numconv(keyword);
	        	}
				else {
					strcpy(message,"You must declare a voltage source number for n Ohmic contact");
					alertboxWin(message);
					pFlags->stopthemusic = true;
				}
			}
			else if (!strcmp(keyword,"slope"))
				strcpy(pGlobals->surfcontact,"slope");
		}
	}
	else {
		strcpy(message,"The layers must be ordered from surface to substrate in the input file.");
		alertboxWin(message);
		pFlags->stopthemusic = true;
	}
	

/*  Now we are ready to begin to process the layers*/

	nlayers = 0;		/* initialize layer counter*/
	pGlobals->pTopLayer =(struct layer_type *) malloc(sizeof(struct layer_type)); /*allocate memory for first layer */
	pGlobals->pTopLayer->pDownLayer = NULL;
	pGlobals->pTopLayer->pUpLayer = NULL;
	pLayer = pGlobals->pTopLayer;	/* initialize current layer pointer */
	loopfound = false;
	
	while (true) {
		layertdefined = false;		/* reset flag for layer thickness*/
		sheetfound = false;
		
/* Get next line from the input file */
		infoLength=getInput(4,info,pGlobals);
		if (infoLength == EOF) {
			strcpy(message,"Error in input file.  Expecting substrate definition.");
			alertboxWin(message);
		}
		
		startsearch = 0;
		fieldlength = fieldtrim(info,&startsearch,keyword,expression,caseInSens);
		if (!strncmp(keyword,"#",1)) continue;	/* look for comment, skip around if it is*/
		
		if (keyword[0] == '}') {
			layerStack_loc = popLayer();
			pLayer = layerloop(layerStack_loc, pLayer);
			continue; /* loop to get next line */
		}

		if (!strncmp(keyword,"sheet",5)){ 
			sheetfound = true;
			layertdefined = true; /* fool loop into ignoring error for this line */
			pLayer->sheet = numconv(expression);
		}
		else {
			/* if a superlattice loop is requested the line must start with a number followed by a { */
			startsearch = 0;
			if ((type=getSingField(keyword,info,&startsearch)) == NUMBER) {
				layerStack_loc.loopcount = atoi(keyword);
				type=getSingField(keyword,info,&startsearch);
				if (keyword[0] == '{') 
					loopfound = true;
					continue; /* loop to get next line */
			}
		}
		
		if (!sheetfound) {
			
/* Allocate memory for the new layer.  If requesting a sheet charge or superlattice loop, no memory is allocated */		
			if (!pFlags->firstlayer) {
				pLayer->pDownLayer = (struct layer_type *) malloc(sizeof(struct layer_type));
				if (pLayer->pDownLayer == NULL) {
					strcpy(message,"Can't allocate memory for layer definitions.  Increase Memory allocation.");
					alertboxWin(message);
					pFlags->stopthemusic = true;
					return;
				}
				pLayer->pDownLayer->pDownLayer = NULL;
				pLayer->pDownLayer->pUpLayer = pLayer;
				pLayer = pLayer->pDownLayer;
			}
			else 
				pFlags->firstlayer = false;
			
			if (loopfound) {
				loopfound = false;
				layerStack_loc.pStartLayer = pLayer;
				pushLayer(layerStack_loc);
			}
			
			pLayer->dyLayer = pGlobals->defaultdy;
			pLayer->noholesLayer = pFlags->defaultNoHoles;
			pLayer->noelecLayer = pFlags->defaultNoElec;
			pLayer->fullyionLayer = pFlags->defFullyIonized;
			strcpy(pLayer->NasLayer,"0.0");
			strcpy(pLayer->NdsLayer,"0.0");
			strcpy(pLayer->xvalLayer,"2.0");
			pLayer->sheet = 0.0;
			pLayer->EfLayer[0] = '\0';
			pLayer->NaLayer[0] = '\0';
			pLayer->EaLayer[0] = '\0';
			pLayer->NdLayer[0] = '\0';
			pLayer->EdLayer[0] = '\0';
			pLayer->NdaLayer[0] = '\0';
			pLayer->EdaLayer[0] = '\0';
			pLayer->NddLayer[0] = '\0';
			pLayer->EddLayer[0] = '\0';
			pLayer->EdsLayer[0] = '\0';
			pLayer->EasLayer[0] = '\0';
			pLayer->xvalLayer[0] = '\0';
			pLayer->yvalLayer[0] = '\0';
			pLayer->zvalLayer[0] = '\0';
			pLayer->wvalLayer[0] = '\0';
			pLayer->EgLayer[0] = '\0';
			pLayer->offsetLayer[0] = '\0';
			pLayer->erLayer[0] = '\0';
			pLayer->eemLayer[0] = '\0';
			pLayer->mhhLayer[0] = '\0';
			pLayer->mlhLayer[0] = '\0';
			pLayer->emobLayer[0] = '\0';
			pLayer->hmobLayer[0] = '\0';
			pLayer->taunLayer[0] = '\0';
			pLayer->taupLayer[0] = '\0';
			pLayer->polarizLayer[0] = '\0';
			pLayer->alphaLayer[0] = '\0';
			pLayer->valleyLayer[0] = '\0';		
		    strcpy(pLayer->contactLayer,"slope");		/* default contact for layer is slope=0*/
		    strcpy(pLayer->EfLayer,"0.0");				/* default for the Fermi level to be 0 */
  
/*  Look for the information for this layer by searching the line from the input file */  
/*  extract first field which is required to be the semiconductor name for the layer,  
/*  or the substrate delimiter to signal the end of the layers */ 

			startsearch = 0;
			fieldlength = fieldtrim(info,&startsearch,seminame,expression,caseInSens);


			if (!strcmp(seminame,"substrate")) {
				while(fieldlength !=0) {
					fieldlength = fieldtrim(info,&startsearch,keyword,expression,caseInSens); /* seperate out the keyword*/
					if (!strcmp(keyword,"schottky")) {
						strcpy(pGlobals->subcontact,"schottky");
						if (strlen(expression) != 0) {
							pGlobals->subbar=numconv(expression);	/* define a barrier height*/
							pFlags->usesubbar = true;
						}
						fieldlength = fieldtrim(info,&startsearch,keyword,expression,caseInSens);
						if (keyword[0] == 'v') {	  
							/* substrate voltage*/
							keyword[0] = ' ';
		        			pGlobals->subvoltage=(int)numconv(keyword);
		        		}
						else {
							strcpy(message,"You must declare a voltage source number for a Schottky");
							alertboxWin(message);
							pFlags->stopthemusic = true;
						}
					}
					else if (!strcmp(keyword,"ohmic")) { 
						strcpy(pGlobals->subcontact,"ohmic");
						fieldlength = fieldtrim(info,&startsearch,keyword,expression,caseInSens);
						if (keyword[0] == 'v') {	  
							/* surface voltage*/
							keyword[0] = ' ';
		        			pGlobals->subvoltage=(int)numconv(keyword);
		        		}
						else {
							strcpy(message,"You must declare a voltage source number for an Ohmic contact");
							alertboxWin(message);
							pFlags->stopthemusic = true;
						}
					}
					else if (!strcmp(keyword,"slope"))
						strcpy(pGlobals->subcontact,"slope");
				}
				pLayer = pLayer->pUpLayer;
				free(pLayer->pDownLayer);
				pLayer->pDownLayer = NULL;
				return;
			}

/*  At this point the seminame must contain a semiconductor name, so:*/

			startsearch = 0;
			fieldlength = fieldtrim(info,&startsearch,seminame,expression,caseSens);
			strcpy(pLayer->nameLayer,seminame);/* save the semiconductor name for confirmation, case sensitive*/
/*			startsearch = 0;
			fieldlength = fieldtrim(info,&startsearch,seminame,caseInSens); /* get case insensitive name for information retrevial */
/*			ioerr_loc = semisearch(seminame,pLayer->xvalLayer,pLayer,simfamily,pGlobals,pFlags);*/	/* get information from materials file*/
/*		    if (pFlags->badversion) return;*/

  
/*  the rest of each line is assumed to contain parameter info for each layer  */
			PostInfoSP(_T("readlayers: Reading each layer parameter info"));
			while (fieldlength != 0) {

				fieldlength = fieldtrim(info,&startsearch,keyword,expression,caseInSens);	/* look for addition info fields*/
	    
				if (!strncmp(keyword,"ohmic",5)) 
					strcpy(pLayer->contactLayer,"ohmic");

				else if (!strncmp(keyword,"slope",5)) {
					strcpy(pLayer->contactLayer,"slope");
				}	

				else if (!strncmp(keyword,"no",2)) {  /* Carrier restrictions requested*/
					if (!strncmp(expression,"el",2)) pLayer->noelecLayer = true;		/* no electrons allowed*/
					else if (!strncmp(expression,"ho",2)) pLayer->noholesLayer = true;		/* no holes allowed*/
				}

				else if (!strncmp(keyword,"elec",4)) 	/* electrons allowed */
					pLayer->noelecLayer = false;

				else if (!strncmp(keyword,"holes",5)) /* holes allowed*/
					pLayer->noholesLayer = false;	

				else if (!strncmp(keyword,"fullyionized",12)) /* Assume dopants are all ionized */
					pLayer->fullyionLayer = true;

				else if (!strncmp(keyword,"ionize",6)) /* Calculate dopant ionization */
					pLayer->fullyionLayer = false;

				else if (!strncmp(keyword,"t",1)) {
					layertdefined = true;
					pLayer->numberLayer = ++nlayers;
					pLayer->tLayer = numconv(expression);
				}
				
				else if (!strncmp(keyword,"x",1)) {
					strcpy(pLayer->xvalLayer,expression);
					infix_postfix(pLayer->xvalLayer);
				}

				else if (!strncmp(keyword,"yval",1)){
					strcpy(pLayer->yvalLayer,expression);
					infix_postfix(pLayer->yvalLayer);
				}

				else if (!strncmp(keyword,"zval",1)){
					strcpy(pLayer->zvalLayer,expression);
					infix_postfix(pLayer->zvalLayer);
				}

				else if (!strncmp(keyword,"wval",1)){
					strcpy(pLayer->wvalLayer,expression);
					infix_postfix(pLayer->wvalLayer);
				}

				else if (!strncmp(keyword,"dy",2)) 
					pLayer->dyLayer = numconv(expression);

/*  A 'Ef' keyword signals that the fermi level is non-zero in this layer.  
/*  identifier is stored in the array voltage*/

				else if (!strncmp(keyword,"ef",2)) {
					strcpy(pLayer->EfLayer,expression);
					infix_postfix(pLayer->EfLayer);
				}
		
/*  The rest of the possible keywords signal an override of the values taken from the  
/*  semiconductors file*/

/*		Energy gap*/
				else if (!strncmp(keyword,"eg",2)) {
					strcpy(pLayer->EgLayer,expression);
					infix_postfix(pLayer->EgLayer);
				}
/*		Band offset	 */ 
		   	 	else if (!strncmp(keyword,"deltaec",7)) {
					strcpy(pLayer->offsetLayer,expression);
					infix_postfix(pLayer->offsetLayer);
				}

/*		Relative dielectric constant*/	  
		    	else if (!strncmp(keyword,"er",2)) {
					strcpy(pLayer->erLayer,expression);
					infix_postfix(pLayer->erLayer);
				}

/*		Electron effective mass	*/  
				else if (!strncmp(keyword,"me",2)) {
					strcpy(pLayer->eemLayer,expression);
					infix_postfix(pLayer->eemLayer);
				}

/*		Hole effective mass	  */
				else if (!strncmp(keyword,"mhh",3)) {
					strcpy(pLayer->mhhLayer,expression);
					infix_postfix(pLayer->mhhLayer);
				}

/*		Light Hole effective mass	*/  
				else if (!strncmp(keyword,"mlh",3)) {
					strcpy(pLayer->mlhLayer,expression);
					infix_postfix(pLayer->mlhLayer);
				}

/*		Donor level	  */
		    	else if (!strncmp(keyword,"ed",2)) {
					strcpy(pLayer->EdLayer,expression);
					infix_postfix(pLayer->EdLayer);
				}

/*		Acceptor level*/	  
				else if (!strncmp(keyword,"ea",2)) {
					strcpy(pLayer->EaLayer,expression);
					infix_postfix(pLayer->EaLayer);
				}

/*		Deep donor level	*/  
				else if (!strncmp(keyword,"edd",3)) {
					strcpy(pLayer->EddLayer,expression);
					infix_postfix(pLayer->EddLayer);
				}

/*		Deep acceptor level	*/  
				else if (!strncmp(keyword,"eda",3)) {
					strcpy(pLayer->EdaLayer,expression);
					infix_postfix(pLayer->EdaLayer);
			}

/*		Deep donor concentration	*/  
		    	else if (!strncmp(keyword,"ndd",3)) {
					strcpy(pLayer->NddLayer,expression);
					infix_postfix(pLayer->NddLayer);
				}

/*		Deep acceptor concentration	 */ 
		    	else if (!strncmp(keyword,"nda",3)) {
					strcpy(pLayer->NdaLayer,expression);
					infix_postfix(pLayer->NdaLayer);
				}

/*		Donor concentration	  */
				else if (!strncmp(keyword,"nd",2)) {
					strcpy(pLayer->NdLayer,expression);
					infix_postfix(pLayer->NdLayer);
				}

/*		Acceptor concentration	  */
				else if (!strncmp(keyword,"na",2)) {
					strcpy(pLayer->NaLayer,expression);
					infix_postfix(pLayer->NaLayer);
				}

/*		electron mobility	  */
		    	else if (!strncmp(keyword,"emob",4)) {
					strcpy(pLayer->emobLayer,expression);
					infix_postfix(pLayer->emobLayer);
				}

/*		hole mobility	  */
		    	else if (!strncmp(keyword,"hmob",4)) {
					strcpy(pLayer->hmobLayer,expression);
					infix_postfix(pLayer->hmobLayer);
				}

/*		electron recombination time	  */
				else if (!strncmp(keyword,"tn",2)) {
					strcpy(pLayer->taunLayer,expression);
					infix_postfix(pLayer->taunLayer);
				}

/*		hole recombination time	  */
				else if (!strncmp(keyword,"tp",2)) {
					strcpy(pLayer->taupLayer,expression);
					infix_postfix(pLayer->taupLayer);
				}

/*		Semiconductor Polarization	  */
		    	else if (!strncmp(keyword,"polariz",7)) {
					strcpy(pLayer->polarizLayer,expression);
					infix_postfix(pLayer->polarizLayer);
				}

/*		Absorption coefficient	  */
				else if (!strncmp(keyword,"alpha",5)) {
					strcpy(pLayer->alphaLayer,expression);
					infix_postfix(pLayer->alphaLayer);
				}

/*		Valley degeneracy	  */
				else if (!strncmp(keyword,"valley",6)) {
					strcpy(pLayer->valleyLayer,expression);
					infix_postfix(pLayer->valleyLayer);
				}
  
/*		Sheet donor concentration	*/  
		    	else if (!strncmp(keyword,"nds",3)) {
					strcpy(pLayer->NdsLayer,expression);
					infix_postfix(pLayer->NdsLayer);
				}

/*		Sheet acceptor concentration	*/  
		    	else if (!strncmp(keyword,"nas",3)) {
					strcpy(pLayer->NasLayer,expression);
					infix_postfix(pLayer->NasLayer);
				}

				else
					if (fieldlength > 0) {
						strcpy(message,"Please correct input file.  Sorry, what does ");
						strcat(message,keyword);
						strcat(message," mean?");
						alertboxWin(message);
						pFlags->stopthemusic = true;
					}
			}
		}

/*		pLayer->NdsLayer = pLayer->NdsLayer*1.0e8/pLayer->tLayer;
		pLayer->NasLayer = pLayer->NasLayer*1.0e8/pLayer->tLayer;	*/

		if (!layertdefined && info[0] != '#') {
			strcpy(message,"Please correct input file. A material definition line must contain at least the material name and thickness.");
			alertboxWin(message);
			pFlags->stopthemusic = true;
			return;
		}
	}

	return;
}


/************************************************************/	
int CSchrPois::semisearch(char seminame[MAX_SEMINAME], double xval, double yval, double zval, double wval, 
							struct dummy_layer_type *pLayer, char simfamily[MAX_SEMINAME], 
							struct global_type *pGlobals, struct flag_type *pFlags) 
{
/*  this routine searches the materials data structure in its quest to find   
/*  the desired data  */


	char message[MAX_MESSAGE],dummyname[MAX_SEMINAME],dummyname2[MAX_SEMINAME];
	struct mtrl_list *pMtrlList; 


/* First, see if the needed material is in the materials data structure*/
	strcpy(dummyname,seminame);
	lowercase(dummyname);
	pMtrlList = pGlobals->pTopMtrl;
	strcpy(dummyname2,pMtrlList->name);
	lowercase(dummyname2);
	while(strcmp(dummyname2,dummyname)) {
		if (pMtrlList->pNext_mtrl == NULL) {
			pMtrlList = NULL;
			break;
		}
		else {
			pMtrlList=pMtrlList->pNext_mtrl;
			strcpy(dummyname2,pMtrlList->name);
			lowercase(dummyname2);
		}
	}

/*  If the material is not in the materials list, issue an error	*/
	if (pMtrlList == NULL) {
		strcpy(message,"Material ");
		strcat(message,seminame);
		strcat(message," not found.  Please update materials file");
		alertboxWin(message);
		pFlags->badversion = true;
		return 1;	/* kick out if material is not in matrl file*/
	}

/*  Check to make sure that all materials in the simulation are of the same family*/

	if (pFlags->checkfamily) {
		if (pFlags->firstlayer) {
			strcpy(simfamily,pMtrlList->family);
			pFlags->firstlayer = false;
		}
		else if (strcmp(simfamily,pMtrlList->family)) {
			strcpy(message,"Two different material families are used in your input file. All layers must be of one family.");
			alertboxWin(message);
			pFlags->badversion = true;
			return 1;
		}
	}


/* Get the data from the materials structure */

	pLayer->NdLayer = pMtrlList->pMaterial->Nd;
	pLayer->NaLayer = pMtrlList->pMaterial->Na;
	pLayer->NddLayer = pMtrlList->pMaterial->Ndd;
	pLayer->NdaLayer = pMtrlList->pMaterial->Nda;

	pLayer->EgLayer = MatEval(pMtrlList->pMaterial->Egline,xval,yval,zval,wval,&pFlags->badversion,pGlobals);
			if (pFlags->badversion) 
				return 1;

	pLayer->offsetLayer = MatEval(pMtrlList->pMaterial->offsetline,xval,yval,zval,wval,&pFlags->badversion,pGlobals);
			if (pFlags->badversion) 
				return 1;

	pLayer->erLayer = MatEval(pMtrlList->pMaterial->erline,xval,yval,zval,wval,&pFlags->badversion,pGlobals);
			if (pFlags->badversion) 
				return 1;

	pLayer->EdLayer = MatEval(pMtrlList->pMaterial->Edline,xval,yval,zval,wval,&pFlags->badversion,pGlobals);
			if (pFlags->badversion) 
				return 1;

	pLayer->EaLayer = MatEval(pMtrlList->pMaterial->Ealine,xval,yval,zval,wval,&pFlags->badversion,pGlobals);
			if (pFlags->badversion) 
				return 1;

	pLayer->EddLayer = MatEval(pMtrlList->pMaterial->Eddline,xval,yval,zval,wval,&pFlags->badversion,pGlobals);
			if (pFlags->badversion) 
			return 1;

	pLayer->EdaLayer = MatEval(pMtrlList->pMaterial->Edaline,xval,yval,zval,wval,&pFlags->badversion,pGlobals);
			if (pFlags->badversion) 
				return 1;

	pLayer->eemLayer = MatEval(pMtrlList->pMaterial->eemline,xval,yval,zval,wval,&pFlags->badversion,pGlobals);
			if (pFlags->badversion) 
				return 1;

	pLayer->valleyLayer = MatEval(pMtrlList->pMaterial->valleyline,xval,yval,zval,wval,&pFlags->badversion,pGlobals);
			if (pFlags->badversion) 
				return 1;

	pLayer->mhhLayer = MatEval(pMtrlList->pMaterial->mhhline,xval,yval,zval,wval,&pFlags->badversion,pGlobals);
			if (pFlags->badversion) 
				return 1;

	pLayer->mlhLayer = MatEval(pMtrlList->pMaterial->mlhline,xval,yval,zval,wval,&pFlags->badversion,pGlobals);
			if (pFlags->badversion) 
				return 1;

	pLayer->emobLayer = MatEval(pMtrlList->pMaterial->emobline,xval,yval,zval,wval,&pFlags->badversion,pGlobals);
			if (pFlags->badversion) 
				return 1;

	pLayer->hmobLayer = MatEval(pMtrlList->pMaterial->hmobline,xval,yval,zval,wval,&pFlags->badversion,pGlobals);
			if (pFlags->badversion) 
				return 1;

	pLayer->taupLayer = MatEval(pMtrlList->pMaterial->taupline,xval,yval,zval,wval,&pFlags->badversion,pGlobals);
			if (pFlags->badversion) 
				return 1;

	pLayer->taunLayer = MatEval(pMtrlList->pMaterial->taunline,xval,yval,zval,wval,&pFlags->badversion,pGlobals);
			if (pFlags->badversion) 
				return 1;

	pLayer->polarizLayer = MatEval(pMtrlList->pMaterial->polarizline,xval,yval,zval,wval,&pFlags->badversion,pGlobals);
			if (pFlags->badversion) 
				return 1;

	pLayer->alphaLayer = MatEval(pMtrlList->pMaterial->alphaline,xval,yval,zval,wval,&pFlags->badversion,pGlobals);
			if (pFlags->badversion) 
				return 1;

	return 0;
}


/***********************************************************/
double CSchrPois::MatEval(struct mtrl_line_type * pCurrentInterval, double xval, double yval, double zval, 
				double wval, int *loc_badversion, struct global_type *pGlobals) {

/***********************************************************/

	int rightrange;
	double functionvalue;
	char message[MAX_MESSAGE];

	rightrange = false;        
  
/*  Get the parameter info that is stored in the materials structure */

/*  If the x value is in the specification range for this parameter, store the information*/
	while(pCurrentInterval != NULL) {
		if (xval <= pCurrentInterval->xmax  && xval >= pCurrentInterval->xmin &&
				yval <= pCurrentInterval->ymax  && yval >= pCurrentInterval->ymin &&
				zval <= pCurrentInterval->zmax  && zval >= pCurrentInterval->zmin &&
				wval <= pCurrentInterval->wmax  && wval >= pCurrentInterval->wmin ) {

			functionvalue = evaluate(pCurrentInterval->expression, pGlobals);	/*calculate the parameter*/
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

/**************************************************/
int CSchrPois::loadMaterials(struct global_type *pGlobals, struct flag_type *pFlags) {
  
/***************************************************************************/
/*This routine loads the materials file into a linked list, the first element of which */
/* is pointed to by pGlobals->pTopMtrl */

	//char seminame[MAX_SEMINAME];
	char field[MAX_EXPRESS],access,extension[MAX_EXTENSION],message[MAX_MESSAGE];
	char materialLine[MAX_INPUTLINE];
	int semistart,infoLength,firstMaterial,ioerr_loc,fieldlength,matrlCount;
	struct mtrl_list *pMtrlList;

	firstMaterial = true;
	pMtrlList = pGlobals->pTopMtrl;

	PostInfoSP(_T("Loading materials"));

/*	 open materials file	*/
	access='r';
	strcpy(extension,"");
	ioerr_loc = fileopen(3, pGlobals->matrlfile, access, extension, pGlobals, "Load Materials file with extension .txt");
	if(ioerr_loc)
		return 1;

	/* read data line, which will then be analyzed */
	infoLength=getInput(3,materialLine,pGlobals);

	semistart = 0;
	fieldlength=fieldtrim(materialLine,&semistart,field,extension,caseInSens);
	if (strcmp(field,pGlobals->materialsversion)) {
	strcpy(message,"Wrong Version of the Materials File");
		alertboxWin(message);
		pFlags->badversion = true;
		fileclose (3, pGlobals);
		PostInfoSP(_T("Loading materials failed"));
		return 1;
	}

/*  Now that we know the material file exists, and is the correct version, load the materials  
  	by loading the materials info into the materials structure.*/
	matrlCount = 0;
	while (infoLength != EOF) {
		infoLength=getInput(3,materialLine,pGlobals);
  
  		if(materialLine[0] == '#')
  			continue;
  		
/*	   first field contains the semiconductor name*/
		semistart = 0;
		fieldlength = getSingField(field,materialLine,&semistart);
		/*lowercase(field);*/
		
		if (fieldlength != '\0') {			
			if (firstMaterial) {
				pMtrlList->pMaterial = (struct mtrl_type *) 
					malloc(sizeof(struct mtrl_type)); /* pointer for first material layer */
				firstMaterial = false;
			}
			else {
				pMtrlList->pNext_mtrl = (struct mtrl_list *) malloc(sizeof(struct mtrl_list));
				pMtrlList->pNext_mtrl->pNext_mtrl = NULL;
				pMtrlList->pNext_mtrl->pMaterial = NULL;
				pMtrlList = pMtrlList->pNext_mtrl;
				pMtrlList->pMaterial = (struct mtrl_type *) 
					malloc(sizeof(struct mtrl_type)); /* pointer for next material layer */
			}
			
			matrlCount++;
			strcpy(pMtrlList->name,field); /* Save semiconductor name */
			sprintf(message,"material #%i - %s",matrlCount,pMtrlList->name);
			PostInfoSP(message);
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
	
			infoLength=getInput(3,materialLine,pGlobals); /* get next line (default dopings) */
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
				
			infoLength=getInput(3,materialLine,pGlobals); /* get next line (Eg info) */
			lowercase(materialLine);
			pMtrlList->pMaterial->Egline = parameterInfo(materialLine);
			infoLength=getInput(3,materialLine,pGlobals); /* get next line (Delta EC info) */
			lowercase(materialLine);
			pMtrlList->pMaterial->offsetline = parameterInfo(materialLine);
			infoLength=getInput(3,materialLine,pGlobals); /* get next line (Rel diel const info) */
			lowercase(materialLine);
			pMtrlList->pMaterial->erline = parameterInfo(materialLine);
			infoLength=getInput(3,materialLine,pGlobals); /* get next line (Ed info) */
			lowercase(materialLine);
			pMtrlList->pMaterial->Edline = parameterInfo(materialLine);
			infoLength=getInput(3,materialLine,pGlobals); /* get next line (Ea info) */
			lowercase(materialLine);
			pMtrlList->pMaterial->Ealine = parameterInfo(materialLine);
			infoLength=getInput(3,materialLine,pGlobals); /* get next line (Edd info) */
			lowercase(materialLine);
			pMtrlList->pMaterial->Eddline = parameterInfo(materialLine);
			infoLength=getInput(3,materialLine,pGlobals); /* get next line (Eda info) */
			lowercase(materialLine);
			pMtrlList->pMaterial->Edaline = parameterInfo(materialLine);
			infoLength=getInput(3,materialLine,pGlobals); /* get next line (elec eff mass info) */
			lowercase(materialLine);
			pMtrlList->pMaterial->eemline = parameterInfo(materialLine);
			infoLength=getInput(3,materialLine,pGlobals); /* get next line (Cond band degen info) */
			lowercase(materialLine);
			pMtrlList->pMaterial->valleyline = parameterInfo(materialLine);
			infoLength=getInput(3,materialLine,pGlobals); /* get next line (heavy hole eff mass info) */
			lowercase(materialLine);
			pMtrlList->pMaterial->mhhline = parameterInfo(materialLine);
			infoLength=getInput(3,materialLine,pGlobals); /* get next line (Light hole eff mass info) */
			lowercase(materialLine);
			pMtrlList->pMaterial->mlhline = parameterInfo(materialLine);
			infoLength=getInput(3,materialLine,pGlobals); /* get next line (elec mobil info) */
			lowercase(materialLine);
			pMtrlList->pMaterial->emobline = parameterInfo(materialLine);
			infoLength=getInput(3,materialLine,pGlobals); /* get next line (hole mobil info) */
			lowercase(materialLine);
			pMtrlList->pMaterial->hmobline = parameterInfo(materialLine);
			infoLength=getInput(3,materialLine,pGlobals); /* get next line (elec recomb time info) */
			lowercase(materialLine);
			pMtrlList->pMaterial->taunline = parameterInfo(materialLine);
			infoLength=getInput(3,materialLine,pGlobals); /* get next line (hole recomb time info) */
			lowercase(materialLine);
			pMtrlList->pMaterial->taupline = parameterInfo(materialLine);
			infoLength=getInput(3,materialLine,pGlobals); /* get next line (polariz info) */
			lowercase(materialLine);
			pMtrlList->pMaterial->polarizline = parameterInfo(materialLine);
			infoLength=getInput(3,materialLine,pGlobals); /* get next line (absorption info) */
			lowercase(materialLine);
			pMtrlList->pMaterial->alphaline = parameterInfo(materialLine);
				
			
			infoLength=getInput(3,materialLine,pGlobals);  /* skip the "end" line */
		} 
	}
	PostInfoSP(_T("Completed loading materials"));

	pFlags->nomtrlfile = false;

	return 0;
}
/**************************************************/
struct mtrl_line_type * CSchrPois::parameterInfo(char materialLine[]) {

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

/**************************************************/
struct layer_type * CSchrPois::layerloop(struct layer_stack_type layerStack_loc, struct layer_type *pLayer) {

/**************************************************/
	
	int i,layernumber;
	char message[MAX_MESSAGE];
	struct layer_type *pLayer_loc,*pLayer_new;
	
	pLayer_new = pLayer;
	layernumber = pLayer->numberLayer;
	for (i=1;i<=layerStack_loc.loopcount-1;i++) {
		pLayer_loc = layerStack_loc.pStartLayer;
		while (pLayer_loc->pUpLayer != pLayer) {
			pLayer_new->pDownLayer = (struct layer_type *) malloc(sizeof(struct layer_type));
			if (pLayer_new->pDownLayer == NULL) {
				strcpy(message,"Can't allocate memory for layer definitions.  Increase Memory allocation.");
				alertboxWin(message);
				return NULL;
			}
			pLayer_new->pDownLayer->pDownLayer = NULL;
			pLayer_new->pDownLayer->pUpLayer = pLayer_new;
			pLayer_new = pLayer_new->pDownLayer;
			
			strcpy(pLayer_new->nameLayer,pLayer_loc->nameLayer);
			pLayer_new->dyLayer = pLayer_loc->dyLayer;
			pLayer_new->tLayer = pLayer_loc->tLayer;
			pLayer_new->noholesLayer = pLayer_loc->noholesLayer;
			pLayer_new->noelecLayer = pLayer_loc->noelecLayer;
			pLayer_new->fullyionLayer = pLayer_loc->fullyionLayer;
			strcpy(pLayer_new->NasLayer,pLayer_loc->NasLayer);
			strcpy(pLayer_new->NdsLayer,pLayer_loc->NdsLayer);
			pLayer_new->sheet = pLayer_loc->sheet;
			strcpy(pLayer_new->xvalLayer,pLayer_loc->xvalLayer);
			strcpy(pLayer_new->EfLayer,pLayer_loc->EfLayer);
			strcpy(pLayer_new->NaLayer,pLayer_loc->NaLayer);
			strcpy(pLayer_new->EaLayer,pLayer_loc->EaLayer);
			strcpy(pLayer_new->NdLayer,pLayer_loc->NdLayer);
			strcpy(pLayer_new->EdLayer,pLayer_loc->EdLayer);
			strcpy(pLayer_new->NdaLayer,pLayer_loc->NdaLayer);
			strcpy(pLayer_new->EdaLayer,pLayer_loc->EdaLayer);
			strcpy(pLayer_new->NddLayer,pLayer_loc->NddLayer);
			strcpy(pLayer_new->EddLayer,pLayer_loc->EddLayer);
			strcpy(pLayer_new->NdsLayer,pLayer_loc->NdsLayer);
			strcpy(pLayer_new->EdsLayer,pLayer_loc->EdsLayer);
			strcpy(pLayer_new->NasLayer,pLayer_loc->NasLayer);
			strcpy(pLayer_new->EasLayer,pLayer_loc->EasLayer);
			strcpy(pLayer_new->xvalLayer,pLayer_loc->xvalLayer);
			strcpy(pLayer_new->yvalLayer,pLayer_loc->yvalLayer);
			strcpy(pLayer_new->zvalLayer,pLayer_loc->zvalLayer);
			strcpy(pLayer_new->wvalLayer,pLayer_loc->wvalLayer);
			strcpy(pLayer_new->EgLayer,pLayer_loc->EgLayer);
			strcpy(pLayer_new->offsetLayer,pLayer_loc->offsetLayer);
			strcpy(pLayer_new->erLayer,pLayer_loc->erLayer);
			strcpy(pLayer_new->eemLayer,pLayer_loc->eemLayer);
			strcpy(pLayer_new->mhhLayer,pLayer_loc->mhhLayer);
			strcpy(pLayer_new->mlhLayer,pLayer_loc->mlhLayer);
			strcpy(pLayer_new->emobLayer,pLayer_loc->emobLayer);
			strcpy(pLayer_new->hmobLayer,pLayer_loc->hmobLayer);
			strcpy(pLayer_new->taunLayer,pLayer_loc->taunLayer);
			strcpy(pLayer_new->taupLayer,pLayer_loc->taupLayer);
			strcpy(pLayer_new->polarizLayer,pLayer_loc->polarizLayer);
			strcpy(pLayer_new->alphaLayer,pLayer_loc->alphaLayer);
			strcpy(pLayer_new->valleyLayer,pLayer_loc->valleyLayer);
		    strcpy(pLayer_new->contactLayer,pLayer_loc->contactLayer);
		    strcpy(pLayer_new->EfLayer,pLayer_loc->EfLayer);
		    pLayer_new->numberLayer = ++layernumber;
		    pLayer_loc = pLayer_loc->pDownLayer;	
		} 
	}
	return pLayer_new;
}

/* Push layer pointer and loop count onto the stack */
void CSchrPois::pushLayer(struct layer_stack_type layerStack_loc)
/**************************************************/
{
	char message[MAX_MESSAGE];
	if (layerStackPointer < MAXLOOP)
		layerStack[layerStackPointer++] = layerStack_loc;
	else {
		strcpy(message,"Error: stack full, can't push new layer value\n");
		alertboxWin(message);
	}
}


/* pop: pop and return top value from layer stack */
struct layer_stack_type CSchrPois::popLayer()
/**************************************************/
{
	char message[MAX_MESSAGE];
	struct layer_stack_type dummy;

	dummy.loopcount=0;
	dummy.pStartLayer=NULL;

	if (layerStackPointer > 0)
		return layerStack[--layerStackPointer];
	else {
		strcpy(message,"Error: Layer stack empty\n");
		alertboxWin(message);
		return dummy;
	}
}

