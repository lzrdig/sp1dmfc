#include "stdafx.h"
#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "poisp_types.h"
#include "poisp_defs.h"
#include "poisp_keywords.h"


#define MAXVAL 100	/* maximum depth of value stack */

int sp = 0;			/* next free stack position */
int spop = 0;		/* next free op stack position */
double val[MAXVAL];	/* value stack */

/* Sructure for operators with operator name and precedence. */
struct op_type {
	char op_name[40];	/* operator name */
	int op_pres;		/* operator precedence */
};

void push(double);
double pop(void);
struct op_type op[MAXVAL];	/* op stack */
struct op_type pop_op(void); /* pop op stack */
void push_op(struct op_type); /* push op stack */
int iskeyword(char field[]);  /* determine field is a keyword*/
int lin_grade(char expression[]); /* determine if the field represents a linear grading */


/*******************************************************/
int fieldtrim(char in[], int *begin, char field[], char subfield[], int keepCase) {
/*******************************************************/
	  
/*	This routine strips off the first field after start and returns  
 	it in field.  Fields are delimited by spaces or tabs.  The variable start is  
 	updated so that a subsequent call will find the next field. */

	int i,j,length,first,beginfield,endfield,type,sign_possible,beginlast;
	int charCounter;
	char tmpfield[MAX_EXPRESS];
  
	/*  set the initial conditions */

	strcpy(field,"");
	strcpy(subfield,"");
	length=0;
	beginfield=-1;
	endfield=-1;
	first = true;	/* set the first time flag */
	sign_possible = true;
	i=*begin;
	beginlast = *begin;
	j=0;
  
/* first look for the first non-blank character in the input string, store its position   
   in beginfield, and reset the first flag.  Then look for the next blank, tab, comma,  
   or = and store its position in endfield. */

	if (in[*begin] == '\0') {
		field[0] = '\0';
		return 0;
	}
	type = getop(field,in,begin,&sign_possible);
	if (type == '\0' || type == '\n') return 0; /* kick out if no field is found*/
	beginlast = *begin;
	
	
	type = getop(tmpfield,in,begin,&sign_possible);
	lowercase(tmpfield);
	charCounter = 0;
	while(!iskeyword(tmpfield) && type != '\0' && type != '\n' && (strlen(subfield)+strlen(tmpfield) < MAX_EXTENSION)) {
		strcat(subfield,tmpfield);  /*if the found field is not a keyword, append it to subfield*/
		beginlast = *begin;
		type = getop(tmpfield,in,begin,&sign_possible);
		lowercase(tmpfield);
		charCounter = strlen(subfield);
	}
	*begin = beginlast;
	
	length = (int)strlen(field);
	    
	 /*convert any alphabetic characters to lower case */
	if (!keepCase) {
		lowercase(field);
		lowercase(subfield);
	}
	  	
	return length;
}


/*****************************************************/
int getop(char s[], char input[], int *j, int *sign_possible)
/*****************************************************/
/* getop: get next operator or numeric operand */
{
	/* sign_possible is a rather strange flag.  It is set "true" if there is a possiblity of a leading "-" being
	the sign of a number and not an operator, and if found will insert a chs operator.  If set "false" then "-"
	signs are treated as operators. */
	
	int i, c, chs;
	
	chs = false;
	while ((s[0] = c = input[(*j)++]) == ' ' || c == '\t' || c == '=')
		;
	s[1] = '\0';
	if (!isdigit(c) && c != '.' && (((c == '-' || c == '+') && !*sign_possible) || c=='*' || c=='/' || c=='^' || c=='(' || c==')' || c=='{' || c=='}' || c=='\0')) {
		*sign_possible = false;
		return c;	/* not a number. Single charater operator or paren*/
	}
	i=0;
	if (!isdigit(c) && c != '.' && c != '-' && c != '+') {
		/* collect operator or variable name */
		while ((s[++i] = c = input[(*j)++]) != ' ' && c != '.' && c != '-' && c != '+' && c != '*' && c != '/' 
				&& c != '^' && c != '(' && c != '{' && c != '\t' && c != ')' && c != '}' && c != '\0' && c != '\n'&& c != '=')
				;
		*sign_possible = false;
		(*j)--;
		s[i] = '\0';
		c = s[i-1];
		return c;	/* not a number */
	}
	if (isdigit(c) || c == '-' || c== '+') { /* collect integer part */
		if (c == '-') chs = true;
		if (c == '-' || c== '+') s[0] = input[(*j)++];
		while (isdigit(s[++i] = c = input[(*j)++]))
			;
	}
	if (c == '.')	/* collect fraction part */
		while (isdigit(s[++i] = c = input[(*j)++]))
			;
	if (c == 'e'|| c == 'E') 
	{
		s[i] = 'e';
		if (input[*j] == '+' || input[*j] == '-')
			s[++i] = input[(*j)++];
		while (isdigit(s[++i] = c = input[(*j)++])) /* collect exponent */
			;
	}
	(*j)--;
	s[i] = '\0';
	if (chs) strcat(s," chs");
	*sign_possible = false;
	return NUMBER;
}



/*****************************************************/
int getSingField(char s[], char input[], int *j) {
/*****************************************************/
/* getSingField: get a single operator */
	/* This routine returns the contents of a single field which was delimited by spaces tabs, or = signs */
	
	int i, c;
	
	/*while ((s[0] = c = input[(*j)++]) == ' ' || c == '\t' || c == '=') /* skip leading whitespace */
		/*;*/
	while ((c=input[(*j)]) == ' ' || c == '\t' || c == '=')
		(*j)++;
	/*s[1] = '\0';*/
	s[0] = '\0';
	if (input[(*j)] == '\0')
		return 0;	/* kick out if the end of the string */
		
	i=0;
	/* collect operator or variable name */
	while ((s[i++] = c = input[(*j)++]) != ' ' && c != '\t' && c != '\0' && c != '\n' && c != '=')
				;
	(*j)--;
	s[i-1] = '\0';
	if (isdigit(s[0]) || s[0]=='.' || (s[0]=='-' && (isdigit(s[1]) || s[1]=='.')))
		return NUMBER;
	else
		return s[0];	/* not a number */
}


/*************************************/	
static int iskeyword(char value[]) {
/*************************************/	
	
	/*This binary search determines if the field contains a keyword*/
	int position;
	int begin = 0; 
	int end;
	int cond = 0;

	if (value[0] == 'v') {
		if (isdigit(value[1])) return 1; /*special case for voltage sources */
	}
	
	end = (sizeof(keywords)/sizeof(keywords[0])) - 1;
	while(begin <= end) {
		position = (begin + end) / 2;
		if((cond = strcmp(keywords[position], value)) == 0)
   			return 1; /* return 1 if keyword is found*/
		else if(cond < 0)
			begin = position + 1;
		else
			end = position - 1;
	}

	return 0;
}
/*************************************/		
void val_append(char expression[]) {

/**************************************************/

/* This routine replaces x, y, z, or w with xval, yval, zval, or wval in a string expression.
	   Used to eliminate confusion between mole-fraction and real space coordinates */
	   
	int i,fieldlength;
	char dummy[MAX_EXPRESS],var[MAX_EXPRESS];
	
	strcpy(dummy,"");
	i=0;
	fieldlength = getSingField(var,expression,&i); /* get first operator in this expression */
	
	while(fieldlength != '\0'){
		switch(var[0]){
			case 'x':
				strcat(dummy,"xval");
				break;
			case 'y':
				strcat(dummy,"yval");
				break;
			case 'z':
				strcat(dummy,"zval");
				break;
			case 'w':
				strcat(dummy,"wval");
				break;
			default:
				strcat(dummy,var);
				break;
		}
		strcat(dummy," ");
		fieldlength = getSingField(var,expression,&i); /* get next operator in this expression */
	}
	strcpy(expression,dummy);
}

/*************************************/	
double numconv(char number[MAX_KEYWORD]) {
/*************************************/

/* Convert string to number, converting to Ang or cm-3 */
	
	char *pchar;
	double convfac,dnum;

	dnum = strtod(number,&pchar);

	if (*pchar == 'n' && *(pchar+1) == 'm')   
		/*thickness is given in nm */
		convfac = 10.0e0;
	else if (*pchar == 'u' && *(pchar+1) == 'm')   
		/*thickness is given in microns */
		convfac = 10000.0e0; 
	else if (*pchar == 'm' && *(pchar+1) == 'm')   
		/*thickness is given in millimeters */
		convfac = 1.0e7; 
	else if (*pchar == 'm' && *(pchar+1) == '-' && *(pchar+2) == '3')   
		/*doping concentration given in m^3 */
		convfac = 1.0e6; 
	else if (*pchar == 'm' && *(pchar+1) == '-' && *(pchar+2) == '2')   
		/*doping concentration given in m^2 */
		convfac = 1.0e4;
	else  
		/*thickness was given in angstroms or doping in cm-3 */
		convfac = 1.0e0;
	
	dnum = convfac*dnum;	/* convert to Ang, cm-2, or cm-3 */
	
	return dnum;
}
/*****************************************************/	
void lowercase(char input[]) {	 
/*****************************************************/	 

/*   This routine changes all letters of an input string to lower case */
	
	int i;
	
	i=0;
	while (input[i] != '\0') {
		input[i] = tolower(input[i]);	/*convert character to lower case*/
		i++;
	}
}
/*****************************************************/	
double d_sign(double *a, double *b) {	 

/*****************************************************/	 

/*  This function mimics the fortran sign function */
	
	if (*b <= 0.0)
		return -fabs(*a);
	else
		return fabs(*a);
}


/* push f onto value stack */
void push(double f)
{
	char message[MAX_MESSAGE];
	if (sp < MAXVAL)
		val[sp++] = f;
	else {
		strcpy(message,"error: stack full, can't push new value\n");
		alertbox(message);
	}
}


/* pop: pop and return top value from stack */
double pop(void)
{
	char message[MAX_MESSAGE];
	if (sp > 0)
		return val[--sp];
	else {
		strcpy(message,"error: stack empty\n");
		alertbox(message);
		return 0.0;
	}
}

/* push operator onto operator stack */
void push_op(struct op_type op_loc)
{
	char message[MAX_MESSAGE];
	if (spop < MAXVAL)
		op[spop++] = op_loc;
	else {
		strcpy(message,"error: stack full, can't push new operator\n");
		alertbox(message);
	}
}


/* pop: pop and return top value from op stack */
struct op_type pop_op(void)
{
	struct op_type dummy;
	char message[MAX_MESSAGE];

	strcpy(dummy.op_name,"");
	dummy.op_pres=0;

	if (spop > 0)
		return op[--spop];
	else {
		strcpy(message,"error: stack empty\n");
		alertbox(message);
		return dummy;
	}
}
/*****************************************************/
int lin_grade(char expression[]) {
/*****************************************************/
  
/*  This routine seperates out the fields '-' delimiter, 
returning a 1 if a call for grading is found  */

	char gradstart[MAX_EXPRESS],s[MAX_EXPRESS],gradstop[MAX_EXPRESS];
	double a,b;
	int type,j,sign_possible;
	struct global_type *pDummyGlobals;
	
	pDummyGlobals = NULL;

	if (expression[0] == '\0') return 0; /* empty string */
	
	j=0;
	sign_possible = true;
	type = getop(s,expression,&j,&sign_possible);
	if (type != NUMBER) return 0;
	strcpy(gradstart,s);
	sign_possible = false;
	type = getop(s,expression,&j,&sign_possible);
	if (s[0] != '-') return 0;
	sign_possible = true;
	type = getop(s,expression,&j,&sign_possible);
	if (type != NUMBER) return 0;
	strcpy(gradstop,s);
	type = getop(s,expression,&j,&sign_possible);
	if (type != '\0') return 0;

/* At this point it has been determined that this is a request for linear grading.
produce the post-fix expression for the grading, and return it in expression */
		
	a = evaluate(gradstart,pDummyGlobals);
	b = evaluate(gradstop,pDummyGlobals);
	sprintf(expression, "%11.4e", fabs(b-a));
	strcat(expression," y *");
	sprintf(gradstart, "%11.4e",a);
	strcat(expression,gradstart);
	if ((b-a) >= 0.0)
		strcat(expression," +");
	else
		strcat(expression," -");
	return 1;
}

/*****************************************************/
void infix_postfix(char expression[]) {
/*****************************************************/

	int sign_possible,type,j,precedence;
	int operat(char s[]);
	int op_prec(char s[]);
	struct op_type op_loc;
	char s[MAX_KEYWORD], dummy[MAX_EXPRESS];
	
	if(lin_grade(expression)) return;	/* check for linear grading, return linear grading postfix expression*/
	
	sign_possible = true;
	j=0;
	dummy[0] = '\0';
	while ((type = getop(s,expression,&j,&sign_possible)) != '\0') {
	
		if ((type == NUMBER || !operat(s)) && type != ')' && type != '}' && type != '(' && type != '{') {
			strcat(dummy,s);
			strcat(dummy," ");
		}
		
		else if (!strcmp(s,"(") || !strcmp(s,"{")) {
			strcpy(op_loc.op_name,"(");
			op_loc.op_pres = 9;
			push_op(op_loc);
			sign_possible = true;
		}
		else if (!strcmp(s,")") || !strcmp(s,"}")) {
			op_loc = pop_op();
			while (strcmp(op_loc.op_name,"(")) {
				strcat(dummy,op_loc.op_name);
				strcat(dummy," ");
				op_loc = pop_op();
			}
		}
		
		else {
			precedence = op_prec(s);
			if (spop>0) {
				op_loc = pop_op();
				if (precedence >= op_loc.op_pres) {
					strcat(dummy,op_loc.op_name); /* if operator is of greater or equal precedence, append to result */
					strcat(dummy," ");
					while (spop > 0 && precedence >= (op_loc=pop_op()).op_pres ) {
						strcat(dummy,op_loc.op_name);
						strcat(dummy," ");
					}
					if (precedence < op_loc.op_pres)
						push_op(op_loc); /* if operator was not appended to string, push back on stack */
				}
				else
					push_op(op_loc);
			}
			strcpy(op_loc.op_name,s);
			op_loc.op_pres = precedence;
			push_op(op_loc);
		}
	}
	
	while (spop > 0) {
		op_loc = pop_op();
		strcat(dummy,op_loc.op_name);
		strcat(dummy," ");
	}
	
	strcpy(expression, dummy); /* copy the converted post-fix expression to the char var expression */
	return;
}


/*****************************************************/
int op_prec(char s[]) {
/*****************************************************/

	if (s[0]=='^') return 1;

	if (s[0]=='*' || s[0]=='/') return 2;

	if (s[0] == '-' || s[0] == '+') return 3;
	
	return 0;
}


/*****************************************************/
int operat(char s[]) {
/*****************************************************/

	if (s[0] == '-' || s[0] == '+' || s[0]=='*' || s[0]=='/' || s[0]=='^') return 1;
	
	if(!strcmp(s,"exp") || !strcmp(s,"log") || !strcmp(s,"sin") || !strcmp(s,"asin") || !strcmp(s,"cos") || !strcmp(s,"acos") || !strcmp(s,"tan")
			|| !strcmp(s,"atan") || !strcmp(s,"sinh") || !strcmp(s,"cosh") || !strcmp(s,"tanh") || !strcmp(s,"abs") || !strcmp(s,"sqrt") 
			|| !strcmp(s,"log10"))
		return 1;
	else
		return 0;
}

/*****************************************************/	
struct mesh_type *ymesh(double position, struct global_type *pGlobals) {
/*****************************************************/	
 
/* Given a real space coordinate, position, ymesh returns the index of the nearest mesh point */
	
	int meshfound;
	struct mesh_type *pMesh,*ypos;
	
	meshfound = false;
	pMesh = pGlobals->pTopMesh;
	ypos=0;
	
	do {
		if (pMesh->pDownMesh != NULL) {
			if ((position >= pMesh->meshcoord) && (position <= pMesh->pDownMesh->meshcoord)) {
				if ((pMesh->pDownMesh->meshcoord-position) < (position-pMesh->meshcoord)) 
					ypos = pMesh->pDownMesh;
				else
					ypos = pMesh;
	      
				meshfound = true;
			}
		}
	} while((pMesh=pMesh->pDownMesh) != NULL);
	
	if (!meshfound) 
		ypos = pGlobals->pBottomMesh;	/* allow for overflow */
	if (position < 0.0e0) 
		ypos = pGlobals->pTopMesh;	/* allow for underflow */
		
	return ypos;
}
/*****************************************************/	
double evaluate(char expression[], struct global_type *pGlobals)
/*****************************************************/	
{
	int type,j,sign_possible;
	double op2;
	char s[MAXOP], message[MAX_MESSAGE];
	
	j=0;
	sign_possible = false;
	
	while ((type = getSingField(s,expression,&j)) != '\0') {
		switch (type) {
		case NUMBER:
			push(atof(s));
			break;
		case '+':
			push(pop() + pop());
			break;
		case '*':
			push(pop() * pop());
			break;
		case '^':
			op2 = pop();
			push(pow(pop(),op2));
			break;
		case '-':
			op2 = pop();
			push(pop() - op2);
			break;
		case '/':
			op2 = pop();
			if (op2 != 0.0)
				push(pop() / op2);
			else 
			{
				strcpy(message,"error: zero divisor (evaluate)\n");
				alertboxWin(message);	
			}
			break;
		default: 
			if (!strcmp(s,"chs")) push(-1.0 * pop());
			else if (!strcmp(s,"y")) push(pGlobals->y_loc);
			else if (!strcmp(s,"xval")) push(pGlobals->xval);
			else if (!strcmp(s,"yval")) push(pGlobals->yval);
			else if (!strcmp(s,"wval")) push(pGlobals->wval);
			else if (!strcmp(s,"zval")) push(pGlobals->zval);
			else if (!strcmp(s,"temp")) push(pGlobals->temperature);
			else if (!strcmp(s,"yabs")) push(pGlobals->y);
			else if (!strcmp(s,"exp")) push(exp(pop()));
			else if (!strcmp(s,"log")) push(log(pop()));
			else if (!strcmp(s,"sin")) push(sin(pop()));
			else if (!strcmp(s,"asin")) push(asin(pop()));
			else if (!strcmp(s,"cos")) push(cos(pop()));
			else if (!strcmp(s,"acos")) push(acos(pop()));
			else if (!strcmp(s,"tan")) push(tan(pop()));
			else if (!strcmp(s,"sinh")) push(sinh(pop()));
			else if (!strcmp(s,"cosh")) push(cosh(pop()));
			else if (!strcmp(s,"tanh")) push(tanh(pop()));
			else if (!strcmp(s,"abs")) push(abs((const long)pop()));
			else if (!strcmp(s,"log10")) push(log10(pop()));
			else if (!strcmp(s,"sqrt")) push(sqrt(pop()));
			else 
			{
				strcpy(message,"error: unknown command (evaluate) \n");
				alertboxWin(message);	
			}
			break;
		}
	}
	return pop();
}

