/*====================================================================

			      �ʹ�¤����

                                               S.Kurohashi 91.10. 9
                                               S.Kurohashi 93. 5.31

    $Id$
====================================================================*/
#include "knp.h"

extern int Possibility;
extern int MAX_Case_frame_num;

CF_MATCH_MGR	*Cf_match_mgr = NULL;	/* ����ΰ� */
TOTAL_MGR	Work_mgr;

int	DISTANCE_STEP	= 5;
int	RENKAKU_STEP	= 2;
int	STRONG_V_COST	= 8;
int	ADJACENT_TOUTEN_COST	= 5;
int	LEVELA_COST	= 4;
int	TEIDAI_STEP	= 2;

/*==================================================================*/
			  void realloc_cmm()
/*==================================================================*/
{
    Cf_match_mgr = (CF_MATCH_MGR *)realloc_data(Cf_match_mgr, 
						sizeof(CF_MATCH_MGR)*(MAX_Case_frame_num), 
						"realloc_cmm");
}

/*==================================================================*/
		 void init_case_frame(CASE_FRAME *cf)
/*==================================================================*/
{
    int j;

    for (j = 0; j < CF_ELEMENT_MAX; j++) {
	if (Thesaurus == USE_BGH) {
	    cf->ex[j] = 
		(char *)malloc_data(sizeof(char)*EX_ELEMENT_MAX*BGH_CODE_SIZE, 
				    "init_case_frame");
	}
	else if (Thesaurus == USE_NTT) {
	    cf->ex[j] = 
		(char *)malloc_data(sizeof(char)*SM_ELEMENT_MAX*SM_CODE_SIZE, 
				    "init_case_frame");
	}
	cf->sm[j] = 
	    (char *)malloc_data(sizeof(char)*SM_ELEMENT_MAX*SM_CODE_SIZE, 
				"init_case_frame");
	cf->ex_list[j] = (char **)malloc_data(sizeof(char *), 
					      "init_case_frame");
	cf->ex_list[j][0] = (char *)malloc_data(sizeof(char)*WORD_LEN_MAX, 
						"init_case_frame");
	cf->ex_freq[j] = (int *)malloc_data(sizeof(int), 
					    "init_case_frame");
    }
}

/*==================================================================*/
		    void init_case_analysis_cmm()
/*==================================================================*/
{
    if (OptAnalysis == OPT_CASE || 
	OptAnalysis == OPT_CASE2) {

	/* ���cmm�ΰ���� */
	Cf_match_mgr = 
	    (CF_MATCH_MGR *)malloc_data(sizeof(CF_MATCH_MGR)*ALL_CASE_FRAME_MAX, 
					"init_case_analysis_cmm");

	init_mgr_cf(&Work_mgr);
    }
}

/*==================================================================*/
		void clear_case_frame(CASE_FRAME *cf)
/*==================================================================*/
{
    int j;

    for (j = 0; j < CF_ELEMENT_MAX; j++) {
	free(cf->ex[j]);
	free(cf->sm[j]);
	free(cf->ex_list[j][0]);
	free(cf->ex_list[j]);
	free(cf->ex_freq[j]);
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
    {"�ˤ�����", "�˥�����", 29},
    {"�ˤĤŤ�", "�˥ĥť�", 30},
    {"�ˤ��碌��", "�˥��糧��", 31},
    {"�ˤ���٤�", "�˥���٥�", 32},
    {"�ˤʤ��", "�˥ʥ��", 33},
    {"�Ȥ���", "�ȥ���", 34},
    {"�ˤ���", "�˥���", 35},
    {"�ˤ������", "�˥������", 36},
    {"�Ȥ���", "�ȥ���", 37},	/* ���Ȥ�����? */
    {"����", "����", 38},	/* �˳�, ̵�ʤǻ��֤Ǥ����Τ���֤Ȥ����ʤȤ��ư��� */
    {"�ޤ�", "�ޥ�", 39},	/* ��������ʤ��ʤǤ��뤬������¦�γʤȤ���ɽ�����뤿���
				   �񤤤Ƥ��� */
    {"����", "����", 40},
    {"��", "��", 41},		/* �ʥե졼��Υγ� */
    {"����", "����", 42},
    {"���δط�", "���δط�", 43},
    {"����", "����", 42},
    {"���δط�", "���δط�", 43},	/* for backward compatibility */
    {"��", "��", 1},		/* NTT����Ǥϡ֥����׹�ʸ���֥ϥ���
				   �� NTT����Ρ֥ϡפ�1(code)���Ѵ�����뤬,
				      1�����������ǡ֥��פ��Ѵ������ */
    {"̤", "̤", -3},		/* �ʥե졼��ˤ�ä�ưŪ�˳�����Ƥ�ʤ���ꤹ�� */
    {"��", "��", -2},		/* ������ʸ���､���� */
    {NULL, NULL, -1}		/* �ʽ�����������Τ��(���������) */
};

/* �� �ʤκ�������Ѥ����顢PP_NUMBER(const.h)���Ѥ��뤳�� */

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
	return END_M;

    /* fprintf(stderr, "Invalid string (%s) in PP !\n", cp); */
    return END_M;
}

int pp_hstr_to_code(char *cp)
{
    int i;
    for (i = 0; PP_str_to_code[i].hstr; i++)
	if (str_eq(PP_str_to_code[i].hstr, cp))
	    return PP_str_to_code[i].code;
    return END_M;
}

char *pp_code_to_kstr(int num)
{
    return PP_str_to_code[num].kstr;
}

char *pp_code_to_hstr(int num)
{
    return PP_str_to_code[num].hstr;
}

char *pp_code_to_kstr_in_context(CF_PRED_MGR *cpm_ptr, int num)
{
    if ((cpm_ptr->cf.type_flag && MatchPP(num, "��")) || cpm_ptr->cf.type == CF_NOUN) {
	return "��";
    }   
    return pp_code_to_kstr(num);
}

/*==================================================================*/
		    int MatchPPn(int n, int *list)
/*==================================================================*/
{
    int i;

    if (n < 0) {
	return 0;
    }

    for (i = 0; list[i] != END_M; i++) {
	if (n == list[i]) {
	    return 1;
	}
    }
    return 0;
}

/*==================================================================*/
		     int MatchPP(int n, char *pp)
/*==================================================================*/
{
    if (n < 0) {
	return 0;
    }
    if (str_eq(pp_code_to_kstr(n), pp)) {
	return 1;
    }
    return 0;
}

/*==================================================================*/
		    int MatchPP2(int *n, char *pp)
/*==================================================================*/
{
    int i;

    /* �ʤ���������Ĵ�٤����ʤ����뤫�ɤ��� */

    if (n < 0) {
	return 0;
    }

    for (i = 0; *(n+i) != END_M; i++) {
	if (str_eq(pp_code_to_kstr(*(n+i)), pp)) {
	    return 1;
	}
    }
    return 0;
}

/*==================================================================*/
		 int CheckCfAdjacent(CASE_FRAME *cf)
/*==================================================================*/
{
    int i;
    for (i = 0; i < cf->element_num; i++) {
	if (cf->adjacent[i] && 
	    MatchPP(cf->pp[i][0], "����")) {
	    return FALSE;
	}
    }
    return TRUE;
}

/*==================================================================*/
	  int CheckCfClosest(CF_MATCH_MGR *cmm, int closest)
/*==================================================================*/
{
    return cmm->cf_ptr->adjacent[cmm->result_lists_d[0].flag[closest]];
}

/*==================================================================*/
double find_best_cf(SENTENCE_DATA *sp, CF_PRED_MGR *cpm_ptr, int closest, int decide)
/*==================================================================*/
{
    int i, j, frame_num = 0, pat_num;
    CASE_FRAME *cf_ptr = &(cpm_ptr->cf);
    TAG_DATA *b_ptr = cpm_ptr->pred_b_ptr;
    CF_MATCH_MGR tempcmm;

    /* �����Ǥʤ��λ��μ¸� */
    if (cf_ptr->element_num == 0) {
	/* �����Ѹ��Τ��٤Ƥγʥե졼��� OR���ޤ���
	   �ʥե졼�ब 1 �ĤΤȤ��Ϥ��줽�Τ�� �ˤ���ͽ�� */
	if (b_ptr->cf_num > 1) {
	    for (i = 0; i < b_ptr->cf_num; i++) {
		if ((b_ptr->cf_ptr+i)->etcflag & CF_SUM) {
		    (Cf_match_mgr + frame_num++)->cf_ptr = b_ptr->cf_ptr + i;
		    break;
		}
	    }
	    /* OR�ʥե졼�ब�ʤ��Ȥ�
	       ��ư,��,��פλ��꤬�ʤ����Ȥ�����Ф����ʤ� */
	    if (frame_num == 0) {
		(Cf_match_mgr + frame_num++)->cf_ptr = b_ptr->cf_ptr;
	    }
	}
	else {
	    (Cf_match_mgr + frame_num++)->cf_ptr = b_ptr->cf_ptr;
	}
	case_frame_match(cpm_ptr, Cf_match_mgr, OptCFMode, -1);
	cpm_ptr->score = (int)Cf_match_mgr->score;
	cpm_ptr->cmm[0] = *Cf_match_mgr;
	cpm_ptr->result_num = 1;
    }
    else {
	int hiragana_prefer_flag = 0;

	/* ɽ�����Ҥ餬�ʤξ��: 
	   �ʥե졼���ɽ�����Ҥ餬�ʤξ�礬¿����ФҤ餬�ʤγʥե졼��Τߤ��оݤˡ�
	   �Ҥ餬�ʰʳ���¿����ФҤ餬�ʰʳ��Τߤ��оݤˤ��� */
	if (check_str_type(b_ptr->head_ptr->Goi) == TYPE_HIRAGANA) {
	    int hiragana_count = 0;
	    for (i = 0; i < b_ptr->cf_num; i++) {
		if (check_str_type((b_ptr->cf_ptr + i)->entry) == TYPE_HIRAGANA) {
		    hiragana_count++;
		}
	    }
	    if (2 * hiragana_count > b_ptr->cf_num) {
		hiragana_prefer_flag = 1;
	    }
	    else {
		hiragana_prefer_flag = -1;
	    }
	}

	/* �ʥե졼������ */
	for (i = 0; i < b_ptr->cf_num; i++) {
	    /* �ʥե졼�ब1�ĤǤϤʤ��Ȥ� */
	    if (b_ptr->cf_num != 1) {
		/* OR �γʥե졼������ */
		if ((b_ptr->cf_ptr+i)->etcflag & CF_SUM) {
		    continue;
		}
		/* ľ���ʤ������ξ��ʤɤ���� */
		else if (CheckCfAdjacent(b_ptr->cf_ptr+i) == FALSE) {
		    continue;
		}
		else if ((hiragana_prefer_flag > 0 && check_str_type((b_ptr->cf_ptr + i)->entry) != TYPE_HIRAGANA) || 
			 (hiragana_prefer_flag < 0 && check_str_type((b_ptr->cf_ptr + i)->entry) == TYPE_HIRAGANA)) {
		    continue;
		}
	    }
	    (Cf_match_mgr + frame_num++)->cf_ptr = b_ptr->cf_ptr + i;
	}

	if (frame_num == 0) {
	    cpm_ptr->score = -2;
	    return -2;
	}

	cpm_ptr->result_num = 0;
	for (i = 0; i < frame_num; i++) {

	    /* �����ǽ
	       EXAMPLE
	       SEMANTIC_MARKER */

	    /* closest ������С�ľ�������ǤΤߤΥ������ˤʤ� */
	    case_frame_match(cpm_ptr, Cf_match_mgr+i, OptCFMode, closest);

	    /* ��̤��Ǽ */
	    cpm_ptr->cmm[cpm_ptr->result_num] = *(Cf_match_mgr+i);

	    /* DEBUG������: ���� print_good_crrspnds() �ǻȤ� Cf_match_mgr �Υ������������� */
	    if (OptDisplay == OPT_DEBUG && closest > -1 && !OptEllipsis) {
		pat_num = count_pat_element((Cf_match_mgr+i)->cf_ptr, &((Cf_match_mgr+i)->result_lists_p[0]));
		if (!((Cf_match_mgr+i)->score < 0 || pat_num == 0)) {
		    (Cf_match_mgr+i)->score = (OptCaseFlag & OPT_CASE_USE_PROBABILITY) ? (Cf_match_mgr+i)->pure_score[0] : ((Cf_match_mgr+i)->pure_score[0] / sqrt((double)pat_num));
		}
	    }

	    /* ��������˥����� */
	    for (j = cpm_ptr->result_num - 1; j >= 0; j--) {
		if (cpm_ptr->cmm[j].score < cpm_ptr->cmm[j+1].score || 
		    (cpm_ptr->cmm[j].score != CASE_MATCH_FAILURE_PROB && 
		     cpm_ptr->cmm[j].score == cpm_ptr->cmm[j+1].score && (
			(closest > -1 && 
			 (CheckCfClosest(&(cpm_ptr->cmm[j+1]), closest) == TRUE && 
			  CheckCfClosest(&(cpm_ptr->cmm[j]), closest) == FALSE)) || 
			(closest < 0 && 
			 cpm_ptr->cmm[j].sufficiency < cpm_ptr->cmm[j+1].sufficiency)))) {
		    tempcmm = cpm_ptr->cmm[j];
		    cpm_ptr->cmm[j] = cpm_ptr->cmm[j+1];
		    cpm_ptr->cmm[j+1] = tempcmm;
		}
		else {
		    break;
		}
	    }
	    if (cpm_ptr->result_num < CMM_MAX - 1) {
		cpm_ptr->result_num++;
	    }
	}

	/* ��������Ʊ���γʥե졼��θĿ������� */
	if (cpm_ptr->result_num > 0) {
	    double top;
	    int cflag = 0;
	    cpm_ptr->tie_num = 1;
	    top = cpm_ptr->cmm[0].score;
	    if (closest > -1 && 
		CheckCfClosest(&(cpm_ptr->cmm[0]), closest) == TRUE) {
		cflag = 1;
	    }
	    for (i = 1; i < cpm_ptr->result_num; i++) {
		/* score ���ǹ�ǡ�
		   ľ�������Ǥ��ʥե졼���ľ���ʤ˥ޥå����Ƥ����Τ������(0���ܤ�����å�)
		   ľ�������Ǥ��ʥե졼���ľ���ʤ˥ޥå����Ƥ��뤳�Ȥ����
		   ��
		   score ���ǹ�Ǥ��뤳�Ȥ����ˤ���
		*/
		if (cpm_ptr->cmm[i].score == top) {
/*		if (cpm_ptr->cmm[i].score == top && 
		    (cflag == 0 || CheckCfClosest(&(cpm_ptr->cmm[i]), closest) == TRUE)) { */
		    cpm_ptr->tie_num++;
		}
		else {
		    break;
		}
	    }
	}

	/* �Ȥꤢ��������
	   closest > -1: decided ������ */
	cpm_ptr->score = (int)cpm_ptr->cmm[0].score;
    }

    /* ʸ̮����: ľ�������ǤΥ����������Ͱʾ�ʤ�ʥե졼������ */
    if (decide) {
	if (OptEllipsis) {
	    if (closest > -1 && cpm_ptr->score > CF_DECIDE_THRESHOLD) {
		if (cpm_ptr->tie_num > 1) {
		    cpm_ptr->decided = CF_CAND_DECIDED;
		}
		else {
		    cpm_ptr->decided = CF_DECIDED;
		    /* exact match ���ơ��ǹ����γʥե졼�ब�ҤȤĤʤ顢���������ɽ�� */
		    if (cpm_ptr->score == EX_match_exact) {
			cpm_ptr->result_num = 1;
		    }
		}
	    }
	    else if (closest == -1 && cpm_ptr->cf.element_num > 0 && 
		     check_feature(cpm_ptr->pred_b_ptr->f, "�Ѹ�:��")) {
		cpm_ptr->decided = CF_DECIDED;
	    }
	}
	else if (closest > -1) {
	    cpm_ptr->decided = CF_DECIDED;
	}
    }

    if (cf_ptr->element_num != 0) {
	/* ľ���ʤ�����Ȥ���ľ���ʤΥ�����������θ����Ƥ��ʤ��Τǡ�
	   ���٤ƤγʤΥ�������­����������������Τˤ��� */
	if (closest > -1) {
	    for (i = 0; i < cpm_ptr->result_num; i++) {
		/* ������Ƽ��ԤΤȤ�(score==-1)�ϡ�pure_score ���������Ƥ��ʤ� */
		/* ����¦��Ǥ�ճʤ����ʤ�������Ƥ��ʤ��Ȥ�(score==0)�ϡ�ʬ��ʬ��Ȥ��0�ˤʤ� */
		pat_num = count_pat_element(cpm_ptr->cmm[i].cf_ptr, &(cpm_ptr->cmm[i].result_lists_p[0]));
		if (cpm_ptr->cmm[i].score < 0 || pat_num == 0) {
		    break;
		}
		cpm_ptr->cmm[i].score = (OptCaseFlag & OPT_CASE_USE_PROBABILITY) ? cpm_ptr->cmm[i].pure_score[0] : (cpm_ptr->cmm[i].pure_score[0] / sqrt((double)pat_num));
	    }
	    /* ľ���ʥ�������Ʊ���γʥե졼��򡢤��٤ƤΥ�������sort */
	    for (i = cpm_ptr->tie_num - 1; i >= 1; i--) {
		for (j = i - 1; j >= 0; j--) {
		    if (cpm_ptr->cmm[i].score > cpm_ptr->cmm[j].score) {
			tempcmm = cpm_ptr->cmm[i];
			cpm_ptr->cmm[i] = cpm_ptr->cmm[j];
			cpm_ptr->cmm[j] = tempcmm;
		    }
		}
	    }
	}
	cpm_ptr->score = cpm_ptr->cmm[0].score;
    }

    if (OptDisplay == OPT_DEBUG) {
	print_data_cframe(cpm_ptr, Cf_match_mgr);
	/* print_good_crrspnds(cpm_ptr, Cf_match_mgr, frame_num); */
	for (i = 0; i < cpm_ptr->result_num; i++) {
	    print_crrspnd(cpm_ptr, &cpm_ptr->cmm[i]);
	}
    }

    return cpm_ptr->score;
}

/*==================================================================*/
int get_closest_case_component(SENTENCE_DATA *sp, CF_PRED_MGR *cpm_ptr)
/*==================================================================*/
{
    /* �Ѹ�������ˤ�������Ǥ����
       ��äȤ��Ѹ��˶ᤤ��Τ�õ��
       (����ʸ��Ͻ���: num == -1) 
       �оݳ�: ���, �˳� */

    int i, min = -1, elem_b_num;

    /* ľ�������Ǥ����� */
    for (i = 0; i < cpm_ptr->cf.element_num; i++) {
	/* ʣ��̾��ΰ���: ľ���Ȥ��ʤ� */
	if (cpm_ptr->elem_b_ptr[i]->inum > 0) {
	    return -1;
	}
	/* �֡�����ˡ� */
	else if (cpm_ptr->pred_b_ptr->num == cpm_ptr->elem_b_ptr[i]->num) {
	    return i;
	}
	/* �Ѹ��ˤ�äȤ�ᤤ�����Ǥ�õ�� 
	   <���>:̵�� �ʳ� */
	else if (cpm_ptr->elem_b_num[i] > -2 && /* ��ά�γ����Ǥ���ʤ� */
		 cpm_ptr->elem_b_ptr[i]->num <= cpm_ptr->pred_b_ptr->num && 
		 min < cpm_ptr->elem_b_ptr[i]->num && 
		 !(MatchPP(cpm_ptr->cf.pp[i][0], "��") && 
		   check_feature(cpm_ptr->elem_b_ptr[i]->f, "���"))) {
	    min = cpm_ptr->elem_b_ptr[i]->num;
	    elem_b_num = i;
	}
    }

    /* 1. ���, �˳ʤǤ���Ȥ�
       2. <����>�˥ޥå����ʤ� 1, 2 �ʳ��γ� (MatchPP(cpm_ptr->cf.pp[elem_b_num][0], "��"))
       3. �Ѹ���ľ����̤�� (���줬�Ϥ��ޤäƤ�褤)
       �����ƻ�, Ƚ����?
       check_feature ���Ƥ�褤
       ����ѻ�: cpm_ptr->cf.pp[elem_b_num][1] == END_M */
    if (min != -1) {
	/* ���ꤷ�ʤ�:
	   1. �Ƕ�����Ǥ��ؼ���ξ�� ���ʤ����ޥå�������?
	   2. ���ʤǰ�̣�Ǥ��ʤ��Ȥ� */
	if (check_feature((sp->tag_data+min)->f, "�ؼ���") || 
	    (Thesaurus == USE_NTT && 
	     (sp->tag_data+min)->SM_code[0] == '\0' && 
	     MatchPP(cpm_ptr->cf.pp[elem_b_num][0], "��"))) {
	    return -2;
	}
	else if ((cpm_ptr->cf.pp[elem_b_num][0] == -1 && /* ̤�� */
		  (cpm_ptr->pred_b_ptr->num == min + 1 || 
		   (cpm_ptr->pred_b_ptr->num == min + 2 && /* ���줬�Ϥ��ޤäƤ����� */
		    (check_feature((sp->tag_data + min + 1)->f, "����") || 
		     check_feature((sp->tag_data + min + 1)->f, "��:Ϣ��"))))) || 
		 MatchPP(cpm_ptr->cf.pp[elem_b_num][0], "��") || 
		 MatchPP(cpm_ptr->cf.pp[elem_b_num][0], "��") || 
		 (((cpm_ptr->cf.pp[elem_b_num][0] > 0 && 
		    cpm_ptr->cf.pp[elem_b_num][0] < 9) || /* ���ܳ� */
		   MatchPP(cpm_ptr->cf.pp[elem_b_num][0], "�ޥ�")) && 
		   !cf_match_element(cpm_ptr->cf.sm[elem_b_num], "����", FALSE))) {
	    cpm_ptr->cf.adjacent[elem_b_num] = TRUE;	/* ľ���ʤΥޡ��� */
	    return elem_b_num;
	}
    }
    return -1;
}

/*==================================================================*/
       static int number_compare(const void *i, const void *j)
/*==================================================================*/
{
    /* sort function */
    return *(const int *)i-*(const int *)j;
}

/*==================================================================*/
	       char *inputcc2num(CF_PRED_MGR *cpm_ptr)
/*==================================================================*/
{
    int i, numbers[CF_ELEMENT_MAX];
    char str[70], token[3], *key;

    for (i = 0; i < cpm_ptr->cf.element_num; i++) {
	numbers[i] = cpm_ptr->elem_b_ptr[i]->num;
    }

    qsort(numbers, cpm_ptr->cf.element_num, sizeof(int), number_compare);

    str[0] = '\0';
    for (i = 0; i < cpm_ptr->cf.element_num; i++) {
	if (i) {
	    sprintf(token, " %d", numbers[i]);
	}
	else {
	    sprintf(token, "%d", numbers[i]);
	}
	strcat(str, token);
    }
    sprintf(token, " %d", cpm_ptr->pred_b_ptr->num);
    strcat(str, token);

    key = strdup(str);
    return key;
}

typedef struct cpm_cache {
    char *key;
    CF_PRED_MGR *cpm;
    struct cpm_cache *next;
} CPM_CACHE;

CPM_CACHE *CPMcache[TBLSIZE];

/*==================================================================*/
			 void InitCPMcache()
/*==================================================================*/
{
    memset(CPMcache, 0, sizeof(CPM_CACHE *)*TBLSIZE);
}

/*==================================================================*/
			 void ClearCPMcache()
/*==================================================================*/
{
    int i;
    CPM_CACHE *ccp, *next;

    for (i = 0; i < TBLSIZE; i++) {
	if (CPMcache[i]) {
	    ccp = CPMcache[i];
	    while (ccp) {
		free(ccp->key);
		clear_case_frame(&(ccp->cpm->cf));
		free(ccp->cpm);
		next = ccp->next;
		free(ccp);
		ccp = next;
	    }
	    CPMcache[i] = NULL;
	}
    }
}

/*==================================================================*/
		void RegisterCPM(CF_PRED_MGR *cpm_ptr)
/*==================================================================*/
{
    int num;
    char *key;
    CPM_CACHE **ccpp;

    key = inputcc2num(cpm_ptr);
    if (key == NULL) {
	return;
    }
    num = hash(key, strlen(key));

    ccpp = &(CPMcache[num]);
    while (*ccpp) {
	ccpp = &((*ccpp)->next);
    }

    *ccpp = (CPM_CACHE *)malloc_data(sizeof(CPM_CACHE), "RegisterCPM");
    (*ccpp)->key = key;
    (*ccpp)->cpm = (CF_PRED_MGR *)malloc_data(sizeof(CF_PRED_MGR), "RegisterCPM");
    init_case_frame(&((*ccpp)->cpm->cf));
    copy_cpm((*ccpp)->cpm, cpm_ptr, 0);
    (*ccpp)->next = NULL;
}

/*==================================================================*/
	     CF_PRED_MGR *CheckCPM(CF_PRED_MGR *cpm_ptr)
/*==================================================================*/
{
    int num;
    char *key;
    CPM_CACHE *ccp;

    key = inputcc2num(cpm_ptr);
    if (key == NULL) {
	return NULL;
    }
    num = hash(key, strlen(key));

    ccp = CPMcache[num];
    while (ccp) {
	if (str_eq(key, ccp->key)) {
	    return ccp->cpm;
	}
	ccp = ccp->next;
    }
    return NULL;
}

/*==================================================================*/
double case_analysis(SENTENCE_DATA *sp, CF_PRED_MGR *cpm_ptr, TAG_DATA *t_ptr)
/*==================================================================*/
{
    /*
                                              ����
      ���Ϥγ����Ǥ��ʤ����                    -3
      �ʥե졼�ब�ʤ����                      -2
      ����¦��ɬ�ܳʤ��Ĥ���(����������)      -1
      ��������                               score (0�ʾ�)
    */

    int closest;
    CF_PRED_MGR *cache_ptr;

    /* ����� */
    cpm_ptr->pred_b_ptr = t_ptr;
    cpm_ptr->score = -1;
    cpm_ptr->result_num = 0;
    cpm_ptr->tie_num = 0;
    cpm_ptr->cmm[0].cf_ptr = NULL;
    cpm_ptr->decided = CF_UNDECIDED;

    /* ����ʸ¦�γ��������� */
    set_data_cf_type(cpm_ptr);
    closest = make_data_cframe(sp, cpm_ptr);

    /* �ʥե졼����ϥ����å�
    if (cpm_ptr->cf.element_num == 0) {
	cpm_ptr->cmm[0].cf_ptr = NULL;
	return -3;
    }
    */

    /* cache */
    if (OptAnalysis == OPT_CASE && 
	(cache_ptr = CheckCPM(cpm_ptr))) {
	copy_cpm(cpm_ptr, cache_ptr, 0);
	return cpm_ptr->score;
    }

    /* ��äȤ⥹�����Τ褤�ʥե졼�����ꤹ��
       ʸ̮����: ľ�������Ǥ��ʤ���гʥե졼�����ꤷ�ʤ� */

    /* ľ�������Ǥ������� (closest > -1) �ΤȤ��ϳʥե졼�����ꤹ�� */
    find_best_cf(sp, cpm_ptr, closest, 1);

    if (OptAnalysis == OPT_CASE) {
	RegisterCPM(cpm_ptr);
    }

    return cpm_ptr->score;
}

/*==================================================================*/
int all_case_analysis(SENTENCE_DATA *sp, TAG_DATA *t_ptr, TOTAL_MGR *t_mgr)
/*==================================================================*/
{
    CF_PRED_MGR *cpm_ptr;
    int i;
    double one_case_point;

    /* �ʥե졼���̵ͭ������å�: set_pred_caseframe()�ξ��˽��� */
    if (t_ptr->para_top_p != TRUE && 
	t_ptr->cf_num > 0) {

	if (t_mgr->pred_num >= CPM_MAX) {
	    fprintf(stderr, ";; too many predicates in a sentence. (> %d)\n", CPM_MAX);
	    exit(1);
	}

	cpm_ptr = &(t_mgr->cpm[t_mgr->pred_num]);

	one_case_point = case_analysis(sp, cpm_ptr, t_ptr);

	/* ����������(����¦��ɬ�ܳʤ��Ĥ�)���ˤ��ΰ�¸��¤�β��Ϥ�
	   ������
	if (one_case_point == -1) return FALSE;
	*/

	t_mgr->score += one_case_point;
	t_mgr->pred_num++;
    }

    for (i = 0; t_ptr->child[i]; i++) {
	if (all_case_analysis(sp, t_ptr->child[i], t_mgr) == FALSE) {
	    return FALSE;
	}
    }

    return TRUE;
}

/*==================================================================*/
      void copy_cf_with_alloc(CASE_FRAME *dst, CASE_FRAME *src)
/*==================================================================*/
{
    int i, j;

    dst->type = src->type;
    dst->element_num = src->element_num;
    for (i = 0; i < src->element_num; i++) {
	dst->oblig[i] = src->oblig[i];
	dst->adjacent[i] = src->adjacent[i];
	for (j = 0; j < PP_ELEMENT_MAX; j++) {
	    dst->pp[i][j] = src->pp[i][j];
	}
	if (src->pp_str[i]) {
	    dst->pp_str[i] = strdup(src->pp_str[i]);
	}
	else {
	    dst->pp_str[i] = NULL;
	}
	if (src->sm[i]) {
	    dst->sm[i] = strdup(src->sm[i]);
	}
	else {
	    dst->sm[i] = NULL;
	}
	if (src->ex[i]) {
	    dst->ex[i] = strdup(src->ex[i]);
	}
	else {
	    dst->ex[i] = NULL;
	}
	if (src->ex_list[i]) {
	    dst->ex_list[i] = (char **)malloc_data(sizeof(char *)*src->ex_size[i], 
						   "copy_cf_with_alloc");
	    dst->ex_freq[i] = (int *)malloc_data(sizeof(int)*src->ex_size[i], 
						 "copy_cf_with_alloc");
	    for (j = 0; j < src->ex_num[i]; j++) {
		dst->ex_list[i][j] = strdup(src->ex_list[i][j]);
		dst->ex_freq[i][j] = src->ex_freq[i][j];
	    }
	}
	else {
	    dst->ex_list[i] = NULL;
	    dst->ex_freq[i] = NULL;
	}
	dst->ex_size[i] = src->ex_size[i];
	dst->ex_num[i] = src->ex_num[i];
	if (src->semantics[i]) {
	    dst->semantics[i] = strdup(src->semantics[i]);
	}
	else {
	    dst->semantics[i] = NULL;
	}
    }
    dst->voice = src->voice;
    dst->cf_address = src->cf_address;
    dst->cf_size = src->cf_size;
    strcpy(dst->cf_id, src->cf_id);
    strcpy(dst->pred_type, src->pred_type);
    strcpy(dst->imi, src->imi);
    dst->etcflag = src->etcflag;
    if (src->feature) {
	dst->feature = strdup(src->feature);
    }
    else {
	dst->feature = NULL;
    }
    if (src->entry) {
	dst->entry = strdup(src->entry);
    }
    else {
	dst->entry = NULL;
    }
    for (i = 0; i < CF_ELEMENT_MAX; i++) {
	dst->samecase[i][0] = src->samecase[i][0];
	dst->samecase[i][1] = src->samecase[i][1];
    }
    dst->cf_similarity = src->cf_similarity;
    /* weight, pred_b_ptr ��̤���� */
}

/*==================================================================*/
	    void copy_cf(CASE_FRAME *dst, CASE_FRAME *src)
/*==================================================================*/
{
    int i, j;

    dst->type = src->type;
    dst->type_flag = src->type_flag;
    dst->element_num = src->element_num;
/*    for (i = 0; i < CF_ELEMENT_MAX; i++) { */
    for (i = 0; i < src->element_num; i++) {
	dst->oblig[i] = src->oblig[i];
	dst->adjacent[i] = src->adjacent[i];
	for (j = 0; j < PP_ELEMENT_MAX; j++) {
	    dst->pp[i][j] = src->pp[i][j];
	}
	dst->pp_str[i] = src->pp_str[i];	/* �����Ȥ�������ꤢ�� */
	/* for (j = 0; j < SM_ELEMENT_MAX*SM_CODE_SIZE; j++) {
	    dst->sm[i][j] = src->sm[i][j];
	} */
	if (src->sm[i]) strcpy(dst->sm[i], src->sm[i]);
	if (src->ex[i]) strcpy(dst->ex[i], src->ex[i]);
	strcpy(dst->ex_list[i][0], src->ex_list[i][0]);
	for (j = 0; j < src->ex_num[i]; j++) {
	    dst->ex_freq[i][j] = src->ex_freq[i][j];
	}
	dst->ex_size[i] = src->ex_size[i];
	dst->ex_num[i] = src->ex_num[i];
    }
    dst->voice = src->voice;
    dst->cf_address = src->cf_address;
    dst->cf_size = src->cf_size;
    strcpy(dst->cf_id, src->cf_id);
    strcpy(dst->pred_type, src->pred_type);
    strcpy(dst->imi, src->imi);
    dst->etcflag = src->etcflag;
    dst->feature = src->feature;
    dst->entry = src->entry;
    for (i = 0; i < CF_ELEMENT_MAX; i++) {
	dst->samecase[i][0] = src->samecase[i][0];
	dst->samecase[i][1] = src->samecase[i][1];
    }
    dst->pred_b_ptr = src->pred_b_ptr;
    dst->cf_similarity = src->cf_similarity;
}

/*==================================================================*/
     void copy_cpm(CF_PRED_MGR *dst, CF_PRED_MGR *src, int flag)
/*==================================================================*/
{
    int i;

    if (flag) {
	copy_cf_with_alloc(&dst->cf, &src->cf);
    }
    else {
	copy_cf(&dst->cf, &src->cf);
    }
    dst->pred_b_ptr = src->pred_b_ptr;
    for (i = 0; i < CF_ELEMENT_MAX; i++) {
	dst->elem_b_ptr[i] = src->elem_b_ptr[i];
	dst->elem_b_num[i] = src->elem_b_num[i];
	dst->elem_s_ptr[i] = src->elem_s_ptr[i];
    }
    dst->score = src->score;
    dst->result_num = src->result_num;
    dst->tie_num = src->tie_num;
    for (i = 0; i < CMM_MAX; i++) {
	dst->cmm[i] = src->cmm[i];
    }
    dst->decided = src->decided;
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
	copy_cpm(&dst->cpm[i], &src->cpm[i], 0);
    }
    dst->ID = src->ID;
}

/*==================================================================*/
	void call_case_analysis(SENTENCE_DATA *sp, DPND dpnd)
/*==================================================================*/
{
    int i, j, k;
    int one_topic_score, topic_score, topic_score_sum = 0, topic_slot[2], distance_cost = 0;
    char *cp;

    /* �ʹ�¤���ϤΥᥤ��ؿ� */

    /* ��¸��¤�ں��� */

    dpnd_info_to_bnst(sp, &dpnd);
    dpnd_info_to_tag(sp, &dpnd);
    make_dpnd_tree(sp);
    bnst_to_tag_tree(sp);
	
    if (OptDisplay == OPT_DEBUG)
	print_kakari(sp, OPT_TREE);

    /* �ʲ��Ϻ���ΰ�ν���� */
	
    Work_mgr.pssb = Possibility;
    Work_mgr.dpnd = dpnd;
    Work_mgr.score = 0;
    Work_mgr.pred_num = 0;
    Work_mgr.dflt = 0;
    for (i = 0; i < sp->Bnst_num; i++)
	Work_mgr.dflt += dpnd.dflt[i];
    
    /* �ʲ��ϸƤӽФ� */

    if (all_case_analysis(sp, sp->tag_data + sp->Tag_num - 1, &Work_mgr) == TRUE)
	Possibility++;
    else
	return;

    /* ������ default �Ȥε�Υ�Τ���, �������� */

    for (i = 0; i < sp->Bnst_num - 1; i++) {
	/* ���� -> ��٥�:A (�롼��Ǥ��η����������������ϡ�
	   �����ǥ����Ȥ�Ϳ����) */
	if (!(OptCaseFlag & OPT_CASE_USE_PROBABILITY) && 
	    check_feature((sp->bnst_data + i)->f, "��:����") && 
	    check_feature((sp->bnst_data + dpnd.head[i])->f, "��٥�:A")) {
	    distance_cost += LEVELA_COST;
	}

	if (dpnd.dflt[i] > 0) {
	    /* ���� */
	    if (!(OptCaseFlag & OPT_CASE_USE_PROBABILITY) && 
		check_feature((sp->bnst_data + i)->f, "����")) {
		distance_cost += dpnd.dflt[i];

		/* ����ˤĤ��Ʊ󤯤˷��äƤ��ޤä�ʸ��ε�Υ������ */
		for (j = 0; j < i - 1; j++) {
		    if (dpnd.head[i] == dpnd.head[j]) {
			for (k = j + 1; k < i; k++) {
			    if (Mask_matrix[j][k] && Quote_matrix[j][k] && Dpnd_matrix[j][k] && Dpnd_matrix[j][k] != 'd') {
				distance_cost += dpnd.dflt[i]*TEIDAI_STEP;
			    }
			}
		    }
		}
		continue;
	    }
	    /* ����ʳ� */
	    /* ����¦��Ϣ�ѤǤʤ��Ȥ� */
	    if (!(OptCaseFlag & OPT_CASE_USE_PROBABILITY) && 
		!check_feature((sp->bnst_data + i)->f, "��:Ϣ��")) {
		/* ��ʬ���������ʤ����٤ζ����Ѹ� (Ϣ�ΰʳ�) ��ۤ��Ƥ���Ȥ� */
		if (!check_feature((sp->bnst_data + i)->f, "����")) {
		    if (dpnd.head[i] > i + 1 && 
			subordinate_level_check("B", sp->bnst_data + i + 1) && 
			(cp = (char *)check_feature((sp->bnst_data + i + 1)->f, "��"))) {
			if (strcmp(cp+3, "Ϣ��") && strcmp(cp+3, "Ϣ��")) {
			    distance_cost += STRONG_V_COST;
			}
		    }
		}
		/* ��ʬ������������*/
		else {
		    /* �٤˷���Ȥ� */
		    if (dpnd.head[i] == i + 1) {
			distance_cost += ADJACENT_TOUTEN_COST;
		    }
		}
	    }

	    /* ��ΨŪ: ����ʤɤΥ����� (tentative) */
	    if (OptCaseFlag & OPT_CASE_USE_PROBABILITY) {
		if (check_feature((sp->bnst_data + i)->f, "��:Ϣ��") && 
		    !check_feature((sp->bnst_data + i)->f, "�Ѹ�")) {
		    distance_cost += dpnd.dflt[i]*DISTANCE_STEP;
		}
	    }
	    /* �ǥե���ȤȤκ� x 2 ���Υ�Υ����ȤȤ���
	       �����������ƻ�����Ϣ�ʤξ��� x 1 */
	    else {
		if (!check_feature((sp->bnst_data + i)->f, "��:Ϣ��") || 
		    check_feature((sp->bnst_data + i)->f, "�Ѹ�:��")) {
		    distance_cost += dpnd.dflt[i]*DISTANCE_STEP;
		}
		else {
		    distance_cost += dpnd.dflt[i]*RENKAKU_STEP;
		}
	    }
	}		    
    }

    Work_mgr.score -= distance_cost;

    if (!(OptCaseFlag & OPT_CASE_USE_PROBABILITY)) {
	for (i = sp->Bnst_num - 1; i > 0; i--) {
	    /* ʸ�������Ѹ����Ȥ������������� */
	    if ((cp = (char *)check_feature((sp->bnst_data + i)->f, "�����"))) {

		/* topic_slot[0]	���ְʳ��ΥϳʤΥ���å�
		   topic_slot[1]	��<<����>>�ϡפΥ���å�
		   ξ���Ȥ� 1 �ʲ��������Ĥ��ʤ�
		*/

		topic_slot[0] = 0;
		topic_slot[1] = 0;
		one_topic_score = 0;

		/* ����¦��õ�� */
		for (j = i - 1; j >= 0; j--) {
		    if (dpnd.head[j] != i) {
			continue;
		    }
		    if (check_feature((sp->bnst_data + j)->f, "����")) {
			if (check_feature((sp->bnst_data + j)->f, "����")) {
			    topic_slot[1]++;
			}
			else {
			    topic_slot[0]++;
			}
			sscanf(cp, "%*[^:]:%d", &topic_score);
			one_topic_score += topic_score;
		    }
		}

		if (topic_slot[0] > 0 || topic_slot[1] > 0) {
		    one_topic_score += 20;
		}
		Work_mgr.score += one_topic_score;
		if (OptDisplay == OPT_DEBUG) {
		    topic_score_sum += one_topic_score;
		}
	    }
	}
    }

    if (OptDisplay == OPT_DEBUG) {
	if (OptCaseFlag & OPT_CASE_USE_PROBABILITY) {
	    fprintf(stdout, "�� %d�� (��Υ���� %d��)\n", 
		    Work_mgr.score, distance_cost);
	}
	else {
	    fprintf(stdout, "�� %d�� (��Υ���� %d�� (%d��) ���ꥹ���� %d��)\n", 
		    Work_mgr.score, distance_cost, Work_mgr.dflt*2, topic_score_sum);
	}
    }
        
    /* ����� */

    if (Work_mgr.score > sp->Best_mgr->score ||
	(Work_mgr.score == sp->Best_mgr->score && 
	 compare_dpnd(sp, &Work_mgr, sp->Best_mgr) == TRUE))
	copy_mgr(sp->Best_mgr, &Work_mgr);
}

/*==================================================================*/
      int add_cf_slot(CF_PRED_MGR *cpm_ptr, char *cstr, int num)
/*==================================================================*/
{
    if (cpm_ptr->cmm[0].cf_ptr->element_num >= CF_ELEMENT_MAX) {
	return FALSE;
    }

    _make_ipal_cframe_pp(cpm_ptr->cmm[0].cf_ptr, cstr, cpm_ptr->cmm[0].cf_ptr->element_num, CF_PRED);
    cpm_ptr->cmm[0].result_lists_d[0].flag[num] = cpm_ptr->cmm[0].cf_ptr->element_num;
    cpm_ptr->cmm[0].result_lists_d[0].score[num] = 0;
    cpm_ptr->cmm[0].result_lists_p[0].flag[cpm_ptr->cmm[0].cf_ptr->element_num] = num;
    cpm_ptr->cmm[0].result_lists_p[0].score[cpm_ptr->cmm[0].cf_ptr->element_num] = 0;
    cpm_ptr->cmm[0].cf_ptr->element_num++;

    return TRUE;
}

/*==================================================================*/
    int assign_cf_slot(CF_PRED_MGR *cpm_ptr, int cnum, int num)
/*==================================================================*/
{
    /* �ʥե졼��Τ��γʤˤ��Ǥ��б��դ�������� */
    if (cpm_ptr->cmm[0].result_lists_p[0].flag[cnum] != UNASSIGNED) {
	return FALSE;
    }

    cpm_ptr->cmm[0].result_lists_d[0].flag[num] = cnum;
    cpm_ptr->cmm[0].result_lists_d[0].score[num] = 0;
    cpm_ptr->cmm[0].result_lists_p[0].flag[cnum] = num;
    cpm_ptr->cmm[0].result_lists_p[0].score[cnum] = 0;

    return TRUE;
}

/*==================================================================*/
		int check_ga2_ok(CF_PRED_MGR *cpm_ptr)
/*==================================================================*/
{
    int i;
    for (i = 0; i < cpm_ptr->cmm[0].cf_ptr->element_num; i++) {
	/* ������Ƥʤ��Υ���, ���, �˳ʤ�¸�ߤ���ʤ�С������Բ� */
	if (cpm_ptr->cmm[0].result_lists_p[0].flag[i] == UNASSIGNED && 
	    (MatchPP(cpm_ptr->cmm[0].cf_ptr->pp[i][0], "��") || 
	     MatchPP(cpm_ptr->cmm[0].cf_ptr->pp[i][0], "��") || 
	     MatchPP(cpm_ptr->cmm[0].cf_ptr->pp[i][0], "��"))) {
	    return 0;
	}
    }
    return 1;
}

/*==================================================================*/
      void decide_voice(SENTENCE_DATA *sp, CF_PRED_MGR *cpm_ptr)
/*==================================================================*/
{
    TAG_DATA *check_b_ptr;

    if (cpm_ptr->cmm[0].cf_ptr->voice == FRAME_ACTIVE) {
	cpm_ptr->pred_b_ptr->voice = 0;
    }
    else {
	cpm_ptr->pred_b_ptr->voice = VOICE_UKEMI;
    }

    /* �ʤ��ʤ�ʤ��褦�� */
    check_b_ptr = cpm_ptr->pred_b_ptr;
    while (check_b_ptr->parent && check_b_ptr->parent->para_top_p == TRUE) {
	check_b_ptr->parent->voice = cpm_ptr->pred_b_ptr->voice;
	check_b_ptr = check_b_ptr->parent;
    }
}

/*==================================================================*/
	   char *make_print_string(TAG_DATA *bp, int flag)
/*==================================================================*/
{
    int i, start = 0, end = 0, length = 0;
    char *ret;

    /*
       flag == 1: ��Ω����
       flag == 0: �Ǹ�μ�Ω��
    */

    if (flag) {
	/* ��Ƭ��ߤ� */
	for (i = 0; i < bp->mrph_num; i++) {
	    /* ��°���ü����� */
	    if (strcmp(Class[(bp->mrph_ptr + i)->Hinshi][0].id, "�ü�") || 
		check_feature((bp->mrph_ptr + i)->f, "��Ω")) {
		start = i;
		break;
	    }
	}

	/* ������ߤ� */
	for (i = bp->mrph_num - 1; i >= start; i--) {
	    /* �ü�, ����, ��ư��, Ƚ������� */
	    if ((strcmp(Class[(bp->mrph_ptr + i)->Hinshi][0].id, "�ü�") || 
		 check_feature((bp->mrph_ptr + i)->f, "��Ω")) && 
		strcmp(Class[(bp->mrph_ptr + i)->Hinshi][0].id, "����") && 
		strcmp(Class[(bp->mrph_ptr + i)->Hinshi][0].id, "��ư��") && 
		strcmp(Class[(bp->mrph_ptr + i)->Hinshi][0].id, "Ƚ���")) {
		end = i;
		break;
	    }
	}

	if (start > end) {
	    start = bp->jiritu_ptr-bp->mrph_ptr;
	    end = bp->settou_num+bp->jiritu_num - 1;
	}

	for (i = start; i <= end; i++) {
	    length += strlen((bp->mrph_ptr + i)->Goi2);
	}
	if (length == 0) {
	    return NULL;
	}
	ret = (char *)malloc_data(length + 1, "make_print_string");
	*ret = '\0';
	for (i = start; i <= end; i++) {
	    strcat(ret, (bp->mrph_ptr + i)->Goi2);
	}
    }
    else {
	ret = strdup(bp->head_ptr->Goi2);
    }
    return ret;
}

/*==================================================================*/
    void record_match_ex(SENTENCE_DATA *sp, CF_PRED_MGR *cpm_ptr)
/*==================================================================*/
{
    int i, num, pos;
    char feature_buffer[DATA_LEN];

    for (i = 0; i < cpm_ptr->cf.element_num; i++) {
	num = cpm_ptr->cmm[0].result_lists_d[0].flag[i];
	if (num != NIL_ASSIGNED && /* ������Ƥ����� */
	    cpm_ptr->elem_b_ptr[i] && 
	    cpm_ptr->elem_b_num[i] < 0) { /* ��ά, ����, Ϣ�ν��� */
	    pos = cpm_ptr->cmm[0].result_lists_p[0].pos[num];
	    if (pos == MATCH_NONE || pos == MATCH_SUBJECT) {
		sprintf(feature_buffer, "�ޥå�����;%s:%s-%s", 
			pp_code_to_kstr_in_context(cpm_ptr, cpm_ptr->cmm[0].cf_ptr->pp[num][0]), 
			cpm_ptr->elem_b_ptr[i]->head_ptr->Goi, 
			pos == MATCH_NONE ? "NONE" : "SUBJECT");
	    }
	    else {
		sprintf(feature_buffer, "�ޥå�����;%s:%s-%s:%d", 
			pp_code_to_kstr_in_context(cpm_ptr, cpm_ptr->cmm[0].cf_ptr->pp[num][0]), 
			cpm_ptr->elem_b_ptr[i]->head_ptr->Goi, 
			cpm_ptr->cmm[0].cf_ptr->ex_list[num][pos], 
			cpm_ptr->cmm[0].result_lists_p[0].score[num]);
	    }
	    assign_cfeature(&(cpm_ptr->pred_b_ptr->f), feature_buffer);
	}
    }
}

/*==================================================================*/
  void after_case_analysis(SENTENCE_DATA *sp, CF_PRED_MGR *cpm_ptr)
/*==================================================================*/
{
    int i, c;

    if (cpm_ptr->score < 0) {
	return;
    }

    /* ̤�б��γ����Ǥν��� */

    for (i = 0; i < cpm_ptr->cf.element_num; i++) {
	if (cpm_ptr->cmm[0].result_lists_d[0].flag[i] == NIL_ASSIGNED) {
	    /* ̤��, Ϣ�� */
	    if (cpm_ptr->elem_b_num[i] == -1) {
		/* <����> => ���� */
		if (check_feature(cpm_ptr->elem_b_ptr[i]->f, "����")) {
		    if (check_cf_case(cpm_ptr->cmm[0].cf_ptr, "����") < 0) {
			add_cf_slot(cpm_ptr, "����", i);
		    }
		}
		/* ��ż�칽ʸ�γ��Υ��� */
		else if (cpm_ptr->elem_b_ptr[i]->num < cpm_ptr->pred_b_ptr->num && 
			 check_feature(cpm_ptr->elem_b_ptr[i]->f, "��:̤��") && 
			 cpm_ptr->pred_b_ptr->num != cpm_ptr->elem_b_ptr[i]->num+1 && /* �Ѹ���ľ���ǤϤʤ� (�¤ϡ��⤦�ҤȤĤΥ��ʤ������ˤ��뤳�Ȥ���ˤ�����) */
			 check_ga2_ok(cpm_ptr)) {
		    if (check_cf_case(cpm_ptr->cmm[0].cf_ptr, "����") < 0) {
			add_cf_slot(cpm_ptr, "����", i);
		    }
		}
		/* ����¾ => ���δط�
		   ʣ��̾�����¦: ��α
		   �Ѹ�ľ���Υγ�: ��α */
		else if (cpm_ptr->cf.type != CF_NOUN && 
			 !(cpm_ptr->elem_b_ptr[i]->inum > 0 && 
			   cpm_ptr->elem_b_ptr[i]->parent == cpm_ptr->pred_b_ptr) && 
			 cpm_ptr->cf.pp[i][0] != pp_kstr_to_code("̤") && 
			 MatchPP2(cpm_ptr->cf.pp[i], "���δط�")) { /* �ֳ��δط��פβ�ǽ�������� */
		    if ((c = check_cf_case(cpm_ptr->cmm[0].cf_ptr, "���δط�")) < 0) {
			add_cf_slot(cpm_ptr, "���δط�", i);
		    }
		    else {
			assign_cf_slot(cpm_ptr, c, i);
		    }
		}
	    }
	    /* �ʤ���������Ƥ��뤬���ʥե졼��¦�ˤ��γʤ��ʤ��ä���� */
	    /* �� �Ȥꤦ��ʤ�ʣ������Ȥ�: �س� */
	    else {
		if (check_cf_case(cpm_ptr->cmm[0].cf_ptr, pp_code_to_kstr(cpm_ptr->cf.pp[i][0])) < 0) {
		    add_cf_slot(cpm_ptr, pp_code_to_kstr(cpm_ptr->cf.pp[i][0]), i);
		}
	    }
	}
    }
}

/*==================================================================*/
 char *make_cc_string(char *word, int tag_n, char *pp_str, int cc_type,
		      int dist, char *sid)
/*==================================================================*/
{
    char *buf;

    buf = (char *)malloc_data(strlen(pp_str) + strlen(word) + strlen(sid) + (dist ? log(dist) : 0) + 11, 
			      "make_cc_string");

    sprintf(buf, "%s/%c/%s/%d/%d/%s", 
	    pp_str, 
	    cc_type == -2 ? 'O' : 	/* ��ά */
	    cc_type == -3 ? 'D' : 	/* �ȱ� */
	    cc_type == -1 ? 'N' : 'C', 
	    word, 
	    tag_n, 
	    dist, 
	    sid);
    return buf;
}

/*==================================================================*/
void record_case_analysis(SENTENCE_DATA *sp, CF_PRED_MGR *cpm_ptr, 
			  ELLIPSIS_MGR *em_ptr, int lastflag)
/*==================================================================*/
{
    int i, j, num, sent_n, tag_n, dist_n;
    char feature_buffer[DATA_LEN], relation[DATA_LEN], buffer[DATA_LEN], *word, *sid, *cp;
    ELLIPSIS_COMPONENT *ccp;

    /* voice ���� */
    if (cpm_ptr->pred_b_ptr->voice == VOICE_UNKNOWN) {
	decide_voice(sp, cpm_ptr);
    }

    /* �ֳʥե졼���Ѳ��ץե饰���Ĥ��Ƥ���ʥե졼�����Ѥ������ */
    if (cpm_ptr->cmm[0].cf_ptr->etcflag & CF_CHANGE) {
	assign_cfeature(&(cpm_ptr->pred_b_ptr->f), "�ʥե졼���Ѳ�");
    }

    /* ����¦�γƳ����Ǥε��� */
    for (i = 0; i < cpm_ptr->cf.element_num; i++) {
	/* ��ά���Ϥη�̤Ͻ���
	   �ؼ���β��Ϥ򤹤���ϡ��ؼ������� */
	if (cpm_ptr->elem_b_num[i] <= -2) {
	    continue;
	}

	num = cpm_ptr->cmm[0].result_lists_d[0].flag[i];

	/* ������Ƥʤ� */
	if (num == NIL_ASSIGNED) {
	    strcpy(relation, "--");
	}
	/* ������Ƥ��Ƥ���� */
	else if (num >= 0) {
	    /* �ʥե졼��˳�ꤢ�ƤƤ��륬���� */
	    if (MatchPP(cpm_ptr->cmm[0].cf_ptr->pp[num][0], "����")) {
		strcpy(relation, "����");
		/* sprintf(feature_buffer, "%sȽ��", relation);
		assign_cfeature(&(cpm_ptr->elem_b_ptr[i]->f), feature_buffer); */
	    }
	    else {
		strcpy(relation, 
		       pp_code_to_kstr_in_context(cpm_ptr, cpm_ptr->cmm[0].cf_ptr->pp[num][0]));
	    }
	}
	/* else: UNASSIGNED �Ϥʤ��Ϥ� */


	/* feature ���Ѹ�ʸ���Ϳ���� */
	word = make_print_string(cpm_ptr->elem_b_ptr[i], 0);
	if (word) {
	    if (cpm_ptr->elem_b_ptr[i]->num >= 0) {
		sprintf(feature_buffer, "�ʴط�%d:%s:%s", 
			cpm_ptr->elem_b_ptr[i]->num, 
			relation, word);
	    }
	    /* ʸ�����������Ǥξ�� */
	    else {
		sprintf(feature_buffer, "�ʴط�%d:%s:%s", 
			cpm_ptr->elem_b_ptr[i]->parent->num, 
			relation, word);
	    }
	    assign_cfeature(&(cpm_ptr->pred_b_ptr->f), feature_buffer);
	    free(word);
	}
    }

    /* �ʲ��Ϸ�� buffer������ */
    sprintf(feature_buffer, "�ʲ��Ϸ��:%s:", cpm_ptr->cmm[0].cf_ptr->cf_id);
    for (i = 0; i < cpm_ptr->cmm[0].cf_ptr->element_num; i++) {
	num = cpm_ptr->cmm[0].result_lists_p[0].flag[i];
	ccp = em_ptr ? CheckEllipsisComponent(&(em_ptr->cc[cpm_ptr->cmm[0].cf_ptr->pp[i][0]]), 
					      cpm_ptr->cmm[0].cf_ptr->pp_str[i]) : NULL;

	if (i != 0) {
	    strcat(feature_buffer, ";");
	}

	/* ������Ƥʤ� */
	if (num == UNASSIGNED) {
	    /* ������Ƥʤ� */
	    sprintf(buffer, "%s/U/-/-/-/-", 
		    pp_code_to_kstr_in_context(cpm_ptr, cpm_ptr->cmm[0].cf_ptr->pp[i][0]));
	    strcat(feature_buffer, buffer);
	}
	/* ������Ƥ��� */
	else {
	    /* �㳰���� */
	    if (ccp && ccp->bnst < 0) {
		sprintf(buffer, "%s/E/%s/-/-/-", 
			pp_code_to_kstr_in_context(cpm_ptr, cpm_ptr->cmm[0].cf_ptr->pp[i][0]), 
			ETAG_name[abs(ccp->bnst)]);
		strcat(feature_buffer, buffer);
	    }
	    else {
		/* ��ά�ξ�� (�ü쥿���ʳ�) */
		if (ccp && cpm_ptr->elem_b_num[num] <= -2) {
		    sid = ccp->s->KNPSID ? ccp->s->KNPSID + 5 : NULL;
		    dist_n = ccp->dist;
		    sent_n = ccp->s->Sen_num;
		}
		/* Ʊʸ�� */
		else {
		    sid = sp->KNPSID ? sp->KNPSID + 5 : NULL;
		    dist_n = 0;
		    sent_n = sp->Sen_num;
		}

		/* ����λҶ� 
		   ��ά��: OK 
		   ľ�ܤη��������: ̤����(elem_b_ptr�� para_top_p) */
		if (cpm_ptr->elem_b_ptr[num]->para_type == PARA_NORMAL && 
		    cpm_ptr->elem_b_ptr[num]->parent && 
		    cpm_ptr->elem_b_ptr[num]->parent->para_top_p) {
		    for (j = 0; cpm_ptr->elem_b_ptr[num]->parent->child[j]; j++) {
			if (cpm_ptr->elem_b_ptr[num] == cpm_ptr->elem_b_ptr[num]->parent->child[j] || /* target */
			    cpm_ptr->elem_b_ptr[num]->parent->child[j]->para_type != PARA_NORMAL || /* ����ǤϤʤ� */
			    (cpm_ptr->pred_b_ptr->num < cpm_ptr->elem_b_ptr[num]->num && /* Ϣ�ν����ξ��ϡ� */
			     (cpm_ptr->elem_b_ptr[num]->parent->child[j]->num < cpm_ptr->pred_b_ptr->num || /* �Ѹ�������Ϥ����ʤ� */
			      cpm_ptr->elem_b_ptr[num]->num < cpm_ptr->elem_b_ptr[num]->parent->child[j]->num))) { /* ����������λҤ����λҤ���Ϥ����ʤ� */
			    continue;
			}
			word = make_print_string(cpm_ptr->elem_b_ptr[num]->parent->child[j], 0);
			cp = make_cc_string(word ? word : "(null)", cpm_ptr->elem_b_ptr[num]->parent->child[j]->num, 
					    pp_code_to_kstr_in_context(cpm_ptr, cpm_ptr->cmm[0].cf_ptr->pp[i][0]), 
					    cpm_ptr->elem_b_num[num], dist_n, sid ? sid : "?");
			strcat(feature_buffer, cp);
			strcat(feature_buffer, ";");
			free(cp);
			if (word) free(word);
		    }
		}

		word = make_print_string(cpm_ptr->elem_b_ptr[num], 0);
		tag_n = cpm_ptr->elem_b_ptr[num]->num;
		cp = make_cc_string(word ? word : "(null)", tag_n, 
				    pp_code_to_kstr_in_context(cpm_ptr, cpm_ptr->cmm[0].cf_ptr->pp[i][0]), 
				    cpm_ptr->elem_b_num[num], dist_n, sid ? sid : "?");
		strcat(feature_buffer, cp);
		free(cp);

		/* �ʡ���ά�ط�����¸ (ʸ̮������) */
		if (OptEllipsis) {
		    RegisterTagTarget(cpm_ptr->pred_b_ptr->head_ptr->Goi, 
				      cpm_ptr->pred_b_ptr->voice, 
				      cpm_ptr->cmm[0].cf_ptr->cf_address, 
				      cpm_ptr->cmm[0].cf_ptr->pp[i][0], 
				      cpm_ptr->cmm[0].cf_ptr->type == CF_NOUN ? cpm_ptr->cmm[0].cf_ptr->pp_str[i] : NULL, 
				      word, sent_n, tag_n, CREL);
		}
		if (word) free(word);
	    }
	}
    }

    assign_cfeature(&(cpm_ptr->pred_b_ptr->f), feature_buffer);
}

/*==================================================================*/
		 char *get_mrph_rep(MRPH_DATA *m_ptr)
/*==================================================================*/
{
    char *cp;

    if (cp = strstr(m_ptr->Imi, "��ɽɽ��:")) {
	return cp + 9;
    }
    return NULL;
}

/*==================================================================*/
  void lexical_disambiguation_by_case_analysis(CF_PRED_MGR *cpm_ptr)
/*==================================================================*/
{
    /* �ʲ��Ϸ�̤�������Ǥ�ۣ������ä�Ԥ� */

    char *rep_cp = get_mrph_rep(cpm_ptr->pred_b_ptr->head_ptr);

    /* ���ߤη�������ɽɽ���ȳʥե졼���ɽ�����ۤʤ���Τ��ѹ� */
    if (rep_cp && 
	strncmp(rep_cp, cpm_ptr->cmm[0].cf_ptr->entry, strlen(cpm_ptr->cmm[0].cf_ptr->entry)) && 
	(check_feature(cpm_ptr->pred_b_ptr->head_ptr->f, "����ۣ��") || /* ������ۣ����Ѹ� */
	 (check_str_type(cpm_ptr->pred_b_ptr->head_ptr->Goi) == TYPE_HIRAGANA && 
	  check_feature(cpm_ptr->pred_b_ptr->head_ptr->f, "��ۣ")))) { /* ��ۣ�ʤҤ餬�� */
	FEATURE *fp;
	MRPH_DATA m;

	fp = cpm_ptr->pred_b_ptr->head_ptr->f;
	while (fp) {
	    if (!strncmp(fp->cp, "ALT-", 4)) {
		sscanf(fp->cp + 4, "%[^-]-%[^-]-%[^-]-%d-%d-%d-%d-%[^\n]", 
		       m.Goi2, m.Yomi, m.Goi, 
		       &m.Hinshi, &m.Bunrui, 
		       &m.Katuyou_Kata, &m.Katuyou_Kei, m.Imi);
		rep_cp = get_mrph_rep(&m);
		/* ���򤷤��ʥե졼���ɽ���Ȱ��פ�����ɽɽ�����ķ����Ǥ����� */
		if (rep_cp && 
		    !strncmp(rep_cp, cpm_ptr->cmm[0].cf_ptr->entry, 
			     strlen(cpm_ptr->cmm[0].cf_ptr->entry))) {
		    char *imip, *cp;

		    /* ���ߤη����Ǥ�ALT����¸ */
		    assign_feature_alt_mrph(&(cpm_ptr->pred_b_ptr->head_ptr->f), 
					    cpm_ptr->pred_b_ptr->head_ptr);

		    strcpy(cpm_ptr->pred_b_ptr->head_ptr->Goi, m.Goi);
		    strcpy(cpm_ptr->pred_b_ptr->head_ptr->Yomi, m.Yomi);
		    cpm_ptr->pred_b_ptr->head_ptr->Hinshi = m.Hinshi;
		    cpm_ptr->pred_b_ptr->head_ptr->Bunrui = m.Bunrui;
		    cpm_ptr->pred_b_ptr->head_ptr->Katuyou_Kata = m.Katuyou_Kata;
		    cpm_ptr->pred_b_ptr->head_ptr->Katuyou_Kei = m.Katuyou_Kei;
		    strcpy(cpm_ptr->pred_b_ptr->head_ptr->Imi, m.Imi);
		    delete_cfeature(&(cpm_ptr->pred_b_ptr->head_ptr->f), fp->cp);

		    /* ��̣�����feature�� */
		    if (m.Imi[0] == '\"') { /* �̾� "" �ǳ���Ƥ��� */
			imip = &m.Imi[1];
			if (cp = strchr(imip, '\"')) {
			    *cp = '\0';
			}
		    }
		    else {
			imip = m.Imi;
		    }
		    imi2feature(imip, cpm_ptr->pred_b_ptr->head_ptr);
		    assign_cfeature(&(cpm_ptr->pred_b_ptr->head_ptr->f), "������ۣ�������");
		    break;
		}
	    }
	    fp = fp->next;
	}
    }
}

/*==================================================================*/
       int get_dist_from_work_mgr(BNST_DATA *bp, BNST_DATA *hp)
/*==================================================================*/
{
    int i, dist = 0;

    /* ��������å� */
    if (Work_mgr.dpnd.check[bp->num].num == -1) {
	return -1;
    }
    for (i = 0; i < Work_mgr.dpnd.check[bp->num].num; i++) {
	if (Work_mgr.dpnd.check[bp->num].pos[i] == hp->num) {
	    dist = ++i;
	    break;
	}
    }
    if (dist == 0) {
	return -1;
    }
    else if (dist > 1) {
	dist = 2;
    }
    return dist;
}

/*====================================================================
                               END
====================================================================*/
