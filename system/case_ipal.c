/*====================================================================

		       �ʹ�¤����: �ʥե졼��¦

                                               S.Kurohashi 93. 5.31

    $Id$
====================================================================*/
#include "knp.h"

FILE *ipal_fp;
DBM_FILE ipal_db;

CASE_FRAME 	*Case_frame_array = NULL; 	/* �ʥե졼�� */
int 	   	Case_frame_num;			/* �ʥե졼��� */
int 	   	MAX_Case_frame_num = 0;		/* ����ʥե졼��� */

IPAL_FRAME Ipal_frame;
int MAX_ipal_frame_length = 0;
unsigned char *ipal_str_buf;

int	IPALExist;

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
	data_filename = (char *)check_dict_filename(DICT[CF_DATA], TRUE);
    }
    else {
	data_filename = (char *)check_dict_filename(IPAL_DAT_NAME, FALSE);
    }

    if (DICT[CF_INDEX_DB]) {
	index_db_filename = (char *)check_dict_filename(DICT[CF_INDEX_DB], TRUE);
    }
    else {
	index_db_filename = (char *)check_dict_filename(IPAL_DB_NAME, FALSE);
    }

    if ((ipal_fp = fopen(data_filename, "rb")) == NULL) {
#ifdef DEBUG
	fprintf(stderr, "Cannot open CF DATA <%s>.\n", data_filename);
#endif
	IPALExist = FALSE;
    }
    else if ((ipal_db = DBM_open(index_db_filename, O_RDONLY, 0)) == NULL) {
	fprintf(stderr, "Cannot open CF INDEX Database <%s>.\n", index_db_filename);
	/* �ʥե졼�� DATA ���ɤ��Τˡ�DB ���ɤ�ʤ��Ȥ��Ͻ���� */
	exit(1);
    } 
    else {
	IPALExist = TRUE;
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
	    if (Thesaurus == USE_BGH) {
		free(sp->Best_mgr->cpm[i].cf.ex[j]);
		sp->Best_mgr->cpm[i].cf.ex[j] = NULL;
	    }
	    else if (Thesaurus == USE_NTT) {
		free(sp->Best_mgr->cpm[i].cf.ex2[j]);
		sp->Best_mgr->cpm[i].cf.ex2[j] = NULL;
	    }
	    free(sp->Best_mgr->cpm[i].cf.sm[j]);
	    sp->Best_mgr->cpm[i].cf.sm[j] = NULL;
	}
    }
}

/*==================================================================*/
		 void init_mgr_cf(SENTENCE_DATA *sp)
/*==================================================================*/
{
    int i, j;

    for (i = 0; i < CPM_MAX; i++) {
	for (j = 0; j < CF_ELEMENT_MAX; j++) {
	    if (Thesaurus == USE_BGH) {
		sp->Best_mgr->cpm[i].cf.ex[j] = 
		    (char *)malloc_data(sizeof(char)*EX_ELEMENT_MAX*BGH_CODE_SIZE, "init_cf");
	    }
	    else if (Thesaurus == USE_NTT) {
		sp->Best_mgr->cpm[i].cf.ex2[j] = 
		    (char *)malloc_data(sizeof(char)*SM_ELEMENT_MAX*SM_CODE_SIZE, "init_cf");
	    }
	    sp->Best_mgr->cpm[i].cf.sm[j] = 
		(char *)malloc_data(sizeof(char)*SM_ELEMENT_MAX*SM_CODE_SIZE, "init_cf");
	}
    }
}

/*==================================================================*/
		   void init_cf2(SENTENCE_DATA *sp)
/*==================================================================*/
{
    if (OptAnalysis == OPT_CASE || 
	OptAnalysis == OPT_CASE2) {

	Case_frame_array = (CASE_FRAME *)malloc_data(sizeof(CASE_FRAME)*ALL_CASE_FRAME_MAX, "init_cf");
	MAX_Case_frame_num = ALL_CASE_FRAME_MAX;
	init_cf_structure(Case_frame_array, MAX_Case_frame_num);

	init_mgr_cf(sp);
    }
}

/*==================================================================*/
			   void close_cf()
/*==================================================================*/
{
    if (IPALExist == TRUE) {
	fclose(ipal_fp);
	DBM_close(ipal_db);
    }
}

/*==================================================================*/
	     char *get_ipal_address(unsigned char *word)
/*==================================================================*/
{
    if (IPALExist == FALSE)
	return NULL;

    return db_get(ipal_db, word);
}

/*==================================================================*/
	  IPAL_FRAME *get_ipal_frame(int address, int size)
/*==================================================================*/
{
    if (size > MAX_ipal_frame_length) {
	MAX_ipal_frame_length += ALLOCATION_STEP*((size-MAX_ipal_frame_length)/ALLOCATION_STEP+1);
	Ipal_frame.DATA = 
	    (unsigned char *)realloc_data(Ipal_frame.DATA, 
					  sizeof(unsigned char)*MAX_ipal_frame_length, 
					  "get_ipal_frame");
	ipal_str_buf = 
	    (unsigned char *)realloc_data(ipal_str_buf, 
					  sizeof(unsigned char)*MAX_ipal_frame_length, 
					  "get_ipal_frame");
    }

    fseek(ipal_fp, (long)address, 0);
    if (fread(&Ipal_frame, sizeof(IPAL_FRAME_INDEX), 1, ipal_fp) < 1) {
	fprintf(stderr, "Error in fread.\n");
	exit(1);
    }
    if (fread(Ipal_frame.DATA, size-sizeof(IPAL_FRAME_INDEX), 1, ipal_fp) < 1) {
	fprintf(stderr, "Error in fread.\n");
	exit(1);
    }
    return &Ipal_frame;
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
void _make_ipal_cframe_pp(CASE_FRAME *c_ptr, unsigned char *cp, int num)
/*==================================================================*/
{
    /* ������ɤߤ��� */

    unsigned char *point;
    int pp_num = 0;

    if (!strcmp(cp+strlen(cp)-2, "��")) {
	c_ptr->adjacent[num] = TRUE;
	*(cp+strlen(cp)-2) = '\0';
    }

    if (!strcmp(cp+strlen(cp)-2, "��")) {
	c_ptr->oblig[num] = FALSE;
    }
    else {
	/* ̤���� (�ҤȤĤ�γʤ򸫤Ʒ���) */
	c_ptr->oblig[num] = END_M;
    }

    point = cp; 
    while (point = extract_ipal_str(point, ipal_str_buf)) {
	if (pp_num == 0 && c_ptr->oblig[num] == END_M) {
	    if (str_eq(ipal_str_buf, "��") || 
		str_eq(ipal_str_buf, "��") || 
		str_eq(ipal_str_buf, "��") || 
		str_eq(ipal_str_buf, "��") || 
		str_eq(ipal_str_buf, "���")) {
		c_ptr->oblig[num] = TRUE;
	    }
	    else {
		c_ptr->oblig[num] = FALSE;
	    }
	}
	c_ptr->pp[num][pp_num++] = pp_kstr_to_code(ipal_str_buf);
    }
    c_ptr->pp[num][pp_num] = END_M;
}
    
/*==================================================================*/
void _make_ipal_cframe_sm(CASE_FRAME *c_ptr, unsigned char *cp, int num, int flag)
/*==================================================================*/
{
    /* ��̣�ޡ������ɤߤ��� */

    unsigned char *point;
    int i, sm_num = 0;
    char buf[SM_ELEMENT_MAX*SM_CODE_SIZE];

    if (*cp == '\0') {
	return;
    }

    point = cp;
    buf[0] = '\0';
    while (point = extract_ipal_str(point, ipal_str_buf)) {
        if (ipal_str_buf[0] == '-') {
	    if (c_ptr->sm_false[num] == NULL) {
		c_ptr->sm_false[num] = (int *)malloc_data(sizeof(int)*SM_ELEMENT_MAX, 
							  "_make_ipal_cframe_sm");
		for (i = 0; i < sm_num; i++) {
		    c_ptr->sm_false[num][i] = FALSE;
		}
	    }
	    c_ptr->sm_false[num][sm_num] = TRUE;
	    strcpy(ipal_str_buf, &ipal_str_buf[1]);
	}
	else if (c_ptr->sm_false[num]) {
	    c_ptr->sm_false[num][sm_num] = FALSE;
	}

	sm_num++;
	if (sm_num >= SM_ELEMENT_MAX){
	    fprintf(stderr, "Not enough sm_num !!\n");
	    break;
	}

	if (!strncmp(ipal_str_buf, "����", 4)) {
	    strcat(buf, (char *)sm2code("����"));
	}
	else {
	    strcat(buf, (char *)sm2code(ipal_str_buf));
	}
    }
    c_ptr->sm[num] = strdup(buf);

    if (flag & STOREtoCF) {
	c_ptr->semantics[num] = strdup(cp);
    }
}

/*==================================================================*/
void _make_ipal_cframe_ex(CASE_FRAME *c_ptr, unsigned char *cp, int num, int flag)
/*==================================================================*/
{
    /* ����ɤߤ��� */

    unsigned char *point, *point2;
    int i, max, count = 0, length = 0;
    char *code, **destination, *buf;
    extern char *get_bgh();
    extern char *get_sm();
    char *(*get_code)();

    if (*cp == '\0') {
	return;
    }

    /* �����꥽�����ˤ�äƴؿ��ʤɤ򥻥å� */
    if (flag & USE_BGH) {
	get_code = get_bgh;
	destination = &c_ptr->ex[num];
	max = EX_ELEMENT_MAX*BGH_CODE_SIZE;
    }
    else if (flag & USE_NTT) {
	get_code = get_sm;
	destination = &c_ptr->ex2[num];
	max = SM_ELEMENT_MAX*SM_CODE_SIZE;
    }

    /* �����ͤ��ʤ��Ȥ����ޤ���� */
    buf = (char *)malloc_data(sizeof(char)*max, "_make_ipal_cframe_ex");

    point = cp;
    *buf = '\0';
    while (point = extract_ipal_str(point, ipal_str_buf)) {
	point2 = ipal_str_buf;

	/* �֣��Σ¡פΡ֣¡פ��������
	for (i = strlen(point2) - 2; i > 2; i -= 2) {
	    if (!strncmp(point2+i-2, "��", 2)) {
		point2 += i;
		break;
	    }
	}
	*/

	if (*point2 != '\0') {
	    code = (char *)get_code(point2);
	    if (code) {
		if (strlen(buf) + strlen(code) >= max) {
		    fprintf(stderr, "Too many EX <%s> (%2dth).\n", ipal_str_buf, count);
		    free(code);
		    break;
		}
		strcat(buf, code);
		free(code);
	    }
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
      int check_examples(unsigned char *cp, unsigned char *list)
/*==================================================================*/
{
    if (!list) {
	return FALSE;
    }

    while (list = extract_ipal_str(list, ipal_str_buf)) {
	if (str_eq(cp, ipal_str_buf)) {
	    return TRUE;
	}
    }
    return FALSE;
}

/*==================================================================*/
		int check_agentive(unsigned char *cp)
/*==================================================================*/
{
    unsigned char *point;

    point = cp;
    while (point = extract_ipal_str(point, ipal_str_buf))
      if (!strcmp(ipal_str_buf, "��")) return TRUE;
    return FALSE;
}

/*==================================================================*/
  void _make_ipal_cframe(IPAL_FRAME *i_ptr, CASE_FRAME *cf_ptr, int address, int size)
/*==================================================================*/
{
    int i, j = 0, ga_p = NULL;
    char ast_cap[32];

    cf_ptr->ipal_address = address;
    cf_ptr->ipal_size = size;
    cf_ptr->concatenated_flag = 0;
    strcpy(cf_ptr->ipal_id, i_ptr->DATA+i_ptr->id); 
    strcpy(cf_ptr->imi, i_ptr->DATA+i_ptr->imi);
    /* OR �γʥե졼��ϡֽҸ��ǡפˡ��¥ե졼��פȽ񤤤Ƥ��� */
    if (str_eq(i_ptr->DATA+i_ptr->jyutugoso, "�¥ե졼��")) {
	cf_ptr->flag = CF_SUM;
    }
    else {
	cf_ptr->flag = CF_NORMAL;
    }

    /* �����Ǥ��ɲ� */

    if (cf_ptr->voice == FRAME_PASSIVE_I ||
	cf_ptr->voice == FRAME_CAUSATIVE_WO_NI ||
	cf_ptr->voice == FRAME_CAUSATIVE_WO ||
	cf_ptr->voice == FRAME_CAUSATIVE_NI) {
	_make_ipal_cframe_pp(cf_ptr, "��", j);
	/* _make_ipal_cframe_sm(cf_ptr, "�ģɣ֡��ȣգ�", j, Thesaurus); ����̵�� */
	_make_ipal_cframe_sm(cf_ptr, "����", j, USE_NTT_WITH_STORE);
	_make_ipal_cframe_ex(cf_ptr, "��", j, Thesaurus);
	j++;
    }

    /* �Ƴ����Ǥν��� */

    for (i = 0; i < CASE_MAX_NUM && j < CASE_MAX_NUM && *(i_ptr->DATA+i_ptr->kaku_keishiki[i]); i++, j++) { 
	cf_ptr->adjacent[j] = FALSE;
	_make_ipal_cframe_pp(cf_ptr, i_ptr->DATA+i_ptr->kaku_keishiki[i], j);
	_make_ipal_cframe_sm(cf_ptr, i_ptr->DATA+i_ptr->imisosei[i], j, USE_NTT_WITH_STORE);
	if (Thesaurus == USE_BGH) {
	    _make_ipal_cframe_ex(cf_ptr, i_ptr->DATA+i_ptr->meishiku[i], j, USE_BGH_WITH_STORE);
	}
	else if (Thesaurus == USE_NTT) {
	    _make_ipal_cframe_ex(cf_ptr, i_ptr->DATA+i_ptr->meishiku[i], j, USE_NTT_WITH_STORE);
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
		if (ga_p == NULL) {
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
	    if (!strcmp(i_ptr->DATA+i_ptr->kaku_keishiki[i], "��")) {
		if (cf_ptr->voice == FRAME_PASSIVE_I)
		  _make_ipal_cframe_pp(cf_ptr, "��", j);
		else if (cf_ptr->voice == FRAME_PASSIVE_1)
		  _make_ipal_cframe_pp(cf_ptr, 
				       i_ptr->DATA+i_ptr->tyoku_ukemi1, j);
		else if (cf_ptr->voice == FRAME_PASSIVE_2)
		  _make_ipal_cframe_pp(cf_ptr, 
				       i_ptr->DATA+i_ptr->tyoku_ukemi2, j);
	    }
	    /* ľ�� �ˡ��˥�åơ��������� */
	    else if ((cf_ptr->voice == FRAME_PASSIVE_1 && 
		      (!strcmp(i_ptr->DATA+i_ptr->kaku_keishiki[i], 
			       i_ptr->DATA+i_ptr->tyoku_noudou1) ||
		       (sprintf(ast_cap, "%s��", 
				i_ptr->DATA+i_ptr->tyoku_noudou1) &&
			!strcmp(i_ptr->DATA+i_ptr->kaku_keishiki[i], 
				ast_cap)))) ||
		     (cf_ptr->voice == FRAME_PASSIVE_2 && 
		      (!strcmp(i_ptr->DATA+i_ptr->kaku_keishiki[i], 
			       i_ptr->DATA+i_ptr->tyoku_noudou2) ||
		       (sprintf(ast_cap, "%s��", 
				i_ptr->DATA+i_ptr->tyoku_noudou2) &&
			!strcmp(i_ptr->DATA+i_ptr->kaku_keishiki[i], 
				ast_cap))))) {
		_make_ipal_cframe_pp(cf_ptr, "��", j);
	    }
	}
	else if (cf_ptr->voice == FRAME_POSSIBLE) {	/* ��ǽ */
	    if (!strcmp(i_ptr->DATA+i_ptr->kaku_keishiki[i], "��")) {
		 _make_ipal_cframe_pp(cf_ptr, "������", j);
	     }
	}
	else if (cf_ptr->voice == FRAME_SPONTANE) {	/* ��ȯ */
	    if (!strcmp(i_ptr->DATA+i_ptr->kaku_keishiki[i], "��")) {
		 _make_ipal_cframe_pp(cf_ptr, "��", j);
	     }
	}
    }
    cf_ptr->element_num = j;
}

/*==================================================================*/
		     void f_num_inc(int *f_num_p)
/*==================================================================*/
{
    (*f_num_p)++;
    if ((Case_frame_num + *f_num_p) >= MAX_Case_frame_num) {
	realloc_cf();
	realloc_cmm();
    }
}

/*==================================================================*/
int make_ipal_cframe_subcontract(BNST_DATA *b_ptr, int start, char *verb)
/*==================================================================*/
{
    IPAL_FRAME *i_ptr;
    CASE_FRAME *cf_ptr;
    int f_num = 0, address, break_flag = 0, size, match;
    char *pre_pos, *cp, *address_str, *vtype = NULL;

    if (!verb)
    	return f_num;

    cf_ptr = Case_frame_array+start;

    address_str = get_ipal_address(verb);

    /* �ʤ���� */
    if (!address_str)
	return f_num;

    if (vtype = (char *)check_feature(b_ptr->f, "�Ѹ�"))
	vtype += 5;

    for (cp = pre_pos = address_str; ; cp++) {
	if (*cp == '/' || *cp == '\0') {
	    if (*cp == '\0')
		break_flag = 1;
	    else 
		*cp = '\0';
	    
	    /* IPAL�ǡ������ɤߤ��� */
	    match = sscanf(pre_pos, "%d:%d", &address, &size);
	    if (match != 2) {
		fprintf(stderr, "CaseFrame Dictionary Index error (it seems version 1.).\n");
		exit(1);
	    }

	    i_ptr = get_ipal_frame(address, size);
	    pre_pos = cp + 1;

	    /* �Ѹ��Υ����פ��ޥå����ʤ���� (���Ѹ��ʤ��̲�) */
	    if (vtype && strncmp(i_ptr->DATA+i_ptr->id, vtype, 2)) {
		if (break_flag)
		    break;
		else
		    continue;
	    }

	    (cf_ptr+f_num)->entry = strdup(verb);

	    /* ǽư�� */
	    if (b_ptr->voice == NULL) {
		(cf_ptr+f_num)->voice = FRAME_ACTIVE;
		_make_ipal_cframe(i_ptr, cf_ptr+f_num, address, size);
		f_num_inc(&f_num);
		cf_ptr = Case_frame_array+start;
	    }

	    /* ���� */
	    if (b_ptr->voice == VOICE_SHIEKI ||
		b_ptr->voice == VOICE_MORAU) {
		if (!strcmp(i_ptr->DATA+i_ptr->sase, "����򡤥˻���"))
		  (cf_ptr+f_num)->voice = FRAME_CAUSATIVE_WO_NI;
		else if (!strcmp(i_ptr->DATA+i_ptr->sase, "�����"))
		  (cf_ptr+f_num)->voice = FRAME_CAUSATIVE_WO;
		else if (!strcmp(i_ptr->DATA+i_ptr->sase, "�˻���"))
		  (cf_ptr+f_num)->voice = FRAME_CAUSATIVE_NI;
		
		_make_ipal_cframe(i_ptr, cf_ptr+f_num, address, size);
		f_num_inc(&f_num);
		cf_ptr = Case_frame_array+start;
	    }
	    
	    /* ���� */
	    if (b_ptr->voice == VOICE_UKEMI ||
		b_ptr->voice == VOICE_MORAU) {
		/* ľ�ܼ��ȣ� */
		if (*(i_ptr->DATA+i_ptr->tyoku_noudou1)) {
		    (cf_ptr+f_num)->voice = FRAME_PASSIVE_1;
		    _make_ipal_cframe(i_ptr, cf_ptr+f_num, address, size);
		    f_num_inc(&f_num);
		    cf_ptr = Case_frame_array+start;
		}
		/* ľ�ܼ��ȣ� */
		if (*(i_ptr->DATA+i_ptr->tyoku_noudou2)) {
		    (cf_ptr+f_num)->voice = FRAME_PASSIVE_2;
		    _make_ipal_cframe(i_ptr, cf_ptr+f_num, address, size);
		    f_num_inc(&f_num);
		    cf_ptr = Case_frame_array+start;
		}
		/* ���ܼ��� */
		if (str_part_eq(i_ptr->DATA+i_ptr->rare, "�ּ�")) {
		    (cf_ptr+f_num)->voice = FRAME_PASSIVE_I;
		    _make_ipal_cframe(i_ptr, cf_ptr+f_num, address, size);
		    f_num_inc(&f_num);
		    cf_ptr = Case_frame_array+start;
		}
	    }
	    /* ��ǽ��º�ɡ���ȯ */
	    if (b_ptr->voice == VOICE_UKEMI) {
		if (str_part_eq(i_ptr->DATA+i_ptr->rare, "��ǽ")) {
		    (cf_ptr+f_num)->voice = FRAME_POSSIBLE;
		    _make_ipal_cframe(i_ptr, cf_ptr+f_num, address, size);
		    f_num_inc(&f_num);
		    cf_ptr = Case_frame_array+start;
		}
		if (str_part_eq(i_ptr->DATA+i_ptr->rare, "º��")) {
		    (cf_ptr+f_num)->voice = FRAME_POLITE;
		    _make_ipal_cframe(i_ptr, cf_ptr+f_num, address, size);
		    f_num_inc(&f_num);
		    cf_ptr = Case_frame_array+start;
		}
		if (str_part_eq(i_ptr->DATA+i_ptr->rare, "��ȯ")) {
		    (cf_ptr+f_num)->voice = FRAME_SPONTANE;
		    _make_ipal_cframe(i_ptr, cf_ptr+f_num, address, size);
		    f_num_inc(&f_num);
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
 int make_ipal_cframe(SENTENCE_DATA *sp, BNST_DATA *b_ptr, int start)
/*==================================================================*/
{
    int f_num = 0, plus_num, i, j;
    char *verb, buffer[3][WORD_LEN_MAX];
    BNST_DATA *pre_ptr;

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
    /* �� ���Τ��� �롼�벽���� */
    else if (b_ptr->jiritu_num > 1 && L_Jiritu_M(b_ptr)->Hinshi == 14 && 
	     !check_feature(b_ptr->f, "���Ѹ�")) {
	assign_cfeature(&(b_ptr->f), "̾��+������");
	sprintf(buffer[0], "%s%s", (b_ptr->jiritu_ptr+b_ptr->jiritu_num-2)->Goi, L_Jiritu_M(b_ptr)->Goi);
	verb = buffer[0];
	f_num = make_ipal_cframe_subcontract(b_ptr, start, verb);
	if (f_num != 0) {
	    Case_frame_num += f_num;
	    for (i = 0; i < f_num; i++) {
		(Case_frame_array+start+i)->concatenated_flag = 0;
	    }
	    return f_num;
	}
	verb = L_Jiritu_M(b_ptr)->Goi;
    }
    else {
	verb = L_Jiritu_M(b_ptr)->Goi;
    }

    f_num = make_ipal_cframe_subcontract(b_ptr, start, verb);
    Case_frame_num += f_num;
    for (i = 0; i < f_num; i++) {
	(Case_frame_array+start+i)->concatenated_flag = 0;
    }

    if (b_ptr->num < 1) {
	return f_num;
    }

    buffer[0][0] = '\0';
    buffer[1][0] = '\0';
    buffer[2][0] = '\0';
    buffer[0][WORD_LEN_MAX-1] = '\n';
    buffer[1][WORD_LEN_MAX-1] = '\n';

    pre_ptr = sp->bnst_data+b_ptr->num-1;

    /* ���٤�ʸ���ñ��򤹤٤Ʒ��
       ��¤����塢��������ط��ˤ��뤫�ɤ��������å� */

    /* ���٤�ʸ�᤬̤�ʤǤ����� */
    if (check_feature(pre_ptr->f, "��:̤��")) {
	for (i = 0; i < pre_ptr->settou_num; i++) {
	    strcat(buffer[0], (pre_ptr->settou_ptr+i)->Goi);
	}
	for (i = 0; i < pre_ptr->jiritu_num; i++) {
	    strcat(buffer[0], (pre_ptr->jiritu_ptr+i)->Goi);
	}
	strcpy(buffer[1], buffer[0]);
	/* ����, ��ʤǥ����å������� */
	strcat(buffer[0], "��");
	strcat(buffer[1], "��");
    }
    else {
	for (i = 0; i < pre_ptr->mrph_num; i++) {
	    strcat(buffer[0], (pre_ptr->mrph_ptr+i)->Goi);
	}
    }

    for (i = 0; buffer[i][0]; i++) {
	/* ��ʬ��ʸ�����Ƭ���ȼ�Ω����� */
	for (j = 0; j < b_ptr->settou_num; j++) {
	    strcat(buffer[i], (b_ptr->settou_ptr+j)->Goi);
	}
	for (j = 0; j < b_ptr->jiritu_num; j++) {
	    strcat(buffer[i], (b_ptr->jiritu_ptr+j)->Goi);
	}

	if (buffer[i][WORD_LEN_MAX-1] != '\n') {
	    fprintf(stderr, "buffer overflowed.\n");
	    return f_num;
	}

	verb = buffer[i];
	plus_num = make_ipal_cframe_subcontract(b_ptr, start+f_num, verb);
	Case_frame_num += plus_num;
	for (j = f_num; j < f_num+plus_num; j++) {
	    (Case_frame_array+start+j)->concatenated_flag = 1;
	}
	f_num += plus_num;
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

    if (MAX_ipal_frame_length == 0) {
	ipal_str_buf = 
	    (unsigned char *)realloc_data(ipal_str_buf, 
					  sizeof(unsigned char)*ALLOCATION_STEP, 
					  "make_default_cframe");
    }

    _make_ipal_cframe_pp(cf_ptr, "����", num++);
    _make_ipal_cframe_pp(cf_ptr, "���", num++);
    _make_ipal_cframe_pp(cf_ptr, "�ˡ�", num++);
    _make_ipal_cframe_pp(cf_ptr, "�ء�", num++);
    _make_ipal_cframe_pp(cf_ptr, "����", num++);

    cf_ptr->ipal_address = -1;
    cf_ptr->element_num = num;
    cf_ptr->ipal_id[0] = '\0';
    cf_ptr->flag = CF_NORMAL;

    for (i = 0; i < num; i++) {
	cf_ptr->pp[i][1] = END_M;
    }

    f_num_inc(&f_num);
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
    int i, start[BNST_MAX];
    BNST_DATA  *b_ptr;

    Case_frame_num = 0;

    for (i = 0, b_ptr = sp->bnst_data; i < sp->Bnst_num; i++, b_ptr++) {
	/* ���򥳡��ѥ������Ϥ����Ȥ��˼�Ω�줬�ʤ���礬���� */
	if (b_ptr->jiritu_ptr != NULL && 
	    (check_feature(b_ptr->f, "�Ѹ�") ||
	     check_feature(b_ptr->f, "���Ѹ�") || 
	     check_feature(b_ptr->f, "����̾��ʲ���"))) {

	    /* �ʲ���2�Ĥν�����feature��٥�ǵ�ư���Ƥ��� */
	    /* set_pred_voice(b_ptr); �������� */
	    /* get_scase_code(b_ptr); ɽ�س� */

	    start[i] = Case_frame_num;
	    make_case_frames(sp, b_ptr);
	}
	else {
	    start[i] = -1;
	}
    }

    /* �ʥե졼���ʸ��إ�󥯤��� */
    for (i = 0; i < sp->Bnst_num; i++) {
	if (start[i] >= 0) {
	    (sp->bnst_data+i)->cf_ptr = Case_frame_array+start[i];
	}
	else {
	    b_ptr->cf_ptr = NULL;
	    b_ptr->cf_num = 0;
	}
    }
}

/*==================================================================*/
			   void clear_cf()
/*==================================================================*/
{
    int i, j;

    for (i = 0; i < Case_frame_num; i++) {
	for (j = 0; j < CF_ELEMENT_MAX; j++) {
	    if (Thesaurus == USE_BGH) {
		if ((Case_frame_array+i)->ex[j]) {
		    free((Case_frame_array+i)->ex[j]);
		    (Case_frame_array+i)->ex[j] = NULL;
		}
	    }
	    else if (Thesaurus == USE_NTT) {
		if ((Case_frame_array+i)->ex2[j]) {
		    free((Case_frame_array+i)->ex2[j]);
		    (Case_frame_array+i)->ex2[j] = NULL;
		}
	    }
	    if ((Case_frame_array+i)->sm[j]) {
		free((Case_frame_array+i)->sm[j]);
		(Case_frame_array+i)->sm[j] = NULL;
	    }
	    if ((Case_frame_array+i)->sm_false[j]) {
		free((Case_frame_array+i)->sm_false[j]);
		(Case_frame_array+i)->sm_false[j] = NULL;
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
    }
}

/*==================================================================*/
	       void MakeInternalBnst(SENTENCE_DATA *sp)
/*==================================================================*/
{
    int i, j, suffix_num;
    BNST_DATA *bp;

    /* �Ǹ�μ�Ω�줬����̾��ΤȤ��ˡ�����������̾���
       ����ʸ��Ȥ��ƹͤ��� */

    for (i = 0; i < sp->Bnst_num; i++) {
	bp = sp->bnst_data+i;
	bp->internal_num = 0;

	/* ��Ω������������̾��Ǥʤ���� */
	if (bp->jiritu_ptr == NULL || !check_feature(L_Jiritu_M(bp)->f, "����") || 
	    check_feature(bp->f, "�Ѹ�:Ƚ")) {
	    continue;
	}

	/* ʣ��̾��Ǥ��ꡢHead ��ľ��������Ǥʤ�
	   (�֣���Ϣ���פʤɤ����) */
	if (bp->jiritu_num > 1 && 
	    !((bp->jiritu_ptr+bp->jiritu_num-2)->Bunrui == 7 && 
	      (bp->jiritu_ptr+bp->jiritu_num-2)->Hinshi == 6)) {
	    suffix_num = 0;
	    for (j = 2; j <= bp->jiritu_num; j++) {
		/* ̾�� or ����Ǥ���� */
		if ((bp->jiritu_ptr+bp->jiritu_num-j)->Hinshi == 6 || 
		    (bp->jiritu_ptr+bp->jiritu_num-j)->Hinshi == 8) {
		    if (bp->internal_num == 0) {
			bp->internal = (BNST_DATA *)malloc_data(sizeof(BNST_DATA), "MakeInternalBnst");
			bp->internal_max = 1;
		    }
		    else if (bp->internal_num >= bp->internal_max) {
			bp->internal = (BNST_DATA *)realloc_data(bp->internal, 
								 sizeof(BNST_DATA)*(bp->internal_max <<= 1), 
								 "MakeInternalBnst");
		    }
		    assign_cfeature(&(bp->f), "����ʸ��");
		    memset(bp->internal+bp->internal_num, 0, sizeof(BNST_DATA));
		    (bp->internal+bp->internal_num)->num = -1;
		    (bp->internal+bp->internal_num)->mrph_num = suffix_num+1;
		    (bp->internal+bp->internal_num)->mrph_ptr = bp->jiritu_ptr+bp->jiritu_num-j;
		    (bp->internal+bp->internal_num)->jiritu_num = suffix_num+1;
		    (bp->internal+bp->internal_num)->jiritu_ptr = bp->jiritu_ptr+bp->jiritu_num-j;
		    strcpy((bp->internal+bp->internal_num)->Jiritu_Go, 
			   (bp->internal+bp->internal_num)->jiritu_ptr->Goi);

		    /* ʸ��롼���Ŭ�Ѥ��� */
		    _assign_general_feature(bp->internal+bp->internal_num, 1, BnstRuleType);

		    get_bgh_code(bp->internal+bp->internal_num);
		    get_sm_code(bp->internal+bp->internal_num);
		    assign_sm_aux_feature(bp->internal+bp->internal_num);
		    assign_cfeature(&((bp->internal+bp->internal_num)->f), "��:ʸ����");
		    (bp->internal+bp->internal_num)->parent = bp;
		    bp->internal_num++;
		    break;	/* �Ȥꤢ�������ҤȤĤǤ���н���� */
		}
		/* ����������¸���Ƥ����ơ�̾�줬�褿�Ȥ��ˤ��äĤ��� */
		else if ((bp->jiritu_ptr+bp->jiritu_num-j)->Hinshi == 14) {
		    suffix_num++;
		}
	    }
	}
    }
}

/*==================================================================*/
	     int check_cf_case(CASE_FRAME *cfp, char *pp)
/*==================================================================*/
{
    int i;
    for (i = 0; i < cfp->element_num; i++) {
	if (MatchPP(cfp->pp[i][0], pp)) {
	    return 1;
	}
    }
    return 0;
}

/*====================================================================
                               END
====================================================================*/
