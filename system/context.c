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

char *ExtraTags[] = {"�оݳ�", "��;�", "���ΰ���", "������ʪ", ""};

char *ETAG_name[] = {
    "", "", "������:��", "��;�", "������:����", 
    "��ʸ", "��ʸ", "�оݳ�"};

float	AssignReferentThreshold = 0.67;
float	AssignReferentThresholdDecided = 0.50;
float	AssignGaCaseThreshold = 0.67;	/* ���ʤ�ڼ��ΰ��̡ۤˤ������� */
float	AssignReferentThresholdHigh = 0.80;
float	AssignReferentThresholdAnonymousThing = 0.90;

float	AssignReferentThresholdForSVM = -0.95;

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
	num = sp->Best_mgr->cpm[i].pred_b_ptr->num;	/* �����Ѹ���ʸ���ֹ� */
	*(sp_new->cpm+i) = sp->Best_mgr->cpm[i];
	sp_new->bnst_data[num].cpm_ptr = sp_new->cpm+i;
	(sp_new->cpm+i)->pred_b_ptr = sp_new->bnst_data+num;
	for (j = 0; j < (sp_new->cpm+i)->cf.element_num; j++) {
	    /* ��ά����ʤ������� */
	    if ((sp_new->cpm+i)->elem_b_num[j] != -2) {
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

    if (Thesaurus == USE_BGH) {
	exd = cand->BGH_code;
	exp = cf_ptr->ex[n];
	step = BGH_CODE_SIZE;
    }
    else if (Thesaurus == USE_NTT) {
	exd = cand->SM_code;
	exp = cf_ptr->ex2[n];
	step = SM_CODE_SIZE;
    }

    if (check_feature(cand->f, "��ͭ����Ÿ���ػ�")) {
	expand = SM_NO_EXPAND_NE;
    }
    else {
	expand = SM_EXPAND_NE;
    }

    /* ��̣�Ǥʤ� (�ʥե졼�������Ϥ���) 
       ����ˤ��뤿��� -1 ���֤� */
    if (!exd[0] && cf_ptr->ex_list[n]) {
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
	/* ex_score = CalcWordsSimilarity(cand->Jiritu_Go, cf_ptr->ex_list[n], cf_ptr->ex_num[n], pos); */
	/* ex_score = CalcSimilarity(exd, exp); */
	if (Thesaurus == USE_BGH) {
	    ex_score /= 7;
	}
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
    ex_score = (float)CalcSimilarity(exd, exp, 0);
    if (Thesaurus == USE_BGH) {
	ex_score /= 7;
    }

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
	!check_feature(bp->f, "����") && 
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
	if (cpm_ptr->elem_b_num[i] != -2 && 
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
       cpm_ptr->elem_b_num[num] == -2 
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
		cpm_ptr->elem_b_num[num] != -2 && /* ��ά�γ����Ǥ���ʤ� */
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
		cpm_ptr->elem_b_num[num] != -2 && /* ��ά�γ����Ǥ���ʤ� */
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

#ifdef USE_SVM
/*==================================================================*/
void EllipsisDetectForVerbSubcontractExtraTagsWithSVM(SENTENCE_DATA *cs, ELLIPSIS_MGR *em_ptr, 
						      CF_PRED_MGR *cpm_ptr, CF_MATCH_MGR *cmm_ptr, 
						      char *tag, CASE_FRAME *cf_ptr, int n)
/*==================================================================*/
{
    float score;
    int predabstract, agentflag, hobunflag, passive, sahen1, sahen2, tame, renkaku;
    int soto1 = 0, soto2 = 0, soto3 = 0;
    int headjikan = 0, headsoutai = 0, headkeifuku = 0, headpred = 0;
    int anonymousp, firstp, untarget;
    int case_ga = 0, case_wo = 0, case_ni = 0;
    char feature_buffer[50000], *cp;
    BNST_DATA *b_ptr;

    if (sm_match_check(sm2code("���ʪ"), cpm_ptr->pred_b_ptr->SM_code)) {
	predabstract = 1;
    }
    else {
	predabstract = 0;
    }

    /* �ʥե졼�ब <����> ���Ĥ��ɤ��� */
    if (cf_ptr->etcflag & CF_GA_SEMI_SUBJECT) {
	agentflag = 2;	/* ����� */
    }
    else if (cf_match_element(cf_ptr->sm[n], "����", FALSE)) {
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

    /* ���� */
    if (check_feature(cpm_ptr->pred_b_ptr->f, "�����") || 
	check_feature(cpm_ptr->pred_b_ptr->f, "������") || 
	check_feature(cpm_ptr->pred_b_ptr->f, "�ɲü���")) {
	passive = 1;
    }
    else {
	passive = 0;
    }

    if (check_feature(cpm_ptr->pred_b_ptr->f, "����̾��ʲ���")) {
	sahen1 = 1;
    }
    else {
	sahen1 = 0;
    }
    if (check_feature(cpm_ptr->pred_b_ptr->f, "����")) {
	sahen2 = 1;
    }
    else {
	sahen2 = 0;
    }

    if (check_feature(cpm_ptr->pred_b_ptr->f, "ID:���ʤ����")) {
	tame = 1;
    }
    else {
	tame = 0;
    }

    if (check_feature(cpm_ptr->pred_b_ptr->f, "��:Ϣ��")) {
	renkaku = 1;
    }
    else {
	renkaku = 0;
    }

    if ((b_ptr = GetRealParent(cs, cpm_ptr->pred_b_ptr))) {
	if (check_feature(b_ptr->f, "���δط�")) {
	    soto1 = 1;
	}
	else if (check_feature(b_ptr->f, "���δط���ǽ��")) {
	    soto2 = 1;
	}
	else if (check_feature(b_ptr->f, "���δط�Ƚ��")) {
	    soto3 = 1;
	}

	if (check_feature(b_ptr->f, "����")) {
	    headjikan = 1;
	}
	if (check_feature(b_ptr->f, "����̾��")) {
	    headsoutai = 1;
	}
	if (check_feature(b_ptr->f, "����̾��")) {
	    headkeifuku = 1;
	}
	if (check_feature(b_ptr->f, "�Ѹ�")) {
	    headpred = 1;
	}
    }

    if ((cp = check_feature(cpm_ptr->pred_b_ptr->f, "�������")) && 
	MatchPP(cf_ptr->pp[n][0], cp+9)) {
	anonymousp = 1;
    }
    else {
	anonymousp = 0;
    }

    if ((cp = check_feature(cpm_ptr->pred_b_ptr->f, "��;�")) && 
	MatchPP(cf_ptr->pp[n][0], cp+7)) {
	firstp = 1;
    }
    else {
	firstp = 0;
    }

    if ((cp = check_feature(cpm_ptr->pred_b_ptr->f, "�оݳ�")) && 
	MatchPP(cf_ptr->pp[n][0], cp+7)) {
	untarget = 1;
    }
    else {
	untarget = 0;
    }

    if (MatchPP(cf_ptr->pp[n][0], "��")) {
	case_ga = 1;
    }
    else if (MatchPP(cf_ptr->pp[n][0], "��")) {
	case_wo = 1;
    }
    else if (MatchPP(cf_ptr->pp[n][0], "��")) {
	case_ni = 1;
    }

    sprintf(feature_buffer, "1:%.3f 2:-1 3:-1 4:-1 5:-1 6:%d 7:%d 8:-1 9:-1 10:-1 11:-1 12:-1 13:-1 14:-1 15:-1 16:-1 17:-1 18:%d 19:%d 20:%d 21:%d 22:%d 23:%d 24:%d 25:%d 26:%d 27:%d 28:%d 29:%d 30:%d 31:%d 32:%d 33:%d 34:%d 35:%d 36:%d 37:%d 38:0 39:0 40:0 41:0 42:0 43:0 44:%d 45:%d 46:%d 47:%d 48:-1", 
	    (float)-1, /* 1 */
	    agentflag == 1 ? 1 : 0, agentflag == 2 ? 1 : 0, /* 6-7 */
	    passive, sahen1, sahen2, tame, renkaku, soto1, soto2, soto3, /* 18-25 */
	    headjikan, headsoutai, headkeifuku, headpred, /* 26-29 */
	    anonymousp, firstp, untarget, /* 30-32 */
	    case_ga, case_wo, case_ni, /* 33-35 */
	    hobunflag, predabstract, /* 36-37 */
	    strcmp(tag, ExtraTags[0]) == 0 ? 1 : 0, /* 44 */
	    strcmp(tag, ExtraTags[1]) == 0 ? 1 : 0, /* 45 */
	    strcmp(tag, ExtraTags[2]) == 0 ? 1 : 0, /* 46 */
	    strcmp(tag, ExtraTags[3]) == 0 ? 1 : 0); /* 47 */
    score = svm_classify(feature_buffer);

    /* fprintf(stderr, "DEBUG %s %f %s\n", tag, (float)score, feature_buffer); */

    if (score > maxscore) {
	char feature_buffer2[50000];
	maxscore = score;
	maxrawscore = 1.0;
	maxtag = tag;
	sprintf(feature_buffer2, "SVM-%s:%s", pp_code_to_kstr(cf_ptr->pp[n][0]), feature_buffer);
	assign_cfeature(&(em_ptr->f), feature_buffer2);
    }

    sprintf(feature_buffer, "C��;%s;%s;-1;-1;%.3f|-1", tag, 
	    pp_code_to_kstr(cf_ptr->pp[n][0]), 
	    score);
    assign_cfeature(&(em_ptr->f), feature_buffer);

}

/*==================================================================*/
void _EllipsisDetectForVerbSubcontractWithSVM(SENTENCE_DATA *s, SENTENCE_DATA *cs, ELLIPSIS_MGR *em_ptr, 
					      CF_PRED_MGR *cpm_ptr, CF_MATCH_MGR *cmm_ptr, 
					      BNST_DATA *bp, CASE_FRAME *cf_ptr, int n, int type)
/*==================================================================*/
{
    float score, weight, pascore, pcscore, mcscore, rawscore, topicscore, distscore;
    char feature_buffer[50000], *cp;
    int ac, pac, pcc, pcc2, mcc, topicflag, distance, agentflag, firstsc, subtopicflag, sameflag;
    int exception = 0, pos = MATCH_NONE, casematch, candagent, scopeflag, passive, headscope;
    int hobunflag, predabstract, tame, renkaku, soto1 = 0, soto2 = 0, soto3 = 0;
    int sahen1, sahen2, headjikan = 0, headsoutai = 0, headkeifuku = 0, headpred = 0;
    int anonymousp, firstp, untarget, case_ga = 0, case_wo = 0, case_ni = 0, smnone;
    int important = 0, gmcc;
    BNST_DATA *b_ptr;

    /* cs �ΤȤ�������̣������ */
    Bcheck[bp->num] = 1;

    /* �о��Ѹ��ȸ��䤬Ʊ����Ω��ΤȤ�
       Ƚ���ξ��������� */
    if (str_eq(cpm_ptr->pred_b_ptr->Jiritu_Go, bp->Jiritu_Go)) {
	if (!check_feature(cpm_ptr->pred_b_ptr->f, "�Ѹ�:Ƚ")) {
	    return;
	}
	sameflag = 1;
    }
    else {
	sameflag = 0;
    }

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

    /* ��󥯤��줿����ˤ�륹���� */
    ac = CheckAnaphor(alist, bp->Jiritu_Go);

    /* ���Ǥ˽и������Ѹ��Ȥ��γ����ǤΥ��å� */
    pac = CheckPredicate(L_Jiritu_M(cpm_ptr->pred_b_ptr)->Goi, cpm_ptr->pred_b_ptr->voice, 
			 cf_ptr->ipal_address, 
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

    /* ���䤬����˷��뤫�ɤ��� */
    gmcc = CheckLastClause(cs->Sen_num-distance, cf_ptr->pp[n][0], bp->Jiritu_Go);

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

    /* �� �ٰʳ��� A��B �ϥ롼���Ϳ�����Ƥ��ʤ� */
    if (!check_feature(bp->f, "�����ɽ��") && 
	check_feature(bp->f, "��:�γ�") && 
	bp->parent && 
	check_feature(bp->parent->f, "����ɽ��")) {
	assign_cfeature(&(bp->f), "�����ɽ��");
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
    if (cf_ptr->etcflag & CF_GA_SEMI_SUBJECT) {
	agentflag = 2;	/* ����� */
    }
    else if (cf_match_element(cf_ptr->sm[n], "����", FALSE)) {
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

    if (distance > 0 || bp->num < cpm_ptr->pred_b_ptr->num) {
	scopeflag = 1;
    }
    else {
	scopeflag = 0;
    }

    if (distance == 0 && bp->parent && bp->parent->num > cpm_ptr->pred_b_ptr->num) {
	headscope = 1;
    }
    else {
	headscope = 0;
    }

    /* ���� */
    if (check_feature(cpm_ptr->pred_b_ptr->f, "�����") || 
	check_feature(cpm_ptr->pred_b_ptr->f, "������") || 
	check_feature(cpm_ptr->pred_b_ptr->f, "�ɲü���")) {
	passive = 1;
    }
    else {
	passive = 0;
    }

    if (check_feature(cpm_ptr->pred_b_ptr->f, "����̾��ʲ���")) {
	sahen1 = 1;
    }
    else {
	sahen1 = 0;
    }
    if (check_feature(cpm_ptr->pred_b_ptr->f, "����")) {
	sahen2 = 1;
    }
    else {
	sahen2 = 0;
    }

    if (check_feature(cpm_ptr->pred_b_ptr->f, "ID:���ʤ����")) {
	tame = 1;
    }
    else {
	tame = 0;
    }

    if (check_feature(cpm_ptr->pred_b_ptr->f, "��:Ϣ��")) {
	renkaku = 1;
    }
    else {
	renkaku = 0;
    }

    /* critical!! (�������οƤ����ɬ�פ�����) */
    if ((b_ptr = GetRealParent(cs, cpm_ptr->pred_b_ptr))) {
	if (check_feature(b_ptr->f, "���δط�")) {
	    soto1 = 1;
	}
	else if (check_feature(b_ptr->f, "���δط���ǽ��")) {
	    soto2 = 1;
	}
	else if (check_feature(b_ptr->f, "���δط�Ƚ��")) {
	    soto3 = 1;
	}

	if (check_feature(b_ptr->f, "����")) {
	    headjikan = 1;
	}
	if (check_feature(b_ptr->f, "����̾��")) {
	    headsoutai = 1;
	}
	if (check_feature(b_ptr->f, "����̾��")) {
	    headkeifuku = 1;
	}
	if (check_feature(b_ptr->f, "�Ѹ�")) {
	    headpred = 1;
	}
    }

    if ((cp = check_feature(cpm_ptr->pred_b_ptr->f, "�������")) && 
	MatchPP(cf_ptr->pp[n][0], cp+9)) {
	anonymousp = 1;
    }
    else {
	anonymousp = 0;
    }

    if ((cp = check_feature(cpm_ptr->pred_b_ptr->f, "��;�")) && 
	MatchPP(cf_ptr->pp[n][0], cp+7)) {
	firstp = 1;
    }
    else {
	firstp = 0;
    }

    if ((cp = check_feature(cpm_ptr->pred_b_ptr->f, "�оݳ�")) && 
	MatchPP(cf_ptr->pp[n][0], cp+7)) {
	untarget = 1;
    }
    else {
	untarget = 0;
    }

    if (MatchPP(cf_ptr->pp[n][0], "��")) {
	case_ga = 1;
    }
    else if (MatchPP(cf_ptr->pp[n][0], "��")) {
	case_wo = 1;
    }
    else if (MatchPP(cf_ptr->pp[n][0], "��")) {
	case_ni = 1;
    }

    /* N �� �� N ���� */
    if (sameflag) {
	rawscore = 1;
    }
    else if (casematch != -1 && type >= RANK3 && check_feature(cpm_ptr->pred_b_ptr->f, "�Ѹ�:Ƚ")) {
	rawscore = 1.0;
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

    if (distance == 0 || /* �о�ʸ */
	(distance == 1 && (topicflag || subtopicflag || pcc)) || /* ��ʸ */
	/* (distance == 2 && pcc2) || * 2ʸ�� */
	(s->Sen_num == 1 && (topicflag || subtopicflag || firstsc)) || /* ��Ƭʸ */
	(pcc || mcc || firstsc)) { /* ����ʳ��Ǽ���ξ�ά�λؼ��о� */
	important = 1;

	sprintf(feature_buffer, "1:%.3f 2:%d 3:%d 4:%d 5:%d 6:%d 7:%d 8:%d 9:%d 10:%d 11:%d 12:%d 13:%d 14:%d 15:%d 16:%d 17:%d 18:%d 19:%d 20:%d 21:%d 22:%d 23:%d 24:%d 25:%d 26:%d 27:%d 28:%d 29:%d 30:%d 31:%d 32:%d 33:%d 34:%d 35:%d 36:%d 37:%d 38:%d 39:%d 40:%d 41:%d 42:%d 43:%d 44:%d 45:%d 46:%d 47:%d 48:%d", 
		rawscore, distance, s->Sen_num == 1 ? 1: 0, /* 1-3 */
		topicflag, subtopicflag, /* 4-5 */
		agentflag == 1 ? 1 : 0, agentflag == 2 ? 1 : 0, candagent, /* 6-8 */
		scopeflag, headscope, pac, ac, casematch, /* 9-13 */
		sameflag, exception, smnone, important, /* 14-17 */
		passive, sahen1, sahen2, tame, renkaku, /* 18-22 */
		soto1, soto2, soto3, /* 23-25 */
		headjikan, headsoutai, headkeifuku, headpred, /* 26-29 */
		anonymousp, firstp, untarget, /* 30-32 */
		case_ga, case_wo, case_ni, /* 33-35 */
		hobunflag, predabstract, /* 36-37 */
		type == 1 ? 1 : 0, type == 2 ? 1 : 0, 
		type == 3 ? 1 : 0, type == 4 ? 1 : 0, type == 5 ? 1 : 0, 
		type > 5 ? 1 : 0, 
		0, 0, 0, 0, 
		gmcc);
	score = svm_classify(feature_buffer);

	if (VerboseLevel >= VERBOSE2) {
	    fprintf(stderr, "DEBUG %s %s %f %s\n", cpm_ptr->pred_b_ptr->Jiritu_Go, bp->Jiritu_Go, (float)score, feature_buffer);
	}

	if (rawscore > 0 || smnone == 1) {
	    if (score > maxscore) {
		char feature_buffer2[50000];
		maxscore = score;
		maxrawscore = rawscore;
		maxs = s;
		maxpos = pos;
		if (bp->num < 0) {
		    maxi = bp->parent->num;
		}
		else {
		    maxi = bp->num;
		}
		sprintf(feature_buffer2, "SVM-%s:%s", pp_code_to_kstr(cf_ptr->pp[n][0]), feature_buffer);
		assign_cfeature(&(em_ptr->f), feature_buffer2);
		maxtag = NULL;
	    }

	    /* ��ά���� */
	    sprintf(feature_buffer, "C��;%s;%s;%d;%d;%.3f|%.3f", bp->Jiritu_Go, 
		    pp_code_to_kstr(cf_ptr->pp[n][0]), 
		    distance, maxi, 
		    score, rawscore);
	    assign_cfeature(&(em_ptr->f), feature_buffer);
	    sprintf(feature_buffer, "�ؽ�FEATURE;%s;%s;%.3f|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d", 
		    bp->Jiritu_Go, 
		    pp_code_to_kstr(cf_ptr->pp[n][0]), 
		    rawscore, type, ac, pac, pcc, mcc, topicflag, subtopicflag, agentflag, candagent, distance, bp->num, casematch, hobunflag, predabstract, sameflag, exception, smnone, important);
	    assign_cfeature(&(em_ptr->f), feature_buffer);
	}
    }
}
#endif

/*==================================================================*/
void _EllipsisDetectForVerbSubcontract(SENTENCE_DATA *s, SENTENCE_DATA *cs, ELLIPSIS_MGR *em_ptr, 
				       CF_PRED_MGR *cpm_ptr, CF_MATCH_MGR *cmm_ptr, 
				       BNST_DATA *bp, CASE_FRAME *cf_ptr, int n, int type)
/*==================================================================*/
{
    float score, weight, pascore, pcscore, mcscore, rawscore, topicscore, distscore;
    float addscore = 0;
    char feature_buffer[DATA_LEN];
    int ac, pac, pcc, pcc2, mcc, topicflag, distance, agentflag, firstsc, subtopicflag, sameflag;
    int exception = 0, pos = MATCH_NONE, casematch, candagent, hobunflag, predabstract, smnone;
    int important = 0;

    /* cs �ΤȤ�������̣������ */
    Bcheck[bp->num] = 1;

    /* �о��Ѹ��ȸ��䤬Ʊ����Ω��ΤȤ�
       Ƚ���ξ��������� */
    if (str_eq(cpm_ptr->pred_b_ptr->Jiritu_Go, bp->Jiritu_Go)) {
	if (!check_feature(cpm_ptr->pred_b_ptr->f, "�Ѹ�:Ƚ")) {
	    return;
	}
	sameflag = 1;
    }
    else {
	sameflag = 0;
    }

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
			 cf_ptr->ipal_address, 
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

    /* �� �ٰʳ��� A��B �ϥ롼���Ϳ�����Ƥ��ʤ� */
    if (!check_feature(bp->f, "�����ɽ��") && 
	check_feature(bp->f, "��:�γ�") && 
	bp->parent && 
	check_feature(bp->parent->f, "����ɽ��")) {
	assign_cfeature(&(bp->f), "�����ɽ��");
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

    if (casematch != -1 && type == RANKP) {
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
		weight *= 0.6;
		score = weight*pascore*rawscore;
	    }
	    else {
		weight *= 0.6;
		score = weight*pascore*rawscore;
	    }
	}
	else {
	    if (subtopicflag == 1) {
		weight *= 0.6;
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
	maxscore = score;
	maxs = s;
	maxpos = pos;
	if (bp->num < 0) {
	    maxi = bp->parent->num;
	}
	else {
	    maxi = bp->num;
	}
    }

    /* ��ά���� (rawscore == 0 �ξ������Ȥ��ƽ���) */
    sprintf(feature_buffer, "C��;%s;%s;%d;%d;%.3f|%.3f", bp->Jiritu_Go, 
	    pp_code_to_kstr(cf_ptr->pp[n][0]), 
	    distance, maxi, 
	    score, rawscore);
    assign_cfeature(&(em_ptr->f), feature_buffer);
    sprintf(feature_buffer, "�ؽ�FEATURE;%s;%s;%.3f|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d", 
	    bp->Jiritu_Go, 
	    pp_code_to_kstr(cf_ptr->pp[n][0]), 
	    rawscore, type, ac, pac, pcc, mcc, topicflag, subtopicflag, agentflag, candagent, distance, bp->num, casematch, hobunflag, predabstract, sameflag, exception, smnone, important);
    assign_cfeature(&(em_ptr->f), feature_buffer);
}

/*==================================================================*/
void EllipsisDetectForVerbSubcontract(SENTENCE_DATA *s, SENTENCE_DATA *cs, ELLIPSIS_MGR *em_ptr, 
				      CF_PRED_MGR *cpm_ptr, CF_MATCH_MGR *cmm_ptr, 
				      BNST_DATA *bp, CASE_FRAME *cf_ptr, int n, int type)
/*==================================================================*/
{
#ifdef USE_SVM
    if (OptSVM == OPT_SVM)
	_EllipsisDetectForVerbSubcontractWithSVM(s, cs, em_ptr, 
						 cpm_ptr, cmm_ptr, 
						 bp, cf_ptr, n, type);
    else
#endif
	_EllipsisDetectForVerbSubcontract(s, cs, em_ptr, 
					  cpm_ptr, cmm_ptr, 
					  bp, cf_ptr, n, type);
}

/*==================================================================*/
void SearchCaseComponent(SENTENCE_DATA *s, ELLIPSIS_MGR *em_ptr, 
			 CF_PRED_MGR *cpm_ptr, CF_MATCH_MGR *cmm_ptr, 
			 BNST_DATA *bp, CASE_FRAME *cf_ptr, int n, int rank)
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
		bp->cpm_ptr->elem_b_num[num] != -2 && /* �����Ǥ���ά����ä���ΤǤ���Ȥ��Ϥ��� */
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
		EllipsisDetectForVerbSubcontract(s, s, em_ptr, cpm_ptr, cmm_ptr, 
						 bp->cpm_ptr->elem_b_ptr[num], 
						 cf_ptr, n, rank);
	    }
	}
    }
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

    if (OptSVM == OPT_SVM) {
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
    cpm_ptr->elem_b_num[d] = -2;
    c_ptr->weight[d] = 0;
    c_ptr->adjacent[d] = FALSE;
    if (!demonstrative) {
	_make_data_cframe_sm(cpm_ptr, b_ptr);	/* ����: ��Ǽ��꤬ c_ptr->element_num ���� */
	_make_data_cframe_ex(cpm_ptr, b_ptr);
	c_ptr->element_num++;
    }
    /* �ؼ���ξ�硢��Ȥλؼ����ʬ�Υ�����������Ƥ��� */
    else {
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
	if (cpm_ptr->elem_b_num[i] == -2) {
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

    if (OptSVM == OPT_SVM) {
	maxscore = AssignReferentThresholdForSVM;
	maxtag = NULL;
    }
    else {
	maxscore = 0;
    }

    cs = sentence_data + sp->Sen_num - 1;
    memset(Bcheck, 0, sizeof(int)*BNST_MAX);

    /* �Ƥ�ߤ� (PARA �ʤ� child �Ѹ�) */
    if (cpm_ptr->pred_b_ptr->parent) {
	/* �Ƥ� PARA */
	if (cpm_ptr->pred_b_ptr->parent->para_top_p) {
	    /* ��ʬ��������Ѹ� */
	    for (i = 0; cpm_ptr->pred_b_ptr->parent->child[i]; i++) {
		/* PARA �λҶ��ǡ���ʬ��ʳ��������Ѹ� */
		if (cpm_ptr->pred_b_ptr->parent->child[i] != cpm_ptr->pred_b_ptr &&
		    cpm_ptr->pred_b_ptr->parent->child[i]->para_type == PARA_NORMAL) {
		    SearchCaseComponent(cs, em_ptr, cpm_ptr, cmm_ptr, 
					cpm_ptr->pred_b_ptr->parent->child[i], cf_ptr, n, RANKP);
		}
	    }

	    /* Ϣ�ѤǷ�����Ѹ� (����ΤȤ�) */
	    if (cpm_ptr->pred_b_ptr->parent->parent && 
		check_feature(cpm_ptr->pred_b_ptr->f, "��:Ϣ��")) {
		SearchCaseComponent(cs, em_ptr, cpm_ptr, cmm_ptr, 
				    cpm_ptr->pred_b_ptr->parent->parent, cf_ptr, n, RANK3);
	    }
	}
	/* �Ȥꤢ������Ϣ�ѤǷ���ҤȤľ�ο��Ѹ��Τ� */
	else if (check_feature(cpm_ptr->pred_b_ptr->f, "��:Ϣ��")) {
	    SearchCaseComponent(cs, em_ptr, cpm_ptr, cmm_ptr, 
				cpm_ptr->pred_b_ptr->parent, cf_ptr, n, RANK3);
	}
	/* �֡�������פʤ� */
	else if (check_feature(cpm_ptr->pred_b_ptr->f, "��°�᰷��")) {
	    SearchCaseComponent(cs, em_ptr, cpm_ptr, cmm_ptr, 
				cpm_ptr->pred_b_ptr->parent->parent, cf_ptr, n, RANK3);
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
		    SearchCaseComponent(cs, em_ptr, cpm_ptr, cmm_ptr, 
					cpm_ptr->pred_b_ptr->child[i]->child[j], cf_ptr, n, RANK3);
		}
	    }
	}
	else {
	    if (check_feature(cpm_ptr->pred_b_ptr->child[i]->f, "��:Ϣ��")) {
		SearchCaseComponent(cs, em_ptr, cpm_ptr, cmm_ptr, 
				    cpm_ptr->pred_b_ptr->child[i], cf_ptr, n, RANK3);
	    }
	}
    }

    if ((cp = check_feature(cpm_ptr->pred_b_ptr->f, "�ȱ��ҥ��"))) {
	if (str_eq(cp, "�ȱ��ҥ��:��")) {
	    /* ��������Ѹ��˷�������Ǥ�ߤ� (����̾��ΤȤ�) */
	    SearchCaseComponent(cs, em_ptr, cpm_ptr, cmm_ptr, 
				cs->bnst_data+cpm_ptr->pred_b_ptr->dpnd_head, cf_ptr, n, RANK3);

	    /* ��������Ѹ��˷��뽾°��˷�������Ǥ�ߤ� */
	    for (i = 0; (cs->bnst_data+cpm_ptr->pred_b_ptr->dpnd_head)->child[i]; i++) {
		if (check_feature((cs->bnst_data+cpm_ptr->pred_b_ptr->dpnd_head)->child[i]->f, "�Ѹ�")) {
		    SearchCaseComponent(cs, em_ptr, cpm_ptr, cmm_ptr, 
					(cs->bnst_data+cpm_ptr->pred_b_ptr->dpnd_head)->child[i], 
					cf_ptr, n, RANK3);
		}
	    }
	}
	else {
	    i = cpm_ptr->pred_b_ptr->num+atoi(cp+11);
	    if (i >= 0 && i < cs->Bnst_num) {
		SearchCaseComponent(cs, em_ptr, cpm_ptr, cmm_ptr, cs->bnst_data+i, cf_ptr, n, RANK3);
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
		if (i < cpm_ptr->pred_b_ptr->num && 
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
			EllipsisDetectForVerbSubcontract(s, cs, em_ptr, cpm_ptr, cmm_ptr, 
							 s->bnst_data+i, cf_ptr, n, RANK2);
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

	    EllipsisDetectForVerbSubcontract(s, cs, em_ptr, cpm_ptr, cmm_ptr, 
					     s->bnst_data+i, cf_ptr, n, RANK1);
	}
	if (current)
	    current = 0;
    }

#ifdef USE_SVM
    if (OptSVM == OPT_SVM) {
	if (MatchPP(cf_ptr->pp[n][0],"��") || 
	    MatchPP(cf_ptr->pp[n][0], "��") || 
	    MatchPP(cf_ptr->pp[n][0], "��") || 
	    MatchPP(cf_ptr->pp[n][0], "���") || 
	    MatchPP(cf_ptr->pp[n][0], "����") || 
	    MatchPP(cf_ptr->pp[n][0], "�ޥ�")) {
	    sprintf(feature_buffer, "��ά�����ʤ�-%s", 
		    pp_code_to_kstr(cf_ptr->pp[n][0]));
	    assign_cfeature(&(em_ptr->f), feature_buffer);
	    return 0;
	}

	for (i = 0; ExtraTags[i][0]; i++) {
	    EllipsisDetectForVerbSubcontractExtraTagsWithSVM(cs, em_ptr, cpm_ptr, cmm_ptr, 
							     ExtraTags[i], cf_ptr, n);
	}

	if (maxscore > AssignReferentThresholdForSVM) {
	    if (maxtag) {
		if (str_eq(maxtag, "���ΰ���")) {
		    sprintf(feature_buffer, "C��;��������:�͡�;%s;-1;-1;1", 
			    pp_code_to_kstr(cf_ptr->pp[n][0]));
		    assign_cfeature(&(em_ptr->f), feature_buffer);
		    return 0;
		}
		else if (str_eq(maxtag, "�оݳ�")) {
		    sprintf(feature_buffer, "C��;���оݳ���;%s;-1;-1;1", 
			    pp_code_to_kstr(cf_ptr->pp[n][0]));
		    assign_cfeature(&(em_ptr->f), feature_buffer);
		    AppendToCF(cpm_ptr, cmm_ptr, cpm_ptr->pred_b_ptr, cf_ptr, n, maxscore, -1);
		    return 1;
		}
		else if (str_eq(maxtag, "������ʪ")) {
		    sprintf(feature_buffer, "C��;���㳰:�ʤ���;%s;-1;-1;1", 
			    pp_code_to_kstr(cf_ptr->pp[n][0]));
		    assign_cfeature(&(em_ptr->f), feature_buffer);
		    /* �����祹�����λؼ��оݤ� dummy �ǳʥե졼�����¸ 
		       ���줬���ۤ��γʤθ���ˤʤ�ʤ��ʤ�Τ������ */
		    AppendToCF(cpm_ptr, cmm_ptr, cpm_ptr->pred_b_ptr, cf_ptr, n, maxscore, -1);
		    return 1;
		}
		else if (str_eq(maxtag, "��;�")) {
		    sprintf(feature_buffer, "C��;�ڰ�;Ρ�;%s;-1;-1;1", 
			    pp_code_to_kstr(cf_ptr->pp[n][0]));
		    assign_cfeature(&(em_ptr->f), feature_buffer);
		    AppendToCF(cpm_ptr, cmm_ptr, cpm_ptr->pred_b_ptr, cf_ptr, n, maxscore, -1);
		    return 1;
		}
	    }

	    if (check_feature(cpm_ptr->pred_b_ptr->f, "��ά�ʻ���")) {
		;
	    }
	    else {
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
#endif

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
       1. �Ѹ������Ȥǥ˳� (��Ȥϥ���) �� <����> ��Ȥ�Ȥ�
       2. �֡�����(��)�פǥ��ʤ� <����> ��Ȥ�Ȥ� 
       3. ���� V ���� N (���δط�, !Ƚ���), ����̾��, ����̾��Ͻ���
       4. �����������ͤ�겼�ǥ��� <����> ��Ȥ�Ȥ� */
    else if (((cp = check_feature(cpm_ptr->pred_b_ptr->f, "�������")) && 
	 MatchPP(cf_ptr->pp[n][0], cp+9)) || 
	(MatchPP(cf_ptr->pp[n][0], "��") && 
	 cf_match_element(cf_ptr->sm[n], "����", FALSE) && 
	 (maxscore <= AssignGaCaseThreshold || 
	  check_feature(cpm_ptr->pred_b_ptr->f, "ID:���ʤ����"))) || 
	(MatchPP(cf_ptr->pp[n][0], "��") && 
	 cf_match_element(cf_ptr->sm[n], "����", FALSE) && 
	 !cf_match_element(cf_ptr->sm[n], "���", FALSE) && 
	 maxscore <= AssignGaCaseThreshold && 
	 (check_feature(cpm_ptr->pred_b_ptr->f, "�����") || 
	  check_feature(cpm_ptr->pred_b_ptr->f, "������") || 
	  check_feature(cpm_ptr->pred_b_ptr->f, "�ɲü���") || 
	  check_feature(cpm_ptr->pred_b_ptr->f, "����̾��ʲ���"))) || 
	(cpm_ptr->pred_b_ptr->parent && 
	 (check_feature(cpm_ptr->pred_b_ptr->parent->f, "���δط�") || 
	  check_feature(cpm_ptr->pred_b_ptr->parent->f, "���δط���ǽ��") || 
	  check_feature(cpm_ptr->pred_b_ptr->parent->f, "���δط�Ƚ��")) && 
	 !check_feature(cpm_ptr->pred_b_ptr->parent->f, "����") && 
	 !check_feature(cpm_ptr->pred_b_ptr->parent->f, "����̾��") && 
	 !check_feature(cpm_ptr->pred_b_ptr->parent->f, "����̾��") && 
	 !check_feature(cpm_ptr->pred_b_ptr->parent->f, "�Ѹ�") && 
	 check_feature(cpm_ptr->pred_b_ptr->f, "��:Ϣ��") && 
	 cf_ptr->pp[n][0] == pp_kstr_to_code("��"))) {
	/* �ʥե졼������롼�פΤȤ��� feature ��Ϳ����Τ����� */
	sprintf(feature_buffer, "C��;��������:�͡�;%s;-1;-1;1", 
		pp_code_to_kstr(cf_ptr->pp[n][0]));
	assign_cfeature(&(em_ptr->f), feature_buffer);
	em_ptr->cc[cf_ptr->pp[n][0]].bnst = ELLIPSIS_TAG_UNSPECIFIED_PEOPLE;
    }
    /* ���ξ��Ͼ�ά���Ǥ�õ������Ͽ���ʤ� 
       (�������Ǥϥǡ����򸫤뤿�ᡢ�������ά���Ϥ�ԤäƤ���) 
       �ǳ�, �ȳ�, ����, �����, �ޥǳ� */
    else if (MatchPP(cf_ptr->pp[n][0],"��") || 
	     MatchPP(cf_ptr->pp[n][0], "��") || 
	     MatchPP(cf_ptr->pp[n][0], "��") || 
	     MatchPP(cf_ptr->pp[n][0], "���") || 
	     MatchPP(cf_ptr->pp[n][0], "����") || 
	     MatchPP(cf_ptr->pp[n][0], "�ޥ�") || 
	     MatchPP(cf_ptr->pp[n][0], "����") || 
	     MatchPP(cf_ptr->pp[n][0], "��") || 
	     MatchPP(cf_ptr->pp[n][0], "���δط�")) {
	sprintf(feature_buffer, "��ά�����ʤ�-%s", 
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
	     check_feature(cpm_ptr->pred_b_ptr->f, "����̾��ʲ���") && 
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
       ����¾�ξ�������: AssignReferentThreshold */
    else if (maxscore > 0 && 
	     ((cpm_ptr->decided == CF_DECIDED && 
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

	word = make_print_string(maxs->bnst_data+maxi);

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

#ifdef INTEGRATE_JUMAN
/*==================================================================*/
     void EllipsisDetectForNoun(SENTENCE_DATA *cs, BNST_DATA *bp)
/*==================================================================*/
{
    char **def;
    int i, j, ssize = 5, scount, current, bend;
    SENTENCE_DATA *sbuf, *sp, *s;
    float score;
    char feature_buffer[DATA_LEN], *buffer;

    /* ̾������ʸ����� */
    def = GetDefinitionFromBunsetsu(bp);
    if (!def) {
	return;
    }

    sbuf = (SENTENCE_DATA *)malloc_data(sizeof(SENTENCE_DATA)*ssize, 
					"EllipsisDetectForNoun");

    for (scount = 0; *(def+scount); scount++) {
#ifdef DEBUGMORE
	fprintf(Outfp, "���ʸ[%s] %d: %s\n", bp->Jiritu_Go, scount, *(def+scount));
#endif
	buffer = (char *)malloc_data(strlen(*(def+scount))+100, "EllipsisDetectForNoun");
	sprintf(buffer, "C���ʸ;%d:%s", scount, *(def+scount));
	assign_cfeature(&(bp->f), buffer);
	free(buffer);

	if (scount >= ssize) {
	    sbuf = (SENTENCE_DATA *)realloc_data(sbuf, sizeof(SENTENCE_DATA)*(ssize <<= 1), 
						 "EllipsisDetectForNoun");
	}
	sp = sbuf+scount;

	/* ���ʸ����� */
	InitSentence(sp);
	ParseSentence(sp, *(def+scount));

	/* ���ʸ�˴ޤޤ��̾����Ф��ơ�����ʸ (ʸ��) ��̾��ǻ��Ƥ����Τ�õ�� */
	for (i = sp->Bnst_num-1; i >= 0; i--) {
	    if (CheckPureNoun(sp->bnst_data+i)) {
		/* sp->bnst_data+i: ���ʸ����θ� */
		maxscore = 0;
		current = 1;
#ifdef DEBUGMORE
		fprintf(Outfp, "���ʸ -- %s\n", (sp->bnst_data+i)->Jiritu_Go);
#endif
		/* ����ʸ (ʸ��) */
		for (s = cs; s >= sentence_data; s--) {
		    bend = s->Bnst_num;

		    for (j = bend-1; j >= 0; j--) {
			if (!CheckPureNoun(s->bnst_data+j))
			    continue;
			/* s->bnst_data+j: ����ʸ������θ� */
			score = CalcSimilarityForNoun(s->bnst_data+j, sp->bnst_data+i);
			if (score > maxscore) {
			    maxscore = score;
			    maxs = s;
			    maxi = j;
			}
			if (score > 0) {
			    /* ��ά���� */
			    sprintf(feature_buffer, "C��;%s;%s:%.3f", (s->bnst_data+j)->Jiritu_Go, 
				    (sp->bnst_data+i)->Jiritu_Go, 
				    score);
			    assign_cfeature(&(bp->f), feature_buffer);
#ifdef DEBUGMORE
			    fprintf(Outfp, "\t%.3f %s\n", score, (s->bnst_data+j)->Jiritu_Go);
#endif
			}
		    }
		    if (current)
			current = 0;
		}
		if (maxscore > 0) {
		    /* ���ꤷ����ά�ط� */
		    sprintf(feature_buffer, "C��;��%s��;%s:%.3f", (maxs->bnst_data+maxi)->Jiritu_Go, 
			    (sp->bnst_data+i)->Jiritu_Go, 
			    maxscore);
		    assign_cfeature(&(bp->f), feature_buffer);
#ifdef DEBUGMORE
		    fprintf(Outfp, "\t�� %s\n", (maxs->bnst_data+maxi)->Jiritu_Go);
#endif
		}
#ifdef DEBUGMORE
		fputc('\n', Outfp);
#endif
	    }
	}
	clear_cf(0);
    }

    /* ������ʸ�ǡ����� scount �� free */
    for (i = 0; i < scount; i++)
	ClearSentence(sbuf+i);
}
#endif

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

    /* �ʤ�Ϳ����줿���֤� */
    for (j = 0; *order[j]; j++) {
	for (i = 0; i < cf_ptr->element_num; i++) {
	    if ((MatchPP(cf_ptr->pp[i][0], order[j]) && 
		 cmm_ptr->result_lists_p[0].flag[i] == UNASSIGNED) || 
		(OptDemo == TRUE && /* ������Ƥ����äơ��ؼ���ΤȤ� */
		 cmm_ptr->result_lists_p[0].flag[i] != UNASSIGNED && 
		 MatchPP(cf_ptr->pp[i][0], order[j]) && 
		 check_feature(cpm_ptr->elem_b_ptr[cmm_ptr->result_lists_p[0].flag[i]]->f, "��ά�����оݻؼ���")) && 
		!(toflag && MatchPP(cf_ptr->pp[i][0], "��"))) {
		if ((MarkEllipsisCase(cpm_ptr, cf_ptr, i)) == 0) {
		    continue;
		}
		result = EllipsisDetectForVerb(sp, em_ptr, cpm_ptr, cmm_ptr, cf_ptr, i, mainflag);
		AppendCfFeature(em_ptr, cpm_ptr, cf_ptr, i);
		if (result) {
		    em_ptr->cc[cf_ptr->pp[i][0]].score = maxscore;

		    if (OptSVM == OPT_SVM) {
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
	    !MatchPP(cf_ptr->pp[i][0], "����")) {
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

    /* ����γʥե졼��ˤĤ��ƾ�ά���Ϥ�¹� */

    for (l = 0; l < frame_num; l++) {
	/* OR �γʥե졼������ */
	if (((*(cf_array+l))->etcflag & CF_SUM) && frame_num != 1) {
	    continue;
	}

	/* �ʥե졼��򲾻��� */
	cmm.cf_ptr = *(cf_array+l);
	cpm_ptr->result_num = 1;
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
	DeleteFromCF(&workem, cpm_ptr, &cmm);
	ClearAnaphoraList(banlist);
    }
    free(cf_array);
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

    sp_new = PreserveSentence(sp);

    /* ���Ѹ�������å� (ʸ������) */
    for (j = 0; j < sp->Best_mgr->pred_num; j++) {
	cpm_ptr = &(sp->Best_mgr->cpm[j]);

	/* �ʥե졼�ब�ʤ���� (���ʤ��餤õ���Ƥ⤤�����⤷��ʤ�) */
	if (cpm_ptr->result_num == 0 || 
	    cpm_ptr->cmm[0].cf_ptr->ipal_address == -1 || 
	    cpm_ptr->cmm[0].score == -2) {
	    continue;
	}

	/* ��ά���Ϥ��ʤ��Ѹ�
	   1. �롼��ǡ־�ά�����ʤ���feature ���Ĥ��Ƥ�����
	   2. ���Ѹ� (�֥���̾��ʲ��ϡפϽ���)
	   3. �ʲ���̵�� (�֡��ߤ���� �ʤ�) check_feature(cpm_ptr->pred_b_ptr->f, "�ʲ���̵��") || 
	   4. �������ʤΥ���̾��
	   5. ��٥�:A-
	   6. �ʡ���ˡ���
	   7. ���ơ��Ѹ��� (A), �� �������Ѹ��� �� A- 
	   check_feature(cpm_ptr->pred_b_ptr->f, "ID:���ơ��Ѹ���") */
	if (check_feature(cpm_ptr->pred_b_ptr->f, "��ά�����ʤ�")) {
	    continue;
	}
	else if((!check_feature(cpm_ptr->pred_b_ptr->f, "����̾��ʲ���") && 
		 check_feature(cpm_ptr->pred_b_ptr->f, "���Ѹ�")) || 
		(check_feature(cpm_ptr->pred_b_ptr->f, "����̾��ʲ���") && 
		 check_feature(L_Jiritu_M(cpm_ptr->pred_b_ptr)->f, "��������")) || 
		check_feature(cpm_ptr->pred_b_ptr->f, "��٥�:A-") || 
		check_feature(cpm_ptr->pred_b_ptr->f, "ID:�ʡ���ˡ���")) {
	    assign_cfeature(&(cpm_ptr->pred_b_ptr->f), "��ά�����ʤ�");
	    continue;
	}
	    

	cmm_ptr = &(cpm_ptr->cmm[0]);
	cf_ptr = cmm_ptr->cf_ptr;

	/* ����ʸ�μ��� */
	if (lastflag == 1 && 
	    !check_feature(cpm_ptr->pred_b_ptr->f, "�����") && 
	    !check_feature(cpm_ptr->pred_b_ptr->f, "�ʲ���̵��")) {
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
		DeleteFromCF(&workem, cpm_ptr, &(cpm_ptr->cmm[0]));
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
		assign_gaga_slot(sp, cpm_ptr);
		assign_ga_subject(sp, cpm_ptr); /* CF_CAND_DECIDED �ξ��ϹԤäƤ��뤬 */
		/* fix_sm_place(sp, cpm_ptr); */
		/* �ʲ��Ϥη�̤� feature �� */
		record_case_analysis(sp, cpm_ptr, &maxem, mainflag);
		/* ��¸����ǡ����ˤ� (�����Ψ)
		cs = sentence_data + sp->Sen_num - 1;
		for (i = 0; i < cpm_ptr->cf.element_num; i++) {
		    if (cpm_ptr->elem_b_num[i] == -2) {
			continue;
		    }
		    bn = cpm_ptr->elem_b_ptr[i]->num;
		    if (sp->bnst_data[bn].pred_b_ptr) {
			cs->bnst_data[bn].pred_b_ptr = cs->bnst_data + (sp->bnst_data[bn].pred_b_ptr - sp->bnst_data);
		    }
		} */
	    }
	    else {
		/* �ʲ��Ϥη�̤� feature �� */
		record_case_analysis(sp, cpm_ptr, &maxem, mainflag);
	    }

	    /* ����̾��Υ��ʤϤޤ��������Τǵ�Ͽ���ʤ� -> ����̾�줹�٤Ƶ�Ͽ���ʤ�
	       MatchPP(cf_ptr->pp[i][0], "��") */
	    if (!check_feature(cpm_ptr->pred_b_ptr->f, "����̾��ʲ���")) {
		for (i = 0; i < CASE_MAX_NUM; i++) {
		    if (maxem.cc[i].s) {
			/* ��󥯤��줿���Ȥ�Ͽ���� */
			RegisterAnaphor(alist, (maxem.cc[i].s->bnst_data+maxem.cc[i].bnst)->Jiritu_Go);
			/* �Ѹ��ȳ����ǤΥ��åȤ�Ͽ (�ʴط��Ȥ϶��̤�����) */
			RegisterPredicate(L_Jiritu_M(cpm_ptr->pred_b_ptr)->Goi, 
					  cpm_ptr->pred_b_ptr->voice, 
					  cpm_ptr->cmm[0].cf_ptr->ipal_address, 
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

    /* ���θ�������å�
    for (i = sp->Bnst_num-1; i >= 0; i--) {
	if (CheckPureNoun(sp->bnst_data+i)) {
	    EllipsisDetectForNoun(sentence_data+sp->Sen_num-1, sp->bnst_data+i);
	}
    }
    */

    PreserveCPM(sp_new, sp);
    clear_cf(0);
}

/*====================================================================
                               END
====================================================================*/
