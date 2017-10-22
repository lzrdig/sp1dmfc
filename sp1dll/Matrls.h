#pragma once

//#include "Poisson\poisp_types.h"
#include "Poisson\poisp_defs.h"
#include "SchrPois.h"

struct mtrl_line_type;
struct mtrl_list;

class CMatrls
{
public:
	CMatrls(void);
	virtual ~CMatrls(void);

	mtrl_list * m_pMtrlListTop;
	FILE *m_pFile[10];
	char m_sMtrlFileName[MAX_FILENAME];
	char materialsversion[14];			/* version of the materials file*/

	int LoadMtrls(char matrlfile[MAX_FILENAME]);
	int GetLineFromFile(int unit, char info[MAX_INPUTLINE]);
	int OpenMtrlFile(int unit, char name[MAX_FILENAME], char access, char extension[MAX_EXTENSION], char caption[MAX_FILENAME]);
	struct mtrl_line_type * ParameterInfo(char materialLine[]);
	double MatEval(struct mtrl_line_type * pCurrentInterval, 
					double xval, double yval, double zval, 
					double wval, int *loc_badversion, double tmprt);
	void ListMtrls(char * txtMatrlsData);
	void PopulateMtrlsTree(CTreeCtrl * pMtrlsTree, double tmprt, double xval, bool evaluate);
};
