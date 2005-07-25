/*====================================================================

			      ɽ�سʾ���

                                               S.Kurohashi 92.10.21
                                               S.Kurohashi 93. 5.31

    $Id$
====================================================================*/
#include "knp.h"

DBM_FILE	scase_db;
int		ScaseDicExist;
int		OptUseScase;

/*==================================================================*/
			  void init_scase()
/*==================================================================*/
{
    char *filename;

    if (OptUseScase == FALSE) {
	ScaseDicExist = FALSE;
	return;
    }

    if (DICT[SCASE_DB]) {
	filename = check_dict_filename(DICT[SCASE_DB], FALSE);
    }
    else {
	filename = check_dict_filename(SCASE_DB_NAME, FALSE);
    }

    if (OptDisplay == OPT_DEBUG) {
	fprintf(Outfp, "Opening %s ... ", filename);
    }

    if ((scase_db = DB_open(filename, O_RDONLY, 0)) == NULL) {
	if (OptDisplay == OPT_DEBUG) {
	    fputs("failed.\n", Outfp);
	}
	ScaseDicExist = FALSE;
#ifdef DEBUG
	fprintf(stderr, ";; Cannot open SCASE dictionary <%s>.\n", filename);
#endif
    } else {
	if (OptDisplay == OPT_DEBUG) {
	    fputs("done.\n", Outfp);
	}
	ScaseDicExist = TRUE;
    }
    free(filename);
}

/*==================================================================*/
                    void close_scase()
/*==================================================================*/
{
    if (ScaseDicExist == TRUE)
      DB_close(scase_db);
}

/*==================================================================*/
                    char *get_scase(char *cp)
/*==================================================================*/
{
    int i;
    char *value;

    if (ScaseDicExist == FALSE)
	return NULL;

    value = db_get(scase_db, cp);
    
    if (value) {
	for (i = 0; *(value+i) != '\0'; i++)
	  *(value+i) -= '0';
	return value;
    }
    else {
	return NULL;
    }
}

/*==================================================================*/
		 void get_scase_code(BNST_DATA *ptr)
/*==================================================================*/
{
    int i;
    char *cp, *ans, *anscp, *str_buffer, *vtype, voice[3];

    /* �����: init_bnst �Ǥ⤷�Ƥ��� */
    for (i = 0, cp = ptr->SCASE_code; i < SCASE_CODE_SIZE; i++, cp++) *cp = 0;

    if (ScaseDicExist == TRUE && 
	(vtype = check_feature(ptr->f, "�Ѹ�")) && 
	strcmp(vtype, "�Ѹ�:Ƚ")) { /* Ƚ���ǤϤʤ���� */
	vtype += 5;

	voice[0] = '\0';
	if (ptr->voice & VOICE_UKEMI) {
	    strcpy(voice, ":P");
	}
	else if (ptr->voice & VOICE_SHIEKI) {
	    strcpy(voice, ":C");
	}
	else if (ptr->voice & VOICE_SHIEKI_UKEMI) {
	    strcpy(voice, ":PC");
	}

	if (ptr->type == IS_BNST_DATA) {
	    str_buffer = make_pred_string(ptr->head_tag_ptr); /* �Ǹ�Υ���ñ�� (�֡��Τϡפξ���1����) */
	}
	else {
	    str_buffer = make_pred_string((TAG_DATA *)ptr);
	}
	strcat(str_buffer, ":");
	strcat(str_buffer, vtype);
	if (voice[0]) strcat(str_buffer, voice);

	ans = get_scase(str_buffer);

	if (ans != NULL) {
	    /* DEBUG ɽ�� */
	    if (OptDisplay == OPT_DEBUG) {
		char *print_buffer;

		print_buffer = (char *)malloc_data(strlen(str_buffer) + 10, "get_scase_code");
		sprintf(print_buffer, "SCASEUSE:%s", str_buffer);
		assign_cfeature(&(ptr->f), print_buffer);
		free(print_buffer);
	    }

	    cp = ptr->SCASE_code;
	    anscp = ans;
	    for (i = 0; i < SCASE_CODE_SIZE; i++) *cp++ = *anscp++;
	    free(ans);
	    free(str_buffer);
	    goto Match;
	}
	else {
	    free(str_buffer);
	}
    }

    /* Ƚ���ʤɤξ��,
       ɽ�سʼ��񤬤ʤ����, 
       �ޤ��ϼ���ˤʤ��Ѹ��ξ�� */
    
    if (check_feature(ptr->f, "�Ѹ�:Ƚ")) {
	ptr->SCASE_code[case2num("����")] = 1;
    } 
    else if (check_feature(ptr->f, "�Ѹ�:��")) {
	ptr->SCASE_code[case2num("����")] = 1;
	ptr->SCASE_code[case2num("�˳�")] = 1;
	/* ���ƻ��ɽ�سʤ���Ϳ�������Ѥ�¿���Τ�����
	ptr->SCASE_code[case2num("����")] = 1;
	ptr->SCASE_code[case2num("�ȳ�")] = 1;
	*/
    } 
    else if (check_feature(ptr->f, "�Ѹ�:ư")) {
	ptr->SCASE_code[case2num("����")] = 1;
	ptr->SCASE_code[case2num("���")] = 1;
	ptr->SCASE_code[case2num("�˳�")] = 1;
	ptr->SCASE_code[case2num("�س�")] = 1;
	ptr->SCASE_code[case2num("�ȳ�")] = 1;
    }

  Match:

    /* ���������ˤ�뽤�� */

    if (ptr->voice & VOICE_SHIEKI) {
	ptr->SCASE_code[case2num("���")] = 1;
	ptr->SCASE_code[case2num("�˳�")] = 1;
    }
    else if (ptr->voice & VOICE_UKEMI || 
	     ptr->voice & VOICE_SHIEKI_UKEMI) {
	ptr->SCASE_code[case2num("�˳�")] = 1;
    }
    else if (ptr->voice & VOICE_MORAU || 
	     ptr->voice & VOICE_HOSHII) {
	ptr->SCASE_code[case2num("���")] = 1;
	ptr->SCASE_code[case2num("�˳�")] = 1;
    }
}

/*==================================================================*/
		 void set_pred_voice(BNST_DATA *ptr)
/*==================================================================*/
{
    /* �������������� */

    char *cp;

    ptr->voice = 0;

    if (cp = check_feature(ptr->f, "��")) {
	char *token, *str;

	str = strdup(cp + 3);
	token = strtok(str, "|");
	while (token) {
	    if (!strcmp(token, "��ư")) {
		ptr->voice |= VOICE_UKEMI;
	    }
	    else if (!strcmp(token, "����")) {
		ptr->voice |= VOICE_SHIEKI;
	    }
	    else if (!strcmp(token, "��餦")) {
		ptr->voice |= VOICE_MORAU;
	    }
	    else if (!strcmp(token, "�ۤ���")) {
		ptr->voice |= VOICE_HOSHII;
	    }
	    else if (!strcmp(token, "����&��ư")) {
		ptr->voice |= VOICE_SHIEKI_UKEMI;
	    }
	    /* �ֲ�ǽ�פ�̤���� */

	    token = strtok(NULL, "|");
	}
	free(str);
    }
}

/*==================================================================*/
	       char *make_pred_string(TAG_DATA *t_ptr)
/*==================================================================*/
{
    char *buffer;

    /* �Ѹ�������, voice��ʬ(7)����ݤ��Ƥ��� */

    /* �֡ʡ���ˡ��ˡ� �ΤȤ��� �֤���� ��õ�� */
    if (check_feature(t_ptr->f, "ID:�ʡ���ˡ���")) {
	buffer = (char *)malloc_data(12, "make_pred_string"); /* 4 + 8 */
	strcpy(buffer, "����");
    }
    /* �ַ��ƻ�+�ʤ�פʤ� */
    else if (check_feature(t_ptr->f, "���Ѹ����Т�")) {
	buffer = (char *)malloc_data(strlen(t_ptr->head_ptr->Goi2) + strlen((t_ptr->head_ptr + 1)->Goi) + 8, 
				     "make_pred_string");
	strcpy(buffer, t_ptr->head_ptr->Goi2);
	strcat(buffer, (t_ptr->head_ptr + 1)->Goi);
    }
    /* �ַ��ƻ�촴+Ū���פʤ� */
    else if (check_feature(t_ptr->f, "���Ѹ����Т�")) {
	buffer = (char *)malloc_data(strlen((t_ptr->head_ptr - 1)->Goi2) + strlen(t_ptr->head_ptr->Goi) + 8, 
				     "make_pred_string");
	strcpy(buffer, (t_ptr->head_ptr - 1)->Goi2);
	strcat(buffer, t_ptr->head_ptr->Goi);
    }
    else {
	buffer = (char *)malloc_data(strlen(t_ptr->head_ptr->Goi) + 8, "make_pred_string");
	strcpy(buffer, t_ptr->head_ptr->Goi);
    }

    return buffer;
}

/*====================================================================
                               END
====================================================================*/
