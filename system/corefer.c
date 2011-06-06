/*====================================================================

			     �����Ȳ���

                                               R.SASANO 05. 9.24

    $Id$
====================================================================*/
#include "knp.h"

int ASCEND_SEN_MAX = 20;
int corefer_id = 0;
DBM_FILE synonym_db;
char *SynonymFile;

/*==================================================================*/
			void init_Synonym_db()
/*==================================================================*/
{
    char *db_filename;

    if (SynonymFile) {
	db_filename = check_dict_filename(SynonymFile, TRUE);
    }
    else {
	db_filename = check_dict_filename(SYONONYM_DIC_DB_NAME, FALSE);
    }

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

    /* OptCorefer >= 4�ξ��Ͻ�������Ƥ��뤫�ɤ������Ѥ��ʤ� */
    if (OptCorefer >= 4) return 0;

    /* ��°����ʸ�᤬��������Ƥ��ʤ���� */
    if (!b_ptr->child[0]) {
	return 0;
    }

    if (OptCorefer == 1) {
	/* "ľ��������"�Ǥ�����ϼ缭�ʳ��Ǥ⽤������Ƥ���ȹͤ��� */
	if (tag_ptr->head_ptr != b_ptr->head_ptr) {
	    if (check_feature(tag_ptr->f, "ľ��������")) 
		return 1;
	    else 
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
    /* ���κݡ���Ƭ�η����ǤΤ���ɽɽ������¸���ơ��̤���¸�����ȱ�������������� */
    /* ex. ��Ω�Ƥ�������� �� ��Ω�Ƥ�������ס���Ω���Ƥ�/���Ƥ����+����� */

    int i, j, k, l, tag_num, mrph_num, rep_flag;
    char word[WORD_LEN_MAX * 2], word_rep[WORD_LEN_MAX * 2], buf[WORD_LEN_MAX * 2], *cp;
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
		    while (*cp != ':') cp++;
		    sprintf(buf, "�ȱ������:%s", cp + 1);
		    assign_cfeature(&((tag_ptr + j)->f), buf, FALSE);
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
		    while (*cp != ':') cp++;
		    /* cp + 1 ���оݤθ�ͭɽ��ʸ����ؤΥݥ��� */
		    for (k = 0; 
			 strncmp(cp + k + 1, ((tag_ptr + j)->mrph_ptr + mrph_num)->Goi2, 
				 strlen(((tag_ptr + j)->mrph_ptr + mrph_num)->Goi2));
			 k++);
		    strncpy(word, cp + 1, k);
		    word[k] = '\0';
		    strcat(word, ((tag_ptr + j)->mrph_ptr + mrph_num)->Goi2);
		    sprintf(buf, "�ȱ������:%s", word);
		    assign_cfeature(&((tag_ptr + j)->f), buf, FALSE);
		}
	    }
	    
	    else {
		/* ��ͭɽ����θ��缭�Ȥ��ʤ����
		   
		/* ���졢����̾�졢������٤˷�����ƻ�Ͻ��� */
		if ((tag_ptr + j)->head_ptr->Hinshi == 6 &&
		    (tag_ptr + j)->head_ptr->Bunrui > 7 &&
		    (tag_ptr + j)->head_ptr->Bunrui < 9 ||
		    (tag_ptr + j)->head_ptr->Hinshi == 3 &&
		    check_feature((tag_ptr + j)->f, "��:��")) {
		    continue;
		}

		word[0] = word_rep[0] = '\0';
		for (k = (tag_ptr + j)->head_ptr - (sp->bnst_data + i)->mrph_ptr; 
		     k >= 0; k--) {
		    
		    /* ��Ƭ���ü졢�ȱ���Ƭ���ϴޤ�ʤ� */
		    if (!word[0] &&
			(((tag_ptr + j)->head_ptr - k)->Hinshi == 1 ||
			 check_feature(((tag_ptr + j)->head_ptr - k)->f, "�ȱ���Ƭ��")))
			continue;
		    
		    /* �֡��פʤɤ�����ϴޤ�ʤ� */
		    if (!strcmp(((tag_ptr + j)->head_ptr - k)->Goi2, "��") ||
			check_feature(((tag_ptr + j)->head_ptr - k)->f, "��̽�")) {
			if (k > 0) word[0] = word_rep[0] = '\0';
		    }
		    else {
			if (OptCorefer == 5) word[0] = '\0';
			strcat(word, ((tag_ptr + j)->head_ptr - k)->Goi2);
			if (word_rep[0] == '\0') {
			    if (k > 0 &&
				((cp = check_feature(((tag_ptr + j)->head_ptr - k)->f, "��ɽɽ���ѹ�")) ||
				 (cp = check_feature(((tag_ptr + j)->head_ptr - k)->f, "��ɽɽ��")))) {
				strcat(word_rep, strchr(cp, ':') + 1);
				strcat(word_rep, "+");
			    }
			}
			else {
			    strcat(word_rep, ((tag_ptr + j)->head_ptr - k)->Goi2);
			}
		    }
		}

		if (word[0]) {
		    sprintf(buf, "�ȱ������:%s", word);
		    assign_cfeature(&((tag_ptr + j)->f), buf, FALSE);
		}
		if (word_rep[0]) {
		    sprintf(buf, "�Ծȱ������:%s", word_rep);
		    assign_cfeature(&((tag_ptr + j)->f), buf, FALSE);
		}
	    }
	}
    }

    /* �����Τդ꤬�ʤؤ��б� */
    for (j = 0; j < sp->Tag_num; j++) {
	
	for (i = 0; i < (sp->tag_data + j)->mrph_num; i++) {

	    if (j + 1 < sp->Tag_num &&
		(check_feature(((sp->tag_data + j)->mrph_ptr + i + 1)->f, 
			      "�Ҥ餬��") ||
		!strcmp(((sp->tag_data + j)->mrph_ptr + i + 1)->Goi2, "��"))) break;
	    
	    word[0] = '\0';
	    for (k = 0; (sp->tag_data + j)->mrph_ptr + i - k > sp->mrph_data; k++) {
		if (!check_feature(((sp->tag_data + j)->mrph_ptr + i - k)->f, 
				   "�Ҥ餬��") &&
		    strcmp(((sp->tag_data + j)->mrph_ptr + i - k)->Goi2, "��")) break;
	    }
	    if (k > 0 && 
		check_feature(((sp->tag_data + j)->mrph_ptr + i - k)->f, "��̻�")) {
		for (k = k - 1; k >= 0; k--) 
		    strcat(word, ((sp->tag_data + j)->mrph_ptr + i - k)->Goi2);
	    }
	    
	    if (word[0]) {
		sprintf(buf, "�ȱ������:%s", word);
		assign_cfeature(&((sp->tag_data + j)->f), buf, FALSE);
		assign_cfeature(&((sp->tag_data + j)->f), "�ɤ���", FALSE);
	    }
	}		
    }
}

/*==================================================================*/
int compare_strings(char *antecedent, char *anaphor, char *ana_ne, 
		    int yomi_flag, TAG_DATA *tag_ptr, char *rep)
/*==================================================================*/
{
    /* �ȱ���������Ի�������� */
    /* yomi_flag��Ω�äƤ�����ϴ������ɤߤξȱ� */
    /* rep�����������Ƭ�����Ǥ���ɽɽ����������Ӥ�����(ex.��Ω�Ƥ������� = Ω�Ƥ��������) */

    int i, j, left, right;
    char *ant_ne, word[WORD_LEN_MAX * 4], *value;

    ant_ne = check_feature(tag_ptr->f, "NE");

    /* �ɤ����ξ�� */
    if (yomi_flag) { 
    /* ex. �������Ϻ�ʤʤ����ޡ����錄��) */
    /* ���
       �Ȥꤢ������̾�ξ��Τ�
       �����ޥå�ʸ�����߸����ޥå�ʸ������2 > anaphoraʸ����
       �Ǥ����硢�ɤ�����ɽ�路�Ƥ����Ƚ�� 
       ������antecedent��ľ�夬<��̻�>�ξ��(yomi_flag=2)�ξ���Ϣ³���Ƥ���ȹͤ�
       �ޥå�ʸ�����ξ��ʤ�����2ʸ���ܡ��ʥ� */
           
	if (!ant_ne || strncmp(ant_ne, "NE:PERSON", 7)) return 0;

	left = right = 0;
	for (i = 0; i < strlen(anaphor); i += BYTES4CHAR) {
	    if (strncmp(antecedent + i, anaphor + i, BYTES4CHAR)) {
		break;
	    }
	    left++;
	}  
	for (j = 0; j < strlen(anaphor); j += BYTES4CHAR) {
	    if (strncmp(antecedent + strlen(antecedent) - j - BYTES4CHAR, 
			anaphor + strlen(anaphor) - j - BYTES4CHAR, BYTES4CHAR)) {
		break;
	    } 
	    right++;
	}
	if (yomi_flag == 2) (left > right) ? (right += 2) : (left += 2);
	if (left * right * 2 * BYTES4CHAR > strlen(anaphor)) return 1;
	return 0;
    }

    /* �ۤʤ����θ�ͭɽ���ξ����Բ� */ 
    if (ana_ne && ant_ne && strncmp(ana_ne, ant_ne, 7)) return 0;

    /* rep�����������Ƭ�����Ǥ���ɽɽ����������Ӥ����� */
    if (rep) {
	if (!strncmp(anaphor, rep, strlen(rep)) &&
	    !strncmp(anaphor + strlen(rep), "+", 1) &&
	    !strcmp(anaphor + strlen(rep) + 1, antecedent)) return 1;
    }

    /* Ʊɽ���ξ�� */
    if (!strcmp(antecedent, anaphor)) return 1;

    /* ��ͭɽ����Ʊɽ���ξ��(ʸ���ޤ������ͭɽ���Τ���) */
    if (ant_ne && ana_ne && !strcmp(ant_ne, ana_ne)) {
	return 1;
    }

    /* ��Ի줬PERSON�Ǥ�����Ͼȱ�����䤬��Ի�������Ƭ�˴ޤޤ�Ƥ����OK */
    /* ��Ի줬LOCATION�Ǥ�����Ϥ���˾ȱ�����䤬��������1ʸ������û�������Τ�OK */
    /* ex. ¼���ٻ�=¼������ʬ��=��ʬ */
    if (ant_ne && strlen(ant_ne) > strlen(antecedent) && /* ��Ի줬NE���ΤǤ��� */
	!strcmp(ant_ne + strlen(ant_ne) - strlen(antecedent), antecedent) &&
	(!strncmp(ant_ne, "NE:PERSON", 7) && ana_ne && !strncmp(ana_ne, "NE:PERSON", 7) || 
	 !strncmp(ant_ne, "NE:LOCATION", 7) && strlen(antecedent) - strlen(anaphor) == BYTES4CHAR &&
	 check_feature(tag_ptr->head_ptr->f, "��������")) &&
	!strncmp(antecedent, anaphor, strlen(anaphor))) return 1;
    
    /* Ʊ��ɽ�������ɤ߹���ʤ��ä����Ϥ����ǽ�λ */
    if (!synonym_db) return 0;

    /* ���Τޤ�Ʊ��ɽ���������Ͽ����Ƥ����� */
    word[0] = '\0';
    strcpy(word, anaphor);
    strcat(word, ":");
    strcat(word, antecedent);
    if (value = db_get(synonym_db, word)) {
	free(value);
	return 1;
    } 

    /* ���夫��Ʊ��ɽ����ʸ���������ƻĤ��ʸ����Υڥ�����Ӥ��� */
    /* �ֶ�ͻ��������-����פȡ֥ǥ�Хƥ���-����פ�ǧ���Ǥ��� */
    /* �����ܶ�ԡפȡ�����פΤ褦��Ʊ��ɽ����Ʊ��ʸ����ޤ����ǧ���Ǥ��ʤ� */
    for (i = 0; i < strlen(anaphor); i += BYTES4CHAR) {
	if (strncmp(antecedent + i, anaphor + i, BYTES4CHAR)) {
	    break;
	}
    }  
    for (j = 0; j < strlen(anaphor); j += BYTES4CHAR) {
	if (strncmp(antecedent + strlen(antecedent) - j - BYTES4CHAR, 
		    anaphor + strlen(anaphor) - j - BYTES4CHAR, BYTES4CHAR)) {
	    break;
	} 
    }
    if (strlen(anaphor) < i + j) return 0; /* ��ʸ����� ���� �ΤȤ� */

    memset(word, 0, sizeof(char) * WORD_LEN_MAX * 4);
    strncpy(word, anaphor + i, strlen(anaphor) - i - j);
    strcat(word, ":");
    strncat(word, antecedent + i, strlen(antecedent) - i - j);
    strcat(word, "\0");

    if (value = db_get(synonym_db, word)) {
	free(value);
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

    int j, k, l, m, length, yomi_flag, word2_flag, setubi_flag;
    /* word1:�֡��פʤɤ������ޤ�ʤ���Ի����(��Ի����1)
       word2:�֡��פʤɤ������ޤ����Ի����(��Ի����2)
       yomi2:��Ի����2���ɤ��� 
       anaphor_rep:�ȱ���������Ƭ�����Ǥ���ɽɽ����������� */
    char word1[WORD_LEN_MAX], word2[WORD_LEN_MAX], yomi2[WORD_LEN_MAX], buf[WORD_LEN_MAX], 
	*anaphor_rep;
    char *cp, CO[WORD_LEN_MAX];
    SENTENCE_DATA *sdp;
    TAG_DATA *tag_ptr;
 
    if ((cp = check_feature((sp->tag_data + i)->f, "�Ծȱ������"))) {
	anaphor_rep = strchr(cp, ':') + 1;
    }
    else {
	anaphor_rep = NULL;
    }
    yomi_flag = (check_feature((sp->tag_data + i)->f, "�ɤ���")) ? 1 : 0;

    sdp = sentence_data + sp->Sen_num - 1;
    for (j = 0; j <= sdp - sentence_data; j++) { /* �ȱ��褬��ʸ���� */
	if (j >= ASCEND_SEN_MAX) break; /* ASCEND_SEN_MAX�ʾ�����ʸ�Ϲ�θ���ʤ� */

	for (k = (j != 0) ? (sdp - j)->Tag_num - 1 : i - 1; k >= 0; k--) { /* �ȱ���Υ��� */
	    
	    tag_ptr = (sdp - j)->tag_data + k;	    		
	    
	    /* �ȱ������Ǥ�����ʳ�����Ի����Ȥ��ʤ� */
	    if (!check_feature(tag_ptr->f, "�ȱ������")) continue;
			
	    /* setubi��Ϳ����줿��硢��³��̾������������� */
	    if (setubi && tag_ptr->head_ptr < tag_ptr->mrph_ptr + tag_ptr->mrph_num - 1 && strcmp((tag_ptr->head_ptr + 1)->Goi2, setubi)) continue;

    	    /* �Ծȱ���ǽ����������Ϳ����Ƥ�������������Ⱦȱ������Ӥ�Ԥ�
	       Ʊɽ���Ǥ���ж����ȴط��ˤ���ȷ��� */
	    setubi_flag = 0;
	    if (!setubi && check_feature(tag_ptr->f, "�Ծȱ���ǽ������")) {
		for (l = 1; l <= tag_ptr->fuzoku_num; l++) {
		    if ((tag_ptr->head_ptr + l) && !strcmp((tag_ptr->head_ptr + l)->Goi2, anaphor)) {
			setubi_flag = l;
			break;
		    }
		}
	    }
	    
	    for (l = tag_ptr->head_ptr - (tag_ptr->b_ptr)->mrph_ptr; l >= 0; l--) {
		
		word1[0] = word2[0] = yomi2[0] = '\0';
		for (m = setubi_flag ? 0 : l; m >= 0; m--) {
		    /* ��Ƭ���ü졢�ȱ���Ƭ���ϴޤ�ʤ� */
		    if (!strncmp(word1, "\0", 1) &&
			((tag_ptr->head_ptr - m)->Hinshi == 1 ||
			 check_feature((tag_ptr->head_ptr - m)->f, "�ȱ���Ƭ��")))
			continue;
		    /* �֡��פʤɤ�����ϴޤ�ʤ�(word1) */
		    if (!strcmp((tag_ptr->head_ptr - m)->Goi2, "��") ||
			!strcmp((tag_ptr->head_ptr - m)->Goi2, "��") ||
			check_feature((tag_ptr->head_ptr - m)->f, "��̽�")) {
			word1[0] = '\0';
		    }
		    else {
			if (strlen(word1) + strlen((tag_ptr->head_ptr - m)->Goi2) >= WORD_LEN_MAX) break;
			strcat(word1, (tag_ptr->head_ptr - m)->Goi2); /* ��Ի����1 */
		    }
		    if (strlen(word2) + strlen((tag_ptr->head_ptr - m)->Goi2) >= WORD_LEN_MAX)	break;
		    strcat(word2, (tag_ptr->head_ptr - m)->Goi2); /* ��Ի����2 */
		    if (strlen(yomi2) + strlen((tag_ptr->head_ptr - m)->Yomi) >= WORD_LEN_MAX) break;
		    strcat(yomi2, (tag_ptr->head_ptr - m)->Yomi); /* ��Ի����2���ɤ��� */
		}
		if (setubi_flag) {
		    strcpy(word1, (tag_ptr->head_ptr + setubi_flag)->Goi2);
		}
		if (!word1[0]) continue;

		/* Ʊ��ʸ�����Ի���䤬�ȱ���˴ޤޤ�Ƥ�����Ͻ���(ex.�����ܡ����ܹ��) n*/
		if (j == 0 && (sp->tag_data + i)->b_ptr == (sp->tag_data + k)->b_ptr) {
		    length = 0;
		    for (m = 0; (sp->tag_data + k + 1)->head_ptr + m <= (sp->tag_data + i)->head_ptr; m++) {
			length += strlen(((sp->tag_data + k + 1)->head_ptr + m)->Goi2);
		    }
		    if (length < strlen(anaphor)) continue;
		}
		    		
		word2_flag = 0;
		if (setubi_flag ||
		    compare_strings(word1, anaphor, ne, 0, tag_ptr, NULL) ||
		    compare_strings(word2, anaphor, ne, 0, tag_ptr, NULL) && (word2_flag = 1) ||
		    /* ʸ�����Ƭ�ޤǴޤ����ľ���δ��ܶ���θ������� */
		    /* (ex.��Ω�Ƥ������� = Ω�Ƥ��������) */
		    l == tag_ptr->head_ptr - (tag_ptr->b_ptr)->mrph_ptr && anaphor_rep && 
		    !check_feature(tag_ptr->f, "ʸ����") && /* ʸ��μ缭�ξ��Τ� */
		    tag_ptr->b_ptr->child[0] && /* ľ���δ��ܶ�ȷ�������ط��ˤ��� */
		    check_feature((tag_ptr->b_ptr - 1)->f, "Ϣ�ν���") &&
		    ((cp = check_feature((tag_ptr->b_ptr - 1)->head_ptr->f, "��ɽɽ���ѹ�")) ||
		     (cp = check_feature((tag_ptr->b_ptr - 1)->head_ptr->f, "��ɽɽ��"))) &&
		    compare_strings(word1, anaphor_rep, ne, 0, tag_ptr, strchr(cp, ':') + 1) ||
		    /* �ɤ����ξ��(Ʊ��ʸ����10���ܶ�̤��) */
		    yomi_flag && j == 0 && (i - k < 10) &&
		    compare_strings(yomi2, anaphor, ne, 1, tag_ptr, NULL) ||
		    yomi_flag && j == 0 && (i - k < 10) &&
		    check_feature((tag_ptr + 1)->f, "��̻�") &&
		    compare_strings(yomi2, anaphor, ne, 2, tag_ptr, NULL) ||
		    /* �;�̾��ξ������� */
		    (check_feature((sp->tag_data + i)->f, "�;���̾��") &&
		     check_feature(tag_ptr->f, "NE:PERSON")) ||
		    /* ����̾��ξ������� */
		    (!j && (k == i - 1) && check_feature(tag_ptr->f, "�Բ��ϳ�-��") &&
		     check_feature((sp->tag_data + i)->f, "�Լ���̾��") &&
		     sms_match(sm2code("����"), tag_ptr->SM_code, SM_NO_EXPAND_NE))) {
		    
		    /* �֡��פʤɤ������ޤ᤿���Τ�Ʊ��ɽ�������ä���� */
		    if (word2_flag) strcpy(word1, word2);
		    
		    /* Ʊ��ɽ���Ǥ���� */
		    if (j == 0) {
			sprintf(buf, "C��;��%s%s��;=;0;%d;9.99:%s(Ʊ��ʸ):%dʸ��",
				word1, setubi ? setubi : "", k, 
				sp->KNPSID ? sp->KNPSID + 5 : "?", k);
		    }
		    else {
			sprintf(buf, "C��;��%s%s��;=;%d;%d;9.99:%s(%dʸ��):%dʸ��",
				word1, setubi ? setubi : "", j, k, 
				(sdp - j)->KNPSID ? (sdp - j)->KNPSID + 5 : "?", j, k);
		    }
		    assign_cfeature(&((sp->tag_data + i)->f), buf, FALSE);
		    assign_cfeature(&((sp->tag_data + i)->f), "������", FALSE); 
		    sprintf(buf, "�Զ�����:=/O/%s%s/%d/%d/-", word1, setubi ? setubi : "", k, j);
		    assign_cfeature(&((sp->tag_data + i)->f), buf, FALSE);	
		    
		    /* COREFER_ID����Ϳ */   
		    if ((cp = check_feature(tag_ptr->f, "COREFER_ID"))) {
			assign_cfeature(&((sp->tag_data + i)->f), cp, FALSE);
		    }
		    else {
			corefer_id++;
			sprintf(CO, "COREFER_ID:%d", corefer_id);
			assign_cfeature(&((sp->tag_data + i)->f), CO, FALSE);
			assign_cfeature(&(tag_ptr->f), CO, FALSE);
			if (j > 0) {
			    sprintf(CO, "REFERRED:%d-%d", j, k);
			    assign_cfeature(&((sp->tag_data + i)->f), CO, FALSE);
			}
		    }
		    
		    /* ��ͭɽ����corefer�δط��ˤ������ͭɽ���Ȥߤʤ� */
		    if (OptNE) {
			if (!check_feature((sp->tag_data + i)->f, "NE") &&
			    !check_feature((sp->tag_data + i)->f, "NE��") &&
			    !check_feature((sp->tag_data + i)->f, "�;���̾��") &&
			    !check_feature((sp->tag_data + i)->f, "�Լ���̾��") &&
			    (cp = check_feature(tag_ptr->f, "NE")) && !setubi ||
			    yomi_flag && 
			    (cp = check_feature(tag_ptr->f, "NE:PERSON"))) {
			    cp += 3; /* "NE:"���ɤ����Ф� */
			    while (*cp != ':') cp++;
			    if (!strcmp(cp + 1, word1)) {
				ne_corefer(sp, i, anaphor,
					   check_feature(tag_ptr->f, "NE"), yomi_flag);
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
	 int person_post(SENTENCE_DATA *sp, char *cp, int i)
/*==================================================================*/
{
    /* PERSON + �� ��"="��������Ϳ */

    int j, flag;
    char buf[WORD_LEN_MAX], CO[WORD_LEN_MAX];
    MRPH_DATA *mrph_ptr;
    TAG_DATA *tag_ptr;

    tag_ptr = sp->tag_data + i; /* tag_ptr�ϳμ¤�¸�� */
    mrph_ptr = tag_ptr->mrph_ptr;
    /* ���������ޤ�NE��Ǥ�����Τ��оݤȤ��� */
    if (!check_feature((mrph_ptr - 1)->f, "NE") && /* mrph_ptr - 1 �ϳμ¤�¸�� */
	!(check_feature((mrph_ptr - 2)->f, "NE") && /* ľ�����ܶ��NE�ʤΤǡ�mrph_ptr - 1 ��NE�Ǥʤ��ʤ� mrph_ptr - 2 ��¸�� */
	  (mrph_ptr - 1)->Hinshi == 1 && 
	  (mrph_ptr - 1)->Bunrui == 5)) /* ľ��������Ǥ��� */
	return 0;

    flag = 0;
    for (j = 0; mrph_ptr - sp->mrph_data + j < sp->Mrph_num; j++) {
	if (check_feature((mrph_ptr + j)->f, "��̾����")) {
	    flag = 1;
	    continue;
	}
	else if (check_feature((mrph_ptr + j)->f, "NE") ||
		 check_feature((mrph_ptr + j)->f, "��ͭ����")) {
	    /* ����Ū�ˤϡ��֥å��塦����ꥫ������ */
	    /* ������̱�޴���Ĺ�ʤɤ����ꤷ�Ƥ��� */
	    /* ���󥰥�å���������Ĺ */
	    continue;
	}
	else break;
    }
    if (!flag) return 0;
	
    /* ʣ���Υ����ˤޤ����äƤ�����ϼ��Υ����˿ʤ� */
    while (j > tag_ptr->mrph_num) {
	j -= tag_ptr->mrph_num;
	tag_ptr++;
    }
    
    sprintf(buf, "C��;��%s��;=;0;%d;9.99:%s(Ʊ��ʸ):%dʸ��",
	    cp, i - 1, sp->KNPSID ? sp->KNPSID + 5 : "?", i - 1); 
    assign_cfeature(&(tag_ptr->f), buf, FALSE);
    assign_cfeature(&(tag_ptr->f), "������(��)", FALSE);
    sprintf(buf, "�Զ�����:=/O/%s/%d/%d/-", cp, i - 1, 0);
    assign_cfeature(&(tag_ptr->f), buf, FALSE);
    
    /* COREFER_ID����Ϳ */
    if (cp = check_feature(tag_ptr->f, "COREFER_ID")) {
	assign_cfeature(&((sp->tag_data + i - 1)->f), cp, FALSE);
    }
    else if (cp = check_feature((sp->tag_data + i - 1)->f, "COREFER_ID")) {
	assign_cfeature(&(tag_ptr->f), cp, FALSE);
    }
    else {
	corefer_id++;
	sprintf(CO, "COREFER_ID:%d", corefer_id);
	assign_cfeature(&(tag_ptr->f), CO, FALSE);
	assign_cfeature(&((sp->tag_data + i - 1)->f), CO, FALSE);
    }
    
    return 1;
}

/*==================================================================*/
	 int recognize_apposition(SENTENCE_DATA *sp, int i)
/*==================================================================*/
{
    /* �֥��ƥ���:�� + "��" + PERSON�פʤɤν���(Ʊ��) */

    int j, k;
    char *cp, buf[WORD_LEN_MAX], CO[WORD_LEN_MAX];
    MRPH_DATA *head_ptr, *mrph_ptr, *tail_ptr;

    /* �����ʳ���i-1���ܤδ��ܶ���������ޤ���"��"��ȼ�äƤ��� */

    /* Ʊ�ʤ�ǧ�������� */
    /* i-1���ܤδ��ܶ�μ缭�����ǤΥ��ƥ��� i���ܤδ��ܶ����Ƭ������     */
    /* ��                                    PERSON (single or head)       */
    /* �ȿ�������                            ORGANIZATION (single or head) */
    /* ���-���ߡ����-���������-����¾     LOCATION (single or head)     */
    /* �͹�ʪ-���ʪ                         ARTIFACT or ̤�θ�            */

    head_ptr = (sp->tag_data + i - 1)->head_ptr; /* i-1���ܤμ缭������ */
    mrph_ptr = (sp->tag_data + i)->mrph_ptr;     /* i���ܤ���Ƭ������ */
    
    /* head_ptr��mrph_ptr�δ֤ϡ��������ޤ���"��"�Τ߲� */
    if (mrph_ptr - head_ptr > 2 ||
	mrph_ptr - head_ptr == 2 && 
	!check_feature((sp->tag_data + i - 1)->f, "����") &&
	strcmp(((sp->tag_data + i)->mrph_ptr - 1)->Goi2, "��")) return 0;

    if (/* NE:PERSON */
	check_category(head_ptr->f, "��") &&
	!check_feature((sp->tag_data + i - 1)->b_ptr->mrph_ptr->f, "NE:PERSON") &&
	(check_feature(mrph_ptr->f, "NE:PERSON:head") || 
	 check_feature(mrph_ptr->f, "NE:PERSON:single")) ||
	
	/* NE:ORGANIZATION */
	check_category(head_ptr->f, "�ȿ�������") &&
	!check_feature((sp->tag_data + i - 1)->b_ptr->mrph_ptr->f, "NE:ORGANIZATION") &&
	(check_feature(mrph_ptr->f, "NE:ORGANIZATION:head") || 
	 check_feature(mrph_ptr->f, "NE:ORGANIZATION:single")) ||
	
	/* NE:LOCATION */
	(check_category(head_ptr->f, "���-����") ||
	 check_category(head_ptr->f, "���-����") ||
	 check_category(head_ptr->f, "���-����¾")) &&
	strcmp(head_ptr->Goi2, "����") &&
	!check_feature((sp->tag_data + i - 1)->b_ptr->mrph_ptr->f, "NE:LOCATION") &&
	(check_feature(mrph_ptr->f, "NE:LOCATION:head") || 
	 check_feature(mrph_ptr->f, "NE:LOCATION:single")) ||

	/* NE:ARTIFACT */
	check_category(head_ptr->f, "�͹�ʪ-���ʪ") &&
	(check_feature(mrph_ptr->f, "NE:ARTIFACT:head") || 
	 check_feature(mrph_ptr->f, "NE:ARTIFACT:single") ||
	 !check_feature(mrph_ptr->f, "NE") && check_feature(mrph_ptr->f, "̤�θ�"))) {	

	/* ��ͭɽ���ν�λ������ܶ�˲��Ϸ�̤���Ϳ */
	j = i;
	if (check_feature(mrph_ptr->f, "NE")) 
	    while (!check_feature((sp->tag_data + j)->f, "NE")) j++;
	
	/* A, B, C�ʤɤΤ褦������¤����θ��Ф��ɻ� */
	/* (�ʲ��ǡ�i���ܤδ��ܶ��ľ���η����Ǥ��������ޤ���"��") */
	/* i-1���ܤδ��ܶ��ޤ�ʸ��ľ���η����Ǥ���i���ܤδ��ܶ��ľ���η����ǤȰ��פ�������Բ� */
	if ((sp->tag_data + i - 1)->b_ptr != sp->bnst_data &&
	    !strcmp(((sp->tag_data + i - 1)->b_ptr->mrph_ptr - 1)->Goi2,
		    ((sp->tag_data + i)->mrph_ptr - 1)->Goi2)) return 0;
	/* ��ͭɽ������ޤ�ʸ��κǸ�η����Ǥ���i���ܤδ��ܶ��ľ���η����ǤȰ��פ�������Բ� */
	/* �������������ޤ���Ͻ��� */
	if (!check_feature((sp->tag_data + j)->b_ptr->f, "����") &&
	    !strcmp(((sp->tag_data + j)->b_ptr->mrph_ptr + 
		     (sp->tag_data + j)->b_ptr->mrph_num - 1)->Goi2,
		    ((sp->tag_data + i)->mrph_ptr - 1)->Goi2)) {
	    return 0;
	}
	/* ��ͭɽ��ľ��η����Ǥ���i���ܤδ��ܶ��ľ���η����ǤȰ��פ�������Բ� */
	tail_ptr = (sp->tag_data + i)->mrph_ptr;
	k = 0;
	while (k < (sp->tag_data + j)->mrph_num &&
	       check_feature(((sp->tag_data + j)->mrph_ptr + k)->f, "NE")) k++;	       
	if ((k != (sp->tag_data + j)->mrph_num) &&
	    !strcmp(((sp->tag_data + j)->mrph_ptr + k)->Goi2,
		    ((sp->tag_data + i)->mrph_ptr - 1)->Goi2)) {
	    return 0;
	}
	
	/* ��ͭɽ����ޤ���ܶ礬�����ȼ�����ޤ��ϡ�ʸ���Ǥ���
	   �ޤ��ϡ�ľ��ˡ�PERSON + ��̾�����פǤ�����ʳ����Բ� */
	if (!check_feature((sp->tag_data + j)->f, "����") &&
	    !check_feature((sp->tag_data + j)->f, "ʸ��") &&
	    !(check_feature((sp->tag_data + j)->f, "NE:PERSON") &&
	      check_feature((sp->tag_data + j + 1)->mrph_ptr->f, "��̾����"))) return 0;
	      	
	sprintf(buf, "C��;��%s��;=;0;%d;9.99:%s(Ʊ��ʸ):%dʸ��",
		head_ptr->Goi2, i - 1, sp->KNPSID ? sp->KNPSID + 5 : "?", i - 1);
	assign_cfeature(&((sp->tag_data + j)->f), buf, FALSE);
	assign_cfeature(&((sp->tag_data + j)->f), "Ʊ��", FALSE);
	sprintf(buf, "�Զ�����:=/O/null/%d/%d/-", i - 1, 0);
	assign_cfeature(&((sp->tag_data + i)->f), buf, FALSE);	

	/* COREFER_ID����Ϳ */
	if (cp = check_feature((sp->tag_data + j)->f, "COREFER_ID")) {
	    assign_cfeature(&((sp->tag_data + i - 1)->f), cp, FALSE);
	}
	else if (cp = check_feature((sp->tag_data + i - 1)->f, "COREFER_ID")) {
	    assign_cfeature(&((sp->tag_data + j)->f), cp, FALSE);
	}
	else {
	    corefer_id++;
	    sprintf(CO, "COREFER_ID:%d", corefer_id);
	    assign_cfeature(&((sp->tag_data + j)->f), CO, FALSE);
	    assign_cfeature(&((sp->tag_data + i - 1)->f), CO, FALSE);
	}

	/* ����̾��¦�ˡ־�ά���Ϥʤ��פ���Ϳ����Ƥ�����Ͻ���� */
	if (check_feature((sp->tag_data + i - 1)->f, "��ά���Ϥʤ�")) {
	    delete_cfeature(&((sp->tag_data + i - 1)->f), "��ά���Ϥʤ�");
	}

	return 1;
    }
    return 0;
}

/*==================================================================*/
	       void corefer_analysis(SENTENCE_DATA *sp)
/*==================================================================*/
{
    int i, person;
    char *anaphor, *cp, *ne;
    MRPH_DATA *mrph_ptr;
    sp = sentence_data + sp->Sen_num - 1;

    for (i = sp->Tag_num - 1; i >= 0 ; i--) { /* ����ʸ�Υ���ñ��:i���ܤΥ����ˤĤ��� */

	/* �����Ȳ��Ϥ�Ԥ���� */
	/* �ȱ������Ǥ��ꡢ��ͭɽ����θ졢�ޤ��� */
	/* Ϣ�λ���ֻؼ���ʳ��˽�������Ƥ��ʤ��� */
	if ((anaphor = check_feature((sp->tag_data + i)->f, "�ȱ������")) &&
	    (check_feature((sp->tag_data + i)->f, "NE") ||  
	     check_feature((sp->tag_data + i)->f, "NE��") || /* DATA��LOCATION�ʤɰ��� */
	     check_feature((sp->tag_data + i)->f, "�ɤ���") ||
	     !get_modify_num(sp->tag_data + i) || /* ��������Ƥ��ʤ� */
	     (((sp->tag_data + i)->mrph_ptr - 1)->Hinshi == 1 && 
	      ((sp->tag_data + i)->mrph_ptr - 1)->Bunrui == 2) || /* ľ���������Ǥ��� */
	     check_feature(((sp->tag_data + i)->b_ptr->child[0])->f, "Ϣ�λ���ֻؼ���") ||
	     check_feature(((sp->tag_data + i)->b_ptr->child[0])->f, "�ȱ���Ƭ��"))) {
	    
	    /* �ؼ���ξ�� */
	    if (check_feature((sp->tag_data + i)->f, "�ؼ���")) {
		continue; /* �����ǤϽ����򤷤ʤ� */
	    }

	    /* ���ܶ礬��ͭɽ����ޤޤ������ġ����ܶ�缭�θ�˷����Ǥ������硢�����mrph_ptr�Ȥ��� */
	    mrph_ptr = NULL;
	    ne = check_feature((sp->tag_data + i)->f, "NE");
	    if (!ne && ((sp->tag_data + i)->mrph_ptr + (sp->tag_data + i)->mrph_num) - (sp->tag_data + i)->head_ptr > 1)
		mrph_ptr = (sp->tag_data + i)->head_ptr + 1;		
	    
	    /* ��Ԥ���ɽ���ȶ����ȴط��ˤ��뤫������å� */
	    if (mrph_ptr && /* ̾�������������դ�����Ϥޤ���������ޤ᤿��Τ�Ĵ�٤� */
		mrph_ptr->Hinshi == 14 && mrph_ptr->Bunrui < 5 &&
		search_antecedent(sp, i, anaphor + strlen("�ȱ������") + 1, mrph_ptr->Goi2, NULL) ||
		search_antecedent(sp, i, anaphor + strlen("�ȱ������") + 1, NULL, ne)) { /* ���̤ξ�� */

		/* ���Ǥ˸��Ĥ��ä������ȴط��˴ޤޤ��ط��ϲ��Ϥ��ʤ� */
		/* e.g. �ȱ��줬�ֹ�Ω��ءפʤ�ֹ�Ω�פϾȱ���Ȥ��ƹ�θ���ʤ� */
		if (!strcmp(anaphor + strlen("�ȱ������") + 1, (sp->tag_data + i)->mrph_ptr->Goi2)) continue; /* �������Ǥ���������Ϲ�θ���� */
		while (i > 0) {
		    if ((cp = check_feature((sp->tag_data + i - 1)->f, "�ȱ������")) &&
			!strncmp(cp, anaphor, strlen(cp))) {
			i--;
			assign_cfeature(&((sp->tag_data + i)->f), "��������", FALSE);
			if (!strcmp(cp + strlen("�ȱ������") + 1, (sp->tag_data + i)->mrph_ptr->Goi2)) break; /* �������Ǥ�������ʤ餽���ޤ� */
		    }
		    else break;
		}
		continue;
	    }		
	}

	/* PERSON + ��̾���� �ν���(������(��)) */
	if (i > 0 && (cp = check_feature((sp->tag_data + i - 1)->f, "NE:PERSON"))) {
	    person_post(sp, cp + 10, i);
	}

	/* �֥��ƥ���:�� + "��" + PERSON�פʤɤν���(Ʊ��) */
	if (i > 0 && 
	    !check_feature((sp->tag_data + i - 1)->f, "NE")) {
	    recognize_apposition(sp, i);
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
		while (*cp != '/') cp++;
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
		cp += strlen("�ȱ������") + 1;
		
		if (j == 0) {
		    sprintf(buf, "C��;��%s��;=;0;%d;9.99:%s(Ʊ��ʸ):%dʸ��",
			    cp, k, sp->KNPSID ? sp->KNPSID + 5 : "?", k);
		}
		else {
		    sprintf(buf, "C��;��%s��;=;%d;%d;9.99:%s(%dʸ��):%dʸ��",
			    cp, j, k, 
			    (sdp - j)->KNPSID ? (sdp - j)->KNPSID + 5 : "?", j, k);
		}
		assign_cfeature(&((sp->tag_data + i)->f), buf, FALSE);
		assign_cfeature(&((sp->tag_data + i)->f), "������", FALSE); 
		sprintf(buf, "�Զ�����:=/O/%s/%d/%d/-", cp, k, j);
		assign_cfeature(&((sp->tag_data + i)->f), buf, FALSE);	
		
		/* COREFER_ID����Ϳ */   
		if ((cp = check_feature(tag_ptr->f, "COREFER_ID"))) {
		    assign_cfeature(&((sp->tag_data + i)->f), cp, FALSE);
		}
		else if ((cp = check_feature((sp->tag_data + i)->f, "COREFER_ID"))) {
		    assign_cfeature(&(tag_ptr->f), cp, FALSE);
		}
		else {
		    corefer_id++;
		    sprintf(CO, "COREFER_ID:%d", corefer_id);
		    assign_cfeature(&((sp->tag_data + i)->f), CO, FALSE);
		    assign_cfeature(&(tag_ptr->f), CO, FALSE);
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
		while (*cp != '/') cp++;
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
