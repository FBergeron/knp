/*====================================================================
				 CKY

    $Id$
====================================================================*/

#include "knp.h"

typedef struct _CKY *CKYptr;
typedef struct _CKY {
    int		i;
    int		j;
    char	cp;
    double	score;		/* score at this point */
    double	para_score;	/* coordination score */
    int		para_flag;	/* coordination flag */
    char	dpnd_type;	/* type of dependency (D or P) */
    int		direction;	/* direction of dependency */
    int         index;          /* index of dpnd rule for Chinese */
    BNST_DATA	*b_ptr;
    int 	scase_check[SCASE_CODE_SIZE];
    int		un_count;
    CF_PRED_MGR *cpm_ptr;	/* case components */
    CKYptr	left;		/* pointer to the left child */
    CKYptr	right;		/* pointer to the right child */
    CKYptr	next;		/* pointer to the next CKY data at this point */
} CKY;

#define PARA_THRESHOLD	0
#define	CKY_TABLE_MAX	800000 
//#define	CKY_TABLE_MAX	19000000 
CKY *cky_matrix[BNST_MAX][BNST_MAX];/* CKY����γư��֤κǽ��CKY�ǡ����ؤΥݥ��� */
CKY cky_table[CKY_TABLE_MAX];	  /* an array of CKY data */
int cpm_allocated_cky_num = -1;

/* add a clausal modifiee to CPM */
void make_data_cframe_rentai_simple(CF_PRED_MGR *pre_cpm_ptr, TAG_DATA *d_ptr, TAG_DATA *t_ptr) {

    /* ���δط��ʳ��ΤȤ��ϳ����Ǥ� (���δط��Ǥ���ƻ�ΤȤ��ϳ����Ǥˤ���) */
    if (!check_feature(t_ptr->f, "���δط�") || check_feature(d_ptr->f, "�Ѹ�:��")) {
	_make_data_cframe_pp(pre_cpm_ptr, d_ptr, FALSE);
    }
    /* ��դ˳��δط��ˤ��� */
    else {
	pre_cpm_ptr->cf.pp[pre_cpm_ptr->cf.element_num][0] = pp_hstr_to_code("���δط�");
	pre_cpm_ptr->cf.pp[pre_cpm_ptr->cf.element_num][1] = END_M;
	pre_cpm_ptr->cf.oblig[pre_cpm_ptr->cf.element_num] = FALSE;
    }

    _make_data_cframe_sm(pre_cpm_ptr, t_ptr);
    _make_data_cframe_ex(pre_cpm_ptr, t_ptr);
    pre_cpm_ptr->elem_b_ptr[pre_cpm_ptr->cf.element_num] = t_ptr;
    pre_cpm_ptr->elem_b_ptr[pre_cpm_ptr->cf.element_num]->next = NULL;
    pre_cpm_ptr->elem_b_num[pre_cpm_ptr->cf.element_num] = -1;
    pre_cpm_ptr->cf.weight[pre_cpm_ptr->cf.element_num] = 0;
    pre_cpm_ptr->cf.adjacent[pre_cpm_ptr->cf.element_num] = FALSE;
    pre_cpm_ptr->cf.element_num++;
}

/* add coordinated case components to CPM */
TAG_DATA **add_coordinated_phrases(CKY *cky_ptr, TAG_DATA **next) {
    while (cky_ptr) { /* ������ʬ�Υ����å� */
	if (cky_ptr->para_flag || cky_ptr->dpnd_type == 'P') {
	    break;
	}
	cky_ptr = cky_ptr->right;
    }

    if (!cky_ptr) {
	return NULL;
    }
    else if (cky_ptr->para_flag) { /* parent of <PARA> + <PARA> */
	return add_coordinated_phrases(cky_ptr->left, add_coordinated_phrases(cky_ptr->right, next));
    }
    else if (cky_ptr->dpnd_type == 'P') {
	*next = cky_ptr->left->b_ptr->tag_ptr + cky_ptr->left->b_ptr->tag_num - 1;
	(*next)->next = NULL;
	return &((*next)->next);
    }
    else {
	return NULL;
    }
}

char check_dpnd_possibility (SENTENCE_DATA *sp, int dep, int gov, int begin, int relax_flag) {
    if ((OptParaFix == 0 && 
	 begin >= 0 && 
	 (sp->bnst_data + dep)->para_num != -1 && 
	 Para_matrix[(sp->bnst_data + dep)->para_num][begin][gov] >= PARA_THRESHOLD) || /* para score is more than threshold */
	(OptParaFix == 1 && 
	 Mask_matrix[dep][gov] == 2)) {   /* ����P */
	return 'P';
    }
    else if (OptParaFix == 1 && 
	     Mask_matrix[dep][gov] == 3) { /* ����I */
	return 'I';
    }
    else if (Dpnd_matrix[dep][gov] && Quote_matrix[dep][gov] && 
	     ((Language != CHINESE && (OptParaFix == 0 || Mask_matrix[dep][gov] == 1)) ||
	      (Language == CHINESE && Mask_matrix[dep][gov] != 0))) {
	return Dpnd_matrix[dep][gov];
    }
    else if ((Dpnd_matrix[dep][gov] == 'R' || relax_flag) && Language != CHINESE) { /* relax */
	return 'R';
    }

    return '\0';
}

/* make an array of dependency possibilities */
void make_work_mgr_dpnd_check(SENTENCE_DATA *sp, CKY *cky_ptr, BNST_DATA *d_ptr) {
    int i, count = 0, start;

    /* �٤ˤ�������¤(1+1)�˷�����ϵ�Υ1�Ȥ��� */
    if (cky_ptr->right && cky_ptr->right->dpnd_type == 'P' && cky_ptr->right->j < d_ptr->num + 3)
	start = cky_ptr->right->j;
    else
	start = d_ptr->num + 1;

    for (i = start; i < sp->Bnst_num; i++) {
	if (check_dpnd_possibility(sp, d_ptr->num, i, -1, ((i == sp->Bnst_num - 1) && count == 0) ? TRUE : FALSE)) {
	    Work_mgr.dpnd.check[d_ptr->num].pos[count] = i;
	    count++;
	}
    }

    Work_mgr.dpnd.check[d_ptr->num].num = count;
}

int convert_to_dpnd(SENTENCE_DATA *sp, TOTAL_MGR *Best_mgr, CKY *cky_ptr) {
    int i;
    char *cp;

    /* make case analysis result for clausal modifiee */
    if (OptAnalysis == OPT_CASE) {
	if (cky_ptr->cpm_ptr->pred_b_ptr && 
	    Best_mgr->cpm[cky_ptr->cpm_ptr->pred_b_ptr->pred_num].pred_b_ptr == NULL) {
	    copy_cpm(&(Best_mgr->cpm[cky_ptr->cpm_ptr->pred_b_ptr->pred_num]), cky_ptr->cpm_ptr, 0);
	    cky_ptr->cpm_ptr->pred_b_ptr->cpm_ptr = &(Best_mgr->cpm[cky_ptr->cpm_ptr->pred_b_ptr->pred_num]);
	}

	if (cky_ptr->left && cky_ptr->right && 
	    cky_ptr->left->cpm_ptr->pred_b_ptr && 
	    check_feature(cky_ptr->left->cpm_ptr->pred_b_ptr->f, "��:Ϣ��")) { /* clausal modifiee */
	    CF_PRED_MGR *cpm_ptr = &(Best_mgr->cpm[cky_ptr->left->cpm_ptr->pred_b_ptr->pred_num]);

	    if (cpm_ptr->pred_b_ptr == NULL) { /* �ޤ�Best_mgr�˥��ԡ����Ƥ��ʤ��Ȥ� */
		copy_cpm(cpm_ptr, cky_ptr->left->cpm_ptr, 0);
		cky_ptr->left->cpm_ptr->pred_b_ptr->cpm_ptr = cpm_ptr;
	    }

	    make_work_mgr_dpnd_check(sp, cky_ptr->left, cky_ptr->right->b_ptr);
	    make_data_cframe_rentai_simple(cpm_ptr, cpm_ptr->pred_b_ptr, 
					   cky_ptr->right->b_ptr->tag_ptr + cky_ptr->right->b_ptr->tag_num - 1);
	    find_best_cf(sp, cpm_ptr, get_closest_case_component(sp, cpm_ptr), 1);
	}
    }

    if (cky_ptr->left && cky_ptr->right) {
	if (OptDisplay == OPT_DEBUG) {
	    printf("(%d, %d): (%d, %d) (%d, %d)\n", cky_ptr->i, cky_ptr->j, cky_ptr->left->i, cky_ptr->left->j, cky_ptr->right->i, cky_ptr->right->j);
	}

	if (cky_ptr->para_flag == 0) {
	    if (cky_ptr->dpnd_type != 'P' && 
		(cp = check_feature(cky_ptr->left->b_ptr->f, "��:̵�ʽ�°")) != NULL) {
		sscanf(cp, "%*[^:]:%*[^:]:%d", &(Best_mgr->dpnd.head[cky_ptr->left->b_ptr->num]));
		Best_mgr->dpnd.type[cky_ptr->left->b_ptr->num] = 
		    Dpnd_matrix[cky_ptr->left->b_ptr->num][cky_ptr->right->b_ptr->num];
	    }
	    else {
		if (cky_ptr->direction == RtoL) { /* <- */
		    Best_mgr->dpnd.head[cky_ptr->right->b_ptr->num] = cky_ptr->left->b_ptr->num;
		    Best_mgr->dpnd.type[cky_ptr->right->b_ptr->num] = cky_ptr->dpnd_type;
		}
		else { /* -> */
		    Best_mgr->dpnd.head[cky_ptr->left->b_ptr->num] = cky_ptr->right->b_ptr->num;
		    Best_mgr->dpnd.type[cky_ptr->left->b_ptr->num] = cky_ptr->dpnd_type;
		}
	    }

	    if (Language == CHINESE && cky_ptr->dpnd_type != 'P') {
		if (cky_ptr->para_score > PARA_THRESHOLD) {
		    if (cky_ptr->direction == RtoL) { /* <- */
			Best_mgr->dpnd.head[cky_ptr->right->b_ptr->num] = cky_ptr->left->b_ptr->num;
			Best_mgr->dpnd.type[cky_ptr->right->b_ptr->num] = cky_ptr->dpnd_type;
			(sp->bnst_data + cky_ptr->right->b_ptr->num)->is_para = 1;
			(sp->bnst_data + cky_ptr->left->b_ptr->num)->is_para = 2;
		    }
		    else { /* -> */
			Best_mgr->dpnd.head[cky_ptr->left->b_ptr->num] = cky_ptr->right->b_ptr->num;
			Best_mgr->dpnd.type[cky_ptr->left->b_ptr->num] = cky_ptr->dpnd_type;
			(sp->bnst_data + cky_ptr->left->b_ptr->num)->is_para = 1;
			(sp->bnst_data + cky_ptr->right->b_ptr->num)->is_para = 2;
		    }
		}
	    }		    
	}

	convert_to_dpnd(sp, Best_mgr, cky_ptr->left);
	convert_to_dpnd(sp, Best_mgr, cky_ptr->right);
    }
    else {
	if (OptDisplay == OPT_DEBUG) {
	    printf("(%d, %d)\n", cky_ptr->i, cky_ptr->j);
	}
    }
}

int check_scase (BNST_DATA *g_ptr, int *scase_check, int rentai, int un_count) {
    int vacant_slot_num = 0;

    /* �����Ƥ��륬��,���,�˳�,���� */
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

/* conventional scoring function */
double calc_score(SENTENCE_DATA *sp, CKY *cky_ptr) {
    CKY *right_ptr = cky_ptr->right, *tmp_cky_ptr = cky_ptr;
    BNST_DATA *g_ptr = cky_ptr->b_ptr, *d_ptr;
    int i, k, pred_p = 0, topic_score = 0;
    int ha_check = 0, *un_count;
    int rentai, vacant_slot_num, *scase_check;
    int count, pos, default_pos;
    int verb, comma;
    double one_score = 0;
    char *cp, *cp2;

    /* �оݤ��Ѹ��ʳ��Υ������򽸤�� (right�򤿤ɤ�ʤ���left�Υ�������­��) */
    while (tmp_cky_ptr) {
	if (tmp_cky_ptr->direction == LtoR ? tmp_cky_ptr->left : tmp_cky_ptr->right) {
	    one_score += tmp_cky_ptr->direction == LtoR ? tmp_cky_ptr->left->score : tmp_cky_ptr->right->score;
	}
	tmp_cky_ptr = tmp_cky_ptr->direction == LtoR ? tmp_cky_ptr->right : tmp_cky_ptr->left;
    }
    if (OptDisplay == OPT_DEBUG) {
	printf("%.3f=>", one_score);
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
	if (cky_ptr->direction == LtoR ? cky_ptr->left : cky_ptr->right) {
	    d_ptr = cky_ptr->direction == LtoR ? cky_ptr->left->b_ptr : cky_ptr->right->b_ptr;

	    if ((d_ptr->num < g_ptr->num &&
		 (Mask_matrix[d_ptr->num][g_ptr->num] == 2 || /* ����P */
		  Mask_matrix[d_ptr->num][g_ptr->num] == 3)) || /* ����I */
		(g_ptr->num < d_ptr->num &&
		 (Mask_matrix[g_ptr->num][d_ptr->num] == 2 || /* ����P */
		  Mask_matrix[g_ptr->num][d_ptr->num] == 3))) { /* ����I */
		;
	    }
	    else {
		/* ��Υ�����Ȥ�׻����뤿��ˡ��ޤ�������θ����Ĵ�٤� */
		count = 0;
		pos = 0;
		verb = 0;
		comma = 0;
		if (d_ptr->num < g_ptr->num) {
		    for (i = d_ptr->num + 1; i < sp->Bnst_num; i++) {
			if (check_dpnd_possibility(sp, d_ptr->num, i, cky_ptr->i, ((i == sp->Bnst_num - 1) && count == 0) ? TRUE : FALSE)) {
			    if (i == g_ptr->num) {
				pos = count;
			    }
			    count++;
			}
			if (i >= g_ptr->num) {
			    continue;
			}
			if (Language == CHINESE &&
			    (check_feature((sp->bnst_data+i)->f, "VV") ||
			     check_feature((sp->bnst_data+i)->f, "VA") ||
			     check_feature((sp->bnst_data+i)->f, "VC") ||
			     check_feature((sp->bnst_data+i)->f, "VE"))) {
			    verb++;
			}
			if (Language == CHINESE &&
			    check_feature((sp->bnst_data+i)->f, "PU") &&
			    (!strcmp((sp->bnst_data+i)->head_ptr->Goi, ",") ||
			     !strcmp((sp->bnst_data+i)->head_ptr->Goi, "��") ||
			     !strcmp((sp->bnst_data+i)->head_ptr->Goi, ":") ||
			     !strcmp((sp->bnst_data+i)->head_ptr->Goi, "��") ||
			     !strcmp((sp->bnst_data+i)->head_ptr->Goi, "��"))) {
			    comma++;
			}
		    }
		}
		else {
		    for (i = d_ptr->num - 1; i >= 0; i--) {
			if (check_dpnd_possibility(sp, i ,d_ptr->num, cky_ptr->i, FALSE)) {
			    if (i == g_ptr->num) {
				pos = count;
			    }
			    count++;
			}
			if (i <= g_ptr->num) {
			    continue;
			}
			if (Language == CHINESE &&
			    (check_feature((sp->bnst_data+i)->f, "VV") ||
			     check_feature((sp->bnst_data+i)->f, "VA") ||
			     check_feature((sp->bnst_data+i)->f, "VC") ||
			     check_feature((sp->bnst_data+i)->f, "VE"))) {
			    verb++;
			}
			if (Language == CHINESE &&
			    check_feature((sp->bnst_data+i)->f, "PU") &&
			    (!strcmp((sp->bnst_data+i)->head_ptr->Goi, ",") ||
			     !strcmp((sp->bnst_data+i)->head_ptr->Goi, "��") ||
			     !strcmp((sp->bnst_data+i)->head_ptr->Goi, ":") ||
			     !strcmp((sp->bnst_data+i)->head_ptr->Goi, "��") ||
			     !strcmp((sp->bnst_data+i)->head_ptr->Goi, "��"))) {
			    comma++;
			}
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
		else if (Language != CHINESE){
		    one_score -= abs(default_pos - 1 - pos) * 2;
		}

		/* �������Ĥ�Τ��٤ˤ����뤳�Ȥ��ɤ� */
		if (d_ptr->num + 1 == g_ptr->num && 
		    abs(default_pos - 1 - pos) > 0 && 
		    (check_feature(d_ptr->f, "����"))) {
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

	    /* calc score for Chinese */
	    if (Language == CHINESE) {
		/* add score from verb case frame */
		if ((check_feature(g_ptr->f, "VV") ||
		     check_feature(g_ptr->f, "VA") ||
		     check_feature(g_ptr->f, "VC") ||
		     check_feature(g_ptr->f, "VE") ||
		     (check_feature(g_ptr->f, "P") && g_ptr->num < d_ptr->num)) &&
		    (check_feature(d_ptr->f, "NN") ||
		     check_feature(d_ptr->f, "M") ||
		     check_feature(d_ptr->f, "NT") ||
		     check_feature(d_ptr->f, "PN"))) {
		    /* calc case frame score for Chinese */
		    if (Chi_case_prob_matrix[g_ptr->num][d_ptr->num] >= 0.01) {
			one_score += Chi_case_prob_matrix[g_ptr->num][d_ptr->num] * 20;
		    }
		    else if (Chi_case_prob_matrix[g_ptr->num][d_ptr->num] >= 0.001) {
			one_score += Chi_case_prob_matrix[g_ptr->num][d_ptr->num] * 1500;
		    }
		    else if (Chi_case_prob_matrix[g_ptr->num][d_ptr->num] >= 0.0001) {
			one_score += Chi_case_prob_matrix[g_ptr->num][d_ptr->num] * 10000;
		    }
		    else if (Chi_case_prob_matrix[g_ptr->num][d_ptr->num] >= 0.00001) {
			one_score += Chi_case_prob_matrix[g_ptr->num][d_ptr->num] * 50000;
		    }
		    else if (Chi_case_prob_matrix[g_ptr->num][d_ptr->num] >= 0.000001) {
			one_score += Chi_case_prob_matrix[g_ptr->num][d_ptr->num] * 200000;
		    }
		}

		/* add score from nominal case frame */
		if ((check_feature(g_ptr->f, "NN") ||
		     check_feature(g_ptr->f, "NT") ||
		     check_feature(g_ptr->f, "PN") ||
		     check_feature(g_ptr->f, "M")) &&
		    (check_feature(d_ptr->f, "NN") ||
		     check_feature(d_ptr->f, "NR") ||
		     check_feature(d_ptr->f, "M") ||
		     check_feature(d_ptr->f, "NT") ||
		     check_feature(d_ptr->f, "PN"))) {
		    if (check_feature((sp->bnst_data+d_ptr->num+1)->f, "DEG") ||
			(check_feature(g_ptr->f, "NN") &&
			 (check_feature(d_ptr->f, "NT") ||
			  check_feature(d_ptr->f, "M"))) ||
			(check_feature(g_ptr->f, "NN") &&
			 check_feature(d_ptr->f, "NN") &&
			 d_ptr->num - g_ptr->num == 1)) {}
		    else {
			/* calc case frame score for Chinese */
			if (Chi_case_nominal_prob_matrix[g_ptr->num][d_ptr->num] >= 0.01) {
			    one_score += Chi_case_nominal_prob_matrix[g_ptr->num][d_ptr->num] * 20;
			}
			else if (Chi_case_nominal_prob_matrix[g_ptr->num][d_ptr->num] >= 0.001) {
			    one_score += Chi_case_nominal_prob_matrix[g_ptr->num][d_ptr->num] * 1500;
			}
			else if (Chi_case_nominal_prob_matrix[g_ptr->num][d_ptr->num] >= 0.0001) {
			    one_score += Chi_case_nominal_prob_matrix[g_ptr->num][d_ptr->num] * 10000;
			}
			else if (Chi_case_nominal_prob_matrix[g_ptr->num][d_ptr->num] >= 0.00001) {
			    one_score += Chi_case_nominal_prob_matrix[g_ptr->num][d_ptr->num] * 20000;
			}
			else if (Chi_case_nominal_prob_matrix[g_ptr->num][d_ptr->num] >= 0.000001) {
			    one_score += Chi_case_nominal_prob_matrix[g_ptr->num][d_ptr->num] * 100000;
			}
		    }
		}

		/* decrease score if the modifier is root */
		if (d_ptr->num == Chi_root) {
		    one_score -= 15;
		}

		if (cky_ptr->direction == LtoR) {
		    one_score += Chi_dpnd_matrix[d_ptr->num][g_ptr->num].prob_LtoR[cky_ptr->index] * TIME_PROB;

		    /* add penalty for comma and verb */
		    if (exist_chi(sp, d_ptr->num + 1, g_ptr->num - 1, "PU") || 
			(check_feature(d_ptr->f, "DEC") && check_feature(g_ptr->f, "NN")) ||
			(check_feature(d_ptr->f, "VV") && check_feature(g_ptr->f, "NN"))) {
			one_score -= 15 * comma;
		    }
		    else {
			one_score -= 8 * verb;
			one_score -= 15 * comma;
		    }

		    /* add score for stable dpnd */
		    if (d_ptr->num + 1 == g_ptr->num &&
			(((check_feature(d_ptr->f, "CD")) &&
			  (check_feature(g_ptr->f, "DEG") ||
			   check_feature(g_ptr->f, "M"))) ||
			 
			 ((check_feature(d_ptr->f, "DEG")) &&
			  (check_feature(g_ptr->f, "NR") ||
			   check_feature(g_ptr->f, "NN"))) ||

			 ((check_feature(d_ptr->f, "JJ")) &&
			  (check_feature(g_ptr->f, "NN") ||
			   check_feature(g_ptr->f, "DEG"))) ||
			 
			 ((check_feature(d_ptr->f, "DEV")) &&
			   check_feature(g_ptr->f, "VV")) ||
			 
			 ((check_feature(d_ptr->f, "NR-SHORT")) &&
			  (check_feature(g_ptr->f, "NR"))) ||

			 ((check_feature(d_ptr->f, "NT-SHORT")) &&
			  (check_feature(g_ptr->f, "NT"))) ||
			 
			 ((check_feature(d_ptr->f, "NR")) &&
			  (check_feature(g_ptr->f, "DEG") ||
			   check_feature(g_ptr->f, "NN"))) ||

			 ((check_feature(d_ptr->f, "NT")) &&
			  (check_feature(g_ptr->f, "DEG") ||
			   check_feature(g_ptr->f, "NT"))) ||

			 ((check_feature(d_ptr->f, "OD")) &&
			  (check_feature(g_ptr->f, "M"))) ||

			 ((check_feature(d_ptr->f, "PN")) &&
			  (check_feature(g_ptr->f, "DEG"))) ||

			 ((check_feature(d_ptr->f, "SB")) &&
			  (check_feature(g_ptr->f, "VV"))) ||

			 ((check_feature(d_ptr->f, "VA")) &&
			  (check_feature(g_ptr->f, "DEV"))))) {
			one_score += 15;
		    }
		    if (d_ptr->num < g_ptr->num &&
			(((check_feature(d_ptr->f, "AD")) &&
			  (check_feature(g_ptr->f, "BA") ||
			   check_feature(g_ptr->f, "DEG") ||
			   check_feature(g_ptr->f, "DT") ||
			   check_feature(g_ptr->f, "JJ") ||
			   check_feature(g_ptr->f, "LB"))) ||

			 ((check_feature(d_ptr->f, "P")) &&
			  (check_feature(g_ptr->f, "VV") ||
			   check_feature(g_ptr->f, "VA") ||
			   check_feature(g_ptr->f, "VC") ||
			   check_feature(g_ptr->f, "VE"))) ||
			 
			 ((check_feature(d_ptr->f, "CC")) &&
			  (check_feature(g_ptr->f, "AD") ||
			   check_feature(g_ptr->f, "CD") ||
			   check_feature(g_ptr->f, "NR") ||
			   check_feature(g_ptr->f, "NT") ||
			   check_feature(g_ptr->f, "PN") ||
			   check_feature(g_ptr->f, "BA"))) ||
			      
			 ((check_feature(d_ptr->f, "AS")) &&
			  (check_feature(g_ptr->f, "VV"))) ||

			 ((check_feature(d_ptr->f, "CD")) &&
			  (check_feature(g_ptr->f, "BA") ||
			   check_feature(g_ptr->f, "DEG") ||
			   check_feature(g_ptr->f, "DEV") ||
			   check_feature(g_ptr->f, "LC") ||
			   check_feature(g_ptr->f, "M") ||
			   check_feature(g_ptr->f, "NN"))) ||

			 ((check_feature(d_ptr->f, "DEG")) &&
			  (check_feature(g_ptr->f, "DEG") ||
			   check_feature(g_ptr->f, "LC") ||
			   check_feature(g_ptr->f, "M") ||
			   check_feature(g_ptr->f, "NN") ||
			   check_feature(g_ptr->f, "NR") ||
			   check_feature(g_ptr->f, "PN") ||
			   check_feature(g_ptr->f, "VE"))) ||

			 ((check_feature(d_ptr->f, "DER")) &&
			  (check_feature(g_ptr->f, "VA") ||
			   check_feature(g_ptr->f, "VV"))) ||

			 ((check_feature(d_ptr->f, "DEV")) &&
			  (check_feature(g_ptr->f, "BA") ||
			   check_feature(g_ptr->f, "LB") ||
			   check_feature(g_ptr->f, "VA") ||
			   check_feature(g_ptr->f, "VV"))) ||

			 ((check_feature(d_ptr->f, "DT")) &&
			  (check_feature(g_ptr->f, "BA") ||
			   check_feature(g_ptr->f, "DEG") ||
			   check_feature(g_ptr->f, "NN") ||
			   check_feature(g_ptr->f, "NR") ||
			   check_feature(g_ptr->f, "PN") ||
			   check_feature(g_ptr->f, "NT"))) ||

			 ((check_feature(d_ptr->f, "ETC")) &&
			  (check_feature(g_ptr->f, "VV"))) ||

			 ((check_feature(d_ptr->f, "FW")) &&
			  (check_feature(g_ptr->f, "CD") ||
			   check_feature(g_ptr->f, "FW") ||
			   check_feature(g_ptr->f, "M"))) ||

			 ((check_feature(d_ptr->f, "IJ")) &&
			  (check_feature(g_ptr->f, "VV"))) ||

			 ((check_feature(d_ptr->f, "JJ")) &&
			  (check_feature(g_ptr->f, "CD") ||
			   check_feature(g_ptr->f, "DEG") ||
			   check_feature(g_ptr->f, "DEV") ||
			   check_feature(g_ptr->f, "NN") ||
			   check_feature(g_ptr->f, "NR") ||
			   check_feature(g_ptr->f, "PN"))) ||

			 ((check_feature(d_ptr->f, "LC")) &&
			  (check_feature(g_ptr->f, "DEG") ||
			   check_feature(g_ptr->f, "NT"))) ||

			 ((check_feature(d_ptr->f, "MSP")) &&
			  (check_feature(g_ptr->f, "BA") ||
			   check_feature(g_ptr->f, "VA") ||
			   check_feature(g_ptr->f, "VE"))) ||

			 ((check_feature(d_ptr->f, "M")) &&
			  (check_feature(g_ptr->f, "DEC") ||
			   check_feature(g_ptr->f, "DEG") ||
			   check_feature(g_ptr->f, "DEV") ||
			   check_feature(g_ptr->f, "DT") ||
			   check_feature(g_ptr->f, "LC") ||
			   check_feature(g_ptr->f, "NN") ||
			   check_feature(g_ptr->f, "PN"))) ||

			 ((check_feature(d_ptr->f, "NN")) &&
			  (check_feature(g_ptr->f, "CS") ||
			   check_feature(g_ptr->f, "DEG") ||
			   check_feature(g_ptr->f, "DEV") ||
			   check_feature(g_ptr->f, "NP") ||
			   check_feature(g_ptr->f, "SB"))) ||

			 ((check_feature(d_ptr->f, "NR")) &&
			  (check_feature(g_ptr->f, "NN") ||
			   check_feature(g_ptr->f, "DEG") ||
			   check_feature(g_ptr->f, "DT") ||
			   check_feature(g_ptr->f, "NP") ||
			   check_feature(g_ptr->f, "PN"))) ||

			 ((check_feature(d_ptr->f, "NP")) &&
			  (check_feature(g_ptr->f, "NN"))) ||

			 ((check_feature(d_ptr->f, "NT")) &&
			  (check_feature(g_ptr->f, "BA") ||
			   check_feature(g_ptr->f, "DEG") ||
			   check_feature(g_ptr->f, "JJ") ||
			   check_feature(g_ptr->f, "VV") ||
			   check_feature(g_ptr->f, "VE"))) ||

			 ((check_feature(d_ptr->f, "OD")) &&
			  (check_feature(g_ptr->f, "DEC") ||
			   check_feature(g_ptr->f, "JJ") ||
			   check_feature(g_ptr->f, "M") ||
			   check_feature(g_ptr->f, "VA"))) ||

			 ((check_feature(d_ptr->f, "PN")) &&
			  (check_feature(g_ptr->f, "BA") ||
			   check_feature(g_ptr->f, "CD") ||
			   check_feature(g_ptr->f, "DEG") ||
			   check_feature(g_ptr->f, "DT") ||
			   check_feature(g_ptr->f, "ETC") ||
			   check_feature(g_ptr->f, "LB") ||
			   check_feature(g_ptr->f, "NN") ||
			   check_feature(g_ptr->f, "NR") ||
			   check_feature(g_ptr->f, "NT") ||
			   check_feature(g_ptr->f, "P") ||
			   check_feature(g_ptr->f, "PN"))) ||

			 ((check_feature(d_ptr->f, "SP")) &&
			  (check_feature(g_ptr->f, "DEC"))) ||

			 ((check_feature(d_ptr->f, "VA")) &&
			  (check_feature(g_ptr->f, "DEC") ||
			   check_feature(g_ptr->f, "DEV"))) ||

			 ((check_feature(d_ptr->f, "VE")) &&
			  (check_feature(g_ptr->f, "DEV"))) ||

			 ((check_feature(d_ptr->f, "VV")) &&
			  (check_feature(g_ptr->f, "DEC") ||
			   check_feature(g_ptr->f, "DEV"))))) {
			one_score += 10;
		    }
 		    if ((check_feature(d_ptr->f, "VA") ||
			 check_feature(d_ptr->f, "VC") ||
			 check_feature(d_ptr->f, "VE") ||
			 check_feature(d_ptr->f, "VV")) &&
			check_feature(g_ptr->f, "DEC") &&
			exist_chi(sp, d_ptr->num+ 1, g_ptr->num - 1, "verb") == -1) {
			one_score += 30;
		    }
		    if ((check_feature(d_ptr->f, "AD")) &&
			(check_feature(g_ptr->f, "VV") ||
			 check_feature(g_ptr->f, "VA") ||
			 check_feature(g_ptr->f, "VC") ||
			 check_feature(g_ptr->f, "VE"))) {
			one_score += 30;
		    }

		    if ((check_feature(d_ptr->f, "DEC")) &&
			(check_feature(g_ptr->f, "NN") ||
			 check_feature(g_ptr->f, "NR") ||
			 check_feature(g_ptr->f, "NT") ||
			 check_feature(g_ptr->f, "PN"))) {
			one_score += 30;
		    }

		    if (d_ptr->num < g_ptr->num &&
			check_feature(d_ptr->f, "DEG") &&
			(check_feature(g_ptr->f, "NR") ||
			 check_feature(g_ptr->f, "PN"))) {
			one_score -= 20;
		    }

/* 		    if (d_ptr->num < g_ptr->num && */
/* 			check_feature(d_ptr->f, "P") && */
/* 			check_feature(g_ptr->f, "NN")) { */
/* 			one_score -= 20; */
/* 		    } */

		    if (d_ptr->num < g_ptr->num &&
			(check_feature(d_ptr->f, "NN") ||
			 check_feature(d_ptr->f, "NR")) &&
			check_feature(g_ptr->f, "AD")) {
			one_score -= 20;
		    }
		}
		else if (cky_ptr->direction == RtoL) {
		    one_score += Chi_dpnd_matrix[g_ptr->num][d_ptr->num].prob_RtoL[cky_ptr->index] * TIME_PROB;

		    /* add penalty for comma and verb */
		    if (exist_chi(sp, g_ptr->num + 1, d_ptr->num - 1, "PU")) {
			one_score -= 15 * comma;
		    }
		    else {
			one_score -= 8 * verb;
			one_score -= 15 * comma;
		    }

		    /* add score for stable dpnd */
		    if (g_ptr->num + 1 == d_ptr->num &&
			(((check_feature(d_ptr->f, "AS")) &&
			  (check_feature(g_ptr->f, "VE") ||
			   check_feature(g_ptr->f, "VV"))) ||

			 ((check_feature(d_ptr->f, "CD")) &&
			  (check_feature(g_ptr->f, "DT"))) ||

			 ((check_feature(d_ptr->f, "CC")) &&
			  (check_feature(g_ptr->f, "VV") ||
			   check_feature(g_ptr->f, "VA"))) ||

			 ((check_feature(d_ptr->f, "PN")) &&
			  (check_feature(g_ptr->f, "P"))) ||

			 ((check_feature(d_ptr->f, "NT")) &&
			  (check_feature(g_ptr->f, "P"))) ||

			 ((check_feature(g_ptr->f, "VV") ||
			   check_feature(g_ptr->f, "VC") ||
			   check_feature(g_ptr->f, "VE")) &&
			  (check_feature(d_ptr->f, "VC") ||
			   check_feature(d_ptr->f, "VE") ||
			   check_feature(d_ptr->f, "VV")))||

			 ((check_feature(d_ptr->f, "DEC")) &&
			  (check_feature(g_ptr->f, "VC") ||
			   check_feature(g_ptr->f, "VA") ||
			   check_feature(g_ptr->f, "VE") ||
			   check_feature(g_ptr->f, "VV"))) ||

			 ((check_feature(d_ptr->f, "DER")) &&
			  (check_feature(g_ptr->f, "VV"))) ||

			 ((check_feature(d_ptr->f, "ETC")) &&
			  (check_feature(g_ptr->f, "NN") ||
			   check_feature(g_ptr->f, "NR"))))) {
			one_score += 15;
		    }
		    if (g_ptr->num < d_ptr->num &&
			(((check_feature(g_ptr->f, "AD")) &&
			  (check_feature(d_ptr->f, "CC")))||

			 ((check_feature(g_ptr->f, "CD")) &&
			  (check_feature(d_ptr->f, "CC")))||

			 ((check_feature(g_ptr->f, "VV") ||
			   check_feature(g_ptr->f, "VC") ||
			   check_feature(g_ptr->f, "VE")) &&
			  (check_feature(d_ptr->f, "VC") ||
			   check_feature(d_ptr->f, "VE") ||
			   check_feature(d_ptr->f, "VV")))||

			 ((check_feature(g_ptr->f, "DT")) &&
			  (check_feature(d_ptr->f, "CD")||
			   check_feature(d_ptr->f, "M")))||

			 ((check_feature(g_ptr->f, "LB")) &&
			  (check_feature(d_ptr->f, "CC")))||

			 ((check_feature(g_ptr->f, "P")) &&
			  (check_feature(d_ptr->f, "VV") ||
			   check_feature(d_ptr->f, "VA") ||
			   check_feature(d_ptr->f, "VC") ||
			   check_feature(d_ptr->f, "VE"))) ||

			 ((check_feature(g_ptr->f, "NN")) &&
			  (check_feature(d_ptr->f, "X")))||

			 ((check_feature(g_ptr->f, "P")) &&
			  (check_feature(d_ptr->f, "CS") ||
			   check_feature(d_ptr->f, "DT") ||
			   check_feature(d_ptr->f, "JJ") ||
			   check_feature(d_ptr->f, "NR") ||
//			   check_feature(d_ptr->f, "NN") ||
			   check_feature(d_ptr->f, "NT") ||
			   check_feature(d_ptr->f, "PN")))||

			 ((check_feature(g_ptr->f, "VA")) &&
			  (check_feature(d_ptr->f, "NR"))) ||

			 ((check_feature(g_ptr->f, "VC")) &&
			  (check_feature(d_ptr->f, "DT"))) ||

			 ((check_feature(g_ptr->f, "VE")) &&
			  (check_feature(d_ptr->f, "AS") ||
			   check_feature(d_ptr->f, "CC") ||
			   check_feature(d_ptr->f, "NT") ||
			   check_feature(d_ptr->f, "PN")))||

			 ((check_feature(g_ptr->f, "VV")) &&
			  (check_feature(d_ptr->f, "AS") ||
			   check_feature(d_ptr->f, "NN") ||
			   check_feature(d_ptr->f, "DER"))))) {
			one_score += 10;
		    }
		    if (check_feature(g_ptr->f, "P") && check_feature(d_ptr->f, "CS")) {
			one_score += 10;
		    }
		    if (check_feature(g_ptr->f, "AD") && check_feature(d_ptr->f, "DEC")) {
			one_score -= 20;
		    }
		    if (check_feature(g_ptr->f, "NN") && check_feature(d_ptr->f, "VV")) {
			one_score -= 20;
		    }
		    if (check_feature(g_ptr->f, "DEG") && check_feature(d_ptr->f, "NN")) {
			one_score -= 20;
		    }
		}
	    }
	}

	cky_ptr = cky_ptr->direction == LtoR ? cky_ptr->right : cky_ptr->left;
    }

    /* �Ѹ��ξ�硤�ǽ�Ū��̤��,����,���,�˳�,Ϣ�ν������Ф���
       ����,���,�˳ʤΥ���å�ʬ����������Ϳ���� */
    if (pred_p) {
	one_score += check_scase(g_ptr, scase_check, 0, *un_count);
    }

    if (OptDisplay == OPT_DEBUG) {
	printf("%.3f\n", one_score);
    }

    return one_score;
}

/* count dependency possibilities */
int count_distance(SENTENCE_DATA *sp, CKY *cky_ptr, BNST_DATA *g_ptr, int *pos) {
    int i, count = 0;
    *pos = 0;

    for (i = cky_ptr->left->b_ptr->num + 1; i < sp->Bnst_num; i++) {
	if (check_dpnd_possibility(sp, cky_ptr->left->b_ptr->num, i, cky_ptr->i, 
				   ((i == sp->Bnst_num - 1) && count == 0) ? TRUE : FALSE)) {
	    if (i == g_ptr->num) {
		*pos = count;
	    }
	    count++;
	}
    }

    return count;
}

/* scoring function based on case structure probabilities */
double calc_case_probability(SENTENCE_DATA *sp, CKY *cky_ptr, TOTAL_MGR *Best_mgr) {
    CKY *right_ptr = cky_ptr->right, *orig_cky_ptr = cky_ptr;
    BNST_DATA *g_ptr = cky_ptr->b_ptr, *d_ptr;
    TAG_DATA *t_ptr;
    CF_PRED_MGR *cpm_ptr, *pre_cpm_ptr;
    int i, pred_p = 0, count, pos, default_pos, child_num = 0;
    int renyou_modifying_num = 0, adverb_modifying_num = 0, noun_modifying_num = 0, flag;
    double one_score = 0, orig_score;
    char *para_key;

    /* �оݤ��Ѹ��ʳ��Υ������򽸤�� (right�򤿤ɤ�ʤ���left�Υ�������­��) */
    while (cky_ptr) {
	if (cky_ptr->left) {
	    one_score += cky_ptr->left->score;
	}
	cky_ptr = cky_ptr->right;
    }
    if (OptDisplay == OPT_DEBUG) {
	printf("%.3f=>", one_score);
    }

    cky_ptr = orig_cky_ptr;

    if (check_feature(g_ptr->f, "����ñ�̼�:-1") && g_ptr->tag_num > 1) { /* ���Τ� */
	t_ptr = g_ptr->tag_ptr + g_ptr->tag_num - 2;
    }
    else {
	t_ptr = g_ptr->tag_ptr + g_ptr->tag_num - 1;
    }

    if (t_ptr->cf_num > 0) { /* predicate or something which has case frames */
	cky_ptr->cpm_ptr->pred_b_ptr = t_ptr;
	set_data_cf_type(cky_ptr->cpm_ptr); /* set predicate type */
	if (cky_ptr->cpm_ptr->cf.type == CF_PRED) { /* currently, restrict to predicates */
	    pred_p = 1;
	    cpm_ptr = cky_ptr->cpm_ptr;
	    cpm_ptr->score = -1;
	    cpm_ptr->result_num = 0;
	    cpm_ptr->tie_num = 0;
	    cpm_ptr->cmm[0].cf_ptr = NULL;
	    cpm_ptr->decided = CF_UNDECIDED;

	    cpm_ptr->cf.pred_b_ptr = t_ptr;
	    t_ptr->cpm_ptr = cpm_ptr;
	    cpm_ptr->cf.element_num = 0;
	}
	else {
	    cky_ptr->cpm_ptr->pred_b_ptr = NULL;
	}
    }
    else {
	cky_ptr->cpm_ptr->pred_b_ptr = NULL;
    }

    /* check each child */
    while (cky_ptr) {
	if (cky_ptr->left && cky_ptr->para_flag == 0) {
	    d_ptr = cky_ptr->left->b_ptr;
	    flag = 0;

	    /* relax penalty */
	    if (cky_ptr->dpnd_type == 'R') {
		one_score += -1000;
	    }

	    /* coordination */
	    if (OptParaFix == 0) {
		if (d_ptr->para_num != -1 && (para_key = check_feature(d_ptr->f, "�¥�"))) {
		    if (cky_ptr->dpnd_type == 'P') {
			one_score += get_para_exist_probability(para_key, cky_ptr->para_score, TRUE);
			one_score += get_para_ex_probability(para_key, cky_ptr->para_score, d_ptr->tag_ptr + d_ptr->tag_num - 1, t_ptr);
			flag++;
		    }
		    else {
			one_score += get_para_exist_probability(para_key, sp->para_data[d_ptr->para_num].max_score, FALSE);
		    }
		}
	    }

	    /* case component */
	    if (cky_ptr->dpnd_type != 'P' && pred_p) {
		make_work_mgr_dpnd_check(sp, cky_ptr, d_ptr);
		if (make_data_cframe_child(sp, cpm_ptr, d_ptr->tag_ptr + d_ptr->tag_num - 1, child_num, t_ptr->num == d_ptr->num + 1 ? TRUE : FALSE)) {
		    add_coordinated_phrases(cky_ptr->left, &(cpm_ptr->elem_b_ptr[cpm_ptr->cf.element_num - 1]->next));
		    child_num++;
		    flag++;
		}

		if ((check_feature(d_ptr->f, "��:Ϣ��") && 
		     (!check_feature(d_ptr->f, "�Ѹ�") || !check_feature(d_ptr->f, "ʣ�缭"))) || 
		    check_feature(d_ptr->f, "����")) {
		    flag++;
		}
	    }

	    /* clausal modifiee */
	    if (check_feature(d_ptr->f, "��:Ϣ��") && 
		cky_ptr->left->cpm_ptr->pred_b_ptr) { /* �ʥե졼����äƤ���٤� */
		pre_cpm_ptr = cky_ptr->left->cpm_ptr;
		pre_cpm_ptr->pred_b_ptr->cpm_ptr = pre_cpm_ptr;
		make_work_mgr_dpnd_check(sp, cky_ptr, d_ptr);
		make_data_cframe_rentai_simple(pre_cpm_ptr, pre_cpm_ptr->pred_b_ptr, t_ptr);
		add_coordinated_phrases(cky_ptr->right, &(pre_cpm_ptr->elem_b_ptr[pre_cpm_ptr->cf.element_num - 1]->next));

		orig_score = pre_cpm_ptr->score;
		one_score -= orig_score;
		one_score += find_best_cf(sp, pre_cpm_ptr, get_closest_case_component(sp, pre_cpm_ptr), 1);
		pre_cpm_ptr->score = orig_score;
		pre_cpm_ptr->cf.element_num--;
		flag++;
	    }

	    if (OptParaFix == 0 && flag == 0) { /* ̾��ʥե졼��� */
		make_work_mgr_dpnd_check(sp, cky_ptr, d_ptr);
		(d_ptr->tag_ptr + d_ptr->tag_num - 1)->next = NULL; /* �������ǳ�Ǽ��(����¦) */
		t_ptr->next = NULL; /* �������ǳ�Ǽ��(����¦) */
		add_coordinated_phrases(cky_ptr->left, &((d_ptr->tag_ptr + d_ptr->tag_num - 1)->next));
		add_coordinated_phrases(cky_ptr->right, &(t_ptr->next));
		one_score += get_noun_co_ex_probability(d_ptr->tag_ptr + d_ptr->tag_num - 1, t_ptr);
		noun_modifying_num++;
	    }

	    /* penalty of adverb etc. (tentative) */
	    if (check_feature(d_ptr->f, "��:Ϣ��") && !check_feature(d_ptr->f, "�Ѹ�")) {
		count = count_distance(sp, cky_ptr, g_ptr, &pos);
		default_pos = (d_ptr->dpnd_rule->preference == -1) ?
		    count : d_ptr->dpnd_rule->preference;
		one_score -= abs(default_pos - 1 - pos) * 5;
	    }
	}
	cky_ptr = cky_ptr->right;
    }

    if (pred_p) {
	t_ptr->cpm_ptr = cpm_ptr;

	/* �Ѹ�ʸ�᤬�֡ʡ���ˡ��ˡפΤȤ� 
	   �֤���פγʥե졼����Ф��ƥ˳�(Ʊʸ��)������
	   ��ʤϻҶ��ν����ǰ����� */
	if (check_feature(t_ptr->f, "���Ѹ�Ʊʸ��")) {
	    if (_make_data_cframe_pp(cpm_ptr, t_ptr, TRUE)) {
		_make_data_cframe_sm(cpm_ptr, t_ptr);
		_make_data_cframe_ex(cpm_ptr, t_ptr);
		cpm_ptr->elem_b_ptr[cpm_ptr->cf.element_num] = t_ptr;
		cpm_ptr->elem_b_num[cpm_ptr->cf.element_num] = child_num;
		cpm_ptr->cf.weight[cpm_ptr->cf.element_num] = 0;
		cpm_ptr->cf.adjacent[cpm_ptr->cf.element_num] = TRUE;
		cpm_ptr->cf.element_num++;
	    }
	}

	/* call case structure analysis */
	one_score += find_best_cf(sp, cpm_ptr, get_closest_case_component(sp, cpm_ptr), 1);

	/* for each child */
	cky_ptr = orig_cky_ptr;
	while (cky_ptr) {
	    if (cky_ptr->left) {
		d_ptr = cky_ptr->left->b_ptr;
		if (cky_ptr->dpnd_type != 'P') {
		    /* modifying predicate */
		    if (check_feature(d_ptr->f, "��:Ϣ��") && check_feature(d_ptr->f, "�Ѹ�") && 
			!check_feature(d_ptr->f, "ʣ�缭")) {
			make_work_mgr_dpnd_check(sp, cky_ptr, d_ptr);
			one_score += calc_vp_modifying_probability(t_ptr, cpm_ptr->cmm[0].cf_ptr, 
								   d_ptr->tag_ptr + d_ptr->tag_num - 1, 
								   cky_ptr->left->cpm_ptr->cmm[0].cf_ptr);
			renyou_modifying_num++;
		    }

		    /* modifying adverb */
		    if ((check_feature(d_ptr->f, "��:Ϣ��") && !check_feature(d_ptr->f, "�Ѹ�")) || 
			check_feature(d_ptr->f, "����")) {
			make_work_mgr_dpnd_check(sp, cky_ptr, d_ptr);
			one_score += calc_adv_modifying_probability(t_ptr, cpm_ptr->cmm[0].cf_ptr, 
								    d_ptr->tag_ptr + d_ptr->tag_num - 1);
			adverb_modifying_num++;
		    }
		}
	    }
	    cky_ptr = cky_ptr->right;
	}

	one_score += calc_vp_modifying_num_probability(t_ptr, cpm_ptr->cmm[0].cf_ptr, renyou_modifying_num);
	one_score += calc_adv_modifying_num_probability(t_ptr, cpm_ptr->cmm[0].cf_ptr, adverb_modifying_num);
    }

    /* ̾�콤���Ŀ����� */
    if (OptParaFix == 0 && !pred_p || check_feature(t_ptr->f, "�Ѹ�:Ƚ")) {
	one_score += get_noun_co_num_probability(t_ptr, noun_modifying_num);
    }

    if (OptDisplay == OPT_DEBUG) {
	printf("%.3f\n", one_score);
    }

    return one_score;
}

int relax_barrier_for_P(CKY *cky_ptr, int dep, int gov, int *dep_check) {
    while (cky_ptr) {
	if (cky_ptr->left && 
	    cky_ptr->dpnd_type == 'P') {
	    if (*(dep_check + dep) >= cky_ptr->left->j) { /* ����κ�¦���ɤ�����ʤ顢��¦�ޤ�OK�Ȥ��� */
		return TRUE;
	    }
	    else if (cky_ptr->para_flag) {
		if (relax_barrier_for_P(cky_ptr->left, dep, gov, dep_check)) {
		    return TRUE;
		}
	    }
	}
	cky_ptr = cky_ptr->right; /* go below */
    }

    return FALSE;
}

int relax_dpnd_for_P(CKY *cky_ptr, int dep, int gov) {
    int i;

    while (cky_ptr) {
	if (cky_ptr->left && 
	    cky_ptr->dpnd_type == 'P') {
	    for (i = cky_ptr->left->i; i <= cky_ptr->left->j; i++) {
		if (Dpnd_matrix[dep][i] && Quote_matrix[dep][i]) {
		    return TRUE;
		}
	    }
	}
	cky_ptr = cky_ptr->right;
    }

    return FALSE;
}

void fix_predicate_coordination(SENTENCE_DATA* sp) {
    int i, j, k;

    for (i = 0; i < sp->Para_num; i++) {
	if (sp->para_data[i].type == PARA_KEY_P) { /* predicate coordination */
	    if (sp->para_data[i].status == 'x') {
		sp->para_data[i].max_score = -1;
	    }

	    /* modify Para_matrix */
	    for (j = 0; j < sp->Bnst_num; j++) {
		for (k = 0; k < sp->Bnst_num; k++) {
		    /* preserve the best coordination */
		    if (sp->para_data[i].status == 'x' || 
			j != sp->para_data[i].max_path[0] || k != sp->para_data[i].jend_pos) { 
			Para_matrix[i][j][k] = -1;
		    }
		}
	    }

	    /* modify Dpnd_matrix */
	    if (sp->para_data[i].status != 'x') {
		for (j = sp->para_data[i].key_pos + 1; j < sp->Bnst_num; j++) {
		    if (j == sp->para_data[i].jend_pos) {
			Dpnd_matrix[sp->para_data[i].key_pos][j] = 'R';
		    }
		    else {
			Dpnd_matrix[sp->para_data[i].key_pos][j] = 0;
		    }
		}
	    }
	}
    }
}


void discard_bad_coordination(SENTENCE_DATA* sp) {
    int i, j, k;

    for (i = 0; i < sp->Para_num; i++) {
	if (sp->para_data[i].status == 'x') {
	    for (j = 0; j < sp->Bnst_num; j++) {
		for (k = 0; k < sp->Bnst_num; k++) {
		    Para_matrix[i][j][k] = -1;
		}
	    }
	}
    }
}

void handle_incomplete_coordination(SENTENCE_DATA* sp) {
    int i, j;

    for (i = 0; i < sp->Bnst_num; i++) {
	for (j = 0; j < sp->Bnst_num; j++) {
	    if (Mask_matrix[i][j] == 3 && 
		Dpnd_matrix[i][j] == 0) {
		Dpnd_matrix[i][j] = (int)'I';
	    }
	}
    }
}

void extend_para_matrix(SENTENCE_DATA* sp) {
    int i, j, k, l, flag, offset, max_pos;
    double max_score;

    for (i = 0; i < sp->Para_num; i++) {
	if (sp->para_data[i].max_score >= 0) {
	    if (sp->para_data[i].type == PARA_KEY_P) {
		offset = 0;
	    }
	    else { /* in case of noun coordination, only permit modifiers to the words before the para key */
		offset = 1;
	    }

	    /* for each endpos */
	    for (l = sp->para_data[i].key_pos + 1; l < sp->Bnst_num; l++) {
		max_score = -INT_MAX;
		for (j = sp->para_data[i].iend_pos; j >= 0; j--) {
		    if (max_score < Para_matrix[i][j][l]) {
			max_score = Para_matrix[i][j][l];
			max_pos = j;
		    }
		}
		if (max_score >= 0) {
		    /* go up to search modifiers */
		    for (j = max_pos - 1; j >= 0; j--) {
			if (check_stop_extend(sp, j)) { /* extention stop */
			    break;
			}

			/* check dependency to pre-conjunct */
			flag = 0;
			for (k = j + 1; k <= sp->para_data[i].key_pos - offset; k++) {
			    if (Dpnd_matrix[j][k] && Quote_matrix[j][k]) {
				Para_matrix[i][j][l] = max_score;
				flag = 1;
				if (OptDisplay == OPT_DEBUG) {
				    printf("Para Extension (%s-%s-%s) -> %s\n", 
					   (sp->bnst_data + max_pos)->head_ptr->Goi, 
					   (sp->bnst_data + sp->para_data[i].key_pos)->head_ptr->Goi, 
					   (sp->bnst_data + l)->head_ptr->Goi, 
					   (sp->bnst_data + j)->head_ptr->Goi);
				}
				break;
			    }
			}
			if (flag == 0) {
			    break;
			}
		    }
		}
	    }
	}
    }
}

void set_cky(SENTENCE_DATA *sp, CKY *cky_ptr, CKY *left_ptr, CKY *right_ptr, int i, int j, int k, 
	     char dpnd_type, int direction, int index) {
    int l;

    cky_ptr->index = index;
    cky_ptr->i = i;
    cky_ptr->j = j;
    cky_ptr->next = NULL;
    cky_ptr->left = left_ptr;
    cky_ptr->right = right_ptr;
    cky_ptr->direction = direction;
    cky_ptr->dpnd_type = dpnd_type;
    cky_ptr->cp = 'a' + j;
    if (cky_ptr->direction == RtoL) {
	cky_ptr->b_ptr = cky_ptr->left->b_ptr;
    }
    else {
	cky_ptr->b_ptr = cky_ptr->right ? cky_ptr->right->b_ptr : sp->bnst_data + j;
    }
    cky_ptr->un_count = 0;
    for (l = 0; l < SCASE_CODE_SIZE; l++) cky_ptr->scase_check[l] = 0;
    cky_ptr->para_flag = 0;
    cky_ptr->para_score = -1;
    cky_ptr->score = 0;
}

CKY *new_cky_data(int *cky_table_num) {
    CKY *cky_ptr;

    cky_ptr = &(cky_table[*cky_table_num]);
    if (OptAnalysis == OPT_CASE && *cky_table_num > cpm_allocated_cky_num) {
	cky_ptr->cpm_ptr = (CF_PRED_MGR *)malloc_data(sizeof(CF_PRED_MGR), "new_cky_data");
	init_case_frame(&(cky_ptr->cpm_ptr->cf));
	cky_ptr->cpm_ptr->cf.type = 0;
	cpm_allocated_cky_num = *cky_table_num;
    }

    (*cky_table_num)++;
    if (*cky_table_num >= CKY_TABLE_MAX) {
	fprintf(stderr, ";;; cky_table_num exceeded maximum\n");
	return NULL;
    }

    return cky_ptr;
}

void copy_cky_data(CKY *dest, CKY *src) {
    int l;

    if (dest == src) {
	return;
    }

    dest->index = src->index;
    dest->i = src->i;
    dest->j = src->j;
    dest->next = src->next;
    dest->left = src->left;
    dest->right = src->right;
    dest->direction = src->direction;
    dest->dpnd_type = src->dpnd_type;
    dest->cp = src->cp;
    dest->direction = src->direction;
    dest->b_ptr = src->b_ptr;
    dest->un_count = src->un_count;
    for (l = 0; l < SCASE_CODE_SIZE; l++) dest->scase_check[l] = src->scase_check[l];
    dest->para_flag = src->para_flag;
    dest->para_score = src->para_score;
    dest->score = src->score;
    dest->cpm_ptr = src->cpm_ptr;
}

int after_cky(SENTENCE_DATA *sp, TOTAL_MGR *Best_mgr, CKY *cky_ptr) {
    int i, j;

    /* count the number of predicates */
    Best_mgr->pred_num = 0;
    for (i = 0; i < sp->Tag_num; i++) {
	if ((sp->tag_data + i)->cf_num > 0 && 
	    (sp->tag_data + i)->cpm_ptr && (sp->tag_data + i)->cpm_ptr->cf.type == CF_PRED && 
	    (((sp->tag_data + i)->inum == 0 && /* the last basic phrase in a bunsetsu */
	      !check_feature((sp->tag_data + i)->b_ptr->f, "����ñ�̼�:-1")) || 
	     ((sp->tag_data + i)->inum == 1 && 
	      check_feature((sp->tag_data + i)->b_ptr->f, "����ñ�̼�:-1")))) { 
	    (sp->tag_data + i)->pred_num = Best_mgr->pred_num;
	    Best_mgr->pred_num++;
	}
    }

    /* for all possible structures */
    while (cky_ptr) {
	for (i = 0; i < Best_mgr->pred_num; i++) {
	    Best_mgr->cpm[i].pred_b_ptr = NULL;
	}

	if (OptDisplay == OPT_DEBUG) {
	    printf("---------------------\n");
	    printf("score=%.3f\n", cky_ptr->score);
	}

	Best_mgr->dpnd.head[cky_ptr->b_ptr->num] = -1;
	Best_mgr->score = cky_ptr->score;
	sp->score = Best_mgr->score;
	convert_to_dpnd(sp, Best_mgr, cky_ptr);

	/* ̵�ʽ�°: ����ʸ��η�������˽������ */
	for (i = 0; i < sp->Bnst_num - 1; i++) {
	    if (Best_mgr->dpnd.head[i] < 0) {
		/* ���ꤨ�ʤ�������� */
		if (i >= Best_mgr->dpnd.head[i + Best_mgr->dpnd.head[i]]) {
		    if (Language != CHINESE) {
			Best_mgr->dpnd.head[i] = sp->Bnst_num - 1; /* ʸ���˴��� */
		    }
		    continue;
		}
		Best_mgr->dpnd.head[i] = Best_mgr->dpnd.head[i + Best_mgr->dpnd.head[i]];
		/* Best_mgr->dpnd.check[i].pos[0] = Best_mgr->dpnd.head[i]; */
	    }
	}

	/* �ʲ��Ϸ�̤ξ����feature�� */
	if (OptAnalysis == OPT_CASE) {
	    /* �ʲ��Ϸ�̤��Ѹ����ܶ�feature�� */
	    for (i = 0; i < sp->Best_mgr->pred_num; i++) {
		assign_nil_assigned_components(sp, &(sp->Best_mgr->cpm[i])); /* ̤�б������Ǥν��� */

		assign_case_component_feature(sp, &(sp->Best_mgr->cpm[i]), FALSE);

		/* �ʥե졼��ΰ�̣������Ѹ����ܶ�feature�� */
		for (j = 0; j < sp->Best_mgr->cpm[i].cmm[0].cf_ptr->element_num; j++) {
		    append_cf_feature(&(sp->Best_mgr->cpm[i].pred_b_ptr->f), 
				      &(sp->Best_mgr->cpm[i]), sp->Best_mgr->cpm[i].cmm[0].cf_ptr, j);
		}
	    }
	}

	/* to tree structure */
	dpnd_info_to_bnst(sp, &(Best_mgr->dpnd));
	para_recovery(sp);
	if (!(OptExpress & OPT_NOTAG)) {
	    dpnd_info_to_tag(sp, &(Best_mgr->dpnd));
	}
	if (make_dpnd_tree(sp)) {
	    bnst_to_tag_tree(sp); /* ����ñ�̤��ڤ� */

	    /* ��¤�����Υ롼��Ŭ�� */
	    assign_general_feature(sp->tag_data, sp->Tag_num, AfterDpndTagRuleType, FALSE, FALSE);

	    /* record case analysis results */
	    if (OptAnalysis == OPT_CASE) {
		for (i = 0; i < Best_mgr->pred_num; i++) {
		    if (Best_mgr->cpm[i].result_num != 0 && 
			Best_mgr->cpm[i].cmm[0].cf_ptr->cf_address != -1 && 
			Best_mgr->cpm[i].cmm[0].score != CASE_MATCH_FAILURE_PROB) {
			record_case_analysis(sp, &(Best_mgr->cpm[i]), NULL, FALSE);

			/* �ʲ��Ϥη�̤��Ѥ��Ʒ�����ۣ�������� */
			verb_lexical_disambiguation_by_case_analysis(&(sp->Best_mgr->cpm[i]));
			noun_lexical_disambiguation_by_case_analysis(&(sp->Best_mgr->cpm[i]));
		    }
		}
	    }

	    /* print for debug or nbest */
	    if (OptNbest == TRUE) {
		print_result(sp, 0);

		if (OptDisplay == OPT_DEBUG) { /* case analysis results */
		    for (i = 0; i < Best_mgr->pred_num; i++) {
			print_data_cframe(&(Best_mgr->cpm[i]), &(Best_mgr->cpm[i].cmm[0]));
			for (j = 0; j < Best_mgr->cpm[i].result_num; j++) {
			    print_crrspnd(&(Best_mgr->cpm[i]), &(Best_mgr->cpm[i].cmm[j]));
			}
		    }
		}
	    }
	    else if (OptDisplay == OPT_DEBUG) {
		print_kakari(sp, OptExpress & OPT_NOTAG ? OPT_NOTAGTREE : OPT_TREE);
	    }
	}

	cky_ptr = cky_ptr->next;
    }

    return TRUE;
}

void sort_cky_ptrs(CKY **orig_cky_ptr_ptr, int beam) {
    CKY *cky_ptr = *orig_cky_ptr_ptr, **start_cky_ptr_ptr = orig_cky_ptr_ptr, *pre_ptr, *best_ptr, *best_pre_ptr;
    double best_score;
    int i;

    for (i = 0; i < beam && cky_ptr; i++) {
	best_score = -INT_MAX;
	best_pre_ptr = NULL;
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

	if (best_pre_ptr) { /* best_ptr is not at the beginning */
	    best_pre_ptr->next = best_ptr->next;
	    best_ptr->next = *start_cky_ptr_ptr;
	    *start_cky_ptr_ptr = best_ptr;
	}

	start_cky_ptr_ptr = &(best_ptr->next);
	cky_ptr = best_ptr->next;
    }

    // best_ptr->next = NULL; /* do not consider more candidates than beam */
}

int cky (SENTENCE_DATA *sp, TOTAL_MGR *Best_mgr) {
    int i, j, k, l, m, sort_flag, sen_len, cky_table_num, pre_cky_table_num, dep_check[BNST_MAX];
    double best_score, para_score;
    char dpnd_type;
    CKY *cky_ptr, *left_ptr, *right_ptr, *best_ptr, *pre_ptr, *best_pre_ptr, *start_ptr, *sort_pre_ptr;
    CKY **next_pp, **next_pp_for_ij;

    cky_table_num = 0;

    /* initialize */
    for (i = 0; i < sp->Bnst_num; i++) {
	dep_check[i] = -1;
	Best_mgr->dpnd.head[i] = -1;
	Best_mgr->dpnd.type[i] = 'D';
    }

    if (OptParaFix == 0) {
	discard_bad_coordination(sp);
	/* fix_predicate_coordination(sp); */
	/* extend_para_matrix(sp); */
	handle_incomplete_coordination(sp);
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
		if ((cky_ptr = new_cky_data(&cky_table_num)) == NULL) {
		    return FALSE;
		}
		cky_matrix[i][j] = cky_ptr;

		set_cky(sp, cky_ptr, NULL, NULL, i, j, -1, 0, LtoR, -1);
		cky_ptr->score = OptAnalysis == OPT_CASE ? 
		    calc_case_probability(sp, cky_ptr, Best_mgr) : calc_score(sp, cky_ptr);
	    }
	    else {
		next_pp_for_ij = NULL;	/* ���ΰ��֤˰�Ĥ�礬�Ǥ��Ƥʤ��� */
		pre_cky_table_num = cky_table_num;

		/* merge (i .. i+k) and (i+k+1 .. j) */
		for (k = 0; k < j - i; k++) {
		    para_score = (sp->bnst_data + i + k)->para_num == -1 ? -1 : 
			Para_matrix[(sp->bnst_data + i + k)->para_num][i][j];
		    next_pp = NULL;
		    left_ptr = cky_matrix[i][i + k];
		    while (left_ptr) {
			right_ptr = cky_matrix[i + k + 1][j];
			while (right_ptr) {
			    /* make a phrase if condition is satisfied */
			    if ((dpnd_type = check_dpnd_possibility(sp, left_ptr->b_ptr->num, right_ptr->b_ptr->num, i, 
								    (j == sp->Bnst_num - 1) && dep_check[i + k] == -1 ? TRUE : FALSE)) && 
				(dpnd_type == 'P' || 
				 dep_check[i + k] <= 0 || /* no barrier */
				 dep_check[i + k] >= j || /* before barrier */
				 (OptParaFix == 0 && relax_barrier_for_P(right_ptr, i + k, j, dep_check)))) { /* barrier relaxation for P */

				if (Language == CHINESE) {
				    for (l = 0; l < Chi_dpnd_matrix[left_ptr->b_ptr->num][right_ptr->b_ptr->num].count; l++) {
					if (Chi_dpnd_matrix[left_ptr->b_ptr->num][right_ptr->b_ptr->num].direction[l] == 'B') {
					    /* check R first */
					    if (check_chi_dpnd_possibility(i, j, k, left_ptr, right_ptr, sp, 'R')) {
						if ((cky_ptr = new_cky_data(&cky_table_num)) == NULL) {
						    return FALSE;
						}
						if (next_pp == NULL) {
						    start_ptr = cky_ptr;
						}
						else {
						    *next_pp = cky_ptr;
						}

						if (Mask_matrix[i][i + k] == 'N' && Mask_matrix[i + k + 1][j] == 'N') {
						    set_cky(sp, cky_ptr, left_ptr, right_ptr, i, j, k, 'R', LtoR, l); 
						}
						else if (Mask_matrix[i][i + k] == 'G' && Mask_matrix[i + k + 1][j] == 'G') {
						    set_cky(sp, cky_ptr, left_ptr, right_ptr, i, j, k, 'R', LtoR, l); 
						}
						else {
						    set_cky(sp, cky_ptr, left_ptr, right_ptr, i, j, k, 'R', LtoR, l);
						}

						next_pp = &(cky_ptr->next);
					
						if (OptDisplay == OPT_DEBUG) {
						    printf("   (%d,%d), (%d,%d) b=%d [%s%s%s], %c(para=%.3f), score=", 
							   i, i + k, i + k + 1, j, dep_check[i + k], 
							   left_ptr->b_ptr->head_ptr->Goi, 
							   "->", 
							   right_ptr->b_ptr->head_ptr->Goi, 
							   'R', para_score);
						}

						cky_ptr->para_score = para_score;
						cky_ptr->score = OptAnalysis == OPT_CASE ? 
						    calc_case_probability(sp, cky_ptr, Best_mgr) : calc_score(sp, cky_ptr);

						if (Mask_matrix[i][i + k] == 'N' && Mask_matrix[i + k + 1][j] == 'N') {
						    cky_ptr->score += 50;
						    if (OptDisplay == OPT_DEBUG) {
							printf("=>%.3f\n", cky_ptr->score);
						    } 
						}
						else if (Mask_matrix[i][i + k] == 'G' && Mask_matrix[i + k + 1][j] == 'G') {
						    cky_ptr->score += 50;
						    if (OptDisplay == OPT_DEBUG) {
							printf("=>%.3f\n", cky_ptr->score);
						    } 
						}
					    }

					    /* then check L */
					    if (check_chi_dpnd_possibility(i, j, k, left_ptr, right_ptr, sp, 'L')) {
						if ((cky_ptr = new_cky_data(&cky_table_num)) == NULL) {
						    return FALSE;
						}
						if (next_pp == NULL) {
						    start_ptr = cky_ptr;
						}
						else {
						    *next_pp = cky_ptr;
						}

						if (Mask_matrix[i][i + k] == 'V' && Mask_matrix[i + k + 1][j] == 'V') {
						    set_cky(sp, cky_ptr, left_ptr, right_ptr, i, j, k, 'L', RtoL, l); 
						}
						else if (Mask_matrix[i][i + k] == 'E' && Mask_matrix[i + k + 1][j] == 'E') {
						    set_cky(sp, cky_ptr, left_ptr, right_ptr, i, j, k, 'L', RtoL, l); 
						}
						else {
						    set_cky(sp, cky_ptr, left_ptr, right_ptr, i, j, k, 'L', RtoL, l);
						}

						next_pp = &(cky_ptr->next);
					
						if (OptDisplay == OPT_DEBUG) {
						    printf("   (%d,%d), (%d,%d) b=%d [%s%s%s], %c(para=%.3f), score=", 
							   i, i + k, i + k + 1, j, dep_check[i + k], 
							   left_ptr->b_ptr->head_ptr->Goi, 
							   "<-", 
							   right_ptr->b_ptr->head_ptr->Goi, 
							   'L', para_score);
						}

						cky_ptr->para_score = para_score;
						cky_ptr->score = OptAnalysis == OPT_CASE ? 
						    calc_case_probability(sp, cky_ptr, Best_mgr) : calc_score(sp, cky_ptr);

						if (Mask_matrix[i][i + k] == 'V' && Mask_matrix[i + k + 1][j] == 'V') {
						    cky_ptr->score += 50;
						    if (OptDisplay == OPT_DEBUG) {
							printf("=>%.3f\n", cky_ptr->score);
						    } 
						}
						else if (Mask_matrix[i][i + k] == 'E' && Mask_matrix[i + k + 1][j] == 'E') {
						    cky_ptr->score += 50;
						    if (OptDisplay == OPT_DEBUG) {
							printf("=>%.3f\n", cky_ptr->score);
						    } 
						}
					    }
					}
					else {
					    if (!(check_chi_dpnd_possibility(i, j, k, left_ptr, right_ptr, sp, Chi_dpnd_matrix[left_ptr->b_ptr->num][right_ptr->b_ptr->num].direction[l]))) {
						continue;
					    }
					    if ((cky_ptr = new_cky_data(&cky_table_num)) == NULL) {
						return FALSE;
					    }
					    if (next_pp == NULL) {
						start_ptr = cky_ptr;
					    }
					    else {
						*next_pp = cky_ptr;
					    }

					    if (Mask_matrix[i][i + k] == 'N' && Mask_matrix[i + k + 1][j] == 'N') {
						set_cky(sp, cky_ptr, left_ptr, right_ptr, i, j, k, 'R', LtoR, l); 
					    }
					    else if (Mask_matrix[i][i + k] == 'G' && Mask_matrix[i + k + 1][j] == 'G') {
						set_cky(sp, cky_ptr, left_ptr, right_ptr, i, j, k, 'R', LtoR, l); 
					    }
					    else if (Mask_matrix[i][i + k] == 'V' && Mask_matrix[i + k + 1][j] == 'V') {
						set_cky(sp, cky_ptr, left_ptr, right_ptr, i, j, k, 'L', RtoL, l); 
					    }
					    else if (Mask_matrix[i][i + k] == 'E' && Mask_matrix[i + k + 1][j] == 'E') {
						set_cky(sp, cky_ptr, left_ptr, right_ptr, i, j, k, 'L', RtoL, l); 
					    }
					    else {
						set_cky(sp, cky_ptr, left_ptr, right_ptr, i, j, k, 
							Chi_dpnd_matrix[left_ptr->b_ptr->num][right_ptr->b_ptr->num].direction[l], 
							Chi_dpnd_matrix[left_ptr->b_ptr->num][right_ptr->b_ptr->num].direction[l] == 'L' ? RtoL : LtoR, l);
					    }

					    next_pp = &(cky_ptr->next);
					
					    if (OptDisplay == OPT_DEBUG) {
						printf("   (%d,%d), (%d,%d) b=%d [%s%s%s], %c(para=%.3f), score=", 
						       i, i + k, i + k + 1, j, dep_check[i + k], 
						       left_ptr->b_ptr->head_ptr->Goi, 
						       cky_ptr->direction == RtoL ? "<-" : "->", 
						       right_ptr->b_ptr->head_ptr->Goi, 
						       Chi_dpnd_matrix[left_ptr->b_ptr->num][right_ptr->b_ptr->num].direction[l], para_score);
					    }

					    cky_ptr->para_score = para_score;
					    cky_ptr->score = OptAnalysis == OPT_CASE ? 
						calc_case_probability(sp, cky_ptr, Best_mgr) : calc_score(sp, cky_ptr);

					    if (Mask_matrix[i][i + k] == 'N' && Mask_matrix[i + k + 1][j] == 'N') {
						cky_ptr->score += 50;
						if (OptDisplay == OPT_DEBUG) {
						    printf("=>%.3f\n", cky_ptr->score);
						} 
					    }
					    else if (Mask_matrix[i][i + k] == 'G' && Mask_matrix[i + k + 1][j] == 'G') {
						cky_ptr->score += 50;
						if (OptDisplay == OPT_DEBUG) {
						    printf("=>%.3f\n", cky_ptr->score);
						} 
					    }
					    else if (Mask_matrix[i][i + k] == 'V' && Mask_matrix[i + k + 1][j] == 'V') {
						cky_ptr->score += 50;
						if (OptDisplay == OPT_DEBUG) {
						    printf("=>%.3f\n", cky_ptr->score);
						} 
					    }
					    else if (Mask_matrix[i][i + k] == 'E' && Mask_matrix[i + k + 1][j] == 'E') {
						cky_ptr->score += 50;
						if (OptDisplay == OPT_DEBUG) {
						    printf("=>%.3f\n", cky_ptr->score);
						} 
					    }
					}

					if (!OptParaFix) {
					    /* add similarity of coordination */
					    if (cky_ptr->para_score > PARA_THRESHOLD) {
						cky_ptr->score += cky_ptr->para_score * CHI_CKY_BONUS;
					    }
					    if (OptDisplay == OPT_DEBUG) {
						printf("=>%.3f\n", cky_ptr->score);
					    } 
					}
				    }
				}
				else {
				    if ((cky_ptr = new_cky_data(&cky_table_num)) == NULL) {
					return FALSE;
				    }
				    if (next_pp == NULL) {
					start_ptr = cky_ptr;
				    }
				    else {
					*next_pp = cky_ptr;
				    }
				    
				    set_cky(sp, cky_ptr, left_ptr, right_ptr, i, j, k, dpnd_type, 
					    Dpnd_matrix[left_ptr->b_ptr->num][right_ptr->b_ptr->num] == 'L' ? RtoL : LtoR, -1);
				    
				    next_pp = &(cky_ptr->next);
					
				    if (OptDisplay == OPT_DEBUG) {
					printf("   (%d,%d), (%d,%d) b=%d [%s%s%s], %c(para=%.3f), score=", 
					       i, i + k, i + k + 1, j, dep_check[i + k], 
					       left_ptr->b_ptr->head_ptr->Goi, 
					       cky_ptr->direction == RtoL ? "<-" : "->", 
					       right_ptr->b_ptr->head_ptr->Goi, 
					       dpnd_type, para_score);
				    }
				    
				    cky_ptr->para_score = para_score;
				    cky_ptr->score = OptAnalysis == OPT_CASE ? 
					calc_case_probability(sp, cky_ptr, Best_mgr) : calc_score(sp, cky_ptr);
				}
			    }

			    if (Language != CHINESE && 
				OptNbest == FALSE && 
				OptParaFix && 
				!check_feature(right_ptr->b_ptr->f, "�Ѹ�")) { /* consider only the best one if noun */
				break;
			    }
			    right_ptr = right_ptr->next;
			}

			if (Language != CHINESE && 
			    OptNbest == FALSE && 
			    OptParaFix && /* �����ۣ�����������硢̾�줬���󤫤ɤ����ǡ�����̾���ʿ�Ѥ�Ȥ�ս�ǥ��������Ѳ��������� */
			    (!check_feature(left_ptr->b_ptr->f, "�Ѹ�") || /* consider only the best one if noun or VP */
			     check_feature(left_ptr->b_ptr->f, "��:Ϣ��"))) {
			    break;
			}
			left_ptr = left_ptr->next;
		    }

		    if (next_pp) {
			/* ̾��ξ��Ϥ�����1�Ĥ˹ʤäƤ�褤 */

			if (next_pp_for_ij == NULL) {
			    cky_matrix[i][j] = start_ptr;
			}
			else {
			    *next_pp_for_ij = start_ptr;
			}
			next_pp_for_ij = next_pp;

			/* barrier handling */
			if (j != sp->Bnst_num - 1) { /* don't check in case of relaxation */
			    if ((OptParaFix || Dpnd_matrix[i + k][j]) &&  /* don't set barrier in case of P */
				(sp->bnst_data + i + k)->dpnd_rule->barrier.fp[0] && 
				feature_pattern_match(&((sp->bnst_data + i + k)->dpnd_rule->barrier), 
						      (sp->bnst_data + j)->f, 
						      sp->bnst_data + i + k, sp->bnst_data + j) == TRUE) {
				dep_check[i + k] = j; /* set barrier */
			    }
			    else if (dep_check[i + k] == -1) {
				if (Language != CHINESE && 
				    (OptParaFix || Dpnd_matrix[i + k][j]) && /* don't set barrier in case of P */
				    (sp->bnst_data + i + k)->dpnd_rule->preference != -1 && 
				    (sp->bnst_data + i + k)->dpnd_rule->barrier.fp[0] == NULL) { /* no condition */
				    dep_check[i + k] = j; /* set barrier */
				}
				else {
				    dep_check[i + k] = 0; /* ��������������ʤ��Ȥ�1�Ĥ���Ω�������Ȥ򼨤� */
				}
			    }
			}
		    }
		}

		/* coordination that consists of more than 2 phrases */
		if (OptParaFix == 0) {
		    next_pp = NULL;
		    for (k = 0; k < j - i - 1; k++) {
			right_ptr = cky_matrix[i + k + 1][j];
			while (right_ptr) {
			    left_ptr = right_ptr;
			    while (left_ptr && (left_ptr->dpnd_type == 'P' || left_ptr->para_flag)) {
				left_ptr = left_ptr->left;
			    }
			    if (left_ptr && left_ptr != right_ptr) {
				left_ptr = cky_matrix[i][left_ptr->j]; /* ��(i��)�ˤ����� */
				while (left_ptr) {
				    if (left_ptr->dpnd_type == 'P') {
					if ((cky_ptr = new_cky_data(&cky_table_num)) == NULL) {
					    return FALSE;
					}
					if (next_pp == NULL) {
					    start_ptr = cky_ptr;
					}
					else {
					    *next_pp = cky_ptr;
					}

					set_cky(sp, cky_ptr, left_ptr, right_ptr, i, j, k, 'P', LtoR, -1);
					next_pp = &(cky_ptr->next);

					if (OptDisplay == OPT_DEBUG) {
					    printf("** (%d,%d), (%d,%d) b=%d [%s--%s], P(para=--), score=", 
						   i, left_ptr->j, i + k + 1, j, dep_check[i], 
						   (sp->bnst_data + i)->head_ptr->Goi, 
						   (sp->bnst_data + left_ptr->j)->head_ptr->Goi);
					}

					cky_ptr->para_flag = 1;
					cky_ptr->para_score = cky_ptr->left->para_score + cky_ptr->right->para_score;
					cky_ptr->score = OptAnalysis == OPT_CASE ? 
					    calc_case_probability(sp, cky_ptr, Best_mgr) : calc_score(sp, cky_ptr);
				    }
				    left_ptr = left_ptr->next;
				}
			    }
			    right_ptr = right_ptr->next;
			}
			/* if (next_pp) break; */
		    }

		    if (next_pp) {
			if (next_pp_for_ij == NULL) {
			    cky_matrix[i][j] = start_ptr;
			}
			else {
			    *next_pp_for_ij = start_ptr;
			}
			next_pp_for_ij = next_pp;
		    }
		}

		if (next_pp_for_ij) {
		    /* leave the best candidates within the beam */
		    if (OptBeam) {
			sort_cky_ptrs(&(cky_matrix[i][j]), OptBeam);
			next_pp = &(cky_matrix[i][j]);
			for (l = 0; l < OptBeam && *next_pp; l++) {
			    /* swap_cky_data(&(cky_table[pre_cky_table_num + l]), *next_pp); */
			    /* *next_pp = &(cky_table[pre_cky_table_num + l]); */
			    next_pp = &((*next_pp)->next);
			}
			/* cky_table_num = pre_cky_table_num + l; */
			*next_pp = NULL;
		    }
		    /* move the best one to the beginning of the list for the next step */
		    else {
			sort_cky_ptrs(&(cky_matrix[i][j]), 1);
		    }

		    if (Language == CHINESE && cky_matrix[i][j]->next && cky_matrix[i][j]->next->next) {
			/* only keep CHI_CKY_MAX probabilities for word pair */
			m = 1;
			sort_flag = 1;
			while (m < CHI_CKY_MAX && sort_flag) {
			    cky_ptr = cky_matrix[i][j];			    
			    sort_pre_ptr = NULL;
			    for (l = 0; l < m; l++) {
				if (cky_ptr->next) {
				    sort_pre_ptr = cky_ptr;
				    cky_ptr = cky_ptr->next;
				}
				else {
				    break;
				}
			    }
			    if (cky_ptr->next) {
				sort_flag = 1;
				best_score = -INT_MAX;
				pre_ptr = NULL;
				best_pre_ptr = NULL;
				while (cky_ptr) {
				    if (cky_ptr->score > best_score) {
					best_score = cky_ptr->score;
					best_ptr = cky_ptr;
					best_pre_ptr = pre_ptr;
				    }
				    pre_ptr = cky_ptr;
				    cky_ptr = cky_ptr->next;
				}
				if (best_pre_ptr) { /* best����Ƭ�ǤϤʤ���� */
				    best_pre_ptr->next = best_ptr->next;
				    best_ptr->next = sort_pre_ptr->next;
				    sort_pre_ptr->next = best_ptr;
				}
			    }
			    else {
				sort_flag = 0;
			    }
			    m++;
			}
			cky_ptr = cky_matrix[i][j];
			for (l = 0; l < m; l++) {
			    if (cky_ptr->next) {
				cky_ptr = cky_ptr->next;
			    }
			}
			cky_ptr->next = NULL;
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
    if (!cky_ptr) {
	return FALSE;
    }

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

	if (OptNbest == TRUE) {
	    cky_ptr = cky_matrix[0][sp->Bnst_num - 1]; /* when print all possible structures */
	}
	else {
	    cky_ptr = best_ptr;
	}
    }

    return after_cky(sp, Best_mgr, cky_ptr);
}

/* check if there exists special word in one region */
int exist_chi(SENTENCE_DATA *sp, int i, int j, char *type) {
    int k;

    if (!strcmp(type, "noun")) {
	for (k = i; k <= j; k++) {
	    if (check_feature((sp->bnst_data + k)->f, "PU") && 
		(!strcmp((sp->bnst_data+k)->head_ptr->Goi, ",") ||
		 !strcmp((sp->bnst_data+k)->head_ptr->Goi, "��") ||
		 !strcmp((sp->bnst_data+k)->head_ptr->Goi, ":") ||
		 !strcmp((sp->bnst_data+k)->head_ptr->Goi, "��") ||
		 !strcmp((sp->bnst_data+k)->head_ptr->Goi, "��"))) {
		break;
	    }
	    if (check_feature((sp->bnst_data + k)->f, "NN") ||
		check_feature((sp->bnst_data + k)->f, "NT") ||
		check_feature((sp->bnst_data + k)->f, "NR")){
		return k;
	    }
	}
    }
    else if (!strcmp(type, "DEC")) {
	for (k = i; k <= j; k++) {
	    if (check_feature((sp->bnst_data + k)->f, "DEC")) {
		return k;
	    }
	}
    }
    else if (!strcmp(type, "CC")) {
	for (k = i; k <= j; k++) {
	    if (check_feature((sp->bnst_data + k)->f, "CC")) {
		return k;
	    }
	}
    }
    else if (!strcmp(type, "pu")) {
	for (k = i; k <= j; k++) {
	    if (check_feature((sp->bnst_data + k)->f, "PU") && 
		(!strcmp((sp->bnst_data+k)->head_ptr->Goi, ",") ||
		 !strcmp((sp->bnst_data+k)->head_ptr->Goi, "��") ||
		 !strcmp((sp->bnst_data+k)->head_ptr->Goi, ":") ||
		 !strcmp((sp->bnst_data+k)->head_ptr->Goi, "��") ||
		 !strcmp((sp->bnst_data+k)->head_ptr->Goi, "��"))) {
		return k;
	    }
	}
    }
    else if (!strcmp(type, "verb")) {
	for (k = i; k <= j; k++) {
	    if (check_feature((sp->bnst_data + k)->f, "VV") ||
		check_feature((sp->bnst_data + k)->f, "VA")) {
		return k;
	    }
	}
    }
    else if (!strcmp(type, "prep")) {
	for (k = i; k <= j; k++) {
	    if (check_feature((sp->bnst_data + k)->f, "P")) {
		return k;
	    }
	}
    }
    
    return -1;
}

/* check the number of special pos-tag in a sentence */
int check_pos_num_chi(SENTENCE_DATA *sp, char *type) {
    int k;
    int num = 0;

    if (!strcmp(type, "verb")) {
	for (k = 0; k < sp->Bnst_num; k++) {
	    if (check_feature((sp->bnst_data + k)->f, "VV") ||
		check_feature((sp->bnst_data + k)->f, "VA") ||
		check_feature((sp->bnst_data + k)->f, "VC") ||
		check_feature((sp->bnst_data + k)->f, "VE")) {
		num++;
	    }
	}
    }
    else if (!strcmp(type, "DEC")) {
	for (k = 0; k < sp->Bnst_num; k++) {
	    if (check_feature((sp->bnst_data + k)->f, "DEC")) {
		num++;
	    }
	}
    }

    return num;
}

/* check if this node has special child, direction = 0 means check in the left side, direction = 1 means check in the right side */
int has_child_chi(SENTENCE_DATA *sp, CKY *cky_ptr, char *pos, int direction) {
    CKY *ptr = cky_ptr;
    if (ptr->direction == LtoR) {
	if (direction == 0) {
	    if (ptr->left && check_feature((sp->bnst_data + ptr->left->b_ptr->num)->f, pos)) {
		return 1;
	    }
	    if (ptr->right) {
		ptr = ptr->right;
		while (ptr) {
		    if (ptr->direction == LtoR) {
			if (ptr->left && check_feature((sp->bnst_data + ptr->left->b_ptr->num)->f, pos)) {
			    return 1;
			}
			else {
			    ptr = ptr->right;
			}
		    }
		    else {
			ptr = ptr->left;
		    }
		}
	    }
	}
	else {
	    if (ptr->right) {
		ptr = ptr->right;
		while (ptr) {
		    if (ptr->direction == RtoL) {
			if (ptr->right && check_feature((sp->bnst_data + ptr->right->b_ptr->num)->f, pos)) {
			    return 1;
			}
			else {
			    ptr = ptr->left;
			}
		    }
		    else {
			ptr = ptr->right;
		    }
		}
	    }
	}
    }
    else {
	if (direction == 1) {
	    if (ptr->right && check_feature((sp->bnst_data + ptr->right->b_ptr->num)->f, pos)) {
		return 1;
	    }
	    if (ptr->left) {
		ptr = ptr->left;
		while (ptr) {
		    if (ptr->direction == RtoL) {
			if (ptr->right && check_feature((sp->bnst_data + ptr->right->b_ptr->num)->f, pos)) {
			    return 1;
			}
			else {
			    ptr = ptr->left;
			}
		    }
		    else {
			ptr = ptr->right;
		    }
		}
	    }
	}
	else {
	    if (ptr->left) {
		ptr = ptr->left;
		while (ptr) {
		    if (ptr->direction == LtoR) {
			if (ptr->left && check_feature((sp->bnst_data + ptr->left->b_ptr->num)->f, pos)) {
			    return 1;
			}
			else {
			    ptr = ptr->right;
			}
		    }
		    else {
			ptr = ptr->left;
		    }
		}
	    }
	}
    }	
    return 0;
}

int check_chi_dpnd_possibility (int i, int j, int k, CKY *left, CKY *right, SENTENCE_DATA *sp, int direction) {
    int l;

    if (Language != CHINESE) {
	return 1;
    }
    else {
	if (Dpnd_matrix[left->b_ptr->num][right->b_ptr->num] > 0 && Dpnd_matrix[left->b_ptr->num][right->b_ptr->num] != 'O') {
	    if (direction != Dpnd_matrix[left->b_ptr->num][right->b_ptr->num]) {
		return 0;
	    }
	}

	/* check if the dependency follows root constraint (the dependency cannot go across root) */
	if (left->b_ptr->num < Chi_root && right->b_ptr->num > Chi_root) {
	    return 0;
	}

/* 	if ((left->b_ptr->num == Chi_root && direction == 'R') || */
/* 	    (right->b_ptr->num == Chi_root && direction == 'L')) { */
/* 	    return 0; */
/* 	} */

        /* check if this cky corresponds with the grammar rules for Chinese */
	/* sp and main verb */
	if (check_feature((sp->bnst_data + right->b_ptr->num)->f, "SP") && 
	    !check_feature((sp->bnst_data + left->b_ptr->num)->f, "VV") &&
	    !check_feature((sp->bnst_data + left->b_ptr->num)->f, "VC") &&
	    !check_feature((sp->bnst_data + left->b_ptr->num)->f, "VE") &&
	    !check_feature((sp->bnst_data + left->b_ptr->num)->f, "VA") &&
	    direction == 'L') {
	    return 0;
	}

	/* adj and verb cannot have dependency relation */
	if ((check_feature((sp->bnst_data + left->b_ptr->num)->f, "JJ") && 
	     (check_feature((sp->bnst_data + right->b_ptr->num)->f, "VV") ||
	      check_feature((sp->bnst_data + right->b_ptr->num)->f, "VA") ||
	      check_feature((sp->bnst_data + right->b_ptr->num)->f, "VC") ||
	      check_feature((sp->bnst_data + right->b_ptr->num)->f, "VE"))) ||
	    (check_feature((sp->bnst_data + right->b_ptr->num)->f, "JJ") && 
	     (check_feature((sp->bnst_data + left->b_ptr->num)->f, "VV") ||
	      check_feature((sp->bnst_data + left->b_ptr->num)->f, "VA") ||
	      check_feature((sp->bnst_data + left->b_ptr->num)->f, "VC") ||
	      check_feature((sp->bnst_data + left->b_ptr->num)->f, "VE")))) {
	    return 0;
	}

	/* the word before dunhao cannot have left dependency */
	if (direction == 'L' &&
	    check_feature((sp->bnst_data + right->b_ptr->num + 1)->f, "PU") &&
	    !check_feature((sp->bnst_data + right->b_ptr->num + 2)->f, "VV") &&
	    !check_feature((sp->bnst_data + right->b_ptr->num + 2)->f, "VC") &&
	    !check_feature((sp->bnst_data + right->b_ptr->num + 2)->f, "VE") &&
	    !check_feature((sp->bnst_data + right->b_ptr->num + 2)->f, "VA") &&
	    !strcmp((sp->bnst_data + right->b_ptr->num + 1)->head_ptr->Goi, "��")) {
	    return 0;
	}

	/* the noun before dunhao should depend on noun after it */
	if (direction == 'R' &&
	    (check_feature((sp->bnst_data + left->b_ptr->num)->f, "NN") ||
	     check_feature((sp->bnst_data + left->b_ptr->num)->f, "NR") ||
	     check_feature((sp->bnst_data + left->b_ptr->num)->f, "PN") ||
	     check_feature((sp->bnst_data + left->b_ptr->num)->f, "JJ") ||
	     check_feature((sp->bnst_data + left->b_ptr->num)->f, "NT") ||
	     check_feature((sp->bnst_data + left->b_ptr->num)->f, "M") ||
	     check_feature((sp->bnst_data + left->b_ptr->num)->f, "DEG")) &&
	    (!check_feature((sp->bnst_data + right->b_ptr->num)->f, "NN") &&
	     !check_feature((sp->bnst_data + right->b_ptr->num)->f, "NT") &&
	     !check_feature((sp->bnst_data + right->b_ptr->num)->f, "JJ") &&
	     !check_feature((sp->bnst_data + right->b_ptr->num)->f, "NR") &&
	     !check_feature((sp->bnst_data + right->b_ptr->num)->f, "PN") &&
	     !check_feature((sp->bnst_data + right->b_ptr->num)->f, "DEG") &&
	     !check_feature((sp->bnst_data + right->b_ptr->num)->f, "M")) &&
	    check_feature((sp->bnst_data + left->b_ptr->num + 1)->f, "PU") &&
	    !check_feature((sp->bnst_data + left->b_ptr->num + 2)->f, "VV") &&
	    !check_feature((sp->bnst_data + left->b_ptr->num + 2)->f, "VC") &&
	    !check_feature((sp->bnst_data + left->b_ptr->num + 2)->f, "VE") &&
	    !check_feature((sp->bnst_data + left->b_ptr->num + 2)->f, "VA") &&
	    !strcmp((sp->bnst_data + left->b_ptr->num + 1)->head_ptr->Goi, "��")) {
	    return 0;
	}

	/* only the quote PU can be head */
	if ((direction == 'R' && 
	     check_feature((sp->bnst_data + right->b_ptr->num)->f, "PU") &&
	     Chi_quote_end_matrix[right->b_ptr->num][right->b_ptr->num] != right->b_ptr->num) ||
	    (direction == 'L' && 
	     check_feature((sp->bnst_data + left->b_ptr->num)->f, "PU") &&
	     Chi_quote_start_matrix[left->b_ptr->num][left->b_ptr->num] != left->b_ptr->num)) {
	    return 0;
	}

	/* CC cannot depend on AD */
	if (check_feature((sp->bnst_data + left->b_ptr->num)->f, "CC") && 
	    direction == 'R' && 
	    check_feature((sp->bnst_data + right->b_ptr->num)->f, "AD")) {
	    return 0;
	}

	/* for DEG , DEV, DEC and LC, there should not be two modifiers */
	if ((check_feature((sp->bnst_data + right->b_ptr->num)->f, "DEG") ||
	     check_feature((sp->bnst_data + right->b_ptr->num)->f, "DEC") || 
	     check_feature((sp->bnst_data + right->b_ptr->num)->f, "DEV") || 
	     check_feature((sp->bnst_data + right->b_ptr->num)->f, "LC")) && 
	    right->b_ptr->num - right->i > 0 && 
	    direction == 'R') {
	    return 0;
	}

	/* for DEC, if there exists noun between it and previous verb, the noun should depend on verb */
	if (check_feature((sp->bnst_data + right->b_ptr->num)->f, "DEC") &&
	    (check_feature((sp->bnst_data + left->b_ptr->num)->f, "VV") || 
	     check_feature((sp->bnst_data + left->b_ptr->num)->f, "VA")) &&
	    exist_chi(sp, right->i, right->b_ptr->num - 1, "noun") != -1 &&
	    direction == 'R') {
	    return 0;
	}

	/* for DEG, its right head should be noun afterwords */
	if (check_feature((sp->bnst_data + left->b_ptr->num)->f, "DEG") &&
	    (!check_feature((sp->bnst_data + right->b_ptr->num)->f, "NN") &&
	     !check_feature((sp->bnst_data + right->b_ptr->num)->f, "NT") &&
	     !check_feature((sp->bnst_data + right->b_ptr->num)->f, "NR") &&
	     !check_feature((sp->bnst_data + right->b_ptr->num)->f, "PN") &&
	     !check_feature((sp->bnst_data + right->b_ptr->num)->f, "M")) &&
	    direction == 'R') {
	    return 0;
	}

	/* for DEG and DEC, it must have some word before modifying it */
	if (((check_feature((sp->bnst_data + left->b_ptr->num)->f, "DEG") ||
	      check_feature((sp->bnst_data + left->b_ptr->num)->f, "DEC")) &&
	     left->i == left->b_ptr->num &&
	     direction == 'R') ||
	    ((check_feature((sp->bnst_data + right->b_ptr->num)->f, "DEG") ||
	      check_feature((sp->bnst_data + right->b_ptr->num)->f, "DEC")) &&
	     right->i == right->b_ptr->num &&
	     direction == 'L')) {
	    return 0;
	}

	/* for DEC, it must have some verb before modifying it */
	if ((!check_feature((sp->bnst_data + left->b_ptr->num)->f, "VV") &&
	     !check_feature((sp->bnst_data + left->b_ptr->num)->f, "VA") &&
	     !check_feature((sp->bnst_data + left->b_ptr->num)->f, "VC") &&
	     !check_feature((sp->bnst_data + left->b_ptr->num)->f, "VE")) &&
	     check_feature((sp->bnst_data + right->b_ptr->num)->f, "DEC") &&
	    direction == 'R') {
	    return 0;
	}
	
	/* VC and VE must have modifier behind */
	if ((check_feature((sp->bnst_data + left->b_ptr->num)->f, "VC") ||
	     check_feature((sp->bnst_data + left->b_ptr->num)->f, "VE")) &&
	    ((direction == 'R' &&
	      left->j == left->b_ptr->num))) {
	    return 0;
	}

	/* VC and VE must have modifier before */
	if ((check_feature((sp->bnst_data + left->j)->f, "VC") &&
	     left->j != left->b_ptr->num) ||
	    (check_feature((sp->bnst_data + right->i)->f, "VC") &&
	     right->i != right->b_ptr->num)) {
	    return 0;
	}

	/* for verb, there should be only one object afterword */
	if ((check_feature((sp->bnst_data + left->b_ptr->num)->f, "VV") ||
	     check_feature((sp->bnst_data + left->b_ptr->num)->f, "VC") ||
	     check_feature((sp->bnst_data + left->b_ptr->num)->f, "VE") ||
	     check_feature((sp->bnst_data + left->b_ptr->num)->f, "P") ||
	     check_feature((sp->bnst_data + left->b_ptr->num)->f, "VA")) &&
	    (check_feature((sp->bnst_data + right->b_ptr->num)->f, "NN") ||
	     check_feature((sp->bnst_data + right->b_ptr->num)->f, "NR") ||
	     check_feature((sp->bnst_data + right->b_ptr->num)->f, "PN") ||
	     check_feature((sp->bnst_data + right->b_ptr->num)->f, "M") ||
	     check_feature((sp->bnst_data + right->b_ptr->num)->f, "DEG")) &&
	    direction == 'L' &&
	    left->j != left->i &&
	    (has_child_chi(sp, left, "NN", 1)||
	     has_child_chi(sp, left, "NR", 1)||
	     has_child_chi(sp, left, "M", 1)||
	     has_child_chi(sp, left, "DEG", 1)||
	     has_child_chi(sp, left, "PN", 1))) {
	    return 0;
	}

	/* if a verb has object, then between the verb and its object, there should not be another verb depend on the first verb */
	if ((check_feature((sp->bnst_data + left->b_ptr->num)->f, "VV") ||
	     check_feature((sp->bnst_data + left->b_ptr->num)->f, "VC") ||
	     check_feature((sp->bnst_data + left->b_ptr->num)->f, "VE") ||
	     check_feature((sp->bnst_data + left->b_ptr->num)->f, "VA")) &&
	    (check_feature((sp->bnst_data + right->b_ptr->num)->f, "NN") ||
	     check_feature((sp->bnst_data + right->b_ptr->num)->f, "PN") ||
	     check_feature((sp->bnst_data + right->b_ptr->num)->f, "NR")) &&
	    (has_child_chi(sp, left, "VV", 1) ||
	     has_child_chi(sp, left, "VA", 1) ||
	     has_child_chi(sp, left, "VC", 1) ||
	     has_child_chi(sp, left, "VE", 1))) {
	    return 0;
	}    

	/* for verb, there should be only one subject in front of it */
	if ((check_feature((sp->bnst_data + right->b_ptr->num)->f, "VV") ||
	     check_feature((sp->bnst_data + right->b_ptr->num)->f, "VC") ||
	     check_feature((sp->bnst_data + right->b_ptr->num)->f, "VE") ||
	     check_feature((sp->bnst_data + right->b_ptr->num)->f, "VA")) &&
	    (check_feature((sp->bnst_data + left->b_ptr->num)->f, "NN") ||
	     check_feature((sp->bnst_data + left->b_ptr->num)->f, "PN") ||
	     check_feature((sp->bnst_data + left->b_ptr->num)->f, "NR")) &&
	    direction == 'R' &&
	    right->j != right->i &&
	    (has_child_chi(sp, right, "NN", 0)||
	     has_child_chi(sp, right, "NR", 0)||
	     has_child_chi(sp, right, "PN", 0))) {
	    return 0;
	}

	/* for pivot sentence, the noun between the two verbs should depend on the second verb  */
	if (check_feature((sp->bnst_data + right->b_ptr->num)->f, "VV") &&
	    check_feature((sp->bnst_data + left->b_ptr->num)->f, "VV") &&
	    direction == 'L' && left->j != left->i &&
	    (has_child_chi(sp, left, "NN", 1)||
	     has_child_chi(sp, left, "NR", 1)||
	     has_child_chi(sp, left, "LC", 1)||
	     has_child_chi(sp, left, "PN", 1)) &&
	    exist_chi(sp, left->b_ptr->num + 1, right->b_ptr->num - 1, "CC") == -1 &&
	    exist_chi(sp, left->b_ptr->num + 1, right->b_ptr->num - 1, "pu") == -1) {
	    return 0;
	}

	/* for preposition, it must have modifier */
	if ((check_feature((sp->bnst_data + left->b_ptr->num)->f, "P") &&
	     direction == 'R' &&
	     left->j - left->i == 0) ||
	    (check_feature((sp->bnst_data + right->b_ptr->num)->f, "P") &&
	     direction == 'L' &&
	     right->j - right->i == 0)) {
	    return 0;
	}

	/* for preposition, it cannot depend on a preposition */
	if (check_feature((sp->bnst_data + right->b_ptr->num)->f, "P") &&
	    check_feature((sp->bnst_data + left->b_ptr->num)->f, "P")) {
	    return 0;
	}

	/* for preposition, it cannot depend on a noun before */
	if (check_feature((sp->bnst_data + right->b_ptr->num)->f, "P") &&
	    (check_feature((sp->bnst_data + left->b_ptr->num)->f, "NN") ||
	     check_feature((sp->bnst_data + left->b_ptr->num)->f, "NR") ||
	     check_feature((sp->bnst_data + left->b_ptr->num)->f, "PN"))) {
	    return 0;
	}

	/* for preposition, it cannot depend on a CD */
	if (check_feature((sp->bnst_data + left->b_ptr->num)->f, "P") &&
	    check_feature((sp->bnst_data + right->b_ptr->num)->f, "CD") &&
	    direction == 'R') {
	    return 0;
	}

	/* a noun cannot depend on its following preposition */
	if (check_feature((sp->bnst_data + right->b_ptr->num)->f, "P") &&
	    (check_feature((sp->bnst_data + left->b_ptr->num)->f, "NN") || 
	     check_feature((sp->bnst_data + left->b_ptr->num)->f, "NR") || 
	     check_feature((sp->bnst_data + left->b_ptr->num)->f, "PN")) && 
	    direction == 'R') {
	    return 0;
	}

	/* for preposition, if it depend on verb, it should have modifier */
	if ((check_feature((sp->bnst_data + left->b_ptr->num)->f, "P") &&
	    (check_feature((sp->bnst_data + right->b_ptr->num)->f, "VA") || 
	     check_feature((sp->bnst_data + right->b_ptr->num)->f, "VC") || 
	     check_feature((sp->bnst_data + right->b_ptr->num)->f, "VE") || 
	     check_feature((sp->bnst_data + right->b_ptr->num)->f, "VV")) &&
	    direction == 'R' &&
	    left->j == left->b_ptr->num) ||
	    (check_feature((sp->bnst_data + right->b_ptr->num)->f, "P") &&
	     (check_feature((sp->bnst_data + left->b_ptr->num)->f, "VA") || 
	      check_feature((sp->bnst_data + left->b_ptr->num)->f, "VC") || 
	      check_feature((sp->bnst_data + left->b_ptr->num)->f, "VE") || 
	      check_feature((sp->bnst_data + left->b_ptr->num)->f, "VV")) &&
	     direction == 'L' &&
	     right->j == right->b_ptr->num)) {
	    return 0;
	}

	/* for preposition, if it depend on verb before, the verb should have object */
	if (check_feature((sp->bnst_data + right->b_ptr->num)->f, "P") &&
	    (check_feature((sp->bnst_data + left->b_ptr->num)->f, "VC") || 
	     check_feature((sp->bnst_data + left->b_ptr->num)->f, "VE") || 
	     check_feature((sp->bnst_data + left->b_ptr->num)->f, "VV")) &&
	    direction == 'L' &&
	    right->j == sp->Bnst_num - 1) {
	    return 0;
	}

	/* for preposition, if there is LC in the following (no preposibion between them), the words between P and LC should depend on LC */
	if (check_feature((sp->bnst_data + left->b_ptr->num)->f, "P") &&
	    check_feature((sp->bnst_data + right->b_ptr->num)->f, "LC") &&
	    left->j - left->i > 0 &&
	    exist_chi(sp, left->b_ptr->num + 1, right->b_ptr->num - 1, "prep") == -1) {
	    return 0;
	}

	/* for preposition, if there is noun between it and following verb, if preposition is head of the verb, all the noun should depend on verb, if verb is head of preposition, all the noun should depend on preposition */
	if (check_feature((sp->bnst_data + left->b_ptr->num)->f, "P") &&
	    (check_feature((sp->bnst_data + right->b_ptr->num)->f, "VV") ||
	     check_feature((sp->bnst_data + right->b_ptr->num)->f, "VA") || 
	     check_feature((sp->bnst_data + right->b_ptr->num)->f, "VC") ||
	     check_feature((sp->bnst_data + right->b_ptr->num)->f, "VE")) &&
/* 	    ((direction == 'R' && /\* verb is head *\/ */
/* 	      right->j != right->i && */
/* 	      (has_child_chi(sp, right, "NN", 0)|| */
/* 	       has_child_chi(sp, right, "NR", 0)|| */
/* 	       has_child_chi(sp, right, "PN", 0))) || */
	     (direction == 'L' && /* preposition is head */
	      left->j != left->i &&
	      (has_child_chi(sp, left, "NN", 1)||
	       has_child_chi(sp, left, "NR", 1)||
	       has_child_chi(sp, left, "PN", 1)))) {
	    return 0;
	}

	/* for preposition, if it has a VV modifier after it, this VV should have object or subject */
	if (check_feature((sp->bnst_data + left->b_ptr->num)->f, "P") &&
	    check_feature((sp->bnst_data + right->b_ptr->num)->f, "VV") &&
	    direction == 'L' &&
	    (!has_child_chi(sp, right, "NN", 0) &&
	     !has_child_chi(sp, right, "NR", 0) &&
	     !has_child_chi(sp, right, "PN", 0)) &&
	    (!has_child_chi(sp, right, "NN", 1) &&
	     !has_child_chi(sp, right, "NR", 1) &&
	     !has_child_chi(sp, right, "PN", 1))) {
	    return 0;
	}

	/* check if this cky corresponds with the constraint of NP and quote */
	if ((Chi_np_end_matrix[i][i + k] != -1 && j > Chi_np_end_matrix[i][i + k]) ||
	    (Chi_np_start_matrix[i + k + 1][j] != -1 && i < Chi_np_start_matrix[i + k + 1][j])){
	    return 0;
	}
	if ((Chi_quote_end_matrix[i][i + k] != -1 && j > Chi_quote_end_matrix[i][i + k]) ||
	    (Chi_quote_start_matrix[i + k + 1][j] != -1 && i < Chi_quote_start_matrix[i + k + 1][j])){
	    return 0;
	}

	return 1;
    }
}
