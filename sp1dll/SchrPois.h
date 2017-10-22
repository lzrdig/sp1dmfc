#pragma once

#define MAX_FILENAME 128 /* Max length of filenames */
#define MAX_EXTENSION 30 /* Max length of filename extensions */

class CInfoDlg;
class CGrphDlg;
class CMtrlDlg;
class CStructDlg;

class CSchrPois
{
public:
	__declspec(dllexport) CSchrPois(void);
	__declspec(dllexport) virtual ~CSchrPois(void);

	__declspec(dllexport) int InitSPobj(void);
	__declspec(dllexport) int LoadMatrlFile(char * fname);

	int *pIwork;
	int *pIsplit;
	int *pIblock;
	double *pWork;
	struct mesh_type *pMesh;
	struct flag_type *pFlags;
	struct global_type *pGlobals;
	char inputFilePath[MAX_FILENAME+MAX_EXTENSION];
	char matrlFilePath[MAX_FILENAME+MAX_EXTENSION];
	char filePath[MAX_FILENAME+MAX_EXTENSION];
      
	void ShowSPdlg ();
	int PostInfo(CEdit * editWndPtr, CString info);
	int PostInfoSP(CString info);
	int SetInfoboxPtr(CInfoDlg * pInfobox);
	int ReleaseInfoboxPtr();
	int ReleaseGrphdlgPtr();
	int ReleaseMtrldlgPtr();
	int ReleaseViewStrctDlgPtr();

	int solve(double loc_converg, struct global_type *pGlobals, struct flag_type *pFlags);
	int onedpoisson(struct global_type *pGlobals, struct flag_type *pFlags);
	void adjust(struct global_type *pGlobals, struct flag_type *pFlags);

	int multiruns(struct global_type *pGlobals, struct flag_type *pFlags);
	
	int boundstates(struct global_type *pGlobals, struct flag_type *pFlags);
	int schrodingersolve(struct global_type *pGlobals, struct flag_type *pFlags);
	int schdoelectrons(struct global_type *pGlobals, struct flag_type *pFlags);
	int schdoholes(struct global_type *pGlobals, struct flag_type *pFlags);
	int schdoheavyholes(struct global_type *pGlobals, struct flag_type *pFlags);
	int schdolightholes(struct global_type *pGlobals, struct flag_type *pFlags);
	int schsavelectroneigens(struct global_type *pGlobals, struct flag_type *pFlags);
	int schsavehholeigens(struct global_type *pGlobals, struct flag_type *pFlags);
	int schsavelholeigens(struct global_type *pGlobals, struct flag_type *pFlags);
	int schcharge(char carrierType, int fixedBounds,struct global_type *pGlobals, 
					struct flag_type *pFlags);

	void charge(struct global_type *, struct flag_type *);
	void mobilecharge(struct mesh_type *, double );
	void fixedcharge(struct mesh_type *, double );
	void sheetcharge(double *, double *, double *, struct global_type *);
	double cvcharge(struct mesh_type *);
	double electrons(struct mesh_type *, double );
	double electronderiv(struct mesh_type *, double );
	double holes(struct mesh_type *, double );
	double holederiv(struct mesh_type *, double );

	void choose(struct global_type *, struct flag_type *);
	void selrun(struct global_type *, struct flag_type *);
	void script(struct global_type *, struct flag_type *);
	void freeMemPrevSimul(struct global_type *pGlobals, struct flag_type *pFlags);

	int initialize(struct global_type *pGlobals, struct flag_type *pFlags);
	int setdefaults(struct global_type *pGlobals, struct flag_type *pFlags);
	int meshInit(struct mesh_type **ppMesh,  struct layer_type *pLayer, struct dummy_layer_type *pDefaultLayer, 
				struct global_type *pGlobals, struct flag_type *pFlags, int *numMeshPts);
	int meshPointInit (struct mesh_type *pMesh, struct layer_type *pLayer, struct dummy_layer_type *pDefaultLayer, 
				struct global_type *pGlobals);
	int meshInitAddSchr(struct mesh_type *pMesh,  struct layer_type *pLayer, 
				struct global_type **ppGlobals, struct flag_type *pFlags);
	int meshInitAddImplants(struct mesh_type *pMesh, struct global_type *pGlobals, struct flag_type *pFlags);
	int meshInitAddSurfaceCondition(struct global_type **ppGlobals, struct flag_type *pFlags);
	int meshInitAddSubstrateCondition(struct global_type **ppGlobals, struct flag_type *pFlags);
	
	int input(struct global_type *pGlobals, struct flag_type *pFlags);
	int getdy(struct global_type *pGlobals);

	int output(char name[],char extension[],int count, struct global_type *pGlobals, struct flag_type *pFlags);
	int writestatus(char filename[],char extension[], int count, struct global_type *pGlobals, struct flag_type *pFlags);
	int CVout(char * CVdata[], int multicountmax, struct global_type *pGlobals);

	void pushLayer(struct layer_stack_type layerStack);
	struct layer_stack_type popLayer();
	struct layer_type * layerloop(struct layer_stack_type layerStack_loc, struct layer_type *pLayer);
	struct mtrl_line_type * parameterInfo(char materialLine[]);
	double MatEval(struct mtrl_line_type * pCurrentInterval, double xval, double yval, double zval, 
					double wval, int *loc_badversion, struct global_type *pGlobals);

	void readlayers(char info[], struct global_type *pGlobals, struct flag_type *pFlags);
	int semisearch(char seminame[],double xval,double yval,double zval,double wval,
			struct dummy_layer_type *pLayer, char simfamily[], struct global_type *pGlobals, 
			struct flag_type *pFlags);
	int lin_grade(char expression[]);

	int checkRightInputFile(struct global_type *pGlobals);
	void fileclose(int unit, struct global_type *pGlobals);
	int fileopen(int unit, char name[MAX_FILENAME], char access, char extension[MAX_EXTENSION], 
						struct global_type *pGlobals, char caption[MAX_FILENAME]);
	int getInput(int unit, char info[], struct global_type *pGlobals);

	void listmaterials(struct global_type *pGlobals, struct flag_type *pFlags);
	void materialparams(struct global_type *pGlobals, struct flag_type *pFlags);
	int loadMaterials(struct global_type *pGlobals, struct flag_type *pFlags);

	CInfoDlg *m_pInfoDlg;
	CGrphDlg *m_pGrphDlg;
	CMtrlDlg *m_pMtrlDlg;
	CStructDlg *m_pStrctDlg;
};
