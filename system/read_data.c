/*====================================================================

		�����ǲ�������ɤ߹��ߡ�ʸ��ؤΤޤȤ�

                                               S.Kurohashi 91. 6.25
                                               S.Kurohashi 93. 5.31

    $Id$
====================================================================*/
#include "knp.h"

int Bnst_start[MRPH_MAX];
int ArticleID = 0;
int preArticleID = 0;

extern char CorpusComment[BNST_MAX][DATA_LEN];

/*==================================================================*/
void lexical_disambiguation(SENTENCE_DATA *sp, MRPH_DATA *m_ptr, int homo_num)
/*==================================================================*/
{
    int i, j, k, flag, pref_mrph, pref_rule;
    int real_homo_num;
    int uniq_flag[HOMO_MAX];		/* ¾���ʻ줬�ۤʤ�����Ǥʤ� 1 */
    int matched_flag[HOMO_MRPH_MAX];	/* �����줫�η����Ǥȥޥå�����
					   �롼��������ǥѥ������ 1 */
    HomoRule	*r_ptr;
    MRPH_DATA	*loop_ptr, *loop_ptr2;
    char fname[256];

    /* ��������������ۤ��Ƥ���С�������Ĥ��������å����� */
    if (homo_num > HOMO_MAX) {
	homo_num = HOMO_MAX;
    }

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
	    /* ���ѷ�,���ѷ��Τ����줫���㤦 (����, ����, �椯, ...) --> ̵�� */
	    else if (loop_ptr2->Hinshi == loop_ptr->Hinshi &&
		     loop_ptr2->Bunrui == loop_ptr->Bunrui &&
		     str_eq(loop_ptr2->Goi, loop_ptr->Goi) &&
		      (loop_ptr2->Katuyou_Kata != loop_ptr->Katuyou_Kata ||
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

    assign_cfeature(&(m_ptr->f), "��ۣ");
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
	if (r_ptr->pattern->mrphsize > HOMO_MRPH_MAX) {
	    fprintf(stderr, "The number of Rule morphs is too large in HomoRule.\n");
	    exit(1);
	}
	pref_mrph = 0;
	for (k = 0; k < r_ptr->pattern->mrphsize; k++) matched_flag[k] = FALSE;
	for (j = 0, loop_ptr = m_ptr; j < homo_num; j++, loop_ptr++) {
	    if (uniq_flag[j] == 0) continue;
	    flag = FALSE;
	    for (k = 0; k < r_ptr->pattern->mrphsize; k++) {
		if (matched_flag[k] && (r_ptr->pattern->mrph + k)->ast_flag != AST_FLG)
		    continue;
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
	    for (k = 0; k < r_ptr->pattern->mrphsize; k++) {
		if (matched_flag[k] == FALSE) {
		    flag = FALSE;
		    break;
		}
	    }
	    if (flag == TRUE) {
		pref_rule = i;
		break;
	    }
	}
    }

    if (flag == TRUE) {

	if (0 && OptDisplay == OPT_DEBUG) {
	    fprintf(Outfp, "Lexical Disambiguation "
		    "(%dth mrph -> %dth homo by %dth rule : %s :", 
		    m_ptr - sp->mrph_data, pref_mrph, pref_rule, 
		    (m_ptr+pref_mrph)->Goi2);
	    for (i = 0, loop_ptr = m_ptr; i < homo_num; i++, loop_ptr++)
		if (uniq_flag[i]) 
		    fprintf(Outfp, " %s", 
			    Class[loop_ptr->Hinshi][loop_ptr->Bunrui].id);
	    fprintf(Outfp, ")\n");
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
	    fprintf(Outfp, ";; Cannot disambiguate lexical ambiguities"
		    " (%dth mrph : %s ?", m_ptr - sp->mrph_data,
		    (m_ptr+pref_mrph)->Goi2);
	    for (i = 0, loop_ptr = m_ptr; i < homo_num; i++, loop_ptr++)
		if (uniq_flag[i]) 
		    fprintf(Outfp, " %s", 
			    Class[loop_ptr->Hinshi][loop_ptr->Bunrui].id);
	    fprintf(Outfp, ")\n");
	}
    }
}

/*==================================================================*/
		       int readtoeos(FILE *fp)
/*==================================================================*/
{
    U_CHAR input_buffer[DATA_LEN];

    while (1) {
	if (fgets(input_buffer, DATA_LEN, fp) == NULL) return EOF;
	if (str_eq(input_buffer, "EOS\n")) return FALSE;
    }
}

/*==================================================================*/
	     int read_mrph_file(FILE *fp, U_CHAR *buffer)
/*==================================================================*/
{
    int len;
#ifdef _WIN32
    char *EUCbuffer;
#endif

    if (fgets(buffer, DATA_LEN, fp) == NULL) return EOF;

#ifdef _WIN32
    EUCbuffer = toStringEUC(buffer);
    strcpy(buffer, EUCbuffer);
    free(EUCbuffer);
#endif

    /* Server �⡼�ɤξ��� ��� \r\n �ˤʤ�*/
    if (OptMode == SERVER_MODE) {
	len = strlen(buffer);
	if (len > 2 && buffer[len-1] == '\n' && buffer[len-2] == '\r') {
	    buffer[len-2] = '\n';
	    buffer[len-1] = '\0';
	}

	if (buffer[0] == EOf) 
	    return EOF;
    }

    return TRUE;
}

/*==================================================================*/
	      int read_mrph(SENTENCE_DATA *sp, FILE *fp)
/*==================================================================*/
{
    U_CHAR input_buffer[DATA_LEN];
    MRPH_DATA  *m_ptr = sp->mrph_data;
    int homo_num, offset, mrph_item, i, len, homo_flag;

    sp->Mrph_num = 0;
    homo_num = 0;
    ErrorComment = NULL;
    PM_Memo[0] = '\0';
    input_buffer[DATA_LEN-1] = '\n';

    while (1) {
	if (read_mrph_file(fp, input_buffer) == EOF) return EOF;

	if (input_buffer[DATA_LEN-1] != '\n' || input_buffer[strlen(input_buffer)-1] != '\n') {
	    fprintf(stderr, "Too long mrph <%s> !\n", input_buffer);
	    return FALSE;
	}

	/* -i �ˤ�륳���ȹ� */
	if (OptIgnoreChar && *input_buffer == OptIgnoreChar) {
	    fprintf(Outfp, "%s", input_buffer);
	    fflush(Outfp);
	    continue;
	}

	/* # �ˤ�������Υ����ȹ� */

	if (input_buffer[0] == '#') {
	    input_buffer[strlen(input_buffer)-1] = '\0';
	    sp->Comment = strdup(input_buffer);
	    sp->KNPSID = (char *)malloc_data(strlen(input_buffer), "read_mrph");
	    sscanf(input_buffer, "# %s", sp->KNPSID);

	    /* ʸ�Ϥ��Ѥ�ä����ͭ̾�쥹���å��򥯥ꥢ */
	    if (!strncmp(input_buffer, "# S-ID", 6)) {
		sscanf(input_buffer, "# S-ID:%d", &ArticleID);
		if (ArticleID && preArticleID && ArticleID != preArticleID) {
		    if (OptNE != OPT_NORMAL) {
			clearNE();
		    }
		    if (OptDisc == OPT_DISC) {
			ClearSentences(sp);
		    }
		}
		preArticleID = ArticleID;
	    }
	}

	/* ���ϺѤߤξ�� */

	else if (sp->Mrph_num == 0 && input_buffer[0] == '*') {
	    OptInput = OPT_PARSED;
	    if (OptDisc == OPT_DISC) {
		OptAnalysis = OPT_CASE2;
	    }
	    sp->Bnst_num = 0;
	    for (i = 0; i < MRPH_MAX; i++) Bnst_start[i] = 0;
	    if (sscanf(input_buffer, "* %d%c", 
		       &(sp->Best_mgr->dpnd.head[sp->Bnst_num]),
		       &(sp->Best_mgr->dpnd.type[sp->Bnst_num])) != 2)  {
		fprintf(stderr, "Invalid input <%s> !\n", input_buffer);
		OptInput = OPT_RAW;
		return readtoeos(fp);
	    }
	    Bnst_start[sp->Mrph_num] = 1;
	    sp->Bnst_num++;
	}
	else if (input_buffer[0] == '*') {
	    if (OptInput != OPT_PARSED || 
		sscanf(input_buffer, "* %d%c", 
		       &(sp->Best_mgr->dpnd.head[sp->Bnst_num]),
		       &(sp->Best_mgr->dpnd.type[sp->Bnst_num])) != 2) {
		fprintf(stderr, "Invalid input <%s> !\n", input_buffer);
		return readtoeos(fp);
	    }
	    Bnst_start[sp->Mrph_num] = 1;
	    sp->Bnst_num++;
	}	    

	/* ʸ�� */

	else if (str_eq(input_buffer, "EOS\n")) {
	    /* �����Ǥ���Ĥ�ʤ��Ȥ� */
	    if (sp->Mrph_num == 0) {
		return FALSE;
	    }

	    if (homo_num) {	/* ����Ʊ���۵��쥻�åȤ�����н������� */
		lexical_disambiguation(sp, m_ptr - homo_num - 1, homo_num + 1);
		sp->Mrph_num -= homo_num;
		m_ptr -= homo_num;
		homo_num = 0;
	    }
	    return TRUE;
	}

	/* �̾�η����� */

	else {

	    /* Ʊ���۵��줫�ɤ��� */
	    if (input_buffer[0] == '@' && input_buffer[1] == ' ' && input_buffer[2] != '@') {
		homo_flag = 1;
	    }
	    else {
		homo_flag = 0;
	    }
	    
	    if (homo_num && homo_flag == 0) {

		/* Ʊ���۵���ޡ������ʤ���С�����Ʊ���۵��쥻�åȤ������
	           lexical_disambiguation��Ƥ�ǽ��� */		   

		lexical_disambiguation(sp, m_ptr - homo_num - 1, homo_num + 1);
		sp->Mrph_num -= homo_num;
		m_ptr -= homo_num;
		homo_num = 0;
	    }

	    /* �������ۤ��ʤ��褦�˥����å� */
	    if (sp->Mrph_num >= MRPH_MAX) {
		fprintf(stderr, "Too many mrph (%s %s%s...)!\n", 
			sp->Comment ? sp->Comment : "", sp->mrph_data, sp->mrph_data+1);
		sp->Mrph_num = 0;
		return readtoeos(fp);
	    }

	    /* �����Ǿ��� :
	       ����(���ѷ�) �ɤ� ����(����) 
	       �ʻ�(+�ֹ�) ��ʬ��(+�ֹ�) ���ѷ�(+�ֹ�) ���ѷ�(+�ֹ�) 
	       ��̣����
	    */

	    offset = homo_flag ? 2 : 0;
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
		if (sp->Comment) fprintf(stderr, "(%s)\n", sp->Comment);
		return readtoeos(fp);
	    }   
	    m_ptr->type = 0;
	    /* clear_feature(&(m_ptr->f)); 
	       main��ʸ���ȤΥ롼�פ���Ƭ�ǽ����˰�ư */

	    /* Ʊ���۵���ϰ�ö sp->mrph_data �ˤ���� */
	    if (homo_flag) homo_num++;

	    sp->Mrph_num++;
	    m_ptr++;
	}
    }
}

/*==================================================================*/
	    void change_mrph(MRPH_DATA *m_ptr, FEATURE *f)
/*==================================================================*/
{
    char h_buffer[62], b_buffer[62], kata_buffer[62], kei_buffer[62];
    int num;

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
		    int break_feature(FEATURE *fp)
/*==================================================================*/
{
    while (fp) {
	if (!strcmp(fp->cp, "&break:normal")) 
	    return RLOOP_BREAK_NORMAL;
	else if (!strcmp(fp->cp, "&break:jump")) 
	    return RLOOP_BREAK_JUMP;
	else if (!strncmp(fp->cp, "&break", strlen("&break")))
	    return RLOOP_BREAK_NORMAL;
	fp = fp->next;
    }
    return RLOOP_BREAK_NONE;
}

/*==================================================================*/
       void assign_mrph_feature(MrphRule *s_r_ptr, int r_size,
				MRPH_DATA *s_m_ptr, int m_length,
				int mode, int break_mode, int direction)
/*==================================================================*/
{
    /* �����ϰ�(ʸ����,ʸ����ʤ�)���Ф��Ʒ����ǤΥޥå��󥰤�Ԥ� */

    int i, j, k, match_length, feature_break_mode;
    MrphRule *r_ptr;
    MRPH_DATA *m_ptr;

    /* ��������Ŭ�Ѥ�����ϥǡ����Τ�����򤵤��Ƥ���ɬ�פ����� */
    if (direction == RtoL)
	s_m_ptr += m_length-1;
    
    /* MRM
       	1.self_pattern����Ƭ�η����ǰ���
	  2.�롼��
	    3.self_pattern�������η����ǰ���
	�ν�˥롼�פ���� (3�Υ롼�פ�regexpmrphrule_match����)
	
	break_mode == RLOOP_BREAK_NORMAL
	    2�Υ�٥��break����
	break_mode == RLOOP_BREAK_JUMP
	    2�Υ�٥��break����self_patternĹ����1�Υ롼�פ�ʤ��
     */

    if (mode == RLOOP_MRM) {
	for (i = 0; i < m_length; i++) {
	    r_ptr = s_r_ptr;
	    m_ptr = s_m_ptr+(i*direction);
	    for (j = 0; j < r_size; j++, r_ptr++) {
		if ((match_length = 
		     regexpmrphrule_match(r_ptr, m_ptr, 
					  direction == LtoR ? i : m_length-i-1, 
					  direction == LtoR ? m_length-i : i+1)) != -1) {
		    for (k = 0; k < match_length; k++)
			assign_feature(&((s_m_ptr+i*direction+k)->f), 
				       &(r_ptr->f), s_m_ptr+i*direction+k);
		    feature_break_mode = break_feature(r_ptr->f);
		    if (break_mode == RLOOP_BREAK_NORMAL ||
			feature_break_mode == RLOOP_BREAK_NORMAL) {
			break;
		    } else if (break_mode == RLOOP_BREAK_JUMP ||
			       feature_break_mode == RLOOP_BREAK_JUMP) {
			i += match_length - 1;
			break;
		    }
		}
	    }
	}
    }

    /* RMM
       	1.�롼��
	  2.self_pattern����Ƭ�η����ǰ���
	    3.self_pattern�������η����ǰ���
	�ν�˥롼�פ���� (3�Υ롼�פ�regexpmrphrule_match����)
	
	break_mode == RLOOP_BREAK_NORMAL||RLOOP_BREAK_JUMP
	    2�Υ�٥��break���� (�����λȤ����Ϲͤ��ˤ�����)
    */

    else if (mode == RLOOP_RMM) {
	r_ptr = s_r_ptr;
	for (j = 0; j < r_size; j++, r_ptr++) {
	    feature_break_mode = break_feature(r_ptr->f);
	    for (i = 0; i < m_length; i++) {
		m_ptr = s_m_ptr+(i*direction);
		if ((match_length = 
		     regexpmrphrule_match(r_ptr, m_ptr, 
					  direction == LtoR ? i : m_length-i-1, 
					  direction == LtoR ? m_length-i : i+1)) != -1) {
		    for (k = 0; k < match_length; k++)
			assign_feature(&((s_m_ptr+i*direction+k)->f), 
				       &(r_ptr->f), s_m_ptr+i*direction+k);
		    if (break_mode == RLOOP_BREAK_NORMAL ||
			break_mode == RLOOP_BREAK_JUMP ||
			feature_break_mode == RLOOP_BREAK_NORMAL ||
			feature_break_mode == RLOOP_BREAK_JUMP) {
			break;
		    }
		}
	    }
	}
    }
}

/*==================================================================*/
void assign_bnst_feature(BnstRule *s_r_ptr, int r_size,
			 BNST_DATA *s_b_ptr, int b_length,
			 int mode, int break_mode, int direction)
/*==================================================================*/
{
    /* �����ϰ�(ʸ����,ʸ����ʤ�)���Ф���ʸ��Υޥå��󥰤�Ԥ� */

    int i, j, k, match_length, feature_break_mode;
    BnstRule *r_ptr;
    BNST_DATA *b_ptr;

    /* ��������Ŭ�Ѥ�����ϥǡ����Τ�����򤵤��Ƥ���ɬ�פ����� */
    if (direction == RtoL)
	s_b_ptr += b_length-1;
    
    /* MRM
       	1.self_pattern����Ƭ��ʸ�����
	  2.�롼��
	    3.self_pattern��������ʸ�����
	�ν�˥롼�פ���� (3�Υ롼�פ�regexpbnstrule_match����)
	
	break_mode == RLOOP_BREAK_NORMAL
	    2�Υ�٥��break����
	break_mode == RLOOP_BREAK_JUMP
	    2�Υ�٥��break����self_patternĹ����1�Υ롼�פ�ʤ��
     */

    if (mode == RLOOP_MRM) {
	for (i = 0; i < b_length; i++) {
	    r_ptr = s_r_ptr;
	    b_ptr = s_b_ptr+(i*direction);
	    for (j = 0; j < r_size; j++, r_ptr++) {
		if ((match_length = 
		     regexpbnstrule_match(r_ptr, b_ptr, 
					  direction == LtoR ? i : b_length-i-1, 
					  direction == LtoR ? b_length-i : i+1)) != -1) {
		    for (k = 0; k < match_length; k++)
			assign_feature(&((s_b_ptr+i*direction+k)->f), 
				       &(r_ptr->f), s_b_ptr+i*direction+k);
		    feature_break_mode = break_feature(r_ptr->f);
		    if (break_mode == RLOOP_BREAK_NORMAL ||
			feature_break_mode == RLOOP_BREAK_NORMAL) {
			break;
		    } else if (break_mode == RLOOP_BREAK_JUMP ||
			       feature_break_mode == RLOOP_BREAK_JUMP) {
			i += match_length - 1;
			break;
		    }
		}
	    }
	}
    }

    /* RMM
       	1.�롼��
	  2.self_pattern����Ƭ��ʸ�����
	    3.self_pattern��������ʸ�����
	�ν�˥롼�פ���� (3�Υ롼�פ�regexpbnstrule_match����)
	
	break_mode == RLOOP_BREAK_NORMAL||RLOOP_BREAK_JUMP
	    2�Υ�٥��break���� (�����λȤ����Ϲͤ��ˤ�����)
    */

    else if (mode == RLOOP_RMM) {
	r_ptr = s_r_ptr;
	for (j = 0; j < r_size; j++, r_ptr++) {
	    feature_break_mode = break_feature(r_ptr->f);
	    for (i = 0; i < b_length; i++) {
		b_ptr = s_b_ptr+(i*direction);
		if ((match_length = 
		     regexpbnstrule_match(r_ptr, b_ptr, 
					  direction == LtoR ? i : b_length-i-1, 
					  direction == LtoR ? b_length-i : i+1)) != -1) {
		    for (k = 0; k < match_length; k++)
			assign_feature(&((s_b_ptr+i*direction+k)->f), 
				       &(r_ptr->f), s_b_ptr+i*direction+k);
		    if (break_mode == RLOOP_BREAK_NORMAL ||
			break_mode == RLOOP_BREAK_JUMP ||
			feature_break_mode == RLOOP_BREAK_NORMAL ||
			feature_break_mode == RLOOP_BREAK_JUMP) {
			break;
		    }
		}
	    }
	}
    }
}

/*==================================================================*/
     void _assign_general_feature(void *data, int size, int flag)
/*==================================================================*/
{
    int i;
    void (*assign_function)();

    /* �����Ǥ�ʸ�ᤫ�ˤĤ��Ƥξ��ʬ�� */
    if (flag == MorphRuleType) {
	assign_function = assign_mrph_feature;
    }
    else if (flag == BnstRuleType) {
	assign_function = assign_bnst_feature;
    }

    for (i = 0; i < GeneralRuleNum; i++) {
	if ((GeneralRuleArray+i)->type == flag) {
	    assign_function((GeneralRuleArray+i)->RuleArray, 
			    (GeneralRuleArray+i)->CurRuleSize, 
			    data, size, 
			    (GeneralRuleArray+i)->mode, 
			    (GeneralRuleArray+i)->breakmode, 
			    (GeneralRuleArray+i)->direction);
	}
    }
}

/*==================================================================*/
       void assign_general_feature(SENTENCE_DATA *sp, int flag)
/*==================================================================*/
{
    int i, size;
    void (*assign_function)();
    void *data;

    /* �����Ǥ�ʸ�ᤫ�ˤĤ��Ƥξ��ʬ�� */
    if (flag == MorphRuleType) {
	assign_function = assign_mrph_feature;
	data = sp->mrph_data;
	size = sp->Mrph_num;
    }
    else if (flag == BnstRuleType) {
	assign_function = assign_bnst_feature;
	data = sp->bnst_data;
	size = sp->Bnst_num;
    }

    for (i = 0; i < GeneralRuleNum; i++) {
	if ((GeneralRuleArray+i)->type == flag) {
	    assign_function((GeneralRuleArray+i)->RuleArray, 
			    (GeneralRuleArray+i)->CurRuleSize, 
			    data, size, 
			    (GeneralRuleArray+i)->mode, 
			    (GeneralRuleArray+i)->breakmode, 
			    (GeneralRuleArray+i)->direction);
	}
    }
}

/*==================================================================*/
      BNST_DATA *init_bnst(SENTENCE_DATA *sp, MRPH_DATA *m_ptr)
/*==================================================================*/
{
    int i;
    char *cp;
    BNST_DATA *b_ptr;

    b_ptr = sp->bnst_data + sp->Bnst_num;
    b_ptr->num = sp->Bnst_num;
    sp->Bnst_num++;
    if (sp->Bnst_num > BNST_MAX) {
	fprintf(stderr, "Too many bnst (%s %s%s...)!\n", 
		sp->Comment ? sp->Comment : "", sp->mrph_data, sp->mrph_data+1);
	sp->Bnst_num = 0;
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
    b_ptr->voice = 0;

    b_ptr->length = 0;
    b_ptr->space = 0;

    b_ptr->pred_b_ptr = NULL;
    
    for (i = 0, cp = b_ptr->SCASE_code; i < SCASE_CODE_SIZE; i++, cp++) *cp = 0;

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
/*==================================================================*/
{
    if (b_ptr->jiritu_num == 0)
	b_ptr->jiritu_ptr = m_ptr;

    if ((strlen(b_ptr->Jiritu_Go) + strlen(m_ptr->Goi)) >= WORD_LEN_MAX) {
	fprintf(stderr, ";; Too big Jiritu_Go (%s%s...)\n",
		b_ptr->Jiritu_Go, m_ptr->Goi);
    } else {
	strcat(b_ptr->Jiritu_Go, m_ptr->Goi);
    }
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
		 int make_bunsetsu(SENTENCE_DATA *sp)
/*==================================================================*/
{
    int prev_stat, now_stat = -1;
    int i, j;
    MRPH_DATA	*m_ptr;
    BNST_DATA	*b_ptr = NULL;
    
    sp->Bnst_num = 0;
    sp->Max_New_Bnst_num = 0;
    prev_stat = MRPH_SUFX;

    for (i = 0, m_ptr = sp->mrph_data; i < sp->Mrph_num; i++, m_ptr++) {

	if (check_feature(m_ptr->f, "��ʣ��") ||
	    check_feature(m_ptr->f, "��Ω"))
	    now_stat = MRPH_INDP;
	else if (check_feature(m_ptr->f, "��°"))
	    now_stat = MRPH_SUFX;
	else if (check_feature(m_ptr->f, "��Ƭ"))
	    now_stat = MRPH_PRFX;
	else
	    fprintf(Outfp, ";; Invalid input \n");

	/* �� ��Ŭ�ڤ����� (�������ǤϤ��ޤ롤��Ƭ������)
	   ��������Ω��Ȥ��ư��� (����Ū) */

	if (i == 0 && now_stat == MRPH_SUFX) {
	    fprintf(Outfp, 
		    ";; Invalid input (prefix and suffix)\"%s%s ... \"!\n",
		    sp->mrph_data[0].Goi2, sp->mrph_data[1].Goi2);
	    /* return FALSE; */
	    now_stat = MRPH_INDP;
	} else if (prev_stat == MRPH_PRFX &&
		   now_stat == MRPH_SUFX) {
	    fprintf(Outfp, 
		    ";; Invalid input (prefix and suffix)\"... %s%s ... \"!\n",
		    sp->mrph_data[i-1].Goi2, sp->mrph_data[i].Goi2);
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
		if ((b_ptr = init_bnst(sp, m_ptr)) == NULL) return FALSE;
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
		    if ((b_ptr = init_bnst(sp, m_ptr)) == NULL) return FALSE;
		push_indp(b_ptr, m_ptr);
		break;
	    case MRPH_SUFX:
		if ((b_ptr = init_bnst(sp, m_ptr)) == NULL) return FALSE;
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

    for (i = 0, b_ptr = sp->bnst_data; i < sp->Bnst_num; i++, b_ptr++) {
	for (j = 0, m_ptr = b_ptr->mrph_ptr; j < b_ptr->mrph_num;
					     j++, m_ptr++) {
	    if ((b_ptr->length += strlen(m_ptr->Goi2)) >
		BNST_LENGTH_MAX) {
		fprintf(stderr, "Too big bnst (%s %s%s...)!\n", 
			sp->Comment ? sp->Comment : "", b_ptr->mrph_ptr, b_ptr->mrph_ptr+1);
		return FALSE;
	    }
	}
    }
    return TRUE;
}

/*==================================================================*/
	       int make_bunsetsu_pm(SENTENCE_DATA *sp)
/*==================================================================*/
{
    int i, j, prev_stat = MRPH_SUFX;
    char *cp;
    MRPH_DATA	*m_ptr;
    BNST_DATA	*b_ptr = sp->bnst_data;

    for (i = 0, m_ptr = sp->mrph_data; i < sp->Mrph_num; i++, m_ptr++) {
	if (Bnst_start[i]) {
	    if (i != 0) b_ptr++;
	    b_ptr->num = b_ptr-sp->bnst_data;
	    b_ptr->mrph_ptr = m_ptr;
	    b_ptr->mrph_num = 1;
	    b_ptr->jiritu_ptr = NULL;
	    b_ptr->jiritu_num = 0;
	    b_ptr->settou_ptr = NULL;
	    b_ptr->settou_num = 0;
	    b_ptr->fuzoku_ptr = NULL;
	    b_ptr->fuzoku_num = 0;
	    b_ptr->length = 0;
	    b_ptr->cpm_ptr = NULL;
	    b_ptr->voice = 0;
	    b_ptr->pred_b_ptr = NULL;
	    for (j = 0, cp = b_ptr->SCASE_code; j < SCASE_CODE_SIZE; j++, cp++)
		*cp = 0;
	    /* clear_feature(&(b_ptr->f));
	       main��ʸ���ȤΥ롼�פ���Ƭ�ǽ����˰�ư */
	} else {
	    b_ptr->mrph_num ++;
	}

	if (check_feature(m_ptr->f, "��ʣ��") || check_feature(m_ptr->f, "��Ω")) {
	    if (prev_stat == MRPH_SUFX || 
		(prev_stat == MRPH_INDP && !check_feature(m_ptr->f, "��ʣ��")) || !b_ptr->jiritu_ptr) {
		b_ptr->jiritu_ptr = m_ptr;
		b_ptr->jiritu_num = 0;
	    }
	    b_ptr->jiritu_num ++;
	    prev_stat = MRPH_INDP;
	}
	else if (check_feature(m_ptr->f, "��Ƭ")) {
	    if (prev_stat != MRPH_PRFX || !b_ptr->settou_ptr) {
		b_ptr->settou_ptr = m_ptr;
		b_ptr->settou_num = 0;
	    }
	    b_ptr->settou_num ++;
	    prev_stat = MRPH_PRFX;
	}
	else if (check_feature(m_ptr->f, "��°")) {
	    if (!b_ptr->fuzoku_ptr) {
		b_ptr->fuzoku_ptr = m_ptr;
	    }
	    b_ptr->fuzoku_num ++;
	    prev_stat = MRPH_SUFX;
	}
    }
    
    for (i = 0, b_ptr = sp->bnst_data; i < sp->Bnst_num; i++, b_ptr++) {
	
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

	*b_ptr->Jiritu_Go = '\0';
	for (j = 0; j < b_ptr->jiritu_num; j++) {
	    if ((strlen(b_ptr->Jiritu_Go) + strlen((b_ptr->jiritu_ptr+j)->Goi)) >= WORD_LEN_MAX) {
		fprintf(stderr, ";; Too big Jiritu_Go (%s%s...)\n",
			b_ptr->Jiritu_Go, (b_ptr->jiritu_ptr+j)->Goi);
	    } else {
		strcat(b_ptr->Jiritu_Go, (b_ptr->jiritu_ptr+j)->Goi);
	    }
	}
    }
    return TRUE;
}

/*====================================================================
				 END
====================================================================*/
