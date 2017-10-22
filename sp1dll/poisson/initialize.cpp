#include "stdafx.h"
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

#include "poisp_types.h"
#include "poisp_defs.h"
#include "..\schrpois.h"

int CSchrPois::initialize(struct global_type *pGlobals, struct flag_type *pFlags) {

/*  This routine initializes the constants used in the program and finds an  
    initial solution of the poisson equation which is used to start the  
    Newton-Raphson interation */

	int k,ioerr_loc;
	int numMeshPts;
	struct layer_type *pLayer;
	struct dummy_layer_type *pDefaultLayer;
	struct mesh_type *pMesh;


    /* initialize constants */
	pGlobals->kT = 8.61733e-5*pGlobals->temperature;
	pGlobals->y=0;  
    	
/*  The physical parameters of the structure at each point are held in storage  
    vectors, so that a calculation involving the point i will use the information  
    stored in location i of the storage vector.  For example, the conduction  
    band at the point i is given by v(i) (the solution of the poisson eq.)  
    plus offset(i) (the quasipotential at that point).  */ 

	PostInfoSP(_T("initialize: Initializing mesh (meshInit)"));
	
	pDefaultLayer = (struct dummy_layer_type *) malloc(sizeof(struct dummy_layer_type)); /* allocate default layer information */
	pGlobals->pTopMesh = (struct mesh_type *) malloc(sizeof(struct mesh_type));	/* allocate first mesh point */
	pGlobals->pTopMesh->pUpMesh = NULL;
	pGlobals->pTopMesh->pDownMesh = NULL;
	pMesh = pGlobals->pTopMesh;		/* initialize mesh pointer */
	pLayer = pGlobals->pTopLayer;	/* initialize layer pointer */

	k=-1;		// mesh point counter
	numMeshPts = 0;
  
	ioerr_loc = meshInit(&pMesh, pLayer, pDefaultLayer, pGlobals, pFlags, &numMeshPts);
	if(ioerr_loc)	return 1;
	/* Mesh initializiation done */

	k = numMeshPts+1;

	/* Add in implanted dopants.  Add these to dopants already in the structure. */
	PostInfoSP(_T("initialize: Adding implanted dopants (meshInitAddImplants)"));
	if (pFlags->implant) {
		ioerr_loc = meshInitAddImplants(pMesh, pGlobals, pFlags);
	}

	/*  if both the pFlags->schrodinger start and stop points have been defined,  
     convert start and stop to mesh pointers, and set the pFlags->schrodinger flag */
	PostInfoSP(_T("initialize: Adding Schrodinger region (meshInitAddSchr)"));
	if (pFlags->schstartflag && pFlags->schstopflag) {
		ioerr_loc = meshInitAddSchr(pMesh,  pLayer, &pGlobals, pFlags);
	}

	pGlobals->pStartSim = pGlobals->pTopMesh;
	
	/* 	 set surface boundary condition */
	PostInfoSP(_T("initialize: Setting surface boundary condition (meshInitAddSurfaceCondition)"));
	ioerr_loc = meshInitAddSurfaceCondition(&pGlobals, pFlags);
	
	PostInfoSP(_T("initialize: Setting substrate boundary condition (meshInitAddSubstrateCondition)"));
	/* 	 set substrate boundary condition */
	ioerr_loc = meshInitAddSubstrateCondition(&pGlobals, pFlags);

	/* fill in params */
	if (pFlags->readParam) {
		/*readParamFile();*/
		if (pFlags->stopthemusic) return 1;	
	}

	free(pDefaultLayer);
	pFlags->noinputfile = false;		/* declare that there is a valid input file */

	return 0;
}

int CSchrPois::meshInit(struct mesh_type **ppMesh,  struct layer_type *pLayer, 
				struct dummy_layer_type *pDefaultLayer, struct global_type *pGlobals, 
				struct flag_type *pFlags, int *numMeshPts)
{
	/* This function initilazes a mesh and fills in the info for each mesh point.*/
	
	double yincrement;
	int firstmesh,k,i,ioerr_loc;
	char message[MAX_MESSAGE];
	char simfamily[MAX_SEMINAME];
	
	struct mesh_type *pMesh = *ppMesh;
	
	PostInfoSP(_T("meshInit: setting up layer's mesh"));	
	
	k=-1;		// mesh point counter
	pGlobals->y=0.0;
	firstmesh = true;
	pFlags->firstlayer = true;	
	do {		
		pGlobals->y_loc = 0.0;
		yincrement = 1.0/((int) (pLayer->tLayer/pLayer->dyLayer+0.5e0));
			
		/* initalize the mesh points */	
		for (i=0;i<((int) (pLayer->tLayer/pLayer->dyLayer+0.5e0)); i++) {	
			if (!firstmesh) {	
				pMesh->pDownMesh = (struct mesh_type *) malloc(sizeof(struct mesh_type));
				if (pMesh->pDownMesh == NULL) {
					strcpy(message,"meshInit: Can't allocate memory for mesh array.  Increase Memory allocation.");
					alertboxWin(message);
					pFlags->stopthemusic = true;
					return 1;
				}
				pMesh->pDownMesh->pDownMesh = NULL;
				pMesh->pDownMesh->pUpMesh = pMesh;
				pMesh = pMesh->pDownMesh;
			}
			else
				firstmesh = false;
			
			/* Now assign values to the parameters at this mesh point.  The basic plan is to first calculate the 
			semiconductor composition, then retrieve data from the materials file.  If a parameter has been overridden
			in the input file, this value is used and the data from the materials file is ignored, for that parameter only. */
			
			pGlobals->xval = 999.0; /* default value for unused xval */
			pGlobals->yval = 999.0; /* default value for unused yval */
			pGlobals->zval = 999.0; /* default value for unused zval */
			pGlobals->wval = 999.0; /* default value for unused wval */
			if (pLayer->xvalLayer[0] != '\0') pGlobals->xval = evaluate(pLayer->xvalLayer, pGlobals);
			if (pLayer->yvalLayer[0] != '\0') pGlobals->yval = evaluate(pLayer->yvalLayer, pGlobals);
			if (pLayer->zvalLayer[0] != '\0') pGlobals->zval = evaluate(pLayer->zvalLayer, pGlobals);
			if (pLayer->wvalLayer[0] != '\0') pGlobals->wval = evaluate(pLayer->wvalLayer, pGlobals);
			ioerr_loc = semisearch(pLayer->nameLayer,pGlobals->xval,pGlobals->yval,pGlobals->zval,pGlobals->wval,
									pDefaultLayer,simfamily,pGlobals,pFlags);	/* get information from materials file*/
		    if (pFlags->badversion) return 0;
		    
		    ioerr_loc = meshPointInit(pMesh,pLayer,pDefaultLayer,pGlobals); /* load up most of the mesh point parameters */
		    
			pMesh->meshcoord = pGlobals->y;		/* init real space coordinate */
			pGlobals->y = pGlobals->y + pLayer->dyLayer;
	    	k= k+1;
	    	pGlobals->y_loc = pGlobals->y_loc + yincrement;
		}
		if (pLayer->sheet < 0.0) 
			pMesh->Na = pMesh->Na - pLayer->sheet/pMesh->dy*1e8;
		else
			pMesh->Nd = pMesh->Nd + pLayer->sheet/pMesh->dy*1e8;
				
	} while((pLayer=pLayer->pDownLayer) != NULL);
	
	/* Add another mesh point to be the bottom mesh */
	pLayer = pMesh->pUpMesh->pLayer;	
	pMesh->pDownMesh = (struct mesh_type *) malloc(sizeof(struct mesh_type));
	if (pMesh->pDownMesh == NULL) {
		strcpy(message,"meshInit: Can't allocate memory for mesh array.  Increase Memory allocation.");
		alertboxWin(message);
		pFlags->stopthemusic = true;
		return 1;
	}
	pMesh->pDownMesh->pDownMesh = NULL;
	pMesh->pDownMesh->pUpMesh = pMesh;
	pMesh = pMesh->pDownMesh;
		    
	ioerr_loc = meshPointInit(pMesh,pLayer,pDefaultLayer,pGlobals); /* load up most of the mesh point parameters */

	pMesh->meshcoord = pGlobals->y;		/* init real space coordinate */
	pGlobals->y = pGlobals->y + pLayer->dyLayer;
	pGlobals->pBottomMesh = pMesh;
					
	PostInfoSP(_T("meshInit: Mesh initialization done"));
	
	*numMeshPts = k;
	
	*ppMesh = pMesh;
	
	return 0;
}

int CSchrPois::meshPointInit (struct mesh_type *pMesh, struct layer_type *pLayer, 
				struct dummy_layer_type *pDefaultLayer, struct global_type *pGlobals) {

/************************************************************/	

	double doping,tem;
	
	pMesh->dy = pLayer->dyLayer;
	if (strcmp(pLayer->EgLayer,""))
		pMesh->Eg = evaluate(pLayer->EgLayer, pGlobals);
	else
		pMesh->Eg = pDefaultLayer->EgLayer;
	if (strcmp(pLayer->offsetLayer,""))
		pMesh->offset = evaluate(pLayer->offsetLayer, pGlobals);
	else
		pMesh->offset = pDefaultLayer->offsetLayer;
	if (strcmp(pLayer->eemLayer,""))
		pMesh->eem = evaluate(pLayer->eemLayer, pGlobals);
	else
		pMesh->eem = pDefaultLayer->eemLayer;
	if (strcmp(pLayer->valleyLayer,""))
		pMesh->valley = evaluate(pLayer->valleyLayer, pGlobals);
	else
		pMesh->valley = pDefaultLayer->valleyLayer;
	if (strcmp(pLayer->erLayer,""))
		pMesh->esemi = evaluate(pLayer->erLayer, pGlobals)*dicon;		/* initialize dielectric const. array */
	else
		pMesh->esemi = pDefaultLayer->erLayer*dicon;
	if (strcmp(pLayer->mhhLayer,""))
		pMesh->ehm = evaluate(pLayer->mhhLayer, pGlobals);
	else
		pMesh->ehm = pDefaultLayer->mhhLayer;
	if (strcmp(pLayer->mlhLayer,""))
		pMesh->elhm = evaluate(pLayer->mlhLayer, pGlobals);
	else
		pMesh->elhm = pDefaultLayer->mlhLayer;
	if (strcmp(pLayer->NdLayer,""))
		pMesh->Nd = evaluate(pLayer->NdLayer, pGlobals);
	else
		pMesh->Nd = pDefaultLayer->NdLayer;
	if (strcmp(pLayer->NaLayer,""))
		pMesh->Na = evaluate(pLayer->NaLayer, pGlobals);
	else
		pMesh->Na = pDefaultLayer->NaLayer;
	if (strcmp(pLayer->NdsLayer,""))
		pMesh->Nds = evaluate(pLayer->NdsLayer, pGlobals);
	else
		pMesh->Nds = pDefaultLayer->NdsLayer;
	if (strcmp(pLayer->NasLayer,""))
		pMesh->Nas = evaluate(pLayer->NasLayer, pGlobals);
	else
		pMesh->Nas = pDefaultLayer->NasLayer;
	if (strcmp(pLayer->NddLayer,""))
		pMesh->Ndd = evaluate(pLayer->NddLayer, pGlobals);
	else
		pMesh->Ndd = pDefaultLayer->NddLayer;
	if (strcmp(pLayer->NdaLayer,""))
		pMesh->Nda = evaluate(pLayer->NdaLayer, pGlobals);
	else
		pMesh->Nda = pDefaultLayer->NdaLayer;
	if (strcmp(pLayer->polarizLayer,""))
		pMesh->polariz = evaluate(pLayer->polarizLayer, pGlobals);
	else
		pMesh->polariz = pDefaultLayer->polarizLayer;
	if (pMesh->pUpMesh != NULL) 
		pMesh->polcharge = (pMesh->pUpMesh->polariz - pMesh->polariz)/(q*pMesh->dy*1.0e-8); /* Polarization charge density */
	else
		pMesh->polcharge = 0.0;
	if (strcmp(pLayer->EdLayer,""))
		pMesh->Ed = evaluate(pLayer->EdLayer, pGlobals);
	else
		pMesh->Ed = pDefaultLayer->EdLayer;
	if (strcmp(pLayer->EaLayer,""))
		pMesh->Ea = evaluate(pLayer->EaLayer, pGlobals);
	else
		pMesh->Ea = pDefaultLayer->EaLayer;
	if (strcmp(pLayer->EddLayer,""))
		pMesh->Edd = evaluate(pLayer->EddLayer, pGlobals);
	else
		pMesh->Edd = pDefaultLayer->EddLayer;
	if (strcmp(pLayer->EdaLayer,""))
		pMesh->Eda = evaluate(pLayer->EdaLayer, pGlobals);
	else
		pMesh->Eda = pDefaultLayer->EdaLayer;
	if (strcmp(pLayer->emobLayer,""))
		pMesh->emob = evaluate(pLayer->emobLayer, pGlobals);
	else
		pMesh->emob = pDefaultLayer->emobLayer;
	if (strcmp(pLayer->hmobLayer,""))
		pMesh->hmob = evaluate(pLayer->hmobLayer, pGlobals);
	else
		pMesh->hmob = pDefaultLayer->hmobLayer;
	if (strcmp(pLayer->EfLayer,""))
		pMesh->Efn = evaluate(pLayer->EfLayer, pGlobals);
	else
		pMesh->Efn = pDefaultLayer->EfLayer;
	pMesh->Efp = pMesh->Efn;
	pMesh->pLayer = pLayer;			/* initialize layer pointer */
	pMesh->noholes = pLayer->noholesLayer;
	pMesh->noelec = pLayer->noelecLayer;
	pMesh->fullyionized = pLayer->fullyionLayer;
	pMesh->aBohrElec = 59758.87*pMesh->esemi/pMesh->eem;	/* electron Bohr radius (cm) */
	pMesh->aBohrHhole = 59758.87*pMesh->esemi/pMesh->ehm;	/* heavy hole Bohr radius (cm) */
	pMesh->aBohrLhole = 59758.87*pMesh->esemi/pMesh->elhm;	/* light hole Bohr radius (cm) */
	pMesh->aBohrHhole=0.5e0*(pMesh->aBohrHhole+pMesh->aBohrLhole);	/* for now average the hole Bohr radii */
	if (pGlobals->kT > 0.0e0) {
		pMesh->Nc = 6.038165e21*pMesh->valley*pow(pMesh->eem*pGlobals->kT,1.5);	/* conduction band eff. den. st. */
		pMesh->Nv = 6.038165e21*(pow(pMesh->ehm,1.5) + pow(pMesh->elhm,1.5))*pow(pGlobals->kT,1.5);	/* valence band eff. den. st. */
	}
	else {
		pMesh->Nc = 6.813341e21*pMesh->valley*pow(pMesh->eem,1.5);	/* conduction band eff. den. st. */
		pMesh->Nv = 6.813341e21*(pow(pMesh->ehm,1.5) + pow(pMesh->elhm,1.5));	/* valence band eff. den. st. */
	}  
	/* find the initial flat band condition for each mesh point */	
	doping = pMesh->Nd - pMesh->Na;
		if (doping > 0.0) {
			if (pGlobals->kT > 1.0e-10) {
				tem = doping/(6.038165e21*pMesh->valley*pow(pMesh->eem*pGlobals->kT,1.5));
				pMesh->v = -pMesh->offset - pGlobals->kT*(log(tem)+0.3535533391*tem);
			}
			else
				pMesh->v = -pMesh->offset;
		}
		else if (doping < 0.0) {
			if (pGlobals->kT > 1.0e-10) {
				tem = -doping/pMesh->Nv;
				pMesh->v = pMesh->Eg-pMesh->offset + pGlobals->kT*(log(tem)+0.3535533391*tem);
			}
			else
				pMesh->v = pMesh->Eg - pMesh->offset;
		}
		else
			pMesh->v = pMesh->Eg/2.0-pMesh->offset;
	
	return 0;
}
	
int CSchrPois::meshInitAddSchr(struct mesh_type *pMesh,  struct layer_type *pLayer, 
				struct global_type **ppGlobals, struct flag_type *pFlags)
{
	double tem = 0;
	struct global_type *pGlobals = *ppGlobals;
	char message[MAX_MESSAGE];

	
	PostInfoSP(_T("meshInitAddSchr: Converting SchStart and SchStop to mesh points"));

	pGlobals->pSchStart = ymesh(pGlobals->realschstart,pGlobals);
	pGlobals->pSchStop = ymesh(pGlobals->realschstop,pGlobals);

	/* Find the pGlobals->order of the Schrodinger matrices*/
	pGlobals->order =1;
	pMesh = pGlobals->pSchStart;
	while ((pMesh = pMesh->pDownMesh) != pGlobals->pSchStop) 
		pGlobals->order = pGlobals->order +1;
	pGlobals->order = pGlobals->order + 1;

	if (pGlobals->order > 0) { 
		pFlags->schrodinger = true;
		pGlobals->pPotential = (double *) malloc(pGlobals->order*sizeof(double));
		if (pGlobals->pPotential == NULL) {
			strcpy(message,"meshInitAddSchr: Can't allocate memory for Schrodinger array.  Increase Memory allocation.");
			alertboxWin(message);
			pFlags->stopthemusic = true;
			return 1;
		}
		pGlobals->pMass = (double *) malloc(pGlobals->order*sizeof(double));
		if (pGlobals->pMass == NULL) {
			strcpy(message,"meshInitAddSchr: Can't allocate memory for Schrodinger array.  Increase Memory allocation.");
			alertboxWin(message);
			pFlags->stopthemusic = true;
			return 1;
		}
		pGlobals->pSdy = (double *) malloc(pGlobals->order*sizeof(double));
		if (pGlobals->pSdy == NULL) {
			strcpy(message,"meshInitAddSchr: Can't allocate memory for Schrodinger array.  Increase Memory allocation.");
			alertboxWin(message);
			pFlags->stopthemusic = true;
			return 1;
		}
		pGlobals->pEval = (double *) malloc(pGlobals->order*sizeof(double));
		if (pGlobals->pEval == NULL) {
			strcpy(message,"meshInitAddSchr: Can't allocate memory for Schrodinger array.  Increase Memory allocation.");
			alertboxWin(message);
			pFlags->stopthemusic = true;
			return 1;
		}
		pGlobals->pEvcharge = (double *) malloc(pGlobals->order*sizeof(double));
		if (pGlobals->pEvcharge == NULL) {
			strcpy(message,"meshInitAddSchr: Can't allocate memory for Schrodinger array.  Increase Memory allocation.");
			alertboxWin(message);
			pFlags->stopthemusic = true;
			return 1;
		}
		pGlobals->pEvderiv = (double *) malloc(pGlobals->order*sizeof(double));
		if (pGlobals->pEvderiv == NULL) {
			strcpy(message,"meshInitAddSchr: Can't allocate memory for Schrodinger array.  Increase Memory allocation.");
			alertboxWin(message);
			pFlags->stopthemusic = true;
			return 1;
		}
		pFlags->schMemAlloc = true; /* Schrodinger memory has been allocated */
	}
	else {
		strcpy(message,"meshInitAddSchr: Schrodinger interval must be at least one mesh point.");
		alertboxWin(message);
		pFlags->stopthemusic = true;
		return 1;
	}

	/* Check for a flat Fermi Level in the Schrodinger region */
	tem = 1.e-5*((int)(1.e5*pGlobals->pSchStart->Efn+0.5e0));
	pMesh = pGlobals->pSchStart;
	while ((pMesh = pMesh->pDownMesh) != pGlobals->pSchStop) {
		if (1.e-5*((int) (1.e5*pMesh->Efn+0.5e0)) != tem) {
			strcpy(message,"meshInitAddSchr: The Fermi level must be constant in the Schrodinger region");
			alertboxWin(message);
			pFlags->stopthemusic = true;
			return 1;
		}
	}
	
	return 0;
}


int CSchrPois::meshInitAddSurfaceCondition(struct global_type **ppGlobals, 
				struct flag_type *pFlags)
{
	struct global_type *pGlobals = *ppGlobals;
	char message[MAX_MESSAGE];
	
	PostInfoSP(_T("meshInitAddSurfaceCondition: Setting surface boundary conditions"));
	if (!strncmp(pGlobals->surfcontact,"schottky",8)) {
		if (pFlags->usesurfbar) { 
			pGlobals->pTopMesh->v = pGlobals->surfbar-pGlobals->vbias[pGlobals->surfvoltage]-pGlobals->pTopMesh->offset;	/* surface voltage */
			pGlobals->pTopMesh->Efn = -pGlobals->vbias[pGlobals->surfvoltage];
		}
	}  	
	else if (!strncmp(pGlobals->surfcontact,"ohmic",5)) {
		pGlobals->pTopMesh->v = pGlobals->pTopMesh->v - pGlobals->vbias[pGlobals->surfvoltage];			/* ohmic contact */
		pGlobals->pTopMesh->Efn = -pGlobals->vbias[pGlobals->surfvoltage];
		if (pFlags->multiruns) {
			pFlags->multiruns = false;		/* only 0V bias allowed for ohmic contact */
			strcpy(message,"meshInitAddSurfaceCondition: Surface ohmic contact not allowed with multirun");
			alertboxWin(message);
		}  
		if (fabs(pGlobals->pTopMesh->Efn - pGlobals->pTopMesh->pDownMesh->Efn) > 1e-3) {
			pGlobals->vbias[pGlobals->surfvoltage] = 0.0e0;
			strcpy(message,"meshInitAddSurfaceCondition: Surface ohmic contact bias not equal to Ef in semiconductor");
			alertboxWin(message);
		}
	} 	
	else if (!strncmp(pGlobals->surfcontact,"slope",5)) {
	  /* create the dummy mesh point used to set the slope equal to zero */	
		pGlobals->pTopMesh->pUpMesh = (struct mesh_type *) malloc(sizeof(struct mesh_type));
		if (pGlobals->pTopMesh->pUpMesh == NULL) {
			strcpy(message,"meshInitAddSurfaceCondition: Can't allocate memory for mesh array.  Increase Memory allocation.");
			alertboxWin(message);
			pFlags->stopthemusic = true;
			return 1;
		}
		pGlobals->pTopMesh->pUpMesh->pUpMesh = NULL;
		pGlobals->pTopMesh->pUpMesh->dy = pGlobals->pTopMesh->dy;
		pGlobals->pTopMesh->pUpMesh->esemi = pGlobals->pTopMesh->esemi;
	}
	
	*ppGlobals = pGlobals;
	
	return 0;
}

int CSchrPois::meshInitAddSubstrateCondition(struct global_type **ppGlobals, 
				struct flag_type *pFlags)
{
	char message[MAX_MESSAGE];

	PostInfoSP(_T("meshInitAddSubstrateCondition: Setting substrate boundary conditions"));
	if (!strncmp(pGlobals->subcontact,"schottky",8)) {
		if (pFlags->usesubbar) {
			pGlobals->pBottomMesh->v = pGlobals->subbar-pGlobals->vbias[pGlobals->subvoltage]-pGlobals->pBottomMesh->offset;	/*substrate voltage */
			pGlobals->pBottomMesh->Efn = -pGlobals->vbias[pGlobals->subvoltage];
		}
	}
	else if (!strncmp(pGlobals->subcontact,"ohmic",5)) {
		pGlobals->pBottomMesh->v = pGlobals->pBottomMesh->v - pGlobals->vbias[pGlobals->subvoltage];		/* ohmic contact */
		pGlobals->pBottomMesh->Efn = -pGlobals->vbias[pGlobals->subvoltage];
		if (fabs(pGlobals->pBottomMesh->Efn+pGlobals->vbias[pGlobals->subvoltage]) > 1e-3) {
			pGlobals->vbias[pGlobals->subvoltage] = 0.0e0;
			strcpy(message,"meshInitAddSubstrateCondition: Surface ohmic contact bias not equal to Ef in semiconductor");
			alertboxWin(message);
		}
	} 	
	else if (!strncmp(pGlobals->subcontact,"slope",5)) {
	  /* create the dummy mesh point used to set the slope equal to zero */	
		pGlobals->pBottomMesh->pDownMesh = (struct mesh_type *) malloc(sizeof(struct mesh_type));
		if (pGlobals->pBottomMesh->pDownMesh == NULL) {
			strcpy(message,"meshInitAddSubstrateCondition: Can't allocate memory for mesh array.  Increase Memory allocation.");
			alertboxWin(message);
			pFlags->stopthemusic = true;
			return 1;
		}
		pGlobals->pBottomMesh->pDownMesh->pDownMesh = NULL;
		pGlobals->pBottomMesh->pDownMesh->pUpMesh = pGlobals->pBottomMesh;
		pGlobals->pBottomMesh->pDownMesh->dy = pGlobals->pBottomMesh->dy;
		pGlobals->pBottomMesh->pDownMesh->esemi = pGlobals->pBottomMesh->esemi;
	}
	
	return 0;
}

int CSchrPois::meshInitAddImplants(struct mesh_type *pMesh, 
				struct global_type *pGlobals, struct flag_type *pFlags)
{
	int j;
	for (j=0; j<pGlobals->impnumber; j++) {
		if (pGlobals->imptype[j] == 'd') {
			pMesh = pGlobals->pTopMesh;
			do {
				pGlobals->y = pMesh->meshcoord;
				pMesh->Nd = pMesh->Nd + pGlobals->dose[j]*exp(-0.5e0*((pGlobals->y-pGlobals->Rp[j])/(pGlobals->dRp[j]*pGlobals->dRp[j])));
			} while((pMesh=pMesh->pDownMesh) != NULL);
		}
		else if (pGlobals->imptype[j] == 'a') {
			pMesh = pGlobals->pTopMesh;
			do {
				pGlobals->y = pMesh->meshcoord;
				pMesh->Na = pMesh->Na + pGlobals->dose[j]*exp(-0.5e0*((pGlobals->y-pGlobals->Rp[j])/(pGlobals->dRp[j]*pGlobals->dRp[j])));
			} while((pMesh=pMesh->pDownMesh) != NULL);
		}
	}
	
	return 0;
}