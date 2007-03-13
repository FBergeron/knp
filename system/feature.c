/*====================================================================

			     FEATURE����

                                               S.Kurohashi 96. 7. 4

    $Id$
====================================================================*/
#include "knp.h"

/*
  FEATURE�ν����ˤϼ��Σ����ब����

  	(1) �ե�����(S���ޤ���ʸ����) ==���ԡ�==> �롼�빽¤��

	(2) �롼�빽¤�� ==��Ϳ==> �����Ǥޤ���ʸ�ṽ¤��
        	<��:��>��<��:��>�Ȥ���FEATURE�ؤξ�� (�ʤ���п���)
                <^��>��<��:��>�κ�� (�ʤ����̵��)
		<&��>�ϴؿ��ƽ�
			&ɽ��:��Ϳ -- ��������ˤ��ɽ�س���Ϳ
			&ɽ��:��� -- ���٤Ƥ�ɽ�سʺ��
			&ɽ��:���� -- ������Ϳ
			&ɽ��:^���� -- ���ʺ��
			&MEMO:�� -- MEMO�ؤν񤭹���

	(3) �롼�빽¤�� <==�ȹ�==> �����Ǥޤ���ʸ�ṽ¤��
	       	<��>��<��:��>�Ȥ���FEATURE�������OK
	    	<^��>��<��:��>�Ȥ���FEATURE���ʤ����OK
	    	<&��>�ϴؿ��ƽ�
			&���ѿ��� -- ɽ��������,��ʸ��,����,�������� (������)
			&���� -- ɽ�������� (������)
	    		&ɽ��:���� -- ���ʤ����� (ʸ��)
	    		&ɽ��:�ȹ� -- ����ɽ�سʤ����ˤ��� (����)
			&D:n -- ��¤�δ֤���Υn���� (����)
			&��٥�:�� -- �������ʾ� (����)
			&��٥�:l -- ���Ȥ�l�ʾ� (����)
			&��¦:�� -- ���ˡ� (����)

	�� �ץ������Ƿ����Ǥޤ���ʸ�ṽ¤�Τ�FEATURE��Ϳ����
	����(2)�Τʤ��� assign_cfeature ���Ѥ��롥

	�� �ץ������Ƿ����Ǥޤ���ʸ�ṽ¤�Τ�����FEATURE�����
	���ɤ�����Ĵ�٤����(3)�Τʤ��� check_feature ���Ѥ��롥
*/

/*==================================================================*/
	    void print_one_feature(char *cp, FILE *filep)
/*==================================================================*/
{
    if (!strncmp(cp, "����Ϳ:", strlen("����Ϳ:"))) { /* ����Ϳ������Τ�ɽ������Ȥ���(-nbest) */
	if (OptExpress == OPT_TABLE)
	    fprintf(filep, "��%s��", cp + strlen("����Ϳ:")); 
	else
	    fprintf(filep, "<%s>", cp + strlen("����Ϳ:")); 
    }
    else {
	if (OptExpress == OPT_TABLE)
	    fprintf(filep, "��%s��", cp);
	else
	    fprintf(filep, "<%s>", cp);
    }
}

/*==================================================================*/
	      void print_feature(FEATURE *fp, FILE *filep)
/*==================================================================*/
{
    /* <f1><f2> ... <f3> �Ȥ��������ν��� 
       (�������ԤǤϤ��ޤ�feature��ɽ�����ʤ�) */

    while (fp) {
	if (fp->cp && 
	    (strncmp(fp->cp, "��", strlen("��")) ||
	     OptDisplay == OPT_DEBUG))
	    print_one_feature(fp->cp, filep);
	fp = fp->next;
    }
}

/*==================================================================*/
	  void print_some_feature(FEATURE *fp, FILE *filep)
/*==================================================================*/
{
    /* <f1><f2> ... <f3> �Ȥ��������ν��� 
       ���ꤷ����Τ�����ɽ�� */

    while (fp) {
	if (fp->cp && strncmp(fp->cp, "��", strlen("��")) && !strncmp(fp->cp, "C", 1))
	    print_one_feature(fp->cp, filep);
	fp = fp->next;
    }
}
/*==================================================================*/
	      void print_feature2(FEATURE *fp, FILE *filep)
/*==================================================================*/
{
    /* (f1 f2 ... f3) �Ȥ��������ν���
       (�������ԤǤϤ��ޤ�feature��ɽ�����ʤ�) */
    if (fp) {
	fprintf(filep, "("); 
	while (fp) {
	    if (fp->cp && strncmp(fp->cp, "��", strlen("��"))) {
		fprintf(filep, "%s", fp->cp); 
		if (fp->next) fprintf(filep, " "); 		
	    }
	    fp = fp->next;
	}
	fprintf(filep, ")"); 
    } else {
	fprintf(filep, "NIL"); 
    }
}

/*==================================================================*/
		   void clear_feature(FEATURE **fpp)
/*==================================================================*/
{
    FEATURE *fp, *next;

    fp = *fpp;
    *fpp = NULL;

    while (fp) {
	next = fp->next;
	free(fp->cp);
	free(fp);
	fp = next;
    }
}

/*==================================================================*/
	   void delete_cfeature(FEATURE **fpp, char *type)
/*==================================================================*/
{
    FEATURE *prep = NULL;

    while (*fpp) {
	if (comp_feature((*fpp)->cp, type) == TRUE) {
	    FEATURE *next;
	    free((*fpp)->cp);
	    if (prep == NULL) {
		next = (*fpp)->next;
		free(*fpp);
		*fpp = next;
	    }
	    else {
		next = (*fpp)->next;
		free(*fpp);
		prep->next = next;
	    }
	    return;
	}
	prep = *fpp;
	fpp = &(prep->next);
    }
}

/*==================================================================*/
	       void delete_temp_feature(FEATURE **fpp)
/*==================================================================*/
{
    /* ����Ϳ����feature���� */

    FEATURE *prep = NULL;

    while (*fpp) {
	if (comp_feature((*fpp)->cp, "����Ϳ") == TRUE) {
	    FEATURE *next;
	    free((*fpp)->cp);
	    if (prep == NULL) {
		next = (*fpp)->next;
		free(*fpp);
		*fpp = next;
	    }
	    else {
		next = (*fpp)->next;
		free(*fpp);
		prep->next = next;
	    }
	    fpp = &(prep->next);
	    continue;
	}
	prep = *fpp;
	fpp = &(prep->next);
    }
}    

/*
 *
 *  �ե�����(S���ޤ���ʸ����) ==���ԡ�==> �롼�빽¤��
 *
 */

/*==================================================================*/
	   void copy_cfeature(FEATURE **fpp, char *fname)
/*==================================================================*/
{
    while (*fpp) fpp = &((*fpp)->next);

    if (!((*fpp) = (FEATURE *)(malloc(sizeof(FEATURE)))) ||
	!((*fpp)->cp = (char *)(malloc(strlen(fname) + 1)))) {
	fprintf(stderr, "Can't allocate memory for FEATURE\n");
	exit(-1);
    }
    strcpy((*fpp)->cp, fname);
    (*fpp)->next = NULL;
}

/*==================================================================*/
	      void list2feature(CELL *cp, FEATURE **fpp)
/*==================================================================*/
{
    while (!Null(car(cp))) {
	copy_cfeature(fpp, _Atom(car(cp)));
	fpp = &((*fpp)->next);
	cp = cdr(cp);	    
    }
}

/*==================================================================*/
      void list2feature_pattern(FEATURE_PATTERN *f, CELL *cell)
/*==================================================================*/
{
    /* �ꥹ�� ((ʸƬ)(�θ�)(����)) �ʤɤ�FEATURE_PATTERN���Ѵ� */

    int nth = 0;

    while (!Null(car(cell))) {
	clear_feature(f->fp+nth);		/* ?? &(f->fp[nth]) */ 
	list2feature(car(cell), f->fp+nth);	/* ?? &(f->fp[nth]) */ 
	cell = cdr(cell);
	nth++;
    }
    f->fp[nth] = NULL;
}

/*==================================================================*/
      void string2feature_pattern_OLD(FEATURE_PATTERN *f, char *cp)
/*==================================================================*/
{
    /* ʸ���� "ʸƬ|�θ�|����" �ʤɤ�FEATURE_PATTERN���Ѵ�
       ����list2feature_pattern���б������Τ���,
       OR������AND�ϥ��ݡ��Ȥ��Ƥ��ʤ� */

    int nth = 0;
    char buffer[256], *scp, *ecp;

    if (cp == NULL || cp[0] == '\0') {
	f->fp[nth] = NULL;
	return;
    }

    strcpy(buffer, cp);
    scp = ecp = buffer;
    while (*ecp) {
	if (*ecp == '|') {
	    *ecp = '\0';
	    clear_feature(f->fp+nth);		/* ?? &(f->fp[nth]) */
	    copy_cfeature(f->fp+nth, scp);	/* ?? &(f->fp[nth]) */
	    nth++;
	    scp = ecp + 1; 
	}
	ecp ++;
    }
    
    clear_feature(f->fp+nth);			/* ?? &(f->fp[nth]) */ 
    copy_cfeature(&(f->fp[nth]), scp);
    nth++;

    f->fp[nth] = NULL;
}

/*==================================================================*/
      void string2feature_pattern(FEATURE_PATTERN *f, char *cp)
/*==================================================================*/
{
    /* ʸ���� "ʸƬ|�θ�|����" �ʤɤ�FEATURE_PATTERN���Ѵ�
       ����list2feature_pattern���б������Τ���,
       OR������AND�ϥ��ݡ��Ȥ��Ƥ��ʤ� */

    int nth;
    char buffer[256], *start_cp, *loop_cp;
    FEATURE **fpp;
    
    if (!*cp) {
	f->fp[0] = NULL;
	return;
    }

    strcpy(buffer, cp);
    nth = 0;
    clear_feature(f->fp+nth);
    fpp = f->fp+nth;
    loop_cp = buffer;
    start_cp = loop_cp;
    while (*loop_cp) {
	if (*loop_cp == '&' && *(loop_cp+1) == '&') {
	    *loop_cp = '\0';
	    copy_cfeature(fpp, start_cp);
	    fpp = &((*fpp)->next);
	    loop_cp += 2;
	    start_cp = loop_cp;
	}
	else if (*loop_cp == '|' && *(loop_cp+1) == '|') {
	    *loop_cp = '\0';
	    copy_cfeature(fpp, start_cp);
	    nth++;
	    clear_feature(f->fp+nth);
	    fpp = f->fp+nth;
	    loop_cp += 2;
	    start_cp = loop_cp;
	}
	else {
	    loop_cp ++;
	}
    }
    copy_cfeature(fpp, start_cp);

    nth++;
    f->fp[nth] = NULL;
}

/*
 *
 * �롼�빽¤�� ==��Ϳ==> �����Ǥޤ���ʸ�ṽ¤��
 *
 */

/*==================================================================*/
	   void append_feature(FEATURE **fpp, FEATURE *afp)
/*==================================================================*/
{
    while (*fpp) {
	fpp = &((*fpp)->next);
    }
    *fpp = afp;
}    

/*==================================================================*/
void assign_cfeature(FEATURE **fpp, char *fname, int temp_assign_flag)
/*==================================================================*/
{
    /* temp_assign_flag: TRUE�ΤȤ��ֲ���Ϳ�פ�Ƭ�ˤĤ��� */

    char type[256];

    /* ��񤭤β�ǽ��������å� */

    sscanf(fname, "%[^:]", type);	/* �� fname��":"���ʤ�����
					   type��fname���Τˤʤ� */

    /* quote('"')���":"���ڤäƤ���С���Ȥ��᤹ */
    if (strcmp(type, fname)) {
	int i, count = 0;

	for (i = 0; i < strlen(type); i++) {
	    if (type[i] == '"') {
		count++;
	    }
	}
	if (count % 2 == 1) { /* '"'����� */
	    strcpy(type, fname);
	}
    }

    while (*fpp) {
	if (comp_feature((*fpp)->cp, type) == TRUE) {
	    free((*fpp)->cp);
	    if (!((*fpp)->cp = (char *)(malloc(strlen(fname) + 1)))) {
		fprintf(stderr, "Can't allocate memory for FEATURE\n");
		exit(-1);
	    }
	    strcpy((*fpp)->cp, fname);
	    return;	/* ��񤭤ǽ�λ */
	}
	fpp = &((*fpp)->next);
    }

    /* ��񤭤Ǥ��ʤ�����������ɲ� */

    if (!((*fpp) = (FEATURE *)(malloc(sizeof(FEATURE)))) ||
	!((*fpp)->cp = (char *)(malloc(strlen(fname) + 8)))) {
	fprintf(stderr, "Can't allocate memory for FEATURE\n");
	exit(-1);
    }
    if (temp_assign_flag) {
	strcpy((*fpp)->cp, "����Ϳ:");
	strcat((*fpp)->cp, fname);
    }
    else {
	strcpy((*fpp)->cp, fname);
    }
    (*fpp)->next = NULL;
}    

/*==================================================================*/
void assign_feature(FEATURE **fpp1, FEATURE **fpp2, void *ptr, int offset, int length, int temp_assign_flag)
/*==================================================================*/
{
    /*
     *  �롼���Ŭ�Ѥη�̡��롼�뤫�鹽¤�Τ�FEATURE����Ϳ����
     *  ��¤�μ��Ȥ��Ф���������ǽ�Ȥ��Ƥ���
     */

    int i;
    char *cp, *pat;
    char buffer[256];
    FEATURE **fpp, *next;

    while (*fpp2) {

	if (*((*fpp2)->cp) == '^') {	/* ����ξ�� */
	    
	    fpp = fpp1;
	    
	    while (*fpp) {
		if (comp_feature((*fpp)->cp, &((*fpp2)->cp[1])) == TRUE) {
		    free((*fpp)->cp);
		    next = (*fpp)->next;
		    free(*fpp);
		    *fpp = next;
		} else {
		    fpp = &((*fpp)->next);
		}
	    }
	
	} else if (*((*fpp2)->cp) == '&') {	/* �ؿ��ξ�� */

	    if (!strcmp((*fpp2)->cp, "&ɽ��:��Ϳ")) {
		set_pred_voice((BNST_DATA *)ptr + offset);	/* �������� */
		get_scase_code((BNST_DATA *)ptr + offset);	/* ɽ�س� */
	    }
	    else if (!strcmp((*fpp2)->cp, "&ɽ��:���")) {
		for (i = 0, cp = ((BNST_DATA *)ptr + offset)->SCASE_code; 
		     i < SCASE_CODE_SIZE; i++, cp++) 
		    *cp = 0;		
	    }
	    else if (!strncmp((*fpp2)->cp, "&ɽ��:^", strlen("&ɽ��:^"))) {
		((BNST_DATA *)ptr + offset)->
		    SCASE_code[case2num((*fpp2)->cp + strlen("&ɽ��:^"))] = 0;
	    }
	    else if (!strncmp((*fpp2)->cp, "&ɽ��:", strlen("&ɽ��:"))) {
		((BNST_DATA *)ptr + offset)->
		    SCASE_code[case2num((*fpp2)->cp + strlen("&ɽ��:"))] = 1;
	    }
	    else if (!strncmp((*fpp2)->cp, "&MEMO:", strlen("&MEMO:"))) {
		strcat(PM_Memo, " ");
		strcat(PM_Memo, (*fpp2)->cp + strlen("&MEMO:"));
	    }
	    else if (!strncmp((*fpp2)->cp, "&�ʻ��ѹ�:", strlen("&�ʻ��ѹ�:"))) {
		change_mrph((MRPH_DATA *)ptr + offset, *fpp2);
	    }
	    else if (!strncmp((*fpp2)->cp, "&��̣����Ϳ:", strlen("&��̣����Ϳ:"))) {
		assign_sm((BNST_DATA *)ptr + offset, (*fpp2)->cp + strlen("&��̣����Ϳ:"));
	    }
	    else if (!strncmp((*fpp2)->cp, "&ʣ�缭�ʲ���", strlen("&ʣ�缭�ʲ���"))) {
		cp = make_fukugoji_string((TAG_DATA *)ptr + offset + 1);
		if (cp) {
		    assign_cfeature(&(((TAG_DATA *)ptr + offset)->f), cp, temp_assign_flag);
		}
	    }
	    else if (!strncmp((*fpp2)->cp, "&����������Ϳ:", strlen("&����������Ϳ:"))) {
		sprintf(buffer, "%s:%s", 
			(*fpp2)->cp + strlen("&����������Ϳ:"), 
			((MRPH_DATA *)matched_ptr)->Goi);
		assign_cfeature(&(((BNST_DATA *)ptr + offset)->f), buffer, temp_assign_flag);
	    }
	    /* &����:n:FEATURE : FEATURE������  */
	    else if (!strncmp((*fpp2)->cp, "&����:", strlen("&����:"))) {
		pat = (*fpp2)->cp + strlen("&����:");
		sscanf(pat, "%d", &i);
		pat = strchr(pat, ':');
		pat++;
		if ((cp = check_feature(((TAG_DATA *)ptr + offset)->f, pat))) {
		    assign_cfeature(&(((TAG_DATA *)ptr + offset + i)->f), cp, temp_assign_flag);
		}
		else { /* �ʤ��ʤ顢��Ȥ��餢���Τ��� */
		    delete_cfeature(&(((TAG_DATA *)ptr + offset + i)->f), pat);
		}
		if (((TAG_DATA *)ptr + offset)->bnum >= 0) { /* ʸ����ڤ�Ǥ⤢��Ȥ� */
		    if ((cp = check_feature((((TAG_DATA *)ptr + offset)->b_ptr)->f, pat))) {
			assign_cfeature(&((((TAG_DATA *)ptr + offset)->b_ptr + i)->f), cp, temp_assign_flag);
		    }
		    else {
			delete_cfeature(&((((TAG_DATA *)ptr + offset)->b_ptr + i)->f), pat);
		    }
		}
	    }
	    /* ��������°�� : °�����������򤹤٤�<��°>�ˤ��� */
	    else if (!strncmp((*fpp2)->cp, "&��������°��", strlen("&��������°��"))) {
		for (i = 0; i < ((TAG_DATA *)ptr + offset)->mrph_num; i++) {
		    delete_cfeature(&((((TAG_DATA *)ptr + offset)->mrph_ptr + i)->f), "��Ω");
		    delete_cfeature(&((((TAG_DATA *)ptr + offset)->mrph_ptr + i)->f), "��̣ͭ");
		    assign_cfeature(&((((TAG_DATA *)ptr + offset)->mrph_ptr + i)->f), "��°", temp_assign_flag);
		}
	    }
	    /* ��ư���� : ��ư�����������������å� (�ޥå���ʬ����) */
	    else if (!strncmp((*fpp2)->cp, "&��ư����:", strlen("&��ư����:"))) {
		if (offset == 0 && check_auto_dic((MRPH_DATA *)ptr, length, (*fpp2)->cp + strlen("&��ư����:"))) {
		    for (i = 0; i < length; i ++) {
			assign_cfeature(&(((MRPH_DATA *)ptr + i)->f), (*fpp2)->cp + strlen("&��ư����:"), temp_assign_flag);
		    }
		}
	    }
	} else {			/* �ɲäξ�� */
	    assign_cfeature(fpp1, (*fpp2)->cp, temp_assign_flag);	
	}

	fpp2 = &((*fpp2)->next);
    }
}

/*==================================================================*/
	void copy_feature(FEATURE **dst_fpp, FEATURE *src_fp)
/*==================================================================*/
{
    while (src_fp) {
	assign_cfeature(dst_fpp, src_fp->cp, FALSE);
	src_fp = src_fp->next;
    }
}

/*
 *
 * �롼�빽¤�� <==�ȹ�==> �����Ǥޤ���ʸ�ṽ¤��
 *
 */

/*==================================================================*/
	     int comp_feature(char *data, char *pattern)
/*==================================================================*/
{
    /* 
     *  �������� �ޤ��� ��ʬ����(pattern��û��,����ʸ����':')�ʤ�ޥå�
     */
    
    if (data && !strcmp(data, pattern)) {
	return TRUE;
    } else if (data && !strncmp(data, pattern, strlen(pattern)) &&
	       data[strlen(pattern)] == ':') {
	return TRUE;
    } else {
	return FALSE;
    }
}

/*==================================================================*/
	     int comp_feature_NE(char *data, char *pattern)
/*==================================================================*/
{
    char decision[9];

    decision[0] = '\0';
    sscanf(data, "%*[^:]:%*[^:]:%s", decision);

    if (decision[0] && !strcmp(decision, pattern))
	return TRUE;
    else
	return FALSE;
}

/*==================================================================*/
	    char *check_feature(FEATURE *fp, char *fname)
/*==================================================================*/
{
    while (fp) {
	if (comp_feature(fp->cp, fname) == TRUE) {
	    return fp->cp;
	}
	fp = fp->next;
    }
    return NULL;
}

/*==================================================================*/
	    char *check_feature_NE(FEATURE *fp, char *fname)
/*==================================================================*/
{
    while (fp) {
	if (comp_feature_NE(fp->cp, fname) == TRUE) {
	    return fp->cp;
	}
	fp = fp->next;
    }
    return NULL;
}

/*==================================================================*/
	int compare_threshold(int value, int threshold, char *eq)
/*==================================================================*/
{
    if (str_eq(eq, "lt")) {
	if (value < threshold)
	    return TRUE;
	else
	    return FALSE;
    }
    else if (str_eq(eq, "le")) {
	if (value <= threshold)
	    return TRUE;
	else
	    return FALSE;
    }
    else if (str_eq(eq, "gt")) {
	if (value > threshold)
	    return TRUE;
	else
	    return FALSE;
    }
    else if (str_eq(eq, "ge")) {
	if (value >= threshold)
	    return TRUE;
	else
	    return FALSE;
    }
    return FALSE;
}

/*==================================================================*/
      int check_Bunrui_others(MRPH_DATA *mp, int flag)
/*==================================================================*/
{
    if (mp->Bunrui != 3 && /* ��ͭ̾�� */
	mp->Bunrui != 4 && /* ��̾ */
	mp->Bunrui != 5 && /* ��̾ */
	mp->Bunrui != 6) /* �ȿ�̾ */
	return flag;

    if (check_feature(mp->f, "��ۣ-����¾"))
	return flag;

    return 1-flag;
}

/*==================================================================*/
      int check_Bunrui(MRPH_DATA *mp, char *class, int flag)
/*==================================================================*/
{
    char string[14];

    if (str_eq(Class[6][mp->Bunrui].id, class))
	return flag;

    sprintf(string, "��ۣ-%s", class);
    if (check_feature(mp->f, string))
	return flag;

    return 1-flag;
}

/*==================================================================*/
		    int check_char_type(int code)
/*==================================================================*/
{
    /* �������ʤ� "��" */
    if ((0xa5a0 < code && code < 0xa6a0) || code == 0xa1bc) {
	return TYPE_KATAKANA;
    }
    /* �Ҥ餬�� */
    else if (0xa4a0 < code && code < 0xa5a0) {
	return TYPE_HIRAGANA;
    }
    /* ���� */
    else if (0xb0a0 < code || code == 0xa1b9) {
	return TYPE_KANJI;
    }
    /* ������ "��", "��" */
    else if ((0xa3af < code && code < 0xa3ba) || code == 0xa1a5 || code == 0xa1a6) {
	return TYPE_SUUJI;
    }
    /* ����ե��٥å� */
    else if (0xa3c0 < code && code < 0xa3fb) {
	return TYPE_EIGO;
    }
    /* ���� */
    else {
	return TYPE_KIGOU;
    }
}

/*==================================================================*/
		int check_str_type(unsigned char *ucp)
/*==================================================================*/
{
    int code = 0, precode = 0;

    while (*ucp) {
	code = (*ucp << 8) + *(ucp + 1);
	code = check_char_type(code);
	if (precode && precode != code) {
	    return 0;
	}
	precode = code;
	ucp += 2;
    }

    return code;
}

/*==================================================================*/
 int check_function(char *rule, FEATURE *fd, void *ptr1, void *ptr2)
/*==================================================================*/
{
    /* rule : �롼��
       fd : �ǡ���¦��FEATURE
       p1 : ��������ξ�硤����¦�ι�¤��(MRPH_DATA,BNST_DATA�ʤ�)
       p2 : �ǡ����ι�¤��(MRPH_DATA,BNST_DATA�ʤ�)
    */

    int i, code, type, pretype, flag;
    char *cp;
    unsigned char *ucp; 

    /* &���ѿ��� : ���ѿ��� �����å� (�������ʳ�) (�����ǥ�٥�) */

    if (!strcmp(rule, "&���ѿ���")) { /* euc-jp */
	ucp = ((MRPH_DATA *)ptr2)->Goi2;
	while (*ucp) {
	    code = (*ucp)*0x100+*(ucp+1);
	    if (!(0xa1a5 < code && code < 0xa4a0) && /* ������ϰ� */
		!(0xa5a0 < code && code < 0xb0a0))
		return FALSE;
	    ucp += 2;
	}	    
	return TRUE;
    }

    /* &���� : ���� �����å� (�����ǥ�٥�) */

    else if (!strcmp(rule, "&����")) { /* euc-jp */
	ucp = ((MRPH_DATA *)ptr2)->Goi2;
	while (*ucp) {
	    code = (*ucp)*0x100+*(ucp+1);
	    if (code >= 0xb0a0 ||	/* �������ϰ� */
		code == 0xa1b9 || 	/* �� */
		(code == 0xa4ab && ucp == (unsigned char *)((MRPH_DATA *)ptr2)->Goi2) ||	/* �� */
		(code == 0xa5ab && ucp == (unsigned char *)((MRPH_DATA *)ptr2)->Goi2) ||	/* �� */
		(code == 0xa5f6 && ucp == (unsigned char *)((MRPH_DATA *)ptr2)->Goi2))		/* �� */
	      ;
	    else 
	      return FALSE;
	    ucp += 2;
	}	    
	return TRUE;
    }

    /* &���ʴ��� : ���ʴ��������å� (�����ǥ�٥�) */

    else if (!strcmp(rule, "&���ʴ���")) { /* euc-jp */
	ucp = ((MRPH_DATA *)ptr2)->Goi2;
	while (*ucp) {
	    code = (*ucp)*0x100+*(ucp+1);
	    code = check_char_type(code);
	    if (!(code == TYPE_KANJI || code == TYPE_HIRAGANA))
		return FALSE;
	    ucp += 2;
	}	    
	return TRUE;
    }

    /* &�Ҥ餬�� : �Ҥ餬�� �����å� (�����ǥ�٥�) */

    else if (!strcmp(rule, "&�Ҥ餬��")) { /* euc-jp */
	ucp = ((MRPH_DATA *)ptr2)->Goi2;
	while (*ucp) {
	    code = (*ucp)*0x100+*(ucp+1);
	    if (check_char_type(code) != TYPE_HIRAGANA)
		return FALSE;
	    ucp += 2;
	}	    
	return TRUE;
    }

    /* &�����Ҥ餬�� : �����ΰ�ʸ�����Ҥ餬�ʤ� �����å� (�����ǥ�٥�) */

    else if (!strcmp(rule, "&�����Ҥ餬��")) { /* euc-jp */
	ucp = ((MRPH_DATA *)ptr2)->Goi2;	/* ɽ��������å� */
	ucp += strlen(ucp) - BYTES4CHAR;
	code = (*ucp)*0x100+*(ucp+1);
	if (check_char_type(code) != TYPE_HIRAGANA)
	    return FALSE;
	return TRUE;
    }

    /* &����ʸ���� : ������ʸ����� �����å� (�����ǥ�٥�) */

    else if (!strncmp(rule, "&����ʸ����:", strlen("&����ʸ����:"))) {
	cp = rule + strlen("&����ʸ����:");

	/* �ѥ�����������礭�����FALSE */
	if (strlen(cp) > strlen(((MRPH_DATA *)ptr2)->Goi2))
	    return FALSE;

	ucp = ((MRPH_DATA *)ptr2)->Goi2;	/* ɽ��������å� */
	ucp += strlen(ucp)-strlen(cp);
	if (strcmp(ucp, cp))
	    return FALSE;
	return TRUE;
    }

    /* &�������� : �������� �����å� (�����ǥ�٥�) */

    else if (!strcmp(rule, "&��������")) { /* euc-jp */
	ucp = ((MRPH_DATA *)ptr2)->Goi2;
	while (*ucp) {
	    code = (*ucp)*0x100+*(ucp+1);
	    if (check_char_type(code) != TYPE_KATAKANA)
		return FALSE;
	    ucp += 2;
	}	    
	return TRUE;
    }

    /* &���� : ���� �����å� (�����ǥ�٥�) */

    else if (!strcmp(rule, "&����")) { /* euc-jp */
	ucp = ((MRPH_DATA *)ptr2)->Goi2;
	while (*ucp) {
	    code = (*ucp)*0x100+*(ucp+1);
	    if (check_char_type(code) != TYPE_SUUJI)
		return FALSE;
	    ucp += 2;
	}	    
	return TRUE;
    }

    /* &�ѵ��� : �ѵ��� �����å� (�����ǥ�٥�) */

    else if (!strcmp(rule, "&�ѵ���")) { /* euc-jp */
	ucp = ((MRPH_DATA *)ptr2)->Goi2;
	while (*ucp) {
	    code = (*ucp)*0x100+*(ucp+1);
	    type = check_char_type(code);
	    if (type != TYPE_EIGO && type != TYPE_KIGOU)
		return FALSE;
	    ucp += 2;
	}	    
	return TRUE;
    }

    /* &���� : ���� �����å� (�����ǥ�٥�) */

    else if (!strcmp(rule, "&����")) { /* euc-jp */
	ucp = ((MRPH_DATA *)ptr2)->Goi2;
	while (*ucp) {
	    code = (*ucp)*0x100+*(ucp+1);
	    type = check_char_type(code);
	    if (type != TYPE_KIGOU)
		return FALSE;
	    ucp += 2;
	}	    
	return TRUE;
    }

    /* &���� : ���� (����+...) �����å� (�����ǥ�٥�) */

    else if (!strcmp(rule, "&����")) { /* euc-jp */
	ucp = ((MRPH_DATA *)ptr2)->Goi2;
	pretype = 0;
	while (*ucp) {
	    code = (*ucp)*0x100+*(ucp+1);
	    type = check_char_type(code);
	    if (pretype && pretype != type)
		return TRUE;
	    pretype = type;
	    ucp += 2;
	}
	return FALSE;
    }

    /* &��ʸ�� : ʸ���� �����å� (�����ǥ�٥�) */

    else if (!strcmp(rule, "&��ʸ��")) {
	if (strlen(((MRPH_DATA *)ptr2)->Goi2) == BYTES4CHAR)
	    return TRUE;
	else 
	    return FALSE;
    }

    /* &��̣��: ��̣�ǥ����å� (������) */

    else if (!strncmp(rule, "&��̣��:", strlen("&��̣��:"))) {
	if (Thesaurus != USE_NTT || ((MRPH_DATA *)ptr2)->SM == NULL) {
	    return FALSE;
	}

	cp = rule + strlen("&��̣��:");
	/* �������ä����̣°��̾, ����ʳ��ʤ饳���ɤ��Τޤ� */
	if (*cp & 0x80) { /* euc-jp */
	    if (SM2CODEExist == TRUE)
		cp = sm2code(cp);
	    else
		cp = NULL;
	    flag = SM_NO_EXPAND_NE;
	}
	else {
	    flag = SM_CHECK_FULL;
	}

	if (cp) {
	    for (i = 0; ((MRPH_DATA *)ptr2)->SM[i]; i+=SM_CODE_SIZE) {
		if (_sm_match_score(cp, 
				    &(((MRPH_DATA *)ptr2)->SM[i]), flag))
		    return TRUE;
	    }
	}
	return FALSE;
    }

    /* &ʸ���̣��: ��̣�ǥ����å� (ʸ��) */

    else if (!strncmp(rule, "&ʸ���̣��:", strlen("&ʸ���̣��:"))) {
	if (Thesaurus != USE_NTT) {
	    return FALSE;
	}

	cp = rule + strlen("&ʸ���̣��:");
	/* �������ä����̣°��̾, ����ʳ��ʤ饳���ɤ��Τޤ� */
	if (*cp & 0x80) { /* euc-jp */
	    if (SM2CODEExist == TRUE)
		cp = sm2code(cp);
	    else
		cp = NULL;
	    flag = SM_NO_EXPAND_NE;
	}
	else {
	    flag = SM_CHECK_FULL;
	}

	if (cp) {
	    for (i = 0; ((BNST_DATA *)ptr2)->SM_code[i]; i+=SM_CODE_SIZE) {
		if (_sm_match_score(cp, &(((BNST_DATA *)ptr2)->SM_code[i]), flag))
		    return TRUE;
	    }
	}
	return FALSE;
    }

    /* &ʸ������̣��: ʸ��Τ��٤Ƥΰ�̣�Ǥ������̣�ǰʲ��ˤ��뤫�ɤ��� */

    else if (!strncmp(rule, "&ʸ������̣��:", strlen("&ʸ������̣��:"))) {
	if (Thesaurus != USE_NTT) {
	    return FALSE;
	}

	cp = rule + strlen("&ʸ������̣��:");
	/* �������ä����̣°��̾, ����ʳ��ʤ饳���ɤ��Τޤ� */
	if (*cp & 0x80) { /* euc-jp */
	    if (SM2CODEExist == TRUE)
		cp = sm2code(cp);
	    else
		cp = NULL;
	}

	if (cp && ((BNST_DATA *)ptr2)->SM_code[0] && 
	    sm_all_match(((BNST_DATA *)ptr2)->SM_code, cp)) {
		return TRUE;
	}
	return FALSE;
    }

    /* �����Ǥ�Ĺ�� */
    
    else if (!strncmp(rule, "&������Ĺ:", strlen("&������Ĺ:"))) {
	cp = rule + strlen("&������Ĺ:");
	if (cp)
	    code = atoi(cp);
	else
	    code = 0;
	if (strlen(((MRPH_DATA *)ptr2)->Goi2) >= code * BYTES4CHAR) {
	    return TRUE;
	}
	return FALSE;
    }

    else if (!strncmp(rule, "&����������:", strlen("&����������:"))) {
	cp = rule + strlen("&����������:");
	i = strlen(((MRPH_DATA *)ptr2)->Goi2) - strlen(cp);
	if (*cp && i >= 0 && !strcmp((((MRPH_DATA *)ptr2)->Goi2)+i, cp)) {
	    return TRUE;
	}
	return FALSE;
    }

    /* &ɽ��: ɽ�سʥ����å� (ʸ���٥�,������٥�) */

    else if (!strncmp(rule, "&ɽ��:", strlen("&ɽ��:"))) {
	if (!strcmp(rule + strlen("&ɽ��:"), "�ȹ�")) {
	    if ((cp = check_feature(((BNST_DATA *)ptr1)->f, "��")) == NULL) {
		return FALSE;
	    }
	    if (((BNST_DATA *)ptr2)->
		SCASE_code[case2num(cp + strlen("��:"))]) {
		return TRUE;
	    }
	    else {
		return FALSE;
	    }
	}
	else if (((BNST_DATA *)ptr2)->
		 SCASE_code[case2num(rule + strlen("&ɽ��:"))]) {
	    return TRUE;
	}
	else {
	    return FALSE;
 	}
    }

    /* &D : ��Υ��� (������٥�) */

    else if (!strncmp(rule, "&D:", strlen("&D:"))) {
	if (((BNST_DATA *)ptr2 - (BNST_DATA *)ptr1)
	    <= atoi(rule + strlen("&D:"))) {
	    return TRUE;
	}
	else {
	    return FALSE;
	}
    }

    /* &��٥�:�� : �Ѹ��Υ�٥���� (������٥�) */

    else if (!strcmp(rule, "&��٥�:��")) {
	return subordinate_level_comp((BNST_DATA *)ptr1, 
				      (BNST_DATA *)ptr2);
    }

    /* &��٥�:X : �Ѹ�����٥�X�ʾ�Ǥ��뤫�ɤ��� */

    else if (!strncmp(rule, "&��٥�:", strlen("&��٥�:"))) {
	return subordinate_level_check(rule + strlen("&��٥�:"), fd);
	/* (BNST_DATA *)ptr2); */
    }

    /* &��¦ : ��¦��FEATURE�����å� (������٥�) */
    
    else if (!strncmp(rule, "&��¦:", strlen("&��¦:"))) {
	cp = rule + strlen("&��¦:");
	if ((*cp != '^' && check_feature(((BNST_DATA *)ptr1)->f, cp)) ||
	    (*cp == '^' && !check_feature(((BNST_DATA *)ptr1)->f, cp))) {
	    return TRUE;
	} else {
	    return FALSE;
	}
    }

    /* &��¦�����å� : ��¦��FEATURE�����å� (ʸ��롼��) */
    
    else if (!strncmp(rule, "&��¦�����å�:", strlen("&��¦�����å�:"))) {
	cp = rule + strlen("&��¦�����å�:");
	for (i = 0; ((BNST_DATA *)ptr2)->child[i]; i++) {
	    if (check_feature(((BNST_DATA *)ptr2)->child[i]->f, cp)) {
		return TRUE;
	    }
	}
	return FALSE;
    }

    /* &��¦�����å� : ��¦��FEATURE�����å� (ʸ��롼��) */
    
    else if (!strncmp(rule, "&��¦�����å�:", strlen("&��¦�����å�:"))) {
	cp = rule + strlen("&��¦�����å�:");
	if (((BNST_DATA *)ptr2)->parent &&
	    check_feature(((BNST_DATA *)ptr2)->parent->f, cp)) {
	    return TRUE;
	} else {
	    return FALSE;
	}
    }

    /* &��Ω����� : ��Ω�줬Ʊ�����ɤ��� */
    
    else if (!strncmp(rule, "&��Ω�����", strlen("&��Ω�����"))) {
	/* if (!strcmp(((BNST_DATA *)ptr1)->head_ptr->Goi, 
	   ((BNST_DATA *)ptr2)->head_ptr->Goi)) { */
	if (!strcmp(((BNST_DATA *)ptr1)->Jiritu_Go, 
		    ((BNST_DATA *)ptr2)->Jiritu_Go)) {
	    return TRUE;
	} else {
	    return FALSE;
	}
    }


    /* &ʸ����ȹ� : �����Ȥ�ʸ������ʬ�ޥå� by kuro 00/12/28 */
    
    else if (!strncmp(rule, "&ʸ����ȹ�:", strlen("&ʸ����ȹ�:"))) {
      	cp = rule + strlen("&ʸ����ȹ�:");
	if (strstr(((MRPH_DATA *)ptr2)->Goi, cp)) {
	    return TRUE;
	} else {
	    return FALSE;
	}
    }

    /* &ST : ����¤���ϤǤ�����٤����� (�����Ǥ�̵��) */
    
    else if (!strncmp(rule, "&ST", strlen("&ST"))) {
	return TRUE;
    }

    /* &OPTCHECK : ���ץ����Υ����å� */
    
    else if (!strncmp(rule, "&OptCheck:", strlen("&OptCheck:"))) {
	char **opt;

	cp = rule + strlen("&OptCheck:");
	if (*cp == '-') { /* '-'��ޤ�Ǥ��������Ф� */
	    cp++;
	}

	for (opt = Options; *opt != NULL; opt++) {
	    if (!strcasecmp(cp, *opt)) {
		return TRUE;	    
	    }
	}
	return FALSE;
    }

    /*
    else if (!strncmp(rule, "&����", strlen("&����"))) {
	if (sm_all_match(((BNST_DATA *)ptr2)->SM_code, "1128********")) {
	    return TRUE;
	}
	else {
	    return FALSE;
	}
    } */

    /* &�� : �֤�����å� */

    else if (!strncmp(rule, "&��:", strlen("&��:"))) {
	cp = rule + strlen("&��:");
	if ((!strcmp(cp, "ǽư") && ((BNST_DATA *)ptr2)->voice == 0) || 
	    (!strcmp(cp, "��ư") && (((BNST_DATA *)ptr2)->voice & VOICE_UKEMI || 
				     ((BNST_DATA *)ptr2)->voice & VOICE_SHIEKI_UKEMI)) || 
	    (!strcmp(cp, "����") && (((BNST_DATA *)ptr2)->voice & VOICE_SHIEKI || 
				     ((BNST_DATA *)ptr2)->voice & VOICE_SHIEKI_UKEMI))) {
	    return TRUE;
	}
	else {
	    return FALSE;
	}
    }

    /* &���� : �����Ǥޤ���ʸ��Υݥ��󥿤򵭲� */

    else if (!strcmp(rule, "&����")) {
	matched_ptr = ptr2;
	return TRUE;
    }

    else {
#ifdef DEBUG
	fprintf(stderr, "Invalid Feature-Function (%s)\n", rule);
#endif
	return TRUE;
    }
}

/*==================================================================*/
 int feature_AND_match(FEATURE *fp, FEATURE *fd, void *p1, void *p2)
/*==================================================================*/
{
    int value;

    while (fp) {
	if (fp->cp[0] == '^' && fp->cp[1] == '&') {
	    value = check_function(fp->cp+1, fd, p1, p2);
	    if (value == TRUE) {
		return FALSE;
	    }
	} else if (fp->cp[0] == '&') {
	    value = check_function(fp->cp, fd, p1, p2);
	    if (value == FALSE) {
		return FALSE;
	    }
	} else if (fp->cp[0] == '^') {
	    if (check_feature(fd, fp->cp+1)) {
		return FALSE;
	    }
	} else {
	    if (!check_feature(fd, fp->cp)) {
		return FALSE;
	    }
	}
	fp = fp->next;
    }
    return TRUE;
}

/*==================================================================*/
int feature_pattern_match(FEATURE_PATTERN *fr, FEATURE *fd,
			  void *p1, void *p2)
/*==================================================================*/
{
    /* fr : �롼��¦��FEATURE_PATTERN,
       fd : �ǡ���¦��FEATURE
       p1 : ��������ξ�硤����¦�ι�¤��(MRPH_DATA,BNST_DATA�ʤ�)
       p2 : �ǡ���¦�ι�¤��(MRPH_DATA,BNST_DATA�ʤ�)
    */

    int i, value;

    /* PATTERN���ʤ���Хޥå� */
    if (fr->fp[0] == NULL) return TRUE;

    /* OR�γƾ���Ĵ�٤� */
    for (i = 0; fr->fp[i]; i++) {
	value = feature_AND_match(fr->fp[i], fd, p1, p2);
	if (value == TRUE) 
	    return TRUE;
    }
    return FALSE;
}

/*====================================================================
                               END
====================================================================*/
