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
    if ((bgh_db = DBM_open(BGH_DB_NAME, O_RDONLY, 0)) == NULL) {
        /* 
	   fprintf(stderr, "Can not open Database <%s>.\n", BGH_DB_NAME);
	   exit(1);
	*/
	BGHExist = FALSE;
    } else {
	BGHExist = TRUE;
    }
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
    key.dptr = cp;
    if ((key.dsize = strlen(cp)) >= DBM_KEY_MAX) {
	fprintf(stderr, "Too long key <%s>.\n", cp);
	cont_str[0] = '\0';
	return cont_str;
    }  
    
    content = DBM_fetch(bgh_db, key);
    if (content.dptr) {
	strncpy(cont_str, content.dptr, content.dsize);
	cont_str[content.dsize] = '\0';
#ifdef	GDBM
	free(content.dptr);
	content.dsize = 0;
#endif
    }
    else {
	cont_str[0] = '\0';
    }

    return cont_str;
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
    int strt, end, last, stop, i, overflow_flag = 0;
    char str_buffer[BNST_LENGTH_MAX];    

    str_buffer[BNST_LENGTH_MAX-1] = GUARD;

    /* 
       ʣ���ΰ���
       		�ޤ���°�����ꡤ��Ω��򸺤餷�Ƥ���
		�Ʒ���������Ф��Ƥޤ�ɽ�����Ĵ�١������ɤ����Ĵ�٤�
    */

    if (BGHExist == FALSE) return;
    /* ptr->BGH_num ��init_bnst��0�˽��������Ƥ��� */

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
		    overflowed_function(str_buffer, BNST_LENGTH_MAX, "get_bgh_code");
		    break;
		}
	    }

	    if (overflow_flag) {
		overflow_flag = 0;
		return;
	    }

	    strcpy(ptr->BGH_code, get_bgh(str_buffer));
	    if (*(ptr->BGH_code)) goto Match;

	    /* ɽ�����Ǹ帶�� */

	    if (!str_eq((ptr->mrph_ptr + end - 1)->Goi,
			(ptr->mrph_ptr + end - 1)->Goi2)) {
		*str_buffer = '\0';
		for (i = strt; i < end - 1; i++) {
		    strcat(str_buffer, (ptr->mrph_ptr + i)->Goi2);
		    if (str_buffer[BNST_LENGTH_MAX-1] != GUARD) {
			overflow_flag = 1;
			overflowed_function(str_buffer, BNST_LENGTH_MAX, "get_bgh_code");
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
		    overflowed_function(str_buffer, BNST_LENGTH_MAX, "get_bgh_code");
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

		strcpy(ptr->BGH_code, get_bgh(str_buffer));
		if (*(ptr->BGH_code)) goto Match;
	    }

	    /* �ɤߤΤޤ� */

	    *str_buffer = '\0';
	    for (i = strt; i < end; i++) {
		strcat(str_buffer, (ptr->mrph_ptr + i)->Yomi);
		if (str_buffer[BNST_LENGTH_MAX-1] != GUARD) {
		    overflow_flag = 1;
		    overflowed_function(str_buffer, BNST_LENGTH_MAX, "get_bgh_code");
		    break;
		}
	    }

	    if (overflow_flag) {
		overflow_flag = 0;
		return;
	    }

	    strcpy(ptr->BGH_code, get_bgh(str_buffer));
	    if (*(ptr->BGH_code)) goto Match;
	}
    }
  Match:
    ptr->BGH_num = strlen(ptr->BGH_code) / 10;
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
	for (i = 1; c1[i] == c2[i] && i < 10; i++)
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
