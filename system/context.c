/*====================================================================

			       ʸ̮����

                                               S.Kurohashi 98. 9. 8

    $Id$
====================================================================*/
#include "knp.h"

char Ga_Memory[256];

/*==================================================================*/
			 void copy_sentence()
/*==================================================================*/
{
    /* ʸ���Ϸ�̤��ݻ� */

    int i, j;
    SENTENCE_DATA *sp_new = sentence_data + sp->Sen_num - 1;

    sp_new->Sen_num = sp->Sen_num;

    sp_new->Mrph_num = sp->Mrph_num;
    sp_new->mrph_data = (MRPH_DATA *)malloc_data(sizeof(MRPH_DATA)*sp->Mrph_num, 
						 "MRPH DATA");
    for (i = 0; i < sp->Mrph_num; i++) {
	sp_new->mrph_data[i] = sp->mrph_data[i];
	sp->mrph_data[i].f = NULL;
    }

    sp_new->Bnst_num = sp->Bnst_num;
    sp_new->bnst_data = 
	(BNST_DATA *)malloc_data(sizeof(BNST_DATA)*(sp->Bnst_num + sp->New_Bnst_num), 
				 "BNST DATA");
    for (i = 0; i < sp->Bnst_num + sp->New_Bnst_num; i++) {
	sp_new->bnst_data[i] = sp->bnst_data[i];
	sp_new->bnst_data[i].mrph_ptr += sp_new->mrph_data - sp->mrph_data;
	sp_new->bnst_data[i].settou_ptr += sp_new->mrph_data - sp->mrph_data;
	sp_new->bnst_data[i].jiritu_ptr += sp_new->mrph_data - sp->mrph_data;
	sp_new->bnst_data[i].fuzoku_ptr += sp_new->mrph_data - sp->mrph_data;
	sp_new->bnst_data[i].parent += sp_new->bnst_data - sp->bnst_data;
	for (j = 0; sp_new->bnst_data[i].child[j]; j++) {
	    sp_new->bnst_data[i].child[j]+= sp_new->bnst_data - sp->bnst_data;
	}
	sp->bnst_data[i].f = NULL;
    }

    sp_new->para_data = (PARA_DATA *)malloc_data(sizeof(PARA_DATA)*Para_num, 
				 "PARA DATA");
    for (i = 0; i < Para_num; i++) {
	sp_new->para_data[i] = sp->para_data[i];
	sp_new->para_data[i].manager_ptr += sp_new->para_manager - sp->para_manager;
    }

    sp_new->para_manager = 
	(PARA_MANAGER *)malloc_data(sizeof(PARA_MANAGER)*Para_M_num, 
				    "PARA MANAGER");
    for (i = 0; i < Para_M_num; i++) {
	sp_new->para_manager[i] = sp->para_manager[i];
	sp_new->para_manager[i].parent = sp_new->para_manager - sp->para_manager;
	for (j = 0; j < sp_new->para_manager[i].child_num; j++) {
	    sp_new->para_manager[i].child[j] = sp_new->para_manager - sp->para_manager;
	}
	sp_new->para_manager[i].bnst_ptr = sp_new->bnst_data - sp->bnst_data;
    }
}

/*==================================================================*/
void assign_GA2pred(BNST_DATA *pred_ptr, char *GA, char *comment)
/*==================================================================*/
{
    char result[256];
    sprintf(result, "C���ʿ���:%s:%s", comment, GA);
    assign_cfeature(&(pred_ptr->f), result);
    /* printf("  %s\n", result); */
}    

/*==================================================================*/
		 void GA_detection(BNST_DATA *b_ptr)
/*==================================================================*/
{
    int i;
    char *cp;
    SENTENCE_DATA *prev_sp;

    /* �Ѹ��Υ��ʤ򤵤����ץ�ȥ����� */

    if (b_ptr->para_top_p == TRUE) goto Match;

    if (check_feature(b_ptr->f, "�Ѹ�") &&
	!check_feature(b_ptr->f, "��٥�:A-")) {

	/* printf("�Ѹ�ȯ�� %s\n", b_ptr->Jiritu_Go); */

	/* �ޤ��Ҷ���ߤ� */

	for (i = 0; b_ptr->child[i]; i++) {
	    if (check_feature(b_ptr->child[i]->f, "��:����")) {
		assign_GA2pred(b_ptr, b_ptr->child[i]->Jiritu_Go, "����");
		goto Match;
	    } else if (check_feature(b_ptr->child[i]->f, "��:̤��")) {
		assign_GA2pred(b_ptr, b_ptr->child[i]->Jiritu_Go, "̤��");
		goto Match;
	    }
	}

	/* PARA�˷��äƤ�����ʬ */

	if (b_ptr->para_type == PARA_NORMAL) {
	    for (i = 0; b_ptr->parent->child[i]; i++) {
		if (b_ptr->parent->child[i]->para_type == PARA_NIL &&
		    check_feature(b_ptr->parent->child[i]->f, "��:����")) {
		    assign_GA2pred(b_ptr, b_ptr->parent->child[i]->Jiritu_Go, "����");
		    goto Match;
		}
		else if (b_ptr->parent->child[i]->para_type == PARA_NIL &&
			 check_feature(b_ptr->parent->child[i]->f, "��:̤��")) {
		    assign_GA2pred(b_ptr, b_ptr->parent->child[i]->Jiritu_Go, "̤��");
		    goto Match;
		}
		
	    }		
	}

	/* Ϣ�ν��������ߤ� */
	    
	if (check_feature(b_ptr->f, "��:Ϣ��") &&
	    b_ptr->parent &&
	    !check_feature(b_ptr->parent->f, "���δط�")) {
	    assign_GA2pred(b_ptr, b_ptr->parent->Jiritu_Go, "������");
	    goto Match;
	}

	/* Ϣ�ѽ�����Υ��� */

	if (check_feature(b_ptr->f, "��:Ϣ��") &&
	    b_ptr->parent &&
	    (cp = (char *)check_feature(b_ptr->parent->f, "C���ʿ���")) != NULL) {
	    assign_GA2pred(b_ptr, cp + strlen("C���ʿ���:"), "���Ѹ�");
	    goto Match;
	}

	/* ��������ʤ�����ʸ */

	prev_sp = sentence_data + sp->Sen_num - 2;
	if ((cp = (char *)check_feature(prev_sp->bnst_data[prev_sp->Bnst_num - 1].f,
					"C���ʿ���")) != NULL) {
	    assign_GA2pred(b_ptr, cp + strlen("C���ʿ���:"), "��ʸ");
	}
    }
    Match:

    for (i = 0; b_ptr->child[i]; i++) {
	GA_detection(b_ptr->child[i]);
    }
}    

/*==================================================================*/
		      void discourse_analysis()
/*==================================================================*/
{
    char *cp;

    /* clear_context(); ������ʸ̮��������clear���� */

    GA_detection(sp->bnst_data + sp->Bnst_num -1);
    print_result();

    copy_sentence();
}

/*====================================================================
                               END
====================================================================*/
