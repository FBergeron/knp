/*====================================================================

		      unsupervised learning ��Ϣ

                                             S.Kurohashi 1999. 4. 1

    $Id$
====================================================================*/
#include "knp.h"

void CheckCandidates(SENTENCE_DATA *sp)
{
    int i, j;
    TOTAL_MGR *tm = &Best_mgr;
    char buffer[DATA_LEN], buffer2[256], *cp;

    /* ��ʸ�ᤴ�Ȥ˥����å��Ѥ� feature ��Ϳ���� */
    for (i = 0; i < sp->Bnst_num; i++)
	if (tm->dpnd.check[i].num != -1) {
	    /* ����¦ -> ������ */
	    sprintf(buffer, "����");
	    for (j = 0; j < tm->dpnd.check[i].num; j++) {

		/* �ƥ��� -check -optionalcase �ǳ� */
		if (OptOptionalCase) {
		    if ((cp = (char *)check_feature(sp->bnst_data[i].f, "��")) != NULL) {
			if (str_eq(cp+3, OptOptionalCase)) {
			    corpus_optional_case_comp(sp, sp->bnst_data+i, cp+3, sp->bnst_data[tm->dpnd.check[i].pos[j]], NULL);
			}
		    }
		}

		/* ����ɤ� */
		sprintf(buffer2, ":%d", tm->dpnd.check[i].pos[j]);
		if (strlen(buffer)+strlen(buffer2) >= DATA_LEN) {
		    fprintf(stderr, "Too long string <%s> (%d) in CheckCandidates.\n", buffer, tm->dpnd.check[i].num);
		    return;
		}
		strcat(buffer, buffer2);
	    }
	    assign_cfeature(&(sp->bnst_data[i].f), buffer);
	}
}
