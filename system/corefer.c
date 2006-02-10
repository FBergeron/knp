/*====================================================================

			     �����Ȳ���

                                               R.SASANO 05. 9.24

    $Id$
====================================================================*/
#include "knp.h"

int COREFER_ID = 0;
DBM_FILE synonym_db;
char *SynonymFile;

/* ʸ�����Ǥ��ݻ����� */
typedef struct entity_cache {
    char            *key;
    int	            frequency;
    struct entity_cache *next;
} ENTITY_CACHE;

ENTITY_CACHE *entity_cache[TBLSIZE];

/*==================================================================*/
			 void init_entity_cache()
/*==================================================================*/
{
    memset(entity_cache, 0, sizeof(ENTITY_CACHE *)*TBLSIZE);
}

/*==================================================================*/
			void clear_entity_cache()
/*==================================================================*/
{
    int i;
    ENTITY_CACHE *ecp, *next;

    for (i = 0; i < TBLSIZE; i++) {
	ecp = entity_cache[i];
	while (ecp) {
	    free(ecp->key);
	    next = ecp->next;
	    free(ecp);
	    ecp = next;
	}
	entity_cache[i] = NULL;
    }
}

/*==================================================================*/
       void register_entity_cache(char *key)
/*==================================================================*/
{
    /* ʸ�����Ǥ���Ͽ���� */

    ENTITY_CACHE **ecpp;

    ecpp = &(entity_cache[hash(key, strlen(key))]);
    while (*ecpp && (*ecpp)->key && strcmp((*ecpp)->key, key)) {
	ecpp = &((*ecpp)->next);
    }
    if (!(*ecpp)) {
	*ecpp = (ENTITY_CACHE *)malloc_data(sizeof(ENTITY_CACHE), "register_entity_cache");
	memset(*ecpp, 0, sizeof(ENTITY_CACHE));
    }
    if (!(*ecpp)->key) {
	(*ecpp)->key = strdup(key);
	(*ecpp)->next = NULL;
    }
    (*ecpp)->frequency++;
}

/*==================================================================*/
	     int check_entity_cache(char *key)
/*==================================================================*/
{
    ENTITY_CACHE *ecp;

    ecp = entity_cache[hash(key, strlen(key))];
    if (!ecp || !ecp->key) {
	return 0;
    }
    while (ecp) {
	if (!strcmp(ecp->key, key)) {
	    return ecp->frequency;
	}
	ecp = ecp->next;
    }
    return 0;
}

/*==================================================================*/
			void init_Synonym_db()
/*==================================================================*/
{
    char *db_filename;

    if (!SynonymFile) return;

    db_filename = check_dict_filename(SynonymFile, TRUE);

    if ((synonym_db = DB_open(db_filename, O_RDONLY, 0)) == NULL) {
	if (OptDisplay == OPT_DEBUG) {
	    fprintf(Outfp, "Opening %s ... failed.\n", db_filename);
	}
	fprintf(stderr, ";; Cannot open Synonym Database <%s>.\n", db_filename);
    } 
    else {
	if (OptDisplay == OPT_DEBUG) {
	    fprintf(Outfp, "Opening %s ... done.\n", db_filename);
	}
    }
    free(db_filename);
}

/*==================================================================*/
		       void close_Synonym_db()
/*==================================================================*/
{
    if (!SynonymFile) return;
    DB_close(synonym_db);
}

/*==================================================================*/
		int get_modify_num(TAG_DATA *tag_ptr)
/*==================================================================*/
{
    /* ���������Ƥ����Ĥ�ʸ��˽�������Ƥ��뤫���֤� */
    /* ���Σ£äȤʤäƤ�����ϣ����¤˷��äƤ��뤫��Ƚ�Ǥ�Ԥ� */

    int i, ret;
    BNST_DATA *b_ptr;

    b_ptr = tag_ptr->b_ptr;

    /* OptCorefer == 4�ξ��Ͻ�������Ƥ��뤫�ɤ������Ѥ��ʤ� */
    if (OptCorefer == 4) return 0;

    /* ��°����ʸ�᤬��������Ƥ��ʤ���� */
    if (!b_ptr->child[0]) {
	return 0;
    }

    if (OptCorefer == 1) {
	/* �������缭�Ǥʤ���� */
	/* ľ����ʸ��μ缭�Ȥδ֤�̾��ʥե졼��˵����줿�ط����ʤ���� */
	/* ľ����ʸ��ϼ缭�˷��äƤ���ȹͤ�����������Ƥ��ʤ��Ȥߤʤ� */
	if (tag_ptr->head_ptr != b_ptr->head_ptr &&
	    (!(tag_ptr)->cf_ptr ||
	     check_examples((b_ptr - 1)->head_ptr->Goi2,
			    strlen((b_ptr - 1)->head_ptr->Goi2),
			    tag_ptr->cf_ptr->ex_list[0],
			    tag_ptr->cf_ptr->ex_num[0]) == -1)) {
	    return 0;
	}
    }
    else if (OptCorefer == 3) {
	/* ʸ��μ缭�Ǥʤ��ʤ齤������Ƥ��ʤ���Ƚ�Ǥ��� */
    	if (tag_ptr->head_ptr != b_ptr->head_ptr) {
	    return 0;   
	}
    }

    /* ��°����ʸ�᤬��������Ƥ����餽�ο����֤� */
    if ((b_ptr->child[0])->para_type) {
	b_ptr = b_ptr->child[0];
    }
    for (i = ret = 0; b_ptr->child[i]; i++) {
	if (!check_feature((b_ptr->child[i])->f, "��:�����") &&
	    !check_feature((b_ptr->child[i])->f, "��:Ʊ��̤��") &&
	    !check_feature((b_ptr->child[i])->f, "��:Ʊ��Ϣ��") &&
	    !check_feature((b_ptr->child[i])->f, "��:Ʊ��Ϣ��"))
	    ret++; 
    }
    return ret;
}

/*==================================================================*/
	    void assign_anaphor_feature(SENTENCE_DATA *sp)
/*==================================================================*/
{
    /* ʣ��̾��˾ȱ������Ȥ���feature����Ϳ���� */

    /* ʣ��̾�� */
    /* ��ͭɽ���ϴ���Ū�ˤ��Τޤ�(LOCATION��DATE��ʬ��) */
    /* ����ʳ����оݤθ줫��ʸ����Ƭ�ޤǤ���Ͽ */

    int i, j, k, l, tag_num, mrph_num;
    char word[WORD_LEN_MAX * 2], buf[WORD_LEN_MAX * 2], *cp;
    TAG_DATA *tag_ptr;      

    /* ʸ��ñ�̤�ʸ����������� */
    for (i = 0; i < sp->Bnst_num; i++) {
	
	if (!check_feature((sp->bnst_data + i)->f, "�θ�")) continue;

	tag_num = (sp->bnst_data + i)->tag_num;
	tag_ptr = (sp->bnst_data + i)->tag_ptr;
	
  	for (j = tag_num - 1; j >= 0; j--) {
	    
	    /* ��ͭɽ����Ǥ����� */
	    if (check_feature((tag_ptr + j)->f, "NE") ||
		check_feature((tag_ptr + j)->f, "NE��")) {
		
		/* ��ͭɽ���μ缭�ˤ���Ϳ */
		if ((cp = check_feature((tag_ptr + j)->f, "NE"))) {
		    cp += 3; /* "NE:"���ɤ����Ф� */
		    while (strncmp(cp, ":", 1)) cp++;
		    register_entity_cache(cp + 1);
		    sprintf(buf, "�ȱ������:%s", cp + 1);
		    assign_cfeature(&((tag_ptr + j)->f), buf);
		    continue;
		} 
		
		/* ��ͭɽ����Ǥ�����(DATE�ޤ���LOCATION�ξ��) */
		mrph_num = (tag_ptr + j)->mrph_num - 1;
		if (/* DATE�Ǥ���л���̾�졢̾����̾����������ڤ� */
		    check_feature(((tag_ptr + j)->mrph_ptr + mrph_num)->f, "NE:DATE") &&
		    (((tag_ptr + j)->mrph_ptr + mrph_num)->Hinshi == 6 &&
		     ((tag_ptr + j)->mrph_ptr + mrph_num)->Bunrui == 10 ||
		     ((tag_ptr + j)->mrph_ptr + mrph_num)->Hinshi == 14 &&
		     ((tag_ptr + j)->mrph_ptr + mrph_num)->Bunrui == 3) || 
		    /* LOCATION�Ǥ����̾�����ü����������ڤ� */
		    check_feature(((tag_ptr + j)->mrph_ptr + mrph_num)->f, "NE:LOCATION") &&
		    ((tag_ptr + j)->mrph_ptr + mrph_num)->Hinshi == 14 &&
		    ((tag_ptr + j)->mrph_ptr + mrph_num)->Bunrui == 4) {
		    
		    for (k = 0; !(cp = check_feature((tag_ptr + j + k)->f, "NE")); k++);
		    cp += 3; /* "NE:"���ɤ����Ф� */
		    while (strncmp(cp, ":", 1)) cp++;
		    /* cp + 1 ���оݤθ�ͭɽ��ʸ����ؤΥݥ��� */
		    for (k = 0; 
			 strncmp(cp + k + 1, ((tag_ptr + j)->mrph_ptr + mrph_num)->Goi2, 
				 strlen(((tag_ptr + j)->mrph_ptr + mrph_num)->Goi2));
			 k++);
		    strncpy(word, cp + 1, k);
		    word[k] = '\0';
		    strcat(word, ((tag_ptr + j)->mrph_ptr + mrph_num)->Goi2);
		    register_entity_cache(word);
		    sprintf(buf, "�ȱ������:%s", word);
		    assign_cfeature(&((tag_ptr + j)->f), buf);
		}
	    }
	    
	    else {
		/* ��ͭɽ����θ��缭�Ȥ��ʤ����
		   
		/* ���졢����̾�졢����Ū̾�� */
		/* ������٤˷�����ƻ�Ͻ��� */
		if ((tag_ptr + j)->head_ptr->Hinshi == 6 &&
		    (tag_ptr + j)->head_ptr->Bunrui > 7 &&
		    (tag_ptr + j)->head_ptr->Bunrui != 10 ||
		    (tag_ptr + j)->head_ptr->Hinshi == 3 &&
		    check_feature((tag_ptr + j)->f, "��:��")) {
		    continue;
		}
		
		word[0] = '\0';
		for (k = (tag_ptr + j)->head_ptr - (sp->bnst_data + i)->mrph_ptr; 
		     k >= 0; k--) {
		    
		    /* ��Ƭ���ü졢�ȱ���Ƭ���ϴޤ�ʤ� */
		    if (!strncmp(word, "\0", 1) &&
			(((tag_ptr + j)->head_ptr - k)->Hinshi == 1 ||
			 check_feature(((tag_ptr + j)->head_ptr - k)->f, "�ȱ���Ƭ��")))
			continue;
		    
		    /* �֡��פ�����ϴޤ�ʤ� */
		    if (!strcmp(((tag_ptr + j)->head_ptr - k)->Goi2, "��")) {
			if (k > 0) word[0] = '\0';
		    }
		    else {
			strcat(word, ((tag_ptr + j)->head_ptr - k)->Goi2);
		    }
		}
		if (strncmp(word, "\0", 1)) {
		    sprintf(buf, "�ȱ������:%s", word);
		    assign_cfeature(&((tag_ptr + j)->f), buf);
		    register_entity_cache(word);
		}
	    }
	}
    }
}

/*==================================================================*/
int compare_strings(char *antecedent, char *anaphor, char *ant_ne, char *ana_ne)
/*==================================================================*/
{
    /* �ȱ���������Ի�������� */
    
    int i, j;
    char word[WORD_LEN_MAX * 4];

    /* �ۤʤ����θ�ͭɽ���ξ����Բ� */ 
    if (ana_ne && ant_ne && strncmp(ana_ne, ant_ne, 7)) return 0;

    /* Ʊɽ���ξ�� */
    if (!strcmp(antecedent, anaphor)) return 1;

    /* ��ͭɽ����Ʊɽ���ξ��(ʸ���ޤ������ͭɽ���Τ���) */
    if (ant_ne && ana_ne && !strcmp(ant_ne, ana_ne)) {
	return 1;
    }

    /* ��Ի줬PERSON���Ǥ�����Ͼȱ�����䤬��Ի�������Ƭ�˴ޤޤ�Ƥ����OK */
    /* ��Ի줬LOCATION���Ǥ�����Ϥ���˾ȱ�����䤬1ʸ������û�������Τ�OK */
    /* ex. ¼���ٻ�=¼������ʬ��=��ʬ */
    if (ant_ne && strlen(ant_ne) > strlen(antecedent) && /* ��Ի줬NE���ΤǤ��� */
	!strcmp(ant_ne + strlen(ant_ne) - strlen(antecedent), antecedent) &&
	(!strncmp(ant_ne, "NE:PERSON", 7) || 
	 !strncmp(ant_ne, "NE:LOCATION", 7) && strlen(antecedent) - strlen(anaphor) == 2) &&
	!strncmp(antecedent, anaphor, strlen(anaphor))) return 1;
    
    /* Ʊ��ɽ�������ɤ߹���ʤ��ä����Ϥ����ǽ�λ */
    if (!synonym_db) return 0;

    /* ���Τޤ�Ʊ��ɽ���������Ͽ����Ƥ����� */
    word[0] = '\0';
    strcpy(word, anaphor);
    strcat(word, ":");
    strcat(word, antecedent);
    if (db_get(synonym_db, word)) {
	return 1;
    } 

    /* ���夫��Ʊ��ɽ����ʸ���������ƻĤ��ʸ����Υڥ�����Ӥ��� */
    /* �ֶ�ͻ��������-����פȡ֥ǥ�Хƥ���-����פ�ǧ���Ǥ��� */
    /* �����ܶ�ԡפȡ�����פΤ褦��Ʊ��ɽ����Ʊ��ʸ����ޤ����ǧ���Ǥ��ʤ� */
    for (i = 0; i < strlen(anaphor); i += 2) {
	if (strncmp(antecedent + i, anaphor + i, 2)) {
	    break;
	}
    }  
    for (j = 0; j < strlen(anaphor); j += 2) {
	if (strncmp(antecedent + strlen(antecedent) - j - 2, 
		    anaphor + strlen(anaphor) - j -2, 2)) {
	    break;
	} 
    }
    if (strlen(anaphor) < i + j) return 0; /* ��ʸ����� ���� �ΤȤ� */

    memset(word, 0, sizeof(char) * WORD_LEN_MAX * 4);
    strncpy(word, anaphor + i, strlen(anaphor) - i - j);
    strcat(word, ":");
    strncat(word, antecedent + i, strlen(antecedent) - i - j);
    strcat(word, "\0");

    if (db_get(synonym_db, word)) {
	return 1;
    }   
    return 0;
}

/*==================================================================*/
int search_antecedent(SENTENCE_DATA *sp, int i, char *anaphor, char *setubi, char *ne)
/*==================================================================*/
{
    /* ���Ϥ��줿�����ȡ������ȴط��ˤ��륿���������ʸ���鸡������ */
    /* setubi��Ϳ����줿����ľ�����������ޤ��õ������ */

    /* �����ȴط��ˤ���줬���Ĥ��ä����Ϸ�̤�feature����Ϳ���� */
    /* �����ȴط��ˤ���Ȥ��줿�ȱ���ʸ�������Ƭ�Υ������ֹ� */
    /* ���Ĥ���ʤ��ä�����-2���֤� */

    int j, k, l, m;
    char word[WORD_LEN_MAX], word2[WORD_LEN_MAX], buf[WORD_LEN_MAX];
    char *cp, *ant_ne, CO[WORD_LEN_MAX];
    SENTENCE_DATA *sdp;
    TAG_DATA *tag_ptr;
 
    sdp = sentence_data + sp->Sen_num - 1;
    for (j = 0; j <= sdp - sentence_data; j++) { /* �ȱ��褬��ʸ���� */
	
	for (k = j ? (sdp - j)->Tag_num - 1 : i - 1; k >= 0; k--) { /* �ȱ���Υ��� */
   
	    tag_ptr = (sdp - j)->tag_data + k;	    		
	    ant_ne = check_feature(tag_ptr->f, "NE");
		
	    /* �ȱ������Ǥ�����ʳ�����Ի����Ȥ��ʤ� */
	    if (!check_feature(tag_ptr->f, "�ȱ������")) continue;
			
	    /* setubi��Ϳ����줿��硢��³��̾������������� */
	    if (setubi && strcmp((tag_ptr->head_ptr + 1)->Goi2, setubi)) continue;

	    for (l = tag_ptr->head_ptr - (tag_ptr->b_ptr)->mrph_ptr; l >= 0; l--) {

		word[0] = word2[0] = '\0';
		for (m = l; m >= 0; m--) {
		    /* ��Ƭ���ü졢�ȱ���Ƭ���ϴޤ�ʤ� */
		    if (!strncmp(word, "\0", 1) &&
			((tag_ptr->head_ptr - m)->Hinshi == 1 ||
			 check_feature((tag_ptr->head_ptr - m)->f, "�ȱ���Ƭ��")))
			continue;
		    /* �֡��פ�����ϴޤ�ʤ� */
		    if (!strcmp((tag_ptr->head_ptr - m)->Goi2, "��")) {
			word[0] = '\0';
		    }
		    else {
			strcat(word, (tag_ptr->head_ptr - m)->Goi2); /* ��Ի���� */
		    }
		    strcat(word2, (tag_ptr->head_ptr - m)->Goi2); /* ��Ի����2 */
		}	
		if (!strncmp(word, "\0", 1)) continue;
		
		if (compare_strings(word, anaphor, ant_ne, ne) ||
		    compare_strings(word2, anaphor, ant_ne, ne) ||
		    /* �;�̾��ξ������� */
		    (check_feature((sp->tag_data + i)->f, "�;���̾��") &&
		     check_feature(tag_ptr->f, "NE:PERSON")) ||
		    /* ����̾��ξ������� */
		    (!j && (k == i - 1) && check_feature(tag_ptr->f, "���ϳ�-��") &&
		     check_feature((sp->tag_data + i)->f, "�Լ���̾��") &&
		     sm_match_check(sm2code("����"), tag_ptr->SM_code, SM_NO_EXPAND_NE)))
		    {
		    
		    /* �֡��פ������ޤ᤿���Τ�Ʊ��ɽ�������ä���� */
		    if (!compare_strings(word, anaphor, ant_ne, ne) &&
			compare_strings(word2, anaphor, ant_ne, ne)) {
			strcpy(word, word2);
		    }

		    /* Ʊ��ɽ���Ǥ���� */
		    if (j == 0) {
			sprintf(buf, "C��;��%s%s��;=;0;%d;9.99:%s(Ʊ��ʸ):%dʸ��",
				word, setubi ? setubi : "", k, 
				sp->KNPSID ? sp->KNPSID + 5 : "?", k);
		    }
		    else {
			sprintf(buf, "C��;��%s%s��;=;%d;%d;9.99:%s(%dʸ��):%dʸ��",
				word, setubi ? setubi : "", j, k, 
				(sdp - j)->KNPSID ? (sdp - j)->KNPSID + 5 : "?", j, k);
		    }
		    assign_cfeature(&((sp->tag_data + i)->f), buf);
		    assign_cfeature(&((sp->tag_data + i)->f), "������"); 

		    /* COREFER_ID����Ϳ */   
		    if ((cp = check_feature(tag_ptr->f, "COREFER_ID"))) {
			assign_cfeature(&((sp->tag_data + i)->f), cp);
		    }
		    else {
			COREFER_ID++;
			sprintf(CO, "COREFER_ID:%d", COREFER_ID);
			assign_cfeature(&((sp->tag_data + i)->f), CO);
			assign_cfeature(&(tag_ptr->f), CO);
		    }
    
		    /* ��ͭɽ����corefer�δط��ˤ������ͭɽ���Ȥߤʤ� */
		    if (OptNE) {
			if (!check_feature((sp->tag_data + i)->f, "NE") &&
			    !check_feature((sp->tag_data + i)->f, "NE��") &&
			    !check_feature((sp->tag_data + i)->f, "�;���̾��") &&
			    (cp = check_feature(tag_ptr->f, "NE")) && !setubi) {
			    cp += 3; /* "NE:"���ɤ����Ф� */
			    while (strncmp(cp, ":", 1)) cp++;
			    if (!strcmp(cp + 1, word)) {
				ne_corefer(sp, i, anaphor,
					   check_feature(tag_ptr->f, "NE"));
			    }
			} 
		    }
		    return 1;
		}
	    }
	}	    
    }
    return 0;
}

/*==================================================================*/
int person_post(SENTENCE_DATA *sp, TAG_DATA *tag_ptr, char *cp, int j)
/*==================================================================*/
{
    /* PERSON + �� ��"="��������Ϳ */

    int i, flag;
    char buf[WORD_LEN_MAX], CO[WORD_LEN_MAX];
    MRPH_DATA *mrph_ptr;
    TAG_DATA *tag_ptr_cp;
    
    tag_ptr_cp = tag_ptr;
    mrph_ptr = tag_ptr->mrph_ptr;
    /* ���������ޤ�NE��Ǥ�����Τ��оݤȤ��� */
    if (!check_feature((mrph_ptr - 1)->f, "NE") &&
	!(check_feature((mrph_ptr - 2)->f, "NE") &&
	  (mrph_ptr - 1)->Hinshi == 1 && 
	  (mrph_ptr - 1)->Bunrui == 5)) /* ľ��������Ǥ��� */
	return 0;

    flag = 0;
    for (i = 0;; i++) {
	if (check_feature((mrph_ptr + i)->f, "��̾����")) {
	    flag = 1;
	    continue;
	}
	else if (check_feature((mrph_ptr + i)->f, "NE") ||
		 check_feature((mrph_ptr + i)->f, "��ͭ����")) {
	    /* ����Ū�ˤϡ��֥å��塦����ꥫ������ */
	    /* ������̱�޴���Ĺ�ʤɤ����ꤷ�Ƥ��� */
	    /* ���󥰥�å���������Ĺ */
	    continue;
	}
	else break;
    }
    if (!flag) return 0;
	
    /* ʣ���Υ����ˤޤ����äƤ�����ϼ��Υ����˿ʤ� */
    while (i > tag_ptr->mrph_num) {
	i -= tag_ptr->mrph_num;
	tag_ptr++;
    }
    
    sprintf(buf, "C��;��%s��;=;0;%d;9.99:%s(Ʊ��ʸ):%dʸ��",
	    cp, j, sp->KNPSID ? sp->KNPSID + 5 : "?", 
	    tag_ptr - sp->tag_data);
    assign_cfeature(&(tag_ptr->f), buf);
    assign_cfeature(&(tag_ptr->f), "������(��)");
    
    /* COREFER_ID����Ϳ */   
    if (cp = check_feature(tag_ptr->f, "COREFER_ID")) {
	assign_cfeature(&((tag_ptr_cp - 1)->f), cp);
    }
    else if (cp = check_feature((tag_ptr_cp - 1)->f, "COREFER_ID")) {
	assign_cfeature(&(tag_ptr->f), cp);
    }
    else {
	COREFER_ID++;
	sprintf(CO, "COREFER_ID:%d", COREFER_ID);
	assign_cfeature(&(tag_ptr->f), CO);
			assign_cfeature(&((sp->tag_data + i)->f), CO);
    }
    
    return 1;
}

/*==================================================================*/
	       void corefer_analysis(SENTENCE_DATA *sp)
/*==================================================================*/
{
    int i, person;
    char *anaphor, *cp, *ne;
    MRPH_DATA *mrph_ptr;

    for (i = 0; i < sp->Tag_num; i++) { /* ����ʸ�Υ���ñ��:i���ܤΥ����ˤĤ��� */

	/* �����Ȳ��Ϥ�Ԥ���� */
	/* �ȱ������Ǥ��ꡢ��ͭɽ����θ졢�ޤ��� */
	/* Ϣ�λ���ֻؼ���ʳ��˽�������Ƥ��ʤ��� */
	if ((anaphor = check_feature((sp->tag_data + i)->f, "�ȱ������")) &&
	    (check_feature((sp->tag_data + i)->f, "NE") ||  
	     check_feature((sp->tag_data + i)->f, "NE��") || /* DATA��LOCATION�ʤɰ��� */
	     !get_modify_num(sp->tag_data + i) || /* ��������Ƥ��ʤ� */
	     (((sp->tag_data + i)->mrph_ptr - 1)->Hinshi == 1 && 
	      ((sp->tag_data + i)->mrph_ptr - 1)->Bunrui == 2) || /* ľ���������Ǥ��� */
	     check_feature(((sp->tag_data + i)->b_ptr->child[0])->f, "Ϣ�λ���ֻؼ���") ||
	     check_feature(((sp->tag_data + i)->b_ptr->child[0])->f, "�ȱ���Ƭ��"))) {
	    
	    /* �ؼ���ξ�� */
	    if (check_feature((sp->tag_data + i)->f, "�ؼ���")) {
		continue; /* �����ǤϽ����򤷤ʤ� */
	    }
	    
	    mrph_ptr = (sp->tag_data + i)->head_ptr + 1;
	    /* ̾�������������դ�����ͭɽ���ʳ��θ�Ϥޤ���������ޤ᤿��Τ�Ĵ�٤� */
	    if (!((ne = check_feature((sp->tag_data + i)->f, "NE"))) &&
		mrph_ptr->Hinshi == 14 && mrph_ptr->Bunrui < 5)
		search_antecedent(sp, i, anaphor+11, mrph_ptr->Goi2, NULL);
    
	    /* ���̤ξ�� */
	    search_antecedent(sp, i, anaphor+11, NULL, ne);
	}
	/* PERSON + ��̾���� �ν��� */
	if ((cp = check_feature((sp->tag_data + i)->f, "NE:PERSON"))) {
	    person_post(sp, sp->tag_data + i + 1, cp + 10, i);
	}
    }
}

/*==================================================================*/
int search_antecedent_after_br(SENTENCE_DATA *sp, TAG_DATA *tag_ptr1, int i)
/*==================================================================*/
{
    int j, k, l, tag, sent;
    char *cp, buf[WORD_LEN_MAX], CO[WORD_LEN_MAX];
    SENTENCE_DATA *sdp;
    TAG_DATA *tag_ptr, *tag_ptr2;
 
    sdp = sentence_data + sp->Sen_num - 1;
    for (j = 0; j <= sdp - sentence_data; j++) { /* �ȱ��褬��ʸ���� */
	
	for (k = j ? (sdp - j)->Tag_num - 1 : i - 1; k >= 0; k--) { /* �ȱ���Υ��� */
   
	    tag_ptr = (sdp - j)->tag_data + k;	    		
		
	    /* �ȱ������Ǥ�����ʳ�����Ի����Ȥ��ʤ� */
	    if (!check_feature(tag_ptr->f, "�ȱ������")) continue;

	    /* �ȱ�������Ʊ��ɽ���Τ�Τ�����Ի����Ȥ��ʤ� */
	    if (strcmp((sp->tag_data + i)->head_ptr->Goi2, tag_ptr->head_ptr->Goi2))
		continue;
	    
	    /* �ʲ��Ϸ�̤����� */
	    sprintf(buf, "�ʲ��Ϸ��:%s:̾1", tag_ptr->head_ptr->Goi2);
	    cp = check_feature(tag_ptr->f, buf);
	    if (!cp) continue;
	    
	    /* <�ʲ��Ϸ��:���:̾1:��/O/���󥱡���/0/1/?> */
	    for (l = 0; l < 3; l++) {
		while (strncmp(cp, "/", 1)) cp++;
		    cp++;
	    }
	    if (!sscanf(cp, "%d/%d/", &tag, &sent)) continue;
  
	    /* �ؼ���Υ����ؤΥݥ��� */
	    tag_ptr2 = (sdp - j - sent)->tag_data + tag;

	    /* �ؼ���Υ����������ȴط��ˤ��뤫��Ƚ�� */
	    if (check_feature(tag_ptr1->f, "COREFER_ID") &&
		check_feature(tag_ptr2->f, "COREFER_ID") &&
		!strcmp(check_feature(tag_ptr1->f, "COREFER_ID"),
			check_feature(tag_ptr2->f, "COREFER_ID"))) {

		cp = check_feature(tag_ptr->f, "�ȱ������");
		cp += 11;
		
		if (j == 0) {
		    sprintf(buf, "C��;��%s��;=;0;%d;9.99:%s(Ʊ��ʸ):%dʸ��",
			    cp, k, sp->KNPSID ? sp->KNPSID + 5 : "?", k);
		}
		else {
		    sprintf(buf, "C��;��%s��;=;%d;%d;9.99:%s(%dʸ��):%dʸ��",
			    cp, j, k, 
			    (sdp - j)->KNPSID ? (sdp - j)->KNPSID + 5 : "?", j, k);
		}
		assign_cfeature(&((sp->tag_data + i)->f), buf);
		assign_cfeature(&((sp->tag_data + i)->f), "������"); 
		
		/* COREFER_ID����Ϳ */   
		if ((cp = check_feature(tag_ptr->f, "COREFER_ID"))) {
		    assign_cfeature(&((sp->tag_data + i)->f), cp);
		}
		else if ((cp = check_feature((sp->tag_data + i)->f, "COREFER_ID"))) {
		    assign_cfeature(&(tag_ptr->f), cp);
		}
		else {
		    COREFER_ID++;
		    sprintf(CO, "COREFER_ID:%d", COREFER_ID);
		    assign_cfeature(&((sp->tag_data + i)->f), CO);
		    assign_cfeature(&(tag_ptr->f), CO);
		}
		return 1;
	    }    
	}
    }
}

/*==================================================================*/
	  void corefer_analysis_after_br(SENTENCE_DATA *sp)
/*==================================================================*/
{
    int i, j, tag, sent;
    char *cp, buf[WORD_LEN_MAX];
    MRPH_DATA *mrph_ptr;
    TAG_DATA *tag_ptr;

    for (i = 0; i < sp->Tag_num; i++) {

	/* �ȱ������Ǥ�����ʳ�����Ի����Ȥ��ʤ� */
	if (!check_feature((sp->tag_data + i)->f, "�ȱ������")) continue;
	/* ̾��˸���(���������оݳ�) */
	if ((sp->tag_data + i)->head_ptr->Hinshi != 6) continue;

	/* �����ȥ������ʤ����ʲ��Ϸ�̤����� */
	sprintf(buf, "�ʲ��Ϸ��:%s:̾1", (sp->tag_data + i)->head_ptr->Goi2);
	if (!check_feature((sp->tag_data + i)->f, "COREFER_ID") &&
	    (cp = check_feature((sp->tag_data + i)->f, buf))) {

	    /* <�ʲ��Ϸ��:���:̾1:��/O/���󥱡���/0/1/?> */
	    for (j = 0; j < 3; j++) {
		while (strncmp(cp, "/", 1)) cp++;
		cp++;
	    }
	    if (sscanf(cp, "%d/%d/", &tag, &sent)) {
		/* �ؼ���Υ����ؤΥݥ��� */
		tag_ptr = ((sentence_data + sp->Sen_num - 1 - sent)->tag_data + tag);
		search_antecedent_after_br(sp, tag_ptr, i);
	    }
	}	
    }	   
}
/*====================================================================
                               END
====================================================================*/
