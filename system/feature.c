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
			&#num:�� -- ����num���ܤ��ѿ��ˤ�뿷����FEATURE����Ϳ

	(3) �롼�빽¤�� <==�ȹ�==> �����Ǥޤ���ʸ�ṽ¤��
	       	<��>��<��:��>�Ȥ���FEATURE�������OK
	    	<^��>��<��:��>�Ȥ���FEATURE���ʤ����OK
	    	<&��>�ϴؿ��ƽ�
			&���ѿ��� -- ɽ��������,��ʸ��,����,�����ʥ� (������)
			&���� -- ɽ�������� (������)
	    		&ɽ��:���� -- ���ʤ����� (ʸ��)
	    		&ɽ��:�ȹ� -- ����ɽ�سʤ����ˤ��� (����)
			&D:n -- ��¤�δ֤���Υn���� (����)
			&��٥�:�� -- �������ʾ� (����)
			&��٥�:l -- ���Ȥ�l�ʾ� (����)
			&��¦:�� -- ���ˡ� (����)
			&#num:�� -- <��:��>�Ȥ���FEATURE�������OK
			            <��:��>��num���ܤ��ѿ��˳�Ǽ

	�� �ץ������Ƿ����Ǥޤ���ʸ�ṽ¤�Τ�FEATURE��Ϳ����
	����(2)�Τʤ��� assign_cfeature ���Ѥ��롥

	�� �ץ������Ƿ����Ǥޤ���ʸ�ṽ¤�Τ�����FEATURE�����
	���ɤ�����Ĵ�٤����(3)�Τʤ��� check_feature ���Ѥ��롥
*/

/*==================================================================*/
	      void print_feature(FEATURE *fp, FILE *filep)
/*==================================================================*/
{
    /* <f1><f2> ... <f3> �Ȥ��������ν��� 
       (�������ԤǤϤ��ޤ�feature��ɽ�����ʤ�) */

    while (fp) {
	if (fp->cp && strncmp(fp->cp, "��", 2))
	    fprintf(filep, "<%s>", fp->cp); 
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
	if (fp->cp && strncmp(fp->cp, "��", 2) && !strncmp(fp->cp, "C", 1))
	    fprintf(filep, "<%s>", fp->cp); 
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
	    if (fp->cp && strncmp(fp->cp, "��", 2)) {
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

    if (cp == NULL || cp[0] == NULL) {
	f->fp[nth] = NULL;
	return;
    }

    strcpy(buffer, cp);
    scp = ecp = buffer;
    while (*ecp) {
	if (*ecp == '|') {
	    *ecp = NULL;
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
	   void assign_cfeature(FEATURE **fpp, char *fname)
/*==================================================================*/
{
    char type[256];

    /* ��񤭤β�ǽ��������å� */

    sscanf(fname, "%[^:]", type);	/* �� fname��":"���ʤ�����
					   type��fname���Τˤʤ� */
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
	!((*fpp)->cp = (char *)(malloc(strlen(fname) + 1)))) {
	fprintf(stderr, "Can't allocate memory for FEATURE\n");
	exit(-1);
    }
    strcpy((*fpp)->cp, fname);
    (*fpp)->next = NULL;
}    

/*==================================================================*/
    void assign_feature(FEATURE **fpp1, FEATURE **fpp2, void *ptr)
/*==================================================================*/
{
    /*
     *  �롼���Ŭ�Ѥη�̡��롼�뤫�鹽¤�Τ�FEATURE����Ϳ����
     *  ��¤�μ��Ȥ��Ф���������ǽ�Ȥ��Ƥ���
     */

    int i, flag;
    char *cp;
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
		set_pred_voice((BNST_DATA *)ptr);	/* �������� */
		get_scase_code((BNST_DATA *)ptr);	/* ɽ�س� */
	    }
	    else if (!strcmp((*fpp2)->cp, "&ɽ��:���")) {
		for (i = 0, cp = ((BNST_DATA *)ptr)->SCASE_code; 
		     i < SCASE_CODE_SIZE; i++, cp++) 
		    *cp = 0;		
	    }
	    else if (!strncmp((*fpp2)->cp, "&ɽ��:^", strlen("&ɽ��:^"))) {
		((BNST_DATA *)ptr)->
		    SCASE_code[case2num((*fpp2)->cp + strlen("&ɽ��:^"))] = 0;
	    }
	    else if (!strncmp((*fpp2)->cp, "&ɽ��:", strlen("&ɽ��:"))) {
		((BNST_DATA *)ptr)->
		    SCASE_code[case2num((*fpp2)->cp + strlen("&ɽ��:"))] = 1;
	    }
	    else if (!strncmp((*fpp2)->cp, "&MEMO:", strlen("&MEMO:"))) {
		strcat(PM_Memo, " ");
		strcat(PM_Memo, (*fpp2)->cp + strlen("&MEMO:"));
	    }
	    else if (!strncmp((*fpp2)->cp, "&#", strlen("&#"))) {
		int gnum;
		char fprefix[64], fname[64];
		sscanf((*fpp2)->cp, "&#%d:%s", &gnum, fprefix);
		sprintf(fname, "%s%s", fprefix, G_Feature[gnum]);
		assign_cfeature(fpp1, fname);
	    }
	    else if (!strncmp((*fpp2)->cp, "&�ʻ��ѹ�:", strlen("&�ʻ��ѹ�:"))) {
		change_mrph((MRPH_DATA *)ptr, *fpp2);
	    }
	    else if (!strncmp((*fpp2)->cp, "&��̣����Ϳ:", strlen("&��̣����Ϳ:"))) {
		assign_sm((BNST_DATA *)ptr, (*fpp2)->cp + strlen("&��̣����Ϳ:"));
	    }
	    /*
	    else if (!strncmp((*fpp2)->cp, "&����", strlen("&����"))) {
		assign_f_from_dic(fpp1, ((MRPH_DATA *)ptr)->Goi);
	    }
	    */
	} else {			/* �ɲäξ�� */
	    assign_cfeature(fpp1, (*fpp2)->cp);	
	}

	fpp2 = &((*fpp2)->next);
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
	      int whether_corpus_compare(BNST_DATA *bp)
/*==================================================================*/
{
    /* �ޥå�����С�FALSE �򤫤�����
       ����Ū������Ѥ����Ҹ�η���������Ϥ�Ԥ�ʤ���
       */
    char *type;

    /* ʸ��ݥ��󥿤� NULL? */
    if (bp == NULL)
	return FALSE;

    type = (char *)check_feature(bp->f, "ID");

    if (type) {
	type +=3;
	if (!strcmp(type, "���ȡʰ��ѡ�") || 
	    !strcmp(type, "�ʼ�Ϣ�ѡ�") || 
	    !strcmp(type, "�ʶ��ڡ�") || 
	    !strcmp(type, "��ʣ�缭Ϣ�ѡ�") || 
	    !strcmp(type, "���Ρ�"))
	    return FALSE;
    }
    
    return TRUE;
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
	int _check_function_NE(struct _pos_s *p, char *class)
/*==================================================================*/
{
    if (str_eq(class, "��̾")) {
	return p->Location;
    }
    else if (str_eq(class, "��̾")) {
	return p->Person;
    }
    else if (str_eq(class, "�ȿ�̾")) {
	return p->Organization;
    }
    else if (str_eq(class, "��ͭ̾��")) {
	return p->Artifact;
    }
    else if (str_eq(class, "����¾")) {
	return p->Others;
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
      int check_function_NE(char *rule, void *ptr1, void *ptr2)
/*==================================================================*/
{
    char category[7], cp1[128], cp2[128], cp3[128], cp4[128];
    int n, threshold;

   /* 5 XB:���ʴ���:��̾:le:33
      4 ñ��:��̾:gt:3
      3 ��:��̾:1
      2 ʸ����:��������
      4 ʸ����:��̾:le:4
      3 ����:gt:3 */

    n = sscanf(rule, "%[^:]:%[^:]:%[^:]:%[^:]:%[^:]", category, cp1, cp2, cp3, cp4);

    if (n == 2) {
	if (str_eq(category, "ʸ����")) {
	    if (strcmp((char *)check_class((MRPH_DATA *)ptr2), cp1))
		return FALSE;
	    else
		return TRUE;
	}

	fprintf(stderr, "Invalid rule! (%s).\n", rule);
	return FALSE;
    }
    else if (n == 3) {
	if (str_eq(category, "��")) {
	    threshold = atoi(cp2);
	    if (str_eq(cp1, "����¾"))
		return check_Bunrui_others((MRPH_DATA *)ptr2, threshold);
	    else
		return check_Bunrui((MRPH_DATA *)ptr2, cp1, threshold);
	}
	else if (str_eq(category, "����")) {
	    threshold = atoi(cp2);
	    return compare_threshold(((MRPH_DATA *)ptr2)->eNE.self.Count, 
				     threshold, cp1);
	}

	fprintf(stderr, "Invalid rule! (%s).\n", rule);
	return FALSE;
    }
    else if (n == 4) {
	threshold = atoi(cp3);

	if (str_eq(category, "ñ��"))
	    return compare_threshold(_check_function_NE(&((MRPH_DATA *)ptr2)->eNE.self, cp1), 
				     threshold, cp2);
	else if (str_eq(category, "ʸ����"))
	    return compare_threshold(_check_function_NE(&((MRPH_DATA *)ptr2)->eNE.selfSM, cp1), 
				     threshold, cp2);
	else if (str_eq(category, "��"))
	    return compare_threshold(_check_function_NE(&((MRPH_DATA *)ptr2)->eNE.Case, cp1), 
				     threshold, cp2);

	fprintf(stderr, "Invalid rule! (%s).\n", rule);
	return FALSE;
    }
    else if (n == 5) {
	threshold = atoi(cp4);

	if (str_eq(category, "XB"))
	    return (compare_threshold(_check_function_NE(&((MRPH_DATA *)ptr2)->eNE.XB, cp2), 
				     threshold, cp3) && 
		    str_eq(((MRPH_DATA *)ptr2)->eNE.XB.Type, cp1));
	else if (str_eq(category, "AX"))
	    return (compare_threshold(_check_function_NE(&((MRPH_DATA *)ptr2)->eNE.AX, cp2), 
				     threshold, cp3) && 
		    str_eq(((MRPH_DATA *)ptr2)->eNE.XB.Type, cp1));
	else if (str_eq(category, "A��X"))
	    return (compare_threshold(_check_function_NE(&((MRPH_DATA *)ptr2)->eNE.AnoX, cp2), 
				     threshold, cp3) && 
		    str_eq(((MRPH_DATA *)ptr2)->eNE.XB.Type, cp1));
	else if (str_eq(category, "X��B"))
	    return (compare_threshold(_check_function_NE(&((MRPH_DATA *)ptr2)->eNE.XnoB, cp2), 
				     threshold, cp3) && 
		    str_eq(((MRPH_DATA *)ptr2)->eNE.XB.Type, cp1));

	fprintf(stderr, "Invalid rule! (%s).\n", rule);
	return FALSE;
    }

    fprintf(stderr, "Invalid rule! (%s).\n", rule);
    return FALSE;
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
 int check_function(char *rule, FEATURE *fd, void *ptr1, void *ptr2)
/*==================================================================*/
{
    /* rule : �롼��
       fd : �ǡ���¦��FEATURE
       p1 : �롼��¦�ι�¤��(MRPH_DATA,BNST_DATA�ʤ�)
       p2 : �ǡ���¦�ι�¤��(MRPH_DATA,BNST_DATA�ʤ�)
    */

    int i, code, type, pretype, flag;
    char *cp;
    unsigned char *ucp; 
    static BNST_DATA *pre1 = NULL, *pre2 = NULL;
    static char *prerule = NULL;

    /* &���ѿ��� : ���ѿ��� �����å� (�������ʳ�) (�����ǥ�٥�) */

    if (!strcmp(rule, "&���ѿ���")) {
	ucp = ((MRPH_DATA *)ptr2)->Goi;
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

    else if (!strcmp(rule, "&����")) {
	ucp = ((MRPH_DATA *)ptr2)->Goi;
	while (*ucp) {
	    code = (*ucp)*0x100+*(ucp+1);
	    if (code >= 0xb0a0 ||	/* �������ϰ� */
		code == 0xa1b9 || 	/* �� */
		(code == 0xa4ab && ucp == ((MRPH_DATA *)ptr2)->Goi) ||	/* �� */
		(code == 0xa5ab && ucp == ((MRPH_DATA *)ptr2)->Goi) ||	/* �� */
		(code == 0xa5f6 && ucp == ((MRPH_DATA *)ptr2)->Goi))	/* �� */
	      ;
	    else 
	      return FALSE;
	    ucp += 2;
	}	    
	return TRUE;
    }

    /* &���ʴ��� : ���ʴ��������å� (�����ǥ�٥�) */

    else if (!strcmp(rule, "&���ʴ���")) {
	ucp = ((MRPH_DATA *)ptr2)->Goi;
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

    else if (!strcmp(rule, "&�Ҥ餬��")) {
	ucp = ((MRPH_DATA *)ptr2)->Goi;
	while (*ucp) {
	    code = (*ucp)*0x100+*(ucp+1);
	    if (check_char_type(code) != TYPE_HIRAGANA)
		return FALSE;
	    ucp += 2;
	}	    
	return TRUE;
    }

    /* &�����Ҥ餬�� : �����ΰ�ʸ�����Ҥ餬�ʤ� �����å� (�����ǥ�٥�) */

    else if (!strcmp(rule, "&�����Ҥ餬��")) {
	ucp = ((MRPH_DATA *)ptr2)->Goi2;	/* ɽ��������å� */
	ucp += strlen(ucp)-2;
	code = (*ucp)*0x100+*(ucp+1);
	if (check_char_type(code) != TYPE_HIRAGANA)
	    return FALSE;
	return TRUE;
    }

    /* &�������� : �������� �����å� (�����ǥ�٥�) */

    else if (!strcmp(rule, "&��������")) {
	ucp = ((MRPH_DATA *)ptr2)->Goi;
	while (*ucp) {
	    code = (*ucp)*0x100+*(ucp+1);
	    if (check_char_type(code) != TYPE_KATAKANA)
		return FALSE;
	    ucp += 2;
	}	    
	return TRUE;
    }

    /* &���� : ���� �����å� (�����ǥ�٥�) */

    else if (!strcmp(rule, "&����")) {
	ucp = ((MRPH_DATA *)ptr2)->Goi;
	while (*ucp) {
	    code = (*ucp)*0x100+*(ucp+1);
	    if (check_char_type(code) != TYPE_SUUJI)
		return FALSE;
	    ucp += 2;
	}	    
	return TRUE;
    }

    /* &�ѵ��� : �ѵ��� �����å� (�����ǥ�٥�) */

    else if (!strcmp(rule, "&�ѵ���")) {
	ucp = ((MRPH_DATA *)ptr2)->Goi;
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

    else if (!strcmp(rule, "&����")) {
	ucp = ((MRPH_DATA *)ptr2)->Goi;
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

    else if (!strcmp(rule, "&����")) {
	ucp = ((MRPH_DATA *)ptr2)->Goi;
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
	if (strlen(((MRPH_DATA *)ptr2)->Goi) == 2)
	    return TRUE;
	else 
	    return FALSE;
    }

    /* &��ͭ: ��ͭ̾�� Feature �����å� (�����ǥ�٥�) */

    else if (!strncmp(rule, "&��ͭ:", strlen("&��ͭ:")))
	return check_function_NE(rule + strlen("&��ͭ:"), ptr1, ptr2);

    /* &��ͭC: ��ͭ̾�� ���饹�����å� */

    else if (!strncmp(rule, "&��ͭC:", strlen("&��ͭC:"))) {
	if (check_feature_NE(((MRPH_DATA *)ptr2)->f, rule + strlen("&��ͭC:")))
	    return TRUE;
	else
	    return FALSE;
    }

    /* &��̣��: ��̣�ǥ����å� */

    else if (!strncmp(rule, "&��̣��:", strlen("&��̣��:"))) {
	if (((MRPH_DATA *)ptr2)->SM == NULL) {
	    return FALSE;
	}

	cp = rule + strlen("&��̣��:");
	/* �������ä����̣°��̾, ����ʳ��ʤ饳���ɤ��Τޤ� */
	if (*cp & 0x80) {
	    if (SM2CODEExist == TRUE)
		cp = (char *)sm2code(cp);
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

    else if (!strncmp(rule, "&ʸ���̣��:", strlen("&ʸ���̣��:"))) {
	cp = rule + strlen("&ʸ���̣��:");
	/* �������ä����̣°��̾, ����ʳ��ʤ饳���ɤ��Τޤ� */
	if (*cp & 0x80) {
	    if (SM2CODEExist == TRUE)
		cp = (char *)sm2code(cp);
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

    /* �����Ǥ�Ĺ�� */
    
    else if (!strncmp(rule, "&������Ĺ:", strlen("&������Ĺ:"))) {
	cp = rule + strlen("&������Ĺ:");
	if (cp)
	    code = atoi(cp);
	else
	    code = 0;
	if (strlen(((MRPH_DATA *)ptr2)->Goi) >= code*2) {
	    return TRUE;
	}
	return FALSE;
    }

    else if (!strncmp(rule, "&����������:", strlen("&����������:"))) {
	cp = rule + strlen("&����������:");
	i = strlen(((MRPH_DATA *)ptr2)->Goi) - strlen(cp);
	if (*cp && i >= 0 && !strcmp((((MRPH_DATA *)ptr2)->Goi)+i, cp)) {
	    return TRUE;
	}
	return FALSE;
    }

    /* &�Ǿȱ� ��ͭ̾��ȱ������å� */

    else if (!strncmp(rule, "&�Ǿȱ�:", strlen("&�Ǿȱ�:")))
	return check_correspond_NE((MRPH_DATA *)ptr2, rule + strlen("&�Ǿȱ�:"));

    /* &ɽ��: ɽ�سʥ����å� (ʸ���٥�,������٥�) */

    else if (!strncmp(rule, "&ɽ��:", strlen("&ɽ��:"))) {
	if (!strcmp(rule + strlen("&ɽ��:"), "�ȹ�")) {
	    if ((cp = check_feature(((BNST_DATA *)ptr1)->f, "��")) == NULL) {
		return FALSE;
	    }
	    if (((BNST_DATA *)ptr2)->
		SCASE_code[case2num(cp + strlen("��:"))]) {
		return TRUE;
	    } else {
		return FALSE;
	    }
	}
	else if (((BNST_DATA *)ptr2)->
	    	SCASE_code[case2num(rule + strlen("&ɽ��:"))]) {
	    return TRUE;
	} else {
	    return FALSE;
 	}
    }

    /* &D : ��Υ��� (������٥�) */

    else if (!strncmp(rule, "&D:", strlen("&D:"))) {
	if (((BNST_DATA *)ptr2 - (BNST_DATA *)ptr1)
	    <= atoi(rule + strlen("&D:"))) {
	    return TRUE;
	} else {
	    return FALSE;
	}
    }

    /* &��٥� : �Ѹ��Υ�٥���� (������٥�) */

    else if (!strncmp(rule, "&��٥�:", strlen("&��٥�:"))) {
	if ((OptInhibit & OPT_INHIBIT_CLAUSE) || (whether_corpus_compare((BNST_DATA *)ptr1) == FALSE)) {
	    if (!strcmp(rule + strlen("&��٥�:"), "��"))
		/* �Ҹ�֤ζ������� */
		return subordinate_level_comp((BNST_DATA *)ptr1, 
					      (BNST_DATA *)ptr2);
	    else
		/* �Ҹ�Ϸ��������ǽ�� */
		return subordinate_level_check(rule + strlen("&��٥�:"),
					       (BNST_DATA *)ptr2);
	}
	else {
	    /* &��٥� ��Ϣ³���ƥ����å������Τ�̵�̤��� */
	    if (prerule && 
		!strncmp(prerule, "&��٥�:", strlen("&��٥�:")) && 
		ptr1 == pre1 && 
		ptr2 == pre2) {
		return TRUE;
	    }

	    prerule = rule;
	    pre1 = ptr1;
	    pre2 = ptr2;

	    return corpus_clause_comp((BNST_DATA *)ptr1, 
				      (BNST_DATA *)ptr2, 
				      TRUE);
	}
    }

    /* &��٥�ػ� : �Ѹ��Υ�٥�����å� (������٥�) --  tentative */

    else if (!strncmp(rule, "&��٥�ػ�:", strlen("&��٥�ػ�:"))) {
	/* �Ҹ�Ϸ��������ǽ�� */
	return subordinate_level_forbid(rule + strlen("&��٥�ػ�:"),
					       (BNST_DATA *)ptr2);
    }

    /* &�ᶭ�� : ��֤��ɥ����å� */

    else if (!strncmp(rule, "&�ᶭ��:", strlen("&�ᶭ��:"))) {
	if ((OptInhibit & OPT_INHIBIT_CLAUSE))
	    /* 
	       1. �롼��˽񤤤Ƥ����٥��궯�����Ȥ�����å�
	       2. ����¦������¦�Υ�٥뤬�������Ȥ�����å�
	    */
	    return (subordinate_level_check(rule + strlen("&�ᶭ��:"),
					    (BNST_DATA *)ptr2) && 
		    subordinate_level_comp((BNST_DATA *)ptr1, 
				       (BNST_DATA *)ptr2));
	else
	    return corpus_clause_barrier_check((BNST_DATA *)ptr1, 
					       (BNST_DATA *)ptr2);
    }


    /* &�ʽ� : �ʤȽҸ���Ϲ��������å� */

    else if (!strncmp(rule, "&�ʽ�:", strlen("&�ʽ�:"))) {
	if (OptInhibit & OPT_INHIBIT_CASE_PREDICATE)
	    return subordinate_level_check(rule + strlen("&�ʽ�:"),
					   (BNST_DATA *)ptr2);
	else
	    return corpus_case_predicate_check((BNST_DATA *)ptr1, 
					       (BNST_DATA *)ptr2);
    }

    /* &���� : �ʤȽҸ���ɥ����å� */

    else if (!strncmp(rule, "&����:", strlen("&����:"))) {
	if (OptInhibit & OPT_INHIBIT_BARRIER)
	    return subordinate_level_check(rule + strlen("&����:"),
					   (BNST_DATA *)ptr2);
	else
	    return corpus_barrier_check((BNST_DATA *)ptr1, 
					(BNST_DATA *)ptr2);
    }

    /* &����Ϣ�� : �ʤȽҸ���ɥ����å� (Ϣ��) */

    else if (!strncmp(rule, "&����Ϣ��:", strlen("&����Ϣ��:"))) {
	if (OptInhibit & OPT_INHIBIT_BARRIER)
	    return subordinate_level_check(rule + strlen("&����Ϣ��:"),
					   (BNST_DATA *)ptr2);
	else
	    /* ��٥�ν��� */
	    return subordinate_level_check(rule + strlen("&����Ϣ��:"),
					   (BNST_DATA *)ptr2);
	    /* ����Ū�˽������ʤ� */
	    /* return FALSE; */
	    /* ����Ū�˽������� */
	    /* return corpus_barrier_check((BNST_DATA *)ptr1, 
					(BNST_DATA *)ptr2); */
    }

    /* &�������� : �ʤȽҸ���ɥ����å� (����, �׻�) */

    else if (!strncmp(rule, "&��������:", strlen("&��������:"))) {
	/* �롼��ˤ���� */
	if (OptInhibit & OPT_INHIBIT_BARRIER) {
	    /* �����Ȥ��Ȥ� */
	    if (!(OptInhibit & OPT_INHIBIT_OPTIONAL_CASE)) {
		return subordinate_level_check_special(rule + strlen("&��������:"),
						       (BNST_DATA *)ptr2);
	    }
	    else {
		return subordinate_level_check(rule + strlen("&��������:"),
					       (BNST_DATA *)ptr2);
	    }
	}
	else{
	    return corpus_barrier_check((BNST_DATA *)ptr1, 
					(BNST_DATA *)ptr2);
	}
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

    /* &#? : FEATURE�����å����ѿ��ؤγ�Ǽ */
    
    else if (!strncmp(rule, "&#", strlen("&#"))) {
	int gnum;
	char fname[64];
	sscanf(rule, "&#%d:%s", &gnum, fname);
	if (cp = check_feature(fd, fname)) {
	    strcpy(G_Feature[gnum], cp);
	    return TRUE;
	} else {
	    return FALSE;
	}
    }

    /* &��Ω����� : ��Ω�줬Ʊ�����ɤ��� */
    
    else if (!strncmp(rule, "&��Ω�����", strlen("&��Ω�����"))) {
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

    else if (!strncmp(rule, "&����", strlen("&����"))) {
	if (sm_all_match(((BNST_DATA *)ptr2)->SM_code, "1128********")) {
	    return TRUE;
	}
	else {
	    return FALSE;
	}
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
       p1 : �롼��¦�ι�¤��(MRPH_DATA,BNST_DATA�ʤ�)
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
