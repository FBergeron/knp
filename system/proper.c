/*====================================================================

			     ��ͭ̾�����

                                               S.Kurohashi 96. 7. 4

    $Id$
====================================================================*/
#include "knp.h"

/*==================================================================*/
		   char *NEcheck(BNST_DATA *b_ptr)
/*==================================================================*/
{
    char *class;

    if (!(class = check_feature(b_ptr->f, "��̾")) && 
	!(class = check_feature(b_ptr->f, "��̾")) && 
	!(class = check_feature(b_ptr->f, "�ȿ�̾")) && 
	!(class = check_feature(b_ptr->f, "��ͭ̾��"))) {
	return NULL;
    }
    return class;
}

/*==================================================================*/
	 int NEparaCheck(BNST_DATA *b_ptr, BNST_DATA *h_ptr)
/*==================================================================*/
{
    char *class, str[11];

    /* b_ptr ����ͭɽ���Ǥ��� */
    if ((class = NEcheck(b_ptr)) == NULL) {
	return FALSE;
    }

    sprintf(str, "%s��", class);

    /* h_ptr �ϸ�ͭɽ���ǤϤʤ�
       class �Ȱ��פ���֡����פ��Ĥ��Ƥ��� */
    if (NEcheck(h_ptr) || 
	!check_feature(h_ptr->f, str)) {
	return FALSE;
    }

    /* h_ptr ��Ʊ����ͭɽ�����饹�ˤ��� */
    assign_cfeature(&(h_ptr->f), class);
    assign_cfeature(&(h_ptr->f), "������OK");

    return TRUE;
}

/*==================================================================*/
	       void ne_para_analysis(SENTENCE_DATA *sp)
/*==================================================================*/
{
    int i, value;
    char *cp;

    for (i = 0; i < sp->Bnst_num; i++) {
	/* ���꤬��������Ϥ��Ƥ��� */
	if (check_feature(sp->bnst_data[i].f, "����") || 
	    !check_feature(sp->bnst_data[i].f, "�θ�") || 
	    (sp->bnst_data[i].dpnd_head > 0 && 
	     !check_feature(sp->bnst_data[sp->bnst_data[i].dpnd_head].f, "�θ�"))) {
	    continue;
	}
	cp = check_feature(sp->bnst_data[i].f, "�·���");
	if (cp) {
	    value = atoi(cp+strlen("�·���:"));
	    if (value > 2) {
		/* ����¦��Ĵ�٤Ƹ�ͭ̾�줸��ʤ��ä��顢����¦��Ĵ�٤� */
		if (NEparaCheck(&(sp->bnst_data[i]), &(sp->bnst_data[sp->bnst_data[i].dpnd_head])) == FALSE)
		    NEparaCheck(&(sp->bnst_data[sp->bnst_data[i].dpnd_head]), &(sp->bnst_data[i]));
		continue;
	    }
	}

	cp = check_feature(sp->bnst_data[i].f, "�·�ʸ���");
	if (cp) {
	    value = atoi(cp+strlen("�·�ʸ���:"));
	    if (value > 1) {
		if (NEparaCheck(&(sp->bnst_data[i]), &(sp->bnst_data[sp->bnst_data[i].dpnd_head])) == FALSE)
		    NEparaCheck(&(sp->bnst_data[sp->bnst_data[i].dpnd_head]), &(sp->bnst_data[i]));
	    }
	}
    }
}

/*====================================================================
                               END
====================================================================*/
