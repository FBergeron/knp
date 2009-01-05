/*====================================================================

			      �ڹ�¤����

                                               S.Kurohashi 92.10.17
                                               S.Kurohashi 93. 5.31

    $Id$
====================================================================*/
#include "knp.h"

/*==================================================================*/
	   void init_bnst_tree_property(SENTENCE_DATA *sp)
/*==================================================================*/
{
    int i;

    for (i = 0; i < BNST_MAX; i++) {
	sp->bnst_data[i].parent = NULL;
	sp->bnst_data[i].child[0] = NULL;
	sp->bnst_data[i].para_top_p = FALSE;
	sp->bnst_data[i].para_type = PARA_NIL;
	sp->bnst_data[i].to_para_p = FALSE;
    }
}

/*==================================================================*/
	    void init_tag_tree_property(SENTENCE_DATA *sp)
/*==================================================================*/
{
    int i;

    for (i = 0; i < TAG_MAX; i++) {
	sp->tag_data[i].parent = NULL;
	sp->tag_data[i].child[0] = NULL;
	sp->tag_data[i].para_top_p = FALSE;
	sp->tag_data[i].para_type = PARA_NIL;
	sp->tag_data[i].to_para_p = FALSE;
    }
}

/*==================================================================*/
	   void init_mrph_tree_property(SENTENCE_DATA *sp)
/*==================================================================*/
{
    int i;

    for (i = 0; i < MRPH_MAX; i++) {
	sp->mrph_data[i].parent = NULL;
	sp->mrph_data[i].child[0] = NULL;
	sp->mrph_data[i].para_top_p = FALSE;
	sp->mrph_data[i].para_type = PARA_NIL;
	sp->mrph_data[i].to_para_p = FALSE;
    }
}

/*==================================================================*/
BNST_DATA *t_add_node(BNST_DATA *parent, BNST_DATA *child, int pos)
/*==================================================================*/
{
    int i, child_num;

    for (child_num = 0; parent->child[child_num]; child_num++)
	;

    if (pos == -1) {
	parent->child[child_num] = child;
	parent->child[child_num + 1] = NULL;
    }
    else {
	for (i = child_num; i >= pos; i--)
	    parent->child[i + 1] = parent->child[i];
	parent->child[pos] = child;
    }

    return parent;
}

/*==================================================================*/
BNST_DATA *t_attach_node(BNST_DATA *parent, BNST_DATA *child, int pos)
/*==================================================================*/
{
    child->parent = parent;

    return t_add_node(parent, child, pos);
}
/*==================================================================*/
    BNST_DATA *t_del_node(BNST_DATA *parent, BNST_DATA *child)
/*==================================================================*/
{
    int i, j;
    
    for (i = 0; parent->child[i]; i++) {
	if (parent->child[i] == child) {
	    for (j = i; parent->child[j]; j++)
	      parent->child[j] = parent->child[j + 1];
	    break;
	}
    }

    child->parent = NULL;
    
    return child;
}

/*==================================================================*/
	       int make_simple_tree(SENTENCE_DATA *sp)
/*==================================================================*/
{
    int i, j, k, child_num, pre_node_child_num;
    int buffer[BNST_MAX];
    BNST_DATA *tmp_b_ptr;

    /* dpnd.head[i]��buffer[i]�˥��ԡ�����3�İʾ夫��ʤ�����¤�Ǥ�
       ������٤�head����������head���ѹ����롥
       �ޤ���ʬ����η������������head���ѹ����롥*/

    for (i = 0; i < sp->Bnst_num - 1; i++)
	buffer[i] = sp->bnst_data[i].dpnd_head;

    for (i = 0; i < sp->Para_M_num; i++) {
	for (j = 0; j < sp->para_manager[i].part_num - 1; j++) {
	    buffer[sp->para_manager[i].end[j]] = 
		sp->para_manager[i].end[sp->para_manager[i].part_num - 1];

	    /*
	    printf(">>> (%d,%d) %d -> %d\n", i, j, sp->para_manager[i].end[j],
		   sp->para_manager[i].end[sp->para_manager[i].part_num - 1]);
	    */

	    for (k = sp->para_manager[i].start[j];
		 k <= sp->para_manager[i].end[j]; k++)
		if (Mask_matrix[k][sp->para_manager[i].end[j]] == 3) 
		    buffer[k] = 
			sp->para_manager[i].end[sp->para_manager[i].part_num - 1];
	}
    }

    /* ��¸��¤�ڹ�¤����դ� */

    pre_node_child_num = 0;
    for (j = sp->Bnst_num - 1; j >= 0; j--) { /* ����¦ */
	if (pre_node_child_num != 0) {
	    child_num = pre_node_child_num;
	    pre_node_child_num = 0;
	}
	else {
	    child_num = 0;
	}
	for (i = j - 1; i >= 0; i--) { /* ����¦ */
	    if (sp->bnst_data[i].num == -1) {
		continue; /* ������ǥޡ������줿�Ρ��� */
	    }
	    if (buffer[i] == j) { /* i -> j */
		if (sp->bnst_data[j].num == -1) { /* ������ǥޡ������줿�Ρ��� */
		    if (j - i == 1) { /* �ޡ���¦ -> �ޡ������줿¦: �����å� */
			continue;
		    }
		    else { /* �ޡ������줿�Ρ��ɤ˷���Ρ��� (ľ���ʳ�) */
			sp->bnst_data[j - 1].child[pre_node_child_num++] = sp->bnst_data + i;
		    }
		}
		else {
		    sp->bnst_data[j].child[child_num++] = sp->bnst_data + i;
		}
		if (child_num >= PARA_PART_MAX) {
		    child_num = PARA_PART_MAX-1;
		    break;
		}
		sp->bnst_data[i].parent = sp->bnst_data[j].num == -1 ? sp->bnst_data + j - 1 : sp->bnst_data + j; /* ������ǥޡ������줿�Ρ��ɤʤ�� -1 */
		if (Mask_matrix[i][j] == 3) {
		    sp->bnst_data[i].para_type = PARA_INCOMP;
		}
		/* PARA_NORMAL��Ÿ�����˥��å� */
	    }
	}
	sp->bnst_data[j].child[child_num] = NULL;
    }

    /* �Ҷ���sort */
    for (j = sp->Bnst_num - 1; j >= 0; j--) {
	for (child_num = 0; sp->bnst_data[j].child[child_num]; child_num++) {
	    ;
	}
	if (child_num < 2) { /* 2�İʾ�Τ� */
	    continue;
	}
	for (i = 0; i < child_num - 1; i++) {
	    for (k = i + 1; k < child_num; k++) {
		if (sp->bnst_data[j].child[i]->num < sp->bnst_data[j].child[k]->num) {
		    tmp_b_ptr = sp->bnst_data[j].child[i];
		    sp->bnst_data[j].child[i] = sp->bnst_data[j].child[k];
		    sp->bnst_data[j].child[k] = tmp_b_ptr;
		}
	    }
	}
    }

    return TRUE;
}

/*==================================================================*/
BNST_DATA *strong_corr_node(SENTENCE_DATA *sp, PARA_DATA *p_ptr, BNST_DATA *b_ptr)
/*==================================================================*/
{
    int i;

    for (i = p_ptr->jend_pos - p_ptr->key_pos - 1; i >= 0; i--) {
	if (sp->bnst_data + p_ptr->max_path[i] == b_ptr)
	  return sp->bnst_data + p_ptr->key_pos + i + 1;
    }
    return NULL;
}

/*==================================================================*/;
   void strong_para_expand(SENTENCE_DATA *sp, PARA_MANAGER *m_ptr)
/*==================================================================*/
{
    int i, j, k;
    PARA_DATA *p_ptr, *pp_ptr;
    BNST_DATA *start_b_ptr, *b_ptr, *bb_ptr;

    /* ��������˷���ʸ���Ÿ�� : ���ԡ�̵ */

    for (i = 0; i < m_ptr->child_num; i++)
      strong_para_expand(sp, m_ptr->child[i]);


    p_ptr = sp->para_data + m_ptr->para_data_num[0];
    if (p_ptr->status == 's') {
	start_b_ptr = sp->bnst_data + m_ptr->start[0];
	for (i = m_ptr->start[0], b_ptr = start_b_ptr; i < m_ptr->end[0]; 
	     i++, b_ptr++)
	    for (j = 0; b_ptr->child[j]; j++)
		if (b_ptr->child[j] < start_b_ptr) {
		    b_ptr->child[j]->to_para_p = TRUE;
		    bb_ptr = b_ptr;
		    for (k = 0, pp_ptr = p_ptr; k < m_ptr->para_num; k++, pp_ptr++)
			if ((bb_ptr = strong_corr_node(sp, pp_ptr, bb_ptr)))
			    t_add_node(bb_ptr, b_ptr->child[j], -1);
		}
    }
}

/*==================================================================*/
  int get_correct_postprocessed_bnst_num(SENTENCE_DATA *sp, int num)
/*==================================================================*/
{
    int i;

    for (i = num; i >= 0; i--) {
	if ((sp->bnst_data + i)->num != -1) {
	    return i;
	}
    }
    return i;
}

/*==================================================================*/
     int para_top_expand(SENTENCE_DATA *sp, PARA_MANAGER *m_ptr)
/*==================================================================*/
{
    int i;
    BNST_DATA *new_ptr, *end_ptr, *pre_end_ptr;

    /* �����ޤȤ��Ρ��ɤ�����

       B  B<P> B  B<P>(end_ptr) B  B �� .... B(new_ptr)
                ��                  (�������ޤǤ��̾��ʸ��)
                ��
                B(new_ptr) �򤳤��������� B<P>(end_ptr)�����Ƥ򥳥ԡ�
		B<P>(end_ptr) �� PARA(�����ޤȤ��Ρ���)�Ȥʤ�
    */

    for (i = 0; i < m_ptr->child_num; i++) {
	if (para_top_expand(sp, m_ptr->child[i]) == FALSE) {
	    return FALSE;
	}
    }

    end_ptr = sp->bnst_data + get_correct_postprocessed_bnst_num(sp, m_ptr->end[m_ptr->part_num - 1]);
    pre_end_ptr = sp->bnst_data + get_correct_postprocessed_bnst_num(sp, m_ptr->end[m_ptr->part_num - 2]);
	
    new_ptr = sp->bnst_data + sp->Bnst_num + sp->New_Bnst_num;
    sp->New_Bnst_num++;
    if ((sp->Bnst_num + sp->New_Bnst_num) > BNST_MAX) {
	fprintf(stderr, ";; Too many nodes in expanding para top .\n");
	return FALSE;
    }
    if (sp->Max_New_Bnst_num < sp->New_Bnst_num) {
	sp->Max_New_Bnst_num = sp->New_Bnst_num;
    }
    *new_ptr = *end_ptr;	/* ���ԡ� */

    /*
      new_ptr �� end_ptr �򥳥ԡ������������f(feature�ؤΥݥ���)����
      f�μ��Τ��ݥ���Ȥ��졤free����ݤ�����Ȥʤ롥�����,

      	end_ptr->f = NULL;
	
      �Ȥ����н褷�Ƥ���������Ǥ�����������ʸ�᤬���θ���Ѹ��γʲ��Ϥ�
      �����ǤȤߤʤ���ʤ����ޤ����٤�make_tree��Ԥ�f���ʤ��ʤäƤ��ޤ�
      �ʤɤ����꤬���ä��Τǡ�clear_feature��ʸ���Ȥβ��ϤΥ롼�פ�
      ��Ƭ�ǹԤ��褦�˽������� (98/02/07)
    */

    /* �ҥΡ��ɤ����� */

    new_ptr->child[0] = NULL;
    t_attach_node(end_ptr, new_ptr, 0);
    while (pre_end_ptr < end_ptr->child[1] && 
	   end_ptr->child[1]->para_type != PARA_INCOMP)
	t_attach_node(new_ptr, t_del_node(end_ptr, end_ptr->child[1]), -1);

    /* �ե饰(PARA,<P>)������ */

    end_ptr->para_type = PARA_NIL;
    end_ptr->para_top_p = TRUE;
    new_ptr->para_type = PARA_NORMAL;
    for (i = 0; i < m_ptr->part_num - 1; i++)
	(sp->bnst_data + get_correct_postprocessed_bnst_num(sp, m_ptr->end[i]))->para_type = PARA_NORMAL;

    return TRUE;
}

/*==================================================================*/
	      void para_modifier_expand(BNST_DATA *b_ptr)
/*==================================================================*/
{
    int i, j, k;
    
    /* PARA �˷��äƤ���Ρ��ɤ� <P> �˷����� : ���ԡ�̵ */

    if (b_ptr->para_top_p == TRUE) {
	for ( i=0; b_ptr->child[i]; i++ ) {
	    if (b_ptr->child[i]->para_type == PARA_NIL && 
		!check_feature(b_ptr->child[i]->f, "��:Ϣ��")) {

		/* b_ptr->child[i] ����ʸ�� */

		b_ptr->child[i]->to_para_p = TRUE;

		for ( j=0; b_ptr->child[j]; j++ ) {
		    if (b_ptr->child[j]->para_type == PARA_NORMAL) {

			/* b_ptr->child[j] <P>ʸ�� */
			
			for ( k=0; b_ptr->child[j]->child[k]; k++ );
			b_ptr->child[j]->child[k] = b_ptr->child[i];
			b_ptr->child[j]->child[k+1] = NULL;
		    }
		}
		b_ptr->child[i] = NULL;
	    }
	}
    }

    for ( i=0; b_ptr->child[i]; i++ )
	para_modifier_expand(b_ptr->child[i]);
}

/*==================================================================*/
   void incomplete_para_expand(SENTENCE_DATA *sp, BNST_DATA *b_ptr)
/*==================================================================*/
{
    int i, j, para_pos;
    int new_num, child_num;
    BNST_DATA *para_ptr, *new_ptr;
    BNST_DATA *pre_childs[10], *pos_childs[10];

    /* ��ʬ�����Ÿ�� : ���ԡ�ͭ(�Ҹ줬���ǡ�������ȤνҸ��PARA��) */

    para_pos = -1;

    for ( i=0; b_ptr->child[i]; i++ )
	if (b_ptr->child[i]->para_top_p == TRUE)
	    for ( j=0; b_ptr->child[i]->child[j]; j++ )
		if (b_ptr->child[i]->child[j]->para_type == PARA_INCOMP) {
		    para_pos = i;
		    break;
		}

    if (para_pos != -1) {

	/* ��Ȥν������Ǥ򥹥ȥå� */

	for ( i=0; b_ptr->child[i] && i < para_pos; i++ )
	    pre_childs[i] = b_ptr->child[i];
	pre_childs[i] = NULL;
	for ( i=para_pos+1, j = 0; b_ptr->child[i]; i++, j++ )
	    pos_childs[j] = b_ptr->child[i];
	pos_childs[j] = NULL;

	para_ptr = b_ptr->child[para_pos];

	new_num = 0;
	for ( i=0; para_ptr->child[i]; i++ )
	    if (para_ptr->child[i]->para_type == PARA_NORMAL) {
	      
		new_ptr = sp->bnst_data + sp->Bnst_num + sp->New_Bnst_num;
		sp->New_Bnst_num++;
		if ((sp->Bnst_num + sp->New_Bnst_num) > BNST_MAX) {
		    fprintf(stderr, 
			    ";; Too many nodes in expanding incomplete para .\n");
		    exit(1);
		}
		*new_ptr = *b_ptr;		/* ���ԡ� */
		new_ptr->f = NULL;		/* ��ա��� �������ʤ��ȸ��SF */

		new_ptr->parent = b_ptr;		/* ���Ρ��ɤο�(��ʬ����) */

		b_ptr->child[new_num] = new_ptr; 	/* ���Ρ��ɤ� PARA */

		/* �������Ρ��ɤλҤ�����
		   (���ν����Ρ��ɡ�<P>��<I>, ���ν����Ρ���) */

		child_num = 0;
		for (j=0; pre_childs[j]; j++)
		    new_ptr->child[child_num++] = pre_childs[j];
		new_ptr->child[child_num++] = para_ptr->child[i];
		while (para_ptr->child[i+1] &&
		       para_ptr->child[i+1]->para_type == PARA_INCOMP) {
		    new_ptr->child[child_num++] = para_ptr->child[i+1];
		    i++;
		}
		for (j=0; pos_childs[j]; j++)
		    new_ptr->child[child_num++] = pos_childs[j];
		new_ptr->child[child_num++] = NULL;

		new_ptr++;
		new_num++;
	    }
	b_ptr->child[new_num] = NULL;
	b_ptr->para_top_p = TRUE;
    }

    for ( i=0; b_ptr->child[i]; i++ )
      incomplete_para_expand(sp, b_ptr->child[i]);
}

/*==================================================================*/
		int make_dpnd_tree(SENTENCE_DATA *sp)
/*==================================================================*/
{
    int i;

    init_bnst_tree_property(sp);

    sp->New_Bnst_num = 0;    				/* ����� */

    if (make_simple_tree(sp) == FALSE) {		/* ����դ� */
	return FALSE;
    }
	
    if (OptExpandP == TRUE) {
	for (i = 0; i < sp->Para_M_num; i++) {		/* �������Ÿ�� */
	    if (sp->para_manager[i].parent == NULL) {
		strong_para_expand(sp, sp->para_manager + i);
	    }
	}
    }

    for (i = 0; i < sp->Para_M_num; i++) {		/* PARA��Ÿ�� */
	if (sp->para_manager[i].parent == NULL) {
	    if (para_top_expand(sp, sp->para_manager + i) == FALSE) {
		return FALSE;
	    }
	}
    }

    if (OptExpandP == TRUE) {
	para_modifier_expand(sp->bnst_data + sp->Bnst_num - 1);	/* PARA������Ÿ�� */
    }

    /*
    incomplete_para_expand(sp->bnst_data + sp->Bnst_num - 1);*/	/* ��ʬ�����Ÿ�� */

    return TRUE;
}

/*==================================================================*/
	  void para_info_to_tag(BNST_DATA *bp, TAG_DATA *tp)
/*==================================================================*/
{
    tp->para_num = bp->para_num;
    tp->para_key_type = bp->para_key_type;
    tp->para_top_p = bp->para_top_p;
    tp->para_type = bp->para_type;
    tp->to_para_p = bp->to_para_p;
}

/*==================================================================*/
	 void para_info_to_mrph(BNST_DATA *bp, MRPH_DATA *mp)
/*==================================================================*/
{
    mp->para_num = bp->para_num;
    mp->para_key_type = bp->para_key_type;
    mp->para_top_p = bp->para_top_p;
    mp->para_type = bp->para_type;
    mp->to_para_p = bp->to_para_p;
}

/*==================================================================*/
    int find_head_tag_from_bnst(BNST_DATA *bp, int target_offset)
/*==================================================================*/
{
    int offset = 0, gov;
    char *cp, *cp2;

    if ((cp = check_feature(bp->f, "����ñ�̼�")) ||
	(cp = check_feature(bp->f, "ľ��������"))) {
	if ((cp2 = strchr(cp, ':'))) {
	    offset = atoi(cp2 + 1);
	    if (offset > 0 || bp->tag_num <= -1 * offset) {
		offset = 0;
	    }
	}
    }

    for (gov = bp->tag_num - 1 + offset; gov >= 0; gov--) {
	if ((bp->tag_ptr + gov)->num != -1) {
	    if (target_offset <= 0) {
		break;
	    }
	    else {
		target_offset--;
	    }
	}
    }
    return gov;
}

/*==================================================================*/
	   int find_head_tag_from_dpnd_bnst(BNST_DATA *bp)
/*==================================================================*/
{
    int offset = 0, gov;
    char *cp, *cp2;

    /* �֥���ñ�̼�̵��פΤȤ��Ϸ������Ǹ�Υ���ñ�̤Ȥ��� */
    if (!check_feature(bp->f, "����ñ�̼�̵��") && 
	((cp = check_feature(bp->parent->f, "����ñ�̼�")) ||
	 (cp = check_feature(bp->parent->f, "ľ��������")))) {
	if ((cp2 = strchr(cp, ':'))) {
	    offset = atoi(cp2 + 1);
	    if (offset > 0 || bp->parent->tag_num <= -1 * offset) {
		offset = 0;
	    }
	}
    }

    for (gov = bp->parent->tag_num - 1 + offset; gov >= 0; gov--) {
	if ((bp->parent->tag_ptr + gov)->num != -1) {
	    break;
	}
    }
    return gov;
}

/*==================================================================*/
MRPH_DATA *find_head_mrph_from_dpnd_bnst(BNST_DATA *dep_ptr, BNST_DATA *gov_ptr)
/*==================================================================*/
{
    BNST_DATA *bp;

    /* �������Ƚ��줬���ꡢ���긵��Ϣ�Ѥʤ顢����������Ǥ�缭̾��ǤϤʤ�Ƚ���ˤ��� */
    if (dep_ptr && 
	gov_ptr->head_ptr + 1 <= gov_ptr->mrph_ptr + gov_ptr->mrph_num - 1 && /* �缭�����Ǥμ��η����Ǥ�¸�� */
	!strcmp(Class[(gov_ptr->head_ptr + 1)->Hinshi][0].id, "Ƚ���") && /* ���η����Ǥ�Ƚ��� */
	!(check_feature(dep_ptr->f, "Ϣ�ν���") || 
	  check_feature(dep_ptr->f, "��:��") || 
	  check_feature(dep_ptr->f, "��:ʸ����") || 
	  check_feature(dep_ptr->f, "�¥�:̾"))) {
//	!(dep_ptr->para_type == PARA_NIL || /* ����ΤȤ��ϺǸ夫��2���ܤ����ǤΤ߽��� */
//	  ((bp = (BNST_DATA *)search_nearest_para_child((TAG_DATA *)dep_ptr->parent)) && dep_ptr->num == bp->num))) {
	return gov_ptr->head_ptr + 1;
    }
    else {
	return gov_ptr->head_ptr;
    }
}

/*==================================================================*/
	       int bnst_to_tag_tree(SENTENCE_DATA *sp)
/*==================================================================*/
{
    int i, j, offset, last_b_flag = 1, gov, head, gov_head, pre_bp_num;
    char *cp;
    BNST_DATA *bp;
    TAG_DATA *tp;

    /* ʸ����ڹ�¤���饿��ñ�̤��ڹ�¤���Ѵ� */

    init_tag_tree_property(sp);
    sp->New_Tag_num = 0;

    /* new bnst -> tag */
    for (i = sp->New_Bnst_num - 1; i >= 0; i--) { /* <PARA>(1)-<PARA>(2) �ΤȤ��Τ���˸夫�餹�� */
	bp = sp->bnst_data + sp->Bnst_num + i;

	/* new�ΰ��copy */

	if ((head = find_head_tag_from_bnst(bp, 0)) < 0) { /* �缭���ܶ� */
	    head = bp->tag_num - 1;
	}
	*(sp->tag_data + sp->Tag_num + sp->New_Tag_num) = *(bp->tag_ptr + head);
	sp->New_Tag_num++;

	tp = sp->tag_data + sp->Tag_num + sp->New_Tag_num - 1; /* New�ΰ�˥��ԡ������缭���ܶ�ؤΥݥ��� */

	para_info_to_tag(bp, tp);
	tp->child[0] = NULL;

	/* <PARA>�ΤȤ���head�Τ� */
	if (bp->para_top_p == FALSE) {
	    /* ʸ����μ缭���ܶ�����¦ */
	    if (head > 0 && (pre_bp_num = find_head_tag_from_bnst(bp, 1)) >= 0) {
		/* ʸ���⥿��ñ�̤οƤ� <P>(-<PARA>) �ΤȤ� */
		(bp->tag_ptr + pre_bp_num)->parent = tp; /* �缭�ΤҤȤ��� -> �缭 */
		t_add_node((BNST_DATA *)tp, 
			   (BNST_DATA *)(bp->tag_ptr + pre_bp_num), -1);

		/* ʸ���� */
		for (j = 0; j < pre_bp_num; j++) {
		    for (gov = j + 1; gov <= pre_bp_num; gov++) {
			if ((bp->tag_ptr + gov)->num != -1) {
			    break;
			}
		    }
		    if (gov > pre_bp_num || /* ������Ƿ����褬�ʤ��ʤä����ܶ� */
			(bp->tag_ptr + j)->num == -1) { /* ������ǥޡ������줿���ܶ� */
			continue;
		    }
		    (bp->tag_ptr + j)->parent = bp->tag_ptr + gov;
		    t_add_node((BNST_DATA *)(bp->tag_ptr + gov), 
			       (BNST_DATA *)(bp->tag_ptr + j), -1);
		}
		/* �缭���ܶ�� bp->tag_ptr ����Ϥ��ɤ�ʤ� (New����) */
	    }
	}

	/* �ƤȻҤΥ�󥯤Ĥ� (new) */
	gov_head = find_head_tag_from_dpnd_bnst(bp); /* ������μ缭���ܶ� */
	tp->parent = bp->parent->tag_ptr + gov_head; /* PARA�� */
	t_add_node((BNST_DATA *)(bp->parent->tag_ptr + gov_head), 
		   (BNST_DATA *)tp, -1);

	/* ʸ����μ缭���ܶ���� (PARA����Ĥ�δ��ܶ��) */
	if (bp->parent < sp->bnst_data + sp->Bnst_num) { /* �Ƥ�New�ΤȤ��Ϥ��Ǥ����ꤷ�Ƥ��� */
	    tp = bp->parent->tag_ptr + gov_head;
	    for (j = head + 1; j < bp->tag_num; j++) {
		if ((bp->tag_ptr + j)->num == -1) {
		    continue;
		}
		tp->parent = bp->tag_ptr + j;
		t_add_node((BNST_DATA *)(bp->tag_ptr + j), 
			   (BNST_DATA *)tp, -1);
		tp = bp->tag_ptr + j;
	    }
	    tp->parent = NULL; /* ������̤��Υޡ��� */
	}

	/* PARA�ޤ��ϴ��ܶ�1�ĤΤȤ��ϡ�tag_ptr��New¦�ˤ��Ƥ��� */
	if (1 || bp->para_top_p == TRUE || bp->tag_num == 1) {
	    bp->tag_ptr = sp->tag_data + sp->Tag_num + sp->New_Tag_num - 1;
	    bp->tag_num = 1;
	}
    }

    /* orig */
    for (i = sp->Bnst_num - 1; i >= 0; i--) {
	bp = sp->bnst_data + i;
	if (bp->num == -1) { /* ������ǥޡ������줿ʸ�� */
	    continue;
	}

	if ((head = find_head_tag_from_bnst(bp, 0)) < 0) { /* �缭���ܶ� */
	    head = bp->tag_num - 1;
	}
	para_info_to_tag(bp, bp->tag_ptr + head);

	/* <PARA>�ΤȤ���head�Τߤ�����tag_ptr, tag_num���ѹ��Ϥ��ʤ� */
	if (bp->para_top_p == FALSE) {
	    /* ʸ���� */
	    for (j = 0; j < bp->tag_num - 1; j++) {
		for (gov = j + 1; gov < bp->tag_num; gov++) {
		    if ((bp->tag_ptr + gov)->num != -1) {
			break;
		    }
		}
		if (gov >= bp->tag_num || /* ������Ƿ����褬�ʤ��ʤä����ܶ� */
		    (bp->tag_ptr + j)->num == -1) { /* ������ǥޡ������줿���ܶ� */
		    continue;
		}
		(bp->tag_ptr + j)->parent = bp->tag_ptr + gov;
		t_add_node((BNST_DATA *)(bp->tag_ptr + gov), 
			   (BNST_DATA *)(bp->tag_ptr + j), -1);
	    }
	}

	if (last_b_flag) { /* �Ǹ��ʸ�� (�����������Τ� i == Bnst_num - 1 �Ȥϸ¤�ʤ�) */
	    last_b_flag = 0;
	    continue;
	}

	/* �ƤȻ� */
	if (bp->parent) {
	    for (head = bp->tag_num - 1; head >= 0; head--) { /* �Ǹ�δ��ܶ�򤵤��� */
		if ((bp->tag_ptr + head)->num != -1) {
		    break;
		}
	    }
	    tp = bp->tag_ptr + head;
	    if (bp->para_top_p == TRUE) { /* PARA�ξ���new�����Ǿ����������Ƥ����礬���� */
		while (tp->parent) {
		    tp = tp->parent;
		}
	    }

	    offset = find_head_tag_from_dpnd_bnst(bp); /* ����ñ����η������롼�뤫������ */
	    tp->parent = bp->parent->tag_ptr + offset;
	    t_add_node((BNST_DATA *)(bp->parent->tag_ptr + offset), 
		       (BNST_DATA *)tp, -1);
	}
	else {
	    if (Language != CHINESE) {
		fprintf(stderr, ";; %s(%d)'s parent doesn't exist!\n", bp->Jiritu_Go, i);
	    }
	}
    }
}

/*==================================================================*/
	       int bnst_to_mrph_tree(SENTENCE_DATA *sp)
/*==================================================================*/
{
    int i, j, offset, last_b_flag = 1, gov, head, gov_head, pre_bp_num;
    char *cp;
    BNST_DATA *bp;
    MRPH_DATA *mp, *tmp_mp, *head_ptr;

    /* ʸ����ڹ�¤��������Ǥ��ڹ�¤���Ѵ� */

    init_mrph_tree_property(sp);
    sp->New_Mrph_num = 0;

    /* new bnst -> tag */
    for (i = sp->New_Bnst_num - 1; i >= 0; i--) { /* <PARA>(1)-<PARA>(2) �ΤȤ��Τ���˸夫�餹�� */
	bp = sp->bnst_data + sp->Bnst_num + i;
	// head_ptr = bp->mrph_ptr + bp->mrph_num - 1; // bp->head_ptr; /* ���缭�����ǡ� */
	head_ptr = find_head_mrph_from_dpnd_bnst(NULL, bp); /* �缭������ */

	/* new�ΰ��copy */

	*(sp->mrph_data + sp->Mrph_num + sp->New_Mrph_num) = *head_ptr; /* �缭������ */
	sp->New_Mrph_num++;
	mp = sp->mrph_data + sp->Mrph_num + sp->New_Mrph_num - 1; /* New�ΰ�˥��ԡ������缭�����ǤؤΥݥ��� */

	para_info_to_mrph(bp, mp);
	mp->child[0] = NULL;

	/* <PARA>�ΤȤ���head�Τ� */
	if (bp->para_top_p == FALSE) {
	    /* ʸ����μ缭�����Ǥ����¦ */
	    if (head_ptr > bp->mrph_ptr) {
		/* ʸ��������ǤοƤ� <P>(-<PARA>) �ΤȤ� */
		(head_ptr - 1)->parent = (BNST_DATA *)mp; /* �缭�ΤҤȤ��� -> �缭 */
		t_add_node((BNST_DATA *)mp, 
			   (BNST_DATA *)(head_ptr - 1), -1);

		/* ʸ���� */
		for (tmp_mp = head_ptr - 2; tmp_mp >= bp->mrph_ptr; tmp_mp--) {
		    tmp_mp->parent = (BNST_DATA *)(tmp_mp + 1);
		    t_add_node((BNST_DATA *)(tmp_mp + 1), 
			       (BNST_DATA *)tmp_mp, -1);
		}
	    }
	}

	/* �ƤȻҤΥ�󥯤Ĥ� (new) */
	mp->parent = (BNST_DATA *)find_head_mrph_from_dpnd_bnst(bp, bp->parent);  /* ������μ缭������ (PARA��) */
	t_add_node((BNST_DATA *)(mp->parent), 
		   (BNST_DATA *)mp, -1);

	/* ʸ����μ缭�����Ǥ��� (PARA����Ĥ�δ��ܶ��) */
	if (bp->parent < sp->bnst_data + sp->Bnst_num) { /* �Ƥ�New�ΤȤ��Ϥ��Ǥ����ꤷ�Ƥ��� */
	    mp = (MRPH_DATA *)mp->parent; /* PARA */
	    for (tmp_mp = head_ptr + 1; tmp_mp < bp->mrph_ptr + bp->mrph_num; tmp_mp++) {
		mp->parent = (BNST_DATA *)tmp_mp;
		t_add_node((BNST_DATA *)(tmp_mp), 
			   (BNST_DATA *)mp, -1);
		mp = tmp_mp;
	    }
	    mp->parent = NULL; /* ������̤��Υޡ��� */
	}

	/* mrph_ptr��New¦�ˤ��Ƥ��� */
	bp->mrph_ptr = sp->mrph_data + sp->Mrph_num + sp->New_Mrph_num - 1;
	bp->head_ptr = bp->mrph_ptr;
	bp->mrph_num = 1;
    }

    /* orig */
    for (i = sp->Bnst_num - 1; i >= 0; i--) {
	bp = sp->bnst_data + i;
	if (bp->num == -1) { /* ������ǥޡ������줿ʸ�� */
	    continue;
	}

	if (bp->para_type != PARA_NIL) {
	    head_ptr = bp->mrph_ptr + bp->mrph_num - 1;
	}
	else {
	    head_ptr = find_head_mrph_from_dpnd_bnst(NULL, bp); /* �缭������ */
	}
	para_info_to_mrph(bp, head_ptr);

	/* <PARA>�ΤȤ���head�Τߤ�����tag_ptr, tag_num���ѹ��Ϥ��ʤ� */
	if (bp->para_top_p == FALSE) {
	    /* ʸ���� */
	    for (tmp_mp = bp->mrph_ptr + bp->mrph_num - 2; tmp_mp >= bp->mrph_ptr; tmp_mp--) { /* �ǽ������Ǥ�1�������� */
		tmp_mp->parent = (BNST_DATA *)(tmp_mp + 1);
		t_add_node((BNST_DATA *)(tmp_mp + 1), 
			   (BNST_DATA *)tmp_mp, -1);
	    }
	}

	if (last_b_flag) { /* �Ǹ��ʸ�� (�����������Τ� i == Bnst_num - 1 �Ȥϸ¤�ʤ�) */
	    last_b_flag = 0;
	    continue;
	}

	/* �ƤȻ� */
	if (bp->parent) {
	    mp = bp->mrph_ptr + bp->mrph_num - 1; /* ���긵: �ǽ������� */
	    if (bp->para_top_p == TRUE) { /* PARA�ξ���new�����Ǿ����������Ƥ����礬���� */
		while (mp->parent) {
		    mp = (MRPH_DATA*)(mp->parent);
		}
	    }

	    mp->parent = (BNST_DATA *)find_head_mrph_from_dpnd_bnst(bp, bp->parent); /* ����ñ����η������롼�뤫������ */
	    t_add_node((BNST_DATA *)(mp->parent), 
		       (BNST_DATA *)mp, -1);
	}
	else {
	    if (Language != CHINESE) {
		fprintf(stderr, ";; %s(%d)'s parent doesn't exist!\n", bp->Jiritu_Go, i);
	    }
	}
    }
}

/*====================================================================
                               END
====================================================================*/
