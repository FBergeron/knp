/*====================================================================

			     ��¸��¤����

                                               S.Kurohashi 93. 5.31

    $Id$
====================================================================*/
#include "knp.h"

extern char CorpusComment[BNST_MAX][DATA_LEN];
int Possibility;	/* ��¸��¤�β�ǽ���β����ܤ� */
static int dpndID = 0;

/*==================================================================*/
	       void assign_dpnd_rule(SENTENCE_DATA *sp)
/*==================================================================*/
{
    int 	i, j;
    BNST_DATA	*b_ptr;
    DpndRule 	*r_ptr;

    for (i = 0, b_ptr = sp->bnst_data; i < sp->Bnst_num; i++, b_ptr++) {
	for (j = 0, r_ptr = DpndRuleArray; j < CurDpndRuleSize; j++, r_ptr++) {

	    if (feature_pattern_match(&(r_ptr->dependant), b_ptr->f, NULL, NULL) 
		== TRUE) {
		b_ptr->dpnd_rule = r_ptr; 
		break;
	    }
	}

	if (b_ptr->dpnd_rule == NULL) {
	    fprintf(stderr, "No DpndRule for %dth bnst (", i);
	    print_feature(b_ptr->f, stderr);
	    fprintf(stderr, ")\n");

	    /* DpndRuleArray[0] �ϥޥå����ʤ����� */
	    b_ptr->dpnd_rule = DpndRuleArray;
	}
    }
}

/*==================================================================*/
	       void calc_dpnd_matrix(SENTENCE_DATA *sp)
/*==================================================================*/
{
    int i, j, k, value, first_uke_flag;
    BNST_DATA *k_ptr, *u_ptr;

    for (i = 0; i < sp->Bnst_num; i++) {
	k_ptr = sp->bnst_data + i;
	first_uke_flag = 1;
	for (j = i + 1; j < sp->Bnst_num; j++) {
	    u_ptr = sp->bnst_data + j;
	    Dpnd_matrix[i][j] = 0;
	    for (k = 0; k_ptr->dpnd_rule->dpnd_type[k]; k++) {
		value = feature_pattern_match(&(k_ptr->dpnd_rule->governor[k]),
					      u_ptr->f, k_ptr, u_ptr);
		if (value == TRUE) {
		    Dpnd_matrix[i][j] = (int)k_ptr->dpnd_rule->dpnd_type[k];
		    first_uke_flag = 0;
		    break;
		}
	    }
	}
    }
}

/*==================================================================*/
	       int relax_dpnd_matrix(SENTENCE_DATA *sp)
/*==================================================================*/
{
    /* �����褬�ʤ����δ���

       ��̤ˤ��ޥ�����ͥ�褷������������������˷����褦���ѹ�

       �� ���šš֡šššššסšţ� (ʸ��)
       �� �ššš֣��ššţ¡סšš� (��̽�)
       �� �ššš֣��ţ¡��šסšš� (��:ʸ��)
       �� ���šššţ¡֡šššţá� (�¤˷�������ȤϤ��ʤ���
                                      �äȤδط��ϲ��Ϥ��н�)
    */

    int i, j, ok_flag, relax_flag, last_possibility;
  
    relax_flag = FALSE;

    for (i = 0; i < sp->Bnst_num - 1  ; i++) {
	ok_flag = FALSE;
	last_possibility = i;
	for (j = i + 1; j < sp->Bnst_num ; j++) {
	    if (Quote_matrix[i][j]) {
		if (Dpnd_matrix[i][j] > 0) {
		    ok_flag = TRUE;
		    break;
		} else if (check_feature(sp->bnst_data[j].f, "��:ʸ��")) {
		    last_possibility = j;
		    break;
		} else {
		    last_possibility = j;
		}
	    }
	}

	if (ok_flag == FALSE) {
	    if (check_feature(sp->bnst_data[last_possibility].f, "ʸ��") ||
		check_feature(sp->bnst_data[last_possibility].f, "��:ʸ��") ||
		check_feature(sp->bnst_data[last_possibility].f, "��̽�")) {
		Dpnd_matrix[i][last_possibility] = 'R';
		relax_flag = TRUE;
	    }
	}
    }

    return relax_flag;
}

/*==================================================================*/
int check_uncertain_d_condition(SENTENCE_DATA *sp, DPND *dp, int gvnr)
/*==================================================================*/
{
    /* ������(���å�)�� d �η��������������

       �� ���β�ǽ�ʷ�����(D)�����İʾ��� ( d - - D �ʤ� )
       �� d������������
       �� ���긵��d�θ��Ʊ����	��) ���ܤǺǽ�˵��ԤǹԤ�줿
       �� d(������)��d�θ��Ʊ����	��) ����Ƿײ���˵��Ԥ��ѹ����줿
    */

    int i, next_D;
    char *dpnd_cp, *gvnr_cp, *next_cp;

    next_D = 0;
    for (i = gvnr + 1; i < sp->Bnst_num ; i++) {
	if (Mask_matrix[dp->pos][i] &&
	    Quote_matrix[dp->pos][i] &&
	    dp->mask[i] &&
	    Dpnd_matrix[dp->pos][i] == 'D') {
	    next_D = i;
	    break;
	}
    }
    dpnd_cp = (char *)check_feature(sp->bnst_data[dp->pos].f, "��");
    gvnr_cp = (char *)check_feature(sp->bnst_data[gvnr].f, "��");
    if (gvnr < sp->Bnst_num-1) {
	next_cp = (char *)check_feature(sp->bnst_data[gvnr+1].f, "��");	
    }
    else {
	next_cp = NULL;
    }

    if (next_D == 0 ||
	gvnr + 2 < next_D ||
	check_feature(sp->bnst_data[gvnr].f, "����") ||
	(gvnr + 2 == next_D && gvnr < sp->Bnst_num-1 &&
	 check_feature(sp->bnst_data[gvnr+1].f, "�θ�") &&
	 ((dpnd_cp && next_cp && !strcmp(dpnd_cp, next_cp)) ||
	  (gvnr_cp && next_cp && !strcmp(gvnr_cp, next_cp))))) {
	/* fprintf(stderr, "%d -> %d OK\n", i, j); */
	return 1;
    } else {
	return 0;
    }
}

/*==================================================================*/
 int compare_dpnd(SENTENCE_DATA *sp, TOTAL_MGR *new, TOTAL_MGR *best)
/*==================================================================*/
{
    int i;

    return FALSE;

    if (Possibility == 1 || new->dflt < best->dflt) {
	return TRUE;
    } else {
	for (i = sp->Bnst_num - 2; i >= 0; i--) {
	    if (new->dpnd.dflt[i] < best->dpnd.dflt[i]) 
		return TRUE;
	    else if (new->dpnd.dflt[i] > best->dpnd.dflt[i]) 
		return FALSE;
	}
    }

    fprintf(stderr, "Error in compare_dpnd !!\n");
    exit(1);
}

/*==================================================================*/
	 void dpnd_info_to_bnst(SENTENCE_DATA *sp, DPND *dp)
/*==================================================================*/
{
    /* ��������˴ؤ����ξ���� DPND ���� BNST_DATA �˥��ԡ� */

    int		i;
    BNST_DATA	*b_ptr;

    for (i = 0, b_ptr = sp->bnst_data; i < sp->Bnst_num; i++, b_ptr++) {

	if (i == sp->Bnst_num - 1){		/* �Ǹ��ʸ�� */
	    b_ptr->dpnd_head = -1;
	    b_ptr->dpnd_type = 'D';
	} else if (dp->type[i] == 'd' || dp->type[i] == 'R') {
	    b_ptr->dpnd_head = dp->head[i];
	    b_ptr->dpnd_type = 'D';	/* relax��������D�� */
	} else {
	    b_ptr->dpnd_head = dp->head[i];
	    b_ptr->dpnd_type = dp->type[i];
	}
	b_ptr->dpnd_dflt = dp->dflt[i];

	strcpy(b_ptr->dpnd_int, "0");	/* ʸ�᷸���������ΰ��֡�����̤���� */
	strcpy(b_ptr->dpnd_ext, "");	/* ¾��ʸ�᷸�����ǽ��������̤���� */
    }
}

/*==================================================================*/
	       void para_postprocess(SENTENCE_DATA *sp)
/*==================================================================*/
{
    int i;

    for (i = 0; i < sp->Bnst_num; i++) {
	if (check_feature((sp->bnst_data + i)->f, "�Ѹ�") &&
	    (sp->bnst_data + i)->para_num != -1 &&
	    sp->para_data[(sp->bnst_data + i)->para_num].status != 'x') {
	    
	    assign_cfeature(&((sp->bnst_data + i)->f), "�����:30");
	}
    }
}

/*==================================================================*/
	  void dpnd_evaluation(SENTENCE_DATA *sp, DPND dpnd)
/*==================================================================*/
{
    int i, j, k, one_score, score, rentai, vacant_slot_num;
    int topic_score, optional_flag = 0;
    int optional_score = 0, total_optional_score = 0;
    int scase_check[SCASE_CODE_SIZE], ha_check, un_count, pred_p;
    char *cp, *cp2, *buffer;
    BNST_DATA *g_ptr, *d_ptr;

    /* ��¸��¤������ɾ��������δؿ�
       (��ʸ��ˤĤ��ơ������˷��äƤ���ʸ���ɾ������׻�)

       ɾ�����
       ========
       0. �������default���֤Ȥκ���ڥʥ�ƥ���(kakari_uke.rule)

       1. �֡��ϡ�(����,��:̤��)�η������ͥ�褵����Τ�����
       		(bnst_etc.rule�ǻ��ꡤ����Υ�����������ϸ�ץ����ǻ���)

       2. �֡��ϡפϰ�Ҹ�˰�ķ��뤳�Ȥ�ͥ��(����,���̤���)

       3. ���٤Ƥγ����Ǥ�Ʊ��ɽ�سʤ���Ҹ�˰�ķ��뤳�Ȥ�ͥ��(��������)

       4. ̤�ʡ�Ϣ�ν�����ϥ�,��,�˳ʤ�;�äƤ��륹��åȿ�����������Ϳ
    */

    /* �����ѥ��λ���ǡ����ν���� */
    if (!(OptInhibit & OPT_INHIBIT_OPTIONAL_CASE)) {
	for (i = 1; i < sp->Bnst_num; i++) {
	    dpnd.op[i].data = NULL;
	    dpnd.op[i].candidatesdata = NULL;
	}
    }

    score = 0;
    for (i = 1; i < sp->Bnst_num; i++) {
	g_ptr = sp->bnst_data + i;

	one_score = 0;
	for (k = 0; k < SCASE_CODE_SIZE; k++) scase_check[k] = 0;
	ha_check = 0;
	un_count = 0;

	if (check_feature(g_ptr->f, "�Ѹ�") ||
	    check_feature(g_ptr->f, "���Ѹ�")) {
	    pred_p = 1;
	} else {
	    pred_p = 0;
	}

	for (j = i-1; j >= 0; j--) {
	    d_ptr = sp->bnst_data + j;

	    if (dpnd.head[j] == i) {

		/* �������DEFAULT�ΰ��֤Ȥκ���ڥʥ�ƥ���
		     �� �����C,B'����Ʊ󤯤˷��뤳�Ȥ����뤬�����줬
		        ¾�η�����˱ƶ����ʤ��褦,�ڥʥ�ƥ��˺���Ĥ��� */

		if (check_feature(d_ptr->f, "����")) {
		    one_score -= dpnd.dflt[j];
		} else {
		    one_score -= dpnd.dflt[j] * 2;
		}
	    
		/* �������Ĥ�Τ��٤ˤ����뤳�Ȥ��ɤ� */

		if (j + 1 == i && check_feature(d_ptr->f, "����")) {
		    one_score -= 5;
		}

		/* ��������������ѥ����¸�ߤ��뤫�ɤ��� */
		if ((cp = (char *)check_feature(d_ptr->f, "��")) != NULL) {
		    if (!(OptInhibit & OPT_INHIBIT_OPTIONAL_CASE)) {
			/* ͥ�赬§ */
			optional_score = CorpusExampleDependencyCalculation(sp, d_ptr, cp+3, i, &(dpnd.check[j]), &dpnd.op[j]);
			/* optional_score = corpus_optional_case_comp(d_ptr, cp+3, g_ptr, &dpnd.op[j]); */

			/* one_score += optional_score*10; */
			/* ��Υ�Ť� */ /* j �� i �˷��äƤ��� */
			/* optional_score += corpus_optional_case_comp(d_ptr, cp+3, g_ptr)*10*(sp->Bnst_num-1-i)/(sp->Bnst_num-1-j); */
			if (optional_score > 0) {
			    dpnd.op[j].flag = TRUE;
			    dpnd.op[j].weight = optional_score;
			    dpnd.op[j].type = cp+3;
			    if (dpnd.comment) {
				buffer = dpnd.comment;
				dpnd.comment = (char *)malloc(strlen(buffer)+strlen(CorpusComment[j])+2);
				strcpy(dpnd.comment, buffer);
				strcat(dpnd.comment, " ");
				strcat(dpnd.comment, CorpusComment[j]);
			    }
			    else {
				dpnd.comment = strdup(CorpusComment[j]);
			    }
			    optional_flag = 1;
			    total_optional_score += optional_score;
			    /* total_optional_score += optional_score*5; */
			}
		    }
		}

		if (pred_p &&
		    (cp = (char *)check_feature(d_ptr->f, "��")) != NULL) {
		    
		    /* ̤�� ����(�֡��ϡ�)�ΰ��� */

		    if (check_feature(d_ptr->f, "����") &&
			!strcmp(cp, "��:̤��")) {

			/* ʸ��, �֡����פʤ�, ������, C, B'�˷��뤳�Ȥ�ͥ�� */

			if ((cp2 = (char *)check_feature(g_ptr->f, "�����")) 
			    != NULL) {
			    sscanf(cp2, "%*[^:]:%d", &topic_score);
			    one_score += topic_score;
			}
			/* else {one_score -= 15;} */

			/* ��Ĥ������ˤ�������Ϳ���� (����,���̤���)
			     �� ʣ�������꤬Ʊ��Ҹ�˷��뤳�Ȥ��ɤ� */

			if (check_feature(d_ptr->f, "����") ||
			    check_feature(d_ptr->f, "����")) {
			    one_score += 10;
			} else if (ha_check == 0){
			    one_score += 10;
			    ha_check = 1;
			}
		    }

		    k = case2num(cp+3);

		    /* �����ǰ��̤ΰ��� */

		    /* ̤�� : �����Ƥ�������Ƕ�����åȤ�Ĵ�٤� (����,���̤���) */

		    if (!strcmp(cp, "��:̤��")) {
			if (check_feature(d_ptr->f, "����") ||
			    check_feature(d_ptr->f, "����")) {
			    one_score += 10;
			} else {
			    un_count++;
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
			    one_score += 10;
			    break;
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
	    }
	}

	/* �Ѹ��ξ�硤�ǽ�Ū��̤��,����,���,�˳�,Ϣ�ν������Ф���
	   ����,���,�˳ʤΥ���å�ʬ����������Ϳ���� */

	if (pred_p) {

	    /* Ϣ�ν����ξ�硤���褬
	       ������̾��,����Ū̾��
	       ����ͽ���,�ָ����ߡפʤ�
	       �Ǥʤ���а�Ĥγ����Ǥȹͤ��� */

	    if (check_feature(g_ptr->f, "��:Ϣ��")) {
		if (check_feature(sp->bnst_data[dpnd.head[i]].f, "���δط�")) {
		    rentai = 0;
		    one_score += 10;	/* ���δط��ʤ餳���ǲ��� */
		} else {
		    rentai = 1;	/* ����ʳ��ʤ��Ƕ�������åȤ�����å� */
		}
	    } else {
		rentai = 0;
	    }

	    /* �����Ƥ��륬��,���,�˳�,���� */

	    vacant_slot_num = 0;
	    if ((g_ptr->SCASE_code[case2num("����")]
		 - scase_check[case2num("����")]) == 1) {
		vacant_slot_num ++;
	    }
	    if ((g_ptr->SCASE_code[case2num("���")]
		 - scase_check[case2num("���")]) == 1) {
		vacant_slot_num ++;
	    }
	    if ((g_ptr->SCASE_code[case2num("�˳�")]
		 - scase_check[case2num("�˳�")]) == 1 &&
		rentai == 1 &&
		check_feature(g_ptr->f, "�Ѹ�:ư")) {
		vacant_slot_num ++;
		/* �˳ʤ�ư���Ϣ�ν����ξ�������θ���Ĥޤ�Ϣ��
		   �����˳�����Ƥ�����ǡ�̤�ʤΥ���åȤȤϤ��ʤ� */
	    }
	    if ((g_ptr->SCASE_code[case2num("����")]
		 - scase_check[case2num("����")]) == 1) {
		vacant_slot_num ++;
	    }

	    /* ��������å�ʬ����Ϣ�ν�����̤�ʤ˥�������Ϳ���� */

	    if ((rentai + un_count) <= vacant_slot_num) 
		one_score += (rentai + un_count) * 10;
	    else 
		one_score += vacant_slot_num * 10;
	}

	score += one_score;

	if (OptDisplay == OPT_DEBUG) {
	    if (i == 1) 
		fprintf(Outfp, "Score:    ");
	    if (pred_p) {
		fprintf(Outfp, "%2d*", one_score);
	    } else {
		fprintf(Outfp, "%2d ", one_score);
	    }
	}
    }

    if (OptDisplay == OPT_DEBUG) {
	fprintf(Outfp, "=%d", score);
	if (optional_flag)
	    fprintf(Outfp, "+%d=%d", total_optional_score, score+total_optional_score);
	fprintf(Outfp, "\n");
    }

    if (OptDisplay == OPT_DEBUG) {
	dpnd_info_to_bnst(sp, &dpnd);
	make_dpnd_tree(sp);
	print_kakari(sp);
    }

    if (score > sp->Best_mgr->score) {
	sp->Best_mgr->dpnd = dpnd;
	sp->Best_mgr->score = score;
	sp->Best_mgr->ID = dpndID;
	Possibility++;
    }

    /* ��������Ȥä��Ȥ� */
    if (optional_flag) {
	if (!OptLearn) {
	    score += total_optional_score;
	    if (score > Op_Best_mgr.score) {
		Op_Best_mgr.dpnd = dpnd;
		Op_Best_mgr.score = score;
		Op_Best_mgr.ID = dpndID;
	    }
	}
	else {
	    fprintf(Outfp, ";;;OK ���� %d %s %d\n", dpndID, sp->KNPSID ? sp->KNPSID : "", score);
	    for (i = 0;i < sp->Bnst_num; i++) {
		if (dpnd.op[i].flag) {
		    fprintf(Outfp, ";;;OK * %d %d %d %s\n", i, dpnd.head[i], dpnd.op[i].weight, dpnd.op[i].type);
		}
	    }
	}
    }
    dpndID++;
}

/*==================================================================*/
	    void decide_dpnd(SENTENCE_DATA *sp, DPND dpnd)
/*==================================================================*/
{
    int i, count, possibilities[BNST_MAX], default_pos, d_possibility;
    int MaskFlag = 0;
    char *cp;
    BNST_DATA *b_ptr;
    
    if (OptDisplay == OPT_DEBUG) {
	if (dpnd.pos == sp->Bnst_num - 1) {
	    fprintf(Outfp, "------");
	    for (i = 0; i < sp->Bnst_num; i++)
	      fprintf(Outfp, "-%02d", i);
	    fputc('\n', Outfp);
	}
	fprintf(Outfp, "In %2d:", dpnd.pos);
	for (i = 0; i < sp->Bnst_num; i++)
	    fprintf(Outfp, " %2d", dpnd.head[i]);
	fputc('\n', Outfp);
    }

    dpnd.pos --;

    /* ʸƬ�ޤǲ��Ϥ�����ä���ɾ���ؿ����� */

    if (dpnd.pos == -1) {
	/* ����ʸ��η�������˽������ */
	for (i = 0; i < sp->Bnst_num -1; i++)
	    if (dpnd.head[i] < 0) {
		dpnd.head[i] = dpnd.head[i+dpnd.head[i]];
		dpnd.check[i].pos[0] = dpnd.head[i];
	    }

	if (OptAnalysis == OPT_DPND ||
	    OptAnalysis == OPT_CASE2) {
	    dpnd_evaluation(sp, dpnd);
	} 
	else if (OptAnalysis == OPT_CASE) {
	    call_case_analysis(sp, dpnd);
	}
	return;
    }

    b_ptr = sp->bnst_data + dpnd.pos;
    dpnd.f[dpnd.pos] = b_ptr->f;

    /* (���η���ˤ��)��򺹾������� (dpnd.mask �� 0 �ʤ鷸��ʤ�) */

    if (dpnd.pos < sp->Bnst_num -2)
	for (i = dpnd.pos + 2; i < dpnd.head[dpnd.pos+1]; i++)
	    dpnd.mask[i] = 0;
    
    /* ����¤�Υ���ʸ��, ��ʬ�����ʸ��<I>
       (���Ǥ˹Ԥ�줿����¤���Ϥη�̤�ޡ����������) */

    for (i = dpnd.pos + 1; i < sp->Bnst_num; i++) {
	if (Mask_matrix[dpnd.pos][i] == 2) {
	    dpnd.head[dpnd.pos] = i;
	    dpnd.type[dpnd.pos] = 'P';
	    /* �����å��� */
	    /* ����ξ��ϰ�դ˷�ޤäƤ���Τǡ������󤲤�Τϰ�̣���ʤ� */
	    dpnd.check[dpnd.pos].num = 1;
	    dpnd.check[dpnd.pos].pos[0] = i;
	    decide_dpnd(sp, dpnd);
	    return;
	} else if (Mask_matrix[dpnd.pos][i] == 3) {
	    dpnd.head[dpnd.pos] = i;
	    dpnd.type[dpnd.pos] = 'I';

	    dpnd.check[dpnd.pos].num = 1;
	    dpnd.check[dpnd.pos].pos[0] = i;
	    decide_dpnd(sp, dpnd);
	    return;
	}
    }

    /* ����ʸ��η�������˽������  ��) �֡������Τϰ��������� */

    if ((cp = (char *)check_feature(b_ptr->f, "��:̵�ʽ�°")) != NULL) {
        sscanf(cp, "%*[^:]:%*[^:]:%d", &(dpnd.head[dpnd.pos]));
        dpnd.type[dpnd.pos] = 'D';
        dpnd.dflt[dpnd.pos] = 0;
	dpnd.check[dpnd.pos].num = 1;
        decide_dpnd(sp, dpnd);
        return;
    }

    /* �̾�η���������� */

    /* ������θ����Ĵ�٤� */
    
    count = 0;
    d_possibility = 1;
    for (i = dpnd.pos + 1; i < sp->Bnst_num; i++) {
	if (Mask_matrix[dpnd.pos][i] &&
	    Quote_matrix[dpnd.pos][i] &&
	    dpnd.mask[i]) {

	    if (d_possibility && Dpnd_matrix[dpnd.pos][i] == 'd') {
		if (check_uncertain_d_condition(sp, &dpnd, i)) {
		    possibilities[count] = i;
		    count++;
		}
		d_possibility = 0;
	    }
	    else if (Dpnd_matrix[dpnd.pos][i] && 
		     Dpnd_matrix[dpnd.pos][i] != 'd') {
		possibilities[count] = i;
		count++;
		d_possibility = 0;
	    }

	    /* �Хꥢ�Υ����å� */
	    if (count &&
		b_ptr->dpnd_rule->barrier.fp[0] &&
		feature_pattern_match(&(b_ptr->dpnd_rule->barrier), 
				      sp->bnst_data[i].f,
				      b_ptr, sp->bnst_data + i) == TRUE)
		break;
	}
	else {
	    MaskFlag = 1;
	}
    }

    /* �ºݤ˸����Ĥ��äƤ���(���δؿ��κƵ�Ū�ƤӽФ�) */

    if (count) {

	/* preference �ϰ��ֶ᤯:1, ������:2, �Ǹ�:-1
	   default_pos �ϰ��ֶ᤯:1, ������:2, �Ǹ�:count ���ѹ� */

	default_pos = (b_ptr->dpnd_rule->preference == -1) ?
	    count: b_ptr->dpnd_rule->preference;

	dpnd.check[dpnd.pos].num = count;	/* ����� */
	dpnd.check[dpnd.pos].def = default_pos;	/* �ǥե���Ȥΰ��� */
	for (i = 0; i < count; i++) {
	    dpnd.check[dpnd.pos].pos[i] = possibilities[i];
	}

	/* ��դ˷��ꤹ���� */

	if (b_ptr->dpnd_rule->barrier.fp[0] == NULL || 
	    b_ptr->dpnd_rule->decide) {
	    if (default_pos <= count) {
		dpnd.head[dpnd.pos] = possibilities[default_pos - 1];
	    } else {
		dpnd.head[dpnd.pos] = possibilities[count - 1];
		/* default_pos �� 2 �ʤΤˡ�count�� 1 �����ʤ���� */
	    }
	    dpnd.type[dpnd.pos] = Dpnd_matrix[dpnd.pos][dpnd.head[dpnd.pos]];
	    dpnd.dflt[dpnd.pos] = 0;
	    decide_dpnd(sp, dpnd);
	} 

	/* ���٤Ƥβ�ǽ����Ĥ���Ф���� */
	/* ��֤η�������ξ��ϰ�դ˷���٤� */

	else {
	    for (i = 0; i < count; i++) {
		dpnd.head[dpnd.pos] = possibilities[i];
		dpnd.type[dpnd.pos] = Dpnd_matrix[dpnd.pos][dpnd.head[dpnd.pos]];
		dpnd.dflt[dpnd.pos] = abs(default_pos - 1 - i);
		decide_dpnd(sp, dpnd);
	    }
	}
    } 

    /* �����褬�ʤ����
       ʸ��������˥ޥ�������Ƥ��ʤ���С�ʸ���˷���Ȥ��� */

    else {
	if (Mask_matrix[dpnd.pos][sp->Bnst_num - 1]) {
	    dpnd.head[dpnd.pos] = sp->Bnst_num - 1;
	    dpnd.type[dpnd.pos] = 'D';
	    dpnd.dflt[dpnd.pos] = 10;
	    dpnd.check[dpnd.pos].num = 1;
	    dpnd.check[dpnd.pos].pos[0] = sp->Bnst_num - 1;
	    decide_dpnd(sp, dpnd);
	}
    }
}

/*==================================================================*/
	     void when_no_dpnd_struct(SENTENCE_DATA *sp)
/*==================================================================*/
{
    int i;

    sp->Best_mgr->dpnd.head[sp->Bnst_num - 1] = -1;

    for (i = sp->Bnst_num - 2; i >= 0; i--) {
	sp->Best_mgr->dpnd.head[i] = i + 1;
	sp->Best_mgr->dpnd.type[i] = 'D';
	sp->Best_mgr->dpnd.check[i].num = 1;
	sp->Best_mgr->dpnd.check[i].pos[0] = i + 1;
    }

    sp->Best_mgr->score = 0;
}

/*==================================================================*/
	       int after_decide_dpnd(SENTENCE_DATA *sp)
/*==================================================================*/
{
    int i, j, lastflag = -1;
    BNST_DATA *check_b_ptr;
    

    if (OptInput == OPT_PARSED) {
	Possibility = 1;
    }

    /* ��¸��¤����� �ʲ��Ϥ�Ԥ���� */

    if (OptAnalysis == OPT_CASE2) {
	sp->Best_mgr->score = -10000;
	call_case_analysis(sp, sp->Best_mgr->dpnd);
    }

    if (Possibility != 0) {
	if (OptAnalysis == OPT_CASE ||
	    OptAnalysis == OPT_CASE2) {
	    /* �ʲ��Ϥη�̤��Ѹ�ʸ��� */
	    for (i = 0; i < sp->Best_mgr->pred_num; i++) {
		sp->Best_mgr->cpm[i].pred_b_ptr->cpm_ptr = &(sp->Best_mgr->cpm[i]);
		/* �� ����Ū
		   ����ΤȤ��� make_dpnd_tree() ��ƤӽФ��� cpm_ptr ���ʤ��ʤ�Τǡ�
		   �����ǥ��ԡ����Ƥ��� */
		check_b_ptr = sp->Best_mgr->cpm[i].pred_b_ptr;
		while (check_b_ptr->parent && check_b_ptr->parent->para_top_p == TRUE && 
		       check_b_ptr->parent->cpm_ptr == NULL) {
		    check_b_ptr->parent->cpm_ptr = &(sp->Best_mgr->cpm[i]);
		    check_b_ptr = check_b_ptr->parent;
		}

		if (lastflag < 0 && !check_feature(sp->Best_mgr->cpm[i].pred_b_ptr->f, "�����")) {
		    lastflag = i;
		}

		/* �Ƴ����Ǥο��Ѹ�������
		   �� ʸ̮���ϤΤȤ��˳ʥե졼�����ꤷ�Ƥʤ��Ƥ�ʲ��ϤϹԤäƤ���Τ�
		      ������������� */
		for (j = 0; j < sp->Best_mgr->cpm[i].cf.element_num; j++) {
		    /* ��ά���Ϥη�� or Ϣ�ν����Ͻ��� */
		    if (sp->Best_mgr->cpm[i].elem_b_num[j] == -2 || 
			sp->Best_mgr->cpm[i].elem_b_ptr[j]->num > sp->Best_mgr->cpm[i].pred_b_ptr->num) {
			continue;
		    }
		    sp->Best_mgr->cpm[i].elem_b_ptr[j]->pred_b_ptr = sp->Best_mgr->cpm[i].pred_b_ptr;
		}

		/* ʸ̮���ϤΤȤ��ϳʥե졼����ꤷ�Ƥ����Ѹ��ˤĤ��ƤΤ� */
		if (OptDisc != OPT_DISC || sp->Best_mgr->cpm[i].decided == CF_DECIDED) {
		    assign_gaga_slot(sp, &(sp->Best_mgr->cpm[i]));
		    assign_ga_subject(sp, &(sp->Best_mgr->cpm[i]));
		    fix_sm_place(sp, &(sp->Best_mgr->cpm[i]));
		    /* record_match_ex(sp, &(sp->Best_mgr->cpm[i])); */
		    /* �ʲ��Ϥη�̤� feature�� */
		    record_case_analysis(sp, &(sp->Best_mgr->cpm[i]), NULL, lastflag == i ? 1 : 0);
		}
		else if (sp->Best_mgr->cpm[i].decided == CF_CAND_DECIDED) {
		    assign_ga_subject(sp, &(sp->Best_mgr->cpm[i]));
		}
	    }
	    /* ��ʤ� feature ��(��ͭ̾��ǧ��������)
	    assign_agent();
	    */
	}
	return TRUE;
    } else { 
	return FALSE;
    }
}

/*==================================================================*/
	    int detect_dpnd_case_struct(SENTENCE_DATA *sp)
/*==================================================================*/
{
    int i;
    DPND dpnd;

    sp->Best_mgr->score = -10000; /* �������ϡ֤���礭���׻������촹����Τǡ�
				    ����ͤϽ�ʬ���������Ƥ��� */
    sp->Best_mgr->dflt = 0;
    sp->Best_mgr->ID = -1;
    Possibility = 0;
    dpndID = 0;

    Op_Best_mgr.score = -10000;
    Op_Best_mgr.ID = -1;

    /* ������֤ν���� */

    for (i = 0; i < sp->Bnst_num; i++) {
	dpnd.head[i] = -1;
	dpnd.dflt[i] = 0;
	dpnd.mask[i] = 1;
	memset(&(dpnd.op[i]), 0, sizeof(struct _optionalcase));
	memset(&(dpnd.check[i]), 0, sizeof(CHECK_DATA));
	dpnd.check[i].num = -1;
	dpnd.f[i] = NULL;
    }
    dpnd.pos = sp->Bnst_num - 1;
    dpnd.flag = 0;
    dpnd.comment = NULL;

    /* �ʲ��ϥ���å���ν���� */
    if (OptAnalysis == OPT_CASE) {
	InitCPMcache();
    }

    /* ��¸��¤���� --> �ʹ�¤���� */

    decide_dpnd(sp, dpnd);

    /* �ʲ��ϥ���å���ν���� */
    if (OptAnalysis == OPT_CASE) {
	ClearCPMcache();
    }

    /* ��¤�����ν��� */

    return after_decide_dpnd(sp);
}

/*==================================================================*/
	       void memo_by_program(SENTENCE_DATA *sp)
/*==================================================================*/
{
    /*
     *  �ץ����ˤ����ؤν񤭹���
     */

    /* ���¤���˵�Ͽ������
    int i;

    for (i = 0; i < sp->Bnst_num - 1; i++) {
	if (sp->Best_mgr->dpnd.type[i] == 'd') {
	    strcat(PM_Memo, " ����d");
	    sprintf(PM_Memo+strlen(PM_Memo), "(%d)", i);
	} else if (sp->Best_mgr->dpnd.type[i] == 'R') {
	    strcat(PM_Memo, " ����R");
	    sprintf(PM_Memo+strlen(PM_Memo), "(%d)", i);
	}
    }
    */

    /* �󤤷����������˵�Ͽ������

    for (i = 0; i < sp->Bnst_num - 1; i++) {
	if (sp->Best_mgr->dpnd.head[i] > i + 3 &&
	    !check_feature(sp->bnst_data[i].f, "��") &&
	    !check_feature(sp->bnst_data[i].f, "����") &&
	    !check_feature(sp->bnst_data[i].f, "�Ѹ�") &&
	    !check_feature(sp->bnst_data[i].f, "��:����") &&
	    !check_feature(sp->bnst_data[i].f, "�Ѹ�:̵") &&
	    !check_feature(sp->bnst_data[i].f, "�¥�") &&
	    !check_feature(sp->bnst_data[i+1].f, "��̻�")) {
	    strcat(PM_Memo, " ��");
	    sprintf(PM_Memo+strlen(PM_Memo), "(%d)", i);
	}
    }
    */
}

/*====================================================================
                               END
====================================================================*/
