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
	filename = (char *)check_dict_filename(DICT[SM_DB], TRUE);
    }
    else {
	filename = (char *)check_dict_filename(SM_DB_NAME, FALSE);
    }

    if (OptDisplay == OPT_DEBUG) {
	fprintf(Outfp, "Opening %s ... ", filename);
    }

    if ((sm_db = DBM_open(filename, O_RDONLY, 0)) == NULL) {
	if (OptDisplay == OPT_DEBUG) {
	    fputs("failed.\n", Outfp);
	}
	SMExist = FALSE;
#ifdef DEBUG
	fprintf(stderr, "Cannot open NTT word dictionary <%s>.\n", filename);
#endif
    }
    else {
	if (OptDisplay == OPT_DEBUG) {
	    fputs("done.\n", Outfp);
	}
	SMExist = TRUE;
    }
    free(filename);

    /* ��̣�� <=> ��̣�ǥ����� */
    if (DICT[SM2CODE_DB]) {
	filename = (char *)check_dict_filename(DICT[SM2CODE_DB], TRUE);
    }
    else {
	filename = (char *)check_dict_filename(SM2CODE_DB_NAME, FALSE);
    }

    if (OptDisplay == OPT_DEBUG) {
	fprintf(Outfp, "Opening %s ... ", filename);
    }

    if ((sm2code_db = DBM_open(filename, O_RDONLY, 0)) == NULL) {
	if (OptDisplay == OPT_DEBUG) {
	    fputs("failed.\n", Outfp);
	}
	SM2CODEExist = FALSE;
#ifdef DEBUG
	fprintf(stderr, "Cannot open NTT sm dictionary <%s>.\n", filename);
#endif
    }
    else {
	if (OptDisplay == OPT_DEBUG) {
	    fputs("done.\n", Outfp);
	}
	SM2CODEExist = TRUE;
    }
    free(filename);

    /* ��ͭ̾���η� <=> ����̾���η� */
    if (DICT[SMP2SMG_DB]) {
	filename = (char *)check_dict_filename(DICT[SMP2SMG_DB], TRUE);
    }
    else {
	filename = (char *)check_dict_filename(SMP2SMG_DB_NAME, FALSE);
    }

    if (OptDisplay == OPT_DEBUG) {
	fprintf(Outfp, "Opening %s ... ", filename);
    }

    if ((smp2smg_db = DBM_open(filename, O_RDONLY, 0)) == NULL) {
	if (OptDisplay == OPT_DEBUG) {
	    fputs("failed.\n", Outfp);
	}
	SMP2SMGExist = FALSE;
#ifdef DEBUG
	fprintf(stderr, "Cannot open NTT smp smg table <%s>.\n", filename);
#endif
    }
    else {
	if (OptDisplay == OPT_DEBUG) {
	    fputs("done.\n", Outfp);
	}
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
		   int ne_check_all_sm(char *code)
/*==================================================================*/
{
    int i;

    /* ���٤Ƥΰ�̣°������ͭ̾��ʤ� TRUE */

    for (i = 0; *(code+i); i+=SM_CODE_SIZE) {
	if (*(code+i) != '2') {
	    return FALSE;
	}
    }
    return TRUE;
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

	/* ���٤Ƥΰ�̣°������ͭ̾��ΤȤ� */
	if (ne_check_all_sm(code) == TRUE) {
	    for (i = 0; code[i]; i+=SM_CODE_SIZE) {
		if (code[i] == '2' && 
		    strncmp(code+i, "2001030", 7)) { /* ��� �ǤϤʤ� */
		    strncpy(code+pos, code+i, SM_CODE_SIZE);
		    pos += SM_CODE_SIZE;
		}
	    }
	}
	else {
	    /* ��̣�Ǥ���Ϳ�����ʻ� */
	    for (i = 0; code[i]; i+=SM_CODE_SIZE) {
		if (code[i] == '3' ||	/* ̾ */
		    code[i] == '4' ||	/* ̾(����) */
		    code[i] == '5' ||	/* ̾(��ư) */
		    code[i] == '6' ||	/* ̾(ž��) */
		    code[i] == '7' ||	/* ���� */
		    code[i] == '9' ||	/* ���� */
		    code[i] == 'a') {	/* ��̾ */
		    strncpy(code+pos, code+i, SM_CODE_SIZE);
		    pos += SM_CODE_SIZE;
		}
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
    int strt, end, stop, i;
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
	    !strcmp(Class[(ptr->fuzoku_ptr + stop)->Hinshi][0].id, "Ƚ���") ||
	    !strcmp(Class[(ptr->fuzoku_ptr + stop)->Hinshi][0].id, "��ư��") ||
	    !strcmp(Class[(ptr->fuzoku_ptr + stop)->Hinshi][0].id, "�ü�") ||
	    (!strcmp(Class[(ptr->fuzoku_ptr + stop)->Hinshi][0].id, "������") &&
	    strcmp(Class[(ptr->fuzoku_ptr + stop)->Bunrui][0].id, "̾����̾��������")))
	    break;

    end = ptr->settou_num + ptr->jiritu_num + stop;
    for (strt =0 ; strt < (ptr->settou_num + ptr->jiritu_num); strt++) {

	/* ɽ���Τޤ� */

	*str_buffer = '\0';
	for (i = strt; i < end; i++) {
	    if (strlen(str_buffer)+strlen((ptr->mrph_ptr + i)->Goi2)+2 > BNST_LENGTH_MAX) {
		overflowed_function(str_buffer, BNST_LENGTH_MAX, "get_sm_code");
		return;
	    }
	    strcat(str_buffer, (ptr->mrph_ptr + i)->Goi2);
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
		if (strlen(str_buffer)+strlen((ptr->mrph_ptr + i)->Goi2)+2 > BNST_LENGTH_MAX) {
		    overflowed_function(str_buffer, BNST_LENGTH_MAX, "get_sm_code");
		    return;
		}
		strcat(str_buffer, (ptr->mrph_ptr + i)->Goi2);
	    }

	    if (strlen(str_buffer)+strlen((ptr->mrph_ptr + end - 1)->Goi)+2 > BNST_LENGTH_MAX) {
		overflowed_function(str_buffer, BNST_LENGTH_MAX, "get_sm_code");
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

    if (ptr->SM_num > 0) {
	sprintf(feature_buffer, "SM:%s:%s", str_buffer, ptr->SM_code);
	assign_cfeature(&(ptr->f), feature_buffer);
    }
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
		       char *_smp2smg(char *cp)
/*==================================================================*/
{
    char *code, key[SM_CODE_SIZE+1];

    /* �ͤ�Ĺ���Ƥ� 52 bytes ���餤 */

    if (SMP2SMGExist == FALSE) {
	cont_str[0] = '\0';
	return cont_str;
    }

    strncpy(key, cp, SM_CODE_SIZE);
    key[SM_CODE_SIZE] = '\0';

    code = db_get(smp2smg_db, key);
    return code;
}

/*==================================================================*/
		  char *smp2smg(char *cpd, int flag)
/*==================================================================*/
{
    char *cp, *start;
    int storep = 0, inc, use = 1;

    if (SMP2SMGExist == FALSE) {
	fprintf(stderr, ";;; Cannot open smp2smg table!\n");
	return NULL;
    }

    start = _smp2smg(cpd);

    if (start == NULL) {
	return NULL;
    }

    for (cp = start; *cp; cp+=SM_CODE_SIZE) {
	use = 1;
	if (*(cp+SM_CODE_SIZE) == '/') {
	    inc = 1;
	}
	else if (!strncmp(cp+SM_CODE_SIZE, " side-effect", 12)) {
	    if (*(cp+SM_CODE_SIZE+12) == '/') {
		inc = 13;		
	    }
	    /* ����ǽ���� */
	    else {
		inc = 0;
	    }
	    /* flag == FALSE �ξ�� side-effect ��Ȥ�ʤ� */
	    if (flag == FALSE) {
		use = 0;
	    }
	}
	else if (*(cp+SM_CODE_SIZE) != '\0') {
	    fprintf(stderr, ";;; Invalid delimiter! <%c> (%s)\n", 
		    *(cp+SM_CODE_SIZE), "smp2smg");
	    inc = 1;
	}
	/* ����ǽ���� '\0' */
	else {
	    inc = 0;
	}

	if (use) {
	    strncpy(start+storep, cp, SM_CODE_SIZE);
	    storep+=SM_CODE_SIZE;
	}
	if (inc) {
	    cp += inc;
	}
	else {
	    break;
	}
    }

    if (storep) {
	*(start+storep) = '\0';
	return start;
    }
    free(start);
    return NULL;
}

/*==================================================================*/
		     int sm_code_depth(char *cp)
/*==================================================================*/
{
    int i;

    /* ��̣�ǥ����ɤο������֤��ؿ� (0 .. SM_CODE_SIZE-1) */

    for (i = 1; i < SM_CODE_SIZE; i++) {
	if (*(cp+i) == '*') {
	    return i-1;
	}
    }
    return SM_CODE_SIZE-1;
}

/*==================================================================*/
	      float _ntt_code_match(char *c1, char *c2)
/*==================================================================*/
{
    int i, d1, d2, min;

    if ((*c1 == '2' && *c2 != '2') || 
	(*c1 != '2' && *c2 == '2')) {
	return 0;
    }

    d1 = sm_code_depth(c1);
    d2 = sm_code_depth(c2);

    if (d1 + d2 == 0) {
	return 0;
    }

    min = d1 < d2 ? d1 : d2;

    if (min == 0) {
	return 0;
    }

    for (i = 1; i <= min; i++) {
	if (*(c1+i) != *(c2+i)) {
	    return (float)2*(i-1)/(d1+d2);
	}
    }
    return (float)2*min/(d1+d2);
}

/*==================================================================*/
	  float ntt_code_match(char *c1, char *c2, int flag)
/*==================================================================*/
{
    if (flag == SM_EXPAND_NE) {
	float score, maxscore = 0;
	char *cp1, *cp2;
	int f1 = 0, f2 = 0;

	if (*c1 == '2') {
	    c1 = smp2smg(c1, FALSE);
	    if (!c1) {
		return 0;
	    }
	    f1 = 1;
	}
	if (*c2 == '2') {
	    c2 = smp2smg(c2, FALSE);
	    if (!c2) {
		if (f1 == 1) {
		    free(c1);
		}
		return 0;
	    }
	    f2 = 1;
	}

	for (cp1 = c1; *cp1; cp1+=SM_CODE_SIZE) {
	    for (cp2 = c2; *cp2; cp2+=SM_CODE_SIZE) {
		score = _ntt_code_match(cp1, cp2);
		if (score > maxscore) {
		    maxscore = score;
		}
	    }
	}
	if (f1 == 1) {
	    free(c1);
	}
	if (f2 == 1) {
	    free(c2);
	}
	return maxscore;
    }
    else {
	return _ntt_code_match(c1, c2);
    }
}

/*==================================================================*/
	      float CalcSimilarity(char *exd, char *exp)
/*==================================================================*/
{
    int i, j, step;
    float score = 0, tempscore;

    /* �ɤ��餫������Υ����ɤ��ʤ��Ȥ� */
    if (!(exd && exp && *exd && *exp)) {
	return score;
    }

    if (Thesaurus == USE_BGH) {
	step = BGH_CODE_SIZE;
    }
    else if (Thesaurus == USE_NTT) {
	step = SM_CODE_SIZE;
    }

    /* ����ޥå������������ */
    for (j = 0; exp[j]; j+=step) {
	for (i = 0; exd[i]; i+=step) {
	    if (Thesaurus == USE_BGH) {
		tempscore = (float)_ex_match_score(exp+j, exd+i);
	    }
	    else if (Thesaurus == USE_NTT) {
		tempscore = ntt_code_match(exp+j, exd+i, SM_EXPAND_NE);
	    }
	    if (tempscore > score) {
		score = tempscore;
	    }
	}
    }
    return score;
}

/*==================================================================*/
	       int sm_match_check(char *pat, char *codes)
/*==================================================================*/
{
    int i;

    if (codes == NULL) {
	return FALSE;
    }

    for (i = 0; *(codes+i); i += SM_CODE_SIZE) {
	if (_sm_match_score(pat, codes+i, SM_NO_EXPAND_NE) > 0) {
	    return TRUE;
	}
    }
    return FALSE;
}

/*==================================================================*/
		int assign_sm(BNST_DATA *bp, char *cp)
/*==================================================================*/
{
    char *code;
    code = sm2code(cp);

    /* ���Ǥˤ��ΰ�̣°�����äƤ���Ȥ� */
    if (sm_match_check(code, bp->SM_code) == TRUE) {
	return FALSE;
    }

    /* ������?�� */
    strcat(bp->SM_code, code);
    bp->SM_num++;
    return TRUE;
}

/*==================================================================*/
		      int sm_time_match(char *c)
/*==================================================================*/
{
    char *p, flag = 0;

    /* ��ͭ̾��ΤȤ��ʳ��ǡ����٤Ƥΰ�̣°�������֤Ǥ���� TRUE */
    for (p = c;*p; p+=SM_CODE_SIZE) {
	/* ��ͭ̾��ΤȤ���Τ��� */
	if (*p == '2') {
	    continue;
	}
	/* ���֤Υ����� */
	if (!comp_sm("1128********", p, 1)) {
	    return FALSE;
	}
	else if (!flag) {
	    flag = 1;
	}
    }

    if (flag) {
	return TRUE;
    }
    else {
	return FALSE;
    }
}

/*==================================================================*/
	       void assign_time_feature(BNST_DATA *bp)
/*==================================================================*/
{
    /* <����> �ΰ�̣�Ǥ�����äƤ��ʤ���� <����> ��Ϳ���� */
    if (!check_feature(bp->f, "����") && 
	sm_time_match(bp->SM_code)) {
	assign_cfeature(&(bp->f), "����");
    }
}

/*====================================================================
                               END
====================================================================*/
