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

#define RANK1	1
#define RANK2	2
#define RANK3	3

ALIST alist[TBLSIZE];
PALIST palist[TBLSIZE];

int hash(unsigned char *key, int keylen);
extern int	EX_match_subject;

/*==================================================================*/
		       void InitAnaphoraList()
/*==================================================================*/
{
    memset(alist, 0, sizeof(ALIST)*TBLSIZE);
    memset(palist, 0, sizeof(PALIST)*TBLSIZE);
}

/*==================================================================*/
		       void ClearAnaphoraList()
/*==================================================================*/
{
    int i;
    ALIST *p, *next;
    for (i = 0; i < TBLSIZE; i++) {
	if (alist[i].key) {
	    free(alist[i].key);
	    alist[i].key = NULL;
	}
	if (alist[i].next) {
	    p = alist[i].next;
	    alist[i].next = NULL;
	    while (p) {
		free(p->key);
		next = p->next;
		free(p);
		p = next;
	    }
	}
	alist[i].count = 0;
    }
}

/*==================================================================*/
		   void RegisterAnaphor(char *key)
/*==================================================================*/
{
    ALIST *ap;

    ap = &(alist[hash(key, strlen(key))]);
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
		     int CheckAnaphor(char *key)
/*==================================================================*/
{
    ALIST *ap;

    ap = &(alist[hash(key, strlen(key))]);
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
      void StoreCaseComponent(CASE_COMPONENT **ccpp, char *word)
/*==================================================================*/
{
    while (*ccpp) {
	if (!strcmp((*ccpp)->word, word)) {
	    (*ccpp)->count++;
	    return;
	}
	ccpp = &((*ccpp)->next);
    }
    *ccpp = (CASE_COMPONENT *)malloc_data(sizeof(CASE_COMPONENT), "StoreCaseComponent");
    (*ccpp)->word = strdup(word);
    (*ccpp)->count = 1;
    (*ccpp)->next = NULL;
}

/*==================================================================*/
	void RegisterPredicate(char *key, int pp, char *word)
/*==================================================================*/
{
    PALIST *pap;

    /* ʣ�缭�ʤɤγʤϽ��� */
    if (pp == END_M || pp > 8) {
	return;
    }

    pap = &(palist[hash(key, strlen(key))]);
    if (pap->key) {
	PALIST **papp;
	papp = &pap;
	do {
	    if (!strcmp((*papp)->key, key)) {
		StoreCaseComponent(&((*papp)->cc[pp]), word);
		return;
	    }
	    papp = &((*papp)->next);
	} while (*papp);
	*papp = (PALIST *)malloc_data(sizeof(PALIST), "RegisterPredicate");
	(*papp)->key = strdup(key);
	memset((*papp)->cc, 0, sizeof(CASE_COMPONENT *)*CASE_MAX_NUM);
	StoreCaseComponent(&((*papp)->cc[pp]), word);
	(*papp)->next = NULL;
    }
    else {
	pap->key = strdup(key);
	StoreCaseComponent(&(pap->cc[pp]), word);
    }
}

/*==================================================================*/
	  int CheckPredicate(char *key, int pp, char *word)
/*==================================================================*/
{
    PALIST *pap;
    CASE_COMPONENT *ccp;

    /* ʣ�缭�ʤɤγʤϽ��� */
    if (pp == END_M || pp > 8) {
	return 0;
    }

    pap = &(palist[hash(key, strlen(key))]);
    if (!pap->key) {
	return 0;
    }
    while (pap) {
	if (!strcmp(pap->key, key)) {
	    ccp = pap->cc[pp];
	    while (ccp) {
		if (!strcmp(ccp->word, word)) {
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
    ClearAnaphoraList();
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
		void copy_sentence(SENTENCE_DATA *sp)
/*==================================================================*/
{
    /* ʸ���Ϸ�̤��ݻ� */

    int i, j, num, cfnum = 0;
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



	sp_new->bnst_data[i].mrph_ptr += sp_new->mrph_data - sp->mrph_data;
	sp_new->bnst_data[i].settou_ptr += sp_new->mrph_data - sp->mrph_data;
	sp_new->bnst_data[i].jiritu_ptr += sp_new->mrph_data - sp->mrph_data;
	sp_new->bnst_data[i].fuzoku_ptr += sp_new->mrph_data - sp->mrph_data;
	sp_new->bnst_data[i].parent += sp_new->bnst_data - sp->bnst_data;
	for (j = 0; sp_new->bnst_data[i].child[j]; j++) {
	    sp_new->bnst_data[i].child[j]+= sp_new->bnst_data - sp->bnst_data;
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
	    (sp_new->cpm+i)->elem_b_ptr[j] = sp_new->bnst_data+(sp_new->cpm+i)->elem_b_num[j];
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

    /* ���ΤΥޥå��� */
    if (cf_ptr->sm[n]) {
	for (j = 0; cf_ptr->sm[n][j]; j+=step) {
	    if (strncmp(cf_ptr->sm[n]+j, sm2code("����"), SM_CODE_SIZE))
		continue;
	    for (i = 0; cand->SM_code[i]; i+=step) {
		if (_sm_match_score(cf_ptr->sm[n]+j, cand->SM_code+i, SM_NO_EXPAND_NE)) {
		    score = (float)EX_match_subject/11;
		}
	    }
	}
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
    if (!check_feature(bp->f, "����")) {
	return TRUE;
    }
    return FALSE;
}

/*==================================================================*/
		   int CheckPureNoun(BNST_DATA *bp)
/*==================================================================*/
{
    if (check_feature(bp->f, "�θ�") && 
	!check_feature(bp->f, "����̾��") && 
	!check_feature(bp->f, "����") && 
	!check_feature(bp->f, "����") && 
	CheckTargetNoun(bp)) {
	return TRUE;
    }
    return FALSE;
}

/*==================================================================*/
void EllipsisDetectForVerbSubcontract(SENTENCE_DATA *s, SENTENCE_DATA *cs, CF_PRED_MGR *cpm_ptr, 
				      BNST_DATA *bp, CASE_FRAME *cf_ptr, int n, int type)
/*==================================================================*/
{
    float score, weight, ascore, pascore, rawscore, topicscore;
    char feature_buffer[DATA_LEN];
    int ac, pac, topicflag;

    /* ��󥯤�����ˤ�륹���� */
    weight = 1.0+0.2*(type-1);

    /* ��󥯤��줿����ˤ�륹���� */
    ac = CheckAnaphor(bp->Jiritu_Go);
    ascore = 1.0+0.1*ac;

    /* ���Ǥ˽и������Ѹ��Ȥ��γ����ǤΥ����� */
    pac = CheckPredicate(L_Jiritu_M(cpm_ptr->pred_b_ptr)->Goi, cf_ptr->pp[n][0], bp->Jiritu_Go);
    pascore = 1.0+0.2*pac;

    /* ����Υ����� */
    if (check_feature(bp->f, "����")) {
	topicflag = 1;
	topicscore = 1.2;
    }
    else {
	topicflag = 0;
	topicscore = 1.0;
    }

    rawscore = CalcSimilarityForVerb(bp, cf_ptr, n);
    score = ascore*pascore*weight*rawscore*topicscore;

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
    sprintf(feature_buffer, "C��;%s;%s:%.3f|%.3f", bp->Jiritu_Go, 
	    pp_code_to_kstr(cf_ptr->pp[n][0]), 
	    score, rawscore);
    assign_cfeature(&(cpm_ptr->pred_b_ptr->f), feature_buffer);
    sprintf(feature_buffer, "�ؽ�FEATURE;%s;%s:%.3f|%d|%d|%d|%d|%d", 
	    bp->Jiritu_Go, 
	    pp_code_to_kstr(cf_ptr->pp[n][0]), 
	    rawscore, type, ac, pac, topicflag, cs-s);
    assign_cfeature(&(cpm_ptr->pred_b_ptr->f), feature_buffer);

    Bcheck[bp->num] = 1;
}

/*==================================================================*/
void SearchCaseComponent(SENTENCE_DATA *s, CF_PRED_MGR *cpm_ptr, 
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
		bp->cpm_ptr->elem_b_ptr[num] != cpm_ptr->pred_b_ptr && 	/* �����Ǥ����Ѹ��ΤȤ��Ϥ��� */
		CheckTargetNoun(bp->cpm_ptr->elem_b_ptr[num])) {
		/* �����Ǥγʤΰ��� (�ʤˤ�ꤱ��) */
		if (cf_ptr->pp[n][0] == bp->cpm_ptr->cmm[0].cf_ptr->pp[i][0]) {
		    EllipsisDetectForVerbSubcontract(s, s, cpm_ptr, bp->cpm_ptr->elem_b_ptr[num], 
						     cf_ptr, n, RANK3);
		}
		/* �ʤ��԰��� */
		else {
		    EllipsisDetectForVerbSubcontract(s, s, cpm_ptr, bp->cpm_ptr->elem_b_ptr[num], 
						     cf_ptr, n, RANK2);
		}
	    }
	}
    }
}

/*==================================================================*/
	 int CheckCaseComponent(CF_PRED_MGR *cpm_ptr, int n)
/*==================================================================*/
{
    int i;

    for (i = 0; i < cpm_ptr->cf.element_num; i++) {
	if (cpm_ptr->elem_b_ptr[i]->num == n) {
	    if (cpm_ptr->cmm[0].result_lists_d[0].flag[i] >= 0) {
		return TRUE;
	    }
	    else {
		return FALSE;
	    }
	}
    }
    return FALSE;
}

/*==================================================================*/
void EllipsisDetectForVerb(SENTENCE_DATA *sp, CF_PRED_MGR *cpm_ptr, CASE_FRAME *cf_ptr, int n)
/*==================================================================*/
{
    /* �Ѹ��Ȥ��ξ�ά�ʤ�Ϳ������ */

    /* cf_ptr = cpm_ptr->cmm[0].cf_ptr �Ǥ��� */
    /* �Ѹ� cpm_ptr �� cf_ptr->pp[n][0] �ʤ���ά����Ƥ���
       cf_ptr->ex[n] �˻��Ƥ���ʸ���õ�� */

    int i, current = 1, bend;
    char feature_buffer[DATA_LEN], etc_buffer[DATA_LEN];
    SENTENCE_DATA *s, *cs;

    maxscore = 0;
    cs = sentence_data + sp->Sen_num - 1;
    memset(Bcheck, 0, BNST_MAX);

    /* ��ά����Ƥ���ʤ�ޡ��� */
    sprintf(feature_buffer, "C��ά-%s", 
	    pp_code_to_kstr(cf_ptr->pp[n][0]));
    assign_cfeature(&(cpm_ptr->pred_b_ptr->f), feature_buffer);

    /* �Ƥ�ߤ� (PARA �ʤ� child �Ѹ�) */
    if (cpm_ptr->pred_b_ptr->parent) {
	/* �Ƥ� PARA */
	if (cpm_ptr->pred_b_ptr->parent->para_top_p) {
	    /* ��ʬ��������Ѹ� */
	    for (i = 0; cpm_ptr->pred_b_ptr->parent->child[i]; i++) {
		/* PARA �λҶ��ǡ���ʬ��ʳ��������Ѹ� */
		if (cpm_ptr->pred_b_ptr->parent->child[i] != cpm_ptr->pred_b_ptr &&
		    cpm_ptr->pred_b_ptr->parent->child[i]->para_type == PARA_NORMAL) {
		    SearchCaseComponent(cs, cpm_ptr, cpm_ptr->pred_b_ptr->parent->child[i], 
					cf_ptr, n);
		}
	    }

	    /* Ϣ�ѤǷ�����Ѹ� (����ΤȤ�) */
	    if (cpm_ptr->pred_b_ptr->parent->parent && 
		check_feature(cpm_ptr->pred_b_ptr->f, "��:Ϣ��")) {
		SearchCaseComponent(cs, cpm_ptr, cpm_ptr->pred_b_ptr->parent->parent, cf_ptr, n);
	    }
	}
	/* �Ȥꤢ������Ϣ�ѤǷ���ҤȤľ�ο��Ѹ��Τ� */
	else if (check_feature(cpm_ptr->pred_b_ptr->f, "��:Ϣ��")) {
	    SearchCaseComponent(cs, cpm_ptr, cpm_ptr->pred_b_ptr->parent, cf_ptr, n);
	}
    }

    /* �Ҷ� (�Ѹ�) �򸫤� */
    for (i = 0; cpm_ptr->pred_b_ptr->child[i]; i++) {
	if (check_feature(cpm_ptr->pred_b_ptr->child[i]->f, "�Ѹ�")) {
	    SearchCaseComponent(cs, cpm_ptr, cpm_ptr->pred_b_ptr->child[i], cf_ptr, n);
	}
    }

    /* ����ʸ���θ���õ�� (�����Ѹ��γ����ǤˤʤäƤ����ΰʳ�) */
    for (s = cs; s >= sentence_data; s--) {
	bend = s->Bnst_num;

	for (i = bend-1; i >= 0; i--) {
	    if (current && 
		(Bcheck[i] || 
		 (s->bnst_data+i)->dpnd_head == cpm_ptr->pred_b_ptr->num || /* �Ѹ���ľ�ܷ���ʤ� */
		 i == cpm_ptr->pred_b_ptr->num || /* ��ʬ���Ȥ���ʤ� */
		 CheckCaseComponent(cpm_ptr, i))) { /* ���Ѹ�������ʸ�������ǤȤ��Ƥ⤿�ʤ� */
		continue;
	    }

	    /* ��ά���ǤȤʤ뤿��ΤȤꤢ�����ξ�� */
	    if (!CheckPureNoun(s->bnst_data+i)) {
		continue;
	    }

	    EllipsisDetectForVerbSubcontract(s, cs, cpm_ptr, s->bnst_data+i, cf_ptr, n, RANK1);
	}
	if (current)
	    current = 0;
    }

    /* �ڼ��ΰ��̡�
       1. �Ѹ������Ȥǥ˳� (��Ȥϥ���) �� <����> ��Ȥ�Ȥ�
       2. �֡�����(��)�פǥ��ʤ� <����> ��Ȥ�Ȥ� 
       3. ���� V ���� N (���δط�), ����̾��, ����̾��Ͻ��� */
    if ((check_feature(cpm_ptr->pred_b_ptr->f, "ID:���ʤ����") && 
	 cf_ptr->pp[n][0] == pp_kstr_to_code("��") && 
	 cf_match_element(cf_ptr->sm[n], "����", SM_CODE_SIZE)) || 
	(cf_ptr->pp[n][0] == pp_kstr_to_code("��") && 
	cf_match_element(cf_ptr->sm[n], "����", SM_CODE_SIZE) && 
	(check_feature(cpm_ptr->pred_b_ptr->f, "�����") || 
	 check_feature(cpm_ptr->pred_b_ptr->f, "������") || 
	 check_feature(cpm_ptr->pred_b_ptr->f, "����̾��ʲ���"))) || 
	(cpm_ptr->pred_b_ptr->parent && 
	 (check_feature(cpm_ptr->pred_b_ptr->parent->f, "���δط�") || 
	  check_feature(cpm_ptr->pred_b_ptr->parent->f, "���δط�Ƚ��")) && 
	 !check_feature(cpm_ptr->pred_b_ptr->parent->f, "����̾��") && 
	 !check_feature(cpm_ptr->pred_b_ptr->parent->f, "����̾��") && 
	 check_feature(cpm_ptr->pred_b_ptr->f, "��:Ϣ��") && 
	 cf_ptr->pp[n][0] == pp_kstr_to_code("��"))) {
	sprintf(feature_buffer, "C��;�ڼ��ΰ��̡�;%s:1", 
		pp_code_to_kstr(cf_ptr->pp[n][0]));
	assign_cfeature(&(cpm_ptr->pred_b_ptr->f), feature_buffer);
	sprintf(feature_buffer, "��ά�����ʤ�-%s", 
		pp_code_to_kstr(cf_ptr->pp[n][0]));
	assign_cfeature(&(cpm_ptr->pred_b_ptr->f), feature_buffer);
    }
    /* ���ξ��Ͼ�ά���Ǥ�õ������Ͽ���ʤ� 
       (�������Ǥϥǡ����򸫤뤿�ᡢ�������ά���Ϥ�ԤäƤ���) 
       1. �ǳ�
       2. �ȳ�
       3. ���� */
    else if (cf_ptr->pp[n][0] == pp_kstr_to_code("��") ||  
	     cf_ptr->pp[n][0] == pp_kstr_to_code("��") || 
	     cf_ptr->pp[n][0] == pp_kstr_to_code("���")) {
	sprintf(feature_buffer, "��ά�����ʤ�-%s", 
		pp_code_to_kstr(cf_ptr->pp[n][0]));
	assign_cfeature(&(cpm_ptr->pred_b_ptr->f), feature_buffer);
    }
    else if (maxscore > 0) {
	if (cs == maxs) {
	    strcpy(etc_buffer, "Ʊ��ʸ");
	}
	else if (cs-maxs > 0) {
	    sprintf(etc_buffer, "%dʸ��", cs-maxs);
	}

	/* ���ꤷ����ά�ط� */
	sprintf(feature_buffer, "C��;��%s��;%s:%.3f:%s(%s):%dʸ��", 
		(maxs->bnst_data+maxi)->Jiritu_Go, 
		pp_code_to_kstr(cf_ptr->pp[n][0]), 
		maxscore, maxs->KNPSID ? maxs->KNPSID+5 : "?", 
		etc_buffer, maxi);
	assign_cfeature(&(cpm_ptr->pred_b_ptr->f), feature_buffer);

	/* ��󥯤��줿���Ȥ�Ͽ���� */
	RegisterAnaphor((maxs->bnst_data+maxi)->Jiritu_Go);
	/* RegisterPredicate(L_Jiritu_M(cpm_ptr->pred_b_ptr)->Goi, cf_ptr->pp[n][0], 
	   (maxs->bnst_data+maxi)->Jiritu_Go); */
    }
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
	      void discourse_analysis(SENTENCE_DATA *sp)
/*==================================================================*/
{
    int i, j, num, toflag;
    CF_PRED_MGR *cpm_ptr;
    CF_MATCH_MGR *cmm_ptr;
    CASE_FRAME *cf_ptr;

    copy_sentence(sp);
    clear_cf();

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
	   7. ���ơ��Ѹ��� */
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

	for (i = 0; i < cf_ptr->element_num; i++) {
	    num = cmm_ptr->result_lists_p[0].flag[i];
	    /* �ʲ��γʤξ�硢��ά���Ǥ�ǧ�ꤷ�ʤ�
	       ���ֳ�, ������, ̵��, ʣ�缭
	    */
	    if (num == UNASSIGNED && 
		!(toflag && str_eq(pp_code_to_kstr(cmm_ptr->cf_ptr->pp[i][0]), "��")) && 
		!(cmm_ptr->cf_ptr->pp[i][0] > 8 && cmm_ptr->cf_ptr->pp[i][0] < 38) && 
		!str_eq((char *)pp_code_to_kstr(cmm_ptr->cf_ptr->pp[i][0]), "����") && 
		!str_eq((char *)pp_code_to_kstr(cmm_ptr->cf_ptr->pp[i][0]), "��") && 
		!str_eq((char *)pp_code_to_kstr(cmm_ptr->cf_ptr->pp[i][0]), "����")) {
		EllipsisDetectForVerb(sp, cpm_ptr, cmm_ptr->cf_ptr, i);
	    }
	}
    }

    /* ���θ�������å�
    for (i = sp->Bnst_num-1; i >= 0; i--) {
	if (CheckPureNoun(sp->bnst_data+i)) {
	    EllipsisDetectForNoun(sentence_data+sp->Sen_num-1, sp->bnst_data+i);
	}
    }
    */
}

/*====================================================================
                               END
====================================================================*/
