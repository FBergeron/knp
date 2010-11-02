/*====================================================================

			     ��ư��������

                                         Daisuke Kawahara 2007. 3. 13

    $Id$
====================================================================*/
#include "knp.h"

DBM_FILE auto_dic_db;
int AutoDicExist;

char *used_auto_dic_features[AUTO_DIC_FEATURES_MAX];
int used_auto_dic_features_num = 0;

/*==================================================================*/
			 void init_auto_dic()
/*==================================================================*/
{
    char *filename;

    if (DICT[AUTO_DIC_DB]) {
	filename = check_dict_filename(DICT[AUTO_DIC_DB], TRUE);
    }
    else {
	filename = check_dict_filename(AUTO_DIC_DB_NAME, FALSE);
    }

    if (OptDisplay == OPT_DEBUG) {
	fprintf(Outfp, "Opening %s ... ", filename);
    }

    if ((auto_dic_db = DB_open(filename, O_RDONLY, 0)) == NULL) {
	if (OptDisplay == OPT_DEBUG) {
	    fputs("failed.\n", Outfp);
	}
	AutoDicExist = FALSE;
#ifdef DEBUG
	fprintf(stderr, ";; Cannot open AUTO dictionary <%s>.\n", filename);
#endif
    }
    else {
	if (OptDisplay == OPT_DEBUG) {
	    fputs("done.\n", Outfp);
	}
	AutoDicExist = TRUE;
    }
    free(filename);
}

/*==================================================================*/
			void close_auto_dic()
/*==================================================================*/
{
    if (AutoDicExist == TRUE) {
	DB_close(auto_dic_db);
    }
}

/*==================================================================*/
		   char *lookup_auto_dic(char *str)
/*==================================================================*/
{
    return db_get(auto_dic_db, str);
}

/*==================================================================*/
int check_auto_dic(MRPH_DATA *m_ptr, int assign_pos, int m_length, char *rule_value, int temp_assign_flag)
/*==================================================================*/
{
    int i, flag, length;
    char *ret, *dic_str, *rep_str, key[DATA_LEN];

    if (AutoDicExist == FALSE) {
	return FALSE;
    }

    if (used_auto_dic_features_num > 0) { /* ���Ѥ��뼫ư����°�������ꤵ��Ƥ���Ȥ� */
	flag = FALSE;
	for (i = 0; i < used_auto_dic_features_num; i++) {
	    if (!strcmp(used_auto_dic_features[i], rule_value)) {
		flag = TRUE;
		break;
	    }
	}
	if (flag == FALSE) { /* �ޥå����ʤ��ä� */
	    return FALSE;
	}
    }

    /* ��¦��ҤȤĤ���û�����Ƥ��� */
    for (; m_length > 0; m_length--) {
	key[0] = '\0';
	for (i = 0; i < m_length; i++) { /* �������󤫤饭������ */
	    if (i) strcat(key, "+");
	    if (rep_str = get_mrph_rep_from_f(m_ptr + i, FALSE)) {
		if (strlen(key) + strlen(rep_str) + 2 > DATA_LEN) {
		    return FALSE;
		}
		strcat(key, rep_str);
	    }
	    else { /* ���졢��ư��ʤɤ���ɽɽ�����ʤ� */
		strcat(key, (m_ptr + i)->Goi2); /* ɽ�� */
	    }
	}

	dic_str = lookup_auto_dic(key);

	if (dic_str) {
	    int cmp_length = strlen(rule_value); /* strncmp��Ĺ�� */
	    char *token = strtok(dic_str, "|"); /* ������ܤ���ڤ� (dict/auto/Makefile.am�ǻ���) */
	    ret = NULL;
	    while (token) {
		if (!strncmp(token, rule_value, cmp_length)) { /* ������ܤȥ롼�뤫��Ϳ����줿ʸ���󤬥ޥå� */
		    ret = (char *)malloc_data(strlen(token) + 9, "check_auto_dic");
		    sprintf(ret, "%s:%d-%d", token, m_ptr->num, (m_ptr + m_length - 1)->num);
		    break;
		}
		token = strtok(NULL, "|");
	    }

	    free(dic_str);
	    if (ret) { /* �ޥå�����С�feature��assign_pos�η����Ǥ���Ϳ���ƽ�λ */
		assign_cfeature(&((m_ptr + assign_pos)->f), ret, temp_assign_flag);
		free(ret);
		return TRUE;
	    }
	}
	if (assign_pos) { /* ������feature����Ϳ������ϡ�������ˤ��餹 */
	    assign_pos--;
	}
	/* ����ˤʤ���С����Υ롼�פ����ꡢ��¦��û������ */
    }

    return FALSE;
}

/*====================================================================
                               END
====================================================================*/
