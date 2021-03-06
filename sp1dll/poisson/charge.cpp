#include "stdafx.h"

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

#include "poisp_types.h"

#include "..\schrpois.h"


void CSchrPois::charge(struct global_type *pGlobals, struct flag_type *pFlags) {
  
/*  This routine finds the electron and hole concentration at */  
/*  each mesh point using three dimensional density of states */

	struct mesh_type *pMesh;

	pMesh=pGlobals->pStartMesh;
	if (pFlags->schex) {
		if (pMesh != pGlobals->pSchStart && pMesh->pUpMesh != pGlobals->pTopMesh) {
			do { 
				mobilecharge(pMesh,pGlobals->kT);
				fixedcharge(pMesh,pGlobals->kT);
			} while ((pMesh = pMesh->pDownMesh) != pGlobals->pSchStart);
		}
		schrodingersolve(pGlobals,pFlags);
		do { /* Find fixed charge concentration in Schrodinger mesh */
			fixedcharge(pMesh,pGlobals->kT);	
		} while ((pMesh = pMesh->pDownMesh) != pGlobals->pSchStop->pDownMesh);
		if (pMesh->pDownMesh != NULL) {
			do { 
				mobilecharge(pMesh,pGlobals->kT);
				fixedcharge(pMesh,pGlobals->kT);
			} while ((pMesh = pMesh->pDownMesh) != pGlobals->pStopMesh->pDownMesh);
		}
	}
	else {
		do { 
			mobilecharge(pMesh,pGlobals->kT);
			fixedcharge(pMesh,pGlobals->kT);
		} while ((pMesh = pMesh->pDownMesh) != pGlobals->pStopMesh->pDownMesh);
	}

	return;
}



void CSchrPois::mobilecharge(struct mesh_type *pMesh, double kT) {
  
/* calculate the charge density in the semiconductor */  

  
/* Electron concentration */ 

	pMesh->n = electrons(pMesh,kT);
	pMesh->deriv = electronderiv(pMesh,kT);  
  
  
/* Hole concentration  */

	pMesh->p = holes(pMesh,kT);
	pMesh->deriv = pMesh->deriv + holederiv(pMesh,kT);

	return;
}


double CSchrPois::electrons(struct mesh_type *pMesh, double kT)  {
	
/***********************************************/

	double eta,electronMesh,tem;  

	if (pMesh->noelec) 
		electronMesh = 1.0e-10;
	else if (kT < 1.0e-10) {  
		/* Allow for T=0 in simulations */
		eta = pMesh->Efn-(pMesh->v+pMesh->offset);
		if (eta > 0.0e0) 
			electronMesh = (2.0e0*pMesh->Nc*pow(eta,1.5))/3.0e0;
		else {
			electronMesh =1.0e-10;
			pMesh->deriv = 0.0;
		}
	}
	else {

		eta = (pMesh->Efn-pMesh->v-pMesh->offset)/kT;  
  
/* calculate electron concentration */ 

		if (eta < -4.0e0) {
			if (eta < -150.0e0) 
				electronMesh =1.0e-10;
			else  
				/* nondegenerate, Boltzmann */
				electronMesh = pMesh->Nc*exp(eta);
		}
		else if (eta < 6.577095e0) {   
			/* Unger approx. */
			tem = log(1.0e0 + exp(eta)); 
			electronMesh = pMesh->Nc*tem*(1.0e0 + 0.15e0*tem);
		}
		else  
			/* Sommerfeld approx. */
			electronMesh = pMesh->Nc*0.75225e0*pow(eta,1.5)*(1.0e0+1.3e0/(eta*eta));
	}
	return electronMesh;
}


double CSchrPois::electronderiv(struct mesh_type *pMesh, double kT) {  
	

	double eta,electronderivMesh,tem;

	if (pMesh->noelec) 
		electronderivMesh = 0.0e0;
	else if (kT < 1.0e-10) {  
		/* Allow for T=0 in simulations */
		eta = pMesh->Efn-(pMesh->v+pMesh->offset);
		if (eta > 0.0e0) 
			electronderivMesh = pMesh->Nc*sqrt(eta);
		else
			electronderivMesh = 0.0e0;
	}
	else {

		eta = (pMesh->Efn-pMesh->v-pMesh->offset)/kT; 
  
		/* calculate derivative of the electron concentration */ 

		if (eta < -4.0e0) {
			if (eta < -150.0e0) 
				electronderivMesh = 0.0;
	    	else  
				/* nondegenerate, Boltzmann */
				electronderivMesh = pMesh->Nc*exp(eta)/kT;
		}
		else if (eta < 6.577095e0) {   
			/* Unger approx. */
			tem = log(1.0e0 + exp(eta));
			electronderivMesh = pMesh->Nc*(1.0e0 + 0.30e0*tem)/(kT*(1.0e0+exp(-eta)));
		}
		else  
			/* Sommerfeld approx. */
			electronderivMesh = -pMesh->Nc*0.75225e0*sqrt(eta)/kT*(0.65e0/(eta*eta)-1.5e0);
	}
	return electronderivMesh;
}



double CSchrPois::holes(struct mesh_type *pMesh, double kT) { 

/***********************************************/

	double eta, holesMesh,tem;

	if (pMesh->noholes) 
		holesMesh = 1.0e-10;

	else if (kT < 1.0e-10) {  
		/* Allow for T=0 in simulations */
		eta = pMesh->v+pMesh->offset-pMesh->Eg-pMesh->Efp;
		if (eta > 0.0e0) 
			holesMesh = (2.0e0*pMesh->Nv * pow(eta,1.5))/3.0e0;
		else
			holesMesh =1.0e-10;
	}
	else {

		eta = (pMesh->v+pMesh->offset-pMesh->Eg-pMesh->Efp)/kT;  

		if (eta < -4.0) {
			if (eta < -150) 
				holesMesh = 1.0e-10;
			else  
				/* nondegenerate */
				holesMesh = pMesh->Nv*exp(eta);
		}
		else if (eta < 6.577095e0) {   
			/* Unger approx. */
			tem = log(1.0e0 + exp(eta));
			holesMesh = pMesh->Nv*tem*(1.0e0 + 0.15e0*tem);
		}
		else  
			/* Sommerfeld approx. */
			holesMesh = pMesh->Nv*0.75225e0*pow(eta,1.5)*(1.0e0+1.3e0/(eta*eta));
	}
	return holesMesh;
}




double CSchrPois::holederiv(struct mesh_type *pMesh, double kT) {  


	double eta, holederivMesh,tem;

	if (pMesh->noholes) 
		holederivMesh = 0.0e0;

	else if (kT < 1.0e-10) {  
		/* Allow for T=0 in simulations */
		eta = pMesh->v+pMesh->offset-pMesh->Eg-pMesh->Efp;
		if (eta > 0.0e0)
			holederivMesh = pMesh->deriv + pMesh->Nv*sqrt(eta);
		else
			holederivMesh = 0.0e0;
	}
	else {

		eta = (pMesh->v + pMesh->offset - pMesh->Eg - pMesh->Efp)/kT;  

		if (eta < -4.0) {
			if (eta < -150.0) 
				holederivMesh = 0.0e0;
			else  
				/* nondegenerate */
				holederivMesh = pMesh->Nv*exp(eta)/kT;
		}
		else if (eta < 6.577095e0) {   
			/* Unger approx. */
			tem = log(1.0e0 + exp(eta));
			holederivMesh = pMesh->Nv*(1.0e0 + 0.30e0*tem)
				/(kT*(1.0e0+exp(-eta)));
		}
		else  
			/* Sommerfeld approx. */
			holederivMesh = -pMesh->Nv*0.75225e0*sqrt(eta)
				/kT*(0.65e0/(eta*eta) -1.5e0);
	}
	return holederivMesh;
}



void CSchrPois::fixedcharge(struct mesh_type *pMesh, double kT) { 
	
/***********************************************/

	double Ndplus;	/* ionized donor concentration */
	double Naminus;	/* ionized acceptor concentration */
	/*double Ndplus300;	/* ionized donor concentration at 300k */
	/*double Naminus300;	/* ionized acceptor concentration at 300k */
	double deltaed,fkt,deltaed1,deriv1,deriv2,tem2,tem3,tem4,tem;

	deltaed = 2.0*kT;
	deltaed1 = 10.0*kT;
	fkt = .025852;

	/*  Find the fixed charge */
	/* Start with the polarization charge (if any) */
	pMesh->fcharge = pMesh->polcharge;

	/* Now do donors and acceptors */	
	if (pMesh->fullyionized) 
	   pMesh->fcharge = pMesh->fcharge + pMesh->Nd - pMesh->Na;	/*fully ionized charge */
	else if (kT < 1.0e-10) {	/* T=0 Fixed charge calculation */
		deltaed = 1.0e-3;
		deltaed1 = 10.0e-3;
		tem = pMesh->Ed-pMesh->v-pMesh->offset+pMesh->Efn;
		if (tem > 0) 
			Ndplus = 0.0e0;
		else {
			Ndplus = pMesh->Nd;
			tem =  exp((pMesh->Ed-pMesh->v-pMesh->offset)/fkt);
			pMesh->deriv = pMesh->deriv - pMesh->Nd/deltaed*
				(1.0e0/(2.0e0+exp(deltaed1/fkt)/tem)
				-1.0e0/(2.0e0+exp(-deltaed1/fkt)/tem));
		}

		tem = (pMesh->v + pMesh->offset - pMesh->Eg + pMesh->Ea - pMesh->Efp);
		if (tem > 0) 
			Naminus = 0.0e0;
		else {
			Naminus = pMesh->Na;
			tem = exp((pMesh->v + pMesh->offset - pMesh->Eg + pMesh->Ea)/fkt);
			pMesh->deriv = pMesh->deriv + pMesh->Na/deltaed*
				(1.0e0/(2.0e0+exp(-deltaed1/fkt)/tem)
				-1.0e0/(2.0e0+exp(deltaed1/fkt)/tem));
		}
		pMesh->fcharge = pMesh->fcharge + Ndplus - Naminus;
	}
	else {
		tem = (pMesh->Ed - pMesh->v - pMesh->offset + pMesh->Efn)/kT;
		if (tem < 400.0e0) {
			tem = exp(tem);
			Ndplus = pMesh->Nd/(1.0e0+2.0e0*tem);
			if (kT > 6.0e-3) 	/* at very low temps modify deriv */
				pMesh->deriv = pMesh->deriv + 2.0e0*Ndplus*tem/((1.0e0
					+2.0e0*tem)*kT);
			else {
				Ndplus = pMesh->Nd*(1.0e0+kT/(2*deltaed)*
					log((1.0e0+2.0e0*tem*exp(-deltaed/kT))
					/(1.0e0+2.0e0*tem*exp(deltaed/kT))));
				deriv1 = pMesh->Nd/deltaed*
					(1.0e0/(2.0e0+exp(deltaed/kT)/tem)-1.0e0
					/(2.0e0+exp(-deltaed/kT)/tem));
				tem = (pMesh->Ed - pMesh->v - pMesh->offset)/fkt;
				tem = exp(tem);
				deriv2 = pMesh->Nd/deltaed*
					(1.0e0/(2.0e0+exp(deltaed1/fkt)/tem)-1.0e0
					/(2.0e0+exp(-deltaed1/fkt)/tem));
				if (deriv1 > deriv2) 
					deriv1 = deriv2; /* set deriv1 to the smaller of deriv1 and deriv2 */
				pMesh->deriv = pMesh->deriv-deriv1;
			}
		}
		else {
			Ndplus = 0.0e0;	/* no contribution to the charge deriv */
		}

		tem = (pMesh->v + pMesh->offset - pMesh->Eg + pMesh->Ea - pMesh->Efp)/kT;
		if (tem < 400.0e0) {
			tem = exp(tem);
			Naminus = pMesh->Na/(1.0e0+4.0e0*tem);
			if (kT > 6.0e-3) 	/* at very low temps modify deriv */
				pMesh->deriv = pMesh->deriv +2.0e0*Naminus*tem
					/((1.0e0+4.0e0*tem)*kT);
			else {
				Naminus = pMesh->Na*(1.0e0-kT/(2*deltaed)*
 					log((1.0e0+2.0e0*tem*exp(deltaed/kT))
					/(1.0e0+2.0e0*tem*exp(-deltaed/kT))));
				deriv1 = pMesh->Na/deltaed*
					(1.0e0/(2.0e0+exp(-deltaed/kT)/tem)-1.0e0
					/(2.0e0+exp(deltaed/kT)/tem));
				tem = (pMesh->v + pMesh->offset - pMesh->Eg + pMesh->Ea)/fkt;
				tem = exp(tem);
				deriv2 = pMesh->Na/deltaed*
					(1.0e0/(2.0e0+exp(-deltaed1/fkt)/tem)-1.0e0
					/(2.0e0+exp(deltaed1/fkt)/tem));
				if (deriv2 > deriv1)
					deriv1 = deriv2; /* set deriv1 to the larger of deriv1 and deriv2 */
				pMesh->deriv = pMesh->deriv + deriv1;
			}
		}
		else {
			Naminus = 0.0e0;	/* no contribution to the charge deriv */
		}

	   pMesh->fcharge =  pMesh->fcharge + Ndplus - Naminus;
	}

/*  Now do the deep level dopants */

	if (kT == 0.0e0) {	/* T=0 Fixed charge calculation */
		deltaed = 1.0e-3;
		deltaed1 = 10.0e-3;
		tem = pMesh->Edd - pMesh->v - pMesh->offset + pMesh->Efn;
		if (tem > 0) 
			Ndplus = 0.0e0;
		else {
			Ndplus = pMesh->Ndd;
			tem =  exp((pMesh->Edd - pMesh->v - pMesh->offset)/fkt);
			pMesh->deriv = pMesh->deriv - pMesh->Ndd/deltaed*
				(1.0e0/(2.0e0+exp(deltaed1/fkt)/tem)
				-1.0e0/(2.0e0+exp(-deltaed1/fkt)/tem));
		}

		tem = pMesh->v + pMesh->offset - pMesh->Eg + pMesh->Eda - pMesh->Efp;
		if (tem > 0) 
			Naminus = 0.0e0;
		else {
			Naminus = pMesh->Nda;
			tem = exp((pMesh->v + pMesh->offset - pMesh->Eg + pMesh->Eda)/fkt);
			pMesh->deriv = pMesh->deriv + pMesh->Nda/deltaed*
				(1.0e0/(2.0e0+exp(-deltaed1/fkt)/tem)
				-1.0e0/(2.0e0+exp(deltaed1/fkt)/tem));
		}
		pMesh->fcharge = pMesh->fcharge + Ndplus - Naminus;
	}
	
	else {
		/*  Deep donors	 at finite temperature */
		
		tem = (pMesh->Edd - pMesh->v - pMesh->offset + pMesh->Efn)/kT;
		if (tem < 400.0e0) {
	    	tem = exp(tem);
			Ndplus = pMesh->Ndd/(1.0e0+2.0e0*tem);
			if (kT > 6.0e-3) 	/* at very low temps modify deriv */
				pMesh->deriv = pMesh->deriv + 2.0e0*Ndplus*tem/((1.0e0+2.0e0*tem)*kT);
			else {
				Ndplus = pMesh->Ndd*(1.0e0+kT/(2*deltaed)*
					log((1.0e0+2.0e0*tem*exp(-deltaed/kT))
					/(1.0e0+2.0e0*tem*exp(deltaed/kT))));
				deriv1 = pMesh->Ndd/deltaed*
					(1.0e0/(2.0e0+exp(deltaed/kT)/tem)-1.0e0
					/(2.0e0+exp(-deltaed/kT)/tem));
				tem = (pMesh->Edd - pMesh->v - pMesh->offset)/fkt;
				tem = exp(tem);
				deriv2 = pMesh->Ndd/deltaed*
					(1.0e0/(2.0e0+exp(deltaed1/fkt)/tem)-1.0e0
					/(2.0e0+exp(-deltaed1/fkt)/tem));
				if (deriv1 > deriv2) 
					deriv1 = deriv2; /* set deriv1 to the smaller of deriv1 and deriv2 */
				pMesh->deriv = pMesh->deriv-deriv1;
			}
		}
		else {
			Ndplus = 0.0e0;	/* no contribution to the charge deriv */
		}

		/*  Deep acceptors  at finite pGlobals->temperature */

		tem = (pMesh->v + pMesh->offset - pMesh->Eg + pMesh->Eda - pMesh->Efn)/kT;
		if (tem < 400.0e0) {
			tem = exp(tem);
			Naminus = pMesh->Nda/(1.0e0+4.0e0*tem);
			if (kT > 6.0e-3) 	/* at very low temps modify deriv */
				pMesh->deriv = pMesh->deriv +2.0e0*Naminus*tem
 					/((1.0e0+4.0e0*tem)*kT);
			else {
				Naminus = pMesh->Nda*(1.0e0-kT/(2*deltaed)*
					log((1.0e0+2.0e0*tem*exp(deltaed/kT))
					/(1.0e0+2.0e0*tem*exp(-deltaed/kT))));
				deriv1 = pMesh->Nda/deltaed*
					(1.0e0/(2.0e0+exp(-deltaed/kT)/tem)-1.0e0
					/(2.0e0+exp(deltaed/kT)/tem));
				tem = (pMesh->v + pMesh->offset - pMesh->Eg + pMesh->Eda)/fkt;
				tem = exp(tem);
				deriv2 = pMesh->Nda/deltaed*
					(1.0e0/(2.0e0+exp(-deltaed1/fkt)/tem)-1.0e0
					/(2.0e0+exp(deltaed1/fkt)/tem));
				if (deriv2 > deriv1)
					deriv1 = deriv2; /* set deriv1 to the larger of deriv1 and deriv2 */
				pMesh->deriv = pMesh->deriv+deriv1;
			}
		}
		else {
			Naminus = 0.0e0;	/* no contribution to the charge deriv */
		}

		pMesh->fcharge = pMesh->fcharge + Ndplus - Naminus;
	}

	  
/*  Now do the sheet donors and acceptors */

	if (kT < 1.0e-10) {		/* charge at T=0 */
		Ndplus = pMesh->Nds*(pMesh->v + pMesh->offset - pMesh->Edst - pMesh->Efn);
		Naminus = pMesh->Nas * (pMesh->Efp - (pMesh->v + pMesh->offset
			-pMesh->Eg + pMesh->Easb));
		pMesh->fcharge = pMesh->fcharge + Ndplus - Naminus;
		pMesh->deriv = pMesh->deriv + pMesh->Nds + pMesh->Nas;
	}

	else {		/* Finite Temperature */

		/*  Donors first */
		tem =(pMesh->v + pMesh->offset - pMesh->Edst - pMesh->Efn)/kT;
		if (tem < 400.0e0) {	/* Check for numerical overflow */
			tem = exp(tem);
			tem3 = tem/(2.0e0+tem);
			tem = log(2.0e0 + tem);
		}
		else {
			tem3 = 1.0e0;
		}
		tem2 =(pMesh->v + pMesh->offset - pMesh->Edsb - pMesh->Efn)/kT;
		if (tem2 < 400.0e0) {	/* Check for numerical overflow */
			tem2 = exp(tem2);
			tem4 = tem2/(2.0e0+tem2);
			tem2 = log(2.0e0 + tem2);
		}
		else {
			tem4 = 1.0e0;
		}
		Ndplus = pMesh->Nds*kT*(tem-tem2);
		pMesh->deriv = pMesh->deriv + pMesh->Nds*(tem3-tem4);

		/*  Now the acceptors  */

		tem =(pMesh->Efp - (pMesh->v + pMesh->offset - pMesh->Eg + pMesh->Easb))/kT;
		if (tem < 400.0e0) {	/* Check for numerical overflow */
			tem = exp(tem);
			tem3 = tem/(4.0e0+tem);
			tem = log(4.0e0 + tem);
		}
		else {
			tem3 = 1.0e0;
		}
		tem2 =(pMesh->Efp -(pMesh->v + pMesh->offset - pMesh->Eg + pMesh->East))/kT;
		if (tem2 < 400.0e0) {	/* Check for numerical overflow */
			tem2 = exp(tem2);
			tem4 = tem2/(4.0e0+tem2);
			tem2 = log(4.0e0 + tem2);
		}
		else {
			tem4 = 1.0e0;
		}
		Naminus = pMesh->Nas*kT*(tem-tem2);
		pMesh->fcharge = pMesh->fcharge + Ndplus - Naminus;
		pMesh->deriv = pMesh->deriv + pMesh->Nas*(tem3-tem4);
	}

	return ;
}


double CSchrPois::cvcharge(struct mesh_type *pLocMesh) {

/***********************************************/
/*  calculate the sheet charge concentrations of the structure  */
/*  by integrating the charge (using the trapazoidal rule). */

	double cvchargeloc;
	
	cvchargeloc = 0.0e0;
	pLocMesh = pLocMesh->pDownMesh;

	do {
		cvchargeloc = cvchargeloc + (pLocMesh->fcharge + pLocMesh->pUpMesh->fcharge
			+pLocMesh->p + pLocMesh->pUpMesh->p - pLocMesh->n - pLocMesh->pUpMesh->n)
			*1.0e-8*(pLocMesh->dy + pLocMesh->pUpMesh->dy)/4.0e0;
	} while((pLocMesh=pLocMesh->pDownMesh) != NULL);
	
	return cvchargeloc;
}


void CSchrPois::sheetcharge(double *nsTotal, double *psTotal, double *sheetres, struct global_type *pGlobals) {
	
/***********************************************/

	struct layer_type *pLocLayer;
	struct mesh_type *pLocMesh;
	double ns,ps;

/*  calculate the sheet concentrations of the layers in the structure */ 
/*  by integrating the mobile charge (using the trapezoidal rule). */

	pLocLayer = pGlobals->pTopLayer;
	pLocMesh = pGlobals->pTopMesh->pDownMesh;
	
	do {
		pLocLayer->nsLayer = 0.0e0;
		pLocLayer->psLayer = 0.0e0;
	} while((pLocLayer=pLocLayer->pDownLayer) != NULL);
	

	*sheetres = 0.0e0;	/* Sheet resistance of the structure */
	*nsTotal =0.0e0;	/* Total electron sheet concentration for structure */
	*psTotal = 0.0e0;	/* Total hole sheet concentration for structure */
	
	do {
		ns = (pLocMesh->n + pLocMesh->pUpMesh->n)
			*1.e-8*(pLocMesh->dy + pLocMesh->pUpMesh->dy)/4.;  /* integrate elec conc. */
		pLocMesh->pLayer->nsLayer = pLocMesh->pLayer->nsLayer + ns;
		*nsTotal = *nsTotal + ns;
		
	    ps = (pLocMesh->p + pLocMesh->pUpMesh->p)
			*1.e-8*(pLocMesh->dy + pLocMesh->pUpMesh->dy)/4.0;  /* integrate hole conc. */
		pLocMesh->pLayer->psLayer = pLocMesh->pLayer->psLayer + ps;
		*psTotal = *psTotal + ps;
		
		*sheetres = *sheetres + q*(pLocMesh->emob*ns + pLocMesh->hmob*ps); /* sheet resistance (inverse) */
	} while((pLocMesh=pLocMesh->pDownMesh) != pGlobals->pBottomMesh);

	*sheetres = 1.0e0/(*sheetres); /* convert to real sheet resistance */

	return ;
}