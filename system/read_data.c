/*====================================================================

		�����ǲ�������ɤ߹��ߡ�ʸ��ؤΤޤȤ�

                                               S.Kurohashi 91. 6.25
                                               S.Kurohashi 93. 5.31
====================================================================*/
/* $Id$ */
#include <stdio.h>
#include <stdlib.h>
#include <juman.h>
#include "path.h"
#include "const.h"
#include "dbm.h"
#include "extern.h"

int Bnst_start[MRPH_MAX];

extern char CorpusComment[BNST_MAX][DATA_LEN];

/*==================================================================*/
     void lexical_disambiguation(MRPH_DATA *m_ptr, int homo_num)
/*==================================================================*/
{
    int i, j, k, flag, pref_mrph, pref_rule;
    int real_homo_num;
    int uniq_flag[30];		/* ¾���ʻ줬�ۤʤ�����Ǥʤ� 1 */
    int matched_flag[10];	/* �����줫�η����Ǥȥޥå�����
				   �롼��������ǥѥ������ 1 */
    HomoRule	*r_ptr;
    MRPH_DATA	*loop_ptr, *loop_ptr2;
    char fname[256];

    /* �ʻ�(��ʬ��)���ۤʤ�����Ǥ�����Ĥ���uniq_flag��1�ˤ��� */

    uniq_flag[0] = 1;
    real_homo_num = 1;
    for (i = 1; i < homo_num; i++) {
	loop_ptr = m_ptr + i;
	uniq_flag[i] = 1;
	for (j = 0; j < i; j++) {
	    loop_ptr2 = m_ptr + j;

	    /* �ɤ߰ʳ����٤�Ʊ�� --> ̵�� */
	    if (loop_ptr2->Hinshi == loop_ptr->Hinshi &&
		loop_ptr2->Bunrui == loop_ptr->Bunrui &&
		str_eq(loop_ptr2->Goi, loop_ptr->Goi) &&
		loop_ptr2->Katuyou_Kata == loop_ptr->Katuyou_Kata &&
		loop_ptr2->Katuyou_Kei == loop_ptr->Katuyou_Kei) {
		uniq_flag[i] = 0;
		break;		    
	    }
	    /* ����,���ѷ�,���ѷ��Τ����줫�����㤦 --> ̵��(���谷��) */
	    else if (loop_ptr2->Hinshi == loop_ptr->Hinshi &&
		     loop_ptr2->Bunrui == loop_ptr->Bunrui &&
		     (!str_eq(loop_ptr2->Goi, loop_ptr->Goi) ||
		      loop_ptr2->Katuyou_Kata != loop_ptr->Katuyou_Kata ||
		      loop_ptr2->Katuyou_Kei != loop_ptr->Katuyou_Kei)) {
		uniq_flag[i] = 0;
		break;
	    }
	}
	if (uniq_flag[i]) real_homo_num++;
    }

    /* �¼�ŪƱ���۵��줬�ʤ���в�������Ϥ��ʤ� */

    if (real_homo_num == 1) return;


    /* ¿������ޡ�������feature��Ϳ���� */

    for (i = 0; i < homo_num; i++) {
	if (uniq_flag[i] == 0) continue;
	sprintf(fname, "��ۣ-%s", 
		Class[(m_ptr+i)->Hinshi][(m_ptr+i)->Bunrui].id);
	assign_cfeature(&(m_ptr->f), fname);

	/* ��ͭ̾��ʳ��ˤ�"����¾"��դäƤ��� */
	if (m_ptr->Bunrui != 3 && /* ��ͭ̾�� */
	    m_ptr->Bunrui != 4 && /* ��̾ */
	    m_ptr->Bunrui != 5 && /* ��̾ */
	    m_ptr->Bunrui != 6) /* �ȿ�̾ */
	    assign_cfeature(&(m_ptr->f), "��ۣ-����¾");
    }

    /* �롼�� (mrph_homo.rule)�˽��ä�ͥ�褹������Ǥ�����
       �� Ʊ���۵�����ȥ롼����η����ǿ���Ʊ�����Ȥ����
          ��Ʊ���۵��줬�롼����η����ǤΤ����줫�˥ޥå�����Ф褤
	  �롼��κǽ�η����Ǥ˥ޥå�������Τ�ͥ��(pref_mrph ������)
    */

    flag = FALSE;
    pref_mrph = 0;
    pref_rule = 0;
    for (i = 0, r_ptr = HomoRuleArray; i < CurHomoRuleSize; i++, r_ptr++) {
	if (r_ptr->pattern->mrphsize != real_homo_num) continue;
	pref_mrph = 0;
	for (k = 0; k < r_ptr->pattern->mrphsize; k++) matched_flag[k] = FALSE;
	for (j = 0, loop_ptr = m_ptr; j < homo_num; j++, loop_ptr++) {
	    if (uniq_flag[j] == 0) continue;
	    flag = FALSE;
	    for (k = 0; k < r_ptr->pattern->mrphsize; k++) {
		if (matched_flag[k]) continue;
		if (regexpmrph_match(r_ptr->pattern->mrph + k, loop_ptr) 
		    == TRUE) {
		    flag = TRUE;
		    if (k == 0) pref_mrph = j;
		    matched_flag[k] = TRUE;
		    break;
		}
	    }
	    if (flag == FALSE) break;
	}
	if (flag == TRUE) {
	    pref_rule = i;
	    break;
	}
    }

    if (flag == TRUE) {

	if (0 && OptDisplay == OPT_DEBUG) {
	    fprintf(stdout, "Lexical Disambiguation "
		    "(%dth mrph -> %dth homo by %dth rule : %s :", 
		    m_ptr - mrph_data, pref_mrph, pref_rule, 
		    (m_ptr+pref_mrph)->Goi2);
	    for (i = 0, loop_ptr = m_ptr; i < homo_num; i++, loop_ptr++)
		if (uniq_flag[i]) 
		    fprintf(stdout, " %s", 
			    Class[loop_ptr->Hinshi][loop_ptr->Bunrui].id);
	    fprintf(stdout, ")\n");
	}

	/* pref_mrph���ܤΥǡ����򥳥ԡ� */
	strcpy(m_ptr->Goi2, (m_ptr+pref_mrph)->Goi2);
	strcpy(m_ptr->Yomi, (m_ptr+pref_mrph)->Yomi);
	strcpy(m_ptr->Goi, (m_ptr+pref_mrph)->Goi);
	m_ptr->Hinshi = (m_ptr+pref_mrph)->Hinshi;
	m_ptr->Bunrui = (m_ptr+pref_mrph)->Bunrui;
	m_ptr->Katuyou_Kata = (m_ptr+pref_mrph)->Katuyou_Kata;
	m_ptr->Katuyou_Kei = (m_ptr+pref_mrph)->Katuyou_Kei;

	assign_feature(&(m_ptr->f), &((HomoRuleArray + pref_rule)->f), 
		     m_ptr);

    } else {

	if (1 || OptDisplay == OPT_DEBUG) {
	    fprintf(stdout, ";; Cannot disambiguate lexical ambiguities"
		    " (%dth mrph : %s ?", m_ptr - mrph_data,
		    (m_ptr+pref_mrph)->Goi2);
	    for (i = 0, loop_ptr = m_ptr; i < homo_num; i++, loop_ptr++)
		if (uniq_flag[i]) 
		    fprintf(stdout, " %s", 
			    Class[loop_ptr->Hinshi][loop_ptr->Bunrui].id);
	    fprintf(stdout, ")\n");
	}
    }
}

/*==================================================================*/
		       int read_mrph(FILE *fp)
/*==================================================================*/
{
    U_CHAR input_buffer[1024+IMI_MAX];
    U_CHAR imi_buffer[IMI_MAX];
    MRPH_DATA  *m_ptr = mrph_data, *ptr;
    int homo_num, offset, mrph_item, i;

    Mrph_num = 0;
    homo_num = 0;
    Comment[0] = '\0';
    PM_Memo[0] = '\0';

    while (1) {

	if (fgets(input_buffer, 1024+IMI_MAX, fp) == NULL) return EOF;

	if (input_buffer[strlen(input_buffer)-1] != '\n') {
	    fprintf(stderr, "Too long mrph <%s> !\n", input_buffer);
	    return FALSE;
	}

	/* -i �ˤ�륳���ȹ� */
	if ( OptIgnoreChar && *input_buffer == OptIgnoreChar ) {
	    fprintf(stdout, "%s", input_buffer);
	    continue;
	}

	/* # �ˤ�������Υ����ȹ� */

	if (Mrph_num == 0 && input_buffer[0] == '#') {
	    input_buffer[strlen(input_buffer)-1] = '\0';
	    strcpy(Comment, input_buffer);
	}

	/* ���ϺѤߤξ�� */

	else if (Mrph_num == 0 && input_buffer[0] == '*') {
	    OptAnalysis = OPT_PM;
	    Bnst_num = 0;
	    for (i = 0; i < MRPH_MAX; i++) Bnst_start[i] = 0;
	    if (sscanf(input_buffer, "* %d%c", 
		       &Best_mgr.dpnd.head[Bnst_num],
		       &Best_mgr.dpnd.type[Bnst_num]) != 2)  {
		fprintf(stderr, "Invalid input <%s> !\n", input_buffer);
		return FALSE;
	    }
	    Bnst_start[Mrph_num] = 1;
	    Bnst_num++;
	}
	else if (input_buffer[0] == '*') {
	    if (OptAnalysis != OPT_PM || 
		sscanf(input_buffer, "* %d%c", 
		       &Best_mgr.dpnd.head[Bnst_num],
		       &Best_mgr.dpnd.type[Bnst_num]) != 2) {
		fprintf(stderr, "Invalid input <%s> !\n", input_buffer);
		return FALSE;
	    }
	    Bnst_start[Mrph_num] = 1;
	    Bnst_num++;
	}	    

	/* ʸ�� */

	else if (str_eq(input_buffer, "EOS\n")) {
	    
	    if (homo_num) {	/* ����Ʊ���۵��쥻�åȤ�����н������� */
		lexical_disambiguation(m_ptr - homo_num - 1, homo_num + 1);
		Mrph_num -= homo_num;
		m_ptr -= homo_num;
		homo_num = 0;
	    }
	    return TRUE;
	}

	/* �̾�η����� */

	else {
	    
	    if (input_buffer[0] != '@' && homo_num) {

		/* Ʊ���۵���ޡ������ʤ���С�����Ʊ���۵��쥻�åȤ������
	           lexical_disambiguation��Ƥ�ǽ��� */		   

		lexical_disambiguation(m_ptr - homo_num - 1, homo_num + 1);
		Mrph_num -= homo_num;
		m_ptr -= homo_num;
		homo_num = 0;
	    }

	    /* �������ۤ��ʤ��褦�˥����å� */
	    if (Mrph_num >= MRPH_MAX) {
		fprintf(stderr, "Too many mrph (%s %s%s...)!\n", 
			Comment, mrph_data, mrph_data+1);
		Mrph_num = 0;
		return readtoeos(fp);
	    }

	    /* �����Ǿ��� :
	       ����(���ѷ�) �ɤ� ����(����) 
	       �ʻ�(+�ֹ�) ��ʬ��(+�ֹ�) ���ѷ�(+�ֹ�) ���ѷ�(+�ֹ�) 
	       ��̣����
	    */

	    offset = (input_buffer[0] == '@') ? 2 : 0;
	    mrph_item = sscanf(input_buffer + offset,
			       "%s %s %s %*s %d %*s %d %*s %d %*s %d %s", 
			       m_ptr->Goi2, m_ptr->Yomi, m_ptr->Goi, 
			       &(m_ptr->Hinshi), &(m_ptr->Bunrui),
			       &(m_ptr->Katuyou_Kata), &(m_ptr->Katuyou_Kei),
			       m_ptr->Imi);

	    if (mrph_item == 8) {
		;
	    }
	    else if (mrph_item == 7) {
		strcpy(m_ptr->Imi, "NIL");
	    }
	    else {
		fprintf(stderr, "Invalid input (%d items)<%s> !\n", 
			mrph_item, input_buffer);
		if (Comment[0]) fprintf(stderr, "(%s)\n", Comment);
		return FALSE;
	    }   
	    m_ptr->type = 0;
	    /* clear_feature(&(m_ptr->f)); 
	       main��ʸ���ȤΥ롼�פ���Ƭ�ǽ����˰�ư */

	    /* Ʊ���۵���ϰ�ö mrph_data �ˤ���� */
	    if (input_buffer[0] == '@')	homo_num++;

	    Mrph_num++;
	    m_ptr++;
	}
    }
}

/*==================================================================*/
		       int readtoeos(FILE *fp)
/*==================================================================*/
{
    U_CHAR input_buffer[1024+IMI_MAX];

    while (1) {
	if (fgets(input_buffer, 1024+IMI_MAX, fp) == NULL) return EOF;
	if (str_eq(input_buffer, "EOS\n")) return FALSE;
    }
}

/*==================================================================*/
	    void change_mrph(MRPH_DATA *m_ptr, FEATURE *f)
/*==================================================================*/
{
    char *cp;
    char h_buffer[62], b_buffer[62], kata_buffer[62], kei_buffer[62];
    int i, num;

    while (f) {
	if (f->cp && !strncmp(f->cp, "&�ʻ��ѹ�:", strlen("&�ʻ��ѹ�:"))) {
	    
	    m_ptr->Hinshi = 0;
	    m_ptr->Bunrui = 0;
	    m_ptr->Katuyou_Kata = 0;
	    m_ptr->Katuyou_Kei = 0;

	    num = sscanf(f->cp, "%*[^:]:%[^:]:%[^:]:%[^:]:%[^:]", 
			 h_buffer, b_buffer, kata_buffer, kei_buffer);

	    m_ptr->Hinshi = get_hinsi_id(h_buffer);
	    if (num >= 2) {
		if (!strcmp(b_buffer, "*"))
		    m_ptr->Bunrui = 0;
		else 
		    m_ptr->Bunrui = get_bunrui_id(b_buffer, m_ptr->Hinshi);
	    }
	    if (num >= 3) {
		m_ptr->Katuyou_Kata = get_type_id(kata_buffer);
		m_ptr->Katuyou_Kei = get_form_id(kei_buffer, 
						 m_ptr->Katuyou_Kata);
	    }

	    /* �ʻ��ѹ������Ѥʤ��ξ��ϸ������ѹ����� */
	    /* �� ��(���Ѥʤ������Ѥ���)�ϰ��äƤ��ʤ� */
	    if (m_ptr->Katuyou_Kata == 0) {
		strcpy(m_ptr->Goi, m_ptr->Goi2);
	    }
	}
	f = f->next;
    }
}

/*==================================================================*/
		      int get_Bunrui(char *cp)
/*==================================================================*/
{
    int j;

    for (j = 1; Class[6][j].id; j++) {
	if (str_eq(Class[6][j].id, cp))
	    return j;
    }
}

/*==================================================================*/
		       void assign_mrph_prop()
/*==================================================================*/
{
    int i, j;
    MrphRule	*r_ptr;
    MRPH_DATA	*m_ptr;
    
    for (j = 0, r_ptr = MrphRuleArray; j < CurMrphRuleSize; j++, r_ptr++) {
	for (i = 0; i < Mrph_num; i++) {
	    m_ptr = mrph_data + i;
    	    if (regexpmrphrule_match(r_ptr, m_ptr) == TRUE) {
		assign_feature(&(m_ptr->f), &(r_ptr->f), m_ptr);
		change_mrph(m_ptr, r_ptr->f);
	    }
	}
    }
}

/*==================================================================*/
		      void assign_mrph_feature()
/*==================================================================*/
{
    int i, j, k, pos, apos, flag, match_tail;
    char decision[9];
    MrphRule *r_ptr;
    MRPH_DATA *m_ptr;

    for (i = 0; i < Mrph_num; i++) {
	mrph_data[i].SM[0] = '\0';
	init_NE(&(mrph_data[i].NE));
	init_NE(&(mrph_data[i].eNE));
	assign_f_from_dic(i);

	mrph_data[i].eNE.self = mrph_data[i].NE.self;
    }

    /* �Ȥꤢ�������٤Ƥη����ǤˤդäƤ����� */
    for (i = 0; i < Mrph_num; i++) {
	/* �������ܸ� (��Ω��, ̾����̾��������, ̾����Ƭ������) */
	pos = -1;
	for (j = i-1; j >= 0; j--) {
	    if (check_feature(mrph_data[j].f, "��Ω") || 
		(mrph_data[j].Hinshi == 14 && mrph_data[j].Bunrui == 2) || 
		(mrph_data[j].Hinshi == 13 && mrph_data[j].Bunrui == 1)) {
		pos = j;
		break;
	    }
	}
	if (pos != -1)
	    mrph_data[i].eNE.after = mrph_data[pos].NE.after;

	/* ������ܸ� (��Ω��, ̾����̾��������, ̾����Ƭ������) */
	pos = -1;
	for (j = i+1; j < Mrph_num; j++) {
	    if (check_feature(mrph_data[j].f, "��Ω") || 
		(mrph_data[j].Hinshi == 14 && mrph_data[j].Bunrui == 2) || 
		(mrph_data[j].Hinshi == 13 && mrph_data[j].Bunrui == 1)) {
		pos = j;
		break;
	    }
	}
	if (pos != -1)
	    mrph_data[i].eNE.before = mrph_data[pos].NE.before;

	/* A �� B */
	if (flag != 2 && mrph_data[i].Hinshi == 6 && check_feature(mrph_data[i].f, "��Ω")) {
	    flag = 1;
	    apos = i;
	}
	else if (flag == 1 && str_eq(mrph_data[i].Goi, "��") && mrph_data[i].Hinshi == 9) {
	    flag = 2;
	}
	else if (flag == 2 && mrph_data[i].Hinshi == 6 && check_feature(mrph_data[i].f, "��Ω")) {
	    mrph_data[i].eNE.AnoB = mrph_data[apos].NE.AnoB;
	    mrph_data[apos].eNE.BnoA = mrph_data[i].NE.BnoA;

	    flag = 1;
	    apos = i;
	}
	else {
	    flag = 0;
	}
    }

    /* �롼���Ŭ�� (eNE ���Ф���) */
    for (i = 0; i < Mrph_num; i++) {
	m_ptr = mrph_data + i;
	/* feature �� */
	NE2feature(m_ptr);

	/* ��ʬ����� */
	for (j = 0, r_ptr = NERuleArray; j < CurNERuleSize; j++, r_ptr++) {
    	    if (regexpmrphrule_match(r_ptr, m_ptr) == TRUE) {
		assign_feature(&(m_ptr->f), &(r_ptr->f), m_ptr);

		/* �����Ǻ�ʬ����ѹ�
		sscanf(r_ptr->f->cp, "%*[^:]:%*[^:]:%s", decision);
		if (strcmp(decision, "����¾"))
		    m_ptr->Bunrui = get_Bunrui(decision);
		    */
		break;
	    }
	}
    }

    /* ʣ��̾��롼���Ŭ�� */
    for (i = 0; i < Mrph_num; i++) {
	m_ptr = mrph_data + i;

	for (j = 0, r_ptr = CNRuleArray; j < CurCNRuleSize; j++, r_ptr++) {
            if (r_ptr->self_pattern->mrph->Goi[0] &&
                !strcmp(r_ptr->self_pattern->mrph->Goi[0], "����")) {
                if ((match_tail = regexpmrphrule_match2(r_ptr, m_ptr)) != -1) {
                    for (k = i; k < (Mrph_num - match_tail); k++) {
                        m_ptr = mrph_data + k;
                        assign_feature(&(m_ptr->f), &(r_ptr->f), m_ptr);
                    }
                }
            }
	}
    }
}

/*==================================================================*/
	       BNST_DATA *init_bnst(MRPH_DATA *m_ptr)
/*==================================================================*/
{
    int i;
    char *cp;
    BNST_DATA *b_ptr;

    b_ptr = bnst_data + Bnst_num;
    b_ptr->num = Bnst_num;
    Bnst_num++;
    if (Bnst_num > BNST_MAX) {
	fprintf(stderr, "Too many bnst (%s %s%s...)!\n", 
		Comment, mrph_data, mrph_data+1);
	Bnst_num = 0;
	return NULL;
    }

    b_ptr->mrph_ptr = m_ptr;

    b_ptr->mrph_num = 0;
    b_ptr->settou_num = 0;
    b_ptr->jiritu_num = 0;
    b_ptr->fuzoku_num = 0;
    b_ptr->settou_ptr = NULL;
    b_ptr->jiritu_ptr = NULL;
    b_ptr->fuzoku_ptr = NULL;

    b_ptr->Jiritu_Go[0] = '\0';
    b_ptr->BGH_num = 0;
    b_ptr->SM_num = 0;

    b_ptr->para_key_type = PARA_KEY_O;
    b_ptr->para_top_p = FALSE;
    b_ptr->para_type = PARA_NIL;
    b_ptr->to_para_p = FALSE;

    b_ptr->cpm_ptr = NULL;

    b_ptr->length = 0;
    b_ptr->space = 0;
    
    for (i = 0, cp = b_ptr->SCASE_code; i < 11; i++, cp++) *cp = 0;

    /* clear_feature(&(b_ptr->f));
       main��ʸ���ȤΥ롼�פ���Ƭ�ǽ����˰�ư */

    CorpusComment[b_ptr->num][0] = '\0';

    return b_ptr;
}

/*==================================================================*/
	 void push_prfx(BNST_DATA *b_ptr, MRPH_DATA *m_ptr)
/*==================================================================*/
{
    if (b_ptr->settou_num == 0) 
      b_ptr->settou_ptr = m_ptr;
    b_ptr->settou_num++;
    b_ptr->mrph_num++;
}

/*==================================================================*/
	 void push_indp(BNST_DATA *b_ptr, MRPH_DATA *m_ptr)
/*==================================================================*/{
    if (b_ptr->jiritu_num == 0)
      b_ptr->jiritu_ptr = m_ptr;
    while ((strlen(b_ptr->Jiritu_Go) + strlen(m_ptr->Goi)) > b_ptr->Jiritu_Go_Size) {
	b_ptr->Jiritu_Go_Size += WORD_LEN_MAX;
	b_ptr->Jiritu_Go = (char *)realloc_data(b_ptr->Jiritu_Go, b_ptr->Jiritu_Go_Size, "push_indp");
    }
    strcat(b_ptr->Jiritu_Go, m_ptr->Goi);
    b_ptr->jiritu_num++;
    b_ptr->mrph_num++;
}

/*==================================================================*/
	 void push_sufx(BNST_DATA *b_ptr, MRPH_DATA *m_ptr)
/*==================================================================*/
{
    if (b_ptr->fuzoku_num == 0) 
      b_ptr->fuzoku_ptr = m_ptr;
    b_ptr->fuzoku_num++;
    b_ptr->mrph_num++;
}

#define MRPH_PRFX 0	/* ���������ʸ��ˤޤȤ��ݤξ��֥ե饰 */
#define MRPH_INDP 1
#define MRPH_SUFX 2

/*==================================================================*/
			 int make_bunsetsu()
/*==================================================================*/
{
    int prev_stat, now_stat;
    int i, j;
    MRPH_DATA	*m_ptr;
    BNST_DATA	*b_ptr = NULL;
    
    Bnst_num = 0;
    prev_stat = MRPH_SUFX;

    for (i = 0, m_ptr = mrph_data; i < Mrph_num; i++, m_ptr++) {

	if (check_feature(m_ptr->f, "��ʣ��") ||
	    check_feature(m_ptr->f, "��Ω"))
	    now_stat = MRPH_INDP;
	else if (check_feature(m_ptr->f, "��°"))
	    now_stat = MRPH_SUFX;
	else if (check_feature(m_ptr->f, "��Ƭ"))
	    now_stat = MRPH_PRFX;
	else
	    fprintf(stdout, ";; Invalid input \n");

	/* �� ��Ŭ�ڤ����� (�������ǤϤ��ޤ롤��Ƭ������)
	   ��������Ω��Ȥ��ư��� (����Ū) */

	if (i == 0 && now_stat == MRPH_SUFX) {
	    fprintf(stdout, 
		    ";; Invalid input (prefix and suffix)\"%s%s ... \"!\n",
		    mrph_data[0].Goi2, mrph_data[1].Goi2);
	    /* return FALSE; */
	    now_stat = MRPH_INDP;
	} else if (prev_stat == MRPH_PRFX &&
		   now_stat == MRPH_SUFX) {
	    fprintf(stdout, 
		    ";; Invalid input (prefix and suffix)\"... %s%s ... \"!\n",
		    mrph_data[i-1].Goi2, mrph_data[i].Goi2);
	    /* return FALSE; */
	    now_stat = MRPH_INDP;
	}

	/* ���� */

	switch (now_stat) {
	case MRPH_PRFX:
	    switch (prev_stat) {
	    case MRPH_PRFX:
		push_prfx(b_ptr, m_ptr);
		break;
	    default:
		if ((b_ptr = init_bnst(m_ptr)) == NULL) return FALSE;
		push_prfx(b_ptr, m_ptr);
		break;
	    }
	    break;
	  case MRPH_INDP:
	    switch (prev_stat) {
	    case MRPH_PRFX:
		push_indp(b_ptr, m_ptr);
		break;
	    case MRPH_INDP:
		if (!check_feature(m_ptr->f, "��ʣ��"))
		    if ((b_ptr = init_bnst(m_ptr)) == NULL) return FALSE;
		push_indp(b_ptr, m_ptr);
		break;
	    case MRPH_SUFX:
		if ((b_ptr = init_bnst(m_ptr)) == NULL) return FALSE;
		push_indp(b_ptr, m_ptr);
		break;
	    }
	    break;
	  case MRPH_SUFX:
	    if (m_ptr->type == BNST_RULE_SKIP)
	      b_ptr->mrph_num++;
	    else 
	      push_sufx(b_ptr, m_ptr);
	    break;
	}
	
	prev_stat = now_stat;
    }    

    for (i = 0, b_ptr = bnst_data; i < Bnst_num; i++, b_ptr++) {
	for (j = 0, m_ptr = b_ptr->mrph_ptr; j < b_ptr->mrph_num;
					     j++, m_ptr++) {
	    if ((b_ptr->length += strlen(m_ptr->Goi2)) >
		BNST_LENGTH_MAX) {
		fprintf(stderr, "Too big bnst (%s %s%s...)!\n", 
			Comment, b_ptr->mrph_ptr, b_ptr->mrph_ptr+1);
		return FALSE;
	    }
	}
    }
    return TRUE;
}

/*==================================================================*/
			int make_bunsetsu_pm()
/*==================================================================*/
{
    int i, j;
    char *cp;
    MRPH_DATA	*m_ptr;
    BNST_DATA	*b_ptr = bnst_data;

    for (i = 0, m_ptr = mrph_data; i < Mrph_num; i++, m_ptr++) {
	if (Bnst_start[i]) {
	    if (i != 0) b_ptr++;
	    b_ptr->mrph_ptr = m_ptr;
	    b_ptr->mrph_num = 1;
	    b_ptr->jiritu_ptr = m_ptr;
	    b_ptr->jiritu_num = 1;
	    b_ptr->settou_num = 0;
	    b_ptr->fuzoku_num = 0;
	    b_ptr->length = 0;
	    for (j = 0, cp = b_ptr->SCASE_code; j < 11; j++, cp++)
		*cp = 0;
	    /* clear_feature(&(b_ptr->f));
	       main��ʸ���ȤΥ롼�פ���Ƭ�ǽ����˰�ư */
	} else {
	    b_ptr->mrph_num ++;
	    b_ptr->jiritu_num ++;
	}
    }
    
    for (i = 0, b_ptr = bnst_data; i < Bnst_num; i++, b_ptr++) {
	
	assign_cfeature(&(b_ptr->f), "���Ϻ�");

	for (j = 0, m_ptr = b_ptr->mrph_ptr; j < b_ptr->mrph_num;
					     j++, m_ptr++) {
	    if ((b_ptr->length += strlen(m_ptr->Goi2)) >
		BNST_LENGTH_MAX) {
		fprintf(stderr, "Too big bunsetsu (%s...)!\n", 
			b_ptr->mrph_ptr);
		return FALSE;
	    }
	}
    }
    return TRUE;
}

/*====================================================================
				 END
====================================================================*/
