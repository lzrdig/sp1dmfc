#include "stdafx.h"

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

#include "poisp_types.h"
#include "poisp_defs.h"

#include "..\schrpois.h"

	
/***********************************************/

void CSchrPois::adjust(struct global_type *pGlobals, struct flag_type *pFlags) {

/***********************************************/


/*  This subroutine adjusts the voltage vector according to Newton's
/*  method, see Hildebrand, Introduction to Numerical Methods p 583.  
/*  Delta v is found using Crouts method to solve the jacobian matrix (which
/*  fortunately is a tridiagonal matrix) equation.  The maximum change in
/*  the voltage vector is returned as the convergence parameter. */

/*	double wherey;
	char outdata[100],extension2[30],access;
	int ioerr_loc;  /* variables for trouble shooting */
	double deltav;
	struct mesh_type *pMesh;

/*  Find the current error and form the jacobian matrix (in Crout form).  The
/*  Jacobian is the partial derivative of the poisson equation with respect to
/*  each of the pMesh->v.
/*  Deriv is the partial derivative of the charge concentration with respect
/*  to pMesh->v (calculated in subroutine charge), and is included only on the
/*  main diagonal of the jacobian. */


	pMesh = pGlobals->pStartMesh;

/*	 determine surface boundary condition */

	if (!strcmp(pGlobals->surfcontact,"slope")) {
		pMesh->pUpMesh->v = pMesh->pDownMesh->v;
		pMesh->f = 2.0*invh*pMesh->esemi/(pMesh->dy * pMesh->dy);
	}		
	else
		pMesh->f = invh*pMesh->esemi/(pMesh->dy * pMesh->dy);
	
	pMesh->c = -1.0*(invh*2.0e0/(pMesh->dy + pMesh->pUpMesh->dy)
		*(pMesh->esemi*(pMesh->pDownMesh->v - pMesh->v)/pMesh->dy
		-pMesh->pUpMesh->esemi*(pMesh->v - pMesh->pUpMesh->v)/pMesh->pUpMesh->dy)
		-q*(pMesh->fcharge + pMesh->p - pMesh->n));				/* poisson error term */
	pMesh->d = -invh*2.0e0/(pMesh->dy + pMesh->pUpMesh->dy)
		*(pMesh->esemi/pMesh->dy + pMesh->pUpMesh->esemi
		/pMesh->pUpMesh->dy)-q*pMesh->deriv;					/* main diagonal */

	pMesh->e = 0.0e0;

	pGlobals->poierr = pMesh->c;						/* find max poisson error */

	pMesh = pMesh->pDownMesh;
	do {

		pMesh->c = -1.0*(invh*2.0e0/(pMesh->dy+pMesh->pUpMesh->dy)
			*(pMesh->esemi*(pMesh->pDownMesh->v - pMesh->v)/pMesh->dy
			-pMesh->pUpMesh->esemi*(pMesh->v - pMesh->pUpMesh->v)/pMesh->pUpMesh->dy)
			-q*(pMesh->fcharge + pMesh->p - pMesh->n));				/* poisson error term */

		pMesh->e = invh*2.0e0/(pMesh->dy + pMesh->pUpMesh->dy)
			*pMesh->pUpMesh->esemi/pMesh->pUpMesh->dy;					/* subdiagonal */

		pMesh->d = -invh*2.0e0/(pMesh->dy+pMesh->pUpMesh->dy)
			*(pMesh->esemi/pMesh->dy+pMesh->pUpMesh->esemi
			/pMesh->pUpMesh->dy)-q*pMesh->deriv;						/* main diagonal */

		pMesh->f = invh*2.0e0/(pMesh->dy+pMesh->pUpMesh->dy)
			*pMesh->esemi/pMesh->dy;						/* superdiagonal */

		if (fabs(pMesh->c) > fabs(pGlobals->poierr)) 		/* find max poisson error */
			pGlobals->poierr = pMesh->c;

	} while((pMesh = pMesh->pDownMesh) != pGlobals->pStopMesh);

/*	 determine substrate boundary condition */

	pMesh = pGlobals->pStopMesh;
	if (!strcmp(pGlobals->subcontact,"slope")) {
		pMesh->pDownMesh->v = pMesh->pUpMesh->v;
		pMesh->e = 2.0e0*invh*pMesh->pUpMesh->esemi
			*2.0e0/(pMesh->pUpMesh->dy*(pMesh->dy + pMesh->pUpMesh->dy));	/* subdiagonal */
	}
	else
		pMesh->e = invh*pMesh->pUpMesh->esemi
			*2.0e0/(pMesh->pUpMesh->dy*(pMesh->dy+pMesh->pUpMesh->dy));	/* subdiagonal */


	pMesh->c = -1.0*(invh*2.0e0/(pMesh->dy+pMesh->pUpMesh->dy)
		*(pMesh->esemi*(pMesh->pDownMesh->v-pMesh->v)/pMesh->dy
		-pMesh->pUpMesh->esemi*(pMesh->v-pMesh->pUpMesh->v)/pMesh->pUpMesh->dy)
		-q*(pMesh->fcharge + pMesh->p - pMesh->n));				/* poisson error term */

	pMesh->d = -invh*2.0e0/(pMesh->dy+pMesh->pUpMesh->dy)
		*(pMesh->esemi/pMesh->dy+pMesh->pUpMesh->esemi
		/pMesh->pUpMesh->dy)-q*pMesh->deriv;						/* main diagonal */

	pMesh->f = 0.0e0;

	if (fabs(pMesh->c) > fabs(pGlobals->poierr)) { 
		pGlobals->poierr = pMesh->c;
	}

/*  Trouble shooting code */

/*	access='w';
	strcpy(extension2,"");
	ioerr_loc = fileopen(4, "help", access, pGlobals);
	pMesh = pGlobals->pStartMesh;
	do {
	sprintf(outdata, "%11.4e\t%11.4e\t%11.4e\t%11.4e\t%11.4e\t%11.4e\t%11.4e\t%11.4e\t%11.4e\n", 
			pMesh->meshcoord,pMesh->v+pMesh->offset, pMesh->e,
			pMesh->d,pMesh->f,pMesh->c,pMesh->fcharge,pMesh->p,pMesh->n);
	fprintf(pGlobals->pFile[4],outdata);
	} while((pMesh = pMesh->pDownMesh) != pGlobals->pStopMesh);
	fileclose(4, pGlobals);*/

/*  find the correction terms deltav using Crout's reduction */

/*  Form the Crout auxiliary matrix. */

	pMesh = pGlobals->pStartMesh;
	pMesh->f = pMesh->f/pMesh->d;
	pMesh->c = pMesh->c/pMesh->d;

	pMesh=pMesh->pDownMesh;
	do {
		pMesh->d = pMesh->d - pMesh->e*pMesh->pUpMesh->f;
		pMesh->f = pMesh->f/pMesh->d;
		pMesh->c = (pMesh->c-pMesh->e*pMesh->pUpMesh->c)/pMesh->d;
	} while((pMesh=pMesh->pDownMesh) != pGlobals->pStopMesh->pDownMesh);

/*  Trouble shooting code */

/*	access='w';
	strcpy(extension2,"");
	ioerr_loc = fileopen(4, "help1", access, extension2, pGlobals);
	pMesh = pGlobals->pStartMesh;
	do {
	sprintf(outdata, "%11.4e\t%11.4e\t%11.4e\t%11.4e\t%11.4e\t%11.4e\n", 
			pMesh->meshcoord,pMesh->v+pMesh->offset, pMesh->e,
			pMesh->d,pMesh->f,pMesh->c);
	fprintf(pGlobals->pFile[4],outdata);
	} while((pMesh = pMesh->pDownMesh) != pGlobals->pStopMesh);
	fileclose(4, pGlobals);*/


/* find new solution of the voltage v. */

	pMesh = pGlobals->pStopMesh;
	deltav = pMesh->c;
	pMesh->v = pMesh->v + deltav;
	pGlobals->ConvgError = fabs(deltav);

	pMesh = pMesh->pUpMesh;
	do {
		deltav = pMesh->c - pMesh->f*deltav;
		pMesh->v = pMesh->v + deltav;
		if (fabs(deltav) > pGlobals->ConvgError) {
			pGlobals->ConvgError = fabs(deltav);
		}
	} while ((pMesh = pMesh->pUpMesh) != pGlobals->pStartMesh->pUpMesh);

/*	write (*,*) meshcoord(wherey),deriv(wherey),fcharge(wherey),p(wherey)
/*	write (*,*) wherey
/*	write (*,*) "adjust",wherey,meshcoord(wherey),pGlobals->ConvgError
/*	write (*,*) ' '
/*	pause */

	return;
}