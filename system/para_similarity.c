/*====================================================================

			  ʸ�������٤η׻�

                                               S.Kurohashi 91. 6.25
                                               S.Kurohashi 93. 5.31

    $Id$
====================================================================*/
#include "knp.h"

/*==================================================================*/
              int str_part_cmp(char *c1, char *c2)
/*==================================================================*/
{
    int len, len1, len2, pre, post, match;
    
    len1 = strlen(c1);
    len2 = strlen(c2);    
    len = len1 < len2 ? len1 : len2;
    
    pre = 0;
    while (len > pre && *(c1+pre) == *(c2+pre)) 
      pre++;

    post = 0;
    while (len > post && *(c1+len1-post-1) == *(c2+len2-post-1))
      post++;
    
    match = pre > post ? pre : post;
    match % 2 ? match -= 1 : NULL;
    return match;
}

/*==================================================================*/
  int check_fuzoku(BNST_DATA *ptr, int Hinshi, int Bunrui, char *cp)
/*==================================================================*/
{
    int	i;

    if (ptr == NULL) return 0;
    for (i = 0; i < ptr->fuzoku_num; i++) 
      if ((Hinshi == 0 || Hinshi == (ptr->fuzoku_ptr+i)->Hinshi) &&
	  (Bunrui == 0 || Bunrui == (ptr->fuzoku_ptr+i)->Bunrui) &&
	  (cp == NULL  || str_eq((ptr->fuzoku_ptr+i)->Goi, cp)))
	return 1;
    return 0;
}

/*==================================================================*/
int jiritu_fuzoku_check(BNST_DATA *ptr1, BNST_DATA *ptr2, char *cp)
/*==================================================================*/
{
    if ((str_eq(ptr1->Jiritu_Go, cp) && check_fuzoku(ptr2, 0, 0, cp)) ||
	(str_eq(ptr2->Jiritu_Go, cp) && check_fuzoku(ptr1, 0, 0, cp)))
      return 1;
    else 
      return 0;
}

/*==================================================================*/
          int bgh_match(BNST_DATA *ptr1, BNST_DATA *ptr2)
/*==================================================================*/
{
    /* �֤���
       	�����Ǥ�ʬ�����ɽ�����ɤ��ʤ���� 	: -1
	3��̤���ΰ���				: 0
	3��ʾ���פ��Ƥ�����			: (���׷�� - 2)
     */

    int i, j, point, max_point = 0;

    if (! *(ptr1->BGH_code) || ! *(ptr2->BGH_code))
	return -1;

    for (i = 0; ptr1->BGH_code[i]; i+=10)
      for (j = 0; ptr2->BGH_code[j]; j+=10) {
	  point = bgh_code_match(ptr1->BGH_code+i, ptr2->BGH_code+j);
	  if (max_point < point) max_point = point;
      }
    
    if (max_point < 3) 
	return 0;
    else
	return (max_point-2);
}

/*==================================================================*/
    int subordinate_level_comp(BNST_DATA *ptr1, BNST_DATA *ptr2)
/*==================================================================*/
{
    char *level1, *level2;

    level1 = (char *)check_feature(ptr1->f, "��٥�");
    level2 = (char *)check_feature(ptr2->f, "��٥�");

    if (level1 == NULL) return TRUE;		/* ����ʤ� --> ���Ǥ�OK */
    else if (level2 == NULL) return FALSE;	/* ���Ǥ�� --> ����ʤ� */
    else if (levelcmp(level1 + strlen("��٥�:"), 
		      level2 + strlen("��٥�:")) <= 0)	/* ptr1 <= ptr2 �ʤ�OK */
	return TRUE;
    else return FALSE;
}

/*==================================================================*/
	int subordinate_level_check(char *cp, BNST_DATA *ptr2)
/*==================================================================*/
{
    char *level1, *level2;

    level1 = cp;
    level2 = (char *)check_feature(ptr2->f, "��٥�");

    if (level1 == NULL) return TRUE;		/* ����ʤ� --> ���Ǥ�OK */
    else if (level2 == NULL) return FALSE;	/* ���Ǥ�� --> ����ʤ� */
    else if (levelcmp(level1, level2 + strlen("��٥�:")) <= 0)
	return TRUE;				/* ptr1 <= ptr2 �ʤ�OK */
    else return FALSE;
}

/*==================================================================*/
	int subordinate_level_forbid(char *cp, BNST_DATA *ptr2)
/*==================================================================*/
{
    char *level1, *level2;

    level1 = cp;
    level2 = (char *)check_feature(ptr2->f, "��٥�");

    if (level1 == NULL) return TRUE;		/* ����ʤ� --> ���Ǥ�OK */
    else if (level2 == NULL) return FALSE;	/* ���Ǥ�� --> ����ʤ� */
    else if (levelcmp(level1, level2 + strlen("��٥�:")) == 0)
	return FALSE;				/* ptr1 == ptr2 �ʤ�ػ� */
    else return TRUE;
}

/*==================================================================*/
		  int levelcmp(char *cp1, char *cp2)
/*==================================================================*/
{
    int level1, level2;
    if (!strcmp(cp1, "A-"))      level1 = 1;
    else if (!strcmp(cp1, "A"))  level1 = 2;
    else if (!strcmp(cp1, "B-")) level1 = 3;
    else if (!strcmp(cp1, "B"))  level1 = 4;
    else if (!strcmp(cp1, "B+")) level1 = 5;
    else if (!strcmp(cp1, "C"))  level1 = 6;
    else fprintf(stderr, "Invalid level (%s)\n", cp1);
    if (!strcmp(cp2, "A-"))      level2 = 1;
    else if (!strcmp(cp2, "A"))  level2 = 2;
    else if (!strcmp(cp2, "B-")) level2 = 3;
    else if (!strcmp(cp2, "B"))  level2 = 4;
    else if (!strcmp(cp2, "B+")) level2 = 5;
    else if (!strcmp(cp2, "C"))  level2 = 6;
    else fprintf(stderr, "Invalid level (%s)\n", cp2);
    return level1 - level2;
}

/*==================================================================*/
		   int calc_match(int pre, int pos)
/*==================================================================*/
{
    int		i, j, part_mt_point, bgh_mt_point, point = 0;
    int		flag1, flag2, content_word_match;
    BNST_DATA 	*ptr1, *ptr2;

    ptr1 = &(sp->bnst_data[pre]);
    ptr2 = &(sp->bnst_data[pos]);

    /* �Ѹ����θ� */

    if ((check_feature(ptr1->f, "�Ѹ�") &&
	 check_feature(ptr2->f, "�Ѹ�")) ||

	(check_feature(ptr1->f, "�θ�") &&
	 check_feature(ptr2->f, "�θ�")) || 
	
	/* ��Ū���פȡ�Ū���� */
	(check_feature(ptr1->f, "�¥�:̾") && 
	 check_fuzoku(ptr1, 0, 0, "Ū") && 
	 check_fuzoku(ptr2, 0, 0, "Ū��"))

	) {

	/* ��������Ƚ��� -- �θ� ������٤� 0 */
	if (check_feature(ptr1->f, "�Ѹ�:Ƚ") &&
	    !check_feature(ptr1->f, "�Ѹ�:Ƚ:?") && /* �֡��ǡפ���� */
	    check_feature(ptr2->f, "�θ�") &&
	    !check_feature(ptr2->f, "�Ѹ�:Ƚ")) return 0;

	point += 2;

	if (check_feature(ptr1->f, "�θ�") &&
	    check_feature(ptr2->f, "�θ�")) {

	    /* 
	       �θ�Ʊ�Τξ��
	       ����̾Ʊ�� -- 5
	       ����̾Ʊ�� -- 5
	       ���ȿ�̾Ʊ�� -- 5
	       ����̾��̾�ȿ�̾ -- 2 (�����ǲ��ϤΥ�����θ)
	       ������Ʊ�� -- 2 (³��̾��(������)��ɾ��)
	       		�� �����������פ��ʤ��Ƥ�������뤳�Ȥ⤢��
	       		��)�ֿ͸���Ȭ������ͤ��ä������͸�����Ψ�ϰ�̤ǡ��ġ�
	       ������¾Ʊ�� -- ��Ω������
	    */

	    if (check_feature(ptr1->f, "��̾")) {
		flag1 = 0;
	    } else if (check_feature(ptr1->f, "��̾")) {
		flag1 = 1;
	    } else if (check_feature(ptr1->f, "�ȿ�̾")) {
		flag1 = 2;
	    } else if (check_feature(ptr1->f, "����")) {
		flag1 = 3;
	    } else {
		flag1 = 4;
	    }
	    if (check_feature(ptr2->f, "��̾")) {
		flag2 = 0;
	    } else if (check_feature(ptr2->f, "��̾")) {
		flag2 = 1;
	    } else if (check_feature(ptr2->f, "�ȿ�̾")) {
		flag2 = 2;
	    } else if (check_feature(ptr2->f, "����")) {
		flag2 = 3;
	    } else {
		flag2 = 4;
	    }

	    if (flag1 == 0 && flag2 == 0) {
		point += 5;
		content_word_match = 0;
	    }
	    else if (flag1 == 1 && flag2 == 1) {
		point += 5;
		content_word_match = 0;
	    }
	    else if (flag1 == 2 && flag2 == 2) {
		point += 5;
		content_word_match = 0;
	    }
	    else if ((flag1 == 0 || flag1 == 1 || flag1 == 2) &&
		     (flag2 == 0 || flag2 == 1 || flag2 == 2)) {
		point += 2;	/* �ȿ��ȿ�̾�ʤɤ��б����θ */
		content_word_match = 0;
	    }
	    else if (flag1 == 3 && flag2 == 3) {
		point += 2;
		for (i = 0; i < ptr1->mrph_num; i++) 
		    for (j = 0; j < ptr2->mrph_num; j++) 
			if (str_eq((ptr1->mrph_ptr+i)->Goi, 
				   (ptr2->mrph_ptr+j)->Goi) &&
			    check_feature((ptr1->mrph_ptr+i)->f, "������") &&
			    check_feature((ptr2->mrph_ptr+j)->f, "������")) {
			    point += 5;
			    goto Counter_check;
			}
	      Counter_check:
		content_word_match = 0;
	    }
	    else if (flag1 == 4 && flag2 == 4) {
		content_word_match = 1;
	    }
	    else {
		content_word_match = 0;
	    }
	}
	else {
	    content_word_match = 1;
	}

	if (content_word_match == 1) {

	    /* ��Ω��ΰ��� */
	
	    if (str_eq(ptr1->Jiritu_Go, ptr2->Jiritu_Go)) {
		point += 10;
		
	    } else {

		/* ʬ�����ɽ�ˤ������� */
	    
		bgh_mt_point = bgh_match(ptr1, ptr2) * 2; 

		/* ��Ω�����ʬ���� (���ʤ��Ȥ������ʬ�����ɽ�����ɤ��ʤ����) */
	    
		part_mt_point = 0;
		if (bgh_mt_point < 0) {
		    bgh_mt_point = 0;
		    if (check_feature(ptr1->f, "�θ�") &&
			check_feature(ptr2->f, "�θ�"))
			part_mt_point = str_part_cmp(ptr1->Jiritu_Go, ptr2->Jiritu_Go);
		}

		/* ʬ�����ɽ����ʬ���פ������Ϻ���10 */

		if ((part_mt_point + bgh_mt_point) > 10)
		    point += 10;
		else
		    point += (part_mt_point + bgh_mt_point);
	    }
	}

	/* ��°��ΰ��� */
	
	for (i = 0; i < ptr1->fuzoku_num; i++) 
	    for (j = 0; j < ptr2->fuzoku_num; j++) 
	      if (str_eq((ptr1->fuzoku_ptr+i)->Goi, 
			 (ptr2->fuzoku_ptr+j)->Goi))
		  point += 2; /* 3 */

	if ((check_feature(ptr1->f, "�����") &&
	     check_feature(ptr2->f, "������")) ||
	    (check_feature(ptr1->f, "������") &&
	     check_feature(ptr2->f, "�����"))) { 
	    point += 2;
	}
	if ((check_feature(ptr1->f, "������") &&
	     check_feature(ptr2->f, "��������")) ||
	    (check_feature(ptr1->f, "��������") &&
	     check_feature(ptr2->f, "������"))) { 
	    point += 2;
	}
	if ((check_feature(ptr1->f, "���ʤ�") &&
	     check_feature(ptr2->f, "����")) ||
	    (check_feature(ptr1->f, "����") &&
	     check_feature(ptr2->f, "���ʤ�"))) { 
	    point += 2;
	}

	/* �ɲ� */

	if (check_feature(ptr1->f, "����") &&
	    check_feature(ptr2->f, "����"))
	    point += 3;

	/* �֤����,�֤Ǥ���פʤɤμ�Ω����°��Τ��� */

	if (jiritu_fuzoku_check(ptr1, ptr2, "����"))
	    point += 1;

	if (jiritu_fuzoku_check(ptr1, ptr2, "�Ǥ���") ||
	    jiritu_fuzoku_check(ptr1, ptr2, "�����"))
	    point += 3;
    }
    
    return point;
}

/*==================================================================*/
                    void calc_match_matrix()
/*==================================================================*/
{
    int i, j;
    
    for (i = 0; i < sp->Bnst_num; i++) 
      for (j = i+1; j < sp->Bnst_num; j++)
	match_matrix[i][j] = calc_match(i, j);
}

/*====================================================================
                               END
====================================================================*/
