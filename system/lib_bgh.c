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

    if ((bgh_db = db_open(filename, O_RDONLY, 0)) == NULL) {
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
	db_close(bgh_db);
}

/*==================================================================*/
		       char *_get_bgh(char *cp)
/*==================================================================*/
{
    return db_get(bgh_db, cp);
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
