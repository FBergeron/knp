/*====================================================================

			       ʸ̮����

                                         Daisuke Kawahara 2001. 7. 13

    $Id$
====================================================================*/
#include "knp.h"

float maxscore;
float maxrawscore;
SENTENCE_DATA *maxs;
int maxi, maxpos;
char *maxtag, *maxfeatures;
int Bcheck[BNST_MAX];

/* #define USE_RAWSCORE */

#define RANK0	0
#define RANK1	1
#define RANK2	2
#define RANK3	3
#define RANK4	4
#define RANKP	5

#define	PLOC_PV		0x0001
#define	PLOC_PARENTV	0x0010
#define	PLOC_CHILDV	0x0100
#define	PLOC_CHILDPV	0x0300
#define	PLOC_NPARENTV	0x0002	/* ����̾��ο� */
#define	PLOC_NPARENTCV	0x0006	/* ����̾��οƤν�°�� */
#define	PLOC_NOTHERS	0x000e

/* �� ���֤ο����Ѥ����顢LOC_NUMBER(const.h)���Ѥ��뤳�� */

#define	NON_IMPORTANT_WEIGHT	0.6

char *ExtraTags[] = {"�оݳ�", "��;�", "������-��", "������-����", ""};

char *ETAG_name[] = {
    "", "", "������:��", "��;�", "������:����", 
    "��ʸ", "��ʸ", "�оݳ�"};

float	AssignReferentThreshold = 0.67;
float	AssignReferentThresholdDecided = 0.50;
float	AssignGaCaseThreshold = 0.67;	/* ���ʤ�ڼ��ΰ��̡ۤˤ������� */
float	AssignReferentThresholdHigh = 0.80;
float	AssignReferentThresholdAnonymousThing = 0.90;

float	AssignReferentThresholdForSVM = -0.9999;
float	AntecedentDecideThreshold = 0.70;

int	EllipsisSubordinateClauseScore = 10;

ALIST alist[TBLSIZE];		/* ��󥯤��줿ñ��+���٤Υꥹ�� */
ALIST banlist[TBLSIZE];		/* �Ѹ����Ȥζػ�ñ�� */
PALIST palist[TBLSIZE];		/* �Ѹ��ȳ����ǤΥ��åȤΥꥹ�� */
PALIST **ClauseList;		/* ��ʸ�μ��� */
int ClauseListMax = 0;

extern int	EX_match_subject;

#define CASE_ORDER_MAX	3
char *CaseOrder[CASE_ORDER_MAX][4] = {
    {"��", "��", "��", ""}, 
    {"��", "��", "��", ""}, 
    {"��", "��", "��", ""}, 
};
int DiscAddedCases[PP_NUMBER] = {END_M};


/*==================================================================*/
		       void InitAnaphoraList()
/*==================================================================*/
{
    memset(alist, 0, sizeof(ALIST)*TBLSIZE);
    memset(banlist, 0, sizeof(ALIST)*TBLSIZE);
    memset(palist, 0, sizeof(PALIST)*TBLSIZE);
}

/*==================================================================*/
		void InitEllipsisMGR(ELLIPSIS_MGR *em)
/*==================================================================*/
{
    memset(em, 0, sizeof(ELLIPSIS_MGR));
}

/*==================================================================*/
	       void ClearEllipsisMGR(ELLIPSIS_MGR *em)
/*==================================================================*/
{
    clear_feature(&(em->f));
    InitEllipsisMGR(em);
}

/*==================================================================*/
		 void ClearAnaphoraList(ALIST *list)
/*==================================================================*/
{
    int i;
    ALIST *p, *next;
    for (i = 0; i < TBLSIZE; i++) {
	if (list[i].key) {
	    free(list[i].key);
	    list[i].key = NULL;
	}
	if (list[i].next) {
	    p = list[i].next;
	    list[i].next = NULL;
	    while (p) {
		free(p->key);
		next = p->next;
		free(p);
		p = next;
	    }
	}
	list[i].count = 0;
    }
}

/*==================================================================*/
	     void RegisterAnaphor(ALIST *list, char *key)
/*==================================================================*/
{
    ALIST *ap;

    ap = &(list[hash(key, strlen(key))]);
    if (ap->key) {
	ALIST **app;
	app = &ap;
	do {
	    if (!strcmp((*app)->key, key)) {
		(*app)->count++;
		return;
	    }
	    app = &((*app)->next);
	} while (*app);
	*app = (ALIST *)malloc_data(sizeof(ALIST), "RegisterAnaphor");
	(*app)->key = strdup(key);
	(*app)->count = 1;
	(*app)->next = NULL;
    }
    else {
	ap->key = strdup(key);
	ap->count = 1;
    }
}

/*==================================================================*/
	       int CheckAnaphor(ALIST *list, char *key)
/*==================================================================*/
{
    ALIST *ap;

    ap = &(list[hash(key, strlen(key))]);
    if (!ap->key) {
	return 0;
    }
    while (ap) {
	if (!strcmp(ap->key, key)) {
	    return ap->count;
	}
	ap = ap->next;
    }
    return 0;
}

/*==================================================================*/
		       int CheckBasicPP(int pp)
/*==================================================================*/
{
    /* ʣ�缭�ʤɤγʤϽ��� */
    if (pp == END_M || pp > 8 || pp < 0) {
	return 0;
    }
    return 1;
}

/*==================================================================*/
 void StoreCaseComponent(CASE_COMPONENT **ccpp, char *word, int flag)
/*==================================================================*/
{
    /* �����Ǥ���Ͽ���� */

    while (*ccpp) {
	/* ���Ǥ���Ͽ����Ƥ���Ȥ� */
	if (!strcmp((*ccpp)->word, word)) {
	    /* ������ά�ط��Ǻ����ʴط��ʤ顢���٤Ƴʴط��ˤ��� */
	    if ((*ccpp)->flag == EREL && flag == CREL) {
		(*ccpp)->flag = CREL;
	    }
	    (*ccpp)->count++;
	    return;
	}
	ccpp = &((*ccpp)->next);
    }
    *ccpp = (CASE_COMPONENT *)malloc_data(sizeof(CASE_COMPONENT), "StoreCaseComponent");
    (*ccpp)->word = strdup(word);
    (*ccpp)->count = 1;
    (*ccpp)->flag = flag;
    (*ccpp)->next = NULL;
}

/*==================================================================*/
void RegisterClause(PALIST **list, char *key, int pp, char *word, int flag)
/*==================================================================*/
{
    /* �������Ͽ���� */

    if (CheckBasicPP(pp) == 0) {
	return;
    }

    if (*list == NULL) {
	*list = (PALIST *)malloc_data(sizeof(PALIST), "RegisterClause");
	(*list)->key = strdup(key);
	memset((*list)->cc, 0, sizeof(CASE_COMPONENT *)*CASE_MAX_NUM);
	(*list)->next = NULL;
    }
    StoreCaseComponent(&((*list)->cc[pp]), word, flag);
}

/*==================================================================*/
void RegisterLastClause(int Snum, char *key, int pp, char *word, int flag)
/*==================================================================*/
{
    /* ��ʸ�μ������Ͽ���� */

    /* ʸ�ֹ������ź�����Ф��� 1 ¿�� */
    Snum--;

    while (Snum >= ClauseListMax) {
	int i, start;
	start = ClauseListMax;
	if (ClauseListMax == 0) {
	    ClauseListMax = 1;
	    ClauseList = (PALIST **)malloc_data(sizeof(PALIST *)*(ClauseListMax), 
						"RegisterLastClause");
	}
	else {
	    ClauseList = (PALIST **)realloc_data(ClauseList, 
						 sizeof(PALIST *)*(ClauseListMax <<= 1), 
						 "RegisterLastClause");
	}
	/* ����� */
	for (i = start; i < ClauseListMax; i++) {
	    *(ClauseList+i) = NULL;
	}
    }
    RegisterClause(ClauseList+Snum, key, pp, word, flag);
}

/*==================================================================*/
	  int CheckClause(PALIST *list, int pp, char *word)
/*==================================================================*/
{
    int i;
    CASE_COMPONENT *ccp;

    if (list == NULL) {
	return 0;
    }

    if (CheckBasicPP(pp) == 0) {
	return 0;
    }

    /* ���ߤϳʤΰ��פϥ����å����ʤ� */
    for (i = 0; i < CASE_MAX_NUM; i++) {
	ccp = list->cc[i];
	while (ccp) {
	    if (!strcmp(ccp->word, word)) {
		return 1;
	    }
	    ccp = ccp->next;
	}
    }
    return 0;
}

/*==================================================================*/
	  int CheckLastClause(int Snum, int pp, char *word)
/*==================================================================*/
{
    if (Snum < 1 || Snum > ClauseListMax) {
	return 0;
    }
    return CheckClause(*(ClauseList+Snum-1), pp, word);
}

/*==================================================================*/
   void RegisterPredicate(char *key, int voice, int cf_addr, 
			  int pp, char *word, int flag)
/*==================================================================*/
{
    /* �Ѹ��ȳ����Ǥ򥻥åȤ���Ͽ���� */

    PALIST *pap;

    if (CheckBasicPP(pp) == 0) {
	return;
    }

    pap = &(palist[hash(key, strlen(key))]);
    if (pap->key) {
	PALIST **papp;
	papp = &pap;
	do {
	    if (!strcmp((*papp)->key, key) && 
		(*papp)->voice == voice && 
		(*papp)->cf_addr == (*papp)->cf_addr) {
		StoreCaseComponent(&((*papp)->cc[pp]), word, flag);
		return;
	    }
	    papp = &((*papp)->next);
	} while (*papp);
	*papp = (PALIST *)malloc_data(sizeof(PALIST), "RegisterPredicate");
	(*papp)->key = strdup(key);
	(*papp)->voice = voice;
	(*papp)->cf_addr = cf_addr;
	memset((*papp)->cc, 0, sizeof(CASE_COMPONENT *)*CASE_MAX_NUM);
	StoreCaseComponent(&((*papp)->cc[pp]), word, flag);
	(*papp)->next = NULL;
    }
    else {
	pap->key = strdup(key);
	pap->voice = voice;
	pap->cf_addr = cf_addr;
	StoreCaseComponent(&(pap->cc[pp]), word, flag);
    }
}

/*==================================================================*/
     int CheckPredicate(char *key, int voice, int cf_addr, 
			int pp, char *word)
/*==================================================================*/
{
    PALIST *pap;
    CASE_COMPONENT *ccp;

    if (CheckBasicPP(pp) == 0) {
	return 0;
    }

    pap = &(palist[hash(key, strlen(key))]);
    if (!pap->key) {
	return 0;
    }
    while (pap) {
	if (!strcmp(pap->key, key) && 
	    pap->voice == voice && 
	    pap->cf_addr == cf_addr) {
	    ccp = pap->cc[pp];
	    while (ccp) {
		if (!strcmp(ccp->word, word)) {
		    /* �ʴط� */
		    if (ccp->flag == CREL) {
			return 2;
		    }
		    return 1;
		}
		ccp = ccp->next;
	    }
	    return 0;
	}
	pap = pap->next;
    }
    return 0;
}

/*==================================================================*/
		void ClearSentence(SENTENCE_DATA *s)
/*==================================================================*/
{
    free(s->mrph_data);
    free(s->bnst_data);
    free(s->para_data);
    free(s->para_manager);
    if (s->cpm)
	free(s->cpm);
    if (s->cf)
	free(s->cf);
    if (s->KNPSID)
	free(s->KNPSID);
    if (s->Best_mgr) {
	clear_mgr_cf(s);
	free(s->Best_mgr);
    }
}

/*==================================================================*/
		void ClearSentences(SENTENCE_DATA *sp)
/*==================================================================*/
{
    int i;
    for (i = 0; i < sp->Sen_num-1; i++) {
	ClearSentence(sentence_data+i);
    }
    sp->Sen_num = 1;
    ClearAnaphoraList(alist);
    /* palist �� clear */
    /* ClauseList �� clear */
    InitAnaphoraList();
}

/*==================================================================*/
		 void InitSentence(SENTENCE_DATA *s)
/*==================================================================*/
{
    int i, j;

    s->mrph_data = (MRPH_DATA *)malloc_data(sizeof(MRPH_DATA)*MRPH_MAX, "InitSentence");
    s->bnst_data = (BNST_DATA *)malloc_data(sizeof(BNST_DATA)*BNST_MAX, "InitSentence");
    s->para_data = (PARA_DATA *)malloc_data(sizeof(PARA_DATA)*PARA_MAX, "InitSentence");
    s->para_manager = (PARA_MANAGER *)malloc_data(sizeof(PARA_MANAGER)*PARA_MAX, "InitSentence");
    s->Best_mgr = (TOTAL_MGR *)malloc_data(sizeof(TOTAL_MGR), "InitSentence");
    s->Sen_num = 0;
    s->Mrph_num = 0;
    s->Bnst_num = 0;
    s->New_Bnst_num = 0;
    s->KNPSID = NULL;
    s->Comment = NULL;
    s->cpm = NULL;
    s->cf = NULL;

    for (i = 0; i < MRPH_MAX; i++)
	(s->mrph_data+i)->f = NULL;
    for (i = 0; i < BNST_MAX; i++)
	(s->bnst_data+i)->f = NULL;
    for (i = 0; i < PARA_MAX; i++) {
	for (j = 0; j < RF_MAX; j++) {
	    (s->para_data+i)->f_pattern.fp[j] = NULL;
	}
    }

    init_mgr_cf(s->Best_mgr);
}

/*==================================================================*/
	  SENTENCE_DATA *PreserveSentence(SENTENCE_DATA *sp)
/*==================================================================*/
{
    /* ʸ���Ϸ�̤��ݻ� */

    int i, j;
    SENTENCE_DATA *sp_new;

    /* ���Ū���� */
    if (sp->Sen_num > 256) {
	fprintf(stderr, "Sentence buffer overflowed!\n");
	ClearSentences(sp);
    }

    sp_new = sentence_data + sp->Sen_num - 1;
    sp_new->Sen_num = sp->Sen_num;

    sp_new->Mrph_num = sp->Mrph_num;
    sp_new->mrph_data = (MRPH_DATA *)malloc_data(sizeof(MRPH_DATA)*sp->Mrph_num, 
						 "MRPH DATA");
    for (i = 0; i < sp->Mrph_num; i++) {
	sp_new->mrph_data[i] = sp->mrph_data[i];
    }

    sp_new->Bnst_num = sp->Bnst_num;
    sp_new->bnst_data = 
	(BNST_DATA *)malloc_data(sizeof(BNST_DATA)*(sp->Bnst_num + sp->New_Bnst_num), 
				 "BNST DATA");
    for (i = 0; i < sp->Bnst_num + sp->New_Bnst_num; i++) {

	sp_new->bnst_data[i] = sp->bnst_data[i]; /* ������bnst_data�򥳥ԡ� */

	/* SENTENCE_DATA �� �� sp ��, MRPH_DATA ����ФȤ��ƻ��äƤ���    */
	/* Ʊ���� sp �Υ��ФǤ��� BNST_DATA �� MRPH_DATA ����ФȤ���   */
        /* ���äƤ��롣                                                     */
	/* �ǡ�ñ�� BNST_DATA �򥳥ԡ������������ȡ�BNST_DATA ��� MRPH_DATA */
        /* ��, sp �Τۤ��� MRPH_DATA �򺹤����ޤޥ��ԡ������ */
	/* ��äơ��ʲ��ǥݥ��󥿥��ɥ쥹�Τ��������        */


        /*
             sp -> SENTENCE_DATA                                              sp_new -> SENTENCE_DATA 
                  +-------------+				                   +-------------+
                  |             |				                   |             |
                  +-------------+				                   +-------------+
                  |             |				                   |             |
       BNST_DATA  +=============+		   ����������������������������    +=============+ BNST_DATA
                0 |             |��������������������                            0 |             |
                  +-------------+                ����		                   +-------------+
                1 |             |              BNST_DATA	                 1 |             |
                  +-------------+                  +-------------+                 +-------------+
                  |   ������    |	           |             |                 |   ������    |
                  +-------------+	           +-------------+                 +-------------+
                n |             |  ���� MRPH_DATA  |* mrph_ptr   |- ��           n |             |
                  +=============+  ��	           +-------------+  ��             +=============+
                  |             |  ��	MRPH_DATA  |* settou_ptr |  ��             |             |
       MRPH_DATA  +=============+  ��	           +-------------+  ��             +=============+ MRPH_DATA
                0 | * mrph_data |  ��	MRPH_DATA  |* jiritu_ptr |  �� - - - - - 0 | * mrph_data |
                  +-------------+  ��              +-------------+    ��           +-------------+
                  |   ������    |����	 			      ��           |   ������    |
                  +-------------+      		                      ��           +-------------+
                n | * mrph_data |	 			      ��         n | * mrph_data |
                  +=============+	 			      ��           +=============+
                                                                      ��
		                                            ñ�˥��ԡ������ޤޤ���,
		                                            sp_new->bnst_data[i] ��
		                                      	    mrph_data ��, sp �Υǡ�����
		                                            �ؤ��Ƥ��ޤ���
		                                            ���Υǡ�����¤���ݤĤ���ˤϡ�
		                                            ��ʬ����(sp_new)�Υǡ���(����)
		                              		    ��ؤ��褦��,��������ɬ�פ����롣
	*/



	sp_new->bnst_data[i].mrph_ptr = sp_new->mrph_data + (sp->bnst_data[i].mrph_ptr - sp->mrph_data);
	if (sp->bnst_data[i].settou_ptr)
	    sp_new->bnst_data[i].settou_ptr = sp_new->mrph_data + (sp->bnst_data[i].settou_ptr - sp->mrph_data);
	sp_new->bnst_data[i].jiritu_ptr = sp_new->mrph_data + (sp->bnst_data[i].jiritu_ptr - sp->mrph_data);
	if (sp->bnst_data[i].fuzoku_ptr)
	sp_new->bnst_data[i].fuzoku_ptr = sp_new->mrph_data + (sp->bnst_data[i].fuzoku_ptr - sp->mrph_data);
	if (sp->bnst_data[i].parent)
	    sp_new->bnst_data[i].parent = sp_new->bnst_data + (sp->bnst_data[i].parent - sp->bnst_data);
	for (j = 0; sp_new->bnst_data[i].child[j]; j++) {
	    sp_new->bnst_data[i].child[j] = sp_new->bnst_data + (sp->bnst_data[i].child[j] - sp->bnst_data);
	}
	if (sp->bnst_data[i].pred_b_ptr) {
	    sp_new->bnst_data[i].pred_b_ptr = sp_new->bnst_data + (sp->bnst_data[i].pred_b_ptr - sp->bnst_data);
	}
    }

    if (sp->KNPSID)
	sp_new->KNPSID = strdup(sp->KNPSID);
    else
	sp_new->KNPSID = NULL;

    sp_new->para_data = (PARA_DATA *)malloc_data(sizeof(PARA_DATA)*sp->Para_num, 
				 "PARA DATA");
    for (i = 0; i < sp->Para_num; i++) {
	sp_new->para_data[i] = sp->para_data[i];
	sp_new->para_data[i].manager_ptr += sp_new->para_manager - sp->para_manager;
    }

    sp_new->para_manager = 
	(PARA_MANAGER *)malloc_data(sizeof(PARA_MANAGER)*sp->Para_M_num, 
				    "PARA MANAGER");
    for (i = 0; i < sp->Para_M_num; i++) {
	sp_new->para_manager[i] = sp->para_manager[i];
	sp_new->para_manager[i].parent += sp_new->para_manager - sp->para_manager;
	for (j = 0; j < sp_new->para_manager[i].child_num; j++) {
	    sp_new->para_manager[i].child[j] += sp_new->para_manager - sp->para_manager;
	}
	sp_new->para_manager[i].bnst_ptr += sp_new->bnst_data - sp->bnst_data;
    }
    return sp_new;
}

/*==================================================================*/
      void PreserveCPM(SENTENCE_DATA *sp_new, SENTENCE_DATA *sp)
/*==================================================================*/
{
    int i, j, num, cfnum = 0;

    /* �ʲ��Ϸ�̤���¸ */
    sp_new->cpm = 
	(CF_PRED_MGR *)malloc_data(sizeof(CF_PRED_MGR)*sp->Best_mgr->pred_num, 
				   "CF PRED MGR");

    /* �ʥե졼��θĿ�ʬ�������� */
    for (i = 0; i < sp->Best_mgr->pred_num; i++) {
	cfnum += sp->Best_mgr->cpm[i].result_num;
    }
    sp_new->cf = (CASE_FRAME *)malloc_data(sizeof(CASE_FRAME)*cfnum, 
					   "CASE FRAME");

    cfnum = 0;
    for (i = 0; i < sp->Best_mgr->pred_num; i++) {
	*(sp_new->cpm+i) = sp->Best_mgr->cpm[i];
	num = sp->Best_mgr->cpm[i].pred_b_ptr->num;	/* �����Ѹ���ʸ���ֹ� */
	if (num != -1) {
	    sp_new->bnst_data[num].cpm_ptr = sp_new->cpm+i;
	    (sp_new->cpm+i)->pred_b_ptr = sp_new->bnst_data+num;
	}
	/* ����ʸ�� */
	else {
	    /* internal��free���ʤ� */
	    (sp_new->cpm+i)->pred_b_ptr->cpm_ptr = sp_new->cpm+i;
	}
	for (j = 0; j < (sp_new->cpm+i)->cf.element_num; j++) {
	    /* ��ά����ʤ������� */
	    if ((sp_new->cpm+i)->elem_b_num[j] > -2) {
		/* ����ʸ�ᤸ��ʤ� */
		if ((sp_new->cpm+i)->elem_b_ptr[j]->num != -1) {
		    (sp_new->cpm+i)->elem_b_ptr[j] = sp_new->bnst_data+((sp_new->cpm+i)->elem_b_ptr[j]-sp->bnst_data);
		}
	    }
	}

	(sp_new->cpm+i)->pred_b_ptr->cf_ptr = sp_new->cf+cfnum;
	for (j = 0; j < (sp_new->cpm+i)->result_num; j++) {
	    copy_cf_with_alloc(sp_new->cf+cfnum, (sp_new->cpm+i)->cmm[j].cf_ptr);
	    (sp_new->cpm+i)->cmm[j].cf_ptr = sp_new->cf+cfnum;
	    sp->Best_mgr->cpm[i].cmm[j].cf_ptr = sp_new->cf+cfnum;
	    cfnum++;
	}
    }

    /* New�ΰ��ʸ���cpm�ݥ��󥿤�Ϥ�ʤ��� */
    for (i = sp->Bnst_num; i < sp->Bnst_num + sp->New_Bnst_num; i++) {
	if ((sp_new->bnst_data+i)->cpm_ptr) {
	    (sp_new->bnst_data+i)->cpm_ptr = (sp_new->bnst_data+(sp_new->bnst_data+i)->cpm_ptr->pred_b_ptr->num)->cpm_ptr;
	}
    }

    /* ���� cpm ����¸���Ƥ��뤬��Best_mgr ����¸���������������⤷��ʤ� */
    sp_new->Best_mgr = NULL;
}

/*==================================================================*/
float CalcSimilarityForVerb(BNST_DATA *cand, CASE_FRAME *cf_ptr, int n, int *pos)
/*==================================================================*/
{
    char *exd, *exp;
    int i, j, step, expand;
    float score = 0, ex_score;

    exp = cf_ptr->ex[n];
    if (Thesaurus == USE_BGH) {
	exd = cand->BGH_code;
	step = BGH_CODE_SIZE;
    }
    else if (Thesaurus == USE_NTT) {
	exd = cand->SM_code;
	step = SM_CODE_SIZE;
    }

    if (check_feature(cand->f, "�Ը�ͭ����Ÿ���ػ�")) {
	expand = SM_NO_EXPAND_NE;
    }
    else {
	expand = SM_EXPAND_NE;
    }

    /* ��̣�Ǥʤ�
       ����ˤ��뤿��� -1 ���֤� */
    if (!exd[0]) {
	ex_score = -1;
    }
    /* exact match */
    else if (cf_match_exactly(cand, cf_ptr->ex_list[n], cf_ptr->ex_num[n], pos)) {
	ex_score = 1.1;
    }
    else {
	/* ����ޥå������������ */
	ex_score = CalcSmWordsSimilarity(exd, cf_ptr->ex_list[n], cf_ptr->ex_num[n], pos, 
					 cf_ptr->sm_delete[n], expand);
    }

    /* ���ΤΥޥå��� (�Ȥꤢ�������ʤΤȤ�����) */
    if (cf_ptr->sm[n] && 
	(MatchPP(cf_ptr->pp[n][0], "��") || 
	 (MatchPP(cf_ptr->pp[n][0], "��") && 
	  cf_ptr->voice == FRAME_PASSIVE_1))) {
	int flag;
	for (j = 0; cf_ptr->sm[n][j]; j+=step) {
	    if (!strncmp(cf_ptr->sm[n]+j, sm2code("����"), SM_CODE_SIZE)) {
		flag = 3;
	    }
	    else if (!strncmp(cf_ptr->sm[n]+j, sm2code("��"), SM_CODE_SIZE)) {
		flag = 1;
	    }
	    else if (!strncmp(cf_ptr->sm[n]+j, sm2code("�ȿ�"), SM_CODE_SIZE)) {
		flag = 2;
	    }
	    else {
		continue;
	    }
	    if (check_feature(cand->f, "�����")) {
		return 0;
	    }
	    /* �ʥե졼��¦�� <����> ������Ȥ��ˡ�������¦������å� */
	    if (((flag & 1) && check_feature(cand->f, "��̾")) || 
		((flag & 2) && check_feature(cand->f, "�ȿ�̾"))) {
		score = (float)EX_match_subject/11;
		/* ��ͭ̾��ΤȤ��˥�������⤯
		if (EX_match_subject > 8) {
		    * ���Υ��������⤤�Ȥ��ϡ������Ʊ���ˤ��� *
		    score = (float)EX_match_subject/11;
		}
		else {
		    score = (float)9/11;
		} */
		break;
	    }
	    for (i = 0; cand->SM_code[i]; i+=step) {
		if (_sm_match_score(cf_ptr->sm[n]+j, cand->SM_code+i, expand)) {
		    score = (float)EX_match_subject/11;
		    break;
		}
	    }
	    break;
	}
    }

    if (score > 0) {
	*pos = MATCH_SUBJECT;
	return score;
    }
    else {
	return ex_score;
    }
}

/*==================================================================*/
     float CalcSimilarityForNoun(BNST_DATA *dat, BNST_DATA *pat)
/*==================================================================*/
{
    char *exd, *exp;
    float score = 0, ex_score;

    if (Thesaurus == USE_BGH) {
	exd = dat->BGH_code;
	exp = pat->BGH_code;
    }
    else if (Thesaurus == USE_NTT) {
	exd = dat->SM_code;
	exp = pat->SM_code;
    }

    /* ����ޥå������������ */
    ex_score = (float)calc_similarity(exd, exp, 0);

    if (ex_score > score) {
	return ex_score;
    }
    else {
	return score;
    }
}

/*==================================================================*/
		  int CheckTargetNoun(BNST_DATA *bp)
/*==================================================================*/
{
    /* ���ĤǤ�����å������ */

    if (!check_feature(bp->f, "����") && 
	!check_feature(bp->f, "����Ū") && 
	!check_feature(bp->f, "����̾��") && 
	!check_feature(bp->f, "������") && 
	!check_feature(bp->f, "�ؼ���") && 
	!check_feature(bp->f, "ID:�ʡ���ˡ���") && 
	!check_feature(bp->f, "���δط�") && 
	!check_feature(bp->f, "���δط���ǽ��")) {
	return TRUE;
    }
    return FALSE;
}

/*==================================================================*/
		   int CheckPureNoun(BNST_DATA *bp)
/*==================================================================*/
{
    /* !check_feature(bp->f, "����") &&  */
    if ((check_feature(bp->f, "�θ�") || 
	 check_feature(bp->f, "����")) && 
	CheckTargetNoun(bp)) {
	return TRUE;
    }
    return FALSE;
}

/*==================================================================*/
	 int CheckCaseComponent(CF_PRED_MGR *cpm_ptr, int n)
/*==================================================================*/
{
    int i;

    for (i = 0; i < cpm_ptr->cf.element_num; i++) {
	if (cpm_ptr->elem_b_num[i] > -2 && 
	    cpm_ptr->elem_b_ptr[i]->num == n) {
	    return TRUE;
	    /* �����ǤˤʤäƤ��뤫�ɤ�����
	       �б��դ����֤ϴط��ʤ�
	    if (cmm_ptr->result_lists_d[0].flag[i] >= 0) {
		return TRUE;
	    }
	    else {
		return FALSE;
	    } */
	}
    }
    return FALSE;
}

/*==================================================================*/
int CheckEllipsisComponent(CF_PRED_MGR *cpm_ptr, CF_MATCH_MGR *cmm_ptr, BNST_DATA *bp)
/*==================================================================*/
{
    int i, num;

    /* �Ѹ��������Ʊ��ɽ����
       ¾�γʤξ�ά�λؼ��оݤȤ��Ƥ�äƤ��뤫�ɤ���
       cpm_ptr->elem_b_num[num] <= -2 
       => �̾�γ����Ǥ�����å� */

    for (i = 0; i < cmm_ptr->cf_ptr->element_num; i++) {
	num = cmm_ptr->result_lists_p[0].flag[i];
	/* ��ά�λؼ��о� */
	if (num >= 0 && 
	    str_eq(cpm_ptr->elem_b_ptr[num]->Jiritu_Go, bp->Jiritu_Go)) {
	    return 1;
	}
    }
    return 0;
}

/*==================================================================*
   int CheckEllipsisComponent(ELLIPSIS_MGR *em_ptr, BNST_DATA *bp)
 *==================================================================*
{
    int i;

    * �Ѹ��������Ʊ��ɽ����
       ¾�γʤξ�ά�λؼ��оݤȤ��Ƥ�äƤ��뤫�ɤ��� *

    for (i = 0; i < CASE_MAX_NUM; i++) {
	if (em_ptr->cc[i].s && 
	    str_eq((em_ptr->cc[i].s->bnst_data+em_ptr->cc[i].bnst)->Jiritu_Go, bp->Jiritu_Go)) {
	    return 1;
	}
    }
    return 0;
}
*/

/*==================================================================*/
int CheckObligatoryCase(CF_PRED_MGR *cpm_ptr, CF_MATCH_MGR *cmm_ptr, BNST_DATA *bp)
/*==================================================================*/
{
    /* 
       bp: �о�ʸ��
       cpm_ptr: �о�ʸ��η����Ѹ� (bp->parent->cpm_ptr)
    */

    int i, num;

    if (cpm_ptr == NULL) {
	return 0;
    }

    if (cmm_ptr->score != -2) {
	for (i = 0; i < cmm_ptr->cf_ptr->element_num; i++) {
	    num = cmm_ptr->result_lists_p[0].flag[i];
	    /* ���줬Ĵ�٤������ */
	    if (num != UNASSIGNED && 
		cpm_ptr->elem_b_num[num] > -2 && /* ��ά�γ����Ǥ���ʤ� */
		cpm_ptr->elem_b_ptr[num]->num == bp->num) {
		if (MatchPP(cmm_ptr->cf_ptr->pp[i][0], "��") || 
		    MatchPP(cmm_ptr->cf_ptr->pp[i][0], "��") || 
		    MatchPP(cmm_ptr->cf_ptr->pp[i][0], "��") || 
		    MatchPP(cmm_ptr->cf_ptr->pp[i][0], "����")) {
		    return 1;
		}
		return 0;
	    }
	}
    }
    return 0;
}

/*==================================================================*/
int GetCandCase(CF_PRED_MGR *cpm_ptr, CF_MATCH_MGR *cmm_ptr, BNST_DATA *bp)
/*==================================================================*/
{
    /* ����γʤ�����
       bp: �о�ʸ��
       cpm_ptr: �о�ʸ��η����Ѹ� (bp->parent->cpm_ptr)
    */

    int i, num;

    if (cpm_ptr && cpm_ptr->result_num > 0 && cmm_ptr->score != -2) {
	for (i = 0; i < cmm_ptr->cf_ptr->element_num; i++) {
	    num = cmm_ptr->result_lists_p[0].flag[i];
	    /* ���줬Ĵ�٤������ */
	    if (num != UNASSIGNED && 
		cpm_ptr->elem_b_num[num] > -2 && /* ��ά�γ����Ǥ���ʤ� */
		cpm_ptr->elem_b_ptr[num]->num == bp->num) {
		return cmm_ptr->cf_ptr->pp[i][0];
	    }
	}
    }
    return -1;
}

/*==================================================================*/
int CheckCaseCorrespond(CF_PRED_MGR *cpm_ptr, CF_MATCH_MGR *cmm_ptr, 
			BNST_DATA *bp, CASE_FRAME *cf_ptr, int n)
/*==================================================================*/
{
    /* �ʤΰ��פ�Ĵ�٤�
       bp: �о�ʸ��
       cpm_ptr: �о�ʸ��η����Ѹ� (bp->parent->cpm_ptr)
    */

    int i, num;

    if (cpm_ptr->result_num > 0 && cmm_ptr->score != -2) {
	for (i = 0; i < cmm_ptr->cf_ptr->element_num; i++) {
	    num = cmm_ptr->result_lists_p[0].flag[i];
	    /* ���줬Ĵ�٤������ */
	    if (num != UNASSIGNED && 
		cpm_ptr->elem_b_num[num] > -2 && /* ��ά�γ����Ǥ���ʤ� */
		cpm_ptr->elem_b_ptr[num]->num == bp->num) {
		if (cf_ptr->pp[n][0] == cmm_ptr->cf_ptr->pp[i][0] || 
		    (MatchPP(cf_ptr->pp[n][0], "��") && MatchPP(cmm_ptr->cf_ptr->pp[i][0], "����"))) {
		    return 1;
		}
		return 0;
	    }
	}
    }
    return 0;
}

/*==================================================================*/
      BNST_DATA *GetRealParent(SENTENCE_DATA *sp, BNST_DATA *bp)
/*==================================================================*/
{
    if (bp->dpnd_head != -1) {
	return sp->bnst_data+bp->dpnd_head;
    }
    return NULL;
}

/*==================================================================*/
int CountBnstDistance(SENTENCE_DATA *cs, int candn, SENTENCE_DATA *ps, int pn)
/*==================================================================*/
{
    int sdiff, i, diff = 0;

    sdiff = ps-cs;

    if (sdiff > 0) {
	for (i = 1; i < sdiff; i++) {
	    diff += (ps-i)->Bnst_num;
	}
	diff += pn+cs->Bnst_num-candn;
    }
    else {
	diff = pn-candn;
    }

    return diff;
}

/*==================================================================*/
	char *EllipsisSvmFeatures2String(E_SVM_FEATURES *esf)
/*==================================================================*/
{
    int max, i;
    char *buffer, *sbuf;

    /* max = (sizeof(E_SVM_FEATURES)-sizeof(float))/sizeof(int)+1; */
    max = (sizeof(E_SVM_FEATURES)-sizeof(float))/sizeof(int); /* ac ������ƥ��� */
    sbuf = (char *)malloc_data(sizeof(char)*(10+log(max)), 
			       "EllipsisSvmFeatures2String");
    buffer = (char *)malloc_data((sizeof(char)*(10+log(max)))*max+10, 
				 "EllipsisSvmFeatures2String");
    sprintf(buffer, "1:%.5f", esf->similarity);
    for (i = 2; i <= max; i++) {
	sprintf(sbuf, " %d:%d", i, *(esf->c_pp+i-2));
	strcat(buffer, sbuf);
    }
    free(sbuf);

    return buffer;
}

/*==================================================================*/
void EllipsisSvmFeaturesString2Feature(ELLIPSIS_MGR *em_ptr, char *ecp, 
				       char *word, int pp, char *sid, int num)
/*==================================================================*/
{
    char *buffer;

    /* -learn ���Τ߳ؽ���feature��ɽ������ */
    if (OptLearn == FALSE) {
	return;
    }

    buffer = (char *)malloc_data(strlen(ecp)+46+strlen(word), 
				 "EllipsisSvmFeaturesString2FeatureString");
    sprintf(buffer, "SVM�ؽ�FEATURE;%s;%s;%s;%d:%s", 
	    word, pp_code_to_kstr(pp), sid, num, ecp);
    assign_cfeature(&(em_ptr->f), buffer);
    free(buffer);
}

/*==================================================================*/
 E_SVM_FEATURES *EllipsisFeatures2EllipsisSvmFeatures(E_FEATURES *ef)
/*==================================================================*/
{
    E_SVM_FEATURES *f;
    int i;

    f = (E_SVM_FEATURES *)malloc_data(sizeof(E_SVM_FEATURES), "SetEllipsisFeatures");

    f->similarity = ef->similarity;
    for (i = 0; i < PP_NUMBER; i++) {
	f->c_pp[i] = ef->c_pp == i ? 1 : 0;
    }
    f->c_distance = ef->c_distance;
    f->c_dist_bnst = ef->c_dist_bnst;
    f->c_fs_flag = ef->c_fs_flag;
    f->c_location[0] = ef->c_location == PLOC_PV ? 1 : 0;
    f->c_location[1] = ef->c_location == PLOC_PARENTV ? 1 : 0;
    f->c_location[2] = ef->c_location == PLOC_CHILDV ? 1 : 0;
    f->c_location[3] = ef->c_location == PLOC_CHILDPV ? 1 : 0;
    f->c_location[4] = ef->c_location == PLOC_NPARENTV ? 1 : 0;
    f->c_location[5] = ef->c_location == PLOC_NPARENTCV ? 1 : 0;
    f->c_location[6] = ef->c_location == PLOC_NOTHERS ? 1 : 0;
    f->c_topic_flag = ef->c_topic_flag;
    f->c_no_topic_flag = ef->c_no_topic_flag;
    f->c_subject_flag = ef->c_subject_flag;
    f->c_dep_mc_flag = ef->c_dep_mc_flag;
    f->c_dep_p_level[0] = str_eq(ef->c_dep_p_level, "A-") ? 1 : 0;
    f->c_dep_p_level[1] = str_eq(ef->c_dep_p_level, "A") ? 1 : 0;
    f->c_dep_p_level[2] = str_eq(ef->c_dep_p_level, "B-") ? 1 : 0;
    f->c_dep_p_level[3] = str_eq(ef->c_dep_p_level, "B") ? 1 : 0;
    f->c_dep_p_level[4] = str_eq(ef->c_dep_p_level, "B+") ? 1 : 0;
    f->c_dep_p_level[5] = str_eq(ef->c_dep_p_level, "C") ? 1 : 0;
    f->c_prev_p_flag = ef->c_prev_p_flag;
    f->c_get_over_p_flag = ef->c_get_over_p_flag;
    f->c_sm_none_flag = ef->c_sm_none_flag;
    f->c_extra_tag[0] = ef->c_extra_tag == 0 ? 1 : 0;
    f->c_extra_tag[1] = ef->c_extra_tag == 1 ? 1 : 0;
    f->c_extra_tag[2] = ef->c_extra_tag == 2 ? 1 : 0;
    f->c_extra_tag[3] = ef->c_extra_tag == 3 ? 1 : 0;

    /* ��,��,�� */
    for (i = 0; i < 3; i++) {
	f->p_pp[i] = ef->p_pp == i+1 ? 1 : 0;
    }
    f->p_voice[0] = ef->p_voice == VOICE_SHIEKI ? 1 : 0;
    f->p_voice[1] = ef->p_voice == VOICE_UKEMI ? 1 : 0;
    f->p_voice[2] = ef->p_voice == VOICE_MORAU ? 1 : 0;
    f->p_type[0] = ef->p_type == 1 ? 1 : 0;
    f->p_type[1] = ef->p_type == 2 ? 1 : 0;
    f->p_type[2] = ef->p_type == 3 ? 1 : 0;
    f->p_sahen_flag = ef->p_sahen_flag;
    f->p_cf_subject_flag = ef->p_cf_subject_flag;
    f->p_cf_sentence_flag = ef->p_cf_sentence_flag;
    f->p_n_modify_flag = ef->p_n_modify_flag;

    f->c_ac = ef->c_ac;

    return f;
}

/*==================================================================*/
void SetEllipsisFeaturesForPred(E_FEATURES *f, CF_PRED_MGR *cpm_ptr, 
				CASE_FRAME *cf_ptr, int n)
/*==================================================================*/
{
    f->p_pp = cf_ptr->pp[n][0];

    if (check_feature(cpm_ptr->pred_b_ptr->f, "�ɲü���")) {
	f->p_voice = VOICE_UKEMI;
    }
    else {
	/* ǽư(0), VOICE_SHIEKI(1), VOICE_UKEMI(2), VOICE_MORAU(3) */
	f->p_voice = cpm_ptr->pred_b_ptr->voice;
    }

    if (check_feature(cpm_ptr->pred_b_ptr->f, "�Ѹ�:ư")) {
	f->p_type = 1;
    }
    else if (check_feature(cpm_ptr->pred_b_ptr->f, "�Ѹ�:��")) {
	f->p_type = 2;
    }
    else if (check_feature(cpm_ptr->pred_b_ptr->f, "�Ѹ�:Ƚ")) {
	f->p_type = 3;
    }
    else {
	f->p_type = 0;
    }

    if (check_feature(cpm_ptr->pred_b_ptr->f, "����")) {
	f->p_sahen_flag = 1;
    }
    else {
	f->p_sahen_flag = 0;
    }

    f->p_cf_subject_flag = cf_match_element(cf_ptr->sm[n], "����", FALSE) ? 1 : 0;
    f->p_cf_sentence_flag = cf_match_element(cf_ptr->sm[n], "��ʸ", TRUE) ? 1 : 0;
    f->p_n_modify_flag = check_feature(cpm_ptr->pred_b_ptr->f, "��:Ϣ��") ? 1 : 0;
}

/*==================================================================*/
E_FEATURES *SetEllipsisFeatures(SENTENCE_DATA *s, SENTENCE_DATA *cs, 
				CF_PRED_MGR *cpm_ptr, CF_MATCH_MGR *cmm_ptr, 
				BNST_DATA *bp, CASE_FRAME *cf_ptr, int n, int loc)
/*==================================================================*/
{
    E_FEATURES *f;
    char *level;

    f = (E_FEATURES *)malloc_data(sizeof(E_FEATURES), "SetEllipsisFeatures");

    f->pos = MATCH_NONE;
    f->similarity = CalcSimilarityForVerb(bp, cf_ptr, n, &f->pos);

    if (bp->pred_b_ptr) {
	f->c_pp = GetCandCase(bp->pred_b_ptr->cpm_ptr, &(bp->pred_b_ptr->cpm_ptr->cmm[0]), bp);

	if ((level = check_feature(bp->pred_b_ptr->f, "��٥�"))) {
	    strcpy(f->c_dep_p_level, level+7);
	}
	else {
	    f->c_dep_p_level[0] = '\0';
	}
    }
    else {
	f->c_pp = -1;
	f->c_dep_p_level[0] = '\0';
    }

    f->c_distance = cs-s;
    f->c_dist_bnst = CountBnstDistance(s, bp->num, cs, cpm_ptr->pred_b_ptr->num);
    f->c_fs_flag = s->Sen_num == 1 ? 1 : 0;
    f->c_location = loc;
    f->c_topic_flag = check_feature(bp->f, "����ɽ��") ? 1 : 0;
    f->c_no_topic_flag = check_feature(bp->f, "�����ɽ��") ? 1 : 0;
    f->c_subject_flag = sm_match_check(sm2code("����"), bp->SM_code) ? 1 : 0;
    f->c_dep_mc_flag = CheckLastClause(cs->Sen_num-f->c_distance, f->c_pp, bp->Jiritu_Go) ? 1 : 0;

    if (f->c_distance > 0 || 
	(f->c_distance == 0 && bp->num < cpm_ptr->pred_b_ptr->num)) {
	f->c_prev_p_flag = 1;
    }
    else {
	f->c_prev_p_flag = 0;
    }

    if (f->c_distance == 0 && bp->dpnd_head > cpm_ptr->pred_b_ptr->num) {
	f->c_get_over_p_flag = 1;
    }
    else {
	f->c_get_over_p_flag = 0;
    }

    f->c_sm_none_flag = f->similarity < 0 ? 1 : 0;
    f->c_extra_tag = -1;

    /* �Ѹ��˴ؤ���feature������ */
    SetEllipsisFeaturesForPred(f, cpm_ptr, cf_ptr, n);

    /* ���Ȳ�� */
    f->c_ac = CheckAnaphor(alist, bp->Jiritu_Go);

    return f;
}

/*==================================================================*/
E_FEATURES *SetEllipsisFeaturesExtraTags(int tag, CF_PRED_MGR *cpm_ptr, 
					 CASE_FRAME *cf_ptr, int n)
/*==================================================================*/
{
    E_FEATURES *f;

    f = (E_FEATURES *)malloc_data(sizeof(E_FEATURES), "SetEllipsisFeaturesExtraTags");
    memset(f, 0, sizeof(E_FEATURES));
    f->similarity = -1;
    f->c_pp = -1;
    f->c_distance = -1;
    f->c_dist_bnst = -1;
    f->c_extra_tag = tag;

    /* �Ѹ��˴ؤ���feature������ */
    SetEllipsisFeaturesForPred(f, cpm_ptr, cf_ptr, n);

    return f;
}

/*==================================================================*/
void EllipsisDetectForVerbSubcontractExtraTagsWithLearning(SENTENCE_DATA *cs, ELLIPSIS_MGR *em_ptr, 
							   CF_PRED_MGR *cpm_ptr, CF_MATCH_MGR *cmm_ptr, 
							   int tag, CASE_FRAME *cf_ptr, int n)
/*==================================================================*/
{
    E_FEATURES *ef;
    E_SVM_FEATURES *esf;
    float score;
    char *ecp, feature_buffer[DATA_LEN];

    ef = SetEllipsisFeaturesExtraTags(tag, cpm_ptr, cf_ptr, n);
    esf = EllipsisFeatures2EllipsisSvmFeatures(ef);
    ecp = EllipsisSvmFeatures2String(esf);

    if (OptDiscMethod == OPT_SVM) {
#ifdef USE_SVM
	score = svm_classify(ecp);
#endif
    }
    else if (OptDiscMethod == OPT_DT) {
	score = dt_classify(ecp);
    }

    if (score > maxscore) {
	char feature_buffer2[50000];
	maxscore = score;
	maxrawscore = 1.0;
	maxtag = ExtraTags[tag];
	sprintf(feature_buffer2, "SVM-%s:%s", pp_code_to_kstr(cf_ptr->pp[n][0]), ecp);
	assign_cfeature(&(em_ptr->f), feature_buffer2);
    }

    /* ��ά���� */
    sprintf(feature_buffer, "C��;%s;%s;-1;-1;%.3f|-1", ExtraTags[tag], 
	    pp_code_to_kstr(cf_ptr->pp[n][0]), 
	    score);
    assign_cfeature(&(em_ptr->f), feature_buffer);

    /* �ؽ�FEATURE */
    EllipsisSvmFeaturesString2Feature(em_ptr, ecp, ExtraTags[tag], cf_ptr->pp[n][0], 
				      "?", -1);
    free(ef);
    free(esf);
    free(ecp);
}

/*==================================================================*/
void _EllipsisDetectForVerbSubcontractWithLearning(SENTENCE_DATA *s, SENTENCE_DATA *cs, ELLIPSIS_MGR *em_ptr, 
						   CF_PRED_MGR *cpm_ptr, CF_MATCH_MGR *cmm_ptr, 
						   BNST_DATA *bp, CASE_FRAME *cf_ptr, int n, int type, int loc)
/*==================================================================*/
{
    E_FEATURES *ef;
    E_SVM_FEATURES *esf;
    float score;
    char *ecp, feature_buffer[DATA_LEN];

    ef = SetEllipsisFeatures(s, cs, cpm_ptr, cmm_ptr, bp, cf_ptr, n, loc);
    esf = EllipsisFeatures2EllipsisSvmFeatures(ef);
    ecp = EllipsisSvmFeatures2String(esf);

    /* �ؽ�FEATURE */
    EllipsisSvmFeaturesString2Feature(em_ptr, ecp, bp->Jiritu_Go, cf_ptr->pp[n][0], 
				      s->KNPSID ? s->KNPSID+5 : "?", bp->num);

    /* ���Ǥ�¾�γʤλؼ��оݤˤʤäƤ���Ȥ��Ϥ��� */
    if (CheckEllipsisComponent(cpm_ptr, cmm_ptr, bp)) {
	free(ef);
	free(esf);
	free(ecp);
	return;
    }

    if (OptDiscMethod == OPT_SVM) {
#ifdef USE_SVM
	score = svm_classify(ecp);
#endif
    }
    else if (OptDiscMethod == OPT_DT) {
	score = dt_classify(ecp);
    }

    /* if (rawscore > 0 || smnone == 1) { */
    if (score > maxscore) {
	char feature_buffer2[50000];
	maxscore = score;
	maxrawscore = ef->similarity;
	maxs = s;
	maxpos = ef->pos;
	if (bp->num < 0) {
	    maxi = bp->parent->num;
	}
	else {
	    maxi = bp->num;
	}
	sprintf(feature_buffer2, "SVM-%s:%s", pp_code_to_kstr(cf_ptr->pp[n][0]), ecp);
	assign_cfeature(&(em_ptr->f), feature_buffer2);
	maxtag = NULL;
    }

    /* ��ά���� */
    sprintf(feature_buffer, "C��;%s;%s;%d;%d;%.3f|%.3f", bp->Jiritu_Go, 
	    pp_code_to_kstr(cf_ptr->pp[n][0]), 
	    ef->c_distance, maxi, 
	    score, ef->similarity);
    assign_cfeature(&(em_ptr->f), feature_buffer);

    free(ef);
    free(esf);
    free(ecp);
}

/*==================================================================*/
int EllipsisDetectForVerbSubcontractExtraTags(SENTENCE_DATA *cs, ELLIPSIS_MGR *em_ptr, 
					      CF_PRED_MGR *cpm_ptr, CF_MATCH_MGR *cmm_ptr, 
					      int tag, CASE_FRAME *cf_ptr, int n)
/*==================================================================*/
{

    if (OptDiscMethod == OPT_SVM || OptDiscMethod == OPT_DT) {
	EllipsisDetectForVerbSubcontractExtraTagsWithLearning(cs, em_ptr, cpm_ptr, cmm_ptr, 
							      tag, cf_ptr, n);
	if (maxscore > AntecedentDecideThreshold) {
	    return 1;
	}
    }
    else {
	E_FEATURES *ef;
	E_SVM_FEATURES *esf;
	char *ecp;

	ef = SetEllipsisFeaturesExtraTags(tag, cpm_ptr, cf_ptr, n);
	esf = EllipsisFeatures2EllipsisSvmFeatures(ef);
	ecp = EllipsisSvmFeatures2String(esf);
	EllipsisSvmFeaturesString2Feature(em_ptr, ecp, ExtraTags[tag], cf_ptr->pp[n][0], 
					  "?", -1);
	free(ef);
	free(esf);
	free(ecp);
    }
    return 0;
}

/*==================================================================*/
void _EllipsisDetectForVerbSubcontract(SENTENCE_DATA *s, SENTENCE_DATA *cs, ELLIPSIS_MGR *em_ptr, 
				       CF_PRED_MGR *cpm_ptr, CF_MATCH_MGR *cmm_ptr, 
				       BNST_DATA *bp, CASE_FRAME *cf_ptr, int n, int type, int loc)
/*==================================================================*/
{
    float score, weight, pascore, pcscore, mcscore, rawscore, topicscore, distscore;
    float addscore = 0;
    char feature_buffer[DATA_LEN], *ecp;
    int ac, pac, pcc, pcc2, mcc, topicflag, distance, agentflag, firstsc, subtopicflag, sameflag;
    int exception = 0, pos = MATCH_NONE, casematch, candagent, hobunflag, predabstract, smnone;
    int important = 0;
    E_FEATURES *ef;
    E_SVM_FEATURES *esf;

    /* �ؽ�FEATURE */
    ef = SetEllipsisFeatures(s, cs, cpm_ptr, cmm_ptr, bp, cf_ptr, n, loc);
    esf = EllipsisFeatures2EllipsisSvmFeatures(ef);
    ecp = EllipsisSvmFeatures2String(esf);
    EllipsisSvmFeaturesString2Feature(em_ptr, ecp, bp->Jiritu_Go, cf_ptr->pp[n][0], 
				      s->KNPSID ? s->KNPSID+5 : "?", bp->num);
    free(ef);
    free(esf);
    free(ecp);

    /* cs �ΤȤ�������̣������ */
    Bcheck[bp->num] = 1;

    /* �о��Ѹ��ȸ��䤬Ʊ����Ω��ΤȤ�
       Ƚ���ξ��������� */
    if (str_eq(cpm_ptr->pred_b_ptr->Jiritu_Go, bp->Jiritu_Go)) {
	if (!(check_feature(cpm_ptr->pred_b_ptr->f, "�Ѹ�:Ƚ") && 
	      MatchPP(cf_ptr->pp[n][0], "��"))) {
	    return;
	}
	/* Ƚ���ǥ��ʤΤȤ����� */
	sameflag = 1;
    }
    else {
	sameflag = 0;
    }

    /* ���Ǥ�¾�γʤλؼ��оݤˤʤäƤ���Ȥ��Ϥ��� */
    if (CheckEllipsisComponent(cpm_ptr, cmm_ptr, bp)) {
	return;
    }

    /* ���ߤ�ʸ�����оݤȤʤäƤ�������Ǥ�ʸ�ޤǤε�Υ */
    distance = cs-s;

    /* Ʊ��ʸ�ǤϤʤ�Ʊ�ʤ���¦�ϸ���ˤ��ʤ� */
    if (bp->dpnd_type == 'A' && distance != 0) {
	return;
    }

    if (distance > 8) {
	distscore = 0.2;
    }
    else {
	distscore = 1-(float)distance*0.1;
    }

    /* �ʤΰ��פ�����å� 
     bp->parent && bp->parent->cpm_ptr &&  */
    if (distance == 0 && 
	bp->pred_b_ptr && bp->pred_b_ptr->cpm_ptr && 
	bp->num < cpm_ptr->pred_b_ptr->num && 
	check_feature(bp->pred_b_ptr->f, "�Ѹ�")) {
	if (CheckCaseCorrespond(bp->pred_b_ptr->cpm_ptr, &(bp->pred_b_ptr->cpm_ptr->cmm[0]), 
				bp, cf_ptr, n) == 0) {
	    /* ���԰��� */
	    weight = 0.9;
	    casematch = -1;
	}
	else {
	    /* �ʰ��� */
	    weight = 1;
	    casematch = 1;
	}
    }
    else {
	weight = 1;
	casematch = 0;
    }

    /* ��󥯤�����ˤ�륹����
    if (type == RANK2) {
	* weight = (float)EllipsisSubordinateClauseScore/10; *
	weight = 0.95;
    }
    else {
	weight = 1.0;
    } */

    /* ��󥯤��줿����ˤ�륹���� */
    ac = CheckAnaphor(alist, bp->Jiritu_Go);

    /* ���Ǥ˽и������Ѹ��Ȥ��γ����ǤΥ��å� */
    pac = CheckPredicate(L_Jiritu_M(cpm_ptr->pred_b_ptr)->Goi, cpm_ptr->pred_b_ptr->voice, 
			 cf_ptr->cf_address, 
			 cf_ptr->pp[n][0], bp->Jiritu_Go);
    pascore = 1.0+0.5*pac; /* 0.2 */

    /* ��ʸ�μ���Υ����� */
    pcc = CheckLastClause(cs->Sen_num-1, cf_ptr->pp[n][0], bp->Jiritu_Go);

    /* 2ʸ���μ���Υ����� */
    pcc2 = CheckLastClause(cs->Sen_num-2, cf_ptr->pp[n][0], bp->Jiritu_Go);

    /* ���פǤϤʤ������Ǥξ�� */
    if (check_feature(bp->f, "�����")) {
	pcscore = 1.0+0.2*pcc; /* 0.1 */
    }
    else {
	pcscore = 1.0+0.5*pcc; /* 0.1 */
    }

    /* ���ߤ�ʸ�μ���Υ����� */
    mcc = CheckLastClause(cs->Sen_num, cf_ptr->pp[n][0], bp->Jiritu_Go);

    /* ���פǤϤʤ������Ǥξ�� */
    if (check_feature(bp->f, "�����")) {
	mcscore = 1.0+0.2*mcc; /* 0.2 */
    }
    else {
	mcscore = 1.0+0.5*mcc; /* 0.2 */
    }

    /* ����Υ����� */
    if (check_feature(bp->f, "����ɽ��")) {
	topicflag = 1;
	if (distance == 0) {
	    topicscore = 1.5; /* 1.2 */
	}
	else {
	    topicscore = 1.5; /* 1.2 */
	}
    }
    else {
	topicflag = 0;
	topicscore = 1.0;
    }

    if (check_feature(bp->f, "�����ɽ��")) {
	if (distance == 0 && 
	    (check_feature(bp->f, "��̾") || 
	     check_feature(bp->f, "��̾") || 
	     check_feature(bp->f, "�ȿ�̾"))) { 
	    topicflag = 1;
	    subtopicflag = 0;
	}
	else {
	    subtopicflag = 1;
	}
    }
    else {
	subtopicflag = 0;
    }

    /* ���� */
    if (topicscore > 1.0 && 
	(pcscore > 1.0 || mcscore > 1.0)) {
	pcscore = 1.0;
	mcscore = 1.0;
    }

    /* �ʥե졼�ब <����> ���Ĥ��ɤ��� */
    if (cf_match_element(cf_ptr->sm[n], "����", FALSE)) {
	/* sm_match_check(sm2code("����"), cf_ptr->sm[n]) */
	agentflag = 1;
    }
    else {
	agentflag = 0;
    }

    /* �ʥե졼�ब <��ʸ> ���Ĥ��ɤ��� */
    if (cf_match_element(cf_ptr->sm[n], "��ʸ", TRUE)) {
	hobunflag = 1;
    }
    else {
	hobunflag = 0;
    }

    if (sm_match_check(sm2code("����"), bp->SM_code)) {
	candagent = 1;
    }
    else {
	candagent = 0;
    }

    if (sm_match_check(sm2code("���ʪ"), cpm_ptr->pred_b_ptr->SM_code)) {
	predabstract = 1;
    }
    else {
	predabstract = 0;
    }

    /* N �� �� N ���� */
    if (sameflag) {
	rawscore = 1;
    }
    /* V �����Τ� N ����: ����, ��� 
       ��᤿: check_feature(bp->f, "�Ѹ�:Ƚ") */
    else if (cpm_ptr->pred_b_ptr->dpnd_head == bp->num && /* �����Ȼ��ꤷ�Ƥߤ� */
	     check_feature(cpm_ptr->pred_b_ptr->f, "ID:���Ρ�") && 
	     (check_feature(cpm_ptr->pred_b_ptr->f, "��:̤��") || 
	      check_feature(cpm_ptr->pred_b_ptr->f, "��:����"))) {
	rawscore = CalcSimilarityForVerb(bp, cf_ptr, n, &pos);
	/* �� rawscore == 0 �� bp ¦�˰�̣�Ǥ��ʤ�����ͤ��� */
	exception = 1;
    }
    else {
	rawscore = CalcSimilarityForVerb(bp, cf_ptr, n, &pos);
    }

    /* ��̣�Ǥʤ� */
    if (rawscore < 0) {
	smnone = 1;
    }
    else {
	smnone = 0;
    }

    /* ��Ƭʸ�μ��������å� */
    firstsc = CheckLastClause(1, cf_ptr->pp[n][0], bp->Jiritu_Go);

    /* �ߤ���� */
    if (((s->Sen_num == 1 && firstsc == 0) || 
	 (distance == 1 && pcc == 0) || 
	 (distance == 0 && mcc == 0)
	) && 
	((bp->num == s->Bnst_num-1 && check_feature(bp->f, "�Ѹ�:Ƚ")) || /* ʸ����Ƚ��� */
	 (bp->parent && bp->parent->parent && check_feature(bp->parent->parent->f, "����") && subordinate_level_check("B", bp->parent) && 
	  CheckObligatoryCase(bp->parent->cpm_ptr, &(bp->parent->cpm_ptr->cmm[0]), bp)) || /* ������°�� */
	 (check_feature(bp->f, "��:�γ�") && check_feature((s->bnst_data+bp->dpnd_head)->f, "����")))) { /* ����˷���γ�(̾���) */
	if (s->Sen_num == 1) {
	    firstsc = 1;
	}
	else if (distance == 0) {
	    mcc = 1;
	}
	else {
	    pcc = 1;
	}
    }

    if (casematch != -1 && (loc == PLOC_PV || loc == PLOC_PARENTV)) {
	addscore = 0.10;
    }

    /* ��Ƭʸ���Ѹ���ľ���ˤʤ����ǥ�, �ǥϤǤϤʤ��ǳʤ��оݤˤ��ʤ� 
       ¾��Ǥ�ճʤ⤽�����⤷��ʤ�
    if (firstsc && s->Sen_num == 1 && 
	check_feature(bp->f, "��:�ǳ�") && 
	bp->dpnd_head != bp->num+1 && 
	!check_feature(bp->f, "�ǥ�") && 
	!check_feature(bp->f, "�ǥ�")) {
	firstsc = 0;
    } */

    if (distance == 0 || /* �о�ʸ */
	(distance == 1 && (topicflag || subtopicflag || pcc)) || /* ��ʸ */
	/* (distance == 2 && pcc2) || * 2ʸ�� */
	(s->Sen_num == 1 && (topicflag || subtopicflag || firstsc)) || /* ��Ƭʸ */
	(pcc || mcc || firstsc)) { /* ����ʳ��Ǽ���ξ�ά�λؼ��о� */

	/* �о�ʸ�ˤϡ�����������ɬ�� 
	   ����γ����Ǥξ��Ǥ⡢�о��Ѹ�������˽и����Ƥ���ɬ�פ����� */
	if (distance == 0 && !exception && type < RANK4 && !((topicflag || mcc || type >= RANK2) && bp->num < cpm_ptr->pred_b_ptr->num)) {
	    if (check_feature(bp->f, "���") && !check_feature(bp->f, "��̾") && !check_feature(bp->f, "�ȿ�̾")) {
		weight *= NON_IMPORTANT_WEIGHT;
		score = weight*pascore*rawscore;
	    }
	    else {
		weight *= 1; /* NON_IMPORTANT_WEIGHT; */
		score = weight*pascore*rawscore;
	    }
	}
	else {
	    if (subtopicflag == 1) {
		weight *= NON_IMPORTANT_WEIGHT;
		score = weight*pascore*rawscore;
	    }
	    /* Ƚ���ξ��� RANK3 �Τ�Τ�����٤��ʤ����ϵߤ�
	       �ַ뺧�� �� ���ꡢ�� �����������ȡ��� 
	       ���: rawscore == 0 �Ϥ����� */
	    else if (casematch != -1 && type >= RANK3 && check_feature(cpm_ptr->pred_b_ptr->f, "�Ѹ�:Ƚ")) {
		score = weight*pascore*1.0;
	    }
	    else {
		score = weight*pascore*rawscore;
	    }
	}
	important = 1;
    }
    /* ���̤� N-V �Υ��åȤΤȤ��ϵ��� */
    else if (pascore > 1 && rawscore > 0.8) {
	score = weight*pascore*rawscore;
    }
    else {
	/* ��������� */
	score = -1;
	/* return; */
    }

    /* ��Υ���̣
    score *= distscore; */

    /* �ܡ��ʥ� */
    score += addscore;

    if (score > maxscore) {
	BNST_DATA *tp = bp;
	maxscore = score;
	maxs = s;
	maxpos = pos;

	/* ʸ����ξ��Ͽ�ʸ���õ���ˤ��� */
	while (tp->num < 0) {
	    tp = tp->parent;
	}
	maxi = tp->num;
    }

    /* ��ά���� (rawscore == 0 �ξ������Ȥ��ƽ���) */
    sprintf(feature_buffer, "C��;%s;%s;%d;%d;%.3f|%.3f", bp->Jiritu_Go, 
	    pp_code_to_kstr(cf_ptr->pp[n][0]), 
	    distance, maxi, 
	    score, rawscore);
    assign_cfeature(&(em_ptr->f), feature_buffer);
}

/*==================================================================*/
int EllipsisDetectForVerbSubcontract(SENTENCE_DATA *s, SENTENCE_DATA *cs, ELLIPSIS_MGR *em_ptr, 
				     CF_PRED_MGR *cpm_ptr, CF_MATCH_MGR *cmm_ptr, 
				     BNST_DATA *bp, CASE_FRAME *cf_ptr, int n, int type, int loc)
/*==================================================================*/
{
    if (OptDiscMethod == OPT_SVM || OptDiscMethod == OPT_DT) {
	_EllipsisDetectForVerbSubcontractWithLearning(s, cs, em_ptr, 
						      cpm_ptr, cmm_ptr, 
						      bp, cf_ptr, n, type, loc);
	if (maxscore > AntecedentDecideThreshold) {
	    return 1;
	}
    }
    else {
	_EllipsisDetectForVerbSubcontract(s, cs, em_ptr, 
					  cpm_ptr, cmm_ptr, 
					  bp, cf_ptr, n, type, loc);
    }
    return 0;
}

/*==================================================================*/
int SearchCaseComponent(SENTENCE_DATA *s, ELLIPSIS_MGR *em_ptr, 
			CF_PRED_MGR *cpm_ptr, CF_MATCH_MGR *cmm_ptr, 
			BNST_DATA *bp, CASE_FRAME *cf_ptr, int n, int rank, int loc)
/*==================================================================*/
{
    /* cpm_ptr: ��ά�����Ǥ����Ѹ�
       bp:      �����Ǥ�õ���оݤȤʤäƤ����Ѹ�ʸ��
    */

    int i, num;

    /* ¾���Ѹ� (���Ѹ��ʤ�) �γ����Ǥ�����å� 
       ���Ƥγʥե졼��Ϥ��Ǥ˷�ޤäƤ��뤬��
       ����ʳ��ξ���?�� */
    if (bp->cpm_ptr && bp->cpm_ptr->cmm[0].score != -2) {
	for (i = 0; i < bp->cpm_ptr->cmm[0].cf_ptr->element_num; i++) {
	    num = bp->cpm_ptr->cmm[0].result_lists_p[0].flag[i];
	    if (num != UNASSIGNED && 
		bp->cpm_ptr->elem_b_num[num] > -2 && /* �����Ǥ���ά����ä���ΤǤ���Ȥ��Ϥ��� */
		bp->cpm_ptr->elem_b_ptr[num]->num != cpm_ptr->pred_b_ptr->num && /* �����Ǥ����Ѹ��ΤȤ��Ϥ��� (bp->cpm_ptr->elem_b_ptr[num] ������ΤȤ� <PARA> �Ȥʤꡢpointer �ϥޥå����ʤ�) */
		CheckTargetNoun(bp->cpm_ptr->elem_b_ptr[num])) {
		/* �����Ǥγʤΰ��� (�ʤˤ�ꤱ��) */
		if (!check_feature(bp->cpm_ptr->elem_b_ptr[num]->f, "��:�γ�") && /* �γʤγʲ��ϤϿ��Ѥ��ʤ� */
		    !check_feature(bp->cpm_ptr->elem_b_ptr[num]->f, "�ǥ�") && /* �֤Ǥϡפγʲ��ϤϿ��Ѥ��ʤ� */
		    !check_feature(bp->cpm_ptr->elem_b_ptr[num]->f, "�ǥ�") && /* �֤Ǥ�פγʲ��ϤϿ��Ѥ��ʤ� */
		    (cf_ptr->pp[n][0] == bp->cpm_ptr->cmm[0].cf_ptr->pp[i][0] || 
		    (MatchPP(cf_ptr->pp[n][0], "��") && MatchPP(bp->cpm_ptr->cmm[0].cf_ptr->pp[i][0], "����")))) {
		    /* �����ơ������� N �� (�ʤ����פ��Ƥ���Ϣ�ν���) */
		    if (bp->cpm_ptr->elem_b_ptr[num]->num > bp->cpm_ptr->pred_b_ptr->num) {
			/* rank = RANK4; */
			rank++;
		    }
		    else {
			;
			/* rank = RANK3; */
		    }
		}
		else {
		    /* �ʤ��԰��� */
		    rank = RANK2;
		    /* rank--; */
		}
		if (EllipsisDetectForVerbSubcontract(s, s, em_ptr, cpm_ptr, cmm_ptr, 
						     bp->cpm_ptr->elem_b_ptr[num], 
						     cf_ptr, n, rank, loc)) {
		    return 1;
		}
	    }
	}
    }
    return 0;
}

/*==================================================================*/
	int AppendToCF(CF_PRED_MGR *cpm_ptr, CF_MATCH_MGR *cmm_ptr, 
		       BNST_DATA *b_ptr,
		       CASE_FRAME *cf_ptr, int n, float maxscore, int maxpos)
/*==================================================================*/
{
    /* ��ά�λؼ��оݤ�����¦�γʥե졼�������� */

    CASE_FRAME *c_ptr = &(cpm_ptr->cf);
    int d, demonstrative;

    if (c_ptr->element_num >= CF_ELEMENT_MAX) {
	return 0;
    }

    /* �ؼ���ξ�� */
    if (cmm_ptr->result_lists_p[0].flag[n] != UNASSIGNED) {
	d = cmm_ptr->result_lists_p[0].flag[n];
	demonstrative = 1;
    }
    else {
	d = c_ptr->element_num;
	demonstrative = 0;
    }

    /* �б�������ɲ� */
    cmm_ptr->result_lists_p[0].flag[n] = d;
    cmm_ptr->result_lists_d[0].flag[d] = n;
    cmm_ptr->result_lists_p[0].pos[n] = maxpos;
    /* cpm_ptr->cmm[0].result_lists_p[0].score[n] = -1; */

    if (OptDiscMethod == OPT_SVM || OptDiscMethod == OPT_DT) {
#ifdef USE_RAWSCORE
	if (maxrawscore < 0) {
	    cmm_ptr->result_lists_p[0].score[n] = 0;
	}
	else if (maxrawscore > 1.0) {
	    cmm_ptr->result_lists_p[0].score[n] = EX_match_exact;
	}
	else {
	    cmm_ptr->result_lists_p[0].score[n] = *(EX_match_score+(int)(maxrawscore*7));
	}
#else
	if (maxscore < 0) {
	    cmm_ptr->result_lists_p[0].score[n] = 0;
	}
	else {
	    cmm_ptr->result_lists_p[0].score[n] = maxscore > 1 ? EX_match_exact : maxscore*11;
	}
#endif
    }
    else {
	cmm_ptr->result_lists_p[0].score[n] = maxscore > 1 ? EX_match_exact : *(EX_match_score+(int)(maxscore*7));
    }

    c_ptr->pp[d][0] = cf_ptr->pp[n][0];
    c_ptr->pp[d][1] = END_M;
    c_ptr->oblig[d] = TRUE;
    cpm_ptr->elem_b_ptr[d] = b_ptr;
    c_ptr->weight[d] = 0;
    c_ptr->adjacent[d] = FALSE;
    if (!demonstrative) {
	cpm_ptr->elem_b_num[d] = -2;	/* ��ά��ɽ�� */
	_make_data_cframe_sm(cpm_ptr, b_ptr);	/* ����: ��Ǽ��꤬ c_ptr->element_num ���� */
	_make_data_cframe_ex(cpm_ptr, b_ptr);
	c_ptr->element_num++;
    }
    else {
	cpm_ptr->elem_b_num[d] = -3;	/* �ȱ���ɽ�� */

	/* �ؼ���ξ�硢��Ȥλؼ����ʬ�Υ�����������Ƥ��� */
	cmm_ptr->pure_score[0] -= cmm_ptr->result_lists_p[0].score[n];
    }
    return 1;
}

/*==================================================================*/
int DeleteFromCF(ELLIPSIS_MGR *em_ptr, CF_PRED_MGR *cpm_ptr, CF_MATCH_MGR *cmm_ptr)
/*==================================================================*/
{
    int i, count = 0;

    /* ��ά�λؼ��оݤ�����¦�γʥե졼�फ�������� */

    for (i = 0; i < cpm_ptr->cf.element_num; i++) {
	if (cpm_ptr->elem_b_num[i] <= -2) {
	    cmm_ptr->result_lists_p[0].flag[cmm_ptr->result_lists_d[0].flag[i]] = -1;
	    cmm_ptr->result_lists_d[0].flag[i] = -1;
	    count++;
	}
    }
    cpm_ptr->cf.element_num -= count;
    return 1;
}

/*==================================================================*/
int EllipsisDetectForVerb(SENTENCE_DATA *sp, ELLIPSIS_MGR *em_ptr, 
			  CF_PRED_MGR *cpm_ptr, CF_MATCH_MGR *cmm_ptr, 
			  CASE_FRAME *cf_ptr, int n, int last)
/*==================================================================*/
{
    /* �Ѹ��Ȥ��ξ�ά�ʤ�Ϳ������ */

    /* cf_ptr = cpm_ptr->cmm[0].cf_ptr �Ǥ��� */
    /* �Ѹ� cpm_ptr �� cf_ptr->pp[n][0] �ʤ���ά����Ƥ���
       cf_ptr->ex[n] �˻��Ƥ���ʸ���õ�� */

    int i, j, current = 1, bend;
    char feature_buffer[DATA_LEN], etc_buffer[DATA_LEN], *cp;
    SENTENCE_DATA *s, *cs;
    BNST_DATA *bp;

    if (OptDiscMethod == OPT_SVM || OptDiscMethod == OPT_DT) {
	maxscore = AssignReferentThresholdForSVM;
	maxtag = NULL;
    }
    else {
	maxscore = 0;
    }

    cs = sentence_data + sp->Sen_num - 1;
    memset(Bcheck, 0, sizeof(int)*BNST_MAX);

    for (i = 0; ExtraTags[i][0]; i++) {
	if (EllipsisDetectForVerbSubcontractExtraTags(cs, em_ptr, cpm_ptr, cmm_ptr, 
						      i, cf_ptr, n)) {
	    goto EvalAntecedent;
	}
    }

    /* �Ƥ�ߤ� (PARA �ʤ� child �Ѹ�) */
    if (cpm_ptr->pred_b_ptr->parent) {
	/* �Ƥ� PARA */
	if (cpm_ptr->pred_b_ptr->parent->para_top_p) {
	    /* ��ʬ��������Ѹ� */
	    for (i = 0; cpm_ptr->pred_b_ptr->parent->child[i]; i++) {
		/* PARA �λҶ��ǡ���ʬ��ʳ��������Ѹ� */
		if (cpm_ptr->pred_b_ptr->parent->child[i] != cpm_ptr->pred_b_ptr &&
		    cpm_ptr->pred_b_ptr->parent->child[i]->para_type == PARA_NORMAL) {
		    if (SearchCaseComponent(cs, em_ptr, cpm_ptr, cmm_ptr, 
					    cpm_ptr->pred_b_ptr->parent->child[i], cf_ptr, n, RANKP, PLOC_PV)) {
			goto EvalAntecedent;
		    }
		}
	    }

	    /* Ϣ�ѤǷ�����Ѹ� (����ΤȤ�) */
	    if (cpm_ptr->pred_b_ptr->parent->parent && 
		check_feature(cpm_ptr->pred_b_ptr->f, "��:Ϣ��")) {
		if (SearchCaseComponent(cs, em_ptr, cpm_ptr, cmm_ptr, 
					cpm_ptr->pred_b_ptr->parent->parent, cf_ptr, n, RANK3, PLOC_PARENTV)) {
		    goto EvalAntecedent;
		}
	    }
	}
	/* �Ȥꤢ������Ϣ�ѤǷ���ҤȤľ�ο��Ѹ��Τ� */
	else if (check_feature(cpm_ptr->pred_b_ptr->f, "��:Ϣ��")) {
	    if (SearchCaseComponent(cs, em_ptr, cpm_ptr, cmm_ptr, 
				    cpm_ptr->pred_b_ptr->parent, cf_ptr, n, RANK3, PLOC_PARENTV)) {
		goto EvalAntecedent;
	    }
	}
	/* �֡�������פʤ� */
	else if (check_feature(cpm_ptr->pred_b_ptr->f, "��°�᰷��")) {
	    if (SearchCaseComponent(cs, em_ptr, cpm_ptr, cmm_ptr, 
				    cpm_ptr->pred_b_ptr->parent->parent, cf_ptr, n, RANK3, PLOC_PARENTV)) {
		goto EvalAntecedent;
	    }
	}
    }

    /* �Ҷ� (�Ѹ�) �򸫤�
       check ���� feature: �Ѹ� -> ��:Ϣ�� */
    for (i = 0; cpm_ptr->pred_b_ptr->child[i]; i++) {
	/* �Ҷ��� <PARA> */
	if (cpm_ptr->pred_b_ptr->child[i]->para_top_p) {
	    for (j = 0; cpm_ptr->pred_b_ptr->child[i]->child[j]; j++) {
		if (!cpm_ptr->pred_b_ptr->child[i]->child[j]->para_top_p && 
		    check_feature(cpm_ptr->pred_b_ptr->child[i]->child[j]->f, "��:Ϣ��")) {
		    if (SearchCaseComponent(cs, em_ptr, cpm_ptr, cmm_ptr, 
					    cpm_ptr->pred_b_ptr->child[i]->child[j], cf_ptr, n, RANK3, PLOC_CHILDPV)) {
			goto EvalAntecedent;
		    }
		}
	    }
	}
	else {
	    if (check_feature(cpm_ptr->pred_b_ptr->child[i]->f, "��:Ϣ��")) {
		if (SearchCaseComponent(cs, em_ptr, cpm_ptr, cmm_ptr, 
					cpm_ptr->pred_b_ptr->child[i], cf_ptr, n, RANK3, PLOC_CHILDV)) {
		    goto EvalAntecedent;
		}
	    }
	}
    }

    if ((cp = check_feature(cpm_ptr->pred_b_ptr->f, "�ȱ��ҥ��"))) {
	if (str_eq(cp, "�ȱ��ҥ��:��")) {
	    /* ��������Ѹ��˷�������Ǥ�ߤ� (����̾��ΤȤ�) */
	    if (SearchCaseComponent(cs, em_ptr, cpm_ptr, cmm_ptr, 
				    cs->bnst_data+cpm_ptr->pred_b_ptr->dpnd_head, cf_ptr, n, RANK3, PLOC_NPARENTV)) {
		goto EvalAntecedent;
	    }

	    /* ��������Ѹ��˷��뽾°��˷�������Ǥ�ߤ� */
	    for (i = 0; (cs->bnst_data+cpm_ptr->pred_b_ptr->dpnd_head)->child[i]; i++) {
		if (check_feature((cs->bnst_data+cpm_ptr->pred_b_ptr->dpnd_head)->child[i]->f, "�Ѹ�")) {
		    if (SearchCaseComponent(cs, em_ptr, cpm_ptr, cmm_ptr, 
					    (cs->bnst_data+cpm_ptr->pred_b_ptr->dpnd_head)->child[i], 
					    cf_ptr, n, RANK3, PLOC_NPARENTCV)) {
			goto EvalAntecedent;
		    }
		}
	    }
	}
	else {
	    i = cpm_ptr->pred_b_ptr->num+atoi(cp+11);
	    if (i >= 0 && i < cs->Bnst_num) {
		if (SearchCaseComponent(cs, em_ptr, cpm_ptr, cmm_ptr, cs->bnst_data+i, cf_ptr, n, RANK3, PLOC_NOTHERS)) {
		    goto EvalAntecedent;
		}
	    }
	}
    }

    /* ����ʸ���θ���õ�� (�����Ѹ��γ����ǤˤʤäƤ����ΰʳ�) */
    for (s = cs; s >= sentence_data; s--) {
	bend = s->Bnst_num;

	for (i = bend-1; i >= 0; i--) {
	    if (current) {
		if (Bcheck[i]) {
		    continue;
		}
		else if ((!check_feature((s->bnst_data+i)->f, "��:Ϣ��") && 
		     (s->bnst_data+i)->dpnd_head == cpm_ptr->pred_b_ptr->num) || /* �Ѹ���ľ�ܷ���ʤ� (Ϣ�Ѥϲ�) */
		    (!check_feature(cpm_ptr->pred_b_ptr->f, "��:̤��") && /* �֡� V �Τ� N�פ���� */
		     !check_feature(cpm_ptr->pred_b_ptr->f, "��:����") && 
		     (cpm_ptr->pred_b_ptr->dpnd_head == (s->bnst_data+i)->num)) || /* �Ѹ����оݤ˷���ʤ� */
		    CheckCaseComponent(cpm_ptr, i) || /* ���Ѹ�������ʸ�������ǤȤ��Ƥ⤿�ʤ� */
		    CheckAnaphor(banlist, (s->bnst_data+i)->Jiritu_Go)) {
		    RegisterAnaphor(banlist, (s->bnst_data+i)->Jiritu_Go);
		    continue;
		}

		/* ��ʬ���Ȥ���ʤ� (��(N ��) �� N ���פ�����ΤǶػߥꥹ�Ȥ���Ͽ���ʤ�) */
		if (i == cpm_ptr->pred_b_ptr->num) {
		    continue;
		}

		/* �Ѹ��γ����ǤȤʤ�γ� */
		if (0 && i < cpm_ptr->pred_b_ptr->num && 
		    check_feature((s->bnst_data+i)->f, "�γ��Ѹ������å�") && 
		    !MatchPP(cf_ptr->pp[n][0], "��")) {
		    bp = s->bnst_data+i;
		    /* �γʤ�Ϣ³�����Ф� */
		    while (check_feature(bp->f, "��:�γ�")) {
			bp = s->bnst_data+bp->dpnd_head;
		    }
		    /* A �� B �� V 
		       B(bp) �� head �� V �Ǥ���� */
		    if (bp->dpnd_head == cpm_ptr->pred_b_ptr->num) {
			if (EllipsisDetectForVerbSubcontract(s, cs, em_ptr, cpm_ptr, cmm_ptr, 
							     s->bnst_data+i, cf_ptr, n, RANK2, 0)) {
			    goto EvalAntecedent;
			}
			continue;
		    }
		}

		/* A �� B �� �� V */
		if ((cp = check_feature((s->bnst_data+i)->f, "��ά��������å�"))) {
		    if ((s->bnst_data+i+atoi(cp+17))->dpnd_head == cpm_ptr->pred_b_ptr->num) {
			RegisterAnaphor(banlist, (s->bnst_data+i)->Jiritu_Go);
			continue;
		    }
		}
	    }

	    /* ��ά���ǤȤʤ뤿��ΤȤꤢ�����ξ�� */
	    if (!CheckPureNoun(s->bnst_data+i) || 
		CheckAnaphor(banlist, (s->bnst_data+i)->Jiritu_Go)) {
		continue;
	    }

	    if (EllipsisDetectForVerbSubcontract(s, cs, em_ptr, cpm_ptr, cmm_ptr, 
						 s->bnst_data+i, cf_ptr, n, RANK1, 0)) {
		goto EvalAntecedent;
	    }
	}
	if (current)
	    current = 0;
    }

  EvalAntecedent:
    if (OptDiscMethod == OPT_SVM || OptDiscMethod == OPT_DT) {
	if (MatchPP(cf_ptr->pp[n][0],"��") || 
	    MatchPP(cf_ptr->pp[n][0], "��") || 
	    MatchPP(cf_ptr->pp[n][0], "��") || 
	    MatchPP(cf_ptr->pp[n][0], "���") || 
	    MatchPP(cf_ptr->pp[n][0], "����") || 
	    MatchPP(cf_ptr->pp[n][0], "�ޥ�") || 
	    MatchPP(cf_ptr->pp[n][0], "����") || 
	    MatchPP(cf_ptr->pp[n][0], "��") || 
	    MatchPP(cf_ptr->pp[n][0], "���δط�")) {
	    sprintf(feature_buffer, "��ά���Ϥʤ�-%s", 
		    pp_code_to_kstr(cf_ptr->pp[n][0]));
	    assign_cfeature(&(em_ptr->f), feature_buffer);
	    return 0;
	}

	if (maxscore > AssignReferentThresholdForSVM) {
	    if (maxtag) {
		if (str_eq(maxtag, "������-��")) {
		    sprintf(feature_buffer, "C��;��������:�͡�;%s;-1;-1;1", 
			    pp_code_to_kstr(cf_ptr->pp[n][0]));
		    assign_cfeature(&(em_ptr->f), feature_buffer);
		    em_ptr->cc[cf_ptr->pp[n][0]].bnst = ELLIPSIS_TAG_UNSPECIFIED_PEOPLE;
		    return 0;
		}
		else if (str_eq(maxtag, "�оݳ�")) {
		    sprintf(feature_buffer, "C��;���оݳ���;%s;-1;-1;1", 
			    pp_code_to_kstr(cf_ptr->pp[n][0]));
		    assign_cfeature(&(em_ptr->f), feature_buffer);
		    em_ptr->cc[cf_ptr->pp[n][0]].bnst = ELLIPSIS_TAG_EXCEPTION;
		    /* AppendToCF(cpm_ptr, cmm_ptr, cpm_ptr->pred_b_ptr, cf_ptr, n, maxscore, -1); */
		    return 1;
		}
		/*
		else if (str_eq(maxtag, "������ʪ")) {
		    sprintf(feature_buffer, "C��;���㳰:�ʤ���;%s;-1;-1;1", 
			    pp_code_to_kstr(cf_ptr->pp[n][0]));
		    assign_cfeature(&(em_ptr->f), feature_buffer);
		    * �����祹�����λؼ��оݤ� dummy �ǳʥե졼�����¸ 
		       ���줬���ۤ��γʤθ���ˤʤ�ʤ��ʤ�Τ������ *
		    AppendToCF(cpm_ptr, cmm_ptr, cpm_ptr->pred_b_ptr, cf_ptr, n, maxscore, -1);
		    return 1;
		}
		*/
		else if (str_eq(maxtag, "��;�")) {
		    sprintf(feature_buffer, "C��;�ڰ�;Ρ�;%s;-1;-1;1", 
			    pp_code_to_kstr(cf_ptr->pp[n][0]));
		    assign_cfeature(&(em_ptr->f), feature_buffer);
		    em_ptr->cc[cf_ptr->pp[n][0]].bnst = ELLIPSIS_TAG_I_WE;
		    /* AppendToCF(cpm_ptr, cmm_ptr, cpm_ptr->pred_b_ptr, cf_ptr, n, maxscore, -1); */
		    return 1;
		}
		else if (str_eq(maxtag, "������-����")) {
		    sprintf(feature_buffer, "C��;��������:������;%s;-1;-1;1", 
			    pp_code_to_kstr(cf_ptr->pp[n][0]));
		    assign_cfeature(&(em_ptr->f), feature_buffer);
		    em_ptr->cc[cf_ptr->pp[n][0]].bnst = ELLIPSIS_TAG_UNSPECIFIED_CASE;
		    return 1;
		}
	    }

	    /* 
	    if (check_feature(cpm_ptr->pred_b_ptr->f, "��ά�ʻ���")) {
		;
	    }
	    else */ {
		int distance;

		distance = cs-maxs;
		if (distance == 0) {
		    strcpy(etc_buffer, "Ʊ��ʸ");
		}
		else if (distance > 0) {
		    sprintf(etc_buffer, "%dʸ��", distance);
		}

		/* ���ꤷ����ά�ط� */
		sprintf(feature_buffer, "C��;��%s��;%s;%d;%d;%.3f:%s(%s):%dʸ��", 
			(maxs->bnst_data+maxi)->Jiritu_Go, 
			pp_code_to_kstr(cf_ptr->pp[n][0]), 
			distance, maxi, 
			maxscore, maxs->KNPSID ? maxs->KNPSID+5 : "?", 
			etc_buffer, maxi);
		assign_cfeature(&(em_ptr->f), feature_buffer);
		em_ptr->cc[cf_ptr->pp[n][0]].s = maxs;
		em_ptr->cc[cf_ptr->pp[n][0]].bnst = maxi;

		/* �ؼ��оݤ�ʥե졼�����¸ */
		AppendToCF(cpm_ptr, cmm_ptr, maxs->bnst_data+maxi, cf_ptr, n, maxscore, maxpos);

		return 1;
	    }
	}
	return 0;
    }

    /* ��;� */
    if ((cp = check_feature(cpm_ptr->pred_b_ptr->f, "��;�")) && 
	     MatchPP(cf_ptr->pp[n][0], cp+7)) {
	sprintf(feature_buffer, "C��;�ڰ�;Ρ�;%s;-1;-1;1", 
		    pp_code_to_kstr(cf_ptr->pp[n][0]));
	assign_cfeature(&(em_ptr->f), feature_buffer);
	em_ptr->cc[cf_ptr->pp[n][0]].bnst = ELLIPSIS_TAG_I_WE;
	/* AppendToCF(cpm_ptr, cmm_ptr, maxs->bnst_data+maxi, cf_ptr, n, maxscore, maxpos); */
	return 1;
    }
    /* �оݳ� */
    else if ((cp = check_feature(cpm_ptr->pred_b_ptr->f, "�оݳ�")) && 
	     MatchPP(cf_ptr->pp[n][0], cp+7)) {
	sprintf(feature_buffer, "C��;���оݳ���;%s;-1;-1;1", 
		    pp_code_to_kstr(cf_ptr->pp[n][0]));
	assign_cfeature(&(em_ptr->f), feature_buffer);
	em_ptr->cc[cf_ptr->pp[n][0]].bnst = ELLIPSIS_TAG_EXCEPTION;
	/* AppendToCF(cpm_ptr, cmm_ptr, maxs->bnst_data+maxi, cf_ptr, n, maxscore, maxpos); */
	return 1;
    }
    /* ��������:�͡�
       1. �Ѹ������Ȥǥ˳� (��Ȥϥ���) �� <����> ��Ȥꡢ���ͤ�ۤ���Ȥ�
       2. ���� <����> �ǡ����ͤ�ۤ���Ȥ�
       3. ���� V ���� N (���δط�, !Ƚ���), ����̾��, ����̾��Ͻ���
       4. �����������ͤ�겼�ǥ��� <����> ��Ȥ�Ȥ� */
    else if (((cp = check_feature(cpm_ptr->pred_b_ptr->f, "�������")) && 
	      MatchPP(cf_ptr->pp[n][0], cp+9)) || 
	     (cmm_ptr->result_lists_p[0].flag[n] == UNASSIGNED && /* �ؼ���ǤϤʤ� */
	      ((MatchPP(cf_ptr->pp[n][0], "��") && 
		cf_match_element(cf_ptr->sm[n], "����", FALSE) && 
		(maxscore <= AssignGaCaseThreshold)) || 
	       (MatchPP(cf_ptr->pp[n][0], "��") && 
		cf_match_element(cf_ptr->sm[n], "����", FALSE) && 
		!cf_match_element(cf_ptr->sm[n], "���", FALSE) && 
		maxscore <= AssignGaCaseThreshold && 
		(check_feature(cpm_ptr->pred_b_ptr->f, "�����") || 
		 check_feature(cpm_ptr->pred_b_ptr->f, "������") || 
		 check_feature(cpm_ptr->pred_b_ptr->f, "�ɲü���") || 
		 check_feature(cpm_ptr->pred_b_ptr->f, "���Ѹ��ʲ���"))) || 
	       (cpm_ptr->pred_b_ptr->parent && 
		(check_feature(cpm_ptr->pred_b_ptr->parent->f, "���δط�") || 
		 check_feature(cpm_ptr->pred_b_ptr->parent->f, "���δط���ǽ��") || 
		 check_feature(cpm_ptr->pred_b_ptr->parent->f, "���δط�Ƚ��")) && 
		!check_feature(cpm_ptr->pred_b_ptr->parent->f, "����") && 
		!check_feature(cpm_ptr->pred_b_ptr->parent->f, "����̾��") && 
		!check_feature(cpm_ptr->pred_b_ptr->parent->f, "����̾��") && 
		!check_feature(cpm_ptr->pred_b_ptr->parent->f, "�Ѹ�") && 
		check_feature(cpm_ptr->pred_b_ptr->f, "��:Ϣ��") && 
		cf_ptr->pp[n][0] == pp_kstr_to_code("��"))))) {
	/* �ʥե졼������롼�פΤȤ��� feature ��Ϳ����Τ����� */
	sprintf(feature_buffer, "C��;��������:�͡�;%s;-1;-1;1", 
		pp_code_to_kstr(cf_ptr->pp[n][0]));
	assign_cfeature(&(em_ptr->f), feature_buffer);
	em_ptr->cc[cf_ptr->pp[n][0]].bnst = ELLIPSIS_TAG_UNSPECIFIED_PEOPLE;
    }
    /* ���ξ��Ͼ�ά���Ǥ�õ������Ͽ���ʤ� 
       (�������Ǥϥǡ����򸫤뤿�ᡢ�������ά���Ϥ�ԤäƤ���) 
       �ǳ�, �ȳ�, ����, �����, �ޥǳ� */
    else if (!MatchPPn(cf_ptr->pp[n][0], DiscAddedCases) && 
	     (MatchPP(cf_ptr->pp[n][0],"��") || 
	      MatchPP(cf_ptr->pp[n][0], "��") || 
	      MatchPP(cf_ptr->pp[n][0], "��") || 
	      MatchPP(cf_ptr->pp[n][0], "���") || 
	      MatchPP(cf_ptr->pp[n][0], "����") || 
	      MatchPP(cf_ptr->pp[n][0], "�ޥ�") || 
	      MatchPP(cf_ptr->pp[n][0], "����") || 
	      MatchPP(cf_ptr->pp[n][0], "��") || 
	      MatchPP(cf_ptr->pp[n][0], "���δط�"))) {
	sprintf(feature_buffer, "��ά���Ϥʤ�-%s", 
		pp_code_to_kstr(cf_ptr->pp[n][0]));
	assign_cfeature(&(em_ptr->f), feature_buffer);
    }
    else if (check_feature(cpm_ptr->pred_b_ptr->f, "��ά�ʻ���")) {
	;
    }
    /* ����:���� ������Ȥ�����������:�����פȤ���
    else if (maxscore > 0 && 
	     MatchPP(cf_ptr->pp[n][0], "��") && 
	     cf_match_element(cf_ptr->sm[n], "����", TRUE)) {
	sprintf(feature_buffer, "C��;��������:������;%s;-1;-1;1", 
		pp_code_to_kstr(cf_ptr->pp[n][0]));
	assign_cfeature(&(em_ptr->f), feature_buffer);
	em_ptr->cc[cf_ptr->pp[n][0]].bnst = ELLIPSIS_TAG_UNSPECIFIED_CASE;
	AppendToCF(cpm_ptr, cmm_ptr, maxs->bnst_data+maxi, cf_ptr, n, maxscore, maxpos);
	return 1;
    } */
    /* ���:��ʸ ������Ȥ��ǡ��ɤ��ޥå����ʤ��Ȥ��ϡ���ʸ�פȤ��� */
    else if (sp->Sen_num > 1 && 
	     maxscore > 0 && 
	     maxscore < 0.8 && 
	     MatchPP(cf_ptr->pp[n][0], "��") && 
	     cf_match_element(cf_ptr->sm[n], "��ʸ", TRUE)) {
	sprintf(feature_buffer, "C��;����ʸ��;%s;-1;-1;1", 
		pp_code_to_kstr(cf_ptr->pp[n][0]));
	assign_cfeature(&(em_ptr->f), feature_buffer);
	em_ptr->cc[cf_ptr->pp[n][0]].bnst = ELLIPSIS_TAG_PRE_SENTENCE;
	/* AppendToCF(cpm_ptr, cmm_ptr, maxs->bnst_data+maxi, cf_ptr, n, maxscore, maxpos); */
	return 1;
    }
    /* ���ͤ���ɬ�פ��� */
    else if (maxscore > 0 && 
	     maxscore < AssignReferentThresholdAnonymousThing && 
	     MatchPP(cf_ptr->pp[n][0], "��") && 
	     check_feature(cpm_ptr->pred_b_ptr->f, "���Ѹ��ʲ���") && 
	     sm_match_check(sm2code("���ʪ"), cpm_ptr->pred_b_ptr->SM_code)) {
	/* sprintf(feature_buffer, "C��;���㳰:�ʤ���;%s;-1;-1;1", 
		pp_code_to_kstr(cf_ptr->pp[n][0]));
	assign_cfeature(&(em_ptr->f), feature_buffer); */
	/* �����祹�����λؼ��оݤ� dummy �ǳʥե졼�����¸ 
	   ���줬���ۤ��γʤθ���ˤʤ�ʤ��ʤ�Τ������ */
	/* AppendToCF(cpm_ptr, cmm_ptr, maxs->bnst_data+maxi, cf_ptr, n, maxscore, maxpos); */
	return 1;
    }
    /* �ʲ��Ϥǳʥե졼�����ꤷ�Ƥ��������ͤʤ�
       ����¾�ξ�������: AssignReferentThreshold
       �ؼ���ΤȤ������ͤʤ� */
    else if (maxscore > 0 && 
	     (cmm_ptr->result_lists_p[0].flag[n] != UNASSIGNED || /* �ؼ��� */
	      (cpm_ptr->decided == CF_DECIDED && 
	       maxscore > AssignReferentThresholdDecided) || /* maxscore == 0 �ΤȤ���ά�γ�����ƤϤʤ��Τ� > 0 �ˤ���ɬ�פ����� */
	      maxscore > AssignReferentThreshold)) {
	int distance;
	char *word;

	if (distance == 0 && cpm_ptr->pred_b_ptr->num > maxi && 
	    check_feature((maxs->bnst_data+maxi)->f, "�γ��Ѹ������å�") && 
	    maxscore < AssignReferentThresholdHigh) {
	    return 0;
	}

	distance = cs-maxs;
	if (distance == 0) {
	    strcpy(etc_buffer, "Ʊ��ʸ");
	}
	else if (distance > 0) {
	    sprintf(etc_buffer, "%dʸ��", distance);
	}

	word = make_print_string(maxs->bnst_data+maxi, 0);

	/* ���ꤷ����ά�ط� */
	sprintf(feature_buffer, "C��;��%s��;%s;%d;%d;%.3f:%s(%s):%dʸ��", 
		word ? word : "?", 
		pp_code_to_kstr(cf_ptr->pp[n][0]), 
		distance, maxi, 
		maxscore, maxs->KNPSID ? maxs->KNPSID+5 : "?", 
		etc_buffer, maxi);
	assign_cfeature(&(em_ptr->f), feature_buffer);
	if (word) free(word);
	em_ptr->cc[cf_ptr->pp[n][0]].s = maxs;
	em_ptr->cc[cf_ptr->pp[n][0]].bnst = maxi;
	em_ptr->cc[cf_ptr->pp[n][0]].dist = distance;

	/* �ؼ��оݤ�ʥե졼�����¸ */
	AppendToCF(cpm_ptr, cmm_ptr, maxs->bnst_data+maxi, cf_ptr, n, maxscore, maxpos);

	return 1;
    }
    return 0;
}

/*==================================================================*/
	       int GetElementID(CASE_FRAME *cfp, int c)
/*==================================================================*/
{
    /* �ʤ��ֹ椫�顢�ʥե졼��������ֹ���Ѵ����� */

    int i;

    for (i = 0; i < cfp->element_num; i++) {
	if (cfp->pp[i][0] == c) {
	    return i;
	}
    }
    return -1;
}

/*==================================================================*/
int MarkEllipsisCase(CF_PRED_MGR *cpm_ptr, CASE_FRAME *cf_ptr, int n)
/*==================================================================*/
{
    char feature_buffer[DATA_LEN];

    /* ��ά����Ƥ���ʤ�ޡ���
    sprintf(feature_buffer, "C��ά-%s", 
	    pp_code_to_kstr(cf_ptr->pp[n][0]));
    assign_cfeature(&(cpm_ptr->pred_b_ptr->f), feature_buffer);
    */

    /* <������:����> �򥬳ʤȤ��ƤȤ�Ƚ��� */
    if (check_feature(cpm_ptr->pred_b_ptr->f, "���֥���ά") && 
	MatchPP(cf_ptr->pp[n][0], "��")) {
	sprintf(feature_buffer, "C��;��������:������;%s;-1;-1;1", 
		pp_code_to_kstr(cf_ptr->pp[n][0]));
	assign_cfeature(&(cpm_ptr->pred_b_ptr->f), feature_buffer);
	return 0;
    }
    return 1;
}

/*==================================================================*/
void AppendCfFeature(ELLIPSIS_MGR *em_ptr, CF_PRED_MGR *cpm_ptr, CASE_FRAME *cf_ptr, int n)
/*==================================================================*/
{
    char feature_buffer[DATA_LEN];

    /* �ʥե졼�ब <���ν�> ���Ĥ��ɤ��� */
    if (cf_ptr->etcflag & CF_GA_SEMI_SUBJECT) {
	sprintf(feature_buffer, "�ʥե졼��-%s-���ν�", pp_code_to_kstr(cf_ptr->pp[n][0]));
	assign_cfeature(&(em_ptr->f), feature_buffer);
    }
    /* �ʥե졼�ब <����> ���Ĥ��ɤ��� */
    else if (cf_match_element(cf_ptr->sm[n], "����", FALSE)) {
	sprintf(feature_buffer, "�ʥե졼��-%s-����", pp_code_to_kstr(cf_ptr->pp[n][0]));
	assign_cfeature(&(em_ptr->f), feature_buffer);
    }


    /* �ʥե졼�ब <��ʸ> ���Ĥ��ɤ��� */
    if (cf_match_element(cf_ptr->sm[n], "��ʸ", TRUE)) {
	sprintf(feature_buffer, "�ʥե졼��-%s-��ʸ", pp_code_to_kstr(cf_ptr->pp[n][0]));
	assign_cfeature(&(em_ptr->f), feature_buffer);
    }

    if (sm_match_check(sm2code("���ʪ"), cpm_ptr->pred_b_ptr->SM_code)) {
	sprintf(feature_buffer, "�ʥե졼��-%s-���ʪ", pp_code_to_kstr(cf_ptr->pp[n][0]));
	assign_cfeature(&(em_ptr->f), feature_buffer);
    }
}

/*==================================================================*/
float EllipsisDetectForVerbMain(SENTENCE_DATA *sp, ELLIPSIS_MGR *em_ptr, CF_PRED_MGR *cpm_ptr, CF_MATCH_MGR *cmm_ptr, 
				CASE_FRAME *cf_ptr, char **order, int mainflag, int onceflag)
/*==================================================================*/
{
    int i, j, num, result, toflag = 0;
    int cases[PP_NUMBER], count = 0;

    /* onceflag �����ꤵ�줿���ˤϡ�
       ��ά���Ǥ�ҤȤ�ȯ���������ΤȤ��˺Ǥ�褤�ʥե졼���õ��
       �� �ξ��� (cmm ��Ϣ) */

    /* ��<��ʸ>�� ���� V ������ 
       �ȳʤ�����Ȥ�����ʤ��ά�Ȥ��ʤ� 
       (�ʥե졼��¦��<��ʸ>�ϥ����å����Ƥ��ʤ�) */
    for (i = 0; i < cf_ptr->element_num; i++) {
	num = cmm_ptr->result_lists_p[0].flag[i];
	if (num != UNASSIGNED && 
	    MatchPP(cf_ptr->pp[i][0], "��") && 
	    check_feature(cpm_ptr->elem_b_ptr[num]->f, "��ʸ")) {
	    toflag = 1;
	    break;
	}
    }

    for (j = 0; *order[j]; j++) {
	cases[count++] = pp_kstr_to_code(order[j]);
    }
    for (j = 0; DiscAddedCases[j] != END_M; j++) {
	cases[count++] = DiscAddedCases[j];
    }
    cases[count] = END_M;

    /* �ʤ�Ϳ����줿���֤� */
    for (j = 0; cases[j] != END_M; j++) {
	for (i = 0; i < cf_ptr->element_num; i++) {
	    if (((cf_ptr->pp[i][0] == cases[j] && 
		 cmm_ptr->result_lists_p[0].flag[i] == UNASSIGNED) || 
		(OptDemo == TRUE && /* ������Ƥ����äơ��ؼ���ΤȤ� */
		 cmm_ptr->result_lists_p[0].flag[i] != UNASSIGNED && 
		 cf_ptr->pp[i][0] == cases[j] && 
		 check_feature(cpm_ptr->elem_b_ptr[cmm_ptr->result_lists_p[0].flag[i]]->f, "��ά�����оݻؼ���"))) && 
		!(toflag && MatchPP(cf_ptr->pp[i][0], "��"))) {
		if ((MarkEllipsisCase(cpm_ptr, cf_ptr, i)) == 0) {
		    continue;
		}
		result = EllipsisDetectForVerb(sp, em_ptr, cpm_ptr, cmm_ptr, cf_ptr, i, mainflag);
		AppendCfFeature(em_ptr, cpm_ptr, cf_ptr, i);
		if (result) {
		    em_ptr->cc[cf_ptr->pp[i][0]].score = maxscore;

		    if (OptDiscMethod == OPT_SVM || OptDiscMethod == OPT_DT) {
#ifdef USE_RAWSCORE
			em_ptr->score += maxrawscore > 1.0 ? EX_match_exact : maxrawscore < 0 ? 0 : *(EX_match_score+(int)(maxrawscore*7));
#else
			em_ptr->score += maxscore > 1.0 ? EX_match_exact : maxscore < 0 ? 0 : 11*maxscore;
#endif
		    }
		    else {
			if (maxscore == (float)EX_match_subject/11) {
			    em_ptr->score += EX_match_subject;
			}
			else {
			    em_ptr->score += maxscore > 1.0 ? EX_match_exact : *(EX_match_score+(int)(maxscore*7));
			}
		    }

		    if (onceflag) {
			/* �ҤȤĤξ�ά�λؼ��оݤ�ߤĤ����Τǡ�
			   �����Ǥ�äȤ⥹�����ι⤤�ʥե졼����Ĵ������ */
			find_best_cf(sp, cpm_ptr, -1, 0);
			return em_ptr->score;
		    }
		}
	    }
	}
    }

    /* onceflag ���ˡ����ꤵ�줿�ʤ��ɤ�⸫�Ĥ���ʤ���С�
       �ʲ��ǤɤγʤǤ�褤���鸫�Ĥ��� */

    for (i = 0; i < cf_ptr->element_num; i++) {
	num = cmm_ptr->result_lists_p[0].flag[i];
	/* �ʲ��γʤξ�硢��ά���Ǥ�ǧ�ꤷ�ʤ�
	   ���ֳ�, ������, ̵��, ʣ�缭
	*/
	if (num == UNASSIGNED && 
	    !(toflag && str_eq(pp_code_to_kstr(cf_ptr->pp[i][0]), "��")) && 
	    !(cmm_ptr->cf_ptr->pp[i][0] > 8 && cf_ptr->pp[i][0] < 38) && 
	    !MatchPP(cf_ptr->pp[i][0], "����") && 
	    !MatchPP(cf_ptr->pp[i][0], "��") && 
	    !MatchPP(cf_ptr->pp[i][0], "����") && 
	    !MatchPP(cf_ptr->pp[i][0], "���δط�") && 
	    !MatchPP(cf_ptr->pp[i][0], "��")) {
	    if ((MarkEllipsisCase(cpm_ptr, cf_ptr, i)) == 0) {
		continue;
	    }
	    result = EllipsisDetectForVerb(sp, em_ptr, cpm_ptr, cmm_ptr, cf_ptr, i, mainflag);
	    AppendCfFeature(em_ptr, cpm_ptr, cf_ptr, i);
	    if (result) {
		em_ptr->cc[cf_ptr->pp[i][0]].score = maxscore;
		/* 
		if (maxscore == (float)EX_match_subject/11) {
		    em_ptr->score += EX_match_subject;
		}
		else */
		em_ptr->score += maxscore > 1.0 ? EX_match_exact : *(EX_match_score+(int)(maxscore*7));
		if (onceflag) {
		    find_best_cf(sp, cpm_ptr, -1, 0);
		    return em_ptr->score;
		}
	    }
	}
    }

    if (onceflag) {
	return -1;
    }
    return em_ptr->score;
}

/*==================================================================*/
	    int CompareCPM(CF_PRED_MGR *a, CF_PRED_MGR *b)
/*==================================================================*/
{
    int i, j, flag;

    /* �ۤʤ���� 1 ���֤� */

    if (a->cf.element_num != b->cf.element_num) {
	return 1;
    }

    /* ���֤��㤦���Ȥ�����Τ�ñ����ӤǤϤ��� */

    for (i = 0; i < a->cf.element_num; i++) {
	flag = 0;
	for (j = 0; j < b->cf.element_num; j++) {
	    if (a->elem_b_ptr[i] == b->elem_b_ptr[j]) {
		flag = 1;
		break;
	    }
	}
	if (!flag) {
	    return 1;
	}
    }
    return 0;
}
/*==================================================================*/
	   int CompareCMM(CF_PRED_MGR *ap, CF_MATCH_MGR *a, 
			  CF_PRED_MGR *bp, CF_MATCH_MGR *b)
/*==================================================================*/
{
    int i;

    /* �ۤʤ���� 1 ���֤� */

    for (i = 0; i < a->cf_ptr->element_num; i++) {
	if (a->result_lists_p[0].flag[i] != UNASSIGNED && 
	    ap->elem_b_ptr[a->result_lists_p[0].flag[i]] != bp->elem_b_ptr[b->result_lists_p[0].flag[i]]) {
	    return 1;
	}
    }
    return 0;
}

/*==================================================================*/
int CompareAssignList(ELLIPSIS_MGR *maxem, CF_PRED_MGR *cpm, CF_MATCH_MGR *cmm)
/*==================================================================*/
{
    int i;

    for (i = 0; i < maxem->result_num; i++) {
	/* Ʊ����Τ����Ǥˤ����� */
	if (maxem->ecmm[i].cmm.cf_ptr == cmm->cf_ptr && 
	    maxem->ecmm[i].element_num == cpm->cf.element_num && 
	    !CompareCPM(&(maxem->ecmm[i].cpm), cpm) && 
	    !CompareCMM(&(maxem->ecmm[i].cpm), &(maxem->ecmm[i].cmm), cpm, cmm)) {
	    return 0;
	}
    }
    return 1;
}

/*==================================================================*/
      int CompareClosestScore(CF_MATCH_MGR *a, CF_MATCH_MGR *b)
/*==================================================================*/
{
    int i, acount = 0, bcount = 0;
    float ascore = 0, bscore = 0;

    for (i = 0; i < a->cf_ptr->element_num; i++) {
	if (a->result_lists_p[0].flag[i] != UNASSIGNED && 
	    a->cf_ptr->adjacent[i] == TRUE) {
	    acount++;
	    ascore = a->result_lists_p[0].score[i];
	}
    }

    for (i = 0; i < b->cf_ptr->element_num; i++) {
	if (b->result_lists_p[0].flag[i] != UNASSIGNED && 
	    b->cf_ptr->adjacent[i] == TRUE) {
	    bcount++;
	    bscore = b->result_lists_p[0].score[i];
	}
    }

    if (acount < bcount || 
	(acount == bcount && 
	    ascore < bscore)) {
	return 1;
    }
    return 0;
}

/*==================================================================*/
void FindBestCFforContext(SENTENCE_DATA *sp, ELLIPSIS_MGR *maxem, CF_PRED_MGR *cpm_ptr, 
			  char **order, int mainflag)
/*==================================================================*/
{
    int k, l, frame_num;
    CASE_FRAME **cf_array;
    CF_MATCH_MGR cmm;
    ELLIPSIS_CMM tempecmm;
    ELLIPSIS_MGR workem;

    InitEllipsisMGR(&workem);

    if (OptDiscFlag & OPT_DISC_OR_CF) {
	frame_num = 0;
	cf_array = (CASE_FRAME **)malloc_data(sizeof(CASE_FRAME *), "FindBestCFforContext");
	for (l = 0; l < cpm_ptr->pred_b_ptr->cf_num; l++) {
	    if ((cpm_ptr->pred_b_ptr->cf_ptr+l)->etcflag & CF_SUM) {
		*cf_array = cpm_ptr->pred_b_ptr->cf_ptr+l;
		frame_num = 1;
		break;
	    }
	}
    }
    else {
	if (cpm_ptr->decided == CF_CAND_DECIDED) {
	    frame_num = cpm_ptr->tie_num;
	    cf_array = (CASE_FRAME **)malloc_data(sizeof(CASE_FRAME *)*frame_num, "FindBestCFforContext");
	    for (l = 0; l < frame_num; l++) {
		*(cf_array+l) = cpm_ptr->cmm[l].cf_ptr;
	    }
	}
	else {
	    frame_num = cpm_ptr->pred_b_ptr->cf_num;
	    cf_array = (CASE_FRAME **)malloc_data(sizeof(CASE_FRAME *)*frame_num, "FindBestCFforContext");
	    for (l = 0; l < frame_num; l++) {
		*(cf_array+l) = cpm_ptr->pred_b_ptr->cf_ptr+l;
	    }
	}
    }

    /* ����γʥե졼��ˤĤ��ƾ�ά���Ϥ�¹� */

    for (l = 0; l < frame_num; l++) {
	/* OR �γʥե졼������ */
	if (((*(cf_array+l))->etcflag & CF_SUM) && frame_num != 1) {
	    continue;
	}

	/* �ʥե졼��򲾻��� */
	cmm.cf_ptr = *(cf_array+l);
	cpm_ptr->result_num = 1;

	/* ����¦�����Ǥ�����
	   �ȱ����ϻ��Ϥ��Ǥˤ�������Ǥ��񤭤��Ƥ��ޤ��ΤǤ����Ǻ�����
	   ����ʳ��ΤȤ��ϲ��� DeleteFromCF() �Ǿ�ά���Ǥ򥯥ꥢ */
	if (OptDemo == TRUE) {
	    make_data_cframe(sp, cpm_ptr);
	}

	/* ����������Ǥ��б��Ť� */
	case_frame_match(cpm_ptr, &cmm, OptCFMode, -1);
	cpm_ptr->score = cmm.score;

	ClearEllipsisMGR(&workem);
	EllipsisDetectForVerbMain(sp, &workem, cpm_ptr, &cmm, 
				  *(cf_array+l), 
				  order, mainflag, 0);

	if (cmm.score >= 0) {
	    /* ľ�ܤγ����Ǥ����������Ƥ��ʤ���������­�� */
	    workem.score += cmm.pure_score[0];
	    /* ������ */
	    workem.score /= sqrt((double)(count_pat_element(cmm.cf_ptr, 
							     &(cmm.result_lists_p[0]))));
	    cmm.score = workem.score;
	}
	/* �ʲ��ϼ��ԤΤȤ� -- ���Ϥ�ҤȤĤ�����̤�����뤿���
	   ���祹�����Υǥե���Ȥ� -2 �ˤ��Ƥ��� */
	else {
	    workem.score = cmm.score;
	}

	/* DEBUG ɽ�� */
	if (VerboseLevel >= VERBOSE3) {
	    fprintf(stdout, "�� �ʥե졼�� %d\n", l);
	    print_data_cframe(cpm_ptr, &cmm);
	    print_good_crrspnds(cpm_ptr, &cmm, 1);
	    fprintf(stdout, "   FEATURES: ");
	    print_feature(workem.f, Outfp);
	    fputc('\n', Outfp);
	}

	if (workem.score > maxem->score || 
	    (workem.score == maxem->score && 
	     CompareClosestScore(&(maxem->ecmm[0].cmm), &cmm))) {
	    maxem->cpm = workem.cpm;
	    for (k = 0; k < CASE_MAX_NUM; k++) {
		maxem->cc[k] = workem.cc[k];
	    }
	    maxem->score = workem.score;
	    maxem->f = workem.f;
	    workem.f = NULL;

	    /* �ҤȤĤ��Ĥ��餹 */
	    for (k = maxem->result_num >= CMM_MAX-1 ? maxem->result_num-1 : maxem->result_num; k >= 0; k--) {
		maxem->ecmm[k+1] = maxem->ecmm[k];
	    }

	    /* ���󤬺���ޥå� */
	    maxem->ecmm[0].cmm = cmm;
	    maxem->ecmm[0].cpm = *cpm_ptr;
	    maxem->ecmm[0].element_num = cpm_ptr->cf.element_num;

	    if (maxem->result_num < CMM_MAX-1) {
		maxem->result_num++;
	    }
	}
	/* ����������γʥե졼��(�������)����¸ */
	else if (CompareAssignList(maxem, cpm_ptr, &cmm)) {
	    maxem->ecmm[maxem->result_num].cmm = cmm;
	    maxem->ecmm[maxem->result_num].cpm = *cpm_ptr;
	    maxem->ecmm[maxem->result_num].element_num = cpm_ptr->cf.element_num;
	    for (k = maxem->result_num-1; k >= 0; k--) {
		if (maxem->ecmm[k].cmm.score < maxem->ecmm[k+1].cmm.score) {
		    tempecmm = maxem->ecmm[k];
		    maxem->ecmm[k] = maxem->ecmm[k+1];
		    maxem->ecmm[k+1] = tempecmm;
		}
		else {
		    break;
		}
	    }

	    if (maxem->result_num < CMM_MAX-1) {
		maxem->result_num++;
	    }
	}

	/* �ʥե졼����ɲå���ȥ�κ�� */
	if (OptDemo == FALSE) {
	    DeleteFromCF(&workem, cpm_ptr, &cmm);
	}
	ClearAnaphoraList(banlist);
    }
    free(cf_array);
}

/*==================================================================*/
	   void AssignFeaturesByProgram(SENTENCE_DATA *sp)
/*==================================================================*/
{
    /* ����ͽ�� */

    int i;

    for (i = 0; i < sp->Bnst_num; i++) {
	/* �ٰʳ��� A��B �ϥ롼���Ϳ�����Ƥ��ʤ� */
	if (!check_feature((sp->bnst_data+i)->f, "�����ɽ��") && 
	    check_feature((sp->bnst_data+i)->f, "��:�γ�") && 
	    (sp->bnst_data+i)->parent && 
	    check_feature((sp->bnst_data+i)->parent->f, "����ɽ��")) {
	    assign_cfeature(&((sp->bnst_data+i)->f), "�����ɽ��");
	}
    }
}

/*==================================================================*/
	      void DiscourseAnalysis(SENTENCE_DATA *sp)
/*==================================================================*/
{
    int i, j, k, lastflag = 1, mainflag, anum;
    float score;
    ELLIPSIS_MGR workem, maxem;
    SENTENCE_DATA *sp_new;
    CF_PRED_MGR *cpm_ptr;
    CF_MATCH_MGR *cmm_ptr;
    CASE_FRAME *cf_ptr, *origcf;

    InitEllipsisMGR(&workem);
    InitEllipsisMGR(&maxem);

    AssignFeaturesByProgram(sp);

    sp_new = PreserveSentence(sp);

    /* ���Ѹ�������å� (ʸ������) */
    for (j = 0; j < sp->Best_mgr->pred_num; j++) {
	cpm_ptr = &(sp->Best_mgr->cpm[j]);

	/* �ʥե졼�ब�ʤ���� (���ʤ��餤õ���Ƥ⤤�����⤷��ʤ�) */
	if (cpm_ptr->result_num == 0 || 
	    cpm_ptr->cmm[0].cf_ptr->cf_address == -1 || 
	    cpm_ptr->cmm[0].score == -2) {
	    continue;
	}

	/* ��ά���Ϥ��ʤ��Ѹ�
	   1. �롼��ǡ־�ά���Ϥʤ���feature ���Ĥ��Ƥ����� (�֡��ߤ���� �ʤ�)
	   2. ���Ѹ� (�����Ѹ��ʲ��ϡפϽ���)
	   3. �������ʤΥ���̾��
	   4. ��٥�:A- (���ơ��Ѹ��� (A), �� �������Ѹ��� �� A-)
	   5. �ʡ���ˡ��� */
	if (check_feature(cpm_ptr->pred_b_ptr->f, "��ά���Ϥʤ�")) {
	    continue;
	}
	else if((!check_feature(cpm_ptr->pred_b_ptr->f, "���Ѹ��ʲ���") && 
		 check_feature(cpm_ptr->pred_b_ptr->f, "���Ѹ�")) || 
		check_feature(cpm_ptr->pred_b_ptr->f, "��٥�:A-") || 
		check_feature(cpm_ptr->pred_b_ptr->f, "ID:�ʡ���ˡ���")) {
	    assign_cfeature(&(cpm_ptr->pred_b_ptr->f), "��ά���Ϥʤ�");
	    continue;
	}
	    

	cmm_ptr = &(cpm_ptr->cmm[0]);
	cf_ptr = cmm_ptr->cf_ptr;

	/* ����ʸ�μ��� */
	if (lastflag == 1 && 
	    !check_feature(cpm_ptr->pred_b_ptr->f, "�����") && 
	    !check_feature(cpm_ptr->pred_b_ptr->f, "��ά���Ϥʤ�")) {
	    mainflag = 1;
	    lastflag = 0;
	    assign_cfeature(&(cpm_ptr->pred_b_ptr->f), "����");
	}
	else {
	    mainflag = 0;
	}

	/* �����ǤθĿ� */
	anum = 0;
	for (i = 0; i < cf_ptr->element_num; i++) {
	    if (cmm_ptr->result_lists_p[0].flag[i] != UNASSIGNED) {
		anum++;
	    }
	}

	/* ��äȤ⥹�������褯�ʤ���֤Ǿ�ά�λؼ��оݤ���ꤹ�� */

	maxem.score = -2;
	origcf = cpm_ptr->cmm[0].cf_ptr;
	/* ���� cmm ����¸���Ƥ��ʤ����� => ɬ�פʤ� */

	for (i = 0; i < CASE_ORDER_MAX; i++) {
	    if (cpm_ptr->decided == CF_DECIDED) {

		/* ����¦�����Ǥ�����
		   �ȱ����ϻ��Ϥ��Ǥˤ�������Ǥ��񤭤��Ƥ��ޤ��ΤǤ����Ǻ�����
		   ����ʳ��ΤȤ��ϲ��� DeleteFromCF() �Ǿ�ά���Ǥ򥯥ꥢ */
		if (OptDemo == TRUE) {
		    make_data_cframe(sp, cpm_ptr);
		}

		ClearEllipsisMGR(&workem);
		score = EllipsisDetectForVerbMain(sp, &workem, cpm_ptr, &(cpm_ptr->cmm[0]), 
						  cpm_ptr->cmm[0].cf_ptr, 
						  CaseOrder[i], mainflag, 0);
		/* ľ�ܤγ����Ǥ����������Ƥ��ʤ���������­�� */
		workem.score += cpm_ptr->cmm[0].pure_score[0];
		workem.score /= sqrt((double)(count_pat_element(cpm_ptr->cmm[0].cf_ptr, 
								&(cpm_ptr->cmm[0].result_lists_p[0]))));
		if (workem.score > maxem.score) {
		    maxem = workem;
		    /* ��find_cf ���ʤ��ä��餤��ʤ�?��
		       ���δؿ��ǳʥե졼����ꤷ������
		       ���η�̤򥹥������ɽ������ */
		    maxem.result_num = cpm_ptr->result_num;
		    for (k = 0; k < maxem.result_num; k++) {
			maxem.ecmm[k].cmm = cpm_ptr->cmm[k];
			maxem.ecmm[k].cpm = *cpm_ptr;
			maxem.ecmm[k].element_num = cpm_ptr->cf.element_num;
		    }
		    workem.f = NULL;
		}

		/* �ʥե졼����ɲå���ȥ�κ�� */
		if (OptDemo == FALSE) {
		    DeleteFromCF(&workem, cpm_ptr, &(cpm_ptr->cmm[0]));
		}
		ClearAnaphoraList(banlist);
	    }
	    else {
		FindBestCFforContext(sp, &maxem, cpm_ptr, CaseOrder[i], mainflag);
	    }

	    /*
	    if (VerboseLevel >= VERBOSE2) {
		fprintf(stdout, "CASE_ORDER %2d ==> %.3f\n", i, workem.score);
		print_data_cframe(cpm_ptr, &(cpm_ptr->cmm[0]));
		for (k = 0; k < cpm_ptr->result_num; k++) {
		    print_crrspnd(cpm_ptr, &(cpm_ptr->cmm[k]));
		}
		fputc('\n', Outfp);
	    }
	    */
	}

	/* ��äȤ� score �Τ褫�ä��Ȥ߹�碌����Ͽ */
	if (maxem.score > -2) {
	    cpm_ptr->score = maxem.score;
	    maxem.ecmm[0].cmm.score = maxem.score;
	    /* cmm ������ */
	    cpm_ptr->result_num = maxem.result_num;
	    for (k = 0; k < cpm_ptr->result_num; k++) {
		cpm_ptr->cmm[k] = maxem.ecmm[k].cmm;
		cpm_ptr->cmm[k].cpm = (CF_PRED_MGR *)malloc_data(sizeof(CF_PRED_MGR), 
								 "DiscourseAnalysis");
		*cpm_ptr->cmm[k].cpm = maxem.ecmm[k].cpm;
	    }
	    cpm_ptr->cf.element_num = maxem.ecmm[0].element_num;
	    for (k = 0; k < maxem.ecmm[0].element_num; k++) {
		cpm_ptr->elem_b_ptr[k] = maxem.ecmm[0].cpm.elem_b_ptr[k];
		cpm_ptr->elem_b_num[k] = maxem.ecmm[0].cpm.elem_b_num[k];
	    }
	    /* feature ������ */
	    append_feature(&(cpm_ptr->pred_b_ptr->f), maxem.f);
	    maxem.f = NULL;

	    /* ʸ̮���Ϥˤ����Ƴʥե졼�����ꤷ����� */
	    if (cpm_ptr->decided != CF_DECIDED) {
		after_case_analysis(sp, cpm_ptr);
		assign_ga_subject(sp, cpm_ptr); /* CF_CAND_DECIDED �ξ��ϹԤäƤ��뤬 */
		/* fix_sm_place(sp, cpm_ptr); */
		/* �ʲ��Ϥη�̤� feature �� */
		record_match_ex(sp, cpm_ptr);
		record_case_analysis(sp, cpm_ptr, &maxem, mainflag);
	    }
	    else {
		/* �ʲ��Ϥη�̤� feature �� */
		record_case_analysis(sp, cpm_ptr, &maxem, mainflag);
	    }

	    /* ����̾��Υ��ʤϤޤ��������Τǵ�Ͽ���ʤ� -> ����̾�줹�٤Ƶ�Ͽ���ʤ� */
	    if (!check_feature(cpm_ptr->pred_b_ptr->f, "���Ѹ��ʲ���")) {
		for (i = 0; i < CASE_MAX_NUM; i++) {
		    if (maxem.cc[i].s) {
			/* ��󥯤��줿���Ȥ�Ͽ���� */
			RegisterAnaphor(alist, (maxem.cc[i].s->bnst_data+maxem.cc[i].bnst)->Jiritu_Go);
			/* �Ѹ��ȳ����ǤΥ��åȤ�Ͽ (�ʴط��Ȥ϶��̤�����) */
			RegisterPredicate(L_Jiritu_M(cpm_ptr->pred_b_ptr)->Goi, 
					  cpm_ptr->pred_b_ptr->voice, 
					  cpm_ptr->cmm[0].cf_ptr->cf_address, 
					  i, 
					  (maxem.cc[i].s->bnst_data+maxem.cc[i].bnst)->Jiritu_Go, 
					  EREL);
		    }
		}
	    }

	    /* ����ξ�硢��Ͽ */
	    if (mainflag) {
		for (i = 0; i < CASE_MAX_NUM; i++) {
		    if (maxem.cc[i].s) {
			RegisterLastClause(sp->Sen_num, 
					   L_Jiritu_M(cpm_ptr->pred_b_ptr)->Goi, i, 
					   (maxem.cc[i].s->bnst_data+maxem.cc[i].bnst)->Jiritu_Go, EREL);
		    }
		}
	    }
	}
	ClearEllipsisMGR(&maxem);
    }

    PreserveCPM(sp_new, sp);
    clear_cf(0);
}

/*====================================================================
                               END
====================================================================*/
