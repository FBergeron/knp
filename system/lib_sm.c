/*====================================================================

		      ��̣��  �����ץ����

                                               S.Kurohashi 91. 6.25
                                               S.Kurohashi 93. 5.31

    $Id$
====================================================================*/
#include "knp.h"

DBM_FILE	sm_db;
DBM_FILE	sm2code_db;
DBM_FILE	code2sm_db;
DBM_FILE	smp2smg_db;
int		SMExist;
int		SM2CODEExist;
int		CODE2SMExist;
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

    /* ��̣�� => ��̣�ǥ����� */
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

    /* ��̣�ǥ����� => ��̣�� */
    if (DICT[CODE2SM_DB]) {
	filename = (char *)check_dict_filename(DICT[CODE2SM_DB], TRUE);
    }
    else {
	filename = (char *)check_dict_filename(CODE2SM_DB_NAME, FALSE);
    }

    if (OptDisplay == OPT_DEBUG) {
	fprintf(Outfp, "Opening %s ... ", filename);
    }

    if ((code2sm_db = DBM_open(filename, O_RDONLY, 0)) == NULL) {
	if (OptDisplay == OPT_DEBUG) {
	    fputs("failed.\n", Outfp);
	}
	CODE2SMExist = FALSE;
#ifdef DEBUG
	fprintf(stderr, "Cannot open NTT code2sm dictionary <%s>.\n", filename);
#endif
    }
    else {
	if (OptDisplay == OPT_DEBUG) {
	    fputs("done.\n", Outfp);
	}
	CODE2SMExist = TRUE;
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
		       char *_get_sm(char *cp)
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
		   char *get_sm(unsigned char *cp)
/*==================================================================*/
{
    int i;
    char *code;
    unsigned char *hira;

    if ((code = _get_sm(cp))) {
	return code;
    }

    /* ��̣�Ǥʤ����ǡ�
       ���٤Ƥ�ʸ�����������ʤξ��ϤҤ餬�ʤ��Ѵ����Ƽ������ */

    for (i = 0; i < strlen(cp); i += 2) {
	if (*(cp+i) != 0xa5) {
	    return NULL;
	}
    }
    hira = katakana2hiragana(cp);
    code = _get_sm(hira);
    free(hira);
    return code;
}

/*==================================================================*/
		   void get_sm_code(BNST_DATA *ptr)
/*==================================================================*/
{
    int strt, end, stop, i, jiritu;
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

    for (stop = 0; stop < ptr->fuzoku_num; stop++) {
	if (!strcmp(Class[(ptr->fuzoku_ptr + stop)->Hinshi][0].id, "����") ||
	    !strcmp(Class[(ptr->fuzoku_ptr + stop)->Hinshi][0].id, "Ƚ���") ||
	    !strcmp(Class[(ptr->fuzoku_ptr + stop)->Hinshi][0].id, "��ư��") ||
	    !strcmp(Class[(ptr->fuzoku_ptr + stop)->Hinshi][0].id, "�ü�") ||
	    !strcmp(Class[(ptr->fuzoku_ptr + stop)->Hinshi][0].id, "ư��") || /* �Ѹ��ΤȤ�����°���ư����ӽ� */
	    (!strcmp(Class[(ptr->fuzoku_ptr + stop)->Hinshi][0].id, "������") &&
	    strcmp(Class[(ptr->fuzoku_ptr + stop)->Bunrui][0].id, "̾����̾��������")))
	    break;
    }

    for (jiritu = 0; jiritu < ptr->jiritu_num; jiritu++) {
	if (check_feature((ptr->jiritu_ptr + jiritu)->f, "�Ը�ͭ����") && 
	    jiritu != 0) {
	    stop = 0;
	    break;
	}
    }

    end = ptr->settou_num + jiritu + stop;
    for (strt =0 ; strt < (ptr->settou_num + jiritu); strt++) {

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
	else {
	    /* �֤���פʤɰ�����ǤˤʤäƤ����礬����Τ�
	       "��" ��ȤäƤߤ� */
	    if (!strncmp(str_buffer, "��", 2)) {
		code = get_sm(str_buffer+2);
		if (code) {
		    strcpy(ptr->SM_code, code);
		    free(code);
		}
	    }
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
		str_buffer[strlen(str_buffer)-2] = '\0';

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
		       char *code2sm(char *cp)
/*==================================================================*/
{
    char *sm;

    /* sm �� code �� 1:1 �б� 
       -> cont_str �ϰ��ʤ� */

    if (CODE2SMExist == FALSE) {
	cont_str[0] = '\0';
	return cont_str;
    }

    sm = db_get(code2sm_db, cp);
    if (sm) {
	strcpy(cont_str, sm);
	free(sm);
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
		  void merge_smp2smg(BNST_DATA *bp)
/*==================================================================*/
{
    int i;
    char *p;

    /* smp2smg �η�̤򤯤äĤ��� */

    if (bp->SM_code[0] == '\0') {
	return;
    }

    for (i = 0; i < bp->SM_num; i++) {
	if (bp->SM_code[i*SM_CODE_SIZE] == '2') {
	    p = smp2smg(&(bp->SM_code[i*SM_CODE_SIZE]), FALSE);
	    if (p) {
		/* ��줿��� */
		if ((strlen(bp->SM_code)+strlen(p))/SM_CODE_SIZE > SM_ELEMENT_MAX) {
		    return;
		}
		strcat(bp->SM_code, p);
		free(p);
	    }
	}
    }
    bp->SM_num = strlen(bp->SM_code)/SM_CODE_SIZE;
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
	int i, j;
	int f1 = 0, f2 = 0, c1num = 1, c2num = 1;

	if (*c1 == '2') {
	    c1 = smp2smg(c1, FALSE);
	    if (!c1) {
		return 0;
	    }
	    f1 = 1;
	    c1num = strlen(c1)/SM_CODE_SIZE;
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
	    c2num = strlen(c2)/SM_CODE_SIZE;
	}

	for (cp1 = c1, i = 0; i < c1num; cp1+=SM_CODE_SIZE, i++) {
	    for (cp2 = c2, j = 0; j < c2num; cp2+=SM_CODE_SIZE, j++) {
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
    else if (flag == SM_EXPAND_NE_DATA) {
	float score, maxscore = 0;
	char *cp2;
	int i;
	int f2 = 0, c2num = 1;

	/* PATTERN: ��ͭ̾�� */
	if (*c1 == '2') {
	    return _ntt_code_match(c1, c2);
	}

	/* PATTERN: ����̾�� */

	if (*c2 == '2') {
	    c2 = smp2smg(c2, FALSE);
	    if (!c2) {
		return 0;
	    }
	    f2 = 1;
	    c2num = strlen(c2)/SM_CODE_SIZE;
	}

	for (cp2 = c2, i = 0; i < c2num; cp2+=SM_CODE_SIZE, i++) {
	    score = _ntt_code_match(c1, cp2);
	    if (score > maxscore) {
		maxscore = score;
	    }
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
	float CalcSimilarity(char *exd, char *exp, int expand)
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
	if (expand != SM_NO_EXPAND_NE) {
	    expand = SM_EXPAND_NE_DATA;
	}
    }

    /* ����ޥå������������ */
    for (j = 0; exp[j]; j+=step) {
	for (i = 0; exd[i]; i+=step) {
	    if (Thesaurus == USE_BGH) {
		tempscore = (float)_ex_match_score(exp+j, exd+i);
	    }
	    else if (Thesaurus == USE_NTT) {
		tempscore = ntt_code_match(exp+j, exd+i, expand);
	    }
	    if (tempscore > score) {
		score = tempscore;
	    }
	}
    }
    return score;
}

/*==================================================================*/
	    float CalcWordSimilarity(char *exd, char *exp)
/*==================================================================*/
{
    char *(*get_code)();
    char *smd, *smp;
    float score = 0;

    if (Thesaurus == USE_BGH) {
	get_code = get_bgh;
    }
    else if (Thesaurus == USE_NTT) {
	get_code = get_sm;
    }

    smd = (char *)get_code(exd);	/* ���������Ȥ�ʤ������ȤϤʤ� */
    smp = (char *)get_code(exp);

    if (smd && smp) {
	score = CalcSimilarity(smd, smp, 0);
    }

    if (smd) {
	free(smd);
    }
    if (smp) {
	free(smp);
    }
    return score;
}

/*==================================================================*/
	      int DeleteSpecifiedSM(char *sm, char *del)
/*==================================================================*/
{
    int i, j, flag, pos = 0;
    if (Thesaurus == USE_BGH) return 0;

    for (i = 0; sm[i]; i += SM_CODE_SIZE) {
	flag = 1;
	/* ��ͭ�ǤϤʤ��Ȥ����оݤȤ��� */
	if (sm[i] != '2') {
	    for (j = 0; del[j]; j += SM_CODE_SIZE) {
		if (!strncmp(sm+i+1, del+j+1, SM_CODE_SIZE-1)) {
		    flag = 0;
		    break;
		}
	    }
	}
	if (flag) {
	    strncpy(sm+pos, sm+i, SM_CODE_SIZE);
	    pos += SM_CODE_SIZE;
	}
    }
    *(sm+pos) = '\0';
    return 1;
}

/*==================================================================*/
	       int DeleteMatchedSM(char *sm, char *del)
/*==================================================================*/
{
    int i, j, flag, pos = 0;
    if (Thesaurus == USE_BGH) return 0;

    for (i = 0; sm[i]; i += SM_CODE_SIZE) {
	flag = 1;
	/* ��ͭ�ǤϤʤ��Ȥ������å� */
	if (sm[i] != '2') {
	    for (j = 0; del[j]; j += SM_CODE_SIZE) {
		if (_sm_match_score(sm+i, del+j, SM_NO_EXPAND_NE) > 0) {
		    flag = 0;
		    break;
		}
	    }
	}
	if (flag) {
	    strncpy(sm+pos, sm+i, SM_CODE_SIZE);
	    pos += SM_CODE_SIZE;
	}
    }
    *(sm+pos) = '\0';
    return 1;
}

/*==================================================================*/
float CalcSmWordSimilarity(char *smd, char *exp, char *del, int expand)
/*==================================================================*/
{

    char *(*get_code)();
    char *smp;
    float score = 0;

    if (Thesaurus == USE_BGH) {
	get_code = get_bgh;
    }
    else if (Thesaurus == USE_NTT) {
	get_code = get_sm;
    }

    if ((smp = (char *)get_code(exp)) == NULL) {
	return 0;
    }

    if (del) {
	DeleteSpecifiedSM(smp, del);
    }

    if (smd && smp[0]) {
	score = CalcSimilarity(smd, smp, expand);
    }

    free(smp);
    return score;
}

/*==================================================================*/
 float CalcWordsSimilarity(char *exd, char **exp, int num, int *pos)
/*==================================================================*/
{
    int i;
    float maxscore = 0, score;

    for (i = 0; i < num; i++) {
	score = CalcWordSimilarity(exd, *(exp+i));
	if (maxscore < score) {
	    maxscore = score;
	    *pos = i;
	}
    }

    return maxscore;
}

/*==================================================================*/
float CalcSmWordsSimilarity(char *smd, char **exp, int num, int *pos, char *del, int expand)
/*==================================================================*/
{
    int i;
    float maxscore = 0, score;

    for (i = 0; i < num; i++) {
	score = CalcSmWordSimilarity(smd, *(exp+i), del, expand);
	if (maxscore < score) {
	    maxscore = score;
	    *pos = i;
	}
    }

    return maxscore;
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
	       int sm_fix(BNST_DATA *bp, char *targets)
/*==================================================================*/
{
    int i, j, pos = 0;
    char *codes;

    if (bp->SM_code[0] == '\0') {
	return FALSE;
    }

    codes = bp->SM_code;

    for (i = 0; *(codes+i); i += SM_CODE_SIZE) {
	for (j = 0; *(targets+j); j += SM_CODE_SIZE) {
	    if (_sm_match_score(targets+j, codes+i, SM_NO_EXPAND_NE) > 0) {
		strncpy(codes+pos, codes+i, SM_CODE_SIZE);
		pos += SM_CODE_SIZE;
		break;
	    }
	}
    }

    /* match ���ʤ����äƤɤ�ʤȤ�? */
    if (pos != 0) {
	*(codes+pos) = '\0';
	bp->SM_num = strlen(codes)/SM_CODE_SIZE;
    }
    return TRUE;
}

/*==================================================================*/
	       int sm_all_match(char *c, char *target)
/*==================================================================*/
{
    char *p, flag = 0;

    /* ��ͭ̾��ΤȤ��ʳ��ǡ����٤Ƥΰ�̣°�������֤Ǥ���� TRUE */
    for (p = c;*p; p+=SM_CODE_SIZE) {
	/* ��ͭ̾��ΤȤ���Τ��� */
	if (*p == '2') {
	    continue;
	}

	/* ��̣�ǤΥ����å� */
	if (!comp_sm(target, p, 1)) {
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

    if (sm_all_match(bp->SM_code, "1128********")) { /* ���֤Υ����� */
	assign_cfeature(&(bp->f), "������");
	if (!check_feature(bp->f, "����")) {
	    assign_cfeature(&(bp->f), "����");
	}
    }
}

/*==================================================================*/
	      void assign_sm_aux_feature(BNST_DATA *bp)
/*==================================================================*/
{
    /* <����>°������Ϳ���� */
    assign_time_feature(bp);

    /* <���>°������Ϳ���� */
    if (sm_all_match(bp->SM_code, "11**********")) {
	assign_cfeature(&(bp->f), "���");
    }
}

/*====================================================================
                               END
====================================================================*/
