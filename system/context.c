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

/*==================================================================*/
      void copy_cf_with_alloc(CASE_FRAME *dst, CASE_FRAME *src)
/*==================================================================*/
{
    int i, j;

    dst->element_num = src->element_num;
    for (i = 0; i < src->element_num; i++) {
	dst->oblig[i] = src->oblig[i];
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
	free(s->Best_mgr);
	clear_mgr_cf(s);
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
}

/*==================================================================*/
		 void InitSentence(SENTENCE_DATA *s)
/*==================================================================*/
{
    int i, j;

    s->mrph_data = (MRPH_DATA *)malloc_data(sizeof(MRPH_DATA)*MRPH_MAX);
    s->bnst_data = (BNST_DATA *)malloc_data(sizeof(BNST_DATA)*BNST_MAX);
    s->para_data = (PARA_DATA *)malloc_data(sizeof(PARA_DATA)*PARA_MAX);
    s->para_manager = (PARA_MANAGER *)malloc_data(sizeof(PARA_MANAGER)*PARA_MAX);
    s->Best_mgr = (TOTAL_MGR *)malloc_data(sizeof(TOTAL_MGR));
    s->Sen_num = 0;
    s->Mrph_num = 0;
    s->Bnst_num = 0;
    s->New_Bnst_num = 0;
    s->KNPSID = NULL;
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

    int i, j, k, num, cfnum = 0;
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
    float score = -1, tempscore;

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

    /* �ɤ��餫������Υ����ɤ��ʤ��Ȥ� */
    if (!(exd && exp && *exd && *exp)) {
	return 0;
    }

    /* ����ޥå������������ */
    for (j = 0; exp[j]; j+=step) {
	for (i = 0; exd[i]; i+=step) {
	    if (Thesaurus == USE_BGH) {
		tempscore = _ex_match_score(exp+j, exd+i);
		tempscore /= 7;
	    }
	    else if (Thesaurus == USE_NTT) {
		tempscore = ntt_code_match(exp+j, exd+i);
	    }
	    if (tempscore > score) {
		score = tempscore;
	    }
	}
    }

    if (score > 0) {
	return score;
    }
    return 0;
}

/*==================================================================*/
     float CalcSimilarityForNoun(BNST_DATA *dat, BNST_DATA *pat)
/*==================================================================*/
{
    char *exd, *exp;
    int i, j, step;
    float score = -1, tempscore;

    if (Thesaurus == USE_BGH) {
	exd = dat->BGH_code;
	exp = pat->BGH_code;
	step = BGH_CODE_SIZE;
    }
    else if (Thesaurus == USE_NTT) {
	exd = dat->SM_code;
	exp = pat->SM_code;
	step = SM_CODE_SIZE;
    }

    /* �ɤ��餫������Υ����ɤ��ʤ��Ȥ� */
    if (!(exd && exp && *exd && *exp)) {
	return 0;
    }

    /* ����ޥå������������ */
    for (j = 0; exp[j]; j+=step) {
	for (i = 0; exd[i]; i+=step) {
	    if (Thesaurus == USE_BGH) {
		tempscore = _ex_match_score(exp+j, exd+i);
		tempscore /= 7;
	    }
	    else if (Thesaurus == USE_NTT) {
		tempscore = ntt_code_match(exp+j, exd+i);
	    }
	    if (tempscore > score) {
		score = tempscore;
	    }
	}
    }

    if (score > 0) {
	return score;
    }
    return 0;
}

/*==================================================================*/
		   int CheckPureNoun(BNST_DATA *bp)
/*==================================================================*/
{
    if (check_feature(bp->f, "�θ�") && 
	!check_feature(bp->f, "����̾��") && 
	!check_feature(bp->f, "����") && 
	!check_feature(bp->f, "����")) {
	return TRUE;
    }
    return FALSE;
}

/*==================================================================*/
void EllipsisDetectForVerbSubcontract(SENTENCE_DATA *s, CF_PRED_MGR *cpm_ptr, 
				      BNST_DATA *bp, CASE_FRAME *cf_ptr, int n)
/*==================================================================*/
{
    float score;
    char feature_buffer[DATA_LEN];

    /* ��ά���ǤȤʤ뤿��ΤȤꤢ�����ξ�� */
    if (!CheckPureNoun(bp)) {
	return;
    }

    score = CalcSimilarityForVerb(bp, cf_ptr, n);
    if (score > maxscore) {
	maxscore = score;
	maxs = s;
	maxi = bp->num;
    }
    if (score > 0) {
	/* ��ά���� */
	sprintf(feature_buffer, "C%s;%s:%.3f", bp->Jiritu_Go, 
		pp_code_to_kstr(cf_ptr->pp[n][0]), 
		score);
	assign_cfeature(&(cpm_ptr->pred_b_ptr->f), feature_buffer);
    }

    Bcheck[bp->num] = 1;
}

/*==================================================================*/
void SearchCaseComponent(SENTENCE_DATA *s, CF_PRED_MGR *cpm_ptr, 
			 BNST_DATA *bp, CASE_FRAME *cf_ptr, int n)
/*==================================================================*/
{
    int i, num;

    /* ¾���Ѹ� (���Ѹ��ʤ�) �γ����Ǥ�����å� */
    if (bp->cpm_ptr && bp->cpm_ptr->cmm[0].score != -2) {
	for (i = 0; i < bp->cpm_ptr->cmm[0].cf_ptr->element_num; i++) {
	    num = bp->cpm_ptr->cmm[0].result_lists_p[0].flag[i];
	    if (num != UNASSIGNED) {
		/* �����Ǥγʤΰ��� (���ʤΤȤ��Ϥ������⤷��ʤ�) */
		/* cf_ptr->pp[n][0] == cmm_ptr->cf_ptr->pp[i][0] */
		EllipsisDetectForVerbSubcontract(s, cpm_ptr, bp->cpm_ptr->elem_b_ptr[num], cf_ptr, n);
	    }
	}
    }
}

/*==================================================================*/
void EllipsisDetectForVerb(SENTENCE_DATA *sp, CF_PRED_MGR *cpm_ptr, CASE_FRAME *cf_ptr, int n)
/*==================================================================*/
{
    /* �Ѹ��Ȥ��ξ�ά�ʤ�Ϳ������ */

    /* cf_ptr = cpm_ptr->cmm[0].cf_ptr �Ǥ��� */
    /* �Ѹ� cpm_ptr �� cf_ptr->pp[n][0] �ʤ���ά����Ƥ���
       cf_ptr->ex[n] �˻��Ƥ���ʸ���õ�� */

    int i, current = 1, bend, num;
    char feature_buffer[DATA_LEN];
    SENTENCE_DATA *s;
    BNST_DATA *bp;

    maxscore = 0;
    s = sentence_data + sp->Sen_num - 1;
    memset(Bcheck, 0, BNST_MAX);

    /* ��ά����Ƥ���ʤ�ޡ��� */
    sprintf(feature_buffer, "C��ά��ǽ��-%s", 
	    pp_code_to_kstr(cf_ptr->pp[n][0]));
    assign_cfeature(&(cpm_ptr->pred_b_ptr->f), feature_buffer);

    /* �Ƥ�ߤ� (PARA �ʤ� child �Ѹ�) */
    if (cpm_ptr->pred_b_ptr->parent) {
	/* �Ƥ� PARA */
	if (cpm_ptr->pred_b_ptr->parent->para_top_p) {
	    for (i = 0; cpm_ptr->pred_b_ptr->parent->child[i]; i++) {
		/* PARA �λҶ��ǡ���ʬ��ʳ��������Ѹ� */
		if (cpm_ptr->pred_b_ptr->parent->child[i] != cpm_ptr->pred_b_ptr &&
		    cpm_ptr->pred_b_ptr->parent->child[i]->para_type == PARA_NORMAL) {
		    SearchCaseComponent(s, cpm_ptr, cpm_ptr->pred_b_ptr->parent->child[i], 
					cf_ptr, n);
		}
	    }
	}
	/* �Ȥꤢ�������ҤȤľ���Ѹ��Τ� */
	else {
	    bp = cpm_ptr->pred_b_ptr->parent;
	    SearchCaseComponent(s, cpm_ptr, bp, cf_ptr, n);
	}
    }

    /* ����ʸ���θ���õ�� (�����Ѹ��γ����ǤˤʤäƤ����ΰʳ�) */
    for (; s >= sentence_data; s--) {
	if (current) {
	    bend = cpm_ptr->pred_b_ptr->num;
	}
	else {
	    bend = s->Bnst_num;
	}

	for (i = bend; i >= 0; i--) {
	    if (Bcheck[i] || 
		(current && (s->bnst_data+i)->dpnd_head == cpm_ptr->pred_b_ptr->num)) {
		continue;
	    }
	    EllipsisDetectForVerbSubcontract(s, cpm_ptr, s->bnst_data+i, cf_ptr, n);
	}
	if (current)
	    current = 0;
    }

    if (maxscore > 0) {
	/* ���ꤷ����ά�ط� */
	sprintf(feature_buffer, "C��%s��;%s:%.3f", (maxs->bnst_data+maxi)->Jiritu_Go, 
		pp_code_to_kstr(cf_ptr->pp[n][0]), 
		maxscore);
	assign_cfeature(&(cpm_ptr->pred_b_ptr->f), feature_buffer);
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
    char feature_buffer[DATA_LEN];

    /* ̾������ʸ����� */
    def = GetDefinitionFromBunsetsu(bp);
    if (!def) {
	return;
    }

    sbuf = (SENTENCE_DATA *)malloc_data(sizeof(SENTENCE_DATA)*ssize);

    for (scount = 0; *(def+scount); scount++) {
	fprintf(stderr, "���ʸ[%s] %d: %s\n", bp->Jiritu_Go, scount, *(def+scount));

	if (scount >= ssize) {
	    sbuf = (SENTENCE_DATA *)realloc_data(sbuf, sizeof(SENTENCE_DATA)*(ssize <<= 1));
	}
	sp = sbuf+scount;

	/* ���ʸ����� */
	InitSentence(sp);
	ParseSentence(sp, *(def+scount));

	/* ����ʸ (ʸ��) ��̾��ǡ����ʸ�˴ޤޤ��̾��˻��Ƥ����Τ˥�󥯤�Ϥ� */
	for (i = sp->Bnst_num-1; i >= 0; i--) {
	    if (CheckPureNoun(sp->bnst_data+i)) {
		/* sp->bnst_data+i: ���ʸ����θ� */
		maxscore = 0;
		current = 1;
#ifdef DEBUG
		fprintf(Outfp, "���ʸ -- %s\n", (sp->bnst_data+i)->Jiritu_Go);
#endif
		/* ����ʸ�� */
		for (s = cs; s >= sentence_data; s--) {
		    if (current) {
			bend = bp->num;
		    }
		    else {
			bend = s->Bnst_num;
		    }

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
			    sprintf(feature_buffer, "C%s;%s:%.3f", (s->bnst_data+j)->Jiritu_Go, 
				    (sp->bnst_data+i)->Jiritu_Go, 
				    score);
			    assign_cfeature(&(bp->f), feature_buffer);
#ifdef DEBUG
			    fprintf(Outfp, "\t%.3f %s\n", score, (s->bnst_data+j)->Jiritu_Go);
#endif
			}
		    }
		    if (current)
			current = 0;
		}
		if (maxscore > 0) {
		    /* ���ꤷ����ά�ط� */
		    sprintf(feature_buffer, "C��%s��;%s:%.3f", (maxs->bnst_data+maxi)->Jiritu_Go, 
			    (sp->bnst_data+i)->Jiritu_Go, 
			    maxscore);
		    assign_cfeature(&(bp->f), feature_buffer);
#ifdef DEBUG
		    fprintf(Outfp, "\t�� %s\n", (maxs->bnst_data+maxi)->Jiritu_Go);
#endif
		}
#ifdef DEBUG
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
    int i, j, num;
    CF_PRED_MGR *cpm_ptr;
    CF_MATCH_MGR *cmm_ptr;
    CASE_FRAME *cf_ptr;
    BNST_DATA *pred_b_ptr;

    copy_sentence(sp);
    clear_cf();

    /* ���Ѹ�������å� */
    for (j = 0; j < sp->Best_mgr->pred_num; j++) {
	cpm_ptr = &(sp->Best_mgr->cpm[j]);

	/* �ʥե졼�ब�ʤ���� */
	if (cpm_ptr->result_num == 0 || 
	    cpm_ptr->cmm[0].cf_ptr->ipal_address == -1 || 
	    cpm_ptr->cmm[0].score == -2) {
	    continue;
	}

	cmm_ptr = &(cpm_ptr->cmm[0]);
	cf_ptr = cmm_ptr->cf_ptr;
	pred_b_ptr = cpm_ptr->pred_b_ptr;

	for (i = 0; i < cf_ptr->element_num; i++) {
	    num = cmm_ptr->result_lists_p[0].flag[i];
	    /* �Ȥꤢ������ά���Ǥ�ǧ�ꤹ��������� 
	       1. ���Ѹ��ǤϤʤ�
	       2. ���ֳʤǤϤʤ� */
	    if (num == UNASSIGNED && 
		!check_feature(pred_b_ptr->f, "���Ѹ�") && 
		!str_eq((char *)pp_code_to_kstr(cmm_ptr->cf_ptr->pp[i][0]), "����")) {
		EllipsisDetectForVerb(sp, cpm_ptr, cmm_ptr->cf_ptr, i);
	    }
	}
    }

    /* ���θ�������å� */
    for (i = sp->Bnst_num-1; i >= 0; i--) {
	if (CheckPureNoun(sp->bnst_data+i)) {
	    EllipsisDetectForNoun(sentence_data+sp->Sen_num-1, sp->bnst_data+i);
	}
    }
}

/*====================================================================
                               END
====================================================================*/
