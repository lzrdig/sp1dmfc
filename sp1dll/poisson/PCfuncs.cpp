#include "stdafx.h"

/* PC specific functions */
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "poisp_types.h"
#include "poisp_defs.h"



/****************************************/
void alertbox(char messag[]) {
/****************************************/

	/* Show alert message */
	strcat(messag,"\n");
	printf("%s",messag);
}

void alertboxWin(char message[]) {
/****************************************/
	/* Show alert message */

	MessageBox(NULL,message,"Message",MB_OK);
}
