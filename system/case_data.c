/*====================================================================

			  �ʹ�¤����: ����¦

                                               S.Kurohashi 93. 5.31

    $Id$
====================================================================*/
#include "knp.h"

char fukugoji_string[64];

/*==================================================================*/
	     char *make_fukugoji_string(BNST_DATA *b_ptr)
/*==================================================================*/
{
    int i;

    fukugoji_string[0] = '\0';

    /* ����ʸ��ν��� */
    strcat(fukugoji_string, 
	   ((sp->bnst_data+b_ptr->num-1)->fuzoku_ptr+(sp->bnst_data+b_ptr->num-1)->fuzoku_num-1)->Goi);

    /* ����ʸ�� */
    for (i = 0; i < b_ptr->mrph_num; i++) {
	if ((b_ptr->mrph_ptr+i)->Hinshi == 1)	/* �ü�ʳ� */
	    continue;
	strcat(fukugoji_string, 
	       (b_ptr->mrph_ptr+i)->Goi);
    }

    return fukugoji_string;
}

/*==================================================================*/
BNST_DATA *_make_data_cframe_pp(CF_PRED_MGR *cpm_ptr, BNST_DATA *b_ptr)
/*==================================================================*/
{
    int i, num;
    CASE_FRAME *c_ptr = &(cpm_ptr->cf);

    if (b_ptr == NULL) {	/* ������ʸ���､���� */
	c_ptr->pp[c_ptr->element_num][0] = pp_hstr_to_code("��");
	c_ptr->oblig[c_ptr->element_num] = FALSE;
	return b_ptr;
    }
    else if (check_feature(b_ptr->f, "��:����") || 
	     (!check_feature(cpm_ptr->pred_b_ptr->f, "�Ѹ�:Ƚ") &&
	      check_feature(b_ptr->f, "��:�γ�"))) {
	c_ptr->pp[c_ptr->element_num][0] = pp_hstr_to_code("��");
	c_ptr->oblig[c_ptr->element_num] = TRUE;
	return b_ptr;
    }
    else if (check_feature(b_ptr->f, "��:���")) {
	c_ptr->pp[c_ptr->element_num][0] = pp_hstr_to_code("��");
	c_ptr->oblig[c_ptr->element_num] = TRUE;
	return b_ptr;
    }
    else if (check_feature(b_ptr->f, "��:�س�")) {
	c_ptr->pp[c_ptr->element_num][0] = pp_hstr_to_code("��");
	c_ptr->oblig[c_ptr->element_num] = TRUE;
	return b_ptr;
    }

    else if (check_feature(b_ptr->f, "��:�˳�")) {
	c_ptr->pp[c_ptr->element_num][0] = pp_hstr_to_code("��");
	if (check_feature(b_ptr->f, "����"))
	  c_ptr->oblig[c_ptr->element_num] = FALSE;
	else
	  c_ptr->oblig[c_ptr->element_num] = TRUE;
	return b_ptr;
    }
    else if (check_feature(b_ptr->f, "��:����")) {
	c_ptr->pp[c_ptr->element_num][0] = pp_hstr_to_code("���");
	if (check_feature(b_ptr->f, "����"))
	  c_ptr->oblig[c_ptr->element_num] = FALSE;
	else
	  c_ptr->oblig[c_ptr->element_num] = TRUE;
	return b_ptr;
    }

    else if (check_feature(b_ptr->f, "��:�ǳ�")) {
	c_ptr->pp[c_ptr->element_num][0] = pp_hstr_to_code("��");
	c_ptr->oblig[c_ptr->element_num] = FALSE;
	return b_ptr;
    }
    else if (check_feature(b_ptr->f, "��:�����")) {
	c_ptr->pp[c_ptr->element_num][0] = pp_hstr_to_code("����");
	c_ptr->oblig[c_ptr->element_num] = FALSE;
	return b_ptr;
    }
    else if (check_feature(b_ptr->f, "��:�ȳ�")) {
	c_ptr->pp[c_ptr->element_num][0] = pp_hstr_to_code("��");
	c_ptr->oblig[c_ptr->element_num] = FALSE;
	return b_ptr;
    }
    else if (check_feature(b_ptr->f, "��:�ޥǳ�")) {
	c_ptr->pp[c_ptr->element_num][0] = -1;
	c_ptr->oblig[c_ptr->element_num] = FALSE;
	return b_ptr;
    }
    else if (check_feature(b_ptr->f, "��:̵��")) {
	c_ptr->pp[c_ptr->element_num][0] = pp_hstr_to_code("��");
	c_ptr->oblig[c_ptr->element_num] = FALSE;
	return b_ptr;
    }
    else if (check_feature(b_ptr->f, "��:̤��")) {
	c_ptr->pp[c_ptr->element_num][0] = -1;
	if (check_feature(b_ptr->f, "����"))
	  c_ptr->oblig[c_ptr->element_num] = FALSE;
	else 
	  c_ptr->oblig[c_ptr->element_num] = TRUE;
	return b_ptr;
    }
    else if (check_feature(b_ptr->f, "ʣ�缭") && b_ptr->child[0]) {
	c_ptr->pp[c_ptr->element_num][0] = 
	    pp_hstr_to_code(make_fukugoji_string(b_ptr));
	c_ptr->oblig[c_ptr->element_num] = FALSE;
	return b_ptr->child[0];
    }
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

    if (check_feature(b_ptr->f, "��:�ȳ�") && /* ������ -- ʸ */
	check_feature(b_ptr->f, "�Ѹ�")) {
	strcpy(c_ptr->sm[c_ptr->element_num]+SM_CODE_SIZE*sm_num, 
	       (char *)sm2code("��ʸ"));
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

    strcpy(c_ptr->ex[c_ptr->element_num], b_ptr->BGH_code);
}

/*==================================================================*/
     void make_data_cframe(CF_PRED_MGR *cpm_ptr)
/*==================================================================*/
{
    BNST_DATA *b_ptr = cpm_ptr->pred_b_ptr;
    BNST_DATA *cel_b_ptr;
    int i, j, k, child_num;
    
    /* ɽ�س� etc. ������ */

    cpm_ptr->cf.element_num = 0;
    if (check_feature(b_ptr->f, "��:Ϣ��")) {

	/* para_type == PARA_NORMAL �ϡ�V��,V���� PARA N�פΤȤ�
	   ���ΤȤ��Ͽ�(PARA)�ο�(N)������ǤȤ��롥

	   �Ƥ�para_top_p���ɤ�����ߤƤ��V����N��N PARA�פ�
	   ���ȶ��̤��Ǥ��ʤ�
        */

	if (b_ptr->para_type != PARA_NORMAL) {
	    if (b_ptr->parent && 
		!check_feature(b_ptr->parent->f, "���δط�")) {

		_make_data_cframe_pp(cpm_ptr, NULL);
		_make_data_cframe_sm(cpm_ptr, b_ptr->parent);
		_make_data_cframe_ex(cpm_ptr, b_ptr->parent);
		cpm_ptr->elem_b_ptr[cpm_ptr->cf.element_num] = b_ptr->parent;
		cpm_ptr->elem_b_num[cpm_ptr->cf.element_num] = -1;
		cpm_ptr->cf.element_num ++;
	    }
	} else {
	    if (b_ptr->parent && 
		b_ptr->parent->parent && 
		!check_feature(b_ptr->parent->parent->f, "���δط�")) {

		_make_data_cframe_pp(cpm_ptr, NULL);
		_make_data_cframe_sm(cpm_ptr, b_ptr->parent->parent);
		_make_data_cframe_ex(cpm_ptr, b_ptr->parent->parent);
		cpm_ptr->elem_b_ptr[cpm_ptr->cf.element_num] = b_ptr->parent->parent;
		cpm_ptr->elem_b_num[cpm_ptr->cf.element_num] = -1;
		cpm_ptr->cf.element_num ++;
	    }
	}
    }

    for (child_num=0; b_ptr->child[child_num]; child_num++);
    for (i = child_num - 1; i >= 0; i--) {
	if (cel_b_ptr = _make_data_cframe_pp(cpm_ptr, b_ptr->child[i])) {
	    _make_data_cframe_sm(cpm_ptr, cel_b_ptr);
	    _make_data_cframe_ex(cpm_ptr, cel_b_ptr);
	    cpm_ptr->elem_b_ptr[cpm_ptr->cf.element_num] = cel_b_ptr;
	    cpm_ptr->elem_b_num[cpm_ptr->cf.element_num] = i;
	    cpm_ptr->cf.element_num ++;
	}
	if (cpm_ptr->cf.element_num > CF_ELEMENT_MAX) {
	    cpm_ptr->cf.element_num = 0;
	    return;
	}
    }
}

/*==================================================================*/
     void set_pred_voice(BNST_DATA *b_ptr)
/*==================================================================*/
{
    /* �������������� */ /* ������ ����ɬ�� ������ */

    int i;
    b_ptr->voice = NULL;

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



