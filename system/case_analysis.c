/*====================================================================

			      �ʹ�¤����

                                               S.Kurohashi 91.10. 9
                                               S.Kurohashi 93. 5.31

    $Id$
====================================================================*/
#include "knp.h"

extern int Possibility;

CF_MATCH_MGR	*Cf_match_mgr = NULL;	/* ����ΰ� */
/* TOTAL_MGR	Dflt_mgr; */
TOTAL_MGR	Work_mgr;

/*==================================================================*/
		      void init_case_analysis()
/*==================================================================*/
{
    if (OptAnalysis == OPT_CASE || 
	OptAnalysis == OPT_CASE2 || 
	OptAnalysis == OPT_DISC) {
	int i, j;

	Cf_match_mgr = (CF_MATCH_MGR *)malloc_data(sizeof(CF_MATCH_MGR)*IPAL_FRAME_MAX*5, 
						   "init_case_analysis");

	for (i = 0; i < CPM_MAX; i++) {
	    for (j = 0; j < CF_ELEMENT_MAX; j++) {
		if (Thesaurus == USE_BGH) {
		    Work_mgr.cpm[i].cf.ex[j] = 
			(char *)malloc_data(sizeof(char)*EX_ELEMENT_MAX*BGH_CODE_SIZE, "init_case_analysis");
		}
		else if (Thesaurus == USE_NTT) {
		    Work_mgr.cpm[i].cf.ex2[j] = 
			(char *)malloc_data(sizeof(char)*SM_ELEMENT_MAX*SM_CODE_SIZE, "init_case_analysis");
		}
	    }
	}
    }
}

/*====================================================================
		       �ʽ����ʸ���ݥ������б�
====================================================================*/

struct PP_STR_TO_CODE {
    char *hstr;
    char *kstr;
    int  code;
} PP_str_to_code[] = {
    {"��", "��", 0},		/* �ʽ���Τʤ����(����ɽ����) */
    {"��", "��", 1},
    {"��", "��", 2},
    {"��", "��", 3},
    {"����", "����", 4},
    {"��", "��", 5},
    {"���", "���", 6},
    {"��", "��", 7},
    {"��", "��", 8},
    {"�ˤ�ä�", "�˥�å�", 9},
    {"��ᤰ��", "��ᥰ��", 10},	/* ʣ�缭�ط� */
    {"��Ĥ�����", "��ĥ�����", 11},
    {"��Ĥ�����", "��ĥ�����", 12},
    {"��դ����", "��ե����", 13},
    {"��Ϥ����", "��ϥ����", 14},
    {"�ˤ����", "�˥����", 15},
    {"�ˤ���", "�˥���", 16},
    {"�ˤऱ��", "�˥ॱ��", 17},
    {"�ˤȤ�ʤ�", "�˥ȥ�ʥ�", 18},
    {"�ˤ�ȤŤ�", "�˥�ȥť�", 19},
    {"��Τ���", "��Υ���", 20},
    {"�ˤ��", "�˥��", 21},
    {"�ˤ�������", "�˥�������", 22},
    {"�ˤ��󤹤�", "�˥��󥹥�", 23},
    {"�ˤ����", "�˥����", 24},
    {"�ˤ���", "�˥���", 25},
    {"�ˤĤ�", "�˥ĥ�", 26},
    {"�ˤȤ�", "�˥ȥ�", 27},
    {"�ˤ��廊��", "�˥��泌��", 28},
    {"�ˤ�����", "�˥������", 29},
    {"�ˤĤŤ�", "�˥ĥť�", 30},
    {"�ˤ��碌��", "�˥��糧��", 31},
    {"�ˤ���٤�", "�˥���٥�", 32},
    {"�ˤʤ��", "�˥ʥ��", 33},
    {"�Ȥ���", "�ȥ���", 34},
    {"�ˤ���", "�˥���", 35},
    {"�ˤ������", "�˥������", 36},
    {"����", "����", 37},	/* �˳�, ̵�ʤǻ��֤Ǥ����Τ���֤Ȥ����ʤȤ��ư��� */
    {"�ޤ�", "�ޥ�", 38},	/* ��������ʤ��ʤǤ��뤬������¦�γʤȤ���ɽ�����뤿���
				   �񤤤Ƥ��� */
    {"��", "��", 1},		/* NTT����Ǥϡ֥����׹�ʸ���֥ϥ���
				   �� NTT����Ρ֥ϡפ�1(code)���Ѵ�����뤬,
				      1�����������ǡ֥��פ��Ѵ������ */
    {"��", "��", -2},		/* ������ʸ���､���� */
    {NULL, NULL, -1}		/* �ʽ�����������Τ��(���������) */
};

/*====================================================================
			 ʸ���ݥ������б��ؿ�
====================================================================*/
int pp_kstr_to_code(char *cp)
{
    int i;
    for (i = 0; PP_str_to_code[i].kstr; i++)
      if (str_eq(PP_str_to_code[i].kstr, cp))
	return PP_str_to_code[i].code;
    
    if (str_eq(cp, "�˥ȥå�"))		/* ���Ԥġ� IPAL�ΥХ� ?? */
      return pp_kstr_to_code("�˥�å�");
    else if (str_eq(cp, "��"))		/* �����ǤǤʤ��ʤ��� */
      return -1;

    /* fprintf(stderr, "Invalid string (%s) in PP !\n", cp); */
    return -1;
}

int pp_hstr_to_code(char *cp)
{
    int i;
    for (i = 0; PP_str_to_code[i].hstr; i++)
      if (str_eq(PP_str_to_code[i].hstr, cp))
	return PP_str_to_code[i].code;
    return -1;
}

char *pp_code_to_kstr(int num)
{
    return PP_str_to_code[num].kstr;
}

char *pp_code_to_hstr(int num)
{
    return PP_str_to_code[num].hstr;
}

/*
int sm_zstr_to_code(char *cp)
{
    int i;
    for (i = 0; SM_str_to_code[i].zstr; i++)
      if (str_eq(SM_str_to_code[i].zstr, cp))
	return SM_str_to_code[i].code;
    * fprintf(stderr, "Invalid string (%s) in SM !\n", cp); *
    return -1;
}

int sm_str_to_code(char *cp)
{
    int i;
    
    for (i = 0; SM_str_to_code[i].str; i++)
      if (str_eq(SM_str_to_code[i].str, cp))
	return SM_str_to_code[i].code;
    * fprintf(stderr, "Invalid string (%s) in SM !\n", cp); *
    return -1;
}

char *sm_code_to_str(int code)
{
    int i;

    for (i = 0; SM_str_to_code[i].str; i++)
      if (SM_str_to_code[i].code == code)
	return SM_str_to_code[i].str;
    fprintf(stderr, "Invalid code (%d) in SM !\n", code);
    return NULL;
}
*/

/*==================================================================*/
     int case_analysis(CF_PRED_MGR *cpm_ptr, BNST_DATA *b_ptr)
/*==================================================================*/
{
    /*
                                              ����
      ���Ϥγ����Ǥ��ʤ����                    -3
      �ʥե졼�ब�ʤ����                      -2
      ����¦��ɬ�ܳʤ��Ĥ���(����������)      -1
      ��������                               score (0�ʾ�)
    */

    CASE_FRAME *cf_ptr = &(cpm_ptr->cf);
    int i, frame_num, max_score, default_score;
    
    /* ����� */
    cpm_ptr->pred_b_ptr = b_ptr;
    cpm_ptr->score = -1;
    cpm_ptr->result_num = 0;
    cpm_ptr->cmm[0].cf_ptr = NULL;

    /* ����ʸ¦�γ��������� */
    cf_ptr->voice = b_ptr->voice;
    default_score = make_data_cframe(cpm_ptr);

    /* �ʥե졼����ϥ����å�
    if (cf_ptr->element_num == 0) {
	cpm_ptr->cmm[0].cf_ptr = NULL;
	return -3;
    }
    */

    /* �ʥե졼������ */
    frame_num = 0;
    for (i = 0; i < b_ptr->cf_num; i++) {
	/* ���٤��礷���ե졼��ʤΤˤ⤫����餺��
	   ���٤���ʬ�˷���ʤ���¤�ξ��ϥ����å� */
  	if ((b_ptr->cf_ptr + i)->concatenated_flag == 1 && 
  	    b_ptr->num > 0 && (b_ptr-1)->dpnd_head != b_ptr->num) {
  	    continue;
  	}
	(Cf_match_mgr + frame_num++)->cf_ptr = b_ptr->cf_ptr + i;
    }

    if (frame_num == 0) {
	return -2;
    }

    /* �����Ǥʤ��λ��μ¸� 98/12/16 */
    if (cf_ptr->element_num == 0) {
	case_frame_match(cf_ptr, Cf_match_mgr, OptCFMode);
	cpm_ptr->score = Cf_match_mgr->score;
	cpm_ptr->cmm[0] = *Cf_match_mgr;
	cpm_ptr->result_num = 1;
	frame_num = 1;
    }
    else { /* ����else��tentative */
	for (i = 0; i < frame_num; i++) {

	    /* �����ǽ
	       EXAMPLE
	       SEMANTIC_MARKER

	       ChangeLog:
	       ��̣�ޡ��������Ѥ��ѹ� (1998/10/02)
	       ���ץ���������       (1999/06/15)
	       */

	    case_frame_match(cf_ptr, Cf_match_mgr+i, OptCFMode);

	    /* ���γʥե졼��Ȥ��б��դ�������������Ǥ���е��� */

	    if ((Cf_match_mgr+i)->score > cpm_ptr->score) {
		cpm_ptr->score = (Cf_match_mgr+i)->score;
		cpm_ptr->cmm[0] = *(Cf_match_mgr+i);
		cpm_ptr->result_num = 1;
	    }

	    /* ���γʥե졼��Ȥ��б��դ��������������Ʊ���Ǥ⵭�� */

	    else if ((Cf_match_mgr+i)->score == cpm_ptr->score) {
		if (cpm_ptr->result_num >= CMM_MAX)
		    fprintf(stderr, "Not enough cmm.\n");
		else
		    cpm_ptr->cmm[cpm_ptr->result_num++] = *(Cf_match_mgr+i);
	    }
	}
    }

    /* corpus based case analysis 00/01/04

    for (i = 0; i < cpm_ptr->cf.element_num; i++) {
	if (cpm_ptr->elem_b_num[i] == -1) {
	    score -= cpm_ptr->cf.pred_b_ptr.dpnd_dflt * 2;
	} else {
	    score -= cpm_ptr->elem_b_ptr[i].dpnd_dflt * 2;
	}	
    }
    */

    /* ���δط��Υ�������­�� */
    if (cpm_ptr->score > -1)  {
	cpm_ptr->score += default_score;
    }

    if (OptDisplay == OPT_DEBUG) {
	print_data_cframe(cf_ptr);
	print_good_crrspnds(cpm_ptr, Cf_match_mgr, frame_num);
    }

    return cpm_ptr->score;
}

/*==================================================================*/
      int all_case_analysis(BNST_DATA *b_ptr, TOTAL_MGR *t_ptr)
/*==================================================================*/
{
    CF_PRED_MGR *cpm_ptr;
    int i;
    int one_case_point;
    
    if (b_ptr->para_top_p != TRUE && 
	(check_feature(b_ptr->f, "�Ѹ�") || 
	 check_feature(b_ptr->f, "���Ѹ�")) && 
	!check_feature(b_ptr->f, "ʣ�缭")) {

	cpm_ptr = &(t_ptr->cpm[t_ptr->pred_num]);

	one_case_point = case_analysis(cpm_ptr, b_ptr);

	/* ����������(����¦��ɬ�ܳʤ��Ĥ�)���ˤ��ΰ�¸��¤�β��Ϥ�
	   ������
	if (one_case_point == -1) return FALSE;
	*/

	t_ptr->score += one_case_point;
	
	if (t_ptr->pred_num++ >= CPM_MAX) {
	    fprintf(stderr, "too many predicates in a sentence.\n");
	    exit(1);
	}
    }

    for (i = 0; b_ptr->child[i]; i++)
	if (all_case_analysis(b_ptr->child[i], t_ptr) == FALSE)
	    return FALSE;

    return TRUE;
}

/*==================================================================*/
	    void copy_cf(CASE_FRAME *dst, CASE_FRAME *src)
/*==================================================================*/
{
    int i, j;

    dst->element_num = src->element_num;
    for (i = 0; i < CF_ELEMENT_MAX; i++) {
	dst->oblig[i] = src->oblig[i];
	for (j = 0; j < PP_ELEMENT_MAX; j++) {
	    dst->pp[i][j] = src->pp[i][j];
	}
	for (j = 0; j < SM_ELEMENT_MAX*SM_CODE_SIZE; j++) {
	    dst->sm[i][j] = src->sm[i][j];
	}
	for (j = 0; j < SM_ELEMENT_MAX; j++) {
	    dst->sm_flag[i][j] = src->sm_flag[i][j];
	}
	if (Thesaurus == USE_BGH) {
	    strcpy(dst->ex[i], src->ex[i]);
	}
	else if (Thesaurus == USE_NTT) {
	    strcpy(dst->ex2[i], src->ex2[i]);
	}
	dst->examples[i] = src->examples[i];	/* �����Ȥ�������ꤢ�� */
    }
    dst->voice = src->voice;
    dst->ipal_address = src->ipal_address;
    dst->ipal_size = src->ipal_size;
    strcpy(dst->ipal_id, src->ipal_id);
    strcpy(dst->imi, src->imi);
    dst->concatenated_flag = src->concatenated_flag;
}

/*==================================================================*/
	  void copy_cpm(CF_PRED_MGR *dst, CF_PRED_MGR *src)
/*==================================================================*/
{
    int i;

    copy_cf(&dst->cf, &src->cf);
    dst->pred_b_ptr = src->pred_b_ptr;
    for (i = 0; i < CF_ELEMENT_MAX; i++) {
	dst->elem_b_ptr[i] = src->elem_b_ptr[i];
	dst->elem_b_num[i] = src->elem_b_num[i];
    }
    dst->score = src->score;
    dst->result_num = src->result_num;
    for (i = 0; i < CMM_MAX; i++) {
	dst->cmm[i] = src->cmm[i];
    }
}

/*==================================================================*/
	    void copy_mgr(TOTAL_MGR *dst, TOTAL_MGR *src)
/*==================================================================*/
{
    int i;

    dst->dpnd = src->dpnd;
    dst->pssb = src->pssb;
    dst->dflt = src->dflt;
    dst->score = src->score;
    dst->pred_num = src->pred_num;
    for (i = 0; i < CPM_MAX; i++) {
	copy_cpm(&dst->cpm[i], &src->cpm[i]);
    }
    dst->ID = src->ID;
}

/*==================================================================*/
		  void call_case_analysis(DPND dpnd)
/*==================================================================*/
{
    int i, j;
    int topic_score, topic_score_sum = 0, topic_slot[2], distance_cost = 0;
    char *cp;

    /* �ʹ�¤���ϤΥᥤ��ؿ� */

    /* ��¸��¤�ں��� */

    dpnd_info_to_bnst(&dpnd);
    make_dpnd_tree();
	
    if (OptDisplay == OPT_DEBUG)
	print_kakari();

    /* �ʲ��Ϻ���ΰ�ν���� */
	
    Work_mgr.pssb = Possibility;
    Work_mgr.dpnd = dpnd;
    Work_mgr.score = 0;
    Work_mgr.pred_num = 0;
    Work_mgr.dflt = 0;
    for (i = 0; i < sp->Bnst_num; i++)
	Work_mgr.dflt += dpnd.dflt[i];
    
    /* �ʲ��ϸƤӽФ� */

    if (all_case_analysis(sp->bnst_data+sp->Bnst_num-1, &Work_mgr) == TRUE)
	Possibility++;
    else
	return;

    /* corpus based case analysis 00/01/04
       ������ default �Ȥε�Υ�Τ���, �������� */

    for (i = 0; i < sp->Bnst_num-1; i++) {
	if (dpnd.dflt[i] > 0) {
	    /* ���� */
	    if (check_feature((sp->bnst_data+i)->f, "����")) {
		distance_cost += dpnd.dflt[i];
		continue;
	    }
	    /* ����ʳ� */
	    /* ����¦��Ϣ�ѤǤʤ��Ȥ� */
	    if (!check_feature((sp->bnst_data+i)->f, "��:Ϣ��")) {
		/* ��ʬ���������ʤ����٤ζ����Ѹ� (Ϣ�ΰʳ�) ��ۤ��Ƥ���Ȥ� */
		if (!check_feature((sp->bnst_data+i)->f, "����")) {
		    if (dpnd.head[i] > i+1 && 
			subordinate_level_check("B", sp->bnst_data+i+1) && 
			(cp = (char *)check_feature((sp->bnst_data+i+1)->f, "��"))) {
			if (strcmp(cp+3, "Ϣ��") && strcmp(cp+3, "Ϣ��")) {
			    distance_cost += 2;
			}
		    }
		}
		/* ��ʬ������������*/
		else {
		    /* �٤˷���Ȥ� */
		    if (dpnd.head[i] == i+1) {
			distance_cost += 2;
		    }
		}
	    }
	    distance_cost += dpnd.dflt[i]*2;
	}		    
    }

    Work_mgr.score -= distance_cost;

    for (i = sp->Bnst_num-1; i > 0; i--) {
	/* ʸ�������Ѹ����Ȥ������������� */
	if (cp = (char *)check_feature((sp->bnst_data+i)->f, "�����")) {

	    /* topic_slot[0]	���ְʳ��ΥϳʤΥ���å�
	       topic_slot[1]	��<<����>>�ϡפΥ���å�
	       ξ���Ȥ� 1 �ʲ��������Ĥ��ʤ�
	    */

	    topic_slot[0] = 0;
	    topic_slot[1] = 0;

	    /* ����¦��õ�� */
	    for (j = 0; j < i; j++) {
		if (dpnd.head[j] != i) {
		    continue;
		}
		if (check_feature((sp->bnst_data+j)->f, "����")) {
		    if (check_feature((sp->bnst_data+j)->f, "����")) {
			topic_slot[1]++;
		    }
		    else {
			topic_slot[0]++;
		    }
		}
	    }

	    if ((topic_slot[0] == 1 || topic_slot[1] == 1) && 
		(topic_slot[0] < 2 && topic_slot[1] < 2)) {
		sscanf(cp, "%*[^:]:%d", &topic_score);
		/* �Ȥꤢ���������֤� topic_score ���ˤ��Ƥߤ� */
		Work_mgr.score += topic_score*topic_slot[0]+topic_score*topic_slot[1];
		if (OptDisplay == OPT_DEBUG) {
		    topic_score_sum += topic_score*topic_slot[0]+topic_score*topic_slot[1];
		}
	    }

	}
    }

    if (OptDisplay == OPT_DEBUG) {
	fprintf(stdout, "�� %d�� (��Υ���� %d�� (%d��) ���ꥹ���� %d��)\n", 
		Work_mgr.score, distance_cost, Work_mgr.dflt*2, topic_score_sum);
    }
        
    /* ����� */

    if (Work_mgr.score > Best_mgr.score ||
	(Work_mgr.score == Best_mgr.score && 
	 compare_dpnd(&Work_mgr, &Best_mgr) == TRUE))
	copy_mgr(&Best_mgr, &Work_mgr);
    /* if (Work_mgr.dflt == 0) Dflt_mgr = Work_mgr; */
}

/*==================================================================*/
		     void record_case_analysis()
/*==================================================================*/
{
    int i, j, k, num;
    char feature_buffer[256];
    CF_PRED_MGR *cpm_ptr;
    CF_MATCH_MGR *cmm_ptr;
    CASE_FRAME *cf_ptr;
    BNST_DATA *pred_b_ptr;

    /* �ʲ��Ϥη��(Best_mgr������)��feature�Ȥ����Ѹ�ʸ���Ϳ���� */

    for (j = 0; j < Best_mgr.pred_num; j++) {

	cpm_ptr = &(Best_mgr.cpm[j]);

	/* �ʥե졼�ब�ʤ���� */
	if (cpm_ptr->result_num == 0) {
	    continue;
	}

	cmm_ptr = &(cpm_ptr->cmm[0]);
	cf_ptr = cmm_ptr->cf_ptr;
	pred_b_ptr = cpm_ptr->pred_b_ptr;

	sprintf(feature_buffer, "��̣:%s", cf_ptr->imi);
	assign_cfeature(&(pred_b_ptr->f), feature_buffer);

	for (i = 0; i < cf_ptr->element_num; i++) {
	    num = cmm_ptr->result_lists_p[0].flag[i];

	    /* �б��ط� */

	    if (num == UNASSIGNED || cmm_ptr->score == -2) 
		sprintf(feature_buffer, "���س�N%d:NIL:", i+1);
	    else
		sprintf(feature_buffer, "���س�N%d:%d:", i+1, 
			cpm_ptr->elem_b_ptr[num]->num);
	    
	    /* ɽ�س� */

	    for (k = 0; cf_ptr->pp[i][k] != END_M; k++) {
		if (k != 0) 
		    sprintf(feature_buffer+strlen(feature_buffer), "/");
		sprintf(feature_buffer+strlen(feature_buffer), 
			"%s", pp_code_to_kstr(cf_ptr->pp[i][k]));
	    }
	    if (cf_ptr->oblig[i] == FALSE)
		sprintf(feature_buffer+strlen(feature_buffer), "*");

	    sprintf(feature_buffer+strlen(feature_buffer), ":");
	    
	    /* ��̣�� */

	    for (k = 0; cf_ptr->sm[i][k]; k+=SM_CODE_SIZE) {
		if (k != 0) 
		    sprintf(feature_buffer+strlen(feature_buffer), "/");
		sprintf(feature_buffer+strlen(feature_buffer), 
			"%12.12s", &(cf_ptr->sm[i][k]));
	    }

	    /* feature��Ϳ */

	    assign_cfeature(&(pred_b_ptr->f), feature_buffer);
	}
    }
}	    

/*====================================================================
                               END
====================================================================*/
