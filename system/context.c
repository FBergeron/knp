/*====================================================================

			       ʸ̮����

                                               S.Kurohashi 98. 9. 8

    $Id$
====================================================================*/
#include "knp.h"

extern int ArticleID;
/* extern char KNPSID[]; */
extern char SID_box[];

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
void assign_GA2pred(BNST_DATA *pred_ptr, BNST_DATA *GA_ptr, char *comment)
/*==================================================================*/
{
    char result[256];
    sprintf(result, "C���ʿ���:%s:", comment);
    print_bnst(GA_ptr, result+strlen(result));
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
		assign_GA2pred(b_ptr, b_ptr->child[i], "����");
		goto Match;
	    } else if (check_feature(b_ptr->child[i]->f, "��:̤��")) {
		assign_GA2pred(b_ptr, b_ptr->child[i], "̤��");
		goto Match;
	    }
	}

	/* PARA�˷��äƤ�����ʬ */

	if (b_ptr->para_type == PARA_NORMAL) {
	    for (i = 0; b_ptr->parent->child[i]; i++) {
		if (b_ptr->parent->child[i]->para_type == PARA_NIL &&
		    check_feature(b_ptr->parent->child[i]->f, "��:����")) {
		    assign_GA2pred(b_ptr, b_ptr->parent->child[i], "����");
		    goto Match;
		}
		else if (b_ptr->parent->child[i]->para_type == PARA_NIL &&
			 check_feature(b_ptr->parent->child[i]->f, "��:̤��")) {
		    assign_GA2pred(b_ptr, b_ptr->parent->child[i], "̤��");
		    goto Match;
		}
		
	    }		
	}

	/* Ϣ�ν��������ߤ� */
	    
	if (check_feature(b_ptr->f, "��:Ϣ��") &&
	    b_ptr->parent &&
	    b_ptr->parent->para_top_p != TRUE &&
	    !check_feature(b_ptr->parent->f, "���δط�")) {
	    assign_GA2pred(b_ptr, b_ptr->parent, "������");
	    goto Match;
	}
	if (check_feature(b_ptr->f, "��:Ϣ��") &&
	    b_ptr->para_type != PARA_NORMAL &&
	    b_ptr->parent &&
	    b_ptr->parent->para_top_p == TRUE &&
	    b_ptr->parent->parent &&
	    b_ptr->parent->parent->para_top_p != TRUE &&
	    !check_feature(b_ptr->parent->parent->f, "���δط�")&&
	    (cp = (char *)check_feature(b_ptr->parent->parent->f, "C���ʿ���")) != NULL) {
	    /* assign_GA2pred(b_ptr, cp + strlen("C���ʿ���:"), "���Ѹ�"); */
	    goto Match;
/*
  (����ʤȤ�)

            ��ʬ������     HOGE��   <C���ʿ���:HOGE>
                    ����������
             A <P>����      ��
                    ������������
             B <P>��PARA��������
                            C <C���ʿ���:̤��:HOGE>

            ��)�����ǧ�᤿��ΤΡ��������ϴ��ɤˤ����դ��Ȥ��ä�����
               �򷫤��֤����ȤΤʤ��褦���ط��Ԥ��������Ԥ�������
*/

	}
	if (check_feature(b_ptr->f, "��:Ϣ��") &&
	    b_ptr->parent &&
	    b_ptr->parent->para_top_p == TRUE &&
	    b_ptr->parent->parent &&
	    b_ptr->parent->parent->para_top_p != TRUE &&
	    !check_feature(b_ptr->parent->parent->f, "���δط�")) {
	    assign_GA2pred(b_ptr, b_ptr->parent->parent, "������");
	    goto Match;
	}
	if (check_feature(b_ptr->f, "��:Ϣ��") &&
	    b_ptr->parent &&
	    b_ptr->parent->para_top_p == TRUE &&
	    b_ptr->parent->parent &&
	    b_ptr->parent->parent->para_top_p != TRUE &&
	    check_feature(b_ptr->parent->parent->f, "���δط�") &&
	    check_feature(b_ptr->parent->parent->f, "��:Ϣ��")&&
	    (cp = (char *)check_feature(b_ptr->parent->parent->parent->f, "C���ʿ���")) != NULL) {
	    /* assign_GA2pred(b_ptr, cp + strlen("C���ʿ���:"), "���Ѹ�"); */
	    goto Match;
	}


	/* Ϣ�ѽ�����Υ��� */

	if (check_feature(b_ptr->f, "��:Ϣ��") &&
	    b_ptr->parent &&
	    (cp = (char *)check_feature(b_ptr->parent->f, "C���ʿ���")) != NULL) {
	    /* assign_GA2pred(b_ptr, cp + strlen("C���ʿ���:"), "���Ѹ�"); */
	    goto Match;
	}

	/* ��������ʤ�����ʸ */

	if (sp->Sen_num >= 2){
	    prev_sp = sentence_data + (sp->Sen_num - 2); 
	    if (prev_sp->Bnst_num >= 1 ){
		if ((cp = (char *)check_feature(prev_sp->bnst_data[prev_sp->Bnst_num - 1].f,
						"C���ʿ���")) != NULL) {
		    /* assign_GA2pred(b_ptr, cp + strlen("C���ʿ���:"), "��ʸ"); */
		    goto Match;
		}
	    }
	}

	assign_cfeature(&(b_ptr->f), "C����̤ȯ��");
    }
    Match:

    for (i = 0; b_ptr->child[i]; i++) {
	GA_detection(b_ptr->child[i]);
    }
}    


/*==================================================================*/
		      void clear_context(int f)   
/*==================================================================*/
/* sentence_data �����դ�뤫, �ޤ���ʸ�Ϥ��Ѥ�ä��Ȥ�( "# S-ID:"��
   �ֹ�Ǽ���), sentence_data ��clear ���� */
{
    int i;
    SENTENCE_DATA *stock_sp_1, *stock_sp_2;


/*  f == 0 : sp->Sen_num == 256 */
/*  f == 1 : # S-ID ���Ѥ�ä���, "�ڡ�"�ǰ㤦����ˤʤä����Ȥ��� */
    if (f == 0){
	/* sentence_data �����դ줽���ʤȤ�*/

	/* �Ǹ��7ʸ(�ʹ֤�û�������ޥ��å��ʥ�С�) �Υǡ����ϻĤ�*/
	for(i = 6; i >= 0; i--){
	    free(sentence_data - (249 + i));
	    stock_sp_1 = sentence_data - i;
	    stock_sp_2 = sentence_data - (249 + i);
	    stock_sp_2 = stock_sp_1;
	    sp->Sen_num = 7;
	}
	for( i = 0; i <= 249; i++){
	    free(sentence_data - i);
	}
	sp->Sen_num = 7;
    } 
    else if (f == 1){

    /* S_ID ���Ѥ��, �ޤ����̤μ���ˤʤä�("�ڡ�"�ǻϤޤ��) */
	for(i = 0; i < sp->Sen_num ; i++){
	    free(sentence_data -i);
	    sp->Sen_num = 0;
	}
    }
    return;
}

/*==================================================================*/
		      void discourse_analysis()
/*==================================================================*/
{
    int i, flag;
    char *cp;
    char kakko[] = "�ڡ�";

    if(sp->Sen_num == 256){
    /* sentence_data �� overflow ������ */
	printf("The program celars the sentence_data due to the overflow.\n\n");
	flag = 0;
	clear_context(flag);
	return;
    }

    /* ʸ�Ϥζ��ڤ��, S-ID���ֹ椫, ʸƬ���֡ڡǡפǻϤޤ�ʸ���Ǹ��Τ���*/

    else if( strcmp("\\",sp->bnst_data[0].mrph_ptr->Goi) == 0){
	/* ��# S-ID:�פǻϤޤ�Ԥ� sp->bnst_data[0].mrph_ptr->Goi �ϡ��ʤ���"\" 
	   ("#"����ʤ�)*/

	if (strncmp("A-ID",sp->bnst_data[1].mrph_ptr->Goi, 4) == 0){
	}
	else if	((strncmp("S-ID",sp->bnst_data[1].mrph_ptr->Goi, 4) == 0)&&
		 (strncmp(SID_box, sp->bnst_data[1].mrph_ptr->Goi, 14) != 0)){
	/* S-ID ������å�. �ֹ椬�Ѥ�ä��� clear_context */

	    /* S-ID:950101008 ��������14ʸ��. 14ʸ��ʬSID_box�˥��ԡ� */
		strncpy(SID_box, sp->bnst_data[1].mrph_ptr->Goi, 14);
		flag = 1;
		clear_context(flag);
	}
	return;

    } else if ( strcmp(kakko, sp->bnst_data[0].mrph_ptr->Goi) == 0){
	/* �֡ڡǡ� �ǻϤޤ��(������Ѥ����) */
	flag = 1; 
	clear_context(flag);
	return;
    }


    GA_detection(sp->bnst_data + sp->Bnst_num -1);
    
    copy_sentence();
}

/*====================================================================
                               END
====================================================================*/
