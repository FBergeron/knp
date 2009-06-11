/*====================================================================

			     �ó�̤ν���
				   
                                               S.Kurohashi 1996. 4.18
                                               S.Ozaki     1994.12. 1

    $Id$

====================================================================*/
#include "knp.h"

QUOTE_DATA quote_data;

#define PAREN_B "��"
#define PAREN_E "��"
#define PAREN_COMMENT_TEMPLATE "��̻�:�� ��̽�:�� ��̰���:"

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

	if ((!OptChiPos && check_feature((sp->bnst_data+start)->f, "PU") && check_feature((sp->bnst_data+end)->f, "PU")) || OptChiPos) {
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

	    Quote_matrix[start][end] = 0;

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
}

/*==================================================================*/
		     int quote(SENTENCE_DATA *sp)
/*==================================================================*/
{
    int quote_p = FALSE;

    init_quote(sp);

   if (Language != CHINESE ||
	(Language == CHINESE && !OptChiGenerative)) {
	if ((quote_p = check_quote(sp))) {	/* �ó�̤θ��� */
	    if (quote_p == CONTINUE) return quote_p;

	    if (OptDisplay == OPT_DEBUG && Language != CHINESE) print_quote();

	    if (Language != CHINESE) {
		mask_quote(sp);			/* ����ν񤭴��� */
	    }
	    else {
		mask_quote_for_chi(sp); // mask quote for Chinese
	    }
	}
    }

    return quote_p;
}

/*==================================================================*/
	void add_comment(SENTENCE_DATA *sp, char *add_string)
/*==================================================================*/
{
    if (sp->Comment) { /* ��¸�Υ����Ȥȷ�� */
	char *orig_comment = sp->Comment;
	sp->Comment = (char *)malloc_data(strlen(sp->Comment) + strlen(add_string) + 2, "add_comment");
	sprintf(sp->Comment, "%s %s", orig_comment, add_string);
	free(orig_comment);
    }
    else { /* �����ʥ����� */
	sp->Comment = strdup(add_string);
    }
}

/*==================================================================*/
 int process_input_paren(SENTENCE_DATA *sp, SENTENCE_DATA **paren_spp)
/*==================================================================*/
{
    int i, j, paren_mrph_num = 0, paren_level = 0, paren_start, *paren_table, paren_num = 0;
    MRPH_DATA  *m_ptr = sp->mrph_data;
    SENTENCE_DATA next_sentence_data;

    paren_table = (int *)malloc_data(sizeof(int) * sp->Mrph_num, "process_input_paren");
    memset(paren_table, 0, sizeof(int) * sp->Mrph_num); /* initialization */

    /* ��̥����å� */
    for (i = 0; i < sp->Mrph_num; i++) {
	if (!strcmp((m_ptr + i)->Goi, PAREN_B)) { /* beginning of parenthesis */
	    if (paren_level == 0) {
		paren_start = i;
	    }
	    paren_level++;
	}
	else if (!strcmp((m_ptr + i)->Goi, PAREN_E)) { /* end of parenthesis */
	    paren_level--;
	    if (paren_level == 0 && i != paren_start + 1) { /* �ʡˤΤ褦����Ȥ��ʤ����Ͻ��� */
		/* ������оݳ��ˤ���? */
		*(paren_table + paren_start) = 'B'; /* beginning */
		*(paren_table + i) = 'E'; /* end */
		paren_mrph_num += 2;
		for (j = paren_start + 1; j < i; j++) {
		    *(paren_table + j) = 'I'; /* intermediate */
		    paren_mrph_num++; /* �����ʬ�η����ǿ� */
		}
		paren_num++; /* ��̿� */
	    }
	}
    }

    if (paren_num == 0 || paren_num >= PAREN_MAX || 
	(paren_start == 0 && *(paren_table + sp->Mrph_num - 1) == 'E')) { /* ���Τ���̤λ����оݳ� */
	return 0;
    }
    else {
	int paren_count = -1, pre_mrph_is_paren = FALSE, char_pos = 0;

	*paren_spp = (SENTENCE_DATA *)malloc_data(sizeof(SENTENCE_DATA) * paren_num, "process_input_paren");

	/* �Ƴ��ʸ */
	for (i = 0; i < paren_num; i++) {
	    (*paren_spp + i)->Mrph_num = 0;
	    (*paren_spp + i)->mrph_data = (MRPH_DATA *)malloc_data(sizeof(MRPH_DATA) * paren_mrph_num, "process_input_paren");
	    (*paren_spp + i)->KNPSID = (char *)malloc_data(strlen(sp->KNPSID) + 4, "process_input_paren");
	    sprintf((*paren_spp + i)->KNPSID, "%s-%02d", sp->KNPSID, i + 2); /* ���ʸ��ID��-02���� */
	    (*paren_spp + i)->Comment = (char *)malloc_data(strlen(PAREN_COMMENT_TEMPLATE) + 4, "process_input_paren");
	    sprintf((*paren_spp + i)->Comment, "%s", PAREN_COMMENT_TEMPLATE);
	}
	strcat(sp->KNPSID, "-01"); /* ��ʸ��ID��-01��Ĥ��� */
	add_comment(sp, "��̺��"); /* ��ʸ�Υ����ȹԤ� */

	/* ��ʸ�ȳ��ʸ��ʬΥ */
	for (i = j = 0; i < sp->Mrph_num; i++) {
	    if (*(paren_table + i) == 0) { /* ��̤ǤϤʤ���ʬ */
		if (i != j) {
		    *(m_ptr + j) = *(m_ptr + i);
		    (m_ptr + j)->num = j;
		    (m_ptr + i)->f = NULL;
		}
		j++;
		pre_mrph_is_paren = FALSE;
	    }
	    else { /* �����ʬ */
		if (pre_mrph_is_paren == FALSE) { /* �����ʬ������ */
		    paren_count++;
		}
		if (*(paren_table + i) == 'B') { /* ��̻� */
		    sprintf((*paren_spp + paren_count)->Comment, "%s%d", (*paren_spp + paren_count)->Comment, char_pos);
		}
		if (*(paren_table + i) == 'I') { /* ������� */
		    *((*paren_spp + paren_count)->mrph_data + (*paren_spp + paren_count)->Mrph_num) = *(m_ptr + i);
		    ((*paren_spp + paren_count)->mrph_data + (*paren_spp + paren_count)->Mrph_num)->num = (*paren_spp + paren_count)->Mrph_num;
		    (*paren_spp + paren_count)->Mrph_num++;
		    (m_ptr + i)->f = NULL;
		}
		pre_mrph_is_paren = TRUE;
	    }
	    char_pos += strlen((m_ptr + i)->Goi2) / BYTES4CHAR;
	}
	sp->Mrph_num -= paren_mrph_num;
	free(paren_table);

	return paren_num;
    }
}

/*==================================================================*/
void prepare_paren_sentence(SENTENCE_DATA *sp, SENTENCE_DATA *paren_sp)
/*==================================================================*/
{
    int i;

    sp->KNPSID = paren_sp->KNPSID;
    sp->Comment = paren_sp->Comment;
    sp->Mrph_num = paren_sp->Mrph_num;
    for (i = 0; i < paren_sp->Mrph_num; i++) {
	*(sp->mrph_data + i) = *(paren_sp->mrph_data + i);
    }
}

/*====================================================================
                               END
====================================================================*/
