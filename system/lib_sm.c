/*====================================================================

			 NTT  �����ץ����

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

char  		cont_str[DBM_CON_MAX];

/*==================================================================*/
			   void init_ntt()
/*==================================================================*/
{
    char *filename;

    /***  �ǡ����١��������ץ�  ***/
    
    /* ñ�� <=> ��̣�ǥ����� */

    // �ե�����̾����ꤹ��
    if (DICT[SM_DB]) {   
	filename = check_dict_filename(DICT[SM_DB], TRUE);  // .knprc ���������Ƥ���Ȥ�   �� SM_ADD_DB �� const.h ���������Ƥ���
	                                                    //                                  DICT[SM_ADD_DB] �� configfile.c �ǻ��ꤵ��Ƥ���
    }
    else {
	filename = check_dict_filename(SM_DB_NAME, FALSE);  // .knprc ���������Ƥ��ʤ��Ȥ� �� path.h �� default��(SM_DB_NAME) ��Ȥ�
    }

    if (OptDisplay == OPT_DEBUG) {
	fprintf(Outfp, "Opening %s ... ", filename);
    }

    if ((sm_db = DB_open(filename, O_RDONLY, 0)) == NULL) {
	if (OptDisplay == OPT_DEBUG) {
	    fputs("failed.\n", Outfp);
	}
	SMExist = FALSE;
#ifdef DEBUG
	fprintf(stderr, ";; Cannot open NTT word dictionary <%s>.\n", filename);
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
    if (Thesaurus == USE_NTT) {
	if (DICT[SM2CODE_DB]) {
	    filename = check_dict_filename(DICT[SM2CODE_DB], TRUE);
	}
	else {
	    filename = check_dict_filename(SM2CODE_DB_NAME, FALSE);
	}

	if (OptDisplay == OPT_DEBUG) {
	    fprintf(Outfp, "Opening %s ... ", filename);
	}

	if ((sm2code_db = DB_open(filename, O_RDONLY, 0)) == NULL) {
	    if (OptDisplay == OPT_DEBUG) {
		fputs("failed.\n", Outfp);
	    }
	    SM2CODEExist = FALSE;
#ifdef DEBUG
	    fprintf(stderr, ";; Cannot open NTT sm dictionary <%s>.\n", filename);
#endif
	}
	else {
	    if (OptDisplay == OPT_DEBUG) {
		fputs("done.\n", Outfp);
	    }
	    SM2CODEExist = TRUE;
	}
	free(filename);
    }

    /* ��̣�ǥ����� => ��̣�� */
    if (DICT[CODE2SM_DB]) {
	filename = check_dict_filename(DICT[CODE2SM_DB], TRUE);
    }
    else {
	filename = check_dict_filename(CODE2SM_DB_NAME, FALSE);
    }

    if (OptDisplay == OPT_DEBUG) {
	fprintf(Outfp, "Opening %s ... ", filename);
    }

    if ((code2sm_db = DB_open(filename, O_RDONLY, 0)) == NULL) {
	if (OptDisplay == OPT_DEBUG) {
	    fputs("failed.\n", Outfp);
	}
	CODE2SMExist = FALSE;
#ifdef DEBUG
	fprintf(stderr, ";; Cannot open NTT code2sm dictionary <%s>.\n", filename);
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
	filename = check_dict_filename(DICT[SMP2SMG_DB], TRUE);
    }
    else {
	filename = check_dict_filename(SMP2SMG_DB_NAME, FALSE);
    }

    if (OptDisplay == OPT_DEBUG) {
	fprintf(Outfp, "Opening %s ... ", filename);
    }

    if ((smp2smg_db = DB_open(filename, O_RDONLY, 0)) == NULL) {
	if (OptDisplay == OPT_DEBUG) {
	    fputs("failed.\n", Outfp);
	}
	SMP2SMGExist = FALSE;
#ifdef DEBUG
	fprintf(stderr, ";; Cannot open NTT smp smg table <%s>.\n", filename);
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
			   void close_ntt()
/*==================================================================*/
{
    if (SMExist == TRUE)
	DB_close(sm_db);

    if (SM2CODEExist == TRUE)
	DB_close(sm2code_db);

    if (SMP2SMGExist == TRUE)
	DB_close(smp2smg_db);
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
                 char *_get_ntt(char *cp, char *arg)
/*==================================================================*/
{

    /* �ǡ����١���������Ф��� code ��������� */
    int i, j, pos;
    char *code;

    code = db_get(sm_db, cp);
    if (code) {

	/* ��줿�顢�̤�� */
	if (strlen(code) > SM_CODE_SIZE*SM_ELEMENT_MAX) {
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
		if ((*arg && code[i] == *arg) ||	/* ���ꤵ�줿�ʻ� */
		    code[i] == '3' ||	/* ̾ */
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
 int sm_check_match_max(char *exd, char *exp, int expand, char *target)
/*==================================================================*/
{
    int i, j, step = SM_CODE_SIZE, flag;
    float score = 0, tempscore;

    /* �ɤ��餫������Υ����ɤ��ʤ��Ȥ� */
    if (!(exd && exp && *exd && *exp)) {
	return FALSE;
    }

    if (expand != SM_NO_EXPAND_NE) {
	expand = SM_EXPAND_NE_DATA;
    }

    /* ����ޥå������������ */
    for (j = 0; exp[j]; j+=step) {
	for (i = 0; exd[i]; i+=step) {
	    tempscore = ntt_code_match(exp+j, exd+i, expand);
	    if (tempscore > score) {
		score = tempscore;
		/* ξ�� target ��̣�Ǥ�°�� */
		if (sm_match_check(target, exd) && sm_match_check(target, exp)) {
		    flag = TRUE;
		}
		else {
		    flag = FALSE;
		}
	    }
	}
    }
    return flag;
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

    if (!check_feature(bp->f, "����") && 
	sm_all_match(bp->SM_code, sm2code("����"))) {
	assign_cfeature(&(bp->f), "����Ƚ��");
	assign_cfeature(&(bp->f), "����");
    }
}

/*==================================================================*/
	      void assign_sm_aux_feature(BNST_DATA *bp)
/*==================================================================*/
{
    /* �롼������줿 */

    if (Thesaurus != USE_NTT) {
	return;
    }

    /* <����>°������Ϳ���� */
    assign_time_feature(bp);

    /* <���>°������Ϳ���� */
    if (sm_all_match(bp->SM_code, sm2code("���"))) {
	assign_cfeature(&(bp->f), "���");
    }
}

/*==================================================================*/
	      int delete_matched_sm(char *sm, char *del)
/*==================================================================*/
{
    int i, j, flag, pos = 0;

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
	     int delete_specified_sm(char *sm, char *del)
/*==================================================================*/
{
    int i, j, flag, pos = 0;

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
		void fix_sm_person(SENTENCE_DATA *sp)
/*==================================================================*/
{
    int i;

    if (Thesaurus != USE_NTT) return;

    /* ��̾�ΤȤ�: 
       o ����̾���ηϤ�<����>�ʲ��ΰ�̣�Ǥ���
       o ��ͭ̾���ηϤΰ�̣�Ǥΰ���̾���ηϤؤΥޥåԥ󥰤�ػ� */

    for (i = 0; i < sp->Bnst_num; i++) {
	if (check_feature((sp->bnst_data+i)->f, "��̾")) {
	    /* ��ͭ�ΰ�̣�Ǥ����Ĥ����� */
	    delete_matched_sm((sp->bnst_data+i)->SM_code, "100*********"); /* <����>�ΰ�̣�� */
	    assign_cfeature(&((sp->bnst_data+i)->f), "�Ը�ͭ����Ÿ���ػ�");
	}
    }
}

/*==================================================================*/
      void fix_sm_place(SENTENCE_DATA *sp, CF_PRED_MGR *cpm_ptr)
/*==================================================================*/
{
    /* ���Τ������Ѳ�����
       ���ߤ� <���> �Τ� */

    int i, num;

    if (Thesaurus != USE_NTT) return;

    for (i = 0; i < cpm_ptr->cf.element_num; i++) {
	num = cpm_ptr->cmm[0].result_lists_d[0].flag[i];
	/* ��ά�����ǤǤϤʤ�������Ƥ����ä��Ȥ� */
	if (cpm_ptr->elem_b_num[i] > -2 && 
	    num >= 0 && 
	    MatchPP(cpm_ptr->cmm[0].cf_ptr->pp[num][0], "��") && 
	    cf_match_element(cpm_ptr->cmm[0].cf_ptr->sm[num], "���", TRUE)) {
	    /* ��ͭ�������Ѵ����Ƥ��� */
	    merge_smp2smg((BNST_DATA *)cpm_ptr->elem_b_ptr[i]);
	    /* <���>�Τߤ˸��ꤹ�� */
	    sm_fix((BNST_DATA *)cpm_ptr->elem_b_ptr[i], "101*********20**********");
	    assign_cfeature(&(cpm_ptr->elem_b_ptr[i]->f), "�Ը�ͭ����Ÿ���ػ�");
	    assign_cfeature(&(cpm_ptr->elem_b_ptr[i]->f), "�����");
	    break;
	}
    }
}

/*==================================================================*/
   void assign_ga_subject(SENTENCE_DATA *sp, CF_PRED_MGR *cpm_ptr)
/*==================================================================*/
{
    int i, num;

    if (Thesaurus != USE_NTT) return;

    for (i = 0; i < cpm_ptr->cf.element_num; i++) {
	num = cpm_ptr->cmm[0].result_lists_d[0].flag[i];
	/* ��ά�����ǤǤϤʤ�������Ƥ����ä��Ȥ� */
	if (cpm_ptr->elem_b_num[i] > -2 && 
	    cpm_ptr->cmm[0].result_lists_d[0].flag[i] >= 0 && 
	    MatchPP(cpm_ptr->cmm[0].cf_ptr->pp[num][0], "��")) {
	    /* o ���Ǥ˼�����Ϳ����Ƥ��ʤ�
	       o <����> �ǤϤʤ� (<����>�ΤȤ���̣°�����ʤ�)
	       o <�Ѹ�:ư>�Ǥ��� 
	       o �ʥե졼�ब<����>����, <���ν�>�ǤϤʤ�
	       o ����¦����̣�Ǥ��ʤ�����(��ͭ̾��ȿ���)
	         <���ʪ> or <��>�Ȥ�����̣�Ǥ��� (�Ĥޤꡢ<���Ū�ط�>�����ǤϤʤ�)
	    */
	    if (!check_feature(cpm_ptr->elem_b_ptr[i]->f, "������Ϳ") && 
		!check_feature(cpm_ptr->elem_b_ptr[i]->f, "����") && 
		check_feature(cpm_ptr->pred_b_ptr->f, "�Ѹ�:ư") && 
		cf_match_element(cpm_ptr->cmm[0].cf_ptr->sm[num], "����", TRUE) && 
		(cpm_ptr->elem_b_ptr[i]->SM_num == 0 || 
		 /* (!(cpm_ptr->cmm[0].cf_ptr->etcflag & CF_GA_SEMI_SUBJECT) && ( */
		 sm_match_check(sm2code("����"), cpm_ptr->elem_b_ptr[i]->SM_code) || 
		 sm_match_check(sm2code("��̾"), cpm_ptr->elem_b_ptr[i]->SM_code) || /* �ȿ�̾, ��̾�Ϥ��Ǥ˼��� */
		 sm_match_check(sm2code("���ʪ"), cpm_ptr->elem_b_ptr[i]->SM_code) || 
		 sm_match_check(sm2code("��"), cpm_ptr->elem_b_ptr[i]->SM_code))) {
		assign_sm((BNST_DATA *)cpm_ptr->elem_b_ptr[i], "����");
		assign_cfeature(&(cpm_ptr->elem_b_ptr[i]->f), "������Ϳ");
	    }
	    break;
	}
    }
}

/*====================================================================
                               END
====================================================================*/
