/*====================================================================

			   �ʹ�¤����: ɽ��

                                               S.Kurohashi 93. 5.31

    $Id$
====================================================================*/
#include "knp.h"

/*==================================================================*/
	void print_depend_type(CF_PRED_MGR *cpm_ptr, int num)
/*==================================================================*/
{
    char *cp;
    int i;

    /* �������פν��� */
    if ((cp = (char *)check_feature(cpm_ptr->elem_b_ptr[num]->f, "��")) != NULL) {
	fprintf(Outfp, "��");
	for (i = 0; cpm_ptr->cf.pp[num][i] != END_M; i++) {
	    if (i) {
		fputc('/', Outfp);
	    }
	    if (cpm_ptr->cf.pp[num][i] < 0) {
		fprintf(Outfp, "--");
	    }
	    else {
		fprintf(Outfp, "%s", pp_code_to_kstr(cpm_ptr->cf.pp[num][i]));
	    }
	}
	fprintf(Outfp, "��");
    }
}

/*==================================================================*/
	      void print_data_cframe(CF_PRED_MGR *cpm_ptr)
/*==================================================================*/
{
    int i;

    if (cpm_ptr->cf.voice == VOICE_SHIEKI)
	fprintf(Outfp, "��%s(����)��", L_Jiritu_M(cpm_ptr->pred_b_ptr)->Goi);
    else if (cpm_ptr->cf.voice == VOICE_UKEMI)
	fprintf(Outfp, "��%s(����)��", L_Jiritu_M(cpm_ptr->pred_b_ptr)->Goi);
    else if (cpm_ptr->cf.voice == VOICE_MORAU)
	fprintf(Outfp, "��%s(����or����)��", L_Jiritu_M(cpm_ptr->pred_b_ptr)->Goi);
    else
	fprintf(Outfp, "��%s��", L_Jiritu_M(cpm_ptr->pred_b_ptr)->Goi);

    fprintf(Outfp, " [%d]", cpm_ptr->pred_b_ptr->cf_num);

    for (i = 0; i < cpm_ptr->cf.element_num; i++) {

	fputc(' ', Outfp);
	_print_bnst(cpm_ptr->elem_b_ptr[i]);

	/* �������פν��� */
	print_depend_type(cpm_ptr, i);

	/* ��̣�ޡ��� */

	fputc('[', Outfp);

	if (cpm_ptr->cf.sm[i][0]) {
	    fputs("SM:��", Outfp);
	}
	else {
	    fputs("SM:��", Outfp);
	}

	/* ʬ�����ɽ������ */

	if (Thesaurus == USE_BGH) {
	    if (cpm_ptr->cf.ex[i][0]) {
		fputs("BGH:��", Outfp);
	    }
	    else {
		fputs("BGH:��", Outfp);
	    }
	}

	fputc(']', Outfp);	

	if (cpm_ptr->cf.oblig[i] == FALSE)
	    fputc('*', Outfp);
    }
    fputc('\n', Outfp);
}

/*==================================================================*/
   void print_crrspnd(CF_PRED_MGR *cpm_ptr, CF_MATCH_MGR *cmm_ptr)
/*==================================================================*/
{
    int i, j, k, num;

    if (cmm_ptr->cf_ptr->ipal_address == -1)	/* IPAL�ˤʤ���� */
	return;
    
    /* ����, ��̣��ɽ�� */

    fprintf(Outfp, "��%3d�� ", cmm_ptr->score);

    if (cmm_ptr->cf_ptr->concatenated_flag == 1)
	fprintf(Outfp, "<ʸ����ե졼��:%s> ", cmm_ptr->cf_ptr->ipal_id);

    if (cmm_ptr->cf_ptr->voice == FRAME_PASSIVE_I)
	fprintf(Outfp, "(�ּ�)");
    else if (cmm_ptr->cf_ptr->voice == FRAME_PASSIVE_1)
	fprintf(Outfp, "(ľ��1)");
    else if (cmm_ptr->cf_ptr->voice == FRAME_PASSIVE_2)
	fprintf(Outfp, "(ľ��2)");
    else if (cmm_ptr->cf_ptr->voice == FRAME_CAUSATIVE_WO_NI ||
	     cmm_ptr->cf_ptr->voice == FRAME_CAUSATIVE_WO ||
             cmm_ptr->cf_ptr->voice == FRAME_CAUSATIVE_NI)
	fprintf(Outfp, "(����)");
    else if (cmm_ptr->cf_ptr->voice == FRAME_POSSIBLE)
	fprintf(Outfp, "(��ǽ)");
    else if (cmm_ptr->cf_ptr->voice == FRAME_POLITE)
	fprintf(Outfp, "(º��)");
    else if (cmm_ptr->cf_ptr->voice == FRAME_SPONTANE)
	fprintf(Outfp, "(��ȯ)");

    /* fprintf(Outfp, "%s\n", i_ptr->DATA+i_ptr->imi); */
    fputc('\n', Outfp);

    /* �������б���ɽ�� */

    for (k = 0; k < cmm_ptr->result_num; k++) {
	if (k != 0)
	    fputs("---\n", Outfp);
    for (i = 0; i < cmm_ptr->cf_ptr->element_num; i++) {
	num = cmm_ptr->result_lists_p[k].flag[i];
	if (num == UNASSIGNED || cmm_ptr->score == -2) { /* -2�����Τ��԰��� */
	    fprintf(Outfp, " �� --");
	}
	else {
	    fprintf(Outfp, " �� ");
	    _print_bnst(cpm_ptr->elem_b_ptr[num]);

	    /* �������פν��� */
	    print_depend_type(cpm_ptr, num);

	    if (num != UNASSIGNED && cpm_ptr->cf.oblig[num] == FALSE)
		fputc('*', Outfp);

	    /* �ʤ��ȤΥ�������ɽ�� */
	    if (cmm_ptr->result_lists_p[k].score[i] >= 0)
		fprintf(Outfp, "��%2d����", cmm_ptr->result_lists_p[k].score[i]);
	}

	fprintf(Outfp, " : ��");

	for (j = 0; cmm_ptr->cf_ptr->pp[i][j]!= END_M; j++) {
	    if (j != 0) fputc('/', Outfp);
	    fprintf(Outfp, "%s", pp_code_to_kstr(cmm_ptr->cf_ptr->pp[i][j]));
	}
	fprintf(Outfp, "��");

	/* ����ν��� */
	if (cmm_ptr->cf_ptr->voice == FRAME_PASSIVE_I ||
	    cmm_ptr->cf_ptr->voice == FRAME_CAUSATIVE_WO_NI ||
	    cmm_ptr->cf_ptr->voice == FRAME_CAUSATIVE_WO ||
	    cmm_ptr->cf_ptr->voice == FRAME_CAUSATIVE_NI) {
	    if (i == 0)
		fprintf(Outfp, "(��)");
	    else if (cmm_ptr->cf_ptr->examples[i])
		fprintf(Outfp, "(%s)", 
			cmm_ptr->cf_ptr->examples[i]);
	} else if (cmm_ptr->cf_ptr->examples[i]) {
	    fprintf(Outfp, "(%s)", cmm_ptr->cf_ptr->examples[i]);
	}
	  
	if (cmm_ptr->cf_ptr->oblig[i] == FALSE)
	    fputc('*', Outfp);

	/* ��̣�Ǥν��� */
	if (cmm_ptr->cf_ptr->semantics[i]) {
	    fprintf(Outfp, "[%s]", cmm_ptr->cf_ptr->semantics[i]);
	}

	fputc('\n', Outfp);
    }
}
}

/*==================================================================*/
	  void print_good_crrspnds(CF_PRED_MGR *cpm_ptr,
				   CF_MATCH_MGR *cmm_ptr,int ipal_num)
/*==================================================================*/
{
    int i, j, *check;
    int max_num, max_score, max_counts, all_max_score = 0;

    check = (int *)malloc_data(sizeof(int)*ipal_num, "print_good_crrspnds");
    for (i = 0; i < ipal_num; i++) check[i] = 1;
    for (i = 0; i < ipal_num; i++) {
	max_num = -1;
	max_score = -10;	/* case_analysis �Ǥ� -1 �λ������� */
	for (j = 0; j < ipal_num; j++) {
	    if (check[j] && (cmm_ptr+j)->score > max_score) {
		max_score = (cmm_ptr+j)->score;
		max_num = j;
		max_counts = 1;
	    }
	    else if (check[j] && (cmm_ptr+j)->score == max_score) {
		max_counts ++;
	    }
	}
	if (i == 0) all_max_score = max_score;

	/* ɽ������߾��
	if (OptDisplay == OPT_NORMAL || OptDisplay == OPT_DETAIL) {
	    if (max_score != all_max_score && i >= 3) 
		break;
	}
	*/

	print_crrspnd(cpm_ptr, cmm_ptr+max_num);
	check[max_num] = 0;
    }
    free(check);
}

/*==================================================================*/
		       void print_case_result()
/*==================================================================*/
{
    int i, j;
    TOTAL_MGR *tm = &Best_mgr;

    fprintf(Outfp, "�� %d Score:%d, Dflt:%d, Possibility:%d/%d ��\n", 
	    sp->Sen_num, tm->score, tm->dflt, tm->pssb+1, 1);

    /* �嵭���ϤκǸ�ΰ���(��¸��¤�ο�)��1�ˤ��Ƥ��롥
       �����Ȱ��äƤʤ� */

    for (i = tm->pred_num-1; i >= 0; i--) {
	print_data_cframe(&(tm->cpm[i]));
	for (j = 0; j < tm->cpm[i].result_num; j++) {
	    print_crrspnd(&(tm->cpm[i]), &(tm->cpm[i].cmm[j]));
	}
    }
}

/*====================================================================
                               END
====================================================================*/
