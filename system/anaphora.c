/*====================================================================

			       �ȱ�����

                                         Ryohei Sasano 2007. 8. 27

    $Id$
====================================================================*/

#include "knp.h"

#define CASE_CANDIDATE_MAX 20  /* �ȱ������ѳʲ��Ϸ�̤��ݻ������ */
#define ELLIPSIS_RESULT_MAX 5  /* ��ά���Ϸ�̤��ݻ������ */
#define INITIAL_SCORE -10000
#define ENTITY_DECAY_RATE 0.7
#define ELLIPSIS_CASE_NUM 3
#define EX_MATCH_WEIGHT 1.0    /* ���㤬�ޥå��������ȤνŤ�:best=0.5 */
#define LOCATION_WEIGHT 1.0    /* ���֥��ƥ���νŤ�:bset=2.0 */

/* ���֥��ƥ���(������Ѹ��Ǥ��뤫����̵��)  */
#define	LOC_SELF             0 /* ��ʬ����   */
#define	LOC_PARENT           1 /* ��         */
#define	LOC_CHILD            2 /* �Ҷ�       */
#define LOC_PARA_PARENT      3 /* ����(��¦) */
#define	LOC_PARA_CHILD       4 /* ����(��¦) */
#define	LOC_PARENT_N_PARENT  5 /* ���θ��ο� */
#define	LOC_PARENT_V_PARENT  6 /* ���Ѹ��ο� */
#define	LOC_OTHERS_BEFORE    7 /* ����¾(��) */
#define	LOC_OTHERS_AFTER     8 /* ����¾(��) */

/* ���ʡ���ʤ���ޤ�ʤ����Υڥʥ�ƥ� */
double ga_penalty, wo_penalty;

/* ���֥��ƥ�����ݻ� */
int loc_category[BNST_MAX];

/* ���Ϸ�̤��ݻ����뤿���ENTITY_CASE_MGR
   ��Ƭ��CASE_CANDIDATE_MAX�Ĥ˾ȱ������ѳʲ��Ϥη�̤ξ�̤��ݻ���
   ����ELLIPSIS_RESULT_MAX�ĤˤϾ�ά���Ϸ�̤Υ٥��Ȳ���ݻ���
   �Ǹ��1�Ĥϸ��ߤβ��Ϸ�̤��ݻ��˻��Ѥ��� */
CF_TAG_MGR work_ctm[CASE_CANDIDATE_MAX + ELLIPSIS_RESULT_MAX + 1];

/* ��ά���Ϥ��оݤȤ���ʤΥꥹ�� */
char *ELLIPSIS_CASE_LIST[ELLIPSIS_CASE_NUM] = {"��", "��", "��"};

/*==================================================================*/
	       void assign_mrph_num(SENTENCE_DATA *sp)
/*==================================================================*/
{
    int i, count = 0;

    /* mrph��ʸ��Ƭ���餽�η����Ǥν���ޤǤ�ʸ���� */
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
int get_location(char *loc_name, int sent_num, char *kstr, MENTION *mention)
/*==================================================================*/
{
    if (mention->sent_num == sent_num) {
/*	sprintf(loc_name, "%s-%c-C%d-%d", */
	sprintf(loc_name, "%s-%c-C%d", 
		kstr, (mention->flag == '=') ? 'S' : mention->flag,
		loc_category[mention->tag_ptr->b_ptr->num]);
/*		loc_category[mention->tag_ptr->b_ptr->num],
		(mention->entity->salience_score >= 3) ? 3 :
		(mention->entity->salience_score >= 2) ? 2 : 1); */
	return TRUE;
    }
    else if (sent_num - mention->sent_num > 0) {
/*	sprintf(loc_name, "%s-%c-B%d-%d", */
	sprintf(loc_name, "%s-%c-B%d", 
		kstr, (mention->flag == '=') ? 'S' : mention->flag,
		(sent_num - mention->sent_num <= 3 ) ? 
		sent_num - mention->sent_num : 0);
/*		sent_num - mention->sent_num : 0,
		(mention->entity->salience_score >= 3) ? 3 :
		(mention->entity->salience_score >= 2) ? 2 : 1); */
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
    for (i = 0; i < bnst_ptr->num; i++) loc_category[i] = LOC_OTHERS_BEFORE; /* ����¾(��) */
    for (i = bnst_ptr->num + 1; i < sp->Bnst_num; i++) 
	loc_category[i] = LOC_OTHERS_AFTER; /* ����¾(��) */
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
		!bnst_ptr->parent->child[i]->para_top_p) { /* todo::�Ȥꤢ��������������̵�� */

		if (bnst_ptr->parent->child[i]->num > bnst_ptr->num)
		    loc_category[bnst_ptr->parent->child[i]->num] = LOC_PARA_PARENT; /* ����(��¦) */
		else if (bnst_ptr->parent->child[i]->num < bnst_ptr->num)
		    loc_category[bnst_ptr->parent->child[i]->num] = LOC_PARA_CHILD; /* ����(�Ҷ�¦) */
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

/*     if (OptDisplay == OPT_DEBUG) { */
/* 	for (i = 0; i < sp->Bnst_num; i++)  */
/* 	    printf(";; %d %s %d ok?\n", bnst_ptr->num, bnst_ptr->Jiritu_Go, loc_category[i]); */
/* 	printf(";;\n"); */
/*     } */
}

/*==================================================================*/
int read_one_annotation(SENTENCE_DATA *sp, TAG_DATA *tag_ptr, char *token, int co_flag)
/*==================================================================*/
{
    /* ���Ϸ�̤���MENTION��ENTITY��������� */
    /* co_flag���������"="�Τߤ�������ʤ�����"="�ʳ������ */
    char flag, rel[SMALL_DATA_LEN], *cp, loc_name[SMALL_DATA_LEN];
    int i, j, tag_num, sent_num, bnst_num;
    MENTION_MGR *mention_mgr = &(tag_ptr->mention_mgr);
    MENTION *mention_ptr = NULL;
    ENTITY *entity_ptr;

    if (!sscanf(token, "%[^/]/%c/%*[^/]/%d/%d/", rel, &flag, &tag_num, &sent_num)) 
	return FALSE;

    /* �����ȴط����ɤ߹��� */
    if (co_flag && !strcmp(rel, "=") && flag == 'O') {
	mention_ptr = mention_mgr->mention;
	mention_ptr->entity = 
	    substance_tag_ptr((sp - sent_num)->tag_data + tag_num)->mention_mgr.mention->entity;
	mention_ptr->salience_score = mention_ptr->entity->salience_score;
	mention_ptr->entity->salience_score += 
	    (check_feature(tag_ptr->f, "��:̤��") ||
	     check_feature(tag_ptr->f, "ʸ��")) ? 2.0 : 1.0;
	strcpy(mention_ptr->cpp_string, "��");
	if ((cp = check_feature(tag_ptr->f, "��"))) {
	    strcpy(mention_ptr->spp_string, cp + strlen("��:"));
	} 
	else {
	    strcpy(mention_ptr->spp_string, "��");
	}
	mention_ptr->flag = '=';

	/* entity��name��NE�Ǥʤ���tag_ptr��NE�ʤ��name���� */
	if (!strchr(mention_ptr->entity->name, ':') &&
	    (cp = check_feature(tag_ptr->f, "NE"))) {
	    strcpy(mention_ptr->entity->name, cp + strlen("NE:"));
	}
    }

    /* �����Ȱʳ��δط� */
    else if (!co_flag &&
	     (flag == 'N' || flag == 'C' || flag == 'O' || flag == 'D') &&    
	     (!strcmp(rel, "��") || !strcmp(rel, "��") || !strcmp(rel, "��") || !strcmp(rel, "��"))) {
	mention_ptr = mention_mgr->mention + mention_mgr->num;
 	mention_ptr->entity = 
	    substance_tag_ptr((sp - sent_num)->tag_data + tag_num)->mention_mgr.mention->entity;
	mention_ptr->salience_score = mention_ptr->entity->salience_score;
	if (flag == 'O') 
	    mention_ptr->entity->salience_score += 0.5;

	mention_ptr->tag_num = mention_mgr->mention->tag_num;
	mention_ptr->sent_num = mention_mgr->mention->sent_num;
	mention_ptr->tag_ptr = 
	    (sentence_data + mention_ptr->sent_num - 1)->tag_data + mention_ptr->tag_num;
	mention_ptr->flag = flag;
	strcpy(mention_ptr->cpp_string, rel);
	if (flag == 'C' && 
	    (cp = check_feature(((sp - sent_num)->tag_data + tag_num)->f, "��"))) {
	    strcpy(mention_ptr->spp_string, cp + strlen("��:"));
	} 
	else {
	    strcpy(mention_ptr->spp_string, "��");
	}
	mention_mgr->num++;
    }

    if (!mention_ptr) return FALSE;
    mention_ptr->entity->mention[mention_ptr->entity->mentioned_num] = mention_ptr;
    mention_ptr->entity->mentioned_num++;
    if (flag == 'O' || !strcmp(rel, "=")) mention_ptr->entity->antecedent_num++;

    /* �ؽ��Ѱ��֥��ƥ���ν��� */
    if (flag == 'O' && strcmp(rel, "=")) {
	mark_loc_category(sp, tag_ptr);    

	for (j = 0; j < entity_manager.num; j++) {
	    entity_ptr = entity_manager.entity + j;
	    
	    for (i = 0; i < entity_ptr->mentioned_num; i++) {
		
		if ( /* ��ʬ���ȤϤΤ��� */
		    entity_ptr->mention[i]->sent_num == mention_ptr->sent_num &&
		    !loc_category[(entity_ptr->mention[i]->tag_ptr)->b_ptr->num]) continue;
		
		if (OptDisplay == OPT_DEBUG &&
		    get_location(loc_name, mention_ptr->sent_num, rel, entity_ptr->mention[i])) {
		    printf(";; LOCATION-LEARN: %s:%c\n", loc_name,
			   entity_ptr == mention_ptr->entity ? 'T' : 'F');
		}
	    }
	}

    }
    return TRUE;
}

/*==================================================================*/
	   int anaphora_result_to_entity(TAG_DATA *tag_ptr)
/*==================================================================*/
{
    /* �ȱ����Ϸ��ENTITY�˴�Ϣ�դ��� */

    int i, j;
    char *cp;
    MENTION_MGR *mention_mgr = &(tag_ptr->mention_mgr);
    MENTION *mention_ptr = NULL;
    CF_TAG_MGR *ctm_ptr = tag_ptr->ctm_ptr; 

    /* �ʡ���ά���Ϸ�̤��ʤ����Ͻ�λ */
    if (!ctm_ptr) return FALSE;
    
    for (i = 0; i < ctm_ptr->result_num; i++) {
	mention_ptr = mention_mgr->mention + mention_mgr->num;
	mention_ptr->entity = entity_manager.entity + ctm_ptr->entity_num[i];
	mention_ptr->tag_num = mention_mgr->mention->tag_num;
	mention_ptr->sent_num = mention_mgr->mention->sent_num;
	mention_ptr->flag = ctm_ptr->flag[i];
	mention_ptr->tag_ptr = 
	    (sentence_data + mention_ptr->sent_num - 1)->tag_data + mention_ptr->tag_num;
	strcpy(mention_ptr->cpp_string, 
	       pp_code_to_kstr(ctm_ptr->cf_ptr->pp[ctm_ptr->cf_element_num[i]][0]));
	mention_ptr->entity->salience_score = mention_ptr->entity->salience_score;
	/* ����¦��ɽ�س�(�ʲ��Ϸ�̤Τ�) */
	if (i < ctm_ptr->case_result_num) {
	    if (tag_ptr->tcf_ptr->cf.pp[ctm_ptr->tcf_element_num[i]][0] >= FUKUGOJI_START &&
		tag_ptr->tcf_ptr->cf.pp[ctm_ptr->tcf_element_num[i]][0] <= FUKUGOJI_END) {
		strcpy(mention_ptr->spp_string, 
		       pp_code_to_kstr(tag_ptr->tcf_ptr->cf.pp[ctm_ptr->tcf_element_num[i]][0]));
	    }
	    else { 
		if ((cp = check_feature(ctm_ptr->elem_b_ptr[i]->f, "��"))) {
		    strcpy(mention_ptr->spp_string, cp + strlen("��:"));
		} 
		else {
		    strcpy(mention_ptr->spp_string, "��");
		}
	    }
	}
	else {
	    strcpy(mention_ptr->spp_string, "��");
	    mention_ptr->entity->salience_score += 0.5;
	}
	mention_mgr->num++;

	mention_ptr->entity->mention[mention_ptr->entity->mentioned_num] = mention_ptr;
	mention_ptr->entity->mentioned_num++;

	/* ���������å�*/
	if (0 && i >= ctm_ptr->case_result_num &&
	    ctm_ptr->elem_b_ptr[i]->parent &&
	    ctm_ptr->elem_b_ptr[i]->parent->para_top_p) {
	    if (OptDisplay == OPT_DEBUG) 
		printf(";; ����:%s\n", ctm_ptr->elem_b_ptr[i]->head_ptr->Goi2);

	    for (j = 0; ctm_ptr->elem_b_ptr[i]->parent->child[j]; j++) {

		mention_ptr = mention_mgr->mention + mention_mgr->num;
		mention_ptr->entity = 
		    ctm_ptr->elem_b_ptr[i]->parent->child[j]->mention_mgr.mention->entity;
		mention_ptr->tag_num = mention_mgr->mention->tag_num;
		mention_ptr->sent_num = mention_mgr->mention->sent_num;
		mention_ptr->flag = ctm_ptr->flag[i];
		mention_ptr->tag_ptr = 
		    (sentence_data + mention_ptr->sent_num - 1)->tag_data + mention_ptr->tag_num;
		strcpy(mention_ptr->cpp_string, 
		       pp_code_to_kstr(ctm_ptr->cf_ptr->pp[ctm_ptr->cf_element_num[i]][0]));
		strcpy(mention_ptr->spp_string, "��");
		mention_mgr->num++;
		
		mention_ptr->entity->mention[mention_ptr->entity->mentioned_num] = mention_ptr;
		mention_ptr->entity->mentioned_num++;
		mention_ptr->entity->salience_score += 0.5;
	    }
	}
    }
    
    return TRUE;
}

/*==================================================================*/
     int set_tag_case_frame(SENTENCE_DATA *sp, TAG_DATA *tag_ptr)
/*==================================================================*/
{
    /* ENTITY_PRED_MGR���������ؿ�
       make_data_cframe���Ѥ�������ʸ�γʹ�¤��������뤿��
       CF_PRED_MGR���ꡢ����cf�򥳥ԡ����Ƥ��� */
    int i;
    TAG_CASE_FRAME *tcf_ptr = tag_ptr->tcf_ptr;
    CF_PRED_MGR *cpm_ptr;
   
    /* cpm�κ��� */
    cpm_ptr = (CF_PRED_MGR *)malloc_data(sizeof(CF_PRED_MGR), "set_tag_case_frame");
    init_case_frame(&(cpm_ptr->cf));
    cpm_ptr->pred_b_ptr = tag_ptr;

    /* ����ʸ¦�γ��������� */
    set_data_cf_type(cpm_ptr);
    make_data_cframe(sp, cpm_ptr);
    
    /* ENTITY_PRED_MGR�����������ʸ¦�γ����Ǥ򥳥ԡ� */
    tcf_ptr->cf = cpm_ptr->cf;
    tcf_ptr->pred_b_ptr = tag_ptr;
    for (i = 0; i < cpm_ptr->cf.element_num; i++) {
	tcf_ptr->elem_b_ptr[i] = substance_tag_ptr(cpm_ptr->elem_b_ptr[i]);
    }

    /* todo::free(cpm_ptr); free����ɬ�פ���
       ��������tcf_ptr->cf.pred_b_ptr->cpm_ptr�ǻȤ��ΤǤޤ��Ǥ��ʤ�
       get_ex_probability_with_para�� */
    return TRUE;
}

/*==================================================================*/
    int set_cf_candidate(TAG_DATA *tag_ptr, CASE_FRAME **cf_array)
/*==================================================================*/
{
    int i, l, frame_num = 0, hiragana_prefer_flag = 0;
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
		hiragana_prefer_flag = 1;
	    }
	    else {
		hiragana_prefer_flag = -1;
	    }
	}
	
	for (l = 0; l < tag_ptr->cf_num; l++) {
	    if ((tag_ptr->cf_ptr + l)->type == tag_ptr->tcf_ptr->cf.type && 
		(hiragana_prefer_flag == 0 || 
		 (hiragana_prefer_flag > 0 && 
		  check_str_type((tag_ptr->cf_ptr + l)->entry) == TYPE_HIRAGANA) || 
		 (hiragana_prefer_flag < 0 && 
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

    int i, e_num, debug = 0;
    double score;

    /* �оݤγʥե졼�ब���򤵤�뤳�ȤΥ����� */
    score = get_cf_probability(&(tcf_ptr->cf), ctm_ptr->cf_ptr);

    /* �б��դ���줿���Ǥ˴ؤ��륹����(�ʲ��Ϸ��) */
    for (i = 0; i < ctm_ptr->case_result_num; i++) {
	e_num = ctm_ptr->cf_element_num[i];
	
	/* �Ȥꤢ��������::ľ���ʤ������������˴ޤޤ�Ƥ����Τ�ͥ�� */
	/*if (tcf_ptr->cf.adjacent[ctm_ptr->tcf_element_num[i]] == TRUE &&
	    get_ex_probability(ctm_ptr->tcf_element_num[i], &(tcf_ptr->cf),
			       NULL, e_num, ctm_ptr->cf_ptr, FALSE) > FREQ0_ASSINED_SCORE)
			       score -= FREQ0_ASSINED_SCORE; */
	    
	score += 
	    get_ex_probability_with_para(ctm_ptr->tcf_element_num[i], &(tcf_ptr->cf), 
					 e_num, ctm_ptr->cf_ptr) * EX_MATCH_WEIGHT +
	    get_case_function_probability(ctm_ptr->tcf_element_num[i], &(tcf_ptr->cf),
					  e_num, ctm_ptr->cf_ptr);

	if (OptDisplay == OPT_DEBUG && debug) 
	    printf(";; %s-%s:%f:%f ", 
		   ctm_ptr->elem_b_ptr[i]->head_ptr->Goi2, 
		   pp_code_to_kstr(ctm_ptr->cf_ptr->pp[e_num][0]),
		   get_ex_probability_with_para(ctm_ptr->tcf_element_num[i], &(tcf_ptr->cf), 
						e_num, ctm_ptr->cf_ptr),
		   get_case_function_probability(ctm_ptr->tcf_element_num[i], &(tcf_ptr->cf), 
						 e_num, ctm_ptr->cf_ptr));      
    }
    /* ����ʸ�γ����ǤΤ����б��դ����ʤ��ä����Ǥ˴ؤ��륹���� */
    for (i = 0; i < tcf_ptr->cf.element_num - ctm_ptr->case_result_num; i++) {
	if (OptDisplay == OPT_DEBUG && debug) 
	    printf(";; %s:%f/n", 
		  (tcf_ptr->elem_b_ptr[ctm_ptr->non_match_element[i]])->head_ptr->Goi2,
		  score);

	score += FREQ0_ASSINED_SCORE * 3 +
	    get_case_function_probability(ctm_ptr->non_match_element[i], &(tcf_ptr->cf), 
					  NIL_ASSIGNED, ctm_ptr->cf_ptr);
    }
    if (OptDisplay == OPT_DEBUG && debug) printf(";; %f ", score);	   
    /* �ʥե졼��γʤ���ޤäƤ��뤫�ɤ����˴ؤ��륹���� */
    for (e_num = 0; e_num < ctm_ptr->cf_ptr->element_num; e_num++) {
	score += get_case_probability(e_num, ctm_ptr->cf_ptr, ctm_ptr->filled_element[e_num]);	
    }
    //if (OptDisplay == OPT_DEBUG && debug) printf(";; %f\n", score);

    return score;
}

/*==================================================================*/
double calc_ellipsis_score_of_ctm(CF_TAG_MGR *ctm_ptr, TAG_CASE_FRAME *tcf_ptr)
/*==================================================================*/
{
    /* �ʥե졼��Ȥ��б��դ��Υ�������׻�����ؿ�(��ά���Ϥ�ɾ��) */
    int i, j, k, l, e_num, debug = 1, sent_num;
    double score = 0, max_score, local_score, tmp_score, prob, penalty;
    char *cp, key[SMALL_DATA_LEN], loc_name[SMALL_DATA_LEN];
    TAG_DATA *child_ptr;
    ENTITY *entity_ptr;

    /* �����оݤδ��ܶ��ʸ�ֹ� */
    sent_num = tcf_ptr->pred_b_ptr->mention_mgr.mention->sent_num;

    /* �б��դ���줿���Ǥ˴ؤ��륹����(��ά���Ϸ��) */
    for (i = ctm_ptr->case_result_num; i < ctm_ptr->result_num; i++) {
	e_num = ctm_ptr->cf_element_num[i];
	entity_ptr = entity_manager.entity + ctm_ptr->entity_num[i]; /* ��Ϣ�դ���줿ENTITY */	

	/* P(���٤�:ư2,���|����) = P(����|���٤�:ư2,���)/P(����) */
	/* flag='S'�ޤ���'='��mention����Ǻ���Ȥʤ��Τ���� */	
	max_score = FREQ0_ASSINED_SCORE;

	for (j = 0; j < entity_ptr->mentioned_num; j++) {
	    if (entity_ptr->mention[j]->flag != 'S' &&
		entity_ptr->mention[j]->flag != '=') continue;

	    /* P(����|���٤�:ư2,���) */
	    tmp_score = 
		get_ex_probability(ctm_ptr->tcf_element_num[i], &(tcf_ptr->cf), 
				   entity_ptr->mention[j]->tag_ptr, e_num, ctm_ptr->cf_ptr, FALSE);
	    /* /P(����) */
	    tmp_score -= get_key_probability(entity_ptr->mention[j]->tag_ptr);	    
	    if (tmp_score > max_score) {
		if (OptDisplay == OPT_DEBUG && debug) 
		    printf(";; %s %s:%f:%f\n", 
			   ctm_ptr->cf_ptr->cf_id,
			   tmp_score + get_key_probability(entity_ptr->mention[j]->tag_ptr),
			   get_key_probability(entity_ptr->mention[j]->tag_ptr), 
			   entity_ptr->mention[j]->tag_ptr->head_ptr->Goi2);
		max_score = tmp_score;
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
		if (OptDisplay == OPT_DEBUG && debug) printf(";; %s:%f(%f)", key, tmp_score, prob);
		tmp_score -= get_general_probability(key, "KEY");
		if (OptDisplay == OPT_DEBUG && debug) printf(":%f\n", tmp_score);
		
		if (tmp_score > max_score) max_score = tmp_score;
	    }

 	    /* ���ƥ��꤬�������P(���٤�:ư2,���|���ƥ���:��)������å� */
	    if ((OptGeneralCF & OPT_CF_CATEGORY) && 
		(cp = check_feature(entity_ptr->mention[j]->tag_ptr->head_ptr->f, "���ƥ���"))) {

		while (cp = strchr(cp, ':')) {
		    cp++;
		    sprintf(key, "CT:%s:", cp);
		    if (/* !strncmp(key, "CT:��:", 6) && */
			(prob = get_ex_ne_probability(key, e_num, ctm_ptr->cf_ptr, TRUE))) {

			/* P(���ƥ���:��|���٤�:ư2,���) */
			tmp_score = log(prob);
			
			/* /P(���ƥ���:��) */
			*strchr(key + 3, ':') = '\0';
			if (OptDisplay == OPT_DEBUG && debug) printf(";; %s:%f(%f)", key, tmp_score, prob);
			tmp_score -= get_general_probability(key, "KEY");
			if (OptDisplay == OPT_DEBUG && debug) printf(":%f\n", tmp_score);
			
			if (tmp_score > max_score) max_score = tmp_score;
			continue;
		    }
		}
	    }
	    if (tmp_score > max_score) max_score = tmp_score;
	}
	score += max_score * EX_MATCH_WEIGHT + log(0.5);
	//if (!strncmp(pp_code_to_kstr(ctm_ptr->cf_ptr->pp[e_num][0]), "��", 2)) score += log(0.5);

	if (OptDisplay == OPT_DEBUG && debug) 
	    printf(";; %s:%f\n", entity_ptr->name, max_score);	   

	/* mention���Ȥ˥�������׻� */	
	max_score = FREQ0_ASSINED_SCORE;
	for (j = 0; j < entity_ptr->mentioned_num; j++) {
	    tmp_score = 0;

	    /* ��ʬ���ȤϽ��� */
	    if (entity_ptr->mention[j]->sent_num == sent_num &&
		!loc_category[(entity_ptr->mention[j]->tag_ptr)->b_ptr->num]) continue;

	    /* ���� */
	    for (k = 0; tcf_ptr->pred_b_ptr->child[k]; k++) {
		/* �ʲ��ξ���ï�פ򤹤�Υ�ʤȤ���ͥ�褹�� */
		/*     ï�򨡨�������������������             */
		/*    ̣���ˡ�<P>����������������             */
		/* ï�򨡨�����������������������             */
		/*        Ũ��<P>��PARA����������             */
		/*                        ���롣              */
	    if (tcf_ptr->pred_b_ptr->child[k]->para_top_p) {
		    child_ptr = tcf_ptr->pred_b_ptr->child[k]->child[0]; /* ��Ũ�פؤΥݥ��� */
		    for (l = 0; child_ptr->child[l]; l++) {
			/* mention����ï�פ�ؤ��Ƥ��ơ����ġ��ʤ����פ������ */
			if (child_ptr->child[l] == entity_ptr->mention[j]->tag_ptr &&
			    !strncmp(pp_code_to_kstr(ctm_ptr->cf_ptr->pp[e_num][0]),
				     entity_ptr->mention[j]->spp_string, 2)) {
			    tmp_score -= FREQ0_ASSINED_SCORE;
			    break;
			}
		    }
		}
	    }
							    
	    /* P(O:��|̤��) = P(̤��|O:��)/P(̤��) */
	    if (entity_ptr->mention[j]->flag != 'O' && /* ��ά�ξ�� */
		strcmp(entity_ptr->mention[j]->spp_string, "��")) { /* �����ǡ��ʤ��ʤ����Ϲ�θ���ʤ� */
		/* P(̤��|O:��) */
		tmp_score += get_case_interpret_probability(
		    strcmp(entity_ptr->mention[j]->spp_string, "ʸ����") ? 
		    entity_ptr->mention[j]->spp_string : "��", 
		    pp_code_to_kstr(ctm_ptr->cf_ptr->pp[e_num][0]), TRUE);	    
		/* /P(̤��) */
		tmp_score -= get_general_probability(
		    strcmp(entity_ptr->mention[j]->spp_string, "ʸ����") ? 
		    entity_ptr->mention[j]->spp_string : "��", 
		    "ɽ�س�");
	    }
	    if (OptDisplay == OPT_DEBUG && debug) 
		printf(";;   %s|%s:%f\t", 
		       pp_code_to_kstr(ctm_ptr->cf_ptr->pp[e_num][0]),
		       entity_ptr->mention[j]->spp_string, tmp_score);	   
	    
	    /* ���֥��ƥ��� */
	    /* ��ά�ʡ�flag(S,=,O,N,C)���Ȥ˰��֥��ƥ��ꤴ�Ȥ���Ի�Ȥʤ��Ψ���θ
	       ���֥��ƥ���ϡ�������ʸ�Ǥ���� B + ��ʸ����(4ʸ���ʾ��0)
	       Ʊ��ʸ��Ǥ���� C + loc_category �Ȥ�������(ex. ��-O-C3����-=-B2) */
	    get_location(loc_name, sent_num,
			 pp_code_to_kstr(ctm_ptr->cf_ptr->pp[e_num][0]),
			 entity_ptr->mention[j]);
	    tmp_score += get_general_probability("T", loc_name) * LOCATION_WEIGHT;
	    if (OptDisplay == OPT_DEBUG && debug) 
		printf("T|%s:%f\n", loc_name, tmp_score);	   

	    if (tmp_score > max_score) {
		max_score = tmp_score;
		/* ����Υ������Ȥʤä����ܶ����¸(����ؤ��н�Τ���) */
		ctm_ptr->elem_b_ptr[i] = entity_ptr->mention[j]->tag_ptr;
	    }		
	}
	score += max_score;
	if (OptDisplay == OPT_DEBUG && debug) 
	    printf(";; %s:%f\n;; score = %f\n", entity_ptr->name, max_score, score);
    }
    
    /* �б��դ����ʤ��ä��ʥ���åȤ˴ؤ��륹����*/
    /* for (e_num = 0; e_num < ctm_ptr->cf_ptr->element_num; e_num++) {
	if (!ctm_ptr->filled_element[e_num]) {
	    penalty = 
		MatchPP(ctm_ptr->cf_ptr->pp[e_num][0], "��") ? log(ga_penalty) :
		ctm_ptr->cf_ptr->adjacent[e_num] &&
		MatchPP(ctm_ptr->cf_ptr->pp[e_num][0], "��") ? log(wo_penalty) : 0;
	    if (OptDisplay == OPT_DEBUG && debug && penalty) 
		printf(";; ��%s:score = %f ", pp_code_to_kstr(ctm_ptr->cf_ptr->pp[e_num][0]), score);	   	    
	    score += penalty;
	}
	} */

/*     for (e_num = 0; e_num < ctm_ptr->cf_ptr->element_num; e_num++) { */
/* 	if (!ctm_ptr->filled_element[e_num] && */
/* 	    (MatchPP(ctm_ptr->cf_ptr->pp[e_num][0], "��") || */
/* 	     MatchPP(ctm_ptr->cf_ptr->pp[e_num][0], "��") || */
/* 	     MatchPP(ctm_ptr->cf_ptr->pp[e_num][0], "��"))) { */
/* 	    if (OptDisplay == OPT_DEBUG && debug)  */
/* 		printf(";; ��%s:score = %f ", pp_code_to_kstr(ctm_ptr->cf_ptr->pp[e_num][0]), score);	   	     */
/* 	    score += get_case_interpret_probability("������", */
/* 		pp_code_to_kstr(ctm_ptr->cf_ptr->pp[e_num][0]), TRUE); */
/* 	} */
/*     }  */

    if (OptDisplay == OPT_DEBUG && debug) 
	printf(";; %s : the score = %f\n;;\n", ctm_ptr->cf_ptr->cf_id, score);	   
    
    return score;
}

/*==================================================================*/
     int copy_ctm(CF_TAG_MGR *source_ctm, CF_TAG_MGR *target_ctm)
/*==================================================================*/
{
    int i;

    target_ctm->score = source_ctm->score;
    target_ctm->case_score = source_ctm->case_score;
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
	target_ctm->flag[i] = source_ctm->flag[i];
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
    for (j = 0; j < r_num; j++) ctm_ptr->filled_element[ctm_ptr->cf_element_num[j]] = TRUE;

    /* �ޤ������å����Ƥ��ʤ����Ǥ������� */
    if (i < tag_ptr->tcf_ptr->cf.element_num) {

	for (j = 0; tag_ptr->tcf_ptr->cf.pp[i][j] != END_M; j++) {
	    
	    /* ����ʸ��i���ܤγ����Ǥ�ʥե졼���cf.pp[i][j]�ʤ˳�����Ƥ� */
	    for (e_num = 0; e_num < ctm_ptr->cf_ptr->element_num; e_num++) {

		if (tag_ptr->tcf_ptr->cf.pp[i][j] == ctm_ptr->cf_ptr->pp[e_num][0]) {
		    /* �оݤγʤ�������ޤäƤ�������Բ� */
		    if (ctm_ptr->filled_element[e_num] == TRUE) return FALSE;

		    /* ������ǤϽ��� */
		    if (check_feature(tag_ptr->tcf_ptr->elem_b_ptr[i]->f, "�������")) {
			continue;
		    }			    

		    /* ľ���ʤǤ���������¤�ä���(����Ū) */
		    if (tag_ptr->tcf_ptr->cf.adjacent[i] && !(ctm_ptr->cf_ptr->adjacent[e_num])) {
			continue;
		    }
		    
		    /* �б��դ���̤�Ͽ */
		    ctm_ptr->elem_b_ptr[r_num] = tag_ptr->tcf_ptr->elem_b_ptr[i];
		    ctm_ptr->cf_element_num[r_num] = e_num;
		    ctm_ptr->tcf_element_num[r_num] = i;
		    ctm_ptr->entity_num[r_num] = 
			ctm_ptr->elem_b_ptr[r_num]->mention_mgr.mention->entity->num;

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
	for (j = 0; j < r_num; j++) ctm_ptr->flag[j] = 'C';
	/* ��������׻� */
	ctm_ptr->score = ctm_ptr->case_score = calc_score_of_ctm(ctm_ptr, tag_ptr->tcf_ptr);
	/* ��������̤���¸ */
	preserve_ctm(ctm_ptr, 0, CASE_CANDIDATE_MAX);
	return TRUE;
    }
    return FALSE;
}

/*==================================================================*/
int ellipsis_analysis(TAG_DATA *tag_ptr, CF_TAG_MGR *ctm_ptr, int i, int r_num)
/*==================================================================*/
{
    /* ����Ȥʤ�ʥե졼��ȳ����Ǥ��б��դ��ˤĤ��ƾ�ά���Ϥ�¹Ԥ���ؿ�
       �Ƶ�Ū�˸ƤӽФ� 
       i�ˤ�ELLIPSIS_CASE_LIST[]�Τ��������å�������������
       r_num�ˤϳʥե졼��ȴ�Ϣ�դ���줿���Ǥο�������(�ʲ��Ϥη�̴�Ϣ�դ���줿��Τ�ޤ�) */
    int j, k, e_num, exist_flag;
   
    /* ���Ǥ���ޤäƤ���ʥե졼��γʤ�����å� */
    memset(ctm_ptr->filled_element, 0, sizeof(int) * CF_ELEMENT_MAX);
    memset(ctm_ptr->filled_entity, 0, sizeof(int) * ENTITY_MAX);
    for (j = 0; j < r_num; j++) {
	ctm_ptr->filled_element[ctm_ptr->cf_element_num[j]] = TRUE;
	ctm_ptr->filled_entity[ctm_ptr->entity_num[j]] = TRUE;
    }
    /* ��ʬ���Ȥ��Բ� */
    ctm_ptr->filled_entity[tag_ptr->mention_mgr.mention->entity->num] = TRUE;
   
    /* �ޤ������å����Ƥ��ʤ���ά�����оݳʤ������� */
    if (i < ELLIPSIS_CASE_NUM) {
	/* �оݤγʤˤĤ��� */
	exist_flag = 0;
	for (e_num = 0; e_num < ctm_ptr->cf_ptr->element_num; e_num++) {
	    if (ctm_ptr->cf_ptr->pp[e_num][0] != pp_kstr_to_code(ELLIPSIS_CASE_LIST[i])) continue;
	    exist_flag = 1;

	    /* ���Ǥ���ޤäƤ������ϼ��γʤ�����å����� */
	    if (ctm_ptr->filled_element[e_num] == TRUE) {
		ellipsis_analysis(tag_ptr, ctm_ptr, i + 1, r_num);
	    }
	    else {
 		for (k = 0; k < entity_manager.num; k++) {
		    /* todo::salience_score��1�ʲ��ʤ����Ȥ��ʤ�(����Ū) */
		    if (entity_manager.entity[k].salience_score <= 1) continue;
		    /* �оݤ�ENTITY�����Ǥ��б��դ����Ƥ�������Բ� */
		    if (ctm_ptr->filled_entity[k]) continue;

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
	for (j = ctm_ptr->case_result_num; j < r_num; j++) ctm_ptr->flag[j] = 'O';
	/* ��������׻� */
	ctm_ptr->score = calc_ellipsis_score_of_ctm(ctm_ptr, tag_ptr->tcf_ptr) + ctm_ptr->case_score;
	/* ��������̤���¸ */
	preserve_ctm(ctm_ptr, CASE_CANDIDATE_MAX, ELLIPSIS_RESULT_MAX);
    }   
}

/*==================================================================*/
	    int ellipsis_analysis_main(TAG_DATA *tag_ptr)
/*==================================================================*/
{
    /* ������ܶ���оݤȤ��ƾ�ά���Ϥ�Ԥ��ؿ� */
    /* �ʥե졼�ऴ�Ȥ˥롼�פ�� */

    int i, j, k, frame_num = 0;
    CASE_FRAME **cf_array;
    CF_TAG_MGR *ctm_ptr = work_ctm + CASE_CANDIDATE_MAX + ELLIPSIS_RESULT_MAX;

    /* ���Ѥ���ʥե졼������� */
    cf_array = (CASE_FRAME **)malloc_data(sizeof(CASE_FRAME *)*tag_ptr->cf_num, 
					  "ellipsis_analysis_main");
    frame_num = set_cf_candidate(tag_ptr, cf_array);

    /* work_ctm�Υ����������� */
    for (i = 0; i < CASE_CANDIDATE_MAX + ELLIPSIS_RESULT_MAX; i++) work_ctm[i].score = INITIAL_SCORE;

    /* ����γʥե졼��ˤĤ��ƾȱ������ѳʲ��Ϥ�¹� */
    for (i = 0; i < frame_num; i++) {

	/* OR �γʥե졼������ */
	if (((*(cf_array + i))->etcflag & CF_SUM) && frame_num != 1) {
	    continue;
	}
  
	/* ctm_ptr�ν���� */
	ctm_ptr->score = INITIAL_SCORE;

	/* �ʥե졼������ */
 	ctm_ptr->cf_ptr = *(cf_array + i);

	/* �ȱ������ѳʲ���(���CASE_CANDIDATE_MAX�Ĥη�̤��ݻ�����) */
	case_analysis_for_anaphora(tag_ptr, ctm_ptr, 0, 0);	
    }
    if (work_ctm[0].score == INITIAL_SCORE) return FALSE;

    if (OptDisplay == OPT_DEBUG) {
	for (i = 0; i < CASE_CANDIDATE_MAX; i++) {
	    if (work_ctm[i].score == INITIAL_SCORE ||
		work_ctm[i].score < work_ctm[0].score - 50) break;
	    printf(";;�ʲ��ϸ���  :%2d %f %s", i + 1, work_ctm[i].score, work_ctm[i].cf_ptr->cf_id);

	    for (j = 0; j < work_ctm[i].result_num; j++) {
		printf(" %s%s:%s",
		       work_ctm[i].cf_ptr->adjacent[work_ctm[i].cf_element_num[j]] ? "*" : "-",
		       pp_code_to_kstr(work_ctm[i].cf_ptr->pp[work_ctm[i].cf_element_num[j]][0]),
		       work_ctm[i].elem_b_ptr[j]->head_ptr->Goi2);
	    }
	    for (j = 0; j < work_ctm[i].cf_ptr->element_num; j++) {
		if (!work_ctm[i].filled_element[j] && 
		    (MatchPP(work_ctm[i].cf_ptr->pp[j][0], "��") || 
		     MatchPP(work_ctm[i].cf_ptr->pp[j][0], "��") || 
		     MatchPP(work_ctm[i].cf_ptr->pp[j][0], "��")))	    
		    printf(" %s:��", pp_code_to_kstr(work_ctm[i].cf_ptr->pp[j][0]));
			   
	    }
	    printf("\n");
	}
    }

    /* �嵭���б��դ����Ф��ƾ�ά���Ϥ�¹Ԥ��� */
    for (i = 0; i < CASE_CANDIDATE_MAX; i++) {
	if (work_ctm[i].score == INITIAL_SCORE && i > 0) break;
	copy_ctm(&work_ctm[i], ctm_ptr);
	ellipsis_analysis(tag_ptr, ctm_ptr, 0, ctm_ptr->result_num);
    }

    if (OptDisplay == OPT_DEBUG) {
	for (i = CASE_CANDIDATE_MAX; i < CASE_CANDIDATE_MAX + ELLIPSIS_RESULT_MAX; i++) {
 	    if (work_ctm[i].score == INITIAL_SCORE ||
 		work_ctm[i].score < work_ctm[CASE_CANDIDATE_MAX].score - 50) break;
	    printf(";;��ά���ϸ���:%2d %f %s", i - CASE_CANDIDATE_MAX + 1, 
		   work_ctm[i].score, work_ctm[i].cf_ptr->cf_id);

	    for (j = 0; j < work_ctm[i].result_num; j++) {
		printf(" %s%s:%s",
		       work_ctm[i].cf_ptr->adjacent[work_ctm[i].cf_element_num[j]] ? "*" : "-",
		       pp_code_to_kstr(work_ctm[i].cf_ptr->pp[work_ctm[i].cf_element_num[j]][0]),
		       (entity_manager.entity + work_ctm[i].entity_num[j])->name);
	    }
	    for (j = 0; j < work_ctm[i].cf_ptr->element_num; j++) {
		if (!work_ctm[i].filled_element[j] && 
		    (MatchPP(work_ctm[i].cf_ptr->pp[j][0], "��") || 
		     MatchPP(work_ctm[i].cf_ptr->pp[j][0], "��") || 
		     MatchPP(work_ctm[i].cf_ptr->pp[j][0], "��")))	    
		    printf(" %s:��", pp_code_to_kstr(work_ctm[i].cf_ptr->pp[j][0]));
	    }
	    printf("\n", work_ctm[i].cf_ptr->cf_id);
	}
    }
   
    /* BEST�����¸ */
    if (work_ctm[CASE_CANDIDATE_MAX].score == INITIAL_SCORE) return FALSE;
    tag_ptr->ctm_ptr = 
	(CF_TAG_MGR *)malloc_data(sizeof(CF_TAG_MGR), "ellipsis_analysis_main");
    copy_ctm(&work_ctm[CASE_CANDIDATE_MAX], tag_ptr->ctm_ptr);

    /* ���ʤ���ޤ�ʤ���Τ������硢ga_penalty����Ū�˸��餹��ʸ���Ѥ��н������ */
    for (j = 0; j < tag_ptr->ctm_ptr->cf_ptr->element_num; j++) {
	if (!tag_ptr->ctm_ptr->filled_element[j] && 
	    MatchPP(tag_ptr->ctm_ptr->cf_ptr->pp[j][0], "��")) {
	    if (ga_penalty < 1) ga_penalty *= 10;
	    if (OptDisplay == OPT_DEBUG) 
		printf(";;; ga_penalty = %f\n", ga_penalty);	   
	    break;
	}	    
    }    

    free(cf_array);

    return TRUE;
}

/*==================================================================*/
    int make_new_entity(TAG_DATA *tag_ptr, MENTION_MGR *mention_mgr)
/*==================================================================*/
{    
    char *cp;
    ENTITY *entity_ptr;

    if (entity_manager.num >= ENTITY_MAX - 1) { 
	fprintf(stderr, "Entity buffer overflowed!\n");
	exit(1);
    }
    entity_ptr = entity_manager.entity + entity_manager.num;
    entity_ptr->num = entity_manager.num;
    entity_manager.num++;				
    entity_ptr->mention[0] = mention_mgr->mention;
    entity_ptr->mentioned_num = 1;
    entity_ptr->antecedent_num = 0;

    /* ��Ի�ˤʤ�䤹��(����Ū��ʸ��缭�ʤ�1) */
    entity_ptr->salience_score = 
	!(tag_ptr->inum == 0 && 
	  !check_feature(tag_ptr->f, "����̾��") &&
	  check_feature(tag_ptr->f, "�ȱ������") &&
	  !check_feature(tag_ptr->f, "NE��")) ? 0 : 
	(check_feature(tag_ptr->f, "��:̤��") ||
	 check_feature(tag_ptr->f, "ʸ��")) ? 2.0 :
	(check_feature(tag_ptr->f, "��:����") ||
	 check_feature(tag_ptr->f, "��:���")) ? 1.01 : 1.0;

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
    strcpy(mention_mgr->mention->cpp_string, "��");
    if ((cp = check_feature(tag_ptr->f, "��"))) {
	strcpy(mention_mgr->mention->spp_string, cp + strlen("��:"));
    }
    else {
	strcpy(mention_mgr->mention->spp_string, "��");
    }
    mention_mgr->mention->flag = 'S'; /* ��ʬ���� */   
} 

/*==================================================================*/
	    int make_context_structure(SENTENCE_DATA *sp)
/*==================================================================*/
{
    /* �����Ȳ��Ϸ�̤��ɤ߹��ߡ���ά���Ϥ�Ԥ�ʸ�ι�¤���ۤ��� */
    int i, j;
    char *cp;
    TAG_DATA *tag_ptr;
    MENTION_MGR *mention_mgr;
    ga_penalty = (ga_penalty == 1) ? 0.1 : 0.01;
    wo_penalty = 0.1;
   
    /* ��ά�ʳ���MENTION�ν��� */
    for (i = 0; i < sp->Tag_num; i++) { /* ����ʸ�Υ���ñ��:i���ܤΥ����ˤĤ��� */
	tag_ptr = substance_tag_ptr(sp->tag_data + i);
	
	/* ��ʬ����(MENTION)������ */       
	mention_mgr = &(tag_ptr->mention_mgr);
	mention_mgr->mention->tag_num = i;
	mention_mgr->mention->sent_num = sp->Sen_num;
	mention_mgr->mention->tag_ptr = tag_ptr;
	mention_mgr->mention->entity = NULL;
	mention_mgr->mention->salience_score = 0;
	mention_mgr->num = 1;

	/* ���Ϥ���������ɤ߹����� */
	if (OptReadFeature) {
	    /* Ʊ�ʤ����򥳡��ѥ�����Ϳ����ʤ��ΤǼ�ư���� */
	    if (check_feature(tag_ptr->f, "Ʊ��") && (cp = check_feature(tag_ptr->f, "�Զ�����"))) {
		read_one_annotation(sp, tag_ptr, cp + strlen("�Զ�����:"), TRUE);		
	    }
	    /* feature����ʲ��Ϸ�̤���� */    
	    else if (cp = check_feature(tag_ptr->f, "�ʲ��Ϸ��")) {		
		for (cp = strchr(cp + strlen("�ʲ��Ϸ��:"), ':') + 1; *cp; cp++) {
		    if (*cp == ':' || *cp == ';') {
			read_one_annotation(sp, tag_ptr, cp + 1, TRUE);
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

    /* ��ά��MENTION�ν��� */
    /* ���Ϥ���������ɤ߹����� */
    if (OptReadFeature == 1) {

	for (i = sp->Tag_num - 1; i >= 0; i--) { /* ����ʸ�Υ���ñ��:i���ܤΥ����ˤĤ��� */
	    tag_ptr = substance_tag_ptr(sp->tag_data + i);

	    /* �ֺ�����ä��ƺ����ơפʤɤ�������ϲ��Ͻ���������ؤ��� */
	    if (i > 2 && 
		check_feature((sp->tag_data + i    )->f, "�Ѹ�:ư") &&	    
		check_feature((sp->tag_data + i - 1)->f, "�Ѹ�:ư") &&	    
		check_feature((sp->tag_data + i - 2)->f, "����")) 
		tag_ptr = substance_tag_ptr(sp->tag_data + i - 1);
	    if (i > 1 && i < sp->Tag_num - 1 &&
		check_feature((sp->tag_data + i + 1)->f, "�Ѹ�:ư") &&	    
		check_feature((sp->tag_data + i    )->f, "�Ѹ�:ư") &&	    
		check_feature((sp->tag_data + i - 1)->f, "����")) 
		tag_ptr = substance_tag_ptr(sp->tag_data + i + 1);
	    
	    /* todo::�Ѹ��Τ��о�(����Ū) */
	    if (!check_feature(tag_ptr->f, "�Ѹ�") ||
		check_feature(tag_ptr->f, "�Ѹ�:Ƚ")) continue;

	    /* ���λ����Ǥγ�Entity��SALIENCE���� */
	    printf(";; SALIENCE-%d-%d", sp->Sen_num, i);
	    for (j = 0; j < entity_manager.num; j++) {
		printf(":%.3f", (entity_manager.entity + j)->salience_score);
	    }
	    printf("\n");
	
	    /* feature����ʲ��Ϸ�̤���� */
	    if (cp = check_feature(tag_ptr->f, "�ʲ��Ϸ��")) {		
		for (cp = strchr(cp + strlen("�ʲ��Ϸ��:"), ':') + 1; *cp; cp++) {
		    if (*cp == ':' || *cp == ';') {
			read_one_annotation(sp, tag_ptr, cp + 1, FALSE);
		    }
		}
	    }
	}	
	return TRUE;
    }

    /* ��ά���Ϥ�Ԥ���� */
    for (i = sp->Tag_num - 1; i >= 0; i--) { /* ����ʸ�Υ���ñ��:i���ܤΥ����ˤĤ��� */
	tag_ptr = substance_tag_ptr(sp->tag_data + i);

	/* �ֺ�����ä��ƺ����ơפʤɤ�������ϲ��Ͻ���������ؤ��� */
	if (i > 2 && 
	    check_feature((sp->tag_data + i    )->f, "�Ѹ�:ư") &&	    
	    check_feature((sp->tag_data + i - 1)->f, "�Ѹ�:ư") &&	    
	    check_feature((sp->tag_data + i - 2)->f, "����")) 
	    tag_ptr = substance_tag_ptr(sp->tag_data + i - 1);
	if (i > 1 && i < sp->Tag_num - 1 &&
	    check_feature((sp->tag_data + i + 1)->f, "�Ѹ�:ư") &&	    
	    check_feature((sp->tag_data + i    )->f, "�Ѹ�:ư") &&	    
	    check_feature((sp->tag_data + i - 1)->f, "����")) 
	    tag_ptr = substance_tag_ptr(sp->tag_data + i + 1);

	tag_ptr->tcf_ptr = NULL;
	tag_ptr->ctm_ptr = NULL;

	if (tag_ptr->cf_ptr &&
	    !check_feature(tag_ptr->f, "NE") &&
	    !check_feature(tag_ptr->f, "NE��") &&
	    !check_feature(tag_ptr->f, "������") &&
	    !check_feature(tag_ptr->f, "��������")) {

	    /* tag_ptr->tcf_ptr����� */
	    tag_ptr->tcf_ptr = 
		(TAG_CASE_FRAME *)malloc_data(sizeof(TAG_CASE_FRAME), "make_context_structure");
	    set_tag_case_frame(sp, tag_ptr);
	    	    	
	    /* todo::�Ѹ��Τ��о�(����Ū) */
	    if (!check_feature(tag_ptr->f, "�Ѹ�") ||
		check_feature(tag_ptr->f, "�Ѹ�:Ƚ") ||
		tag_ptr->tcf_ptr->cf.type != CF_PRED) continue;
    
	    /* ���֥��ƥ�������� */	    
	    mark_loc_category(sp, tag_ptr);

	    /* ���λ����Ǥγ�Entity��SALIENCE����(�Ȥꤢ����OPT_TABLE�ξ��Τ�) */
	    if (OptExpress == OPT_TABLE) {
		printf(";; SALIENCE-%d-%d", sp->Sen_num, i);
		for (j = 0; j < entity_manager.num; j++) {
		    printf(":%.3f", (entity_manager.entity + j)->salience_score);
		}
		printf("\n");
	    }    
	    
	    /* ��ά���ϥᥤ�� */
	    ellipsis_analysis_main(tag_ptr);

	    /* todo::����Ū�ˤ�ellipsis_analysis_main��cpm��Ȥ�ʤ����� */
	    free(tag_ptr->cpm_ptr);

	    /* todo::tcf����� (����Ū) */
	    for (j = 0; j < CF_ELEMENT_MAX; j++) {
		free(tag_ptr->tcf_ptr->cf.ex[j]);
		tag_ptr->tcf_ptr->cf.ex[j] = NULL;
		free(tag_ptr->tcf_ptr->cf.sm[j]);
		tag_ptr->tcf_ptr->cf.sm[j] = NULL;
		free(tag_ptr->tcf_ptr->cf.ex_list[j][0]);
		free(tag_ptr->tcf_ptr->cf.ex_list[j]);
		free(tag_ptr->tcf_ptr->cf.ex_freq[j]);
	    }
	    free(tag_ptr->tcf_ptr);

	    /* ���Ϸ�̤�ENTITY�ȴ�Ϣ�դ��� */
	    anaphora_result_to_entity(tag_ptr);
	}
    }

}

/*==================================================================*/
		   void print_entities(int sen_num)
/*==================================================================*/
{
    int i, j;
    MENTION *mention_ptr;
    ENTITY *entity_ptr;

    printf(";;SENTENCE %d\n", sen_num); 
    for (i = 0; i < entity_manager.num; i++) {
	entity_ptr = entity_manager.entity + i;

	/* if (entity_ptr->salience_score <= 0.4) continue; */
	printf(";; ENTITY %d [ %s ] %f {\n", i, entity_ptr->name, entity_ptr->salience_score);
	for (j = 0; j < entity_ptr->mentioned_num; j++) {
	    mention_ptr = entity_ptr->mention[j];
	    printf(";;\tMENTION%3d {", j);
	    printf(" SEN:%3d", mention_ptr->sent_num);
	    printf(" TAG:%3d", mention_ptr->tag_num);
	    printf(" (%3d)", mention_ptr->tag_ptr->head_ptr->Num);
	    printf(" CPP: %s", mention_ptr->cpp_string);
	    printf(" SPP: %s", mention_ptr->spp_string);
	    printf(" FLAG: %c", mention_ptr->flag);
	    printf(" WORD: %s", mention_ptr->tag_ptr->head_ptr->Goi2);
	    printf(" SS: %.3f", mention_ptr->salience_score);
	    printf(" }\n");
	}
	printf(";; }\n;;\n");
    }
}

/*==================================================================*/
		  void assign_sc(SENTENCE_DATA *sp)
/*==================================================================*/
{
    /* ����˷��뽾°���Ѹ���<�Խ�°��>����Ϳ */
    int i, j, start;
    TAG_DATA *tag_ptr, *child_ptr;
	     
    for (i = sp->Tag_num - 1; i >= 0; i--) {
	tag_ptr = sp->tag_data + i; 

	if (check_feature(tag_ptr->f, "����")) {
	    start = (tag_ptr->para_top_p) ? 1 : 0; /* ���������å����ʤ��褦�� */
	    for (j = start; tag_ptr->child[j]; j++) {
		child_ptr = substance_tag_ptr(tag_ptr->child[j]);

		/* ��٥뤬B��궯����°�����Ϳ */
		if (check_feature(child_ptr->f, "��:Ϣ��") && 
		    subordinate_level_check("B", ((BNST_DATA *)child_ptr)->f)) {
		    assign_cfeature(&(child_ptr->f), "�Խ�°��", FALSE);
		}
	    }
	    break;
	}
    }
}

/*==================================================================*/
			 void decay_entity()
/*==================================================================*/
{
    /* ENTITY�γ����ͤ򸺿ꤵ���� */
    int i;

    for (i = 0; i < entity_manager.num; i++) {
	entity_manager.entity[i].salience_score *= ENTITY_DECAY_RATE;
    }
}

/*==================================================================*/
	      void anaphora_analysis(SENTENCE_DATA *sp)
/*==================================================================*/
{
    decay_entity();
    assign_sc(sentence_data + sp->Sen_num - 1);
    make_context_structure(sentence_data + sp->Sen_num - 1);
    print_entities(sp->Sen_num);
}
