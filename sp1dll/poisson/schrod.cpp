#include "stdafx.h"
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "poisp_types.h"
#include "poisp_defs.h"
#include "nrutil.h"

#include "..\schrpois.h"


double integrationConst;

/**************************************/	
#define EPS 1.0e-6
#define JMAX 20
#define JMAXP (JMAX+1)
#define K 5
#define FUNC(x) ((*func)(x))


/**************************************/	
double danger(double E)  {
/**************************************/
	return sqrt(E)/(1.0e0+exp(E+integrationConst));
}

/**************************************/	
double ddanger(double E) {
/**************************************/
	return 1.0e0/(1.0e0+exp(E*E+integrationConst));
}

/***************************************/
double QROMB(double (*func)(double), double a, double b) 
/***************************************/ 
{
/*	void polint(double xa[], double ya[], int n, double x, double *y, double *dy);
	double trapzd(double (*func)(double), double a, double b, int n);*/
	void nrerror(char error_text[]);
	double ss,dss;
	double s[JMAXP],h[JMAXP+1];
	int j;
	char message[MAX_MESSAGE];
	
	h[1]=1.0e0;
	for (j=1;j<=JMAX;j++) {
		s[j] = trapzd(func,a,b,j);
		if (j >= K) {
			polint(&h[j-K],&s[j-K],K,0.0e0,&ss,&dss);
			if (fabs(dss) < EPS*fabs(ss)) return ss;
		}
		s[j+1]=s[j];
        h[j+1]=0.25e0*h[j];
	}
	strcpy(message,"Too many steps. Numerical integration failed.");
	alertbox(message);
	return 0.0;
}      

/***************************************/
double trapzd(double (*func)(double), double a, double b, double n) {
/***************************************/  
	double x,tnm,sum,del;
	static double s;
	int it,j;
	
	if (n == 1) {
		return (s=0.5e0*(b-a)*(FUNC(a)+FUNC(b)));
	}
	else {
		for (it=1,j=1; j<n-1;j++) it <<=1;
		tnm = it;
		del = (b-a)/tnm;
		x = a+0.5e0*del;
		for (sum=0.0e0,j=1;j<=it;j++,x+=del) sum+=func(x);
		s = 0.5e0*(s+(b-a)*sum/tnm);
		return s;
	}
}


/**************************************************/
void polint(double xa[],double ya[],int n,double x,double *y,double *dy) {	
/**************************************************/

	int i,m,ns=1;
	double den,dif,dift,ho,hp,w;
	double *c,*d;
	
	dif = fabs(x-xa[1]);
	c = vector(1,n);
	d = vector(1,n);
	for (i=1;i<=n;i++) {
		if ((dift=fabs(x-xa[i])) < dif) {
			ns = i;
			dif = dift;
		}
		c[i] = ya[i];
		d[i] = ya[i];
	}
	*y = ya[ns--];
	for (m=1;m<n;m++) {
		for (i=1;i<=n-m;i++) {
			ho = xa[i]-x;
			hp = xa[i+m]-x;
			w = c[i+1]-d[i];
			if ((den=ho-hp) == 0.0e0) alertbox("Error in routine polint");
			den = w/den;
			d[i] = hp*den;
			c[i] = ho*den;
		}
		*y += (*dy=(2*ns < (n-m) ? c[ns+1] : d[ns--]));
	}
	free_vector(d,1,n);
	free_vector(c,1,n);
}

/**************************************************/
int CSchrPois::schrodingersolve(struct global_type *pGlobals, struct flag_type *pFlags) {      
 
/*  This program calculates some of the one dimensional eigen values and wave  
  functions of an arbitrary potential.  The pFlags->schrodinger equation is descretized  
  at each point and the corresponding matrix is formed and solved.  Standard   
  eispack routines are used to find the eigenvalues and eigenvectors.  The  
  calls to the eispack routines are placed in a second subroutine because   
  the eispack routine require the dimension of various vectors to be   
  defined in the calling routine.  This makes it necessary to embeded the   
  eispack routine calls in a subroutine to fool the eispack routines into  
  accepting general matrices. */

	int ierr,fixedBounds;
	
	PostInfoSP("Performing schrodingersolve");

	fixedBounds = false;  /* Bounds of subband search are not fixed */
	
	ierr = schdoelectrons(pGlobals, pFlags);
	if(ierr) return 1;
	ierr = schdoholes(pGlobals, pFlags);
	if(ierr) return 1;
  	
	return 0;
}



/**************************************************/
int CSchrPois::schcharge(char carrierType, int fixedBounds, struct global_type *pGlobals, struct flag_type *pFlags) 
{
	double *Es;
	double *Ds;
	double *L;
	double *pEigen;
	double statemass;	/* average mass of occupied state */	
	double H;		/* hamiltonian including delta y, but not effec. mass */
	double f;		/* fermi distrib fun */
	double g;		/* derivative function */
	char extension[MAX_EXTENSION];
	double density;
	double abstol; /* tolerance for eigenvalues */
	double tem;
	int ierr,nsplit,idummy,*pIfail,i,j;
	char range,ordering;
	char message[MAX_MESSAGE];


	/*  find the interval of interest for the eigen values  
	  This interval will be from the lowest point in the potential to the highest point. */


	if (!fixedBounds) {
		pGlobals->LB = pGlobals->pPotential[0];
		pGlobals->UB = pGlobals->pPotential[0];
	}
	pGlobals->pEvcharge[0] = 0.0e0;
	pGlobals->pEvderiv[0] = 0.0e0;
	pGlobals->numeval = 0;		/* reset the number of eigenvalues found */
	pFlags->DangerWill = false;	/*reset the danger flag on each iteration */
	Es = (double *) malloc(pGlobals->order*sizeof(double));
	Ds = (double *) malloc(pGlobals->order*sizeof(double));
	L = (double *) malloc(pGlobals->order*sizeof(double));
	pWork = (double *) malloc(4*pGlobals->order*sizeof(double));
	if (pWork == NULL) {
		strcpy(message,"Can't allocate memory for Schrodinger array (pWork).  Increase Memory allocation.");
		alertboxWin(message);
		return 1;
	}
	pIwork = (int *) malloc(3*pGlobals->order*sizeof(int));
	if (pIwork == NULL) {
		strcpy(message,"Can't allocate memory for Schrodinger array (pIwork).  Increase Memory allocation.");
		alertboxWin(message);
		return 1;
	}
	pIsplit = (int *) malloc(pGlobals->order*sizeof(int));
	if (pIsplit == NULL) {
		strcpy(message,"Can't allocate memory for Schrodinger array (pIsplit).  Increase Memory allocation.");
		alertboxWin(message);
		return 1;
	}
	pIblock = (int *) malloc(pGlobals->order*sizeof(int));
	if (pIblock == NULL) {
		strcpy(message,"Can't allocate memory for Schrodinger array (pIblock).  Increase Memory allocation.");
		alertboxWin(message);
		return 1;
	}

	for(i=1; i<pGlobals->order; i++) {
		pGlobals->pEvcharge[i] = 0.0e0;
		pGlobals->pEvderiv[i] = 0.0e0;
		if (!fixedBounds) {
			if (pGlobals->pPotential[i] < pGlobals->LB) 	/* look for lowest point in band diagram */
				pGlobals->LB = pGlobals->pPotential[i];
		}
	}
	
	if (!fixedBounds) {
		if (pGlobals->pPotential[pGlobals->order-1] < pGlobals->UB) pGlobals->UB = pGlobals->pPotential[pGlobals->order-1];	/* find least upper bound  */

		if (pGlobals->UB-pGlobals->pSchStart->Efn > 8.0*pGlobals->kT) {		/* limit range if pGlobals->kT is small */
			if (pGlobals->LB-pGlobals->pSchStart->Efn > 8.0*pGlobals->kT) 
				return 0;				/* return if no eigenvalue can be occupied */
			else
				pGlobals->UB = 8.0*pGlobals->kT+pGlobals->pSchStart->Efn;
		}
		else if (pFlags->printDanger) {
			strcpy(message,"Danger Will Robinson, Danger! Structure does not provide 8kT of confinement at edges of Schrodinger Mesh");
			alertboxWin(message);
			if (pFlags->stopDanger) {
				if (carrierType == 'e') {
					strcpy(message,"Check the energy of the conduction band at Schrodingerstart and Schrodingerstop");
					alertboxWin(message);
				}
				else {
					strcpy(message,"Check the energy of the valence band at Schrodingerstart and Schrodingerstop");
					alertboxWin(message);
				}
	    
	    		strcpy(extension,"");
				output(pGlobals->filename, extension, 1, pGlobals, pFlags);
				return 1;	/* Kill the simulation. */
			}
	   
			else {
				strcpy(message,"Things are going to be a little slow from now on!");
				alertboxWin(message);
				pFlags->printDanger = false;
				pFlags->DangerWill = true;
			}
		}
		else
			pFlags->DangerWill = true;
	}

	if (pGlobals->UB <= pGlobals->LB) {
		pGlobals->numeval = 0;
		free(pWork);
		free(pIwork);
		free(pIsplit);
		free(pIblock);
		free(Es);
		free(Ds);
		free(L);
		return 0;
	}

/*	initialize variables */
	H = -3.81;		/* H = hbar**2/(2*mo) */
	if (pGlobals->kT != 0.0e0) {  
		/* density = m* /(pi*hbar^2)*kT, eff DOS, effective mass part is put in later */
		density = 4.17779e14*pGlobals->kT;
	}
	else  
	/* density = m* /(pi*hbar^2), true DOS for T=0, effective mass part is put in later */
	  density = 4.17779e14;
	
/*	set up matrix */

	L[0] = sqrt(pGlobals->pSdy[0]);
	for (i=1; i<pGlobals->order; i++)
		L[i] = sqrt(0.5e0*(pGlobals->pSdy[i]+pGlobals->pSdy[i-1]));

	i=0;
	Ds[i] = -2.0*H/(pGlobals->pMass[i]*L[i]*L[i]*pGlobals->pSdy[i])+pGlobals->pPotential[i];
	Es[i] = 2.e0*H/(2.0*pGlobals->pMass[i]*pGlobals->pSdy[i]*L[i]*L[i]);
	for (i=1; i<pGlobals->order-1; i++) {
		Ds[i] = -H/(L[i]*L[i])*(2.0e0/((pGlobals->pMass[i]+pGlobals->pMass[i-1])*pGlobals->pSdy[i-1])
			+2.0e0/((pGlobals->pMass[i+1]+pGlobals->pMass[i])*pGlobals->pSdy[i]))+pGlobals->pPotential[i];
		Es[i] = 2.e0*H/((pGlobals->pMass[i-1]+pGlobals->pMass[i])*pGlobals->pSdy[i-1]*L[i-1]*L[i]);
	}
	i=pGlobals->order-1;
	Ds[i] = -H/(L[i]*L[i])*(2.0e0/((pGlobals->pMass[i]+pGlobals->pMass[i-1])*pGlobals->pSdy[i-1])
			+2.0e0/((pGlobals->pMass[i]+pGlobals->pMass[i])*pGlobals->pSdy[i]))+pGlobals->pPotential[i];
	  

  
/*  correct matrix for slope = zero boundary conditions */  

	Ds[0] = Ds[0] + H/(pGlobals->pMass[0]*pGlobals->pSdy[0]*L[0]*L[0]);
	Ds[pGlobals->order-1] = Ds[pGlobals->order-1] + H/(pGlobals->pMass[pGlobals->order-1]*pGlobals->pSdy[pGlobals->order-1]*L[pGlobals->order-1]*L[pGlobals->order-1]);



/* troubleshooting code */
/*	access='w';
	strcpy(extension2,"");
	ierr = fileopen(4, "help", access, extension2, pGlobals);
	for (i=0;i<pGlobals->order;i++){
		fprintf(pGlobals->pFile[4], "%2i\t%10.3e\t%10.3e\t%10.3e\t%10.3e\t%10.3e\t%10.3e\n",i,pGlobals->pSdy[i],pGlobals->pPotential[i],pGlobals->pMass[i],L[i],Es[i],Ds[i]);
	}
	fileclose(4, pGlobals); 

/*  find eigenvalues, and eigenvectors */

	range = 'S';
	abstol = 2.0*dlamch_(&range);
	range = 'V';
	ordering = 'B';
	idummy = 0;
	dstebz_(&range,&ordering, &pGlobals->order, &pGlobals->LB, &pGlobals->UB, &idummy, &idummy, 
		&abstol,Ds, Es, &pGlobals->numeval, &nsplit, 
		pGlobals->pEval, pIblock, pIsplit, pWork, pIwork, &ierr);
	
	if (ierr != 0) {
		if (ierr > 0) {  
			strcpy(message,"Eigenvalue bisection failed.");
			alertboxWin(message);
			pFlags->eigenfail = true;
			return 1;
		}
	}
	
	free(pWork);
	free(pIwork);
	if (pGlobals->numeval == 0) {
		free(pIsplit);
		free(pIblock);
		free(Es);
		free(Ds);
		free(L);
		return 0;
	}
	
	pWork = (double *) malloc(5*pGlobals->order*sizeof(double));
	if (pWork == NULL) {
		strcpy(message,"Can't allocate memory for Schrodinger array (pWork for evec).  Increase Memory allocation.");
		alertboxWin(message);
		return 1;
	}
	pIwork = (int *) malloc(pGlobals->order*sizeof(int));
	if (pIwork == NULL) {
		strcpy(message,"Can't allocate memory for Schrodinger array (pIwork for evec).  Increase Memory allocation.");
		alertboxWin(message);
		return 1;
	}
	pIfail = (int *) malloc(pGlobals->numeval*sizeof(int));
	if (pIfail == NULL) {
		strcpy(message,"Can't allocate memory for Schrodinger array (pIfail).  Increase Memory allocation.");
		alertboxWin(message);
		return 1;
	}
	pEigen = (double *) malloc(pGlobals->order*pGlobals->numeval*sizeof(double));
	if (pEigen == NULL) {
		strcpy(message,"Can't allocate memory for Schrodinger array (pEigen).  Increase Memory allocation.");
		alertboxWin(message);
		return 1;
	}	
		
	dstein_(&pGlobals->order, Ds, Es, &pGlobals->numeval, pGlobals->pEval, pIblock, pIsplit, pEigen, &pGlobals->order, pWork, pIwork, 
		pIfail, &ierr);
		
	pGlobals->pEigenv = (double **) malloc(pGlobals->order*sizeof(double *));
	if (pGlobals->pEigenv == NULL) {
		strcpy(message,"Can't allocate memory for Schrodinger array (pGlobals->pEigenv).  Increase Memory allocation.");
		alertboxWin(message);
		return 1;
	}	
	for (i=0; i<pGlobals->order; i++) {
		pGlobals->pEigenv[i] = (double *) malloc(pGlobals->numeval*sizeof(double));
		if (pGlobals->pEigenv[i] == NULL) {
			strcpy(message,"Can't allocate memory for Schrodinger array (pGlobals->pEigenv, 2nd row).  Increase Memory allocation.");
			alertboxWin(message);
			return 1;
		}	
	}
	
/* transfer eigenvector from temporary storage */	
	for (i=0;i<pGlobals->numeval; i++)
		for (j=0;j<pGlobals->order; j++){
			tem = i*pGlobals->order+j;
			pGlobals->pEigenv[j][i] = pEigen[i*pGlobals->order+j];
	}
	free(pEigen);
	free(pIfail);
	free(pWork);
	free(pIwork);
	free(pIsplit);
	free(pIblock);
	free(Es);
	free(Ds);
		
/* troubleshooting code
	access='w';
	strcpy(extension2,"");
	ierr = fileopen(4, "helpevec", access, extension2, pGlobals);
	for (i=0;i<pGlobals->order;i++){
		fprintf(pGlobals->pFile[4], "%2i\t%10.3e\t%10.3e\t%10.3e\t%10.3e\n",i,pGlobals->pEigenv[i][0],pGlobals->pEigenv[i][1],pGlobals->pEigenv[i][2],pGlobals->pEigenv[i][3]);
	}
	fileclose(4, pGlobals);
/* end troubleshooting code */

	for (i=0;i<pGlobals->numeval; i++) {

/* Find the subband occupation prefactor f, and the derivative prefactor g. */
		if (pGlobals->kT == 0.0e0) {
			f = -density*(pGlobals->pEval[i]-pGlobals->pSchStart->Efn);
			g = density;
		}
		else {
			tem = (pGlobals->pSchStart->Efn - pGlobals->pEval[i])/pGlobals->kT;
			if (tem < 100.0) {
				tem = exp(tem);
				f= density*log(tem+1.0);			
				g= density*tem/(pGlobals->kT*(tem + 1.0));
			}		
			else {
				f = density*tem;
				g = density/pGlobals->kT;
			}
		}
			
		if (pFlags->DangerWill) 
			f=density*log((1.0e0+tem)/(1.0e0+exp((pGlobals->pSchStart->Efn-pGlobals->UB)/pGlobals->kT)));

/*  Renormalize the wavefunctions to account for variable mesh */
		for (j=0;j<pGlobals->order;j++) 
			pGlobals->pEigenv[j][i] = pGlobals->pEigenv[j][i]/L[j];

/*  Find the "average mass" of the subband by calculating an expectation value of the mass of the state   
  by weighting the mass at each point by psi. */

		statemass = 0.0e0;
		tem = 0.0e0;
		for (j=0;j<pGlobals->order-1;j++) {
			statemass = statemass + pGlobals->pSdy[j]*(pGlobals->pEigenv[j][i]*pGlobals->pEigenv[j][i]*pGlobals->pMass[j]+pGlobals->pEigenv[j+1][i]
				*pGlobals->pEigenv[j+1][i]*pGlobals->pMass[j+1])/2.0e0;
			tem = tem + pGlobals->pSdy[j]*(pGlobals->pEigenv[j][i]*pGlobals->pEigenv[j][i]+pGlobals->pEigenv[j+1][i]*pGlobals->pEigenv[j+1][i])/2.0e0;
		}
		statemass = statemass/tem;

		for (j=0; j<pGlobals->order; j++) {
			pGlobals->pEvcharge[j]=pGlobals->pEvcharge[j]+pGlobals->pEigenv[j][i]*pGlobals->pEigenv[j][i]*f*statemass/(1.e-8);		/* find charge concentration */
			pGlobals->pEvderiv[j]=pGlobals->pEvderiv[j]+pGlobals->pEigenv[j][i]*pGlobals->pEigenv[j][i]*g*statemass/(1.e-8);	/* find partial derivative */
		}
/* troubleshooting code */
/*	access='w';
	strcpy(extension2,"");
	ierr = fileopen(4, "helpcharge1", access, extension2, pGlobals);
	for (j=0;j<pGlobals->order;j++){
		fprintf(pGlobals->pFile[4], "%2i\t%10.3e\t%10.3e\t%10.3e\t%10.3e\t%10.3e\n",j,pGlobals->pEvcharge[j],pGlobals->pEvderiv[j],pGlobals->pEigenv[j][0],pGlobals->pEigenv[j][1],pGlobals->pPotential[j]);
	}
	fileclose(4, pGlobals);
/* end troubleshooting code */
	}
	free(L);

	return 0;
}

int CSchrPois::schdoelectrons(struct global_type *pGlobals, struct flag_type *pFlags)
{
	double nthird, n2third;
	double ndanger,dndanger,*pOldn,*pOldderiv,tem;
	int ierr,fixedBounds,i;
	char message[MAX_MESSAGE];
	
	fixedBounds = false;  /* Bounds of subband search are not fixed */

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
		pMesh->n = 1.0e-10;
		pMesh->deriv = 0.0e0;
		pMesh = pMesh->pDownMesh;
	}

/* troubleshooting code 
	access='w';
	ierr = fileopen(4, "help", access, "", pGlobals);
	for (i=0;i<pGlobals->order;i++){
		fprintf(pGlobals->pFile[4], "%13.6e\t%15.8e\t%15.8e\n",pGlobals->pPotential[i],
						pGlobals->pMass[i],pGlobals->pSdy[i]);
	}
	fileclose(4, pGlobals);
/* end troubleshoot */

	pGlobals->numeleceval = 0;
	if (pFlags->schElecMemAlloc) {
		for (i=0; i<pGlobals->order; i++) free(pGlobals->pElecevec[i]);
		free(pGlobals->pElecevec);
		free(pGlobals->pEleceval);
		pFlags->schElecMemAlloc = false;
	}

	if (! pFlags->defaultNoElec) {
		ierr = schcharge('e', fixedBounds, pGlobals, pFlags); 
		if (ierr) {
			strcpy(message,"Error in electron Schrodinger solution.");
			alertboxWin(message);
			return 1;		/* stop the calculation if error */
		}
		pMesh = pGlobals->pSchStart;
		i = 0;
		do {
			if (!pMesh->noelec) {
				pMesh->n = pMesh->valley* pGlobals->pEvcharge[i];
				if (pFlags->exchange && (pOldn[i] > 1.0e0)) {
					n2third = pow(pOldn[i],(2.e0/3.e0));
					nthird = sqrt(n2third);
					pMesh->deriv = pMesh->valley * pGlobals->pEvderiv[i]*(1.0e0 - 4.187e-21
						/(n2third*pMesh->esemi) * pOldderiv[i]*(1.0e0 +0.7734e0/(1.0e0 +13.0274e0
						*pMesh->aBohrElec*nthird)));
				}
				else
					pMesh->deriv = pMesh->valley* pGlobals->pEvderiv[i];


				if (pFlags->DangerWill && !pFlags->ignoreDanger) {
					tem = (pGlobals->UB-pGlobals->pPotential[i])/pGlobals->kT;
					if (tem < 0.0e0) {  
						/* Conduction band is above the upper bound, standard electron calc. */
						pMesh->n = pMesh->n + electrons(pMesh,pGlobals->kT);
						pMesh->deriv = pMesh->deriv + electronderiv(pMesh,pGlobals->kT);
					}
					else {  
						/* Numerical integration to find electron concentration of unbound states */
						integrationConst = (pGlobals->pPotential[i]-pMesh->Efn)/pGlobals->kT;
						ndanger = QROMB(danger,tem,15.0e0+(pMesh->Efn- pGlobals->pPotential[i])/pGlobals->kT);
						tem = sqrt(tem*pGlobals->kT);
						dndanger = QROMB(ddanger,tem,sqrt(15.0e0*pGlobals->kT-pGlobals->pPotential[i]+pMesh->Efn));
						pMesh->n = pMesh->n + pMesh->Nc*1.128379e0*ndanger;	/* n(schrod)+Nc*(2/sqrt(pi))*integral */
						pMesh->deriv = pMesh->deriv + pMesh->Nc*1.128379e0*dndanger/pow(pGlobals->kT,1.5);
					}
				}	
			}
			i = i+1;
		} while ((pMesh= pMesh->pDownMesh) != pGlobals->pSchStop->pDownMesh);  


/* troubleshooting code */
/*	ierr = fileopen(4, "helpcharge", 'w', "", pGlobals);
	pMesh = pGlobals->pSchStart;
	i=0;
	do {
		fprintf(pGlobals->pFile[4], "%13.6e\t%13.6e\t%13.6e\t%13.6e\n",pMesh->meshcoord,pMesh->v+pMesh->offset,pGlobals->pEvcharge[i],pMesh->deriv);
		i++;
	} while((pMesh = pMesh->pDownMesh) != pGlobals->pStopMesh);
	fileclose(4, pGlobals);
/* end troubleshoot */

		/* save the electron eigenvalues and eigenvectors */
		ierr = schsavelectroneigens(pGlobals, pFlags);
		if(ierr) return 1;
	}
	
	/* Free up the workspace memory */
	free(pOldn);
	free(pOldderiv);

	return 0;
}


int CSchrPois::schdoholes(struct global_type *pGlobals, struct flag_type *pFlags)
{
	double pthird;
	int ierr,fixedBounds,i;

	fixedBounds = false;  /* Bounds of subband search are not fixed */

	pMesh = pGlobals->pSchStart;
	for (i=0; i<pGlobals->order; i++) {
		pGlobals->pMass[i] = pMesh->ehm;
		pMesh->p = 1.0e-10;
		pMesh = pMesh->pDownMesh;
		if (pFlags->exchange && (pMesh->p > 1.0e0)) {
			pthird = pow(pMesh->p,(1.e0/3.e0));
			pGlobals->pPotential[i] = -(pMesh->v+pMesh->offset-pMesh->Eg-1.256e-20*pthird/pMesh->esemi
				*(1.0e0+0.034e0/(pMesh->aBohrHhole*pthird)*log(1.0e0+18.376e0*pMesh->aBohrHhole*pthird)));
		}
		else
			pGlobals->pPotential[i] = -(pMesh->v+pMesh->offset-pMesh->Eg);
	}
	pGlobals->numhheval = 0;	
	pGlobals->numeval = 0;
	if (pFlags->schHhMemAlloc) {
		for (i=0; i<pGlobals->order; i++)
			free(pGlobals->pHhevec[i]);
		free(pGlobals->pHhevec);
		free(pGlobals->pHheval);
		pFlags->schHhMemAlloc = false;
	}

	if (!pFlags->defaultNoHoles) {
		/*  Do the heavy holes */
		ierr = schdoheavyholes(pGlobals, pFlags);
		if(ierr) return 1;
		/* save the heavy hole eigenvalues and eigenvectors */
		ierr = schsavehholeigens(pGlobals, pFlags);
		if(ierr) return 1;
		/*  Now do the light holes */
		ierr = schdolightholes(pGlobals, pFlags);
		if(ierr) return 1;
		/* save the light hole eigenvalues eigenvectors */
		ierr = schsavelholeigens(pGlobals, pFlags);
		if(ierr) return 1;
	}
	return 0;
}

int CSchrPois::schsavelectroneigens(struct global_type *pGlobals, struct flag_type *pFlags)
{
	int i,j;
	char message[MAX_MESSAGE];

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
		pFlags->schElecMemAlloc = true;
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

	return 0;
}

int CSchrPois::schsavehholeigens(struct global_type *pGlobals, struct flag_type *pFlags)
{
	int i,j;
	char message[MAX_MESSAGE];

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
		pFlags->schHhMemAlloc = true;
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

	return 0;
}

int CSchrPois::schsavelholeigens(struct global_type *pGlobals, struct flag_type *pFlags)
{
	int i,j;
	char message[MAX_MESSAGE];

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
		pFlags->schLhMemAlloc = true;
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

	return 0;
}

int CSchrPois::schdoheavyholes(struct global_type *pGlobals, struct flag_type *pFlags)
{
	int ierr,fixedBounds,i;
	char message[MAX_MESSAGE];

	fixedBounds = false;  /* Bounds of subband search are not fixed */
	
	ierr = schcharge('h', fixedBounds, pGlobals, pFlags); 
	if (ierr) {
		strcpy(message,"Error in heavy hole Schrodinger solution.");
		alertboxWin(message);
		return 1;		/* stop the calculation if error */
	}
	pMesh = pGlobals->pSchStart;
	i = 0;
	do {
		if (!pMesh->noholes) {
			pMesh->p = pGlobals->pEvcharge[i];
			pMesh->deriv = pMesh->deriv + pGlobals->pEvderiv[i];
		}
		i = i+1;
	} while ((pMesh= pMesh->pDownMesh) != pGlobals->pSchStop->pDownMesh);
	
	return 0;
}

int CSchrPois::schdolightholes(struct global_type *pGlobals, struct flag_type *pFlags)
{
	int ierr,fixedBounds,i;
	char message[MAX_MESSAGE];
	double ndanger, dndanger, tem;

	fixedBounds = false;  /* Bounds of subband search are not fixed */

	pMesh = pGlobals->pSchStart;
	for (i=0; i<pGlobals->order; i++) {
		pGlobals->pMass[i] = pMesh->elhm;
	}
	pGlobals->numlheval = 0;	
	pGlobals->numeval = 0;
	if (pFlags->schLhMemAlloc) {
		for (i=0; i<pGlobals->order; i++)
			free(pGlobals->pLhevec[i]);
		free(pGlobals->pLhevec);
		free(pGlobals->pLheval);
		pFlags->schLhMemAlloc = false;
	}
	
	ierr = schcharge('l', fixedBounds, pGlobals, pFlags); 
	if (ierr) {
		strcpy(message,"Error in light hole Schrodinger solution.");
		alertboxWin(message);
		return 1;		/* stop the calculation if error */
	}
	pMesh = pGlobals->pSchStart;
	i = 0;
	do {
		if (!pMesh->noholes) {
			pMesh->p = pMesh->p + pGlobals->pEvcharge[i];
			pMesh->deriv = pMesh->deriv + pGlobals->pEvderiv[i];
  
  			if (pFlags->DangerWill && !pFlags->ignoreDanger) {
				tem = (pGlobals->UB-pGlobals->pPotential[i])/pGlobals->kT;
				if (tem < 0.0e0) {  
					/* Conduction band is above the upper bound, standard electron calc. */
					pMesh->p = pMesh->p + holes(pMesh,pGlobals->kT);
					pMesh->deriv = pMesh->deriv + holederiv(pMesh,pGlobals->kT);
				}
				else {  
					/* Numerical integration to find hole concentration of unbound states */
					integrationConst = (pGlobals->pPotential[i]-pMesh->Efn)/pGlobals->kT;
					ndanger = QROMB(danger,tem,15.0e0+(pMesh->Efn- pGlobals->pPotential[i])/pGlobals->kT);
					tem = sqrt(tem*pGlobals->kT);
					integrationConst =(pGlobals->pPotential[i]-pMesh->Efn)/pGlobals->kT;
					dndanger = QROMB(ddanger,tem,sqrt(15.0e0*pGlobals->kT-pGlobals->pPotential[i]+pMesh->Efn));
					pMesh->p = pMesh->p + pMesh->Nv*1.128379e0*ndanger;	/* p(schrod)+Nv*(2/sqrt(pi))*integral */
					pMesh->deriv = pMesh->deriv + pMesh->Nv*1.128379e0*dndanger/pow(pGlobals->kT,1.5);
				}
  			}
		}
		i = i+1;
	} while ((pMesh= pMesh->pDownMesh) != pGlobals->pSchStop->pDownMesh);

	return 0;
}