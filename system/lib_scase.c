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
    char *filename;

    if (DICT[SCASE_DB]) {
	filename = (char *)check_dict_filename(DICT[SCASE_DB], FALSE);
    }
    else {
	filename = (char *)check_dict_filename(SCASE_DB_NAME, FALSE);
    }

    if (OptDisplay == OPT_DEBUG) {
	fprintf(Outfp, "Opening %s ... ", filename);
    }

    if ((scase_db = DBM_open(filename, O_RDONLY, 0)) == NULL) {
	if (OptDisplay == OPT_DEBUG) {
	    fputs("failed.\n", Outfp);
	}
	ScaseDicExist = FALSE;
#ifdef DEBUG
	fprintf(stderr, "Cannot open SCASE dictionary <%s>.\n", filename);
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
      DBM_close(scase_db);
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
    int strt, end, last, stop, i, overflow_flag = 0;
    char *cp, *ans, *anscp, str_buffer[2*BNST_LENGTH_MAX], *vtype, *predicate;

    str_buffer[BNST_LENGTH_MAX-1] = GUARD;

    for (i = 0, cp = ptr->SCASE_code; i < 11; i++, cp++) *cp = 0;
    /* init_bnst �Ǥ⤷�Ƥ��� */

    if (ScaseDicExist == TRUE &&
	(vtype = check_feature(ptr->f, "�Ѹ�")) && 
	strcmp(vtype, "�Ѹ�:Ƚ")) {
	vtype += 5;

	/* �ޤ���°�����ꡤ��Ω��򸺤餷�Ƥ��� */

	for (stop = 0; stop < ptr->fuzoku_num; stop++) 
	    if (!strcmp(Class[(ptr->fuzoku_ptr + stop)->Hinshi][0].id, "����"))
		break;

	for (last = stop; last >= 0; last--) {
	    end = ptr->settou_num + ptr->jiritu_num + last;
	    for (strt=0 ; strt<(ptr->settou_num + ptr->jiritu_num); strt++) {
		*str_buffer = '\0';
		for (i = strt; i < end; i++) {
		    strcat(str_buffer, (ptr->mrph_ptr + i)->Goi);
		    if (str_buffer[BNST_LENGTH_MAX-1] != GUARD) {
			overflow_flag = 1;
			overflowed_function(str_buffer, BNST_LENGTH_MAX, "get_scase_code");
			break;
		    }
		}

		if (overflow_flag) {
		    overflow_flag = 0;
		    return;
		}

		/* �Ѹ������פ�ޤޤʤ�����ξ�� */
		ans = get_scase(str_buffer);
		if (ans == NULL) {
		    /* �Ѹ������פ�ޤ༭��ξ�� */
		    predicate = strdup(str_buffer);
		    if (ptr->num > 0) {
			cp = check_feature((ptr-1)->f, "��");
			if (cp) {
			    sprintf(str_buffer, "%s:%s:%s:%s", L_Jiritu_M((ptr-1)), cp+3, predicate, vtype);
			    ans = get_scase(str_buffer);
			}
		    }
		    if (ans == NULL) {
			sprintf(str_buffer, "%s:%s", predicate, vtype);
			ans = get_scase(str_buffer);
		    }
		    /* DEBUG ɽ�� */
		    if (OptDisplay == OPT_DEBUG) {
			if (ans != NULL) {
			    char print_buffer[2*BNST_LENGTH_MAX];
			    sprintf(print_buffer, "SCASEUSE:%s", str_buffer);
			    assign_cfeature(&(ptr->f), print_buffer);
			}
		    }
		    free(predicate);
		}
		if (ans != NULL) {
		    cp = ptr->SCASE_code;
		    anscp = ans;
		    for (i = 0; i < 11; i++) *cp++ = *anscp++;
		    free(ans);
		    goto Match;
		}
	    }
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

/*====================================================================
                               END
====================================================================*/
