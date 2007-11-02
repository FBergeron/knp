/*====================================================================

			       �ȱ�����

                                         Ryohei Sasano 2007. 8. 27

    $Id$
====================================================================*/

#include "knp.h"

#define CASE_CANDIDATE_MAX 20 /* �ȱ������ѳʲ��Ϸ�̤��ݻ������ */
#define ELLIPSIS_RESULT_MAX 1 /* ��ά���Ϸ�̤��ݻ������ */
#define INITIAL_SCORE -10000

/* ���Ϸ�̤��ݻ����뤿���ENTITY_CASE_MGR
   ��Ƭ��CASE_CANDIDATE_MAX�Ĥ˾ȱ������ѳʲ��Ϥη�̤ξ�̤��ݻ���
   ����ELLIPSIS_RESULT_MAX�ĤˤϾ�ά���Ϸ�̤Υ٥��Ȳ���ݻ���
   �Ǹ��1�Ĥϸ��ߤβ��Ϸ�̤��ݻ��˻��Ѥ��� */
CF_TAG_MGR work_ctm[CASE_CANDIDATE_MAX + ELLIPSIS_RESULT_MAX + 1];

/*==================================================================*/
int read_one_annotation(SENTENCE_DATA *sp, TAG_DATA *tag_ptr, char *token, int co_flag)
/*==================================================================*/
{
    /* ���Ϸ�̤���MENTION��ENTITY��������� */
    /* co_flag���������"="�Τߤ�������ʤ�����"="�ʳ������ */

    char flag, rel[SMALL_DATA_LEN], *cp;
    int tag_num, sent_num;
    MENTION_MGR *mention_mgr = &(tag_ptr->mention_mgr);
    MENTION *mention_ptr = NULL;

    if (!sscanf(token, "%[^/]/%c/%*[^/]/%d/%d/", rel, &flag, &tag_num, &sent_num)) 
	return FALSE;

    if (co_flag && !strcmp(rel, "=")) {
	mention_ptr = mention_mgr->mention;
	mention_ptr->entity = ((sp - sent_num)->tag_data + tag_num)->mention_mgr.mention->entity;
	mention_ptr->pp_code = 0;
	mention_ptr->flag = '=';

	/* entity��name��NE�Ǥʤ���tag_ptr��NE�ʤ��name���� */
	if (!strchr(mention_ptr->entity->name, ':') &&
	    (cp = check_feature(tag_ptr->f, "NE"))) {
	    strcpy(mention_ptr->entity->name, cp + strlen("NE:"));
	}
    }

    else if (!co_flag &&
	     (flag == 'N' || flag == 'C' || flag == 'O' || flag == 'D') &&    
	     (!strcmp(rel, "��") || !strcmp(rel, "��") || !strcmp(rel, "��") || !strcmp(rel, "��"))) {
	mention_ptr = mention_mgr->mention + mention_mgr->num;
	mention_ptr->entity = ((sp - sent_num)->tag_data + tag_num)->mention_mgr.mention->entity;
	
	mention_ptr->tag_num = mention_mgr->mention->tag_num;
	mention_ptr->sent_num = mention_mgr->mention->sent_num;
	mention_ptr->flag = flag;
	mention_ptr->pp_code = pp_kstr_to_code(rel);
	mention_mgr->num++;
    }

    if (!mention_ptr) return FALSE;
    mention_ptr->entity->mention[mention_ptr->entity->mentioned_num] = mention_ptr;
    mention_ptr->entity->mentioned_num++;
    if (flag == 'O' || !strcmp(rel, "=")) mention_ptr->entity->antecedent_num++;
    
    return TRUE;
}

/*==================================================================*/
	   int anaphora_result_to_entity(TAG_DATA *tag_ptr)
/*==================================================================*/
{
    /* �ȱ����Ϸ��ENTITY�˴�Ϣ�դ��� */

    int i;
    MENTION_MGR *mention_mgr = &(tag_ptr->mention_mgr);
    MENTION *mention_ptr = NULL;
    CF_TAG_MGR *ctm_ptr = tag_ptr->ctm_ptr; 

    /* �ʡ���ά���Ϸ�̤��ʤ����Ͻ�λ */
    if (!ctm_ptr) return FALSE;
    
    for (i = 0; i < ctm_ptr->result_num; i++) {
	mention_ptr = mention_mgr->mention + mention_mgr->num;
	mention_ptr->entity = ctm_ptr->elem_b_ptr[i]->mention_mgr.mention->entity;
	mention_ptr->tag_num = mention_mgr->mention->tag_num;
	mention_ptr->sent_num = mention_mgr->mention->sent_num;
	//mention_ptr->flag = ctm_ptr->flag[i]; /* todo */
	mention_ptr->flag = 'O'; /* todo */
	mention_ptr->pp_code = 2; /* todo */
	mention_mgr->num++;

	mention_ptr->entity->mention[mention_ptr->entity->mentioned_num] = mention_ptr;
	mention_ptr->entity->mentioned_num++;
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
    for (i = 0; i < CF_ELEMENT_MAX; i++) {
	tcf_ptr->elem_b_ptr[i] = cpm_ptr->elem_b_ptr[i];
    }

    free(cpm_ptr);
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
     int copy_ctm(CF_TAG_MGR *source_ctm, CF_TAG_MGR *target_ctm)
/*==================================================================*/
{
    int i;

    target_ctm->score = source_ctm->score;
    target_ctm->cf_ptr = source_ctm->cf_ptr;
    target_ctm->result_num = source_ctm->result_num;
    for (i = 0; i < CF_ELEMENT_MAX; i++) {
	target_ctm->filled_element[i] = source_ctm->filled_element[i];
	target_ctm->non_match_element[i] = source_ctm->non_match_element[i];
	target_ctm->cf_element_num[i] = source_ctm->cf_element_num[i];
	target_ctm->tcf_element_num[i] = source_ctm->tcf_element_num[i];
	target_ctm->elem_b_ptr[i] = source_ctm->elem_b_ptr[i];
    }
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

		    /* ľ���ʤǤ���������¤�ä���(̤����) */
		    if (0) return FALSE;
		    
		    /* �б��դ���̤�Ͽ */
		    ctm_ptr->elem_b_ptr[r_num] = tag_ptr->tcf_ptr->elem_b_ptr[i];
		    ctm_ptr->cf_element_num[r_num] = e_num;
		    ctm_ptr->tcf_element_num[r_num] = i;

		    /* i+1���ܤ����ǤΥ����å��� */
		    case_analysis_for_anaphora(tag_ptr, ctm_ptr, i + 1, r_num + 1);
		}
	    }    
	}

	for (j = 0; tag_ptr->tcf_ptr->cf.pp[i][j] != END_M; j++) {
	    /* �����򡢥˰ʳ��γʤ�������ϳ�����Ƥʤ�����ͤ��� */
	    if (!MatchPP(tag_ptr->tcf_ptr->cf.pp[i][j], "��") &&
		!MatchPP(tag_ptr->tcf_ptr->cf.pp[i][j], "��") &&
		!MatchPP(tag_ptr->tcf_ptr->cf.pp[i][j], "��")) {		
		/* ����ʸ��i���ܤ����Ǥ��б��դ����ʤ��ä����Ȥ�Ͽ */
		ctm_ptr->non_match_element[i - r_num] = i; 
		case_analysis_for_anaphora(tag_ptr, ctm_ptr, i + 1, r_num);
		break;
	    }
	}
    }

    /* ���٤ƤΥ����å�����λ������� */
    /* �����ʳ���r_num�Ĥ��б��դ����Ƥ��� */
    else {
	ctm_ptr->score = 0;
	ctm_ptr->result_num = r_num;

	/* ��������׻� */
	/* �б��դ���줿���Ǥ˴ؤ��륹���� */
	for (j = 0; j < r_num; j++) {
	    e_num = ctm_ptr->cf_element_num[j];
	    
	    /* if (OptDisplay == OPT_DEBUG) 
		printf(";; %s-%s ", ctm_ptr->elem_b_ptr[j]->head_ptr->Goi2, 
		pp_code_to_kstr(ctm_ptr->cf_ptr->pp[e_num][0])); */
	    
	    ctm_ptr->score += 
		get_ex_probability_with_para(ctm_ptr->tcf_element_num[j], &(tag_ptr->tcf_ptr->cf), 
					     e_num, ctm_ptr->cf_ptr) +
		get_case_interpret_probability(ctm_ptr->tcf_element_num[j], &(tag_ptr->tcf_ptr->cf), 
					       e_num, ctm_ptr->cf_ptr);
	}
	/* ����ʸ�γ����ǤΤ����б��դ����ʤ��ä����Ǥ˴ؤ��륹���� */
	for (j = 0; j < tag_ptr->tcf_ptr->cf.element_num - r_num; j++) {
	    ctm_ptr->score += 
		get_case_interpret_probability(ctm_ptr->non_match_element[j], &(tag_ptr->tcf_ptr->cf), 
					       NIL_ASSIGNED, ctm_ptr->cf_ptr);
	}		    
	/* �ʥե졼��γʤ���ޤäƤ��뤫�ɤ����˴ؤ��륹���� */
	for (e_num = 0; e_num < ctm_ptr->cf_ptr->element_num; e_num++) {
	    ctm_ptr->score += 
		get_case_probability(e_num, ctm_ptr->cf_ptr, ctm_ptr->filled_element[e_num]);
	}
	/* if (OptDisplay == OPT_DEBUG) printf(";; %f\n", ctm_ptr->score); */

	/* ��������̤���¸ */
	for (j = 0; j < CASE_CANDIDATE_MAX; j++) {

	    /* work_ctm�˷�̤���¸ */
	    if (ctm_ptr->score > work_ctm[j].score) {
		for (k = CASE_CANDIDATE_MAX - 1; k > j; k--) {
		    if (work_ctm[k - 1].score > INITIAL_SCORE) {
			copy_ctm(&work_ctm[k - 1], &work_ctm[k]);
		    }
		}
		copy_ctm(ctm_ptr, &work_ctm[j]);
		break;
	    }
	}
    }
}

/*==================================================================*/
int ellipsis_analysis(TAG_DATA *tag_ptr, CF_TAG_MGR *ctm_ptr)
/*==================================================================*/
{
    int j, k;
    
    /* ��������̤���¸ */
    for (j = CASE_CANDIDATE_MAX; j < CASE_CANDIDATE_MAX + ELLIPSIS_RESULT_MAX; j++) {
	
	/* work_ctm�˷�̤���¸ */
	if (ctm_ptr->score > work_ctm[j].score) {
	    for (k = CASE_CANDIDATE_MAX + ELLIPSIS_RESULT_MAX - 1; k > j; k--) {
		if (work_ctm[k - 1].score > INITIAL_SCORE) {
		    copy_ctm(&work_ctm[k - 1], &work_ctm[k]);
		}
	    }
	    copy_ctm(ctm_ptr, &work_ctm[j]);
	    break;
	}
    }

    return 1;
}

/*==================================================================*/
	    int ellipsis_analysis_main(TAG_DATA *tag_ptr)
/*==================================================================*/
{
    /* ������ܶ���оݤȤ��ƾ�ά���Ϥ�Ԥ��ؿ� */
    /* �ʥե졼�ऴ�Ȥ˥롼�פ�� */

    int i, j, frame_num = 0;
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
	
	/* �Ѹ��Τ��о�(����Ū)*/
	if (tag_ptr->tcf_ptr->cf.type != CF_PRED) continue;

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
	    if (work_ctm[i].score == INITIAL_SCORE) break;
	    printf(";;%2d %f %s", i, work_ctm[i].score, work_ctm[i].cf_ptr->cf_id);

	    for (j = 0; j < work_ctm[i].result_num; j++) {
		printf(" %s:%s",
		       pp_code_to_kstr(work_ctm[i].cf_ptr->pp[j][0]),
		       work_ctm[i].elem_b_ptr[j]->head_ptr->Goi2);
	    }
	    printf("\n", work_ctm[i].cf_ptr->cf_id);
	}
    }

    /* �嵭���б��դ����Ф��ƾ�ά���Ϥ�¹Ԥ��� */
    for (i = 0; i < CASE_CANDIDATE_MAX; i++) {
	copy_ctm(&work_ctm[i], ctm_ptr);
	ellipsis_analysis(tag_ptr, ctm_ptr);
    }
    
    /* BEST�����¸ */
    tag_ptr->ctm_ptr = 
	(CF_TAG_MGR *)malloc_data(sizeof(CF_TAG_MGR), "ellipsis_analysis_main");
    copy_ctm(&work_ctm[CASE_CANDIDATE_MAX], tag_ptr->ctm_ptr);

    free(cf_array);

    return TRUE;
}

/*==================================================================*/
	    int make_context_structure(SENTENCE_DATA *sp)
/*==================================================================*/
{
    int i, j;
    char *cp;
    TAG_DATA *tag_ptr;
    MENTION_MGR *mention_mgr;
    ENTITY *entity_ptr;
    
    /* ENTITY������ */
    for (i = 0; i < sp->Tag_num; i++) { /* ����ʸ�Υ���ñ��:i���ܤΥ����ˤĤ��� */

	tag_ptr = sp->tag_data + i; 
	
	/* ��ʬ����(MENTION)������ */
	mention_mgr = &(tag_ptr->mention_mgr);
	mention_mgr->mention->tag_num = i;
	mention_mgr->mention->sent_num = sp->Sen_num;
	mention_mgr->mention->entity = NULL;
	mention_mgr->num = 1;

	/* ���Ϥ���������ɤ߹����� */
	if (OptReadFeature) {

	    /* feature����ʲ��Ϸ�̤���� */
	    if (cp = check_feature(tag_ptr->f, "�ʲ��Ϸ��")) {		
		cp += strlen("�ʲ��Ϸ��:");
		cp = strchr(cp, ':') + 1;	
		for (; *cp; cp++) {
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
	    if (entity_manager.num >= ENTITY_MAX - 1) { 
		fprintf(stderr, "Entity buffer overflowed!\n");
		exit(1);
	    }
	    entity_ptr = entity_manager.entity + entity_manager.num;
	    entity_manager.num++;				
	    entity_ptr->num = entity_manager.num;
	    entity_ptr->mention[0] = mention_mgr->mention;
	    entity_ptr->mentioned_num = 1;
	    entity_ptr->antecedent_num = 0;
	    /* name������(NE > �ȱ������ > �缭�����Ǥθ���) */
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
	    mention_mgr->mention->pp_code = 0;
	    mention_mgr->mention->flag = 'S'; /* ��ʬ���� */
	}
    }

    /* ���Ϥ���������ɤ߹����� */
    if (OptReadFeature) {
	for (i = 0; i < sp->Tag_num; i++) { /* ����ʸ�Υ���ñ��:i���ܤΥ����ˤĤ��� */

	    tag_ptr = sp->tag_data + i; 
	
	    /* feature����ʲ��Ϸ�̤���� */
	    if (cp = check_feature(tag_ptr->f, "�ʲ��Ϸ��")) {		
		cp += strlen("�ʲ��Ϸ��:");
		cp = strchr(cp, ':') + 1;	
		for (; *cp; cp++) {
		    if (*cp == ':' || *cp == ';') {
			read_one_annotation(sp, tag_ptr, cp + 1, FALSE);
		    }
		}
	    }
	}	
	return TRUE;
    }
    
    /* ��ά���� */
    for (i = sp->Tag_num - 1; i >= 0; i--) { /* ����ʸ�Υ���ñ��:i���ܤΥ����ˤĤ��� */

	tag_ptr = sp->tag_data + i; 
	tag_ptr->tcf_ptr = NULL;
	tag_ptr->ctm_ptr = NULL;
	
	if (tag_ptr->cf_ptr &&
	    !check_feature(tag_ptr->f, "��ά���Ϥʤ�") &&
	    !check_feature(tag_ptr->f, "NE") &&
	    !check_feature(tag_ptr->f, "NE��") &&
	    !check_feature(tag_ptr->f, "������") &&
	    !check_feature(tag_ptr->f, "��������")) {

	    /* tag_ptr->tcf_ptr����� */
	    tag_ptr->tcf_ptr = 
		(TAG_CASE_FRAME *)malloc_data(sizeof(TAG_CASE_FRAME), "make_context_structure");
	    set_tag_case_frame(sp, tag_ptr);

	    /* ��ά���ϥᥤ�� */
	    ellipsis_analysis_main(tag_ptr);

	    /* tcf����� (����Ū) */
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
	}
    }

    /* ���Ϸ�̤�ENTITY�ȴ�Ϣ�դ��� */
    for (i = 0; i < sp->Tag_num; i++) {
	anaphora_result_to_entity(sp->tag_data + i);
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

	/* if (!entity_ptr->antecedent_num) continue; */
	if (entity_ptr->mentioned_num == 1) continue;
	
	printf(";; ENTITY %d [ %s ] {\n", i, entity_ptr->name);
	for (j = 0; j < entity_ptr->mentioned_num; j++) {
	    mention_ptr = entity_ptr->mention[j];
	    printf(";;\tMENTION%3d {", j);
	    printf(" SEN:%3d", mention_ptr->sent_num);
	    printf(" TAG:%3d", mention_ptr->tag_num);
	    printf(" PP: %s", pp_code_to_kstr(mention_ptr->pp_code));
	    printf(" FLAG: %c", mention_ptr->flag);
	    printf(" WORD: %s", ((sentence_data + mention_ptr->sent_num - 1)->tag_data + mention_ptr->tag_num)->head_ptr->Goi2);
	    printf(" }\n");
	}
	printf(";; }\n;;\n");
    }
}

/*==================================================================*/
	      void anaphora_analysis(SENTENCE_DATA *sp)
/*==================================================================*/
{
    make_context_structure(sentence_data + sp->Sen_num - 1);
    /* if (OptDisplay == OPT_DEBUG) */ print_entities(sp->Sen_num);
}
