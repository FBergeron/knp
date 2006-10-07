/*====================================================================

				 CKY

    $Id$
====================================================================*/

#include "knp.h"

typedef struct _CKY *CKYptr;
typedef struct _CKY {
    char	cp;
    int		score;
    int		direction; /* direction of dependency */
    BNST_DATA	*b_ptr;
    int 	scase_check[SCASE_CODE_SIZE];
    int		un_count;
    CKYptr	left;	/* pointer to the left child */
    CKYptr	right;	/* pointer to the right child */
    CKYptr	next;	/* pointer to the next CKY data at this point */
} CKY;

#define	CKY_TABLE_MAX	50000
CKY *cky_matrix[BNST_MAX][BNST_MAX];/* CKY����γư��֤κǽ��CKY�ǡ����ؤΥݥ��� */
CKY cky_table[CKY_TABLE_MAX];	  /* CKY�ǡ��������� */


int convert_to_dpnd(TOTAL_MGR *Best_mgr, CKY *cky_ptr, int space, int flag) {
    /* flag == 1 : ���λҤ��
       flag == 0 : ���λҤϤ⤦�񤫤ʤ� */
    int i;
    char *cp;

    if (OptDisplay == OPT_DEBUG) {
	if (flag == 1) {
	    for (i = 0; i < space; i++) 
		printf(" ");
	    printf("%c\n", cky_ptr->cp);
	}
    }

    if (cky_ptr->right) {
	if (Mask_matrix[cky_ptr->left->b_ptr->num][cky_ptr->b_ptr->num] == 2) {
	    Best_mgr->dpnd.head[cky_ptr->left->b_ptr->num] = cky_ptr->b_ptr->num;
	    Best_mgr->dpnd.type[cky_ptr->left->b_ptr->num] = 'P';
	}
	else if (Mask_matrix[cky_ptr->left->b_ptr->num][cky_ptr->b_ptr->num] == 3) {
	    Best_mgr->dpnd.head[cky_ptr->left->b_ptr->num] = cky_ptr->b_ptr->num;
	    Best_mgr->dpnd.type[cky_ptr->left->b_ptr->num] = 'I';
	}
	else {
	    if ((cp = check_feature(cky_ptr->left->b_ptr->f, "��:̵�ʽ�°")) != NULL) {
		sscanf(cp, "%*[^:]:%*[^:]:%d", &(Best_mgr->dpnd.head[cky_ptr->left->b_ptr->num]));
		Best_mgr->dpnd.type[cky_ptr->left->b_ptr->num] = 
		    Dpnd_matrix[cky_ptr->left->b_ptr->num][cky_ptr->b_ptr->num];
	    }
	    else {
		if (cky_ptr->direction == RtoL) { /* <- */
		    Best_mgr->dpnd.head[cky_ptr->b_ptr->num] = cky_ptr->left->b_ptr->num;
		    Best_mgr->dpnd.type[cky_ptr->b_ptr->num] = 
			Dpnd_matrix[cky_ptr->left->b_ptr->num][cky_ptr->b_ptr->num];
		}
		else { /* -> */
		    Best_mgr->dpnd.head[cky_ptr->left->b_ptr->num] = cky_ptr->b_ptr->num;
		    Best_mgr->dpnd.type[cky_ptr->left->b_ptr->num] = 
			Dpnd_matrix[cky_ptr->left->b_ptr->num][cky_ptr->b_ptr->num];
		}
	    }
	}

	convert_to_dpnd(Best_mgr, cky_ptr->right, space + 2, 0);
	convert_to_dpnd(Best_mgr, cky_ptr->left, space + 2, 1);
    }
}

int check_scase (BNST_DATA *g_ptr, int *scase_check, int rentai, int un_count) {
    /* �����Ƥ��륬��,���,�˳�,���� */
    int vacant_slot_num = 0;

    if ((g_ptr->SCASE_code[case2num("����")]
	 - scase_check[case2num("����")]) == 1) {
	vacant_slot_num++;
    }
    if ((g_ptr->SCASE_code[case2num("���")]
	 - scase_check[case2num("���")]) == 1) {
	vacant_slot_num++;
    }
    if ((g_ptr->SCASE_code[case2num("�˳�")]
	 - scase_check[case2num("�˳�")]) == 1 &&
	rentai == 1 &&
	check_feature(g_ptr->f, "�Ѹ�:ư")) {
	vacant_slot_num++;
	/* �˳ʤ�ư���Ϣ�ν����ξ�������θ���Ĥޤ�Ϣ��
	   �����˳�����Ƥ�����ǡ�̤�ʤΥ���åȤȤϤ��ʤ� */
    }
    if ((g_ptr->SCASE_code[case2num("����")]
	 - scase_check[case2num("����")]) == 1) {
	vacant_slot_num++;
    }

    /* ��������å�ʬ����Ϣ�ν�����̤�ʤ˥�������Ϳ���� */
    if ((rentai + un_count) <= vacant_slot_num) {
	return (rentai + un_count) * 10;
    }
    else {
	return vacant_slot_num * 10;
    }
}

int check_dpnd_possibility (int dep, int gov, int relax_flag) {
    if ((Dpnd_matrix[dep][gov] && 
	 Quote_matrix[dep][gov] && 
	 Mask_matrix[dep][gov] == 1) || /* ����ǤϤʤ���硢����ޥ�����̵�뤹������θ����ʤ顢���ξ���Ϥ��� */
	Mask_matrix[dep][gov] == 2 || /* ����P */
	Mask_matrix[dep][gov] == 3) { /* ����I */
	return TRUE;
    }
    else if (relax_flag && Language != CHINESE) { /* relax */
	if (!Dpnd_matrix[dep][gov]) {
	    Dpnd_matrix[dep][gov] = 'R';
	}
	return TRUE;
    }
    return FALSE;
}

int calc_score(SENTENCE_DATA *sp, CKY *cky_ptr) {
    CKY *right_ptr = cky_ptr->right, *tmp_cky_ptr = cky_ptr;
    BNST_DATA *g_ptr = cky_ptr->b_ptr, *d_ptr;
    int i, k, one_score = 0, pred_p = 0, topic_score = 0;
    int ha_check = 0, *un_count;
    int rentai, vacant_slot_num, *scase_check;
    int count, pos, default_pos;
    char *cp, *cp2;

    /* �оݤ��Ѹ��ʳ��Υ������򽸤�� (right�򤿤ɤ�ʤ���left�Υ�������­��) */
    while (tmp_cky_ptr) {
	if (tmp_cky_ptr->left) {
	    one_score += tmp_cky_ptr->left->score;
	}
	tmp_cky_ptr = tmp_cky_ptr->right;
    }
    if (OptDisplay == OPT_DEBUG) {
	printf("%d=>", one_score);
    }

    if (check_feature(g_ptr->f, "�Ѹ�") ||
	check_feature(g_ptr->f, "���Ѹ�")) {
	pred_p = 1;
	for (k = 0; k < SCASE_CODE_SIZE; k++) cky_ptr->scase_check[k] = 0;
	scase_check = &(cky_ptr->scase_check[0]);
	cky_ptr->un_count = 0;
	un_count = &(cky_ptr->un_count);
    }

    /* ���٤ƤλҶ��ˤĤ��� */
    while (cky_ptr) {
	if (cky_ptr->left) {
	    d_ptr = cky_ptr->left->b_ptr;

	    if (Mask_matrix[d_ptr->num][g_ptr->num] == 2 || /* ����P */
		Mask_matrix[d_ptr->num][g_ptr->num] == 3) { /* ����I */
		;
	    }
	    else {
		/* ��Υ�����Ȥ�׻����뤿��ˡ��ޤ�������θ����Ĵ�٤� */
		count = 0;
		for (i = d_ptr->num + 1; i < sp->Bnst_num; i++) {
		    if (check_dpnd_possibility(d_ptr->num, i, ((i == sp->Bnst_num - 1) && count == 0) ? TRUE : FALSE)) {
			if (i == g_ptr->num) {
			    pos = count;
			}
			count++;
		    }
		}

		default_pos = (d_ptr->dpnd_rule->preference == -1) ?
		    count : d_ptr->dpnd_rule->preference;

		/* �������DEFAULT�ΰ��֤Ȥκ���ڥʥ�ƥ���
		   �� �����C,B'����Ʊ󤯤˷��뤳�Ȥ����뤬�����줬
		   ¾�η�����˱ƶ����ʤ��褦,�ڥʥ�ƥ��˺���Ĥ��� */
		if (check_feature(d_ptr->f, "����")) {
		    one_score -= abs(default_pos - 1 - pos);
		}
		else {
		    one_score -= abs(default_pos - 1 - pos) * 2;
		}

		/* �������Ĥ�Τ��٤ˤ����뤳�Ȥ��ɤ� */
		if (d_ptr->num + 1 == g_ptr->num && 
		    abs(default_pos - 1 - pos) > 0 && 
		    check_feature(d_ptr->f, "����")) {
		    one_score -= 5;
		}
	    }
	    
	    if (pred_p && (cp = check_feature(d_ptr->f, "��")) != NULL) {
		    
		/* ̤�� ����(�֡��ϡ�)�ΰ��� */
		if (check_feature(d_ptr->f, "����") && !strcmp(cp, "��:̤��")) {

		    /* ʸ��, �֡����פʤ�, ������, C, B'�˷��뤳�Ȥ�ͥ�� */
		    if ((cp2 = check_feature(g_ptr->f, "�����")) != NULL) {
			sscanf(cp2, "%*[^:]:%d", &topic_score);
			one_score += topic_score;
		    }

		    /* ��Ĥ������ˤ�������Ϳ���� (����,���̤���)
		       �� ʣ�������꤬Ʊ��Ҹ�˷��뤳�Ȥ��ɤ� */
		    if (check_feature(d_ptr->f, "����") ||
			check_feature(d_ptr->f, "����")) {
			one_score += 10;
		    }
		    else if (ha_check == 0){
			one_score += 10;
			ha_check = 1;
		    }
		}

		k = case2num(cp + 3);

		/* �����ǰ��̤ΰ��� */

		/* ̤�� : �����Ƥ�������Ƕ�����åȤ�Ĵ�٤� (����,���̤���) */
		if (!strcmp(cp, "��:̤��")) {
		    if (check_feature(d_ptr->f, "����") ||
			check_feature(d_ptr->f, "����")) {
			one_score += 10;
		    }
		    else {
			(*un_count)++;
		    }
		}

		/* �γ� : �θ��ʳ��ʤ� break 
		   �� ���������γ����Ǥˤ�����Ϳ���ʤ���
		   �� �γʤ�������Ф��������γʤϤ�����ʤ�

		   �� ���θ��פȤ����Τ�Ƚ���Τ��ȡ�������
		   ʸ���ʤɤǤ��Ѹ�:ư�ȤʤäƤ��뤳�Ȥ�
		   ����Τǡ����θ��פǥ����å� */
		else if (!strcmp(cp, "��:�γ�")) {
		    if (!check_feature(g_ptr->f, "�θ�")) {
			/* one_score += 10;
			   break; */
			if (g_ptr->SCASE_code[case2num("����")] &&
			    scase_check[case2num("����")] == 0) {
			    one_score += 10;
			    scase_check[case2num("����")] = 1;
			}
		    }
		} 

		/* ���� : ������ʸ������ΤǾ���ʣ�� */
		else if (!strcmp(cp, "��:����")) {
		    if (g_ptr->SCASE_code[case2num("����")] &&
			scase_check[case2num("����")] == 0) {
			one_score += 10;
			scase_check[case2num("����")] = 1;
		    }
		    else if (g_ptr->SCASE_code[case2num("����")] &&
			     scase_check[case2num("����")] == 0) {
			one_score += 10;
			scase_check[case2num("����")] = 1;
		    }
		}

		/* ¾�γ� : �Ƴ�1�Ĥ������򤢤�����
		   �� �˳ʤξ�硤���֤Ȥ���ʳ��϶��̤��������������⡩ */
		else if (k != -1) {
		    if (scase_check[k] == 0) {
			scase_check[k] = 1;
			one_score += 10;
		    } 
		}

		/* �֡�����Τϡ����פ˥ܡ��ʥ� 01/01/11
		   �ۤȤ�ɤξ�������

		   ������)
		   �ֹ��Ĥ����Τ� Ǥ���� ���ݤ���� ��ͳ�� ��Ĥ餷����

		   �ֻȤ��Τ� ������ ���Ȥ�����
		   �ֱ�������� �ʤ뤫�ɤ����� ��̯�� �Ȥ��������
		   �� ��������ϡ֤���/�Ȥ�����פ˷���Ȱ���

		   ��¾�ͤ� ������Τ� �����ˤʤ� ������Ǥ���
		   �� �������ۣ�������ʸ̮��������

		   ��������)
		   �֤��줬 �֣ͣФ� ʬ����ʤ� ���Ǥ��礦��
		   �֡� ����ʤ� ���� ��������
		   �֥ӥ��� ���Τ� ���Ѥ� ���塣��
		   ���Ȥ� ��ޤ�Τ� �򤱤�줽���ˤʤ� ���Ԥ�������
		   �֤��ޤ� ��Ω�ĤȤ� �פ��ʤ� ����������
		   �֤ɤ� �ޤ�礦���� ����뤵��Ƥ��� ˡ������
		   ��ǧ����뤫�ɤ����� ���줿 ��Ƚ�ǡ�

		   �����ꢨ
		   �֤�������פ��� �Τ褦�ʾ����Ѹ��Ȥߤʤ����Τ�����
		*/

		if (check_feature(d_ptr->f, "�Ѹ�") &&
		    (check_feature(d_ptr->f, "��:̤��") ||
		     check_feature(d_ptr->f, "��:����")) &&
		    check_feature(g_ptr->f, "�Ѹ�:Ƚ")) {
		    one_score += 3;
		}
	    }

	    /* Ϣ�ν����ξ�硤���褬
	       ������̾��,����Ū̾��
	       ����ͽ���,�ָ����ߡפʤ�
	       �Ǥʤ���а�Ĥγ����Ǥȹͤ��� */
	    if (check_feature(d_ptr->f, "��:Ϣ��")) {
		if (check_feature(g_ptr->f, "���δط�") || 
		    check_feature(g_ptr->f, "�롼�볰�δط�")) {
		    one_score += 10;	/* ���δط��ʤ餳���ǲ��� */
		}
		else {
		    /* ����ʳ��ʤ��������åȤ�����å� (Ϣ�ν������դ����Ȥ��Υ������κ�ʬ) */
		    one_score += check_scase(d_ptr, &(cky_ptr->left->scase_check[0]), 1, cky_ptr->left->un_count) - 
			check_scase(d_ptr, &(cky_ptr->left->scase_check[0]), 0, cky_ptr->left->un_count);
		}
	    }
	}

	cky_ptr = cky_ptr->right;
    }

    /* �Ѹ��ξ�硤�ǽ�Ū��̤��,����,���,�˳�,Ϣ�ν������Ф���
       ����,���,�˳ʤΥ���å�ʬ����������Ϳ���� */
    if (pred_p) {
	one_score += check_scase(g_ptr, scase_check, 0, *un_count);
    }

    return one_score;
}

int cky (SENTENCE_DATA *sp, TOTAL_MGR *Best_mgr) {
    int i, j, k, l, sen_len, cky_table_num, best_score, dep_check[BNST_MAX];
    CKY *cky_ptr, *left_ptr, *right_ptr, *best_ptr, *pre_ptr, *best_pre_ptr, *start_ptr;
    CKY **next_pp, **next_pp_for_ij;

    cky_table_num = 0;

    /* initialize */
    for (i = 0; i < sp->Bnst_num; i++) {
	dep_check[i] = -1;
	Best_mgr->dpnd.head[i] = -1;
	Best_mgr->dpnd.type[i] = 'D';
    }

    /* �롼�פϺ����鱦,�������
       i����j�ޤǤ����Ǥ�ޤȤ����� */
    for (j = 0; j < sp->Bnst_num; j++) { /* left to right (�����鱦) */
	for (i = j; i >= 0; i--) { /* bottom to top (�������) */
	    if (OptDisplay == OPT_DEBUG) {
		printf("(%d,%d)\n", i, j);
	    }
	    cky_matrix[i][j] = NULL;
	    if (i == j) {
		cky_ptr = &(cky_table[cky_table_num]);
		cky_table_num++;
		if (cky_table_num >= CKY_TABLE_MAX) {
		    fprintf(stderr, ";;; cky_table_num exceeded maximum\n");
		    exit(1);
		}
		cky_matrix[i][j] = cky_ptr;
		cky_ptr->left = NULL;
		cky_ptr->right = NULL;
		cky_ptr->next = NULL;
		cky_ptr->cp = 'a' + j;
		cky_ptr->score = 0;
		cky_ptr->b_ptr = sp->bnst_data + j;
		for (k = 0; k < SCASE_CODE_SIZE; k++) cky_ptr->scase_check[k] = 0;
		cky_ptr->un_count = 0;
	    } 
	    else {
		next_pp_for_ij = NULL;	/* ���ΰ��֤˰�Ĥ�礬�Ǥ��Ƥʤ��� */

		/* i����i+k�ޤǤ�,i+k+1����j�ޤǤ�ޤȤ�� */
		for (k = 0; k < j - i; k++) {
		    /* ��郎�礦���˶���� */
		    if ((dep_check[i + k] <= 0 || /* �Хꥢ���ʤ��Ȥ� */
			 dep_check[i + k] >= j) && /* �Хꥢ������Ȥ��Ϥ����ޤ� */
			check_dpnd_possibility(i + k, j, (j == sp->Bnst_num - 1) && dep_check[i + k] == -1 ? TRUE : FALSE)) {
			if (OptDisplay == OPT_DEBUG) {
			    printf("   (%d,%d), (%d,%d) [%s->%s], score=", i, i + k, i + k + 1, j, 
				   (sp->bnst_data + i + k)->head_ptr->Goi, 
				   (sp->bnst_data + j)->head_ptr->Goi);
			}
			left_ptr = cky_matrix[i][i + k];
			next_pp = NULL;
			while (left_ptr) {
			    right_ptr = cky_matrix[i + k + 1][j];
			    while (right_ptr) {
				cky_ptr = &(cky_table[cky_table_num]);
				cky_table_num++;
				if (cky_table_num >= CKY_TABLE_MAX) {
				    fprintf(stderr, ";;; cky_table_num exceeded maximum\n");
				    exit(1);
				}
				if (next_pp == NULL) {
				    start_ptr = cky_ptr;
				}
				else {
				    *next_pp = cky_ptr;
				}
				cky_ptr->next = NULL;
				cky_ptr->left = left_ptr;
				cky_ptr->right = right_ptr;
				cky_ptr->direction = Dpnd_matrix[i + k][j] == 'L' ? RtoL : LtoR;
				cky_ptr->cp = 'a' + j;
				cky_ptr->b_ptr = cky_ptr->right->b_ptr;
				next_pp = &(cky_ptr->next);
				cky_ptr->un_count = 0;
				for (l = 0; l < SCASE_CODE_SIZE; l++) cky_ptr->scase_check[l] = 0;
				cky_ptr->score = calc_score(sp, cky_ptr);
				if (OptDisplay == OPT_DEBUG) {
				    printf("%d,", cky_ptr->score);
				}

				right_ptr = right_ptr->next;
			    }
			    left_ptr = left_ptr->next;
			}

			if (next_pp) {
			    /* choose the best one */
			    cky_ptr = start_ptr;
			    best_score = -INT_MAX;
			    while (cky_ptr) {
				if (cky_ptr->score > best_score) {
				    best_score = cky_ptr->score;
				    best_ptr = cky_ptr;
				}
				cky_ptr = cky_ptr->next;
			    }
			    start_ptr = best_ptr;
			    start_ptr->next = NULL;

			    if (next_pp_for_ij == NULL) {
				cky_matrix[i][j] = start_ptr;
			    }
			    else {
				*next_pp_for_ij = start_ptr;
			    }
			    next_pp_for_ij = &(start_ptr->next);

			    /* �Хꥢ�Υ����å� */
			    if (j != sp->Bnst_num - 1) { /* relax���ϥ����å����ʤ� */
				if ((sp->bnst_data + i + k)->dpnd_rule->barrier.fp[0] && 
				    feature_pattern_match(&((sp->bnst_data + i + k)->dpnd_rule->barrier), 
							  (sp->bnst_data + j)->f, 
							  sp->bnst_data + i + k, sp->bnst_data + j) == TRUE) {
				    dep_check[i + k] = j; /* �Хꥢ�ΰ��֤򥻥å� */
				}
				else if (dep_check[i + k] == -1) {
				    dep_check[i + k] = 0; /* ��������������ʤ��Ȥ�1�Ĥ���Ω�������Ȥ򼨤� */
				}
			    }
			}

			if (OptDisplay == OPT_DEBUG) {
			    printf("\n");
			}
		    }
		}
	    }
	}
    }

    if (OptDisplay == OPT_DEBUG) {
	printf(">>> n=%d\n", cky_table_num);
    }

    /* choose the best one */
    cky_ptr = cky_matrix[0][sp->Bnst_num - 1];
    if (cky_ptr->next) { /* if there are more than one possibility */
	best_score = -INT_MAX;
	pre_ptr = NULL;
	while (cky_ptr) {
	    if (cky_ptr->score > best_score) {
		best_score = cky_ptr->score;
		best_ptr = cky_ptr;
		best_pre_ptr = pre_ptr;
	    }
	    pre_ptr = cky_ptr;
	    cky_ptr = cky_ptr->next;
	}
	if (pre_ptr != best_ptr) {
	    if (best_pre_ptr) {
		best_pre_ptr->next = best_ptr->next;
	    }
	    else {
		cky_matrix[0][sp->Bnst_num - 1] = cky_matrix[0][sp->Bnst_num - 1]->next;
	    }
	    pre_ptr->next = best_ptr; /* move the best one to the end of the list */
	    best_ptr->next = NULL;
	}

	/* cky_ptr = cky_matrix[0][sp->Bnst_num - 1]; * when print all possible structures */
	cky_ptr = best_ptr;
    }

    while (cky_ptr) {
	if (OptDisplay == OPT_DEBUG) {
	    printf("---------------------\n");
	    printf("score=%d\n", cky_ptr->score);
	}

	Best_mgr->dpnd.head[cky_ptr->b_ptr->num] = -1;
	convert_to_dpnd(Best_mgr, cky_ptr, 0, 1);
	cky_ptr = cky_ptr->next;
    }

    /* ̵�ʽ�°: ����ʸ��η�������˽������ */
    for (i = 0; i < sp->Bnst_num - 1; i++) {
	if (Best_mgr->dpnd.head[i] < 0) {
	    /* ���ꤨ�ʤ�������� */
	    if (i >= Best_mgr->dpnd.head[i + Best_mgr->dpnd.head[i]]) {
		continue;
	    }
	    Best_mgr->dpnd.head[i] = Best_mgr->dpnd.head[i + Best_mgr->dpnd.head[i]];
	    /* Best_mgr->dpnd.check[i].pos[0] = Best_mgr->dpnd.head[i]; */
	}
    }
}
