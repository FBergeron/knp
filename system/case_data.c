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
	     char *make_fukugoji_string(BNST_DATA *b_ptr)
/*==================================================================*/
{
    int i, fc;
    char buf[SMALL_DATA_LEN];

    /* ��°�줬�ʤ��Ȥ� */
    if (b_ptr->num < 1 || (b_ptr-1)->fuzoku_num == 0) {
	return NULL;
    }

    buf[0] = '\0';

    /* ����ʸ��ν��� */
    strcat(buf, ((b_ptr-1)->fuzoku_ptr+(b_ptr-1)->fuzoku_num-1)->Goi);

    /* ����ʸ�� */
    for (i = 0; i < b_ptr->jiritu_num; i++) {
	if (!strcmp(Class[(b_ptr->jiritu_ptr+i)->Hinshi][0].id, "�ü�")) /* �ü�ʳ� */
	    continue;
	strcat(buf, 
	       (b_ptr->jiritu_ptr+i)->Goi);
    }

    /* �������ɤߤ����� */
    for (i = 0; *(FukugojiTable[i]); i+=2) {
	if (str_eq(buf, FukugojiTable[i])) {
	    strcpy(buf, FukugojiTable[i+1]);
	    break;
	}
    }

    fc = pp_hstr_to_code(buf);
    if (fc != END_M) {
	sprintf(fukugoji_string, "���ϳ�-%s", pp_code_to_kstr(fc));
	return fukugoji_string;
    }
    return NULL;
}

/*==================================================================*/
BNST_DATA *_make_data_cframe_pp(CF_PRED_MGR *cpm_ptr, BNST_DATA *b_ptr, int flag)
/*==================================================================*/
{
    int pp_num = 0, cc, jiritsu_num = 0, closest = FALSE;
    CASE_FRAME *c_ptr = &(cpm_ptr->cf);
    FEATURE *fp;

    if (b_ptr) {
	/* ��Ω��ο� */
	if (!check_feature(b_ptr->f, "����")) {
	    jiritsu_num = b_ptr->jiritu_num;
	}

	/* ��ʣ��̾��ΤȤ���? */
	closest = cpm_ptr->pred_b_ptr->num == b_ptr->num+1 ? TRUE : FALSE;
    }

    /* ������ */
    if (flag == TRUE) {
	if (b_ptr->num > 0 && 
	    check_feature(b_ptr->f, "��:Ϣ��") && 
	    check_feature(b_ptr->f, "ʣ�缭")) {
	    b_ptr--;
	}

	fp = b_ptr->f;
	c_ptr->oblig[c_ptr->element_num] = FALSE;

	while (fp) {
	    if (!strncmp(fp->cp, "���ϳ�-", 7)) {
		cc = pp_kstr_to_code(fp->cp+7);
		if (cc == END_M) {
		    fprintf(stderr, ";; case <%s> in a rule is unknown!\n", fp->cp+7);
		    exit(1);
		}
		c_ptr->pp[c_ptr->element_num][pp_num++] = cc;
		if (pp_num >= PP_ELEMENT_MAX) {
		    fprintf(stderr, ";; not enough pp_num (%d)!\n", PP_ELEMENT_MAX);
		    exit(1);
		}
	    }
	    else if (!strcmp(fp->cp, "ɬ�ܳ�")) {
		c_ptr->oblig[c_ptr->element_num] = TRUE;
	    }
	    else if (!strcmp(fp->cp, "���Ѹ�Ʊʸ��")) {	/* �֡�����ˡפΤȤ� */
		if (cpm_ptr->pred_b_ptr->num != b_ptr->num) {
		    return NULL;
		}
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
    /* ��Ϣ�ν����� */
    else {
	fp = b_ptr->f;
	c_ptr->oblig[c_ptr->element_num] = FALSE;

	while (fp) {
	    if (!strncmp(fp->cp, "����Ϣ��-", 9)) {
		cc = pp_kstr_to_code(fp->cp+9);
		if (cc == END_M) {
		    fprintf(stderr, ";; case <%s> in a rule is unknown!\n", fp->cp+7);
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
   void _make_data_cframe_sm(CF_PRED_MGR *cpm_ptr, BNST_DATA *b_ptr)
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
    /* ���� */
    else if (check_feature(b_ptr->f, "����")) {
	strcpy(c_ptr->sm[c_ptr->element_num]+size*sm_num, 
	       sm2code("����"));
	sm_num++;
    }
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
}

/*==================================================================*/
   void _make_data_cframe_ex(CF_PRED_MGR *cpm_ptr, BNST_DATA *b_ptr)
/*==================================================================*/
{
    CASE_FRAME *c_ptr = &(cpm_ptr->cf);

    if (Thesaurus == USE_BGH) {
	strcpy(c_ptr->ex[c_ptr->element_num], b_ptr->BGH_code);
    }
    else if (Thesaurus == USE_NTT) {
	strcpy(c_ptr->ex[c_ptr->element_num], b_ptr->SM_code);
    }
    strcpy(c_ptr->ex_list[c_ptr->element_num][0], b_ptr->Jiritu_Go);
}

/*==================================================================*/
    void make_data_cframe(SENTENCE_DATA *sp, CF_PRED_MGR *cpm_ptr)
/*==================================================================*/
{
    BNST_DATA *b_ptr = cpm_ptr->pred_b_ptr;
    BNST_DATA *cel_b_ptr;
    int i, child_num, first, closest, orig_child_num = -1;
    char *vtype = NULL;

    cpm_ptr->cf.voice = b_ptr->voice;

    if ((vtype = check_feature(b_ptr->f, "�Ѹ�"))) {
	vtype += 5;
	strcpy(cpm_ptr->cf.pred_type, vtype);
    }
    else if (check_feature(b_ptr->f, "����")) {
	strcpy(cpm_ptr->cf.pred_type, "ư");
    }
    else if (check_feature(b_ptr->f, "̾��Ū���ƻ�촴")) {
	strcpy(cpm_ptr->cf.pred_type, "��");
    }
    else if (check_feature(b_ptr->f, "���Ѹ�")) {
	strcpy(cpm_ptr->cf.pred_type, "��");
    }
    else {
	cpm_ptr->cf.pred_type[0] = '\0';
    }

    cpm_ptr->cf.samecase[0][0] = END_M;
    cpm_ptr->cf.samecase[0][1] = END_M;

    cpm_ptr->cf.pred_b_ptr = b_ptr;
    b_ptr->cpm_ptr = cpm_ptr;

    /* ɽ�س� etc. ������ */

    cpm_ptr->cf.element_num = 0;

    /* �Ѹ�ʸ�᤬�֡ʡ���ˡ��ˡפΤȤ� 
       �֤���פγʥե졼����Ф��ƥ˳�(Ʊʸ��)������ */
    if (check_feature(b_ptr->f, "ID:�ʡ���ˡ���")) {
	if (_make_data_cframe_pp(cpm_ptr, b_ptr, TRUE)) {
	    _make_data_cframe_sm(cpm_ptr, b_ptr);
	    _make_data_cframe_ex(cpm_ptr, b_ptr);
	    cpm_ptr->elem_b_ptr[cpm_ptr->cf.element_num] = b_ptr;
	    cpm_ptr->elem_b_num[cpm_ptr->cf.element_num] = -1;
	    cpm_ptr->cf.weight[cpm_ptr->cf.element_num] = 0;
	    cpm_ptr->cf.adjacent[cpm_ptr->cf.element_num] = TRUE;
	    cpm_ptr->cf.element_num ++;
	}
    }

    if (check_feature(b_ptr->f, "��:Ϣ��")) {

	/* para_type == PARA_NORMAL �ϡ�V��,V���� PARA N�פΤȤ�
	   ���ΤȤ��Ͽ�(PARA)�ο�(N)������ǤȤ��롥

	   �Ƥ�para_top_p���ɤ�����ߤƤ��V����N��N PARA�פ�
	   ���ȶ��̤��Ǥ��ʤ�
        */

	/* �Ѹ�������ǤϤʤ��Ȥ� */
	if (b_ptr->para_type != PARA_NORMAL) {
	    if (b_ptr->parent) {
		/* ���δط��ʳ��ΤȤ��ϳ����Ǥ� (���δط��Ǥ���ƻ�ΤȤ��ϳ����Ǥˤ���) */
		if (!(check_feature(b_ptr->parent->f, "���δط�") || 
		      check_feature(b_ptr->parent->f, "���δط���ǽ��")) || 
		    check_feature(b_ptr->f, "�Ѹ�:��")) {
		    _make_data_cframe_pp(cpm_ptr, b_ptr, FALSE);
		    _make_data_cframe_sm(cpm_ptr, b_ptr->parent);
		    _make_data_cframe_ex(cpm_ptr, b_ptr->parent);
		    cpm_ptr->elem_b_ptr[cpm_ptr->cf.element_num] = b_ptr->parent;
		    cpm_ptr->elem_b_num[cpm_ptr->cf.element_num] = -1;
		    cpm_ptr->cf.weight[cpm_ptr->cf.element_num] = 0;
		    cpm_ptr->cf.adjacent[cpm_ptr->cf.element_num] = FALSE;
		    cpm_ptr->cf.element_num ++;
		}
		else {
		    cpm_ptr->default_score = SOTO_SCORE;
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
		if (!(check_feature(cel_b_ptr->parent->parent->f, "���δط�") || 
		      check_feature(b_ptr->parent->parent->f, "���δط���ǽ��")) || 
		    check_feature(b_ptr->f, "�Ѹ�:��")) {
		    _make_data_cframe_pp(cpm_ptr, cel_b_ptr->parent, FALSE);
		    _make_data_cframe_sm(cpm_ptr, cel_b_ptr->parent->parent);
		    _make_data_cframe_ex(cpm_ptr, cel_b_ptr->parent->parent);
		    cpm_ptr->elem_b_ptr[cpm_ptr->cf.element_num] = cel_b_ptr->parent->parent;
		    cpm_ptr->elem_b_num[cpm_ptr->cf.element_num] = -1;
		    cpm_ptr->cf.weight[cpm_ptr->cf.element_num] = 0;
		    cpm_ptr->cf.adjacent[cpm_ptr->cf.element_num] = FALSE;
		    cpm_ptr->cf.element_num ++;
		}
		else {
		    cpm_ptr->default_score = SOTO_SCORE;
		}
	    }
	}
    }

    /* ��ʬ(�Ѹ�)��ʣ��̾����ΤȤ��ο� */
    if (b_ptr->num == -1) {
	if (b_ptr->parent && b_ptr->parent->num == -1) {
	    _make_data_cframe_pp(cpm_ptr, b_ptr, FALSE);
	    _make_data_cframe_sm(cpm_ptr, b_ptr->parent);
	    _make_data_cframe_ex(cpm_ptr, b_ptr->parent);
	    cpm_ptr->elem_b_ptr[cpm_ptr->cf.element_num] = b_ptr->parent;
	    cpm_ptr->elem_b_num[cpm_ptr->cf.element_num] = -1;
	    cpm_ptr->cf.weight[cpm_ptr->cf.element_num] = 0;
	    cpm_ptr->cf.adjacent[cpm_ptr->cf.element_num] = FALSE;
	    cpm_ptr->cf.element_num ++;
	}
	/* �缭 */
	else {
	    for (child_num = 0; b_ptr->child[child_num]; child_num++);
	    orig_child_num = child_num;
	    for (i = 0; b_ptr->parent->child[i]; i++) {
		b_ptr->child[child_num+i] = b_ptr->parent->child[i];
	    }
	}
    }

    /* �Ҷ�������Ǥ� */
    for (child_num=0; b_ptr->child[child_num]; child_num++);
    for (i = child_num - 1; i >= 0; i--) {
	if ((cel_b_ptr = _make_data_cframe_pp(cpm_ptr, b_ptr->child[i], TRUE))) {
	    /* �֤ߤ��󻰸Ĥ򿩤٤�� �ҤȤ�����̾�������ǤȤ���Ȥ�
	       �֤ߤ���򻰸Ŀ��٤�� �ξ��Ϥ��Τޤ�ξ�������Ǥˤʤ�
	     */
	    if (check_feature(cel_b_ptr->f, "����") && 
		(check_feature(cel_b_ptr->f, "��:����") || check_feature(cel_b_ptr->f, "��:���")) && 
		cel_b_ptr->num > 0 && 
		(check_feature((sp->bnst_data+cel_b_ptr->num-1)->f, "��:����") || 
		 check_feature((sp->bnst_data+cel_b_ptr->num-1)->f, "��:Ʊ��̤��")) && 
		!check_feature((sp->bnst_data+cel_b_ptr->num-1)->f, "����") && 
		!check_feature((sp->bnst_data+cel_b_ptr->num-1)->f, "����")) {
		_make_data_cframe_sm(cpm_ptr, sp->bnst_data+cel_b_ptr->num-1);
		_make_data_cframe_ex(cpm_ptr, sp->bnst_data+cel_b_ptr->num-1);
		cpm_ptr->elem_b_ptr[cpm_ptr->cf.element_num] = sp->bnst_data+cel_b_ptr->num-1;
		cpm_ptr->cf.adjacent[cpm_ptr->cf.element_num] = FALSE;
	    }
	    else {
		/* ľ���ʤΥޡ��� (��������: ������ľ���Τ�) */
		if (i == 0 && b_ptr->num == b_ptr->child[i]->num+1 && 
		    !check_feature(b_ptr->f, "ID:�ʡ���ˡ���")) {
		    cpm_ptr->cf.adjacent[cpm_ptr->cf.element_num] = TRUE;
		}
		else {
		    cpm_ptr->cf.adjacent[cpm_ptr->cf.element_num] = FALSE;
		}
		_make_data_cframe_sm(cpm_ptr, cel_b_ptr);
		_make_data_cframe_ex(cpm_ptr, cel_b_ptr);
		cpm_ptr->elem_b_ptr[cpm_ptr->cf.element_num] = cel_b_ptr;
	    }
	    if (check_feature(cel_b_ptr->f, "��:̤��")) {
		cpm_ptr->elem_b_num[cpm_ptr->cf.element_num] = -1;
	    }
	    else {
		cpm_ptr->elem_b_num[cpm_ptr->cf.element_num] = i;
	    }
	    cpm_ptr->cf.weight[cpm_ptr->cf.element_num] = 0;
	    cpm_ptr->cf.element_num ++;
	}
	if (cpm_ptr->cf.element_num > CF_ELEMENT_MAX) {
	    cpm_ptr->cf.element_num = 0;
	    return;
	}
    }

    /* ʣ��̾��: �Ҷ����Ȥˤ�ɤ� */
    if (orig_child_num >= 0) {
	b_ptr->child[orig_child_num] = NULL;
    }

    /* �Ѹ�������ΤȤ��������Ǥ� expand ���� */
    if (b_ptr->para_type == PARA_NORMAL && 
	b_ptr->parent && 
	b_ptr->parent->para_top_p) {
	child_num = 0;
	for (i = 0; b_ptr->parent->child[i]; i++) {
	    if (b_ptr->parent->child[i]->para_type == PARA_NORMAL) {
		child_num++;
	    }
	}
	for (i = 0; b_ptr->parent->child[i]; i++) {
	    if (b_ptr->parent->child[i]->para_type == PARA_NIL) {
		if ((cel_b_ptr = _make_data_cframe_pp(cpm_ptr, b_ptr->parent->child[i], TRUE))) {
		    _make_data_cframe_sm(cpm_ptr, cel_b_ptr);
		    _make_data_cframe_ex(cpm_ptr, cel_b_ptr);
		    cpm_ptr->elem_b_ptr[cpm_ptr->cf.element_num] = cel_b_ptr;
		    cpm_ptr->elem_b_num[cpm_ptr->cf.element_num] = i;
		    cpm_ptr->cf.weight[cpm_ptr->cf.element_num] = child_num;
		    cpm_ptr->cf.adjacent[cpm_ptr->cf.element_num] = FALSE;
		    cpm_ptr->cf.element_num ++;
		}
		if (cpm_ptr->cf.element_num > CF_ELEMENT_MAX) {
		    cpm_ptr->cf.element_num = 0;
		    return;
		}
	    }
	}
    }

    closest = get_closest_case_component(sp, cpm_ptr);

    /* ľ�������ǤΤҤȤļ����Υγ�
       �� <����>�ʳ�: ��ޡ�γ����� V
          <����>�ʳ�: */
    if (OptCaseFlag & OPT_CASE_NO && 
	closest > -1 && 
	cpm_ptr->elem_b_ptr[closest]->num > 0 && 
	!check_feature((sp->bnst_data+cpm_ptr->elem_b_ptr[closest]->num-1)->f, "����") && 
	!check_feature((sp->bnst_data+cpm_ptr->elem_b_ptr[closest]->num-1)->f, "����") && 
	check_feature((sp->bnst_data+cpm_ptr->elem_b_ptr[closest]->num-1)->f, "��:�γ�")
	/* !check_feature(cpm_ptr->elem_b_ptr[closest]->f, "����") && 
	!check_feature(cpm_ptr->elem_b_ptr[closest]->f, "����") */
	) {
	BNST_DATA *bp;
	bp = sp->bnst_data+cpm_ptr->elem_b_ptr[closest]->num-1;

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

    /* �����Ǥ��ҤȤĤǻ��ֳʤΤߤξ�硢�����Ǥʤ���Ʊ���褦�˰���
       ��ľ���λ��ֳʤǤ�Ȥγʤ������ʤǤ���Ȥ������̤˰����� (���ߤϥ˳ʤΤ�) */

    for (i = 0; i < cpm_ptr->cf.element_num; i++) {
	if (!MatchPP(cpm_ptr->cf.pp[i][0], "����")) {
	    return;
	}
	/* ľ���ʤǥ˳ʤ� <����> �ΤȤ��Ϻ�����ʤ� */
	else if (cpm_ptr->pred_b_ptr->num == cpm_ptr->elem_b_ptr[i]->num+1 && 
	    check_feature(cpm_ptr->elem_b_ptr[i]->f, "��:�˳�")) {
	    return;
	}
    }
    cpm_ptr->cf.element_num = 0;
    return;
}

/*==================================================================*/
		void set_pred_voice(BNST_DATA *b_ptr)
/*==================================================================*/
{
    /* �������������� */ /* ������ ����ɬ�� ������ */

    if (check_feature(b_ptr->f, "������") ||
	check_feature(b_ptr->f, "��������")) {
	b_ptr->voice = VOICE_SHIEKI;
    } else if (check_feature(b_ptr->f, "�����") ||
	       check_feature(b_ptr->f, "������")) {
	b_ptr->voice = VOICE_UKEMI;
    } else if (check_feature(b_ptr->f, "����餦")) {
	b_ptr->voice = VOICE_MORAU;
    }
}

/*====================================================================
                               END
====================================================================*/
