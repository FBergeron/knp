/*====================================================================

			   �ʹ�¤����: ɽ��

                                               S.Kurohashi 93. 5.31

    $Id$
====================================================================*/
#include "knp.h"

extern FILE  *Infp;
extern FILE  *Outfp;

/*==================================================================*/
	      void print_data_cframe(CF_PRED_MGR *cpm_ptr)
/*==================================================================*/
{
    int i, j;
    char *cp;
    char setubi_buffer[256];

    if (cpm_ptr->cf.voice == VOICE_SHIEKI)
      fprintf(Outfp, "��%s(����)��", cpm_ptr->pred_b_ptr->Jiritu_Go);
    else if (cpm_ptr->cf.voice == VOICE_UKEMI)
      fprintf(Outfp, "��%s(����)��", cpm_ptr->pred_b_ptr->Jiritu_Go);
    else if (cpm_ptr->cf.voice == VOICE_MORAU)
      fprintf(Outfp, "��%s(����or����)��", cpm_ptr->pred_b_ptr->Jiritu_Go);
    else
      fprintf(Outfp, "��%s��", cpm_ptr->pred_b_ptr->Jiritu_Go);

    for (i = 0; i < cpm_ptr->cf.element_num; i++) {

	fprintf(Outfp, " ");
	_print_bnst(cpm_ptr->elem_b_ptr[i]);

	/* �ʤν��� */
	if ((cp = (char *)check_feature(cpm_ptr->elem_b_ptr[i]->f, "��"))
	    != NULL) {
	    if (cpm_ptr->cf.pp[i][0] < 0) {
		fprintf(Outfp, "(%s)", cp + strlen("��:"));
	    }
	    else {
		fprintf(Outfp, "(%s)", pp_code_to_kstr(cpm_ptr->cf.pp[i][0]));
	    }
	}

	/*
	fprintf(Outfp, " %s%s", cpm_ptr->elem_b_ptr[i]->Jiritu_Go,
		meishi_setubi(cpm_ptr->elem_b_ptr[i], setubi_buffer));
		*/
		
	fprintf(Outfp, "[");

	/* ��̣�ޡ��� */

	if (cpm_ptr->cf.sm[i][0]) {
	    fprintf(Outfp, "SM:��");
	    /*
	    for (j = 0; cpm_ptr->cf.sm[i][j]; j+=SM_CODE_SIZE) {
		if (j != 0) fprintf(Outfp,  "/");
		fprintf(Outfp, "%12.12s", &(cpm_ptr->cf.sm[i][j]));
	    }
	    */
	}
	else {
	    fprintf(Outfp, "SM:��");
	}

	/* ʬ�����ɽ������ */

	if (cpm_ptr->cf.ex[i][0]) {
	    fprintf(Outfp, "BGH:��");
	    /*
	    for (j = 0; cpm_ptr->cf.ex[i][j]; j+=BGH_CODE_SIZE) {
		if (j != 0) fprintf(Outfp,  "/");
		fprintf(Outfp, "%7.7s", &(cpm_ptr->cf.ex[i][j]));
	    }
	    */
	}
	else {
	    fprintf(Outfp, "BGH:��");
	}

	fprintf(Outfp, "]");	

	/*
	if (cpm_ptr->cf.pp[i][0] >= 0) 
	  fprintf(Outfp, "%s", pp_code_to_hstr(cpm_ptr->cf.pp[i][0]));
	else if (cpm_ptr->elem_b_ptr[i]->fuzoku_ptr)
	  fprintf(Outfp, "(%s)", (cpm_ptr->elem_b_ptr[i]->fuzoku_ptr +
				   cpm_ptr->elem_b_ptr[i]->fuzoku_num-1)->Goi);
	else
	  fprintf(Outfp, "(��)");
	  */
	
	if (cpm_ptr->cf.oblig[i] == FALSE)
	  fprintf(Outfp, "*");
    }
    fputc('\n', Outfp);
}

/*==================================================================*/
   void print_crrspnd(CF_PRED_MGR *cpm_ptr, CF_MATCH_MGR *cmm_ptr)
/*==================================================================*/
{
    int i, j, k, num;
    int match_num, max_match_num, max_match_nth;
    char *cp;
    char setubi_buffer[256];
    char match_code_buffer[16];
    CASE_FRAME *cfd = &(cpm_ptr->cf);
    IPAL_FRAME Ipal_frame;
    IPAL_FRAME *i_ptr = &Ipal_frame;

    if (cmm_ptr->cf_ptr->ipal_address == -1)	/* IPAL�ˤʤ���� */
      return;
    else
      get_ipal_frame(i_ptr, cmm_ptr->cf_ptr->ipal_address);
    
    /* ��������̣��ɽ�� */

    fprintf(Outfp, "��%3d�� ", cmm_ptr->score);

    if (cmm_ptr->cf_ptr->voice == FRAME_PASSIVE_I)
      fprintf(Outfp, "(�ּ�)");
    else if (cmm_ptr->cf_ptr->voice == FRAME_PASSIVE_1 ||
	     cmm_ptr->cf_ptr->voice == FRAME_PASSIVE_2)
      fprintf(Outfp, "(ľ��)");
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

    fprintf(Outfp, "%s\n", i_ptr->DATA+i_ptr->imi);
    /* i_ptr->DATA+i_ptr->bunrei1 */

    /* �������б���ɽ�� */

    for (i = 0; i < cmm_ptr->cf_ptr->element_num; i++) {
	num = cmm_ptr->result_lists_p[0].flag[i];
	if (num == UNASSIGNED || cmm_ptr->score == -2) { /* -2�����Τ��԰��� */
	    fprintf(Outfp, " �� --");
	}
	else {
	    fprintf(Outfp, " �� ");
	    _print_bnst(cpm_ptr->elem_b_ptr[num]);
	    if ((cp = (char *)check_feature(cpm_ptr->elem_b_ptr[num]->f, "��"))
		!= NULL) {
		fprintf(Outfp, "(%s)", cp + strlen("��:"));
	    }

	    /*
	    fprintf(Outfp," %s%s", cpm_ptr->elem_b_ptr[num]->Jiritu_Go, 
		    meishi_setubi(cpm_ptr->elem_b_ptr[num], setubi_buffer));
	    if (cfd->pp[num][0] >= 0) 
	      fprintf(Outfp, "%s", pp_code_to_hstr(cfd->pp[num][0]));
	    else if (l_fuzoku_goi(cpm_ptr->elem_b_ptr[num]))
	      fprintf(Outfp, "(%s)", l_fuzoku_goi(cpm_ptr->elem_b_ptr[num]));
	    else
	      fprintf(Outfp, "(��)");
	    */

	    if (num != UNASSIGNED && cfd->oblig[num] == FALSE)
	      fprintf(Outfp, "*");

	    /* ����ˤ����Ϥξ��
	       ����ޥå��Υ����ɤ���� 

	    max_match_num = -1;
	    max_match_nth = -1;
	    for (j = 0; cfd->ex[num][j]; j+=10) {
		for (k = 0; cmm_ptr->cf_ptr->ex[i][k]; k+=10) {
		    match_num = _ex_match_score(cfd->ex[num]+j, 
						cmm_ptr->cf_ptr->ex[i]+k);
		    if (match_num > max_match_num) {
			max_match_num = match_num;
			max_match_nth = j;
		    }
		}
	    }
	    if (max_match_num >= 6) max_match_num = 7;
	    
	    fprintf(Outfp, " [");
	    for (j = 0; cfd->ex[num][j]; j+=10) {
		if (j != 0) fprintf(Outfp, "/");
		if (j == max_match_nth) {
		    for (k = 0; k < 7; k++) {
			fputc(cfd->ex[num][j+k], Outfp);
			if (k + 1 == max_match_num)
			  fputc('*', Outfp);
		    }
		} else
		  fprintf(Outfp, "%7.7s", &(cfd->ex[num][j]));
	    }
	    fprintf(Outfp, "]");
	    */

	    /* ��̣�Ǥˤ����Ϥξ��
	       �Ȥꤢ��������񤫤ʤ�
	     */
	}
	
	fprintf(Outfp, " : ");
	
	for (j = 0; cmm_ptr->cf_ptr->pp[i][j]!= -1; j++) {
	    if (j != 0) fprintf(Outfp,  "/");
	    fprintf(Outfp, "%s", pp_code_to_kstr(cmm_ptr->cf_ptr->pp[i][j]));
	}

	if (cmm_ptr->cf_ptr->voice == FRAME_PASSIVE_I ||
	    cmm_ptr->cf_ptr->voice == FRAME_CAUSATIVE_WO_NI ||
	    cmm_ptr->cf_ptr->voice == FRAME_CAUSATIVE_WO ||
	    cmm_ptr->cf_ptr->voice == FRAME_CAUSATIVE_NI) {
	    if (i == 0)
	      fprintf(Outfp, "(��)");
	    else
	      fprintf(Outfp, "(%s)", 
		      i_ptr->DATA+i_ptr->meishiku[i-1]);
	} else
	  fprintf(Outfp, "(%s)", i_ptr->DATA+i_ptr->meishiku[i]);
	  
	if (cmm_ptr->cf_ptr->oblig[i] == FALSE)
	    fprintf(Outfp, "*");

	/* ����ˤ����Ϥξ��
	fprintf(Outfp, " [");
	for (j = 0; cmm_ptr->cf_ptr->ex[i][j]; j+=10) {
	    for (k = 0; k < j; k++)
	      if (!strncmp(cmm_ptr->cf_ptr->ex[i]+j, cmm_ptr->cf_ptr->ex[i]+k, 5))
		goto SAME_BREAK_2;
	    if (j != 0) fprintf(Outfp, "/");
	    fprintf(Outfp, "%5.5s", cmm_ptr->cf_ptr->ex[i]+j);
	  SAME_BREAK_2:
	}
	fprintf(Outfp, "]");
	*/

	/* ��̣�Ǥˤ����Ϥξ�� */
	fprintf(Outfp, "[%s]", i_ptr->DATA+i_ptr->imisosei[i]);

	/* ��̣�ǥ����ɤ�񤯾��
	fprintf(Outfp, " [");
	for (j = 0; cmm_ptr->cf_ptr->sm[i][j]; j+=12) {
	    for (k = 0; k < j; k++)
	      if (!strncmp(cmm_ptr->cf_ptr->sm[i]+j, cmm_ptr->cf_ptr->sm[i]+k, 5))
		goto SAME_BREAK_2;
	    if (j != 0) fprintf(Outfp, "/");
	    fprintf(Outfp, "%5.5s", cmm_ptr->cf_ptr->sm[i]+j);
	  SAME_BREAK_2:
	}
	fprintf(Outfp, "]");
	*/

	fprintf(Outfp, "\n");
    }

}

/*==================================================================*/
	  void print_good_crrspnds(CF_PRED_MGR *cpm_ptr,
				   CF_MATCH_MGR *cmm_ptr,int ipal_num)
/*==================================================================*/
{
    int i, j, check[IPAL_FRAME_MAX*3];
    int max_num, max_score, max_counts, all_max_score = 0;
    int print_flag = TRUE;
    
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
}

/*====================================================================
                               END
====================================================================*/
