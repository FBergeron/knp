/*====================================================================

			  �ʹ�¤����: ����¦

                                               S.Kurohashi 93. 5.31

    $Id$
====================================================================*/
#include "knp.h"

int SOTO_SCORE = 7;

char fukugoji_string[64];

char *FukugojiTable[] = {"�����", "��Τ���", 
			 "���̤���", "��Ĥ�����", 
			 "���̤���", "��Ĥ�����", 
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
    int i;

    fukugoji_string[0] = '\0';

    /* ����ʸ��ν��� */
    strcat(fukugoji_string, 
	   ((b_ptr-1)->fuzoku_ptr+(b_ptr-1)->fuzoku_num-1)->Goi);

    /* ����ʸ�� */
    for (i = 0; i < b_ptr->jiritu_num; i++) {
	if ((b_ptr->jiritu_ptr+i)->Hinshi == 1)	/* �ü�ʳ� */
	    continue;
	strcat(fukugoji_string, 
	       (b_ptr->jiritu_ptr+i)->Goi);
    }

    /* �������ɤߤ����� */
    for (i = 0; *(FukugojiTable[i]); i+=2) {
	if (str_eq(fukugoji_string, FukugojiTable[i])) {
	    strcpy(fukugoji_string, FukugojiTable[i+1]);
	}
    }

    return fukugoji_string;
}

/*==================================================================*/
BNST_DATA *_make_data_cframe_pp(CF_PRED_MGR *cpm_ptr, BNST_DATA *b_ptr)
/*==================================================================*/
{
    int i, num, pp_num = 0, jiritsu_num = 0;
    CASE_FRAME *c_ptr = &(cpm_ptr->cf);

    if (b_ptr && !check_feature(b_ptr->f, "����")) {
	jiritsu_num = b_ptr->jiritu_num;
    }

    if (b_ptr == NULL) {	/* ������ʸ���､���� */
	c_ptr->pp[c_ptr->element_num][0] = pp_hstr_to_code("��");
	c_ptr->pp[c_ptr->element_num][1] = END_M;
	c_ptr->oblig[c_ptr->element_num] = FALSE;
	return b_ptr;
    }
    else if (check_feature(b_ptr->f, "����")) {
	c_ptr->pp[c_ptr->element_num][0] = pp_hstr_to_code("����");
	c_ptr->pp[c_ptr->element_num][1] = END_M;
	c_ptr->oblig[c_ptr->element_num] = FALSE;
	return b_ptr;
    }
    else if (check_feature(b_ptr->f, "��:����")) {
	c_ptr->pp[c_ptr->element_num][0] = pp_hstr_to_code("��");
	c_ptr->pp[c_ptr->element_num][1] = END_M;
	c_ptr->oblig[c_ptr->element_num] = TRUE;
	return b_ptr;
    }
    else if (check_feature(b_ptr->f, "��:���")) {
	c_ptr->pp[c_ptr->element_num][0] = pp_hstr_to_code("��");
	c_ptr->pp[c_ptr->element_num][1] = END_M;
	c_ptr->oblig[c_ptr->element_num] = TRUE;
	return b_ptr;
    }
    else if (check_feature(b_ptr->f, "��:�س�")) {
	/* �س� or �˳� */
	c_ptr->pp[c_ptr->element_num][pp_num++] = pp_hstr_to_code("��");
	c_ptr->pp[c_ptr->element_num][pp_num++] = pp_hstr_to_code("��");
	c_ptr->pp[c_ptr->element_num][pp_num] = END_M;
	c_ptr->oblig[c_ptr->element_num] = TRUE;
	return b_ptr;
    }
    else if (check_feature(b_ptr->f, "��:�γ�") && 
	     !check_feature(cpm_ptr->pred_b_ptr->f, "�Ѹ�:Ƚ") && 
	     !check_feature(cpm_ptr->pred_b_ptr->f, "���Ѹ�")) {
	/* ���ܤ��ʤ�����? */
	/* ����٤����ͤ��ߤ��ơ�ñ�ʤ뽤������̤���ɬ�פ����� */

	/* <����> �ʤ���ֳʤΤߤˤ��� */
	if (check_feature(b_ptr->f, "����")) {
	    c_ptr->pp[c_ptr->element_num][pp_num++] = pp_hstr_to_code("����");
	}
	/* else if (check_feature(b_ptr->f, "ʣ��:��̾")) {
	    c_ptr->pp[c_ptr->element_num][pp_num++] = pp_hstr_to_code("��");
	} */
	else if (check_feature(b_ptr->f, "����")) {
	    c_ptr->pp[c_ptr->element_num][pp_num++] = pp_hstr_to_code("��");
	}
	else {
	    c_ptr->pp[c_ptr->element_num][pp_num++] = pp_hstr_to_code("��");
	    c_ptr->pp[c_ptr->element_num][pp_num++] = pp_hstr_to_code("��");
	}
	c_ptr->pp[c_ptr->element_num][pp_num] = END_M;
	c_ptr->oblig[c_ptr->element_num] = FALSE;
	return b_ptr;
    }
    else if (check_feature(b_ptr->f, "��:�˳�")) {
	/* �˳ʤǻ��֤ʤ���ֳ� */
	if (check_feature(b_ptr->f, "����")) {
	    c_ptr->pp[c_ptr->element_num][pp_num++] = pp_hstr_to_code("����");
	    c_ptr->oblig[c_ptr->element_num] = FALSE;
	    if (jiritsu_num > 1) {
		c_ptr->pp[c_ptr->element_num][pp_num++] = pp_hstr_to_code("��");
	    }
	}
	else {
	    c_ptr->pp[c_ptr->element_num][pp_num++] = pp_hstr_to_code("��");
	    c_ptr->oblig[c_ptr->element_num] = TRUE;
	}
	c_ptr->pp[c_ptr->element_num][pp_num] = END_M;
	return b_ptr;
    }
    else if (check_feature(b_ptr->f, "��:����")) {
	if (check_feature(b_ptr->f, "����")) {
	    c_ptr->pp[c_ptr->element_num][pp_num++] = pp_hstr_to_code("����");
	    c_ptr->pp[c_ptr->element_num][pp_num++] = pp_hstr_to_code("���");
	    c_ptr->oblig[c_ptr->element_num] = FALSE;
	}
	else {
	    c_ptr->pp[c_ptr->element_num][pp_num++] = pp_hstr_to_code("���");
	    c_ptr->oblig[c_ptr->element_num] = TRUE;
	}
	c_ptr->pp[c_ptr->element_num][pp_num] = END_M;
	return b_ptr;
    }

    else if (check_feature(b_ptr->f, "��:�ǳ�")) {
	c_ptr->pp[c_ptr->element_num][0] = pp_hstr_to_code("��");
	c_ptr->pp[c_ptr->element_num][1] = END_M;
	c_ptr->oblig[c_ptr->element_num] = FALSE;
	if (check_feature(b_ptr->f, "�ǥ�") || 
	    check_feature(b_ptr->f, "�ǥ�")) {
	    c_ptr->pp[c_ptr->element_num][1] = -1;
	    c_ptr->pp[c_ptr->element_num][2] = END_M;
	}
	return b_ptr;
    }
    else if (check_feature(b_ptr->f, "��:�����")) {
	if (check_feature(b_ptr->f, "����")) {
	    c_ptr->pp[c_ptr->element_num][pp_num++] = pp_hstr_to_code("����");
	    if (jiritsu_num > 1) {
		c_ptr->pp[c_ptr->element_num][pp_num++] = pp_hstr_to_code("����");
	    }
	}
	else {
	    c_ptr->pp[c_ptr->element_num][pp_num++] = pp_hstr_to_code("����");
	}
	c_ptr->pp[c_ptr->element_num][pp_num] = END_M;
	c_ptr->oblig[c_ptr->element_num] = FALSE;
	return b_ptr;
    }
    else if (check_feature(b_ptr->f, "��:�ȳ�")) {
	c_ptr->pp[c_ptr->element_num][0] = pp_hstr_to_code("��");
	c_ptr->pp[c_ptr->element_num][1] = END_M;
	c_ptr->oblig[c_ptr->element_num] = FALSE;
	return b_ptr;
    }
    else if (check_feature(b_ptr->f, "��:�ޥǳ�")) {
	if (check_feature(b_ptr->f, "����")) {
	    c_ptr->pp[c_ptr->element_num][pp_num++] = pp_hstr_to_code("����");
	    if (jiritsu_num > 1) {
		c_ptr->pp[c_ptr->element_num][pp_num++] = -1;
	    }
	}
	else {
	    c_ptr->pp[c_ptr->element_num][pp_num++] = pp_hstr_to_code("�ޤ�");
	    c_ptr->pp[c_ptr->element_num][pp_num++] = -1;
	}
	c_ptr->pp[c_ptr->element_num][pp_num] = END_M;
	c_ptr->oblig[c_ptr->element_num] = FALSE;
	return b_ptr;
    }
    else if (check_feature(b_ptr->f, "��:̵��")) {
	/* ̵�ʤǻ��֤ʤ���ֳ� */
	if (check_feature(b_ptr->f, "����")) {
	    c_ptr->pp[c_ptr->element_num][pp_num++] = pp_hstr_to_code("����");
	    if (jiritsu_num > 1) {
		c_ptr->pp[c_ptr->element_num][pp_num++] = pp_hstr_to_code("��");
	    }
	}
	else {
	    c_ptr->pp[c_ptr->element_num][pp_num++] = pp_hstr_to_code("��");
	}
	c_ptr->pp[c_ptr->element_num][pp_num] = END_M;
	c_ptr->oblig[c_ptr->element_num] = FALSE;
	return b_ptr;
    }
    else if (check_feature(b_ptr->f, "��:̤��") || 
	     check_feature(b_ptr->f, "��:Ʊ��̤��")) {
	if (check_feature(b_ptr->f, "����")) {
	    c_ptr->pp[c_ptr->element_num][pp_num++] = pp_hstr_to_code("����");
	    c_ptr->oblig[c_ptr->element_num] = FALSE;
	    if (jiritsu_num > 1) {
		c_ptr->pp[c_ptr->element_num][pp_num++] = -1;
	    }
	}
	else {
	    /* ����ǤϤʤ��Ȥ��ˤ���?
	    if (check_feature(b_ptr->f, "����")) {
		return NULL;
	    } */
	    c_ptr->pp[c_ptr->element_num][pp_num++] = -1;
	    c_ptr->oblig[c_ptr->element_num] = FALSE;
	}
	c_ptr->pp[c_ptr->element_num][pp_num] = END_M;
	return b_ptr;
    }
    /* ��:Ϣ��, ʣ�缭 */
    else if (check_feature(b_ptr->f, "ʣ�缭") && 
	     check_feature(b_ptr->f, "��:Ϣ��") && 
	     b_ptr->child[0]) {
	int fc = pp_hstr_to_code(make_fukugoji_string(b_ptr));
	if (fc == END_M) {
	    return NULL;
	}
	c_ptr->pp[c_ptr->element_num][0] = fc;
	c_ptr->pp[c_ptr->element_num][1] = END_M;
	c_ptr->oblig[c_ptr->element_num] = FALSE;
	return b_ptr->child[0];
    }
    else if (check_feature(b_ptr->f, "��:Ϣ��") &&
	     !check_feature(cpm_ptr->pred_b_ptr->f, "�Ѹ�:Ƚ")) {
	/* �֡�����Ρ�: �����
	   ��ȯŸ�Ӿ�񤫤����Ƚ��: ���� �����ʤ� */
	if (check_feature(b_ptr->f, "����")) {
	    c_ptr->pp[c_ptr->element_num][pp_num++] = pp_hstr_to_code("����");
	    /* ���ֳʤ⤢�뤫�� */
	}
	/* �֡��ȤΡ�: �ȳ� */
	else if (check_feature(b_ptr->f, "��")) {
	    c_ptr->pp[c_ptr->element_num][pp_num++] = pp_hstr_to_code("��");
	}
	/* �֡��ǤΡ�: �ǳ� */
	else if (check_feature(b_ptr->f, "��")) {
	    c_ptr->pp[c_ptr->element_num][pp_num++] = pp_hstr_to_code("��");
	}
	/* �֡��ޤǤΡ�: �ޥǳ� */
	else if (check_feature(b_ptr->f, "�ޥ�")) {
	    c_ptr->pp[c_ptr->element_num][pp_num++] = pp_hstr_to_code("�ޤ�");
	    /* ���ֳʤ⤢�뤫�� */
	}
	/* �֡��ؤΡ�: �س�, �˳� */
	else if (check_feature(b_ptr->f, "��")) {
	    c_ptr->pp[c_ptr->element_num][pp_num++] = pp_hstr_to_code("��");
	    c_ptr->pp[c_ptr->element_num][pp_num++] = pp_hstr_to_code("��");
	}
	else {
	    return NULL;
	}
	c_ptr->pp[c_ptr->element_num][pp_num] = END_M;
	c_ptr->oblig[c_ptr->element_num] = FALSE;
	return b_ptr;
    }
    /* ʣ��̾�� */
    else if (check_feature(b_ptr->f, "��:ʸ����")) {
	/* ���֤ʤ���ֳʤΤߤˤ���
	   �� �ֿ�������: ���� */
	if (check_feature(b_ptr->f, "����")) {
	    c_ptr->pp[c_ptr->element_num][pp_num++] = pp_hstr_to_code("����");
	}
	/* ��̾�ʤ�Ȥꤢ��������/�ǳʤˤ��� */
	else if (check_feature(b_ptr->f, "ʣ��:��̾")) {
	    c_ptr->pp[c_ptr->element_num][pp_num++] = pp_hstr_to_code("��");
	    c_ptr->pp[c_ptr->element_num][pp_num++] = pp_hstr_to_code("��");
	}
	else {
	    c_ptr->pp[c_ptr->element_num][pp_num++] = pp_hstr_to_code("��");
	    c_ptr->pp[c_ptr->element_num][pp_num++] = pp_hstr_to_code("��");
	    c_ptr->pp[c_ptr->element_num][pp_num++] = pp_hstr_to_code("��");
	}
	c_ptr->pp[c_ptr->element_num][pp_num] = END_M;
	c_ptr->oblig[c_ptr->element_num] = FALSE;
	return b_ptr;
    }
    else if (check_feature(b_ptr->f, "��:����") && check_feature(b_ptr->f, "��̾��")) {
	/* �֣�����夲���� */
	if (check_feature(b_ptr->f, "����")) {
	    c_ptr->pp[c_ptr->element_num][pp_num++] = pp_hstr_to_code("����");
	}
	else if (check_feature(b_ptr->f, "����")) {
	    c_ptr->pp[c_ptr->element_num][pp_num++] = pp_hstr_to_code("��");
	}
	else {
	    c_ptr->pp[c_ptr->element_num][pp_num++] = pp_hstr_to_code("��");
	    c_ptr->pp[c_ptr->element_num][pp_num++] = pp_hstr_to_code("��");
	}
	c_ptr->pp[c_ptr->element_num][pp_num] = END_M;
	c_ptr->oblig[c_ptr->element_num] = FALSE;
	return b_ptr;
    }
    /* �֡����Ӥ����ǡ��� �Τ褦��ɽ�� -> ����
    else if (check_feature(b_ptr->f, "ʣ��:��̾") && 
	     check_feature(b_ptr->f, "ID:����") && 
	     check_feature(b_ptr->f, "�Ѹ�:Ƚ")) {
	c_ptr->pp[c_ptr->element_num][pp_num++] = pp_hstr_to_code("��");
	c_ptr->pp[c_ptr->element_num][pp_num] = END_M;
	c_ptr->oblig[c_ptr->element_num] = FALSE;
	return b_ptr;
    } */
    else {
	return NULL;
    }
}

/*==================================================================*/
   void _make_data_cframe_sm(CF_PRED_MGR *cpm_ptr, BNST_DATA *b_ptr)
/*==================================================================*/
{
    int i, sm_num = 0, qua_flag = FALSE, tim_flag = FALSE;
    CASE_FRAME *c_ptr = &(cpm_ptr->cf);

    /* ������ -- ʸ */
    if (check_feature(b_ptr->f, "��ʸ")) {
	strcpy(c_ptr->sm[c_ptr->element_num]+SM_CODE_SIZE*sm_num, 
	       (char *)sm2code("��ʸ"));
	sm_num++;
    }
    /* ���� */
    else if (check_feature(b_ptr->f, "����")) {
	strcpy(c_ptr->sm[c_ptr->element_num]+SM_CODE_SIZE*sm_num, 
	       (char *)sm2code("����"));
	sm_num++;
    }
    else {
	if (check_feature(b_ptr->f, "����")) {
	    strcpy(c_ptr->sm[c_ptr->element_num]+SM_CODE_SIZE*sm_num, 
		   (char *)sm2code("����"));
	    sm_num++;
	}
	if (check_feature(b_ptr->f, "����")) {
	    strcpy(c_ptr->sm[c_ptr->element_num]+SM_CODE_SIZE*sm_num, 
		   (char *)sm2code("����"));
	    sm_num++;
	}
	if (check_feature(b_ptr->f, "��̾") || 
	    check_feature(b_ptr->f, "�ȿ�̾")) {
	    strcpy(c_ptr->sm[c_ptr->element_num]+SM_CODE_SIZE*sm_num, 
		   (char *)sm2code("����"));
	    sm_num++;
	}
	
	/* for (i = 0; i < b_ptr->SM_num; i++) */
	strcpy(c_ptr->sm[c_ptr->element_num]+SM_CODE_SIZE*sm_num, 
	       b_ptr->SM_code);
	sm_num += strlen(b_ptr->SM_code)/SM_CODE_SIZE;
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
	strcpy(c_ptr->ex2[c_ptr->element_num], b_ptr->SM_code);
    }
}

/*==================================================================*/
    void make_data_cframe(SENTENCE_DATA *sp, CF_PRED_MGR *cpm_ptr)
/*==================================================================*/
{
    BNST_DATA *b_ptr = cpm_ptr->pred_b_ptr;
    BNST_DATA *cel_b_ptr;
    int i, j, k, child_num;
    char *vtype = NULL;

    if (vtype = (char *)check_feature(b_ptr->f, "�Ѹ�")) {
	vtype += 5;
	strcpy(cpm_ptr->cf.ipal_id, vtype);
    }
    else {
	cpm_ptr->cf.ipal_id[0] = '\0';
    }

    cpm_ptr->cf.pred_b_ptr = b_ptr;
    b_ptr->cpm_ptr = cpm_ptr;

    /* ɽ�س� etc. ������ */

    cpm_ptr->cf.element_num = 0;
    if (check_feature(b_ptr->f, "��:Ϣ��")) {

	/* para_type == PARA_NORMAL �ϡ�V��,V���� PARA N�פΤȤ�
	   ���ΤȤ��Ͽ�(PARA)�ο�(N)������ǤȤ��롥

	   �Ƥ�para_top_p���ɤ�����ߤƤ��V����N��N PARA�פ�
	   ���ȶ��̤��Ǥ��ʤ�
        */

	if (b_ptr->para_type != PARA_NORMAL) {
	    if (b_ptr->parent) {
		/* ���δط��ʳ��ΤȤ��ϳ����Ǥ� (���δط��Ǥ���ƻ�ΤȤ��ϳ����Ǥˤ���) */
		if (!(check_feature(b_ptr->parent->f, "���δط�") || 
		      check_feature(b_ptr->parent->f, "���δط���ǽ��")) || 
		    check_feature(b_ptr->f, "�Ѹ�:��")) {
		    _make_data_cframe_pp(cpm_ptr, NULL);
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
	} else {
	    cel_b_ptr = b_ptr;
	    while (cel_b_ptr->parent->para_type == PARA_NORMAL) {
		cel_b_ptr = cel_b_ptr->parent;
	    }

	    if (cel_b_ptr->parent && 
		cel_b_ptr->parent->parent) {
		if (!check_feature(cel_b_ptr->parent->parent->f, "���δط�") || 
		    check_feature(b_ptr->f, "�Ѹ�:��")) {
		    _make_data_cframe_pp(cpm_ptr, NULL);
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

    for (child_num=0; b_ptr->child[child_num]; child_num++);
    for (i = child_num - 1; i >= 0; i--) {
	if (cel_b_ptr = _make_data_cframe_pp(cpm_ptr, b_ptr->child[i])) {
	    /* �֤ߤ��󻰸Ĥ򿩤٤�� �ҤȤ�����̾�������ǤȤ���Ȥ�
	       �֤ߤ���򻰸Ŀ��٤�� �ξ��Ϥ��Τޤ�ξ�������Ǥˤʤ�
	     */
	    if (check_feature(cel_b_ptr->f, "����") && 
		(check_feature(cel_b_ptr->f, "��:����") || check_feature(cel_b_ptr->f, "��:���")) && 
		cel_b_ptr->num > 0 && check_feature((sp->bnst_data+cel_b_ptr->num-1)->f, "��:����") && 
		!check_feature((sp->bnst_data+cel_b_ptr->num-1)->f, "����")) {
		_make_data_cframe_sm(cpm_ptr, sp->bnst_data+cel_b_ptr->num-1);
		_make_data_cframe_ex(cpm_ptr, sp->bnst_data+cel_b_ptr->num-1);
		cpm_ptr->elem_b_ptr[cpm_ptr->cf.element_num] = sp->bnst_data+cel_b_ptr->num-1;
		cpm_ptr->cf.adjacent[cpm_ptr->cf.element_num] = FALSE;
	    }
	    else {
		/* ľ���ʤΥޡ��� (��������: ������ľ���Τ�) */
		if (i == 0 && b_ptr->num == b_ptr->child[i]->num+1) {
		    cpm_ptr->cf.adjacent[cpm_ptr->cf.element_num] = TRUE;
		}
		else {
		    cpm_ptr->cf.adjacent[cpm_ptr->cf.element_num] = FALSE;
		}
		_make_data_cframe_sm(cpm_ptr, cel_b_ptr);
		_make_data_cframe_ex(cpm_ptr, cel_b_ptr);
		cpm_ptr->elem_b_ptr[cpm_ptr->cf.element_num] = cel_b_ptr;
	    }
	    cpm_ptr->elem_b_num[cpm_ptr->cf.element_num] = i;
	    cpm_ptr->cf.weight[cpm_ptr->cf.element_num] = 0;
	    cpm_ptr->cf.element_num ++;
	}
	if (cpm_ptr->cf.element_num > CF_ELEMENT_MAX) {
	    cpm_ptr->cf.element_num = 0;
	    return;
	}
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
		if (cel_b_ptr = _make_data_cframe_pp(cpm_ptr, b_ptr->parent->child[i])) {
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

    /* ʣ��̾��ΤȤ� */
    if (b_ptr->internal_num && !check_feature(b_ptr->f, "̾��+������")) {
	/* �Ȥꤢ�����夫�� 2 �Ĥ�η����Ǥ򰷤� */
	_make_data_cframe_pp(cpm_ptr, b_ptr->internal+b_ptr->internal_num-1);
	_make_data_cframe_sm(cpm_ptr, b_ptr->internal+b_ptr->internal_num-1);
	_make_data_cframe_ex(cpm_ptr, b_ptr->internal+b_ptr->internal_num-1);
	cpm_ptr->elem_b_ptr[cpm_ptr->cf.element_num] = b_ptr->internal+b_ptr->internal_num-1;
	cpm_ptr->elem_b_num[cpm_ptr->cf.element_num] = -1;
	cpm_ptr->cf.weight[cpm_ptr->cf.element_num] = 0;
	cpm_ptr->cf.adjacent[cpm_ptr->cf.element_num] = FALSE;
	cpm_ptr->cf.element_num ++;
	if (cpm_ptr->cf.element_num > CF_ELEMENT_MAX) {
	    cpm_ptr->cf.element_num = 0;
	    return;
	}
    }

    /* �����Ǥ��ҤȤĤǻ��ֳʤΤߤξ�硢�����Ǥʤ���Ʊ���褦�˰���
       ��ľ���λ��ֳʤ����̤˰��äƤ�褤�� */
    /* if (cpm_ptr->cf.element_num == 1 && 
	MatchPP(cpm_ptr->cf.pp[0][0], "����") && cpm_ptr->cf.pp[0][1] == END_M) {
	cpm_ptr->cf.element_num = 0;
    } */
    for (i = 0; i < cpm_ptr->cf.element_num; i++) {
	if (!MatchPP(cpm_ptr->cf.pp[i][0], "����")) {
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

    int i;

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
