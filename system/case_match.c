/*====================================================================

			�ʹ�¤����: �ޥå���

                                               S.Kurohashi 93. 5.31

    $Id$
====================================================================*/
#include "knp.h"

int 	Current_ec_score;	/* ��������Ƥ���ʤ����� */
int 	Current_max_score;	/* ��������Ƥ��ʤ��ʤ����� */
int 	Current_max_m_e;	/* ���ǿ� */
int 	Current_max_m_p;	/* ���Ǥΰ��� */
int 	Current_max_c_e;	/* �򺹿� */

int 	Current_max_num;
LIST 	Current_max_list1[MAX_MATCH_MAX];
LIST 	Current_max_list2[MAX_MATCH_MAX];


int 	SM_match_score[] = {0, 100, 100, 100, 100, 100, 100, 100, 100, 
			    100, 100, 100, 100};
  				/*{0, 5, 8, 10, 12};*/	/* �ӣ��б������� */
int     SM_match_unknown = 10;			 	/* �ǡ���̤��     */

int 	EX_match_score[] = {0, 0, 50, 70, 80, 90, 100, 110}; 
							/* �����б������� */
int     EX_match_unknown = 50; 				/* �ǡ���̤��     */
int     EX_match_sentence = 80;				/* ������ -- ʸ   */
int     EX_match_tim = 80;				/* ������ -- ���� */
int     EX_match_qua = 80;				/* ������ -- ���� */

extern FILE  *Infp;
extern FILE  *Outfp;

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
		for (cp = (char *)smp2smg(cpd); *cp; cp+=SM_CODE_SIZE+1) {
		    if (*(cp+SM_CODE_SIZE) == '/')
			*(cp+SM_CODE_SIZE) = '\0';
		    else if (str_eq(cp+SM_CODE_SIZE, " side-effect"))
			continue;

		    if (*(cp+SM_CODE_SIZE) != '\0')
			fprintf(stderr, ";;; Invalid delimiter! <%c> (%s)\n", 
				*(cp+SM_CODE_SIZE), "_sm_match_score");
		    else {
			current_score = comp_sm(cpp, cp, 1);
			if (current_score > score)
			    score = current_score;
		    }
		}
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

    int  match = 0;

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
 int cf_match_both_element(char *d, char *p, char *target, int unit)
/*==================================================================*/
{
    int i, j;

    for (i = 0; *(d+i); i += unit) {
	if (!strncmp(d+i, (char *)sm2code(target), unit)) {
	    for (j = 0; *(p+j); j += unit) {
		if (!strncmp(p+i, (char *)sm2code(target), unit)) {
		    return TRUE;
		}
	    }
	    return FALSE;
	}
    }
    return FALSE;
}

/*==================================================================*/
	   int elmnt_match_score(int as1, CASE_FRAME *cfd,
				 int as2, CASE_FRAME *cfp, int flag)
/*==================================================================*/
{
    /* ��̣�ޡ����Υޥå����٤η׻� */

    int i, j, k, tmp_score, score = -100, ex_score = -100;

    if (flag == SEMANTIC_MARKER) {
	
	if (cfd->sm[as1][0] == '\0'|| cfp->sm[as2][0] == '\0') 
	    return SM_match_unknown;

	for (j = 0; cfp->sm[as2][j]; j+=SM_CODE_SIZE) {
	    if (!strncmp(cfp->sm[as2]+j, (char *)sm2code("��"), SM_CODE_SIZE)) {

		for (k = 0; cfp->ex[as2][k]; k+=BGH_CODE_SIZE) {
		    for (i = 0; cfd->ex[as1][i]; i+=BGH_CODE_SIZE) {
			tmp_score = 
			    EX_match_score[_ex_match_score(cfp->ex[as2]+k, 
							   cfd->ex[as1]+i)];
			if (tmp_score == 110) {
			    return 100;
			}
		    }
		}
	    }
	    else {
		for (i = 0; cfd->sm[as1][i]; i+=SM_CODE_SIZE) {
		    tmp_score = 
			SM_match_score[_sm_match_score(cfp->sm[as2]+j,
						       cfd->sm[as1]+i, SM_NO_EXPAND_NE)];
		    if (tmp_score && (cfp->sm_flag[as2][j/SM_CODE_SIZE] == FALSE))
			return -100;
		    if (tmp_score > score) score = tmp_score;
		}
	    }
	}
	return score;
    }

    else if (flag == EXAMPLE) {
	
	/* ���� : ������ -- ʸ */
	if (cf_match_both_element(cfd->sm[as1], cfp->sm[as2], 
				  "��ʸ", SM_CODE_SIZE)) {
	    score = EX_match_sentence;
	}
	/* ���� : ������ -- ���� */
	else if (cf_match_both_element(cfd->sm[as1], cfp->sm[as2], 
				       "����", SM_CODE_SIZE)) {
	    score = EX_match_tim;
	}
	/* ���� : ������ -- ���� */
	else if (cf_match_both_element(cfd->sm[as1], cfp->sm[as2], 
				       "����", SM_CODE_SIZE)) {
	    score = EX_match_qua;
	}

	/* ���㤬�ɤ��餫�����Ǥ�ʤ��ä��� */
	if (cfd->ex[as1][0] == '\0' || cfp->ex[as2][0] == '\0') 
	    return EX_match_unknown;

	/* ����Υޥå��� */
	for (j = 0; cfp->ex[as2][j]; j+=BGH_CODE_SIZE) {
	    for (i = 0; cfd->ex[as1][i]; i+=BGH_CODE_SIZE) {
		tmp_score = 
		    EX_match_score[_ex_match_score(cfp->ex[as2]+j, 
						   cfd->ex[as1]+i)];
		if (tmp_score > ex_score) ex_score = tmp_score;
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
	    void eval_assign(CASE_FRAME *cfd, LIST *list1,
			     CASE_FRAME *cfp, LIST *list2,
			     int score)
/*==================================================================*/
{
    /* �ե졼��Υޥå����٤η׻�(��������ʬ�����) */

    int i, j, local_score;
    int local_m_e = 0;
    int local_m_p = 0;
    int local_c_e = 0;
    int pat_element = 0, dat_element = 0;
    
    local_score = score;

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
    for (i = 0; i < cfp->element_num; i++)
      if (!(cfp->oblig[i] == FALSE && list2->flag[i] == UNASSIGNED))
	pat_element++;

    if (local_m_e < dat_element)
	local_score = -1;
    else if (dat_element == 0 || pat_element == 0 || local_m_e == 0)
	local_score = 0;
    else 
	/* local_score = local_score * sqrt((double)local_m_e)
	   / sqrt((double)dat_element * pat_element);*/

	/* local_score = local_score * local_m_e
	   / (dat_element * sqrt((double)pat_element)); */

	/* local_score = local_score / sqrt((double)pat_element); */

	/* corpus based case analysis 00/01/04 */
	local_score /= 10;	/* ���������ʤ�,����11�� */

    /* corpus based case analysis 00/01/04 */
    /* Ǥ�ճʤˤȤꤢ���� 2�� */
    local_score += (cfd->element_num - dat_element) * 2;


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
    int elmnt_score;
    int i, j;

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
	if (list1.flag[i] == UNASSIGNED &&
	    cfd->pp[i][0] >= 0) {
	    target = i;
	    break;
	}
    }

    /* ��������Ƥ���ʽ���ν��� */
    if (target >= 0) {
	ec_match_flag = 0;
	for (i = 0; i < cfp->element_num; i++) {
	    if (list2.flag[i] == UNASSIGNED) {
		for (j = 0; cfp->pp[i][j] >= 0; j++) {
		    if (cfd->pp[target][0] == cfp->pp[i][j] ||
			(cfd->pp[target][0] == pp_hstr_to_code("�ˤ�ä�") &&
			 cfp->pp[i][j] == pp_kstr_to_code("��"))) {

			elmnt_score = 
			  elmnt_match_score(target, cfd, i, cfp, flag);

			if (flag == EXAMPLE || 
			    (flag == SEMANTIC_MARKER && elmnt_score != 0)) {

			    /* �б��դ��򤷤ơ��Ĥ�γ����Ǥν����˿ʤ� */

			    ec_match_flag = 1;
			    list1.flag[target] = i;
			    list2.flag[i] = target;
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
		if ( cfd->pp[target][0] == cfd->pp[i][0]) {
		    assign_list(cfd, list1, cfp, list2, score, flag);
		    break;
		}
	    }
	}
	return;
    }

    /* ��������Ƥ��ʤ��ʽ���Υ����å� */
    for (i = 0; i < cfd->element_num; i++) {
	if (list1.flag[i] == UNASSIGNED) {
	    target = i;
	    break;
	}
    }

    /* ��������Ƥ��ʤ��ʽ���ν��� */
    if (target >= 0) {
	for (i = 0; i < cfp->element_num; i++) {

	    /* "����" --> "�����򡤤�" �� �б���
	       ���ʸ --> ���ȤǤ��б��� */

	    if (list2.flag[i] == UNASSIGNED &&
		((cfd->pp[target][0] == -1 &&
		  cfp->pp[i][1] == -1 &&
		  (cfp->pp[i][0] == pp_kstr_to_code("��") ||
		   cfp->pp[i][0] == pp_kstr_to_code("��") ||
		   cfp->pp[i][0] == pp_kstr_to_code("��"))) ||
		 (cfd->pp[target][0] == -2))) {
		elmnt_score = elmnt_match_score(target, cfd, i, cfp, flag);
		if (elmnt_score != 0 || flag == EXAMPLE) {
		    list1.flag[target] = i;
		    list2.flag[i] = target;
		    assign_list(cfd, list1, cfp, list2, score + elmnt_score, flag);
		    list2.flag[i] = UNASSIGNED;
		}
	    }
	}
	
	list1.flag[target] = NIL_ASSIGNED;
	assign_list(cfd, list1, cfp, list2, score, flag);
	return;
    } 

    /* ɾ�� : ���٤Ƥ��б��դ������ä���� */
    eval_assign(cfd, &list1, cfp, &list2, score);
}

/*==================================================================*/
void case_frame_match(CASE_FRAME *cfd, CF_MATCH_MGR *cmm_ptr, int flag)
/*==================================================================*/
{
    /* �ʥե졼��Υޥå��� */

    LIST assign_d_list, assign_p_list;
    int i;

    /* ����� */

    Current_max_num = 0;
    Current_max_score = -2;
    Current_max_m_e = 0;
    Current_max_m_p = 0;
    Current_max_c_e = 0;
    for (i = 0; i < cfd->element_num; i++)
      assign_d_list.flag[i] = UNASSIGNED;
    for (i = 0; i < cmm_ptr->cf_ptr->element_num; i++)
      assign_p_list.flag[i] = UNASSIGNED;

    /* ���� */

    assign_list(cfd, assign_d_list, cmm_ptr->cf_ptr, assign_p_list, 0, flag);
					    	/* flag �� or ��̣������ */

    /* ����� */

    if (Current_max_num == MAX_MATCH_MAX)
      fprintf(stderr, "Too many case matching result !\n");

    cmm_ptr->score = Current_max_score;
    cmm_ptr->result_num = Current_max_num;
    for (i = 0; i < Current_max_num; i++)
      cmm_ptr->result_lists_p[i] = Current_max_list2[i];


#ifdef CASE_DEBUG
    print_crrspnd(cfd, cmm_ptr);
#endif
}

/*====================================================================
                               END
====================================================================*/
