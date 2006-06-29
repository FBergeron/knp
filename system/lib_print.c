/*====================================================================

			     ���ϥ롼����

                                               S.Kurohashi 91. 6.25
                                               S.Kurohashi 93. 5.31

    $Id$
====================================================================*/
#include "knp.h"

char mrph_buffer[SMALL_DATA_LEN];

/*==================================================================*/
		       char *pp2mrph(char *pp)
/*==================================================================*/
{
    char *hira_pp = katakana2hiragana(pp);
    int hinsi_id = get_hinsi_id("����");

    sprintf(mrph_buffer, "%s %s %s ���� %d �ʽ��� %d * 0 * 0 NIL", 
	    hira_pp, hira_pp, hira_pp, 
	    hinsi_id, 
	    get_bunrui_id("�ʽ���", hinsi_id));

    return mrph_buffer;
}

/*==================================================================*/
	     char pos2symbol(char *hinshi, char *bunrui)
/*==================================================================*/
{
    if (!strcmp(hinshi, "�ü�")) return ' ';
    else if (!strcmp(hinshi, "ư��")) return 'v';
    else if (!strcmp(hinshi, "���ƻ�")) return 'j';
    else if (!strcmp(hinshi, "Ƚ���")) return 'c';
    else if (!strcmp(hinshi, "��ư��")) return 'x';
    else if (!strcmp(hinshi, "̾��") &&
	     !strcmp(bunrui, "��ͭ̾��")) return 'N';
    else if (!strcmp(hinshi, "̾��") &&
	     !strcmp(bunrui, "��̾")) return 'J';
    else if (!strcmp(hinshi, "̾��") &&
	     !strcmp(bunrui, "��̾")) return 'C';
    else if (!strcmp(hinshi, "̾��")) return 'n';
    else if (!strcmp(hinshi, "�ؼ���")) return 'd';
    else if (!strcmp(hinshi, "����")) return 'a';
    else if (!strcmp(hinshi, "����")) return 'p';
    else if (!strcmp(hinshi, "��³��")) return 'c';
    else if (!strcmp(hinshi, "Ϣ�λ�")) return 'm';
    else if (!strcmp(hinshi, "��ư��")) return '!';
    else if (!strcmp(hinshi, "��Ƭ��")) return 'p';
    else if (!strcmp(hinshi, "������")) return 's';

    return '?';
}

/*==================================================================*/
		  void print_mrph(MRPH_DATA *m_ptr)
/*==================================================================*/
{
    fprintf(Outfp, "%s %s %s ", m_ptr->Goi2, m_ptr->Yomi, m_ptr->Goi);

    if (m_ptr->Hinshi >= CLASS_num) {
	fprintf(Outfp, "\n;; Hinshi number is invalid. (%d)\n", m_ptr->Hinshi);
	exit(1);
    }
    fprintf(Outfp, "%s ", Class[m_ptr->Hinshi][0].id);
    fprintf(Outfp, "%d ", m_ptr->Hinshi);
	
    if (m_ptr->Bunrui) 
	fprintf(Outfp, "%s ", Class[m_ptr->Hinshi][m_ptr->Bunrui].id);
    else
	fprintf(Outfp, "* ");
    fprintf(Outfp, "%d ", m_ptr->Bunrui);
	
    if (m_ptr->Katuyou_Kata) 
	fprintf(Outfp, "%s ", Type[m_ptr->Katuyou_Kata].name);
    else                    
	fprintf(Outfp, "* ");
    fprintf(Outfp, "%d ", m_ptr->Katuyou_Kata);
    
    if (m_ptr->Katuyou_Kei) 
	fprintf(Outfp, "%s ", 
		Form[m_ptr->Katuyou_Kata][m_ptr->Katuyou_Kei].name);
    else 
	fprintf(Outfp, "* ");
    fprintf(Outfp, "%d ", m_ptr->Katuyou_Kei);
    
    fprintf(Outfp, "%s", m_ptr->Imi);
}

/*==================================================================*/
		  void print_mrph_f(MRPH_DATA *m_ptr)
/*==================================================================*/
{
    char yomi_buffer[SMALL_DATA_LEN];

    sprintf(yomi_buffer, "(%s)", m_ptr->Yomi);
    fprintf(Outfp, "%-16.16s%-18.18s %-14.14s",
	    m_ptr->Goi2, yomi_buffer, 
	    Class[m_ptr->Hinshi][m_ptr->Bunrui].id);
    if (m_ptr->Katuyou_Kata)
	fprintf(Outfp, " %-14.14s %-12.12s",
		Type[m_ptr->Katuyou_Kata].name,
		Form[m_ptr->Katuyou_Kata][m_ptr->Katuyou_Kei].name);
}

/*==================================================================*/
	     void print_tags(SENTENCE_DATA *sp, int flag)
/*==================================================================*/
{
    /* ���ߤϾ�� flag == 1 (0�ϵ��������) */

    int		i, j, count = 0, b_count = 0, t_table[TAG_MAX], b_table[BNST_MAX];
    char	*cp, cc_feature[SMALL_DATA_LEN];
    FEATURE	*fp;
    BNST_DATA	*pre_bp = NULL;
    MRPH_DATA	*m_ptr;
    TAG_DATA	*t_ptr;

    /* �Ρ��ɤ��������θ���ʤ��顢���ܶ硢ʸ����Ѵ��ơ��֥����� */
    for (i = 0, t_ptr = sp->tag_data; i < sp->Tag_num; i++, t_ptr++) {
	if (t_ptr->num == -1) {
	    continue; /* ������ǥޡ������줿���� */
	}

	/* �ɲåΡ��� */
	if (OptRecoverPerson && pre_bp != t_ptr->b_ptr) { /* ʸ����ڤ��ܤ��Ȥ˥����å� */
	    fp = (t_ptr->b_ptr->tag_ptr + t_ptr->b_ptr->tag_num - 1)->f; /* head�δ��ܶ� */
	    while (fp) {
		if (!strncmp(fp->cp, "�Ծ�ά�;�-", 11)) { /* tag_after_dpnd_and_case.rule���������Ƥ��� */
		    count++;
		    b_count++;
    		}
		fp = fp->next;
	    }
	    pre_bp = t_ptr->b_ptr;
	}

	t_table[i] = count++;

	if (t_ptr->bnum >= 0) { /* ʸ��� */
	    b_table[t_ptr->bnum] = b_count++;
	}
    }

    pre_bp = NULL;
    for (i = 0, t_ptr = sp->tag_data; i < sp->Tag_num; i++, t_ptr++) {
	if (t_ptr->num == -1) {
	    continue; /* ������ǥޡ������줿���� */
	}
	if (flag == 1) {
	    /* �ɲåΡ��� */
	    if (OptRecoverPerson && pre_bp != t_ptr->b_ptr) {
		fp = (t_ptr->b_ptr->tag_ptr + t_ptr->b_ptr->tag_num - 1)->f; /* head�δ��ܶ� */
		while (fp) {
		    if (!strncmp(fp->cp, "�Ծ�ά�;�-", 11)) {
			fprintf(Outfp, "* %dD <�Ρ�������><��:%s��>\n", b_table[t_ptr->b_ptr->num], fp->cp + 11);
			fprintf(Outfp, "+ %dD <�Ρ�������><��:%s��><���ϳ�:%s>\n", 
				t_table[(t_ptr->b_ptr->tag_ptr + t_ptr->b_ptr->tag_num - 1)->num], fp->cp + 11, fp->cp + 11);
			sprintf(cc_feature, "������-%s", fp->cp + 11); /* ��Ȥ���� */
			cp = check_feature((t_ptr->b_ptr->tag_ptr + t_ptr->b_ptr->tag_num - 1)->f, cc_feature);
			cp += strlen(cc_feature) + 1;
			if (!strcmp(cp, "NIL")) {
			    fprintf(stderr, ";;; tag_after_dpnd_and_case.rule is incoherent.\n");
			    exit(1);
			}
			fprintf(Outfp, "%s %s %s ̾�� 6 ����̾�� 1 * 0 * 0 NIL\n", cp, cp, cp);
			fprintf(Outfp, "%s\n", pp2mrph(fp->cp + 11));
		    }
		    fp = fp->next;
		}
		pre_bp = t_ptr->b_ptr;
	    }

	    /* ʸ��� */
	    if (t_ptr->bnum >= 0) {
		fprintf(Outfp, "* %d%c", 
			(sp->bnst_data + t_ptr->bnum)->dpnd_head == -1 ? -1 : b_table[(sp->bnst_data + t_ptr->bnum)->dpnd_head], 
			(sp->bnst_data + t_ptr->bnum)->dpnd_type);
		if ((sp->bnst_data + t_ptr->bnum)->f) {
		    fputc(' ', Outfp);
		    print_feature((sp->bnst_data + t_ptr->bnum)->f, Outfp);
		}
		fputc('\n', Outfp);
	    }

	    fprintf(Outfp, "+ %d%c", t_ptr->dpnd_head == -1 ? -1 : t_table[t_ptr->dpnd_head], t_ptr->dpnd_type);
	    if (t_ptr->f) {
		fputc(' ', Outfp);
		print_feature(t_ptr->f, Outfp);
	    }
	    fputc('\n', Outfp);
	}
	else {
	    fprintf(Outfp, "%c\n", t_ptr->bnum < 0 ? '+' : '*');
	}

	for (j = 0, m_ptr = t_ptr->mrph_ptr; j < t_ptr->mrph_num; j++, m_ptr++) {
	    print_mrph(m_ptr);
	    if (m_ptr->f) {
		fputc(' ', Outfp);
		print_feature(m_ptr->f, Outfp);
	    }
	    fputc('\n', Outfp);
	}
    }
    fputs("EOS\n", Outfp);
}

/*==================================================================*/
	    void print_mrphs(SENTENCE_DATA *sp, int flag)
/*==================================================================*/
{
    int		i, j;
    char *cp;
    MRPH_DATA	*m_ptr;
    BNST_DATA	*b_ptr;

    for (i = 0, b_ptr = sp->bnst_data; i < sp->Bnst_num; i++, b_ptr++) {
	if (b_ptr->num == -1) {
	    continue; /* ������ǥޡ������줿ʸ�� */
	}
	if (flag == 1) {
	    fprintf(Outfp, "* %d%c", b_ptr->dpnd_head, b_ptr->dpnd_type);
	    if (b_ptr->f) {
		fprintf(Outfp, " ");
		print_feature(b_ptr->f, Outfp);
	    }
	    fprintf(Outfp, "\n");
	}
	else {
	    fprintf(Outfp, "*\n");
	}

	for (j = 0, m_ptr = b_ptr->mrph_ptr; j < b_ptr->mrph_num; j++, m_ptr++) {
	    print_mrph(m_ptr);
	    if (m_ptr->f) {
		fprintf(Outfp, " ");
		print_feature(m_ptr->f, Outfp);
	    }
	    /* print_mrph_f(m_ptr); */
	    fprintf(Outfp, "\n");
	}
    }
    fprintf(Outfp, "EOS\n");
}

/*==================================================================*/
		   void _print_bnst(TAG_DATA *ptr)
/*==================================================================*/
{
    int i;

    if (ptr) {
	for (i = 0; i < ptr->mrph_num; i++)
	    fprintf(Outfp, "%s", (ptr->mrph_ptr + i)->Goi2);
    }
    else {
	fprintf(Outfp, "������:��");
    }
}

/*==================================================================*/
		void print_bnst(BNST_DATA *ptr, char *cp)
/*==================================================================*/
{
    int i;

    if (cp && ptr) {
	if ( ptr->para_type == PARA_NORMAL ) strcpy(cp, "<P>");
	else if ( ptr->para_type == PARA_INCOMP ) strcpy(cp, "<I>");
	else			     cp[0] = '\0';
	if ( ptr->para_top_p == TRUE )
	  strcat(cp, "PARA");
	else {
	    strcpy(cp, ptr->mrph_ptr->Goi2);
	    for (i = 1; i < ptr->mrph_num; i++) 
	      strcat(cp, (ptr->mrph_ptr + i)->Goi2);
	}
    } else if (cp == NULL && ptr) {
	if ( ptr->para_top_p == TRUE ) {
	    fprintf(Outfp, "PARA");
	} else {
	    for (i = 0; i < ptr->mrph_num; i++) {
		if (OptDisplay == OPT_NORMAL) {
		    fprintf(Outfp, "%s", (ptr->mrph_ptr + i)->Goi2);
		} else { 
		    fprintf(Outfp, "%s%c", (ptr->mrph_ptr + i)->Goi2, 
			    pos2symbol(Class[(ptr->mrph_ptr + i)->Hinshi]
				       [0].id,
				       Class[(ptr->mrph_ptr + i)->Hinshi]
				       [(ptr->mrph_ptr + i)->Bunrui].id));
		}
	    }
	}

	if ( ptr->para_type == PARA_NORMAL ) fprintf(Outfp, "<P>");
	else if ( ptr->para_type == PARA_INCOMP ) fprintf(Outfp, "<I>");
	if ( ptr->to_para_p == TRUE ) fprintf(Outfp, "(D)");
    }
}

/*==================================================================*/
  void print_data2ipal_corr(BNST_DATA *b_ptr, CF_PRED_MGR *cpm_ptr)
/*==================================================================*/
{
    int i, j, elem_num = 0;
    int offset;
    int flag;

    switch (cpm_ptr->cmm[0].cf_ptr->voice) {
      case FRAME_PASSIVE_I:
      case FRAME_CAUSATIVE_WO_NI:
      case FRAME_CAUSATIVE_WO:
      case FRAME_CAUSATIVE_NI:
	offset = 0;
	break;
      default:
	offset = 1;
	break;
    }

    flag = FALSE;
    if (cpm_ptr->elem_b_num[0] == -1) {
	elem_num = 0;
	flag = TRUE;
    }
    if (flag == TRUE) {
	flag = FALSE;
	for (j = 0; j < cpm_ptr->cmm[0].cf_ptr->element_num; j++)
	  if (cpm_ptr->cmm[0].result_lists_p[0].flag[j] == elem_num) {
	      fprintf(Outfp, " N%d", offset + j);
	      flag = TRUE;
	  }
    }
    if (flag == FALSE)
      fprintf(Outfp, " *");

    for (i = 0; b_ptr->child[i]; i++) {
	flag = FALSE;
	for (j = 0; j < cpm_ptr->cf.element_num; j++)
	  if (cpm_ptr->elem_b_num[j] == i) {
	      elem_num = j;
	      flag = TRUE;
	      break;
	  }
	if (flag == TRUE) {
	    flag = FALSE;
	    for (j = 0; j < cpm_ptr->cmm[0].cf_ptr->element_num; j++)
	      if (cpm_ptr->cmm[0].result_lists_p[0].flag[j] == elem_num) {
		  fprintf(Outfp, " N%d", offset + j);
		  flag = TRUE;
	      }
	}
	if (flag == FALSE)
	  fprintf(Outfp, " *");
    }
}

/*==================================================================*/
		void print_bnst_detail(BNST_DATA *ptr)
/*==================================================================*/
{
    int i;
    MRPH_DATA *m_ptr;
     
    fputc('(', Outfp);	/* ʸ��Ϥ� */

    if ( ptr->para_top_p == TRUE ) {
	if (ptr->child[1] && 
	    ptr->child[1]->para_key_type == PARA_KEY_N)
	  fprintf(Outfp, "noun_para"); 
	else
	  fprintf(Outfp, "pred_para");
    }
    else {
	fprintf(Outfp, "%d ", ptr->num);

	/* ������������ɽ�� (�ɲ�:97/10/29) */

	fprintf(Outfp, "(type:%c) ", ptr->dpnd_type);

	fputc('(', Outfp);
	for (i=0, m_ptr=ptr->mrph_ptr; i < ptr->mrph_num; i++, m_ptr++) {
	    fputc('(', Outfp);
	    print_mrph(m_ptr);
	    fprintf(Outfp, " ");
	    print_feature2(m_ptr->f, Outfp);
	    fputc(')', Outfp);
	}
	fputc(')', Outfp);

	fprintf(Outfp, " ");
	print_feature2(ptr->f, Outfp);

	if (OptAnalysis == OPT_DPND ||
	    !check_feature(ptr->f, "�Ѹ�") ||	/* �Ѹ��Ǥʤ���� */
	    ptr->cpm_ptr == NULL) { 		/* ������ */
	    fprintf(Outfp, " NIL");
	}
	else {
	    fprintf(Outfp, " (");
	    
	    if (ptr->cpm_ptr->cmm[0].cf_ptr == NULL)
		fprintf(Outfp, "-2");	/* �ʥե졼���ENTRY�ʤ� */
	    else if ((ptr->cpm_ptr->cmm[0].cf_ptr)->cf_address == -1)
		fprintf(Outfp, "-1");	/* �����Ǥʤ� */
	    else {
		fprintf(Outfp, "%s", 
			(ptr->cpm_ptr->cmm[0].cf_ptr)->cf_id);
		switch (ptr->cpm_ptr->cmm[0].cf_ptr->voice) {
		case FRAME_ACTIVE:
		    fprintf(Outfp, " ǽư"); break;
		case FRAME_PASSIVE_I:
		    fprintf(Outfp, " �ּ�"); break;
		case FRAME_PASSIVE_1:
		    fprintf(Outfp, " ľ����"); break;
		case FRAME_PASSIVE_2:
		    fprintf(Outfp, " ľ����"); break;
		case FRAME_CAUSATIVE_WO_NI:
		    fprintf(Outfp, " ������"); break;
		case FRAME_CAUSATIVE_WO:
		    fprintf(Outfp, " �����"); break;
		case FRAME_CAUSATIVE_NI:
		    fprintf(Outfp, " �����"); break;
		case FRAME_CAUSATIVE_PASSIVE:
		    fprintf(Outfp, " ����&����"); break;
		case FRAME_POSSIBLE:
		    fprintf(Outfp, " ��ǽ"); break;
		case FRAME_POLITE:
		    fprintf(Outfp, " º��"); break;
		case FRAME_SPONTANE:
		    fprintf(Outfp, " ��ȯ"); break;
		default: break;
		}
		fprintf(Outfp, " (");
		print_data2ipal_corr(ptr, ptr->cpm_ptr);
		fprintf(Outfp, ")");
	    }
	    fprintf(Outfp, ")");

	    /* ------------�ѹ�:�Ҹ���, �ʷ��������-----------------
	    if (ptr->cpm_ptr != NULL &&
		ptr->cpm_ptr->cmm[0].cf_ptr != NULL &&
		(ptr->cpm_ptr->cmm[0].cf_ptr)->cf_address != -1) {
		get_ipal_frame(i_ptr, 
			       (ptr->cpm_ptr->cmm[0].cf_ptr)->cf_address);
		if (i_ptr->DATA[i_ptr->jyutugoso]) {
		    fprintf(Outfp, " �Ҹ��� %s", 
			    i_ptr->DATA+i_ptr->jyutugoso);
		} else {
		    fprintf(Outfp, " �Ҹ��� nil");
		}
		fprintf(Outfp, " �ʷ��� (");
		for (j=0; *((i_ptr->DATA)+(i_ptr->kaku_keishiki[j])) 
			       != NULL; j++){
		    fprintf(Outfp, " %s", 
			    i_ptr->DATA+i_ptr->kaku_keishiki[j]);
		}
		fprintf(Outfp, ")");
	    }
	    ------------------------------------------------------- */
	}
    }
    fputc(')', Outfp);	/* ʸ�Ὢ��� */
}

/*==================================================================*/
	     void print_sentence_slim(SENTENCE_DATA *sp)
/*==================================================================*/
{
    int i;

    init_bnst_tree_property(sp);

    fputc('(', Outfp);
    for ( i=0; i<sp->Bnst_num; i++ )
	print_bnst(&(sp->bnst_data[i]), NULL);
    fputc(')', Outfp);
    fputc('\n', Outfp);
}

/*====================================================================
			       ����ɽ��
====================================================================*/

/*==================================================================*/
void print_M_bnst(SENTENCE_DATA *sp, int b_num, int max_length, int *para_char)
/*==================================================================*/
{
    BNST_DATA *ptr = &(sp->bnst_data[b_num]);
    int i, len, space, comma_p;
    char tmp[BNST_LENGTH_MAX], *cp = tmp;

    if ( ptr->mrph_num == 1 ) {
	strcpy(tmp, ptr->mrph_ptr->Goi2);
	comma_p = FALSE;
    } else {
	strcpy(tmp, ptr->mrph_ptr->Goi2);
	for (i = 1; i < (ptr->mrph_num - 1); i++) 
	  strcat(tmp, (ptr->mrph_ptr + i)->Goi2);

	if (!strcmp(Class[(ptr->mrph_ptr + ptr->mrph_num - 1)->Hinshi][0].id,
		    "�ü�") &&
	    !strcmp(Class[(ptr->mrph_ptr + ptr->mrph_num - 1)->Hinshi]
		    [(ptr->mrph_ptr + ptr->mrph_num - 1)->Bunrui].id, 
		    "����")) {
	    strcat(tmp, ",");
	    comma_p = TRUE;
	} else {
	    strcat(tmp, (ptr->mrph_ptr + ptr->mrph_num - 1)->Goi2);
	    comma_p = FALSE;	      
	}
    }

    space = ptr->para_key_type ?
      max_length-(sp->Bnst_num-b_num-1)*3-2 : max_length-(sp->Bnst_num-b_num-1)*3;
    len = comma_p ? 
      ptr->length - 1 : ptr->length;

    if ( len > space ) {
	if ( (space%2) != (len%2) ) {
	    cp += len + 1 - space;
	    fputc(' ', Outfp);
	} else
	  cp += len - space;
    } else
      for ( i=0; i<space-len; i++ ) fputc(' ', Outfp);

    if ( ptr->para_key_type ) {
	fprintf(Outfp, "%c>", 'a'+ (*para_char));
	(*para_char)++;
    }
    fprintf(Outfp, "%s", cp);
}

/*==================================================================*/
		void print_line(int length, int flag)
/*==================================================================*/
{
    int i;
    for ( i=0; i<(length-1); i++ ) fputc('-', Outfp);
    flag ? fputc(')', Outfp) : fputc('-', Outfp);
    fputc('\n', Outfp);
}

/*==================================================================*/
     void print_matrix(SENTENCE_DATA *sp, int type, int key_pos)
/*==================================================================*/
{
    int i, j, length;
    int over_flag = 0;
    int max_length = 0;
    int para_char = 0;     /* para_key ��ɽ���� */
    PARA_DATA *ptr;

    for ( i=0; i<sp->Bnst_num; i++ )
	for ( j=0; j<sp->Bnst_num; j++ )
	    path_matrix[i][j] = 0;
    
    /* �ѥ��Υޡ����դ�(PARA) */

    if (type == PRINT_PARA) {
	for ( i=0; i<sp->Para_num; i++ ) {
	    ptr = &sp->para_data[i];

	    if (ptr->max_score < 0.0) continue;
	    /* status��x�Ǥ⥹����������л��ͤΤ���ɽ�� */

	    for ( j=ptr->key_pos+1; j<=ptr->jend_pos; j++ )
		path_matrix[ptr->max_path[j-ptr->key_pos-1]][j] =
		    path_matrix[ptr->max_path[j-ptr->key_pos-1]][j] ?
		    -1 : 'a' + i;
	}
    }

    /* Ĺ���η׻� */

    for ( i=0; i<sp->Bnst_num; i++ ) {
	length = sp->bnst_data[i].length + (sp->Bnst_num-i-1)*3;
        if ( sp->bnst_data[i].para_key_type ) length += 2;
	if ( max_length < length )    max_length = length;
    }
    
    /* �����Ѥν��� */

    if ( 0 ) {
	if ( PRINT_WIDTH < sp->Bnst_num*3 ) {
	    over_flag = 1;
	    sp->Bnst_num = PRINT_WIDTH/3;
	    max_length = PRINT_WIDTH;
	} else if ( PRINT_WIDTH < max_length ) {
	    max_length = PRINT_WIDTH;
	}
    }

    if (type == PRINT_PARA)
	fprintf(Outfp, "<< PARA MATRIX >>\n");
    else if (type == PRINT_DPND)
	fprintf(Outfp, "<< DPND MATRIX >>\n");
    else if (type == PRINT_MASK)
	fprintf(Outfp, "<< MASK MATRIX >>\n");
    else if (type == PRINT_QUOTE)
	fprintf(Outfp, "<< QUOTE MATRIX >>\n");
    else if (type == PRINT_RSTR)
	fprintf(Outfp, "<< RESTRICT MATRIX for PARA RELATION>>\n");
    else if (type == PRINT_RSTD)
	fprintf(Outfp, "<< RESTRICT MATRIX for DEPENDENCY STRUCTURE>>\n");
    else if (type == PRINT_RSTQ)
	fprintf(Outfp, "<< RESTRICT MATRIX for QUOTE SCOPE>>\n");

    print_line(max_length, over_flag);
    for ( i=0; i<(max_length-sp->Bnst_num*3); i++ ) fputc(' ', Outfp);
    for ( i=0; i<sp->Bnst_num; i++ ) fprintf(Outfp, "%2d ", i);
    fputc('\n', Outfp);
    print_line(max_length, over_flag);

    for ( i=0; i<sp->Bnst_num; i++ ) {
	print_M_bnst(sp, i, max_length, &para_char);
	for ( j=i+1; j<sp->Bnst_num; j++ ) {

	    if (type == PRINT_PARA) {
		fprintf(Outfp, "%2d", match_matrix[i][j]);
	    } else if (type == PRINT_DPND) {
		if (Dpnd_matrix[i][j] == 0)
		    fprintf(Outfp, " -");
		else
		    fprintf(Outfp, " %c", (char)Dpnd_matrix[i][j]);
		
	    } else if (type == PRINT_MASK) {
		fprintf(Outfp, "%2d", Mask_matrix[i][j]);

	    } else if (type == PRINT_QUOTE) {
		fprintf(Outfp, "%2d", Quote_matrix[i][j]);

	    } else if (type == PRINT_RSTR || 
		       type == PRINT_RSTD ||
		       type == PRINT_RSTQ) {
		if (j <= key_pos) 
		    fprintf(Outfp, "--");
		else if (key_pos < i)
		    fprintf(Outfp, " |");
		else
		    fprintf(Outfp, "%2d", restrict_matrix[i][j]);
	    }

	    switch(path_matrix[i][j]) {
	      case  0:	fputc(' ', Outfp); break;
	      case -1:	fputc('*', Outfp); break;
	      default:	fputc(path_matrix[i][j], Outfp); break;
	    }
	}
	fputc('\n', Outfp);
    }

    print_line(max_length, over_flag);
    
    if (type == PRINT_PARA) {
	for (i = 0; i < sp->Para_num; i++) {
	    fprintf(Outfp, "%c(%c):%4.1f(%4.1f) ", 
		    sp->para_data[i].para_char, 
		    sp->para_data[i].status, 
		    sp->para_data[i].max_score,
		    sp->para_data[i].pure_score);
	}
	fputc('\n', Outfp);
    }
}

/*==================================================================*/
	void assign_para_similarity_feature(SENTENCE_DATA *sp)
/*==================================================================*/
{
    int i;
    char buffer[DATA_LEN];

    for (i = 0; i < sp->Para_num; i++) {
	sprintf(buffer, "���������:%.3f", sp->para_data[i].max_score);
	assign_cfeature(&(sp->bnst_data[sp->para_data[i].key_pos].f), buffer, FALSE);
    }
}

/*====================================================================
	                ����¤�֤δط�ɽ��
====================================================================*/

/*==================================================================*/
void print_para_manager(SENTENCE_DATA *sp, PARA_MANAGER *m_ptr, int level)
/*==================================================================*/
{
    int i;
    
    for (i = 0; i < level * 5; i++)
	fputc(' ', Outfp);

    for (i = 0; i < m_ptr->para_num; i++)
	fprintf(Outfp, " %c", sp->para_data[m_ptr->para_data_num[i]].para_char);
    fputc(':', Outfp);

    for (i = 0; i < m_ptr->part_num; i++) {
	if (m_ptr->start[i] == m_ptr->end[i]) {
	    fputc('(', Outfp);
	    print_bnst(&sp->bnst_data[m_ptr->start[i]], NULL);
	    fputc(')', Outfp);
	} else {
	    fputc('(', Outfp);
	    print_bnst(&sp->bnst_data[m_ptr->start[i]], NULL);
	    fputc('-', Outfp);
	    print_bnst(&sp->bnst_data[m_ptr->end[i]], NULL);
	    fputc(')', Outfp);
	}
    }
    fputc('\n', Outfp);

    for (i = 0; i < m_ptr->child_num; i++)
	print_para_manager(sp, m_ptr->child[i], level+1);
}

/*==================================================================*/
	     void print_para_relation(SENTENCE_DATA *sp)
/*==================================================================*/
{
    int i;
    
    for (i = 0; i < sp->Para_M_num; i++)
	if (sp->para_manager[i].parent == NULL)
	    print_para_manager(sp, &sp->para_manager[i], 0);
}

/*====================================================================
	                �ڹ�¤ɽ��(from JK)
====================================================================*/
static int max_width;			/* �ڤκ����� */

/*==================================================================*/
			   int mylog(int n)
/*==================================================================*/
{
    int i, num = 1;
    for (i=0; i<n; i++)
	num = num*2;
    return(num);
}

/*==================================================================*/
       void calc_self_space(BNST_DATA *ptr, int depth2)
/*==================================================================*/
{
    if (ptr->para_top_p == TRUE) 
	ptr->space = 4;
    else if (OptDisplay == OPT_NORMAL)
	ptr->space = ptr->length;
    else
	ptr->space = ptr->length + ptr->mrph_num; /* *4 */

    if (ptr->para_type == PARA_NORMAL || 
	ptr->para_type == PARA_INCOMP ||
	ptr->to_para_p == TRUE)
	ptr->space += 1;
    ptr->space += (depth2-1)*8;
}

/*==================================================================*/
       void calc_tree_width(BNST_DATA *ptr, int depth2)
/*==================================================================*/
{
    int i;
    
    calc_self_space(ptr, depth2);

    if ( ptr->space > max_width )
	max_width = ptr->space;

    if ( ptr->child[0] )
	for ( i=0; ptr->child[i]; i++ )
	    calc_tree_width(ptr->child[i], depth2+1);
}

/*==================================================================*/
void show_link(int depth, char *ans_flag, char para_type, char to_para_p)
/*==================================================================*/
{
    int i;
    
    if (depth != 1) {

	/* �Ƥؤλ� (������θ) */

	if (para_type == PARA_NORMAL || 
	    para_type == PARA_INCOMP ||
	    to_para_p == TRUE)
	    fprintf(Outfp, "��");
	else 
	    fprintf(Outfp, "����");

	if (ans_flag[depth-1] == '1') 
	    fprintf(Outfp, "��");
	else 
	    fprintf(Outfp, "��");

	fprintf(Outfp, "��");

	/* ����η���λ� */

	for (i = depth - 1; i > 1; i--) {
	    fprintf(Outfp, "����");
	    if (ans_flag[i-1] == '1') 
		fprintf(Outfp, "��");
	    else 
		fprintf(Outfp, "��");
	    fprintf(Outfp, "��");
	}
    }
}

/*==================================================================*/
 void show_self(BNST_DATA *ptr, int depth, char *ans_flag_p, int flag)
/*==================================================================*/
{
    /* 
       depth �ϼ�ʬ�ο���(����1)

       ans_flag �ϼ�ʬ�����褬�Ǹ�λҤ��ɤ���������
       ����n������(�ޤ��ϼ�ʬ)���Ǹ�λҤǤ���� ans_flag[n-1] �� '0'
       �����Ǥʤ���� '1'(���ξ��ޤ����褬ɬ��)
    */

    int i, j;
    char ans_flag[BNST_MAX];

    if (ans_flag_p) {
	strncpy(ans_flag, ans_flag_p, BNST_MAX);
    } else {
	ans_flag[0] = '0';	/* �ǽ�˸ƤФ��Ȥ� */
    }

    if (ptr->child[0]) {
	for (i = 0; ptr->child[i]; i++);

	/* �Ǹ�λҤ� ans_flag �� 0 �� */ 
	ans_flag[depth] = '0';
	show_self(ptr->child[i-1], depth+1, ans_flag, 0);

	if (i > 1) {
	    /* ¾�λҤ� ans_flag �� 1 �� */ 
	    ans_flag[depth] = '1';
	    for (j = i - 2; j > 0; j--) {
		show_self(ptr->child[j], depth+1, ans_flag, 0);
	    }

	    /* flag: 1: ��PARA 2: -<P>PARA */
	    if (ptr->para_top_p == TRUE && 
		ptr->para_type == PARA_NIL &&
		ptr->to_para_p == FALSE) {
		show_self(ptr->child[0], depth+1, ans_flag, 1);
	    } else if (ptr->para_top_p == TRUE) {
		show_self(ptr->child[0], depth+1, ans_flag, 2);
	    } else {
		show_self(ptr->child[0], depth+1, ans_flag, 0);
	    }
	}
    }

    calc_self_space(ptr, depth);
    if ( ptr->para_top_p != TRUE ) {
	for (i = 0; i < max_width - ptr->space; i++) 
	  fputc(' ', Outfp);
    }
    print_bnst(ptr, NULL);
    
    if (flag == 0) {
	show_link(depth, ans_flag, ptr->para_type, ptr->to_para_p);
	if (OptExpress == OPT_TREEF) {
	    print_some_feature(ptr->f, Outfp);
	}
	fputc('\n', Outfp);
    } else if ( flag == 1 ) {
	fprintf(Outfp, "��");
    } else if ( flag == 2 ) {
	fprintf(Outfp, "-");
    }
}

/*==================================================================*/
	 void show_sexp(BNST_DATA *ptr, int depth, int pars)
/*==================================================================*/
{
    int i;

    for (i = 0; i < depth; i++) fputc(' ', Outfp);
    fprintf(Outfp, "(");

    if ( ptr->para_top_p == TRUE ) {
	if (ptr->child[1] && 
	    ptr->child[1]->para_key_type == PARA_KEY_N)
	  fprintf(Outfp, "(noun_para"); 
	else
	  fprintf(Outfp, "(pred_para");

	if (ptr->child[0]) {
	    fputc('\n', Outfp);
	    i = 0;
	    while (ptr->child[i+1] && ptr->child[i+1]->para_type != PARA_NIL) {
		/* <P>�κǸ�ʳ� */
		/* UCHI fputc(',', Outfp); */
		show_sexp(ptr->child[i], depth + 3, 0);	i ++;
	    }
	    if (ptr->child[i+1]) { /* ����¾�������� */
		/* <P>�κǸ� */
		/* UCHI fputc(',', Outfp); */
		show_sexp(ptr->child[i], depth + 3, 1);	i ++;
		/* ����¾�κǸ�ʳ� */
		while (ptr->child[i+1]) {
		    /* UCHI fputc(',', Outfp); */
		    show_sexp(ptr->child[i], depth + 3, 0); i ++;
		}
		/* ����¾�κǸ� */
		/* UCHI fputc(',', Outfp); */
		show_sexp(ptr->child[i], depth + 3, pars + 1);
	    }
	    else {
		/* <P>�κǸ� */
		/* UCHI fputc(',', Outfp); */
		show_sexp(ptr->child[i], depth + 3, pars + 1 + 1);
	    }
	}
    }
    else {
	
	print_bnst_detail(ptr);

	if (ptr->child[0]) {
	    fputc('\n', Outfp);
	    for ( i=0; ptr->child[i+1]; i++ ) {
		/* UCHI fputc(',', Outfp); */
		show_sexp(ptr->child[i], depth + 3, 0);
	    }
	    /* UCHI fputc(',', Outfp); */
	    show_sexp(ptr->child[i], depth + 3, pars + 1);
	} else {
	    for (i = 0; i < pars + 1; i++) fputc(')', Outfp);
	    fputc('\n', Outfp);
	}
    }
}

/*==================================================================*/
	    void print_kakari(SENTENCE_DATA *sp, int type)
/*==================================================================*/
{
    /* ��¸��¤�ڤ�ɽ�� */

    if (type == OPT_SEXP) {
	show_sexp((sp->bnst_data + sp->Bnst_num -1), 0, 0);
    }
    /* ʸ���tree�������Ȥ� */
    else if (type & OPT_NOTAG) {
	max_width = 0;

	calc_tree_width((sp->bnst_data + sp->Bnst_num -1), 1);
	show_self((sp->bnst_data + sp->Bnst_num -1), 1, NULL, 0);
    }
    /* tagñ�̤�tree�������Ȥ� */
    else {
	max_width = 0;

	calc_tree_width((BNST_DATA *)(sp->tag_data + sp->Tag_num -1), 1);
	show_self((BNST_DATA *)(sp->tag_data + sp->Tag_num -1), 1, NULL, 0);
    }

    fprintf(Outfp, "EOS\n");
}


/*====================================================================
			      �����å���
====================================================================*/

/*==================================================================*/
		  void check_bnst(SENTENCE_DATA *sp)
/*==================================================================*/
{
    int 	i, j;
    BNST_DATA 	*ptr;
    char b_buffer[BNST_LENGTH_MAX];

    for (i = 0; i < sp->Bnst_num; i++) {
	ptr = &sp->bnst_data[i];
	
	b_buffer[0] = '\0';

	for (j = 0; j < ptr->mrph_num; j++) {
	    /* buffer overflow */
	    if (strlen(b_buffer) + strlen((ptr->mrph_ptr + j)->Goi2) + 4 > BNST_LENGTH_MAX) {
		break;
	    }

	    if (ptr->mrph_ptr + j == ptr->head_ptr) {
		strcat(b_buffer, "[");
		strcat(b_buffer, (ptr->mrph_ptr + j)->Goi2);
		strcat(b_buffer, "]");
	    }
	    else {
		strcat(b_buffer, (ptr->mrph_ptr + j)->Goi2);
	    }
	    strcat(b_buffer, " ");
	}
	fprintf(Outfp, "%-20s", b_buffer);

	print_feature(ptr->f, Outfp);

	if (check_feature(ptr->f, "�Ѹ�") ||
	    check_feature(ptr->f, "���Ѹ�")) {

	    fprintf(Outfp, " <ɽ�س�:");
	    if (ptr->SCASE_code[case2num("����")])
	      fprintf(Outfp, "��,");
	    if (ptr->SCASE_code[case2num("���")])
	      fprintf(Outfp, "��,");
	    if (ptr->SCASE_code[case2num("�˳�")])
	      fprintf(Outfp, "��,");
	    if (ptr->SCASE_code[case2num("�ǳ�")])
	      fprintf(Outfp, "��,");
	    if (ptr->SCASE_code[case2num("�����")])
	      fprintf(Outfp, "����,");
	    if (ptr->SCASE_code[case2num("�ȳ�")])
	      fprintf(Outfp, "��,");
	    if (ptr->SCASE_code[case2num("����")])
	      fprintf(Outfp, "���,");
	    if (ptr->SCASE_code[case2num("�س�")])
	      fprintf(Outfp, "��,");
	    if (ptr->SCASE_code[case2num("�ޥǳ�")])
	      fprintf(Outfp, "�ޥ�,");
	    if (ptr->SCASE_code[case2num("�γ�")])
	      fprintf(Outfp, "��,");
	    if (ptr->SCASE_code[case2num("����")])
	      fprintf(Outfp, "����,");
	    fprintf(Outfp, ">");
	}

	fputc('\n', Outfp);
    }
}

/*==================================================================*/
      void print_result(SENTENCE_DATA *sp, int case_print_flag)
/*==================================================================*/
{
    /* case_print_flag: �ʲ��Ϸ�̤���� */

    char *date_p, time_string[64];
    time_t t;
    struct tm *tms;
    TOTAL_MGR *tm = sp->Best_mgr;

    /* ���֤μ��� */
    t = time(NULL);
    tms = localtime(&t);
    if (!strftime(time_string, 64, "%Y/%m/%d", tms))
	time_string[0] = '\0';
    
    /* PS���Ϥξ��
       dpnd_info_to_bnst(&(tm->dpnd));
       make_dpnd_tree();
       print_kakari2ps();
       return;
    */ 

    /* �����ϤؤΥѥ�����ޥå���, �ޥå����ʤ���н��Ϥ��ʤ�
       if (OptAnalysis == OPT_AssignF && !PM_Memo[0]) return;
    */

    /* �إå��ν��� */

    if (sp->Comment) {
	fprintf(Outfp, "%s", sp->Comment);
    } else {
	fprintf(Outfp, "# S-ID:%d", sp->Sen_num);
    }

    if (OptInput == OPT_RAW) {
	if ((date_p = (char *)getenv("DATE")))
	    fprintf(Outfp, " KNP:%s", date_p);
	else if (time_string[0])
	    fprintf(Outfp, " KNP:%s", time_string);
    }

    /* N-best�ΤȤ��ϥ���������� */
    if (OptDisplay == OPT_NBEST && !ErrorComment) {
	fprintf(Outfp, " SCORE:%.5f", sp->score);
    }

    /* ���顼������С����顼������ */
    if (ErrorComment) {
	fprintf(Outfp, " ERROR:%s", ErrorComment);
	free(ErrorComment);
	ErrorComment = NULL;
    }

    if (PM_Memo[0]) {
	if (sp->Comment && strstr(sp->Comment, "MEMO")) {
	    fprintf(Outfp, "%s", PM_Memo);
	} else {
	    fprintf(Outfp, " MEMO:%s", PM_Memo);
	}	
    }

    fprintf(Outfp, "\n");

    /* ����� */
    if (OptPostProcess) {
	if (OptExpress == OPT_TAB || 
	    OptExpress == OPT_NOTAG) {
	    tag_bnst_postprocess(sp, 1);
	}
	else {
	    tag_bnst_postprocess(sp, 0); /* �ڹ�¤���ϤΤ��ᡢnum, dpnd_head ���ֹ���դ��ؤ��Ϥ��ʤ� */
	}
    }

    /* ���Ϸ�̤Υᥤ��ν��� */

    if (OptExpress == OPT_TAB) {
	print_tags(sp, 1);
    }
    else if (OptExpress == OPT_NOTAG) {
	print_mrphs(sp, 1);
    }
    else if (OptExpress == OPT_PA) {
	/* FIXME: �ʲ��Ϸ�̤���������Ȥ�ɬ�פ����� */
	print_pa_structure(sp);
    }
    else if (OptExpress == OPT_NOTAGTREE) {
	/* ʸ���tree���� */
	make_dpnd_tree(sp);
	print_kakari(sp, OptExpress);
    }
    else {
	/* ����ñ�̤�tree���� */
	make_dpnd_tree(sp);
	bnst_to_tag_tree(sp); /* ����ñ�̤��ڤ� */
	print_kakari(sp, OptExpress); /* OPT_TREE */
    }

    /* nbest���ץ����ʤɤǤϤ��δؿ���ʣ����ƤФ��ΤǸ�����򸵤��ᤷ�Ƥ��� */
    if (OptPostProcess) {
	undo_tag_bnst_postprocess(sp);
    }

    /* �ʲ��Ϥ�Ԥʤä����ν��� */
    if (case_print_flag && 
	(((OptAnalysis == OPT_CASE || 
	   OptAnalysis == OPT_CASE2) && 
	  (OptDisplay == OPT_DETAIL || 
	   OptDisplay == OPT_DEBUG)) || 
	 (OptEllipsis && 
	  VerboseLevel >= VERBOSE1))) {

	print_case_result(sp);

	/* ���β��ϤΤ���˽�������Ƥ��� */
	tm->pred_num = 0;
    }
}

/*==================================================================*/
    void push_entity(char ***list, char *key, int count, int *max)
/*==================================================================*/
{
    if (*max == 0) {
	*max = ALLOCATION_STEP;
	*list = (char **)malloc_data(sizeof(char *)*(*max), "push_entity");
    }
    else if (*max <= count) {
	*list = (char **)realloc_data(*list, sizeof(char *)*(*max <<= 1), "push_entity");
    }

    *(*list+count) = key;
}

/*==================================================================*/
		  void prepare_entity(BNST_DATA *bp)
/*==================================================================*/
{
    int count = 0, max = 0, i, flag = 0;
    char *cp, **list, *str;

    /* �����ƥ� */
    flag = 0;
    for (i = 0; i < bp->mrph_num; i++) {
	if (!(flag & 0x0001) && (cp = check_feature((bp->mrph_ptr+i)->f, "Modality-�ջ�-����"))) {
	    push_entity(&list, cp, count++, &max);
	    flag |= 0x0001;
	}
	if (!(flag & 0x0002) && (cp = check_feature((bp->mrph_ptr+i)->f, "Modality-�ջ�-�ջ�"))) {
	    push_entity(&list, cp, count++, &max);
	    flag |= 0x0002;
	}
	if (!(flag & 0x0004) && (cp = check_feature((bp->mrph_ptr+i)->f, "Modality-�ջ�-��Ͷ"))) {
	    push_entity(&list, cp, count++, &max);
	    flag |= 0x0004;
	}
	if (!(flag & 0x0008) && (cp = check_feature((bp->mrph_ptr+i)->f, "Modality-�ջ�-��˾"))) {
	    push_entity(&list, cp, count++, &max);
	    flag |= 0x0008;
	}
	if (!(flag & 0x0010) && (cp = check_feature((bp->mrph_ptr+i)->f, "Modality-�ջ�-�ػ�"))) {
	    push_entity(&list, cp, count++, &max);
	    flag |= 0x0010;
	}
	if (!(flag & 0x0020) && (cp = check_feature((bp->mrph_ptr+i)->f, "Modality-�ջ�-���;ΰջ�"))) {
	    push_entity(&list, cp, count++, &max);
	    flag |= 0x0020;
	}
	if (!(flag & 0x0040) && (cp = check_feature((bp->mrph_ptr+i)->f, "Modality-�ջ�-������"))) {
	    push_entity(&list, cp, count++, &max);
	    flag |= 0x0040;
	}
	if (!(flag & 0x0080) && (cp = check_feature((bp->mrph_ptr+i)->f, "Modality-�ջ�-����"))) {
	    push_entity(&list, cp, count++, &max);
	    flag |= 0x0080;
	}
	if (!(flag & 0x0100) && (cp = check_feature((bp->mrph_ptr+i)->f, "Modality-�ջ�-̿��"))) {
	    push_entity(&list, cp, count++, &max);
	    flag |= 0x0100;
	}
	if (!(flag & 0x0200) && (cp = check_feature((bp->mrph_ptr+i)->f, "Modality-����"))) {
	    push_entity(&list, cp, count++, &max);
	    flag |= 0x0200;
	}
	if (!(flag & 0x0400) && (cp = check_feature((bp->mrph_ptr+i)->f, "Modality-����-����"))) {
	    push_entity(&list, cp, count++, &max);
	    flag |= 0x0400;
	}
	if (!(flag & 0x0800) && (cp = check_feature((bp->mrph_ptr+i)->f, "Modality-Ƚ��-��ǽ��"))) {
	    push_entity(&list, cp, count++, &max);
	    flag |= 0x0800;
	}
	if (!(flag & 0x1000) && (cp = check_feature((bp->mrph_ptr+i)->f, "Modality-Ƚ��-��ǽ��-�Բ�ǽ"))) {
	    push_entity(&list, cp, count++, &max);
	    flag |= 0x1000;
	}
	if (!(flag & 0x2000) && (cp = check_feature((bp->mrph_ptr+i)->f, "Modality-Ƚ��-����"))) {
	    push_entity(&list, cp, count++, &max);
	    flag |= 0x2000;
	}
	if (!(flag & 0x4000) && (cp = check_feature((bp->mrph_ptr+i)->f, "Modality-Ƚ��-��ʹ"))) {
	    push_entity(&list, cp, count++, &max);
	    flag |= 0x4000;
	}
	if (!(flag & 0x8000) && (cp = check_feature((bp->mrph_ptr+i)->f, "Modality-Ƚ��-����"))) {
	    push_entity(&list, cp, count++, &max);
	    flag |= 0x8000;
	}
    }

    /* ���Ϥ���feature������н��� */
    if (count) {
	int i, len = 0;
	for (i = 0; i < count; i++) {
	    len += strlen(list[i])+1;
	}
	str = (char *)malloc_data(sizeof(char)*(len+2), "print_entity");
	strcpy(str, "C:");
	for (i = 0; i < count; i++) {
	    if (i != 0) strcat(str, " ");
	    strcat(str, list[i]);
	}
	assign_cfeature(&(bp->f), str, FALSE);
	free(str);
	free(list);
    }
}

/*==================================================================*/
	      void prepare_all_entity(SENTENCE_DATA *sp)
/*==================================================================*/
{
    int i;

    for (i = 0; i < sp->Bnst_num; i++) {
	prepare_entity(sp->bnst_data+i);
    }
}

/*====================================================================
                               END
====================================================================*/
