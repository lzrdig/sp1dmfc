#include "stdafx.h"
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "poisp_types.h"
#include "poisp_defs.h"

#include "..\schrpois.h"


void CSchrPois::materialparams(struct global_type *pGlobals, struct flag_type *pFlags) {
  
/*  This subroutine shows the material parameters for a user selected material.  
  To do this, the input from the user is treated as if it were a line from  
  an input file, and the subroutine semisearch is called to extract the info.  
  The problem is that the info extracted must be put someplace where it won't  
  interfere with existing layers, so it is stored in layer maxlayers+1. */

	char info[MAX_SEMINAME],seminame[MAX_SEMINAME],keyword[MAX_KEYWORD],expression[MAX_EXPRESS],simfamily[MAX_SEMINAME],c;
	int fieldlength, ioerr_loc,i,startsearch;
	double xval;
	struct dummy_layer_type *pdummyLayer;
	
	pdummyLayer =(struct dummy_layer_type *) malloc(sizeof(struct dummy_layer_type)); /*allocate memory for layer */
 	
	xval = 2.0e0;		/* default x value */
	/*pFlags->firstlayer = .true.	! set first layer flag so that family is stored in simfamily
	pFlags->badversion = .false.*/
	
	printf("Enter material name (and x value if a ternary): ");
	/*scanf("%s %s", info,field);*/
	c=getchar();
	for (i=0;i<MAX_SEMINAME-1 && (c=getchar()) != '\n' && c!= '\r';i++)
		info[i] = c;
		info[i+1] = '\0';
	
	startsearch = 0;
	fieldlength = fieldtrim(info,&startsearch,seminame,expression,caseInSens);
	pFlags->checkfamily = false;
	  
/*  Look for the x value of this layer by searching the rest of the line from the input file */  

	while (fieldlength != 0) {
		fieldlength = fieldtrim(info,&startsearch,keyword,expression,caseInSens);
		if (!strncmp(keyword,"x",1)){ 
			xval = numconv(expression);
		}
	}
	
	ioerr_loc = semisearch(seminame,pGlobals->xval,pGlobals->yval,pGlobals->zval,pGlobals->wval,pdummyLayer,simfamily,pGlobals,pFlags);	/* get information from materials file*/
	if (pFlags->badversion) return;
	
	printf("\n");
	printf("Energy Gap = %6.3f eV\n",pdummyLayer->EgLayer);
	printf("Conduction band offset relative to family basis %s = %5.3f eV\n",simfamily,pdummyLayer->offsetLayer);
	printf("Relative dielectric constant = %6.3f\n",pdummyLayer->erLayer);
	printf("Polarization concentration = %9.2e Ccm-2\n",pdummyLayer->polarizLayer);
	printf("Electron effective mass = %6.3f\n",pdummyLayer->eemLayer);
	printf("Cond. band degeneracy = %5.3f\n",pdummyLayer->valleyLayer);
	printf("Heavy hole effective mass = %5.3f\n",pdummyLayer->mhhLayer);
	printf("Light hole effective mass = %5.3f\n",pdummyLayer->mlhLayer);
	printf("Donor level = %5.3f eV\n",pdummyLayer->EdLayer);
	printf("Default donor concentration = %9.2e cm-3\n",pdummyLayer->NdLayer);
	printf("Acceptor level = %9.2e eV\n",pdummyLayer->EaLayer);
	printf("Default acceptor concentration = %9.2e cm-3\n",pdummyLayer->NaLayer);
	printf("Deep donor level = %9.2e eV\n",pdummyLayer->EddLayer);
	printf("Default deep donor concentration =  %9.2e cm-3\n",pdummyLayer->NddLayer);
	printf("Deep acceptor level = %9.2e eV\n",pdummyLayer->EdaLayer);
	printf("Default deep acceptor concentration =  %9.2e cm-3\n",pdummyLayer->NdaLayer);
	printf("Default electron mobility = %10.3e cm2/V-s\n",pdummyLayer->emobLayer);
	printf("Default hole mobility = %10.3e cm2/V-s\n",pdummyLayer->hmobLayer);
	printf("Default electron recombination time = %10.3e s\n",pdummyLayer->taunLayer);
	printf("Default hole recombination time = %10.3e s\n",pdummyLayer->taupLayer);
	printf("Default absorption coefficient = %10.3e\n",pdummyLayer->alphaLayer);
	
	free(pdummyLayer);
	
	return;
}  


void CSchrPois::listmaterials(struct global_type *pGlobals, struct flag_type *pFlags) {
/*************************************/
	  
/*	this subroutine lists the materials contained in the materials file */
	
	char message[MAX_MESSAGE];
	struct mtrl_list *pmaterial;
	
	pmaterial = pGlobals->pTopMtrl;

/*  First, see if the materials file exists */
	if (pGlobals->pTopMtrl == NULL) {
		strcpy(message,"Materials File Not Found");
		alertbox (message);
		return;
	}
	  	
	
	printf("\n");	
	printf("The materials defined in the materials file are:\n");
	
	do {
		printf("%s\t%s\t%s\t%s\n",pmaterial->name,pmaterial->type,pmaterial->family,pmaterial->comment);
	} while ((pmaterial = pmaterial->pNext_mtrl) != NULL);

	return;
}