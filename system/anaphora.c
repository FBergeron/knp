/*====================================================================

			       �ȱ�����

                                         Ryohei Sasano 2007. 8. 27

    $Id$
====================================================================*/

#include "knp.h"

/* ��ά���Ϥ˴ؤ���ѥ�᡼�� */
#define CASE_CANDIDATE_MAX  10    /* �ȱ������ѳʲ��Ϸ�̤��ݻ������ */
#define CASE_CAND_DIF_MAX   2.3   /* �ʲ��Ϥθ���Ȥ��ƹ�θ���륹�����κ��κ�����(log(10)) */
#define ELLIPSIS_RESULT_MAX 10    /* ��ά���Ϸ�̤��ݻ����� */
#define SALIENCE_DECAY_RATE 0.5   /* salience_score�θ���Ψ */
#define SALIENCE_THRESHOLD  0.199 /* �����оݤȤ���salience_score������(=�ϴޤޤʤ�) */
#define FRAME_FOR_ZERO_MAX  256   /* �����å�����ʥե졼��κ���� */
#define INITIAL_SCORE      -10000

/* ʸ�νи����Ǥ�Ϳ����salience_score */
#define SALIENCE_THEMA 2.0 /* ���פ�����(̤��,ʸ��)��Ϳ���� */
#define SALIENCE_CANDIDATE 1.0 /* ��Ի����Ȥ�������(����,��ʤʤ�)��Ϳ���� */
#define SALIENCE_NORMAL 0.2 /* �嵭�ʳ������Ǥ�Ϳ���� */
#define SALIENCE_ZERO 0.2 /* ������̾���Ϳ���� */
#define SALIENCE_ASSO 0.01 /* Ϣ�۾ȱ�����Ի��Ϳ���� */

/* ���֥��ƥ���(������Ѹ��Ǥ��뤫����̵��)    */
#define	LOC_SELF             0 /* ��ʬ����     */
#define	LOC_PARENT           1 /* ��           */
#define	LOC_CHILD            2 /* �Ҷ�         */
#define LOC_PARA_PARENT      3 /* ����(��¦)   */
#define	LOC_PARA_CHILD       4 /* ����(��¦)   */
#define	LOC_PARENT_N_PARENT  5 /* ���θ��ο�   */
#define	LOC_PARENT_V_PARENT  6 /* ���Ѹ��ο�   */
#define	LOC_OTHERS_BEFORE    7 /* ����¾(��)   */
#define	LOC_OTHERS_AFTER     8 /* ����¾(��)   */
#define	LOC_OTHERS_THEME     9 /* ����¾(����) */

/* clear_context���줿�����Ǥ�ʸ����ENTITY����Ͽ */
int base_sentence_num = 0;
int base_entity_num = 0;

/* ���֥��ƥ�����ݻ� */
int loc_category[BNST_MAX];

/* ���Ϸ�̤��ݻ����뤿���ENTITY_CASE_MGR
   ��Ƭ��CASE_CANDIDATE_MAX�Ĥ˾ȱ������ѳʲ��Ϥη�̤ξ�̤��ݻ���
   ����ELLIPSIS_RESULT_MAX�ĤˤϾ�ά���Ϸ�̤Υ٥��Ȳ���ݻ���
   �Ǹ��1�Ĥϸ��ߤβ��Ϸ�̤��ݻ��˻��Ѥ��� */
CF_TAG_MGR work_ctm[CASE_CANDIDATE_MAX + ELLIPSIS_RESULT_MAX + 1];

/* ��ά���Ϥ��оݤȤ���ʤΥꥹ�� */
char *ELLIPSIS_CASE_LIST_VERB[] = {"��", "��", "��", "\0"};
char *ELLIPSIS_CASE_LIST_NOUN[] = {"��", "��", "�Ρ�", "\0"};
char **ELLIPSIS_CASE_LIST = ELLIPSIS_CASE_LIST_VERB;

/* �Ť��դ��ѥ�᡼��(2011-01-28��: ��̣���饹̤��Ϳ�γʥե졼����Ѥ�WEB186�����ǳؽ�) */
double ModifyWeight[4] = {1.2, 1.0, 0.9, 0.0};
double overt_arguments_weight = 2.946190;
double case_feature_weight[ELLIPSIS_CASE_NUM][O_FEATURE_NUM] =
{0.293874, 0.196797, 0.889493, 0.376286, 0.0, 1.554115, 1.037360, 0.690422, 0.0, -3.249247, 0.0, -0.010954, 0.0, -0.068654, 2.014158, 0.0, 0.711347, -0.702408, 1.271197, -0.441480, 0.194778, -0.494900, 2.309454, 1.534050, -0.639825, 0.381546, 0.0, 0.778179, -0.749595, -0.106908, 0.0, 1.096728, 0.231489, 0.238888, -0.521344, -0.458418, -0.217897, -0.858033, 0.0, 0.0, -1.226149, -0.824823, 0.0, -0.735587, -1.163762, 0.0, 0.202103, -0.594142, -0.053225, 0.0, 0.0, 0.0, 0.0, 0.908783, 0.0, -0.419419, -0.894580, -0.003024, 1.327055, 0.0, 2.244296, 0.0, 0.0, 1.916979, 0.0, 1.153493, 0.0, 0.794486, 1.666137, 0.366469, 0.068826, 0.0, 0.122411, 0.0, 0.0, 0.0, 0.0, -0.012800, 0.0, -0.188097, -0.534504, -0.280096, 0.225998, 0.0, 0.0, 0.0, 0.0, -0.285178, 0.0, 0.298579, 0.0, 1.176796, 0.768480, -0.107689, 0.611514, 0.414866, 0.982838, -0.138493, 0.0, -0.214757, 1.361493, 0.010372, 0.0, -3.963254, 0.0, 0.710817, 0.0, 0.0, 0.269061, 0.0, -0.174625, -0.364441, 1.055502, -0.488462, 0.466356, 0.001243, 0.341602, -0.086529, 0.0, 0.0, 0.0, 0.491079, 1.026130, -0.709793, 0.0, -0.102139, -0.526134, -0.638372, 1.416988, 0.408327, -0.021137, 0.953166, 0.0, 0.308014, 0.721824, 0.245818, 0.0, -0.565069, 1.217682, -0.190846, -0.068116, -0.220994, 0.0, -0.197721, 0.0, 0.0, -0.121605, -0.251522, 0.0, 0.287890, 0.700466, -0.033560, 0.436450, 0.0, 0.0, 0.0, 0.0, -0.192601, 0.0, -0.238693, 0.0, 0.536989, -0.724696, -0.191426, 2.290509, 0.0, 0.651111, 0.0, 0.0, 0.424308, 0.0, 1.492621, 0.0, 0.798711, 0.145208, 0.470715, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.951812, 0.0, 0.313880, 0.646073, -0.280939, 0.970555, 0.237569, 0.545644, -0.033643, 0.0, -0.237407, 1.460399, -0.207218, 0.0, -3.907773, -0.015825, -0.246516, 0.0, 0.490245, 0.253118, -0.295895, -0.691475, -0.964992, -0.604863, -0.361975, 0.224190, 0.156908, -0.016997, -0.143109, 0.0, 0.501877, 0.0, -0.519955, -0.556709, 0.305030, 0.0, 0.0, 0.580170, -0.957457, -0.257423, 0.114019, -0.005383, -0.272394, 0.0, -0.077171, -0.153077, -0.260978, 0.0, 1.218572, -0.687563, 0.020776, 0.495005, 1.019286, 0.0, -0.022475, 0.0, -0.179721, 0.202467, -0.226967, 0.0, 0.557915, 0.044945, 0.377369, 0.577381, 0.0, -0.409069, 0.0, 0.0, -0.016988, 0.0, 0.379237, 0.0, -0.034682, 0.482320, 0.0, 0.718544, 0.0, -0.033915, 0.0, 0.0, 0.0, 0.0, 1.077325, 0.0, 0.586872, -0.271912, 0.0, 0.327167, 0.0, 1.133668, 0.0, 0.0, 0.534845, 0.0, 0.530828, 0.0, 0.081448, 0.448965, 0.893330};

/* �Ť��դ��ѥ�᡼��(��������&baseline��) */
/* double ModifyWeight[4] = {-0.7, -0.7, -0.7, 0.0}; */
/* double overt_arguments_weight = 1.0; */
/* double case_feature_weight[ELLIPSIS_CASE_NUM][O_FEATURE_NUM] = */
/* {{1.0, 0.5, 0.5, 0.5, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, */
/*   0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0}, */
/*  {1.0, 0.5, 0.5, 0.5, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, */
/*   0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,}, */
/*  {1.0, 0.5, 0.5, 0.5, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, */
/*   0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,}}; */

/*==================================================================*/
	   void clear_context(SENTENCE_DATA *sp, int init_flag)
/*==================================================================*/
{
    int i;

    if (OptAnaphora & OPT_PRINT_ENTITY) printf(";;\n;;CONTEXT INITIALIZED\n");
    for (i = 0; i < sp->Sen_num - 1; i++) ClearSentence(sentence_data + i);
    if (init_flag) {
	base_sentence_num = base_entity_num = 0;
	corefer_id = 0;
    }
    else {
	base_sentence_num += sp->Sen_num - 1;
	base_entity_num += entity_manager.num;
    }   
    sp->Sen_num = 1;
    entity_manager.num = 0;
}

/*==================================================================*/
	    int match_ellipsis_case(char *key, char **list)
/*==================================================================*/
{
    /* key����ά�оݳʤΤ����줫�ȥޥå����뤫�ɤ���������å� */
    int i;

    /* ������������Ϥ��Υꥹ�Ȥ�
       �ʤ�����ELLIPSIS_CASE_LIST������å����� */
    if (!list) list = ELLIPSIS_CASE_LIST;

    for (i = 0; *list[i]; i++) {
	if (!strcmp(key, list[i])) return TRUE;
    }
    return FALSE;
}

/*==================================================================*/
	       void assign_mrph_num(SENTENCE_DATA *sp)
/*==================================================================*/
{
    /* ʸ��Ƭ���餽�η����Ǥν���ޤǤ�ʸ������Ϳ���� */
    int i, count = 0;

    for (i = 0; i < sp->Mrph_num; i++) {
	count += strlen((sp->mrph_data + i)->Goi2) / 2;
	(sp->mrph_data + i)->Num = count;
    }
}

/*==================================================================*/
	   TAG_DATA *substance_tag_ptr(TAG_DATA *tag_ptr)
/*==================================================================*/
{
    /* tag_ptr�μ��Τ��֤��ؿ�(����¤�ؤ��н�Τ���) */
    /* cast���뤳�Ȥˤ��bnst_ptr���Ф��Ƥ���� */
    while (tag_ptr && tag_ptr->para_top_p) tag_ptr = tag_ptr->child[0];
    return tag_ptr;
}

/*==================================================================*/
int get_location(char *loc_name, int sent_num, char *kstr, MENTION *mention, int old_flag)
/*==================================================================*/
{
    /* Ʊ��ʸ�ξ���*/
    if (mention->sent_num == sent_num) {
	/* C[����˥�]-C[247]�Ϥޤ����Ϥ��Ƥ��ʤ��ս�β��Ϸ�̤�ɬ�פȤʤ뤿��
	   ���ΤޤޤǤϽ��Ϥ���ʤ��ΤǤ����Ƕ���Ū���������� */
	if (!old_flag &&
	    /* flag��'='�ޤ���'S' */
	    (mention->type == '=' || mention->type == 'S') &&
	    /* �����褬[247] */
	    mention->tag_ptr->b_ptr->parent &&
	    (loc_category[mention->tag_ptr->b_ptr->parent->num] == LOC_CHILD ||
	     loc_category[mention->tag_ptr->b_ptr->parent->num] == LOC_PARA_CHILD ||
	     loc_category[mention->tag_ptr->b_ptr->parent->num] == LOC_OTHERS_BEFORE) &&
	    /* �ʤ����ʡ���ʡ��˳ʡ��γʤΤ����줫 */
	    (check_feature(mention->tag_ptr->b_ptr->f, "��:����") ||
	     check_feature(mention->tag_ptr->b_ptr->f, "��:���") ||
	     check_feature(mention->tag_ptr->b_ptr->f, "��:�˳�") ||
	     check_feature(mention->tag_ptr->b_ptr->f, "��:�γ�"))) {
	    sprintf(loc_name, "%s-C%s-C%d", kstr,
		    check_feature(mention->tag_ptr->b_ptr->f, "��:����") ? "��" :
		    check_feature(mention->tag_ptr->b_ptr->f, "��:���") ? "��" :
		    check_feature(mention->tag_ptr->b_ptr->f, "��:�˳�") ? "��" : "��",
		    loc_category[mention->tag_ptr->b_ptr->parent->num]);
	    return TRUE;
	}
	else {
	    sprintf(loc_name, "%s-%c%s-C%d", kstr,
		    (mention->type == '=') ? 'S' : 
		    (mention->type == 'N') ? 'C' : mention->type, 
		    old_flag ? "" : mention->cpp_string,
		    loc_category[mention->tag_ptr->b_ptr->num]);
	    return TRUE;
	}
    }
    else if (sent_num - mention->sent_num == 1 &&
	     (check_feature(mention->tag_ptr->f, "ʸƬ") ||
	      check_feature(mention->tag_ptr->f, "����")) &&
	     check_feature(mention->tag_ptr->f, "��")) {
	sprintf(loc_name, "%s-%c%s-B1B", kstr, 
		(mention->type == '=') ? 'S' : 
		(mention->type == 'N') ? 'C' : mention->type, 
		old_flag ? "" : mention->cpp_string);
	return TRUE;
    }
    else if (sent_num - mention->sent_num == 1 &&
	     check_feature(mention->tag_ptr->f, "ʸ��") &&
	     check_feature(mention->tag_ptr->f, "�Ѹ�:Ƚ")) {
	sprintf(loc_name, "%s-%c%s-B1E", kstr,
		(mention->type == '=') ? 'S' : 
		(mention->type == 'N') ? 'C' : mention->type, 
		old_flag ? "" : mention->cpp_string);
	return TRUE;
    }
    else if (sent_num - mention->sent_num > 0) {
	sprintf(loc_name, "%s-%c%s-B%d", kstr, 
		(mention->type == '=') ? 'S' : 
		(mention->type == 'N') ? 'C' : mention->type, 
		old_flag ? "" : mention->cpp_string,
		(sent_num - mention->sent_num <= 3 ) ? 
		sent_num - mention->sent_num : 0);
	return TRUE;
    }
    else {
	return FALSE;
    }
}

/*==================================================================*/
     void mark_loc_category(SENTENCE_DATA *sp, TAG_DATA *tag_ptr)
/*==================================================================*/
{
    /* ʸ�ᤴ�Ȥ˰��֥��ƥ������Ϳ���� */
    /* �����ǤǤϤʤ��Ѹ�(̾���ޤ�)¦����Ϳ */
    int i, j;
    BNST_DATA *bnst_ptr, *parent_ptr = NULL, *pparent_ptr = NULL;

    bnst_ptr = (BNST_DATA *)substance_tag_ptr((TAG_DATA *)tag_ptr->b_ptr);

    /* ����� */
    /* ����¾(��) */
    for (i = 0; i < bnst_ptr->num; i++) loc_category[i] = LOC_OTHERS_BEFORE;
    /* ����¾(��) */
    for (i = bnst_ptr->num + 1; i < sp->Bnst_num; i++) 
	loc_category[i] = LOC_OTHERS_AFTER;
    loc_category[bnst_ptr->num] = LOC_SELF; /* ��ʬ���� */

    /* ��ʬ������Ǥ����� */    
    /* KNP������¤(Ⱦ�ѿ�����ʸ���ֹ�)                  */
    /*                      ����0<P>��������������        */
    /*                      ����1<P>��������������        */
    /*             ����2<P>������ ����������������        */
    /*             �¤�3<P>������ ����������������        */
    /*             �á�4<P>������ ����������������        */
    /* ���� 5<P>������   �������� ����������������        */
    /* ����10<P>-PARA9<P>-PARA8<P>��PARA6��������         */
    /*                                     �ط���7        */
    /* ʸ��6,7�Τ�para_type=PARA_NIL��6,8,9��para_top_p=1 */
    if (bnst_ptr->para_type == PARA_NORMAL) {
	for (i = 0; bnst_ptr->parent->child[i]; i++) {

	    if (bnst_ptr->parent->child[i]->para_type == PARA_NORMAL &&
		/* todo::�Ȥꤢ��������������̵�� */
		!bnst_ptr->parent->child[i]->para_top_p) {

		/* ����(��¦) */
		if (bnst_ptr->parent->child[i]->num > bnst_ptr->num)
		    loc_category[bnst_ptr->parent->child[i]->num] = LOC_PARA_PARENT;
		/* ����(�Ҷ�¦) */
		else if (bnst_ptr->parent->child[i]->num < bnst_ptr->num)
		    loc_category[bnst_ptr->parent->child[i]->num] = LOC_PARA_CHILD;
	    }
	}
	/* �Ƥ�õ�� */
	parent_ptr = bnst_ptr->parent;
	while (parent_ptr->para_top_p && parent_ptr->parent) parent_ptr = parent_ptr->parent;
	if (parent_ptr->para_top_p) parent_ptr = NULL;	
    }
    /* ��ʬ������Ǥʤ���� */
    else if (bnst_ptr->parent) {
	parent_ptr = bnst_ptr->parent;
    }
    
    /* �ơ����Ѹ��οơ����θ��ο� */
    if (parent_ptr) {
	loc_category[parent_ptr->num] = LOC_PARENT; /* �� */

	/* �ƤοƤ�õ�� */
	if (parent_ptr->parent) {
	    pparent_ptr = parent_ptr->parent;
	    while (pparent_ptr->para_top_p && pparent_ptr->parent) pparent_ptr = pparent_ptr->parent;
	    if (pparent_ptr->para_top_p) pparent_ptr = NULL;
	}

	if (pparent_ptr) {
	    if (check_feature(pparent_ptr->f, "�Ѹ�"))
		loc_category[pparent_ptr->num] = LOC_PARENT_V_PARENT; /* ���Ѹ��ο� */
	    else
		loc_category[pparent_ptr->num] = LOC_PARENT_N_PARENT; /* ���θ��ο� */
	}
    }	           	

    /* �Ҷ� */
    for (i = 0; bnst_ptr->child[i]; i++) {
	/* �Ҥ�����ξ��(ex. ʹ���Ƥ���) */
	/*   ��Ϩ����������������������� */
	/*  ���٤ʤ��顢<P>�������������� */
	/* ����Ϩ����������������������� */
	/*    ���ߤʤ���<P>��PARA�������� */
	/*                   ʹ���Ƥ����� */   
	if (bnst_ptr->child[i]->para_top_p) { 
	    for (j = 0; bnst_ptr->child[i]->child[j]; j++) {
		/* todo::�Ȥꤢ��������������̵�� */		
		if (!bnst_ptr->child[i]->child[j]->para_top_p)
		    loc_category[bnst_ptr->child[i]->child[j]->num] = LOC_CHILD; /* �Ҷ� */
	    }
	}
	else {
	    loc_category[bnst_ptr->child[i]->num] = LOC_CHILD; /* �Ҷ� */
	}
    }		    	   	   	    
    /* ��ʬ������Ǥ�����(ex. ����) */
    /*    ���Ψ�����������������������  */
    /*    �ᤤ������                    */
    /* ����<P>����������������������  */
    /* �¸���<P>��PARA����������������  */
    /*                �������򨡨�����  */
    /*                          ���롣  */
    if (bnst_ptr->para_type == PARA_NORMAL) {
	for (i = 0; bnst_ptr->parent->child[i]; i++) {

	    /* todo::�Ȥꤢ��������������̵�� */		
	    if (bnst_ptr->parent->child[i]->para_type == PARA_NIL) {
		loc_category[bnst_ptr->parent->child[i]->num] = LOC_CHILD; /* �Ҷ� */
	    }
	}
    }

    /* ��ʬ���Ȥ�ۤ��Ʒ���"��" */
    for (i = 0; i < bnst_ptr->num; i++) {
	if ((sp->bnst_data[i].parent)->num &&
	    (sp->bnst_data[i].parent)->num > bnst_ptr->num &&
	    check_feature(sp->bnst_data[i].f, "��")) loc_category[i] = LOC_OTHERS_THEME;
    }

    if (OptDisplay == OPT_DEBUG) {
	for (i = 0; i < sp->Bnst_num; i++)
	    printf(";;LOC %d-%s target_bnst:%d-%d\n", bnst_ptr->num,
		   bnst_ptr->Jiritu_Go, i, loc_category[i]);
    }

    return;
}

/*==================================================================*/
       int check_analyze_tag(TAG_DATA *tag_ptr, int demo_flag)
/*==================================================================*/
{
    /* Ϳ������줿tag_ptr�������оݤ��ɤ���������å� */
    /* demo_flag��Ϳ����줿����"����"�˽�������Ƥ��뤫�ɤ������֤� */

    /* �Ѹ��Ȥ��Ƥβ����оݤǤ�����:CF_PRED(=1)���֤� */
    /* ̾��Ȥ��Ƥβ����оݤǤ�����:CF_NOUN(=2)���֤� */
    /* ����ʳ��ξ���0���֤� */
    int i;
    BNST_DATA *bnst_ptr;

    /* ��ά���Ϥʤ� */
    if (check_feature(tag_ptr->f, "��ά���Ϥʤ�") ||
	check_feature(tag_ptr->f, "NE") ||
	check_feature(tag_ptr->f, "NE��") ||
	check_feature(tag_ptr->f, "Ʊ��") ||
	check_feature(tag_ptr->f, "������") ||
	check_feature(tag_ptr->f, "��������")) return 0;

    /* demo_flag�����äƤ�������θ��Τ��о� */
    if (demo_flag && 
	(!(OptEllipsis & OPT_REL_NOUN) || !check_feature(tag_ptr->f, "�θ�"))) return 0;
   
    /* ̾��Ȥ��Ʋ��Ϥ����� */
    if ((OptEllipsis & OPT_REL_NOUN) && check_feature(tag_ptr->f, "�θ�") &&
	!check_feature(tag_ptr->f, "�Ѹ�����") &&

	/* �Ѹ��β��Ϥ�Ԥ����ϥ��Ѥ��оݳ� */
	!((OptEllipsis & OPT_ELLIPSIS) && check_feature(tag_ptr->f, "����"))) {
	
	/* �缭�ʳ����оݳ� */
	if (check_feature(tag_ptr->f, "ʸ����")) return 0;

	/* ����̾����оݳ� */
	if (check_feature(tag_ptr->f, "����̾��")) return 0;

	bnst_ptr = (BNST_DATA *)substance_tag_ptr((TAG_DATA *)tag_ptr->b_ptr);

	/* ʸ����Ϣ�ѽ�������Ƥ����θ��Ͻ��� */
	if (check_feature(bnst_ptr->f, "ʸ��") && !check_feature(bnst_ptr->f, "ʸƬ") &&
	    bnst_ptr->child[0] && check_feature(bnst_ptr->child[0]->f, "��:Ϣ��")) return 0;

	/* "����"�˽�������Ƥ��뤫�ɤ�����Ƚ�ꤹ����ʳ� */
	/* ��������Ƥ������о�(����Ū) */
	if (!demo_flag) return CF_NOUN; 

	/* "����"�˽�������Ƥ��뤫�ɤ�����Ƚ�ꤹ���� */
	/* "����"�ʳ��˽�������Ƥ����� */
	if (bnst_ptr->child[0] && 
	    strcmp(bnst_ptr->child[0]->head_ptr->Goi2, "����") &&
	    (!bnst_ptr->child[1] || strcmp(bnst_ptr->child[1]->head_ptr->Goi2, "����"))) return 0;
	/* "����"�˽�������Ƥ����� */
	if (demo_flag && bnst_ptr->child[0]) return CF_NOUN;

	if (/* �����ξ�������������˷���ɽ�����ǧ */
	    bnst_ptr->para_type == PARA_NORMAL) {
	    for (i = 0; bnst_ptr->parent->child[i]; i++) {
		if (bnst_ptr->parent->child[i]->para_type == PARA_NIL &&
		    strcmp(bnst_ptr->parent->child[i]->head_ptr->Goi2, "����")) return 0;
		if (demo_flag && 
		    !strcmp(bnst_ptr->parent->child[i]->head_ptr->Goi2, "����")) return 1;
	    }
	}

	if (demo_flag) return 0;
	return CF_NOUN;
    }

    /* �Ѹ��Ȥ��Ʋ��Ϥ����� */
    if ((OptEllipsis & OPT_ELLIPSIS) &&	check_feature(tag_ptr->f, "�Ѹ�")) {

	/* ��°��ϲ��Ϥ��ʤ� */
	if (check_feature(tag_ptr->mrph_ptr->f, "��°")) return 0;

	/* ñ�Ȥ��Ѹ����ء٤ǰϤޤ�Ƥ�����Ͼ�ά���Ϥ��ʤ�(����Ū) */
	if (check_feature(tag_ptr->f, "��̻�") &&
	    check_feature(tag_ptr->f, "��̽�")) return 0;

	/* ���Ѥ�ʸ��缭�Τ��о� */
	if (check_feature(tag_ptr->f, "ʸ����") && 
	    check_feature(tag_ptr->f, "����")) return 0;

	/* Ƚ���β��Ϥ�Ԥ�ʤ������θ����оݳ� */
	if (!(OptAnaphora & OPT_ANAPHORA_COPULA) &&
	    check_feature(tag_ptr->f, "�θ�")) return 0;
	return CF_PRED;
    }
    return 0;
}

/*==================================================================*/
int read_one_annotation(SENTENCE_DATA *sp, TAG_DATA *tag_ptr, char *token, int co_flag)
/*==================================================================*/
{
    /* ���Ϸ�̤���MENTION��ENTITY��������� */
    /* co_flag���������"="�Τߤ�������ʤ�����"="�ʳ������ */
    char type, rel[SMALL_DATA_LEN], *cp, loc_name[SMALL_DATA_LEN];
    int i, j, tag_num, sent_num, bnst_num, diff_sen;
    TAG_DATA *parent_ptr;
    MENTION_MGR *mention_mgr = &(tag_ptr->mention_mgr);
    MENTION *mention_ptr = NULL;
    ENTITY *entity_ptr;
    
    if (!sscanf(token, "%[^/]/%c/%*[^/]/%d/%d/", rel, &type, &tag_num, &sent_num))
	return FALSE;
    if (tag_num == -1) return FALSE;

    /* �����ȴط����ɤ߹��� */
    if (co_flag && 
	(!strcmp(rel, "=") || !strcmp(rel, "=��") || !strcmp(rel, "=��"))) {

	/* ʣ���ζ����Ⱦ�����Ϳ����Ƥ����� */
	if (mention_mgr->mention->entity) {
	    if (mention_mgr->mention->entity->output_num >
		substance_tag_ptr((sp - sent_num)->tag_data + tag_num)->mention_mgr.mention->entity->output_num) {
		mention_mgr->mention->entity->output_num = 
		    substance_tag_ptr((sp - sent_num)->tag_data + tag_num)->mention_mgr.mention->entity->output_num;
	    }
	    else {
		substance_tag_ptr((sp - sent_num)->tag_data + tag_num)->mention_mgr.mention->entity->output_num =
		    mention_mgr->mention->entity->output_num;
	    }	
	}

	mention_ptr = mention_mgr->mention;
	mention_ptr->entity = 
	    substance_tag_ptr((sp - sent_num)->tag_data + tag_num)->mention_mgr.mention->entity;
	mention_ptr->explicit_mention = NULL;

	mention_ptr->salience_score = mention_ptr->entity->salience_score;
	mention_ptr->entity->salience_score += 
	    ((check_feature(tag_ptr->f, "��") || check_feature(tag_ptr->f, "��")) &&
	     check_feature(tag_ptr->f, "��:̤��") && !check_feature(tag_ptr->f, "��̽�") ||
	     check_feature(tag_ptr->f, "Ʊ��") ||
	     check_feature(tag_ptr->f, "ʸ��")) ? SALIENCE_THEMA : 
	    (check_feature(tag_ptr->f, "����") && tag_ptr->para_type != PARA_NORMAL ||
	     check_feature(tag_ptr->f, "��:����") ||
	     check_feature(tag_ptr->f, "��:���")) ? SALIENCE_CANDIDATE : SALIENCE_NORMAL;
	strcpy(mention_ptr->cpp_string, "��");

	parent_ptr = tag_ptr->parent;
	while (parent_ptr && parent_ptr->para_top_p) parent_ptr = parent_ptr->parent;
	if (check_feature(tag_ptr->f, "��:�˳�") || check_feature(tag_ptr->f, "��:�γ�"))
	    mention_ptr->entity->tmp_salience_flag = 1;

	if ((cp = check_feature(tag_ptr->f, "��"))) {
	    strcpy(mention_ptr->spp_string, cp + strlen("��:"));
	} 
	else if (check_feature(tag_ptr->f, "ʸ��")) {
	    strcpy(mention_ptr->spp_string, "ʸ��");
	} 
	else {
	    strcpy(mention_ptr->spp_string, "��");
	}
	mention_ptr->type = '=';

	/* entity��name��"��"�ʤ��name���� */
	if (!strcmp(mention_ptr->entity->name, "��") ||
	    mention_ptr->salience_score == 0 && mention_ptr->entity->salience_score > 0) {
	    if (cp = check_feature(tag_ptr->f, "NE")) {
		strcpy(mention_ptr->entity->name, cp + strlen("NE:"));
	    }
	    else if (cp = check_feature(tag_ptr->f, "�ȱ������")) {
		strcpy(mention_ptr->entity->name, cp + strlen("�ȱ������:"));
	    }
	    else {
		strcpy(mention_ptr->entity->name, tag_ptr->head_ptr->Goi2);
	    }
	}
	/* entity��name��NE�Ǥʤ���tag_ptr��NE�ʤ��name���� */
	if (!strchr(mention_ptr->entity->name, ':') &&
	    (cp = check_feature(tag_ptr->f, "NE"))) {
	    strcpy(mention_ptr->entity->name, cp + strlen("NE:"));
	}
	/* entity��name��NE�Ǥʤ���tag_ptr��Ʊ�ʤʤ��name���� */
	else if (!strchr(mention_ptr->entity->name, ':') &&
		 check_feature(tag_ptr->f, "Ʊ��")) {
	    if (cp = check_feature(tag_ptr->f, "�ȱ������")) {
		strcpy(mention_ptr->entity->name, cp + strlen("�ȱ������:"));
	    }
	    else {
		strcpy(mention_ptr->entity->name, tag_ptr->head_ptr->Goi2);
	    }
	}
    }

    /* �����Ȱʳ��δط� */
    else if (!co_flag && 
	     (type == 'N' || type == 'C' || type == 'O' || type == 'D') &&
	     
	     /* �Ѹ��ξ��Ͼ�ά�оݳʤΤ��ɤ߹��� */
	     (check_analyze_tag(tag_ptr, FALSE) == CF_PRED && match_ellipsis_case(rel, NULL) ||
	      /* ̾��ξ��� */
	      check_analyze_tag(tag_ptr, FALSE) == CF_NOUN && 
	      /* Ϣ�۾ȱ��оݳʤξ��Ϥ��Τޤ��ɤ߹��� */
	      (match_ellipsis_case(rel, NULL) ||
	       /* �Ѹ��ξ�ά�оݳʤξ��ϥΡ��ʤȤ����ɤ߹��� */
	       match_ellipsis_case(rel, ELLIPSIS_CASE_LIST_VERB) && strcpy(rel, "�Ρ�"))) &&
	     /* ��Ի���θ��Τ� */
	     (check_feature(((sp - sent_num)->tag_data + tag_num)->f, "�θ�") ||
	      check_feature(((sp - sent_num)->tag_data + tag_num)->f, "����̾��"))) {	

	if (mention_mgr->num >= MENTION_MAX - 1) return;
	mention_ptr = mention_mgr->mention + mention_mgr->num;
 	mention_ptr->entity = 
	    substance_tag_ptr((sp - sent_num)->tag_data + tag_num)->mention_mgr.mention->entity;
	mention_ptr->explicit_mention = (type == 'C') ?
	    substance_tag_ptr((sp - sent_num)->tag_data + tag_num)->mention_mgr.mention : NULL;
	mention_ptr->salience_score = mention_ptr->entity->salience_score;

	mention_ptr->tag_num = mention_mgr->mention->tag_num;
	mention_ptr->sent_num = mention_mgr->mention->sent_num;
	mention_ptr->tag_ptr = 
	    (sentence_data + mention_ptr->sent_num - 1)->tag_data + mention_ptr->tag_num;
	mention_ptr->type = type;
	strcpy(mention_ptr->cpp_string, rel);
	if (type == 'C' && 
	    (cp = check_feature(((sp - sent_num)->tag_data + tag_num)->f, "��"))) {
	    strcpy(mention_ptr->spp_string, cp + strlen("��:"));
	} 
	else if (type == 'C' && 
		 (check_feature(((sp - sent_num)->tag_data + tag_num)->f, "ʸ��"))) {
	    strcpy(mention_ptr->spp_string, "ʸ��");
	} 		
	else {
	    strcpy(mention_ptr->spp_string, "��");
	}
	mention_mgr->num++;

	/* �����ȥ�����é���Ϣ�ν�����Ǥ������type��'C'���ѹ� */
	if (type == 'O' && check_feature(tag_ptr->f, "Ϣ�ν���") &&
	    tag_ptr->parent->mention_mgr.mention->entity == mention_ptr->entity) {
	    mention_ptr->type = type = 'C';
	}	    

	if (type == 'O') {
	    if (check_analyze_tag(tag_ptr, FALSE) == CF_PRED)		
		mention_ptr->entity->salience_mem += SALIENCE_ZERO;
	    else 
		mention_ptr->entity->salience_mem += SALIENCE_ASSO;
	}  
    }

    if (!mention_ptr) return FALSE;
    mention_ptr->entity->mention[mention_ptr->entity->mentioned_num] = mention_ptr;
    if (mention_ptr->entity->mentioned_num >= MENTIONED_MAX - 1) { 
	fprintf(stderr, "Entity \"%s\" mentiond too many times!\n", mention_ptr->entity->name);
    }
    else {
	mention_ptr->entity->mentioned_num++;
    }

    /* �ؽ��Ѿ���ν��� */
    if ((OptAnaphora & OPT_TRAIN) && type == 'O' && strcmp(rel, "=")) {

	/* ���֥��ƥ���ν��� */
	mark_loc_category(sp, tag_ptr);
	entity_ptr = mention_ptr->entity;

	/* ��ʸ�����mention����äƤ��뤫�ɤ����Υ����å� */
	diff_sen = 4;
	for (i = 0; i < entity_ptr->mentioned_num; i++) {
	    if (mention_ptr->sent_num == entity_ptr->mention[i]->sent_num &&
		loc_category[(entity_ptr->mention[i]->tag_ptr)->b_ptr->num] == LOC_SELF) continue;
	    
	    if (mention_ptr->sent_num - entity_ptr->mention[i]->sent_num < diff_sen)
		diff_sen = mention_ptr->sent_num - entity_ptr->mention[i]->sent_num;
	}

	for (i = 0; i < entity_ptr->mentioned_num; i++) {
	    /* ��äȤ�᤯��ʸ�˽и�����mention�Τ߽��� */
	    if (mention_ptr->sent_num - entity_ptr->mention[i]->sent_num > diff_sen)
		continue;
	    
	    if ( /* ��ʬ���ȤϤΤ��� */
		entity_ptr->mention[i]->sent_num == mention_ptr->sent_num &&
		loc_category[(entity_ptr->mention[i]->tag_ptr)->b_ptr->num] == LOC_SELF) continue;
		
	    if (get_location(loc_name, mention_ptr->sent_num, rel, entity_ptr->mention[i], FALSE)) {
		printf(";;LOCATION-ANT: %s\n", loc_name);
	    }
	}
    }
    return TRUE;
}

/*==================================================================*/
       void expand_result_to_parallel_entity(TAG_DATA *tag_ptr)
/*==================================================================*/
{
    /* �������Ǥ�Ÿ������ */
    int i, j, result_num;
    CF_TAG_MGR *ctm_ptr = tag_ptr->ctm_ptr; 
    TAG_DATA *t_ptr, *para_ptr;
    ENTITY *entity_ptr, *epnd_entity_ptr;
    MENTION *mention_ptr;
    
    result_num = ctm_ptr->result_num;
    for (i = 0; i < result_num; i++) {
	entity_ptr = entity_manager.entity + ctm_ptr->entity_num[i];

	/* �����Ǥ�entity�ξ�ά�ʳ���ľ��νи���õ�� */
	for (j = entity_ptr->mentioned_num - 1; j >= 0; j--) {
	    if (entity_ptr->mention[j]->type == 'S' ||
		entity_ptr->mention[j]->type == '=') break;
	}
	/* Ʊ��ʸ�ξ��Τߤ��оݤȤ��� */
	if (tag_ptr->mention_mgr.mention->sent_num != entity_ptr->mention[j]->sent_num) continue;

	t_ptr = entity_ptr->mention[j]->tag_ptr;

	/* ��������Ǥ�����å� */
	if (t_ptr->para_type == PARA_NORMAL &&
	    t_ptr->parent && t_ptr->parent->para_top_p) {
	    
	    for (j = 0; t_ptr->parent->child[j]; j++) {
		para_ptr = substance_tag_ptr(t_ptr->parent->child[j]);

		if (para_ptr != t_ptr && check_feature(para_ptr->f, "�θ�") &&
		    para_ptr->para_type == PARA_NORMAL &&
		    /* ���Ϥ��ؼ��ˤ�Ŭ�Ѥ��ʤ�(����Ū) */
		    !(ctm_ptr->type[i] == 'O' && 
		      check_analyze_tag(tag_ptr, FALSE) == CF_NOUN) &&
		    /* ��ά�ξ��ϳ�ĥ�褬�����оݤη�����Ǥ����硢��ĥ���ʤ�*/
		    !(ctm_ptr->type[i] == 'O' && tag_ptr->parent == para_ptr)) {    

		    epnd_entity_ptr = para_ptr->mention_mgr.mention->entity;
		    ctm_ptr->filled_entity[epnd_entity_ptr->num] = TRUE;
		    ctm_ptr->entity_num[ctm_ptr->result_num] = epnd_entity_ptr->num;
		    ctm_ptr->type[ctm_ptr->result_num] = ctm_ptr->type[i];
		    ctm_ptr->cf_element_num[ctm_ptr->result_num] = ctm_ptr->cf_element_num[i];
		    ctm_ptr->result_num++;

		    if (OptDisplay == OPT_DEBUG)
			printf(";;EXPANDED %s : %s -> %s\n", 
			       tag_ptr->head_ptr->Goi2, 
			       entity_ptr->name, epnd_entity_ptr->name);

		    if (ctm_ptr->result_num == CF_ELEMENT_MAX) return;
		}
	    }
	}
    }
}

/*==================================================================*/
	  void anaphora_result_to_entity(TAG_DATA *tag_ptr)
/*==================================================================*/
{
    /* �ȱ����Ϸ��ENTITY�˴�Ϣ�դ��� */
    int i, j;
    char *cp;
    MENTION_MGR *mention_mgr = &(tag_ptr->mention_mgr);
    MENTION *mention_ptr = NULL;
    CF_TAG_MGR *ctm_ptr = tag_ptr->ctm_ptr; 

    /* �ʡ���ά���Ϸ�̤��ʤ����Ͻ�λ */
    if (!ctm_ptr) return;
    
    for (i = 0; i < ctm_ptr->result_num; i++) {
	if (mention_mgr->num >= MENTION_MAX - 1) return;
	mention_ptr = mention_mgr->mention + mention_mgr->num;
	mention_ptr->entity = entity_manager.entity + ctm_ptr->entity_num[i];
	mention_ptr->tag_num = mention_mgr->mention->tag_num;
	mention_ptr->sent_num = mention_mgr->mention->sent_num;
	mention_ptr->type = ctm_ptr->type[i];
	mention_ptr->tag_ptr = 
	    (sentence_data + mention_ptr->sent_num - 1)->tag_data + mention_ptr->tag_num;
	strcpy(mention_ptr->cpp_string,
	       pp_code_to_kstr(ctm_ptr->cf_ptr->pp[ctm_ptr->cf_element_num[i]][0]));
	mention_ptr->salience_score = mention_ptr->entity->salience_score;
	/* ����¦��ɽ�س�(�ʲ��Ϸ�̤Τ�) */
	if (i < ctm_ptr->case_result_num) {
	    mention_ptr->explicit_mention = ctm_ptr->elem_b_ptr[i]->mention_mgr.mention;
	    if (tag_ptr->tcf_ptr->cf.pp[ctm_ptr->tcf_element_num[i]][0] >= FUKUGOJI_START &&
		tag_ptr->tcf_ptr->cf.pp[ctm_ptr->tcf_element_num[i]][0] <= FUKUGOJI_END) {
		strcpy(mention_ptr->spp_string, 
		       pp_code_to_kstr(tag_ptr->tcf_ptr->cf.pp[ctm_ptr->tcf_element_num[i]][0]));
	    }
	    else { 
		if ((cp = check_feature(ctm_ptr->elem_b_ptr[i]->f, "��"))) {
		    strcpy(mention_ptr->spp_string, cp + strlen("��:"));
		} 
		else if (check_feature(ctm_ptr->elem_b_ptr[i]->f, "ʸ��")) {
		    strcpy(mention_ptr->spp_string, "ʸ��");
		}
		else {
		    strcpy(mention_ptr->spp_string, "��");
		}
	    }
	}
	else {
	    mention_ptr->explicit_mention = NULL;
	    /* ��ά�Ǥʤ����(expand_result_to_parallel_entity�ǳ�ĥ) */
	    if (ctm_ptr->type[i] != 'O') {
		strcpy(mention_ptr->spp_string, "��");
	    }
	    else {
		strcpy(mention_ptr->spp_string, "��");
		if (check_analyze_tag(tag_ptr, FALSE) == CF_PRED)
		    mention_ptr->entity->salience_score += SALIENCE_ZERO;
		else
		    mention_ptr->entity->salience_score += SALIENCE_ASSO;
	    }
	}
	mention_mgr->num++;

	mention_ptr->entity->mention[mention_ptr->entity->mentioned_num] = mention_ptr;
	if (mention_ptr->entity->mentioned_num >= MENTIONED_MAX - 1) { 
	    fprintf(stderr, "Entity \"%s\" mentiond too many times!\n", mention_ptr->entity->name);
	}
	else {
	    mention_ptr->entity->mentioned_num++;
	}
    }  
}

/*==================================================================*/
     int set_tag_case_frame(SENTENCE_DATA *sp, TAG_DATA *tag_ptr, CF_PRED_MGR *cpm_ptr)
/*==================================================================*/
{
    /* ENTITY_PRED_MGR���������ؿ�
       make_data_cframe���Ѥ�������ʸ�γʹ�¤��������뤿��
       CF_PRED_MGR���ꡢ����cf�򥳥ԡ����Ƥ��� */
    int i;
    TAG_CASE_FRAME *tcf_ptr = tag_ptr->tcf_ptr;
    char *vtype = NULL;  

    /* ����ʸ¦�γ��������� */
    /* set_data_cf_type(cpm_ptr); */
    if (check_analyze_tag(tag_ptr, FALSE) == CF_PRED) {
	vtype = check_feature(tag_ptr->f, "�Ѹ�");
	vtype += strlen("�Ѹ�:");
	strcpy(cpm_ptr->cf.pred_type, vtype);
	cpm_ptr->cf.type = CF_PRED;
    }
    else {
	strcpy(cpm_ptr->cf.pred_type, "̾");
	cpm_ptr->cf.type = CF_NOUN;
    }
    cpm_ptr->cf.type_flag = 0;
    cpm_ptr->cf.voice = tag_ptr->voice;

    /* ����ʸ�γʹ�¤����� */
    make_data_cframe(sp, cpm_ptr);
    
    /* ENTITY_PRED_MGR�����������ʸ¦�γ����Ǥ򥳥ԡ� */
    tcf_ptr->cf = cpm_ptr->cf;
    tcf_ptr->pred_b_ptr = tag_ptr;
    for (i = 0; i < cpm_ptr->cf.element_num; i++) {
	tcf_ptr->elem_b_ptr[i] = substance_tag_ptr(cpm_ptr->elem_b_ptr[i]);
	tcf_ptr->elem_b_num[i] = cpm_ptr->elem_b_num[i];
    }

    return TRUE;
}

/*==================================================================*/
    int set_cf_candidate(TAG_DATA *tag_ptr, CASE_FRAME **cf_array)
/*==================================================================*/
{
    int i, l, frame_num = 0, hiragana_prefer_type = 0;
    CFLIST *cfp;
    char *key;
    
    /* �ʥե졼��cache */
    if (OptUseSmfix == TRUE && CFSimExist == TRUE) {
		
	if ((key = get_pred_id(tag_ptr->cf_ptr->cf_id)) != NULL) {
	    cfp = CheckCF(key);
	    free(key);

	    if (cfp) {
		for (l = 0; l < tag_ptr->cf_num; l++) {
		    for (i = 0; i < cfp->cfid_num; i++) {
			if (((tag_ptr->cf_ptr + l)->type == tag_ptr->tcf_ptr->cf.type) &&
			    ((tag_ptr->cf_ptr + l)->cf_similarity = 
			     get_cfs_similarity((tag_ptr->cf_ptr + l)->cf_id, 
						*(cfp->cfid + i))) > CFSimThreshold) {
			    *(cf_array + frame_num++) = tag_ptr->cf_ptr + l;
			    break;
			}
		    }
		}
		tag_ptr->e_cf_num = frame_num;
	    }
	}
    }

    if (frame_num == 0) {
	/* ɽ�����Ҥ餬�ʤξ��: 
	   �ʥե졼���ɽ�����Ҥ餬�ʤξ�礬¿����ФҤ餬�ʤγʥե졼��Τߤ��оݤˡ�
	   �Ҥ餬�ʰʳ���¿����ФҤ餬�ʰʳ��Τߤ��оݤˤ��� */
	if (!(OptCaseFlag & OPT_CASE_USE_REP_CF) && /* ��ɽɽ���ǤϤʤ����Τ� */
	    check_str_type(tag_ptr->head_ptr->Goi) == TYPE_HIRAGANA) {
	    if (check_feature(tag_ptr->f, "��ɽ�Ҥ餬��")) {
		hiragana_prefer_type = 1;
	    }
	    else {
		hiragana_prefer_type = -1;
	    }
	}

	for (l = 0; l < tag_ptr->cf_num; l++) {
	    if ((tag_ptr->cf_ptr + l)->type == tag_ptr->tcf_ptr->cf.type && 
		(hiragana_prefer_type == 0 || 
		 (hiragana_prefer_type > 0 && 
		  check_str_type((tag_ptr->cf_ptr + l)->entry) == TYPE_HIRAGANA) || 
		 (hiragana_prefer_type < 0 && 
		  check_str_type((tag_ptr->cf_ptr + l)->entry) != TYPE_HIRAGANA))) {
		*(cf_array + frame_num++) = tag_ptr->cf_ptr + l;
	    }
	}
    }
    return frame_num;
}

/*==================================================================*/
double calc_score_of_ctm(CF_TAG_MGR *ctm_ptr, TAG_CASE_FRAME *tcf_ptr)
/*==================================================================*/
{
    /* �ʥե졼��Ȥ��б��դ��Υ�������׻�����ؿ�  */
    int i, j, e_num, debug = 0;
    double score;
    char key[SMALL_DATA_LEN];

    /* �оݤγʥե졼�ब���򤵤�뤳�ȤΥ����� */
    score = get_cf_probability_for_pred(&(tcf_ptr->cf), ctm_ptr->cf_ptr);

    /* �б��դ���줿���Ǥ˴ؤ��륹����(�ʲ��Ϸ��) */
    for (i = 0; i < ctm_ptr->case_result_num; i++) {
	e_num = ctm_ptr->cf_element_num[i];
	
	score += 
	    get_ex_probability_with_para(ctm_ptr->tcf_element_num[i], &(tcf_ptr->cf), e_num, ctm_ptr->cf_ptr) +
	    get_case_function_probability_for_pred(ctm_ptr->tcf_element_num[i], &(tcf_ptr->cf), e_num, ctm_ptr->cf_ptr, TRUE);
	
	if (OptDisplay == OPT_DEBUG && debug)
	    printf(";;�б�����:%s-%s:%f:%f ", ctm_ptr->elem_b_ptr[i]->head_ptr->Goi2, 
		   pp_code_to_kstr(ctm_ptr->cf_ptr->pp[e_num][0]),
		   get_ex_probability_with_para(ctm_ptr->tcf_element_num[i], &(tcf_ptr->cf), e_num, ctm_ptr->cf_ptr),
		   get_case_function_probability_for_pred(ctm_ptr->tcf_element_num[i], &(tcf_ptr->cf), e_num, ctm_ptr->cf_ptr, TRUE));
    }

    /* ����ʸ�γ����ǤΤ����б��դ����ʤ��ä����Ǥ˴ؤ��륹���� */
    for (i = 0; i < tcf_ptr->cf.element_num - ctm_ptr->case_result_num; i++) {
	if (OptDisplay == OPT_DEBUG && debug) 
	    printf(";;�б��ʤ�:%s:%f ", 
		   (tcf_ptr->elem_b_ptr[ctm_ptr->non_match_element[i]])->head_ptr->Goi2, score);	
	score += FREQ0_ASSINED_SCORE + UNKNOWN_CASE_SCORE;
    }
    if (OptDisplay == OPT_DEBUG && debug) printf(";; %f ", score);	   

    /* �ʥե졼��γʤ���ޤäƤ��뤫�ɤ����˴ؤ��륹���� */
    for (e_num = 0; e_num < ctm_ptr->cf_ptr->element_num; e_num++) {
	if (tcf_ptr->cf.type == CF_NOUN) continue;
	score += get_case_probability(e_num, ctm_ptr->cf_ptr, ctm_ptr->filled_element[e_num], NULL);	
    }
    if (OptDisplay == OPT_DEBUG && debug) printf(";; %f\n", score);

    return score;
}

/*==================================================================*/
		int convert_locname_id(char *loc_name)
/*==================================================================*/
{
    /* ���֥��ƥ����ID���Ѵ�[0-83]������¾:-1 */
    /* �и���: S�� + [CO][�����]: 7  */
    /* ���֥�: C[1-9] + B[1-3]	 : 12 */
    int id = 0;

    /* ��-O��-C8 */
    /* ��-O��-B1 */
    if (strlen(loc_name) != 9) return -1;

    /* [SCO] */
    if (loc_name[3] == 'C') id += 12;
    else if (loc_name[3] == 'O') id += 48;
    else if (loc_name[3] != 'S') return -1;

    /* [�����] */
    if (loc_name[3] != 'S') {
	if (!strncmp(loc_name + 4, "��", 2)) id += 12;
	else if (!strncmp(loc_name + 4, "��", 2)) id += 24;
	else if (strncmp(loc_name + 4, "��", 2)) return -1;
    }

    /* [CB] */
    if (loc_name[7] == 'B') id += 9;
    else if (loc_name[7] != 'C') return -1;

    /* [1-9] */
    if (atoi(loc_name + 8) > 0) id += atoi(loc_name + 8) - 1;
    else return -1;

    return id;
}

/*==================================================================*/
double calc_ellipsis_score_of_ctm(CF_TAG_MGR *ctm_ptr, TAG_CASE_FRAME *tcf_ptr)
/*==================================================================*/
{
    /* �ʥե졼��Ȥ��б��դ��Υ�������׻�����ؿ�(��ά���Ϥ�ɾ��) */
    int i, j, loc_num, e_num, sent_num, pp;
    double score = 0, max_score, tmp_ne_ct_score, tmp_score, ex_prob, prob, penalty;
    double *of_ptr, scase_prob_cs, scase_prob, location_prob;
    char *cp, key[SMALL_DATA_LEN], loc_name[SMALL_DATA_LEN];
    ENTITY *entity_ptr;

    /* �����оݤδ��ܶ��ʸ�ֹ� */
    sent_num = tcf_ptr->pred_b_ptr->mention_mgr.mention->sent_num;

    /* omit_feature�ν���� */
    for (i = 0; i < ELLIPSIS_CASE_NUM; i++) {
	for (j = 0; j < O_FEATURE_NUM; j++) {
	    ctm_ptr->omit_feature[i][j] = INITIAL_SCORE;
	}
    }

    /* �б��դ���줿���Ǥ˴ؤ��륹����(��ά���Ϸ��) */
    for (i = ctm_ptr->case_result_num; i < ctm_ptr->result_num; i++) {
	e_num = ctm_ptr->cf_element_num[i];
	entity_ptr = entity_manager.entity + ctm_ptr->entity_num[i]; /* ��Ϣ�դ���줿ENTITY */	
	pp = ctm_ptr->cf_ptr->pp[e_num][0]; /* "��"��"��"��"��"��code�Ϥ��줾��1��2��3 */
	of_ptr = ctm_ptr->omit_feature[pp - 1];

	/* ��ޤä����ɤ��� */
	of_ptr[ASSIGNED] = 1;

	/* �б��դ����ʤ��ä������оݳʤ���ޤ�䤹�� */
	of_ptr[NO_ASSIGNMENT] = get_case_probability(e_num, ctm_ptr->cf_ptr, TRUE, NULL);

	/* P(����|���٤�:ư2,���)/P(����) (��P(���٤�:ư2,���|����)) */
	/* type='S'�ޤ���'='��mention����Ǻ���Ȥʤ��Τ���� */	
	max_score = INITIAL_SCORE;

	for (j = 0; j < entity_ptr->mentioned_num; j++) {
	    if (entity_ptr->mention[j]->type != 'S' && entity_ptr->mention[j]->type != '=') continue;
	    tmp_ne_ct_score = FREQ0_ASSINED_SCORE;

	    /* ���饹�Υ�������׻� */
	    if ((OptGeneralCF & OPT_CF_CLASS) && tcf_ptr->cf.type == CF_PRED) {
		cp = get_bnst_head_canonical_rep(entity_ptr->mention[j]->tag_ptr->b_ptr, OptCaseFlag & OPT_CASE_USE_CN_CF);
		if (cp != NULL && strlen(cp) < SMALL_DATA_LEN - 4) {
 		    sprintf(key, "%s:CL", cp);
		    prob = get_class_probability(key, e_num, ctm_ptr->cf_ptr); 
		    if (prob && log(prob) > of_ptr[CLS_PMI]) of_ptr[CLS_PMI] = log(prob);
		}
	    }

 	    /* ���ƥ��꤬�������P(���٤�:ư2,���|���ƥ���:��)������å� */
	    if ((OptGeneralCF & OPT_CF_CATEGORY) && 
		(cp = check_feature(entity_ptr->mention[j]->tag_ptr->head_ptr->f, "���ƥ���"))) {

		while (strchr(cp, ':') && (cp = strchr(cp, ':')) || (cp = strchr(cp, ';'))) {
		    sprintf(key, "CT:%s:", ++cp);
		    if (strchr(key + 3, ';')) *strchr(key + 3, ';') = ':'; /* tag = CT:�ȿ�������;���ʪ: */
		    
		    if (/* !strncmp(key, "CT:��:", 6) && */
			(prob = get_ex_ne_probability(key, e_num, ctm_ptr->cf_ptr, TRUE))) {
			/* P(���ƥ���:��|���٤�:ư2,���) */
			tmp_score = log(prob);
			
			/* /P(���ƥ���:��) */
			*strchr(key + 3, ':') = '\0';
			tmp_score -= get_general_probability(key, "KEY");		
			if (tmp_score > of_ptr[CEX_PMI]) of_ptr[CEX_PMI] = tmp_score;
			if (tmp_score > tmp_ne_ct_score) tmp_ne_ct_score = tmp_score;
		    }
		}
	    }

	    /* ��ͭɽ���ξ���P(���٤�:ư2,���|ARTIFACT)������å� */
	    if ((OptGeneralCF & OPT_CF_NE) && 
		(cp = check_feature(entity_ptr->mention[j]->tag_ptr->f, "NE")) &&
		(prob = get_ex_ne_probability(cp, e_num, ctm_ptr->cf_ptr, TRUE))) {

		/* P(ARTIFACT|���٤�:ư2,���) */
		tmp_score = log(prob);

		/* /P(ARTIFACT) */
		strcpy(key, cp);
		*strchr(key + 3, ':') = '\0'; /* key = NE:LOCATION */
		tmp_score -= get_general_probability(key, "KEY");		
		if (tmp_score > of_ptr[NEX_PMI]) of_ptr[NEX_PMI] = tmp_score;
		if (tmp_score > tmp_ne_ct_score) tmp_ne_ct_score = tmp_score;
	    }

	    /* P(����|���٤�:ư2,���) */
	    tmp_score = ex_prob = get_ex_probability(ctm_ptr->tcf_element_num[i], &(tcf_ptr->cf), 
						     entity_ptr->mention[j]->tag_ptr, e_num, ctm_ptr->cf_ptr, FALSE);

	    /* /P(����) */
	    tmp_score -= get_key_probability(entity_ptr->mention[j]->tag_ptr);
	    if (tmp_score > of_ptr[EX_PMI]) of_ptr[EX_PMI] = tmp_score;
	    
	    /* ��̾�ξ���of_ptr[NEX_PMI]�ʲ��ˤϤ��ʤ� */
	    if ((OptGeneralCF & OPT_CF_NE) && check_feature(entity_ptr->mention[j]->tag_ptr->f, "NE:PERSON") &&
		of_ptr[EX_PMI] < 0 && of_ptr[EX_PMI] < of_ptr[NEX_PMI]) of_ptr[EX_PMI] = 0;

	    /* ���ƥ��ꡢ��ͭɽ������׻����줿�ͤȤ�ʿ���ͤ���� */
	    if (ex_prob > FREQ0_ASSINED_SCORE && 
		tmp_ne_ct_score > FREQ0_ASSINED_SCORE)
		tmp_score = (tmp_score + tmp_ne_ct_score) / 2;
	    else if (tmp_ne_ct_score > FREQ0_ASSINED_SCORE)
		tmp_score = tmp_ne_ct_score;	

	    if (tmp_score > max_score) {
		max_score = tmp_score;
	    }
	}
	score += max_score;

	/* SALIENCE_SCORE */
	of_ptr[SALIENCE_CHECK] = (entity_ptr->salience_score >= 1.00) ? 1 : 0;

	/* mention���Ȥ˥�������׻� */	
	max_score = FREQ0_ASSINED_SCORE;
	for (j = 0; j < entity_ptr->mentioned_num; j++) {
	    tmp_score = 0;

	    /* ���֥��ƥ���Ǥ��ޤ��θ�Ǥ��ʤ�������ɲ� */
	    if (entity_ptr->mention[j]->sent_num == sent_num &&
		check_feature(entity_ptr->mention[j]->tag_ptr->f, "��") &&
		(check_feature(entity_ptr->mention[j]->tag_ptr->f, "NE:PERSON") ||
		 check_feature(entity_ptr->mention[j]->tag_ptr->head_ptr->f, "���ƥ���:��") ||
		 check_feature(entity_ptr->mention[j]->tag_ptr->f, "NE:ORIGANIZATION") ||
		 check_feature(entity_ptr->mention[j]->tag_ptr->head_ptr->f, "���ƥ���:�ȿ�������"))) {
		of_ptr[WA_IN_THE_SENT] = 1;
	    }
	    if (check_feature(entity_ptr->mention[j]->tag_ptr->f, "NE:PERSON")) {
		of_ptr[NE_PERSON] = 1;
	    }

	    /* ��ʬ���ȤϽ��� */
	    if (entity_ptr->mention[j]->sent_num == sent_num &&
		!loc_category[(entity_ptr->mention[j]->tag_ptr)->b_ptr->num]) continue;

	    /* �����оݳʰʳ��δط��Ͻ��� */
	    if (strcmp(entity_ptr->mention[j]->cpp_string, "��") &&
		!match_ellipsis_case(entity_ptr->mention[j]->cpp_string, NULL)) continue;	
	    
	    /* ���֥��ƥ��� */
	    /* ��ά�ʡ�type(S,=,O,N,C)���Ȥ˰��֥��ƥ��ꤴ�Ȥ���Ի�Ȥʤ��Ψ���θ
	       ���֥��ƥ���ϡ�������ʸ�Ǥ���� B + ��ʸ����(4ʸ���ʾ��0)
	       Ʊ��ʸ��Ǥ���� C + loc_category �Ȥ�������(ex. ��-O-C3����-=-B2) */
	    if (tcf_ptr->cf.type == CF_PRED) {
		get_location(loc_name, sent_num, pp_code_to_kstr(pp), entity_ptr->mention[j], FALSE);		
		location_prob = get_general_probability("PMI", loc_name);
		loc_num = convert_locname_id(loc_name);
		if (loc_num != -1) of_ptr[LOCATION_S + loc_num] = 1;
	    }
	    else {
		get_location(loc_name, sent_num, pp_code_to_kstr(pp), entity_ptr->mention[j], TRUE);
		location_prob = get_general_probability("T", loc_name);
	    }
	    tmp_score += location_prob;

	    if (tmp_score > max_score) {
		max_score = tmp_score;
		/* ����Υ������Ȥʤä����ܶ����¸(����ؤ��н�Τ���) */
		ctm_ptr->elem_b_ptr[i] = entity_ptr->mention[j]->tag_ptr;
	    }
	}
	score += max_score;
    }

    /* �б��դ����ʤ��ä������оݳʤ���ޤ�ˤ��� */
    for (e_num = 0; e_num < ctm_ptr->cf_ptr->element_num; e_num++) {
        if (!ctm_ptr->filled_element[e_num] &&
	    match_ellipsis_case(pp_code_to_kstr(ctm_ptr->cf_ptr->pp[e_num][0]), NULL) &&
	    ctm_ptr->cf_ptr->oblig[e_num]) {
	    of_ptr = ctm_ptr->omit_feature[ctm_ptr->cf_ptr->pp[e_num][0] - 1];
	    of_ptr[NO_ASSIGNMENT] = get_case_probability(e_num, ctm_ptr->cf_ptr, FALSE, NULL);
            score += of_ptr[NO_ASSIGNMENT];
	}
    }

    return score;
}

/*==================================================================*/
     int copy_ctm(CF_TAG_MGR *source_ctm, CF_TAG_MGR *target_ctm)
/*==================================================================*/
{
    int i, j;

    target_ctm->score = source_ctm->score;
    target_ctm->cf_ptr = source_ctm->cf_ptr;
    target_ctm->result_num = source_ctm->result_num;
    target_ctm->case_result_num = source_ctm->case_result_num;
    for (i = 0; i < CF_ELEMENT_MAX; i++) {
	target_ctm->filled_element[i] = source_ctm->filled_element[i];
	target_ctm->non_match_element[i] = source_ctm->non_match_element[i];
	target_ctm->cf_element_num[i] = source_ctm->cf_element_num[i];
	target_ctm->tcf_element_num[i] = source_ctm->tcf_element_num[i];
	target_ctm->entity_num[i] = source_ctm->entity_num[i];
	target_ctm->elem_b_ptr[i] = source_ctm->elem_b_ptr[i];
	target_ctm->type[i] = source_ctm->type[i];
    }
    target_ctm->overt_arguments_score = source_ctm->overt_arguments_score;    
    for (i = 0; i < ELLIPSIS_CASE_NUM; i++) {
	for (j = 0; j < O_FEATURE_NUM; j++) {
	    target_ctm->omit_feature[i][j] = source_ctm->omit_feature[i][j];
	}
    }
}

/*==================================================================*/
      int preserve_ctm(CF_TAG_MGR *ctm_ptr, int start, int num)
/*==================================================================*/
{
    /* start���ܤ���num�Ĥ�work_ctm�Υ���������Ӥ���̤ʤ����¸����
       num�Ĥ�work_ctm�Υ������Ϲ߽�˥����Ȥ���Ƥ��뤳�Ȥ��ꤷ�Ƥ���
       ��¸���줿����1������ʤ��ä�����0���֤� */
    int i, j;
    
    for (i = start; i < start + num; i++) {
	
	/* work_ctm�˷�̤���¸ */
	if (ctm_ptr->score > work_ctm[i].score) {	    
	    for (j = start + num - 1; j > i; j--) {
		if (work_ctm[j - 1].score > INITIAL_SCORE) {
		    copy_ctm(&work_ctm[j - 1], &work_ctm[j]);
		}
	    }
	    copy_ctm(ctm_ptr, &work_ctm[i]);
	    return TRUE;
	}
    }
    return FALSE;
}

/*==================================================================*/
int case_analysis_for_anaphora(TAG_DATA *tag_ptr, CF_TAG_MGR *ctm_ptr, int i, int r_num)
/*==================================================================*/
{
    /* ����γʥե졼��ˤĤ��ƾȱ������ѳʲ��Ϥ�¹Ԥ���ؿ�
       �Ƶ�Ū�˸ƤӽФ�
       i�ˤ�tag_ptr->tcf_ptr->cf.element_num�Τ��������å������� 
       r_num�ˤϤ��Τ����ʥե졼��ȴ�Ϣ�դ���줿���Ǥο������� */   
    int j, k, e_num;

    /* ���Ǥ���ޤäƤ���ʥե졼��γʤ�����å� */
    memset(ctm_ptr->filled_element, 0, sizeof(int) * CF_ELEMENT_MAX);
    for (j = 0; j < r_num; j++) {
	ctm_ptr->filled_element[ctm_ptr->cf_element_num[j]] = TRUE;
	
	/* ������Ƥ���ʤ���ޤäƤ����ΤȤ��ư��� */
	for (k = 0; ctm_ptr->cf_ptr->samecase[k][0] != END_M; k++) {
	    if (ctm_ptr->cf_ptr->samecase[k][0] == ctm_ptr->cf_element_num[j])
		ctm_ptr->filled_element[ctm_ptr->cf_ptr->samecase[k][1]] = TRUE;
	    else if (ctm_ptr->cf_ptr->samecase[k][1] == ctm_ptr->cf_element_num[j])
		ctm_ptr->filled_element[ctm_ptr->cf_ptr->samecase[k][0]] = TRUE;
	}
    }
    
    /* �ޤ������å����Ƥ��ʤ����Ǥ������� */
    if (i < tag_ptr->tcf_ptr->cf.element_num) {

	/* ����ʸ��i���ܤγ����Ǥμ�ꤦ���(cf.pp[i][j])����֤˥����å� */
	for (j = 0; tag_ptr->tcf_ptr->cf.pp[i][j] != END_M; j++) {

	    /* ����ʸ��i���ܤγ����Ǥ�ʥե졼���cf.pp[i][j]�ʤ˳�����Ƥ� */
	    for (e_num = 0; e_num < ctm_ptr->cf_ptr->element_num; e_num++) {

		if (tag_ptr->tcf_ptr->cf.pp[i][j] == ctm_ptr->cf_ptr->pp[e_num][0] &&
		    (tag_ptr->tcf_ptr->cf.type != CF_NOUN || 
		     check_feature(tag_ptr->tcf_ptr->elem_b_ptr[i]->f, "��:�γ�"))) {
		    
		    /* �оݤγʤ�������ޤäƤ�������Բ� */
		    if (ctm_ptr->filled_element[e_num] == TRUE) continue;

		    /* ������ǤϽ��� */
		    if (check_feature(tag_ptr->tcf_ptr->elem_b_ptr[i]->f, "�������")) {
			continue;
		    }	
	    		    
		    /* ����ʸ¦�ǥ�ʤ���ľ���ʤǤ�����ϳʥե졼���ľ���ʤΤߤ��б������� */
		    if (0 && tag_ptr->tcf_ptr->cf.type != CF_NOUN &&
			check_feature(tag_ptr->tcf_ptr->elem_b_ptr[i]->f, "����") &&
			ctm_ptr->cf_ptr->pp[e_num][0] == pp_kstr_to_code("��") &&
			tag_ptr->tcf_ptr->cf.adjacent[i] && !(ctm_ptr->cf_ptr->adjacent[e_num])) {
			continue;
		    }

		    /* ̾��ʥե졼��γʤ�"��"�ȤʤäƤ���Τ�ɽ����"��"���ѹ� */
		    if (tag_ptr->tcf_ptr->cf.type == CF_NOUN) {
			ctm_ptr->cf_ptr->pp[e_num][0] = pp_kstr_to_code("��");
		    }

		    /* �б��դ���̤�Ͽ */
		    ctm_ptr->elem_b_ptr[r_num] = tag_ptr->tcf_ptr->elem_b_ptr[i];
		    ctm_ptr->cf_element_num[r_num] = e_num;
		    ctm_ptr->tcf_element_num[r_num] = i;
    		    ctm_ptr->type[r_num] = tag_ptr->tcf_ptr->elem_b_num[i] == -1 ? 'N' : 'C';
		    ctm_ptr->entity_num[r_num] = ctm_ptr->elem_b_ptr[r_num]->mention_mgr.mention->entity->num;

		    /* i+1���ܤ����ǤΥ����å��� */
		    case_analysis_for_anaphora(tag_ptr, ctm_ptr, i + 1, r_num + 1);
		}
	    }    
	}

	/* �����Ǥ������Ƥʤ���� */
	/* ����ʸ��i���ܤ����Ǥ��б��դ����ʤ��ä����Ȥ�Ͽ */
	ctm_ptr->non_match_element[i - r_num] = i; 
	case_analysis_for_anaphora(tag_ptr, ctm_ptr, i + 1, r_num);
    }

    /* ���٤ƤΥ����å�����λ������� */
    else {
	/* �����ʳ���r_num�Ĥ��б��դ����Ƥ��� */
	ctm_ptr->result_num = ctm_ptr->case_result_num = r_num;
	/* ��������׻� */
	ctm_ptr->score = ctm_ptr->overt_arguments_score = calc_score_of_ctm(ctm_ptr, tag_ptr->tcf_ptr);
	/* ��������̤���¸ */
	preserve_ctm(ctm_ptr, 0, CASE_CANDIDATE_MAX);	
    }

    return TRUE;
}

/*==================================================================*/
int ellipsis_analysis(TAG_DATA *tag_ptr, CF_TAG_MGR *ctm_ptr, int i, int r_num)
/*==================================================================*/
{
    /* ����Ȥʤ�ʥե졼��ȳ����Ǥ��б��դ��ˤĤ��ƾ�ά���Ϥ�¹Ԥ���ؿ�
       �Ƶ�Ū�˸ƤӽФ� 
       i�ˤ�ELLIPSIS_CASE_LIST[]�Τ��������å�������������
       r_num�ˤϳʥե졼��ȴ�Ϣ�դ���줿���Ǥο�������
       (�ʲ��Ϥη�̴�Ϣ�դ���줿��Τ�ޤ�) */
    int j, k, e_num, exist_flag;
    TAG_DATA *para_ptr;
    int pre_filled_element[CF_ELEMENT_MAX], pre_filled_entity[ENTITY_MAX];

    /* �Ƶ�����filled_element, filled_entity����¸ */
    memcpy(pre_filled_element, ctm_ptr->filled_element, sizeof(int) * CF_ELEMENT_MAX);
    memcpy(pre_filled_entity, ctm_ptr->filled_entity, sizeof(int) * ENTITY_MAX);

    /* ���Ǥ���ޤäƤ���ʥե졼��γʤ�����å� */
    memset(ctm_ptr->filled_element, 0, sizeof(int) * CF_ELEMENT_MAX);
    memset(ctm_ptr->filled_entity, 0, sizeof(int) * ENTITY_MAX);
    for (j = 0; j < r_num; j++) {
	/* ��ޤäƤ���ʤ�����å� */
	ctm_ptr->filled_element[ctm_ptr->cf_element_num[j]] = TRUE;
	/* ������Ƥ���ʤ���ޤäƤ����ΤȤ��ư��� */
	for (k = 0; ctm_ptr->cf_ptr->samecase[k][0] != END_M; k++) {
	    if (ctm_ptr->cf_ptr->samecase[k][0] == ctm_ptr->cf_element_num[j])
		ctm_ptr->filled_element[ctm_ptr->cf_ptr->samecase[k][1]] = TRUE;
	    else if (ctm_ptr->cf_ptr->samecase[k][1] == ctm_ptr->cf_element_num[j])
		ctm_ptr->filled_element[ctm_ptr->cf_ptr->samecase[k][0]] = TRUE;
	}
	/* �ʤ���᤿���Ǥ�����å� */
	ctm_ptr->filled_entity[ctm_ptr->entity_num[j]] = TRUE;

	/* �������Ǥ�����å� */
	if (j < ctm_ptr->case_result_num && /* �ʲ��Ϸ�̤ξ�� */
	    check_feature(ctm_ptr->elem_b_ptr[j]->f, "�θ�") &&
	    substance_tag_ptr(ctm_ptr->elem_b_ptr[j])->para_type == PARA_NORMAL) {
    
	    for (k = 0; substance_tag_ptr(ctm_ptr->elem_b_ptr[j])->parent->child[k]; k++) {
		para_ptr = substance_tag_ptr(substance_tag_ptr(ctm_ptr->elem_b_ptr[j])->parent->child[k]);
		ctm_ptr->filled_entity[para_ptr->mention_mgr.mention->entity->num] = TRUE;
	    }
	}
	else if ( /* ��ά���Ϸ�̤ξ���Ʊ��ʸ�ξ��Τ߹�θ */    
	    entity_manager.entity[ctm_ptr->entity_num[j]].mention[0]->sent_num == tag_ptr->mention_mgr.mention->sent_num &&
	    entity_manager.entity[ctm_ptr->entity_num[j]].mention[0]->tag_ptr->para_type == PARA_NORMAL) {
	    
	    for (k = 0; entity_manager.entity[ctm_ptr->entity_num[j]].mention[0]->tag_ptr->parent->child[k]; k++) {
		para_ptr = substance_tag_ptr(entity_manager.entity[ctm_ptr->entity_num[j]].mention[0]->tag_ptr->parent->child[k]);
		ctm_ptr->filled_entity[para_ptr->mention_mgr.mention->entity->num] = TRUE;
	    }
	}
    }

    /* ��ʬ���Ȥ��Բ� */
    ctm_ptr->filled_entity[tag_ptr->mention_mgr.mention->entity->num] = TRUE;

    /* ��ʬ�η�������Բ� */
    if (tag_ptr->parent &&
	(check_analyze_tag(tag_ptr, FALSE) == CF_PRED ||
	 check_feature(tag_ptr->f, "��:�γ�"))) {
	ctm_ptr->filled_entity[substance_tag_ptr(tag_ptr->parent)->mention_mgr.mention->entity->num] = TRUE;
    }
    /* ��������������� */
    if (tag_ptr->parent && check_feature(tag_ptr->parent->f, "�θ�") &&
	tag_ptr->parent->para_top_p) {
	
	for (j = 0; tag_ptr->parent->child[j]; j++) {
	    para_ptr = substance_tag_ptr(tag_ptr->parent->child[j]);
	    
	    if (para_ptr->num > tag_ptr->num &&
		para_ptr != tag_ptr && check_feature(para_ptr->f, "�θ�") &&
		para_ptr->para_type == PARA_NORMAL)
		ctm_ptr->filled_entity[para_ptr->mention_mgr.mention->entity->num] = TRUE;
	}
    }
   
    /* ��ʬ�˷������Ǥϳʲ��Ϥǽ����ѤߤʤΤ��Բ� */
    for (j = 0; tag_ptr->child[j]; j++) {
	ctm_ptr->filled_entity[substance_tag_ptr(tag_ptr->child[j])->mention_mgr.mention->entity->num] = TRUE;
    }  

    /* ��ʬ����������Ǥ��Բ�(���Ϥ��ؼ��ξ��) */
    if (check_analyze_tag(tag_ptr, FALSE) == CF_NOUN &&
	tag_ptr->para_type == PARA_NORMAL &&
	tag_ptr->parent && tag_ptr->parent->para_top_p) {
	
	for (j = 0; tag_ptr->parent->child[j]; j++) {
	    para_ptr = substance_tag_ptr(tag_ptr->parent->child[j]);
	    if (para_ptr != tag_ptr && check_feature(para_ptr->f, "�θ�") &&
		para_ptr->para_type == PARA_NORMAL)
		ctm_ptr->filled_entity[para_ptr->mention_mgr.mention->entity->num] = TRUE;
	}
    }

    /* �ޤ������å����Ƥ��ʤ���ά�����оݳʤ������� */
    if (*ELLIPSIS_CASE_LIST[i]) {
	exist_flag = 0;
	/* ���٤Ƥγʥ���åȤ�Ĵ�١��ʤ�ELLIPSIS_CASE_LIST[i]�Ȱ��פ��Ƥ�����б��դ����������� */
	for (e_num = 0; e_num < ctm_ptr->cf_ptr->element_num; e_num++) {
	    /* ̾��ξ����оݤγʤ�γʤȤ��ư��� */
	    if (tag_ptr->tcf_ptr->cf.type == CF_NOUN)
		ctm_ptr->cf_ptr->pp[e_num][0] = pp_kstr_to_code("��");			    
	    /* �ʤΰ��פ�����å� */
	    if (ctm_ptr->cf_ptr->pp[e_num][0] != pp_kstr_to_code(ELLIPSIS_CASE_LIST[i])) continue;
	    exist_flag = 1;	    

	    /* ���Ǥ���ޤäƤ������ϼ��γʤ�����å����� */
	    if (ctm_ptr->filled_element[e_num] == TRUE) {
		ellipsis_analysis(tag_ptr, ctm_ptr, i + 1, r_num);
	    }
	    else {
 		for (k = 0; k < entity_manager.num; k++) {
		    /* salience_score��SALIENCE_THRESHOLD�ʲ��ʤ����Ȥ��ʤ�
		       �����������оݤ����äƤ���ɽ����
		       �γʤξ��ǡ�Ʊ��ʸ��ǥγʤǽи����Ƥ������ǤϽ��� */
		    if ((entity_manager.entity[k].salience_score <= SALIENCE_THRESHOLD) &&
			!(tag_ptr->tcf_ptr->cf.type == CF_NOUN && 
			  entity_manager.entity[k].tmp_salience_flag) &&
			!(tag_ptr->parent &&
			  substance_tag_ptr(tag_ptr->parent)->mention_mgr.mention->entity->num == 
			  entity_manager.entity[k].num)) continue;

		    /* �оݤ�ENTITY�����Ǥ��б��դ����Ƥ�������Բ� */
		    if (ctm_ptr->filled_entity[k]) continue;

		    /* ��������Ի���䤫�����(����Ū) */
		    if (check_feature(entity_manager.entity[k].mention[0]->tag_ptr->f, "�����")) continue;

		    /* �б��դ���̤�Ͽ
		       (���ܶ�Ȥ��б��դ��ϼ�äƤ��ʤ�����elem_b_ptr�ϻ��Ѥ��ʤ�) */
		    ctm_ptr->cf_element_num[r_num] = e_num;
		    ctm_ptr->entity_num[r_num] = k;
		    
		    /* ���γʤΥ����å��� */
		    ellipsis_analysis(tag_ptr, ctm_ptr, i + 1, r_num + 1);
		}
		/* ���ʤ��Ǽ��γʤ�(������) */
		ellipsis_analysis(tag_ptr, ctm_ptr, i + 1, r_num);
	    }
	}
	/* �оݤγʤ��ʥե졼���¸�ߤ��ʤ����ϼ��γʤ� */
	if (!exist_flag) ellipsis_analysis(tag_ptr, ctm_ptr, i + 1, r_num);
    }
    
    /* ���٤ƤΥ����å�����λ������� */
    else {
	/* �����ʳ���r_num�Ĥ��б��դ����Ƥ��� */
	ctm_ptr->result_num = r_num;
	for (j = ctm_ptr->case_result_num; j < r_num; j++) ctm_ptr->type[j] = 'O';

	/* ��������׻�(���ǥ롢Ϣ�۾ȱ����Ϥ˻���) */
	if ((OptAnaphora & OPT_ANAPHORA_PROB) || tag_ptr->tcf_ptr->cf.type == CF_NOUN) {
	    ctm_ptr->score = calc_ellipsis_score_of_ctm(ctm_ptr, tag_ptr->tcf_ptr) + ctm_ptr->overt_arguments_score;
	}
	/* ��������׻�(�����п���ǥ�) */
	else {
	    calc_ellipsis_score_of_ctm(ctm_ptr, tag_ptr->tcf_ptr);
	    ctm_ptr->score = ctm_ptr->overt_arguments_score * overt_arguments_weight;
    	    for (j = 0; j < ELLIPSIS_CASE_NUM; j++) {
		for (k = 0; k < O_FEATURE_NUM; k++) {
		    ctm_ptr->score += (ctm_ptr->omit_feature[j][k] == INITIAL_SCORE) ?
			0 : ctm_ptr->omit_feature[j][k] * case_feature_weight[j][k];
		}
	    }   
	}

	/* ���Ϥ��ؼ��ξ���"����"�˽�������Ƥ������
	   �б��դ������ʤ��ä�����log(4)���餤�ڥʥ�ƥ�:todo */
	if (tag_ptr->tcf_ptr->cf.type == CF_NOUN && 
	    check_analyze_tag(tag_ptr, TRUE) && r_num == 0) ctm_ptr->score += -1.3863;

	/* ��������̤���¸ */
	preserve_ctm(ctm_ptr, CASE_CANDIDATE_MAX, ELLIPSIS_RESULT_MAX);
    }   
    
    /* filled_element, filled_entity�򸵤��᤹ */
    memcpy(ctm_ptr->filled_element, pre_filled_element, sizeof(int) * CF_ELEMENT_MAX);
    memcpy(ctm_ptr->filled_entity, pre_filled_entity, sizeof(int) * ENTITY_MAX);
    return TRUE;
}

/*==================================================================*/
	    int ellipsis_analysis_main(TAG_DATA *tag_ptr)
/*==================================================================*/
{
    /* ������ܶ���оݤȤ��ƾ�ά���Ϥ�Ԥ��ؿ� */
    /* �ʥե졼�ऴ�Ȥ˥롼�פ�� */
    int i, j, k, frame_num = 0, rnum_check_flag;
    char cp[WORD_LEN_MAX], aresult[WORD_LEN_MAX], gresult[WORD_LEN_MAX];
    CASE_FRAME **cf_array;
    CF_TAG_MGR *ctm_ptr = work_ctm + CASE_CANDIDATE_MAX + ELLIPSIS_RESULT_MAX;
    MENTION *mention_ptr;

    /* ���Ѥ���ʥե졼������� */
    cf_array = (CASE_FRAME **)malloc_data(sizeof(CASE_FRAME *)*tag_ptr->cf_num, "ellipsis_analysis_main");
    frame_num = set_cf_candidate(tag_ptr, cf_array);
    
    if (OptDisplay == OPT_DEBUG) printf(";;CASE FRAME NUM: %d\n", frame_num);

    /* work_ctm�Υ����������� */
    for (i = 0; i < CASE_CANDIDATE_MAX + ELLIPSIS_RESULT_MAX; i++) 
	work_ctm[i].score = INITIAL_SCORE;

    /* FRAME_FOR_ZERO_MAX�İʾ�γʥե졼��ϥ����å����ʤ� */
    if (frame_num > FRAME_FOR_ZERO_MAX) frame_num = FRAME_FOR_ZERO_MAX;

    /* �ȱ������ѳʲ���(���CASE_CANDIDATE_MAX�Ĥη�̤��ݻ�����) */
    for (i = 0; i < frame_num; i++) {

	/* OR �γʥե졼��(�¥ե졼��)����� */
	if (((*(cf_array + i))->etcflag & CF_SUM) && frame_num != 1) {
	    continue;
	}

	/* ctm_ptr�ν���� */
	ctm_ptr->score = INITIAL_SCORE;

	/* �ʥե졼������ */
 	ctm_ptr->cf_ptr = *(cf_array + i);

	/* �ʲ��� */
	case_analysis_for_anaphora(tag_ptr, ctm_ptr, 0, 0);	
    }
    if (work_ctm[0].score == INITIAL_SCORE) return FALSE;
    
    if (OptDisplay == OPT_DEBUG || OptExpress == OPT_TABLE) {
	for (i = 0; i < CASE_CANDIDATE_MAX; i++) {
	    if (work_ctm[i].score == INITIAL_SCORE ||
		work_ctm[i].score < work_ctm[0].score - CASE_CAND_DIF_MAX) break;

	    printf(";;�ʲ��ϸ���%d-%d:%2d %.3f %s",
		   tag_ptr->mention_mgr.mention->sent_num, tag_ptr->num,
		   i + 1, work_ctm[i].score, work_ctm[i].cf_ptr->cf_id);

	    for (j = 0; j < work_ctm[i].result_num; j++) {
		printf(" %s%s:%s",
		       work_ctm[i].cf_ptr->adjacent[work_ctm[i].cf_element_num[j]] ? "*" : "-",
		       pp_code_to_kstr(work_ctm[i].cf_ptr->pp[work_ctm[i].cf_element_num[j]][0]),
		       work_ctm[i].elem_b_ptr[j]->head_ptr->Goi2);
	    }
	    for (j = 0; j < work_ctm[i].cf_ptr->element_num; j++) {
		if (!work_ctm[i].filled_element[j] && 
		    match_ellipsis_case(pp_code_to_kstr(work_ctm[i].cf_ptr->pp[j][0]), NULL))
		    printf(" %s:��", pp_code_to_kstr(work_ctm[i].cf_ptr->pp[j][0]));
	    }
	    printf("\n");
	}
    }
    
    /* �嵭���б��դ����Ф��ƾ�ά���Ϥ�¹Ԥ��� */
    for (i = 0; i < CASE_CANDIDATE_MAX; i++) {
	if ((i > 0 && work_ctm[i].score == INITIAL_SCORE) ||
	    work_ctm[i].score < work_ctm[0].score - CASE_CAND_DIF_MAX) break;
	copy_ctm(&work_ctm[i], ctm_ptr);
	ellipsis_analysis(tag_ptr, ctm_ptr, 0, ctm_ptr->result_num);
    }

    if (OptAnaphora & OPT_TRAIN) {
	/* ���٤Ƥγ��б��դ����ʤ����Ͻ��Ϥ��ʤ� */
	rnum_check_flag = 0;
	for (i = CASE_CANDIDATE_MAX; i < CASE_CANDIDATE_MAX + ELLIPSIS_RESULT_MAX; i++) {
 	    if (work_ctm[i].score == INITIAL_SCORE) break;
	    if (work_ctm[i].result_num - work_ctm[i].case_result_num > 0) {
		rnum_check_flag = 1;
		break;
	    }
	}
	if (rnum_check_flag) {
	    /* ������Ϥ����� */
	    gresult[0] = '\0';
	    for (j = 1; j < tag_ptr->mention_mgr.num; j++) {
		mention_ptr = tag_ptr->mention_mgr.mention + j;	    
		if (mention_ptr->type == 'O') {
		    sprintf(cp, " %s:%d", mention_ptr->cpp_string, mention_ptr->entity->num);
		    strcat(gresult, cp);
		}
	    }

	    for (i = CASE_CANDIDATE_MAX; i < CASE_CANDIDATE_MAX + ELLIPSIS_RESULT_MAX; i++) {
		if (work_ctm[i].score == INITIAL_SCORE) break;
		
		/* ���Ϸ�̤����� */
		aresult[0] = '\0';
		for (j = work_ctm[i].case_result_num; j < work_ctm[i].result_num; j++) {
		    sprintf(cp, " %s:%d",
			    pp_code_to_kstr(work_ctm[i].cf_ptr->pp[work_ctm[i].cf_element_num[j]][0]),
			    (entity_manager.entity + work_ctm[i].entity_num[j])->num);
		    strcat(aresult, cp);
		}

		/* �����򡢤ޤ��ϡ�����������Ϥ򤷤�����
		   ��ά��Ϣ����������0�ʤ��ʤ����ά�����оݳʤ����٤�ľ�ܳ����Ǥˤ�ä���ޤ�ˤǤ�������Ͻ��Ϥ��ʤ� */
		if ((strcmp(aresult, gresult) || !rnum_check_flag) &&
		    work_ctm[i].omit_feature[0][NO_ASSIGNMENT] == INITIAL_SCORE &&
		    work_ctm[i].omit_feature[1][NO_ASSIGNMENT] == INITIAL_SCORE &&
		    work_ctm[i].omit_feature[2][NO_ASSIGNMENT] == INITIAL_SCORE) continue;
		
		/* �������� */
		printf(";;<%s>%d FEATURE: %d, %f,", aresult, i, !strcmp(aresult, gresult) ? 1 : 0,
		       work_ctm[i].overt_arguments_score);
		for (j = 0; j < ELLIPSIS_CASE_NUM; j++) {
		    for (k = 0; k < O_FEATURE_NUM; k++) {
			(work_ctm[i].omit_feature[j][k] == INITIAL_SCORE) ?
			    printf(" 0,") : 
			    (work_ctm[i].omit_feature[j][k] == 0.0) ?
			    printf(" 0,") : 
			    (work_ctm[i].omit_feature[j][k] == 1.0) ?
			    printf(" 1,") : printf(" %f,", work_ctm[i].omit_feature[j][k]);
		    }
		}
		printf("\n");
		if (!strcmp(aresult, gresult)) rnum_check_flag = 0;
	    }		
	    
	    /* ���䤴�Ȥζ��ڤ�Τ���Υ��ߡ����� */
	    printf(";;<dummy %s> FEATURE: -1,", gresult);
	    for (j = 0; j < ELLIPSIS_CASE_NUM * O_FEATURE_NUM + 1; j++) printf(" 0,");
	    printf("\n");
	}
    }

    if (OptDisplay == OPT_DEBUG || OptExpress == OPT_TABLE) {
	for (i = CASE_CANDIDATE_MAX; i < CASE_CANDIDATE_MAX + ELLIPSIS_RESULT_MAX; i++) {
 	    if (work_ctm[i].score == INITIAL_SCORE) break;
	    printf(";;��ά���ϸ���%d-%d:%2d %.3f %s", 
		   tag_ptr->mention_mgr.mention->sent_num,
		   tag_ptr->num, i - CASE_CANDIDATE_MAX + 1, 
		   work_ctm[i].score, work_ctm[i].cf_ptr->cf_id);

	    for (j = 0; j < work_ctm[i].result_num; j++) {
		printf(" %s%s:%s%d",
		       (j < work_ctm[i].case_result_num) ? "" : "*",
		       pp_code_to_kstr(work_ctm[i].cf_ptr->pp[work_ctm[i].cf_element_num[j]][0]),
		       (entity_manager.entity + work_ctm[i].entity_num[j])->name,
		       (entity_manager.entity + work_ctm[i].entity_num[j])->num);
	    }
	    for (j = 0; j < work_ctm[i].cf_ptr->element_num; j++) {
		if (!work_ctm[i].filled_element[j] && 
		    match_ellipsis_case(pp_code_to_kstr(work_ctm[i].cf_ptr->pp[j][0]), NULL)) 
		    printf(" %s:%s", pp_code_to_kstr(work_ctm[i].cf_ptr->pp[j][0]),
			   (work_ctm[i].cf_ptr->oblig[j]) ? "��" : "-");
	    }	    
	    if (tag_ptr->tcf_ptr->cf.type != CF_NOUN) {
		printf(" (0:%.2f", work_ctm[i].overt_arguments_score);
		for (j = 0; j < ELLIPSIS_CASE_NUM; j++) {
		    printf("|%s", ELLIPSIS_CASE_LIST_VERB[j]);
		    for (k = 0; k < O_FEATURE_NUM; k++) {
			if (work_ctm[i].omit_feature[j][k] != INITIAL_SCORE)
			    printf(",%d:%.2f", O_FEATURE_NUM * j + k + 1, work_ctm[i].omit_feature[j][k]);
		    }
		}
		printf(")");
	    }
	    printf("\n");
	}
    }
   
    /* BEST�����¸ */
    if (work_ctm[CASE_CANDIDATE_MAX].score == INITIAL_SCORE) return FALSE;
    copy_ctm(&work_ctm[CASE_CANDIDATE_MAX], tag_ptr->ctm_ptr);
    strcpy(tag_ptr->mention_mgr.cf_id, work_ctm[CASE_CANDIDATE_MAX].cf_ptr->cf_id);
    tag_ptr->mention_mgr.cf_ptr = work_ctm[CASE_CANDIDATE_MAX].cf_ptr;

    /* �ʥե졼������ */
    free(cf_array);

    return TRUE;
}

/*==================================================================*/
    int make_new_entity(TAG_DATA *tag_ptr, MENTION_MGR *mention_mgr)
/*==================================================================*/
{    
    char *cp;
    ENTITY *entity_ptr;

    entity_ptr = entity_manager.entity + entity_manager.num;
    entity_ptr->num = entity_ptr->output_num = entity_manager.num;
    entity_manager.num++;				
    entity_ptr->mention[0] = mention_mgr->mention;
    entity_ptr->mentioned_num = 1;

    /* ��Ի�ˤʤ�䤹��(����Ū��ʸ��缭�ʤ�1) */
    entity_ptr->salience_score = 
	(tag_ptr->inum > 0 || /* ʸ����Ǹ�δ��ܶ�Ǥʤ� */
	 !check_feature(tag_ptr->f, "�ȱ������") ||
	 check_feature(tag_ptr->f, "NE��")) ? 0 : 
	((check_feature(tag_ptr->f, "��") || check_feature(tag_ptr->f, "��")) &&
	 !check_feature(tag_ptr->f, "��̽�") ||
	 check_feature(tag_ptr->f, "ʸ��")) ? SALIENCE_THEMA : /* ʸ�� */
	(check_feature(tag_ptr->f, "����") && tag_ptr->para_type != PARA_NORMAL ||
	 check_feature(tag_ptr->b_ptr->f, "ʸƬ") ||
	 check_feature(tag_ptr->f, "��:����") ||
	 check_feature(tag_ptr->f, "��:���")) ? SALIENCE_CANDIDATE : SALIENCE_NORMAL;
    if (check_feature(tag_ptr->f, "��:�˳�") || check_feature(tag_ptr->f, "��:�γ�"))
	entity_ptr->tmp_salience_flag = 1;

    /* ENTITY��̾�� */
    if (cp = check_feature(tag_ptr->f, "NE")) {
	strcpy(entity_ptr->name, cp + strlen("NE:"));
    }
    else if (cp = check_feature(tag_ptr->f, "�ȱ������")) {
	strcpy(entity_ptr->name, cp + strlen("�ȱ������:"));
    }
    else {
	strcpy(entity_ptr->name, tag_ptr->head_ptr->Goi2);
    }

    mention_mgr->mention->entity = entity_ptr;	    
    mention_mgr->mention->explicit_mention = NULL;    
    strcpy(mention_mgr->mention->cpp_string, "��");
    if ((cp = check_feature(tag_ptr->f, "��"))) {
	strcpy(mention_mgr->mention->spp_string, cp + strlen("��:"));
    }
    else {
	strcpy(mention_mgr->mention->spp_string, "��");
    }
    mention_mgr->mention->type = 'S'; /* ��ʬ���� */   
}

/*==================================================================*/
	 void print_all_location_category(TAG_DATA *tag_ptr)
/*==================================================================*/
{
    int i, j, diff_sen;
    char *cp, type, rel[SMALL_DATA_LEN], loc_name[SMALL_DATA_LEN];
    ENTITY *entity_ptr;
    MENTION *mention_ptr;

    for (i = 0; i < entity_manager.num; i++) {
	mention_ptr = substance_tag_ptr(tag_ptr)->mention_mgr.mention;
	entity_ptr = entity_manager.entity + i;
	
	if (entity_ptr->salience_score == 0) continue;
			
	/* ��ʸ�����mention����äƤ��뤫�ɤ����Υ����å� */
	diff_sen = 4;
	for (j = 0; j < entity_ptr->mentioned_num; j++) {
	    if (mention_ptr->sent_num == entity_ptr->mention[j]->sent_num &&
		loc_category[(entity_ptr->mention[j]->tag_ptr)->b_ptr->num] == LOC_SELF) continue;
	    
	    if (mention_ptr->sent_num - entity_ptr->mention[j]->sent_num < diff_sen)
		diff_sen = mention_ptr->sent_num - entity_ptr->mention[j]->sent_num;
	}
	
	for (j = 0; j < entity_ptr->mentioned_num; j++) {
	    /* ��äȤ�᤯��ʸ�˽и�����mention�Τ߽��� */
	    if (mention_ptr->sent_num - entity_ptr->mention[j]->sent_num > diff_sen)
		continue;
	    
	    if ( /* ��ʬ���ȤϤΤ��� */
		entity_ptr->mention[j]->sent_num == mention_ptr->sent_num &&
		loc_category[(entity_ptr->mention[j]->tag_ptr)->b_ptr->num] == LOC_SELF) continue;
	    
	    if (get_location(loc_name, mention_ptr->sent_num, 
			     check_analyze_tag(tag_ptr, FALSE) == CF_PRED ? "ư" : "̾",
			     entity_ptr->mention[j], FALSE)) {
		printf(";;LOCATION-ALL: %s", loc_name);
		
		if (cp = check_feature(tag_ptr->f, "�ʲ��Ϸ��")) {		
		    for (cp = strchr(cp + strlen("�ʲ��Ϸ��:"), ':') + 1; *cp; cp++) {
			if (*cp == ':' || *cp == ';') {
			    if (sscanf(cp + 1, "%[^/]/%c/", rel, &type) &&
				match_ellipsis_case(rel, NULL) && (type == 'C' || type == 'N')) {
				printf(" -%s", rel);
			    }
			}
		    }
		}
		printf("\n");
	    }
	}
    }
}

/*==================================================================*/
	    int make_context_structure(SENTENCE_DATA *sp)
/*==================================================================*/
{
    /* �����Ȳ��Ϸ�̤��ɤ߹��ߡ���ά���Ϥ�Ԥ�ʸ�ι�¤���ۤ��� */
    int i, j, check_result;
    char *cp;
    TAG_DATA *tag_ptr;
    CF_PRED_MGR *cpm_ptr;
    MENTION_MGR *mention_mgr;
   
    /* ��ά�ʳ���MENTION�ν��� */
    for (i = 0; i < sp->Tag_num; i++) { /* ����ʸ�Υ���ñ��:i���ܤΥ����ˤĤ��� */
	tag_ptr = substance_tag_ptr(sp->tag_data + i);

	/* ��ʬ����(MENTION)������ */       
	mention_mgr = &(tag_ptr->mention_mgr);
	mention_mgr->mention->tag_num = i;
	mention_mgr->mention->sent_num = sp->Sen_num;
	mention_mgr->mention->tag_ptr = tag_ptr;
	mention_mgr->mention->entity = NULL;
	mention_mgr->mention->explicit_mention = NULL;
	mention_mgr->mention->salience_score = 0;
	mention_mgr->num = 1;

	/* ���Ϥ���������ɤ߹����� */
	if (OptReadFeature & OPT_COREFER) {
	    if (cp = check_feature(tag_ptr->f, "�ʲ��Ϸ��")) {		
		for (cp = strchr(cp + strlen("�ʲ��Ϸ��:"), ':') + 1; *cp; cp++) {
		    if (*cp == ':' || *cp == ';') {
			if (read_one_annotation(sp, tag_ptr, cp + 1, TRUE))
			    assign_cfeature(&(tag_ptr->f), "������", FALSE);
		    }
		}
	    }
	}
	/* ��ư���Ϥξ�� */
	else if (cp = check_feature(tag_ptr->f, "�Զ�����")) {
	    read_one_annotation(sp, tag_ptr, cp + strlen("�Զ�����:"), TRUE);
	}

	/* ������ENTITY�Ǥ����� */	
	if (!mention_mgr->mention->entity) {
	    make_new_entity(tag_ptr, mention_mgr);
	}
    }

    /* ��ά���Ϥ�Ԥ���� */
    for (i = sp->Tag_num - 1; i >= 0; i--) { /* ����ʸ�Υ���ñ��:i���ܤΥ����ˤĤ��� */
	tag_ptr = substance_tag_ptr(sp->tag_data + i);
	check_result = check_analyze_tag(tag_ptr, FALSE);
	if (!check_result) continue;	    
	
	/* �����оݳʤ����� */
	ELLIPSIS_CASE_LIST = (check_result == CF_PRED) ?
	    ELLIPSIS_CASE_LIST_VERB : ELLIPSIS_CASE_LIST_NOUN;

	/* ��ά��MENTION�ν��� */
	/* ���Ϥ���������ɤ߹����� */
	if (OptAnaphora & OPT_TRAIN) {
	    for (j = 0; j < entity_manager.num; j++) entity_manager.entity[j].salience_mem = 0;
	}
	if (check_result == CF_PRED && (OptReadFeature & OPT_ELLIPSIS) || 
	    check_result == CF_NOUN && (OptReadFeature & OPT_REL_NOUN)) {

	    /* ���λ����Ǥγ�Entity��SALIENCE���� */
	    if (OptDisplay == OPT_DEBUG || OptExpress == OPT_TABLE) {
		printf(";;SALIENCE-%d-%d", sp->Sen_num, i);
		for (j = 0; j < entity_manager.num; j++) {
		    printf(":%.3f", (entity_manager.entity + j)->salience_score);
		}
		printf("\n");
	    }
	    
	    /* feature����ʲ��Ϸ�̤���� */
	    if (cp = check_feature(tag_ptr->f, "�ʲ��Ϸ��")) {		
		
		/* �����ȴط��ˤ���ɽ���ϳʲ��Ϸ�̤�������ʤ� */
		if (check_feature(tag_ptr->f, "�θ�") &&
		    (strstr(cp, "=/") || strstr(cp, "=��/") || strstr(cp, "=��/"))) {
		    assign_cfeature(&(tag_ptr->f), "������", FALSE);
		    continue;
		}
		
		for (cp = strchr(cp + strlen("�ʲ��Ϸ��:"), ':') + 1; *cp; cp++) {
		    if (*cp == ':' || *cp == ';') {
			read_one_annotation(sp, tag_ptr, cp + 1, FALSE);
		    }
		}
	    }
	}	

	/* ��ά���Ϥ�Ԥ���硢�ޤ��ϡ���������Ϥ����� */
	if (check_result == CF_PRED && !(OptReadFeature & OPT_ELLIPSIS) ||
	    check_result == CF_NOUN && !(OptReadFeature & OPT_REL_NOUN) ||
	    (OptAnaphora & OPT_TRAIN)) {
	    
	    if (tag_ptr->cf_ptr) {

		assign_cfeature(&(tag_ptr->f), "�Ծ�ά����", FALSE);

		/* cpm_ptr�κ���(����Ū�ˤ�tcf_ptr����Ѥ��뤬��set_tag_case_frame�θƤӽФ�������ӡ�
		   get_ex_probability_with_para���tcf_ptr->cf.pred_b_ptr->cpm_ptr�Ȥ��ƻ��Ѥ��Ƥ���) */
		cpm_ptr = (CF_PRED_MGR *)malloc_data(sizeof(CF_PRED_MGR), "make_context_structure: cpm_ptr");
		init_case_frame(&(cpm_ptr->cf));
		cpm_ptr->pred_b_ptr = tag_ptr;

		/* tag_ptr->tcf_ptr����� */
		tag_ptr->tcf_ptr = (TAG_CASE_FRAME *)malloc_data(sizeof(TAG_CASE_FRAME), "make_context_structure: tcf_ptr");
		set_tag_case_frame(sp, tag_ptr, cpm_ptr);
		
		/* ���֥��ƥ�������� */	    
		mark_loc_category(sp, tag_ptr);
		if (OptAnaphora & OPT_TRAIN) { /* ¸�ߤ��뤹�٤Ƥΰ��֥��ƥ������� */
		    print_all_location_category(tag_ptr); 
		}
		
		/* ���λ����Ǥγ�Entity��SALIENCE���� */
		if (OptDisplay == OPT_DEBUG || OptExpress == OPT_TABLE) {
		    printf(";;SALIENCE-%d-%d", sp->Sen_num, i);
		    for (j = 0; j < entity_manager.num; j++) {
			printf(":%.3f", (entity_manager.entity + j)->salience_score);
		    }
		    printf("\n");
		} 
		
		/* ��ά���ϥᥤ�� */
		tag_ptr->ctm_ptr = (CF_TAG_MGR *)malloc_data(sizeof(CF_TAG_MGR), "make_context_structure: ctm_ptr");
		tag_ptr->ctm_ptr->score = INITIAL_SCORE;
		ellipsis_analysis_main(tag_ptr);
				
		if (!(OptAnaphora & OPT_TRAIN) &&
		    tag_ptr->ctm_ptr->score != INITIAL_SCORE) {
		    expand_result_to_parallel_entity(tag_ptr); /* �������Ǥ�Ÿ������ */
		    anaphora_result_to_entity(tag_ptr); /* ���Ϸ�̤�ENTITY�ȴ�Ϣ�դ��� */
		}
		if (OptAnaphora & OPT_TRAIN) {
		    for (j = 0; j < entity_manager.num; j++) 
			entity_manager.entity[j].salience_score += entity_manager.entity[j].salience_mem;
		}

		/* �������� */
		free(tag_ptr->ctm_ptr);
		free(tag_ptr->tcf_ptr);
		clear_case_frame(&(cpm_ptr->cf));
		free(tag_ptr->cpm_ptr);
	    }
	}
    }
}

/*==================================================================*/
		   void print_entities(int sen_num)
/*==================================================================*/
{
    int i, j;
    char *cp;
    MENTION *mention_ptr;
    ENTITY *entity_ptr;
    FEATURE *fp;
    MRPH_DATA m;

    printf(";;\n;;SENTENCE %d\n", sen_num + base_sentence_num); 
    for (i = 0; i < entity_manager.num; i++) {
	entity_ptr = entity_manager.entity + i;
	if (entity_ptr->salience_score < 0.01 && entity_ptr->mentioned_num < 2 ||
	    entity_ptr->salience_score == 0) continue;

	printf(";; ENTITY %d [ %s ] %f {\n", entity_ptr->output_num + base_entity_num, entity_ptr->name, entity_ptr->salience_score);
	for (j = 0; j < entity_ptr->mentioned_num; j++) {
	    mention_ptr = entity_ptr->mention[j];
	    printf(";;\tMENTION%3d {", j);
	    printf(" SEN:%3d", mention_ptr->sent_num + base_sentence_num);
	    printf(" TAG:%3d", mention_ptr->tag_num);
	    printf(" (%3d)", mention_ptr->tag_ptr->head_ptr->Num);
	    printf(" CPP: %4s", mention_ptr->cpp_string);
	    printf(" SPP: %4s", mention_ptr->spp_string);
	    printf(" TYPE: %c", mention_ptr->type);
	    printf(" SS: %.3f", mention_ptr->salience_score);
	    printf(" WORD: %s", mention_ptr->tag_ptr->head_ptr->Goi2);

	    /* �ʥե졼��Υ��Х�å���Ĵ�٤�ݤ�ɬ�פȤʤ���� */
	    if (OptDisplay == OPT_DETAIL) {

		/* �Ѹ��ξ�� */
		if (check_feature(mention_ptr->tag_ptr->f, "�Ѹ�") &&
		    (mention_ptr->type == 'C' || mention_ptr->type == 'N' || mention_ptr->type == 'O')) {

		    printf(" POS: %s", check_feature(mention_ptr->tag_ptr->f, "�Ѹ�") + strlen("�Ѹ�:"));
		    cp = make_pred_string(mention_ptr->tag_ptr, NULL, NULL, OptCaseFlag & OPT_CASE_USE_REP_CF, CF_PRED, FALSE);
		    printf(" KEY: %s", cp);
		    free(cp);

		    /* ��ɽɽ����ۣ����Ѹ��ξ�� */
		    if (check_feature(mention_ptr->tag_ptr->head_ptr->f, "����ۣ��")) {
			
			fp = mention_ptr->tag_ptr->head_ptr->f;
			while (fp) {
			    if (!strncmp(fp->cp, "ALT-", 4)) {
				sscanf(fp->cp + 4, "%[^-]-%[^-]-%[^-]-%d-%d-%d-%d-%[^\n]", 
				       m.Goi2, m.Yomi, m.Goi, 
				       &m.Hinshi, &m.Bunrui, 
				       &m.Katuyou_Kata, &m.Katuyou_Kei, m.Imi);
				cp = make_pred_string(mention_ptr->tag_ptr, &m, NULL, OptCaseFlag & OPT_CASE_USE_REP_CF, CF_PRED, FALSE);
				printf("-%s", cp);
				free(cp);
			    }
			    fp = fp->next;
			}
		    }
		    if (mention_ptr->tag_ptr->voice & VOICE_SHIEKI || 
			check_feature(mention_ptr->tag_ptr->f, "��:����")) {
			printf(" VOICE: C");
		    }
		    else if (mention_ptr->tag_ptr->voice & VOICE_UKEMI ||
			     check_feature(mention_ptr->tag_ptr->f, "��:��ư")) {
			printf(" VOICE: P");
		    }
		    else {
			printf(" VOICE: N");
		    }

		    /* ľ�ܤγ����Ǥδ��ܶ��ֹ� */
		    if (mention_ptr->explicit_mention) {
			printf(" CTAG: %d", mention_ptr->explicit_mention->tag_num);
		    }
		}
		/* �����Ǥξ�� */
		else if (mention_ptr->type == 'S' || mention_ptr->type == '=') {

		    if (mention_ptr->tag_ptr->head_ptr == mention_ptr->tag_ptr->b_ptr->head_ptr) { /* ʸ��缭�Ǥ��뤫�ɤ��� */
			cp = get_bnst_head_canonical_rep(mention_ptr->tag_ptr->b_ptr, OptCaseFlag & OPT_CASE_USE_CN_CF);
		    }
		    else {
			cp = check_feature(mention_ptr->tag_ptr->f, "��������ɽɽ��");
			if (cp) cp += strlen("��������ɽɽ��:");
		    }

		    printf(" POS: %s", Class[mention_ptr->tag_ptr->head_ptr->Hinshi][mention_ptr->tag_ptr->head_ptr->Bunrui].id);
		    printf(" KEY: %s", cp);
		    if (check_feature(mention_ptr->tag_ptr->f, "��ʸ")) {
			printf(" GE: ��ʸ");
		    }
		    else if (check_feature(mention_ptr->tag_ptr->f, "����")) {
			printf(" GE: ����");
		    }
		    else if (check_feature(mention_ptr->tag_ptr->f, "����")) {
			printf(" GE: ����");
		    }
		    if ((cp = check_feature(mention_ptr->tag_ptr->head_ptr->f, "���ƥ���"))) {
			printf(" CT: %s", cp + strlen("���ƥ���:"));
		    }
		    if ((cp = check_feature(mention_ptr->tag_ptr->f, "NE"))) {
			printf(" NE: %s", cp + strlen("NE:"));
		    }
		}
	    }
	    printf(" }\n");
	}
	printf(";; }\n;;\n");
    }
}

/*==================================================================*/
	    void assign_anaphora_result(SENTENCE_DATA *sp)
/*==================================================================*/
{
    /* �ȱ����Ϸ�̤���ܶ��feature����Ϳ */
    int i, j, count;
    char buf[DATA_LEN], tmp[IMI_MAX];
    MENTION *mention_ptr;
    TAG_DATA *tag_ptr;
	     
    for (i = 0; i < sp->Tag_num; i++) {
	tag_ptr = substance_tag_ptr(sp->tag_data + i);

	sprintf(buf, "EID:%d", tag_ptr->mention_mgr.mention->entity->num + base_entity_num);
	assign_cfeature(&(tag_ptr->f), buf, FALSE);
	if (!check_feature(tag_ptr->f, "�Ծ�ά����")) continue;

	sprintf(buf, "�ʹ�¤:%s:", (OptReadFeature & OPT_ELLIPSIS) ? "?" : tag_ptr->mention_mgr.cf_id);		    
	for (j = 1; j < tag_ptr->mention_mgr.num; j++) {
	    mention_ptr = tag_ptr->mention_mgr.mention + j;
	    
	    if (mention_ptr->type == 'N' || mention_ptr->type == 'C' ||
		mention_ptr->type == 'O' || mention_ptr->type == 'D') {
		sprintf(tmp, "%s/%c/%s/%d;", mention_ptr->cpp_string, mention_ptr->type, 
			mention_ptr->entity->name, mention_ptr->entity->num + base_entity_num);
		strcat(buf, tmp);
	    }
	}
	buf[strlen(buf) - 1] = '\0'; /* ������';'��':'���� */
	assign_cfeature(&(tag_ptr->f), buf, FALSE);
    }
}

/*==================================================================*/
			 void decay_entity()
/*==================================================================*/
{
    /* ENTITY�γ����ͤ򸺿ꤵ���� */
    int i;

    for (i = 0; i < entity_manager.num; i++) {
	entity_manager.entity[i].salience_score *= SALIENCE_DECAY_RATE;
	entity_manager.entity[i].tmp_salience_flag = 0;
    }
}

/*==================================================================*/
	      void anaphora_analysis(SENTENCE_DATA *sp)
/*==================================================================*/
{
    if (ModifyWeight[0]) {
	case_feature_weight[0][ASSIGNED] += ModifyWeight[0];
	case_feature_weight[1][ASSIGNED] += ModifyWeight[1];
	case_feature_weight[2][ASSIGNED] += ModifyWeight[2];
	ModifyWeight[0] = 0;
    }
    decay_entity();
    make_context_structure(sentence_data + sp->Sen_num - 1);
    assign_anaphora_result(sentence_data + sp->Sen_num - 1);
    if (OptAnaphora & OPT_PRINT_ENTITY) print_entities(sp->Sen_num);
}
