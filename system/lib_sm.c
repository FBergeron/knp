/*====================================================================

		      ��̣��  �����ץ����

                                               S.Kurohashi 91. 6.25
                                               S.Kurohashi 93. 5.31

    $Id$
====================================================================*/
#include "knp.h"

DBM_FILE	sm_db;
DBM_FILE	sm2code_db;
DBM_FILE	smp2smg_db;
int		SMExist;
int		SM2CODEExist;
int		SMP2SMGExist;

/*==================================================================*/
			    void init_sm()
/*==================================================================*/
{
    char *filename;

    /* ñ�� <=> ��̣�ǥ����� */
    if (DICT[SM_DB]) {
	filename = (char *)check_dict_filename(DICT[SM_DB]);
    }
    else {
	filename = strdup(SM_DB_NAME);
    }

    if ((sm_db = DBM_open(filename, O_RDONLY, 0)) == NULL) {
	SMExist = FALSE;
    }
    else {
	SMExist = TRUE;
    }
    free(filename);

    /* ��̣�� <=> ��̣�ǥ����� */
    if (DICT[SM2CODE_DB]) {
	filename = (char *)check_dict_filename(DICT[SM2CODE_DB]);
    }
    else {
	filename = strdup(SM2CODE_DB_NAME);
    }

    if ((sm2code_db = DBM_open(filename, O_RDONLY, 0)) == NULL) {
	SM2CODEExist = FALSE;
    }
    else {
	SM2CODEExist = TRUE;
    }
    free(filename);

    /* ��ͭ̾���η� <=> ����̾���η� */
    if (DICT[SMP2SMG_DB]) {
	filename = (char *)check_dict_filename(DICT[SMP2SMG_DB]);
    }
    else {
	filename = strdup(SMP2SMG_DB_NAME);
    }

    if ((smp2smg_db = DBM_open(filename, O_RDONLY, 0)) == NULL) {
	SMP2SMGExist = FALSE;
    }
    else {
	SMP2SMGExist = TRUE;
    }
    free(filename);
}

/*==================================================================*/
			   void close_sm()
/*==================================================================*/
{
    if (SMExist == TRUE)
	DBM_close(sm_db);

    if (SM2CODEExist == TRUE)
	DBM_close(sm2code_db);

    if (SMP2SMGExist == TRUE)
	DBM_close(smp2smg_db);
}

/*==================================================================*/
			char *get_sm(char *cp)
/*==================================================================*/
{
    int i, pos, length;
    char *code;

    if (SMExist == FALSE) {
	fprintf(stderr, "Can not open Database <%s>.\n", SM_DB_NAME);
	exit(1);
    }

    code = db_get(sm_db, cp);

    if (code) {
	length = strlen(code);
	/* ��줿�顢�̤�� (�����о�) */
	if (length > SM_CODE_SIZE*SM_ELEMENT_MAX) {
#ifdef DEBUG
	    fprintf(stderr, "Too long SM content <%s>.\n", code);
#endif
	    code[SM_CODE_SIZE*SM_ELEMENT_MAX] = '\0';
	}

	pos = 0;
	/* ��̣�Ǥ���Ϳ�����ʻ� */
	for (i = 0; code[i]; i+=SM_CODE_SIZE) {
	    if (code[i] == '3' ||	/* ̾ */
		code[i] == '4' ||	/* ̾(����) */
		code[i] == '5' ||	/* ̾(��ư) */
		code[i] == '6' ||	/* ̾(ž��) */
		code[i] == '7' ||	/* ���� */
		code[i] == '9' ||	/* ���� */
		code[i] == 'a' ||	/* ��̾ */
		code[i] == 'l' ||	/* ��Ƭ */
		code[i] == 'm' ||	/* �ܼ� */
		code[i] == '2') {	/* �� */
		strncpy(code+pos, code+i, SM_CODE_SIZE);
		pos += SM_CODE_SIZE;
	    }
	}
	code[pos] = '\0';
    }
    return code;
}

/*==================================================================*/
             void get_sm_code(BNST_DATA *ptr)
/*==================================================================*/
{
    int strt, end, last, stop, i, overflow_flag = 0;
    char str_buffer[BNST_LENGTH_MAX], *code;
    char feature_buffer[SM_CODE_SIZE*SM_ELEMENT_MAX+1];

    /* ����� */
    *(ptr->SM_code) = '\0';

    if (SMExist == FALSE) return;

    /* 
       ʣ���ΰ���
       		�ޤ���°�����ꡤ��Ω��򸺤餷�Ƥ���
		�Ʒ���������Ф��Ƥޤ�ɽ�����Ĵ�١������ɤ����Ĵ�٤�
    */

    str_buffer[BNST_LENGTH_MAX-1] = GUARD;

    /* ptr->SM_num ��init_bnst��0�˽��������Ƥ��� */

    for (stop = 0; stop < ptr->fuzoku_num; stop++) 
	if (!strcmp(Class[(ptr->fuzoku_ptr + stop)->Hinshi][0].id, "����") ||
	    !strcmp(Class[(ptr->fuzoku_ptr + stop)->Hinshi][0].id, "Ƚ���"))
	    break;

    for (last = stop; last >= 0; last--) {
	end = ptr->settou_num + ptr->jiritu_num + last;
	for (strt =0 ; strt < (ptr->settou_num + ptr->jiritu_num); strt++) {

	    /* ɽ���Τޤ� */

	    *str_buffer = '\0';
	    for (i = strt; i < end; i++) {
		strcat(str_buffer, (ptr->mrph_ptr + i)->Goi2);
		if (str_buffer[BNST_LENGTH_MAX-1] != GUARD) {
		    overflow_flag = 1;
		    overflowed_function(str_buffer, BNST_LENGTH_MAX, "get_sm_code");
		    break;
		}
	    }

	    if (overflow_flag) {
		overflow_flag = 0;
		return;
	    }

	    code = get_sm(str_buffer);
	    if (code) {
		strcpy(ptr->SM_code, code);
		free(code);
	    }
	    if (*(ptr->SM_code)) goto Match;

	    /* ɽ�����Ǹ帶�� */

	    if (!str_eq((ptr->mrph_ptr + end - 1)->Goi,
			(ptr->mrph_ptr + end - 1)->Goi2)) {
		*str_buffer = '\0';
		for (i = strt; i < end - 1; i++) {
		    strcat(str_buffer, (ptr->mrph_ptr + i)->Goi2);
		    if (str_buffer[BNST_LENGTH_MAX-1] != GUARD) {
			overflow_flag = 1;
			overflowed_function(str_buffer, BNST_LENGTH_MAX, "get_sm_code");
			break;
		    }
		}

		if (overflow_flag) {
		    overflow_flag = 0;
		    return;
		}

		strcat(str_buffer, (ptr->mrph_ptr + end - 1)->Goi);
		if (str_buffer[BNST_LENGTH_MAX-1] != GUARD) {
		    overflow_flag = 1;
		    overflowed_function(str_buffer, BNST_LENGTH_MAX, "get_sm_code");
		    break;
		}

		if (overflow_flag) {
		    overflow_flag = 0;
		    return;
		}

		/* �ʷ��ƻ�ξ��ϸ촴�Ǹ��� */
		if (str_eq(Class[(ptr->mrph_ptr + end - 1)->Hinshi][0].id,
			   "���ƻ�") &&
		    (str_eq(Type[(ptr->mrph_ptr + end - 1)->Katuyou_Kata].name,
			   "�ʷ��ƻ�") ||
		     str_eq(Type[(ptr->mrph_ptr + end - 1)->Katuyou_Kata].name,
			   "�ʷ��ƻ��ü�") ||
		     str_eq(Type[(ptr->mrph_ptr + end - 1)->Katuyou_Kata].name,
			   "�ʥη��ƻ�"))) 
		    str_buffer[strlen(str_buffer)-2] = NULL;

		code = get_sm(str_buffer);
		if (code) {
		    strcpy(ptr->SM_code, code);
		    free(code);
		}
		if (*(ptr->SM_code)) goto Match;
	    }

	    /* �ɤߤΤޤ� */

	    *str_buffer = '\0';
	    for (i = strt; i < end; i++) {
		strcat(str_buffer, (ptr->mrph_ptr + i)->Yomi);
		if (str_buffer[BNST_LENGTH_MAX-1] != GUARD) {
		    overflow_flag = 1;
		    overflowed_function(str_buffer, BNST_LENGTH_MAX, "get_sm_code");
		    break;
		}
	    }

	    if (overflow_flag) {
		overflow_flag = 0;
		return;
	    }

	    code = get_sm(str_buffer);
	    if (code) {
		strcpy(ptr->SM_code, code);
		free(code);
	    }
	    if (*(ptr->SM_code)) goto Match;
	}
    }
  Match:
    ptr->SM_num = strlen(ptr->SM_code) / SM_CODE_SIZE;

    sprintf(feature_buffer, "SM:%s", ptr->SM_code);
    assign_cfeature(&(ptr->f), feature_buffer);
}

/*==================================================================*/
		       char *sm2code(char *cp)
/*==================================================================*/
{
    char *code;

    /* sm �� code �� 1:1 �б� 
       -> cont_str �ϰ��ʤ� */

    if (SM2CODEExist == FALSE) {
	cont_str[0] = '\0';
	return cont_str;
    }

    code = db_get(sm2code_db, cp);
    if (code) {
	strcpy(cont_str, code);
	free(code);
    }
    else {
	cont_str[0] = '\0';
    }
    return cont_str;
}

/*==================================================================*/
		       char *smp2smg(char *cp)
/*==================================================================*/
{
    char *code;

    /* �ͤ�Ĺ���Ƥ� 52 bytes ���餤 */

    if (SMP2SMGExist == FALSE) {
	cont_str[0] = '\0';
	return cont_str;
    }
    
    code = db_get(smp2smg_db, cp);
    if (code) {
	strcpy(cont_str, code);
	free(code);
    }
    else {
	cont_str[0] = '\0';
    }
    return cont_str;
}

/*====================================================================
                               END
====================================================================*/
