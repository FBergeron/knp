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
char KAKKO[] = "�ڡ�";
char IU[] = "����";
char NO[] = "��";
char KOTO[] = "����";
char TO[] = "��";
char BEKI[] = "�٤�";
char GA[] = "��";
char SEMI_CL[]=":";
char HA[] = "��";
char KUTEN[] = "��";



/*==================================================================*/
			 void copy_sentence()
/*==================================================================*/
{
    /* ʸ���Ϸ�̤��ݻ� */

    int i, j, k;

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

	sp_new->bnst_data[i] = sp->bnst_data[i]; /* ������bnst_data�������Ф򥳥ԡ� */

	/* �ʲ�, ñ��˥��ԡ��������Ȥˤ��ݥ��󥿥��ɥ쥹�Τ�������� */
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
                int check_f_modal_b(BNST_DATA *b_ptr)
/*==================================================================*/
{
    	if (check_feature(b_ptr->f, "Modality-�ջ�-�ջ�")||
	    check_feature(b_ptr->f, "Modality-�ջ�-��˾")||
	    check_feature(b_ptr->f, "Modality-�ջ�-�ػ�")||
	    check_feature(b_ptr->f, "Modality-�ջ�-��Ͷ")||
	    check_feature(b_ptr->f, "Modality-�ջ�-̿��")||
	    check_feature(b_ptr->f, "Modality-�ջ�-����")||
	    check_feature(b_ptr->f, "Modality-����")||
	    check_feature(b_ptr->f, "Modality-����-����")||
	    check_feature(b_ptr->f, "Modality-Ƚ��-����")||
	    check_feature(b_ptr->f, "Modality-Ƚ��-��ʹ")||
	    check_feature(b_ptr->f, "Modality-Ƚ��-����")||
	    check_feature(b_ptr->f, "Modality-Ƚ��-��ǽ��")||
	    check_feature(b_ptr->f, "Modality-Ƚ��-��ǽ��-�Բ�ǽ")){
	    return TRUE;
	}
	else {
	    return FALSE;
	}
}

/*==================================================================*/
                int check_f_modal_m(MRPH_DATA *m_ptr)
/*==================================================================*/
{
    	if (check_feature(m_ptr->f, "Modality-�ջ�-�ջ�")||
	    check_feature(m_ptr->f, "Modality-�ջ�-��˾")||
	    check_feature(m_ptr->f, "Modality-�ջ�-�ػ�")||
	    check_feature(m_ptr->f, "Modality-�ջ�-��Ͷ")||
	    check_feature(m_ptr->f, "Modality-�ջ�-̿��")||
	    check_feature(m_ptr->f, "Modality-�ջ�-����")||
	    check_feature(m_ptr->f, "Modality-����")||
	    check_feature(m_ptr->f, "Modality-����-����")||
	    check_feature(m_ptr->f, "Modality-Ƚ��-����")||
	    check_feature(m_ptr->f, "Modality-Ƚ��-��ʹ")||
	    check_feature(m_ptr->f, "Modality-Ƚ��-����")||
	    check_feature(m_ptr->f, "Modality-Ƚ��-��ǽ��")||
	    check_feature(m_ptr->f, "Modality-Ƚ��-��ǽ��-�Բ�ǽ")){
	    return TRUE;
	}
	else {
	    return FALSE;
	}
}


/*==================================================================*/
                int check_f_modal_ISHI(MRPH_DATA *m_ptr)
/*==================================================================*/
{
    if(check_feature(m_ptr->f, "Modality-�ջ�-�ջ�")||
       check_feature(m_ptr->f, "Modality-�ջ�-��˾")){

	    return TRUE;
	}
	else {
	    return FALSE;
	}
}



/*==================================================================*/
  int check_primary_child(BNST_DATA *fst_b_ptr, 
			  BNST_DATA *b_ptr,
			  SENTENCE_DATA *local_sp)
/*==================================================================*/
/* 
 *    ��¹��"��:̤��", "��:����"����äƤ���
 */
{
    int i,j;
    int rk, fl, stop;
    int rk_2;
    BNST_DATA *tmp_bptr, *pos_bptr;
    BNST_DATA *child_box[2][64];

    rk = 0; fl = 0;    /* ��:rank, ��:file */
    rk_2 = 1;
    i = 0 ; j = 0;
    stop = 1;
    child_box[0][0] = b_ptr;

    for(stop; stop; ){
	for(fl = 0; fl < stop; fl++){
	    tmp_bptr = child_box[rk][fl];


	    for(i = 0, j; tmp_bptr->child[i]; i++, j++){

		if ((check_feature(tmp_bptr->f, "���") &&
		     check_feature(tmp_bptr->child[i]->f, "����")&&
		     check_feature(tmp_bptr->child[i]->f, "��")) ||
		    (check_feature(tmp_bptr->child[i]->f, "����") &&
		     check_feature(tmp_bptr->child[i]->f, "��"))){
		    return FALSE;
		}
		
		else if (check_feature(tmp_bptr->child[i]->f, "��:����")){
		    pos_bptr = skip_para_top(tmp_bptr->child[i], "ch");
		    assign_GA2pred(fst_b_ptr, pos_bptr, "����", local_sp);
		    return TRUE;
		}
		else if (check_feature(tmp_bptr->child[i]->f, "��:̤��")){
		    pos_bptr = skip_para_top(tmp_bptr->child[i], "ch");
		    assign_GA2pred(fst_b_ptr, pos_bptr, "̤��", local_sp);
		    return TRUE;
		}

 
#if 0
		else if ((!check_feature(tmp_bptr->f, "ID:����") ||
			  !check_feature(tmp_bptr->child[i]->f, "��")) &&
			 check_feature(tmp_bptr->child[i]->f, "��:̤��")){
		    /*
		     * ��A�ʤ��B��C����פΤȤ�, A���Ф��ơ�B��פϥ��ʤǤϤʤ�
		     * (��) �ȳ��ϴ������ˤ���п���������ʤ�
		     */
		    assign_GA2pred(fst_b_ptr, tmp_bptr->child[i], "̤��", local_sp);
		    return TRUE;
		}
#endif
		
		else child_box[rk_2][j] = tmp_bptr->child[i];
	    }
	}
	child_box[rk_2][j] = NULL; /* �ԤκǸ����ʼ������Ȥ� */
	stop = j;
	j = 0;

	/* rk �� rk_2 �� swap */
	if (rk == 1){
	    rk = 0; rk_2 = 1;
	} else {
	    rk = 1; rk_2 = 0;
	}

    }

    return FALSE; 
}





/*==================================================================*/
BNST_DATA *skip_para_top(BNST_DATA *b_ptr, char *flag)
/*==================================================================*/
/*
 *   ���ʸ��䤬, PARA TOP ���ä��顢������λҤ��Ƥ��֤�
 *   "C���ʿ���" �� feature ����äƤ��� b_ptr �ˤ�, 
 */
{
    /* check child */
    if (strcmp(flag, "ch") == 0){
	if (b_ptr->para_top_p == TRUE){
	    return skip_para_top(b_ptr->child[0], "ch");
	}
	else return b_ptr;
    }

    /* check parent */
    else if (strcmp(flag, "pa") == 0){
	if (b_ptr->para_top_p == TRUE){
	    return skip_para_top(b_ptr->parent, "pa");
	}
	else return b_ptr;
    }

    else {
	printf("ERROR in skip_para_top() : Invalid flag %s\n", flag);
	return NULL;
    }

}


/*==================================================================*/
BNST_DATA *check_ancestor(BNST_DATA *b_ptr)
/*==================================================================*/
/* 
 *   ���������ĤΥϳ�, �⤷���ϥ��ʤ�õ��
 */
{
    int i, j;
    int pos, m_num;
    BNST_DATA *tmp_bptr, *tmp_bptr_next;
    BNST_DATA *ancst[64]; 
   /*
    * i+1���ܤ����Ǥ�, i���ܤ����Ǥο� 
    * ancst[0] �λҶ���, b_ptr.
    * �Ƥ� PARA �Ǥ��Ǽ���Ƥ���.(���Ȥǥ����åפ���)
    * ancst[i]<-->ancst[i+1]
    *  (child <--> parent)
    */

    tmp_bptr = b_ptr;

    for(i = 0; tmp_bptr->parent; i++){
	tmp_bptr = tmp_bptr->parent;
	ancst[i] = tmp_bptr;
    }

    if (i == 0) return NULL; /* b_ptr �� root ���ä� */

    ancst[i] = NULL; /* �Ǹ����ʼ������Ƥ��� */

    
    for(i = 0; ancst[i] != NULL; i++){

	/* �Ƥ�PARA�ʤ�, ���ο� */
	if (ancst[i]->para_top_p == TRUE) continue;

	tmp_bptr = ancst[i]; /* tmp_bptr �Ϻ�����Ȥ���οƤΤĤ�� */

	if (i == 0){
	    pos = position_in_children(b_ptr);
	}
	else if (i > 0){
	    pos = position_in_children(ancst[i-1]);
	}
	else pos = -1;


	if(pos >= 0){

	    /* �ޤ������Ĥˤ����äƤ���ϳʤ�õ�� */
	    pos = pos + 1;
	    for(pos; tmp_bptr->child[pos]; pos ++){

		m_num = tmp_bptr->child[pos]->mrph_num;
		if (check_feature(tmp_bptr->child[pos]->f, "��:̤��") &&
		    ((strcmp(HA, tmp_bptr->child[pos]->mrph_ptr[m_num-1].Goi) == 0) ||
		     (strcmp(KUTEN, tmp_bptr->child[pos]->mrph_ptr[m_num-1].Goi) == 0) &&
		     (strcmp(HA, tmp_bptr->child[pos]->mrph_ptr[m_num-2].Goi) == 0))){

		    return skip_para_top(tmp_bptr->child[pos], "ch");
		}
	    }
	    /* ���Ĥˤ�����ϳʤ��ʤ���, ľ�ϤοƤ����ʤ��ä���, ���ο� */
	    if (check_feature(tmp_bptr->f, "��:����")){
		return tmp_bptr;
	    }
	}
    }

    return NULL;
}


/*==================================================================*/
BNST_DATA *check_posterity(BNST_DATA *b_ptr, int s)
/*==================================================================*/
/* 
 *    ��¹��"��:̤��", "��:����"����äƤ���
 */
{
    /*
     * child_box[][] (���Ԥ��ߤ˻Ȥ�)
     *
     *           |<-     64     ->|
     *           ------------------  
     *  first    | | | | .... | | |
     *           ------------------
     *  second   | | | | .... | | |
     *           ------------------
     */

    /*  
     *   ==================
     *   THE CHILD diagram
     *   =================
     *   �Ĥ��¤Ӥ�, child_box[][]�β��ιԤˤ�����  
     *
     *   ...  third  second  first
     *   ...    ��    ��     ��
     *
     *                G������
     *                F������
     *                E������
     *                      A������
     *          .. ����          ��
     *                D������    ��
     *                      B������
     *                           ��
     *                      C������
     *                           ��
     *                          SELF
     * 
     */

    int i,j;
    int rk, fl, stop;
    int rk_2;
    BNST_DATA *tmp_bptr;
    BNST_DATA *child_box[2][64];

    rk = 0; fl = 0;    /* ��:rank, ��:file */
    rk_2 = 1;
    i = 0 ; j = 0;
    stop = 1;
    child_box[0][0] = b_ptr;

    for(stop; stop; ){
	for(fl = 0; fl < stop; fl++){
	    tmp_bptr = child_box[rk][fl];
/*
	    for(i = 0, j; tmp_bptr->child[i]; i++, j++){
		if(tmp_bptr->child[i]->para_top_p != TRUE &&
		   (check_feature(tmp_bptr->child[i]->f, "��:̤��") ||
		    check_feature(tmp_bptr->child[i]->f, "��:����"))){
		    return tmp_bptr->child[i];
		}
		else child_box[rk_2][j] = tmp_bptr->child[i];
	    }
*/
	    /* 
	     * ����ܤΥ롼�פΤȤ�����, s ���ͤ����� 
	     * �㤨��, ��ʬ�οƤ��� check_posterity()
	     * ��Ƥ���Ȥ�����ʬ���򤱤�������.
	     * ��¹��Ĵ�٤�Ȥ���, s=0 �ǸƤӽФ��Ф���
	     */
	    for(i = s, j; tmp_bptr->child[i]; i++, j++){
		if(tmp_bptr->child[i]->para_top_p != TRUE &&
		   (check_feature(tmp_bptr->child[i]->f, "��:̤��") ||
		    check_feature(tmp_bptr->child[i]->f, "��:����"))){
		    return tmp_bptr->child[i];
		}
		else child_box[rk_2][j] = tmp_bptr->child[i];
	    }
	}
	child_box[rk_2][j] = NULL; /* �ԤκǸ����ʼ������Ȥ� */
	stop = j;
	j = 0;
	s = 0;

	/* rk �� rk_2 �� swap */
	if (rk == 1){
	    rk = 0; rk_2 = 1;
	} else {
	    rk = 1; rk_2 = 0;
	}

    }

    return NULL; 
}




/*==================================================================*/
                 int position_in_children(BNST_DATA *b_ptr)
/*==================================================================*/
{
    int i;
    for(i = 0 ; b_ptr->parent->child[i]; i++){
	if (b_ptr == (b_ptr->parent->child[i])){
	    return i;
	}
    }
    return -1;
}




/*==================================================================*/
    void assign_GA2pred(BNST_DATA *pred_ptr, BNST_DATA *GA_candid, 
			char *comment, SENTENCE_DATA *local_sp)
/*==================================================================*/
{
    int bn, sn;
    char result[256];
    char *cp;
    char MOD_HOLDER[] = "Modality�ݻ���";
    char JISOU[] = "����";

    /* (����) 
     *  check_feature(GA_candid->f, "C���ʿ���") ��Ƚ���, ���δؿ��˰ܤ�
     *  ����, ����GA_detection �Ǥ�äƤ��뤫��, ������flag�Ǥ�Ω�Ƥ�
     *  �ۤ����������⤷��ʤ�����, �ѻ��ˤʤꤽ�������麣��check_feature
     *  ���äƤ���. 
     */
    if (GA_candid == NULL){
	if (strcmp(MOD_HOLDER, comment) == 0){
	sprintf(result, "C���ʿ���:%s:(ʸ#:-):(ʸ��#:-):ɮ��", comment);
	}
	else if (strcmp(JISOU, comment) == 0){
	sprintf(result, "C���ʿ���:%s:(ʸ#:-):(ʸ��#:-):���������֡�����", comment);
	}
	else {
	    printf("Invalid comment : %s\n", comment);
	}
    }
    else if ((cp = check_feature(GA_candid->f, "C����̤ȯ��")) != NULL){
	sprintf(result, "C����̤ȯ��");
    }
    else if((cp=check_feature(GA_candid->f, "C���ʿ���")) != NULL){
	sprintf(result, "C���ʿ���:%s:%s", comment, cp + strlen("C���ʿ���:"));
    }
    else {
	bn = GA_candid - (local_sp->bnst_data); /* GA_candid ��ʸ���ֹ� -1 �ο� */
	sn = local_sp->Sen_num;              /* ʸ���ֹ� */
	sprintf(result, "C���ʿ���:%s:(ʸ#:%d):(ʸ��#:%d):", comment, sn, bn+1);
	print_bnst( GA_candid, result + strlen(result));
    }
    assign_cfeature(&(pred_ptr->f),  result);
}


    /*===========================================
     * strlen("C���ʿ���:")
     * |<------------->|
     * -------------------------------------------
     * |C|��|��|��|��|:| | |...............| | | |
     * -------------------------------------------
     * ��              �� 
     * cp          cp + strlen("C���ʿ���:")
      ===========================================*/


    /*===========================================
      ���� result 
     ===========================================*/
    /*===========================================
     *  strlen(result)
     * |<----------->|
     * -------------------------------------------
     * |C|��|��|��|��|:|��|��|...............| | | |
     * -------------------------------------------
     *��                     �� 
   result       result+ strlen(result) 
     ===========================================*/
    /*===========================================
     * print_bnst(GA, result + strlen(result));
     * ---------------------------------------------
     * |C|��|��|��|��|:|��|��|:|��|��| |.....| | | |
     * ---------------------------------------------
     *                         |<--->|
     *	                         *GA
     *===========================================*/


#if 0
/*==================================================================*/
               char by_case_detection(BNST_DATA *b_ptr)
/*==================================================================*/
{
    int i, j, k;
    int num, flag;
    char feature_buffer[256];
    CF_PRED_MGR *cpm_ptr;
    CF_MATCH_MGR *cmm_ptr;
    CASE_FRAME *cf_ptr;
    BNST_DATA *pred_b_ptr;

    /* �ʲ��Ϥη��(Best_mgr������)��feature�Ȥ����Ѹ�ʸ���Ϳ���� */


}
#endif


/*==================================================================*/
		 void GA_detection(BNST_DATA *b_ptr)
/*==================================================================*/
{
    int i, j, k, s, t;
    int m_num, m_flag_include_ISHI, flag_1, flag_2;
    int tmp;
    char *cp;
    BNST_DATA *pos_bptr;
    SENTENCE_DATA *prev_sp, *local_sp;
    TOTAL_MGR *tm = &Best_mgr;
    CF_PRED_MGR *cpm_ptr;

    local_sp = sp;


    /* �Ѹ��Υ��ʤ򤵤��� */

    /*--------------*/
    /* ̵�뤹���� */
    /*--------------*/

    if(b_ptr->para_top_p == TRUE) goto Match;


    if(check_feature(b_ptr->f, "�Ѹ�:Ƚ") &&
       check_feature(b_ptr->f, "�θ�")){ 
   /*
    *       &&
    *     (!check_feature(b_ptr->f, "����"))){
    */
	goto Match;
    }

    if(check_feature(b_ptr->f, "�Ѹ�") &&
       check_feature(b_ptr->f, "�θ�") &&
       check_feature(b_ptr->f, "����")){
	goto Match;
    }

    if (check_feature(b_ptr->f, "�Ѹ�") &&
	!check_feature(b_ptr->f, "��٥�:A-")){
	
	/* printf("�Ѹ�ȯ�� %s\n", b_ptr->Jiritu_Go); */


	/*------------------------*/
	/* ��ʬ���Ѹ��Ǥ�̵�뤹�� */
	/*------------------------*/


	/*===== �֡��Ȥ����פʤɽ��� =====*/

	for (i = 0; b_ptr->child[i]; i++) {
	    if(check_feature(b_ptr->child[i]->f, "ID:���ȡʰ��ѡ�") &&
	       !check_feature(b_ptr->child[i]->f, "���δط�") &&
	       (!check_feature(b_ptr->f, "����") &&
		!check_feature(b_ptr->f, "���ѥ���")))
		goto Match;
	    /*----------------------------------------------------
	     * (����ʤȤ�)
	     *
	     *      �������Ȩ���������������
	     *                 ������������
	     *                        �ո�
	     *--------------------------------------------------*/
	}

	/*===== �֡��Ȥ���(�Τ�)�ס֡��Ȥ���(����)�פ�"����"����� =====*/

	for(i = 0; b_ptr->child[i]; i++){
	    m_num = b_ptr->child[i]->mrph_num;
	    if((strcmp(IU, b_ptr->mrph_ptr[0].Goi) == 0)&&
	      /*
	       * ((strcmp(NO, b_ptr->mrph_ptr[1].Goi) == 0)||
	       * (strcmp(KOTO, b_ptr->mrph_ptr[1].Goi) == 0)) && 
	       */
	       (strcmp(TO, b_ptr->child[i]->mrph_ptr[m_num-1].Goi) == 0))
		goto Match;
	    /*----------------------------------------------------
	     * (����ʤȤ�)
	     *
	     *  ��Ω���٤����פȨ�����
             *                      ����������������������������
             *                            �ͤ�������������������
	     *--------------------------------------------------*/
	}

	/*===== �Ѹ� + "Modality-����" �ΤȤ� =====*/

	if (check_feature(b_ptr->f, "��:Ϣ��") &&
	    check_feature(b_ptr->f, "Modality-����")){
	    goto Match;
	}


#if 0
	/*------------------------------*/
        /*   �ʲ��Ϥη�̤��饬�ʿ���   */
	/*------------------------------*/
	if((cp = by_case_detection(b_ptr)) != NULL){
	    goto Match;
	}
#endif



	/*--------------------------*/
        /*    �ޤ����㳰Ū�ʽ���    */
	/*--------------------------*/

	/*===== ʸ��� Modality �� feature ������Ȥ� =====*/

	if (check_f_modal_b(b_ptr)){ /* ʸ��� feature �� Modality ������ */
	
	    /* i == 0 �ޤǥ�����ȥ����󤹤��, ʸ������ Modality */
	    /*------------------------------------------------
	     *
	     *  (i == 0) ��  ��������(ʸ��) ���� Modality
	     *
	     *-----------------------------------------------*/
	    for (i = b_ptr->mrph_num; i > 0 ; i--){
		if(check_f_modal_m(&(b_ptr->mrph_ptr[i-1]))){
		    continue;
		}
		else {
		    break;
		}
	    }


	    if (i == 0){ /* ʸ�᤬���� Modality ���ä��� */

		/* j == 0 �ޤǥ�����ȥ����󤹤��, ʸ������ Modality-�ջ� */
		/*------------------------------------------------
		 *
		 *  (j == 0) ��  ��������(ʸ��) ���� Modality-�ջ�
		 *
		 *-----------------------------------------------*/
		for (j = b_ptr->mrph_num; j > 0 ; j--){
		    if(check_f_modal_ISHI(&(b_ptr->mrph_ptr[j-1]))){
			continue;
		    }
		    else {
			break;
		    }
		}

		
		/* ����ʸ��(�Ҷ�)�� Modality �ǽ���äƤ��뤫�ɤ���Ĵ�٤Ƥ��� */
		/*------------------------------------------------
		 *
		 *                      ������������
		 *  (flag_1 == 1) ��              ��������
		 *
		 *-----------------------------------------------*/	    
		flag_1 = 0;
		for(i = 0;  b_ptr->child[i]; i++){
		    m_num = b_ptr->child[i]->mrph_num;
		    if(check_f_modal_m(&(b_ptr->child[i]->mrph_ptr[m_num - 1]))) {
			flag_1 = 1 ;
			break;
		    } else {
			continue;
		    }
		}

		/* ����ʸ��(��)��Ƭ�� Modality ���ɤ���Ĵ�٤Ƥ��� */
		/*------------------------------------------------
		 *
		 *  (flag_2 == 1) ��    ������������
		 *                                ��������
		 *
		 *-----------------------------------------------*/	    
		flag_2 = 0;
		if (b_ptr->parent &&
		    check_f_modal_ISHI(b_ptr->parent->mrph_ptr)){
		    flag_2 = 1;
		}

		/* 
		 *ʸ�������� Modality-�ջ� ����, ����˲���Ĥ��ʤ��Ȥ�����, 
		 * ɮ�Ԥȿ��ꤹ�� 
		 */
		if (j ==0 && flag_1 == 0 && flag_2 == 0){ 
		    assign_GA2pred(b_ptr, NULL, "Modality�ݻ���", local_sp);
		    goto Match;
		}

		/* ����Modality�Ǥ�, �ջפǤʤ��Ȥ���, ���⤷�ʤ� */
		goto Match;
	    }

	    /* ������Modality�ǤϤʤ��Ȥ� */
	    else {
		m_flag_include_ISHI = 0;
		/* Modality-�ջ� ��ޤफ�ɤ�����Ƚ�� */
		for (j = b_ptr->mrph_num; j > 0 ; j--){
		    if(check_f_modal_ISHI(&(b_ptr->mrph_ptr[j-1]))){
			m_flag_include_ISHI = 1;
			break;
		    }
		    else {
			continue;
		    }
		}

		/* Modality-�ջ� ��ޤ�Ǥ���� */
		if(m_flag_include_ISHI == 1){
		    assign_GA2pred(b_ptr, NULL, "Modality�ݻ���", local_sp);
		    goto Match;
		}
	    }
	}



	/*===== �����Ϣ�ν��� =====*/
	if (check_feature(b_ptr->f, "�����Ϣ�Ѹ�")){
	    for (i = 0; b_ptr->child[i]; i++){
		if(check_feature(b_ptr->child[i]->f, "����") &&
		   check_feature(b_ptr->child[i]->f, "���δط�")){
		    assign_GA2pred(b_ptr, NULL, "����", local_sp);
		    goto Match;
		}
	    }
	}


	/*----------------------------*/
        /*    ������������Ƚ���    */
	/*----------------------------*/


	/*===== �ޤ��Ҷ���ߤ� =====*/

        /*
	 * ��¹��"��:̤��", "��:����"����äƤ���Τ����
	 *  if (check_primary_child(b_ptr, b_ptr, local_sp)) goto Match; 
	 */


	for (i = 0; b_ptr->child[i]; i++) {

	    /* ���̤λҶ��ΤȤ� */
	    if (b_ptr->child[i]->para_top_p != TRUE){
		/*�֡�(����|����)+��+(�Ѹ�)�פ��ä��鲿�⤷�ʤ���¾�λҶ��򸫤�    */
		/* (��) �ֹ����񤬣���������������Ƥ���Τ���Ω�ġ��� */
		/* (��) �ֺ�ǯ�����ɤΥ����ޥ�ν��ͤ�����Ǥߤ����� */
		if ((check_feature(b_ptr->f, "���") &&
		     check_feature(b_ptr->child[i]->f, "����")&&
		     check_feature(b_ptr->child[i]->f, "��")) ||
		    (check_feature(b_ptr->child[i]->f, "����") &&
		     check_feature(b_ptr->child[i]->f, "��"))){
		    ;
		}
		
		else if (check_feature(b_ptr->child[i]->f, "��:����")){
		    assign_GA2pred(b_ptr, b_ptr->child[i], "����", local_sp);
		    goto Match;
		    
		} 
		else if (check_feature(b_ptr->child[i]->f, "��:̤��")){
		    assign_GA2pred(b_ptr, b_ptr->child[i], "̤��", local_sp);
		    goto Match;
		}
	    }

	}


	if (check_feature(b_ptr->f, "��:Ϣ��") &&
	    /*
	     * !check_feature(b_ptr->f, "Modality-����") &&
	     */

	    b_ptr->parent &&
	    b_ptr->parent->para_top_p != TRUE &&
	    (!check_feature(b_ptr->f, "����") || 
	     check_feature(b_ptr->f, "�����") ||
	     !check_feature(b_ptr->parent->f, "��:�ǳ�")) &&

	    !check_feature(b_ptr->parent->f, "���δط�") &&
	    !check_feature(b_ptr->parent->f, "���̥�")) {

	    pos_bptr = skip_para_top(b_ptr->parent,"pa");
	    assign_GA2pred(b_ptr, pos_bptr, "������", local_sp);
	    goto Match;
	}


	/*===== Ϣ�ѽ�����Υ��� =====*/

	if (check_feature(b_ptr->f, "��:Ϣ��") &&
	    b_ptr->parent &&
	    b_ptr->parent->para_top_p != TRUE &&
	    check_feature(b_ptr->parent->f, "C���ʿ���")) {

	    assign_GA2pred(b_ptr, b_ptr->parent, "���Ѹ�", local_sp);
	    goto Match;
	}
	    /*----------------------------------------------------
	     *  strlen("C���ʿ���:")
	     *   |<------------->|
	     *   -------------------------------------------
	     *   |C|��|��|��|��|:| | |...............| | | |
	     *   -------------------------------------------
	     *  ��              �� 
	     *  cp          cp + strlen("C���ʿ���:")
	     *---------------------------------------------------*/


	/* Feb/1st */

	/* �ޤ�, ���ߤ��Ѹ����ɤγʥե졼�༭��Υǡ����ʤΤ������ꤹ�� */
#if 0
	if (tm->cpm->pred_num == 0){
	    printf("�ڡۤʤ�\n");
	    printf("b_ptr�ǡ��� %s\n", b_ptr->Jiritu_Go);
	}
	else {
	    for(i = 0; tm->cpm->pred_b_ptr[i] ; i++){
		j = b_ptr - tm->cpm->pred_b_ptr[i];
		if (j == 0){
		    printf("����ǡ���ȯ�� \n");
		    printf("b_ptr�ǡ��� : %s\n", b_ptr->Jiritu_Go);
		    printf("�ʥե졼�༭��ǡ��� : %s\n", tm->cpm->pred_b_ptr[i]->Jiritu_Go);
		    break;
		}
	    }
	    printf("�ʥե졼�༭����Ͽ�ʤ�\n");
	    printf("b_ptr�ǡ��� %s\n", b_ptr->Jiritu_Go);
	}
#endif

	for (i = 0; i < tm->pred_num; i++){
	    cpm_ptr = &(tm->cpm[i]);

	/* �ʤν��� */
	    for (k = 0; k < cpm_ptr->cf.element_num; k++){

		printf("������ : ");
		for (j = 0; j < cpm_ptr->elem_b_ptr[k]->mrph_num; j++){
		    printf(" %s", (cpm_ptr->elem_b_ptr[k]->mrph_ptr + j)->Goi2);
		}
		printf("\n");

		if ((cp = (char *)check_feature(cpm_ptr->elem_b_ptr[k]->f, "��"))
		    != NULL) {
		    if (cpm_ptr->cf.pp[k][0] < 0) {
			printf("(%s)��\n", cp + strlen("��:"));
		    }
		    else {
			printf("(%s)��\n", pp_code_to_kstr(cpm_ptr->cf.pp[k][0]));
		    }
		}
	    }
	}
	/* ���β��ϤΤ���˽�������Ƥ��� */
	tm->pred_num = 0;



#if 0
	/*===== ��ȽϢ�Τޤ���ư��Ϣ�Τϡ��Ҥ��ƤΡ֡��Ρפ�õ�� =====*/

	/* �� �Ƥ�, "��:Ϣ��"�ν������н� */
	/* (��) �ָ�������Ϣ�θ�ġ�     */

	/* �� �Ҷ� */
	/* (��) ���Ƿ���礭������������ */
	for (i = 0; b_ptr->child[i]; i++) {
	    m_num = b_ptr->child[i]->mrph_num;
	    if((check_feature(b_ptr->f, "ID:�ʷ�ȽϢ�Ρ�") ||
		check_feature(b_ptr->f, "ID:��ư��Ϣ�Ρ�")) &&
	        m_num > 0 &&
	       (strcmp(NO, b_ptr->child[i]->mrph_ptr[m_num-1].Goi) == 0) &&
	        check_feature(b_ptr->child[i]->f, "�θ�")){

		pos_bptr = skip_para_top(b_ptr->child[i], "ch");
		assign_GA2pred(b_ptr, pos_bptr, "������(A��B)", local_sp);
		goto Match;
	    }
	}
#endif

	/*===== �����褬Ʊ��ư��ܷ��ƻ���¤ӤΤȤ� =====*/

/* 0 -> comment*/
#if 0 
	if (check_feature(b_ptr->f, "�Ѹ�:ư") &&
	    (!check_feature(b_ptr->f, "��ʸ��")) &&
	    b_ptr->parent){
	    s = position_in_children(b_ptr);
	    if ((s > 0) &&
	        check_feature(b_ptr->parent->child[s-1]->f, "�Ѹ�:��") &&
		b_ptr->child[0]){

		for(i = 0; b_ptr->child[i]; i++){
		    GA_detection(b_ptr->child[i]);
		}
		for(i = 0; b_ptr->child[i]; i++){
		    if (check_feature(b_ptr->child[i]->f, "C����̤ȯ��") ||
			check_feature(b_ptr->child[i]->f, "C���ʿ���:��ʸ") ||
			(check_feature(b_ptr->child[i]->f, "C���ʿ���") == NULL)){

			assign_GA2pred(b_ptr, "[ɮ��]", "��ά����", local_sp);
			goto Match;
			}

		    else if (check_feature(b_ptr->child[i]->f, "C���ʿ���")){
			/* 
			 *  "C���ʿ���" �� feature ����äƤ��� b_ptr �ˤ�, 
			 *  skip_para_top ����ɬ�פϤʤ�.
			 */
			assign_GA2pred(b_ptr, b_ptr->child[i], "������(ư�ܷ�)", local_sp);
			goto Match;
		    }
		}
	    }
	}
	    /*----------------------------------------------------
	     *  (����ʤȤ�)
	     *
             *       �ɤ�����������������
             *         ��  ͷ�����������  <C���ʿ���:��ά����:ɮ��>
             *               �Ť���������<C���ʿ���:������:��>
             *                         ��
	     *---------------------------------------------------*/
#endif

	/*
	 * if (b_ptr->para_type == PARA_NORMAL) {
	 *    for (i = 0; b_ptr->parent->child[i]; i++) {
	 * 	if (b_ptr->parent->child[i]->para_top_p != TRUE &&
	 *	    b_ptr->parent->child[i]->para_type == PARA_NIL &&
	 *	    check_feature(b_ptr->parent->child[i]->f, "��:����")) {
	 *
	 *	    assign_GA2pred(b_ptr, b_ptr->parent->child[i], "����", local_sp);
	 *	    goto Match;
	 *	}
	 *	else if (b_ptr->parent->child[i]->para_top_p != TRUE &&
	 *		 b_ptr->parent->child[i]->para_type == PARA_NIL &&
	 *		 check_feature(b_ptr->parent->child[i]->f, "��:̤��")) {
	 *	    assign_GA2pred(b_ptr, b_ptr->parent->child[i], "̤��", local_sp);
	 *	    goto Match;
	 *	}
	 *	
	 *    }		
	 *  }
	 */



	/*===== Ϣ�ν��������ߤ� =====*/

#if 0
/* �ʲ��Ϥη�̤��Ѥ��� */

	m_num = b_ptr->mrph_num;
	if (check_feature(b_ptr->f, "��:Ϣ��") &&
	    check_feature(b_ptr->f, "�Ѹ�:ư") &&
	    !check_feature(b_ptr->f, "����") &&
	    b_ptr->parent &&
	    check_feature(b_ptr->parent->f, "��:���") &&
	    b_ptr->parent->parent &&
	    check_feature(b_ptr->parent->parent->f, "C���ʿ���")) { 

		assign_GA2pred(b_ptr, b_ptr->parent->parent, "���Ѹ�", local_sp);
		goto Match;
	}
	    /*----------------------------------------------------
	     * (����ʤȤ�)
	     *
             *                        HOGE��   
             *                     ��������
             *    �Ѹ�:ư(��ʬ)����      �� <C���ʿ���:������:HOGE>
             *                   ������������
             *                ��:��ʨ�������
             *                           C  <C���ʿ���:̤��:HOGE>
	     *
             * ��)��¢�ʤ�ǧ������ˤ��᤿
	     *--------------------------------------------------*/


	if (check_feature(b_ptr->f, "��:Ϣ��") &&
	    b_ptr->para_type != PARA_NORMAL &&
	    b_ptr->parent &&
	    b_ptr->parent->para_top_p == TRUE &&
	    b_ptr->parent->parent &&
	    b_ptr->parent->parent->para_top_p != TRUE &&
	    !check_feature(b_ptr->parent->parent->f, "���δط�")&&
	    check_feature(b_ptr->parent->parent->f, "C���ʿ���")){

	    assign_GA2pred(b_ptr, b_ptr->parent->parent, "���Ѹ�", local_sp);
	    goto Match;
	}
	    /*----------------------------------------------------
	     * (����ʤȤ�)
	     *
             *           �����֤�������     ��������   <C���ʿ���:��������>
             *                       ����������
             *   ���ȤΤʤ��褦 <P>����      ��
             *                       ������������
             *           ����� <P>��PARA��������
             *                          ���Ԥ����� <C���ʿ���:̤��:��������>
	     *
             * ��)�����ǧ�᤿��ΤΡ��������ϴ��ɤˤ����դ��Ȥ��ä�
	     *    ���֤򷫤��֤����ȤΤʤ��褦���ط��Ԥ��������Ԥ�
	     *    ������
	     *--------------------------------------------------*/


	if (check_feature(b_ptr->f, "��:Ϣ��") &&
	    b_ptr->parent &&
	    b_ptr->parent->para_top_p == TRUE &&
	    b_ptr->parent->parent &&
	    b_ptr->parent->parent->para_top_p != TRUE &&
	    !check_feature(b_ptr->parent->parent->f, "���δط�") &&
	    (check_feature(b_ptr->parent->parent->f, "��:����") ||
	     check_feature(b_ptr->parent->parent->f, "��:̤��"))) {

	    pos_bptr = skip_para_top(b_ptr->parent->parent, "pa");
	    assign_GA2pred(b_ptr, pos_bptr, "������", local_sp);
	    goto Match;
	}
	    /*----------------------------------------------------
	     * (����ʤȤ�)
	     *
             *     ��ʬ������     
             *             ��������
             *      A <P>����      
             *             ����������
             *      B <P>��PARA��������
             *                    HOGE(Ϣ��)�� 
	     *
	     * ��) �İ��ϡ������פǤϤʤ�����ɽ�פ��ȻפäƤʤ���
	    ----------------------------------------------------*/


	if (check_feature(b_ptr->f, "��:Ϣ��") &&
	    b_ptr->parent &&
	    b_ptr->parent->para_top_p == TRUE &&
	    b_ptr->parent->parent &&
	    b_ptr->parent->parent->para_top_p != TRUE &&
	    check_feature(b_ptr->parent->parent->f, "���δط�") &&
	    check_feature(b_ptr->parent->parent->f, "��:Ϣ��")&&
	    b_ptr->parent->parent->parent &&
	    check_feature(b_ptr->parent->parent->parent->f, "C���ʿ���")) {

	    assign_GA2pred(b_ptr, b_ptr->parent->parent->parent, "���Ѹ�", local_sp);
	    goto Match;
	}


	if (check_feature(b_ptr->f, "��:Ϣ��") && 
	    b_ptr->parent &&
	    check_feature(b_ptr->parent->f, "���δط�") &&
	    check_feature(b_ptr->parent->f, "��:�˳�") &&
	    b_ptr->parent->parent &&
	    check_feature(b_ptr->parent->parent->f, "C���ʿ���")){

	    assign_GA2pred(b_ptr, b_ptr->parent->parent, "���Ѹ�", local_sp);
	    goto Match;
	}

	    /*----------------------------------------------------
	     * (����ʤȤ�)
	     *                    HOGE��
	     *                     ��          
	     *   �Ѹ�A(��ʬ)����   ��    <C���ʿ���:���Ѹ�:HOGE��>
	     *               ���˨���
	     *                     �Ѹ�B <C���ʿ���:̤��:HOGE��>
	     *
	     * ��)���ܿ�����,�бĺƷ���ޤ뤳�Ȥˤʤä�
	     *---------------------------------------------------*/


	/*===== "Modality-����" �ΤȤ��Ͻ���Ѥ�. ������ =====*/
	m_num = b_ptr->mrph_num;
	if (check_feature(b_ptr->f, "��:Ϣ��") &&
	    b_ptr->parent &&
	    (strcmp(NO, b_ptr->parent->mrph_ptr[m_num-1].Goi) == 0) &&
	    b_ptr->parent->mrph_ptr[m_num-1].Bunrui == 3 &&  /* ʬ���ֹ� 3 ����³����  */
	    b_ptr->parent->parent &&
	    check_feature(b_ptr->parent->parent->f, "�θ�")){

	    pos_bptr = skip_para_top(b_ptr->parent->parent,"pa");
	    assign_GA2pred(b_ptr, pos_bptr, "������(Jump Indec+Conj)", local_sp);
	    goto Match;
	}
	    /*----------------------------------------------------
	     * (����ʤȤ�)
	     *
	     *      �ͤ��ʤ��ä�����������������
	     *                    ���ɤΨ������� �֤�(��³����)�פʤ餽����
	     *                             ����
	     *
	     * ��)�ͤ��ʤ��ä����ɤα���
	     *---------------------------------------------------*/





	if (check_feature(b_ptr->f, "��:Ϣ��") &&
	    /*
	     * !check_feature(b_ptr->f, "Modality-����") &&
	     */

	    b_ptr->parent &&
	    b_ptr->parent->para_top_p != TRUE &&
	    (!check_feature(b_ptr->f, "����") || 
	     check_feature(b_ptr->f, "�����") ||
	     !check_feature(b_ptr->parent->f, "��:�ǳ�")) &&

	    !check_feature(b_ptr->parent->f, "���δط�") &&
	    !check_feature(b_ptr->parent->f, "���̥�")) {

	    pos_bptr = skip_para_top(b_ptr->parent,"pa");
	    assign_GA2pred(b_ptr, pos_bptr, "������", local_sp);
	    goto Match;
	}




	/* ���ν���, Ϣ�ʤ����������Τ���������Τ�,,, */
	/*===== PARA�˷��äƤ�����ʬ =====*/

	if(b_ptr->para_type == PARA_NORMAL &&
	   b_ptr->parent->para_top_p == TRUE &&
	   ((pos_bptr = 
	    check_posterity(b_ptr->parent, (s = position_in_children(b_ptr)+1))) != NULL) &&
	   !check_feature(pos_bptr->f, "���δط�")){

	    assign_GA2pred(b_ptr, pos_bptr, "PARA������", local_sp);
	    goto Match;
	}
	    /*-----------------------------------------------------------------------------
	     * (����ʤȤ�)
	     *
  	     *          �⸢����<P>����������������������������
             *      ����ͶƳ������<P>��PARA����������������������
             *        ��ä����ɤ�����<P>��������������������<- PARA������:����ͶƳ������
             *              ���ؤޤ�䤹��<P>��PARA       ������<- PARA������:����ͶƳ������
	     *
	     * 
	     * �⸢��������ͶƳ�����ϲ�ä����ɤ��������ؤޤ�䤹��
	     *-----------------------------------------------------------------------------*/


	/*===== Ϣ�ѽ�����Υ��� =====*/

	if (check_feature(b_ptr->f, "��:Ϣ��") &&
	    b_ptr->parent &&
	    b_ptr->parent->para_top_p != TRUE &&
	    check_feature(b_ptr->parent->f, "C���ʿ���")) {

	    assign_GA2pred(b_ptr, b_ptr->parent, "���Ѹ�", local_sp);
	    goto Match;
	}
	    /*----------------------------------------------------
	     *  strlen("C���ʿ���:")
	     *   |<------------->|
	     *   -------------------------------------------
	     *   |C|��|��|��|��|:| | |...............| | | |
	     *   -------------------------------------------
	     *  ��              �� 
	     *  cp          cp + strlen("C���ʿ���:")
	     *---------------------------------------------------*/




	/*===== �󤯤ο�(����) =====*/

	if (b_ptr->parent &&
	    check_feature(b_ptr->parent->f, "��:�γ�") &&
	    check_feature(b_ptr->parent->f, "�θ�") &&
	    b_ptr->parent->parent &&
	    b_ptr->parent->parent->para_top_p != TRUE &&
	    check_feature(b_ptr->parent->parent->f, "��:�γ�") &&
	    check_feature(b_ptr->parent->parent->f, "�θ�")){
	    
	    pos_bptr = skip_para_top(b_ptr->parent->parent,"pa");
	    assign_GA2pred(b_ptr, pos_bptr, "������(GrandParent)", local_sp);
	    goto Match;
	}
	    /* (��) �ɤ�������ʣ���ʵ��������� */
	    /*----------------------------------------------------
             *   �ɤ�먬����������������������<C���ʿ���:������:���>
             *           ����Ψ���������������
             *           ʣ���ʨ���������������<C���ʿ���:������:����>
             *                   �����Ψ�������
             *                            ����
	     *----------------------------------------------------*/


	if (check_feature(b_ptr->f, "ID:���ȡʤ�����") &&
	    b_ptr->parent->parent &&
	    check_feature(b_ptr->parent->parent->f, "��:�γ�") &&
	    check_feature(b_ptr->parent->parent->f, "�θ�") &&
	    b_ptr->parent->parent->parent &&
	    check_feature(b_ptr->parent->parent->parent->f, "��:�γ�") &&
	    check_feature(b_ptr->parent->parent->parent->f, "�θ�")){

	    pos_bptr = skip_para_top(b_ptr->parent->parent->parent,"pa");
	    assign_GA2pred(b_ptr, pos_bptr, "������(GrandParent)", local_sp);
	    goto Match;
	}
	    /* (��) ����������Ȥ���������������Τ����� */
	    /*----------------------------------------------------
             *  ����������Ȩ�����������<C���ʿ���:������(GrandParent):������>
             *                  ����������������������������
             *                      ������Ψ���������������
             *                              �����ΤΨ�������
             *                                         ����
	     *----------------------------------------------------*/


	/*===== ��:Ϣ��, ��:Ϣ�� �ɤ���Ǥ�褤 =====*/
	if(check_feature(b_ptr->f, "ID:���ʤ����")&&
	   check_feature(b_ptr->f, "��٥�:B+") &&
	   b_ptr->parent &&
	   check_feature(b_ptr->parent->f, "���δط�") &&
	   check_feature(b_ptr->parent->f, "��:Ϣ��") &&
	   check_feature(b_ptr->parent->f, "ID:������") &&
	   check_feature(b_ptr->parent->f, "��٥�:B+") &&
	   b_ptr->parent->parent &&
	   check_feature(b_ptr->parent->parent->f, "�Ѹ�:ư") &&
	   check_feature(b_ptr->parent->parent->f, "C���ʿ���")){

	    assign_GA2pred(b_ptr, b_ptr->parent->parent, "���Ѹ�", local_sp);
	    goto Match;
	}

	    /*----------------------------------------------------
	     * (����ʤȤ�)
	     *                             HOGE��
	     *                              ��
             * (��٥�B+) A���먬������     ����<C���ʿ���:���Ѹ�:HOGE�ϡ�>
             *       (��٥�B+)   ���ᡢ��������
             *      ��������������������    ����
          �� *����                   B�򨡨�����
	     *                              ��
             *                             C���� <C���ʿ���:̤��:HOGE�ϡ�>
	     *
	     * ��)��¢�ʤϡ����ɺĸ��ν�����¥�ʤ��뤿�ᡢ����α�ݶ�
	     *    �μ��������ǧ������ˤ��᤿
	     *---------------------------------------------------*/




	/*===== ��:Ϣ�� �Ǥ� ��:Ϣ�� �Ǥ�ʤ�����, ���Ѹ�=====*/
	if(check_feature(b_ptr->f, "��") &&
	   b_ptr->parent &&
	   check_feature(b_ptr->parent->f, "C���ʿ���")){

		assign_GA2pred(b_ptr, b_ptr->parent, "���Ѹ�", local_sp);
		goto Match;
	}
	    /* (��) ��������줿���Ȼ��ְ�Ҥ���񤷤Ƥ�����*/
	    /*----------------------------------------------------
	     *  ���� ����������������
	     *      ����줿���Ȩ�������<C���ʿ���:���Ѹ�:���ְ�Ҥ�>
	     *        ���ְ�Ҥ���������
             *            ��񤷤Ƥ�����<C���ʿ���:����:���ְ�Ҥ�>
	     *----------------------------------------------------*/


	/*===== �⤦���� PARA �����å� =====*/

	/*
	 * if ((b_ptr->para_type == PARA_NORMAL) &&
	 *    b_ptr->parent &&
	 *    (b_ptr->parent->para_top_p == TRUE) &&
	 *    (b_ptr->parent->parent == FALSE)){
	 */
	 
	if ((b_ptr->para_type == PARA_NORMAL) &&
	     b_ptr->parent &&
	    (b_ptr->parent->para_top_p == TRUE)){
	    /*����Υ��ʤ���äƤ�����*/
	    s = position_in_children(b_ptr);
	    if ((s >= 0) &&
		b_ptr->parent->child[s+1]){
		GA_detection(b_ptr->parent->child[s+1]);
		for(i = s+1; b_ptr->parent->child[i]; i++){
		    if ((cp = (char *)check_feature(b_ptr->parent->child[i]->f, "C���ʿ���")) != NULL){

			assign_GA2pred(b_ptr, b_ptr->parent->child[i], "PARA��", local_sp);
			goto Match;
		    }
		}
	    }
	}
	    /* (��) �졼��������������ܤ����ꡢ�����������¤�ľ�뤻�����
	     *  ���ʤ��ʤä��� */
	    /*-------------------------------------------------------------------
	     *       �졼���������⨡��������������
             *             ����ܤ˨���������������
             *                      ���ꡢ<P>������<C���ʿ���:̤��:�졼��������>
             *     ��������������������������������
             *               ���¤򨡨�������������
             *  ľ�뤻����򤨤ʤ��ʤä���<P>��PARA<C���ʿ���:PARA��:̤��:�졼��������>
	     *-------------------------------------------------------------------*/

/* �ʲ��Ϥη�̤��Ѥ���٤�. �����ޤ� */
#endif


#if 0
/* 
 * ��¹��"��:̤��", "��:����"����äƤ���Τ�, ����ä����٤������Τ�,
 * ������α. �ץ�����ư��Ȥ��Ƥ�, ����ư��.
 */
	/*===== ������������, ��¹��õ���Ƥߤ褦 =====*/

	if ((pos_bptr = check_posterity(b_ptr, 0)) != NULL) {

	    assign_GA2pred(b_ptr, pos_bptr, "��¹", local_sp);
	    goto Match;
	}
#endif


#if 0 /* �����ν�����ʲ��Ϥη�̤��Ѥ�������פˤʤ� */

	/*===== ������������, ���Ĥ�õ���Ƥߤ褦 =====*/

	/*
	 * (NOTE)
	 * ���ν������ܼ�Ū���������Ȥϸ����ʤ���
	 * �����������ν���������������Ψ�Ͼ夬�롣
	 * �����褬���������Ϥ���Ƥ���С������������ȸ����뤫��
	 * ����ʤ�����������
	 */

	/*
	 * (���르�ꥺ��)
	 *
	 * �ޤ�, �Ƥ����ꥹ�Ȥ���.��ʬ��ľ�Ƥ���, �Ǹ������ޤ�. 
	 * (�Ǹ�Ϻ��ä��ˤʤ�?)
	 *
	 * ����, ���Υꥹ�Ȥγ�����(��, ����)�ˤĤ���, ��ʬ���
	 * ��¦�λҶ�(������, �Ĥޤ���ʳ��Τ�)��, �ϳʤޤ��ϥ���
	 * (���Τ�, "��" or "��"�ǽ����ϳ� or ����)����äƤ��ʤ�
	 * ��������å�.
	 *
	 * �����, ����򥬳ʸ���Ȥ���.
	 *
	 * 
	 */

	if ((pos_bptr = check_ancestor(b_ptr)) != NULL){
	    assign_GA2pred(b_ptr, pos_bptr, "����", local_sp);
	    goto Match;
	}
#endif


	/*===== ��������ʤ�����ʸ =====*/

	if (sp->Sen_num >= 2){
	    prev_sp = sentence_data + (sp->Sen_num - 2); 
	    if (prev_sp->Bnst_num >= 1 ){
		if ((cp = check_feature(((prev_sp->bnst_data) + prev_sp->Bnst_num - 1)->f,"C���ʿ���")) != NULL) {

		    assign_GA2pred(b_ptr, ((prev_sp->bnst_data) + prev_sp->Bnst_num - 1) , "��ʸ", local_sp);
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

/*sentence_data �����դ�뤫, ʸ�Ϥ��Ѥ�ä���, sentence_data��clear*/
/* �ºݤ�Ƚ���, read_data.c ����Ǥ�ä�, ��������ƤӽФ��Ƥ���   */

{
    int i;
    SENTENCE_DATA *stock_sp_1, *stock_sp_2;


    /* sentence_data �����դ줽���ʤȤ� (sp->Sen_num ==256)*/
    if (f == 0){  

	/* �Ǹ��7ʸ(�ʹ֤�û�������ޥ��å��ʥ�С�) �Υǡ����ϻĤ�*/
	for(i = 6; i >= 0; i--){
	    free(sentence_data - (249 + i));
	    stock_sp_1 = sentence_data - i;
	    stock_sp_2 = sentence_data - (249 + i);
	    stock_sp_2 = stock_sp_1;
	}
	for( i = 0; i <= 249; i++){
	    free(sentence_data - i);
	}
	sp->Sen_num = 7;
    } 

    /* S_ID ���Ѥ�ä� */
    else if (f == 1){
	for(i = 0; i < sp->Sen_num ; i++){
	    free(sentence_data -i);
/*
 *	for(i = sp->Sen_num; i > 0 ; i--){
 *	    printf("Sentence Data : %s", sentence_data - i);
 *	    free(sentence_data -i);
 */
	}

	    sp->Sen_num = 0;
    }
    return;
}


/*==================================================================*/
		      void discourse_analysis()
/*==================================================================*/
{

    GA_detection(sp->bnst_data + (sp->Bnst_num -1));
    
    copy_sentence();
}


/*===== �ʲ��Ϥη�̤򥬳ʿ�����Ѥ���Ȥ��Υ��르�ꥺ��� =====*/

/*
 * �ʲ� �ֳʲ��ϡפ��C��פ�ά��
 *
 * �����Ѹ����Ф���, C��η�̤�Τ���
 * 
 * (I)C��η�̤����, �Ĥޤ� Best_mgr->cpm->result_num == 1 �ΤȤ�
 *
 *    (I-i)
 *     ���ʤΥ���ȥ꡼�˲��������, ����򥬳ʸ���Ȥ���
 *
 *    (I-ii)
 *    �ʤ����, Ʊ��ʸ��Υ���,̤�ʤΤ�ΤˤĤ���, �ʲ��ϤΥ��ʤ�
 *    ����ȥ꡼��Ȥ�����٤�׻�����, ���������ʾ�ǺǤ��������⤤
 *    ��Τ򥬳ʸ���Ȥ���.
 *
 *    (I-iii)
 *    Ʊ��ʸ��ˤʤ���С���ʸ���Ф���(I-ii)��Ʊ�����Ȥ�Ԥ�.
 *    (�ɤΤ��餤���ʸ�ޤǸ���?)
 *   
 * (II)C��η�̤���İʾ�, �Ĥޤ� Best_mgr->cpm->result_num > 1 �ΤȤ�
 *
 *    (II-i)
 *     �Ȥꤢ����, �ƹ��ܤΥ��ʤΥ���ȥ꡼�򸫤�.
 *     ����Ʊ��, �ޤ���¿���ɤʤ�, ����򥬳ʤˤ���.
 *     ��������ʤ����, ��Ȥ�����٤��⤤��Τˤ��뤷���ʤ�����??
 * 
 *    (II-ii)
 *     �ʤ����, (I-ii)->(I-iii)��¹Ԥ���.
 *
 * 
 */



/*====================================================================
                               END
====================================================================*/
