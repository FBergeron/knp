/*====================================================================

			      呼応の処理

                                               S.Kurohashi 1995. 7. 4
                                               S.Ozaki     1995. 2. 8

    $Id$
====================================================================*/
#include "knp.h"

int 	Koou_matrix[BNST_MAX][BNST_MAX];
int	koou_m_p[BNST_MAX];

/*==================================================================*/
                         void init_koou() 
/*==================================================================*/
{
    int i ,j;

    for (i = 0; i < Bnst_num; i++) {
	koou_m_p[i] = FALSE;
	for (j = 0; j < Bnst_num; j++)
	    Koou_matrix[i][j] = 0;
    }
}

/*==================================================================*/
                         int check_koou() 
/*==================================================================*/
{
    int		i, j, k, flag;
    BNST_DATA   *b_ptr, *c_ptr;
    KoouRule 	*r_ptr;

    flag = FALSE;

    for (i = 0, b_ptr = bnst_data; i < Bnst_num; i++, b_ptr++) {
	for (j = 0, r_ptr = KoouRuleArray; j < CurKoouRuleSize; j++, r_ptr++) {
	    if (_regexpbnst_match(r_ptr->start_pattern, b_ptr) == TRUE) {
		if (OptDisplay == OPT_DEBUG) 
		    fprintf(stderr, "Start (%d) %d\n", j, i);
		for (k = i, c_ptr = b_ptr; k < Bnst_num; k++, c_ptr++) 
		    if (_regexpbnst_match(r_ptr->end_pattern, c_ptr)
			== TRUE) {
			koou_m_p[i] = TRUE;
			flag = TRUE;
			Koou_matrix[i][k] = 1;
			if (OptDisplay == OPT_DEBUG) 
			    fprintf(stdout, "  End %d\n", k);
		    }
		    else if (r_ptr->uke_pattern &&
			     _regexpbnst_match(r_ptr->uke_pattern, c_ptr) 
			     == TRUE) {
			Koou_matrix[i][k] = 2;
			if (OptDisplay == OPT_DEBUG) 
			    fprintf(stdout, "  Uke %d\n", k);
		    }
	    }
	}
    }

    return flag;
}

/*==================================================================*/
	      void mask_for(int si, int start, int end)
/*==================================================================*/
{
    int i, j;

    for (i = si + 1; i < start; i++) {
	for (j = end + 1; j < Bnst_num;j++) {
	    Dpnd_matrix[i][j] = 0;
	}
    }
}

/*==================================================================*/
		   void mask_back(int si, int start)
/*==================================================================*/
{
    int i, j;

    for (i = 0; i < si; i++) {
	for (j = si +1 ; j < start ; j++) {
	    Dpnd_matrix[i][j] = 0;
	}
    }
}

/*==================================================================*/
			 void change_matrix()
/*==================================================================*/
{
    int i, j, f_start, f_end;

    for (i = 0; i < Bnst_num; i++){
	if (koou_m_p[i] == TRUE) {
	    f_start = -1;
	    f_end = -1;
	    for (j = i; j < Bnst_num; j++){
		if (Koou_matrix[i][j] > 0) {
		    Dpnd_matrix[i][j] = (int)'D';
		    if (Koou_matrix[i][j] == 1) {
			f_end = j;
			if (f_start < 0)
			    f_start = j;
		    }
		}
		else
		  Dpnd_matrix[i][j] = 0;
	    }
	    mask_for(i, f_start, f_end);
	    mask_back(i, f_start);
	}
    }
}

/*==================================================================*/
			      int koou()
/*==================================================================*/
{
    int flag;

    init_koou();

    /* 呼応のチェック */
    flag = (check_koou() == TRUE) ? TRUE: FALSE;

    /* 行列の書き換え */
    change_matrix();

    return flag;
}

/*====================================================================
                               END
====================================================================*/
