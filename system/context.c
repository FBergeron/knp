/*====================================================================

			       ʸ̮����

                                               S.Kurohashi 98. 9. 8

    $Id$
====================================================================*/
#include "knp.h"

float maxscore;
SENTENCE_DATA *maxs;
int maxi;
int Bcheck[BNST_MAX];

#define RANK0	0
#define RANK1	1
#define RANK2	2
#define RANK3	3

#define AssignReferentThreshold	0.3
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
   void RegisterPredicate(char *key, int voice, int pp, char *word, int flag)
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
	    if (!strcmp((*papp)->key, key) && (*papp)->voice == voice) {
		StoreCaseComponent(&((*papp)->cc[pp]), word, flag);
		return;
	    }
	    papp = &((*papp)->next);
	} while (*papp);
	*papp = (PALIST *)malloc_data(sizeof(PALIST), "RegisterPredicate");
	(*papp)->key = strdup(key);
	(*papp)->voice = voice;
	memset((*papp)->cc, 0, sizeof(CASE_COMPONENT *)*CASE_MAX_NUM);
	StoreCaseComponent(&((*papp)->cc[pp]), word, flag);
	(*papp)->next = NULL;
    }
    else {
	pap->key = strdup(key);
	pap->voice = voice;
	StoreCaseComponent(&(pap->cc[pp]), word, flag);
    }
}

/*==================================================================*/
     int CheckPredicate(char *key, int voice, int pp, char *word)
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
	if (!strcmp(pap->key, key) && pap->voice == voice) {
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
      void copy_cf_with_alloc(CASE_FRAME *dst, CASE_FRAME *src)
/*==================================================================*/
{
    int i, j;

    dst->element_num = src->element_num;
    for (i = 0; i < src->element_num; i++) {
	dst->oblig[i] = src->oblig[i];
	dst->adjacent[i] = src->adjacent[i];
	for (j = 0; j < PP_ELEMENT_MAX; j++) {
	    dst->pp[i][j] = src->pp[i][j];
	}
	if (src->sm[i]) {
	    dst->sm[i] = strdup(src->sm[i]);
	}
	else {
	    dst->sm[i] = NULL;
	}
	if (Thesaurus == USE_BGH) {
	    if (src->ex[i]) {
		dst->ex[i] = strdup(src->ex[i]);
	    }
	    else {
		dst->ex[i] = NULL;
	    }
	}
	else if (Thesaurus == USE_NTT) {
	    if (src->ex2[i]) {
		dst->ex2[i] = strdup(src->ex2[i]);
	    }
	    else {
		dst->ex2[i] = NULL;
	    }
	}
	if (src->examples[i]) {
	    dst->examples[i] = strdup(src->examples[i]);
	}
	else {
	    dst->examples[i] = NULL;
	}
	if (src->semantics[i]) {
	    dst->semantics[i] = strdup(src->semantics[i]);
	}
	else {
	    dst->semantics[i] = NULL;
	}
    }
    dst->voice = src->voice;
    dst->ipal_address = src->ipal_address;
    dst->ipal_size = src->ipal_size;
    strcpy(dst->ipal_id, src->ipal_id);
    strcpy(dst->imi, src->imi);
    dst->concatenated_flag = src->concatenated_flag;
    dst->flag = src->flag;
    if (src->entry) {
	dst->entry = strdup(src->entry);
    }
    else {
	dst->entry = NULL;
    }
    /* weight, pred_b_ptr ��̤���� */
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

    init_mgr_cf(s);
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
		(sp_new->cpm+i)->elem_b_ptr[j] = sp_new->bnst_data+((sp_new->cpm+i)->elem_b_ptr[j]-sp->bnst_data);
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

    /* ���� cpm ����¸���Ƥ��뤬��Best_mgr ����¸���������������⤷��ʤ� */
    sp_new->Best_mgr = NULL;
}

/*==================================================================*/
float CalcSimilarityForVerb(BNST_DATA *cand, CASE_FRAME *cf_ptr, int n)
/*==================================================================*/
{
    char *exd, *exp;
    int i, j, step;
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

    /* ����ޥå������������ */
    ex_score = (float)CalcSimilarity(exd, exp);
    if (Thesaurus == USE_BGH) {
	ex_score /= 7;
    }

    /* ���ΤΥޥå��� (�Ȥꤢ�������ʤΤȤ�����) */
    if (cf_ptr->sm[n] && MatchPP(cf_ptr->pp[n][0], "��")) {
	for (j = 0; cf_ptr->sm[n][j]; j+=step) {
	    if (strncmp(cf_ptr->sm[n]+j, sm2code("����"), SM_CODE_SIZE))
		continue;
	    /* �ʥե졼��¦�� <����> ������Ȥ��ˡ�������¦������å� */
	    if (check_feature(cand->f, "��̾") || 
		check_feature(cand->f, "�ȿ�̾")) {
		/* ��ͭ̾��ΤȤ��˥�������⤯ */
		if (EX_match_subject > 8) {
		    /* ���Υ��������⤤�Ȥ��ϡ������Ʊ���ˤ��� */
		    score = (float)EX_match_subject/11;
		}
		else {
		    score = (float)9/11;
		}
		break;
	    }
	    for (i = 0; cand->SM_code[i]; i+=step) {
		if (_sm_match_score(cf_ptr->sm[n]+j, cand->SM_code+i, SM_NO_EXPAND_NE)) {
		    score = (float)EX_match_subject/11;
		    break;
		}
	    }
	    break;
	}
    }


    if (ex_score > score) {
	return ex_score;
    }
    else {
	return score;
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
    ex_score = (float)CalcSimilarity(exd, exp);
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
    if (check_feature(bp->f, "�θ�") && 
	!check_feature(bp->f, "����") && 
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
	    if (cpm_ptr->cmm[0].result_lists_d[0].flag[i] >= 0) {
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
   int CheckEllipsisComponent(CF_PRED_MGR *cpm_ptr, BNST_DATA *bp)
/*==================================================================*/
{
    int i, num;

    /* �Ѹ��������Ʊ��ɽ����
       ¾�γʤξ�ά�λؼ��оݤȤ��Ƥ�äƤ��뤫�ɤ��� */

    for (i = 0; i < cpm_ptr->cmm[0].cf_ptr->element_num; i++) {
	num = cpm_ptr->cmm[0].result_lists_p[0].flag[i];
	/* ��ά�λؼ��о� */
	if (num >= 0 && 
	    cpm_ptr->elem_b_num[num] == -2 && 
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
     int CheckObligatoryCase(CF_PRED_MGR *cpm_ptr, BNST_DATA *bp)
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

    if (cpm_ptr->cmm[0].score != -2) {
	for (i = 0; i < cpm_ptr->cmm[0].cf_ptr->element_num; i++) {
	    num = cpm_ptr->cmm[0].result_lists_p[0].flag[i];
	    /* ���줬Ĵ�٤������ */
	    if (num != UNASSIGNED && 
		cpm_ptr->elem_b_num[num] != -2 && /* ��ά�γ����Ǥ���ʤ� */
		cpm_ptr->elem_b_ptr[num]->num == bp->num) {
		if (MatchPP(cpm_ptr->cmm[0].cf_ptr->pp[i][0], "��") || 
		    MatchPP(cpm_ptr->cmm[0].cf_ptr->pp[i][0], "��") || 
		    MatchPP(cpm_ptr->cmm[0].cf_ptr->pp[i][0], "��") || 
		    MatchPP(cpm_ptr->cmm[0].cf_ptr->pp[i][0], "����")) {
		    return 1;
		}
		return 0;
	    }
	}
    }
    return 0;
}

/*==================================================================*/
int CheckCaseCorrespond(CF_PRED_MGR *cpm_ptr, BNST_DATA *bp, CASE_FRAME *cf_ptr, int n)
/*==================================================================*/
{
    /* �ʤΰ��פ�Ĵ�٤�
       bp: �о�ʸ��
       cpm_ptr: �о�ʸ��η����Ѹ� (bp->parent->cpm_ptr)
    */

    int i, num;

    if (cpm_ptr->cmm[0].score != -2) {
	for (i = 0; i < cpm_ptr->cmm[0].cf_ptr->element_num; i++) {
	    num = cpm_ptr->cmm[0].result_lists_p[0].flag[i];
	    /* ���줬Ĵ�٤������ */
	    if (num != UNASSIGNED && cpm_ptr->elem_b_ptr[num]->num == bp->num) {
		if (cf_ptr->pp[n][0] == cpm_ptr->cmm[0].cf_ptr->pp[i][0] || 
		    (MatchPP(cf_ptr->pp[n][0], "��") && MatchPP(cpm_ptr->cmm[0].cf_ptr->pp[i][0], "����"))) {
		    return 1;
		}
		return 0;
	    }
	}
    }
    return 0;
}

/*==================================================================*/
void EllipsisDetectForVerbSubcontract(SENTENCE_DATA *s, SENTENCE_DATA *cs, ELLIPSIS_MGR *em_ptr, 
				      CF_PRED_MGR *cpm_ptr, BNST_DATA *bp, CASE_FRAME *cf_ptr, int n, int type)
/*==================================================================*/
{
    float score, weight, ascore, pascore, pcscore, mcscore, rawscore, topicscore, distscore;
    char feature_buffer[DATA_LEN];
    int ac, pac, pcc, mcc, topicflag, distance, agentflag, firstsc, subtopicflag, sameflag;
    int exception = 0;

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

    if (CheckEllipsisComponent(cpm_ptr, bp)) {
	return;
    }

    /* Ʊ�ʤ���¦�ϸ���ˤ��ʤ� */
    if (bp->dpnd_type == 'A') {
	return;
    }

    /* ���ߤ�ʸ�����оݤȤʤäƤ�������Ǥ�ʸ�ޤǤε�Υ */
    distance = cs-s;

    if (distance > 8) {
	distscore = 0.2;
    }
    else {
	distscore = 1-(float)distance*0.1;
    }

    /* �ʤΰ��פ�����å� */
    if (distance == 0 && bp->parent && bp->parent->cpm_ptr && 
	bp->num < cpm_ptr->pred_b_ptr->num && 
	CheckCaseCorrespond(bp->parent->cpm_ptr, bp, cf_ptr, n) == 0) {
	weight = 0.9;
    }
    else {
	weight = 1;
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
			 cf_ptr->pp[n][0], bp->Jiritu_Go);
    pascore = 1.0+0.5*pac; /* 0.2 */

    /* ��ʸ�μ���Υ����� */
    pcc = CheckLastClause(cs->Sen_num-1, cf_ptr->pp[n][0], bp->Jiritu_Go);

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
	subtopicflag = 1;
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
    if (sm_match_check(sm2code("����"), cf_ptr->sm[n])) {
	agentflag = 1;
    }
    else {
	agentflag = 0;
    }

    /* N �� �� N ���� */
    if (sameflag) {
	rawscore = 1;
    }
    /* V �����Τ� N ����(���ʤ����Ǥʤ���ʤ⤢��) */
    else if (check_feature(cpm_ptr->pred_b_ptr->f, "ID:���Ρ�") && 
	     check_feature(bp->f, "�Ѹ�:Ƚ")) {
	rawscore = 1;
	exception = 1;
    }
    else {
	rawscore = CalcSimilarityForVerb(bp, cf_ptr, n);
    }

    /* ��Ƭʸ�μ��������å� */
    firstsc = CheckLastClause(1, cf_ptr->pp[n][0], bp->Jiritu_Go);

    /* �ߤ���� */
    if (((s->Sen_num == 1 && firstsc == 0) || 
	 (distance == 1 && pcc == 0) || 
	 (distance == 0 && mcc == 0)
	) && 
	((bp->num == s->Bnst_num-1 && check_feature(bp->f, "�Ѹ�:Ƚ")) || /* ʸ����Ƚ��� */
	 (bp->parent && bp->parent->parent && check_feature(bp->parent->parent->f, "����") && subordinate_level_check("B+", bp->parent) && 
	  CheckObligatoryCase(bp->parent->cpm_ptr, bp)) || /* ������°�� */
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

    /* ��Ƭʸ���Ѹ���ľ���ˤʤ����ǥ�, �ǥϤǤϤʤ��ǳʤ��оݤˤ��ʤ� 
       ¾��Ǥ�ճʤ⤽�����⤷��ʤ� */
    if (firstsc && s->Sen_num == 1 && 
	check_feature(bp->f, "��:�ǳ�") && 
	bp->dpnd_head != bp->num+1 && 
	!check_feature(bp->f, "�ǥ�") && 
	!check_feature(bp->f, "�ǥ�")) {
	firstsc = 0;
    }

    if (distance == 0 || /* �о�ʸ */
	(distance == 1 && (topicflag || subtopicflag || pcc)) || /* ��ʸ */
	(s->Sen_num == 1 && (topicflag || subtopicflag || firstsc)) || /* ��Ƭʸ */
	(pcc || mcc || firstsc)) { /* ����ʳ��Ǽ���ξ�ά�λؼ��о� */

	/* �о�ʸ�ˤϡ�����������ɬ�� 
	   ����γ����Ǥξ��Ǥ⡢�о��Ѹ�������˽и����Ƥ���ɬ�פ����� */
	if (distance == 0 && !exception && !((topicflag || mcc || type >= RANK2) && bp->num < cpm_ptr->pred_b_ptr->num)) {
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
	    else if (type == RANK3 && check_feature(cpm_ptr->pred_b_ptr->f, "�Ѹ�:Ƚ")) {
		score = weight*pascore*1.0;
	    }
	    else {
		score = weight*pascore*rawscore;
	    }
	}

    }
    /* ���̤� N-V �Υ��åȤΤȤ��ϵ��� */
    else if (pascore > 1 && rawscore > 0.8) {
	score = weight*pascore*rawscore;
    }
    else {
	score = 0;
    }

    /* ��Υ���̣
    score *= distscore; */

    if (score > maxscore) {
	maxscore = score;
	maxs = s;
	if (bp->num < 0) {
	    maxi = bp->parent->num;
	}
	else {
	    maxi = bp->num;
	}
    }

    /* ��ά���� */
    if (score > 0) {
	sprintf(feature_buffer, "C��;%s;%s;%d;%d;%.3f|%.3f", bp->Jiritu_Go, 
		pp_code_to_kstr(cf_ptr->pp[n][0]), 
		distance, maxi, 
		score, rawscore);
	assign_cfeature(&(em_ptr->f), feature_buffer);
	sprintf(feature_buffer, "�ؽ�FEATURE;%s;%s;%.3f|%d|%d|%d|%d|%d|%d|%d|%d", 
		bp->Jiritu_Go, 
		pp_code_to_kstr(cf_ptr->pp[n][0]), 
		rawscore, type, ac, pac, pcc, mcc, topicflag, agentflag, distance);
	assign_cfeature(&(em_ptr->f), feature_buffer);
    }

    Bcheck[bp->num] = 1;
}

/*==================================================================*/
void SearchCaseComponent(SENTENCE_DATA *s, ELLIPSIS_MGR *em_ptr, CF_PRED_MGR *cpm_ptr, 
			 BNST_DATA *bp, CASE_FRAME *cf_ptr, int n)
/*==================================================================*/
{
    /* cpm_ptr: ��ά�����Ǥ����Ѹ�
       bp:      �����Ǥ�õ���оݤȤʤäƤ����Ѹ�ʸ��
    */

    int i, num;

    /* ¾���Ѹ� (���Ѹ��ʤ�) �γ����Ǥ�����å� */
    if (bp->cpm_ptr && bp->cpm_ptr->cmm[0].score != -2) {
	for (i = 0; i < bp->cpm_ptr->cmm[0].cf_ptr->element_num; i++) {
	    num = bp->cpm_ptr->cmm[0].result_lists_p[0].flag[i];
	    if (num != UNASSIGNED && 
		bp->cpm_ptr->elem_b_num[num] != -2 && /* �����Ǥ���ά����ä���ΤǤ���Ȥ��Ϥ��� */
		bp->cpm_ptr->elem_b_ptr[num]->num != cpm_ptr->pred_b_ptr->num && /* �����Ǥ����Ѹ��ΤȤ��Ϥ��� (bp->cpm_ptr->elem_b_ptr[num] ������ΤȤ� <PARA> �Ȥʤꡢpointer �ϥޥå����ʤ�) */
		CheckTargetNoun(bp->cpm_ptr->elem_b_ptr[num])) {
		/* �����Ǥγʤΰ��� (�ʤˤ�ꤱ��) */
		if (cf_ptr->pp[n][0] == bp->cpm_ptr->cmm[0].cf_ptr->pp[i][0] || 
		    (MatchPP(cf_ptr->pp[n][0], "��") && MatchPP(bp->cpm_ptr->cmm[0].cf_ptr->pp[i][0], "����"))) {
		    EllipsisDetectForVerbSubcontract(s, s, em_ptr, cpm_ptr, bp->cpm_ptr->elem_b_ptr[num], 
						     cf_ptr, n, RANK3);
		}
		/* �ʤ��԰��� */
		else {
		    EllipsisDetectForVerbSubcontract(s, s, em_ptr, cpm_ptr, bp->cpm_ptr->elem_b_ptr[num], 
						     cf_ptr, n, RANK2);
		}
	    }
	}
    }
}

/*==================================================================*/
	int AppendToCF(CF_PRED_MGR *cpm_ptr, BNST_DATA *b_ptr,
		       CASE_FRAME *cf_ptr, int n, float maxscore)
/*==================================================================*/
{
    /* ��ά�λؼ��оݤ�����¦�γʥե졼�������� */

    CASE_FRAME *c_ptr = &(cpm_ptr->cf);

    if (c_ptr->element_num >= CF_ELEMENT_MAX) {
	return 0;
    }

    /* �б�������ɲ� */
    cpm_ptr->cmm[0].result_lists_p[0].flag[n] = c_ptr->element_num;
    cpm_ptr->cmm[0].result_lists_d[0].flag[c_ptr->element_num] = n;
    /* cpm_ptr->cmm[0].result_lists_p[0].score[n] = -1; */
    if (maxscore > 1) {
	cpm_ptr->cmm[0].result_lists_p[0].score[n] = 11;
    }
    else {
	cpm_ptr->cmm[0].result_lists_p[0].score[n] = 11*maxscore;
    }

    c_ptr->pp[c_ptr->element_num][0] = cf_ptr->pp[n][0];
    c_ptr->pp[c_ptr->element_num][1] = END_M;
    c_ptr->oblig[c_ptr->element_num] = TRUE;
    _make_data_cframe_sm(cpm_ptr, b_ptr);
    _make_data_cframe_ex(cpm_ptr, b_ptr);
    cpm_ptr->elem_b_ptr[c_ptr->element_num] = b_ptr;
    cpm_ptr->elem_b_num[c_ptr->element_num] = -2;
    c_ptr->weight[c_ptr->element_num] = 0;
    c_ptr->adjacent[c_ptr->element_num] = FALSE;
    c_ptr->element_num++;
    return 1;
}

/*==================================================================*/
     int DeleteFromCF(ELLIPSIS_MGR *em_ptr, CF_PRED_MGR *cpm_ptr)
/*==================================================================*/
{
    int i, count = 0;

    /* ��ά�λؼ��оݤ�����¦�γʥե졼�फ�������� */

    for (i = 0; i < cpm_ptr->cf.element_num; i++) {
	if (cpm_ptr->elem_b_num[i] == -2) {
	    cpm_ptr->cmm[0].result_lists_p[0].flag[cpm_ptr->cmm[0].result_lists_d[0].flag[i]] = -1;
	    cpm_ptr->cmm[0].result_lists_d[0].flag[i] = -1;
	    count++;
	}
    }
    cpm_ptr->cf.element_num -= count;
    return 1;
}

/*==================================================================*/
int EllipsisDetectForVerb(SENTENCE_DATA *sp, ELLIPSIS_MGR *em_ptr, 
			  CF_PRED_MGR *cpm_ptr, CASE_FRAME *cf_ptr, int n, int last)
/*==================================================================*/
{
    /* �Ѹ��Ȥ��ξ�ά�ʤ�Ϳ������ */

    /* cf_ptr = cpm_ptr->cmm[0].cf_ptr �Ǥ��� */
    /* �Ѹ� cpm_ptr �� cf_ptr->pp[n][0] �ʤ���ά����Ƥ���
       cf_ptr->ex[n] �˻��Ƥ���ʸ���õ�� */

    int i, current = 1, bend;
    char feature_buffer[DATA_LEN], etc_buffer[DATA_LEN], *cp;
    SENTENCE_DATA *s, *cs;

    maxscore = 0;
    cs = sentence_data + sp->Sen_num - 1;
    memset(Bcheck, 0, BNST_MAX);

    /* �Ƥ�ߤ� (PARA �ʤ� child �Ѹ�) */
    if (cpm_ptr->pred_b_ptr->parent) {
	/* �Ƥ� PARA */
	if (cpm_ptr->pred_b_ptr->parent->para_top_p) {
	    /* ��ʬ��������Ѹ� */
	    for (i = 0; cpm_ptr->pred_b_ptr->parent->child[i]; i++) {
		/* PARA �λҶ��ǡ���ʬ��ʳ��������Ѹ� */
		if (cpm_ptr->pred_b_ptr->parent->child[i] != cpm_ptr->pred_b_ptr &&
		    cpm_ptr->pred_b_ptr->parent->child[i]->para_type == PARA_NORMAL) {
		    SearchCaseComponent(cs, em_ptr, cpm_ptr, cpm_ptr->pred_b_ptr->parent->child[i], 
					cf_ptr, n);
		}
	    }

	    /* Ϣ�ѤǷ�����Ѹ� (����ΤȤ�) */
	    if (cpm_ptr->pred_b_ptr->parent->parent && 
		check_feature(cpm_ptr->pred_b_ptr->f, "��:Ϣ��")) {
		SearchCaseComponent(cs, em_ptr, cpm_ptr, cpm_ptr->pred_b_ptr->parent->parent, 
				    cf_ptr, n);
	    }
	}
	/* �Ȥꤢ������Ϣ�ѤǷ���ҤȤľ�ο��Ѹ��Τ� */
	else if (check_feature(cpm_ptr->pred_b_ptr->f, "��:Ϣ��")) {
	    SearchCaseComponent(cs, em_ptr, cpm_ptr, cpm_ptr->pred_b_ptr->parent, 
				cf_ptr, n);
	}
	/* �֡�������פʤ� */
	else if (check_feature(cpm_ptr->pred_b_ptr->f, "��°�᰷��")) {
	    SearchCaseComponent(cs, em_ptr, cpm_ptr, cpm_ptr->pred_b_ptr->parent->parent, 
				cf_ptr, n);
	}
    }

    /* �Ҷ� (�Ѹ�) �򸫤� */
    for (i = 0; cpm_ptr->pred_b_ptr->child[i]; i++) {
	if (check_feature(cpm_ptr->pred_b_ptr->child[i]->f, "�Ѹ�")) {
	    SearchCaseComponent(cs, em_ptr, cpm_ptr, cpm_ptr->pred_b_ptr->child[i], 
				cf_ptr, n);
	}
    }

    if (cp = check_feature(cpm_ptr->pred_b_ptr->f, "�ȱ��ҥ��")) {
	if (str_eq(cp, "�ȱ��ҥ��:��")) {
	    /* ��������Ѹ��˷�������Ǥ�ߤ� (����̾��ΤȤ�) */
	    SearchCaseComponent(cs, em_ptr, cpm_ptr, cs->bnst_data+cpm_ptr->pred_b_ptr->dpnd_head, cf_ptr, n);

	    /* ��������Ѹ��˷��뽾°��˷�������Ǥ�ߤ� */
	    for (i = 0; (cs->bnst_data+cpm_ptr->pred_b_ptr->dpnd_head)->child[i]; i++) {
		if (check_feature((cs->bnst_data+cpm_ptr->pred_b_ptr->dpnd_head)->child[i]->f, "�Ѹ�")) {
		    SearchCaseComponent(cs, em_ptr, cpm_ptr, (cs->bnst_data+cpm_ptr->pred_b_ptr->dpnd_head)->child[i], 
					cf_ptr, n);
		}
	    }
	}
	else {
	    i = cpm_ptr->pred_b_ptr->num+atoi(cp+11);
	    if (i >= 0 && i < cs->Bnst_num) {
		SearchCaseComponent(cs, em_ptr, cpm_ptr, cs->bnst_data+i, cf_ptr, n);
	    }
	}
    }

    /* ����ʸ���θ���õ�� (�����Ѹ��γ����ǤˤʤäƤ����ΰʳ�) */
    for (s = cs; s >= sentence_data; s--) {
	bend = s->Bnst_num;

	for (i = bend-1; i >= 0; i--) {
	    if (current) {
		if (Bcheck[i] || 
		    (!check_feature((s->bnst_data+i)->f, "��:Ϣ��") && 
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

		/* A �� B �� �� V */
		if (cp = check_feature((s->bnst_data+i)->f, "��ά��������å�")) {
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

	    EllipsisDetectForVerbSubcontract(s, cs, em_ptr, cpm_ptr, s->bnst_data+i, cf_ptr, n, RANK1);
	}
	if (current)
	    current = 0;
    }

    /* �ڼ��ΰ��̡�
       1. �Ѹ������Ȥǥ˳� (��Ȥϥ���) �� <����> ��Ȥ�Ȥ�
       2. �֡�����(��)�פǥ��ʤ� <����> ��Ȥ�Ȥ� 
       3. ���� V ���� N (���δط�, !Ƚ���), ����̾��, ����̾��Ͻ���
       4. �����������ͤ�겼�� <����> ��Ȥ�Ȥ� */
    if ((maxscore <= AssignReferentThreshold && 
	 cf_match_element(cf_ptr->sm[n], "����", FALSE)) || 
	(check_feature(cpm_ptr->pred_b_ptr->f, "ID:���ʤ����") && 
	 MatchPP(cf_ptr->pp[n][0], "��") && 
	 cf_match_element(cf_ptr->sm[n], "����", TRUE)) || 
	(MatchPP(cf_ptr->pp[n][0], "��") && 
	cf_match_element(cf_ptr->sm[n], "����", FALSE) && 
	(check_feature(cpm_ptr->pred_b_ptr->f, "�����") || 
	 check_feature(cpm_ptr->pred_b_ptr->f, "������") || 
	 check_feature(cpm_ptr->pred_b_ptr->f, "����̾��ʲ���"))) || 
	(cpm_ptr->pred_b_ptr->parent && 
	 (check_feature(cpm_ptr->pred_b_ptr->parent->f, "���δط�") || 
	  check_feature(cpm_ptr->pred_b_ptr->parent->f, "���δط���ǽ��") || 
	  check_feature(cpm_ptr->pred_b_ptr->parent->f, "���δط�Ƚ��")) && 
	 !check_feature(cpm_ptr->pred_b_ptr->parent->f, "����̾��") && 
	 !check_feature(cpm_ptr->pred_b_ptr->parent->f, "����̾��") && 
	 !check_feature(cpm_ptr->pred_b_ptr->parent->f, "�Ѹ�") && 
	 check_feature(cpm_ptr->pred_b_ptr->f, "��:Ϣ��") && 
	 cf_ptr->pp[n][0] == pp_kstr_to_code("��"))) {
	/* �ʥե졼������롼�פΤȤ��� feature ��Ϳ����Τ����� */
	sprintf(feature_buffer, "C��;�ڼ��ΰ��̡�;%s;-1;-1;1", 
		pp_code_to_kstr(cf_ptr->pp[n][0]));
	assign_cfeature(&(em_ptr->f), feature_buffer);
    }
    /* ���ξ��Ͼ�ά���Ǥ�õ������Ͽ���ʤ� 
       (�������Ǥϥǡ����򸫤뤿�ᡢ�������ά���Ϥ�ԤäƤ���) 
       �ǳ�, �ȳ�, ����, �����, �ޥǳ� */
    else if (MatchPP(cf_ptr->pp[n][0],"��") || 
	     MatchPP(cf_ptr->pp[n][0], "��") || 
	     MatchPP(cf_ptr->pp[n][0], "���") || 
	     MatchPP(cf_ptr->pp[n][0], "����") || 
	     MatchPP(cf_ptr->pp[n][0], "�ޥ�")) {
	sprintf(feature_buffer, "��ά�����ʤ�-%s", 
		pp_code_to_kstr(cf_ptr->pp[n][0]));
	assign_cfeature(&(em_ptr->f), feature_buffer);
    }
    else if (maxscore > 0 && 
	     maxscore < 0.9 && 
	     MatchPP(cf_ptr->pp[n][0],"��") && 
	     sm_match_check(sm2code("���ʪ"), cpm_ptr->pred_b_ptr->SM_code)) {
	sprintf(feature_buffer, "C��;��������ʪ��;%s;-1;-1;1", 
		pp_code_to_kstr(cf_ptr->pp[n][0]));
	assign_cfeature(&(em_ptr->f), feature_buffer);
	/* ���祹�����λؼ��оݤ� dummy �ǳʥե졼�����¸ 
	   ���줬���ۤ��γʤθ���ˤʤ�ʤ��ʤ�Τ����� */
	AppendToCF(cpm_ptr, maxs->bnst_data+maxi, cf_ptr, n, maxscore);
	return 1;
    }
    else if (maxscore > AssignReferentThreshold) {
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
	AppendToCF(cpm_ptr, maxs->bnst_data+maxi, cf_ptr, n, maxscore);

	return 1;
    }
    return 0;
}

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
	clear_cf();
    }

    /* ������ʸ�ǡ����� scount �� free */
    for (i = 0; i < scount; i++)
	ClearSentence(sbuf+i);
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

    /* ��ά����Ƥ���ʤ�ޡ��� */
    sprintf(feature_buffer, "C��ά-%s", 
	    pp_code_to_kstr(cf_ptr->pp[n][0]));
    assign_cfeature(&(cpm_ptr->pred_b_ptr->f), feature_buffer);

    /* <���������֡�����> �򥬳ʤȤ��ƤȤ�Ƚ��� */
    if (check_feature(cpm_ptr->pred_b_ptr->f, "���֥���ά") && 
	MatchPP(cf_ptr->pp[n][0], "��")) {
	sprintf(feature_buffer, "C��;�ڻ��������֡�������;%s;-1;-1;1", 
		pp_code_to_kstr(cf_ptr->pp[n][0]));
	assign_cfeature(&(cpm_ptr->pred_b_ptr->f), feature_buffer);
	return 0;
    }
    return 1;
}

/*==================================================================*/
float EllipsisDetectForVerbMain(SENTENCE_DATA *sp, ELLIPSIS_MGR *em_ptr, CF_PRED_MGR *cpm_ptr, 
				CASE_FRAME *cf_ptr, char **order, int mainflag, int toflag, int onceflag)
/*==================================================================*/
{
    int i, j, num, result;
    CF_MATCH_MGR *cmm_ptr;

    cmm_ptr = &(cpm_ptr->cmm[0]);

    /* �ʤ�Ϳ����줿���֤� */
    for (j = 0; *order[j]; j++) {
	for (i = 0; i < cf_ptr->element_num; i++) {
	    if (MatchPP(cf_ptr->pp[i][0], order[j]) && 
		cmm_ptr->result_lists_p[0].flag[i] == UNASSIGNED && 
		!(toflag && MatchPP(cf_ptr->pp[i][0], "��"))) {
		if ((MarkEllipsisCase(cpm_ptr, cf_ptr, i)) == 0) {
		    continue;
		}
		result = EllipsisDetectForVerb(sp, em_ptr, cpm_ptr, cf_ptr, i, mainflag);
		if (result) {
		    em_ptr->cc[cf_ptr->pp[i][0]].score = maxscore;
		    em_ptr->score += maxscore;
		    if (onceflag) {
			/* �ҤȤĤξ�ά�λؼ��оݤ�ߤĤ����Τǡ�
			   �����Ǥ�äȤ⥹�����ι⤤�ʥե졼����Ĵ������ */
			find_best_cf(sp, cpm_ptr);
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
	    result = EllipsisDetectForVerb(sp, em_ptr, cpm_ptr, cf_ptr, i, mainflag);
	    if (result) {
		em_ptr->cc[cf_ptr->pp[i][0]].score = maxscore;
		em_ptr->score += maxscore;
		if (onceflag) {
		    find_best_cf(sp, cpm_ptr);
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
      int CompareDistance(ELLIPSIS_MGR *max, ELLIPSIS_MGR *work)
/*==================================================================*/
{
/*    int i, d = 0;

    for (i = 0; i < CASE_MAX_NUM; i++) {
	if (max->cc[i].s && max->cc[i].s == work->cc[i].s) {
	    max->cc[i].bnst
	}
    }
*/
}

/*==================================================================*/
	      void DiscourseAnalysis(SENTENCE_DATA *sp)
/*==================================================================*/
{
    int i, j, k, num, toflag, lastflag = 1, mainflag, anum;
    float score;
    ELLIPSIS_MGR workem, maxem;
    SENTENCE_DATA *sp_new;
    CF_PRED_MGR *cpm_ptr;
    CF_MATCH_MGR *cmm_ptr;
    CASE_FRAME *cf_ptr, origcf;

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
	   3. �ʲ���̵�� (�֡��ߤ���� �ʤ�)
	   4. �������ʤΥ���̾��
	   5. ��٥�:A-
	   6. �ʡ���ˡ���
	   7. ���ơ��Ѹ��� (A), �� �������Ѹ��� �� A- */
	if (check_feature(cpm_ptr->pred_b_ptr->f, "��ά�����ʤ�") || 
	    (!check_feature(cpm_ptr->pred_b_ptr->f, "����̾��ʲ���") && 
	     check_feature(cpm_ptr->pred_b_ptr->f, "���Ѹ�")) || 
	    check_feature(cpm_ptr->pred_b_ptr->f, "�ʲ���̵��") || 
	    (check_feature(cpm_ptr->pred_b_ptr->f, "����̾��ʲ���") && 
	     check_feature(L_Jiritu_M(cpm_ptr->pred_b_ptr)->f, "��������")) || 
	    check_feature(cpm_ptr->pred_b_ptr->f, "��٥�:A-") || 
	    check_feature(cpm_ptr->pred_b_ptr->f, "ID:�ʡ���ˡ���") || 
	    check_feature(cpm_ptr->pred_b_ptr->f, "ID:���ơ��Ѹ���")) {
	    continue;
	}
	    

	cmm_ptr = &(cpm_ptr->cmm[0]);
	cf_ptr = cmm_ptr->cf_ptr;

	/* ��<��ʸ>�� ���� V ������ 
	   �ȳʤ�����Ȥ�����ʤ��ά�Ȥ��ʤ� 
	   (�ʥե졼��¦��<��ʸ>�ϥ����å����Ƥ��ʤ�) */
	toflag = 0;
	for (i = 0; i < cf_ptr->element_num; i++) {
	    num = cmm_ptr->result_lists_p[0].flag[i];
	    if (num != UNASSIGNED && 
		str_eq(pp_code_to_kstr(cmm_ptr->cf_ptr->pp[i][0]), "��") && 
		check_feature(cpm_ptr->elem_b_ptr[num]->f, "��ʸ")) {
		toflag = 1;
		break;
	    }
	}

	/* ����ʸ�μ��� */
	if (lastflag == 1 && !check_feature(cpm_ptr->pred_b_ptr->f, "�����")) {
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

	maxem.score = -1;
	origcf = *(cpm_ptr->cmm[0].cf_ptr);

	for (i = 0; i < CASE_ORDER_MAX; i++) {
	    ClearEllipsisMGR(&workem);
	    /* �����Ǥ��ҤȤĤ�ʤ��Ȥ��ϡ���ά�λؼ��оݤ�ҤȤķ��Ƥ���
	       ��äȤ⥹�����ι⤤�ʥե졼������� */
	    if (anum == 0) {
		if ((score = EllipsisDetectForVerbMain(sp, &workem, cpm_ptr, &origcf, 
						       CaseOrder[i], mainflag, toflag, 1)) > -1) {
		    score += EllipsisDetectForVerbMain(sp, &workem, cpm_ptr, cpm_ptr->cmm[0].cf_ptr, 
						       CaseOrder[i], mainflag, toflag, 0);
		}
	    }
	    else {
		score = EllipsisDetectForVerbMain(sp, &workem, cpm_ptr, cpm_ptr->cmm[0].cf_ptr, 
						  CaseOrder[i], mainflag, toflag, 0);
	    }

	    if (workem.score > maxem.score /* || 
		(workem.score == maxem.score && CompareDistance(maxem, workem)) */) {
		maxem = workem;
		maxem.result_num = cpm_ptr->result_num;
		for (k = 0; k < maxem.result_num; k++) {
		    maxem.cmm[k] = cpm_ptr->cmm[k];
		}
		maxem.element_num = cpm_ptr->cf.element_num;
		for (k = 0; k < maxem.element_num; k++) {
		    maxem.elem_b_ptr[k] = cpm_ptr->elem_b_ptr[k];
		    maxem.elem_b_num[k] = cpm_ptr->elem_b_num[k];
		}
		workem.f = NULL;
	    }

	    if (VerboseLevel >= VERBOSE2) {
		fprintf(stdout, "CASE_ORDER %2d ==> %.3f\n", i, workem.score);
		print_data_cframe(cpm_ptr);
		for (k = 0; k < cpm_ptr->result_num; k++) {
		    print_crrspnd(cpm_ptr, &(cpm_ptr->cmm[k]));
		}
		fputc('\n', Outfp);
	    }

	    /* �ʥե졼����ɲå���ȥ�κ�� */
	    DeleteFromCF(&workem, cpm_ptr);

	    ClearAnaphoraList(banlist);
	}

	/* ��äȤ� score �Τ褫�ä��Ȥ߹�碌����Ͽ */
	if (maxem.score > -1) {
	    /* cmm ������ */
	    cpm_ptr->result_num = maxem.result_num;
	    for (k = 0; k < maxem.result_num; k++) {
		cpm_ptr->cmm[k] = maxem.cmm[k];
	    }
	    cpm_ptr->cf.element_num = maxem.element_num;
	    for (k = 0; k < maxem.element_num; k++) {
		cpm_ptr->elem_b_ptr[k] = maxem.elem_b_ptr[k];
		cpm_ptr->elem_b_num[k] = maxem.elem_b_num[k];
	    }
	    /* feature ������ */
	    append_feature(&(cpm_ptr->pred_b_ptr->f), maxem.f);
	    maxem.f = NULL;

	    /* �ʥե졼��˾�ά���������
	    for (i = 0; i < CASE_MAX_NUM; i++) {
		if (maxem.cc[i].s) {
		    AppendToCF(cpm_ptr, maxem.cc[i].s->bnst_data+maxem.cc[i].bnst, cpm_ptr->cmm[0].cf_ptr, 
			       GetElementID(cpm_ptr->cmm[0].cf_ptr, i), maxem.cc[i].score);
		}
	    } */

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
    clear_cf();
}

/*====================================================================
                               END
====================================================================*/
