/*====================================================================

			�ʹ�¤����: �ޥå���

                                               S.Kurohashi 93. 5.31

    $Id$
====================================================================*/
#include "knp.h"

float 	Current_max_score;	/* ���� */
int 	Current_pure_score[MAX_MATCH_MAX];	/* ������������������ */
float	Current_sufficiency;	/* ��ޤꤰ���� */
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

int 	EX_match_score[] = {0, 0, 5, 7, 8, 9, 10, 11};
/* int 	EX_match_score[] = {0, 0, 0, 1, 3, 5, 10, 11}; */
							/* �����б������� */
int     EX_match_unknown = 8; /* 10; */			/* �ǡ���̤��     */
int     EX_match_sentence = 10;				/* ������ -- ʸ   */
int     EX_match_tim = 0;				/* ������ -- ����:���ֳ� */
int     EX_match_tim2 = 12;				/* ������ -- ����:����¾�γ� */
int     EX_match_tim3 = 8;				/* ������ -- ����:������� */
int     EX_match_qua = 9; /* 10; */			/* ������ -- ���� */
int	EX_match_exact = 12;
int	EX_match_subject = 8;
int	EX_match_modification = 0;

int	Thesaurus = USE_NTT;

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

    int i, current_score, score = 0;
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
	      int _ex_match_score(char *cp1, char *cp2)
/*==================================================================*/
{
    /* ���ʬ�����ɽ�����ɤΥޥå����٤η׻� */

    int match = 0;

    /* ñ�̤ι��ܤ�̵�� */
    if (!strncmp(cp1, "11960", 5) || !strncmp(cp2, "11960", 5))
	return 0;
    
    /* ��� */
    match = bgh_code_match(cp1, cp2);

    /* ��̾��ι��ܤ�����٤򲡤����� */
    if ((!strncmp(cp1, "12000", 5) || !strncmp(cp2, "12000", 5)) &&
	match > 3)
	return 3;
    
    return match;
}

/*==================================================================*/
     int _cf_match_element(char *d, char *p, int start, int len)
/*==================================================================*/
{
    int i, j;
    char *code;

    for (i = 0; *(d+i); i += SM_CODE_SIZE) {
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
    return FALSE;
}

/*==================================================================*/
	int cf_match_element(char *d, char *target, int flag)
/*==================================================================*/
{
    char *code;

    /* flag == TRUE  : ���ΰ�̣�Ǥ� exact match
       flag == FALSE : ���ΰ�̣�ǰʲ��ˤ���� match */

    if (d == NULL) {
	return FALSE;
    }

    code = sm2code(target);

    if (flag == TRUE) {
	return _cf_match_element(d, code, 0, SM_CODE_SIZE);
    }
    else {
	/* �� �����ɤ� 2 ʸ���ʾ夢��ɬ�פ����� */
	return _cf_match_element(d, code, 1, sm_code_depth(code));
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
	/* �� �����ɤ� 2 ʸ���ʾ夢��ɬ�פ����� */
	len = sm_code_depth(code);
	if (_cf_match_element(d, code, 1, len) == TRUE && 
	    _cf_match_element(p, code, 1, len) == TRUE) {
	    return TRUE;
	}
	else {
	    return FALSE;
	}
    }
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
    /* ��̣�� : ������ -- ���� (���ʤΤ�) */
    else if (MatchPP(cfp->pp[as2][0], "��") && 
	     cf_match_both_element(cfd->sm[as1], cfp->sm[as2], "����", FALSE)) {
	*pos = MATCH_SUBJECT;
	return EX_match_subject;
    }
    return -100;
}

/*==================================================================*/
int cf_match_exactly(BNST_DATA *d, char **ex_list, int ex_num, int *pos)
/*==================================================================*/
{
    if (!check_feature(d->f, "����̾��") && 
	d->jiritu_ptr != NULL) {
	if (d->jiritu_num > 1 && 
	    check_feature((d->jiritu_ptr+d->jiritu_num-1)->f, "�Ը�ͭ����")) {
	    *pos = check_examples((d->jiritu_ptr+d->jiritu_num-2)->Goi, ex_list, ex_num);
	}
	else {
	    *pos = check_examples(L_Jiritu_M(d)->Goi, ex_list, ex_num);
	}
	if (*pos >= 0) {
	    return 1;
	}
    }
    return 0;
}

/*==================================================================*/
	   int elmnt_match_score(int as1, CASE_FRAME *cfd,
				 int as2, CASE_FRAME *cfp, int flag, int *pos)
/*==================================================================*/
{
    /* ��̣�ޡ����Υޥå����٤η׻� */

    int i, j, k, tmp_score, score = -100, ex_score = -100;
    char *exd, *exp;
    int *match_score;

    if (flag == SEMANTIC_MARKER) {
	
	if (cfd->sm[as1][0] == '\0'|| cfp->sm[as2] == NULL || cfp->sm[as2][0] == '\0') 
	    return SM_match_unknown;

	for (j = 0; cfp->sm[as2][j]; j+=SM_CODE_SIZE) {
	    if (!strncmp(cfp->sm[as2]+j, (char *)sm2code("��"), SM_CODE_SIZE)) {

		for (k = 0; cfp->ex[as2][k]; k+=BGH_CODE_SIZE) {
		    for (i = 0; cfd->ex[as1][i]; i+=BGH_CODE_SIZE) {
			tmp_score = 
			    EX_match_score[_ex_match_score(cfp->ex[as2]+k, 
							   cfd->ex[as1]+i)];
			if (tmp_score == 11) {
			    return 10;
			}
		    }
		}
	    }
	    else {
		for (i = 0; cfd->sm[as1][i]; i+=SM_CODE_SIZE) {
		    tmp_score = 
			SM_match_score[_sm_match_score(cfp->sm[as2]+j,
						       cfd->sm[as1]+i, SM_NO_EXPAND_NE)];
		    if (tmp_score > score) score = tmp_score;
		}
	    }
	}
	return score;
    }

    else if (flag == EXAMPLE) {
	int ga_subject;

	/* �����ʤΤȤ� */
	if (MatchPP(cfd->pp[as1][0], "����")) {
	    return EX_match_modification;
	}

	if (MatchPP(cfp->pp[as2][0], "��") && 
	    cf_match_both_element(cfd->sm[as1], cfp->sm[as2], "����", FALSE)) {
	    ga_subject = 1;
	}
	else {
	    ga_subject = 0;
	}

	/* exact match */
	if (ga_subject == 0 && 
	    cfp->concatenated_flag == 0 && 
	    cf_match_exactly(cfd->pred_b_ptr->cpm_ptr->elem_b_ptr[as1], 
			     cfp->ex_list[as2], cfp->ex_num[as2], pos)) {
	    return EX_match_exact;
	}

	if (Thesaurus == USE_BGH) {
	    exd = cfd->ex[as1];
	    exp = cfp->ex[as2];
	    match_score = EX_match_score;
	}
	else if (Thesaurus == USE_NTT) {
	    exd = cfd->ex2[as1];
	    exp = cfp->ex2[as2];
	    match_score = EX_match_score;
	}

	/* ����<����>���̥����� */
	if (ga_subject) {
	    *pos = MATCH_SUBJECT;
	    return EX_match_subject;
	}
	else {
	    score = elmnt_match_score_each_sm(as1, cfd, as2, cfp, pos);
	}

	/* ���㤬�ɤ��餫�����Ǥ�ʤ��ä��� */
	if (*exd == '\0') {
	    if (*cfd->sm[as1] == '\0') {
		score = EX_match_unknown;
	    }
	    else if (score < 0) {
		score = 0;
	    }
	}
	else if (exp == NULL || *exp == '\0') {
	    /* �ʥե졼��¦������ΰ�̣°�����ʤ��Ȥ� */
	    if (cfp->sm[as2] == NULL) {
		score = EX_match_unknown;
	    }
	    /* ��̣°���Ϥ��뤬��match ���ʤ��Ȥ� */
	    else if (score < 0) {
		score = 0;
	    }
	}
	else {
	    float rawscore;

	    rawscore = CalcSmWordsSimilarity(exd, cfp->ex_list[as2], cfp->ex_num[as2], pos, 
					     cfp->sm_delete[as2], 0);
	    /* rawscore = CalcWordsSimilarity(cfd->ex_list[as1][0], cfp->ex_list[as2], cfp->ex_num[as2], pos); */
	    /* rawscore = CalcSimilarity(exd, exp); */
	    /* ����Υޥå��� */
	    if (Thesaurus == USE_BGH) {
		ex_score = *(match_score+(int)rawscore);
	    }
	    else if (Thesaurus == USE_NTT) {
		ex_score = *(match_score+(int)(rawscore*7));
	    }
	}

	/* �礭�����򤫤��� */
	if (ex_score > score) {
	    return ex_score;
	}
	else {
	    return score;
	}
    }
    return 0;
}

/*==================================================================*/
	 int count_pat_element(CASE_FRAME *cfp, LIST *list2)
/*==================================================================*/
{
    int i, pat_element = 0;
    for (i = 0; i < cfp->element_num; i++) {
	if (!(cfp->oblig[i] == FALSE && list2->flag[i] == UNASSIGNED)) {
	    pat_element++;
	}
    }
    return pat_element;
}

/*==================================================================*/
	    void eval_assign(CASE_FRAME *cfd, LIST *list1,
			     CASE_FRAME *cfp, LIST *list2,
			     int score)
/*==================================================================*/
{
    /* �ե졼��Υޥå����٤η׻�(��������ʬ�����) */

    int i, j;
    int local_m_e = 0;
    int local_m_p = 0;
    int local_c_e = 0;
    int pat_element, dat_element = 0;
    int cf_element = 0;
    float local_score;

    local_score = score;

    /* �� experimental (����٤��⤤ľ���ʤ˥ܡ��ʥ�)
    for (i = 0; i < cfd->element_num; i++) {
	if (cfd->adjacent[i] == TRUE && 
	    list1->flag[i] != NIL_ASSIGNED && 
	    cfp->adjacent[list1->flag[i]] == TRUE && 
	    list1->score[i] > 10) {
	    local_score += 2;
	}
    } */

    /* ���ǿ������Ǥΰ��֡��򺹿� */
    for (i = 0; i < cfd->element_num; i++) {
	if (list1->flag[i] != NIL_ASSIGNED) {
	    local_m_e ++;
	    local_m_p += i;
	    for (j = i+1; j < cfd->element_num; j++)
	      if (list1->flag[j] != NIL_ASSIGNED &&
		  list1->flag[j] < list1->flag[i]) 
		local_c_e --;
	}
    }

    /* ʸ������ǿ�(Ǥ�դǥޥå����Ƥ��ʤ����ǰʳ�) */
    /* �� ������ʸ���､�����Ǥ�հ��� */
    for (i = 0; i < cfd->element_num; i++)
	if (!(cfd->oblig[i] == FALSE && list1->flag[i] == NIL_ASSIGNED))
	    dat_element++;

    /* �ʥե졼��������ǿ�(Ǥ�դǥޥå����Ƥ��ʤ����ǰʳ�) */
    pat_element = count_pat_element(cfp, list2);

    /* �ʥե졼��������ǿ� */
    for (i = 0; i < cfp->element_num; i++) {
	if (list2->flag[i] != UNASSIGNED) {
	    cf_element++;
	}
    }

    if (local_m_e < dat_element) {
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

	local_score = local_score / sqrt((double)pat_element);

	/* corpus based case analysis 00/01/04 */
	/* local_score /= 10;	* ���������ʤ�,����11�� */
    }

    /* corpus based case analysis 00/01/04 */
    /* Ǥ�ճʤ˲��� */
    /* ����� expand ��Ԥä��Ȥ��Υ��������θ����ɬ�פ����� */
    /* local_score += (cfd->element_num - dat_element) * OPTIONAL_CASE_SCORE; */

    if (local_score > Current_max_score || 
	(local_score == Current_max_score &&
	 local_m_e > Current_max_m_e) ||
	(local_score == Current_max_score &&
	 local_m_e == Current_max_m_e &&
	 local_m_p > Current_max_m_p) ||
	(local_score == Current_max_score &&
	 local_m_e == Current_max_m_e &&
	 local_m_p == Current_max_m_p &&
	 local_c_e > Current_max_c_e)) {
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
	    
/*==================================================================*/
	       void assign_list(CASE_FRAME *cfd, LIST list1,
				CASE_FRAME *cfp, LIST list2,
				int score, int flag)
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

    int ec_match_flag;	/* ��������Ƥ���ʽ���
			     �б�ͭ,��̣�ޡ�������:    1
			     �б�ͭ,��̣�ޡ����԰���: -1
			     �б�̵:                   0 */

    int target = -1;	/* �ǡ���¦�ν����оݤγ����� */
    int target_pp = 0;
    int elmnt_score, multi_pp = 0;
    int i, j, pos;

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
    
    /* ��������Ƥ���ʽ���Υ����å� */
    for (i = 0; i < cfd->element_num; i++) {
	if (list1.flag[i] == UNASSIGNED) {
	    for (j = 0; cfd->pp[i][j] != END_M; j++) {
		if (cfd->pp[i][j] >= 0) {
		    target = i;
		    target_pp = j;
		    break;
		}
	    }
	    if (target >= 0) {
		break;
	    }
	}
    }

    /* ��������Ƥ���ʽ���ν��� */
    if (target >= 0) {
	for (i = 0; cfd->pp[target][i] != END_M; i++) {
	    if (cfd->pp[target][i] < 0) {
		multi_pp = 1;
		break;
	    }
	}
	ec_match_flag = 0;
	for (i = 0; i < cfp->element_num; i++) {
	    if (list2.flag[i] == UNASSIGNED) {
		for (target_pp = 0; cfd->pp[target][target_pp] != END_M; target_pp++) {
		    if (cfd->pp[target][target_pp] < 0) {
			continue;
		    }
		    for (j = 0; cfp->pp[i][j] >= 0; j++) {
			if (cfd->pp[target][target_pp] == cfp->pp[i][j] ||
			    (cfd->pp[target][target_pp] == pp_hstr_to_code("�ˤ�ä�") &&
			     cfp->pp[i][j] == pp_kstr_to_code("��"))) {

			    pos = MATCH_NONE;
			    elmnt_score = 
				elmnt_match_score(target, cfd, i, cfp, flag, &pos);

			    if (flag == EXAMPLE || 
				(flag == SEMANTIC_MARKER && elmnt_score != 0)) {

				/* �б��դ��򤷤ơ��Ĥ�γ����Ǥν����˿ʤ� */

				ec_match_flag = 1;
				if (cfd->weight[target]) {
				    elmnt_score /= cfd->weight[target];
				}
				list1.flag[target] = i;
				list2.flag[i] = target;
				list1.score[target] = elmnt_score;
				list2.score[i] = elmnt_score;
				list2.pos[i] = pos;
				assign_list(cfd, list1, cfp, list2, 
					    score + elmnt_score, flag);
				list2.flag[i] = UNASSIGNED;
			    } 
			
			    else {
				/* flag == SEMANTIC_MARKER && elmnt_score == 0
				   ���ʤ�����ʽ�����б�����ʥ���åȤ�����Τ�
				   ��̣�ޡ����԰��פξ��⡤������ʤ�� */

				if (ec_match_flag == 0) ec_match_flag = -1;

				list1.flag[target] = i;
				list2.flag[i] = target;
				list1.score[target] = elmnt_score;
				list2.score[i] = elmnt_score;
				list2.pos[i] = pos;
				/* �б��դ��򤷤ơ��Ĥ�γ����Ǥν����˿ʤ� */
				assign_list(cfd, list1, cfp, list2, 
					    score + elmnt_score, flag);
				list2.flag[i] = UNASSIGNED;
			    }
			    break;
			}
		    }
		}
	    }
	}

	list1.flag[target] = NIL_ASSIGNED;
		/* target���ܤγ����Ǥˤ��б��դ���Ԥ�ʤ��ޡ��� */

	/* Ǥ�ճʤξ��
	   �� Ʊ��ɽ�سʤ��ʥե졼��ˤ����硤�б��դ��򤹤뤳�Ȥ�
	      ���Ǥ˾�ǻ��Ƥ���
	 */
	if (cfd->oblig[target] == FALSE) {
	    assign_list(cfd, list1, cfp, list2, score, flag);
	}

	/* ɬ�ܳʤ��б�̵(ɽ�سʤΰ��פ����Τ��ʤ�)�ξ��
	   �� ���ξ��eval_assign��score�ϥޥ��ʥ��ˤʤ�Ϥ��ʤΤ�,
	      ���θƽФ���ɬ�פ���
	 */
	else if (ec_match_flag == 0) {
	    assign_list(cfd, list1, cfp, list2, score, flag);
	}

	/* ɬ�ܳʤ��б�ͭ�ξ��
	      ����Ʊ���ʽ��줬������б��դ��򤷤ʤ���ǽ����
	   �� ���ξ���eval_assign��score�ϥޥ��ʥ��ˤʤ�Ϥ��ʤΤ�,
	      ���θƽФ���ɬ�פ���
	 */
	else {
	    for (i = target + 1; i < cfd->element_num; i++) {
		if ( cfd->pp[target][target_pp] == cfd->pp[i][0]) {
		    assign_list(cfd, list1, cfp, list2, score, flag);
		    break;
		}
	    }
	}
	/* multi_pp �ΤȤ��� list1.flag[target] �� UNASSIGNED �ˤ��ơ�
	   �����ʤ���˥����å���������������ʤ���˥����å�������
	   ��ξ��������å����� */
	if (multi_pp) {
	    list1.flag[target] = UNASSIGNED;
	    target = -1;
	}
	else {
	    return;
	}
    }

    /* ��������Ƥ��ʤ��ʽ���Υ����å� */
    for (i = 0; i < cfd->element_num; i++) {
	if (list1.flag[i] == UNASSIGNED) {
	    for (j = 0; cfd->pp[i][j] != END_M; j++) {
		if (cfd->pp[i][j] < 0) {
		    target = i;
		    target_pp = j;
		    break;
		}
	    }
	    if (target >= 0) {
		break;
	    }
	}
    }

    /* ��������Ƥ��ʤ��ʽ���ν��� */
    if (target >= 0) {
	int renkaku, mikaku, verb, gaflag = 0, sotoflag = 0, soto_decide;

	if (cfd->pp[target][target_pp] == -2) {
	    renkaku = 1;
	}
	else {
	    renkaku = 0;
	}
	if (cfd->pp[target][target_pp] == -1) {
	    mikaku = 1;
	}
	else {
	    mikaku = 0;
	}
	if (cfd->ipal_id[0] && str_eq(cfd->ipal_id, "ư")) {
	    verb = 1;
	}
	else {
	    verb = 0;
	}

	/* ���Ǥ˥��ʤ˳�����Ƥ��Ƥ��뤫 (����������) */
	if (OptCaseFlag & OPT_CASE_GAGA) {
	    for (i = 0; i < cfp->element_num; i++) {
		if (list2.flag[i] != UNASSIGNED && 
		    cfp->pp[i][1] == END_M && 
		    cfp->pp[i][0] == pp_kstr_to_code("��")) {
		    gaflag = 1;
		    break;
		}
	    }
	}

	/* ���δط����� */
	if (OptCaseFlag & OPT_CASE_SOTO) {
	    sotoflag = 1;
	}

	for (i = 0; i < cfp->element_num; i++) {

	    /* "����" --> "�����򡤤�" �� �б���
	       ���ʸ --> ���ȤǤ��б��� */

	    if (list2.flag[i] == UNASSIGNED &&
		((mikaku &&
		  cfp->pp[i][1] == END_M &&
		  (cfp->pp[i][0] == pp_kstr_to_code("��") ||
		   cfp->pp[i][0] == pp_kstr_to_code("��") || 
		   (gaflag && cfp->pp[i][0] == pp_kstr_to_code("����"))
		   )) ||
		 (renkaku &&
		  cfp->pp[i][1] == END_M &&
		  (cfp->pp[i][0] == pp_kstr_to_code("��") ||
		   cfp->pp[i][0] == pp_kstr_to_code("��") ||
		   (sotoflag && cfp->pp[i][0] == pp_kstr_to_code("���δط�")) ||
		   (verb && cfp->voice == FRAME_ACTIVE && cfp->pp[i][0] == pp_kstr_to_code("��")))))) {
		pos = MATCH_NONE;
		elmnt_score = elmnt_match_score(target, cfd, i, cfp, flag, &pos);
		if (elmnt_score != 0 || flag == EXAMPLE) {
		    if (cfd->weight[target]) {
			elmnt_score /= cfd->weight[target];
		    }
		    list1.flag[target] = i;
		    list2.flag[i] = target;
		    list1.score[target] = elmnt_score;
		    list2.score[i] = elmnt_score;
		    list2.pos[i] = pos;
		    assign_list(cfd, list1, cfp, list2, score + elmnt_score, flag);
		    list2.flag[i] = UNASSIGNED;
		}
	    }
	}

	list1.flag[target] = NIL_ASSIGNED;
	elmnt_score = 0;
	soto_decide = 0;

	/* ���δط����ȿ��ꤷ�ƥܡ��ʥ���Ϳ���� */
	if (renkaku && verb) {
	    /* ���δط������� == SOTO_ADD_SCORE + OPTIONAL_CASE_SCORE */
	    if (OptDisc == OPT_DISC) {
		elmnt_score = 0;
	    }
	    else {
		elmnt_score = SOTO_SCORE;
	    }
	    if (cfd->weight[target]) {
		elmnt_score /= cfd->weight[target];
	    }
	    /* elmnt_score -= OPTIONAL_CASE_SCORE; * Ǥ�ճʥܡ��ʥ���ʬ */

	    /* �ʥե졼��ˡֳ��δط��׳ʤ��ɲ� 
	       ���� cfp->element_num ���ᤷ�Ƥ���Τǡ�
	       Ʊ���ʤ�ʣ�����뤫�ɤ���������å����Ƥ��ʤ� */
	    if (cfp->element_num < CF_ELEMENT_MAX) {
		_make_ipal_cframe_pp(cfp, "���δط�", cfp->element_num);
		list1.flag[target] = cfp->element_num;
		list1.score[target] = elmnt_score;
		list2.flag[cfp->element_num] = target;
		list2.score[cfp->element_num] = elmnt_score;
		cfp->element_num++;
		soto_decide = 1;
	    }
	}
	else if (mikaku) {
	    /* elmnt_score -= OPTIONAL_CASE_SCORE; * Ǥ�ճʥܡ��ʥ���ʬ */
	}
	assign_list(cfd, list1, cfp, list2, score + elmnt_score, flag);

	/* cfp->element_num �� global �˱ƶ�����ΤǸ����ᤷ�Ƥ��� */
	if (soto_decide) {
	    cfp->element_num--;
	}
	return;
    }
    else if (multi_pp == 1) {
	return;
    }

    /* ɾ�� : ���٤Ƥ��б��դ������ä���� */
    eval_assign(cfd, &list1, cfp, &list2, score);
}

/*==================================================================*/
int case_frame_match(CF_PRED_MGR *cpm_ptr, CF_MATCH_MGR *cmm_ptr, int flag, int closest)
/*==================================================================*/
{
    /* �ʥե졼��Υޥå��� */

    LIST assign_d_list, assign_p_list;
    int i;
    CASE_FRAME *cfd = &(cpm_ptr->cf);

    /* ����� */

    Current_max_num = 0;
    Current_max_score = -2;
    Current_sufficiency = 0;
    Current_max_m_e = 0;
    Current_max_m_p = 0;
    Current_max_c_e = 0;
    for (i = 0; i < cfd->element_num; i++) {
	assign_d_list.flag[i] = UNASSIGNED;
	assign_d_list.score[i] = -1;
    }
/*    for (i = 0; i < cmm_ptr->cf_ptr->element_num; i++) { */
    for (i = 0; i < CF_ELEMENT_MAX; i++) {
	assign_p_list.flag[i] = UNASSIGNED;
	assign_p_list.score[i] = -1;
	assign_p_list.pos[i] = -1;
    }

    /* ���� */

    assign_list(cfd, assign_d_list, cmm_ptr->cf_ptr, assign_p_list, 0, flag);
					    	/* flag �� or ��̣������ */

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
    if (closest > -1) {
	/* ľ�������Ǥγ�����Ƥ����뤳�Ȥ���� */
	if (Current_max_list1[0].flag[closest] != NIL_ASSIGNED) {
	    cmm_ptr->score = (float)Current_max_list1[0].score[closest];
	}
	else {
	    cmm_ptr->score = -1;
	    return 0;
	}
    }
    else {
	cmm_ptr->score = Current_max_score;
    }

    /* tentative */
    if (cmm_ptr->cf_ptr->concatenated_flag == 1) {
	cmm_ptr->score += 1;
    }

#ifdef CASE_DEBUG
    print_crrspnd(cfd, cmm_ptr);
#endif
    return 1;
}

/*====================================================================
                               END
====================================================================*/
