/*====================================================================

       ����¤�����ΰ�¸��¤�Υ����å�����¸��ǽ������Υޥ���

                                               S.Kurohashi 93. 5.25
                                               S.Kurohashi 93. 5.31

    $Id$
====================================================================*/
#include "knp.h"

int D_check_array[BNST_MAX];
int D_found_array[BNST_MAX];

extern FILE  *Infp;
extern FILE  *Outfp;

/*==================================================================*/
		    int check_stop_extend(int num)
/*==================================================================*/
{
    if (check_feature(bnst_data[num].f, "����") ||
	check_feature(bnst_data[num].f, "����") ||
	(check_feature(bnst_data[num].f, "��:�ǳ�") &&
	 check_feature(bnst_data[num].f, "��")))
      return TRUE;
    else
      return FALSE;
}

/*==================================================================*/
	       int para_extend_p(PARA_MANAGER *m_ptr)
/*==================================================================*/
{
    /* ����¤�������Ĺ������ : ������ or �Ѹ���ޤ� */

    int i;

    if (para_data[m_ptr->para_data_num[0]].status == 's') 
      return TRUE;

    for (i = m_ptr->start[0]; i <= m_ptr->end[0]; i++)
      if (check_feature(bnst_data[i].f, "�Ѹ�:��"))
	return TRUE;
    return FALSE;
}

/*==================================================================*/
	       int parent_range(PARA_MANAGER *m_ptr)
/*==================================================================*/
{
    /* �Ƥ�����¤����������ϰϱ�Ĺ������
       		��Ƭ��ʬ�˴ޤޤ���� : ���¤ʤ�
		����ʳ��˴ޤޤ���� : ľ���Υ����ΰ��� */

    int i;

    if (m_ptr->parent == NULL) return 0;
    
    for (i = m_ptr->parent->part_num - 1; i > 0; i--) 
      if (m_ptr->parent->start[i] <= m_ptr->start[0])
	return m_ptr->parent->start[i];
    
    return 0;
}

/*==================================================================*/
   int _check_para_d_struct(int str, int end, 
			    int extend_p, int limit, int *s_p)
/*==================================================================*/
{
    int i, j, k, found, success_p = TRUE;
    int hikousa_array[BNST_MAX];

    D_found_array[end] = TRUE; /* I�ޡ�������������ص� */

    for (k = 0; k <= end; k++) hikousa_array[k] = 1;
	/* ��Ĺ��Ĵ�٤�Τ�,���ν������str����Ǥʤ�0���� */

    /* ����¤�����ΰ�¸��¤��Ĵ�٤�
       (��ʸ�᤬��äȤ�ᤤʸ��ˤ�����Ȳ���) */

    for (i = end - 1; i >= str; i--) {
	if (D_check_array[i] == TRUE) {
	    D_found_array[i] = TRUE;
	} else {
	    found = FALSE;
	    for (j = i + 1; j <= end; j++)
		if (Mask_matrix[i][j] &&
		    Quote_matrix[i][j] &&
		    Dpnd_matrix[i][j] &&
		    hikousa_array[j]) {
		    D_found_array[i] = TRUE;
		    for (k = i + 1; k < j; k++) hikousa_array[k] = 0;
		    found = TRUE;
		    break;
		}
	    if (found == FALSE) {
		D_found_array[i] = FALSE;
		/* revise_para_kakari����θƽ�(s_p == NULL)��ɽ���ʤ� */
		if (OptDisplay == OPT_DEBUG && s_p) {
		    fprintf(Outfp, ";; Cannot find a head for bunsetsu <");
		    print_bnst(bnst_data + i, NULL);
		    fprintf(Outfp, ">.\n");
		}
		success_p = FALSE;
		for (k = i + 1; k <= end; k++) hikousa_array[k] = 0;
	    }
	}
    } 

    /* ����¤�����α�Ĺ��ǽ�ϰϤ�Ĵ�٤� */
    
    if (extend_p == TRUE && success_p == TRUE) {
	for (i = str - 1;; i--) {
	    if (i < limit || check_stop_extend(i) == TRUE) {
		*s_p = i + 1;
		break;
	    } else if (D_check_array[i] == TRUE) {
		D_found_array[i] = TRUE;
	    } else {
		found = FALSE;
		for (j = i + 1; j <= end; j++)

		    /* 
		       '< end' �� '<= end' ����, ������������Ĺ����ʸ���
		       ������Ȥʤ����뤫�ɤ������Ѥ�롥

		       �¸��η��,'<= end'�Ȥ����������Τ����٤Ϥ褤��

		       ������) 950101071-030, 950101169-002, 950101074-019
		    */

		    if (Mask_matrix[i][j] && 
			Quote_matrix[i][j] && 
			Dpnd_matrix[i][j] && 
			hikousa_array[j]) {
			D_found_array[i] = TRUE;
			for (k = i + 1; k < j; k++) hikousa_array[k] = 0;
			found = TRUE;
			break;	/* 96/01/22�ޤǤʤ��ʤä�?? */
		    }
		if (found == FALSE) {
		    *s_p = i + 1;
		    break;
		}
	    }
	}
    }

    return success_p;
}

/*==================================================================*/
       int check_error_state(PARA_MANAGER *m_ptr, int error[])
/*==================================================================*/
{
    /* ���顼���֤Υ����å� : 

           ������ǽ�ʾ�� : 2�ĤΤ�������ʬ����ʤ�����¤
			    3�İʾ����ʬ����ʤ�����¤�Ǹ�꤬��Ƭ
			    3�İʾ����ʬ����ʤ�����¤�Ǹ�꤬����

	   ����ʳ��ξ��Ͻ�����ǰ (return -1) */

    int i;

    if (m_ptr->part_num == 2) {
	return m_ptr->para_data_num[0];
    } 
    else if (error[0] == TRUE) {
	for (i = 1; i < m_ptr->part_num; i++)
	  if (error[i] == TRUE) {
	    fprintf(Outfp, 
		    ";; Cannot revise invalid kakari struct in para!!\n");
	    return -1;
	  }
	return m_ptr->para_data_num[0];
    } 
    else if (error[m_ptr->part_num - 1] == TRUE) {
	for (i = 0; i < m_ptr->part_num - 1; i++)
	  if (error[i] == TRUE) {
	    fprintf(Outfp, 
		    ";; Cannot revise invalid kakari struct in para!!\n");
	    return -1;
	  }
	return m_ptr->para_data_num[m_ptr->para_num-1];
    }	    
    else
      return -1;
}

/*==================================================================*/
	     int check_para_d_struct(PARA_MANAGER *m_ptr)
/*==================================================================*/
{
    int i, j, k;
    int noun_flag;
    int start_pos;
    int invalid_flag = FALSE;
    int no_more_error, no_more_error_here;
    int error_type = -1, error_type_here = -1;
    int error_check[PARA_PART_MAX], error_pos[PARA_PART_MAX];
    int revised_p_num;
    BNST_DATA *k_ptr, *u_ptr;
    
    for (i = 0; i < m_ptr->child_num; i++)	/* �Ҷ��κƵ����� */
      if (check_para_d_struct(m_ptr->child[i]) == FALSE)
	return FALSE;
    
    /* �θ�ʸ���Ư����ۣ��ʤ�Τ�������¤���Ϥ����Τˤʤ���ν���
       ----------------------------------------------------------------
       ��) �����֤���ϻ�����Խ��ԡ������Ҥ���ϼ֥᡼�����Υ���ѥ˥��󡣡�
       		���Խ��ԡפ�Ƚ����ά�Ǥ��뤳�Ȥ��狼��

       ��) ��ȯ��ǽ�Ϥ���ͽ��ܡ������̤Ǥ���ϻ���ܤ����䤷���ġ�
           �ֺ�ǯ�ϻͷ�������������󡢼���˻������󤬤���ޤ�����
       		����ͽ��ܡ�,��������������פ����Ѿ�ά�Ǥʤ����Ȥ��狼��
       
       �� �Ǹ��conjunct��head�Ȥη��������¾��conjunct��head�Ȥη������
       �˥��ԡ����롥

       �� ������ˡ�Ǥ�����conjunct�η�������������뤳�Ȥ����Ǥ�����
       ������������������ʤ���

       ��) ������ϣ�����ä���������϶��־������ˣ��������ġ�

       ����Ū�ˤ�,������������feature��Ϳ���Ʒ�������β��Ϥ���ľ��
       ɬ�פ����롥���ξ��ˤϡ�
	��������Ƚ��� �� �⤦������Ƚ���
       	������������ �� ���Ѥ���ä�
       �Ȥ���������Ԥ��Ф褤��������������о��������������Ϥ����
       �褦�ˤʤ롥
    */

    if (m_ptr->status == 's') {
	noun_flag = 1; 
	for (k = 0; k < m_ptr->part_num; k++)
	    if (!check_feature(bnst_data[m_ptr->end[k]].f, "�θ�"))
		noun_flag = 0;
	if (noun_flag) {
	    for (k = 0; k < m_ptr->part_num - 1; k++)
		for (i = m_ptr->start[k]; i < m_ptr->end[k]; i ++) 
		    Dpnd_matrix[i][m_ptr->end[k]] =     
			Dpnd_matrix[i][m_ptr->end[m_ptr->part_num - 1]];
	}
    }

    /* ��¸��¤���ϲ�ǽ���Υ����å� */

    start_pos = m_ptr->start[0];
    for (k = 0; k < m_ptr->part_num; k++)
      if (_check_para_d_struct(m_ptr->start[k], m_ptr->end[k],
			       (k == 0) ? para_extend_p(m_ptr): FALSE, 
			       (k == 0) ? parent_range(m_ptr): 0,
			       &start_pos) == FALSE) {
	  invalid_flag = TRUE;
	  error_check[k] = TRUE;
      }
    
    /* ��¸��¤���Ϥ˼��Ԥ������

       ����������Ȭɴ����,��������Ǽ��ؤ�,��ϵ��Ԥǳ����򤷤Ƥ����
       �Τ褦�˽Ҹ�ξ�ά���줿�ޤ�����¤�򸡽Ф��롥

       ����ʬ�η�����Τʤ�ʸ��ο���Ʊ����,������Ǥ����
       �嵭�Υ����פ�����¤�ȹͤ��롥
       (������Τʤ�ʸ��Υ�����(���ʤʤ�)��,��̩���б�����Ȥϸ¤�ʤ�
       �Τ����¤��ʤ�)

       ���르�ꥺ�� : 
       ��Ƭ��ʬ�γƷ�����Τʤ�ʸ��ˤĤ���
	       	����ʬ�˷���Τʤ������Ʊ�������פ�ʸ�᤬���뤫�ɤ���Ĵ�٤�
       */

    if (invalid_flag == TRUE) {
	
	if (m_ptr->status != 's') {
	    if ((revised_p_num = check_error_state(m_ptr, error_check)) 
		!= -1) {
		revise_para_kakari(revised_p_num, D_found_array);
		return FALSE;
	    } else {
		goto cannnot_revise;
	    }
	}

	for (k = 0; k < m_ptr->part_num; k++)
	  error_pos[k] = m_ptr->end[k];
	while (1) {
	    no_more_error = FALSE;
	    no_more_error_here = FALSE;

	    for (i = error_pos[0] - 1; 
		 D_found_array[i] == TRUE && start_pos <= i; i--);
	    error_pos[0] = i;
	    if (i == start_pos - 1) no_more_error = TRUE;

	    for (k = 1; k < m_ptr->part_num; k++) {
		for (i = error_pos[k] - 1; 
		     D_found_array[i] == TRUE && m_ptr->start[k] <= i; i--);
		error_pos[k] = i;
		if (i == m_ptr->start[k] - 1) no_more_error_here = TRUE;

		/* ���顼���б����Ĥ��ʤ�(��ʬ����Ǥʤ�) */
		if (no_more_error != no_more_error_here) {
		    if ((revised_p_num = check_error_state(m_ptr, error_check))
			!= -1) {
			revise_para_kakari(revised_p_num, D_found_array);
			return FALSE;
		    } else {
			goto cannnot_revise;
		    }
		}
	    }
	    if (no_more_error == TRUE) break;
	    else continue;
	}	
      cannnot_revise:
    }

    /* �����å��Ѥߤΰ� */
    for (k = start_pos; k < m_ptr->end[m_ptr->part_num-1]; k++)
      D_check_array[k] = TRUE;

    /* ��Ƭ��conjunct�Υޥ��� */
    k = 0;
    for (i = 0; i < start_pos; i++) 	       /* < start_pos */
      for (j = m_ptr->start[k]; j <= m_ptr->end[k]; j++)
	Mask_matrix[i][j] = 0;
    /* ���� �¸� end�ξ�Υ��С����ʤ�
    for (i = start_pos; i < m_ptr->start[k]; i++)       end �ξ�
      Mask_matrix[i][m_ptr->end[k]] = 0;
    */
    for (i = m_ptr->start[k]; i <= m_ptr->end[k]; i++)
      for (j = m_ptr->end[k] + 1; j < Bnst_num; j++)
	Mask_matrix[i][j] = 0;

    if (para_data[m_ptr->para_data_num[0]].status == 's') /* ������ ??? */
      for (i = 0; i < m_ptr->start[0]; i++)
	Mask_matrix[i][m_ptr->end[0]] = 0;
    
    /* ������conjunct�Υޥ��� */
    for (k = 1; k < m_ptr->part_num - 1; k++) {
	for (i = 0; i < m_ptr->start[k]; i++)
	  for (j = m_ptr->start[k]; j <= m_ptr->end[k]; j++)
	    Mask_matrix[i][j] = 0;
	for (i = m_ptr->start[k]; i <= m_ptr->end[k]; i++)
	  for (j = m_ptr->end[k] + 1; j < Bnst_num; j++)
	    Mask_matrix[i][j] = 0;
    }
    
    /* ������conjunct�Υޥ��� */
    k = m_ptr->part_num - 1;
    for (i = 0; i < m_ptr->start[k]; i++)
      for (j = m_ptr->start[k]; j < m_ptr->end[k]; j++) /* < end */
	Mask_matrix[i][j] = 0;
    for (i = m_ptr->start[k]; i < m_ptr->end[k]; i++)   /* < end */
      for (j = m_ptr->end[k] + 1; j < Bnst_num; j++)
	Mask_matrix[i][j] = 0;

    /* ����η����� */
    for (k = 0; k < m_ptr->part_num - 1; k++) {
	Mask_matrix[m_ptr->end[k]][m_ptr->end[k+1]] = 2;
	/*
	  Mask_matrix[m_ptr->end[k]][m_ptr->end[m_ptr->part_num - 1]] = 2;
	*/
    }
    if (invalid_flag == TRUE)
	for (k = 0; k < m_ptr->part_num; k++)
	    for (i = m_ptr->start[k]; i <= m_ptr->end[k]; i++)
		if (D_found_array[i] == FALSE) {
		    Mask_matrix[i][m_ptr->end[k]] = 3;
		    Mask_matrix[i][m_ptr->end[m_ptr->part_num - 1]] = 3;
		}

    /* ��ʬ����ξ��,Mask_matrix�Ϻǽ��head�ȺǸ��head��3�ˤ��Ƥ�����
       �ǽ��head��dpnd.head��Ĥ���Ȥ����Ǹ��head��tree������˻Ȥ� */

    return TRUE;
}

/*==================================================================*/
		       void init_mask_matrix()
/*==================================================================*/
{
    int i, j;

    for (i = 0; i < Bnst_num; i++)
      for (j = 0; j < Bnst_num; j++)
	Mask_matrix[i][j] = 1;
}

/*==================================================================*/
			 int check_dpnd_in_para()
/*==================================================================*/
{
    int i;

    /* ����� */

    init_mask_matrix();
    for (i = 0; i < Bnst_num; i++)
      D_check_array[i] = FALSE;

    /* ����¤��η����������å����ޥ��� */
    
    for (i = 0; i < Para_M_num; i++)
      if (para_manager[i].parent == NULL)
	if (check_para_d_struct(&para_manager[i]) == FALSE)
	  return FALSE;

    return TRUE;
}

/*====================================================================
                               END
====================================================================*/
