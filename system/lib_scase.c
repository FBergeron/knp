/*====================================================================

			      ɽ�سʾ���

                                               S.Kurohashi 92.10.21
                                               S.Kurohashi 93. 5.31

    $Id$
====================================================================*/
#include "knp.h"

DBM_FILE	scase_db;
int		ScaseDicExist;

/*==================================================================*/
                    void init_scase()
/*==================================================================*/
{
    if ((scase_db = DBM_open(SCASE_DB_NAME, O_RDONLY, 0)) == NULL) {
        /* fprintf(stderr, 
	   "Can not open Database <%s>.\n", SCASE_DB_NAME);
	   exit(1);
	*/
	ScaseDicExist = FALSE;
    } else {
	ScaseDicExist = TRUE;
    }
}

/*==================================================================*/
                    void close_scase()
/*==================================================================*/
{
    if (ScaseDicExist == TRUE)
      DBM_close(scase_db);
}

/*==================================================================*/
                    char *get_scase(char *cp)
/*==================================================================*/
{
    int i;

    key.dptr = cp;
    if ((key.dsize = strlen(cp)) >= DBM_KEY_MAX) {
	fprintf(stderr, "Too long key <%s>.\n", key_str);
	return NULL;
    }  
    
    content = DBM_fetch(scase_db, key);
    if (content.dptr) {
	strncpy(cont_str, content.dptr, content.dsize);
	for (i = 0; i < content.dsize; i++)
	  cont_str[i] -= '0';
	cont_str[content.dsize] = '\0';
#ifdef	GDBM
	free(content.dptr);
	content.dsize = 0;
#endif
	return cont_str;
    }
    else {
	return NULL;
    }
}

/*==================================================================*/
             void get_scase_code(BNST_DATA *ptr)
/*==================================================================*/
{
    int strt, end, last, stop, i;
    char *cp, *ans, str_buffer[256];    

    for (i = 0, cp = ptr->SCASE_code; i < 11; i++, cp++) *cp = 0;
    /* init_bnst �Ǥ⤷�Ƥ��� */

    if (ScaseDicExist == TRUE &&
	(check_feature(ptr->f, "�Ѹ�:��:ư") ||
	 check_feature(ptr->f, "�Ѹ�:��:��") ||
	 check_feature(ptr->f, "�Ѹ�:��"))) {

	/* �ޤ���°�����ꡤ��Ω��򸺤餷�Ƥ��� */

	for (stop = 0; stop < ptr->fuzoku_num; stop++) 
	    if (!strcmp(Class[(ptr->fuzoku_ptr + stop)->Hinshi][0].id, "����"))
		break;

	for (last = stop; last >= 0; last--) {
	    end = ptr->settou_num + ptr->jiritu_num + last;
	    for (strt=0 ; strt<(ptr->settou_num + ptr->jiritu_num); strt++) {
		*str_buffer = '\0';
		for (i = strt; i < end; i++)
		  strcat(str_buffer, (ptr->mrph_ptr + i)->Goi);
		if ((ans = get_scase(str_buffer)) != NULL) {
		    cp = ptr->SCASE_code;
		    for (i = 0; i < 11; i++) *cp++ = *ans++;
		    goto Match;
		}
	    }
	}
    }

    /* Ƚ���ʤɤξ��,
       ɽ�سʼ��񤬤ʤ����, 
       �ޤ��ϼ���ˤʤ��Ѹ��ξ�� */
    
    if (check_feature(ptr->f, "�Ѹ�:��:Ƚ")) {
	ptr->SCASE_code[case2num("����")] = 1;
    } 
    else if (check_feature(ptr->f, "�Ѹ�:��:��")) {
	ptr->SCASE_code[case2num("����")] = 1;
	ptr->SCASE_code[case2num("�˳�")] = 1;
	ptr->SCASE_code[case2num("����")] = 1;
	ptr->SCASE_code[case2num("�ȳ�")] = 1;
    } 
    else if (check_feature(ptr->f, "�Ѹ�:��:ư")) {
	ptr->SCASE_code[case2num("����")] = 1;
	ptr->SCASE_code[case2num("���")] = 1;
	ptr->SCASE_code[case2num("�˳�")] = 1;
	ptr->SCASE_code[case2num("�س�")] = 1;
	ptr->SCASE_code[case2num("�ȳ�")] = 1;
    }

  Match:

    /* ���������ˤ�뽤�� */

    if (ptr->voice == VOICE_SHIEKI) {
	ptr->SCASE_code[case2num("���")] = 1;
	ptr->SCASE_code[case2num("�˳�")] = 1;
    } else if (ptr->voice == VOICE_UKEMI) {
	ptr->SCASE_code[case2num("�˳�")] = 1;
    } else if (ptr->voice == VOICE_MORAU) {
	ptr->SCASE_code[case2num("���")] = 1;
	ptr->SCASE_code[case2num("�˳�")] = 1;
    }
}

/*==================================================================*/
		      void set_pred_caseframe()
/*==================================================================*/
{
    int i;
    BNST_DATA  *b_ptr;

    for (i = 0, b_ptr = bnst_data; i < Bnst_num; i++, b_ptr++)
	if (check_feature(b_ptr->f, "�Ѹ�:��") ||
	    check_feature(b_ptr->f, "�Ѹ�:��")) {

	    /* �ʲ���2�Ĥν�����feature��٥�ǵ�ư���Ƥ��� */
	    /* set_pred_voice(b_ptr); �������� */
	    /* get_scase_code(b_ptr); ɽ�س� */

 	    if (OptAnalysis == OPT_CASE ||
 		OptAnalysis == OPT_CASE2 ||
 		OptAnalysis == OPT_DISC)
		make_case_frames(b_ptr);/* �ʥե졼�� */
	}
}

/*====================================================================
                               END
====================================================================*/
