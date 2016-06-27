/*====================================================================

			      格構造解析

                                               S.Kurohashi 91.10. 9
                                               S.Kurohashi 93. 5.31

    $Id$
====================================================================*/
#include "knp.h"

extern int Possibility;
extern int MAX_Case_frame_num;

CF_MATCH_MGR	*Cf_match_mgr = NULL;	/* 作業領域 */
TOTAL_MGR	Work_mgr;

int	DISTANCE_STEP	= 5;
int	RENKAKU_STEP	= 2;
int	STRONG_V_COST	= 8;
int	ADJACENT_TOUTEN_COST	= 5;
int	LEVELA_COST	= 4;
int	TEIDAI_STEP	= 2;

/*==================================================================*/
			  void realloc_cmm()
/*==================================================================*/
{
    Cf_match_mgr = (CF_MATCH_MGR *)realloc_data(Cf_match_mgr, 
						sizeof(CF_MATCH_MGR)*(MAX_Case_frame_num), 
						"realloc_cmm");
}

/*==================================================================*/
		 void init_case_frame(CASE_FRAME *cf)
/*==================================================================*/
{
    int j;

    for (j = 0; j < CF_ELEMENT_MAX; j++) {
	if (Thesaurus == USE_BGH) {
	    cf->ex[j] = 
		(char *)malloc_data(sizeof(char)*EX_ELEMENT_MAX*BGH_CODE_SIZE, 
				    "init_case_frame");
	}
	else if (Thesaurus == USE_NTT) {
	    cf->ex[j] = 
		(char *)malloc_data(sizeof(char)*SM_ELEMENT_MAX*SM_CODE_SIZE, 
				    "init_case_frame");
	}
	cf->sm[j] = 
	    (char *)malloc_data(sizeof(char)*SM_ELEMENT_MAX*SM_CODE_SIZE, 
				"init_case_frame");
	cf->ex_list[j] = (char **)malloc_data(sizeof(char *), 
					      "init_case_frame");
	cf->ex_list[j][0] = (char *)malloc_data(sizeof(char)*REPNAME_LEN_MAX, 
						"init_case_frame");
	cf->ex_freq[j] = (int *)malloc_data(sizeof(int), 
					    "init_case_frame");
    }

    cf->cf_id[0] = '\0';
    cf->imi[0] = '\0';
}

/*==================================================================*/
		    void init_case_analysis_cmm()
/*==================================================================*/
{
    if (OptAnalysis == OPT_CASE || 
	OptAnalysis == OPT_CASE2) {

	/* 作業cmm領域確保 */
	Cf_match_mgr = 
	    (CF_MATCH_MGR *)malloc_data(sizeof(CF_MATCH_MGR)*ALL_CASE_FRAME_MAX, 
					"init_case_analysis_cmm");

	init_mgr_cf(&Work_mgr);
    }
}

/*==================================================================*/
		void clear_case_frame(CASE_FRAME *cf)
/*==================================================================*/
{
    int j;

    for (j = 0; j < CF_ELEMENT_MAX; j++) {
	free(cf->ex[j]);
	free(cf->sm[j]);
	free(cf->ex_list[j][0]);
	free(cf->ex_list[j]);
	free(cf->ex_freq[j]);
    }
}

/*====================================================================
		       格助詞の文字−コード対応
====================================================================*/

struct PP_STR_TO_CODE {
    char *hstr;
    char *kstr;
    int  code;
} PP_str_to_code[] = {          /* 文字数はPP_STRING_MAX以内にする */
    {"φ", "φ", 0},		/* 格助詞のないもの(数量表現等) */
    {"が", "ガ", 1},            /* anaphora.cで"が"、"を"、"に"が1、2、3であることを */
    {"を", "ヲ", 2},            /* 利用しているので変更する際は要注意 */
    {"に", "ニ", 3},
    {"から", "カラ", 4},
    {"へ", "ヘ", 5},
    {"より", "ヨリ", 6},
    {"と", "ト", 7},
    {"で", "デ", 8},
    {"によって", "ニヨッテ", 9}, /* 複合辞関係(FUKUGOJI_STARTからFUKUGOJI_ENDまで) */
    {"をめぐる", "ヲメグル", 10},	
    {"をつうじる", "ヲツウジル", 11},
    {"をつうずる", "ヲツウズル", 12},
    {"をふくめる", "ヲフクメル", 13},
    {"をはじめる", "ヲハジメル", 14},
    {"にからむ", "ニカラム", 15},
    {"にそう", "ニソウ", 16},
    {"にむける", "ニムケル", 17},
    {"にともなう", "ニトモナウ", 18},
    {"にもとづく", "ニモトヅク", 19},
    {"をのぞく", "ヲノゾク", 20},
    {"による", "ニヨル", 21},
    {"にたいする", "ニタイスル", 22},
    {"にかんする", "ニカンスル", 23},
    {"にかわる", "ニカワル", 24},
    {"におく", "ニオク", 25},
    {"につく", "ニツク", 26},
    {"にとる", "ニトル", 27},
    {"にくわえる", "ニクワエル", 28},
    {"にかぎる", "ニカギル", 29},
    {"につづく", "ニツヅク", 30},
    {"にあわせる", "ニアワセル", 31},
    {"にくらべる", "ニクラベル", 32},
    {"にならぶ", "ニナラブ", 33},
    {"とする", "トスル", 34},
    {"によるぬ", "ニヨルヌ", 35},
    {"にかぎるぬ", "ニカギルヌ", 36},
    {"という", "トイウ", 37},	/* 〜というと? */
    {"時間", "時間", 38},	/* ニ格, 無格で時間であるものを時間という格として扱う */
    {"まで", "マデ", 39},	/* 明示されない格であるが、辞書側の格として表示するために
				   書いておく */
    {"修飾", "修飾", 40},
    {"の", "ノ", 41},		/* 格フレームのノ格 */
    {"が２", "ガ２", 42},
    {"外の関係", "外の関係", 43},
    {"がが", "ガガ", 42},
    {"外の関係", "外ノ関係", 43},	/* for backward compatibility */
    {"は", "ハ", 1},		/* NTT辞書では「ガガ」構文が「ハガ」
				   ※ NTT辞書の「ハ」は1(code)に変換されるが,
				      1は配列順だけで「ガ」に変換される */
    {"未", "未", -3},		/* 格フレームによって動的に割り当てる格を決定する */
    {"＊", "＊", -2},		/* 埋め込み文の被修飾詞、照応解析の格がない場合にも使用 */
    {NULL, NULL, -1}		/* 格助詞の非明示のもの(提題助詞等) */
};

/* ※ 格の最大数を変えたら、PP_NUMBER(const.h)を変えること */

/*====================================================================
			 文字−コード対応関数
====================================================================*/
int pp_kstr_to_code(char *cp)
{
    int i;

    if (str_eq(cp, "NIL"))
	return END_M;

    for (i = 0; PP_str_to_code[i].kstr; i++)
	if (str_eq(PP_str_to_code[i].kstr, cp))
	    return PP_str_to_code[i].code;
    
    if (str_eq(cp, "ニトッテ"))		/* 「待つ」 IPALのバグ ?? */
	return pp_kstr_to_code("ニヨッテ");
    else if (str_eq(cp, "ノ"))		/* 格要素でなくなる場合 */
	return END_M;

    /* fprintf(stderr, "Invalid string (%s) in PP !\n", cp); */
    return END_M;
}

int pp_hstr_to_code(char *cp)
{
    int i;
    for (i = 0; PP_str_to_code[i].hstr; i++)
	if (str_eq(PP_str_to_code[i].hstr, cp))
	    return PP_str_to_code[i].code;
    return END_M;
}

char *pp_code_to_kstr(int num)
{
    return PP_str_to_code[num].kstr;
}

char *pp_code_to_hstr(int num)
{
    return PP_str_to_code[num].hstr;
}

char *pp_code_to_kstr_in_context(CF_PRED_MGR *cpm_ptr, int num)
{
    if ((cpm_ptr->cf.type_flag && MatchPP(num, "φ")) || cpm_ptr->cf.type == CF_NOUN) {
	return "ノ";
    }   
    return pp_code_to_kstr(num);
}

/*==================================================================*/
		    int MatchPPn(int n, int *list)
/*==================================================================*/
{
    int i;

    if (n < 0) {
	return 0;
    }

    for (i = 0; list[i] != END_M; i++) {
	if (n == list[i]) {
	    return 1;
	}
    }
    return 0;
}

/*==================================================================*/
		     int MatchPP(int n, char *pp)
/*==================================================================*/
{
    if (n < 0) {
	return 0;
    }
    if (str_eq(pp_code_to_kstr(n), pp)) {
	return 1;
    }
    return 0;
}

/*==================================================================*/
		    int MatchPP2(int *n, char *pp)
/*==================================================================*/
{
    int i;

    /* 格の配列の中に調べたい格があるかどうか */

    if (n < 0) {
	return 0;
    }

    for (i = 0; *(n+i) != END_M; i++) {
	if (str_eq(pp_code_to_kstr(*(n+i)), pp)) {
	    return 1;
	}
    }
    return 0;
}

/*==================================================================*/
		int CF_MatchPP(int c, CASE_FRAME *cf)
/*==================================================================*/
{
    int i, j;

    for (i = 0; i < cf->element_num; i++) {
	for (j = 0; cf->pp[i][j] != END_M; j++) {
	    if (cf->pp[i][j] == c) {
		return 1;
	    }
	}
    }
    return 0;
}

/*==================================================================*/
		 int CheckCfAdjacent(CASE_FRAME *cf)
/*==================================================================*/
{
    int i;
    for (i = 0; i < cf->element_num; i++) {
	if (cf->adjacent[i] && 
	    MatchPP(cf->pp[i][0], "修飾")) {
	    return FALSE;
	}
    }
    return TRUE;
}

/*==================================================================*/
	  int CheckCfClosest(CF_MATCH_MGR *cmm, int closest)
/*==================================================================*/
{
    return cmm->cf_ptr->adjacent[cmm->result_lists_d[0].flag[closest]];
}

/*==================================================================*/
	     int have_real_component(CASE_FRAME *cf_ptr)
/*==================================================================*/
{
    /* 入力側が修飾、無格以外をもっているかどうか */

    int i;

    for (i = 0; i < cf_ptr->element_num; i++) {
	if (MatchPP(cf_ptr->pp[i][0], "修飾") || 
	    MatchPP(cf_ptr->pp[i][0], "φ")) {
	    ;
	}
	else {
	    return TRUE;
	}
    }
    return FALSE;
}

/*==================================================================*/
double find_best_cf(SENTENCE_DATA *sp, CF_PRED_MGR *cpm_ptr, int closest, int decide, CF_PRED_MGR *para_cpm_ptr)
/*==================================================================*/
{
    int i, j, frame_num = 0, pat_num;
    CASE_FRAME *cf_ptr = &(cpm_ptr->cf);
    TAG_DATA *b_ptr = cpm_ptr->pred_b_ptr;
    CF_MATCH_MGR tempcmm;

    /* 格要素なしの時の実験 */
    if (cf_ptr->type == CF_PRED && 
	(cf_ptr->element_num == 0 || 
	 have_real_component(cf_ptr) == FALSE)) {
	/* この用言のすべての格フレームの OR、または
	   格フレームが 1 つのときはそれそのもの にする予定 */
	if (b_ptr->cf_num > 1) {
	    for (i = 0; i < b_ptr->cf_num; i++) {
		if ((b_ptr->cf_ptr+i)->etcflag & CF_SUM) {
		    (Cf_match_mgr + frame_num++)->cf_ptr = b_ptr->cf_ptr + i;
		    break;
		}
	    }
	    /* OR格フレームがないとき
	       「動,形,準」の指定がないことがあればこうなる */
	    if (frame_num == 0) {
		(Cf_match_mgr + frame_num++)->cf_ptr = b_ptr->cf_ptr;
	    }
	}
	else {
	    (Cf_match_mgr + frame_num++)->cf_ptr = b_ptr->cf_ptr;
	}
	case_frame_match(cpm_ptr, Cf_match_mgr, OptCFMode, -1, NULL);
	cpm_ptr->score = Cf_match_mgr->score;
	cpm_ptr->cmm[0] = *Cf_match_mgr;
	cpm_ptr->result_num = 1;
    }
    else {
	int hiragana_prefer_flag = 0;

	/* 表記がひらがなの場合: 
	   格フレームの表記がひらがなの場合が多ければひらがなの格フレームのみを対象に、
	   ひらがな以外が多ければひらがな以外のみを対象にする */
	if (!(OptCaseFlag & OPT_CASE_USE_REP_CF) && /* 代表表記ではない場合のみ */
	    check_str_type(b_ptr->head_ptr->Goi, TYPE_HIRAGANA, 0)) {
	    if (check_feature(b_ptr->f, "代表ひらがな")) {
		hiragana_prefer_flag = 1;
	    }
	    else {
		hiragana_prefer_flag = -1;
	    }
	}

	/* 格フレーム設定 */
	for (i = 0; i < b_ptr->cf_num; i++) {
	    /* OR の格フレームを除く */
	    if ((b_ptr->cf_ptr+i)->etcflag & CF_SUM) {
		continue;
	    }
	    else if ((hiragana_prefer_flag > 0 && !check_str_type((b_ptr->cf_ptr + i)->entry, TYPE_HIRAGANA, 0)) || 
		     (hiragana_prefer_flag < 0 && check_str_type((b_ptr->cf_ptr + i)->entry, TYPE_HIRAGANA, 0))) {
		continue;
	    }
	    (Cf_match_mgr + frame_num++)->cf_ptr = b_ptr->cf_ptr + i;
	}

	if (frame_num == 0) { /* 上の処理でひとつも残らないとき */
	    for (i = 0; i < b_ptr->cf_num; i++) {
		(Cf_match_mgr + frame_num++)->cf_ptr = b_ptr->cf_ptr + i;
	    }
	}

	cpm_ptr->result_num = 0;
	for (i = 0; i < frame_num; i++) {

	    /* 選択可能
	       EXAMPLE
	       SEMANTIC_MARKER */

	    /* closest があれば、直前格要素のみのスコアになる */
	    case_frame_match(cpm_ptr, Cf_match_mgr+i, OptCFMode, closest, para_cpm_ptr);

	    /* 結果を格納 */
	    cpm_ptr->cmm[cpm_ptr->result_num] = *(Cf_match_mgr+i);

	    /* DEBUG出力用: 下の print_good_crrspnds() で使う Cf_match_mgr のスコアを正規化 */
	    if (OptDisplay == OPT_DEBUG && closest > -1 && !OptEllipsis) {
		pat_num = count_pat_element((Cf_match_mgr+i)->cf_ptr, &((Cf_match_mgr+i)->result_lists_p[0]));
		if (!((Cf_match_mgr+i)->score < 0 || pat_num == 0)) {
		    (Cf_match_mgr+i)->score = (OptCaseFlag & OPT_CASE_USE_PROBABILITY) ? (Cf_match_mgr+i)->pure_score[0] : ((Cf_match_mgr+i)->pure_score[0] / sqrt((double)pat_num));
		}
	    }

	    /* スコア順にソート */
	    for (j = cpm_ptr->result_num - 1; j >= 0; j--) {
		if (cpm_ptr->cmm[j].score < cpm_ptr->cmm[j+1].score || 
		    ((((OptCaseFlag & OPT_CASE_USE_PROBABILITY) && 
		       cpm_ptr->cmm[j].score != CASE_MATCH_FAILURE_PROB) || 
		      (!(OptCaseFlag & OPT_CASE_USE_PROBABILITY) && 
		       cpm_ptr->cmm[j].score != CASE_MATCH_FAILURE_SCORE)) && 
		     cpm_ptr->cmm[j].score == cpm_ptr->cmm[j+1].score && 
		     ((closest > -1 && 
		       (CheckCfClosest(&(cpm_ptr->cmm[j+1]), closest) == TRUE && 
			CheckCfClosest(&(cpm_ptr->cmm[j]), closest) == FALSE)) || 
		      (closest < 0 && 
		       cpm_ptr->cmm[j].sufficiency < cpm_ptr->cmm[j+1].sufficiency)))) {
		    tempcmm = cpm_ptr->cmm[j];
		    cpm_ptr->cmm[j] = cpm_ptr->cmm[j+1];
		    cpm_ptr->cmm[j+1] = tempcmm;
		}
		else {
		    break;
		}
	    }
	    if (cpm_ptr->result_num < CMM_MAX - 1) {
		cpm_ptr->result_num++;
	    }
	}

	/* スコアが同点の格フレームの個数を設定 */
	if (cpm_ptr->result_num > 0 && 
	    (((OptCaseFlag & OPT_CASE_USE_PROBABILITY) && 
	      cpm_ptr->cmm[0].score != CASE_MATCH_FAILURE_PROB) || 
	     (!(OptCaseFlag & OPT_CASE_USE_PROBABILITY) && 
	      cpm_ptr->cmm[0].score != CASE_MATCH_FAILURE_SCORE))) {
	    double top;
	    int cflag = 0;
	    cpm_ptr->tie_num = 1;
	    top = cpm_ptr->cmm[0].score;
	    if (closest > -1 && 
		CheckCfClosest(&(cpm_ptr->cmm[0]), closest) == TRUE) {
		cflag = 1;
	    }
	    for (i = 1; i < cpm_ptr->result_num; i++) {
		/* score が最高で、
		   直前格要素が格フレームの直前格にマッチしているものがあれば(0番目をチェック)
		   直前格要素が格フレームの直前格にマッチしていることが条件
		   ↓
		   score が最高であることだけにした
		*/
		if (cpm_ptr->cmm[i].score == top) {
/*		if (cpm_ptr->cmm[i].score == top && 
		    (cflag == 0 || CheckCfClosest(&(cpm_ptr->cmm[i]), closest) == TRUE)) { */
		    cpm_ptr->tie_num++;
		}
		else {
		    break;
		}
	    }
	}

	/* とりあえず設定
	   closest > -1: decided 決定用 */
	cpm_ptr->score = (int)cpm_ptr->cmm[0].score;
    }

    /* 文脈解析: 直前格要素のスコアが閾値以上なら格フレームを決定 */
    if (decide) {
	if (OptEllipsis) {
	    if (closest > -1 && cpm_ptr->score > CF_DECIDE_THRESHOLD) {
		if (cpm_ptr->tie_num > 1) {
		    cpm_ptr->decided = CF_CAND_DECIDED;
		}
		else {
		    cpm_ptr->decided = CF_DECIDED;
		    /* exact match して、最高点の格フレームがひとつなら、それだけを表示 */
		    if (cpm_ptr->score == EX_match_exact) {
			cpm_ptr->result_num = 1;
		    }
		}
	    }
	    else if (closest == -1 && cpm_ptr->cf.element_num > 0 && 
		     check_feature(cpm_ptr->pred_b_ptr->f, "用言:形")) {
		cpm_ptr->decided = CF_DECIDED;
	    }
	}
	else if (closest > -1) {
	    cpm_ptr->decided = CF_DECIDED;
	}
    }

    if (cf_ptr->element_num != 0) {
	/* 直前格があるときは直前格のスコアしか考慮されていないので、
	   すべての格のスコアを足して正規化したものにする */
	if (closest > -1) {
	    int slot_i, slot_j, pos_i, pos_j;

	    for (i = 0; i < cpm_ptr->result_num; i++) {
		/* 割り当て失敗のとき(score==-1)は、pure_score は定義されていない */
		/* 入力側に任意格しかなく割り当てがないとき(score==0)は、分子分母ともに0になる */
		pat_num = count_pat_element(cpm_ptr->cmm[i].cf_ptr, &(cpm_ptr->cmm[i].result_lists_p[0]));
		if (cpm_ptr->cmm[i].score < 0 || pat_num == 0) {
		    break;
		}
		cpm_ptr->cmm[i].score = (OptCaseFlag & OPT_CASE_USE_PROBABILITY) ? cpm_ptr->cmm[i].pure_score[0] : (cpm_ptr->cmm[i].pure_score[0] / sqrt((double)pat_num));
	    }
	    /* 直前格スコアが同点の格フレームを、すべてのスコアでsort */
	    for (i = cpm_ptr->tie_num - 1; i >= 1; i--) {
		for (j = i - 1; j >= 0; j--) {
		    slot_i = cpm_ptr->cmm[i].result_lists_d[0].flag[closest];
		    pos_i = (slot_i >= 0 && cpm_ptr->cmm[i].cf_ptr->ex_freq[slot_i]) ? 
			cpm_ptr->cmm[i].result_lists_p[0].pos[slot_i] : -1;
		    slot_j = cpm_ptr->cmm[j].result_lists_d[0].flag[closest];
		    pos_j = (slot_j >= 0 && cpm_ptr->cmm[j].cf_ptr->ex_freq[slot_j]) ? 
			    cpm_ptr->cmm[j].result_lists_p[0].pos[slot_j] : -1;
		    if (cpm_ptr->cmm[i].score > cpm_ptr->cmm[j].score || 
			(pos_i >= 0 && pos_j >= 0 && /* マッチした用例頻度の大きい方を選択 */
			 cpm_ptr->cmm[i].cf_ptr->ex_freq[slot_i][pos_i] > cpm_ptr->cmm[j].cf_ptr->ex_freq[slot_j][pos_j])) {
			tempcmm = cpm_ptr->cmm[i];
			cpm_ptr->cmm[i] = cpm_ptr->cmm[j];
			cpm_ptr->cmm[j] = tempcmm;
		    }
		}
	    }
	}
	cpm_ptr->score = cpm_ptr->cmm[0].score;
    }

    if (OptDisplay == OPT_DEBUG && OptCKY == FALSE) {
	print_data_cframe(cpm_ptr, Cf_match_mgr);
	/* print_good_crrspnds(cpm_ptr, Cf_match_mgr, frame_num); */
	for (i = 0; i < cpm_ptr->result_num; i++) {
	    print_crrspnd(cpm_ptr, &cpm_ptr->cmm[i]);
	}
    }

    return cpm_ptr->score;
}

/*==================================================================*/
int get_closest_case_component(SENTENCE_DATA *sp, CF_PRED_MGR *cpm_ptr)
/*==================================================================*/
{
    /* 用言より前にある格要素の中で
       もっとも用言に近いものを探す
       (内部文節は除く: num == -1) 
       対象格: ヲ格, ニ格 */

    int i, min = -1, elem_b_num;

    /* 直前格要素を走査 */
    for (i = 0; i < cpm_ptr->cf.element_num; i++) {
	if (cpm_ptr->elem_b_ptr[i] == NULL) {
	    ;
	}
	/* 複合名詞の一部: 直前としない */
	else if (cpm_ptr->elem_b_ptr[i]->inum > 0) {
	    return -1;
	}
	/* 「〜を〜に」 */
	else if (cpm_ptr->pred_b_ptr->num == cpm_ptr->elem_b_ptr[i]->num) {
	    return i;
	}
	/* 用言にもっとも近い格要素を探す 
	   <回数>:無格 以外 */
	else if (cpm_ptr->elem_b_num[i] > -2 && /* 省略の格要素じゃない */
		 cpm_ptr->elem_b_ptr[i]->num <= cpm_ptr->pred_b_ptr->num && 
		 min < cpm_ptr->elem_b_ptr[i]->num && 
		 !(MatchPP(cpm_ptr->cf.pp[i][0], "φ") && 
		   check_feature(cpm_ptr->elem_b_ptr[i]->f, "回数"))) {
	    min = cpm_ptr->elem_b_ptr[i]->num;
	    elem_b_num = i;
	}
    }

    /* 1. ヲ格, ニ格であるとき
       2. <主体>にマッチしない 1, 2 以外の格 (MatchPP(cpm_ptr->cf.pp[elem_b_num][0], "ガ"))
       3. 用言の直前の未格 (副詞がはさまってもよい)
       ★形容詞, 判定詞は?
       check_feature してもよい
       条件廃止: cpm_ptr->cf.pp[elem_b_num][1] == END_M */
    if (min != -1) {
	/* 決定しない:
	   1. 最近格要素が指示詞の場合 ★格だけマッチさせる?
	   2. ガ格で意味素がないとき */
	if (check_feature((sp->tag_data+min)->f, "指示詞") || 
	    (Thesaurus == USE_NTT && 
	     (sp->tag_data+min)->SM_code[0] == '\0' && 
	     MatchPP(cpm_ptr->cf.pp[elem_b_num][0], "ガ"))) {
	    return -2;
	}
	else if ((cpm_ptr->cf.pp[elem_b_num][0] == -1 && /* 未格 */
		  (cpm_ptr->pred_b_ptr->num == min + 1 || 
		   (cpm_ptr->pred_b_ptr->num == min + 2 && /* 副詞がはさまっている場合 */
		    (check_feature((sp->tag_data + min + 1)->f, "副詞") || 
		     check_feature((sp->tag_data + min + 1)->f, "係:連用"))))) || 
		 MatchPP(cpm_ptr->cf.pp[elem_b_num][0], "ヲ") || 
		 MatchPP(cpm_ptr->cf.pp[elem_b_num][0], "ニ") || 
		 (((cpm_ptr->cf.pp[elem_b_num][0] > 0 && 
		    cpm_ptr->cf.pp[elem_b_num][0] < 9) || /* 基本格 */
		   MatchPP(cpm_ptr->cf.pp[elem_b_num][0], "マデ")) && 
		   !cf_match_element(cpm_ptr->cf.sm[elem_b_num], "主体", FALSE))) {
	    cpm_ptr->cf.adjacent[elem_b_num] = TRUE;	/* 直前格のマーク */
	    return elem_b_num;
	}
    }
    return -1;
}

/*==================================================================*/
       static int number_compare(const void *i, const void *j)
/*==================================================================*/
{
    /* sort function */
    return *(const int *)i-*(const int *)j;
}

/*==================================================================*/
	       char *inputcc2num(CF_PRED_MGR *cpm_ptr)
/*==================================================================*/
{
    int i, numbers[CF_ELEMENT_MAX];
    char str[70], token[3], *key;

    for (i = 0; i < cpm_ptr->cf.element_num; i++) {
	numbers[i] = cpm_ptr->elem_b_ptr[i]->num;
    }

    qsort(numbers, cpm_ptr->cf.element_num, sizeof(int), number_compare);

    str[0] = '\0';
    for (i = 0; i < cpm_ptr->cf.element_num; i++) {
	if (i) {
	    sprintf(token, " %d", numbers[i]);
	}
	else {
	    sprintf(token, "%d", numbers[i]);
	}
	strcat(str, token);
    }
    sprintf(token, " %d", cpm_ptr->pred_b_ptr->num);
    strcat(str, token);

    key = strdup(str);
    return key;
}

typedef struct cpm_cache {
    char *key;
    CF_PRED_MGR *cpm;
    struct cpm_cache *next;
} CPM_CACHE;

CPM_CACHE *CPMcache[TBLSIZE];

/*==================================================================*/
			 void InitCPMcache()
/*==================================================================*/
{
    memset(CPMcache, 0, sizeof(CPM_CACHE *)*TBLSIZE);
}

/*==================================================================*/
			 void ClearCPMcache()
/*==================================================================*/
{
    int i;
    CPM_CACHE *ccp, *next;

    for (i = 0; i < TBLSIZE; i++) {
	if (CPMcache[i]) {
	    ccp = CPMcache[i];
	    while (ccp) {
		free(ccp->key);
		clear_case_frame(&(ccp->cpm->cf));
		free(ccp->cpm);
		next = ccp->next;
		free(ccp);
		ccp = next;
	    }
	    CPMcache[i] = NULL;
	}
    }
}

/*==================================================================*/
		void RegisterCPM(CF_PRED_MGR *cpm_ptr)
/*==================================================================*/
{
    int num;
    char *key;
    CPM_CACHE **ccpp;

    key = inputcc2num(cpm_ptr);
    if (key == NULL) {
	return;
    }
    num = hash(key, strlen(key));

    ccpp = &(CPMcache[num]);
    while (*ccpp) {
	ccpp = &((*ccpp)->next);
    }

    *ccpp = (CPM_CACHE *)malloc_data(sizeof(CPM_CACHE), "RegisterCPM");
    (*ccpp)->key = key;
    (*ccpp)->cpm = (CF_PRED_MGR *)malloc_data(sizeof(CF_PRED_MGR), "RegisterCPM");
    init_case_frame(&((*ccpp)->cpm->cf));
    copy_cpm((*ccpp)->cpm, cpm_ptr, 0);
    (*ccpp)->next = NULL;
}

/*==================================================================*/
	     CF_PRED_MGR *CheckCPM(CF_PRED_MGR *cpm_ptr)
/*==================================================================*/
{
    int num;
    char *key;
    CPM_CACHE *ccp;

    key = inputcc2num(cpm_ptr);
    if (key == NULL) {
	return NULL;
    }
    num = hash(key, strlen(key));

    ccp = CPMcache[num];
    while (ccp) {
	if (str_eq(key, ccp->key)) {
	    free(key);
	    return ccp->cpm;
	}
	ccp = ccp->next;
    }
    free(key);
    return NULL;
}

/*==================================================================*/
double case_analysis(SENTENCE_DATA *sp, CF_PRED_MGR *cpm_ptr, TAG_DATA *t_ptr)
/*==================================================================*/
{
    /*
                                              戻値
      入力の格要素がない場合                    -3
      格フレームがない場合                      -2
      入力側に必須格が残る場合(解析不成功)      -1
      解析成功                               score (0以上)
    */

    int closest;
    CF_PRED_MGR *cache_ptr;

    /* 初期化 */
    cpm_ptr->pred_b_ptr = t_ptr;
    cpm_ptr->score = -1;
    cpm_ptr->result_num = 0;
    cpm_ptr->tie_num = 0;
    cpm_ptr->cmm[0].cf_ptr = NULL;
    cpm_ptr->decided = CF_UNDECIDED;

    /* 入力文側の格要素設定 */
    set_data_cf_type(cpm_ptr);
    closest = make_data_cframe(sp, cpm_ptr);

    /* 格フレーム解析スキップ
    if (cpm_ptr->cf.element_num == 0) {
	cpm_ptr->cmm[0].cf_ptr = NULL;
	return -3;
    }
    */

    /* cache */
    if (OptAnalysis == OPT_CASE && 
	!(OptCaseFlag & OPT_CASE_USE_PROBABILITY) && 
	(cache_ptr = CheckCPM(cpm_ptr))) {
	copy_cpm(cpm_ptr, cache_ptr, 0);
	return cpm_ptr->score;
    }

    /* もっともスコアのよい格フレームを決定する
       文脈解析: 直前格要素がなければ格フレームを決定しない */

    /* 直前格要素がある場合 (closest > -1) のときは格フレームを決定する */
    find_best_cf(sp, cpm_ptr, closest, 1, NULL);

    if (OptAnalysis == OPT_CASE && 
	!(OptCaseFlag & OPT_CASE_USE_PROBABILITY)) {
	RegisterCPM(cpm_ptr);
    }

    return cpm_ptr->score;
}

/*==================================================================*/
int all_case_analysis(SENTENCE_DATA *sp, TAG_DATA *t_ptr, TOTAL_MGR *t_mgr)
/*==================================================================*/
{
    CF_PRED_MGR *cpm_ptr;
    int i, renyou_modifying_num, adverb_modifying_num, current_pred_num;
    double one_case_point;

    /* 格フレームの有無をチェック: set_pred_caseframe()の条件に従う */
    if (t_ptr->para_top_p != TRUE && 
	t_ptr->cf_num > 0) { /* 格フレーム辞書になくてもデフォルト格フレームがあるので1以上になる */

	if (t_mgr->pred_num >= CPM_MAX) {
	    fprintf(stderr, ";; too many predicates in a sentence. (> %d)\n", CPM_MAX);
	    exit(1);
	}

	cpm_ptr = &(t_mgr->cpm[t_mgr->pred_num]);

	one_case_point = case_analysis(sp, cpm_ptr, t_ptr);

	/* 解析不成功(入力側に必須格が残る)場合にその依存構造の解析を
	   やめる場合
	if (one_case_point == -1) return FALSE;
	*/

	t_mgr->score += one_case_point;
	t_mgr->pred_num++;
    }

    /* 文末はEOSからの生成 (どの構造も等しいので、今のところ考慮しない) *
    if (check_feature(t_ptr->f, "文末") && 
	t_ptr->para_top_p != TRUE && 
	t_ptr->cf_num > 0 && 
	check_feature(t_ptr->f, "用言")) {
	t_mgr->score += calc_vp_modifying_probability(NULL, NULL, t_ptr, cpm_ptr->cmm[0].cf_ptr);
    }
    */

    renyou_modifying_num = 0;
    adverb_modifying_num = 0;
    for (i = 0; t_ptr->child[i]; i++) {
	current_pred_num = t_mgr->pred_num;
	if (all_case_analysis(sp, t_ptr->child[i], t_mgr) == FALSE) {
	    return FALSE;
	}

	if ((OptCaseFlag & OPT_CASE_USE_PROBABILITY) && 
	    t_ptr->para_top_p != TRUE && 
	    t_ptr->cf_num > 0 && 
	    check_feature(t_ptr->f, "用言")) {
	    if (check_feature(t_ptr->child[i]->f, "係:連用") && 
		check_feature(t_ptr->child[i]->f, "用言") && 
		!check_feature(t_ptr->child[i]->f, "複合辞")) {
		t_mgr->score += calc_vp_modifying_probability(t_ptr, cpm_ptr->cmm[0].cf_ptr, 
							      t_ptr->child[i], 
							      t_mgr->cpm[current_pred_num].cmm[0].cf_ptr);
		renyou_modifying_num++;
	    }

	    /* この用言に係る副詞または修飾をカウント */
	    if ((check_feature(t_ptr->child[i]->f, "係:連用") && 
		 !check_feature(t_ptr->child[i]->f, "用言")) || 
		check_feature(t_ptr->child[i]->f, "修飾")) {
		t_mgr->score += calc_adv_modifying_probability(t_ptr, cpm_ptr->cmm[0].cf_ptr, 
							       t_ptr->child[i]);
		adverb_modifying_num++;
	    }
	}
    }

    if ((OptCaseFlag & OPT_CASE_USE_PROBABILITY) && 
	t_ptr->para_top_p != TRUE && 
	t_ptr->cf_num > 0 && 
	check_feature(t_ptr->f, "用言")) {
	t_mgr->score += calc_vp_modifying_num_probability(t_ptr, cpm_ptr->cmm[0].cf_ptr, 
							  renyou_modifying_num);
	t_mgr->score += calc_adv_modifying_num_probability(t_ptr, cpm_ptr->cmm[0].cf_ptr, 
							   adverb_modifying_num);
    }

    return TRUE;
}

/*==================================================================*/
      void copy_cf_with_alloc(CASE_FRAME *dst, CASE_FRAME *src)
/*==================================================================*/
{
    int i, j;

    dst->type = src->type;
    dst->element_num = src->element_num;
    for (i = 0; i < src->element_num; i++) {
	dst->oblig[i] = src->oblig[i];
	dst->adjacent[i] = src->adjacent[i];
	for (j = 0; j < PP_ELEMENT_MAX; j++) {
	    dst->pp[i][j] = src->pp[i][j];
	}
	if (src->pp_str[i]) {
	    dst->pp_str[i] = strdup(src->pp_str[i]);
	}
	else {
	    dst->pp_str[i] = NULL;
	}
	if (src->sm[i]) {
	    dst->sm[i] = strdup(src->sm[i]);
	}
	else {
	    dst->sm[i] = NULL;
	}
	if (src->ex[i]) {
	    dst->ex[i] = strdup(src->ex[i]);
	}
	else {
	    dst->ex[i] = NULL;
	}
	if (src->ex_list[i]) {
	    dst->ex_list[i] = (char **)malloc_data(sizeof(char *)*src->ex_size[i], 
						   "copy_cf_with_alloc");
	    dst->ex_freq[i] = (int *)malloc_data(sizeof(int)*src->ex_size[i], 
						 "copy_cf_with_alloc");
	    for (j = 0; j < src->ex_num[i]; j++) {
		dst->ex_list[i][j] = strdup(src->ex_list[i][j]);
		dst->ex_freq[i][j] = src->ex_freq[i][j];
	    }
	}
	else {
	    dst->ex_list[i] = NULL;
	    dst->ex_freq[i] = NULL;
	}
	dst->ex_size[i] = src->ex_size[i];
	dst->ex_num[i] = src->ex_num[i];
	if (src->semantics[i]) {
	    dst->semantics[i] = strdup(src->semantics[i]);
	}
	else {
	    dst->semantics[i] = NULL;
	}
    }
    dst->voice = src->voice;
    dst->cf_address = src->cf_address;
    dst->cf_size = src->cf_size;
    strcpy(dst->cf_id, src->cf_id);
    strcpy(dst->pred_type, src->pred_type);
    strcpy(dst->imi, src->imi);
    dst->etcflag = src->etcflag;
    if (src->feature) {
	dst->feature = strdup(src->feature);
    }
    else {
	dst->feature = NULL;
    }
    if (src->entry) {
	dst->entry = strdup(src->entry);
    }
    else {
	dst->entry = NULL;
    }
    for (i = 0; i < CF_ELEMENT_MAX; i++) {
	dst->samecase[i][0] = src->samecase[i][0];
	dst->samecase[i][1] = src->samecase[i][1];
    }
    dst->cf_similarity = src->cf_similarity;
    /* weight, pred_b_ptr は未設定 */
}

/*==================================================================*/
	    void copy_cf(CASE_FRAME *dst, CASE_FRAME *src)
/*==================================================================*/
{
    int i, j;

    dst->type = src->type;
    dst->type_flag = src->type_flag;
    dst->element_num = src->element_num;
/*    for (i = 0; i < CF_ELEMENT_MAX; i++) { */
    for (i = 0; i < src->element_num; i++) {
	dst->oblig[i] = src->oblig[i];
	dst->adjacent[i] = src->adjacent[i];
	for (j = 0; j < PP_ELEMENT_MAX; j++) {
	    dst->pp[i][j] = src->pp[i][j];
	}
	dst->pp_str[i] = src->pp_str[i];	/* これを使う場合問題あり */
	/* for (j = 0; j < SM_ELEMENT_MAX*SM_CODE_SIZE; j++) {
	    dst->sm[i][j] = src->sm[i][j];
	} */
	if (src->sm[i]) strcpy(dst->sm[i], src->sm[i]);
	if (src->ex[i]) strcpy(dst->ex[i], src->ex[i]);
	strcpy(dst->ex_list[i][0], src->ex_list[i][0]);
	for (j = 0; j < src->ex_num[i]; j++) {
	    dst->ex_freq[i][j] = src->ex_freq[i][j];
	}
	dst->ex_size[i] = src->ex_size[i];
	dst->ex_num[i] = src->ex_num[i];
    }
    dst->voice = src->voice;
    dst->cf_address = src->cf_address;
    dst->cf_size = src->cf_size;
    strcpy(dst->cf_id, src->cf_id);
    strcpy(dst->pred_type, src->pred_type);
    strcpy(dst->imi, src->imi);
    dst->etcflag = src->etcflag;
    dst->feature = src->feature;
    dst->entry = src->entry;
    for (i = 0; i < CF_ELEMENT_MAX; i++) {
	dst->samecase[i][0] = src->samecase[i][0];
	dst->samecase[i][1] = src->samecase[i][1];
    }
    dst->pred_b_ptr = src->pred_b_ptr;
    dst->cf_similarity = src->cf_similarity;
}

/*==================================================================*/
     void copy_cpm(CF_PRED_MGR *dst, CF_PRED_MGR *src, int flag)
/*==================================================================*/
{
    int i;

    if (flag) {
	copy_cf_with_alloc(&dst->cf, &src->cf);
    }
    else {
	copy_cf(&dst->cf, &src->cf);
    }
    dst->pred_b_ptr = src->pred_b_ptr;
    for (i = 0; i < CF_ELEMENT_MAX; i++) {
	dst->elem_b_ptr[i] = src->elem_b_ptr[i];
	dst->para_b_ptr[i] = src->para_b_ptr[i];
	dst->elem_b_num[i] = src->elem_b_num[i];
	dst->elem_s_ptr[i] = src->elem_s_ptr[i];
    }
    dst->score = src->score;
    dst->result_num = src->result_num;
    dst->tie_num = src->tie_num;
    for (i = 0; i < CMM_MAX; i++) {
	dst->cmm[i] = src->cmm[i];
    }
    dst->decided = src->decided;
}

/*==================================================================*/
	    void copy_mgr(TOTAL_MGR *dst, TOTAL_MGR *src)
/*==================================================================*/
{
    int i;

    dst->dpnd = src->dpnd;
    dst->pssb = src->pssb;
    dst->dflt = src->dflt;
    dst->score = src->score;
    dst->pred_num = src->pred_num;
    for (i = 0; i < CPM_MAX; i++) {
	copy_cpm(&dst->cpm[i], &src->cpm[i], 0);
    }
    dst->ID = src->ID;
}

/*==================================================================*/
int call_case_analysis(SENTENCE_DATA *sp, DPND dpnd, int eos_flag)
/*==================================================================*/
{
    int i, j, k;
    int one_topic_score, topic_score, topic_score_sum = 0, topic_slot[2], distance_cost = 0;
    char *cp;

    /* 格構造解析のメイン関数 */

    /* 依存構造木作成 */

    dpnd_info_to_bnst(sp, &dpnd);
    dpnd_info_to_tag(sp, &dpnd);
    if (make_dpnd_tree(sp) == FALSE) {
	return FALSE;
    }
    bnst_to_tag_tree(sp);
	
    if (OptDisplay == OPT_DEBUG)
	print_kakari(sp, OPT_TREE, 1);

    /* 格解析作業領域の初期化 */
	
    Work_mgr.pssb = Possibility;
    Work_mgr.dpnd = dpnd;
    Work_mgr.score = 0;
    Work_mgr.pred_num = 0;
    Work_mgr.dflt = 0;
    for (i = 0; i < sp->Bnst_num; i++)
	Work_mgr.dflt += dpnd.dflt[i];
    
    /* 格解析呼び出し */

    if (all_case_analysis(sp, sp->tag_data + sp->Tag_num - 1, &Work_mgr) == TRUE)
	Possibility++;
    else
	return FALSE;

    /* ここで default との距離のずれ, 提題を処理 */

    for (i = 0; i < sp->Bnst_num - 1; i++) {
	/* ガ格 -> レベル:A (ルールでこの係り受けを許した場合は、
	   ここでコストを与える) */
	if (!(OptCaseFlag & OPT_CASE_USE_PROBABILITY) && 
	    check_feature((sp->bnst_data + i)->f, "係:ガ格") && 
	    check_feature((sp->bnst_data + dpnd.head[i])->f, "レベル:A")) {
	    distance_cost += LEVELA_COST;
	}

	if (dpnd.dflt[i] > 0) {
	    /* 提題 */
	    if (!(OptCaseFlag & OPT_CASE_USE_PROBABILITY) && 
		check_feature((sp->bnst_data + i)->f, "提題")) {
		distance_cost += dpnd.dflt[i];

		/* 提題につられて遠くに係ってしまった文節の距離コスト */
		for (j = 0; j < i - 1; j++) {
		    if (dpnd.head[i] == dpnd.head[j]) {
			for (k = j + 1; k < i; k++) {
			    if (Mask_matrix[j][k] && Quote_matrix[j][k] && Dpnd_matrix[j][k] && Dpnd_matrix[j][k] != 'd') {
				distance_cost += dpnd.dflt[i]*TEIDAI_STEP;
			    }
			}
		    }
		}
		continue;
	    }
	    /* 提題以外 */
	    /* 係り側が連用でないとき */
	    if (!(OptCaseFlag & OPT_CASE_USE_PROBABILITY) && 
		!check_feature((sp->bnst_data + i)->f, "係:連用")) {
		/* 自分に読点がなく、隣の強い用言 (連体以外) を越えているとき */
		if (!check_feature((sp->bnst_data + i)->f, "読点")) {
		    if (dpnd.head[i] > i + 1 && 
			subordinate_level_check("B", (sp->bnst_data + i + 1)->f) && 
			(cp = (char *)check_feature((sp->bnst_data + i + 1)->f, "係"))) {
			if (strcmp(cp+3, "連体") && strcmp(cp+3, "連格")) {
			    distance_cost += STRONG_V_COST;
			}
		    }
		}
		/* 自分に読点があり*/
		else {
		    /* 隣に係るとき */
		    if (dpnd.head[i] == i + 1) {
			distance_cost += ADJACENT_TOUTEN_COST;
		    }
		}
	    }

	    /* 確率的: 副詞などのコスト (tentative) */
	    if (OptCaseFlag & OPT_CASE_USE_PROBABILITY) {
		if (check_feature((sp->bnst_data + i)->f, "係:連用") && 
		    !check_feature((sp->bnst_data + i)->f, "用言")) {
		    distance_cost += dpnd.dflt[i]*DISTANCE_STEP;
		}
	    }
	    /* デフォルトとの差 x 2 を距離のコストとする
	       ただし、形容詞を除く連格の場合は x 1 */
	    else {
		if (!check_feature((sp->bnst_data + i)->f, "係:連格") || 
		    check_feature((sp->bnst_data + i)->f, "用言:形")) {
		    distance_cost += dpnd.dflt[i]*DISTANCE_STEP;
		}
		else {
		    distance_cost += dpnd.dflt[i]*RENKAKU_STEP;
		}
	    }
	}		    
    }

    Work_mgr.score -= distance_cost;

    if (!(OptCaseFlag & OPT_CASE_USE_PROBABILITY)) {
	for (i = sp->Bnst_num - 1; i > 0; i--) {
	    /* 文末から用言ごとに提題を処理する */
	    if ((cp = (char *)check_feature((sp->bnst_data + i)->f, "提題受"))) {

		/* topic_slot[0]	時間以外のハ格のスロット
		   topic_slot[1]	「<<時間>>は」のスロット
		   両方とも 1 以下しか許可しない
		*/

		topic_slot[0] = 0;
		topic_slot[1] = 0;
		one_topic_score = 0;

		/* 係り側を探す */
		for (j = i - 1; j >= 0; j--) {
		    if (dpnd.head[j] != i) {
			continue;
		    }
		    if (check_feature((sp->bnst_data + j)->f, "提題")) {
			if (check_feature((sp->bnst_data + j)->f, "時間")) {
			    topic_slot[1]++;
			}
			else {
			    topic_slot[0]++;
			}
			sscanf(cp, "%*[^:]:%d", &topic_score);
			one_topic_score += topic_score;
		    }
		}

		if (topic_slot[0] > 0 || topic_slot[1] > 0) {
		    one_topic_score += 20;
		}
		Work_mgr.score += one_topic_score;
		if (OptDisplay == OPT_DEBUG) {
		    topic_score_sum += one_topic_score;
		}
	    }
	}
    }

    if (OptDisplay == OPT_DEBUG) {
	if (OptCaseFlag & OPT_CASE_USE_PROBABILITY) {
	    fprintf(stdout, "■ %.5f点 (距離減点 %d点)\n", 
		    Work_mgr.score, distance_cost);
	}
	else {
	    fprintf(stdout, "■ %d点 (距離減点 %d点 (%d点) 提題スコア %d点)\n", 
		    (int)Work_mgr.score, distance_cost, (int)Work_mgr.dflt*2, topic_score_sum);
	}
    }

    /* -nbestのため出力 */
    if (OptNbest == TRUE) {
	/* featureを仮付与 */
	assign_general_feature(sp->bnst_data, sp->Bnst_num, AfterDpndBnstRuleType, FALSE, TRUE);
	assign_general_feature(sp->tag_data, sp->Tag_num, AfterDpndTagRuleType, FALSE, TRUE);

	/* 格解析の結果をfeatureとして仮付与 */
	for (i = 0; i < Work_mgr.pred_num; i++) {
	    record_case_analysis(sp, &(Work_mgr.cpm[i]), NULL, TRUE);
	}

	sp->score = Work_mgr.score;
	print_result(sp, 0, eos_flag);

	/* 仮付与したfeatureを削除 */
	for (i = 0; i < sp->Bnst_num; i++) {
	    delete_temp_feature(&(sp->bnst_data[i].f));
	}
	for (i = 0; i < sp->Tag_num; i++) {
	    delete_temp_feature(&(sp->tag_data[i].f));
	}
    }
        
    /* 後処理 */

    if (Work_mgr.score > sp->Best_mgr->score ||
	(Work_mgr.score == sp->Best_mgr->score && 
	 compare_dpnd(sp, &Work_mgr, sp->Best_mgr) == TRUE))
	copy_mgr(sp->Best_mgr, &Work_mgr);

    return TRUE;
}

/*==================================================================*/
      int add_cf_slot(CF_PRED_MGR *cpm_ptr, char *cstr, int num)
/*==================================================================*/
{
    if (cpm_ptr->cmm[0].cf_ptr->element_num >= CF_ELEMENT_MAX) {
	return FALSE;
    }

    _make_ipal_cframe_pp(cpm_ptr->cmm[0].cf_ptr, cstr, cpm_ptr->cmm[0].cf_ptr->element_num, CF_PRED);
    cpm_ptr->cmm[0].result_lists_d[0].flag[num] = cpm_ptr->cmm[0].cf_ptr->element_num;
    cpm_ptr->cmm[0].result_lists_d[0].score[num] = 0;
    cpm_ptr->cmm[0].result_lists_p[0].flag[cpm_ptr->cmm[0].cf_ptr->element_num] = num;
    cpm_ptr->cmm[0].result_lists_p[0].score[cpm_ptr->cmm[0].cf_ptr->element_num] = 0;
    cpm_ptr->cmm[0].cf_ptr->element_num++;

    return TRUE;
}

/*==================================================================*/
    int assign_cf_slot(CF_PRED_MGR *cpm_ptr, int cnum, int num)
/*==================================================================*/
{
    /* 格フレームのその格にすでに対応付けがあれば */
    if (cpm_ptr->cmm[0].result_lists_p[0].flag[cnum] != UNASSIGNED) {
	return FALSE;
    }

    cpm_ptr->cmm[0].result_lists_d[0].flag[num] = cnum;
    cpm_ptr->cmm[0].result_lists_d[0].score[num] = 0;
    cpm_ptr->cmm[0].result_lists_p[0].flag[cnum] = num;
    cpm_ptr->cmm[0].result_lists_p[0].score[cnum] = 0;

    return TRUE;
}

/*==================================================================*/
		int check_ga2_ok(CF_PRED_MGR *cpm_ptr)
/*==================================================================*/
{
    int i;
    for (i = 0; i < cpm_ptr->cmm[0].cf_ptr->element_num; i++) {
	/* 割り当てなし、<主体>アリのガ格, ヲ格, ニ格が存在するならば、ガ２不可 */
	if (cpm_ptr->cmm[0].result_lists_p[0].flag[i] == UNASSIGNED && 
	    cf_match_element(cpm_ptr->cmm[0].cf_ptr->sm[i], "主体", FALSE) && 
	    (MatchPP(cpm_ptr->cmm[0].cf_ptr->pp[i][0], "ガ") || 
	     MatchPP(cpm_ptr->cmm[0].cf_ptr->pp[i][0], "ヲ") || 
	     MatchPP(cpm_ptr->cmm[0].cf_ptr->pp[i][0], "ニ"))) {
	    return 0;
	}
    }
    return 1;
}

/*==================================================================*/
      void decide_voice(SENTENCE_DATA *sp, CF_PRED_MGR *cpm_ptr)
/*==================================================================*/
{
    TAG_DATA *check_b_ptr;

    if (cpm_ptr->cmm[0].cf_ptr->voice == FRAME_ACTIVE) {
	cpm_ptr->pred_b_ptr->voice = 0;
    }
    else {
	cpm_ptr->pred_b_ptr->voice = VOICE_UKEMI;
    }

    /* なくならないように */
    check_b_ptr = cpm_ptr->pred_b_ptr;
    while (check_b_ptr->parent && check_b_ptr->parent->para_top_p == TRUE) {
	check_b_ptr->parent->voice = cpm_ptr->pred_b_ptr->voice;
	check_b_ptr = check_b_ptr->parent;
    }
}

/*==================================================================*/
	   char *make_print_string(TAG_DATA *bp, int flag)
/*==================================================================*/
{
    int i, start = 0, end = 0, length = 0;
    char *ret;

    /*
       flag == 1: 自立語列
       flag == 0: 最後の自立語
    */

    if (flag) {
	/* 先頭をみる */
	for (i = 0; i < bp->mrph_num; i++) {
	    /* 付属の特殊を除く */
	    if (strcmp(Class[(bp->mrph_ptr + i)->Hinshi][0].id, "特殊") || 
		check_feature((bp->mrph_ptr + i)->f, "自立")) {
		start = i;
		break;
	    }
	}

	/* 末尾をみる */
	for (i = bp->mrph_num - 1; i >= start; i--) {
	    /* 特殊, 助詞, 助動詞, 判定詞を除く */
	    if ((strcmp(Class[(bp->mrph_ptr + i)->Hinshi][0].id, "特殊") || 
		 check_feature((bp->mrph_ptr + i)->f, "自立")) && 
		strcmp(Class[(bp->mrph_ptr + i)->Hinshi][0].id, "助詞") && 
		strcmp(Class[(bp->mrph_ptr + i)->Hinshi][0].id, "助動詞") && 
		strcmp(Class[(bp->mrph_ptr + i)->Hinshi][0].id, "判定詞")) {
		end = i;
		break;
	    }
	}

	if (start > end) {
	    start = bp->jiritu_ptr-bp->mrph_ptr;
	    end = bp->settou_num+bp->jiritu_num - 1;
	}

	for (i = start; i <= end; i++) {
	    length += strlen((bp->mrph_ptr + i)->Goi2);
	}
	if (length == 0) {
	    return NULL;
	}
	ret = (char *)malloc_data(length + 1, "make_print_string");
	*ret = '\0';
	for (i = start; i <= end; i++) {
	    strcat(ret, (bp->mrph_ptr + i)->Goi2);
	}
    }
    else {
	ret = strdup(bp->head_ptr->Goi2);
    }
    return ret;
}

/*==================================================================*/
    void record_match_ex(SENTENCE_DATA *sp, CF_PRED_MGR *cpm_ptr)
/*==================================================================*/
{
    int i, num, pos;
    char feature_buffer[DATA_LEN];

    for (i = 0; i < cpm_ptr->cf.element_num; i++) {
	num = cpm_ptr->cmm[0].result_lists_d[0].flag[i];
	if (num != NIL_ASSIGNED && /* 割り当てがある */
	    cpm_ptr->elem_b_ptr[i]) { /* && cpm_ptr->elem_b_num[i] < 0) { * 省略, 〜は, 連体修飾に限定する場合 */
	    pos = cpm_ptr->cmm[0].result_lists_p[0].pos[num];
	    if (pos == MATCH_NONE || pos == MATCH_SUBJECT) {
		sprintf(feature_buffer, "マッチ用例;%s:%s-%s", 
			pp_code_to_kstr_in_context(cpm_ptr, cpm_ptr->cmm[0].cf_ptr->pp[num][0]), 
			cpm_ptr->elem_b_ptr[i]->head_ptr->Goi, 
			pos == MATCH_NONE ? "NONE" : "SUBJECT");
	    }
	    else {
		sprintf(feature_buffer, "マッチ用例;%s:%s-%s:%.5f", 
			pp_code_to_kstr_in_context(cpm_ptr, cpm_ptr->cmm[0].cf_ptr->pp[num][0]), 
			cpm_ptr->elem_b_ptr[i]->head_ptr->Goi, 
			cpm_ptr->cmm[0].cf_ptr->ex_list[num][pos], 
			cpm_ptr->cmm[0].result_lists_p[0].score[num]);
	    }
	    assign_cfeature(&(cpm_ptr->pred_b_ptr->f), feature_buffer, FALSE);
	}
    }
}

/*==================================================================*/
void record_closest_cc_match(SENTENCE_DATA *sp, CF_PRED_MGR *cpm_ptr)
/*==================================================================*/
{
    /* 用言について、直前格要素のマッチスコアをfeatureに */

    int num, pos, closest;
    char feature_buffer[DATA_LEN];

    if (!check_feature(cpm_ptr->pred_b_ptr->f, "用言")) {
	return;
    }

    if ((closest = get_closest_case_component(sp, cpm_ptr)) >= 0 && 
	cpm_ptr->elem_b_ptr[closest]->num + 1 == cpm_ptr->pred_b_ptr->num) {
	num = cpm_ptr->cmm[0].result_lists_d[0].flag[closest];
	if (num != NIL_ASSIGNED) {
	    pos = cpm_ptr->cmm[0].result_lists_p[0].pos[num];
	    if (pos != MATCH_SUBJECT) {
		sprintf(feature_buffer, "直前格マッチ:%d", 
			(int)cpm_ptr->cmm[0].result_lists_p[0].score[num]);
		assign_cfeature(&(cpm_ptr->pred_b_ptr->f), feature_buffer, FALSE);
	    }
	}
    }
}

/*==================================================================*/
  void assign_nil_assigned_components(SENTENCE_DATA *sp, CF_PRED_MGR *cpm_ptr)
/*==================================================================*/
{
    int i, c;

    if (cpm_ptr->score < 0) {
	return;
    }

    /* 未対応の格要素の処理 */

    for (i = 0; i < cpm_ptr->cf.element_num; i++) {
	if (cpm_ptr->cmm[0].result_lists_d[0].flag[i] == NIL_ASSIGNED) {
	    /* 未格, 連格 */
	    if (cpm_ptr->elem_b_num[i] == -1) {
		/* <時間> => 時間 */
		if (check_feature(cpm_ptr->elem_b_ptr[i]->f, "時間")) {
		    if (check_cf_case(cpm_ptr->cmm[0].cf_ptr, "時間") < 0) {
			add_cf_slot(cpm_ptr, "時間", i);
		    }
		}
		/* 二重主語構文の外のガ格 */
		else if (cpm_ptr->elem_b_ptr[i]->num < cpm_ptr->pred_b_ptr->num && 
			 check_feature(cpm_ptr->elem_b_ptr[i]->f, "係:未格") && 
			 cpm_ptr->pred_b_ptr->num != cpm_ptr->elem_b_ptr[i]->num+1 && /* 用言の直前ではない (実は、もうひとつのガ格よりも前にあることを条件にしたい) */
			 check_ga2_ok(cpm_ptr)) {
		    if (check_cf_case(cpm_ptr->cmm[0].cf_ptr, "ガ２") < 0) {
			add_cf_slot(cpm_ptr, "ガ２", i);
		    }
		}
		/* その他 => 外の関係
		   複合名詞の前側: 保留
		   用言直前のノ格: 保留 */
		else if (cpm_ptr->cf.type != CF_NOUN && 
			 !(cpm_ptr->elem_b_ptr[i]->inum > 0 && 
			   cpm_ptr->elem_b_ptr[i]->parent == cpm_ptr->pred_b_ptr) && 
			 cpm_ptr->cf.pp[i][0] != pp_kstr_to_code("未") && 
			 MatchPP2(cpm_ptr->cf.pp[i], "外の関係")) { /* 「外の関係」の可能性あるもの */
		    if ((c = check_cf_case(cpm_ptr->cmm[0].cf_ptr, "外の関係")) < 0) {
			add_cf_slot(cpm_ptr, "外の関係", i);
		    }
		    else {
			assign_cf_slot(cpm_ptr, c, i);
		    }
		}
	    }
	    /* 格は明示されているが、格フレーム側にその格がなかった場合 */
	    /* ★ とりうる格が複数あるとき: ヘ格 */
	    else {
		if (check_cf_case(cpm_ptr->cmm[0].cf_ptr, pp_code_to_kstr(cpm_ptr->cf.pp[i][0])) < 0) {
		    add_cf_slot(cpm_ptr, pp_code_to_kstr(cpm_ptr->cf.pp[i][0]), i);
		}
	    }
	}
    }
}

/*==================================================================*/
char *make_cc_string(char *word, int tag_n, char *pp_str, int cc_type,
		     int dist, char *sid)
/*==================================================================*/
{
    char *buf;
    int word_allocated_flag = 1;

    if (word == NULL) { /* 文字列がないとき */
        word = "(null)";
        word_allocated_flag = 0;
    }

    buf = (char *)malloc_data(strlen(pp_str) + strlen(word) + strlen(sid) + (dist ? log(dist) : 0) + 11, 
			      "make_cc_string");

    if (tag_n < 0) { /* 後処理により併合された基本句 */
        if (OptDisplay == OPT_SIMPLE)
            sprintf(buf, "%s/-", pp_str);
        else
            sprintf(buf, "%s/U/-/-/-/-", pp_str);
    }
    else {
        if (OptDisplay == OPT_SIMPLE)
            sprintf(buf, "%s/%s", 
                    pp_str, 
                    word);
        else
            sprintf(buf, "%s/%c/%s/%d/%d/%s", 
                    pp_str, 
                    cc_type == -2 ? 'O' : 	/* 省略 */
                    cc_type == -3 ? 'D' : 	/* 照応 */
                    cc_type == -1 ? 'N' : 'C', 
                    word, 
                    tag_n, 
                    dist, 
                    sid);
    }

    if (word_allocated_flag)
        free(word);

    return buf;
}

/*==================================================================*/
void append_cf_feature(FEATURE **fpp, CF_PRED_MGR *cpm_ptr, CASE_FRAME *cf_ptr, int n)
/*==================================================================*/
{
    char feature_buffer[DATA_LEN];

    /* 格フレームのガ格が<主体準>をもつかどうか */
    if ((cf_ptr->etcflag & CF_GA_SEMI_SUBJECT) && 
	MatchPP(cf_ptr->pp[n][0], "ガ")) {
	sprintf(feature_buffer, "Ｔ格フレーム-%s-主体準", pp_code_to_kstr_in_context(cpm_ptr, cf_ptr->pp[n][0]));
	assign_cfeature(fpp, feature_buffer, FALSE);
    }
    /* 格フレームが<主体>をもつかどうか */
    else if (cf_match_element(cf_ptr->sm[n], "主体", FALSE)) {
	sprintf(feature_buffer, "Ｔ格フレーム-%s-主体", pp_code_to_kstr_in_context(cpm_ptr, cf_ptr->pp[n][0]));
	assign_cfeature(fpp, feature_buffer, FALSE);
    }

    /* 格フレームが<補文>をもつかどうか *
    if (cf_match_element(cf_ptr->sm[n], "補文", TRUE)) {
	sprintf(feature_buffer, "Ｔ格フレーム-%s-補文", pp_code_to_kstr_in_context(cpm_ptr, cf_ptr->pp[n][0]));
	assign_cfeature(fpp, feature_buffer, FALSE);
    }
    */
}

/*==================================================================*/
void assign_case_component_feature(SENTENCE_DATA *sp, CF_PRED_MGR *cpm_ptr, 
				   int temp_assign_flag)
/*==================================================================*/
{
    int i, num;
    char feature_buffer[DATA_LEN], *word;

    /* 用言の各スロットの割り当てを用言featureに */
    for (i = 0; i < cpm_ptr->cmm[0].cf_ptr->element_num; i++) {
	num = cpm_ptr->cmm[0].result_lists_p[0].flag[i];

	/* 割り当てなし */
	if (num == UNASSIGNED) {
	    sprintf(feature_buffer, "格要素-%s:NIL", 
		    pp_code_to_kstr_in_context(cpm_ptr, cpm_ptr->cmm[0].cf_ptr->pp[i][0]));
	}
	/* 割り当てあり (省略以外の場合) */
	else if (cpm_ptr->elem_b_num[num] > -2) {
	    word = make_print_string(cpm_ptr->elem_b_ptr[num], 0);
	    if (word) {
		sprintf(feature_buffer, "格要素-%s:%s", 
			pp_code_to_kstr_in_context(cpm_ptr, cpm_ptr->cmm[0].cf_ptr->pp[i][0]), word);
		free(word);
	    }
	}

	assign_cfeature(&(cpm_ptr->pred_b_ptr->f), feature_buffer, temp_assign_flag);
    }
}

/*==================================================================*/
void cat_case_analysis_result_parallel_child(char *buffer, CF_PRED_MGR *cpm_ptr, int cf_i, 
					     int dist_n, char *sid)
/*==================================================================*/
{
    int j;
    char *cp;
    int num = cpm_ptr->cmm[0].result_lists_p[0].flag[cf_i];

    /* 並列の子供を取得して、bufferにcat */

    /* 省略の先行詞の場合: elem_b_ptrの親がpara_top_p */
    if (!cpm_ptr->cf.type == CF_NOUN && 
	cpm_ptr->elem_b_ptr[num]->para_type == PARA_NORMAL && 
	cpm_ptr->elem_b_ptr[num]->parent && 
	cpm_ptr->elem_b_ptr[num]->parent->para_top_p) {
	for (j = 0; cpm_ptr->elem_b_ptr[num]->parent->child[j]; j++) {
	    if (cpm_ptr->elem_b_ptr[num] == cpm_ptr->elem_b_ptr[num]->parent->child[j] || /* target自身 */
		cpm_ptr->elem_b_ptr[num]->parent->child[j]->para_type != PARA_NORMAL || /* 並列ではない */
		(cpm_ptr->pred_b_ptr->num < cpm_ptr->elem_b_ptr[num]->num && /* 連体修飾の場合は、 */
		 (cpm_ptr->elem_b_ptr[num]->parent->child[j]->num < cpm_ptr->pred_b_ptr->num || /* 用言より前はいけない */
		  cpm_ptr->elem_b_ptr[num]->num < cpm_ptr->elem_b_ptr[num]->parent->child[j]->num))) { /* 新たな並列の子が元の子より後はいけない */
		continue;
	    }
	    cp = make_cc_string(make_print_string(cpm_ptr->elem_b_ptr[num]->parent->child[j], 0), cpm_ptr->elem_b_ptr[num]->parent->child[j]->num, 
				pp_code_to_kstr_in_context(cpm_ptr, cpm_ptr->cmm[0].cf_ptr->pp[cf_i][0]), 
				cpm_ptr->elem_b_num[num], dist_n, sid ? sid : "?");
	    strcat(buffer, cp);
	    strcat(buffer, ";");
	    free(cp);
	}
    }

    /* 直接の係り受けの場合: elem_b_ptrがpara_top_p */
    if (cpm_ptr->elem_b_ptr[num]->para_top_p) {
	for (j = 1; cpm_ptr->elem_b_ptr[num]->child[j]; j++) { /* 0は自分と同じでチェックされている */
	    if (cpm_ptr->elem_b_ptr[num]->child[j]->para_type == PARA_NORMAL && 
		(cpm_ptr->pred_b_ptr->num > cpm_ptr->elem_b_ptr[num]->num || 
		 cpm_ptr->elem_b_ptr[num]->child[j]->num > cpm_ptr->pred_b_ptr->num)) { /* 連体修飾の場合は用言より後のみ */
		cp = make_cc_string(make_print_string(cpm_ptr->elem_b_ptr[num]->child[j], 0), cpm_ptr->elem_b_ptr[num]->child[j]->num, 
				    pp_code_to_kstr_in_context(cpm_ptr, cpm_ptr->cmm[0].cf_ptr->pp[cf_i][0]), 
				    cpm_ptr->elem_b_num[num], dist_n, sid ? sid : "?");
		strcat(buffer, cp);
		strcat(buffer, ";");
		free(cp);
	    }
	}
    }
}

/*==================================================================*/
void assign_feature_samecase(CF_PRED_MGR *cpm_ptr, int temp_assign_flag)
/*==================================================================*/
{
    int i;
    char feature_buffer[DATA_LEN], *case_str1, *case_str2;

    for (i = 0; cpm_ptr->cmm[0].cf_ptr->samecase[i][0] != END_M; i++) {
	/* 格が存在し、「修飾」ではない */
	if ((case_str1 = pp_code_to_kstr(cpm_ptr->cmm[0].cf_ptr->samecase[i][0])) && strcmp(case_str1, "修飾")) {
	    if ((case_str2 = pp_code_to_kstr(cpm_ptr->cmm[0].cf_ptr->samecase[i][1])) && strcmp(case_str2, "修飾")) {
		sprintf(feature_buffer, "類似格;%s＝%s", case_str1, case_str2);
		assign_cfeature(&(cpm_ptr->pred_b_ptr->f), feature_buffer, temp_assign_flag);
	    }
	}
    }
}

/*==================================================================*/
     int find_aligned_case(CF_ALIGNMENT *cf_align, int src_case)
/*==================================================================*/
{
    int i;

    for (i = 0; cf_align->aligned_case[i][0] != END_M; i++) {
	if (cf_align->aligned_case[i][0] == src_case) {
	    return cf_align->aligned_case[i][1];
	}
    }

    return src_case;
}

/*==================================================================*/
ELLIPSIS_COMPONENT *CheckEllipsisComponent(ELLIPSIS_COMPONENT *ccp, char *pp_str)
/*==================================================================*/
{
    if (!pp_str) {
	return ccp;
    }
    else {
	while (ccp) {
	    if (ccp->pp_str && !strcmp(ccp->pp_str, pp_str)) {
		return ccp;
	    }
	    ccp = ccp->next;
	}
    }
    return NULL;
}

/*==================================================================*/
char *_retrieve_parent_case_component(SENTENCE_DATA *sp, TAG_DATA *pred_ptr, 
                                      int target_case_num, char *case_str)
/*==================================================================*/
{
    int i, num, case_num;
    CF_PRED_MGR *cpm_ptr;
    TAG_DATA *parent_ptr = pred_ptr->parent, *elem_b_ptr;

    while (parent_ptr && check_feature(parent_ptr->f, "機能的基本句")) {
        if (parent_ptr->cpm_ptr) {
            cpm_ptr = parent_ptr->cpm_ptr;
            /* それぞれの格要素 */
            for (i = 0; i < cpm_ptr->cmm[0].cf_ptr->element_num; i++) {
                num = cpm_ptr->cmm[0].result_lists_p[0].flag[i];
                if (num != UNASSIGNED) { /* 割り当てあり */
                    case_num = cpm_ptr->cmm[0].cf_ptr->pp[i][0];
                    if (case_num == target_case_num) { /* 元の格と同じ格 */
                        elem_b_ptr = cpm_ptr->elem_b_ptr[num];
                        if (elem_b_ptr->num != pred_ptr->num) /* 述語と同じならダメ */
                            return make_cc_string(make_print_string(elem_b_ptr, 0), elem_b_ptr->num, case_str,  
                                                  cpm_ptr->elem_b_num[num], 0, sp->KNPSID ? sp->KNPSID + 5 : "?");
                    }
                }
            }
        }
        parent_ptr = parent_ptr->parent;
    }
    return NULL;
}

/*==================================================================*/
char *retrieve_parent_case_component(SENTENCE_DATA *sp, TAG_DATA *pred_ptr, 
                                     int target_case_num, char *case_str)
/*==================================================================*/
{
    char *cp;

    /* 「ガ」のときはまず「ガ２」を探しにいく */
    if (MatchPP(target_case_num, "ガ") && 
        (cp = _retrieve_parent_case_component(sp, pred_ptr, pp_kstr_to_code("ガ２"), case_str)))
        return cp;
    else
        return _retrieve_parent_case_component(sp, pred_ptr, target_case_num, case_str);
}

/*==================================================================*/
void record_case_analysis_result(SENTENCE_DATA *sp, CF_PRED_MGR *cpm_ptr, 
				 ELLIPSIS_MGR *em_ptr, int temp_assign_flag, 
				 char *feature_head, CF_ALIGNMENT *cf_align)
/*==================================================================*/
{
    int i, num, dist_n, sent_n, tag_n, first_arg_flag = 1, case_num;
    char feature_buffer[DATA_LEN], buffer[DATA_LEN], *sid, *cp, *case_str;
    ELLIPSIS_COMPONENT *ccp;
    TAG_DATA *elem_b_ptr, *pred_b_ptr = cpm_ptr->pred_b_ptr;

    /* 格フレーム側からの記述
       => ★「格要素-ガ」などを集めるように修正する */

    /* 述語が後処理により併合された場合: 併合された先の基本句を探す */
    while (pred_b_ptr->num < 0) {
	pred_b_ptr--;
    }

    /* 格フレームID */
    sprintf(feature_buffer, "%s", feature_head);
    if (OptDisplay != OPT_SIMPLE) {
        strcat(feature_buffer, ":");
        strcat(feature_buffer, cf_align ? cf_align->cf_id : cpm_ptr->cmm[0].cf_ptr->cf_id);
    }

    /* それぞれの格要素 */
    for (i = 0; i < cpm_ptr->cmm[0].cf_ptr->element_num; i++) {
	num = cpm_ptr->cmm[0].result_lists_p[0].flag[i];
	ccp = em_ptr ? CheckEllipsisComponent(&(em_ptr->cc[cpm_ptr->cmm[0].cf_ptr->pp[i][0]]), 
					      cpm_ptr->cmm[0].cf_ptr->pp_str[i]) : NULL;
	case_num = cf_align ? find_aligned_case(cf_align, cpm_ptr->cmm[0].cf_ptr->pp[i][0]) : cpm_ptr->cmm[0].cf_ptr->pp[i][0];
	if (case_num == END_M) { /* 正規化時に、対応先の格がない(NIL)場合 */
	    continue;
	}
	case_str = pp_code_to_kstr_in_context(cpm_ptr, case_num);

	/* 割り当てなし */
	if (num == UNASSIGNED) { /* 正規化時は割り当てなしを表示しない, 通常は必須格のみ(-print-case-all-slot時はすべて) */
	    if (!cf_align && ((OptCaseFlag & OPT_CASE_PRINT_ALL_SLOT) || 
                              (cpm_ptr->cmm[0].cf_ptr->oblig[i] == TRUE && 
                               !MatchPP(cpm_ptr->cmm[0].cf_ptr->pp[i][0], "修飾") && 
                               !MatchPP(cpm_ptr->cmm[0].cf_ptr->pp[i][0], "外の関係")))) {
		if (first_arg_flag) /* 格フレームIDの後の":" */
		    strcat(feature_buffer, ":");
                else /* 2つ目以降の格要素なら区切り";"を出力 */
                    strcat(feature_buffer, ";");
		first_arg_flag = 0;
                if (OptDisplay == OPT_SIMPLE)
                    sprintf(buffer, "%s/-", case_str);
                else
                    sprintf(buffer, "%s/U/-/-/-/-", case_str);
                /* 割り当てない場合に親(機能的基本句)から項を取得する */
                if ((OptCaseFlag & OPT_CASE_POSTPROCESS_PA) && 
                    !check_feature(pred_b_ptr->f, "機能的基本句") && /* 自分は機能的基本句ではない */
                    (cp = retrieve_parent_case_component(sp, pred_b_ptr, case_num, case_str))) {
                    strcat(feature_buffer, cp);
                    free(cp);
                }
                else
                    strcat(feature_buffer, buffer);
	    }
	}
	/* 割り当てあり */
	else {
	    if (first_arg_flag) /* 格フレームIDの後の":" */
		strcat(feature_buffer, ":");
            else /* 2つ目以降の格要素なら区切り";"を出力 */
                strcat(feature_buffer, ";");
	    first_arg_flag = 0;
	    /* 例外タグ */
	    if (cpm_ptr->elem_b_num[num] <= -2 && cpm_ptr->elem_s_ptr[num] == NULL) {
                if (OptDisplay == OPT_SIMPLE)
                    sprintf(buffer, "%s/%s", case_str, ETAG_name[2]); /* 不特定-人 */
                else
                    sprintf(buffer, "%s/E/%s/-/-/-", case_str, ETAG_name[2]); /* 不特定-人 */
		strcat(feature_buffer, buffer);
	    }
	    else {
		/* 省略の場合 (特殊タグ以外) */
		if (cpm_ptr->elem_b_num[num] <= -2) {
		    sid = cpm_ptr->elem_s_ptr[num]->KNPSID ? 
			cpm_ptr->elem_s_ptr[num]->KNPSID + 5 : NULL;
		    dist_n = sp->Sen_num - cpm_ptr->elem_s_ptr[num]->Sen_num;
		    sent_n = cpm_ptr->elem_s_ptr[num]->Sen_num;
		}
		/* 同文内 */
		else {
		    sid = sp->KNPSID ? sp->KNPSID + 5 : NULL;
		    dist_n = 0;
		    sent_n = sp->Sen_num;
		}

		/* 並列の子供 */
		cat_case_analysis_result_parallel_child(feature_buffer, cpm_ptr, i, dist_n, sid);

		if (cpm_ptr->elem_b_ptr[num]->num < 0) { /* 後処理により併合された基本句 */
		    elem_b_ptr = cpm_ptr->elem_b_ptr[num];
		    while (elem_b_ptr->num < 0) {
			elem_b_ptr--;
		    }
		    if (elem_b_ptr->num >= pred_b_ptr->num) { /* 併合された連体修飾は用言自身になってしまうので非表示 */
			tag_n = -1;
		    }
		    else {
			tag_n = elem_b_ptr->num;
		    }
		}
		else {
		    elem_b_ptr = cpm_ptr->elem_b_ptr[num];
		    tag_n = elem_b_ptr->num;
		}
		cp = make_cc_string(make_print_string(elem_b_ptr, 0), tag_n, case_str, 
				    cpm_ptr->elem_b_num[num], dist_n, sid ? sid : "?");
		strcat(feature_buffer, cp);
		free(cp);

		/* 格・省略関係の保存 (文脈解析用) */
		if (OptEllipsis) {
		    RegisterTagTarget(pred_b_ptr->head_ptr->Goi, 
				      pred_b_ptr->voice, 
				      cpm_ptr->cmm[0].cf_ptr->cf_address, 
				      cpm_ptr->cmm[0].cf_ptr->pp[i][0], 
				      cpm_ptr->cmm[0].cf_ptr->type == CF_NOUN ? cpm_ptr->cmm[0].cf_ptr->pp_str[i] : NULL, 
				      make_print_string(elem_b_ptr, 0), sent_n, tag_n, CREL);
		}
	    }
	}
    }

    assign_cfeature(&(pred_b_ptr->f), feature_buffer, temp_assign_flag);
}

/*==================================================================*/
void record_case_analysis(SENTENCE_DATA *sp, CF_PRED_MGR *cpm_ptr, 
			  ELLIPSIS_MGR *em_ptr, int temp_assign_flag)
/*==================================================================*/
{
    /* temp_assign_flag: TRUEのときfeatureを「仮付与」する */

    int i, num;
    char feature_buffer[DATA_LEN], *relation, *case_str, *word, *sid, *cp;
    TAG_DATA *elem_b_ptr, *pred_b_ptr = cpm_ptr->pred_b_ptr;

    /* 述語が後処理により併合された場合: 併合された先の基本句を探す */
    while (pred_b_ptr->num < 0) {
	pred_b_ptr--;
    }

    /* voice 決定 */
    if (pred_b_ptr->voice == VOICE_UNKNOWN) {
	decide_voice(sp, cpm_ptr);
    }

    /* 主節かどうかチェック
    check_feature(pred_b_ptr->f, "主節")
    */

    /* 「格フレーム変化」フラグがついている格フレームを使用した場合 */
    if (cpm_ptr->cmm[0].cf_ptr->etcflag & CF_CHANGE) {
	assign_cfeature(&(pred_b_ptr->f), "格フレーム変化", temp_assign_flag);
    }

    /* 類似格をfeatureに */
    assign_feature_samecase(cpm_ptr, temp_assign_flag);

    /* 入力側の各格要素の記述 */
    for (i = 0; i < cpm_ptr->cf.element_num; i++) {
	/* 省略解析の結果は除く
	   指示詞の解析をする場合は、指示詞を除く */
	if (cpm_ptr->elem_b_num[i] <= -2) {
	    continue;
	}
	/* 後処理により併合された連体修飾詞を除く */
	if (cpm_ptr->elem_b_ptr[i]->num < 0) {
	    elem_b_ptr = cpm_ptr->elem_b_ptr[i];
	    while (elem_b_ptr->num < 0) {
		elem_b_ptr--;
	    }
	    if (elem_b_ptr->num >= pred_b_ptr->num) { /* 連体修飾 */
		continue;
	    }
	}
	else {
	    elem_b_ptr = cpm_ptr->elem_b_ptr[i];
	}

	num = cpm_ptr->cmm[0].result_lists_d[0].flag[i];

	/* 割り当てなし */
	if (num == NIL_ASSIGNED) {
	    continue;
	}
	/* 割り当てられている格 */
	else if (num >= 0) {
	    case_str = pp_code_to_kstr_in_context(cpm_ptr, cpm_ptr->cmm[0].cf_ptr->pp[num][0]);
            relation = (char *)malloc_data(strlen(case_str) + 2, "record_case_analysis");
            strcpy(relation, case_str);
            if ((OptCaseFlag & OPT_CASE_PRINT_OBLIG) && cpm_ptr->cmm[0].cf_ptr->oblig[num] == FALSE) /* 任意格情報を表示する場合 */
                strcat(relation, "*");
	}
	/* else: UNASSIGNED はないはず */

	/* featureを格要素文節に与える */
	if (elem_b_ptr->num < pred_b_ptr->num) {
	    sprintf(feature_buffer, "解析格:%s", relation);
	}
	else {
	    sprintf(feature_buffer, "解析連格:%s", relation);
	}
	assign_cfeature(&(elem_b_ptr->f), feature_buffer, temp_assign_flag);

	/* feature を用言文節に与える */
	word = make_print_string(elem_b_ptr, 0);
	if (word) {
            if (OptCaseFlag & OPT_CASE_PRINT_SCORE) /* 格ごとのスコアを出す場合 */
		sprintf(feature_buffer, "格関係%d:%s:%s:%.3f", 
			elem_b_ptr->num >= 0 ? elem_b_ptr->num : elem_b_ptr->parent->num, 
			relation, word, cpm_ptr->cmm[0].result_lists_d[0].score[i]);
            else
		sprintf(feature_buffer, "格関係%d:%s:%s", 
			elem_b_ptr->num >= 0 ? elem_b_ptr->num : elem_b_ptr->parent->num, 
			relation, word);
	    assign_cfeature(&(pred_b_ptr->f), feature_buffer, temp_assign_flag);
	    free(word);
	}
        free(relation);
    }

    /* 格フレーム側からの格解析結果の記述 */
    record_case_analysis_result(sp, cpm_ptr, em_ptr, temp_assign_flag, "格解析結果", NULL);

    /* 正規化格解析結果 */
    for (i = 0; cpm_ptr->cmm[0].cf_ptr->cf_align[i].cf_id != NULL; i++) {
	sprintf(feature_buffer, "正規化格解析結果-%d", i);
	record_case_analysis_result(sp, cpm_ptr, em_ptr, temp_assign_flag, feature_buffer, &(cpm_ptr->cmm[0].cf_ptr->cf_align[i]));
    }
}

/*==================================================================*/
void record_all_case_analisys(SENTENCE_DATA *sp, int temp_assign_flag)
/*==================================================================*/
{
    int i;

    for (i = 0; i < sp->Best_mgr->pred_num; i++) {
	if (sp->Best_mgr->cpm[i].pred_b_ptr == NULL) { /* 述語ではないと判断したものはスキップ */
	    continue;
	}
	if (((OptCaseFlag & OPT_CASE_USE_PROBABILITY) && 
             sp->Best_mgr->cpm[i].cmm[0].score != CASE_MATCH_FAILURE_PROB) || 
            (!(OptCaseFlag & OPT_CASE_USE_PROBABILITY) && 
             sp->Best_mgr->cpm[i].cmm[0].score != CASE_MATCH_FAILURE_SCORE)) {
	    record_case_analysis(sp, &(sp->Best_mgr->cpm[i]), NULL, temp_assign_flag);
	}
    }
}

/*==================================================================*/
   void decide_alt_mrph(MRPH_DATA *m_ptr, int alt_num, char *f_str)
/*==================================================================*/
{
    if (alt_num == 0) {
	assign_cfeature(&(m_ptr->f), f_str, FALSE);
    }
    else {
	int alt_count = 1;
	FEATURE *fp = m_ptr->f;
	MRPH_DATA m;

	/* ALTをチェック */
	while (fp) {
	    if (!strncmp(fp->cp, "ALT-", 4)) {
		if (alt_count == alt_num) { /* target */
		    sscanf(fp->cp + 4, "%[^-]-%[^-]-%[^-]-%d-%d-%d-%d-%[^\n]", 
			   m.Goi2, m.Yomi, m.Goi, 
			   &m.Hinshi, &m.Bunrui, 
			   &m.Katuyou_Kata, &m.Katuyou_Kei, m.Imi);
		    /* 現在の形態素をALTに保存 */
		    assign_feature_alt_mrph(&(m_ptr->f), m_ptr);
		    /* このALTを最終結果の形態素にする */
		    delete_existing_features(m_ptr); /* 現在の形態素featureを削除 */
		    copy_mrph(m_ptr, &m, TRUE);
		    delete_cfeature(&(m_ptr->f), fp->cp);
		    assign_cfeature(&(m_ptr->f), f_str, FALSE);
		    break;
		}
		alt_count++;
	    }
	    fp = fp->next;
	}
    }
}

/*==================================================================*/
int _noun_lexical_disambiguation_by_case_analysis(CF_PRED_MGR *cpm_ptr, int i, int exact_flag)
/*==================================================================*/
{
    /* 格解析結果から名詞の曖昧性解消を行う

    対象の形態素: cpm_ptr->elem_b_ptr[i]->head_ptr */

    int num, pos, expand, alt_num, alt_count, rep_length, rep_malloc_flag = 0;
    char *rep_strt, *exd;
    float score, tmp_score;
    FEATURE *fp;
    MRPH_DATA m;

    num = cpm_ptr->cmm[0].result_lists_d[0].flag[i];
    alt_num = -1;
    score = 0;
    alt_count = 0;

    if (!exact_flag) {
	if (check_feature(cpm_ptr->elem_b_ptr[i]->f, "Ｔ固有一般展開禁止")) {
	    expand = SM_NO_EXPAND_NE;
	}
	else {
	    expand = SM_EXPAND_NE;
	}
    }

    /* まず現在の形態素をチェック */
    if (OptCaseFlag & OPT_CASE_USE_REP_CF) { 
	rep_strt = get_mrph_rep(cpm_ptr->elem_b_ptr[i]->head_ptr); /* 代表表記 */
	rep_length = get_mrph_rep_length(rep_strt);
	if (rep_length == 0) { /* なければ作る */
	    rep_strt = make_mrph_rn(cpm_ptr->elem_b_ptr[i]->head_ptr);
	    rep_length = strlen(rep_strt);
	    rep_malloc_flag = 1;
	}
    }
    else {
	rep_strt = cpm_ptr->elem_b_ptr[i]->head_ptr->Goi;
	rep_length = strlen(rep_strt);
    }
    if (rep_strt && rep_length) {
	if (exact_flag) { /* exact matchによるチェック */
	    if (cf_match_exactly(rep_strt, rep_length, 
				 cpm_ptr->cmm[0].cf_ptr->ex_list[num], 
				 cpm_ptr->cmm[0].cf_ptr->ex_num[num], &pos)) {
		score = cpm_ptr->cmm[0].cf_ptr->ex_freq[num][pos];
		alt_num = alt_count; /* 0 */
	    }
	}
	else { /* 意味素によるチェック */
	    if (exd = get_str_code_with_len(rep_strt, rep_length, Thesaurus)) {
		score = _calc_similarity_sm_cf(exd, expand, 
					       cpm_ptr->elem_b_ptr[i]->head_ptr->Goi2, 
					       cpm_ptr->cmm[0].cf_ptr, num, &pos);
		if (score > 0) {
		    alt_num = alt_count; /* 0 */
		}
		free(exd);
	    }
	}
    }

    if (rep_malloc_flag) {
	free(rep_strt);
    }

    /* ALTをチェック */
    alt_count++;
    fp = cpm_ptr->elem_b_ptr[i]->head_ptr->f;
    while (fp) {
	if (!strncmp(fp->cp, "ALT-", 4)) {
	    rep_malloc_flag = 0;
	    sscanf(fp->cp + 4, "%[^-]-%[^-]-%[^-]-%d-%d-%d-%d-%[^\n]", 
		   m.Goi2, m.Yomi, m.Goi, 
		   &m.Hinshi, &m.Bunrui, 
		   &m.Katuyou_Kata, &m.Katuyou_Kei, m.Imi);
	    if (OptCaseFlag & OPT_CASE_USE_REP_CF) {
		rep_strt = get_mrph_rep(&m); /* 代表表記 */
		rep_length = get_mrph_rep_length(rep_strt);
		if (rep_length == 0) { /* なければ作る */
		    rep_strt = make_mrph_rn(&m);
		    rep_length = strlen(rep_strt);
		    rep_malloc_flag = 1;
		}
	    }
	    else {
		rep_strt = m.Goi;
		rep_length = strlen(rep_strt);
	    }
	    if (rep_strt && rep_length) {
		if (exact_flag) { /* exact matchによるチェック */
		    if (cf_match_exactly(rep_strt, rep_length, 
					 cpm_ptr->cmm[0].cf_ptr->ex_list[num], 
					 cpm_ptr->cmm[0].cf_ptr->ex_num[num], &pos)) {
			tmp_score = cpm_ptr->cmm[0].cf_ptr->ex_freq[num][pos];
			if (score < tmp_score) {
			    score = tmp_score;
			    alt_num = alt_count;
			}
		    }
		}
		else { /* 意味素によるチェック */
		    if (exd = get_str_code_with_len(rep_strt, rep_length, Thesaurus)) {
			tmp_score = _calc_similarity_sm_cf(exd, expand, 
							   cpm_ptr->elem_b_ptr[i]->head_ptr->Goi2, 
							   cpm_ptr->cmm[0].cf_ptr, num, &pos);
			if (score < tmp_score) {
			    score = tmp_score;
			    alt_num = alt_count;
			}
			free(exd);
		    }
		}
	    }

	    if (rep_malloc_flag) {
		free(rep_strt);
	    }
	    alt_count++;
	}
	fp = fp->next;
    }

    /* 決定 */
    if (alt_num > -1) {
	decide_alt_mrph(cpm_ptr->elem_b_ptr[i]->head_ptr, alt_num, "名詞曖昧性解消");
	return TRUE;
    }
    return FALSE;
}

/*==================================================================*/
void noun_lexical_disambiguation_by_case_analysis(CF_PRED_MGR *cpm_ptr)
/*==================================================================*/
{
    int i;

    for (i = 0; i < cpm_ptr->cf.element_num; i++) {
	if (!cpm_ptr->elem_b_ptr[i] || 
	    !check_feature(cpm_ptr->elem_b_ptr[i]->head_ptr->f, "品曖") || /* 曖昧な形態素 */
	    check_feature(cpm_ptr->elem_b_ptr[i]->head_ptr->f, "用言曖昧性解消")) {
	    continue;
	}
	/* 省略の格要素、格フレームとあまりマッチしないときなどは対象としない */
	else if (cpm_ptr->elem_b_num[i] < -1 || /* 省略ではない */
		 cpm_ptr->cmm[0].result_lists_d[0].flag[i] < 0 || /* 割り当てあり */
		 cpm_ptr->cmm[0].result_lists_p[0].pos[cpm_ptr->cmm[0].result_lists_d[0].flag[i]] == MATCH_SUBJECT || /* <主体>matchではない */
		 ((OptCaseFlag & OPT_CASE_USE_PROBABILITY) && cpm_ptr->cmm[0].result_lists_d[0].score[i] < FREQ0_ASSINED_SCORE) || /* スコアが必要 */
		 (!(OptCaseFlag & OPT_CASE_USE_PROBABILITY) && cpm_ptr->cmm[0].result_lists_d[0].score[i] <= CF_DECIDE_THRESHOLD) || /* スコアが必要 */
		 check_feature(cpm_ptr->elem_b_ptr[i]->head_ptr->f, "音訓解消")) { /*音訓解消はされていない */
	    continue;
	}

	/* exactマッチをチェックして名詞の曖昧性解消 */
	if (!_noun_lexical_disambiguation_by_case_analysis(cpm_ptr, i, 1)) {
	    /* マッチした意味素をもとに名詞の曖昧性解消 */
	    _noun_lexical_disambiguation_by_case_analysis(cpm_ptr, i, 0);
	}
    }
}

/*==================================================================*/
void verb_lexical_disambiguation_by_case_analysis(CF_PRED_MGR *cpm_ptr)
/*==================================================================*/
{
    /* 格解析結果から用言の曖昧性解消を行う */

    char *rep_cp;
    FEATURE *fp;
    MRPH_DATA m;

    /* 直前格が1つ以上割り当てられていることを条件とする */
    if (count_assigned_adjacent_element(cpm_ptr->cmm[0].cf_ptr, &(cpm_ptr->cmm[0].result_lists_p[0])) && 
	(check_feature(cpm_ptr->pred_b_ptr->head_ptr->f, "原形曖昧") || /* 原形が曖昧な用言 */
	 (check_str_type(cpm_ptr->pred_b_ptr->head_ptr->Goi, TYPE_HIRAGANA, 0) && 
	  check_feature(cpm_ptr->pred_b_ptr->head_ptr->f, "品曖"))) && /* 品曖なひらがな */
	!check_feature(cpm_ptr->pred_b_ptr->head_ptr->f, "音訓解消")) { /*音訓解消はされていない */
	/* 現在の形態素でよいとき */
	if ((rep_cp = get_mrph_rep(cpm_ptr->pred_b_ptr->head_ptr)) && 
	    !strncmp(rep_cp, cpm_ptr->cmm[0].cf_ptr->entry, 
		     strlen(cpm_ptr->cmm[0].cf_ptr->entry))) {
	    assign_cfeature(&(cpm_ptr->pred_b_ptr->head_ptr->f), "用言曖昧性解消", FALSE);
	    delete_cfeature(&(cpm_ptr->pred_b_ptr->head_ptr->f), "名詞曖昧性解消"); /* あれば削除 */
	    return;
	}

	/* 現在の形態素代表表記と格フレームの表記が異なる場合のみ形態素を変更 */

	fp = cpm_ptr->pred_b_ptr->head_ptr->f;
	while (fp) {
	    if (!strncmp(fp->cp, "ALT-", 4)) {
		sscanf(fp->cp + 4, "%[^-]-%[^-]-%[^-]-%d-%d-%d-%d-%[^\n]", 
		       m.Goi2, m.Yomi, m.Goi, 
		       &m.Hinshi, &m.Bunrui, 
		       &m.Katuyou_Kata, &m.Katuyou_Kei, m.Imi);
		rep_cp = get_mrph_rep(&m);
		/* 選択した格フレームの表記と一致する代表表記をもつ形態素を選択 */
		if (rep_cp && 
		    !strncmp(rep_cp, cpm_ptr->cmm[0].cf_ptr->entry, 
			     strlen(cpm_ptr->cmm[0].cf_ptr->entry))) {
		    /* 現在の形態素をALTに保存 */
		    assign_feature_alt_mrph(&(cpm_ptr->pred_b_ptr->head_ptr->f), 
					    cpm_ptr->pred_b_ptr->head_ptr);
		    /* このALTを最終結果の形態素にする */
		    delete_existing_features(cpm_ptr->pred_b_ptr->head_ptr); /* 現在の形態素featureを削除 */
		    copy_mrph(cpm_ptr->pred_b_ptr->head_ptr, &m, TRUE);
		    delete_cfeature(&(cpm_ptr->pred_b_ptr->head_ptr->f), fp->cp);
		    assign_cfeature(&(cpm_ptr->pred_b_ptr->head_ptr->f), "用言曖昧性解消", FALSE);
		    delete_cfeature(&(cpm_ptr->pred_b_ptr->head_ptr->f), "名詞曖昧性解消"); /* あれば削除 */
		    break;
		}
	    }
	    fp = fp->next;
	}
    }
}

/*==================================================================*/
       int get_dist_from_work_mgr(BNST_DATA *bp, BNST_DATA *hp)
/*==================================================================*/
{
    int i, dist = 0;

    /* 候補チェック */
    if (Work_mgr.dpnd.check[bp->num].num == -1) {
	return -1;
    }
    for (i = 0; i < Work_mgr.dpnd.check[bp->num].num; i++) {
	if (Work_mgr.dpnd.check[bp->num].pos[i] == hp->num) {
	    dist = ++i;
	    break;
	}
    }
    if (dist == 0) {
	return -1;
    }
    else if (dist > 1) {
	dist = 2;
    }
    return dist;
}

/*====================================================================
                               END
====================================================================*/
