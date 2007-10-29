/*====================================================================

			  �ʹ�¤����: ����¦

                                               S.Kurohashi 93. 5.31

    $Id$
====================================================================*/
#include "knp.h"

int SOTO_SCORE = 7;

char fukugoji_string[SMALL_DATA_LEN];

char *FukugojiTable[] = {"�����", "��Τ���", 
			 "���̤���", "��Ĥ�����", 
			 "���̤���", "��Ĥ�����", 
			 "���̤�", "��Ĥ�����", 
			 "��ޤ��", "��դ����", 
			 "��Ϥ��", "��Ϥ����", 
			 "�����", "�ˤ����", 
			 "�˱褦", "�ˤ���", 
			 "�˸�����", "�ˤऱ��", 
			 "��ȼ��", "�ˤȤ�ʤ�", 
			 "�˴�Ť�", "�ˤ�ȤŤ�", 
			 "���Ф���", "�ˤ�������", 
			 "�˴ؤ���", "�ˤ��󤹤�", 
			 "������", "�ˤ����", 
			 "�˲ä���", "�ˤ��廊��", 
			 "�˸¤�", "�ˤ�����", 
			 "��³��", "�ˤĤŤ�", 
			 "�˹�碌��", "�ˤ��碌��", 
			 "����٤�", "�ˤ���٤�", 
			 "���¤�", "�ˤʤ��", 
			 "�˸¤��", "�ˤ������", 
			 ""};

/*==================================================================*/
	     char *make_fukugoji_string(TAG_DATA *b_ptr)
/*==================================================================*/
{
    int i, fc;
    char buf[SMALL_DATA_LEN];
    TAG_DATA *pre_b_ptr = b_ptr - 1;

    /* ��°�줬�ʤ��Ȥ� */
    if (b_ptr->num < 1 || pre_b_ptr->fuzoku_num == 0) {
	return NULL;
    }

    buf[0] = '\0';

    /* ����ʸ��ν��� */
    strcat(buf, (pre_b_ptr->fuzoku_ptr + pre_b_ptr->fuzoku_num - 1)->Goi);

    /* ����ʸ�� */
    for (i = 0; i < b_ptr->jiritu_num; i++) {
	if (!strcmp(Class[(b_ptr->jiritu_ptr + i)->Hinshi][0].id, "�ü�")) /* �ü�ʳ� */
	    continue;
	strcat(buf, 
	       (b_ptr->jiritu_ptr + i)->Goi);
    }

    /* �������ɤߤ����� */
    for (i = 0; *(FukugojiTable[i]); i += 2) {
	if (str_eq(buf, FukugojiTable[i])) {
	    strcpy(buf, FukugojiTable[i + 1]);
	    break;
	}
    }

    fc = pp_hstr_to_code(buf);
    if (fc != END_M) {
	sprintf(fukugoji_string, "�Բ��ϳ�-%s", pp_code_to_kstr(fc));
	return fukugoji_string;
    }
    return NULL;
}

/*==================================================================*/
int check_cc_relation(CF_PRED_MGR *cpm_ptr, TAG_DATA *b_ptr, char *pp_str)
/*==================================================================*/
{
    int i;

    if (!cpm_ptr || !cpm_ptr->cmm[0].cf_ptr) {
	return 0;
    }

    for (i = 0; i < cpm_ptr->cf.element_num; i++) {
	if (cpm_ptr->elem_b_ptr[i] && 
	    cpm_ptr->elem_b_ptr[i]->num == b_ptr->num && 
	    MatchPP(cpm_ptr->cmm[0].cf_ptr->pp[cpm_ptr->cmm[0].result_lists_d[0].flag[i]][0], 
		    pp_str)) {
	    return 1;
	}
    }
    return 0;
}

/*==================================================================*/
int _make_data_from_feature_to_pp(CF_PRED_MGR *cpm_ptr, TAG_DATA *b_ptr, 
				  int *pp_num, char *fcp)
/*==================================================================*/
{
    CASE_FRAME *c_ptr = &(cpm_ptr->cf);
    int cc;

    /* �Ѹ��ι�Ȥʤ��� */
    if (cpm_ptr->cf.type == CF_PRED) {
	if (!strncmp(fcp, "�Բ��ϳ�-", strlen("�Բ��ϳ�-"))) {
	    cc = pp_kstr_to_code(fcp+strlen("�Բ��ϳ�-"));
	    if (cc == END_M) {
		fprintf(stderr, ";; case <%s> in a rule is unknown!\n", fcp+9);
		exit(1);
	    }
	    c_ptr->pp[c_ptr->element_num][(*pp_num)++] = cc;
	    if (*pp_num >= PP_ELEMENT_MAX) {
		fprintf(stderr, ";; not enough pp_num (%d)!\n", PP_ELEMENT_MAX);
		exit(1);
	    }
	}
	else if (!strcmp(fcp, "��ɬ�ܳ�")) {
	    c_ptr->oblig[c_ptr->element_num] = TRUE;
	}
	else if (!strcmp(fcp, "���Ѹ�Ʊʸ��")) {	/* �֡�����ˡפΤȤ� */
	    if (cpm_ptr->pred_b_ptr->num != b_ptr->num) {
		return FALSE;
	    }
	}
    }
    /* ̾��ι�Ȥʤ��� */
    else {
	if (!strcmp(fcp, "��̾���")) {
	    /* ���: Ʊ�ʤǤϤʤ� 
	             Ϣ�ν�����ξ��Ϥ��δط������δط� */
	    if (b_ptr->dpnd_type != 'A' &&
		(!check_feature(b_ptr->f, "��:Ϣ��") || 
		 check_cc_relation(b_ptr->cpm_ptr, cpm_ptr->pred_b_ptr, "���δط�"))) {
		c_ptr->pp[c_ptr->element_num][(*pp_num)++] = 0;
		c_ptr->pp_str[c_ptr->element_num] = NULL;
	    }
	}
    }

    return TRUE;
}

/*==================================================================*/
TAG_DATA *_make_data_cframe_pp(CF_PRED_MGR *cpm_ptr, TAG_DATA *b_ptr, int flag)
/*==================================================================*/
{
    int pp_num = 0, cc, not_flag;
    char *buffer, *start_cp, *loop_cp;
    CASE_FRAME *c_ptr = &(cpm_ptr->cf);
    FEATURE *fp;

    /* flag == TRUE:  ������
       flag == FALSE: ��Ϣ�ν����� */

    /* ������ */
    if (flag == TRUE) {
	if (b_ptr->num > 0 && /* ʣ�缭�ʤɤϤҤȤ����δ��ܶ��ߤ� */
	    check_feature(b_ptr->f, "������ɽ��ľ������")) {
	    b_ptr--;
	}

	/* �֡���N�����׶ػ� (��=>�롼���) */
	if (cpm_ptr->cf.type == CF_PRED && 
	    check_feature(b_ptr->f, "��:�γ�") && 
	    check_feature(cpm_ptr->pred_b_ptr->f, "�Ѹ�:Ƚ")) {
	    return NULL;
	}

	c_ptr->oblig[c_ptr->element_num] = FALSE;

	/* �������ߤ��� */
	if (start_cp = check_feature(b_ptr->f, "����")) {
	    buffer = strdup(start_cp+strlen("����:"));
	    start_cp = buffer;
	    loop_cp = start_cp;
	    flag = 1; /* 2: OK, 1: ̤��, 0: NG */
	    not_flag = 0;
	    while (*loop_cp) {
		if (flag == 1 && *loop_cp == '&' && *(loop_cp+1) == '&') {
		    *loop_cp = '\0';
		    if ((!not_flag && !check_feature(cpm_ptr->pred_b_ptr->f, start_cp)) || 
			(not_flag && check_feature(cpm_ptr->pred_b_ptr->f, start_cp))) {
			flag = 0; /* NG */
		    }
		    loop_cp += 2;
		    start_cp = loop_cp;
		    not_flag = 0;
		}
		else if (flag < 2 && *loop_cp == '|' && *(loop_cp+1) == '|') {
		    if (flag == 1) {
			*loop_cp = '\0';
			if ((!not_flag && check_feature(cpm_ptr->pred_b_ptr->f, start_cp)) || 
			    (not_flag && !check_feature(cpm_ptr->pred_b_ptr->f, start_cp))) {
			    flag = 2; /* OK */
			}
		    }
		    else {
			flag = 1; /* 0 -> 1 */
		    }
		    loop_cp += 2;
		    start_cp = loop_cp;
		    not_flag = 0;
		}
		else if (*loop_cp == ':') {
		    *loop_cp = '\0';
		    if (flag == 2 || (flag == 1 && 
			(!not_flag && check_feature(cpm_ptr->pred_b_ptr->f, start_cp)) || 
			(not_flag && !check_feature(cpm_ptr->pred_b_ptr->f, start_cp)))) {
			if (_make_data_from_feature_to_pp(cpm_ptr, b_ptr, &pp_num, loop_cp+1) == FALSE) {
			    free(buffer);
			    return NULL;
			}
		    }
		    break;
		}
		else {
		    if (*loop_cp == '^') {
			not_flag = 1;
		    }
		    loop_cp++;
		}
	    }
	    free(buffer);
	}

	if (check_feature(b_ptr->f, "�Գ�ľ�廲��")) { /* �֡���(��)�פʤɤγʤϡ����פ����γʤ�ߤ� */
	    fp = (b_ptr + 1)->f;
	}
	else {
	    fp = b_ptr->f;
	}

	/* feature����ʤ� */
	while (fp) {
	    if (_make_data_from_feature_to_pp(cpm_ptr, b_ptr, &pp_num, fp->cp) == FALSE) {
		return NULL;
	    }
	    fp = fp->next;
	}

	if (pp_num) {
	    c_ptr->pp[c_ptr->element_num][pp_num] = END_M;
	    return b_ptr;
	}
	else {
	    return NULL;
	}
    }
    /* ��Ϣ�ν����� (�Ȥꤢ�����Ѹ��ΤȤ��Τ�) */
    else if (cpm_ptr->cf.type == CF_PRED) {
	fp = b_ptr->f;
	c_ptr->oblig[c_ptr->element_num] = FALSE;

	while (fp) {
	    if (!strncmp(fp->cp, "�Բ���Ϣ��-", strlen("�Բ���Ϣ��-"))) {
		cc = pp_kstr_to_code(fp->cp+strlen("�Բ���Ϣ��-"));
		if (cc == END_M) {
		    fprintf(stderr, ";; case <%s> in a rule is unknown!\n", fp->cp+11);
		    exit(1);
		}
		c_ptr->pp[c_ptr->element_num][pp_num++] = cc;
		if (pp_num >= PP_ELEMENT_MAX) {
		    fprintf(stderr, ";; not enough pp_num (%d)!\n", PP_ELEMENT_MAX);
		    exit(1);
		}
	    }
	    fp = fp->next;
	}
	c_ptr->pp[c_ptr->element_num][pp_num] = END_M;
	return b_ptr;
    }
    return NULL;
}

/*==================================================================*/
   void _make_data_cframe_sm(CF_PRED_MGR *cpm_ptr, TAG_DATA *b_ptr)
/*==================================================================*/
{
    int sm_num = 0, size;
    CASE_FRAME *c_ptr = &(cpm_ptr->cf);

    if (Thesaurus == USE_NTT) {
	size = SM_CODE_SIZE;
    }
    else if (Thesaurus == USE_BGH) {
	size = BGH_CODE_SIZE;
    }

    /* ������ -- ʸ */
    if (check_feature(b_ptr->f, "��ʸ")) {
	strcpy(c_ptr->sm[c_ptr->element_num]+size*sm_num, 
	       sm2code("��ʸ"));
	sm_num++;
    }
    /* ���� *
    else if (check_feature(b_ptr->f, "����")) {
	strcpy(c_ptr->sm[c_ptr->element_num]+size*sm_num, 
	       sm2code("����"));
	sm_num++;
	} */
    else {
	if (check_feature(b_ptr->f, "����")) {
	    strcpy(c_ptr->sm[c_ptr->element_num]+size*sm_num, 
		   sm2code("����"));
	    sm_num++;
	}
	if (check_feature(b_ptr->f, "����")) {
	    strcpy(c_ptr->sm[c_ptr->element_num]+size*sm_num, 
		   sm2code("����"));
	    sm_num++;
	}
	/* ��ͭ̾�� => ���� */
	if (check_feature(b_ptr->f, "��̾") || 
	    check_feature(b_ptr->f, "�ȿ�̾")) {
	    strcpy(c_ptr->sm[c_ptr->element_num]+size*sm_num, 
		   sm2code("����"));
	    sm_num++;
	}
	
	/* ���� */
	if (Thesaurus == USE_NTT) {
	    /* ������Ȥ���Τǰ�̣�Ǥ��٤ƥ��ԡ� */
	    strcpy(c_ptr->sm[c_ptr->element_num]+size*sm_num, 
		   b_ptr->SM_code);
	    sm_num += strlen(b_ptr->SM_code)/size;	    
	}
	else if (Thesaurus == USE_BGH) {
	    if (bgh_match_check(sm2code("����"), b_ptr->BGH_code)) {
		strcpy(c_ptr->sm[c_ptr->element_num]+size*sm_num, 
		       sm2code("����"));
		sm_num++;
	    }
	}
    }

    *(c_ptr->sm[c_ptr->element_num]+size*sm_num) = '\0';
}

/*==================================================================*/
   void _make_data_cframe_ex(CF_PRED_MGR *cpm_ptr, TAG_DATA *b_ptr)
/*==================================================================*/
{
    int i = 1;
    CASE_FRAME *c_ptr = &(cpm_ptr->cf);
    char *cp;

    i = 2;

    if (Thesaurus == USE_BGH) {
	strcpy(c_ptr->ex[c_ptr->element_num], b_ptr->BGH_code);
    }
    else if (Thesaurus == USE_NTT) {
	strcpy(c_ptr->ex[c_ptr->element_num], b_ptr->SM_code);
    }

    if ((OptCaseFlag & OPT_CASE_USE_REP_CF) && 
	(cp = get_mrph_rep_from_f(b_ptr->head_ptr, FALSE))) {
	strcpy(c_ptr->ex_list[c_ptr->element_num][0], cp);
    }
    else {
	strcpy(c_ptr->ex_list[c_ptr->element_num][0], b_ptr->head_ptr->Goi);
    }
    c_ptr->ex_num[c_ptr->element_num] = 1;
    c_ptr->ex_freq[c_ptr->element_num][0] = 1;
}

/*==================================================================*/
	      void set_data_cf_type(CF_PRED_MGR *cpm_ptr)
/*==================================================================*/
{
    TAG_DATA *b_ptr = cpm_ptr->pred_b_ptr;
    char *vtype = NULL;

    cpm_ptr->cf.type = CF_PRED;
    cpm_ptr->cf.type_flag = 0;
    cpm_ptr->cf.voice = b_ptr->voice;

    if ((vtype = check_feature(b_ptr->f, "�Ѹ�"))) {
	vtype += strlen("�Ѹ�:");
	strcpy(cpm_ptr->cf.pred_type, vtype);
    }
    else if ((vtype = check_feature(b_ptr->f, "���Ѹ��ʲ���"))) {
	vtype += strlen("���Ѹ��ʲ���:");
	strcpy(cpm_ptr->cf.pred_type, vtype);
    }
    else if (check_feature(b_ptr->f, "���Ѹ�")) {
	strcpy(cpm_ptr->cf.pred_type, "��");
    }
    else if (check_feature(b_ptr->f, "�θ�")) {
	strcpy(cpm_ptr->cf.pred_type, "̾");
	cpm_ptr->cf.type = CF_NOUN;
    }
    else {
	cpm_ptr->cf.pred_type[0] = '\0';
    }

    if (cpm_ptr->cf.type == CF_PRED &&
	(check_feature(b_ptr->f, "����") || 
	 check_feature(b_ptr->f, "�Ѹ�:Ƚ"))) {
	cpm_ptr->cf.type_flag = 1;
    }
}

/*==================================================================*/
int make_data_cframe_child(SENTENCE_DATA *sp, CF_PRED_MGR *cpm_ptr, TAG_DATA *child_ptr, int child_num, int closest_flag)
/*==================================================================*/
{
    TAG_DATA *cel_b_ptr;

    if ((cel_b_ptr = _make_data_cframe_pp(cpm_ptr, child_ptr, TRUE))) {
	/* �֤ߤ��󻰸Ĥ򿩤٤�� �ҤȤ�����̾�������ǤȤ���Ȥ�
	   �֤ߤ���򻰸Ŀ��٤�� �ξ��Ϥ��Τޤ�ξ�������Ǥˤʤ�
	*/

	if (check_feature(cel_b_ptr->f, "����") && 
	    (check_feature(cel_b_ptr->f, "��:����") || check_feature(cel_b_ptr->f, "��:���")) && 
	    cel_b_ptr->num > 0 && 
	    (check_feature((sp->tag_data + cel_b_ptr->num - 1)->f, "��:��") || 
	     check_feature((sp->tag_data + cel_b_ptr->num - 1)->f, "��:Ʊ��̤��")) && 
	    !check_feature((sp->tag_data + cel_b_ptr->num - 1)->f, "����") && 
	    !check_feature((sp->tag_data + cel_b_ptr->num - 1)->f, "����")) {
	    _make_data_cframe_sm(cpm_ptr, sp->tag_data + cel_b_ptr->num - 1);
	    _make_data_cframe_ex(cpm_ptr, sp->tag_data + cel_b_ptr->num - 1);
	    cpm_ptr->elem_b_ptr[cpm_ptr->cf.element_num] = sp->tag_data + cel_b_ptr->num - 1;
	    cpm_ptr->cf.adjacent[cpm_ptr->cf.element_num] = FALSE;
	}
	else {
	    /* ľ���ʤΥޡ��� (��������: ������ľ���Τ�) */
	    if (closest_flag) {
		cpm_ptr->cf.adjacent[cpm_ptr->cf.element_num] = TRUE;
	    }
	    else {
		cpm_ptr->cf.adjacent[cpm_ptr->cf.element_num] = FALSE;
	    }
	    _make_data_cframe_sm(cpm_ptr, cel_b_ptr);
	    _make_data_cframe_ex(cpm_ptr, cel_b_ptr);
	    cpm_ptr->elem_b_ptr[cpm_ptr->cf.element_num] = cel_b_ptr;
	}

	cpm_ptr->elem_b_ptr[cpm_ptr->cf.element_num]->next = NULL; /* �������ǳ�Ǽ�� */

	/* �ʤ���������Ƥ��ʤ����Ȥ�ޡ��� */
	if (check_feature(cel_b_ptr->f, "��:̤��") || 
	    check_feature(cel_b_ptr->f, "��:�γ�") || 
	    cel_b_ptr->inum > 0) {
	    cpm_ptr->elem_b_num[cpm_ptr->cf.element_num] = -1;
	}
	else {
	    cpm_ptr->elem_b_num[cpm_ptr->cf.element_num] = child_num;
	}

	cpm_ptr->cf.weight[cpm_ptr->cf.element_num] = 0;
	cpm_ptr->cf.element_num++;
	if (cpm_ptr->cf.element_num > CF_ELEMENT_MAX) {
	    cpm_ptr->cf.element_num = 0;
	}
	return TRUE;
    }

    return FALSE;
}

/*==================================================================*/
 int make_data_cframe_rentai(SENTENCE_DATA *sp, CF_PRED_MGR *cpm_ptr)
/*==================================================================*/
{
    TAG_DATA *b_ptr = cpm_ptr->pred_b_ptr, *cel_b_ptr = NULL;
    int renkaku_exception_p = 0;

    if (check_feature(b_ptr->f, "�����ǻ���:2")) {
	renkaku_exception_p = 1;
    }

    /* ��Ϣ�ν����� */
    if ((check_feature(b_ptr->f, "��:Ϣ��") && 
	 (b_ptr->para_type != PARA_NORMAL || 
	  b_ptr->num == b_ptr->parent->num)) || /* �Ѹ�����ʤ顢��V�����פ�<PARA>��Ʊ���Ȥ��Τ� */
	(b_ptr->para_type == PARA_NORMAL && /* �����Ϣ�ν��� */
	 check_feature(b_ptr->f, "��:Ϣ��") && /* ��ñ���Ϣ�ѷ��˸��ꤹ�٤��� */
	 b_ptr->parent->para_top_p && 
	 check_feature(b_ptr->parent->child[0]->f, "��:Ϣ��")) || 
	renkaku_exception_p) {

	/* para_type == PARA_NORMAL �ϡ�V��,V���� PARA N�פΤȤ�
	   ���ΤȤ��Ͽ�(PARA)�ο�(N)������ǤȤ��롥

	   �Ƥ�para_top_p���ɤ�����ߤƤ��V����N��N PARA�פ�
	   ���ȶ��̤��Ǥ��ʤ�
        */

	/* �Ѹ�������ǤϤʤ��Ȥ� */
	if (b_ptr->para_type != PARA_NORMAL) {
	    if (b_ptr->parent) {
		/* ���Τ� */
		if (renkaku_exception_p && 
		    b_ptr->parent->parent) {
		    if (check_feature(b_ptr->parent->parent->f, "�θ�")) {
			cel_b_ptr = b_ptr->parent->parent;
			_make_data_cframe_pp(cpm_ptr, b_ptr, FALSE);
		    }
		}
		else {
		    cel_b_ptr = b_ptr->parent;
		    _make_data_cframe_pp(cpm_ptr, b_ptr, FALSE);
		}

		if (cel_b_ptr) {
		    _make_data_cframe_sm(cpm_ptr, cel_b_ptr);
		    _make_data_cframe_ex(cpm_ptr, cel_b_ptr);
		    cpm_ptr->elem_b_ptr[cpm_ptr->cf.element_num] = cel_b_ptr;
		    cpm_ptr->elem_b_num[cpm_ptr->cf.element_num] = -1;
		    cpm_ptr->cf.weight[cpm_ptr->cf.element_num] = 0;
		    cpm_ptr->cf.adjacent[cpm_ptr->cf.element_num] = FALSE;
		    cpm_ptr->cf.element_num++;
		}
	    }
	}
	/* �Ѹ�������ΤȤ� */
	else {
	    cel_b_ptr = b_ptr;
	    while (cel_b_ptr->parent->para_type == PARA_NORMAL) {
		cel_b_ptr = cel_b_ptr->parent;
	    }

	    if (cel_b_ptr->parent && 
		cel_b_ptr->parent->parent) {
		_make_data_cframe_pp(cpm_ptr, cel_b_ptr->parent, FALSE);
		_make_data_cframe_sm(cpm_ptr, cel_b_ptr->parent->parent);
		_make_data_cframe_ex(cpm_ptr, cel_b_ptr->parent->parent);
		cpm_ptr->elem_b_ptr[cpm_ptr->cf.element_num] = cel_b_ptr->parent->parent;
		cpm_ptr->elem_b_num[cpm_ptr->cf.element_num] = -1;
		cpm_ptr->cf.weight[cpm_ptr->cf.element_num] = 2; /* �������Ρ� */
		cpm_ptr->cf.adjacent[cpm_ptr->cf.element_num] = FALSE;
		cpm_ptr->cf.element_num++;
	    }
	}
    }

    return TRUE;
}

/*==================================================================*/
    int make_data_cframe(SENTENCE_DATA *sp, CF_PRED_MGR *cpm_ptr)
/*==================================================================*/
{
    TAG_DATA *b_ptr = cpm_ptr->pred_b_ptr;
    TAG_DATA *cel_b_ptr = NULL;
    int i, child_num, first, closest, orig_child_num = -1, renkaku_exception_p;

    cpm_ptr->cf.samecase[0][0] = END_M;
    cpm_ptr->cf.samecase[0][1] = END_M;

    cpm_ptr->cf.pred_b_ptr = b_ptr;
    b_ptr->cpm_ptr = cpm_ptr;

    /* ɽ�س� etc. ������ */

    cpm_ptr->cf.element_num = 0;

    /* Ϣ�ν��� */
    make_data_cframe_rentai(sp, cpm_ptr);

    for (child_num = 0; b_ptr->child[child_num]; child_num++);

    /* ��ʬ(�Ѹ�)��ʣ��̾���� */
    if (b_ptr->inum > 0) {
	TAG_DATA *t_ptr;

	/* ��ʬ(�Ѹ�)��ʣ��̾����ΤȤ��ο� : ��Ϣ�ν����찷��
	   �� Ϣ�ʤΤȤ�(�֡������Τϡ�)�Ϥ��Ǥ˰��äƤ��� */
	if (cpm_ptr->cf.type == CF_PRED && /* �Ȥꤢ�������ѤΤȤ��Τ� */
	    !check_feature(b_ptr->f, "��:Ϣ��")) {
	    if (!check_feature(b_ptr->parent->f, "���δط�") || 
		check_feature(b_ptr->f, "�Ѹ�:��")) {
		_make_data_cframe_pp(cpm_ptr, b_ptr, FALSE);
	    }
	    else {
		cpm_ptr->cf.pp[cpm_ptr->cf.element_num][0] = pp_hstr_to_code("���δط�");
		cpm_ptr->cf.pp[cpm_ptr->cf.element_num][1] = END_M;
		cpm_ptr->cf.oblig[cpm_ptr->cf.element_num] = FALSE;
	    }
	    _make_data_cframe_sm(cpm_ptr, b_ptr->parent);
	    _make_data_cframe_ex(cpm_ptr, b_ptr->parent);
	    cpm_ptr->elem_b_ptr[cpm_ptr->cf.element_num] = b_ptr->parent;
	    cpm_ptr->elem_b_num[cpm_ptr->cf.element_num] = -1;
	    cpm_ptr->cf.weight[cpm_ptr->cf.element_num] = 0;
	    cpm_ptr->cf.adjacent[cpm_ptr->cf.element_num] = FALSE;
	    cpm_ptr->cf.element_num++;
	}

	/* ʸ���head�˷���̾��μ�갷�� *

	t_ptr = b_ptr->parent;
	while (1) {
	    if (t_ptr->cpm_ptr) { * �̤γʲ����о� *
		t_ptr = NULL;
		break;
	    }
	    if (t_ptr->inum == 0) {
		break;
	    }
	    t_ptr = t_ptr->parent;
	}

	* ... n3 n2 n1
	   ʣ��̾�����(��������ߤ�)�ǽ���Ѹ�(�ʲ����о�)���Ф���
	   n1�λҶ���ȤäƤ��� *
	if (t_ptr) {
	    orig_child_num = child_num;
	    for (i = 0; t_ptr->child[i]; i++) {
		* ʸ�������ʳ� *
		if (t_ptr->child[i]->inum == 0) {
		    b_ptr->child[child_num++] = t_ptr->child[i];
		}
	    }
	}
	*/
    }

    /* �Ҷ�������Ǥ� */
    for (i = child_num - 1; i >= 0; i--) {
	if (make_data_cframe_child(sp, cpm_ptr, b_ptr->child[i], i, 
				   i == 0 && b_ptr->num == b_ptr->child[i]->num + 1 && 
				   !check_feature(b_ptr->f, "���Ѹ�Ʊʸ��") ? TRUE : FALSE)) {
	    if (cpm_ptr->cf.element_num == 0) { /* �Ҷ�������Ϥ��ʤΤˡ����ʤ��ä� */
		return -1;
	    }
	}
    }

    /* ʣ��̾��: �Ҷ����Ȥˤ�ɤ� */
    if (orig_child_num >= 0) {
	b_ptr->child[orig_child_num] = NULL;
    }

    /* �Ѹ�ʸ�᤬�֡ʡ���ˡ��ˡפΤȤ� 
       �֤���פγʥե졼����Ф��ƥ˳�(Ʊʸ��)������
       ��ʤϻҶ��ν����ǰ����� */
    if (check_feature(b_ptr->f, "���Ѹ�Ʊʸ��")) {
	if (_make_data_cframe_pp(cpm_ptr, b_ptr, TRUE)) {
	    _make_data_cframe_sm(cpm_ptr, b_ptr);
	    _make_data_cframe_ex(cpm_ptr, b_ptr);
	    cpm_ptr->elem_b_ptr[cpm_ptr->cf.element_num] = b_ptr;
	    cpm_ptr->elem_b_num[cpm_ptr->cf.element_num] = child_num;
	    cpm_ptr->cf.weight[cpm_ptr->cf.element_num] = 0;
	    cpm_ptr->cf.adjacent[cpm_ptr->cf.element_num] = TRUE;
	    cpm_ptr->cf.element_num++;
	}
    }

    /* �Ѹ�������ΤȤ��������Ǥ� expand ���� */
    if (b_ptr->para_type == PARA_NORMAL && 
	b_ptr->parent && 
	b_ptr->parent->para_top_p) {
	child_num = 0;

	/* <PARA>�˷���Ҷ�������å� */
	for (i = 0; b_ptr->parent->child[i]; i++) {
	    if (b_ptr->parent->child[i]->para_type == PARA_NORMAL) {
		child_num++;
	    }
	}
	for (i = 0; b_ptr->parent->child[i]; i++) {
	    if (b_ptr->parent->child[i]->para_type == PARA_NIL && 
		b_ptr->parent->child[i]->num < b_ptr->num) {
		if ((cel_b_ptr = _make_data_cframe_pp(cpm_ptr, b_ptr->parent->child[i], TRUE))) {
		    _make_data_cframe_sm(cpm_ptr, cel_b_ptr);
		    _make_data_cframe_ex(cpm_ptr, cel_b_ptr);
		    cpm_ptr->elem_b_ptr[cpm_ptr->cf.element_num] = cel_b_ptr;

		    /* �ʤ���������Ƥ��ʤ����Ȥ�ޡ��� */
		    if (check_feature(cel_b_ptr->f, "��:̤��") || 
			check_feature(cel_b_ptr->f, "��:�γ�")) {
			cpm_ptr->elem_b_num[cpm_ptr->cf.element_num] = -1;
		    }
		    else {
			cpm_ptr->elem_b_num[cpm_ptr->cf.element_num] = i;
		    }

		    cpm_ptr->cf.weight[cpm_ptr->cf.element_num] = child_num;
		    cpm_ptr->cf.adjacent[cpm_ptr->cf.element_num] = FALSE;
		    cpm_ptr->cf.element_num++;
		}
		if (cpm_ptr->cf.element_num > CF_ELEMENT_MAX) {
		    cpm_ptr->cf.element_num = 0;
		    return -1;
		}
	    }
	}
    }

    /* ľ�������Ǥμ��� */
    closest = get_closest_case_component(sp, cpm_ptr);

    /* ľ�������ǤΤҤȤļ����Υγ�
       �� <����>�ʳ�: ��ޡ�γ����� V
          <����>�ʳ�: */
    if (OptCaseFlag & OPT_CASE_NO && 
	closest > -1 && 
	cpm_ptr->elem_b_ptr[closest]->num > 0 && 
	!check_feature((sp->tag_data + cpm_ptr->elem_b_ptr[closest]->num - 1)->f, "����") && 
	!check_feature((sp->tag_data + cpm_ptr->elem_b_ptr[closest]->num - 1)->f, "����") && 
	check_feature((sp->tag_data + cpm_ptr->elem_b_ptr[closest]->num - 1)->f, "��:�γ�")) {
	TAG_DATA *bp;
	bp = sp->tag_data + cpm_ptr->elem_b_ptr[closest]->num - 1;

	/* ������Ƥ�ʤϳʥե졼��ˤ�ä�ưŪ���Ѥ�� */
	cpm_ptr->cf.pp[cpm_ptr->cf.element_num][0] = pp_hstr_to_code("̤");
	cpm_ptr->cf.pp[cpm_ptr->cf.element_num][1] = END_M;
	cpm_ptr->cf.sp[cpm_ptr->cf.element_num] = pp_hstr_to_code("��"); /* ɽ�س� */
	cpm_ptr->cf.oblig[cpm_ptr->cf.element_num] = FALSE;
	_make_data_cframe_sm(cpm_ptr, bp);
	_make_data_cframe_ex(cpm_ptr, bp);
	cpm_ptr->elem_b_ptr[cpm_ptr->cf.element_num] = bp;
	cpm_ptr->elem_b_num[cpm_ptr->cf.element_num] = -1;
	cpm_ptr->cf.weight[cpm_ptr->cf.element_num] = 0;
	cpm_ptr->cf.adjacent[cpm_ptr->cf.element_num] = FALSE;
	if (cpm_ptr->cf.element_num < CF_ELEMENT_MAX) {
	    cpm_ptr->cf.element_num++;
	}
    }

    return closest; /* �ʲ��Ϻ������ͽ�� */

    /* �����Ǥ��ҤȤĤǻ��ֳʤΤߤξ�硢�����Ǥʤ���Ʊ���褦�˰���
       ��ľ���λ��ֳʤǤ�Ȥγʤ������ʤǤ���Ȥ������̤˰����� (���ߤϥ˳ʤΤ�) */

    for (i = 0; i < cpm_ptr->cf.element_num; i++) {
	if (!MatchPP(cpm_ptr->cf.pp[i][0], "����")) {
	    return closest;
	}
	/* ľ���ʤǥ˳ʤ� <����> �ΤȤ��Ϻ�����ʤ� */
	else if (cpm_ptr->pred_b_ptr->num == cpm_ptr->elem_b_ptr[i]->num + 1 && 
		 check_feature(cpm_ptr->elem_b_ptr[i]->f, "��:�˳�")) {
	    return closest;
	}
    }
    cpm_ptr->cf.element_num = 0;
    return -1;
}

/*==================================================================*/
		 void set_pred_voice(BNST_DATA *ptr)
/*==================================================================*/
{
    /* �������������� */

    char *cp;

    ptr->voice = 0;

    if (cp = check_feature(ptr->f, "��")) {
	char *token, *str;

	str = strdup(cp + strlen("��:"));
	token = strtok(str, "|");
	while (token) {
	    if (!strcmp(token, "��ư")) {
		ptr->voice |= VOICE_UKEMI;
	    }
	    else if (!strcmp(token, "����")) {
		ptr->voice |= VOICE_SHIEKI;
	    }
	    else if (!strcmp(token, "��餦")) {
		ptr->voice |= VOICE_MORAU;
	    }
	    else if (!strcmp(token, "�ۤ���")) {
		ptr->voice |= VOICE_HOSHII;
	    }
	    else if (!strcmp(token, "����&��ư")) {
		ptr->voice |= VOICE_SHIEKI_UKEMI;
	    }
	    /* �ֲ�ǽ�פ�̤���� */

	    token = strtok(NULL, "|");
	}
	free(str);
    }
}

/*====================================================================
                               END
====================================================================*/
