/*====================================================================

		      unsupervised learning ��Ϣ

                                             S.Kurohashi 1999. 4. 1

    $Id$
====================================================================*/
#include "knp.h"

/* Server Client extention */
extern FILE  *Infp;
extern FILE  *Outfp;
extern int   OptMode;

void CheckCandidates() {
    int i, j;
    TOTAL_MGR *tm = &Best_mgr;
    char buffer[256], buffer2[256];

    /* ��ʸ�ᤴ�Ȥ˥����å��Ѥ� feature ��Ϳ���� */
    for (i = 0; i < Bnst_num; i++)
	if (tm->dpnd.check[i].num != -1) {
	    /* ����¦ -> ������ */
	    sprintf(buffer, "Check:%2d %2d", i, tm->dpnd.head[i]);
	    for (j = 0; j < tm->dpnd.check[i].num; j++) {
		/* ����ɤ� */
		sprintf(buffer2, ":%d", tm->dpnd.check[i].pos[j]);
		strcat(buffer, buffer2);
	    }
	    assign_cfeature(&(bnst_data[i].f), buffer);
	}
}
