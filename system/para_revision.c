/*====================================================================

		      ����¤�֤δط��ˤ�뽤��

                                               S.Kurohashi 91.10.17
                                               S.Kurohashi 93. 5.31

    $Id$
====================================================================*/
#include "knp.h"

#define REVISE_SPRE	1
#define REVISE_SPOS	2
#define REVISE_PRE	3
#define REVISE_POS	4

extern int D_check_array[BNST_MAX];
extern int D_found_array[BNST_MAX];

static int judge_matrix[4][4] = {
    {1, 1, 0, 1},
    {1, 1, 0, 1},
    {1, 1, 0, 0},
    {1, 1, 0, 0}
};
static int judge_matrix_pos_str[4][4] = { /* �夬������ */
    {0, 0, 0, 1},
    {1, 1, 0, 1},
    {1, 1, 0, 0},
    {1, 1, 0, 0}
};
static int judge_matrix_pre_str[4][4] = { /* ���������� */
    {0, 1, 0, 1},
    {0, 1, 0, 1},
    {0, 0, 0, 0},
    {1, 1, 0, 0}
};

/*==================================================================*/
      void print_restrict_matrix(SENTENCE_DATA *sp, int key_pos)
/*==================================================================*/
{
    int i, j;
    
    fprintf(Outfp, "<< restrict matrix >>\n");	
    for ( i=0; i<=key_pos; i++ ) {
	for ( j=key_pos+1; j<sp->Bnst_num; j++ )
	    fprintf(Outfp, "%3d", restrict_matrix[i][j]);
	fputc('\n', Outfp);
    }
}

/*==================================================================*/
	void set_restrict_matrix(SENTENCE_DATA *sp, 
				 int a1, int a2, int a3, 
				 int b1, int b2, int b3,
				 int flag)
/*==================================================================*/
{
    /* ���¹���ν��� */

    int i, rel_pre, rel_pos;
    
    switch ( flag ) {
      case REVISE_SPRE: case REVISE_PRE:
	for ( a1=0; a1<=a2; a1++ ) {
	    if ( (a2+1) < b1 )   rel_pre = 0;
	    else if ( (a2+1) == b1 ) rel_pre = 1;
	    else if ( a1 < b1 )  rel_pre = 2;
	    else                 rel_pre = 3;	
	    for ( a3=a2+1; a3<sp->Bnst_num; a3++ ) {
		if ( a3 < b1 ) {	/* ��ʣ���ʤ� */
		    restrict_matrix[a1][a3] = 1;
		} else {
		    if ( a3 < b2 )       rel_pos = 0;
		    else if ( a3 == b2 ) rel_pos = 1;
		    else if ( a3 < b3 )  rel_pos = 2;
		    else                 rel_pos = 3;
		    if ( flag == REVISE_SPRE )
		      restrict_matrix[a1][a3]= 
			judge_matrix_pos_str[rel_pre][rel_pos];
		    else 
		      restrict_matrix[a1][a3] = 
			judge_matrix[rel_pre][rel_pos];
		}
	    }
	}
	break;
      case REVISE_SPOS: case REVISE_POS:
	for ( b1=0; b1<=b2; b1++ ) {
	    if ( a3 < b1 ) {		/* ��ʣ���ʤ� */
		for ( b3=b2+1; b3<sp->Bnst_num; b3++ ) {
		    restrict_matrix[b1][b3] = 1;
		}
	    } else {
		if ( (a2+1) < b1 )   rel_pre = 0;
		else if ( (a2+1) == b1 ) rel_pre = 1;
		else if ( a1 < b1 )  rel_pre = 2;
		else                 rel_pre = 3;	
		for ( b3=b2+1; b3<sp->Bnst_num; b3++ ) {
		    if ( a3 < b2 )       rel_pos = 0;
		    else if ( a3 == b2 ) rel_pos = 1;
		    else if ( a3 < b3 )  rel_pos = 2;
		    else                 rel_pos = 3;
		    if ( flag == REVISE_SPOS )
		      restrict_matrix[b1][b3]= 
			judge_matrix_pre_str[rel_pre][rel_pos];
		    else 
		      restrict_matrix[b1][b3] = 
			judge_matrix[rel_pre][rel_pos];
		}
	    }
	}
	break;
      default: break;
    }

    /* ���� */
    
    if (OptDisplay == OPT_DEBUG) {
	if ( flag == REVISE_SPRE || flag == REVISE_PRE )
	    print_matrix(sp, PRINT_RSTR, a2);
	else if ( flag == REVISE_SPOS || flag == REVISE_POS )
	    print_matrix(sp, PRINT_RSTR, b2);
    }
}

/*==================================================================*/
      void revise_para_rel(SENTENCE_DATA *sp, int pre, int pos)
/*==================================================================*/
{
    /* ����¤�֤δط��ˤ�뽤�� */

    int a1, a2, a3, b1, b2, b3;
    PARA_DATA *ptr1, *ptr2;

    ptr1 = &(sp->para_data[pre]);
    ptr2 = &(sp->para_data[pos]);

    a1 = ptr1->max_path[0];
    a2 = ptr1->key_pos;
    a3 = ptr1->jend_pos;
    b1 = ptr2->max_path[0];
    b2 = ptr2->key_pos;
    b3 = ptr2->jend_pos;

    /* ����������� -> ������ */
    if ( ptr1->status != 's' && ptr2->status == 's' ) {
	set_restrict_matrix(sp, a1, a2, a3, b1, b2, b3, REVISE_SPRE);
	Revised_para_num = pre;
    }
    /* ������������ -> ����� */
    else if ( ptr1->status == 's' && ptr2->status != 's' ) {
	set_restrict_matrix(sp, a1, a2, a3, b1, b2, b3, REVISE_SPOS);
	Revised_para_num = pos;
    }
    /* ��������� -> ������ */
    else if ( ptr1->max_score <= ptr2->max_score ) {
	set_restrict_matrix(sp, a1, a2, a3, b1, b2, b3, REVISE_PRE);
	Revised_para_num = pre;
    }
    /* ��������� -> ����� */
    else {
	set_restrict_matrix(sp, a1, a2, a3, b1, b2, b3, REVISE_POS);
	Revised_para_num = pos;
    }
}

/*==================================================================*/
   void revise_para_kakari(SENTENCE_DATA *sp, int num, int *array)
/*==================================================================*/
{
    /* ����������ˤ�뽤�� */

    int i, j, k;
    PARA_DATA *ptr = sp->para_data + num;

    for (i = 0; i < sp->Bnst_num; i++)
      for (j = i + 1; j < sp->Bnst_num; j++)
	restrict_matrix[i][j] = 1;

    /* ����Υ�����̵��

       ������Υ�����¿���ʤꡤ�ޤ�����Ȥ��ư���ʤ����η�������
       �ꤹ��褦�ˤʤä��Τǡ�������ʬ�Ϻ��

    for (i = 0; i < sp->Bnst_num; i++) {
	if (D_check_array[i] == FALSE && check_feature(sp->bnst_data[i].f, "�¥�"))
	    D_check_array[i] = TRUE;
    }
    */

    /* ���������� */

    if (_check_para_d_struct(sp, 0, ptr->key_pos, FALSE, 0, NULL) == FALSE) {
	for (k = ptr->key_pos; D_found_array[k] == TRUE; k--)
	  ;
	for (i = 0; i <= k; i++)
	  for (j = ptr->key_pos + 1; j < sp->Bnst_num; j++)
	    restrict_matrix[i][j] = 0;
    }

    /* ���������� */	

    for (j = ptr->key_pos + 2; j < sp->Bnst_num; j++)
	if (_check_para_d_struct(sp, ptr->key_pos + 1, j, FALSE, 0, NULL) == FALSE)
	    for (i = 0; i <= ptr->key_pos; i++) {
		restrict_matrix[i][j] = 0;
	    }
    
    Revised_para_num = num;

    if (OptDisplay == OPT_DEBUG)
	print_matrix(sp, PRINT_RSTD, ptr->key_pos);
}

/*====================================================================
                               END
====================================================================*/
