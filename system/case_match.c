/*====================================================================

			�ʹ�¤����: �ޥå���

                                               S.Kurohashi 93. 5.31

    $Id$
====================================================================*/
#include "knp.h"

double 	Current_max_score;	/* ���� */
double 	Current_pure_score[MAX_MATCH_MAX];	/* ������������������ */
double	Current_sufficiency;	/* ��ޤꤰ���� */
int 	Current_max_m_e;	/* ���ǿ� */
int 	Current_max_m_p;	/* ���Ǥΰ��� */
int 	Current_max_c_e;	/* �򺹿� */

int 	Current_max_num;
LIST 	Current_max_list1[MAX_MATCH_MAX];
LIST 	Current_max_list2[MAX_MATCH_MAX];


int 	SM_match_score[] = {0, 10, 10, 10, 10, 10, 10, 10, 10, 
			    10, 10, 10, 10};
  				/*{0, 5, 8, 10, 12};*/	/* �ӣ��б������� */
int     SM_match_unknown = 10;			 	/* �ǡ���̤��     */

/* int 	EX_match_score[] = {0, 0, 5, 7, 8, 9, 10, 11}; */
/* int 	EX_match_score[] = {0, 0, 0, 1, 3, 5, 10, 11}; */
int 	EX_match_score[]  = {0, 0, 0, 1, 3, 5, 8, 11};
int 	EX_match_score2[] = {0, 0, 0, 1, 2, 4, 7, 11};
							/* �����б������� */
int     EX_match_unknown = 6;				/* �ǡ���̤��     */
int     EX_match_sentence = 10;				/* ������ -- ʸ   */
int     EX_match_tim = 0;				/* ������ -- ����:���ֳ� */
int     EX_match_tim2 = 12;				/* ������ -- ����:����¾�γ� */
int     EX_match_tim3 = 8;				/* ������ -- ����:������� */
int     EX_match_qua = 9; /* 10; */			/* ������ -- ���� */
int	EX_match_exact = 12;
int	EX_match_subject = 8;
int	EX_match_modification = 0;
int	EX_match_demonstrative = 0;

int	SOTO_THRESHOLD = 0; /* if -probcase; otherwise DEFAULT_SOTO_THRESHOLD */
/*int	NOUN_THRESHOLD = 5; /* ���Ϥ��ؼ��ط������� */
int	NOUN_THRESHOLD = 5; /* ���Ϥ��ؼ��ط������� */
int	CASE_ASSIGN_THRESHOLD = 0;

/*==================================================================*/
	    void print_assign(LIST *list, CASE_FRAME *cf)
/*==================================================================*/
{
    /* �б��ꥹ�Ȥ�ɽ�� */

    int i;
    for (i = 0; i < cf->element_num; i++) {
	if (list->flag[i] == NIL_ASSIGNED)
	  fprintf(Outfp, "  X");
	else
	  fprintf(Outfp, "%3d", list->flag[i]+1);
    }
    fprintf(Outfp, "\n");
}

/*==================================================================*/
	     int comp_sm(char *cpp, char *cpd, int start)
/*==================================================================*/
{
    /* start ��������å�����
       ���̤� 1 
       �ʻ줴�ȥ����å�����Ȥ��� 0 */

    int i;

    for (i = start; i < SM_CODE_SIZE; i++) {
	if (cpp[i] == '*')
	    return i;
	else if (cpd[i] == '*')
	    return 0;
	else if (cpp[i] != cpd[i])
	    return 0;
    }
    return SM_CODE_SIZE;
}

/*==================================================================*/
	 int _sm_match_score(char *cpp, char *cpd, int flag)
/*==================================================================*/
{
    /*
      NTT�ΰ�̣�Ǥΰ���ܤ��ʻ����
      �ʥե졼�� <-----> �ǡ���
       x(��ʸ)   <-----> x����OK
       1(̾��)   <-----> x�ʳ�OK
                         (̾��ʳ��Τ�Τ�get_sm�λ������ӽ� 99/01/13)
    */

    /* 
       flag == SM_EXPAND_NE    : ��ͭ̾���̣°�������̾���̣°�����Ѵ�����
       flag == SM_NO_EXPAND_NE : ��ͭ̾���̣°�������̾���̣°�����Ѵ����ʤ�
       flag == SM_CHECK_FULL   : �����ɤΰ�ʸ���ܤ�������å�����
     */

    int current_score, score = 0;
    char *cp;

    if (flag == SM_CHECK_FULL)
	return comp_sm(cpp, cpd, 0);

    if (cpp[0] == 'x') {
	if (cpd[0] == 'x')
	    return SM_CODE_SIZE;
	else
	    return 0;
    } else {
	if (cpd[0] == 'x')
	    return 0;
    }

    /* ��̣�ޡ����Υޥå����٤η׻�

       ���ѥ��������* --- �ޥå�
       ���ǡ��������* --- �ޥå����ʤ�
       ���Ǹ�ޤǰ��� --- �ޥå�

         �ޥå� : �ޥå����볬�ؤο������֤�
	 �ޥå����ʤ��Ȥ� : 0���֤�
    */

    /* �ǡ�������ͭ̾��ΤȤ� */
    if (cpd[0] == '2') {
	if (flag == SM_EXPAND_NE && cpp[0] != '2') {
	    if (SMP2SMGExist == FALSE) {
		fprintf(stderr, ";;; Cannot open smp2smg table!\n");
		return 0;
	    }
	    else {
		char *start;
		start = _smp2smg(cpd);
		if (start == NULL) {
		    return score;
		}
		for (cp = start; *cp; cp+=SM_CODE_SIZE) {
		    if (*cp == '/') {
			cp++;
		    }
		    else if (cp != start) {
			fprintf(stderr, ";;; Invalid delimiter! <%c> (%s)\n", 
				*cp, start);
		    }

		    /* �����ѥե饰�������̣���Ѵ��ϹԤ�ʤ� */
		    if (!strncmp(cp+SM_CODE_SIZE, " side-effect", 12)) {
			cp += 12; /* " side-effect" ��ʬ�ʤ�� */
			continue;
		    }

		    current_score = comp_sm(cpp, cp, 1);
		    if (current_score > score) {
			score = current_score;
		    }
		}
		free(start);
		return score;
	    }
	}
	else if (flag == SM_NO_EXPAND_NE && cpp[0] == '2')
	    return comp_sm(cpp, cpd, 1);
	else
	    return 0;
    }
    /* ξ���Ȥ����̾��ΤȤ� */
    else if (cpp[0] != '2')
	return comp_sm(cpp, cpd, 1);
    else
	return 0;
}

/*==================================================================*/
	   int sms_match(char *cpp, char *cpd, int expand)
/*==================================================================*/
{
    int i;

    if (Thesaurus == USE_BGH) {
	return bgh_match_check(cpp, cpd);
    }
    else if (Thesaurus == USE_NTT) {
	return sm_match_check(cpp, cpd, expand);
    }

    return 0;
}

/*==================================================================*/
     int _cf_match_element(char *d, char *p, int start, int len)
/*==================================================================*/
{
    int i, j;
    char *code;

    if (Thesaurus == USE_BGH) {
	for (i = 0; *(d+i); i += BGH_CODE_SIZE) {
	    for (j = 0; *(p+j); j += BGH_CODE_SIZE) {
		if (!strncmp(d+i+start, p+j+start, len)) {
		    return TRUE;
		}
	    }
	}
    }
    else if (Thesaurus == USE_NTT) {
	for (i = 0; *(d+i); i += SM_CODE_SIZE) {
	    /* ��ͭ̾���η� */
	    if (*(d+i) == '2') {
		/* �����ηϤ˥ޥåԥ� 
		   �� side-effect ��̵�뤹�� */
		code = smp2smg(d+i, TRUE);
		if (code == NULL) {
		    continue;
		}
		for (j = 0; *(code+j); j += SM_CODE_SIZE) {
		    if (!strncmp(code+j+start, p+start, len)) {
			free(code);
			return TRUE;
		    }
		}
		free(code);
	    }
	    else {
		if (!strncmp(d+i+start, p+start, len)) {
		    return TRUE;
		}
	    }
	}
    }
    return FALSE;
}

/*==================================================================*/
	int cf_match_element(char *d, char *target, int flag)
/*==================================================================*/
{
    char *code;
    int size;

    /* flag == TRUE  : ���ΰ�̣�Ǥ� exact match
       flag == FALSE : ���ΰ�̣�ǰʲ��ˤ���� match */

    if (d == NULL) {
	return FALSE;
    }

    if (Thesaurus == USE_BGH) {
	size = BGH_CODE_SIZE;
    }
    else if (Thesaurus == USE_NTT) {
	size = SM_CODE_SIZE;
    }

    code = sm2code(target);

    if (flag == TRUE) {
	return _cf_match_element(d, code, 0, size);
    }
    else {
	/* �� �����ɤ�2ʸ���ʾ夢��ɬ�פ����� */
	return _cf_match_element(d, code, 1, code_depth(code, size));
    }
}

/*==================================================================*/
 int cf_match_both_element(char *d, char *p, char *target, int flag)
/*==================================================================*/
{
    int len;
    char *code;

    /* ξ���� target ��¸�ߤ��뤫�����å� */

    /* flag == TRUE  : ���ΰ�̣�Ǥ� exact match
       flag == FALSE : ���ΰ�̣�ǰʲ��ˤ���� match */

    if (p == NULL) {
	return FALSE;
    }

    code = sm2code(target);

    if (Thesaurus == USE_BGH) {
	if (_cf_match_element(d, code, 0, BGH_CODE_SIZE) == TRUE && 
	    _cf_match_element(p, code, 0, BGH_CODE_SIZE) == TRUE) {
	    return TRUE;
	}
	else {
	    return FALSE;
	}
    }
    else if (Thesaurus == USE_NTT) {
	if (flag == TRUE) {
	    if (_cf_match_element(d, code, 0, SM_CODE_SIZE) == TRUE && 
		_cf_match_element(p, code, 0, SM_CODE_SIZE) == TRUE) {
		return TRUE;
	    }
	    else {
		return FALSE;
	    }
	}
	else {
	    /* �� �����ɤ� 2 ʸ���ʾ夢��ɬ�פ�����
	       1ʸ����(�ʻ�)��̵�뤷�ơ�Ϳ����줿�����ɰʲ��ˤ��뤫�ɤ��������å� */
	    len = code_depth(code, SM_CODE_SIZE);
	    if (_cf_match_element(d, code, 1, len) == TRUE && 
		_cf_match_element(p, code, 1, len) == TRUE) {
		return TRUE;
	    }
	    else {
		return FALSE;
	    }
	}
    }
}

/*==================================================================*/
	      int cf_match_sm(int as1, CASE_FRAME *cfd,
			      int as2, CASE_FRAME *cfp, int *pos)
/*==================================================================*/
{
    if (cf_match_both_element(cfd->sm[as1], cfp->sm[as2], "����", TRUE) || 
	cf_match_both_element(cfd->sm[as1], cfp->sm[as2], "��ʸ", TRUE) || 
	cf_match_both_element(cfd->sm[as1], cfp->sm[as2], "����", TRUE) || 
	cf_match_both_element(cfd->sm[as1], cfp->sm[as2], "����", TRUE)) {
	*pos = MATCH_SUBJECT;
	return 1;
    }
    return 0;
}

/*==================================================================*/
       int elmnt_match_score_each_sm(int as1, CASE_FRAME *cfd,
				     int as2, CASE_FRAME *cfp, int *pos)
/*==================================================================*/
{
    /* ��̣�� : ������ -- ��ʸ */
    if (cf_match_both_element(cfd->sm[as1], cfp->sm[as2], "��ʸ", TRUE)) {
	return EX_match_sentence;
    }
    /* ��̣�� : ������ -- ���� */
    else if (cf_match_both_element(cfd->sm[as1], cfp->sm[as2], "����", TRUE)) {
	/* �ʥե졼��¦�����ֳʤξ��ϥ��������㤯 */
	if (MatchPP(cfp->pp[as2][0], "����")) {
	    return EX_match_tim;
	}
	/* �ʥե졼��:���ֳʰʳ�, ����¦:�������
	   �ʤ�ۣ��ʤȤ���
	   1. <����>���ֳ� : <����>���ֳ� (score == 0)
	   2. ����������̤γ� : ����������̤γ�
	   3. <����>���̤γ� : <����>���̤γ� (here) */
	else if (cfd->pp[as1][1] != END_M) {
	    return EX_match_tim3;
	}
	else {
	    return EX_match_tim2;
	}
    }
    /* ��̣�� : ������ -- ���� */
    else if (cf_match_both_element(cfd->sm[as1], cfp->sm[as2], "����", TRUE)) {
	return EX_match_qua;
    }
    return -100;
}

/*==================================================================*/
   int cf_match_sm_thesaurus(TAG_DATA *tp, CASE_FRAME *cfp, int n)
/*==================================================================*/
{
    int step, expand, non_subj_flag = 0;
    char *code;

    if (Thesaurus == USE_BGH) {
	step = BGH_CODE_SIZE;
    }
    else if (Thesaurus == USE_NTT) {
	step = SM_CODE_SIZE;
    }

    if (check_feature(tp->f, "�Ը�ͭ����Ÿ���ػ�")) {
	expand = SM_NO_EXPAND_NE;
    }
    else {
	expand = SM_EXPAND_NE;
    }

    if (check_feature(tp->f, "�����")) {
	non_subj_flag = 1;
    }

    if (Thesaurus == USE_BGH) {
	code = tp->BGH_code;
    }
    else {
	code = tp->SM_code;
    }    

    /* ��̣°���Υޥå��� */
    if (cfp->sm[n]) {
	int i, j;

	for (j = 0; cfp->sm[n][j]; j += step) {
	    /* �ʥե졼��-����, ��, �ȿ�
	       ���� <=> <����>, ��̾, �ȿ�̾
	       ��   <=> <��>, ��̾
	       �ȿ� <=> <�ȿ�>, �ȿ�̾
	    �� ��̾�ˤ�<��>, �ȿ�̾�ˤ�<�ȿ�>��rule����Ϳ���� */
	    if (!strncmp(cfp->sm[n] + j, sm2code("����"), step) || 
		!strncmp(cfp->sm[n] + j, sm2code("��"), step) || 
		!strncmp(cfp->sm[n] + j, sm2code("�ȿ�"), step)) {
		if (non_subj_flag == 0 && 
		    !MatchPP(cfp->pp[n][0], "��") && /* ���ΤΥޥå��� (��ʰʳ�) */
		    sms_match(cfp->sm[n] + j, code, expand)) {
		    return 1;
		}
	    }
	    /* �ʥե졼��-ư�� <=> <̾(ž��)>, <����> */
	    else if (Thesaurus == USE_NTT && !strncmp(cfp->sm[n] + j, sm2code("ư��"), SM_CODE_SIZE)) {
		if (sms_match(sm2code("̾(ž��)"), code, SM_CHECK_FULL) || 
		    sms_match(sm2code("����"), code, SM_CHECK_FULL)) {
		    return 1;
		}
	    }
	    /* �ʥե졼��-��� <=> <���> */
	    else if (!strncmp(cfp->sm[n] + j, sm2code("���"), step)) {
		if (sms_match(cfp->sm[n] + j, code, expand)) {
		    return 1;
		}
	    }
	}
    }
    return 0;
}

/*==================================================================*/
  int dat_match_sm(int as1, CASE_FRAME *cfd, TAG_DATA *tp, char *sm)
/*==================================================================*/
{
    int expand;
    char *code;

    if (tp == NULL) {
	tp = cfd->pred_b_ptr->cpm_ptr->elem_b_ptr[as1];
    }

    if (!strcmp(sm, "����")) {
	if (check_feature(tp->f, "�����")) {
	    return 0;
	}

	if (Thesaurus == USE_BGH) {
	    code = tp->BGH_code;
	}
	else {
	    code = tp->SM_code;
	}
    }
    else if (!cfd || as1 < 0) {
	if (Thesaurus == USE_BGH) {
	    code = tp->BGH_code;
	}
	else {
	    code = tp->SM_code;
	}
    }
    else {
	code = cfd->sm[as1];
    }

    if (check_feature(tp->f, "�Ը�ͭ����Ÿ���ػ�")) {
	expand = SM_NO_EXPAND_NE;
    }
    else {
	expand = SM_EXPAND_NE;
    }

    if (sms_match(sm2code(sm), code, expand)) {
	return 1;
    }

    return 0;
}

/*==================================================================*/
int cf_match_exactly(char *word, int word_len, char **ex_list, int ex_num, int *pos)
/*==================================================================*/
{
    int ret_pos;

    if ((ret_pos = check_examples(word, word_len, ex_list, ex_num)) >= 0) {
	*pos = ret_pos;
	return 1;
    }

    return 0;
}

/*==================================================================*/
int cf_match_exactly_for_canonical_rep(char *word, int word_len, char **ex_list, int ex_num, int *pos)
/*==================================================================*/
{
    char *token;

    /* ��������ɽɽ���ξ���?���ڤäƥ����å� */
    token = strtok(word, "?");
    while (token) {
	if (cf_match_exactly(token, strlen(token), ex_list, ex_num, pos)) {
	    return 1;
	}
	token = strtok(NULL, "?");
    }

    return 0;
}

/*==================================================================*/
float _calc_similarity_sm_cf(char *exd, int expand, char *unmatch_word, 
			     CASE_FRAME *cfp, int n, int *pos)
/*==================================================================*/
{
    /* ����ٷ׻�: ��̣�Ƿ� - �ʥե졼��γ�
       unmatch_word: �ޥå����������ʤ�ñ�� */

    if (cfp->sm_specify[n]) { /* ��̣������ */
	return calc_similarity(exd, cfp->sm_specify[n], expand);
    }
    else {
	return calc_sm_words_similarity(exd, cfp->ex_list[n], cfp->ex_num[n], pos, 
					cfp->sm_delete[n], expand, unmatch_word);
    }
}

/*==================================================================*/
float calc_similarity_word_cf(TAG_DATA *tp, CASE_FRAME *cfp, int n, int *pos)
/*==================================================================*/
{
    char *exd, *strp, *cp;
    int expand, strp_malloc_flag = 0, rep_length, exact_matched_flag = 0;
    float ex_score = 0;
    FEATURE *fp;
    MRPH_DATA m;

    if (Thesaurus == USE_BGH) {
	exd = tp->BGH_code;
    }
    else if (Thesaurus == USE_NTT) {
	exd = tp->SM_code;
    }

    if (check_feature(tp->f, "�Ը�ͭ����Ÿ���ػ�")) {
	expand = SM_NO_EXPAND_NE;
    }
    else {
	expand = SM_EXPAND_NE;
    }

    if (OptCaseFlag & OPT_CASE_USE_REP_CF) {
	if ((OptCaseFlag & OPT_CASE_USE_CREP_CF) && /* ������(�缭)��ɽɽ�� */
	    (cp = get_bnst_head_canonical_rep(tp->b_ptr, OptCaseFlag & OPT_CASE_USE_CN_CF))) {
	    strp = strdup(cp);
	    strp_malloc_flag = 1;
	}
	else if ((strp = get_mrph_rep_from_f(tp->head_ptr, FALSE)) == NULL) { /* feature�����ɽɽ�� */
	    strp = make_mrph_rn(tp->head_ptr); /* �ʤ���к�� */
	    strp_malloc_flag = 1;
	}
    }
    else {
	strp = tp->head_ptr->Goi;
    }

    /* exact match */
    if (!check_feature(tp->f, "����̾��")) {
	if (cf_match_exactly_for_canonical_rep(strp, strlen(strp), 
					       cfp->ex_list[n], cfp->ex_num[n], pos)) {
	    exact_matched_flag = 1;
	}
	/* ��������ɽɽ����Ȥ�ʤ���ɽɽ���ξ���ALT��Ĵ�٤� */
	else if ((OptCaseFlag & OPT_CASE_USE_REP_CF) && 
		 !(OptCaseFlag & OPT_CASE_USE_CREP_CF)) {
	    fp = tp->head_ptr->f;
	    while (fp) {
		if (!strncmp(fp->cp, "ALT-", 4)) {
		    if (strp_malloc_flag) {
			free(strp);
			strp_malloc_flag = 0;
		    }
		    sscanf(fp->cp + 4, "%[^-]-%[^-]-%[^-]-%d-%d-%d-%d-%[^\n]", 
			   m.Goi2, m.Yomi, m.Goi, 
			   &m.Hinshi, &m.Bunrui, 
			   &m.Katuyou_Kata, &m.Katuyou_Kei, m.Imi);
		    strp = get_mrph_rep(&m); /* ��ɽɽ�� */
		    rep_length = get_mrph_rep_length(strp);
		    if (rep_length == 0) { /* �ʤ���к�� */
			strp = make_mrph_rn(&m);
			rep_length = strlen(strp);
			strp_malloc_flag = 1;
		    }

		    if (cf_match_exactly(strp, rep_length, 
					 cfp->ex_list[n], cfp->ex_num[n], pos)) {
			exact_matched_flag = 1;
			break;
		    }
		}
		fp = fp->next;
	    }
	}
    }
    if (strp_malloc_flag) {
	free(strp);
    }

    if (exact_matched_flag) {
	return 1.1;
    }
    /* ��̣�Ǥʤ�
       ����ˤ��뤿��� -1 ���֤� */
    else if (!exd[0]) {
	return -1;
    }
    /* ��̣�� match */
    else {
	return _calc_similarity_sm_cf(exd, expand, NULL, cfp, n, pos);
    }
}

/*==================================================================*/
 float calc_similarity_word_cf_with_sm(TAG_DATA *tp, CASE_FRAME *cfp,
				       int n, int *pos)
/*==================================================================*/
{
    float ex_rawscore;

    ex_rawscore = calc_similarity_word_cf(tp, cfp, n, pos);

    /* exact�ޥå� */
    if (ex_rawscore > 1.0) {
	return ex_rawscore;
    }

    /* ���Υޥå� */
    if (cf_match_sm_thesaurus(tp, cfp, n)) {
	*pos = MATCH_SUBJECT;
	return (float)EX_match_subject / 11;
    }

    return ex_rawscore;
}

/*==================================================================*/
	   int elmnt_match_score(int as1, CASE_FRAME *cfd, 
				 int as2, CASE_FRAME *cfp, 
				 int flag, int *pos, double *score, 
				 CF_PRED_MGR *para_cpm_ptr)
/*==================================================================*/
{
    /* ��̣�ޡ����Υޥå����٤η׻� */

    int i, j, k;
    char *exd, *exp;
    int *match_score;

    *score = -100;
    exd = cfd->ex[as1];
    exp = cfp->ex[as2];
    match_score = EX_match_score;

    if (flag == SEMANTIC_MARKER) {
	int tmp_score;

	if (SMExist == FALSE || 
	    cfd->sm[as1][0] == '\0'|| 
	    cfp->sm[as2] == NULL || 
	    cfp->sm[as2][0] == '\0') {
	    *score = SM_match_unknown;
	    return TRUE;
	}

	for (j = 0; cfp->sm[as2][j]; j+=SM_CODE_SIZE) {
	    /* ����Ū�����㤬�񤤤Ƥ����� */
	    if (!strncmp(cfp->sm[as2]+j, sm2code("��"), SM_CODE_SIZE)) {
		tmp_score = (int)calc_similarity(exd, exp, 0);
		if (tmp_score == 1) {
		    *score = 10;
		    return TRUE;
		}
	    }
	    else {
		/* �������¤ˤ��ޥå� (NTT�������饹��������) */
		for (i = 0; cfd->sm[as1][i]; i+=SM_CODE_SIZE) {
		    tmp_score = 
			SM_match_score[_sm_match_score(cfp->sm[as2]+j,
						       cfd->sm[as1]+i, SM_NO_EXPAND_NE)];
		    if (tmp_score > *score) *score = tmp_score;
		}
	    }
	}
	return TRUE;
    }

    else if (flag == EXAMPLE) {
	int ex_score;
	float ex_rawscore;

	/* ��ΨŪ�ʲ��ϤΤȤ� */
	if (OptCaseFlag & OPT_CASE_USE_PROBABILITY) {
	    /* �ޥå���Ĵ�٤�Ȥ� *
	       cf_match_exactly(cfd->pred_b_ptr->cpm_ptr->elem_b_ptr[as1]->head_ptr->Goi, 
	       strlen(cfd->pred_b_ptr->cpm_ptr->elem_b_ptr[as1]->head_ptr->Goi), 
	       cfp->ex_list[as2], cfp->ex_num[as2], pos);
	       cf_match_sm(as1, cfd, as2, cfp, pos);
	    */

	    *score = get_ex_probability_with_para(as1, cfd, as2, cfp) + get_case_probability(as2, cfp, TRUE, para_cpm_ptr) + get_case_function_probability(as1, cfd, as2, cfp, FALSE);
	    return TRUE;
	}

	/* �����ʤΤȤ� */
	if (MatchPP(cfd->pp[as1][0], "����")) {
	    *score = EX_match_modification;
	    return TRUE;
	}

	/* �ؼ���ΤȤ� */
	if (check_feature(cfd->pred_b_ptr->cpm_ptr->elem_b_ptr[as1]->f, "�ؼ���")) {
	    *score = EX_match_demonstrative;
	    return TRUE;
	}

	/* ���Υޥå� -- ���ʤǰ�̣�Ǥʤ��ΤȤ���ͭ̾����Ȼפ� *
	    (cfd->ex[as1][0] == '\0' && 
	     cf_match_element(cfp->sm[as2], "����", TRUE))) {
	*/

	if (cfd->pred_b_ptr->cpm_ptr->cf.type == CF_PRED) {   
	    /* ����Υޥå��� */
	    ex_rawscore = calc_similarity_word_cf_with_sm(cfd->pred_b_ptr->cpm_ptr->elem_b_ptr[as1], cfp, as2, pos);
	}
	else {
	    ex_rawscore = calc_similarity_word_cf(cfd->pred_b_ptr->cpm_ptr->elem_b_ptr[as1], cfp, as2, pos);
	}
	
	if (MatchPP(cfp->pp[as2][0], "���δط�")) {
	    /* ���δط��ΤȤ��������饹��Ȥ�ʤ� */
	    if (ex_rawscore > 1.0) {
		*score = *(match_score + 7);
		return TRUE;
	    }
	    else {
		*score = 0;
		return FALSE;
	    }
	}
	else {
	    /* exact match */
	    if (ex_rawscore > 1.0) {
		*score = EX_match_exact; /* (int)(ex_rawscore * EX_match_score[7]) */
		return TRUE;
	    }
	}

	/* <����>���̥����� */
	if (*pos == MATCH_SUBJECT) {
	    *score = EX_match_subject;
	    return TRUE;
	}
	else {
	    /* <��ʸ>, <����>, <����> */
	    *score = elmnt_match_score_each_sm(as1, cfd, as2, cfp, pos);
	}

	/* ����¦������ΰ�̣°�����ʤ���� */
	if (*exd == '\0' && *cfd->sm[as1] == '\0') {
	    ex_rawscore = 0; /* ex_rawscore == -1 �ΤϤ� */
	    *score = EX_match_unknown;
	}

	/* �ʲ����ѥ��������Ѵ� */
	ex_score = *(match_score + (int)(ex_rawscore * 7));
	/*
	if (Thesaurus == USE_NTT && 
	    sm_check_match_max(exd, exp, 0, sm2code("���"))) { * <���>�Υޥå����㤯 *
	    ex_score = EX_match_score2[(int)(ex_rawscore * 7)];
	}
	*/

	/* �礭�����򤫤��� */
	if (ex_score > *score) {
	    *score = ex_score;
	}

	/* ����, ��̣�ǤΥޥå��������� */
	if (*score > CASE_ASSIGN_THRESHOLD) {
	    return TRUE;
	}
	else {
	    return FALSE;
	}
    }
    return FALSE;
}

/*==================================================================*/
	 int count_pat_element(CASE_FRAME *cfp, LIST *list2)
/*==================================================================*/
{
    int i, pat_element = 0;

    /* ���٤Ƥγʤ�Ǥ�ճʤ��� 0 ���֤��� 0 �ǽ������Ƥ��ޤ� */

    for (i = 0; i < cfp->element_num; i++) {
	if (!(cfp->oblig[i] == FALSE && list2->flag[i] == UNASSIGNED)) {
	    pat_element++;
	}
    }
    return pat_element;
}

/*==================================================================*/
  int count_assigned_adjacent_element(CASE_FRAME *cfp, LIST *list2)
/*==================================================================*/
{
    int i, count = 0;

    /* ������Ƥ�����ľ���ʤο�������� (�ʥե졼��¦) */

    for (i = 0; i < cfp->element_num; i++) {
	if (cfp->adjacent[i] == TRUE && list2->flag[i] != UNASSIGNED) {
	    count++;
	}
    }
    return count;
}

/*==================================================================*/
	 int check_same_case(int dp, int pp, CASE_FRAME *cf)
/*==================================================================*/
{
    int i, p1, p2;

    if (dp < pp) {
	p1 = dp;
	p2 = pp;
    }
    else {
	p1 = pp;
	p2 = dp;
    }

    for (i = 0; cf->samecase[i][0] != END_M; i++) {
	if (cf->samecase[i][0] == p1 && 
	    cf->samecase[i][1] == p2) {
	    return 1;
	}
    }
    return 0;
}

/*==================================================================*/
		int check_case(CASE_FRAME *cf, int c)
/*==================================================================*/
{
    int i, j;

    for (i = 0; i < cf->element_num; i++) {
	for (j = 0; cf->pp[i][j] != END_M; j++) {
	    if (cf->pp[i][j] == c) {
		return i;
	    }
	}
    }
    return -1;
}

/*==================================================================*/
int check_adjacent_assigned(CASE_FRAME *cfd, CASE_FRAME *cfp, LIST *list1)
/*==================================================================*/
{
    int i;
	       
    for (i = 0; i < cfd->element_num; i++) {
	if (cfd->adjacent[i] == TRUE && 
	    list1->flag[i] != NIL_ASSIGNED && 
	    cfp->adjacent[list1->flag[i]] == TRUE) {
	    return TRUE;
	}
    }
    return FALSE;
}

/*==================================================================*/
	    void eval_assign_score(CASE_FRAME *cfd, LIST *list1,
				   CASE_FRAME *cfp, LIST *list2,
				   int score, int closest)
/*==================================================================*/
{
    /* �ե졼��Υޥå����٤η׻�(��������ʬ�����) */

    int i, j;
    int local_m_e = 0; /* �ǡ���¦������ƿ� */
    int local_m_p = 0;
    int local_c_e = 0;
    int pat_element, dat_element = 0;
    int cf_element = 0, lastpp;
    int unassigned_ga = 0;
    float local_score;

    local_score = score;

    /* ���ǿ������Ǥΰ��֡��򺹿� */
    for (i = 0; i < cfd->element_num; i++) {
	if (list1->flag[i] != NIL_ASSIGNED) {
	    local_m_e++;
	    local_m_p += i;
	    for (j = i+1; j < cfd->element_num; j++) {
		if (list1->flag[j] != NIL_ASSIGNED &&
		    list1->flag[j] < list1->flag[i]) {
		    local_c_e--;
		}
	    }
	}
    }

    /* ʸ������ǿ�(Ǥ�դǥޥå����Ƥ��ʤ����ǰʳ�) */
    /* �� ������ʸ���､�����Ǥ�հ��� */
    for (i = 0; i < cfd->element_num; i++) {
	if (!(cfd->oblig[i] == FALSE && list1->flag[i] == NIL_ASSIGNED)) {
	    dat_element++;
	}
    }

    /* �ʥե졼��������ǿ�(Ǥ�դǥޥå����Ƥ��ʤ����ǰʳ�) */
    pat_element = count_pat_element(cfp, list2);

    /* �ʥե졼��������ǿ� */
    for (i = 0; i < cfp->element_num; i++) {
	if (list2->flag[i] != UNASSIGNED) {
	    cf_element++;
	    lastpp = cfp->pp[i][0];
	}
	/* ������ƤΤʤ����ʤ����� */
	else if (MatchPP(cfp->pp[i][0], "��")) {
	    unassigned_ga = 1;
	}
    }


#ifdef CASE_DEBUG
    fprintf(Outfp, "dat %s score=%.3f m_e=%d dat=%d pat=%d ", 
	    cfp->cf_id, local_score, local_m_e, dat_element, pat_element);
    for (i = 0; i < cfd->element_num; i++)
	fprintf(Outfp, "%d ", list1->flag[i]);
    fputc('\n', Outfp);
#endif

    if ((local_m_e < dat_element) || 
	/* (����¦)ɬ�ܳʤ�ľ���ʤΥޥå�����Ȥ��� */
	(closest > -1 && cfd->oblig[closest] == TRUE && list1->flag[closest] == NIL_ASSIGNED) || 
	/* ���δط������Υޥå����򤱤� */
	(!OptEllipsis && 
	 cf_element == 1 && MatchPP(lastpp, "���δط�"))) {
	local_score = -1;
    }
    else if (dat_element == 0 || pat_element == 0 || local_m_e == 0) {
	local_score = 0;
    }
    else {
	/* local_score = local_score * sqrt((double)local_m_e)
	   / sqrt((double)dat_element * pat_element);*/

	/* local_score = local_score * local_m_e
	   / (dat_element * sqrt((double)pat_element)); */

	/* Ʊ���ʥե졼��Ǥ��б��դ��˱ƶ� */
	local_score = local_score / sqrt((double)pat_element);

	/* corpus based case analysis 00/01/04 */
	/* local_score /= 10;	* ���������ʤ�,����11�� */
    }

    /* corpus based case analysis 00/01/04 */
    /* Ǥ�ճʤ˲��� */
    /* ����� expand ��Ԥä��Ȥ��Υ��������θ����ɬ�פ����� */
    /* local_score += (cfd->element_num - dat_element) * OPTIONAL_CASE_SCORE; */

    if (0 && OptEllipsis) {
	if (local_score > Current_max_score) {
	    Current_max_list1[0] = *list1;
	    Current_max_list2[0] = *list2;
	    Current_max_score = local_score;
	    Current_pure_score[0] = score;
	    Current_sufficiency = (float)cf_element/cfp->element_num;
	    Current_max_m_e = local_m_e;
	    Current_max_m_p = local_m_p;
	    Current_max_c_e = local_c_e;
	    Current_max_num = 1;
	}
	else if (local_score == Current_max_score && 
		 Current_max_num < MAX_MATCH_MAX) {
	    Current_max_list1[Current_max_num] = *list1;
	    Current_max_list2[Current_max_num] = *list2;
	    Current_pure_score[Current_max_num] = score;
	    Current_max_num++;
	}
    }
    else {
	if (local_score > Current_max_score || 
	    (local_score == Current_max_score &&
	     local_m_e > Current_max_m_e) ||
	    (local_score == Current_max_score &&
	     local_m_e == Current_max_m_e &&
	     local_m_p > Current_max_m_p) ||
	    (local_score == Current_max_score &&
	     local_m_e == Current_max_m_e &&
	     local_m_p == Current_max_m_p &&
	     local_c_e > Current_max_c_e) || 
	    (local_score == Current_max_score &&
	     local_m_e == Current_max_m_e &&
	     local_m_p == Current_max_m_p &&
	     local_c_e == Current_max_c_e && 
	     unassigned_ga == 0)) {
	    Current_max_list1[0] = *list1;
	    Current_max_list2[0] = *list2;
	    Current_max_score = local_score;
	    Current_pure_score[0] = score;
	    Current_sufficiency = (float)cf_element/cfp->element_num;
	    Current_max_m_e = local_m_e;
	    Current_max_m_p = local_m_p;
	    Current_max_c_e = local_c_e;
	    Current_max_num = 1;
	}
	else if (local_score == Current_max_score &&
		 local_m_e == Current_max_m_e &&
		 local_m_p == Current_max_m_p &&
		 local_c_e == Current_max_c_e &&
		 Current_max_num < MAX_MATCH_MAX) {
	    Current_max_list1[Current_max_num] = *list1;
	    Current_max_list2[Current_max_num] = *list2;
	    Current_pure_score[Current_max_num] = score;
	    Current_max_num++;
	}
    }
}

/*==================================================================*/
	 void eval_assign_prob(CASE_FRAME *cfd, LIST *list1,
			       CASE_FRAME *cfp, LIST *list2,
			       double score, int closest, 
			       CF_PRED_MGR *para_cpm_ptr)
/*==================================================================*/
{
    /* �ե졼��Υޥå����٤�ɾ�� (��Ψ��) */

    int i;
    int cf_element = 0, have_topic = 0;
    double local_score;

    /* �ʥե졼���Ψ */
    if (cfp->type == CF_PRED) { /* �Ȥꤢ�������Ѹ��Τ� */
	score = get_cf_probability(cfd, cfp);
    }
    else {
	score = 0;
    }

    /* ����¦�����å� */
    for (i = 0; i < cfd->element_num; i++) {
	cf_element++;

	if (list1->flag[i] == NIL_ASSIGNED && 
	    cfd->pred_b_ptr->cpm_ptr->elem_b_ptr[i]->num < cfd->pred_b_ptr->num && 
	    check_feature(cfd->pred_b_ptr->cpm_ptr->elem_b_ptr[i]->f, "����")) {
	    have_topic = 1;
	}

	/* Ϣ�ν�����������Ψ */
	score += get_np_modifying_probability(i, cfd);

	score += list1->score[i];

	if (MatchPP(cfd->pp[i][0], "��") || 
	    MatchPP(cfd->pp[i][0], "����")) {
	    ;
	}
	/* ������Ƥʤ� */
	else if (list1->flag[i] == NIL_ASSIGNED) {
	    if (CF_MatchPP(cfd->pp[i][0], cfp)) {
		score += NIL_ASSINED_SCORE;
	    }
	    /* �б�����ʥ���åȤ��ʤ���� => ����Ū�˳ʥ���åȤ�������Ƴ������ */
	    else {
		score += get_case_probability_from_str(pp_code_to_kstr(cfd->pp[i][0]), cfp, TRUE, para_cpm_ptr);
	    }
	    /* score += NIL_ASSINED_SCORE;
	       score += get_case_interpret_probability(i, cfd, list1->flag[i], cfp); */
	}
	/* �ʲ���Ψ P(ɽ�س�|�ʥ���å�) *
	else {
	    score += get_case_interpret_probability(i, cfd, list1->flag[i], cfp);
	}
	*/
    }

    score += get_topic_generating_probability(have_topic, cfd->pred_b_ptr);

    /* �ʥե졼��γ�������Ψ */
    for (i = 0; i < cfp->element_num; i++) {
	if (list2->flag[i] != UNASSIGNED) { /* ������Ƥ��� */
	    /* cf_element++; */
	    /* list2->score[i]: ��������������Ψ�����äƤ��� */
	    /* list2->score[i] += get_case_probability(i, cfp, TRUE) / cfd->weight[list2->flag[i]]; */
	}
	else { /* ������Ƥʤ� */
	    list2->score[i] = get_case_probability(i, cfp, FALSE, para_cpm_ptr);
	    score += list2->score[i];
	}
    }
    score += get_case_num_probability(cfp, cf_element, para_cpm_ptr); /* ������ƤΤ���Ŀ� */

    local_score = score;

    /* (����¦)ɬ�ܳʤ�ľ���ʤΥޥå�����Ȥ��� */
    if (closest > -1 && cfd->oblig[closest] == TRUE && list1->flag[closest] == NIL_ASSIGNED) {
	local_score -= 500;
    }


    if (local_score > Current_max_score) {
	Current_max_list1[0] = *list1;
	Current_max_list2[0] = *list2;
	Current_max_score = local_score;
	Current_pure_score[0] = score;
	Current_max_num = 1;
    }
    else if (local_score == Current_max_score &&
	     cfp->type == CF_PRED && /* ̾��β��ϻ���Ʊ������¸���ʤ� */
	     Current_max_num < MAX_MATCH_MAX) {
	Current_max_list1[Current_max_num] = *list1;
	Current_max_list2[Current_max_num] = *list2;
	Current_pure_score[Current_max_num] = score;
	Current_max_num++;
    }
}

int assign_list(CASE_FRAME *cfd, LIST list1,
		CASE_FRAME *cfp, LIST list2,
		double score, int flag, int closest, 
		CF_PRED_MGR *para_cpm_ptr);

/*==================================================================*/
	    int _assign_list(CASE_FRAME *cfd, LIST list1,
			     CASE_FRAME *cfp, LIST list2,
			     double score, int flag, int assign_flag, int closest, 
			     CF_PRED_MGR *para_cpm_ptr)
/*==================================================================*/
{
    /* 
       ʸ��γ����Ǥȳʥե졼��γ����Ǥ��б��դ�

       �����δؿ��ΰ��θƤӽФ��ǽ�������Τ�ʸ��γ����ǰ��

       ����������Ƥ��������(����,��ʤʤ�)������С���������,
         �ʤ������������Ƥ��ʤ�������(̤��,���ʸ�ʤ�)�����

       ��list.flag[i]�ˤ�i���ܤγ����Ǥ��б��դ��ξ������ݻ�

	  UNASSINGED ---- �б��դ��ޤ�
	  NIL_ASSINGED -- �б��դ����ʤ����Ȥ����
          j(����¾)------ ����j���ܤ��б��դ�

       ����������Ƥ���ʽ����ɬ�ܳʤǡ��б�����ʥ���åȤ�����Τ�
         ��̣�ޡ����԰��פξ�硤�ʥե졼�बʸ���Ф�����Ŭ���Ȥ�����
	 ��ϰ�̣�ޡ����λ��꤬�Ť�����Τǡ�����б��դ���Ԥäƽ���
	 ��ʤ�롥
    */

    int target = -1;	/* �ǡ���¦�ν����оݤγ����� */
    int target_pp = 0;
    int gaflag = 0, sotoflag = 0, toflag = 0, match_result;
    int i, j, pos, case_available = 0;
    double elmnt_score;

#ifdef CASE_DEBUG
    fprintf(Outfp, "dat");
    for (i = 0; i < cfd->element_num; i++)
	fprintf(Outfp, "%d ", list1.flag[i]);
    fputc('\n', Outfp);
    fprintf(Outfp, "pat");
    for (i = 0; i < cfp->element_num; i++)
	fprintf(Outfp, "%d ", list2.flag[i]);
    fputc('\n', Outfp);
#endif 
    
    /* �ޤ�������ƤΤʤ��ʽ���Υ����å� */
    for (i = 0; i < cfd->element_num; i++) {
	if (list1.flag[i] == UNASSIGNED) {
		/* if ((OptCaseFlag & OPT_CASE_USE_PROBABILITY) && 
		(MatchPP(cfd->pp[i][0], "����") || 
		MatchPP(cfd->pp[i][0], "��"))) {
		list1.flag[i] = NIL_ASSIGNED;
		continue;
		} */
	    if ((assign_flag == TRUE && cfd->pred_b_ptr->cpm_ptr->elem_b_num[i] != -1) || 
		(assign_flag == FALSE && cfd->pred_b_ptr->cpm_ptr->elem_b_num[i] == -1)) {
		target = i;
		break;
	    }
	}
    }

    if (target >= 0) {
	/* ���Ǥ˥��ʤ˳�����Ƥ����뤫�ɤ��� (����������Ʋ�ǽ���ɤ���) */
	for (i = 0; i < cfp->element_num; i++) {
	    if (list2.flag[i] != UNASSIGNED && 
		MatchPP2(cfp->pp[i], "��")) {
		gaflag = 1;
		break;
	    }
	}

	/* <����>���ɤ��� (���δط�������Ʋ�ǽ���ɤ���) */
	if (!cf_match_element(cfd->sm[target], "����", FALSE)) {
	    sotoflag = 1;
	}

	/* ���Ǥ���ʸ�ȳʤ˳�����Ƥ����뤫�ɤ��� (��ʳ�����Ʋ�ǽ���ɤ���) */
	for (i = 0; i < cfp->element_num; i++) {
	    if (list2.flag[i] != UNASSIGNED && 
		MatchPP2(cfp->pp[i], "��")) {
		/* cf_match_element(cfp->sm[i], "��ʸ", TRUE)) { */
		toflag = 1;
		break;
	    }
	}

	/* �ʥե졼��γʥ롼�� */
	for (i = 0; i < cfp->element_num; i++) {
	    /* �ʥե졼��ζ����Ƥ���� */
	    if (list2.flag[i] == UNASSIGNED) {
		/* ��ᤵ�줦��ʤΥ롼�� */
		for (target_pp = 0; cfd->pp[target][target_pp] != END_M; target_pp++) {
		    for (j = 0; cfp->pp[i][j] >= 0; j++) { /* ��ư���۳ʥե졼��ˤ�ʣ���γʤϤʤ� */
			if ((cfd->pp[target][target_pp] == cfp->pp[i][j] && 
			     !((cfp->pp[i][j] == pp_kstr_to_code("���δط�") && !sotoflag) || 
			       (cfp->pp[i][j] == pp_kstr_to_code("����") && !gaflag) || 
			       /* (cfp->pp[i][j] == pp_kstr_to_code("��") && toflag) || */
			       (cfp->pp[i][j] == pp_kstr_to_code("��") && 
				check_adjacent_assigned(cfd, cfp, &list1) == FALSE))) || 
			    (cfd->pp[target][target_pp] == pp_kstr_to_code("̤") && 
			     check_same_case(cfd->sp[target], cfp->pp[i][j], cfp))) {
			    case_available = 1;
			    pos = MATCH_NONE;
			    match_result = 
				elmnt_match_score(target, cfd, i, cfp, flag, &pos, &elmnt_score, para_cpm_ptr);

			    if ((OptCaseFlag & OPT_CASE_USE_PROBABILITY) || 
				(cfp->pp[i][j] != pp_kstr_to_code("���δط�") && 
				 cfp->pp[i][j] != pp_kstr_to_code("��") && 
				 cfd->pred_b_ptr->cpm_ptr->cf.type != CF_NOUN) || 
				((cfp->pp[i][j] == pp_kstr_to_code("���δط�") || 
				  cfp->pp[i][j] == pp_kstr_to_code("��")) &&
				 elmnt_score >= SOTO_THRESHOLD) || 
				(cfd->pred_b_ptr->cpm_ptr->cf.type == CF_NOUN && 
				 elmnt_score >= NOUN_THRESHOLD)) {
				if ((flag == EXAMPLE) || 
				    (flag == SEMANTIC_MARKER && elmnt_score >= 0)) {
				    /* �б��դ��򤷤ơ��Ĥ�γ����Ǥν����˿ʤ�
				       �� flag == SEMANTIC_MARKER && elmnt_score == 0
				       ���ʤ�����ʽ�����б�����ʥ���åȤ�����Τ�
				       ��̣�ޡ����԰��פξ��⡤������ʤ�� */

				    if (cfd->weight[target]) {
					elmnt_score /= cfd->weight[target];
				    }
				    list1.flag[target] = i;
				    list2.flag[i] = target;
				    list1.score[target] = elmnt_score;
				    list2.score[i] = elmnt_score;
				    list2.pos[i] = pos;
				    assign_list(cfd, list1, cfp, list2, 
						score + elmnt_score, flag, closest, para_cpm_ptr);
				    list2.flag[i] = UNASSIGNED;
				    list2.pos[i] = MATCH_NONE;
				}
			    }
			    break;
			}
		    }
		}
	    }
	}

	if (!(cfd->pred_b_ptr->cpm_ptr->cf.type == CF_PRED && /* �Ȥꤢ�����Ѹ��Τ� */
	      (OptCaseFlag & OPT_CASE_USE_PROBABILITY)) || 
	    !case_available) {
	    /* target���ܤγ����Ǥˤ��б��դ���Ԥ�ʤ��ޡ��� */
	    list1.flag[target] = NIL_ASSIGNED;

	    /* Ǥ�ճʤȤ��б��դ���Ԥ�ʤ����
	       �� Ʊ��ɽ�سʤ��ʥե졼��ˤ����硤�б��դ��򤹤뤳�Ȥ�
	       ���Ǥ˾�ǻ��Ƥ���
	       if (cfd->oblig[target] == FALSE) */
	    /* ɬ�ܳʤ��б�̵(ɽ�سʤΰ��פ����Τ��ʤ�)�ξ��
	       => eval_assign���Ե���
	       ɬ�ܳʤ��б�ͭ�ξ��
	       => ����Ʊ���ʽ��줬������б��դ��򤷤ʤ���ǽ����? */

	    /* ������Ƥʤ��Υ����� */
	    elmnt_score = FREQ0_ASSINED_SCORE + get_case_function_probability_for_pred(target, cfd, NIL_ASSIGNED, cfp, FALSE);
	    if (cfd->weight[target]) {
		elmnt_score /= cfd->weight[target];
	    }
	    list1.score[target] = elmnt_score;

	    assign_list(cfd, list1, cfp, list2, 
			(OptCaseFlag & OPT_CASE_USE_PROBABILITY) ? score + elmnt_score : score, 
			flag, closest, para_cpm_ptr);
	}
	return FALSE;
    }
    return TRUE;
}

/*==================================================================*/
	     int assign_list(CASE_FRAME *cfd, LIST list1,
			     CASE_FRAME *cfp, LIST list2,
			     double score, int flag, int closest, 
			     CF_PRED_MGR *para_cpm_ptr)
/*==================================================================*/
{
    /* ̤��, Ϣ�ʰʳ�����˳������ */

    if (_assign_list(cfd, list1, cfp, list2, score, flag, TRUE, closest, para_cpm_ptr) == FALSE) {
	return FALSE;
    }
    if (_assign_list(cfd, list1, cfp, list2, score, flag, FALSE, closest, para_cpm_ptr) == FALSE) {
	return FALSE;
    }

    /* ɾ�� : ���٤Ƥ��б��դ�������ä���� */
    if (OptCaseFlag & OPT_CASE_USE_PROBABILITY) {
	eval_assign_prob(cfd, &list1, cfp, &list2, score, closest, para_cpm_ptr);
    }
    else {
	eval_assign_score(cfd, &list1, cfp, &list2, (int)score, closest);
    }
    return TRUE;
}

/*==================================================================*/
int case_frame_match(CF_PRED_MGR *cpm_ptr, CF_MATCH_MGR *cmm_ptr, int flag, int closest, CF_PRED_MGR *para_cpm_ptr)
/*==================================================================*/
{
    /* �ʥե졼��Υޥå��� */

    LIST assign_d_list, assign_p_list;
    int i;
    CASE_FRAME *cfd = &(cpm_ptr->cf);

    /* ����� */

    Current_max_num = 0;
    Current_max_score = (OptCaseFlag & OPT_CASE_USE_PROBABILITY) ? CASE_MATCH_FAILURE_PROB : CASE_MATCH_FAILURE_SCORE;
    Current_sufficiency = 0;
    Current_max_m_e = 0;
    Current_max_m_p = 0;
    Current_max_c_e = 0;

    for (i = 0; i < cfd->element_num; i++) {
	assign_d_list.flag[i] = UNASSIGNED;
	assign_d_list.score[i] = -1;
    }

    /* for (i = 0; i < cmm_ptr->cf_ptr->element_num; i++) { */
    for (i = 0; i < CF_ELEMENT_MAX; i++) {
	assign_p_list.flag[i] = UNASSIGNED;
	assign_p_list.score[i] = -1;
	assign_p_list.pos[i] = -1;
    }

    /* ���� */

    /* flag: �� or ��̣������ */
    assign_list(cfd, assign_d_list, cmm_ptr->cf_ptr, assign_p_list, 0, flag, closest, para_cpm_ptr);

    /* ����� */

    if (Current_max_num == MAX_MATCH_MAX) {
	fprintf(stderr, "; Too many case matching result !\n");
    }

    cmm_ptr->sufficiency = Current_sufficiency;
    cmm_ptr->result_num = Current_max_num;
    for (i = 0; i < Current_max_num; i++) {
	cmm_ptr->result_lists_p[i] = Current_max_list2[i];
	cmm_ptr->result_lists_d[i] = Current_max_list1[i];
	cmm_ptr->pure_score[i] = Current_pure_score[i];
    }

    /* ľ�������ǤΥ������Τߤ��Ѥ���Ȥ� */
    if (closest > -1 && Current_max_score >= 0 && 
	Current_max_list1[0].flag[closest] != NIL_ASSIGNED) {
	/* ľ�������Ǥγ�����Ƥ����뤳�Ȥ���� */
	cmm_ptr->score = Current_max_list1[0].score[closest];
    }
    else {
	cmm_ptr->score = Current_max_score;
    }

#ifdef CASE_DEBUG
    print_crrspnd(cfd, cmm_ptr);
#endif
    return 1;
}

/*====================================================================
                               END
====================================================================*/
