/*====================================================================

			      �ʹ�¤����

                                               S.Kurohashi 91.10. 9
                                               S.Kurohashi 93. 5.31

    $Id$
====================================================================*/
#include "knp.h"

extern int Possibility;

CF_MATCH_MGR	Cf_match_mgr[IPAL_FRAME_MAX * 5];	/* ����ΰ� */
TOTAL_MGR	Dflt_mgr;
TOTAL_MGR	Work_mgr;

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
    {"��դ���", "��ե���", 13},
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
    {"�ˤ�뤺", "�˥�륺", 35},
    {"�ˤ����뤺", "�˥����륺", 36},
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
    int i, frame_num, max_score;
    
    /* ����� */
    cpm_ptr->pred_b_ptr = b_ptr;
    cpm_ptr->score = -1;
    cpm_ptr->result_num = 0;

    /* ����ʸ¦�γ��������� */
    cf_ptr->voice = b_ptr->voice;
    make_data_cframe(cpm_ptr);

    /* �ʥե졼����ϥ����å�
    if (cf_ptr->element_num == 0) {
	cpm_ptr->cmm[0].cf_ptr = NULL;
	return -3;
    }
    */

    /* �ʥե졼������ */
    frame_num = cpm_ptr->pred_b_ptr->cf_num;
    if (frame_num == 0) {
	return -2;
    }
    for (i = 0; i < frame_num; i++)
      (Cf_match_mgr + i)->cf_ptr = cpm_ptr->pred_b_ptr->cf_ptr + i;

    /* �����Ǥʤ��λ��μ¸� 98/12/16 */
    if (cf_ptr->element_num == 0) {
	case_frame_match(cf_ptr, Cf_match_mgr, OptCFMode);
	cpm_ptr->score = Cf_match_mgr->score;
	cpm_ptr->cmm[0] = *Cf_match_mgr;
	cpm_ptr->result_num = 1;
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
    
    if (b_ptr->para_top_p != TRUE && check_feature(b_ptr->f, "�Ѹ�") && 
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
		  void call_case_analysis(DPND dpnd)
/*==================================================================*/
{
    int i;
    int topic_score;
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
       ������default�Ȥε�Υ�Τ���,�������� */
    Work_mgr.score -= Work_mgr.dflt * 2;
    for (i = 0; i < sp->Bnst_num; i++) {
	if (check_feature((sp->bnst_data+i)->f, "����") &&
	    (cp = (char *)check_feature(
		(sp->bnst_data+(sp->bnst_data+i)->dpnd_head)->f, "�����"))) {
	    sscanf(cp, "%*[^:]:%d", &topic_score);
	    Work_mgr.score += topic_score;
	}
    }
        
    /* ����� */

    if (Work_mgr.score > Best_mgr.score ||
	(Work_mgr.score == Best_mgr.score && 
	 compare_dpnd(&Work_mgr, &Best_mgr) == TRUE))
	Best_mgr = Work_mgr; 
    if (Work_mgr.dflt == 0) Dflt_mgr = Work_mgr;
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
	cmm_ptr = &(cpm_ptr->cmm[0]);
	cf_ptr = cmm_ptr->cf_ptr;
	pred_b_ptr = cpm_ptr->pred_b_ptr;

	if (!cf_ptr) continue;	/* �ʥե졼�ब�ʤ���� */

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

	    for (k = 0; cf_ptr->sm[i][k]; k+=12) {
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
