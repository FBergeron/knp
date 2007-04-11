/*====================================================================

		       ʸ��֤���ӡ�����ٷ׻�

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
    while (len > pre && *(c1 + pre) == *(c2 + pre)) {
	pre++;
    }

    post = 0;
    while (len > post && *(c1 + len1 - post - 1) == *(c2 + len2 - post - 1)) {
	post++;
    }
    
    match = pre > post ? pre : post;
    match -= match % BYTES4CHAR;
    match = 2 * match / BYTES4CHAR; /* 5ʸ����10�� */
    return match;
}

/*==================================================================*/
  int check_fuzoku(BNST_DATA *ptr, int Hinshi, int Bunrui, char *cp)
/*==================================================================*/
{
    int	i;

    /* ���פ�����°�줬����п� */

    if (ptr == NULL) return 0;
    for (i = ptr->mrph_num - 1; i >= 0 ; i--) {
	if (check_feature((ptr->mrph_ptr + i)->f, "��°")) {
	    if ((Hinshi == 0 || Hinshi == (ptr->mrph_ptr + i)->Hinshi) &&
		(Bunrui == 0 || Bunrui == (ptr->mrph_ptr + i)->Bunrui) &&
		(cp == NULL  || str_eq((ptr->mrph_ptr + i)->Goi, cp))) {
		return 1;
	    }
	}
	/* ��Ω��ʤ� */
	else {
	    return 0;
	}
    }
    return 0;
}

/*==================================================================*/
int check_fuzoku_substr(BNST_DATA *ptr, int Hinshi, int Bunrui, char *cp)
/*==================================================================*/
{
    int	i;

    if (ptr == NULL) return 0;
    for (i = ptr->mrph_num - 1; i >= 0 ; i--) {
	if (check_feature((ptr->mrph_ptr + i)->f, "��°")) {
	    if ((Hinshi == 0 || Hinshi == (ptr->mrph_ptr + i)->Hinshi) &&
		(Bunrui == 0 || Bunrui == (ptr->mrph_ptr + i)->Bunrui) &&
		(cp == NULL  || strstr((ptr->mrph_ptr + i)->Goi, cp))) {
		return 1;
	    }
	}
	/* ��Ω��ʤ� */
	else {
	    return 0;
	}
    }
    return 0;
}

/*==================================================================*/
int check_bnst_substr(BNST_DATA *ptr, int Hinshi, int Bunrui, char *cp)
/*==================================================================*/
{
    int	i;

    if (ptr == NULL) return 0;
    for (i = 0; i < ptr->mrph_num; i++) 
      if ((Hinshi == 0 || Hinshi == (ptr->mrph_ptr + i)->Hinshi) &&
	  (Bunrui == 0 || Bunrui == (ptr->mrph_ptr + i)->Bunrui) &&
	  (cp == NULL  || strstr((ptr->mrph_ptr + i)->Goi, cp)))
	return 1;
    return 0;
}

/*==================================================================*/
int jiritu_fuzoku_check(BNST_DATA *ptr1, BNST_DATA *ptr2, char *cp)
/*==================================================================*/
{
    if ((str_eq(ptr1->head_ptr->Goi, cp) && check_fuzoku(ptr2, 0, 0, cp)) || 
	(str_eq(ptr2->head_ptr->Goi, cp) && check_fuzoku(ptr1, 0, 0, cp)))
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

    for (i = 0; ptr1->BGH_code[i]; i+=BGH_CODE_SIZE)
	for (j = 0; ptr2->BGH_code[j]; j+=BGH_CODE_SIZE) {
	    point = bgh_code_match(ptr1->BGH_code+i, ptr2->BGH_code+j);
	    if (max_point < point) max_point = point;
	}

    return Max(max_point - 2, 0);
}

/*==================================================================*/
	    int sm_match(BNST_DATA *ptr1, BNST_DATA *ptr2)
/*==================================================================*/
{
    /* �֤���
       	�����Ǥ� NTT �����ɤ��ʤ���� 	: -1
	����				: BGH_CODE_SIZE-2 == 8	
     */

    int i, j, code_size;
    float point, max_point = 0;

    if (! *(ptr1->SM_code) || ! *(ptr2->SM_code))
	return -1;

    code_size = THESAURUS[ParaThesaurus].code_size;

    for (i = 0; ptr1->SM_code[i]; i+=code_size)
	for (j = 0; ptr2->SM_code[j]; j+=code_size) {
	    if (ParaThesaurus == USE_NTT) {
		point = ntt_code_match(ptr1->SM_code+i, ptr2->SM_code+j, SM_EXPAND_NE);
	    }
	    else {
		point = general_code_match(&THESAURUS[ParaThesaurus], ptr1->SM_code+i, ptr2->SM_code+j);
	    }
	    if (max_point < point) max_point = point;
	}

    /* ����� 0.4 �ʲ����ڤ� */
    max_point = (max_point-0.4)*(BGH_CODE_SIZE-2)/(BGH_CODE_SIZE-4)*BGH_CODE_SIZE;
    if (max_point < 0)
	return 0;
    else
	return (int)(max_point);
}

/*==================================================================*/
    int subordinate_level_comp(BNST_DATA *ptr1, BNST_DATA *ptr2)
/*==================================================================*/
{
    char *level1, *level2;

    level1 = check_feature(ptr1->f, "��٥�");
    level2 = check_feature(ptr2->f, "��٥�");

    if (level1 == NULL) return TRUE;		/* �ʤ�:���Ǥ� -> T */
    else if (level2 == NULL) return FALSE;	/* ���Ǥ�:�ʤ� -> F */
    else if (levelcmp(level1 + strlen("��٥�:"), 
		      level2 + strlen("��٥�:")) <= 0)	/* ptr1 <= ptr2 -> T */
	return TRUE;
    else return FALSE;
}

/*==================================================================*/
	int subordinate_level_check(char *cp, FEATURE *f)
/*==================================================================*/
{
    char *level1, *level2;

    level1 = cp;
    level2 = check_feature(f, "��٥�");

    if (level1 == NULL) return TRUE;		/* �ʤ�:���Ǥ� -> T */
    else if (level2 == NULL) return FALSE;	/* ���Ǥ�:�ʤ� -> F */
    else if (levelcmp(level1, level2 + strlen("��٥�:")) <= 0)
	return TRUE;				/* cp <= f -> T */
    else return FALSE;
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
	 int calc_match(SENTENCE_DATA *sp, int pre, int pos)
/*==================================================================*/
{
    int		i, j, part_mt_point, mt_point, point = 0;
    int		flag1, flag2, content_word_match;
    char	*counter1, *counter2;
    char        str1[4], str2[4];
    char        str1_bk[WORD_LEN_MAX], str2_bk[WORD_LEN_MAX];
    BNST_DATA 	*ptr1, *ptr2;
    float       similarity;

    ptr1 = &(sp->bnst_data[pre]);
    ptr2 = &(sp->bnst_data[pos]);

    /* �Ѹ����θ� */

    if (Language != CHINESE) {
	if ((check_feature(ptr1->f, "�Ѹ�") &&
	     check_feature(ptr2->f, "�Ѹ�")) ||

	    (check_feature(ptr1->f, "�θ�") &&
	     check_feature(ptr2->f, "�θ�")) || 

	    (check_feature(ptr1->f, "����") && /* �ְ졢���񤹤�פǤ��θ����Ѹ��Ȥʤ뤿�� */
	     check_feature(ptr2->f, "����")) || 
	
	    /* ��Ū���פȡ�Ū���� */
	    (check_feature(ptr1->f, "�¥�:̾") && 
	     check_feature(ptr1->f, "����׻�:Ū") && 
	     check_feature(ptr2->f, "����׻�:Ū"))
	    /* check_bnst_substr(ptr1, 0, 0, "Ū") && 
	       check_bnst_substr(ptr2, 0, 0, "Ū��")) */
	    ) {

	    /* ��������Ƚ��� -- �θ� ������٤� 0 */
	    if (check_feature(ptr1->f, "�Ѹ�:Ƚ") &&
		!check_feature(ptr1->f, "�¥�:��") && /* �֡��ǤϤʤ��ס֤Ǥ��Ȥ��פ���� */
		check_feature(ptr2->f, "�θ�") &&
		!check_feature(ptr2->f, "�Ѹ�:Ƚ")) return 0;
	
	    /* �֤���ס֤����פ�¾���θ�������٤�Ϳ���ʤ��褦�� */

	    if ((check_feature(ptr1->f, "����-����") &&
		 !check_feature(ptr2->f, "����-����")) ||
		(!check_feature(ptr1->f, "����-����") &&
		 check_feature(ptr2->f, "����-����"))) return 0;

	    /* ʣ�缭�Ȥ���ʳ�������� 0 */

	    if ((check_feature(ptr1->f, "ʣ�缭") &&
		 !check_feature(ptr2->f, "ʣ�缭")) ||
		(!check_feature(ptr1->f, "ʣ�缭") &&
		 check_feature(ptr2->f, "ʣ�缭"))) return 0;

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
		   ������Ʊ�� -- 2			
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
		    /* } else if (check_feature(ptr1->f, "����")) {
		       flag1 = 4; */
		} else {
		    flag1 = 5;
		}

		if (check_feature(ptr2->f, "��̾")) {
		    flag2 = 0;
		} else if (check_feature(ptr2->f, "��̾")) {
		    flag2 = 1;
		} else if (check_feature(ptr2->f, "�ȿ�̾")) {
		    flag2 = 2;
		} else if (check_feature(ptr2->f, "����")) {
		    flag2 = 3;
		    /* } else if (check_feature(ptr2->f, "����")) {
		       flag2 = 4; */
		} else {
		    flag2 = 5;
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

		    counter1 = check_feature(ptr1->f, "������");
		    counter2 = check_feature(ptr2->f, "������");
		    if ((!counter1 && !counter2) ||
			!counter1 ||
			(counter1 && counter2 && !strcmp(counter1, counter2))) {
			point += 5;
		    }
		    content_word_match = 0;
		}
		else if (flag1 == 4 && flag2 == 4) {
		    point += 2;
		    content_word_match = 0;
		}
		else if (flag1 == 5 && flag2 == 5) {
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
	
		/* if (str_eq(ptr1->head_ptr->Goi, ptr2->head_ptr->Goi)) { */
		if (str_eq(ptr1->Jiritu_Go, ptr2->Jiritu_Go)) {
		    point += 10;
		
		} else {

		    /* �������饹�ˤ������� */

		    if (ParaThesaurus == USE_NONE) {
			mt_point = -1;
		    }
		    else if (ParaThesaurus == USE_BGH) {
			mt_point = bgh_match(ptr1, ptr2) * 2;
		    }
		    else {
			mt_point = sm_match(ptr1, ptr2) * 2;
		    }

		    if (check_feature(ptr1->f, "�Ѹ�") &&
			check_feature(ptr2->f, "�Ѹ�")) {
		    
			/* ���������� �֤���פΥ������饹����٤Ϻ���2 */
			if (str_eq(ptr1->Jiritu_Go, "����") ||
			    str_eq(ptr2->Jiritu_Go, "����")) {
			    mt_point = Min(mt_point, 2);
			}
		
			/* �����ɸ졤�夬�ɸ�Ǥʤ��������٤򤪤�����
			   ��)�����迴�Ǥ��뤫�ɤ���[ʬ����ޤ��󤬡�����ʤ����]�������ߤ�����Ǥ� */
			if (check_feature(ptr1->f, "�ɸ�") &&
			    !check_feature(ptr2->f, "�ɸ�")) {
			    mt_point = Min(mt_point, 2);
			}
		    }		    

		    /* ��Ω�����ʬ���� (���ʤ��Ȥ�����ΰ�̣°�������ɤ��ʤ����) */
	    
		    part_mt_point = 0;
		    if (mt_point < 0) {
			mt_point = 0;
			if (check_feature(ptr1->f, "�θ�") &&
			    check_feature(ptr2->f, "�θ�"))
			    part_mt_point = str_part_cmp(ptr1->head_ptr->Goi, ptr2->head_ptr->Goi);
		    }

		    /* �������饹����ʬ���פ������Ϻ���10 */
		    point += Min(part_mt_point + mt_point, 10);
		}
	    }

	    /* �缭�����Ǥ���, �������ʳ�����°��ΰ��� */

	    for (i = ptr1->mrph_num - 1; i >= 0 ; i--) {
		if (check_feature((ptr1->mrph_ptr + i)->f, "��°") && 
		    ptr1->mrph_ptr + i > ptr1->head_ptr) {
		    if (!strcmp(Class[(ptr1->mrph_ptr + i)->Hinshi][0].id, "������")) {
			continue;
		    }
		    for (j = ptr2->mrph_num - 1; j >= 0 ; j--) {
			if (check_feature((ptr2->mrph_ptr + j)->f, "��°") && 
			    ptr2->mrph_ptr + j > ptr2->head_ptr) {
			    if (!strcmp(Class[(ptr2->mrph_ptr + j)->Hinshi][0].id, "������")) {
				continue;
			    }
			    if (str_eq((ptr1->mrph_ptr + i)->Goi, 
				       (ptr2->mrph_ptr + j)->Goi)) {
				point += 2; /* 3 */
			    }
			}
			else {
			    break;
			}
		    }
		}
		else {
		    break;
		}
	    }

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
	    if (check_feature(ptr1->f, "����") &&
		check_feature(ptr2->f, "����")) { 
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
    }
    else { /*for Chinese*/
	/* if there is a PU between two words except for DunHao, similarity is 0 */
	for (i = ptr1->num + 1; i < ptr2->num; i++) {
	    if (check_feature((sp->bnst_data+i)->f, "PU") && strcmp((sp->bnst_data+i)->head_ptr->Goi, "��") && strcmp((sp->bnst_data+i)->head_ptr->Goi, "��")) {
		point = 0;
		return point;
	    }
	}

	/* Add point for nouns with similar characters */
	if ((check_feature(ptr1->f, "NN") && check_feature(ptr2->f, "NN")) ||
	    (check_feature(ptr1->f, "NR") && check_feature(ptr2->f, "NR"))) {
	    for (i = 3; i <= (strlen(ptr1->head_ptr->Goi)  < strlen(ptr2->head_ptr->Goi) ? strlen(ptr1->head_ptr->Goi) : strlen(ptr2->head_ptr->Goi)); i += 3) {
		strcpy(str1, "   ");
		strcpy(str2, "   ");
		strncpy(str1, ptr1->head_ptr->Goi + (strlen(ptr1->head_ptr->Goi) - i), 3);
		strncpy(str2, ptr2->head_ptr->Goi + (strlen(ptr2->head_ptr->Goi) - i), 3);
		if (strcmp(str1,str2) != 0) {
		    break;
		}
	    }
	    if (i > 3 && i < strlen(ptr1->head_ptr->Goi) && i < strlen(ptr2->head_ptr->Goi)) {
		point += 5;
	    }
	}
	
	/* Normalize figures and add point for similar words regardless of figures */
	if ((check_feature(ptr1->f, "CD") && check_feature(ptr2->f, "CD")) || 
	    (check_feature(ptr1->f, "OD") && check_feature(ptr2->f, "OD")) || 
	    ((check_feature(ptr1->f, "NT") || check_feature(ptr1->f, "NT-SHORT"))&& check_feature(ptr2->f, "NT"))) {
	    strcpy(str1_bk, "");
	    strcpy(str2_bk, "");
	    for (i = 0; i < strlen(ptr1->head_ptr->Goi) - 3; i+=3) {
		strcpy(str1, "   ");
		strncpy(str1, ptr1->head_ptr->Goi + i, 3);
		if (!is_figure(str1)) {
		    strcat(str1_bk, str1);
		}
	    }
	    for (i = 0; i < strlen(ptr2->head_ptr->Goi) - 3; i+=3) {
		strcpy(str2, "   ");
		strncpy(str2, ptr2->head_ptr->Goi + i, 3);
		if (!is_figure(str2)) {
		    strcat(str2_bk, str2);
		}
	    }
	if (!strcmp(str1_bk, str2_bk)) {
		point += 5;
	    }
	}

	similarity = similarity_chinese(ptr1, ptr2);

	if (similarity > 0.29) {
	    point = 10 * similarity + 7.1;
	}
	else {
	    point = 10 * similarity;
	}

	if (!strcmp(ptr1->head_ptr->Goi, ptr2->head_ptr->Goi)) {
	    point += 5;
	}
	if (!strcmp(ptr1->head_ptr->Pos, ptr2->head_ptr->Pos)) {
	    point += 2;

	}
    }
    
    return point;
}

/*==================================================================*/
	      void calc_match_matrix(SENTENCE_DATA *sp)
/*==================================================================*/
{
    int i, j;
    
    for (i = 0; i < sp->Bnst_num; i++) 
	for (j = i+1; j < sp->Bnst_num; j++)
	    match_matrix[i][j] = calc_match(sp, i, j);
}

/*==================================================================*/
	      int is_figure(char *s)
/*==================================================================*/
{
    int value = 0;
    if (!strcmp(s, "��") || 
	!strcmp(s, "��") || 
	!strcmp(s, "��") || 
	!strcmp(s, "��") || 
	!strcmp(s, "��") || 
	!strcmp(s, "��") || 
	!strcmp(s, "��") || 
	!strcmp(s, "��") || 
	!strcmp(s, "��") || 
	!strcmp(s, "��") || 
	!strcmp(s, "��") || 
	!strcmp(s, "��") || 
	!strcmp(s, "��") || 
	!strcmp(s, "���") || 
	!strcmp(s, "��") || 
	!strcmp(s, "��") || 
	!strcmp(s, "��") || 
	!strcmp(s, "ϻ") || 
	!strcmp(s, "��") || 
	!strcmp(s, "Ȭ") || 
	!strcmp(s, "��") || 
	!strcmp(s, "��") || 
	!strcmp(s, "��") || 
	!strcmp(s, "ɴ") || 
	!strcmp(s, "��") || 
	!strcmp(s, "��") || 
	!strcmp(s, "$ARZ(B") || 
	!strcmp(s, "��") || 
	!strcmp(s, "ʬ") || 
	!strcmp(s, "Ƿ") ||
	!strcmp(s, "ǯ") || 
	!strcmp(s, "��") || 
	!strcmp(s, "��")) {
	value = 1;
    }
    return value;
}

/*====================================================================
                               END
====================================================================*/
