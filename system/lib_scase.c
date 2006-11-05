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
		   char *mrph2case(BNST_DATA *bp)
/*==================================================================*/
{
    int i;

    for (i = bp->mrph_num - 1; i >= 0 ; i--) {
	if (check_feature((bp->mrph_ptr + i)->f, "��°")) {
	    if (!strcmp(Class[(bp->mrph_ptr + i)->Hinshi][0].id, "����") && 
		!strcmp(Class[(bp->mrph_ptr + i)->Hinshi][(bp->mrph_ptr + i)->Bunrui].id, "�ʽ���")) {
		return (bp->mrph_ptr + i)->Goi;
	    }
	}
	else {
	    return NULL;
	}
    }

    return NULL;
}

/*==================================================================*/
	   char *make_pred_string_for_scase(BNST_DATA *bp)
/*==================================================================*/
{
    char *buffer, *pp = NULL, *verb;
    BNST_DATA *cbp;

    verb = make_pred_string((TAG_DATA *)bp, NULL, NULL, FALSE); /* OptCaseFlag & OPT_CASE_USE_REP_CF */

    /* cbp = get_quasi_closest_case_component((TAG_DATA *)bp, 
       bp->num < 1 ? NULL : (TAG_DATA *)(bp - 1)); */

    if (bp->num > 0) {
	cbp = bp - 1;
	pp = mrph2case(cbp);

	if (pp) {
	    char *pp_katakana = hiragana2katakana(pp);

	    buffer = (char *)malloc_data(strlen(cbp->head_ptr->Goi) + strlen(pp_katakana) + strlen(verb) + 10, 
					 "make_pred_string_for_scase");
	    sprintf(buffer, "%s:%s-%s", cbp->head_ptr->Goi, pp_katakana, verb);
	    free(verb);
	    free(pp_katakana);
	    return buffer;
	}
	else {
	    return verb;
	}
    }
    else {
	return verb;
    }
}

/*==================================================================*/
	      void or_scase_code(char **dst, char *src)
/*==================================================================*/
{
    if (*dst == NULL) {
	*dst = src;
    }
    else if (src) {
	int i;

	for (i = 0; *(*dst + i) != '\0'; i++) {
	    *(*dst + i) |= *(src + i);
	}

	free(src);
    }
    /* *dst�����ä�src���ʤ��Ȥ��Ϥʤˤ⤷�ʤ� */
}

/*==================================================================*/
		 void get_scase_code(BNST_DATA *ptr)
/*==================================================================*/
{
    int i;
    char *cp, *ans = NULL, *anscp, *str_buffer = NULL, *vtype, voice[3];

    /* �����: init_bnst �Ǥ⤷�Ƥ��� */
    for (i = 0, cp = ptr->SCASE_code; i < SCASE_CODE_SIZE; i++, cp++) *cp = 0;

    if (ScaseDicExist == TRUE && 
	(vtype = check_feature(ptr->f, "�Ѹ�")) && 
	strcmp(vtype, "�Ѹ�:Ƚ")) { /* Ƚ���ǤϤʤ���� */
	vtype += strlen("�Ѹ�:");

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

	/* �ޤ���ľ�������ǤȤ��ȤǸ��� *
	str_buffer = make_pred_string_for_scase(ptr);
	strcat(str_buffer, ":");
	strcat(str_buffer, vtype);
	if (voice[0]) strcat(str_buffer, voice);

	ans = get_scase(str_buffer);
	*/

	if (ans == NULL) { /* �ʤ���С��Ѹ������Ǹ��� */
	    if (str_buffer) {
		free(str_buffer);
	    }
	    str_buffer = make_pred_string((TAG_DATA *)ptr, NULL, NULL, FALSE); /* OptCaseFlag & OPT_CASE_USE_REP_CF */
	    strcat(str_buffer, ":");
	    strcat(str_buffer, vtype);
	    if (voice[0]) strcat(str_buffer, voice);

	    ans = get_scase(str_buffer);

	    /* ��ɽɽ����ۣ����Ѹ��ξ�� => scase.dat����ɽɽ�������켡����� */
	    if (0 && check_feature(ptr->head_ptr->f, "����ۣ��")) {
		FEATURE *fp;
		MRPH_DATA m;
		char *str;
		char *new_ans;

		fp = ptr->head_ptr->f;
		while (fp) {
		    if (!strncmp(fp->cp, "ALT-", 4)) {
			sscanf(fp->cp + 4, "%[^-]-%[^-]-%[^-]-%d-%d-%d-%d-%[^\n]", 
			       m.Goi2, m.Yomi, m.Goi, 
			       &m.Hinshi, &m.Bunrui, 
			       &m.Katuyou_Kata, &m.Katuyou_Kei, m.Imi);
			free(str_buffer);
			str_buffer = make_pred_string((TAG_DATA *)ptr, &m, NULL, FALSE); /* OptCaseFlag & OPT_CASE_USE_REP_CF */
			strcat(str_buffer, ":");
			strcat(str_buffer, vtype);
			if (voice[0]) strcat(str_buffer, voice);

			new_ans = get_scase(str_buffer);
			or_scase_code(&ans, new_ans);
		    }
		    fp = fp->next;
		}
	    }
	}

	if (ans != NULL) {
	    /* DEBUG ɽ�� */
	    if (OptDisplay == OPT_DEBUG) {
		char *print_buffer;

		print_buffer = (char *)malloc_data(strlen(str_buffer) + 10, "get_scase_code");
		sprintf(print_buffer, "SCASEUSE:%s", str_buffer);
		assign_cfeature(&(ptr->f), print_buffer, FALSE);
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

/*====================================================================
                               END
====================================================================*/
