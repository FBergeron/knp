/*====================================================================

		      ʬ�����ɽ  �����ץ����

                                               S.Kurohashi 91. 6.25
                                               S.Kurohashi 93. 5.31

    $Id$
====================================================================*/
#include "knp.h"

DBM_FILE	bgh_db;
int		BGHExist;

/*==================================================================*/
			   void init_bgh()
/*==================================================================*/
{
    char *filename;

    if (DICT[BGH_DB]) {
	filename = (char *)check_dict_filename(DICT[BGH_DB], TRUE);
    }
    else {
	filename = (char *)check_dict_filename(BGH_DB_NAME, FALSE);
    }

    if (OptDisplay == OPT_DEBUG) {
	fprintf(Outfp, "Opening %s ... ", filename);
    }

    if ((bgh_db = DBM_open(filename, O_RDONLY, 0)) == NULL) {
	if (OptDisplay == OPT_DEBUG) {
	    fputs("failed.\n", Outfp);
	}
	BGHExist = FALSE;
#ifdef DEBUG
	fprintf(stderr, "Cannot open BGH dictionary <%s>.\n", filename);
#endif
    } else {
	if (OptDisplay == OPT_DEBUG) {
	    fputs("done.\n", Outfp);
	}
	BGHExist = TRUE;
    }
    free(filename);
}

/*==================================================================*/
                    void close_bgh()
/*==================================================================*/
{
    if (BGHExist == TRUE)
	DBM_close(bgh_db);
}

/*==================================================================*/
                    char *get_bgh(char *cp)
/*==================================================================*/
{
    if (BGHExist == TRUE)
	return db_get(bgh_db, cp);
    else
	return NULL;
}

/*==================================================================*/
	    char *meishi_setubi(BNST_DATA *ptr, char *cp)
/*==================================================================*/
{
    /* ���� �ȤäƤ��ʤ� */

    int i, flag = 0;		/* case_print �ǻ��� */

    *cp = '\0';    
    for (i = 0; i < ptr->fuzoku_num; i++) {

	if (!strcmp(Class[(ptr->fuzoku_ptr + i)->Hinshi][0].id, "������") &&
	    !strcmp(Class[(ptr->fuzoku_ptr + i)->Hinshi]
		    [(ptr->fuzoku_ptr + i)->Bunrui].id, "̾����̾��������")) {
	    strcat(cp, (ptr->fuzoku_ptr + i)->Goi);
	    flag = 1;
	}	
	else if (flag == 1)
	  break;
	else
	  strcat(cp, (ptr->fuzoku_ptr + i)->Goi);
    }    
    
    if (flag == 0)
      *cp = '\0';

    return cp;
}

/*==================================================================*/
	void overflowed_function(char *str, int max, char *function)
/*==================================================================*/
{
    str[max-1] = '\0';
    fprintf(stderr, "Too long key <%s> in %s.\n", str, function);
    str[max-1] = GUARD;
}

/*==================================================================*/
		  void get_bgh_code(BNST_DATA *ptr)
/*==================================================================*/
{
    int strt, end, stop, i;
    char str_buffer[BNST_LENGTH_MAX], *code;
    char feature_buffer[BNST_LENGTH_MAX];

    /* ����� */
    *(ptr->BGH_code) = '\0';

    if (BGHExist == FALSE) return;

    /* 
       ʣ���ΰ���
       		�ޤ���°�����ꡤ��Ω��򸺤餷�Ƥ���
		�Ʒ���������Ф��Ƥޤ�ɽ�����Ĵ�١������ɤ����Ĵ�٤�
    */

    str_buffer[BNST_LENGTH_MAX-1] = GUARD;

    /* ptr->BGH_num ��init_bnst��0�˽��������Ƥ��� */

    for (stop = 0; stop < ptr->fuzoku_num; stop++) 
	if (!strcmp(Class[(ptr->fuzoku_ptr + stop)->Hinshi][0].id, "����") ||
	    !strcmp(Class[(ptr->fuzoku_ptr + stop)->Hinshi][0].id, "Ƚ���") ||
	    !strcmp(Class[(ptr->fuzoku_ptr + stop)->Hinshi][0].id, "��ư��") ||
	    !strcmp(Class[(ptr->fuzoku_ptr + stop)->Hinshi][0].id, "�ü�") ||
	    !strcmp(Class[(ptr->fuzoku_ptr + stop)->Hinshi][0].id, "ư��") || /* �Ѹ��ΤȤ�����°���ư����ӽ� */
	    (!strcmp(Class[(ptr->fuzoku_ptr + stop)->Hinshi][0].id, "������") &&
	     strcmp(Class[(ptr->fuzoku_ptr + stop)->Bunrui][0].id, "̾����̾��������")))
	    break;

    end = ptr->settou_num + ptr->jiritu_num + stop;
    for (strt =0 ; strt < (ptr->settou_num + ptr->jiritu_num); strt++) {

	/* ɽ���Τޤ� */

	*str_buffer = '\0';
	for (i = strt; i < end; i++) {
	    if (strlen(str_buffer)+strlen((ptr->mrph_ptr + i)->Goi2)+2 > BNST_LENGTH_MAX) {
		overflowed_function(str_buffer, BNST_LENGTH_MAX, "get_bgh_code");
		return;
	    }
	    strcat(str_buffer, (ptr->mrph_ptr + i)->Goi2);
	}

	code = get_bgh(str_buffer);

	/* ����Ȥ� */
	if (code) {
	    if (strlen(code) > EX_ELEMENT_MAX*BGH_CODE_SIZE) {
		strncpy(ptr->BGH_code, code, EX_ELEMENT_MAX*BGH_CODE_SIZE);
		ptr->BGH_code[EX_ELEMENT_MAX*BGH_CODE_SIZE] = '\0';
		fprintf(stderr, "Too many BGH code <%s>.\n", str_buffer);
	    }
	    strcpy(ptr->BGH_code, code);
	    free(code);
	}
	if (*(ptr->BGH_code)) goto Match;

	/* ɽ�����Ǹ帶�� */

	if (!str_eq((ptr->mrph_ptr + end - 1)->Goi,
		    (ptr->mrph_ptr + end - 1)->Goi2)) {
	    *str_buffer = '\0';
	    for (i = strt; i < end - 1; i++) {
		if (strlen(str_buffer)+strlen((ptr->mrph_ptr + i)->Goi2)+2 > BNST_LENGTH_MAX) {
		    overflowed_function(str_buffer, BNST_LENGTH_MAX, "get_bgh_code");
		    return;
		}
		strcat(str_buffer, (ptr->mrph_ptr + i)->Goi2);
	    }

	    if (strlen(str_buffer)+strlen((ptr->mrph_ptr + end - 1)->Goi)+2 > BNST_LENGTH_MAX) {
		overflowed_function(str_buffer, BNST_LENGTH_MAX, "get_bgh_code");
		return;
	    }
	    strcat(str_buffer, (ptr->mrph_ptr + end - 1)->Goi);

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

	    code = get_bgh(str_buffer);

	    if (code) {
		if (strlen(code) > EX_ELEMENT_MAX*BGH_CODE_SIZE) {
		    strncpy(ptr->BGH_code, code, EX_ELEMENT_MAX*BGH_CODE_SIZE);
		    ptr->BGH_code[EX_ELEMENT_MAX*BGH_CODE_SIZE] = '\0';
		    fprintf(stderr, "Too many BGH code <%s>.\n", str_buffer);
		}
		strcpy(ptr->BGH_code, code);
		free(code);
	    }
	    if (*(ptr->BGH_code)) goto Match;
	}
    }

  Match:
    ptr->BGH_num = strlen(ptr->BGH_code) / BGH_CODE_SIZE;

    if (ptr->BGH_num > 0) {
	sprintf(feature_buffer, "BGH:%s", str_buffer);
	assign_cfeature(&(ptr->f), feature_buffer);
    }
}

/*==================================================================*/
		int bgh_code_match(char *c1, char *c2)
/*==================================================================*/
{
    int i, point = 0;

    /* 1���ܰ��� -> 1,2,3,4,5,6-7,8-10�������

       1�����԰��� -> 1���ܤ�4(����¾)�ʳ��ʤ� 2��4������� 
       		      2���ܰʹ߰��פξ�� 1���ܤϰ��פȤߤʤ� */

    if (c1[0] == c2[0]) {
	point = 1;
	for (i = 1; c1[i] == c2[i] && i < BGH_CODE_SIZE; i++)
	    if (i != 5 && i != 7 && i != 8)
		point ++;
    }
    else if (c1[0] != '4' && c2[0] != '4' && c1[1] == c2[1]) {
	point = 2;
	for (i = 2; c1[i] == c2[i] && i < 4; i++)	
	    point ++;
    }	     

    return point;
}

/*====================================================================
                               END
====================================================================*/
