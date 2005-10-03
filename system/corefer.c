/*====================================================================

			     �����Ȳ���

                                               R.SASANO 05. 9.24

    $Id$
====================================================================*/
#include "knp.h"

/*==================================================================*/
	       void assign_anaphor_feature(SENTENCE_DATA *sp)
/*==================================================================*/
{
    /* �ȱ������Ȥ���feature����Ϳ���� */
    /* �ȱ������Ȥ�ʣ��̾��������ȤʤäƤ����Τ��� */

    /* ����ʣ��̾��δ�� */
    /* 	��ͭɽ�� */
    /* 	�����θ줬����̾��Ǥ�����ڤ� */
    /* 	�����θ�Ȥ������θ줬̾��ʥե졼��ˤ����ȹ礻�ξ����ڤ� */
    /* 	�����θ�Τ��ʲ��θ�Ǥ�����ڤ� */
    /* 		���� ���� ���� */

    int i, j, k, tag_num;
    char word[64], buf[64], *cp;
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

 	    if (/* �缭�Ǥ��� */
		j == tag_num - 1 ||
		
		/* ľ���̾�줬����̾��Ǥ��� */
		((tag_ptr + j + 1)->mrph_ptr)->Hinshi == 6 &&
		((tag_ptr + j + 1)->mrph_ptr)->Bunrui == 2 ||

		/* ľ���̾��γʥե졼��������¸�ߤ��� */
		(tag_ptr + j + 1)->cf_ptr &&
		check_examples(((tag_ptr + j)->mrph_ptr)->Goi2,
				((tag_ptr + j + 1)->cf_ptr)->ex_list[0],
				((tag_ptr + j + 1)->cf_ptr)->ex_num[0]) >= 0) {

		memset(word, 0, sizeof(char)*64);
		for (k = (tag_ptr + j)->head_ptr - (sp->bnst_data + i)->mrph_ptr; 
		     k >= 0; k--) {

		    /* ̾����Ƭ���ϴޤ�ʤ� */
		    if (((tag_ptr + j)->head_ptr - k)->Hinshi == 13 &&
			((tag_ptr + j)->head_ptr - k)->Bunrui == 1) break;

		    if (!(tag_ptr + j)->anaphor_mrph_num) {
			(tag_ptr + j)->anaphor_mrph_num = k + 1;
		    }

		    strcat(word, ((tag_ptr + j)->head_ptr - k)->Goi2);
		}
		sprintf(buf, "�ȱ������:%s", word);
		assign_cfeature(&((tag_ptr + j)->f), buf);
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
		(tag_ptr + j)->anaphor_mrph_num = (tag_ptr + j)->proper_mrph_num;
		continue;
	    }
	}   
    }
}

/*==================================================================*/
               int search_antecedent(SENTENCE_DATA *sp, int i, 
				     char *anaphor, char *setubi)
/*==================================================================*/
{
    /* �Ѹ��Ȥ��ξ�ά�ʤ�Ϳ������ */

    /* cf_ptr = cpm_ptr->cmm[0].cf_ptr �Ǥ��� */
    /* �Ѹ� cpm_ptr �� cf_ptr->pp[n][0] �ʤ���ά����Ƥ���
       cf_ptr->ex[n] �˻��Ƥ���ʸ���õ�� */

    int j, k, l, anaphor_mrph_num;
    char word[64], buf[64];
    SENTENCE_DATA *sdp;
    TAG_DATA *tag_ptr;
 
    sdp = sentence_data + sp->Sen_num - 1;
    for (j = 0; j <= sdp - sentence_data; j++) { /* �ȱ��褬��ʸ���� */
	
	for (k = j ? (sdp - j)->Tag_num - 1 : i - 1;
	     k >= (sp->tag_data + i)->anaphor_mrph_num - 1; k--) { /* �ȱ���Υ��� */
	    
	    tag_ptr = (sdp - j)->tag_data + k;	    		
	    if (!check_feature((tag_ptr)->f, "�θ�")) continue;

	    memset(word, 0, sizeof(char)*52);
	    anaphor_mrph_num = (sp->tag_data + i)->anaphor_mrph_num;

	    /* ��³��̾��������������Ϳ */
	    if (setubi && strcmp((tag_ptr->head_ptr + 1)->Goi2, setubi))
		continue;
		
	    for (l = anaphor_mrph_num - 1; l >= 0; l--) 
		strcat(word, (tag_ptr->head_ptr - l)->Goi2); /* ��Ի���� */
	    
	    if (!strcmp(anaphor, word)) { /* Ʊɽ���θ����Ի�Ȥߤʤ� */
		if (j == 0) {
		    sprintf(buf, "C��;��%s%s��;=;0;%d;9.99:%s(Ʊ��ʸ):%dʸ��",
			    word, setubi ? setubi : "", k, sp->KNPSID ? sp->KNPSID + 5 : "?", k);
		}
		else {
		    sprintf(buf, "C��;��%s%s��;=;%d;%d;9.99:%s(%dʸ��):%dʸ��",
			    word, setubi ? setubi : "", j, k, sp->KNPSID ? sp->KNPSID + 5 : "?", j, k);
		}
		assign_cfeature(&((sp->tag_data + i)->f), buf);

		/* ʣ���Υ����˴ط�������ν��� */
		for (l = 0; l < anaphor_mrph_num; i--) 
		    l += (sp->tag_data + i)->mrph_num;
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
    
    for (i = sp->Tag_num - 1; i >= 0; i--) { /* ����ʸ�Υ���ñ�� */

	if (!(anaphor = check_feature((sp->tag_data + i)->f, "�ȱ������")) ||
	    !check_feature((sp->tag_data + i)->f, "NE") &&
	    ((sp->tag_data + i)->b_ptr)->child[0]) /* ��������Ƥ��� */
	    continue;

	if (check_feature((sp->tag_data + i)->f, "�ؼ���")) {
	    /* �ؼ���ξ�� */
	    continue;
	}
	
	mrph_ptr = (sp->tag_data + i)->head_ptr + 1; /* �缭�μ��η����� */
	if (mrph_ptr->Hinshi == 14 && mrph_ptr->Bunrui < 5) {
	    /* ̾�������������դ��Ƥ����� */
	    next_i = search_antecedent(sp, i, anaphor+11, mrph_ptr->Goi2);
	    if (next_i != -2) {
		i = next_i;
		continue;
	    }
	}
	next_i = search_antecedent(sp, i, anaphor+11, NULL);
	i = (next_i == -2) ? i : next_i;
    }
}

/*====================================================================
                               END
====================================================================*/
