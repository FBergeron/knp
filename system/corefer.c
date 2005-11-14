/*====================================================================

			     �����Ȳ���

                                               R.SASANO 05. 9.24

    $Id$
====================================================================*/
#include "knp.h"

DBM_FILE synonym_db;

char *SynonymFile;

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
	       int get_modify_num(BNST_DATA *b_ptr)
/*==================================================================*/
{
    /* ���������Ƥ����Ĥ�ʸ��˽�������Ƥ��뤫���֤� */
    
    int i;

    if (!b_ptr->child[0]) {
	return 0;
    }
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
    /* �ȱ������Ȥ���feature����Ϳ���� */
    /* �ȱ������Ȥ϶���ʣ��̾��������ȤʤäƤ����Τ��� */

    /* ����ʣ��̾��δ�� */
    /* 	��ͭɽ��(LOCATION��DATE���ڤ�) */
    /* 	�����θ줬����̾��Ǥ�����ڤ� */
    /* 	�����θ�Ȥ������θ줬̾��ʥե졼��ˤ����ȹ礻�ξ����ڤ� */

    int i, j, k, tag_num, mrph_num;
    char word[WORD_LEN_MAX * 2], buf[WORD_LEN_MAX * 2], *cp;
    TAG_DATA *tag_ptr;
    
    for (i = 0; i < sp->Bnst_num; i++) {
	
	if (!check_feature((sp->bnst_data + i)->f, "�θ�")) continue;

	tag_num = (sp->bnst_data+i)->tag_num;
	tag_ptr = (sp->bnst_data + i)->tag_ptr;

  	for (j = tag_num - 1; j >= 0; j--) {

	    /* ��ͭɽ����Ǥ�����ϸ�� */
	    if (check_feature((tag_ptr + j)->f, "NE") ||
		check_feature(((tag_ptr + j)->mrph_ptr + 
			       (tag_ptr + j)->mrph_num - 1)->f, "NE")) {
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
		
		/* ľ���̾�줬����̾��Ǥ��� */
		((tag_ptr + j + 1)->mrph_ptr)->Hinshi == 6 &&
		((tag_ptr + j + 1)->mrph_ptr)->Bunrui == 2 ||

		/* ľ�夬̾�����������Ǥ��� */
		((tag_ptr + j + 1)->mrph_ptr)->Hinshi == 14 &&
		((tag_ptr + j + 1)->mrph_ptr)->Bunrui < 5 ||

		/* ľ���̾��γʥե졼��������¸�ߤ��� */
		(tag_ptr + j + 1)->cf_ptr &&
		check_examples(((tag_ptr + j)->mrph_ptr)->Goi2,
			       strlen(((tag_ptr + j)->mrph_ptr)->Goi2),
				((tag_ptr + j + 1)->cf_ptr)->ex_list[0],
				((tag_ptr + j + 1)->cf_ptr)->ex_num[0]) >= 0) {

		word[0] = '\0';
		for (k = (tag_ptr + j)->head_ptr - (sp->bnst_data + i)->mrph_ptr; 
		     k >= 0; k--) {

		    /* ��Ƭ��̾����Ƭ�����ü�ϴޤ�ʤ� */
		    if (!strncmp(word, "\0", 1) &&
			(((tag_ptr + j)->head_ptr - k)->Hinshi == 1 ||
			 ((tag_ptr + j)->head_ptr - k)->Hinshi == 13 &&
			 ((tag_ptr + j)->head_ptr - k)->Bunrui == 1))
			continue;
		    strcat(word, ((tag_ptr + j)->head_ptr - k)->Goi2);
		}
		if (strncmp(word, "\0", 1)) {
		    sprintf(buf, "�ȱ������:%s", word);
		    assign_cfeature(&((tag_ptr + j)->f), buf);
		}
	    } else {
		break;
	    }
	}
	
 	for (j = tag_num - 1; j >= 0; j--) {   
	    
	    /* ��ͭɽ���μ缭�ˤ���Ϳ */
	    if ((cp = check_feature((tag_ptr + j)->f, "NE"))) {
		while (strncmp(cp, ")", 1)) cp++;
		sprintf(buf, "�ȱ������:%s", cp + 1);
		assign_cfeature(&((tag_ptr + j)->f), buf);
		continue;
	    } 
	    /* ��ͭɽ����Ǥ����� */
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
		for (k = 0; 
		     strncmp(cp + k + 1, ((tag_ptr + j)->mrph_ptr + mrph_num)->Goi2, 
			     strlen(((tag_ptr + j)->mrph_ptr + mrph_num)->Goi2));
		     k++);
		word[0] = '\0';
		strncpy(word, cp + 1, k);
		strcat(word, ((tag_ptr + j)->mrph_ptr + mrph_num)->Goi2);
		sprintf(buf, "�ȱ������:%s", word);
		assign_cfeature(&((tag_ptr + j)->f), buf);	
	    }			
	}   
    }
}

/*==================================================================*/
               int compare_strings(char *antecedent, char *anaphor)
/*==================================================================*/
{
    /* �ȱ���������Ի�������� */

    int i, j;
    char word[WORD_LEN_MAX * 4];

    /* Ʊɽ���ξ�� */
    if (!strcmp(antecedent, anaphor)) return 1;

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
               int search_antecedent(SENTENCE_DATA *sp, int i, 
				     char *anaphor, char *setubi)
/*==================================================================*/
{
    /* ���Ϥ��줿�����ȡ������ȴط��ˤ��륿���������ʸ���鸡������ */
    /* setubi��Ϳ����줿����ľ�����������ޤ��õ������ */

    /* �����ȴط��ˤ���줬���Ĥ��ä����Ϸ�̤�feature����Ϳ���� */
    /* �����ȴط��ˤ���Ȥ��줿�ȱ���ʸ�������Ƭ�Υ������ֹ� */
    /* ���Ĥ���ʤ��ä�����-2���֤� */

    int j, k, l, m;
    char word[WORD_LEN_MAX * 2], buf[WORD_LEN_MAX * 2];
    SENTENCE_DATA *sdp;
    TAG_DATA *tag_ptr;
 
    sdp = sentence_data + sp->Sen_num - 1;
    for (j = 0; j <= sdp - sentence_data; j++) { /* �ȱ��褬��ʸ���� */
	
	for (k = j ? (sdp - j)->Tag_num - 1 : i - 1; k >= 0; k--) { /* �ȱ���Υ��� */
   
	    tag_ptr = (sdp - j)->tag_data + k;	    		

	    /* setubi��Ϳ����줿��硢��³��̾������������� */
	    if (setubi && strcmp((tag_ptr->head_ptr + 1)->Goi2, setubi))
		continue;
		
	    for (l = tag_ptr->head_ptr - (tag_ptr->b_ptr)->mrph_ptr; l >= 0; l--) {

		word[0] = '\0';
		for (m = l; m >= 0; m--) {
		    strcat(word, (tag_ptr->head_ptr - m)->Goi2); /* ��Ի���� */
		}	

		if (compare_strings(word, anaphor)) { /* Ʊ��ɽ���Ǥ���� */
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
		    assign_cfeature(&((sp->tag_data + i)->f), "������");
		    assign_cfeature(&((sp->tag_data + i)->f), buf);
		    
		    /* ʣ���Υ����˴ط�������ν��� */
		    for (m = 0; m < l; i--) 
			m += (sp->tag_data + i)->mrph_num;
		    return i;
		}
	    }
	}	    
    }
    return -2;
}

/*==================================================================*/
               int search_antecedent_NE(SENTENCE_DATA *sp, int i, 
					char *anaphor)
/*==================================================================*/
{
    int j, k;
    char *word, buf[WORD_LEN_MAX * 2];
    SENTENCE_DATA *sdp;
    TAG_DATA *tag_ptr;
 
    sdp = sentence_data + sp->Sen_num - 1;
    for (j = 0; j <= sdp - sentence_data; j++) { /* �ȱ��褬��ʸ���� */
	for (k = j ? (sdp - j)->Tag_num - 1 : i - 1; k >= 0; k--) { /* �ȱ���Υ��� */  
	    tag_ptr = (sdp - j)->tag_data + k;	    		
	    if (!(word = check_feature(tag_ptr->f, "NE"))) continue;

	    while (strncmp(word, ")", 1)) word++;
	    if (compare_strings(++word, anaphor)) {
		if (j == 0) {
		    sprintf(buf, "C��;��%s��;=;0;%d;9.99:%s(Ʊ��ʸ):%dʸ��",
			    word, k, sp->KNPSID ? sp->KNPSID + 5 : "?", k);
		}
		else {
		    sprintf(buf, "C��;��%s��;=;%d;%d;9.99:%s(%dʸ��):%dʸ��",
			    word, j, k, sp->KNPSID ? sp->KNPSID + 5 : "?", j, k);
		}
		assign_cfeature(&((sp->tag_data + i)->f), "������");
		assign_cfeature(&((sp->tag_data + i)->f), buf);

		while (i > 0 &&
		       check_feature(((sp->tag_data + i)->mrph_ptr +
				      (sp->tag_data + i)->mrph_num)->f, "NE")) i--;
		return i;
	    }
	}
    }
    return -2;
}

/*==================================================================*/
	       void corefer_analysis(SENTENCE_DATA *sp)
/*==================================================================*/
{
    int i, next_i;
    char *anaphor;
    MRPH_DATA *mrph_ptr;
    
    for (i = sp->Tag_num - 1; i >= 0; i--) { /* ����ʸ�Υ���ñ��:i���ܤΥ����ˤĤ��� */

	/* �����Ȳ��Ϥ�Ԥ���� */
	/* �ȱ������Ǥ��ꡢ��ͭɽ����θ졢�ޤ��� */
	/* Ϣ�λ���ֻؼ���ʳ��˽�������Ƥ��ʤ��� */
	if (!(anaphor = check_feature((sp->tag_data + i)->f, "�ȱ������")) ||
	    !check_feature((sp->tag_data + i)->f, "NE") &&
	    !check_feature(((sp->tag_data + i)->mrph_ptr +
			    (sp->tag_data + i)->mrph_num)->f, "NE") &&
	    get_modify_num((sp->tag_data + i)->b_ptr) && /* ��������Ƥ��� */
	    !check_feature((((sp->tag_data + i)->b_ptr)->child[0])->f, "Ϣ�λ���ֻؼ���"))
	    continue;

	/* �ؼ���ξ�� */
	if (check_feature((sp->tag_data + i)->f, "�ؼ���")) {
	    continue;
	}

	/* ��ͭɽ���ξ�� */
	if (check_feature((sp->tag_data + i)->f, "NE")) {
	    next_i = search_antecedent_NE(sp, i, anaphor+11);
	    if (next_i != -2) {
		i = next_i;
		continue;
	    }
	}
	
	/* ̾�������������դ��Ƥ�����Ϥޤ���������ޤ᤿��Τ�Ĵ�٤� */
	mrph_ptr = (sp->tag_data + i)->head_ptr + 1;
	if (mrph_ptr->Hinshi == 14 && mrph_ptr->Bunrui < 5) {
	    next_i = search_antecedent(sp, i, anaphor+11, mrph_ptr->Goi2);
	    if (next_i != -2) {
		i = next_i;
		continue;
	    }
	}
	/* ���̤ξ�� */
	next_i = search_antecedent(sp, i, anaphor+11, NULL);
	i = (next_i == -2) ? i : next_i;
    }
}

/*====================================================================
                               END
====================================================================*/
