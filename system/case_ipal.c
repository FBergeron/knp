/*====================================================================

		       �ʹ�¤����: �ʥե졼��¦

                                               S.Kurohashi 93. 5.31

    $Id$
====================================================================*/
#include "knp.h"

FILE *cf_fp;
DBM_FILE cf_db;

CASE_FRAME 	*Case_frame_array = NULL; 	/* �ʥե졼�� */
int 	   	Case_frame_num;			/* �ʥե졼��� */
int 	   	MAX_Case_frame_num = 0;		/* ����ʥե졼��� */

CF_FRAME CF_frame;
int MAX_cf_frame_length = 0;
unsigned char *cf_str_buf;

int	CFExist;
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
	OptAnalysis == OPT_CASE2) {

	/* �ʥե졼���ΰ���� */
	Case_frame_array = (CASE_FRAME *)malloc_data(sizeof(CASE_FRAME)*ALL_CASE_FRAME_MAX, 
						     "init_case_analysis_cpm");
	MAX_Case_frame_num = ALL_CASE_FRAME_MAX;
	init_cf_structure(Case_frame_array, MAX_Case_frame_num);

	/* Best_mgr��cpm�ΰ���� */
	init_mgr_cf(sp->Best_mgr);
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
	     char *get_ipal_address(unsigned char *word)
/*==================================================================*/
{
    if (CFExist == FALSE)
	return NULL;

    return db_get(cf_db, word);
}

/*==================================================================*/
	   CF_FRAME *get_ipal_frame(int address, int size)
/*==================================================================*/
{
    int i, c1, c2, count = 0;
    char *cp;

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

    fseek(cf_fp, (long)address, 0);
    if (fread(CF_frame.DATA, size, 1, cf_fp) < 1) {
	fprintf(stderr, "Error in fread.\n");
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
		else if (count >= CF_ELEMENT_MAX) {
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

    if (cp = index(CF_frame.DATA, ':')) {
	strncpy(CF_frame.pred_type, cp+1, 2);
	CF_frame.pred_type[2] = '\0';
    }
    else {
	CF_frame.pred_type[0] = '\0';
    }

    return &CF_frame;
}

/*==================================================================*/
unsigned char *extract_ipal_str(unsigned char *dat, unsigned char *ret)
/*==================================================================*/
{
    if (*dat == '\0' || !strcmp(dat, "��")) 
      return NULL;

    while (1) {
	if (*dat == '\0') {
	    *ret = '\0';
	    return dat;
	}
	/* ���٤����Ҥ��Ƥ����� */
	else if (*dat == ':') {
	    *ret++ = '\0';	/* ':' -> '\0' */
	    dat++;
	}
	/* ����Ǥ��ڤ� */
	else if (*dat == ' ') {
	    *ret = '\0';
	    return dat+1;
	}
	else if (*dat < 0x80) {
	    *ret++ = *dat++;
	}
	else if (!strncmp(dat, "��", 2) || 
		 !strncmp(dat, "��", 2) ||
		 !strncmp(dat, "��", 2) ||
		 !strncmp(dat, "��", 2)) {
	    *ret = '\0';
	    return dat+2;
	}
	else {
	    *ret++ = *dat++;
	    *ret++ = *dat++;
	}
    }
}

/*==================================================================*/
int _make_ipal_cframe_pp(CASE_FRAME *c_ptr, unsigned char *cp, int num)
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
    else if (!strcmp(cp+strlen(cp)-2, "��")) {
	c_ptr->adjacent[num] = TRUE;
	*(cp+strlen(cp)-2) = '\0';
    }

    /* Ǥ�ճ� */
    if (!strcmp(cp+strlen(cp)-2, "��")) {
	c_ptr->oblig[num] = FALSE;
    }
    else {
	/* ̤���� (�ҤȤĤ�γʤ򸫤Ʒ���) */
	c_ptr->oblig[num] = END_M;
    }

    point = cp; 
    while ((point = extract_ipal_str(point, cf_str_buf))) {
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
	c_ptr->pp[num][pp_num] = pp_kstr_to_code(cf_str_buf);
	if (c_ptr->pp[num][pp_num] == END_M) {
	    fprintf(stderr, ";; Unknown case (%s) in PP!\n", cf_str_buf);
	    exit(1);
	}
	pp_num++;
    }

    c_ptr->pp[num][pp_num] = END_M;
    return TRUE;
}
    
/*==================================================================*/
void _make_ipal_cframe_sm(CASE_FRAME *c_ptr, unsigned char *cp, int num, int flag)
/*==================================================================*/
{
    /* ��̣�ޡ������ɤߤ��� */

    unsigned char *point;
    int size, sm_num = 0, sm_print_num = 0, mlength, sm_delete_sm_max = 0;
    char buf[SM_ELEMENT_MAX*SM_CODE_SIZE], *sm_delete_sm = NULL, *temp, *str;

    if (*cp == '\0') {
	return;
    }

    if (flag & USE_BGH) {
	size = BGH_CODE_SIZE;
    }
    else if (flag & USE_NTT) {
	size = SM_CODE_SIZE;
    }

    str = strdup(cp);
    *str = '\0';
    point = cp;
    buf[0] = '\0';
    while ((point = extract_ipal_str(point, cf_str_buf))) {
	/* �ȤäƤϤ����ʤ���̣�� (NTT) */
        if (cf_str_buf[0] == '-') {
	    if (Thesaurus == USE_BGH) continue;
	    if (c_ptr->sm_delete[num] == NULL) {
		c_ptr->sm_delete_size[num] = SM_ELEMENT_MAX;
		c_ptr->sm_delete[num] = (char *)malloc_data(
		    sizeof(char)*c_ptr->sm_delete_size[num]*SM_CODE_SIZE+1, 
		    "_make_ipal_cframe_sm");
		*c_ptr->sm_delete[num] = '\0';

		if (PrintDeletedSM) {
		    sm_delete_sm_max = sizeof(char)*ALLOCATION_STEP;
		    sm_delete_sm = (char *)malloc_data(sm_delete_sm_max, 
						       "_make_ipal_cframe_sm");
		    strcpy(sm_delete_sm, "��̣�Ǻ��:");
		}
		else {
		    sm_delete_sm = strdup("��̣�Ǻ��");
		}
	    }
	    else if (c_ptr->sm_delete_num[num] >= c_ptr->sm_delete_size[num]) {
		c_ptr->sm_delete[num] = (char *)realloc_data(c_ptr->sm_delete[num], 
		    sizeof(char)*(c_ptr->sm_delete_size[num] <<= 1)*SM_CODE_SIZE+1, 
		    "_make_ipal_cframe_sm");
	    }

	    /* NTT code���񤤤Ƥ���Ȥ� */
	    if (cf_str_buf[1] == '1') {
		strcat(c_ptr->sm_delete[num], &cf_str_buf[1]);

		if (PrintDeletedSM) {
		    temp = code2sm(&cf_str_buf[1]);
		    /* -1 �ǤϤʤ��Τ� '/' ��ʬ */
		    if (strlen(sm_delete_sm)+strlen(temp) > sm_delete_sm_max-2) {
			sm_delete_sm = (char *)realloc_data(sm_delete_sm, 
							    sm_delete_sm_max <<= 1, 
							    "_make_ipal_cframe_sm");
		    }
		    strcat(sm_delete_sm, "/");
		    strcat(sm_delete_sm, temp);
		}
	    }
	    else {
		/* NULL: core dump */
		strcat(c_ptr->sm_delete[num], sm2code(&cf_str_buf[1]));
	    }
	    c_ptr->sm_delete_num[num]++;
	    continue;
	}

	/* ���̤ΰ�̣�� */

	sm_num++;
	sm_print_num++;
	if (sm_num >= SM_ELEMENT_MAX){
	    fprintf(stderr, ";;; Not enough sm_num !!\n");
	    break;
	}

	if (!strncmp(cf_str_buf, "����", 4)) {
	    /* �����<����>�ΤȤ�������ʤ� */
	    if (sm_num > 1 && !strncmp(&buf[size*(sm_num-2)], sm2code("����"), size)) {
		sm_num--;
	    }
	    else {
		strcat(buf, sm2code("����"));
	    }
	}
	else if (!strncmp(cf_str_buf, "���ν�", 6)) {
	    strcat(buf, sm2code("����"));
	    c_ptr->etcflag |= CF_GA_SEMI_SUBJECT;
	}
	else {
	    strcat(buf, sm2code(cf_str_buf));
	}

	if ((flag & STOREtoCF) && sm_print_num <= EX_PRINT_NUM) {
	    if (str[0])	strcat(str, "/");
	    strcat(str, cf_str_buf);
	}
    }

    if (buf[0]) {
	c_ptr->sm[num] = strdup(buf);
    }

    if (flag & STOREtoCF) {
	if (sm_print_num <= EX_PRINT_NUM) {
	    mlength = strlen(str)+1;
	    if (sm_delete_sm) mlength += strlen(sm_delete_sm)+1; /* "/"��ʬ */
	    c_ptr->semantics[num] = (char *)malloc_data(sizeof(char)*mlength, 
							"_make_ipal_cframe_sm");
	    strcpy(c_ptr->semantics[num], str);
	    if (sm_delete_sm) {
		if (str[0]) strcat(c_ptr->semantics[num], "/");
		strcat(c_ptr->semantics[num], sm_delete_sm);
	    }
	}
	else {
	    /* "...\0" �� 4 ��ʬ���䤹 */
	    mlength = strlen(str)+4;
	    if (sm_delete_sm) mlength += strlen(sm_delete_sm)+2; /* "/"��ʬ */
	    c_ptr->semantics[num] = (char *)malloc_data(sizeof(char)*mlength, 
							"_make_ipal_cframe_sm");
	    strcpy(c_ptr->semantics[num], str);
	    if (sm_delete_sm) {
		if (str[0]) strcat(c_ptr->semantics[num], "/");
		strcat(c_ptr->semantics[num], sm_delete_sm);
	    }
	    strcat(c_ptr->semantics[num], "...");
	}
    }
    free(str);
    if (sm_delete_sm) free(sm_delete_sm);
}

/*==================================================================*/
void _make_ipal_cframe_ex(CASE_FRAME *c_ptr, unsigned char *cp, int num, int flag)
/*==================================================================*/
{
    /* ����ɤߤ��� */

    unsigned char *point, *point2;
    int max, count = 0, length = 0, thesaurus = USE_NTT;
    char *code, **destination, *buf;

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
    while ((point = extract_ipal_str(point, cf_str_buf))) {
	point2 = cf_str_buf;

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
		if (strlen(buf) + strlen(code) >= max) {
		    /* fprintf(stderr, "Too many EX <%s> (%2dth).\n", cf_str_buf, count); */
		    free(code);
		    break;
		}
		strcat(buf, code);
		free(code);
	    }

	    if (c_ptr->ex_size[num] == 0) {
		c_ptr->ex_size[num] = 10;	/* ������ݿ� */
		c_ptr->ex_list[num] = (char **)malloc_data(sizeof(char *)*c_ptr->ex_size[num], 
							    "_make_ipal_cframe_ex");
	    }
	    else if (c_ptr->ex_num[num] >= c_ptr->ex_size[num]) {
		c_ptr->ex_list[num] = (char **)realloc_data(c_ptr->ex_list[num], 
							    sizeof(char *)*(c_ptr->ex_size[num] <<= 1), 
							    "_make_ipal_cframe_ex");
	    }
	    c_ptr->ex_list[num][c_ptr->ex_num[num]++] = strdup(point2);
	}

	count++;
	if (flag & STOREtoCF && count == EX_PRINT_NUM) {
	    length = point-cp-2;
	}
    }

    *destination = strdup(buf);
    free(buf);

    if (flag & STOREtoCF) {
	if (*cp) {
	    if (count <= EX_PRINT_NUM) {
		c_ptr->examples[num] = strdup(cp);
	    }
	    else {
		/* "...\0" �� 4 ��ʬ���䤹 */
		c_ptr->examples[num] = (char *)malloc_data(sizeof(char)*(length+4), 
							   "_make_ipal_cframe_ex");
		strncpy(c_ptr->examples[num], cp, length);
		*(c_ptr->examples[num]+length) = '\0';
		strcat(c_ptr->examples[num], "...");
	    }
	}
	else {
	    c_ptr->examples[num] = NULL;
	}
    }
}

/*==================================================================*/
       int check_examples(char *cp, char **ex_list, int ex_num)
/*==================================================================*/
{
    int i;

    if (!ex_list) {
	return -1;
    }

    for (i = 0; i < ex_num; i++) {
	if (str_eq(cp, *(ex_list+i))) {
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
    while ((point = extract_ipal_str(point, cf_str_buf)))
	if (!strcmp(cf_str_buf, "��")) return TRUE;
    return FALSE;
}

/*==================================================================*/
     void _make_ipal_cframe(CF_FRAME *i_ptr, CASE_FRAME *cf_ptr,
			    int address, int size, char *verb)
/*==================================================================*/
{
    int i, j = 0, ga_p = FALSE, c1, c2, count = 0;
    char ast_cap[32], *token, *cp, *buf;

    cf_ptr->cf_address = address;
    cf_ptr->cf_size = size;
    cf_ptr->concatenated_flag = 0;
    strcpy(cf_ptr->cf_id, i_ptr->DATA); 
    cf_ptr->etcflag = i_ptr->etcflag;
    cf_ptr->entry = strdup(verb);
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


    /* �����Ǥ��ɲ� */

    if (cf_ptr->voice == FRAME_PASSIVE_I ||
	cf_ptr->voice == FRAME_CAUSATIVE_WO_NI ||
	cf_ptr->voice == FRAME_CAUSATIVE_WO ||
	cf_ptr->voice == FRAME_CAUSATIVE_NI) {
	_make_ipal_cframe_pp(cf_ptr, "��", j);
	_make_ipal_cframe_sm(cf_ptr, "���ν�", j, 
			     Thesaurus == USE_NTT ? USE_NTT_WITH_STORE : USE_BGH_WITH_STORE);
	_make_ipal_cframe_ex(cf_ptr, "��", j, Thesaurus);
	j++;
    }

    /* �Ƴ����Ǥν��� */

    for (i = 0; i < i_ptr->casenum && j < CASE_MAX_NUM; i++, j++) { 
	cf_ptr->adjacent[j] = FALSE;
	if (_make_ipal_cframe_pp(cf_ptr, i_ptr->cs[i].kaku_keishiki, j) == FALSE) {
	    j--;
	    continue;
	}
	if (Thesaurus == USE_BGH) {
	    _make_ipal_cframe_ex(cf_ptr, i_ptr->cs[i].meishiku, j, USE_BGH_WITH_STORE);
	    _make_ipal_cframe_sm(cf_ptr, i_ptr->cs[i].imisosei, j, USE_BGH_WITH_STORE);
	}
	else if (Thesaurus == USE_NTT) {
	    _make_ipal_cframe_ex(cf_ptr, i_ptr->cs[i].meishiku, j, USE_NTT_WITH_STORE);
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
		      _make_ipal_cframe_pp(cf_ptr, "�򡿥�", j);
		    else if (cf_ptr->voice == FRAME_CAUSATIVE_WO)
		      _make_ipal_cframe_pp(cf_ptr, "��", j);
		    else if (cf_ptr->voice == FRAME_CAUSATIVE_NI)
		      _make_ipal_cframe_pp(cf_ptr, "��", j);
		} else {
		    _make_ipal_cframe_pp(cf_ptr, "��", j); /* ��������ʸ */
		}
	    }
	}
	else if (cf_ptr->voice == FRAME_PASSIVE_I ||	/* ���� */
		 cf_ptr->voice == FRAME_PASSIVE_1 ||
		 cf_ptr->voice == FRAME_PASSIVE_2) {
	    /* ���� �����ˡ�ľ�� �����ˡ��˥�åơ����� */
	    if (!strcmp(i_ptr->cs[i].kaku_keishiki, "��")) {
		if (cf_ptr->voice == FRAME_PASSIVE_I)
		  _make_ipal_cframe_pp(cf_ptr, "��", j);
		else if (cf_ptr->voice == FRAME_PASSIVE_1)
		  _make_ipal_cframe_pp(cf_ptr, 
				       "�ˡ��˥�롿����", j);
		else if (cf_ptr->voice == FRAME_PASSIVE_2)
		  _make_ipal_cframe_pp(cf_ptr, 
				       "�ˡ��˥�롿����", j);
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
		_make_ipal_cframe_pp(cf_ptr, "��", j);
	    }
	}
	else if (cf_ptr->voice == FRAME_POSSIBLE) {	/* ��ǽ */
	    if (!strcmp(i_ptr->cs[i].kaku_keishiki, "��")) {
		_make_ipal_cframe_pp(cf_ptr, "������", j);
	     }
	}
	else if (cf_ptr->voice == FRAME_SPONTANE) {	/* ��ȯ */
	    if (!strcmp(i_ptr->cs[i].kaku_keishiki, "��")) {
		_make_ipal_cframe_pp(cf_ptr, "��", j);
	     }
	}
    }
    cf_ptr->element_num = j;
}

/*==================================================================*/
BNST_DATA *get_quasi_closest_case_component(SENTENCE_DATA *sp, BNST_DATA *b_ptr)
/*==================================================================*/
{
    BNST_DATA *tp;

    if (b_ptr->num < 1) {
	return NULL;
    }

    if (check_feature(b_ptr->f, "ID:�ʡ���ˡ���")) {
	return b_ptr;
    }

    tp = sp->bnst_data+b_ptr->num-1;

    if (!check_feature(tp->f, "�θ�")) {
	return NULL;
    }

    if (check_feature(tp->f, "�ؼ���") || 
	(tp->SM_code[0] == '\0' && 
	 check_feature(tp->f, "��:����"))) {
	return NULL;
    }

    if (check_feature(tp->f, "��:���") || 
	check_feature(tp->f, "��:�˳�") || 
	(!cf_match_element(tp->SM_code, "����", FALSE) && 
	(check_feature(tp->f, "��:����") || 
	 check_feature(tp->f, "��:�����") || 
	 check_feature(tp->f, "��:�س�") || 
	 check_feature(tp->f, "��:����") || 
	 check_feature(tp->f, "��:�ȳ�") || 
	 check_feature(tp->f, "��:�ޥǳ�")))) {
	return tp;
    }
    return NULL;
}

/*==================================================================*/
		  char *feature2case(BNST_DATA *bp)
/*==================================================================*/
{
    char *cp, *buffer;

    if (check_feature(bp->f, "ID:�ʡ���ˡ���")) {
	buffer = (char *)malloc_data(3, "feature2case");
	strcpy(buffer, "��");
	return buffer;
    }
    else if ((cp = check_feature(bp->f, "��"))) {
	buffer = strdup(cp+3);
	if (!strncmp(buffer+strlen(buffer)-2, "��", 2)) {
	    *(buffer+strlen(buffer)-2) = '\0';
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
int _make_ipal_cframe_subcontract(SENTENCE_DATA *sp, BNST_DATA *b_ptr, int start, 
				  char *verb, int voice)
/*==================================================================*/
{
    CF_FRAME *i_ptr;
    CASE_FRAME *cf_ptr;
    BNST_DATA *cbp;
    int f_num = 0, address, break_flag = 0, size, match, c;
    char *pre_pos, *cp, *address_str, *vtype = NULL;

    if (!verb)
    	return f_num;

    cf_ptr = Case_frame_array+start;

    /* ľ�������Ǥ򤯤äĤ��Ƹ��� */
    cbp = get_quasi_closest_case_component(sp, b_ptr);
    if (cbp) {
	char *buffer, *pp;

	pp = feature2case(cbp);
	if (pp) {
	    buffer = (char *)malloc_data(strlen(L_Jiritu_M(cbp)->Goi)+strlen(pp)+strlen(verb)+3, 
					 "_make_ipal_cframe_subcontract");
	    sprintf(buffer, "%s-%s-%s", L_Jiritu_M(cbp)->Goi, pp, verb);
	    address_str = get_ipal_address(buffer);
	    free(buffer);
	    free(pp);
	}
	if (!pp || !address_str) {
	    address_str = get_ipal_address(verb);
	}
    }
    else {
	address_str = get_ipal_address(verb);
    }

    /* �ʤ���� */
    if (!address_str)
	return f_num;

    if ((vtype = check_feature(b_ptr->f, "�Ѹ�"))) {
	vtype += 5;
    }
    else if ((vtype = check_feature(b_ptr->f, "����"))) {
	vtype = "ư";
    }
    else if ((vtype = check_feature(b_ptr->f, "̾��Ū���ƻ�촴"))) {
	vtype = "��";
    }
    else if ((vtype = check_feature(b_ptr->f, "���Ѹ�"))) {
	;
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

	    i_ptr = get_ipal_frame(address, size);
	    pre_pos = cp + 1;

	    /* �Ѹ��Υ����פ��ޥå����ʤ���� (���Ѹ��ʤ��̲�) */
	    if (vtype) {
		if (strncmp(vtype, i_ptr->pred_type, 2)) {
		    if (break_flag)
			break;
		    else
			continue;
		}
	    }

	    /* ǽư�� */
	    if (voice == 0) {
		(cf_ptr+f_num)->voice = FRAME_ACTIVE;
		_make_ipal_cframe(i_ptr, cf_ptr+f_num, address, size, verb);

		/* �ʥե졼����� */
		if (b_ptr->voice == VOICE_SHIEKI) {
		    /* �Ȥꤢ���� */
		    (cf_ptr+f_num)->voice = FRAME_CAUSATIVE_NI;
		}
		/* �ʥե졼����� */
		else if (b_ptr->voice) {
		    /* �˳ʤ��ʤ��Ȥ� */
		    if ((c = check_cf_case(cf_ptr+f_num, "��")) < 0) {
			_make_ipal_cframe_pp(cf_ptr+f_num, "��", (cf_ptr+f_num)->element_num);
			_make_ipal_cframe_sm(cf_ptr+f_num, "���ν�", (cf_ptr+f_num)->element_num, 
					     Thesaurus == USE_NTT ? USE_NTT_WITH_STORE : USE_BGH_WITH_STORE);
			(cf_ptr+f_num)->element_num++;
		    }
		    /* �˳ʤϤ��뤱��<����>���ʤ��Ȥ� */
		    else if (sm_match_check(sm2code("����"), (cf_ptr+f_num)->sm[c]) == FALSE) {
			_make_ipal_cframe_sm(cf_ptr+f_num, "���ν�", c, 
					     Thesaurus == USE_NTT ? USE_NTT_WITH_STORE : USE_BGH_WITH_STORE);
		    }
		    (cf_ptr+f_num)->voice = FRAME_PASSIVE_1;
		}
		
		f_num_inc(start, &f_num);
		cf_ptr = Case_frame_array+start;
	    }

	    /* ���� */
	    if (voice == VOICE_SHIEKI ||
		voice == VOICE_MORAU) {
		if (i_ptr->voice & CF_CAUSATIVE_WO && i_ptr->voice & CF_CAUSATIVE_NI)
		  (cf_ptr+f_num)->voice = FRAME_CAUSATIVE_WO_NI;
		else if (i_ptr->voice & CF_CAUSATIVE_WO)
		  (cf_ptr+f_num)->voice = FRAME_CAUSATIVE_WO;
		else if (i_ptr->voice & CF_CAUSATIVE_NI)
		  (cf_ptr+f_num)->voice = FRAME_CAUSATIVE_NI;
		
		_make_ipal_cframe(i_ptr, cf_ptr+f_num, address, size, verb);
		f_num_inc(start, &f_num);
		cf_ptr = Case_frame_array+start;
	    }
	    
	    /* ���� */
	    if (voice == VOICE_UKEMI ||
		voice == VOICE_MORAU) {
		/* ľ�ܼ��ȣ� */
		if (i_ptr->voice & CF_PASSIVE_1) {
		    (cf_ptr+f_num)->voice = FRAME_PASSIVE_1;
		    _make_ipal_cframe(i_ptr, cf_ptr+f_num, address, size, verb);
		    f_num_inc(start, &f_num);
		    cf_ptr = Case_frame_array+start;
		}
		/* ľ�ܼ��ȣ� */
		if (i_ptr->voice & CF_PASSIVE_2) {
		    (cf_ptr+f_num)->voice = FRAME_PASSIVE_2;
		    _make_ipal_cframe(i_ptr, cf_ptr+f_num, address, size, verb);
		    f_num_inc(start, &f_num);
		    cf_ptr = Case_frame_array+start;
		}
		/* ���ܼ��� */
		if (i_ptr->voice & CF_PASSIVE_I) {
		    (cf_ptr+f_num)->voice = FRAME_PASSIVE_I;
		    _make_ipal_cframe(i_ptr, cf_ptr+f_num, address, size, verb);
		    f_num_inc(start, &f_num);
		    cf_ptr = Case_frame_array+start;
		}
	    }
	    /* ��ǽ��º�ɡ���ȯ */
	    if (voice == VOICE_UKEMI) {
		if (i_ptr->voice & CF_POSSIBLE) {
		    (cf_ptr+f_num)->voice = FRAME_POSSIBLE;
		    _make_ipal_cframe(i_ptr, cf_ptr+f_num, address, size, verb);
		    f_num_inc(start, &f_num);
		    cf_ptr = Case_frame_array+start;
		}
		if (i_ptr->voice & CF_POLITE) {
		    (cf_ptr+f_num)->voice = FRAME_POLITE;
		    _make_ipal_cframe(i_ptr, cf_ptr+f_num, address, size, verb);
		    f_num_inc(start, &f_num);
		    cf_ptr = Case_frame_array+start;
		}
		if (i_ptr->voice & CF_SPONTANE) {
		    (cf_ptr+f_num)->voice = FRAME_SPONTANE;
		    _make_ipal_cframe(i_ptr, cf_ptr+f_num, address, size, verb);
		    f_num_inc(start, &f_num);
		    cf_ptr = Case_frame_array+start;
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
int make_ipal_cframe_subcontract(SENTENCE_DATA *sp, BNST_DATA *b_ptr, int start, char *verb)
/*==================================================================*/
{
    int f_num = 0, plus_num;

    if (check_feature(b_ptr->f, "���ѻ�")) {
	/* ǽư�� 
	   b_ptr->voice == 0 �ǹԤ� */
	b_ptr->voice = 0;
	f_num = _make_ipal_cframe_subcontract(sp, b_ptr, start, verb, 0);
	b_ptr->voice = VOICE_UNKNOWN;
    }

    /* ����, ����γʥե졼�� */
    if (b_ptr->voice == VOICE_UKEMI ||
	b_ptr->voice == VOICE_MORAU || 
	b_ptr->voice == VOICE_UNKNOWN || 
	b_ptr->voice == VOICE_SHIEKI) {
	if (b_ptr->voice == VOICE_SHIEKI) {
	    strcat(verb, ":C");
	}
	else {
	    strcat(verb, ":P");
	}
	plus_num = _make_ipal_cframe_subcontract(sp, b_ptr, start+f_num, verb, 0);
	if (plus_num != 0) {
	    return f_num+plus_num;
	}
	*(verb+strlen(verb)-2) = '\0';	/* �ߤĤ���ʤ��ä����Ȥˤ�ɤ� */
    }

    if (b_ptr->voice == VOICE_UNKNOWN) {
	/* ���� */
	f_num += _make_ipal_cframe_subcontract(sp, b_ptr, start+f_num, verb, VOICE_UKEMI);
	return f_num;
    }
    return _make_ipal_cframe_subcontract(sp, b_ptr, start, verb, b_ptr->voice);
}

/*==================================================================*/
 int make_ipal_cframe(SENTENCE_DATA *sp, BNST_DATA *b_ptr, int start)
/*==================================================================*/
{
    int f_num = 0, i;
    char *verb, buffer[3][WORD_LEN_MAX];

    /* ��Ω����������Ѥ��Ƴʥե졼�༭������ */

    if (!b_ptr->jiritu_ptr) {
	return f_num;
    }

    /* �֡ʡ���ˡ��ˡ� �ΤȤ��� �֤���� ��õ�� */
    if (check_feature(b_ptr->f, "ID:�ʡ���ˡ���")) {
	sprintf(buffer[0], "����");
	verb = buffer[0];
    }
    /* �֡�����, �֡�Ū���� �� ̾��+������(��Ω��) �η��Τ��
       (���Ѹ��ǤϤʤ� �� �֡�ǯ�פʤɤλ��֤����) */
    else if (b_ptr->jiritu_num > 1 && !strcmp(Class[L_Jiritu_M(b_ptr)->Hinshi][0].id, "������") && 
	     !check_feature(b_ptr->f, "���Ѹ�")) {
	sprintf(buffer[0], "%s%s", (b_ptr->jiritu_ptr+b_ptr->jiritu_num-2)->Goi, L_Jiritu_M(b_ptr)->Goi);
	verb = buffer[0];

	/* �ַ��ƻ�+����� */
	if (check_feature(b_ptr->f, "���ƻ쥹��")) {
	    strcat(verb, "����");
	}

	f_num = make_ipal_cframe_subcontract(sp, b_ptr, start, verb);
	if (f_num != 0) {
	    Case_frame_num += f_num;
	    for (i = 0; i < f_num; i++) {
		(Case_frame_array+start+i)->concatenated_flag = 0;
	    }
	    return f_num;
	}
	strcpy(buffer[0], L_Jiritu_M(b_ptr)->Goi);
    }
    else {
	strcpy(buffer[0], L_Jiritu_M(b_ptr)->Goi);
	verb = buffer[0];
    }

    /* �ַ��ƻ�+����� */
    if (check_feature(b_ptr->f, "���ƻ쥹��")) {
	strcat(verb, "����");
    }

    f_num = make_ipal_cframe_subcontract(sp, b_ptr, start, verb);
    Case_frame_num += f_num;
    for (i = 0; i < f_num; i++) {
	(Case_frame_array+start+i)->concatenated_flag = 0;
    }

    return f_num;
}

/*==================================================================*/
	 int make_default_cframe(BNST_DATA *b_ptr, int start)
/*==================================================================*/
{
    int i, num = 0, f_num = 0;
    CASE_FRAME *cf_ptr;

    cf_ptr = Case_frame_array+start;

    if (MAX_cf_frame_length == 0) {
	cf_str_buf = 
	    (unsigned char *)realloc_data(cf_str_buf, 
					  sizeof(unsigned char)*ALLOCATION_STEP, 
					  "make_default_cframe");
    }

    if (check_feature(b_ptr->f, "�Ѹ�:Ƚ")) {
	_make_ipal_cframe_pp(cf_ptr, "����", num);
	_make_ipal_cframe_sm(cf_ptr, "���ν�", num++, 
			     Thesaurus == USE_NTT ? USE_NTT_WITH_STORE : USE_BGH_WITH_STORE);
	cf_ptr->cf_address = -1;	/* �� ɽ�����뤿��ˤ����ͤ��Ѥ���ɬ�פ����� */
	strcpy(cf_ptr->pred_type, "Ƚ");
    }
    else {
	_make_ipal_cframe_pp(cf_ptr, "����", num++);
	_make_ipal_cframe_pp(cf_ptr, "���", num++);
	_make_ipal_cframe_pp(cf_ptr, "�ˡ�", num++);
	_make_ipal_cframe_pp(cf_ptr, "�ء�", num++);
	_make_ipal_cframe_pp(cf_ptr, "����", num++);
	cf_ptr->cf_address = -1;
	cf_ptr->pred_type[0] = '\0';
    }

    cf_ptr->element_num = num;
    cf_ptr->etcflag = CF_NORMAL;

    for (i = 0; i < num; i++) {
	cf_ptr->pp[i][1] = END_M;
    }

    cf_ptr->samecase[0][0] = END_M;
    cf_ptr->samecase[0][1] = END_M;

    f_num_inc(start, &f_num);
    Case_frame_num++;
    b_ptr->cf_num = 1;
    return 1;
}

/*==================================================================*/
      void make_case_frames(SENTENCE_DATA *sp, BNST_DATA *b_ptr)
/*==================================================================*/
{
    if ((b_ptr->cf_num = make_ipal_cframe(sp, b_ptr, Case_frame_num)) == 0) {
	make_default_cframe(b_ptr, Case_frame_num);
    }
}

/*==================================================================*/
	      void set_pred_caseframe(SENTENCE_DATA *sp)
/*==================================================================*/
{
    int i, j, start;
    BNST_DATA  *b_ptr;

    Case_frame_num = 0;

    for (i = 0, b_ptr = sp->bnst_data; i < sp->Bnst_num; i++, b_ptr++) {
	/* ���򥳡��ѥ������Ϥ����Ȥ��˼�Ω�줬�ʤ���礬���� */
	if (b_ptr->jiritu_ptr != NULL && 
	    ((!(b_ptr->internal_num > 0 && 
		check_feature(b_ptr->internal->f, "���Ѹ��ʲ���") && 
		check_feature(b_ptr->f, "�Ѹ�:ư")) && /* (ʣ��)����ư��ξ���ʸ�����������ǳʲ��� */
	      check_feature(b_ptr->f, "�Ѹ�")) || /* ���Ѹ��ϤȤꤢ�����оݳ� */
	     (check_feature(b_ptr->f, "���Ѹ��ʲ���") && b_ptr->internal_num == 0))) { /* ʣ��̾��ǤϤʤ�����̾��, ���ƻ�촴 */

	    /* �ʲ���2�Ĥν�����feature��٥�ǵ�ư���Ƥ���
	       set_pred_voice(b_ptr); ��������
	       get_scase_code(b_ptr); ɽ�س� */

	    start = Case_frame_num;
	    make_case_frames(sp, b_ptr);
	}
	else {
	    b_ptr->cf_ptr = NULL;
	    b_ptr->cf_num = 0;
	}

	/* ʸ�������Υ���̾��, ���ƻ�촴�β����� */
	for (j = 0; j < b_ptr->internal_num; j++) {
	    if (check_feature((b_ptr->internal+j)->f, "���Ѹ��ʲ���")) {
		start = Case_frame_num;
		make_case_frames(sp, b_ptr->internal+j);
	    }
	}
    }

    /* ��ʸ�ᤫ��ʥե졼��ؤΥ���դ� */
    start = 0;
    for (i = 0, b_ptr = sp->bnst_data; i < sp->Bnst_num; i++, b_ptr++) {
	if (b_ptr->cf_num) {
	    b_ptr->cf_ptr = Case_frame_array+start;
	    start += b_ptr->cf_num;
	}

	for (j = 0; j < b_ptr->internal_num; j++) {
	    if ((b_ptr->internal+j)->cf_num) {
		(b_ptr->internal+j)->cf_ptr = Case_frame_array+start;
		start += (b_ptr->internal+j)->cf_num;
	    }
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
	    if ((Case_frame_array+i)->ex_list[j]) {
		for (k = 0; k < (Case_frame_array+i)->ex_num[j]; k++) {
		    if ((Case_frame_array+i)->ex_list[j][k]) {
			free((Case_frame_array+i)->ex_list[j][k]);
		    }
		}
		free((Case_frame_array+i)->ex_list[j]);
		(Case_frame_array+i)->ex_list[j] = NULL;
		(Case_frame_array+i)->ex_size[j] = 0;
		(Case_frame_array+i)->ex_num[j] = 0;
	    }
	    if ((Case_frame_array+i)->examples[j]) {
		free((Case_frame_array+i)->examples[j]);
		(Case_frame_array+i)->examples[j] = NULL;
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
	      void make_internal_bnst(SENTENCE_DATA *sp)
/*==================================================================*/
{
    int i, j, k, suffix_num;
    BNST_DATA *bp;

    /* ʸ�����ʣ��̾��Τ��줾��򲾤�ʸ��Ȥ��ƹͤ��� */

    for (i = 0; i < sp->Bnst_num; i++) {
	bp = sp->bnst_data+i;
	bp->internal_num = 0;

	if (bp->jiritu_ptr == NULL || 
	    bp->jiritu_num < 2) {
	    continue;
	}

	suffix_num = 0;

	for (j = 1; j <= bp->jiritu_num; j++) {
	    /* ̾��, ����, ���ƻ�촴�Ǥ���� */
	    /* ʣ��̾��Ǥ��ꡢHead ��ľ��������Ǥʤ�
	       (�֣���Ϣ���פʤɤ����) */
	    if ((!strcmp(Class[(bp->jiritu_ptr+bp->jiritu_num-j)->Hinshi][0].id, "̾��") && 
		 strcmp(Class[(bp->jiritu_ptr+bp->jiritu_num-j)->Hinshi][(bp->jiritu_ptr+bp->jiritu_num-j)->Bunrui].id, "����")) || 
		!strcmp(Class[(bp->jiritu_ptr+bp->jiritu_num-j)->Hinshi][0].id, "����") || 
		(!strcmp(Class[(bp->jiritu_ptr+bp->jiritu_num-j)->Hinshi][0].id, "���ƻ�") && 
		 !strcmp(Form[(bp->jiritu_ptr+bp->jiritu_num-j)->Katuyou_Kata][(bp->jiritu_ptr+bp->jiritu_num-j)->Katuyou_Kei].name, "�촴"))) {
		if (bp->internal_num == 0) {
		    bp->internal = (BNST_DATA *)malloc_data(sizeof(BNST_DATA), "make_internal_bnst");
		    bp->internal_max = 1;
		}
		else if (bp->internal_num >= bp->internal_max) {
		    bp->internal = (BNST_DATA *)realloc_data(bp->internal, 
							     sizeof(BNST_DATA)*(bp->internal_max <<= 1), 
							     "make_internal_bnst");
		}

		memset(bp->internal+bp->internal_num, 0, sizeof(BNST_DATA));
		(bp->internal+bp->internal_num)->num = -1;
		(bp->internal+bp->internal_num)->mrph_num = suffix_num+1;
		(bp->internal+bp->internal_num)->mrph_ptr = bp->jiritu_ptr+bp->jiritu_num-j;
		(bp->internal+bp->internal_num)->jiritu_num = suffix_num+1;
		(bp->internal+bp->internal_num)->jiritu_ptr = bp->jiritu_ptr+bp->jiritu_num-j;
		strcpy((bp->internal+bp->internal_num)->Jiritu_Go, 
		       (bp->internal+bp->internal_num)->jiritu_ptr->Goi);

		get_bnst_code(bp->internal+bp->internal_num, USE_BGH);
		get_bnst_code(bp->internal+bp->internal_num, USE_NTT);

		/* ʸ��롼���Ŭ�Ѥ��� */
		assign_cfeature(&((bp->internal+bp->internal_num)->f), "ʸ����");
		_assign_general_feature(bp->internal+bp->internal_num, 1, BnstRuleType);
		assign_cfeature(&((bp->internal+bp->internal_num)->f), "��:ʸ����");

		suffix_num = 0;
		bp->internal_num++;
	    }
	    /* ����������¸���Ƥ����ơ�̾�줬�褿�Ȥ��ˤ��äĤ��� */
	    else if (!strcmp(Class[(bp->jiritu_ptr+bp->jiritu_num-j)->Hinshi][0].id, "������")) {
		suffix_num++;
	    }
	    else {
		suffix_num = 0;
	    }
	}

	for (j = 0; j < bp->internal_num; j++) {
	    /* �ƤȻҤΥ���դ� */
	    if (j > 0) { /* �缭(�ǽ�Τ��)�ʳ� */
		(bp->internal+j)->parent = bp->internal+j-1;
		t_add_node(bp->internal+j-1, bp->internal+j, -1); /* �� */
	    }
	    /* �缭 */
	    else {
		bp->internal->parent = bp;
	    }
	}
    }
}

/*==================================================================*/
		int find_upper_bnst_num(BNST_DATA *bp)
/*==================================================================*/
{
    while (bp->num == -1) {
	bp = bp->parent;
    }
    return bp->num;
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

/*====================================================================
                               END
====================================================================*/
