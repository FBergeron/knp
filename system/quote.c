/*====================================================================

			     �ó�̤ν���
				   
                                               S.Kurohashi 1996. 4.18
                                               S.Ozaki     1994.12. 1

    $Id$
====================================================================*/
#include "knp.h"

QUOTE_DATA quote_data;

/*==================================================================*/
                         void init_quote()
/*==================================================================*/
{
    int i, j;

    for (i = 0; i < QUOTE_MAX; i++) {
	quote_data.in_num[i] = -1;
	quote_data.out_num[i] = -1;    
    }

    for (i = 0; i < Bnst_num; i++)
      for (j = 0; j < Bnst_num; j++)
	Quote_matrix[i][j] = 1;
}

/*==================================================================*/
                         void print_quote()
/*==================================================================*/
{
    int i;

    for (i = 0; quote_data.in_num[i] >= 0; i++) {
	fprintf(stdout,"Quote_num %d in %d out %d \n", i,
		quote_data.in_num[i], quote_data.out_num[i]);
    }
}

/*==================================================================*/
                         int check_quote() 
/*==================================================================*/
{
    /*
      "�����֡����ס������֡ߡߡס���"  
      "�����֡����֡ߡߡߡס����ס���" 
      "�����֡�����������������������"  
      "�ߡߡߡߡߡߡߡߡߡߡߡߡס���"  �ʤɤΥѥ�������н�
    */

    int i, j, k, stack[QUOTE_MAX], s_num, quote_p = FALSE;

    k = 0;
    s_num = -1;

    for (i = 0; i < Bnst_num; i++) {

	if (check_feature(bnst_data[i].f, "��̻�")) {
	    /* �������ۤ��ʤ��������å�(�Ǹ�����Ǥ��ֿͤʤΤǡ�������Ѥ��Ƥ�
	       �����ʤ�) */
	    if (k >= QUOTE_MAX-1) {
		fprintf(stderr, "Too many quote (%s) ...\n", Comment);
		return CONTINUE;
	    }
	    s_num ++;
	    stack[s_num] = k;
	    quote_data.in_num[k] = i;
	    k++;
	}
	if (check_feature(bnst_data[i].f, "��̽�")) {
	    if (s_num == -1) {
		quote_data.out_num[k] = i; /* ��̽���¿����� */
		k++;
	    } else {
		quote_data.out_num[stack[s_num]] = i;
		s_num--;
	    }
	}		  
    }

    for (i = 0; i < k; i++) {

	/* ��̤��Ĥ��Ƥ��ʤ�����, ʸƬ�ޤ���ʸ���򶭳��� */	

	if (quote_data.in_num[i] == -1) 
	    quote_data.in_num[i] = 0;
	if (quote_data.out_num[i] == -1)
	    quote_data.out_num[i] = Bnst_num - 1;

	/* ��ʸ��γ�̤��θ���ʤ����
	if (quote_data.in_num[i] != quote_data.out_num[i])
	quote_p = TRUE;
	*/

	quote_p = TRUE;
    }

    return quote_p;
}

/*==================================================================*/
                         void mask_quote() 
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
	       �����������ʸ��ˤ�Ϣ��,Ϣ��,�γ�,Ʊ��Ϣ�ΤΤ߷����Ȥ��롥
	       		��) �ֻ�Ρ������ε������פϡġ�

	       �Ѹ���Ϣ�Ѥ����뤳�Ȥ⵩�ˤϤ��뤬�������������̾�ξ���
	       ���ϸ�꤬���̤����ޤ��Τ�̵�뤹�롥
	       		��) ���ब������ˤ��ä��פ��Ȥϡġ�
	    */

	    if (Quote_matrix[i][end] &&
		(check_feature(bnst_data[i].f, "��:Ϣ��") ||
		 check_feature(bnst_data[i].f, "��:Ϣ��") ||
		 check_feature(bnst_data[i].f, "��:�γ�") ||
		 check_feature(bnst_data[i].f, "��:Ʊ��Ϣ��")))
		;
	    else 
		Quote_matrix[i][end] = 0;
	}

	/* ��̤α��Υޥ��� */

	for (i = start; i < end; i++)
	    for (j = end + 1; j < Bnst_num; j++)
		Quote_matrix[i][j] = 0;

	/* �����ζ����α���Υޥ��� 
	   (�����α��ϳ����Ƥ��� --> ����ʸ����P�ˤʤ�) */

	for (i = start; i < end; i++)
	    if (check_feature(bnst_data[i].f, "��:ʸ��"))
		for (j = start; j < i; j++)
		    for (l = i+1; l <= end; l++)
			Quote_matrix[j][l] = 0;
    }
}

/*==================================================================*/
                         int quote() 
/*==================================================================*/
{
    int quote_p = FALSE;

    init_quote();

    if (quote_p = check_quote()) {	/* �ó�̤θ��� */
	if (quote_p == CONTINUE) return quote_p;

	if (OptDisplay == OPT_DEBUG) print_quote();

	mask_quote();			/* ����ν񤭴��� */
    }

    return quote_p;
}

/*====================================================================
                               END
====================================================================*/
