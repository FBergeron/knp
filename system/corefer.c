/*====================================================================

			     �����Ȳ���

                                               R.SASANO 05. 9.24

    $Id$
====================================================================*/
#include "knp.h"

DBM_FILE synonym_db;

char *SynonymFile;

/* ʸ�����Ǥ��ݻ����� */
typedef struct entity_cache {
    char            *key;
    int	            frequency;
    struct entity_cache *next;
} ENTITY_CACHE;

ENTITY_CACHE entity_cache[TBLSIZE];

/*==================================================================*/
			 void init_entity_cache()
/*==================================================================*/
{
    memset(entity_cache, 0, sizeof(ENTITY_CACHE)*TBLSIZE);
}

/*==================================================================*/
			void clear_entity_cache()
/*==================================================================*/
{
    int i;
    ENTITY_CACHE *ecp, *next;

    for (i = 0; i < TBLSIZE; i++) {
	ecp = entity_cache[i].next;
	while (ecp) {
	    free(ecp->key);
	    next = ecp->next;
	    free(ecp);
	    ecp = next;
	}
    }
    init_entity_cache();
}

/*==================================================================*/
       void register_entity_cache(char *key)
/*==================================================================*/
{
    /* ʸ�����Ǥ���Ͽ���� */

    ENTITY_CACHE *ecp;

    ecp = &(entity_cache[hash(key, strlen(key))]);
    while (ecp && ecp->key && strcmp(ecp->key, key)) {
	ecp = ecp->next;
    }
    if (!ecp) {
	ecp = (ENTITY_CACHE *)malloc_data(sizeof(ENTITY_CACHE), "register_entity_cache");
	memset(entity_cache, 0, sizeof(ENTITY_CACHE));
    }
    if (!ecp->key) {
	ecp->key = strdup(key);
	ecp->next = NULL;
    }
    ecp->frequency++;
}

/*==================================================================*/
	     int check_entity_cache(char *key)
/*==================================================================*/
{
    ENTITY_CACHE *ecp;

    ecp = &(entity_cache[hash(key, strlen(key))]);
    if (!ecp->key) {
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
    /* ���Σ£á������ȤʤäƤ�����ϣ����¤˷��äƤ��뤫��Ƚ�Ǥ�Ԥ� */

    int i;
    BNST_DATA *b_ptr;

    b_ptr = tag_ptr->b_ptr;

    /* ��°����ʸ�᤬��������Ƥ��ʤ���� */
    if (!b_ptr->child[0]) {
	return 0;
    }

    /* ʸ�����Ƭ�Υ����Ǽ缭�Ǥʤ���� */
    /* ľ����ʸ��μ缭�Ȥδ֤�̾��ʥե졼��˵����줿�ط����ʤ���� */
    /* ľ����ʸ��ϼ缭�˷��äƤ���ȹͤ�����������Ƥ��ʤ��Ȥߤʤ� */
    if (tag_ptr == b_ptr->tag_ptr && 
	b_ptr->tag_num > 1 &&
	!((tag_ptr)->cf_ptr &&
	check_examples(((tag_ptr - 1)->mrph_ptr)->Goi2,
		       strlen(((tag_ptr - 1)->mrph_ptr)->Goi2),
		       (tag_ptr->cf_ptr)->ex_list[0],
		       (tag_ptr->cf_ptr)->ex_num[0]) >= 0)) {
	return 0;
    }

    /* �嵭�ʳ� */
    if ((b_ptr->child[0])->para_type) {
	b_ptr = b_ptr->child[0];
    }
    for (i = 0; b_ptr->child[i]; i++);
    return i;
}

/*==================================================================*/
	    void assign_anaphor_feature(SENTENCE_DATA *sp)
/*==================================================================*/
{
    /* ʣ��̾��˾ȱ������Ȥ���feature����Ϳ���� */

    /* ʣ��̾�� */   
    /* 	��ͭɽ��(LOCATION��DATE��ʬ��) */
    /* 	�����θ줬����̾��Ǥ�����ڤ� */
    /* 	�����θ�Ȥ������θ줬̾��ʥե졼��ˤ����ȹ礻�ξ����ڤ� */

    int i, j, k, l, tag_num, mrph_num;
    char word[WORD_LEN_MAX * 2], buf[WORD_LEN_MAX * 2], *cp;
    TAG_DATA *tag_ptr;
    
    

    /* ʸ��ñ�̤�ʸ����������� */
    for (i = 0; i < sp->Bnst_num; i++) {
	
	if (!check_feature((sp->bnst_data + i)->f, "�θ�")) continue;

	tag_num = (sp->bnst_data + i)->tag_num;
	tag_ptr = (sp->bnst_data + i)->tag_ptr;
	
	/* �ޤ���ʸ����������ʣ��̾��������Υ����˾ȱ���������Ϳ���Ƥ��� */
	/* ex. �ֶ�ͻ�������ʼ����פȤ���ʸ����Ф��Ƥ� */
	/*      �饹�ȤΥ�����<�ȱ������:��ͻ�������ʼ��>�� */
	/*      �������Υ�����<�ȱ������:��ͻ��������>����Ϳ���� */
  	for (j = tag_num - 1; j >= 0; j--) {

	    /* ��ͭɽ����Ǥ�����ϸ�� */
	    if (check_feature((tag_ptr + j)->f, "NE") ||
		check_feature(((tag_ptr + j)->mrph_ptr)->f, "NE")) {
		break;
	    }

	    /* ���졢����̾�졢����Ū̾�졢����̾�� */
	    /* ������٤˷�����ƻ�Ͻ��� */
	    if (((tag_ptr + j)->head_ptr)->Hinshi == 6 &&
		((tag_ptr + j)->head_ptr)->Bunrui > 7 ||
		((tag_ptr + j)->head_ptr)->Hinshi == 3 &&
		check_feature((tag_ptr + j)->f, "��:��")) {
		continue;
	    }

 	    if (/* �缭�Ǥ��� */
		j == tag_num - 1 ||
		
		/* ľ���̾�줬����̾�졢���ƻ�Ǥ��� */
		((tag_ptr + j + 1)->mrph_ptr)->Hinshi == 6 &&
		((tag_ptr + j + 1)->mrph_ptr)->Bunrui == 2 ||
		((tag_ptr + j + 1)->mrph_ptr)->Hinshi == 3 ||

		/* ľ�夬̾�����������Ǥ��� */
		((tag_ptr + j + 1)->mrph_ptr)->Hinshi == 14 &&
		((tag_ptr + j + 1)->mrph_ptr)->Bunrui < 5 ||

		/* ľ���̾��γʥե졼��������¸�ߤ��� */
		(tag_ptr + j + 1)->cf_ptr &&
		check_examples(((tag_ptr + j)->head_ptr)->Goi2,
			       strlen(((tag_ptr + j)->head_ptr)->Goi2),
				((tag_ptr + j + 1)->cf_ptr)->ex_list[0],
				((tag_ptr + j + 1)->cf_ptr)->ex_num[0]) >= 0) {

		word[0] = '\0';
		for (k = (tag_ptr + j)->head_ptr - (sp->bnst_data + i)->mrph_ptr; 
		     k >= 0; k--) {

		    /* ��Ƭ���ü졢�ȱ���Ƭ���ϴޤ�ʤ� */
		    if (!strncmp(word, "\0", 1) &&
			(((tag_ptr + j)->head_ptr - k)->Hinshi == 1 ||
			 check_feature(((tag_ptr + j)->head_ptr - k)->f, "�ȱ���Ƭ��")))
			continue;
		    strcat(word, ((tag_ptr + j)->head_ptr - k)->Goi2);
		}
		if (strncmp(word, "\0", 1)) {
		    sprintf(buf, "�ȱ������:%s", word);
		    assign_cfeature(&((tag_ptr + j)->f), buf);
		    register_entity_cache(word);
		}
	    } else {
		break;
	    }
	}
	
	/* ��ͭɽ����ޤ�ʸ��Ǥ����� */
 	for (j = tag_num - 1; j >= 0; j--) {   
	    
	    /* ��ͭɽ���μ缭�ˤ���Ϳ */
	    if ((cp = check_feature((tag_ptr + j)->f, "NE"))) {
		while (strncmp(cp, ")", 1)) cp++;
		register_entity_cache(cp + 1);
		sprintf(buf, "�ȱ������:%s", cp + 1);
		assign_cfeature(&((tag_ptr + j)->f), buf);
		continue;
	    } 
	    /* ��ͭɽ����Ǥ�����(DATE�ޤ���LOCATION�ξ��) */
	    mrph_num = (tag_ptr + j)->mrph_num - 1;
	    if ((cp = check_feature(((tag_ptr + j)->mrph_ptr + mrph_num)->f, "NE")) &&
		/* DATE�Ǥ���л���̾�졢̾����̾����������ڤ� */
		(!strncmp(cp + 3, "DATE", 4) && 
		 (((tag_ptr + j)->mrph_ptr + mrph_num)->Hinshi == 6 &&
		  ((tag_ptr + j)->mrph_ptr + mrph_num)->Bunrui == 10 ||
		  ((tag_ptr + j)->mrph_ptr + mrph_num)->Hinshi == 14 &&
		  ((tag_ptr + j)->mrph_ptr + mrph_num)->Bunrui == 3) || 
		 /* LOCATION�Ǥ����̾�����ü����������ڤ� */
		 !strncmp(cp + 3, "LOCATION", 8) && 
		 ((tag_ptr + j)->mrph_ptr + mrph_num)->Hinshi == 14 &&
		 ((tag_ptr + j)->mrph_ptr + mrph_num)->Bunrui == 4)) {
		
		for (k = 0; !(cp = check_feature((tag_ptr + j + k)->f, "NE")); k++);
		while (strncmp(cp, ")", 1)) cp++;
		/* cp + 1 ���оݤθ�ͭɽ���ؤΥݥ��� */
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
	    /* ��ͭ���������ɽ��(ex. ���ԥ����̥�) */
	    if (j < tag_num - 1 && 
		check_feature(((tag_ptr + j + 1)->mrph_ptr + mrph_num)->f, "NE") &&
		!check_feature(((tag_ptr + j)->mrph_ptr + mrph_num)->f, "NE")) {
		word[0] = '\0';
		for (k = (tag_ptr + j)->head_ptr - (sp->bnst_data + i)->mrph_ptr; 
		     k >= 0; k--) {
		    
		    /* ��Ƭ���ü졢�ȱ���Ƭ���ϴޤ�ʤ� */
		    if (!strncmp(word, "\0", 1) &&
			(((tag_ptr + j)->head_ptr - k)->Hinshi == 1 ||
			 check_feature(((tag_ptr + j)->head_ptr - k)->f, "�ȱ���Ƭ��")))
			continue;
		    strcat(word, ((tag_ptr + j)->head_ptr - k)->Goi2);
		}
		if (strncmp(word, "\0", 1)) {
		    register_entity_cache(word);
		    sprintf(buf, "�ȱ������:%s", word);
		    assign_cfeature(&((tag_ptr + j)->f), buf);
		}
	    }	    
	}   
	/* �Ǹ��ʸ��Ƭ���鸫�Ƥ���entity_cache��¸�ߤ���ɽ���Ǥ������Ϳ���� */
  	for (j = 0; j < tag_num; j++) {

	    if (check_feature((tag_ptr + j)->f, "�ȱ������") ||
		check_feature((tag_ptr + j)->f, "NE")) break;

	    word[0] = '\0';
	    for (k = (tag_ptr + j)->head_ptr - (sp->bnst_data + i)->mrph_ptr; 
		 k >= 0; k--) 
		strcat(word, ((tag_ptr + j)->head_ptr - k)->Goi2);

	    if (check_entity_cache(word)) {
		register_entity_cache(word);
		sprintf(buf, "�ȱ������:%s", word);
		assign_cfeature(&((tag_ptr + j)->f), buf);
	    }
	}
    }
}

/*==================================================================*/
	 int compare_strings(char *antecedent, char *anaphor, int flag)
/*==================================================================*/
{
    /* �ȱ���������Ի�������� */
    /* flag��Ω�äƤ������antecednt����Ƭ��anaphor���ޤޤ�Ƥ����OK */

    int i, j;
    char word[WORD_LEN_MAX * 4];

    /* Ʊɽ���ξ�� */
    if (!strcmp(antecedent, anaphor)) return 1;

    /* flag��Ω�äƤ����� */
    if (flag && !strncmp(antecedent, anaphor, strlen(anaphor))) return 1;

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
    /* �����ܶ�ԡפȡ�����פΤ褦��Ʊ��ɽ��Ʊ��ʸ����ޤ���ˤ�̤�б� */
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

    int j, k, l, m, flag;
    char word[WORD_LEN_MAX], buf[WORD_LEN_MAX], *cp;
    SENTENCE_DATA *sdp;
    TAG_DATA *tag_ptr;
 
    sdp = sentence_data + sp->Sen_num - 1;
    for (j = 0; j <= sdp - sentence_data; j++) { /* �ȱ��褬��ʸ���� */
	
	for (k = j ? (sdp - j)->Tag_num - 1 : i - 1; k >= 0; k--) { /* �ȱ���Υ��� */
   
	    tag_ptr = (sdp - j)->tag_data + k;	    		

	    /* �ȱ�����䡢��Ի���䤬�Ȥ�˸�ͭɽ���Ǥ������ */
	    /* Ʊ��θ�ͭɽ���Τߤ���Ի�Ȥ��� */ 
	    if (ne && check_feature(tag_ptr->f, "NE") &&
		strncmp(ne, check_feature(tag_ptr->f, "NE"), 7))
		continue;
		
	    /* setubi��Ϳ����줿��硢��³��̾������������� */
	    if (setubi && strcmp((tag_ptr->head_ptr + 1)->Goi2, setubi))
		continue;

	    /* ��ͭ̾����Ǥ�����ϼ缭�ʳ�����Ի����Ȥ��ʤ� */
	    /* ������PERSON�ξ��Τ��㳰�Ȥ��� */
	    if (!check_feature(tag_ptr->f, "�ȱ������") &&
		check_feature((tag_ptr->head_ptr)->f, "NE")) continue;

	    for (l = tag_ptr->head_ptr - (tag_ptr->b_ptr)->mrph_ptr; l >= 0; l--) {

		/* flag��Ω�ä����Ͼȱ�����䤬��Ի�������Ƭ�˴ޤޤ�Ƥ����OK */
		flag = 0;
		if (check_feature((tag_ptr->head_ptr)->f, "NE:PERSONtail") ||
		    check_feature((tag_ptr->head_ptr)->f, "NE:LOCATIONtail"))
		    flag = 1;

		word[0] = '\0';
		for (m = l; m >= 0; m--) {
		    strcat(word, (tag_ptr->head_ptr - m)->Goi2); /* ��Ի���� */
		    if (flag && m < l &&
			(check_feature((tag_ptr->head_ptr - m)->f, "NE:PERSONhead") ||
			 check_feature((tag_ptr->head_ptr - m)->f, "NE:LOCATIONhead")))
			flag = 0;				
		}	

		if (compare_strings(word, anaphor, flag)) { /* Ʊ��ɽ���Ǥ���� */
		    if (j == 0) {
			sprintf(buf, "C��;��%s%s��;=;0;%d;9.99:%s(Ʊ��ʸ):%dʸ��",
				word, setubi ? setubi : "", k, 
				sp->KNPSID ? sp->KNPSID + 5 : "?", k);
		    }
		    else {
			sprintf(buf, "C��;��%s%s��;=;%d;%d;9.99:%s(%dʸ��):%dʸ��",
				word, setubi ? setubi : "", j, k, 
				sp->KNPSID ? sp->KNPSID + 5 : "?", j, k);
		    }
		    assign_cfeature(&((sp->tag_data + i)->f), buf);
		    assign_cfeature(&((sp->tag_data + i)->f), "������"); 
#ifdef USE_SVM
		    if (OptNE) {
			if ((cp = check_feature(tag_ptr->f, "NE")) && !setubi) {
			    while (strncmp(cp, ")", 1)) cp++;
			    if (!strcmp(cp + 1, word)) {
				ne_corefer(sp, i, anaphor,
					   check_feature(tag_ptr->f, "NE"));
			    }
			} 
		    }
#endif
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
    char buf[WORD_LEN_MAX];
    MRPH_DATA *mrph_ptr;

    mrph_ptr = tag_ptr->mrph_ptr;
    /* ���������ޤ�NE��Ǥ�����Τ��оݤȤ��� */
    if (!check_feature((mrph_ptr - 1)->f, "NE") &&
	!(check_feature((mrph_ptr - 2)->f, "NE") &&
	  (mrph_ptr - 1)->Hinshi == 1 && 
	  (mrph_ptr - 1)->Bunrui == 5)) /* ľ��������Ǥ��� */
	return 0;
	
    flag = 0;
    for (i = 0;; i++) {
	if (check_feature((mrph_ptr + i)->f, "NE")) {
	    /* ����Ū�ˤϡ��֥å��塦����ꥫ������ */
	    /* ������̱�޴���Ĺ�ʤɤ����ꤷ�Ƥ��� */
	    continue;
	}
	else if (check_feature((mrph_ptr + i)->f, "��̾����")) {
	    flag = 1;
	    continue;
	}
	else break;
    }
    if (!flag) return 0;
	
    /* ʣ���Υ����ˤޤ����äƤ�����ϼ��Υ����˿ʤ� */
    while (i >= tag_ptr->mrph_num) {
	i -= tag_ptr->mrph_num;
	tag_ptr++;
    }
    
    sprintf(buf, "C��;��%s��;=;0;%d;9.99:%s(Ʊ��ʸ):%dʸ��",
	    cp, j, sp->KNPSID ? sp->KNPSID + 5 : "?", 
	    tag_ptr - sp->tag_data);
    assign_cfeature(&(tag_ptr->f), buf);
    assign_cfeature(&(tag_ptr->f), "������");    
}

/*==================================================================*/
	       void corefer_analysis(SENTENCE_DATA *sp)
/*==================================================================*/
{
    int i;
    char *anaphor, *cp, *ne;
    MRPH_DATA *mrph_ptr;
    
    for (i = sp->Tag_num - 1; i >= 0; i--) { /* ����ʸ�Υ���ñ��:i���ܤΥ����ˤĤ��� */

	/* PERSON + ��̾���� �ν��� */
	if ((cp = check_feature((sp->tag_data + i)->f, "NE")) &&
	    !strncmp(cp + 4, "PERSON", 6)) {
	    person_post(sp, sp->tag_data + i + 1, cp + 11, i);
	}

	/* �����Ȳ��Ϥ�Ԥ���� */
	/* �ȱ������Ǥ��ꡢ��ͭɽ����θ졢�ޤ��� */
	/* Ϣ�λ���ֻؼ���ʳ��˽�������Ƥ��ʤ��� */
	if ((anaphor = check_feature((sp->tag_data + i)->f, "�ȱ������")) &&
	    (check_feature((sp->tag_data + i)->f, "NE") ||
	     check_feature(((sp->tag_data + i)->mrph_ptr +
			    (sp->tag_data + i)->mrph_num - 1)->f, "NE") ||
	     !get_modify_num(sp->tag_data + i) || /* ��������Ƥ��ʤ� */
	     (((sp->tag_data + i)->mrph_ptr - 1)->Hinshi == 1 && 
	      ((sp->tag_data + i)->mrph_ptr - 1)->Bunrui == 2) || /* ľ���������Ǥ��� */
	     check_feature((((sp->tag_data + i)->b_ptr)->child[0])->f, 
			   "Ϣ�λ���ֻؼ���"))) {
	    /* �ؼ���ξ�� */
	    if (check_feature((sp->tag_data + i)->f, "�ؼ���")) {
		continue; /* �����ǤϽ����򤷤ʤ� */
	    }   
	    mrph_ptr = (sp->tag_data + i)->head_ptr + 1;
	    if (/* ̾�������������դ�����ͭɽ���ʳ��θ�Ϥޤ���������ޤ᤿��Τ�Ĵ�٤� */
		!((ne = check_feature((sp->tag_data + i)->f, "NE"))) &&
		mrph_ptr->Hinshi == 14 && mrph_ptr->Bunrui < 5 &&
		search_antecedent(sp, i, anaphor+11, mrph_ptr->Goi2, NULL) ||
		/* ���̤ξ�� */
		search_antecedent(sp, i, anaphor+11, NULL, ne)) {
		    continue;
	    }
	}
    }
}

/*====================================================================
                               END
====================================================================*/
