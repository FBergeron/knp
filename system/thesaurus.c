/*====================================================================

		      �������饹 �����ץ����

                                               S.Kurohashi 93. 5.31

    $Id$
====================================================================*/
#include <knp.h>

int 	Thesaurus = USE_NTT;
int	ParaThesaurus = USE_BGH;

/*==================================================================*/
			void init_thesaurus()
/*==================================================================*/
{
    if (Thesaurus == USE_BGH || ParaThesaurus == USE_BGH) {
	init_bgh();
    }

    if (Thesaurus == USE_NTT || ParaThesaurus == USE_NTT) {
	init_ntt();
    }
}

/*==================================================================*/
			void close_thesaurus()
/*==================================================================*/
{
    if (Thesaurus == USE_BGH || ParaThesaurus == USE_BGH) {
	close_bgh();
    }

    if (Thesaurus == USE_NTT || ParaThesaurus == USE_NTT) {
	close_ntt();
    }
}

/*==================================================================*/
	   char *get_str_code(unsigned char *cp, int flag)
/*==================================================================*/
{
    int i, exist;
    char *code, arg = '\0';
    unsigned char *hira;
    char *(*get_code)();

    /* ʸ����ΰ�̣�ǥ����ɤ���� */

    if (flag & USE_NTT) {
	exist = SMExist;
	get_code = _get_ntt;
	if (flag & USE_SUFFIX_SM) {
	    arg = 'm';
	}
	else if (flag & USE_PREFIX_SM) {
	    arg = 'l';
	}
    }
    else if (flag & USE_BGH) {
	exist = BGHExist;
	get_code = _get_bgh;
    }

    if (exist == FALSE) return NULL;

    if ((code = get_code(cp, &arg))) {
	return code;
    }

    /* ��̣�Ǥ��ʤ����ǡ�
       ���٤Ƥ�ʸ�����������ʤξ��ϤҤ餬�ʤ��Ѵ����Ƽ������ */

    for (i = 0; i < strlen(cp); i += 2) {
	if (*(cp+i) != 0xa5) {
	    return NULL;
	}
    }
    hira = katakana2hiragana(cp);
    code = get_code(hira, &arg);
    free(hira);
    return code;
}

/*==================================================================*/
	void overflowed_function(char *str, int max, char *function)
/*==================================================================*/
{
    str[max-1] = '\0';
    fprintf(stderr, ";; Too long key <%s> in %s.\n", str, function);
    str[max-1] = GUARD;
}

/*==================================================================*/
	     void get_bnst_code(BNST_DATA *ptr, int flag)
/*==================================================================*/
{
    int strt, end, i, lookup_pos = 0;
    char str_buffer[BNST_LENGTH_MAX], *code;
    char *result_code;
    int *result_num, exist, code_unit;

    /* ʸ��ΰ�̣�ǥ����ɤ���� */

    if (flag == USE_NTT) {
	result_code = ptr->SM_code;
	result_num = &ptr->SM_num;
	exist = SMExist;
	code_unit = SM_CODE_SIZE;
    }
    else if (flag == USE_BGH) {
	result_code = ptr->BGH_code;
	result_num = &ptr->BGH_num;
	exist = BGHExist;
	code_unit = BGH_CODE_SIZE;
    }

    /* ����� */
    *result_code = '\0';

    if (exist == FALSE) return;

    /* 
       ʣ���ΰ���
       		�ޤ���°�����ꡤ��Ω��򸺤餷�Ƥ���
		�Ʒ���������Ф��Ƥޤ�ɽ�����Ĵ�١������ɤ����Ĵ�٤�
    */

    str_buffer[BNST_LENGTH_MAX-1] = GUARD;

    /* result_num ��init_bnst��0�˽��������Ƥ��� */

    /* ʬ�����ɽ�ξ��:
       �֤���װʳ�����°���ư��Ϻ������
       �ַ뺧���Ϥ���: �ֻϤ��פϺ�������ַ뺧����פǸ���
       (ʬ�����ɽ�Ǥϥ���̾��ϡ֤�����դ�����Ͽ����Ƥ���) */

    if (flag == USE_BGH && 
	ptr->mrph_ptr + ptr->mrph_num - 1 > ptr->head_ptr && 
	!strcmp(Class[(ptr->head_ptr + 1)->Hinshi][0].id, "ư��") && 
	!strcmp((ptr->head_ptr + 1)->Goi, "����")) {
	end = ptr->head_ptr - ptr->mrph_ptr + 1;
    }
    else {
	end = ptr->head_ptr - ptr->mrph_ptr;
    }

    /* �����󥿤Τߤǰ��� */
    if (check_feature((ptr->head_ptr)->f, "������")) {
	lookup_pos = USE_SUFFIX_SM;
	strt = end;
    }
    else {
	strt = 0;
    }

    for (; strt <= end; strt++) {

	/* ɽ���Τޤ� */
	*str_buffer = '\0';
	for (i = strt; i <= end; i++) {
	    if (strlen(str_buffer) + strlen((ptr->mrph_ptr + i)->Goi2) + 2 > BNST_LENGTH_MAX) {
		overflowed_function(str_buffer, BNST_LENGTH_MAX, "get_bnst_code");
		return;
	    }
	    strcat(str_buffer, (ptr->mrph_ptr + i)->Goi2);
	}

	code = get_str_code(str_buffer, flag | lookup_pos);

	if (code) {
	    strcpy(result_code, code);
	    free(code);
	}

	if (*result_code) goto Match;

	/* ɽ�����Ǹ帶�� */

	if (!str_eq((ptr->mrph_ptr + end)->Goi, 
		    (ptr->mrph_ptr + end)->Goi2)) {
	    *str_buffer = '\0';
	    for (i = strt; i < end; i++) {
		if (strlen(str_buffer) + strlen((ptr->mrph_ptr + i)->Goi2) + 2 > BNST_LENGTH_MAX) {
		    overflowed_function(str_buffer, BNST_LENGTH_MAX, "get_bnst_code");
		    return;
		}
		strcat(str_buffer, (ptr->mrph_ptr + i)->Goi2);
	    }

	    if (strlen(str_buffer) + strlen((ptr->mrph_ptr + end)->Goi) + 2 > BNST_LENGTH_MAX) {
		overflowed_function(str_buffer, BNST_LENGTH_MAX, "get_bnst_code");
		return;
	    }
	    strcat(str_buffer, (ptr->mrph_ptr + end)->Goi);

	    /* �ʷ��ƻ�ξ��ϸ촴�Ǹ��� */
	    if (str_eq(Class[(ptr->mrph_ptr + end)->Hinshi][0].id, 
		       "���ƻ�") && 
		(str_eq(Type[(ptr->mrph_ptr + end)->Katuyou_Kata].name, 
			"�ʷ��ƻ�") || 
		 str_eq(Type[(ptr->mrph_ptr + end)->Katuyou_Kata].name, 
			"�ʷ��ƻ��ü�") || 
		 str_eq(Type[(ptr->mrph_ptr + end)->Katuyou_Kata].name, 
			"�ʥη��ƻ�")))
		str_buffer[strlen(str_buffer) - 2] = '\0';

	    code = get_str_code(str_buffer, flag | lookup_pos);

	    if (code) {
		strcpy(result_code, code);
		free(code);
	    }
	    if (*result_code) goto Match;
	}
    }

  Match:
    *result_num = strlen(result_code) / code_unit;

    if (*result_num > 0) {
	if (flag == USE_NTT) {
	    char feature_buffer[BNST_LENGTH_MAX + SM_CODE_SIZE * SM_ELEMENT_MAX + 4];
	    sprintf(feature_buffer, "SM:%s:%s", str_buffer, result_code);
	    assign_cfeature(&(ptr->f), feature_buffer);
	}
	else if (flag == USE_BGH) {
	    char feature_buffer[BNST_LENGTH_MAX + 4];
	    sprintf(feature_buffer, "BGH:%s", str_buffer);
	    assign_cfeature(&(ptr->f), feature_buffer);
	}
    }
}

/*==================================================================*/
       float calc_similarity(char *exd, char *exp, int expand)
/*==================================================================*/
{
    int i, j, step;
    float score = 0, tempscore;

    /* ����ٷ׻�: ��̣�� - ��̣�� */

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
		tempscore = (float)bgh_code_match_for_case(exp+j, exd+i);
	    }
	    else if (Thesaurus == USE_NTT) {
		tempscore = ntt_code_match(exp+j, exd+i, expand);
	    }
	    if (tempscore > score) {
		score = tempscore;
	    }
	}
    }

    /* ���������������
       NTT: 0 �� 1.0
       BGH: 0 �� 7 */
    if (Thesaurus == USE_BGH) {
	score /= 7;
    }

    /* ������: 0 �� 1.0 */
    return score;
}

/*==================================================================*/
	    float CalcWordSimilarity(char *exd, char *exp)
/*==================================================================*/
{
    char *smd, *smp;
    float score = 0;

    /* ����ٷ׻�: ñ�� - ñ�� */

    smd = get_str_code(exd, Thesaurus);
    smp = get_str_code(exp, Thesaurus);

    if (smd && smp) {
	score = calc_similarity(smd, smp, 0);
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
float CalcSmWordSimilarity(char *smd, char *exp, char *del, int expand)
/*==================================================================*/
{
    char *smp;
    float score = 0;

    /* ����ٷ׻�: ��̣�� - ñ�� */

    /* NTT����ڤȤʤ�Τǡ��Ȥꤢ�������������� 03/03/27 by kuro */
    if (!strcmp(exp, "�Ȥ���")) return 0;

    if ((smp = get_str_code(exp, Thesaurus)) == NULL) {
	return 0;
    }

    if (Thesaurus == USE_NTT && del) {
	delete_specified_sm(smp, del);
    }

    if (smd && smp[0]) {
	score = calc_similarity(smd, smp, expand);
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

    /* ����ٷ׻�: ñ�� - ñ�췲 */

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

    /* ����ٷ׻�: ��̣�� - ñ�췲 */

    for (i = 0; i < num; i++) {
	score = CalcSmWordSimilarity(smd, *(exp+i), del, expand);
	if (maxscore < score) {
	    maxscore = score;
	    *pos = i;
	}
    }

    return maxscore;
}

/*====================================================================
                               END
====================================================================*/
