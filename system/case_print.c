/*====================================================================

			   �ʹ�¤����: ɽ��

                                               S.Kurohashi 93. 5.31

    $Id$
====================================================================*/
#include "knp.h"

int	EX_PRINT_NUM = 10;
int	PrintFrequency = 0;

/*==================================================================*/
   void print_depend_type(CF_PRED_MGR *cpm_ptr, int num, int flag)
/*==================================================================*/
{
    int i;

    /* �������פν���
       flag == FALSE : �б���ǽ�ʳʤ���Ϥ��ʤ�
    */

    /* ��ά�ΤȤ� */
    if (cpm_ptr->elem_b_num[num] == -2) {
	fputs("�Ծʡ�", Outfp);
	return;
    }
    /* �ȱ��ΤȤ� */
    else if (cpm_ptr->elem_b_num[num] == -3) {
	fputs("�Ծȡ�", Outfp);
	return;
    }
    else if (flag == FALSE && cpm_ptr->elem_b_num[num] == -1) {
	fputs("��--��", Outfp);
	return;
    }

    fputs("��", Outfp);

    if (cpm_ptr->cf.type == CF_PRED) {
	for (i = 0; cpm_ptr->cf.pp[num][i] != END_M; i++) {
	    if (i) {
		fputc('/', Outfp);
	    }
	    if (cpm_ptr->cf.pp[num][i] < 0) {
		fputs("--", Outfp);
	    }
	    else {
		fprintf(Outfp, "%s", pp_code_to_kstr(cpm_ptr->cf.pp[num][i]));
	    }
	}
    }
    else {
	fputs("--", Outfp);
    }

    fputs("��", Outfp);
}

/*==================================================================*/
 void print_data_cframe(CF_PRED_MGR *cpm_ptr, CF_MATCH_MGR *cmm_ptr)
/*==================================================================*/
{
    int i;

    fputs("��", Outfp);

    if (cpm_ptr->result_num > 0 && cmm_ptr->cf_ptr->entry) {
	fprintf(Outfp, "%s", cmm_ptr->cf_ptr->entry);
    }
    else {
	fprintf(Outfp, "%s", cpm_ptr->pred_b_ptr->head_ptr->Goi);
    }

    if (cpm_ptr->cf.voice == VOICE_SHIEKI)
	fputs("(����)��", Outfp);
    else if (cpm_ptr->cf.voice == VOICE_UKEMI)
	fputs("(����)��", Outfp);
    else if (cpm_ptr->cf.voice == VOICE_SHIEKI_UKEMI)
	fputs("(����&����)��", Outfp);
    else if (cpm_ptr->cf.voice == VOICE_MORAU)
	fputs("(��餦)��", Outfp);
    else if (cpm_ptr->cf.voice == VOICE_HOSHII)
	fputs("(�ۤ���)��", Outfp);
    else
	fputs("��", Outfp);

    fprintf(Outfp, " %s [%d]", cpm_ptr->cf.pred_type, 
	    cpm_ptr->pred_b_ptr->cf_num > 1 ? cpm_ptr->pred_b_ptr->cf_num-1 : 1);

    /* �ʥե졼�����ꤷ����ˡ */
    if (cpm_ptr->decided == CF_DECIDED) {
	fputs(" D", Outfp);
    }
    else if (cpm_ptr->decided == CF_CAND_DECIDED) {
	fputs(" C", Outfp);
    }
    else {
	fputs(" U", Outfp);
    }

    for (i = 0; i < cpm_ptr->cf.element_num; i++) {

	fputc(' ', Outfp);
	_print_bnst(cpm_ptr->elem_b_ptr[i]);

	/* �������פν��� */
	print_depend_type(cpm_ptr, i, TRUE);

	/* ��̣�ޡ��� */

	fputc('[', Outfp);

	if (Thesaurus == USE_NTT) {
	    if (cpm_ptr->cf.sm[i][0]) {
		fputs("SM:��", Outfp);
	    }
	    else {
		fputs("SM:��", Outfp);
	    }
	}

	/* ʬ�����ɽ������ */

	else if (Thesaurus == USE_BGH) {
	    if (cpm_ptr->cf.ex[i][0]) {
		fputs("BGH:��", Outfp);
	    }
	    else {
		fputs("BGH:��", Outfp);
	    }
	}

	fputc(']', Outfp);	

	/* Ǥ�ճʤ����Ǥ�ޡ��� */
	if (cpm_ptr->cf.oblig[i] == FALSE) {
	    fputc('*', Outfp);
	}
    }
    fputc('\n', Outfp);
}

struct _sort_kv {
    int	key;
    int	value;
};

/*==================================================================*/
       static int number_compare(const void *i, const void *j)
/*==================================================================*/
{
    /* sort function */
    return ((const struct _sort_kv *)i)->value-((const struct _sort_kv *)j)->value;
}

/*==================================================================*/
   void print_crrspnd(CF_PRED_MGR *cpm_ptr, CF_MATCH_MGR *cmm_ptr)
/*==================================================================*/
{
    int i, j, k, l, num, print_num;
    struct _sort_kv elist[CF_ELEMENT_MAX];

    if (cmm_ptr->cf_ptr->cf_address == -1)	/* �ʥե졼�ब�ʤ���� */
	return;

    /* ����, ��̣��ɽ�� */

    fprintf(Outfp, "��%6.2f�� (%d/%.3f) %s ", 
	    cmm_ptr->score, cmm_ptr->pure_score[0], 
	    sqrt((double)(count_pat_element(cmm_ptr->cf_ptr, 
					    &(cmm_ptr->result_lists_p[0])))), 
	    cmm_ptr->cf_ptr->cf_id);

    if (cmm_ptr->cf_ptr->feature) {
	fprintf(Outfp, "%s ", cmm_ptr->cf_ptr->feature);
    }

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
    else if (cmm_ptr->cf_ptr->voice == FRAME_CAUSATIVE_PASSIVE)
	fprintf(Outfp, "(����&����)");
    else if (cmm_ptr->cf_ptr->voice == FRAME_POSSIBLE)
	fprintf(Outfp, "(��ǽ)");
    else if (cmm_ptr->cf_ptr->voice == FRAME_POLITE)
	fprintf(Outfp, "(º��)");
    else if (cmm_ptr->cf_ptr->voice == FRAME_SPONTANE)
	fprintf(Outfp, "(��ȯ)");

    /* fprintf(Outfp, "%s\n", i_ptr->DATA + i_ptr->imi); */
    fputs("-----------------------------------\n", Outfp);

    /* �������б���ɽ�� */

    for (k = 0; k < cmm_ptr->result_num; k++) {
	if (k != 0)
	    fputs("---\n", Outfp);

	/* �ʤ򥽡��Ȥ��ƽ��� */
	for (i = 0; i < cmm_ptr->cf_ptr->element_num; i++) {
	    elist[i].key = i;
	    elist[i].value = cmm_ptr->cf_ptr->pp[i][0];
	}
	qsort(elist, cmm_ptr->cf_ptr->element_num, sizeof(struct _sort_kv), number_compare);

	for (l = 0; l < cmm_ptr->cf_ptr->element_num; l++) {
	    i = elist[l].key;
	    num = cmm_ptr->result_lists_p[k].flag[i];

	    if (cmm_ptr->cf_ptr->adjacent[i] == TRUE)
		fputs(" �� ", Outfp);
	    else
		fputs(" �� ", Outfp);

	    if (num == UNASSIGNED || cmm_ptr->score == -2) { /* -2�����Τ��԰��� */
		fputs("--", Outfp);
	    }
	    else {
		_print_bnst(cpm_ptr->elem_b_ptr[num]);

		/* �������פν��� */
		print_depend_type(cpm_ptr, num, FALSE);

		if (num != UNASSIGNED && cpm_ptr->cf.oblig[num] == FALSE)
		    fputc('*', Outfp);

		/* �ʤ��ȤΥ�������ɽ�� */
		if (cmm_ptr->result_lists_p[k].score[i] >= 0)
		    fprintf(Outfp, "��%2d����", cmm_ptr->result_lists_p[k].score[i]);
	    }

	    fprintf(Outfp, " : ��");

	    if (cmm_ptr->cf_ptr->type == CF_PRED) {
		for (j = 0; cmm_ptr->cf_ptr->pp[i][j] != END_M; j++) {
		    if (j != 0) fputc('/', Outfp);
		    fprintf(Outfp, "%s", pp_code_to_kstr(cmm_ptr->cf_ptr->pp[i][j]));
		}
	    }
	    else {
		fprintf(Outfp, "%s", cmm_ptr->cf_ptr->pp_str[i]);
	    }

	    fprintf(Outfp, "��");

	    /* ����ν��� */
	    if (cmm_ptr->cf_ptr->ex_list[i]) {
		print_num = EX_PRINT_NUM < 0 ? cmm_ptr->cf_ptr->ex_num[i] : 
		    cmm_ptr->cf_ptr->ex_num[i] > EX_PRINT_NUM ? 
		    EX_PRINT_NUM : cmm_ptr->cf_ptr->ex_num[i];
		fputc('(', Outfp);
		for (j = 0; j < print_num; j++) {
		    if (j != 0) fputc('/', Outfp);
		    if (j == cmm_ptr->result_lists_p[k].pos[i]) fputs("��", Outfp);
		    if (PrintFrequency) {
			fprintf(Outfp, "%s:%d", cmm_ptr->cf_ptr->ex_list[i][j], cmm_ptr->cf_ptr->ex_freq[i][j]);
		    }
		    else {
			fprintf(Outfp, "%s", cmm_ptr->cf_ptr->ex_list[i][j]);
		    }
		    if (j == cmm_ptr->result_lists_p[k].pos[i]) fputs("��", Outfp);
		}
		if (cmm_ptr->result_lists_p[k].pos[i] >= print_num) {
		    fputs("/��", Outfp);
		    if (PrintFrequency) {
			fprintf(Outfp, "%s:%d", cmm_ptr->cf_ptr->ex_list[i][cmm_ptr->result_lists_p[k].pos[i]], 
				cmm_ptr->cf_ptr->ex_freq[i][cmm_ptr->result_lists_p[k].pos[i]]);
		    }
		    else {
			fprintf(Outfp, "%s", cmm_ptr->cf_ptr->ex_list[i][cmm_ptr->result_lists_p[k].pos[i]]);
		    }
		    fputs("��", Outfp);
		}
		if (print_num != cmm_ptr->cf_ptr->ex_num[i])
		    fputs("...", Outfp);
		fputc(')', Outfp);
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
				   CF_MATCH_MGR *cmm_ptr, int ipal_num)
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
	      void print_case_result(SENTENCE_DATA *sp)
/*==================================================================*/
{
    int i, j;
    TOTAL_MGR *tm = sp->Best_mgr;

    fputs("<Case Structure Analysis Data>\n", Outfp);
    fprintf(Outfp, "�� %d Score:%d, Dflt:%d, Possibility:%d/%d ��\n", 
	    sp->Sen_num, tm->score, tm->dflt, tm->pssb+1, 1);

    /* �嵭���ϤκǸ�ΰ���(��¸��¤�ο�)��1�ˤ��Ƥ��롥
       �����Ȱ��äƤʤ� */

    for (i = tm->pred_num - 1; i >= 0; i--) {
	if (i != tm->pred_num - 1) {
	    fputc('\n', Outfp);
	}
	print_data_cframe(&(tm->cpm[i]), &(tm->cpm[i].cmm[0]));
	for (j = 0; j < tm->cpm[i].result_num; j++) {
	    if (OptEllipsis) {
		print_crrspnd(tm->cpm[i].cmm[j].cpm ? tm->cpm[i].cmm[j].cpm : &(tm->cpm[i]), 
			      &(tm->cpm[i].cmm[j]));
		free(tm->cpm[i].cmm[j].cpm);
	    }
	    else {
		print_crrspnd(&(tm->cpm[i]), &(tm->cpm[i].cmm[j]));
	    }
	}
    }
    fputs("</Case Structure Analysis Data>\n", Outfp);
}

/*==================================================================*/
	      void print_pa_structure(SENTENCE_DATA *sp)
/*==================================================================*/
{
    int p, i, num;
    CF_PRED_MGR *cpm_ptr;
    char relation[DATA_LEN], *word;

    /* ��������֤ˡ�Predicate-Argument Structure ����� */

    for (p = sp->Best_mgr->pred_num - 1; p >= 0; p--) {
	cpm_ptr = &(sp->Best_mgr->cpm[p]);
	fprintf(Outfp, "%2d %s", sp->Best_mgr->pred_num-1-p, cpm_ptr->pred_b_ptr->head_ptr->Goi);

	/* ����¦�γƳ����Ǥε��� */
	for (i = 0; i < cpm_ptr->cf.element_num; i++) {
	    /* �ؼ���β��Ϥ򤹤���ϡ��ؼ������� */
	    if ((OptEllipsis & OPT_DEMO) && 
		check_feature(cpm_ptr->elem_b_ptr[i]->f, "��ά�����оݻؼ���")) {
		continue;
	    }

	    num = cpm_ptr->cmm[0].result_lists_d[0].flag[i];

	    /* ������Ƥʤ� */
	    if (num == NIL_ASSIGNED) {
		/* ������Ƥʤ�����������¦�γʤ���������Ƥ�����Ϥ����ɽ��
		   (�ʤβ�ǽ���ϤҤȤĤ����ʤ���̤�ʰʳ�) */
		/* ���֤ءפⰷ������ (���ߤϡ֤�/�ˡפȤʤäƤ���) */
		if (cpm_ptr->cf.pp[i][1] == END_M && 
		    cpm_ptr->cf.pp[i][0] >= 0) {
		    strcpy(relation, pp_code_to_kstr(cpm_ptr->cf.pp[i][0]));
		}
		else {
		    continue;
		}
	    }
	    /* ������Ƥ��Ƥ���� */
	    else if (num >= 0) {
		strcpy(relation, pp_code_to_kstr(cpm_ptr->cmm[0].cf_ptr->pp[num][0]));
	    }

	    word = make_print_string(cpm_ptr->elem_b_ptr[i], 0);
	    if (word) {
		/* ��ά�ξ��� * ����Ϳ */
		fprintf(Outfp, " %s:%s%s", word, relation, cpm_ptr->elem_b_num[i] <= -2 ? "*" : "");
		free(word);
	    }
	}
	fputc('\n', Outfp);
    }
    fputs("EOS\n", Outfp);
}

/*====================================================================
                               END
====================================================================*/
