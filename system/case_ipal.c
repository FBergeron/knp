/*====================================================================

		       �ʹ�¤����: �ʥե졼��¦

                                               S.Kurohashi 93. 5.31

    $Id$
====================================================================*/
#include "knp.h"

FILE *ipal_fp;
DBM_FILE ipal_db;

CASE_FRAME 	Case_frame_array[ALL_CASE_FRAME_MAX]; 	/* �ʥե졼�� */
int 	   	Case_frame_num;				/* �ʥե졼��� */

IPAL_FRAME Ipal_frame;

int	IPALExist;

/*
 *	IPAL �� �ɤ߽Ф�
 */

/*==================================================================*/
			  void init_ipal()
/*==================================================================*/
{
#ifdef _WIN32
    if ((ipal_fp = fopen(IPAL_DAT_NAME, "rb")) == NULL) {
#else
    if ((ipal_fp = fopen(IPAL_DAT_NAME, "r")) == NULL) {
#endif

	if (OptAnalysis == OPT_CASE ||
	    OptAnalysis == OPT_CASE2) {
	    fprintf(stderr, "Cannot open IPAL data <%s>.\n", IPAL_DAT_NAME);
	}
	IPALExist = FALSE;
    }
    else if ((ipal_db = DBM_open(IPAL_DB_NAME, O_RDONLY, 0)) == NULL) {
	fprintf(stderr, "Cannot open Database <%s>.\n", IPAL_DB_NAME);
	exit(1);
    } 
    else {
	IPALExist = TRUE;
    }
}

/*==================================================================*/
			  void close_ipal()
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
	 void get_ipal_frame(IPAL_FRAME *i_ptr, int address)
/*==================================================================*/
{
    fseek(ipal_fp, (long)address, 0);
    if (fread(i_ptr, sizeof(IPAL_FRAME), 1, ipal_fp) < 1) {
	fprintf(stderr, "Error in fread.\n");
	exit(1);
    }
}

/*
 *	IPAL -> �ʥե졼��
 */

unsigned char ipal_str_buf[IPAL_DATA_SIZE];

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

    if (!strcmp(cp+strlen(cp)-2, "��"))
      c_ptr->oblig[num] = FALSE;
    else
      c_ptr->oblig[num] = TRUE;

    point = cp; 
    while (point = extract_ipal_str(point, ipal_str_buf))
      c_ptr->pp[num][pp_num++] = pp_kstr_to_code(ipal_str_buf);
    c_ptr->pp[num][pp_num] = END_M;
}
    
/*==================================================================*/
void _make_ipal_cframe_sm(CASE_FRAME *c_ptr, unsigned char *cp, int num)
/*==================================================================*/
{
    /* ��̣�ޡ������ɤߤ��� */

    unsigned char *point;
    int sm_num = 0;

    point = cp;
    c_ptr->sm[num][0] = '\0';
    while (point = extract_ipal_str(point, ipal_str_buf)) {
        if (ipal_str_buf[0] == '-') {
	  c_ptr->sm_flag[num][sm_num] = FALSE;
	  strcpy(ipal_str_buf, &ipal_str_buf[1]);
	}
	else
	  c_ptr->sm_flag[num][sm_num] = TRUE;

	strcpy(c_ptr->sm[num]+SM_CODE_SIZE*sm_num, 
	       (char *)sm2code(ipal_str_buf));
	sm_num++;
	if (sm_num >= SM_ELEMENT_MAX){
	  fprintf(stderr, "Not enough sm_num !!\n");
	  exit(1);
	}

	/* c_ptr->sm[num][sm_num++] = sm_zstr_to_code(); */
    }
    /* ������ strcpy �� \0 ������ */
}

/*==================================================================*/
void _make_ipal_cframe_ex(CASE_FRAME *c_ptr, unsigned char *cp, int num, int flag)
/*==================================================================*/
{
    /* ����ɤߤ��� */

    unsigned char *point, *point2;
    int i, max;
    char *code, *destination;
    extern char *get_bgh();
    extern char *get_sm();
    char *(*get_code)();

    /* �����꥽�����ˤ�äƴؿ��ʤɤ򥻥å� */
    if (flag == USE_BGH) {
	get_code = get_bgh;
	destination = c_ptr->ex[num];
	max = EX_ELEMENT_MAX*BGH_CODE_SIZE;
    }
    else if (flag == USE_NTT) {
	get_code = get_sm;
	destination = c_ptr->ex2[num];
	max = SM_ELEMENT_MAX*SM_CODE_SIZE;
    }

    point = cp;
    c_ptr->ex[num][0] = '\0';
    while (point = extract_ipal_str(point, ipal_str_buf)) {
	
	/* �֣��Σ¡פΡ֣¡פ�������� */
	point2 = ipal_str_buf;
	for (i = strlen(point2) - 2; i > 2; i -= 2) {
	    if (!strncmp(point2+i-2, "��", 2)) {
		point2 += i;
		break;
	    }
	}
	code = (char *)get_code(point2);
	if (code) {
	    strcat(destination, code);
	    free(code);
	}
	if (strlen(destination) >= max) {
	    fprintf(stderr, "Too many EX <%s>.\n", ipal_str_buf);
	    break;
	}
    }
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
     void _make_ipal_cframe(CASE_FRAME *cf_ptr, int address)
/*==================================================================*/
{
    int i, j = 0, ga_p = NULL;
    IPAL_FRAME *i_ptr = &Ipal_frame;
    char ast_cap[32];

    cf_ptr->ipal_address = address;
    strcpy(cf_ptr->ipal_id, i_ptr->DATA+i_ptr->id); 
    strcpy(cf_ptr->imi, i_ptr->DATA+i_ptr->imi);

    /* �����Ǥ��ɲ� */

    if (cf_ptr->voice == FRAME_PASSIVE_I ||
	cf_ptr->voice == FRAME_CAUSATIVE_WO_NI ||
	cf_ptr->voice == FRAME_CAUSATIVE_WO ||
	cf_ptr->voice == FRAME_CAUSATIVE_NI) {
	_make_ipal_cframe_pp(cf_ptr, "��", j);
	_make_ipal_cframe_sm(cf_ptr, "�ģɣ֡��ȣգ�", j);
	_make_ipal_cframe_ex(cf_ptr, "��", j, USE_BGH);
	_make_ipal_cframe_ex(cf_ptr, "��", j, USE_NTT);
	j++;
    }

    /* �Ƴ����Ǥν��� */

    for (i = 0; i < 5 && *(i_ptr->DATA+i_ptr->kaku_keishiki[i]); i++, j++) { 
	_make_ipal_cframe_pp(cf_ptr, i_ptr->DATA+i_ptr->kaku_keishiki[i], j);
	_make_ipal_cframe_sm(cf_ptr, i_ptr->DATA+i_ptr->imisosei[i], j);
	_make_ipal_cframe_ex(cf_ptr, i_ptr->DATA+i_ptr->meishiku[i], j, USE_BGH);
	_make_ipal_cframe_ex(cf_ptr, i_ptr->DATA+i_ptr->meishiku[i], j, USE_NTT);

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
    if ((Case_frame_num + *f_num_p) > ALL_CASE_FRAME_MAX) {
	fprintf(stderr, "Not enough Case_frame_array !!\n");
	exit(1);
    }
}

/*==================================================================*/
      int make_ipal_cframe(BNST_DATA *b_ptr, CASE_FRAME *cf_ptr)
/*==================================================================*/
{
    IPAL_FRAME *i_ptr = &Ipal_frame;
    int f_num = 0, address, break_flag = 0;
    char *pre_pos, *cp, *address_str, *vtype = NULL;
    
    address_str = get_ipal_address(L_Jiritu_M(b_ptr)->Goi);

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
	    address = atoi(pre_pos);
	    get_ipal_frame(i_ptr, address);
	    pre_pos = cp + 1;

	    /* �Ѹ��Υ����פ��ޥå����ʤ���� */
	    if (vtype && strncmp(i_ptr->DATA+i_ptr->id, vtype, 2)) {
		if (break_flag)
		    break;
		else
		    continue;
	    }

	    /* ǽư�� */
	    if (b_ptr->voice == NULL) {
		(cf_ptr+f_num)->voice = FRAME_ACTIVE;
		_make_ipal_cframe(cf_ptr+f_num, address);
		f_num_inc(&f_num);
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
		
		_make_ipal_cframe(cf_ptr+f_num, address);
		f_num_inc(&f_num);
	    }
	    
	    /* ���� */
	    if (b_ptr->voice == VOICE_UKEMI ||
		b_ptr->voice == VOICE_MORAU) {
		/* ľ�ܼ��ȣ� */
		if (*(i_ptr->DATA+i_ptr->tyoku_noudou1)) {
		    (cf_ptr+f_num)->voice = FRAME_PASSIVE_1;
		    _make_ipal_cframe(cf_ptr+f_num, address);
		    f_num_inc(&f_num);
		}
		/* ľ�ܼ��ȣ� */
		if (*(i_ptr->DATA+i_ptr->tyoku_noudou2)) {
		    (cf_ptr+f_num)->voice = FRAME_PASSIVE_2;
		    _make_ipal_cframe(cf_ptr+f_num, address);
		    f_num_inc(&f_num);
		}
		/* ���ܼ��� */
		if (str_part_eq(i_ptr->DATA+i_ptr->rare, "�ּ�")) {
		    (cf_ptr+f_num)->voice = FRAME_PASSIVE_I;
		    _make_ipal_cframe(cf_ptr+f_num, address);
		    f_num_inc(&f_num);
		}
	    }
	    /* ��ǽ��º�ɡ���ȯ */
	    if (b_ptr->voice == VOICE_UKEMI) {
		if (str_part_eq(i_ptr->DATA+i_ptr->rare, "��ǽ")) {
		    (cf_ptr+f_num)->voice = FRAME_POSSIBLE;
		    _make_ipal_cframe(cf_ptr+f_num, address);
		    f_num_inc(&f_num);
		}
		if (str_part_eq(i_ptr->DATA+i_ptr->rare, "º��")) {
		    (cf_ptr+f_num)->voice = FRAME_POLITE;
		    _make_ipal_cframe(cf_ptr+f_num, address);
		    f_num_inc(&f_num);
		}
		if (str_part_eq(i_ptr->DATA+i_ptr->rare, "��ȯ")) {
		    (cf_ptr+f_num)->voice = FRAME_SPONTANE;
		    _make_ipal_cframe(cf_ptr+f_num, address);
		    f_num_inc(&f_num);
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
   int make_default_cframe(BNST_DATA *b_ptr, CASE_FRAME *cf_ptr)
/*==================================================================*/
{
    int i, num = 0;

    _make_ipal_cframe_pp(cf_ptr, "����", num++);
    _make_ipal_cframe_pp(cf_ptr, "���", num++);
    _make_ipal_cframe_pp(cf_ptr, "�ˡ�", num++);
    _make_ipal_cframe_pp(cf_ptr, "�ء�", num++);
    _make_ipal_cframe_pp(cf_ptr, "����", num++);

    cf_ptr->ipal_address = -1;
    cf_ptr->element_num = num;

    for (i = 0; i < num; i++) {
	cf_ptr->pp[i][1] = END_M;
	cf_ptr->sm[i][0] = '\0';
	cf_ptr->ex[i][0] = '\0';
    }

    return 1;
}

/*==================================================================*/
	       void make_case_frames(BNST_DATA *b_ptr)
/*==================================================================*/
{
    int f_num;
	
    if ((f_num = make_ipal_cframe(b_ptr, Case_frame_array + Case_frame_num)) 
	!= 0) {
	b_ptr->cf_ptr = Case_frame_array + Case_frame_num;
	b_ptr->cf_num = f_num;
	Case_frame_num += f_num;
    } else {
	make_default_cframe(b_ptr, Case_frame_array + Case_frame_num);
	b_ptr->cf_ptr = Case_frame_array + Case_frame_num;
	b_ptr->cf_num = 1;
	/* fprintf(stderr, "No entry for <%s> in IPAL.\n",b_ptr->Jiritu_Go); */

	if ((Case_frame_num += 1) > ALL_CASE_FRAME_MAX) {
	    fprintf(stderr, "Not enough Case_frame_array !!\n");
	    exit(1);
	}
    }
}

/*====================================================================
                               END
====================================================================*/
