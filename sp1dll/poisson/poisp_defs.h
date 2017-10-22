
/* Definitions and functions used in 1D Poisson */

/* Token definitions */
//#define true 1
//#define false 0
#define caseSens 1
#define caseInSens 0
#define q 1.60218e-19		/* electron charge*/
#define dicon 8.85418e-14	/* free space dielectric constant*/
#define invh 1.0e16			/* mesh space multiplier */
#define MAXOP 100			/* max size of operand or operator or layer loop stack */
#define NUMBER '0'			/* signal that a number was found */

#define MAX_EXPRESS 500		/* size of the expression string */
#define MAX_SEMINAME 128	/* Max length of semiconductor names */
#define MAX_MESSAGE 1024	/* Max length of message string */
#define MAX_FILENAME 128	/* Max length of filenames */
#define MAX_EXTENSION 30	/* Max length of filename extensions */
#define MAX_KEYWORD 30		/* Max length of keywords */
#define MAX_VOLTSOURCES 10	/* Max number of voltage sources */
#define MAX_IMPLANTS 10		/* Max number of implants */
#define MAX_INPUTLINE 500	/* Max length of input line */

typedef int integer;


//??? int currentfile(struct global_type *pGlobals, struct flag_type *pFlags);


/* Global Function prototypes */
int fieldtrim(char info[], int *startsearch,char field[],char subfield[],int keepCase);
void alertboxWin(char messag[]);
void alertbox(char messag[]);
void lowercase(char input[]);
void infix_postfix(char expression[]);
void script(struct global_type *pGlobals, struct flag_type *pFlags);
void cleanup(void);
int getop(char s[], char input[], int *j, int *sign_possible);
int getSingField(char s[], char input[], int *j);
double numconv(char field[]);
double evaluate(char[],struct global_type *pGlobals); 
struct mesh_type * ymesh(double position,struct global_type *pGlobals);
void val_append(char expression[]); /* function to append val to x, y, z, or w */

int dstebz_(char *range, char *ordering, integer *n, double *vl, double *vu, integer *il, integer *iu, double *abstol, 
	double *d__, double *e, integer *m, integer *nsplit, double *w, int *iblock, integer *isplit, double *work, 
	integer *iwork, integer *info);
int dstein_(int *n, double *d__, double *e, int *m, double *w, int *iblock, 
		int *isplit, double *z__, int *ldz, double *work, int *iwork, int *ifail, int *info);

double QROMB(double (*func)(double), double a, double b);
double dlamch_(char *cmach);
double danger(double E);
double ddanger(double E);
double trapzd(double (*func)(double), double a, double b, double n);
void polint(double xa[],double ya[],int n,double x,double *y,double *dy);






