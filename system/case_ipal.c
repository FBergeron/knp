/*====================================================================

		       �ʹ�¤����: �ʥե졼��¦

                                               S.Kurohashi 93. 5.31

    $Id$
====================================================================*/
#include "knp.h"

FILE *cf_fp;
DBM_FILE cf_db;
FILE *cf_noun_fp;
DBM_FILE cf_noun_db;
DBM_FILE cf_sim_db;
DBM_FILE cf_case_db;
DBM_FILE cf_ex_db;
DBM_FILE case_db;
DBM_FILE cfp_db;
DBM_FILE renyou_db;
DBM_FILE adverb_db;
DBM_FILE para_db;
DBM_FILE noun_co_db;
DBM_FILE chi_case_db;

CASE_FRAME 	*Case_frame_array = NULL; 	/* �ʥե졼�� */
int 	   	Case_frame_num;			/* �ʥե졼��� */
int 	   	MAX_Case_frame_num = 0;		/* ����ʥե졼��� */

char *db_buf = NULL;
int db_buf_size = 0;

CF_FRAME CF_frame;
int MAX_cf_frame_length = 0;
unsigned char *cf_str_buf;

int	CFExist;
int	CFNounExist;
int	CFSimExist;
int	CFCaseExist;
int	CFExExist;
int	CaseExist;
int	CfpExist;
int	RenyouExist;
int	AdverbExist;
int	ParaExist;
int	NounCoExist;
int     CHICaseExist;

int	PrintDeletedSM = 0;

/*==================================================================*/
	   void init_cf_structure(CASE_FRAME *p, int size)
/*==================================================================*/
{
    memset(p, 0, sizeof(CASE_FRAME)*size);
}

/*==================================================================*/
			  void realloc_cf()
/*==================================================================*/
{
    Case_frame_array = (CASE_FRAME *)realloc_data(Case_frame_array, 
						  sizeof(CASE_FRAME)*(MAX_Case_frame_num+ALLOCATION_STEP), 
						  "realloc_cf");
    init_cf_structure(Case_frame_array+MAX_Case_frame_num, ALLOCATION_STEP);
    MAX_Case_frame_num += ALLOCATION_STEP;
}

/*==================================================================*/
			    void init_cf()
/*==================================================================*/
{
    char *index_db_filename, *data_filename;

    if (DICT[CF_DATA]) {
	data_filename = check_dict_filename(DICT[CF_DATA], TRUE);
    }
    else {
	data_filename = check_dict_filename(CF_DAT_NAME, FALSE);
    }

    if (DICT[CF_INDEX_DB]) {
	index_db_filename = check_dict_filename(DICT[CF_INDEX_DB], TRUE);
    }
    else {
	index_db_filename = check_dict_filename(CF_DB_NAME, FALSE);
    }

    if (OptDisplay == OPT_DEBUG) {
	fprintf(Outfp, "Opening %s ... ", data_filename);
    }

    if ((cf_fp = fopen(data_filename, "rb")) == NULL) {
	if (OptDisplay == OPT_DEBUG) {
	    fputs("failed.\n", Outfp);
	}
#ifdef DEBUG
	fprintf(stderr, ";; Cannot open CF DATA <%s>.\n", data_filename);
#endif
	CFExist = FALSE;
    }
    else if ((cf_db = DB_open(index_db_filename, O_RDONLY, 0)) == NULL) {
	if (OptDisplay == OPT_DEBUG) {
	    fprintf(Outfp, "done.\nOpening %s ... failed.\n", index_db_filename);
	}
	fprintf(stderr, ";; Cannot open CF INDEX Database <%s>.\n", index_db_filename);
	/* �ʥե졼�� DATA ���ɤ��Τˡ�DB ���ɤ�ʤ��Ȥ��Ͻ���� */
	exit(1);
    } 
    else {
	if (OptDisplay == OPT_DEBUG) {
	    fprintf(Outfp, "done.\nOpening %s ... done.\n", index_db_filename);
	}
	CFExist = TRUE;
    }

    free(data_filename);
    free(index_db_filename);

    /* �ʥե졼�������DB (cfsim.db) */
    cf_sim_db = open_dict(CF_SIM_DB, CF_SIM_DB_NAME, &CFSimExist);

    /* �ʳ�ΨDB (cfcase.db) */
    cf_case_db = open_dict(CF_CASE_DB, CF_CASE_DB_NAME, &CFCaseExist);

    /* �����ΨDB (cfex.db) *
    cf_ex_db = open_dict(CF_EX_DB, CF_EX_DB_NAME, &CFExExist);
    */
    CFExExist = FALSE;

    /* �ʥե졼�������ΨDB (cfp.db) */
    cfp_db = open_dict(CFP_DB, CFP_DB_NAME, &CfpExist);

    /* �ʲ���ΨDB (case.db) */
    case_db = open_dict(CASE_DB, CASE_DB_NAME, &CaseExist);

    /* Ϣ�ѳ�ΨDB (renyou.db) */
    renyou_db = open_dict(RENYOU_DB, RENYOU_DB_NAME, &RenyouExist);

    /* �����ΨDB (adverb.db) */
    adverb_db = open_dict(ADVERB_DB, ADVERB_DB_NAME, &AdverbExist);

    /* �����ΨDB (para.db) */
    para_db = open_dict(PARA_DB, PARA_DB_NAME, &ParaExist);

    /* ̾�춦����ΨDB (noun-co.db) */
    noun_co_db = open_dict(NOUN_CO_DB, NOUN_CO_DB_NAME, &NounCoExist);

    /* Chinese case-frame DB (chicase.db) */
    chi_case_db = open_dict(CHI_CASE_DB, CHI_CASE_DB_NAME, &CHICaseExist);
}

/*==================================================================*/
			 void init_noun_cf()
/*==================================================================*/
{
    char *index_db_filename, *data_filename;

    if (DICT[CF_NOUN_DATA]) {
	data_filename = check_dict_filename(DICT[CF_NOUN_DATA], TRUE);
    }
    else {
	data_filename = check_dict_filename(CF_NOUN_DAT_NAME, FALSE);
    }

    if (DICT[CF_NOUN_INDEX_DB]) {
	index_db_filename = check_dict_filename(DICT[CF_NOUN_INDEX_DB], TRUE);
    }
    else {
	index_db_filename = check_dict_filename(CF_NOUN_DB_NAME, FALSE);
    }

    if (OptDisplay == OPT_DEBUG) {
	fprintf(Outfp, "Opening %s ... ", data_filename);
    }

    if ((cf_noun_fp = fopen(data_filename, "rb")) == NULL) {
	if (OptDisplay == OPT_DEBUG) {
	    fputs("failed.\n", Outfp);
	}
#ifdef DEBUG
	fprintf(stderr, ";; Cannot open CF(noun) DATA <%s>.\n", data_filename);
#endif
	CFNounExist = FALSE;
    }
    else if ((cf_noun_db = DB_open(index_db_filename, O_RDONLY, 0)) == NULL) {
	if (OptDisplay == OPT_DEBUG) {
	    fprintf(Outfp, "done.\nOpening %s ... failed.\n", index_db_filename);
	}
	fprintf(stderr, ";; Cannot open CF(noun) INDEX Database <%s>.\n", index_db_filename);
	/* �ʥե졼�� DATA ���ɤ��Τˡ�DB ���ɤ�ʤ��Ȥ��Ͻ���� */
	exit(1);
    } 
    else {
	if (OptDisplay == OPT_DEBUG) {
	    fprintf(Outfp, "done.\nOpening %s ... done.\n", index_db_filename);
	}
	CFNounExist = TRUE;
    }

    free(data_filename);
    free(index_db_filename);
}

/*==================================================================*/
		 void clear_mgr_cf(SENTENCE_DATA *sp)
/*==================================================================*/
{
    int i, j;

    for (i = 0; i < CPM_MAX; i++) {
	for (j = 0; j < CF_ELEMENT_MAX; j++) {
	    free(sp->Best_mgr->cpm[i].cf.ex[j]);
	    sp->Best_mgr->cpm[i].cf.ex[j] = NULL;

	    free(sp->Best_mgr->cpm[i].cf.sm[j]);
	    sp->Best_mgr->cpm[i].cf.sm[j] = NULL;

	    free(sp->Best_mgr->cpm[i].cf.ex_list[j][0]);
	    free(sp->Best_mgr->cpm[i].cf.ex_list[j]);
	    free(sp->Best_mgr->cpm[i].cf.ex_freq[j]);
	}
    }
}

/*==================================================================*/
		   void init_mgr_cf(TOTAL_MGR *tmp)
/*==================================================================*/
{
    int i;

    for (i = 0; i < CPM_MAX; i++) {
	init_case_frame(&tmp->cpm[i].cf);
    }
}

/*==================================================================*/
	    void init_case_analysis_cpm(SENTENCE_DATA *sp)
/*==================================================================*/
{
    if (OptAnalysis == OPT_CASE || 
	OptAnalysis == OPT_CASE2 ||
	OptUseNCF) {

	/* �ʥե졼���ΰ���� */
	Case_frame_array = (CASE_FRAME *)malloc_data(sizeof(CASE_FRAME)*ALL_CASE_FRAME_MAX, 
						     "init_case_analysis_cpm");
	MAX_Case_frame_num = ALL_CASE_FRAME_MAX;
	init_cf_structure(Case_frame_array, MAX_Case_frame_num);

	/* Best_mgr��cpm�ΰ���� */
	init_mgr_cf(sp->Best_mgr);

	/* ̾��-��̣��HASH�ν���� */
	memset(smlist, 0, sizeof(SMLIST)*TBLSIZE);
    }
}

/*==================================================================*/
			   void close_cf()
/*==================================================================*/
{
    if (CFExist == TRUE) {
	fclose(cf_fp);
	DB_close(cf_db);
    }
}

/*==================================================================*/
			 void close_noun_cf()
/*==================================================================*/
{
    if (CFNounExist == TRUE) {
	fclose(cf_noun_fp);
	DB_close(cf_noun_db);
    }
}

/*==================================================================*/
	char *get_ipal_address(unsigned char *word, int flag)
/*==================================================================*/
{
    if (flag == CF_PRED) {
	if (CFExist == FALSE) {
	    return NULL;
	}

	return db_get(cf_db, word);
    }
    else {
	if (CFNounExist == FALSE) {
	    return NULL;
	}

	return db_get(cf_noun_db, word);
    }
}

/*==================================================================*/
      CF_FRAME *get_ipal_frame(int address, int size, int flag)
/*==================================================================*/
{
    int i, c1, c2, count = 0;
    char *cp;
    FILE *fp;

    if (flag == CF_PRED) {
	fp = cf_fp;
    }
    else {
	fp = cf_noun_fp;
    }

    if (size > MAX_cf_frame_length) {
	MAX_cf_frame_length += ALLOCATION_STEP*((size-MAX_cf_frame_length)/ALLOCATION_STEP+1);
	CF_frame.DATA = 
	    (unsigned char *)realloc_data(CF_frame.DATA, 
					  sizeof(unsigned char)*MAX_cf_frame_length, 
					  "get_ipal_frame");
	cf_str_buf = 
	    (unsigned char *)realloc_data(cf_str_buf, 
					  sizeof(unsigned char)*MAX_cf_frame_length, 
					  "get_ipal_frame");
    }

    fseek(fp, (long)address, 0);
    if (fread(CF_frame.DATA, size, 1, fp) < 1) {
	fprintf(stderr, ";; Error in fread.\n");
	exit(1);
    }

    /* �ɤ�, ɽ��, ���������� */
    CF_frame.casenum = 0;
    for (i = 0; i < size-1; i++) {
	if (*(CF_frame.DATA+i) == '\0') {
	    if (count == 0)
		CF_frame.yomi = CF_frame.DATA+i+1;
	    else if (count == 1)
		CF_frame.hyoki = CF_frame.DATA+i+1;
	    else if (count == 2)
		CF_frame.feature = CF_frame.DATA+i+1;
	    else {
		if (count%3 == 0) {
		    CF_frame.cs[count/3-1].kaku_keishiki = CF_frame.DATA+i+1;
		    CF_frame.casenum++;
		    if (CF_frame.casenum > CASE_MAX_NUM) {
			fprintf(stderr, ";; # of cases is more than MAX (%d).\n", CASE_MAX_NUM);
		    }
		}
		else if (count%3 == 1)
		    CF_frame.cs[count/3-1].meishiku = CF_frame.DATA+i+1;
		else if (count%3 == 2)
		    CF_frame.cs[count/3-1].imisosei = CF_frame.DATA+i+1;
	    }
	    count++;
	}
    }

    count = 0;
    CF_frame.voice = 0;
    CF_frame.etcflag = CF_NORMAL;
    if (*CF_frame.feature) {
	char *string, *token, *buf;
	string = strdup(CF_frame.feature);
	token = strtok(string, " ");
	while (token) {
	    if (!strcmp(token, "�¥ե졼��")) {
		CF_frame.etcflag |= CF_SUM;
	    }
	    else if (!strcmp(token, "�ʥե졼���Ѳ�")) {
		CF_frame.etcflag |= CF_CHANGE;
	    }
	    else if (!strcmp(token, "�����")) {
		CF_frame.voice |= CF_CAUSATIVE_WO;
	    }
	    else if (!strcmp(token, "�˻���")) {
		CF_frame.voice |= CF_CAUSATIVE_NI;
	    }
	    else if (!strcmp(token, "ľ��1")) {
		CF_frame.voice |= CF_PASSIVE_1;
	    }
	    else if (!strcmp(token, "ľ��2")) {
		CF_frame.voice |= CF_PASSIVE_2;
	    }
	    else if (!strcmp(token, "�ּ�")) {
		CF_frame.voice |= CF_PASSIVE_I;
	    }
	    else if (!strcmp(token, "��ǽ")) {
		CF_frame.voice |= CF_POSSIBLE;
	    }
	    else if (!strcmp(token, "º��")) {
		CF_frame.voice |= CF_POLITE;
	    }
	    else if (!strcmp(token, "��ȯ")) {
		CF_frame.voice |= CF_SPONTANE;
		;
	    }
	    /* merged cases */
	    else if ((cp = strstr(token, "��")) != NULL) {
		buf = strdup(token);
		cp = buf+(cp-token);
		*cp = '\0';
		/* if (!strncmp(buf+strlen(buf)-2, "��", 2)) *(buf+strlen(buf)-2) = '\0';
		if (!strncmp(cp+strlen(cp+2), "��", 2)) *(cp+strlen(cp+2)) = '\0'; */
		c1 = pp_kstr_to_code(buf);
		c2 = pp_kstr_to_code(cp+2);
		free(buf);

		if (c1 == END_M || c2 == END_M) {
		    fprintf(stderr, ";; Can't understand <%s> as merged cases\n", token);
		}
		/* �������å� */
		else if (count >= CF_ELEMENT_MAX - 1) {
		    break;
		}
		else {
		    /* �����������ʤ���������� */
		    if (c1 > c2) {
			CF_frame.samecase[count][0] = c2;
			CF_frame.samecase[count][1] = c1;
		    }
		    else {
			CF_frame.samecase[count][0] = c1;
			CF_frame.samecase[count][1] = c2;
		    }
		    count++;
		}
	    }
	    token = strtok(NULL, " ");
	}
	free(string);
    }

    CF_frame.samecase[count][0] = END_M;
    CF_frame.samecase[count][1] = END_M;

    if (cp = strchr(CF_frame.DATA, ':')) {
	strncpy(CF_frame.pred_type, cp+1, 2);
	CF_frame.pred_type[2] = '\0';
    }
    else {
	CF_frame.pred_type[0] = '\0';
    }

    return &CF_frame;
}

/*==================================================================*/
unsigned char *extract_ipal_str(unsigned char *dat, unsigned char *ret, int flag)
/*==================================================================*/
{
    int freq;

    /* flag == TRUE: �����դ����֤� */

    if (*dat == '\0' || !strcmp(dat, "��")) {
	return NULL;
    }

    while (1) {
	if (*dat == '\0') {
	    *ret = '\0';
	    return dat;
	}
	/* ���٤����Ҥ��Ƥ����� */
	else if (*dat == ':') {
	    if (flag) {
		*ret++ = *dat;
	    }
	    /* flag == FALSE: ���٤��֤��ʤ� */
	    else {
		*ret++ = '\0';	/* ':' -> '\0' */
	    }
	    dat++;
	}
	/* ����Ǥ��ڤ� */
	else if (*dat == ' ') {
	    *ret = '\0';
	    return dat+1;
	}
	else if (*dat < 0x80) { /* OK? */
	    *ret++ = *dat++;
	}
	else if (!strncmp(dat, "��", strlen("��")) || 
		 !strncmp(dat, "��", strlen("��")) ||
		 !strncmp(dat, "��", strlen("��")) ||
		 !strncmp(dat, "��", strlen("��"))) {
	    *ret = '\0';
	    return dat+strlen("��"); /* OK? */
	}
	else {
	    *ret++ = *dat++;
	    *ret++ = *dat++;
	}
    }
}

/*==================================================================*/
int _make_ipal_cframe_pp(CASE_FRAME *c_ptr, unsigned char *cp, int num, int flag)
/*==================================================================*/
{
    /* ������ɤߤ��� */

    unsigned char *point;
    int pp_num = 0;

    /* ľ���� */
    if (*(cp+strlen(cp)-1) == '*') {
	c_ptr->adjacent[num] = TRUE;
	*(cp+strlen(cp)-1) = '\0';
    }
    else if (!strcmp(cp+strlen(cp)-strlen("��"), "��")) {
	c_ptr->adjacent[num] = TRUE;
	*(cp+strlen(cp)-2) = '\0';
    }

    /* Ǥ�ճ� */
    if (!strcmp(cp+strlen(cp)-strlen("��"), "��")) {
	c_ptr->oblig[num] = FALSE;
    }
    else {
	/* ̤���� (�ҤȤĤ�γʤ򸫤Ʒ���) */
	c_ptr->oblig[num] = END_M;
    }

    point = cp; 
    while ((point = extract_ipal_str(point, cf_str_buf, FALSE))) {
	if (pp_num == 0 && c_ptr->oblig[num] == END_M) {
	    if (str_eq(cf_str_buf, "��") || 
		str_eq(cf_str_buf, "��") || 
		str_eq(cf_str_buf, "��") || 
		str_eq(cf_str_buf, "��") || 
		str_eq(cf_str_buf, "���")) {
		c_ptr->oblig[num] = TRUE;
	    }
	    else {
		c_ptr->oblig[num] = FALSE;
	    }
	}

	if (flag == CF_PRED) {
	    c_ptr->pp[num][pp_num] = pp_kstr_to_code(cf_str_buf);
	    if (c_ptr->pp[num][pp_num] == END_M) {
		fprintf(stderr, ";; Unknown case (%s) in PP!\n", cf_str_buf);
		exit(1);
	    }
	    c_ptr->pp_str[num] = NULL;
	}
	else {
	    c_ptr->pp[num][pp_num] = 0;
	    c_ptr->pp_str[num] = strdup(cf_str_buf);
	    c_ptr->oblig[num] = TRUE;
	}

	pp_num++;
    }

    c_ptr->pp[num][pp_num] = END_M;
    return TRUE;
}

/*==================================================================*/
	 char *append_str(char **dst, char *src, char *delim)
/*==================================================================*/
{
    if (src && *src) {
	if (*dst == NULL) {
	    *dst = strdup(src);
	}
	else {
	    *dst = (char *)realloc_data(*dst, strlen(*dst) + (delim ? strlen(delim) : 0) + strlen(src) + 1, "append_str");
	    if (delim) {
		strcat(*dst, delim);
	    }
	    strcat(*dst, src);
	}
    }

    return *dst;
}

/*==================================================================*/
void _make_ipal_cframe_sm(CASE_FRAME *c_ptr, unsigned char *cp, int num, int flag)
/*==================================================================*/
{
    /* ��̣�ޡ������ɤߤ��� */

    unsigned char *point;
    int size, sm_num = 0, sm_print_num = 0, mlength, sm_delete_sm_max = 0, sm_specify_sm_max = 0;
    char buf[SM_ELEMENT_MAX * SM_CODE_SIZE], *sm_delete_sm = NULL, *sm_specify_sm = NULL, *temp, *str;

    if (*cp == '\0') {
	return;
    }

    if (flag & USE_BGH) {
	size = BGH_CODE_SIZE;
    }
    else if (flag & USE_NTT) {
	size = SM_CODE_SIZE;
    }
    else {
	return;
    }

    str = strdup(cp);
    *str = '\0';
    point = cp;
    buf[0] = '\0';
    while ((point = extract_ipal_str(point, cf_str_buf, FALSE))) {
	/* ��̣������ */
        if (cf_str_buf[0] == '+') {
	    if (c_ptr->sm_specify[num] == NULL) {
		c_ptr->sm_specify_size[num] = SM_ELEMENT_MAX;
		c_ptr->sm_specify[num] = (char *)malloc_data(sizeof(char)*c_ptr->sm_specify_size[num] * size + 1, "_make_ipal_cframe_sm");
		*c_ptr->sm_specify[num] = '\0';

		if (flag & USE_NTT) {
		    sm_specify_sm_max = sizeof(char) * ALLOCATION_STEP;
		    sm_specify_sm = (char *)malloc_data(sm_specify_sm_max, "_make_ipal_cframe_sm");
		    strcpy(sm_specify_sm, "��̣������:");
		}
		else {
		    sm_specify_sm = strdup("��̣������");
		}
	    }
	    else if (c_ptr->sm_specify_num[num] >= c_ptr->sm_specify_size[num]) {
		c_ptr->sm_specify[num] = (char *)realloc_data(c_ptr->sm_specify[num], 
							      sizeof(char)*(c_ptr->sm_specify_size[num] <<= 1) * size + 1, "_make_ipal_cframe_sm");
	    }

	    /* code���񤤤Ƥ���Ȥ� */
	    if (cf_str_buf[1] == '1') {
		strcat(c_ptr->sm_specify[num], &cf_str_buf[1]);

		if (flag & USE_NTT) { /* ɽ���Ѥΰ�̣��̾�ؤ��Ѵ� (NTT�Τ�) */
		    temp = code2sm(&cf_str_buf[1]);
		    if (temp[0]) {
			/* -1 �ǤϤʤ��Τ� '/' ��ʬ */
			if (strlen(sm_specify_sm) + strlen(temp) > sm_specify_sm_max - 2) {
			    sm_specify_sm = (char *)realloc_data(sm_specify_sm, sm_specify_sm_max <<= 1, "_make_ipal_cframe_sm");
			}
			strcat(sm_specify_sm, "/");
			strcat(sm_specify_sm, temp);
		    }
		}
	    }
	    else if (flag & USE_NTT) { /* ��̣��̾�Ǥλ��� (NTT�Τ�) */
		strcat(c_ptr->sm_specify[num], sm2code(&cf_str_buf[1]));
	    }
	    c_ptr->sm_specify_num[num]++;
	}
	/* �ȤäƤϤ����ʤ���̣�� */
        else if (cf_str_buf[0] == '-') {
	    if (c_ptr->sm_delete[num] == NULL) {
		c_ptr->sm_delete_size[num] = SM_ELEMENT_MAX;
		c_ptr->sm_delete[num] = (char *)malloc_data(sizeof(char)*c_ptr->sm_delete_size[num] * size + 1, "_make_ipal_cframe_sm");
		*c_ptr->sm_delete[num] = '\0';

		if (PrintDeletedSM && (flag & USE_NTT)) {
		    sm_delete_sm_max = sizeof(char) * ALLOCATION_STEP;
		    sm_delete_sm = (char *)malloc_data(sm_delete_sm_max, "_make_ipal_cframe_sm");
		    strcpy(sm_delete_sm, "��̣�Ǻ��:");
		}
		else {
		    sm_delete_sm = strdup("��̣�Ǻ��");
		}
	    }
	    else if (c_ptr->sm_delete_num[num] >= c_ptr->sm_delete_size[num]) {
		c_ptr->sm_delete[num] = (char *)realloc_data(c_ptr->sm_delete[num], sizeof(char)*(c_ptr->sm_delete_size[num] <<= 1) * size + 1, 
							     "_make_ipal_cframe_sm");
	    }

	    /* code���񤤤Ƥ���Ȥ� */
	    if (cf_str_buf[1] == '1') {
		strcat(c_ptr->sm_delete[num], &cf_str_buf[1]);

		if (PrintDeletedSM && (flag & USE_NTT)) { /* ɽ���Ѥΰ�̣��̾�ؤ��Ѵ� (NTT�Τ�) */
		    temp = code2sm(&cf_str_buf[1]);
		    if (temp[0]) {
			/* -1 �ǤϤʤ��Τ� '/' ��ʬ */
			if (strlen(sm_delete_sm)+strlen(temp) > sm_delete_sm_max-2) {
			    sm_delete_sm = (char *)realloc_data(sm_delete_sm, sm_delete_sm_max <<= 1, "_make_ipal_cframe_sm");
			}
			strcat(sm_delete_sm, "/");
			strcat(sm_delete_sm, temp);
		    }
		}
	    }
	    else if (flag & USE_NTT) { /* ��̣��̾�Ǥλ��� (NTT�Τ�) */
		strcat(c_ptr->sm_delete[num], sm2code(&cf_str_buf[1]));
	    }
	    c_ptr->sm_delete_num[num]++;
	}
	/* ���̤ΰ�̣�� */
	else {
	    sm_num++;
	    sm_print_num++;
	    if (sm_num >= SM_ELEMENT_MAX){
		break;
	    }

	    if (!strncmp(cf_str_buf, "����", strlen("����"))) {
		/* �����<����>�ΤȤ�������ʤ� */
		if (sm_num > 1 && !strncmp(&buf[size*(sm_num-2)], sm2code("����"), size)) {
		    sm_num--;
		}
		else {
		    strcat(buf, sm2code("����"));
		}
	    }
	    else if (!strncmp(cf_str_buf, "���ν�", strlen("���ν�"))) {
		strcat(buf, sm2code("����"));
		if (MatchPP(c_ptr->pp[num][0], "��")) { /* ���ϡ����ʰʳ���<���ν�>��Ϳ���Ƥ�<����>��Ʊ���ˤʤ� */
		    c_ptr->etcflag |= CF_GA_SEMI_SUBJECT;
		}
	    }
	    else {
		strcat(buf, sm2code(cf_str_buf));
	    }
 	
	    if ((flag & STOREtoCF) && 
		(EX_PRINT_NUM < 0 || sm_print_num <= EX_PRINT_NUM)) {
		if (str[0])	strcat(str, "/");
		strcat(str, cf_str_buf);
	    }
	}
    }

    if (buf[0]) {
	append_str(&(c_ptr->sm[num]), buf, NULL);
    }

    if (flag & STOREtoCF) {
	append_str(&(c_ptr->semantics[num]), str, "/");
	append_str(&(c_ptr->semantics[num]), sm_delete_sm, "/");
	append_str(&(c_ptr->semantics[num]), sm_specify_sm, "/");
	if (EX_PRINT_NUM >= 0 && sm_print_num > EX_PRINT_NUM) {
	    append_str(&(c_ptr->semantics[num]), "...", "/");
	}
    }
    free(str);
    if (sm_delete_sm) free(sm_delete_sm);
    if (sm_specify_sm) free(sm_specify_sm);
}

/*==================================================================*/
		  int split_freq(unsigned char *cp)
/*==================================================================*/
{
    int freq;

    while (1) {
	if (*cp == ':') {
	    sscanf(cp + 1, "%d", &freq);
	    *cp = '\0';
	    return freq;
	}
	else if (*cp == '\0') {
	    return 1;
	}
	cp++;
    }
    return 0;
}

/*==================================================================*/
void _make_ipal_cframe_ex(CASE_FRAME *c_ptr, unsigned char *cp, int num, 
			  int flag, int fflag)
/*==================================================================*/
{
    /* ����ɤߤ��� */

    /* fflag: ����1��Ȥ����ɤ���
              �ʤ����δط��ΤȤ������Ȥ� */

    unsigned char *point, *point2;
    int max, count = 0, thesaurus = USE_NTT, freq, over_flag = 0, agent_count = 0;
    int sub_agent_flag = 0;
    char *code, **destination, *buf;

    c_ptr->freq[num] = 0;

    if (*cp == '\0') {
	return;
    }

    /* �����꥽�����ˤ�äƴؿ��ʤɤ򥻥å� */
    destination = &c_ptr->ex[num];
    if (flag & USE_BGH) {
	thesaurus = USE_BGH;
	max = EX_ELEMENT_MAX*BGH_CODE_SIZE;
    }
    else if (flag & USE_NTT) {
	thesaurus = USE_NTT;
	max = SM_ELEMENT_MAX*SM_CODE_SIZE;
    }

    /* �����ͤ��ʤ��Ȥ����ޤ��� */
    buf = (char *)malloc_data(sizeof(char)*max, "_make_ipal_cframe_ex");

    point = cp;
    *buf = '\0';
    while ((point = extract_ipal_str(point, cf_str_buf, TRUE))) {
	point2 = cf_str_buf;

	/* ���٤���� */
	freq = split_freq(point2);

	if (!strcmp(point2, "<���ν�>")) {
	    sub_agent_flag = 1;
	    continue;
	}

	/* fflag == TRUE: ����1���� */
	if (!(OptCaseFlag & OPT_CASE_USE_PROBABILITY) && 
	    fflag && freq < 2) {
	    continue;
	}

	/* �֣��Σ¡פΡ֣¡פ��������
	for (i = strlen(point2) - 2; i > 2; i -= 2) {
	    if (!strncmp(point2+i-2, "��", 2)) {
		point2 += i;
		break;
	    }
	}
	*/

	if (*point2 != '\0') {
	    code = get_str_code(point2, thesaurus);
	    if (code) {
		/* <����>�Υ����å� */
		if (cf_match_element(code, "����", FALSE)) {
		    agent_count += freq;
		}

		if (!over_flag) {
		    if (strlen(buf) + strlen(code) >= max) {
			/* fprintf(stderr, "Too many EX <%s> (%2dth).\n", cf_str_buf, count); */
			over_flag = 1;
		    }
		    else {
			strcat(buf, code);
		    }
		}
		free(code);
	    }

	    if (c_ptr->ex_size[num] == 0) {
		c_ptr->ex_size[num] = 10;	/* ������ݿ� */
		c_ptr->ex_list[num] = (char **)malloc_data(sizeof(char *)*c_ptr->ex_size[num], 
							   "_make_ipal_cframe_ex");
		c_ptr->ex_freq[num] = (int *)malloc_data(sizeof(int)*c_ptr->ex_size[num], 
							 "_make_ipal_cframe_ex");
	    }
	    else if (c_ptr->ex_num[num] >= c_ptr->ex_size[num]) {
		c_ptr->ex_list[num] = (char **)realloc_data(c_ptr->ex_list[num], 
							    sizeof(char *)*(c_ptr->ex_size[num] <<= 1), 
							    "_make_ipal_cframe_ex");
		c_ptr->ex_freq[num] = (int *)realloc_data(c_ptr->ex_freq[num], 
							  sizeof(int)*c_ptr->ex_size[num], 
							  "_make_ipal_cframe_ex");
	    }
	    c_ptr->ex_list[num][c_ptr->ex_num[num]] = strdup(point2);
	    c_ptr->ex_freq[num][c_ptr->ex_num[num]++] = freq;

	    c_ptr->freq[num] += freq;
	    count++;
	}
    }

    /* <����>���ɲ� */
    if (agent_count || sub_agent_flag) {
	if (c_ptr->ex_size[num] == 0) {
	    c_ptr->ex_size[num] = 1;	/* ������ݿ� */
	    c_ptr->ex_list[num] = (char **)malloc_data(sizeof(char *)*c_ptr->ex_size[num], 
						       "_make_ipal_cframe_ex");
	    c_ptr->ex_freq[num] = (int *)malloc_data(sizeof(int)*c_ptr->ex_size[num], 
						     "_make_ipal_cframe_ex");
	}
	else if (c_ptr->ex_num[num] >= c_ptr->ex_size[num]) {
	    c_ptr->ex_list[num] = (char **)realloc_data(c_ptr->ex_list[num], 
							sizeof(char *)*(c_ptr->ex_size[num] <<= 1), 
							"_make_ipal_cframe_ex");
	    c_ptr->ex_freq[num] = (int *)realloc_data(c_ptr->ex_freq[num], 
						      sizeof(int)*c_ptr->ex_size[num], 
						      "_make_ipal_cframe_ex");
	}
	c_ptr->ex_list[num][c_ptr->ex_num[num]] = strdup("<����>");
	c_ptr->ex_freq[num][c_ptr->ex_num[num]++] = agent_count ? agent_count : 1;
	if (c_ptr->freq[num] == 0) {
	    c_ptr->freq[num] = 1;
	}
    }

    *destination = strdup(buf);
    free(buf);
}

/*==================================================================*/
 int check_examples(char *cp, int cp_len, char **ex_list, int ex_num)
/*==================================================================*/
{
    int i;

    if (!ex_list) {
	return -1;
    }

    for (i = 0; i < ex_num; i++) {
	if (strlen(*(ex_list + i)) == cp_len && 
	    !strncmp(cp, *(ex_list + i), cp_len)) {
	    return i;
	}
    }
    return -1;
}

/*==================================================================*/
		int check_agentive(unsigned char *cp)
/*==================================================================*/
{
    unsigned char *point;

    point = cp;
    while ((point = extract_ipal_str(point, cf_str_buf, FALSE)))
	if (!strcmp(cf_str_buf, "��")) return TRUE;
    return FALSE;
}

/*==================================================================*/
     void _make_ipal_cframe(CF_FRAME *i_ptr, CASE_FRAME *cf_ptr,
			    int address, int size, char *verb, int flag)
/*==================================================================*/
{
    int i, j = 0, ga_p = FALSE, c1, c2, count = 0;
    char ast_cap[32], *token, *cp, *buf;

    cf_ptr->type = flag;
    cf_ptr->cf_address = address;
    cf_ptr->cf_size = size;
    strcpy(cf_ptr->cf_id, i_ptr->DATA); 
    cf_ptr->etcflag = i_ptr->etcflag;
    cf_ptr->entry = strdup(cf_ptr->cf_id);
    sscanf(cf_ptr->cf_id, "%[^:]", cf_ptr->entry); /* �ʥե졼����Ѹ�ɽ�� (��ɽɽ��) */
    strcpy(cf_ptr->pred_type, i_ptr->pred_type);
    for (i = 0; i_ptr->samecase[i][0] != END_M; i++) {
	cf_ptr->samecase[i][0] = i_ptr->samecase[i][0];
	cf_ptr->samecase[i][1] = i_ptr->samecase[i][1];
    }
    cf_ptr->samecase[i][0] = END_M;
    cf_ptr->samecase[i][1] = END_M;

    if (*(i_ptr->feature)) {
	cf_ptr->feature = strdup(i_ptr->feature);
    }
    else {
	cf_ptr->feature = NULL;
    }
    cf_ptr->cf_similarity = 0;


    /* �����Ǥ��ɲ� */

    if (cf_ptr->voice == FRAME_PASSIVE_I ||
	cf_ptr->voice == FRAME_CAUSATIVE_WO_NI ||
	cf_ptr->voice == FRAME_CAUSATIVE_WO ||
	cf_ptr->voice == FRAME_CAUSATIVE_NI) {
	_make_ipal_cframe_pp(cf_ptr, "��", j, flag);
	_make_ipal_cframe_sm(cf_ptr, "���ν�", j, 
			     Thesaurus == USE_NTT ? USE_NTT_WITH_STORE : USE_BGH_WITH_STORE);
	_make_ipal_cframe_ex(cf_ptr, "��", j, Thesaurus, FALSE);
	j++;
    }
    else if (cf_ptr->voice == FRAME_CAUSATIVE_PASSIVE) {
	_make_ipal_cframe_pp(cf_ptr, "��", j, flag);
	_make_ipal_cframe_sm(cf_ptr, "���ν�", j, 
			     Thesaurus == USE_NTT ? USE_NTT_WITH_STORE : USE_BGH_WITH_STORE);
	_make_ipal_cframe_ex(cf_ptr, "��", j, Thesaurus, FALSE);
	j++;
    }

    /* �Ƴ����Ǥν��� */

    for (i = 0; i < i_ptr->casenum && j < CASE_MAX_NUM; i++, j++) { 
	cf_ptr->adjacent[j] = FALSE;
	if (_make_ipal_cframe_pp(cf_ptr, i_ptr->cs[i].kaku_keishiki, j, flag) == FALSE) {
	    j--;
	    continue;
	}
	if (Thesaurus == USE_BGH) {
	    _make_ipal_cframe_ex(cf_ptr, i_ptr->cs[i].meishiku, j, USE_BGH_WITH_STORE, 
				 (OptCaseFlag & OPT_CASE_USE_EX_ALL) ? 0 : !MatchPP(cf_ptr->pp[j][0], "���δط�"));
	    _make_ipal_cframe_sm(cf_ptr, i_ptr->cs[i].imisosei, j, USE_BGH_WITH_STORE);
	}
	else if (Thesaurus == USE_NTT) {
	    _make_ipal_cframe_ex(cf_ptr, i_ptr->cs[i].meishiku, j, USE_NTT_WITH_STORE, 
				 (OptCaseFlag & OPT_CASE_USE_EX_ALL) ? 0 : !MatchPP(cf_ptr->pp[j][0], "���δط�"));
	    _make_ipal_cframe_sm(cf_ptr, i_ptr->cs[i].imisosei, j, USE_NTT_WITH_STORE);
	}

	/* ǽư : Agentive ���ʤ�Ǥ��Ū�Ȥ�����
	if (cf_ptr->voice == FRAME_ACTIVE &&
	    i == 0 && 
	    cf_ptr->pp[i][0] == pp_kstr_to_code("��") &&
	    check_agentive(i_ptr->DATA+i_ptr->jyutugoso) == TRUE)
	  cf_ptr->oblig[i] = FALSE;
	*/

	if (cf_ptr->voice == FRAME_CAUSATIVE_WO_NI ||	/* ���� */
	    cf_ptr->voice == FRAME_CAUSATIVE_WO ||
	    cf_ptr->voice == FRAME_CAUSATIVE_NI) {
	    /* �� �� �򡤥� */
	    if (cf_ptr->pp[j][0] == pp_kstr_to_code("��")) {
		if (ga_p == FALSE) {
		    ga_p = TRUE;
		    if (cf_ptr->voice == FRAME_CAUSATIVE_WO_NI)
		      _make_ipal_cframe_pp(cf_ptr, "�򡿥�", j, flag);
		    else if (cf_ptr->voice == FRAME_CAUSATIVE_WO)
		      _make_ipal_cframe_pp(cf_ptr, "��", j, flag);
		    else if (cf_ptr->voice == FRAME_CAUSATIVE_NI)
		      _make_ipal_cframe_pp(cf_ptr, "��", j, flag);
		} else {
		    _make_ipal_cframe_pp(cf_ptr, "��", j, flag); /* ��������ʸ */
		}
	    }
	}
	else if (cf_ptr->voice == FRAME_PASSIVE_I ||	/* ���� */
		 cf_ptr->voice == FRAME_PASSIVE_1 ||
		 cf_ptr->voice == FRAME_PASSIVE_2) {
	    /* ���� �����ˡ�ľ�� �����ˡ��˥�åơ����� */
	    if (!strcmp(i_ptr->cs[i].kaku_keishiki, "��")) {
		if (cf_ptr->voice == FRAME_PASSIVE_I)
		  _make_ipal_cframe_pp(cf_ptr, "��", j, flag);
		else if (cf_ptr->voice == FRAME_PASSIVE_1)
		  _make_ipal_cframe_pp(cf_ptr, 
				       "�ˡ��˥�롿����", j, flag);
		else if (cf_ptr->voice == FRAME_PASSIVE_2)
		  _make_ipal_cframe_pp(cf_ptr, 
				       "�ˡ��˥�롿����", j, flag);
	    }
	    /* ľ�� �ˡ��˥�åơ��������� */
	    else if ((cf_ptr->voice == FRAME_PASSIVE_1 && 
		      (!strcmp(i_ptr->cs[i].kaku_keishiki, "��") ||
		       (sprintf(ast_cap, "���") &&
			!strcmp(i_ptr->cs[i].kaku_keishiki, ast_cap)))) ||
		     (cf_ptr->voice == FRAME_PASSIVE_2 && 
		      (!strcmp(i_ptr->cs[i].kaku_keishiki, "��") ||
		       (sprintf(ast_cap, "�ˡ�") &&
			!strcmp(i_ptr->cs[i].kaku_keishiki, ast_cap))))) {
		_make_ipal_cframe_pp(cf_ptr, "��", j, flag);
	    }
	}
	else if (cf_ptr->voice == FRAME_POSSIBLE) {	/* ��ǽ */
	    if (!strcmp(i_ptr->cs[i].kaku_keishiki, "��")) {
		_make_ipal_cframe_pp(cf_ptr, "������", j, flag);
	     }
	}
	else if (cf_ptr->voice == FRAME_SPONTANE) {	/* ��ȯ */
	    if (!strcmp(i_ptr->cs[i].kaku_keishiki, "��")) {
		_make_ipal_cframe_pp(cf_ptr, "��", j, flag);
	     }
	}
    }
    cf_ptr->element_num = j;
}

/*==================================================================*/
TAG_DATA *get_quasi_closest_case_component(TAG_DATA *t_ptr, TAG_DATA *pre_ptr)
/*==================================================================*/
{
    if (t_ptr->num < 1 || 
	(t_ptr->type == IS_TAG_DATA && t_ptr->inum != 0)) {
	return NULL;
    }

    if (check_feature(t_ptr->f, "ID:�ʡ���ˡ���")) {
	return t_ptr;
    }

    if (pre_ptr->type == IS_TAG_DATA && pre_ptr->inum != 0) {
	return NULL;
    }

    if (!check_feature(pre_ptr->f, "�θ�")) {
	return NULL;
    }

    if (check_feature(pre_ptr->f, "�ؼ���") || 
	(pre_ptr->SM_code[0] == '\0' && 
	 check_feature(pre_ptr->f, "��:����"))) {
	return NULL;
    }

    if (check_feature(pre_ptr->f, "��:���") || 
	check_feature(pre_ptr->f, "��:�˳�") || 
	(!cf_match_element(pre_ptr->SM_code, "����", FALSE) && 
	(check_feature(pre_ptr->f, "��:����") || 
	 check_feature(pre_ptr->f, "��:�����") || 
	 check_feature(pre_ptr->f, "��:�س�") || 
	 check_feature(pre_ptr->f, "��:����") || 
	 check_feature(pre_ptr->f, "��:�ȳ�") || 
	 check_feature(pre_ptr->f, "��:�ޥǳ�")))) {
	return pre_ptr;
    }
    return NULL;
}

/*==================================================================*/
		   char *feature2case(TAG_DATA *tp)
/*==================================================================*/
{
    char *cp, *buffer;

    if (check_feature(tp->f, "ID:�ʡ���ˡ���")) {
	buffer = (char *)malloc_data(strlen("��")+1, "feature2case");
	strcpy(buffer, "��");
	return buffer;
    }
    else if ((cp = check_feature(tp->f, "��"))) {
	buffer = strdup(cp+strlen("��:"));
	if (!strncmp(buffer+strlen(buffer)-strlen("��"), "��", strlen("��"))) {
	    *(buffer+strlen(buffer)-strlen("��")) = '\0';
	    if (pp_kstr_to_code(buffer) != END_M) {
		return buffer;
	    }
	}
	free(buffer);
    }
    return NULL;
}

/*==================================================================*/
	       void f_num_inc(int start, int *f_num_p)
/*==================================================================*/
{
    (*f_num_p)++;
    if ((start + *f_num_p) >= MAX_Case_frame_num) {
	realloc_cf();
	realloc_cmm();
    }
}

/*==================================================================*/
int _make_ipal_cframe_subcontract(SENTENCE_DATA *sp, TAG_DATA *t_ptr, int start, 
				  char *verb, int voice, int flag)
/*==================================================================*/
{
    CF_FRAME *i_ptr;
    CASE_FRAME *cf_ptr;
    TAG_DATA *cbp;
    int f_num = 0, address, break_flag = 0, size, match, c;
    char *pre_pos, *cp, *address_str, *vtype = NULL;

    if (!verb)
    	return f_num;

    cf_ptr = Case_frame_array + start;

    /* ľ�������Ǥ򤯤äĤ��Ƹ��� */
    if (flag == CF_PRED) {
	/* �Ҥ餬�ʤ�ۣ�����Τ���Ȥ��ϡ��ʲ��Ϥ�ۣ������ä��뤿���
	   �����ǤϤ��٤Ƥγʥե졼��򸡺����Ƥ��� */
	if (check_str_type(t_ptr->head_ptr->Goi) == TYPE_HIRAGANA && 
	    check_feature(t_ptr->head_ptr->f, "��ۣ")) {
	    address_str = get_ipal_address(verb, flag);
	}
	else {
	    cbp = get_quasi_closest_case_component(t_ptr, t_ptr->num < 1 ? NULL : t_ptr - 1);
	    if (cbp) {
		char *buffer, *pp;

		pp = feature2case(cbp);
		if (pp) {
		    buffer = (char *)malloc_data(strlen(cbp->head_ptr->Goi) + strlen(pp) + strlen(verb) + 3, 
						 "_make_ipal_cframe_subcontract");
		    sprintf(buffer, "%s-%s-%s", cbp->head_ptr->Goi, pp, verb);
		    address_str = get_ipal_address(buffer, flag);
		    free(buffer);
		    free(pp);
		}
		if (!pp || !address_str) {
		    address_str = get_ipal_address(verb, flag);
		}
	    }
	    else {
		address_str = get_ipal_address(verb, flag);
	    }
	}
    }
    else {
	address_str = get_ipal_address(verb, flag);
    }

    /* �ʤ���� */
    if (!address_str)
	return f_num;

    if (flag == CF_NOUN && (vtype = check_feature(t_ptr->f, "�θ�"))) {
	vtype = "̾";
    }
    else if ((vtype = check_feature(t_ptr->f, "�Ѹ�"))) {
	vtype += strlen("�Ѹ�:");
    }
    else if ((vtype = check_feature(t_ptr->f, "���Ѹ��ʲ���"))) {
	vtype += strlen("���Ѹ��ʲ���:");
    }

    for (cp = pre_pos = address_str; ; cp++) {
	if (*cp == '/' || *cp == '\0') {
	    if (*cp == '\0')
		break_flag = 1;
	    else 
		*cp = '\0';
	    
	    /* �ʥե졼����ɤߤ��� */
	    match = sscanf(pre_pos, "%d:%d", &address, &size);
	    if (match != 2) {
		fprintf(stderr, ";; CaseFrame Dictionary Index error (it seems version 1.).\n");
		exit(1);
	    }

	    i_ptr = get_ipal_frame(address, size, flag);
	    pre_pos = cp + 1;

	    /* �Ѹ��Υ����פ��ޥå����ʤ���� (���Ѹ��ʤ��̲�) */
	    if (vtype) {
		if (strncmp(vtype, i_ptr->pred_type, strlen("̾"))) {
		    if (break_flag)
			break;
		    else
			continue;
		}
	    }

	    /* ǽư�� or �ʥե졼����֤��ޤޤ���� */
	    if (voice == 0) {
		/* CF_NOUN�ξ�硢�����ǥޥå� */
		(cf_ptr + f_num)->voice = FRAME_ACTIVE;
		_make_ipal_cframe(i_ptr, cf_ptr + f_num, address, size, verb, flag);

		/* �ʲ� flag == CF_PRED �ΤϤ� */

		/* �ʥե졼�����/�ʥե졼�����&����*/
		if (t_ptr->voice & VOICE_SHIEKI || 
		    t_ptr->voice & VOICE_SHIEKI_UKEMI) {
		    /* �˳ʤ��ʤ��Ȥ� */
		    if ((c = check_cf_case(cf_ptr + f_num, "��")) < 0) {
			_make_ipal_cframe_pp(cf_ptr + f_num, "��", (cf_ptr + f_num)->element_num, flag);
			_make_ipal_cframe_sm(cf_ptr + f_num, "����", (cf_ptr + f_num)->element_num, 
					     Thesaurus == USE_NTT ? USE_NTT_WITH_STORE : USE_BGH_WITH_STORE);
			(cf_ptr+f_num)->element_num++;
		    }
		    /* �˳ʤϤ��뤱��<����>���ʤ��Ȥ� */
		    else if (sms_match(sm2code("����"), (cf_ptr + f_num)->sm[c], SM_NO_EXPAND_NE) == FALSE) {
			_make_ipal_cframe_sm(cf_ptr + f_num, "����", c, 
					     Thesaurus == USE_NTT ? USE_NTT_WITH_STORE : USE_BGH_WITH_STORE);
		    }
		    if (t_ptr->voice & VOICE_SHIEKI) {
			(cf_ptr + f_num)->voice = FRAME_CAUSATIVE_NI;
		    }
		    else if (t_ptr->voice & VOICE_SHIEKI_UKEMI) {
			(cf_ptr + f_num)->voice = FRAME_CAUSATIVE_PASSIVE;
		    }
		}
		/* �ʥե졼����� */
		else if (t_ptr->voice & VOICE_UKEMI || 
			 t_ptr->voice & VOICE_UNKNOWN) {
		    /* ��/�˥��/����ʤ��ʤ��Ȥ� */
		    if ((c = check_cf_case(cf_ptr + f_num, "��")) < 0 && 
			(c = check_cf_case(cf_ptr + f_num, "�˥��")) < 0 && 
			(c = check_cf_case(cf_ptr + f_num, "����")) < 0) {
			_make_ipal_cframe_pp(cf_ptr + f_num, "��", (cf_ptr + f_num)->element_num, flag);
			_make_ipal_cframe_sm(cf_ptr + f_num, "����", (cf_ptr + f_num)->element_num, 
					     Thesaurus == USE_NTT ? USE_NTT_WITH_STORE : USE_BGH_WITH_STORE);
			(cf_ptr+f_num)->element_num++;
		    }
		    /* ��/�˥��/����ʤϤ��뤱��<����>���ʤ��Ȥ� */
		    else if (sms_match(sm2code("����"), (cf_ptr + f_num)->sm[c], SM_NO_EXPAND_NE) == FALSE) {
			_make_ipal_cframe_sm(cf_ptr + f_num, "����", c, 
					     Thesaurus == USE_NTT ? USE_NTT_WITH_STORE : USE_BGH_WITH_STORE);
		    }
		    (cf_ptr + f_num)->voice = FRAME_PASSIVE_1;
		}
		
		f_num_inc(start, &f_num);
		cf_ptr = Case_frame_array + start;
	    }

	    /* ���� */
	    if (voice & VOICE_SHIEKI) {
		if (i_ptr->voice & CF_CAUSATIVE_WO && i_ptr->voice & CF_CAUSATIVE_NI)
		  (cf_ptr + f_num)->voice = FRAME_CAUSATIVE_WO_NI;
		else if (i_ptr->voice & CF_CAUSATIVE_WO)
		  (cf_ptr + f_num)->voice = FRAME_CAUSATIVE_WO;
		else if (i_ptr->voice & CF_CAUSATIVE_NI)
		  (cf_ptr + f_num)->voice = FRAME_CAUSATIVE_NI;
		
		_make_ipal_cframe(i_ptr, cf_ptr + f_num, address, size, verb, flag);
		f_num_inc(start, &f_num);
		cf_ptr = Case_frame_array + start;
	    }
	    
	    /* ���� */
	    if (voice & VOICE_UKEMI) {
		/* ľ�ܼ��ȣ� */
		if (i_ptr->voice & CF_PASSIVE_1) {
		    (cf_ptr + f_num)->voice = FRAME_PASSIVE_1;
		    _make_ipal_cframe(i_ptr, cf_ptr + f_num, address, size, verb, flag);
		    f_num_inc(start, &f_num);
		    cf_ptr = Case_frame_array + start;
		}
		/* ľ�ܼ��ȣ� */
		if (i_ptr->voice & CF_PASSIVE_2) {
		    (cf_ptr + f_num)->voice = FRAME_PASSIVE_2;
		    _make_ipal_cframe(i_ptr, cf_ptr + f_num, address, size, verb, flag);
		    f_num_inc(start, &f_num);
		    cf_ptr = Case_frame_array + start;
		}
		/* ���ܼ��� */
		if (i_ptr->voice & CF_PASSIVE_I) {
		    (cf_ptr + f_num)->voice = FRAME_PASSIVE_I;
		    _make_ipal_cframe(i_ptr, cf_ptr + f_num, address, size, verb, flag);
		    f_num_inc(start, &f_num);
		    cf_ptr = Case_frame_array + start;
		}
	    }

	    /* ��餦/�ۤ��� */
	    if (voice & VOICE_MORAU || 
		voice & VOICE_HOSHII) {
		/* �˻��� (���ܼ��ȤǤ�Ʊ�� */
		if (i_ptr->voice & CF_CAUSATIVE_NI) {
		    (cf_ptr + f_num)->voice = FRAME_CAUSATIVE_NI;
		    _make_ipal_cframe(i_ptr, cf_ptr + f_num, address, size, verb, flag);
		    f_num_inc(start, &f_num);
		    cf_ptr = Case_frame_array + start;
		}
 	    }

	    /* ������/�������� */
	    if (voice & VOICE_SHIEKI_UKEMI) {
		(cf_ptr + f_num)->voice = FRAME_CAUSATIVE_PASSIVE;
		_make_ipal_cframe(i_ptr, cf_ptr + f_num, address, size, verb, flag);
		f_num_inc(start, &f_num);
		cf_ptr = Case_frame_array + start;
	    } 

	    /* ��ǽ��º�ɡ���ȯ */
	    if (voice & VOICE_UKEMI) {
		if (i_ptr->voice & CF_POSSIBLE) {
		    (cf_ptr + f_num)->voice = FRAME_POSSIBLE;
		    _make_ipal_cframe(i_ptr, cf_ptr + f_num, address, size, verb, flag);
		    f_num_inc(start, &f_num);
		    cf_ptr = Case_frame_array + start;
		}
		if (i_ptr->voice & CF_POLITE) {
		    (cf_ptr + f_num)->voice = FRAME_POLITE;
		    _make_ipal_cframe(i_ptr, cf_ptr + f_num, address, size, verb, flag);
		    f_num_inc(start, &f_num);
		    cf_ptr = Case_frame_array + start;
		}
		if (i_ptr->voice & CF_SPONTANE) {
		    (cf_ptr + f_num)->voice = FRAME_SPONTANE;
		    _make_ipal_cframe(i_ptr, cf_ptr + f_num, address, size, verb, flag);
		    f_num_inc(start, &f_num);
		    cf_ptr = Case_frame_array + start;
		}
	    }
	    if (break_flag)
		break;
	}
    }
    free(address_str);
    return f_num;
}

/*==================================================================*/
int make_ipal_cframe_subcontract(SENTENCE_DATA *sp, TAG_DATA *t_ptr, int start, char *in_verb, int flag)
/*==================================================================*/
{
    int f_num = 0, plus_num;
    char *verb;

    if (flag == CF_NOUN) {
	return _make_ipal_cframe_subcontract(sp, t_ptr, start, in_verb, 0, flag);
    }

    verb = (char *)malloc_data(strlen(in_verb) + 4, "make_ipal_cframe_subcontract");
    strcpy(verb, in_verb);

    if (t_ptr->voice == VOICE_UNKNOWN) {
	t_ptr->voice = 0; /* ǽư�֤�try */
	f_num = _make_ipal_cframe_subcontract(sp, t_ptr, start, verb, 0, flag);

	/* ���ΤȤ�����Ȥξ���ͤ��ʤ� */
	free(verb);
	return f_num;

	t_ptr->voice = VOICE_UNKNOWN;
    }

    /* ����, ����γʥե졼�� */
    if (t_ptr->voice == VOICE_UNKNOWN || 
	t_ptr->voice & VOICE_UKEMI ||
	t_ptr->voice & VOICE_SHIEKI || 
	t_ptr->voice & VOICE_SHIEKI_UKEMI) {
	int suffix = 0;

	if (t_ptr->voice & VOICE_SHIEKI) {
	    strcat(verb, ":C");
	    suffix = 2;
	}
	else if (t_ptr->voice & VOICE_UKEMI || 
		 t_ptr->voice & VOICE_UNKNOWN) {
	    strcat(verb, ":P");
	    suffix = 2;
	}
	else if (t_ptr->voice & VOICE_SHIEKI_UKEMI) {
	    strcat(verb, ":PC");
	    suffix = 3;
	}

	plus_num = _make_ipal_cframe_subcontract(sp, t_ptr, start + f_num, verb, 0, flag);
	if (plus_num != 0) {
	    free(verb);
	    return f_num + plus_num;
	}
	*(verb + strlen(verb) - suffix) = '\0'; /* �ߤĤ���ʤ��ä����Ȥˤ�ɤ� */
    }

    if (t_ptr->voice == VOICE_UNKNOWN) {
	f_num += _make_ipal_cframe_subcontract(sp, t_ptr, start + f_num, verb, VOICE_UKEMI, flag); /* ���� */
    }
    else {
	f_num = _make_ipal_cframe_subcontract(sp, t_ptr, start, verb, t_ptr->voice, flag);
    }
    free(verb);    
    return f_num;
}

/*==================================================================*/
char *make_pred_string(TAG_DATA *t_ptr, MRPH_DATA *m_ptr, char *orig_form, int use_rep_flag)
/*==================================================================*/
{
    char *buffer, *main_pred = NULL, *cp, *rep_strt;
    int rep_length, main_pred_malloc_flag = 0;

    /* orig_form == 1: ��ǽư��Τ�Ȥη����Ѥ���Ȥ� */

    /* m_ptr == NULL: ��ư��η����Ǥ� t_ptr->head_ptr ���Ѥ���
       otherwise    : ��ư��η����ǤȤ��� m_ptr ���Ѥ��� (ALT�Τ��) */

    /* �Ѹ�������, voice��ʬ(7)����ݤ��Ƥ��� */

    /* ��ɽɽ����Ȥ�������ɽɽ��������Ȥ� */
    if (use_rep_flag) {
	if (m_ptr) {
	    rep_strt = get_mrph_rep(m_ptr);
	    rep_length = get_mrph_rep_length(rep_strt);
	    if (rep_length) {
		main_pred = (char *)malloc_data(rep_length + 1, "make_pred_string");
		strncpy(main_pred, rep_strt, rep_length);
		*(main_pred + rep_length) = '\0';
		main_pred_malloc_flag = 1;
	    }
	}
	else {
	    if ((main_pred = get_mrph_rep_from_f(t_ptr->head_ptr)) == NULL) {
		main_pred = make_mrph_rn(t_ptr->head_ptr);
		main_pred_malloc_flag = 1;
	    }
	}
    }
    if (main_pred == NULL) {
	main_pred = m_ptr ? m_ptr->Goi : t_ptr->head_ptr->Goi;
    }

    /* �֡ʡ���ˡ��ˡ� �ΤȤ��� �֤���� ��õ�� */
    if (check_feature(t_ptr->f, "ID:�ʡ���ˡ���")) {
	buffer = (char *)malloc_data(strlen("����/����") + 8, "make_pred_string"); /* 9(euc) + 8 */
	if (use_rep_flag) {
	    strcpy(buffer, "����/����");
	}
	else {
	    strcpy(buffer, "����");
	}
    }
    /* �ַ��ƻ�+�ʤ�פʤ� */
    else if (check_feature(t_ptr->f, "���Ѹ����Т�")) {
	if (use_rep_flag) {
	    if ((cp = get_mrph_rep_from_f(t_ptr->head_ptr + 1))) {
		buffer = (char *)malloc_data(strlen(main_pred) + strlen(cp) + 9, 
					     "make_pred_string");
		strcpy(buffer, main_pred);
		strcat(buffer, "+");
		strcat(buffer, cp);
	    }
	    else {
		buffer = (char *)malloc_data(strlen(main_pred) + strlen((t_ptr->head_ptr + 1)->Goi) + 9, 
					     "make_pred_string");
		strcpy(buffer, main_pred);
		strcat(buffer, "+");
		strcat(buffer, (t_ptr->head_ptr + 1)->Goi);
	    }
	}
	else {
	    buffer = (char *)malloc_data(strlen(t_ptr->head_ptr->Goi2) + strlen((t_ptr->head_ptr + 1)->Goi) + 8, 
					 "make_pred_string");
	    strcpy(buffer, t_ptr->head_ptr->Goi2);
	    strcat(buffer, (t_ptr->head_ptr + 1)->Goi);
	}
    }
    /* �ַ��ƻ�촴+Ū���פʤ� */
    else if (check_feature(t_ptr->f, "���Ѹ����Т�")) {
	if (use_rep_flag &&
	    (cp = get_mrph_rep_from_f(t_ptr->head_ptr - 1))) {
	    buffer = (char *)malloc_data(strlen(cp) + strlen(main_pred) + 9, 
					 "make_pred_string");
	    strcpy(buffer, cp);
	    strcat(buffer, "+");
	}
	else {
	    buffer = (char *)malloc_data(strlen((t_ptr->head_ptr - 1)->Goi2) + strlen(main_pred) + 8, 
					 "make_pred_string");
	    strcpy(buffer, (t_ptr->head_ptr - 1)->Goi2);
	}
	strcat(buffer, main_pred);
    }
    else {
	if (orig_form) {
	    buffer = (char *)malloc_data(strlen(orig_form) + 8, "make_pred_string");
	    strcpy(buffer, orig_form);
	}
	else {
	    buffer = (char *)malloc_data(strlen(main_pred) + 8, "make_pred_string");
	    strcpy(buffer, main_pred);
	}
    }

    if (main_pred_malloc_flag) {
	free(main_pred);
    }

    return buffer;
}

/*==================================================================*/
int make_ipal_cframe(SENTENCE_DATA *sp, TAG_DATA *t_ptr, int start, int flag)
/*==================================================================*/
{
    int f_num = 0;
    char *cp, *pred_string, *new_pred_string;

    /* ��Ω����������Ѥ��Ƴʥե졼�༭������ */

    if (!t_ptr->jiritu_ptr) {
	return f_num;
    }

    pred_string = make_pred_string(t_ptr, NULL, NULL, OptCaseFlag & OPT_CASE_USE_REP_CF);
    f_num += make_ipal_cframe_subcontract(sp, t_ptr, start, pred_string, flag);

    /* ��ɽɽ����ۣ����Ѹ��ξ�� */
    if (check_feature(t_ptr->head_ptr->f, "����ۣ��")) {
	FEATURE *fp;
	MRPH_DATA m;
	char *str;

	fp = t_ptr->head_ptr->f;
	while (fp) {
	    if (!strncmp(fp->cp, "ALT-", 4)) {
		sscanf(fp->cp + 4, "%[^-]-%[^-]-%[^-]-%d-%d-%d-%d-%[^\n]", 
		       m.Goi2, m.Yomi, m.Goi, 
		       &m.Hinshi, &m.Bunrui, 
		       &m.Katuyou_Kata, &m.Katuyou_Kei, m.Imi);
		new_pred_string = make_pred_string(t_ptr, &m, NULL, OptCaseFlag & OPT_CASE_USE_REP_CF);
		/* ��ɽ�Ȱۤʤ��� */
		if (strcmp(pred_string, new_pred_string)) {
		    f_num += make_ipal_cframe_subcontract(sp, t_ptr, start + f_num, new_pred_string, flag);
		}
		free(new_pred_string);
	    }
	    fp = fp->next;
	}
    }

    free(pred_string);

    /* �ʤ��Ȥ��ǡ���ǽư��ΤȤ��ϡ���Ȥη���Ȥ� */
    if (f_num == 0 && 
	(cp = check_feature(t_ptr->head_ptr->f, "��ǽư��"))) {
	pred_string = make_pred_string(t_ptr, NULL, cp + strlen("��ǽư��:"), OptCaseFlag & OPT_CASE_USE_REP_CF);
	f_num += make_ipal_cframe_subcontract(sp, t_ptr, start, pred_string, flag);	
	free(pred_string);
    }

    Case_frame_num += f_num;
    return f_num;
}

/*==================================================================*/
	 int make_default_cframe(TAG_DATA *t_ptr, int start)
/*==================================================================*/
{
    int i, num = 0, f_num = 0, rep_name_malloc_flag = 0;
    CASE_FRAME *cf_ptr;
    char *cp, *rep_name;

    cf_ptr = Case_frame_array + start;

    if (MAX_cf_frame_length == 0) {
	cf_str_buf = 
	    (unsigned char *)realloc_data(cf_str_buf, 
					  sizeof(unsigned char)*ALLOCATION_STEP, 
					  "make_default_cframe");
    }

    cf_ptr->pred_type[0] = '\0';
    cf_ptr->cf_address = -1;
    if (cp = check_feature(t_ptr->f, "�Ѹ�")) {
	_make_ipal_cframe_pp(cf_ptr, "����", num, CF_PRED);
	_make_ipal_cframe_sm(cf_ptr, "���ν�", num++, 
			     Thesaurus == USE_NTT ? USE_NTT_WITH_STORE : USE_BGH_WITH_STORE);

	if (!strcmp(cp, "�Ѹ�:Ƚ")) {
	    strcpy(cf_ptr->pred_type, "Ƚ");
	}
	else if (!strcmp(cp, "�Ѹ�:ư")) {
	    strcpy(cf_ptr->pred_type, "ư");
	    _make_ipal_cframe_pp(cf_ptr, "���", num++, CF_PRED);
	    _make_ipal_cframe_pp(cf_ptr, "�ˡ�", num++, CF_PRED);
	    _make_ipal_cframe_pp(cf_ptr, "�ء�", num++, CF_PRED);
	    _make_ipal_cframe_pp(cf_ptr, "����", num++, CF_PRED);
	}
	else if (!strcmp(cp, "�Ѹ�:��")) {
	    strcpy(cf_ptr->pred_type, "��");
	    _make_ipal_cframe_pp(cf_ptr, "�ˡ�", num++, CF_PRED);
	    _make_ipal_cframe_pp(cf_ptr, "����", num++, CF_PRED);
	}
	else {
	    return FALSE;
	}
    }

    cf_ptr->element_num = num;
    cf_ptr->etcflag = CF_NORMAL;
    /* ��ɽɽ���ʥե졼��ΤȤ��ϡ�ID�ʤɤ���ɽɽ���ˤ��� */
    if (OptCaseFlag & OPT_CASE_USE_REP_CF) {
	if ((rep_name = get_mrph_rep_from_f(t_ptr->head_ptr)) == NULL) {
	    rep_name = make_mrph_rn(t_ptr->head_ptr);
	    rep_name_malloc_flag = 1;
	}
	sprintf(cf_ptr->cf_id, "%s:%s0", rep_name, cf_ptr->pred_type);
	if (rep_name_malloc_flag) {
	    cf_ptr->entry = rep_name;
	}
	else {
	    cf_ptr->entry = strdup(rep_name);
	}
    }
    else {
	sprintf(cf_ptr->cf_id, "%s:%s0", t_ptr->head_ptr->Goi, cf_ptr->pred_type);
	cf_ptr->entry = strdup(t_ptr->head_ptr->Goi);
    }

    for (i = 0; i < num; i++) {
	cf_ptr->pp[i][1] = END_M;
    }

    cf_ptr->samecase[0][0] = END_M;
    cf_ptr->samecase[0][1] = END_M;

    f_num_inc(start, &f_num);
    Case_frame_num++;
    t_ptr->cf_num = 1;
    return TRUE;
}

/*==================================================================*/
  void make_caseframes(SENTENCE_DATA *sp, TAG_DATA *t_ptr, int flag)
/*==================================================================*/
{
    t_ptr->cf_num += make_ipal_cframe(sp, t_ptr, Case_frame_num, flag);

    /* �ʤ��Ȥ����Ѹ��ΤȤ��ϡ�default�γʥե졼���Ĥ��� */
    if (t_ptr->cf_num == 0 && flag == CF_PRED) {
	make_default_cframe(t_ptr, Case_frame_num);
    }
}

/*==================================================================*/
		void set_caseframes(SENTENCE_DATA *sp)
/*==================================================================*/
{
    int i, j, start, hiragana_count, pred_num = 0;
    TAG_DATA  *t_ptr;

    Case_frame_num = 0;

    for (i = 0, t_ptr = sp->tag_data; i < sp->Tag_num; i++, t_ptr++) {
	/* ���򥳡��ѥ������Ϥ����Ȥ��˼�Ω�줬�ʤ���礬���� */
	t_ptr->cf_num = 0;

	if (t_ptr->jiritu_ptr != NULL && 
	    !check_feature(t_ptr->f, "�ʲ��Ϥʤ�")) {
	    if (OptUseCF &&
		(!OptEllipsis || 
		 (OptEllipsis & OPT_ELLIPSIS) || 
		 (OptEllipsis & OPT_DEMO)) && 
		(check_feature(t_ptr->f, "�Ѹ�") || /* ���Ѹ��ϤȤꤢ�����оݳ� */
		 (check_feature(t_ptr->f, "���Ѹ��ʲ���") && /* ����̾��, ���ƻ�촴 (��ΨŪ�ʳ�) */
		  (!(OptCaseFlag & OPT_CASE_USE_PROBABILITY) || 
		   (t_ptr->inum == 1 && /* ��ΨŪ�ξ��ϡ��ָ���ͽ����פΤ褦�ʤȤ��Τ� */
		    check_feature(t_ptr->b_ptr->f, "����ñ�̼�:-1")))))) { 
		
		set_pred_voice((BNST_DATA *)t_ptr); /* �������� */

		make_caseframes(sp, t_ptr, CF_PRED);
		t_ptr->e_cf_num = t_ptr->cf_num;
	    }
	    /* ̾��ʥե졼�� */
	    if (OptUseNCF && 
		check_feature(t_ptr->f, "�θ�")) {
		make_caseframes(sp, t_ptr, CF_NOUN);
	    }
	}
	else {
	    t_ptr->cf_ptr = NULL;
	}
    }

    /* �ƥ���ñ�̤���ʥե졼��ؤΥ���դ� */
    start = 0;
    for (i = 0, t_ptr = sp->tag_data; i < sp->Tag_num; i++, t_ptr++) {
	if (t_ptr->cf_num) {
	    t_ptr->cf_ptr = Case_frame_array + start;
	    t_ptr->pred_num = pred_num++;

	    /* ɽ�����Ҥ餬�ʤξ��: 
	       �ʥե졼���ɽ�����Ҥ餬�ʤξ�礬¿����ФҤ餬�ʤγʥե졼��Τߤ��оݤˡ�
	       �Ҥ餬�ʰʳ���¿����ФҤ餬�ʰʳ��Τߤ��оݤˤ��뤿���feature����Ϳ */
	    if (!(OptCaseFlag & OPT_CASE_USE_REP_CF) && /* ��ɽɽ���ǤϤʤ����Τ� */
		check_str_type(t_ptr->head_ptr->Goi) == TYPE_HIRAGANA) {
		hiragana_count = 0;
		for (j = 0; j < t_ptr->cf_num; j++) {
		    if (check_str_type((t_ptr->cf_ptr + j)->entry) == TYPE_HIRAGANA) {
			hiragana_count++;
		    }
		}
		if (2 * hiragana_count > t_ptr->cf_num) {
		    assign_cfeature(&(t_ptr->f), "��ɽ�Ҥ餬��", FALSE);
		}
	    }

	    start += t_ptr->cf_num;
	}
    }
}

/*==================================================================*/
		       void clear_cf(int flag)
/*==================================================================*/
{
    int i, j, k, end;

    end = flag ? MAX_Case_frame_num : Case_frame_num;

    for (i = 0; i < end; i++) {
	for (j = 0; j < CF_ELEMENT_MAX; j++) {
	    if ((Case_frame_array+i)->pp_str[j]) {
		free((Case_frame_array+i)->pp_str[j]);
		(Case_frame_array+i)->pp_str[j] = NULL;
	    }
	    if ((Case_frame_array+i)->ex[j]) {
		free((Case_frame_array+i)->ex[j]);
		(Case_frame_array+i)->ex[j] = NULL;
	    }
	    if ((Case_frame_array+i)->sm[j]) {
		free((Case_frame_array+i)->sm[j]);
		(Case_frame_array+i)->sm[j] = NULL;
	    }
	    if ((Case_frame_array+i)->sm_delete[j]) {
		free((Case_frame_array+i)->sm_delete[j]);
		(Case_frame_array+i)->sm_delete[j] = NULL;
		(Case_frame_array+i)->sm_delete_size[j] = 0;
		(Case_frame_array+i)->sm_delete_num[j] = 0;
	    }
	    if ((Case_frame_array+i)->sm_specify[j]) {
		free((Case_frame_array+i)->sm_specify[j]);
		(Case_frame_array+i)->sm_specify[j] = NULL;
		(Case_frame_array+i)->sm_specify_size[j] = 0;
		(Case_frame_array+i)->sm_specify_num[j] = 0;
	    }
	    if ((Case_frame_array+i)->ex_list[j]) {
		for (k = 0; k < (Case_frame_array+i)->ex_num[j]; k++) {
		    if ((Case_frame_array+i)->ex_list[j][k]) {
			free((Case_frame_array+i)->ex_list[j][k]);
		    }
		}
		free((Case_frame_array+i)->ex_list[j]);
		free((Case_frame_array+i)->ex_freq[j]);
		(Case_frame_array+i)->ex_list[j] = NULL;
		(Case_frame_array+i)->ex_size[j] = 0;
		(Case_frame_array+i)->ex_num[j] = 0;
	    }
	    if ((Case_frame_array+i)->semantics[j]) {
		free((Case_frame_array+i)->semantics[j]);
		(Case_frame_array+i)->semantics[j] = NULL;
	    }
	}
	if ((Case_frame_array+i)->entry) {
	    free((Case_frame_array+i)->entry);
	    (Case_frame_array+i)->entry = NULL;
	}
	if ((Case_frame_array+i)->feature) {
	    free((Case_frame_array+i)->feature);
	    (Case_frame_array+i)->feature = NULL;
	}
    }
}

/*==================================================================*/
	     int check_cf_case(CASE_FRAME *cfp, char *pp)
/*==================================================================*/
{
    int i;
    for (i = 0; i < cfp->element_num; i++) {
	if (MatchPP2(cfp->pp[i], pp)) {
	    return i;
	}
    }
    return -1;
}

/*==================================================================*/
		    char *malloc_db_buf(int size)
/*==================================================================*/
{
    if (db_buf_size == 0) {
	db_buf_size = DATA_LEN;
	db_buf = (char *)malloc_data(db_buf_size, "malloc_db_buf");
    }

    while (db_buf_size < size) {
	db_buf = (char *)realloc_data(db_buf, db_buf_size <<= 1, "malloc_db_buf");
    }

    return db_buf;
}

/*==================================================================*/
	    float get_cfs_similarity(char *cf1, char *cf2)
/*==================================================================*/
{
    char *key, *value;
    char *verb1, *verb2;
    float ret;
    int id1, id2;

    if (CFSimExist == FALSE || cf1 == NULL || cf2 == NULL) {
	return 0;
    }

    /* Ʊ���Ȥ� */
    if (!strcmp(cf1, cf2)) {
	return 1.0;
    }

    verb1 = strdup(cf1);
    verb2 = strdup(cf2);
    sscanf(cf1, "%[^0-9]%d", verb1, &id1);
    sscanf(cf2, "%[^0-9]%d", verb2, &id2);

    key = (char *)malloc_data(sizeof(char) * (strlen(cf1) + strlen(cf2) + 2), 
			      "get_cfs_similarity");

    /* ��¦��id���������ʤ�褦��key������ */
    if (id1 > id2) {
	sprintf(key, "%s%d-%s%d", verb2, id2, verb1, id1);
    }
    else {
	sprintf(key, "%s%d-%s%d", verb1, id1, verb2, id2);
    }

    value = db_get(cf_sim_db, key);
    if (value) {
	    ret = atof(value);
	    free(value);
    }
    free(key);
    free(verb1);
    free(verb2);

    return ret;
}

/*==================================================================*/
     double get_cf_probability(CASE_FRAME *cfd, CASE_FRAME *cfp)
/*==================================================================*/
{
    /* �ʥե졼�������Ψ P(���٤�:ư2|���٤�:ư)
       KNP�ʲ��Ϸ�� (cfp.prob) */

    char *vtype, *key, *value, voice[3];
    double ret;
    int num;
    TAG_DATA *tp = cfd->pred_b_ptr;

    if (CfpExist == FALSE) {
	return 0;
    }

    if ((vtype = check_feature(tp->f, "�Ѹ�"))) {
	vtype += strlen("�Ѹ�:");
    }
    else if ((vtype = check_feature(tp->f, "���Ѹ��ʲ���"))) {
	vtype += strlen("���Ѹ��ʲ���:");
    }
    else {
	return UNKNOWN_CF_SCORE;
    }

    /* �Ѹ�ɽ���ϳʥե졼��ID������Ф��ʤ� */

    key = malloc_db_buf(strlen(cfp->cf_id) + strlen(tp->head_ptr->Goi) + 8);
    if ((num = sscanf(cfp->cf_id, "%*[^:]:%*[^:]:%[PC]%*d", voice)) == 1) {
	sprintf(key, "%s|%s:%s:%s", cfp->cf_id, tp->head_ptr->Goi, vtype, voice);
    }
    else {
	sprintf(key, "%s|%s:%s", cfp->cf_id, tp->head_ptr->Goi, vtype);
    }

    value = db_get(cfp_db, key);

    if (value) {
	ret = atof(value);
	if (VerboseLevel >= VERBOSE3) {
	    fprintf(Outfp, ";; (CF) %s: P(%s) = %lf\n", tp->head_ptr->Goi, key, ret);
	}
	free(value);
	ret = log(ret);
    }
    else {
	if (VerboseLevel >= VERBOSE3) {
	    fprintf(Outfp, ";; (CF) %s: P(%s) = 0\n", tp->head_ptr->Goi, key);
	}
	ret = UNKNOWN_CF_SCORE;
    }

    return ret;
}

/*==================================================================*/
   double get_case_probability(int as2, CASE_FRAME *cfp, int aflag)
/*==================================================================*/
{
    /* �ʳ�Ψ P(���ʡ�|���٤�:ư2)
       KNP�ʲ��Ϸ�̤���׻� (cfcases.prob) */

    char *key, *value, *verb;
    double ret;

    if (CFCaseExist == FALSE) {
	return 0;
    }

    /* �Ѹ�ɽ�� */
    verb = strdup(cfp->cf_id);
    sscanf(cfp->cf_id, "%[^0-9]%*d", verb);

    key = malloc_db_buf(strlen(pp_code_to_kstr(cfp->pp[as2][0])) + 
			strlen(cfp->cf_id) + 2);

    /* �Ѹ�ɽ���Ǥ�ä������褤�ߤ��� */
    sprintf(key, "%s|%s", pp_code_to_kstr(cfp->pp[as2][0]), verb); /* cfp->cf_id); */
    value = db_get(cf_case_db, key);

    /* if (!value) { * back-off *
	sprintf(key, "%s|%s", pp_code_to_kstr(cfp->pp[as2][0]), verb);
	value = db_get(cf_case_db, key);
	} */
    free(verb);

    if (value) {
	ret = atof(value);
	if (VerboseLevel >= VERBOSE3) {
	    fprintf(Outfp, ";; (C) P(%s) = %lf\n", key, ret);
	}
	free(value);
	if (aflag == FALSE) {
	    ret = 1 - ret;
	}
	if (ret == 0) {
	    ret = UNKNOWN_CASE_SCORE;
	}
	else {
	    ret = log(ret);
	}
    }
    else {
	if (VerboseLevel >= VERBOSE3) {
	    fprintf(Outfp, ";; (C) P(%s) = 0\n", key);
	}
	if (aflag == FALSE) {
	    ret = 0;
	}
	else {
	    ret = UNKNOWN_CASE_SCORE;
	}
    }

    return ret;
}

/*==================================================================*/
      double get_case_num_probability(CASE_FRAME *cfp, int num)
/*==================================================================*/
{
    /* �ʤθĿ���Ψ P(2|���٤�:ư2)
       KNP�ʲ��Ϸ�̤���׻� (cfcases.prob) */

    char *key, *value, *verb;
    double ret;

    if (CFCaseExist == FALSE) {
	return 0;
    }

    /* �Ѹ�ɽ�� */
    verb = strdup(cfp->cf_id);
    sscanf(cfp->cf_id, "%[^0-9]%*d", verb);

    key = malloc_db_buf(strlen(cfp->cf_id) + 6);

    sprintf(key, "%d|N:%s", num, verb); /* cfp->cf_id */
    value = db_get(cf_case_db, key);

    /* if (!value) {
	sprintf(key, "%d|N:%s", num, verb);
	value = db_get(cf_case_db, key);
	} */
    free(verb);

    if (value) {
	ret = atof(value);
	if (VerboseLevel >= VERBOSE3) {
	    fprintf(Outfp, ";; (C) P(%s) = %lf\n", key, ret);
	}
	free(value);
	ret = log(ret);
    }
    else {
	if (VerboseLevel >= VERBOSE3) {
	    fprintf(Outfp, ";; (C) P(%s) = 0\n", key);
	}
	ret = UNKNOWN_CASE_SCORE;
    }

    return ret;
}

/*==================================================================*/
double _get_ex_probability_internal(char *key, int as2, CASE_FRAME *cfp)
/*==================================================================*/
{
    int i;
    double ret = 0;

    for (i = 0; i < cfp->ex_num[as2]; i++) {
	if (!strcmp(key, cfp->ex_list[as2][i])) {
	    ret = (double)cfp->ex_freq[as2][i] / cfp->freq[as2];
	    if (VerboseLevel >= VERBOSE3) {
		fprintf(Outfp, ";; P(%s) = %lf\n", key, ret);
	    }
	    return ret;
	}
    }

    if (VerboseLevel >= VERBOSE3) {
	fprintf(Outfp, ";; P(%s) = 0\n", key);
    }
    return 0;
}

/*==================================================================*/
		double _get_ex_probability(char *key)
/*==================================================================*/
{
    char *value;
    double ret = 0;

    if (CFExExist == FALSE) {
	return 0;
    }

    value = db_get(cf_ex_db, key);

    if (value) {
	ret = exp(-1 * atof(value));
	if (VerboseLevel >= VERBOSE3) {
	    fprintf(Outfp, ";; P(%s) = %lf\n", key, ret);
	}
	free(value);
    }
    else {
	if (VerboseLevel >= VERBOSE3) {
	    fprintf(Outfp, ";; P(%s) = 0\n", key);
	}
    }

    return ret;
}

/*==================================================================*/
  double _get_sm_probability(TAG_DATA *dp, int as2, CASE_FRAME *cfp)
/*==================================================================*/
{
    int i;
    double ret, max = 0;
    char *key, *sm_code = dp->SM_code;
    char code[SM_CODE_SIZE + 1];
    code[SM_CODE_SIZE] = '\0';

    key = malloc_db_buf(SM_CODE_SIZE + 
			strlen(cfp->cf_id) + strlen(pp_code_to_kstr(cfp->pp[as2][0])) + 3);

    /* �ư�̣�Ǥ��Ȥ�Ĵ�٤ơ�max��Ȥ� */
    for (i = 0; *(sm_code + i); i += SM_CODE_SIZE) {
	strncpy(code, sm_code + i, SM_CODE_SIZE);
	code[0] = '1';
	sprintf(key, "%s|%s,%s", 
		code, 
		cfp->cf_id, pp_code_to_kstr(cfp->pp[as2][0]));
	ret = _get_ex_probability(key);
	if (ret > max) {
	    max = ret;
	}
    }

    return max;
}

/*==================================================================*/
double _get_soto_default_probability(TAG_DATA *dp, int as2, CASE_FRAME *cfp)
/*==================================================================*/
{
    int i;
    double ret, max = 0;
    char *key, *sm_code = dp->SM_code;
    char code[SM_CODE_SIZE + 1];
    code[SM_CODE_SIZE] = '\0';

    if (CFExExist == FALSE) {
	return 0;
    }

    if (strlen(dp->head_ptr->Goi) > SM_CODE_SIZE) {
	key = malloc_db_buf(strlen(dp->head_ptr->Goi) + 18);
    }
    else {
	key = malloc_db_buf(SM_CODE_SIZE + 18);
    }

    /* ɽ����search */
    sprintf(key, "%s|DEFAULT,���δط�", dp->head_ptr->Goi);
    if (ret = _get_ex_probability(key)) {
	return ret;
    }

    /* ��̣�Ǥ�search: max��Ȥ� */
    for (i = 0; *(sm_code + i); i += SM_CODE_SIZE) {
	strncpy(code, sm_code + i, SM_CODE_SIZE);
	code[0] = '1';
	sprintf(key, "%s|DEFAULT,���δط�", code);
	if (ret = _get_ex_probability(key) && ret > max) {
	    max = ret;
	}
    }

    return max;
}

/*==================================================================*/
  double get_ex_probability(int as1, CASE_FRAME *cfd, TAG_DATA *dp,
			    int as2, CASE_FRAME *cfp)
/*==================================================================*/
{
    /* �����Ψ P(����|���٤�:ư2,���)
       �ʥե졼�फ��׻� (cfex.prob) */

    char *key = NULL, *mrph_str;
    double ret;
    int rep_malloc_flag = 0;

    /* dp�λ��꤬�ʤ���С�as1��cfd������ */
    if (dp == NULL) {
	dp = cfd->pred_b_ptr->cpm_ptr->elem_b_ptr[as1];
    }

    key = malloc_db_buf(strlen("<��ʸ>") + strlen(cfp->cf_id) + strlen(pp_code_to_kstr(cfp->pp[as2][0])) + 3);
    *key = '\0';

    if (check_feature(dp->f, "��ʸ")) {
	sprintf(key, "<��ʸ>");
	/* sprintf(key, "<��ʸ>|%s,%s", 
	   cfp->cf_id, pp_code_to_kstr(cfp->pp[as2][0])); */
    }
    else if (dat_match_sm(as1, cfd, dp, "����")) {
	sprintf(key, "<����>");
    }
    else if (check_feature(dp->f, "����")) {
	sprintf(key, "<����>");
    }
    else if (check_feature(dp->f, "����")) {
	sprintf(key, "<����>");
    }

    if (*key) {
	/* if (ret = _get_ex_probability(key)) { */
	if (ret = _get_ex_probability_internal(key, as2, cfp)) {
	    return log(ret);
	}
    }

    if (OptCaseFlag & OPT_CASE_USE_REP_CF) {
	mrph_str = get_mrph_rep_from_f(dp->head_ptr);
	if (mrph_str == NULL) {
	    mrph_str = make_mrph_rn(dp->head_ptr);
	    rep_malloc_flag = 1;
	}
    }
    else {
	mrph_str = dp->head_ptr->Goi;
    }
    key = malloc_db_buf(strlen(mrph_str) + 
			strlen(cfp->cf_id) + strlen(pp_code_to_kstr(cfp->pp[as2][0])) + 3);
    sprintf(key, "%s", mrph_str);
    /* sprintf(key, "%s|%s,%s", dp->head_ptr->Goi, 
       cfp->cf_id, pp_code_to_kstr(cfp->pp[as2][0])); */
    if (rep_malloc_flag) {
	free(mrph_str);
	rep_malloc_flag = 0;
    }

    /* if (ret = _get_ex_probability(key)) { */
    if (ret = _get_ex_probability_internal(key, as2, cfp)) {
	ret = log(ret);
    }
    /* ��ɽɽ���ξ���ALT��Ĵ�٤� */
    else if (OptCaseFlag & OPT_CASE_USE_REP_CF) {
	int rep_length;
	FEATURE *fp = dp->head_ptr->f;
	MRPH_DATA m;

	while (fp) {
	    if (!strncmp(fp->cp, "ALT-", 4)) {
		sscanf(fp->cp + 4, "%[^-]-%[^-]-%[^-]-%d-%d-%d-%d-%[^\n]", 
		       m.Goi2, m.Yomi, m.Goi, 
		       &m.Hinshi, &m.Bunrui, 
		       &m.Katuyou_Kata, &m.Katuyou_Kei, m.Imi);
		mrph_str = get_mrph_rep(&m); /* ��ɽɽ�� */
		rep_length = get_mrph_rep_length(mrph_str);
		if (rep_length == 0) { /* �ʤ���к�� */
		    mrph_str = make_mrph_rn(&m);
		    rep_length = strlen(mrph_str);
		    rep_malloc_flag = 1;
		}

		key = malloc_db_buf(rep_length + 1);
		strncpy(key, mrph_str, rep_length);
		*(key + rep_length) = '\0';
		if (rep_malloc_flag) {
		    free(mrph_str);
		    rep_malloc_flag = 0;
		}

		if (ret = _get_ex_probability_internal(key, as2, cfp)) {
		    ret = log(ret);
		    return ret;
		}
	    }
	    fp = fp->next;
	}
	ret = FREQ0_ASSINED_SCORE;
    }
    /* else if (ret = _get_sm_probability(dp, as2, cfp)) { * ��̣�Ǥ�back-off *
	ret = log(ret);
    } */
    /* else if (MatchPP(cfp->pp[as2][0], "���δط�") && 
	     (ret = _get_soto_default_probability(dp, as2, cfp))) {
	ret = log(ret);
    } */
    else {
	ret = FREQ0_ASSINED_SCORE; /* ����0�� */
    }

    return ret;
}

/*==================================================================*/
    double get_ex_probability_with_para(int as1, CASE_FRAME *cfd,
					int as2, CASE_FRAME *cfp)
/*==================================================================*/
{
    int j, count = 1;
    TAG_DATA *tp = cfd->pred_b_ptr->cpm_ptr->elem_b_ptr[as1];
    double score;

    /* ��ʬ���� */
    score = get_ex_probability(as1, cfd, NULL, as2, cfp);

    if (OptCKY) {
	return score;
    }

    /* ��ʬ����������� */
    if (tp->para_top_p) {
	for (j = 1; tp->child[j]; j++) { /* 0�ϼ�ʬ��Ʊ�� */
	    if (tp->child[j]->para_type == PARA_NORMAL) {
		score += get_ex_probability(-1, cfd, tp->child[j], as2, cfp);
		count++;
	    }
	}
    }

    return score / count;
}

/*==================================================================*/
    double get_np_modifying_probability(int as1, CASE_FRAME *cfd)
/*==================================================================*/
{
    int dist = 0;
    char *type = NULL, *key, *value;
    double ret;

    if (CaseExist == FALSE) {
	return 0;
    }

    /* tp -> hp */
    if (cfd->pred_b_ptr->cpm_ptr->elem_b_ptr[as1]->num > cfd->pred_b_ptr->num) { /* Ϣ�ν��� */
	if (type = check_feature(cfd->pred_b_ptr->f, "�Ѹ�")) {
	    type += strlen("�Ѹ�:");
	}

	/* ��������å� */
	dist = get_dist_from_work_mgr(cfd->pred_b_ptr->b_ptr, 
				      cfd->pred_b_ptr->cpm_ptr->elem_b_ptr[as1]->b_ptr);
	if (dist <= 0) {
	    return UNKNOWN_CASE_SCORE;
	}
	else if (dist > 1) {
	    dist = 2;
	}
    }

    key = malloc_db_buf(10);
    sprintf(key, "%s,%d|R", type ? type : "NIL", dist);
    if (value = db_get(case_db, key)) {
	if (VerboseLevel >= VERBOSE3) {
	    fprintf(Outfp, ";; (RE) %s -> %s: P(%s,%d|R) = %s\n", 
		    type ? cfd->pred_b_ptr->head_ptr->Goi : "NIL", 
		    cfd->pred_b_ptr->cpm_ptr->elem_b_ptr[as1]->head_ptr->Goi, 
		    type ? type : "NIL", dist, value);
	}

	ret = log(atof(value));
	free(value);
    }
    else {
	ret = UNKNOWN_CASE_SCORE;
    }

    return ret;
}

/*==================================================================*/
double get_topic_generating_probability(int have_topic, TAG_DATA *g_ptr)
/*==================================================================*/
{
    int topic_score = 0;
    char *cp, *key, *value;
    double ret;

    if (CaseExist == FALSE) {
	return 0;
    }

    /* ���ꥹ���� */
    if (cp = check_feature(g_ptr->f, "�����")) {
	sscanf(cp, "%*[^:]:%d", &topic_score);
	if (topic_score > 0 && topic_score < 30) {
	    topic_score = 10;
	}
    }

    key = malloc_db_buf(7);
    sprintf(key, "%d|W:%d", have_topic, topic_score);
    if (value = db_get(case_db, key)) {
	if (VerboseLevel >= VERBOSE3) {
	    fprintf(Outfp, ";; (W) %s: P(%d|W:%d) = %s\n", 
		    g_ptr->head_ptr->Goi, have_topic, 
		    topic_score, value);
	}

	ret = log(atof(value));
	free(value);
    }
    else {
	ret = UNKNOWN_CASE_SCORE;
    }

    return ret;
}

/*==================================================================*/
     double get_case_interpret_probability(int as1, CASE_FRAME *cfd,
					   int as2, CASE_FRAME *cfp)
/*==================================================================*/
{
    int wa_flag, topic_score = 0, touten_flag, i, dist, negation_flag, 	np_modifying_flag, closest_pred_flag = 0;
    char *scase, *cp, *key, *value, *value2, *value3, *vtype;
    double ret;
    TAG_DATA *tp, *tp2, *hp;

    if (CaseExist == FALSE) {
	return 0;
    }

    /* tp -> hp */
    if (cfd->pred_b_ptr->cpm_ptr->elem_b_ptr[as1]->num > cfd->pred_b_ptr->num) { /* Ϣ�ν��� */
	tp = cfd->pred_b_ptr;
	hp = cfd->pred_b_ptr->cpm_ptr->elem_b_ptr[as1];
	np_modifying_flag = 1;
    }
    else {
	tp = cfd->pred_b_ptr->cpm_ptr->elem_b_ptr[as1];
	hp = cfd->pred_b_ptr;
	np_modifying_flag = 0;
    }

    if (vtype = check_feature(hp->f, "�Ѹ�")) {
	vtype += strlen("�Ѹ�:");
    }

    /* ʣ�缭 */
    if (cfd->pp[as1][0] > 8 && cfd->pp[as1][0] < 38) {
	scase = pp_code_to_kstr(cfd->pp[as1][0]); /* ����¦��ɽ�س� */
	tp2 = &(current_sentence_data.tag_data[tp->num + 1]); /* ������֤ϡפʤɤ�����å����륿��ñ�� */
    }
    else { 
	if ((scase = check_feature(tp->f, "��")) == NULL) {
	    return UNKNOWN_CASE_SCORE;
	}
	scase += 3; /* ����¦��ɽ�س� */
	tp2 = tp;
    }

    /* �٤��Ѹ������뤫�ɤ��� */
    if (np_modifying_flag == 0) {
	if (get_dist_from_work_mgr(tp2->b_ptr, current_sentence_data.tag_data[tp2->num + 1].b_ptr) > 0) {
	    closest_pred_flag = 1;
	}
    }

    /* �֤ϡ�, ����, ����Υ����å� */
    wa_flag = check_feature(tp2->f, "��") ? 1 : 0;
    touten_flag = check_feature(tp2->f, "����") ? 1 : 0;
    negation_flag = check_feature(hp->f, "����ɽ��")   ? 1 
		  : check_feature(hp->f, "������ɽ��") ? 1
		  : 0;

    /* ���ꥹ���� */
    if (cp = check_feature(hp->f, "�����")) {
	sscanf(cp, "%*[^:]:%d", &topic_score);
	if (topic_score > 0 && topic_score < 30) {
	    topic_score = 10;
	}
    }

    /* ��������å� */
    dist = get_dist_from_work_mgr(tp2->b_ptr, hp->b_ptr);
    if (dist <= 0) {
	return UNKNOWN_CASE_SCORE;
    }
    else if (dist > 1) {
	dist = 2;
    }

    /* �ʤβ�� */
    if (as2 != NIL_ASSIGNED) {
	key = malloc_db_buf(strlen(scase) + strlen(pp_code_to_kstr(cfp->pp[as2][0])) + 20);
	sprintf(key, "%s|C:%s", scase, pp_code_to_kstr(cfp->pp[as2][0]));
    }
    else {
	key = malloc_db_buf(strlen(scase) + 24);
	sprintf(key, "%s|C:--", scase);
    }
    value = db_get(case_db, key);

    /* ���������� */
    if (np_modifying_flag) {
	sprintf(key, "%d|PϢ��:%d", touten_flag, dist);
    }
    else {
	sprintf(key, "%d|P:%d,%d,%d,%d", touten_flag, dist, closest_pred_flag, topic_score, wa_flag);
    }
    value2 = db_get(case_db, key);

    /* �֤ϡפ����� */
    if (np_modifying_flag || !vtype) {
	value3 = "1.0";
    }
    else {
	/* sprintf(key, "%d|T:%d,%d,%d,%d,%d", wa_flag, dist, closest_pred_flag, topic_score, touten_flag, negation_flag); */
	sprintf(key, "%d|T:%d,%d,%d,%d,%d,%d", wa_flag, dist, closest_pred_flag, topic_score, touten_flag, negation_flag, strcmp(vtype, "Ƚ") == 0 ? 1 : 0);
	value3 = db_get(case_db, key);
    }

    if (value && value2 && value3) {
	ret = atof(value) * atof(value2) * atof(value3);
	if (VerboseLevel >= VERBOSE3) {
	    fprintf(Outfp, ";; (CC) %s -> %s: P(%s,%d,%d|%s,%d,%d,%d,%d) = %lf (C:%s * P:%s * T:%s)\n", 
		    tp->head_ptr->Goi, hp->head_ptr->Goi, 
		    scase, touten_flag, wa_flag, as2 == NIL_ASSIGNED ? "--" : pp_code_to_kstr(cfp->pp[as2][0]), 
		    dist, closest_pred_flag, topic_score, negation_flag, ret, value, value2, value3);
	}
	free(value);
	free(value2);
	if (!np_modifying_flag && vtype) {
	    free(value3);
	}
	ret = log(ret);
    }
    else {
	if (VerboseLevel >= VERBOSE3) {
	    fprintf(Outfp, ";; (CC) %s -> %s: P(%s,%d,%d|%s,%d,%d,%d,%d) = 0 (C:%s * P:%s * T:%s)\n", 
		    tp->head_ptr->Goi, hp->head_ptr->Goi, 
		    scase, touten_flag, wa_flag, as2 == NIL_ASSIGNED ? "--" : pp_code_to_kstr(cfp->pp[as2][0]), 
		    dist, closest_pred_flag, topic_score, negation_flag, value ? value : "", value2 ? value2 : "", value3 ? value3 : "");
	}
	ret = UNKNOWN_CASE_SCORE;
    }

    return ret;
}

/*==================================================================*/
double calc_vp_modifying_probability(TAG_DATA *gp, CASE_FRAME *g_cf, TAG_DATA *dp, CASE_FRAME *d_cf)
/*==================================================================*/
{
    int touten_flag, dist, closest_pred_flag = 0;
    char *g_pred, *d_pred, *g_id, *d_id, *key, *value, *g_level;
    double ret1 = 0, ret2 = 0, ret3 = 0;

    if (RenyouExist == FALSE) {
	return 0;
    }

    /* EOS -> ʸ�� */
    if (gp == NULL) {
	g_pred = strdup("EOS");
    }
    else if (g_cf) {
	g_pred = strdup(g_cf->cf_id);
	sscanf(g_cf->cf_id, "%[^0-9]:%*d", g_pred);
    }
    else {
	g_pred = NULL;
    }

    /* �Ѹ� -> �Ѹ� */
    if (g_pred && d_cf) {
	d_pred = strdup(d_cf->cf_id);
	sscanf(d_cf->cf_id, "%[^0-9]:%*d", d_pred);

	key = malloc_db_buf(strlen(g_pred) + strlen(d_pred) + 2);
	sprintf(key, "%s|%s", d_pred, g_pred);
	value = db_get(renyou_db, key);
	if (value) {
	    ret3 = atof(value);
	    free(value);
	}

	if (VerboseLevel >= VERBOSE2) {
	    fprintf(Outfp, ";; (R) %s: P(%s|%s) = %lf\n", gp ? gp->head_ptr->Goi : "EOS", d_pred, g_pred, ret3);
	}

	free(g_pred);
	free(d_pred);
    }
    else {
	ret3 = 1;
    }

    if (gp == NULL) {
	if (ret3) {
	    return log(ret3);
	}
	else {
	    return UNKNOWN_RENYOU_SCORE;
	}
    }


    touten_flag = check_feature(dp->b_ptr->f, "����") ? 1 : 0;

    if ((dist = get_dist_from_work_mgr(dp->b_ptr, gp->b_ptr)) < 0) {
	return UNKNOWN_RENYOU_SCORE;
    }

    if (get_dist_from_work_mgr(dp->b_ptr, dp->b_ptr + 1) > 0) {
	closest_pred_flag = 1;
    }

    /* ���������� */
    key = malloc_db_buf(strlen("Ϣ��") + 8);
    sprintf(key, "%d|PϢ��:%d,%d", touten_flag, dist, closest_pred_flag);
    value = db_get(case_db, key);
    if (value) {
	ret1 = atof(value);
	if (VerboseLevel >= VERBOSE2) {
	    fprintf(Outfp, ";; (R1) %s: P(%d|PϢ��:%d,%d) = %lf\n", gp->head_ptr->Goi, touten_flag, dist, closest_pred_flag, ret1);
	}
	free(value);
    }

    /* ID -> ID */
    if ((g_id = check_feature(gp->f, "ID")) && 
	(d_id = check_feature(dp->f, "ID"))) {
	g_id += 3;
	d_id += 3;
	key = malloc_db_buf(strlen(g_id) + strlen(d_id) + 2);
	sprintf(key, "%s|%s", d_id, g_id);
	value = db_get(renyou_db, key);
	if (value) {
	    ret2 = atof(value);
	    if (VerboseLevel >= VERBOSE2) {
		fprintf(Outfp, ";; (R) %s: P(%s|%s) = %lf\n", gp->head_ptr->Goi, d_id, g_id, ret2);
	    }
	    free(value);
	}
    }

    /* �ʥե졼��ID => �ʥե졼��ID *
    if (0 && g_cf && d_cf) {
	key = malloc_db_buf(strlen(g_cf->cf_id) + strlen(d_cf->cf_id) + 2);
	sprintf(key, "%s|%s", d_cf->cf_id, g_cf->cf_id);
	value = db_get(renyou_db, key);
	if (value) {
	    ret3 = atof(value);
	    if (VerboseLevel >= VERBOSE3) {
		fprintf(Outfp, ";; (R) %s: P(%s|%s) = %lf\n", gp->head_ptr->Goi, d_cf->cf_id, g_cf->cf_id, ret3);
	    }
	    free(value);
	}
    }
    else {
	ret3 = 1;
    }
    */

    /* ��٥� => ��٥� *
    if ((g_level = check_feature(gp->f, "��٥�")) == NULL) {
	return UNKNOWN_RENYOU_SCORE;
    }
    g_level += 7;

    key = malloc_db_buf(strlen(g_level) + 6);
    sprintf(key, "%d|%d,%s", check_feature(dp->f, "����") ? 1 : 0, dist, g_level);
    value = db_get(renyou_func_db, key);
    if (value) {
	ret3 = atof(value);
	if (VerboseLevel >= VERBOSE3) {
	    fprintf(Outfp, ";; (R) %s -> %s: P(,=%d|%d,%s) = %lf\n", dp->head_ptr->Goi, gp->head_ptr->Goi, 
		    check_feature(dp->f, "����") ? 1 : 0, dist, g_level, ret3);
	}
	free(value);
    }
    ret3 = 1;
    */

    if (ret1) {
	ret1 = log(ret1);
    }
    else {
	ret1 = UNKNOWN_RENYOU_SCORE;
    }
    if (ret2) {
	ret2 = log(ret2);
    }
    else {
	ret2 = UNKNOWN_RENYOU_SCORE;
    }
    if (ret3) {
	ret3 = log(ret3);
    }
    else {
	ret3 = UNKNOWN_RENYOU_SCORE;
    }
    return ret1 + ret2 + ret3;
}

/*==================================================================*/
double calc_vp_modifying_num_probability(TAG_DATA *t_ptr, CASE_FRAME *cfp, int num)
/*==================================================================*/
{
    char *pred, *id, *key, *value;
    double ret1 = 0, ret2 = 0;

    if (RenyouExist == FALSE) {
	return 0;
    }

    /* ID */
    if (id = check_feature(t_ptr->f, "ID")) {
	id += 3;
	key = malloc_db_buf(strlen(id) + 6);
	sprintf(key, "%d|N:%s", num, id);
	value = db_get(renyou_db, key);
	if (value) {
	    ret1 = atof(value);
	    if (VerboseLevel >= VERBOSE2) {
		fprintf(Outfp, ";; (RN) %s: P(%d|%s) = %lf\n", t_ptr->head_ptr->Goi, num, id, ret1);
	    }
	    free(value);
	}
    }

    if (cfp) {
	pred = strdup(cfp->cf_id);
	sscanf(cfp->cf_id, "%[^0-9]:%*d", pred);
	key = malloc_db_buf(strlen(pred) + 6);
	sprintf(key, "%d|N:%s", num, pred);
	value = db_get(renyou_db, key);
	if (value) {
	    ret2 = atof(value);
	    free(value);
	}

	if (VerboseLevel >= VERBOSE2) {
	    fprintf(Outfp, ";; (RN) %s: P(%d|%s) = %lf\n", t_ptr->head_ptr->Goi, num, pred, ret2);
	}

	free(pred);
    }
    else {
	ret2 = 1;
    }

    /* �ʥե졼�� *
    if (0 && cfp) {
	key = malloc_db_buf(strlen(cfp->cf_id) + 6);
	sprintf(key, "%d|N:%s", num, cfp->cf_id);
	value = db_get(renyou_db, key);
	if (value) {
	    ret2 = atof(value);
	    if (VerboseLevel >= VERBOSE3) {
		fprintf(Outfp, ";; (R) %s: P(%d|%s) = %lf\n", t_ptr->head_ptr->Goi, num, cfp->cf_id, ret2);
	    }
	    free(value);
	}
    }
    else {
	ret2 = 1;
    }
    */

    if (ret1 && ret2) {
	return log(ret1) + log(ret2);
    }
    else {
	return UNKNOWN_RENYOU_SCORE;
    }
}

/*==================================================================*/
double calc_adv_modifying_probability(TAG_DATA *gp, CASE_FRAME *cfp, TAG_DATA *dp)
/*==================================================================*/
{
    char *pred, *key, *value;
    int touten_flag, dist;
    double ret1, ret2;

    if (AdverbExist == FALSE) {
	return 0;
    }

    /* ���� -> �Ѹ� */
    if (cfp) {
	pred = strdup(cfp->cf_id);
	sscanf(cfp->cf_id, "%[^0-9]:%*d", pred);
	key = malloc_db_buf(strlen(pred) + strlen(dp->head_ptr->Goi) + 2);
	sprintf(key, "%s|%s", dp->head_ptr->Goi, pred);
	value = db_get(adverb_db, key);
	if (value) {
	    ret1 = atof(value);
	    free(value);
	    if (VerboseLevel >= VERBOSE1) {
		fprintf(Outfp, ";; (A) P(%s) = %lf\n", key, ret1);
	    }
	    ret1 = log(ret1);
	}
	else {
	    if (VerboseLevel >= VERBOSE1) {
		fprintf(Outfp, ";; (A) P(%s) = 0\n", key);
	    }
	    ret1 = UNKNOWN_RENYOU_SCORE;
	}
	free(pred);


	/* ���������� */
	touten_flag = check_feature(dp->b_ptr->f, "����") ? 1 : 0;

	if ((dist = get_dist_from_work_mgr(dp->b_ptr, gp->b_ptr)) < 0) {
	    ret2 = UNKNOWN_RENYOU_SCORE;
	}
	else {
	    int closest_pred_flag = 0;

	    if (get_dist_from_work_mgr(dp->b_ptr, dp->b_ptr + 1) > 0) {
		closest_pred_flag = 1;
	    }
	    key = malloc_db_buf(strlen("����") + 8);
	    sprintf(key, "%d|P����:%d,%d", touten_flag, dist, closest_pred_flag);
	    value = db_get(case_db, key);
	    if (value) {
		ret2 = atof(value);
		if (VerboseLevel >= VERBOSE1) {
		    fprintf(Outfp, ";; (A_P) [%s -> %s] : P(%s) = %lf\n", dp->head_ptr->Goi, gp->head_ptr->Goi, key, ret2);
		}
		free(value);
		ret2 = log(ret2);
	    }
	    else {
		ret2 = UNKNOWN_RENYOU_SCORE;
		if (VerboseLevel >= VERBOSE1) {
		    fprintf(Outfp, ";; (A_P) [%s -> %s] : P(%s) = 0\n", dp->head_ptr->Goi, gp->head_ptr->Goi, key);
		}
	    }
	}
	return ret1 + ret2;
    }
    else {
	return 0;
    }
}

/*==================================================================*/
double calc_adv_modifying_num_probability(TAG_DATA *t_ptr, CASE_FRAME *cfp, int num)
/*==================================================================*/
{
    char *pred, *key, *value;
    double ret = 0;

    if (AdverbExist == FALSE) {
	return 0;
    }

    if (cfp) {
	pred = strdup(cfp->cf_id);
	sscanf(cfp->cf_id, "%[^0-9]:%*d", pred);
	key = malloc_db_buf(strlen(pred) + 6);
	sprintf(key, "%d|N:%s", num, pred);
	value = db_get(adverb_db, key);
	if (value) {
	    ret = atof(value);
	    free(value);
	}

	if (VerboseLevel >= VERBOSE2) {
	    fprintf(Outfp, ";; (AN) %s: P(%d|%s) = %lf\n", t_ptr->head_ptr->Goi, num, pred, ret);
	}

	free(pred);

	if (ret) {
	    return log(ret);
	}
	else {
	    return UNKNOWN_RENYOU_SCORE;
	}
    }
    else {
	return 0;
    }
}

/*==================================================================*/
		  int bin_sim_score(double score)
/*==================================================================*/
{
    if (score < 1.0) {
	return 0;
    }
    else if (score < 2.0) {
	return 1;
    }
    else if (score < 3.0) {
	return 2;
    }
    else if (score < 4.0) {
	return 3;
    }
    else {
	return 4;
    }
}

/*==================================================================*/
double get_para_exist_probability(char *para_key, double score, int flag)
/*==================================================================*/
{
    char *key, *value;
    double ret1 = 0, ret2 = 0;
    int binned_score = bin_sim_score(score);

    if (CaseExist == FALSE) {
	return 0;
    }

    key = malloc_db_buf(strlen(para_key) + 12);
    if (flag) {
	sprintf(key, "1,%d|PARA:%s", binned_score, para_key);
    }
    else {
	sprintf(key, "0|PARA:%s", para_key);
    }
    value = db_get(case_db, key);
    if (value) {
	ret1 = atof(value);
	if (VerboseLevel >= VERBOSE1) {
	    fprintf(Outfp, ";; (PARA) : P(%s) = %lf\n", key, ret1);
	}
	free(value);
    }
    else {
	if (VerboseLevel >= VERBOSE1) {
	    fprintf(Outfp, ";; (PARA) : P(%s) = 0\n", key);
	}
    }

    /* 
    sprintf(key, "%s|PTYPE:%d", para_key, binned_score);
    value = db_get(case_db, key);
    if (value) {
	ret2 = atof(value);
	if (VerboseLevel >= VERBOSE1) {
	    fprintf(Outfp, ";; (PTYPE) : P(%s) = %lf\n", key, ret2);
	}
	free(value);
    }
    else {
	if (VerboseLevel >= VERBOSE1) {
	    fprintf(Outfp, ";; (PTYPE) : P(%s) = 0\n", key);
	}
    }
    */

    if (ret1) {
	ret1 = log(ret1);
    }
    else {
	ret1 = UNKNOWN_RENYOU_SCORE;
    }
    /* if (ret2) {
	ret2 = log(ret2);
    }
    else {
	ret2 = UNKNOWN_RENYOU_SCORE;
	} */

    return ret1 + ret2;
}

/*==================================================================*/
double get_para_ex_probability(char *para_key, double score, TAG_DATA *dp, TAG_DATA *gp)
/*==================================================================*/
{
    char *key, *value;
    double ret;

    if (ParaExist == FALSE) {
	return 0;
    }

    if (!strcmp(dp->head_ptr->Goi, gp->head_ptr->Goi)) {
	return 0;
    }

    key = malloc_db_buf(strlen(dp->head_ptr->Goi) + strlen(para_key) + strlen(gp->head_ptr->Goi) + 5);
    /* sprintf(key, "%s|%d,%s,%s", dp->head_ptr->Goi, bin_sim_score(score), para_key, gp->head_ptr->Goi); */
    sprintf(key, "%s|%s,%s", dp->head_ptr->Goi, para_key, gp->head_ptr->Goi);

    value = db_get(para_db, key);
    if (value) {
	ret = atof(value);
	if (VerboseLevel >= VERBOSE1) {
	    fprintf(Outfp, ";; (PARA_EX) : P(%s) = %lf\n", key, ret);
	}
	free(value);
    }
    else {
	if (VerboseLevel >= VERBOSE1) {
	    fprintf(Outfp, ";; (PARA_EX) : P(%s) = 0\n", key);
	}
	return FREQ0_ASSINED_SCORE;
    }

    if (ret) {
	return log(ret);
    }
    else {
	return FREQ0_ASSINED_SCORE;
    }
}

/*==================================================================*/
double get_noun_co_ex_probability(TAG_DATA *dp, TAG_DATA *gp)
/*==================================================================*/
{
    char *key, *value;
    int touten_flag, dist;
    double ret1, ret2;

    if (NounCoExist == FALSE) {
	return 0;
    }

    key = malloc_db_buf(strlen(dp->head_ptr->Goi) + strlen(gp->head_ptr->Goi) + 2);
    sprintf(key, "%s|%s", dp->head_ptr->Goi, gp->head_ptr->Goi);

    value = db_get(noun_co_db, key);
    if (value) {
	ret1 = atof(value);
	if (VerboseLevel >= VERBOSE1) {
	    fprintf(Outfp, ";; (NOUN_EX) : P(%s) = %lf\n", key, ret1);
	}
	free(value);
	ret1 = log(ret1);
    }
    else {
	if (VerboseLevel >= VERBOSE1) {
	    fprintf(Outfp, ";; (NOUN_EX) : P(%s) = 0\n", key);
	}
	ret1 = FREQ0_ASSINED_SCORE;
    }

    /* ���������� */
    touten_flag = check_feature(dp->b_ptr->f, "����") ? 1 : 0;

    if ((dist = get_dist_from_work_mgr(dp->b_ptr, gp->b_ptr)) < 0) {
	ret2 = FREQ0_ASSINED_SCORE;
    }
    else {
	int closest_ok_flag = 0;

	if (get_dist_from_work_mgr(dp->b_ptr, dp->b_ptr + 1) > 0) {
	    closest_ok_flag = 1;
	}
	key = malloc_db_buf(strlen("Ϣ��") + 8);
	sprintf(key, "%d|PϢ��:%d,%d", touten_flag, dist, closest_ok_flag);
	value = db_get(case_db, key);
	if (value) {
	    ret2 = atof(value);
	    if (VerboseLevel >= VERBOSE1) {
		fprintf(Outfp, ";; (NOUN_P) [%s -> %s] : P(%s) = %lf\n", dp->head_ptr->Goi, gp->head_ptr->Goi, key, ret2);
	    }
	    free(value);
	    ret2 = log(ret2);
	}
	else {
	    ret2 = FREQ0_ASSINED_SCORE;
	    if (VerboseLevel >= VERBOSE1) {
		fprintf(Outfp, ";; (NOUN_P) [%s -> %s] : P(%s) = 0\n", dp->head_ptr->Goi, gp->head_ptr->Goi, key);
	    }
	}
    }

    return ret1 + ret2;
}

/*==================================================================*/
      double get_noun_co_num_probability(TAG_DATA *gp, int num)
/*==================================================================*/
{
    char *key, *value;
    double ret;

    if (NounCoExist == FALSE) {
	return 0;
    }

    key = malloc_db_buf(strlen(gp->head_ptr->Goi) + 6);
    sprintf(key, "%d|N:%s", num, gp->head_ptr->Goi);

    value = db_get(noun_co_db, key);
    if (value) {
	ret = atof(value);
	if (VerboseLevel >= VERBOSE1) {
	    fprintf(Outfp, ";; (NOUN_NUM) : P(%s) = %lf\n", key, ret);
	}
	free(value);
	ret = log(ret);
    }
    else {
	if (VerboseLevel >= VERBOSE1) {
	    fprintf(Outfp, ";; (NOUN_NUM) : P(%s) = 0\n", key);
	}
	ret = FREQ0_ASSINED_SCORE;
    }

    return ret;
}

/* get case-frame probability for Chinese */
/*==================================================================*/
   double get_chi_case_probability(BNST_DATA *g_ptr, BNST_DATA *d_ptr)
/*==================================================================*/
{
    char *key, *value;
    double ret;

    if (CHICaseExist == FALSE) {
	return 0;
    }

    key = malloc_db_buf(strlen(g_ptr->head_ptr->Goi) + 
			strlen(d_ptr->head_ptr->Goi) + 2);

    /* �Ѹ�ɽ���Ǥ�ä������褤�ߤ��� */
    sprintf(key, "%s:%s", g_ptr->head_ptr->Goi, d_ptr->head_ptr->Goi);
    value = db_get(chi_case_db, key);

    if (value) {
	ret = atof(value);
	free(value);
    }
    else {
	ret = 0.0;
    }

    return ret;
}

/*====================================================================
                               END
====================================================================*/
