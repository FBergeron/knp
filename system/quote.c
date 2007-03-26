/*====================================================================

			     �ó�̤ν���
				   
                                               S.Kurohashi 1996. 4.18
                                               S.Ozaki     1994.12. 1

    $Id$
====================================================================*/
#include "knp.h"

QUOTE_DATA quote_data;

/*==================================================================*/
		  void init_quote(SENTENCE_DATA *sp)
/*==================================================================*/
{
    int i, j;

    for (i = 0; i < QUOTE_MAX; i++) {
	quote_data.in_num[i] = -1;
	quote_data.out_num[i] = -1;    
    }

    for (i = 0; i < sp->Bnst_num; i++) {
	for (j = 0; j < sp->Bnst_num; j++) {
	    Quote_matrix[i][j] = 1;
	    Chi_quote_start_matrix[i][j] = -1;
	    Chi_quote_end_matrix[i][j] = -1;
	}
    }
}

/*==================================================================*/
                         void print_quote()
/*==================================================================*/
{
    int i;

    for (i = 0; quote_data.in_num[i] >= 0; i++) {
	fprintf(Outfp,"Quote_num %d in %d out %d \n", i,
		quote_data.in_num[i], quote_data.out_num[i]);
    }
}

/*==================================================================*/
		  int check_quote(SENTENCE_DATA *sp)
/*==================================================================*/
{
    /*
      "�����֡����ס������֡ߡߡס���"  
      "�����֡����֡ߡߡߡס����ס���" 
      "�����֡�����������������������"  
      "�ߡߡߡߡߡߡߡߡߡߡߡߡס���"  �ʤɤΥѥ�������н�
    */

    int i, k, stack[QUOTE_MAX], s_num, quote_p = FALSE;

    k = 0;
    s_num = -1;

    for (i = 0; i < sp->Bnst_num; i++) {

	if (check_feature(sp->bnst_data[i].f, "��̻�")) {
	    /* �������ۤ��ʤ��������å�(�Ǹ�����Ǥ��ֿͤʤΤǡ�������Ѥ��Ƥ�
	       �����ʤ�) */
	    if (k >= QUOTE_MAX-1) {
		fprintf(stderr, ";; Too many quote (%s) ...\n", sp->Comment ? sp->Comment : "");
		return CONTINUE;
	    }
	    s_num ++;
	    stack[s_num] = k;
	    quote_data.in_num[k] = i;
	    k++;

	    /* �֡ء� �򰷤������Τ��Ȥ򷫤��֤� */
	    if (check_feature(sp->bnst_data[i].f, "��̻ϣ�")) {
		if (k >= QUOTE_MAX-1) {
		    fprintf(stderr, ";; Too many quote (%s) ...\n", sp->Comment ? sp->Comment : "");
		    return CONTINUE;
		}
		s_num ++;
		stack[s_num] = k;
		quote_data.in_num[k] = i;
		k++;
	    }
	}
	if (check_feature(sp->bnst_data[i].f, "��̽�")) {
	    if (s_num == -1) {
		if (k >= QUOTE_MAX-1) {
		    fprintf(stderr, ";; Too many quote (%s) ...\n", sp->Comment ? sp->Comment : "");
		    return CONTINUE;
		}
		quote_data.out_num[k] = i; /* ��̽���¿����� */
		k++;
	    } else {
		quote_data.out_num[stack[s_num]] = i;
		s_num--;
	    }


	    /* �š١� �򰷤������Τ��Ȥ򷫤��֤� */
	    if (check_feature(sp->bnst_data[i].f, "��̽���")) {
		if (s_num == -1) {
		    if (k >= QUOTE_MAX-1) {
			fprintf(stderr, ";; Too many quote (%s) ...\n", sp->Comment ? sp->Comment : "");
			return CONTINUE;
		    }
		    quote_data.out_num[k] = i; /* ��̽���¿����� */
		    k++;
		} else {
		    quote_data.out_num[stack[s_num]] = i;
		    s_num--;
		}
	    }
	}		  
    }

    for (i = 0; i < k; i++) {

	/* ��̤��Ĥ��Ƥ��ʤ�����, ʸƬ�ޤ���ʸ���򶭳��� */	

	if (quote_data.in_num[i] == -1) 
	    quote_data.in_num[i] = 0;
	if (quote_data.out_num[i] == -1)
	    quote_data.out_num[i] = sp->Bnst_num - 1;

	/* ��ʸ��γ�̤��θ���ʤ����
	if (quote_data.in_num[i] != quote_data.out_num[i])
	quote_p = TRUE;
	*/

	quote_p = TRUE;
    }

    return quote_p;
}

/*==================================================================*/
		  void mask_quote(SENTENCE_DATA *sp)
/*==================================================================*/
{
    int i, j, k, l, start, end;

    for (k = 0; quote_data.in_num[k] >= 0; k++) {

	start = quote_data.in_num[k];
	end = quote_data.out_num[k];

	if (start == end) continue;	/* ��ʸ������γ�̤�̵�� */

	/* ��̤ξ�Υޥ��� */

	for (i = 0; i < start; i++) {
	    for (j = start; j < end; j++)
		Quote_matrix[i][j] = 0;

	    /* 
	       �����������ʸ��ˤ�Ϣ��,Ϣ��,�γ�,Ʊ��Ϣ��,�������Τ�
	       �����Ȥ��롥
	       		��) �ֻ�Ρ������ε������פϡġ�

	       �Ѹ���Ϣ�Ѥ����뤳�Ȥ⵩�ˤϤ��뤬�������������̾�ξ���
	       ���ϸ�꤬���̤����ޤ��Τ�̵�뤹�롥
	       		��) ���ब������ˤ��ä��פ��Ȥϡġ�
	    */

	    if (Quote_matrix[i][end] &&
		(check_feature(sp->bnst_data[i].f, "��:Ϣ��") ||
		 check_feature(sp->bnst_data[i].f, "��:Ϣ��") ||
		 check_feature(sp->bnst_data[i].f, "��:�γ�") ||
		 check_feature(sp->bnst_data[i].f, "��:Ʊ��Ϣ��") ||
		 check_feature(sp->bnst_data[i].f, "��:�������")))
		;
	    else 
		Quote_matrix[i][end] = 0;
	}

	/* ��̤α��Υޥ��� */

	for (i = start; i < end; i++)
	    for (j = end + 1; j < sp->Bnst_num; j++)
		Quote_matrix[i][j] = 0;

	/* �����ζ����α���Υޥ��� 
	   (�����α��ϳ����Ƥ��� --> ����ʸ����P�ˤʤ�) */

	for (l = start; l < end; l++)
	    if (check_feature(sp->bnst_data[l].f, "��:ʸ��"))
		for (i = start; i < l; i++)
		    for (j = l + 1; j <= end; j++)
			Quote_matrix[i][j] = 0;
    }
}

/*==================================================================*/
		  void mask_quote_for_chi(SENTENCE_DATA *sp)
/*==================================================================*/
{
    int i, j, k, start, end;

    for (k = 0; quote_data.in_num[k] >= 0; k++) {

	start = quote_data.in_num[k];
	end = quote_data.out_num[k];

	if (start == end) continue;	/* ��ʸ������γ�̤�̵�� */

	/* ��̤ξ�Υޥ��� */

	for (i = 0; i < start; i++) {
	    Quote_matrix[i][start] = 0;
	    Quote_matrix[i][end] = 0;
	}

	/* ��̤α��Υޥ��� */

	for (i = end + 1; i < sp->Bnst_num; i++) {
	    Quote_matrix[start][i] = 0;
	    Quote_matrix[end][i] = 0;
	}	

	for (j = start; j <= end; j++) {
	    for (i = j; i <= end; i++) {
		Chi_quote_start_matrix[j][i] = start;
		Chi_quote_end_matrix[j][i] = end;
	    }
	}
	Chi_quote_start_matrix[start][end] = -1;
	Chi_quote_end_matrix[start][end] = -1;
    }
}

/*==================================================================*/
		     int quote(SENTENCE_DATA *sp)
/*==================================================================*/
{
    int quote_p = FALSE;

    init_quote(sp);

    if ((quote_p = check_quote(sp))) {	/* �ó�̤θ��� */
	if (quote_p == CONTINUE) return quote_p;

	if (OptDisplay == OPT_DEBUG) print_quote();

	if (Language != CHINESE) {
	    mask_quote(sp);			/* ����ν񤭴��� */
	}
	else {
	    mask_quote_for_chi(sp); // mask quote for Chinese
	}
    }

    return quote_p;
}

/*====================================================================
                               END
====================================================================*/
