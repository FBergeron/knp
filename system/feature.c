/*====================================================================

			     FEATURE処理

                                               S.Kurohashi 96. 7. 4

    $Id$
====================================================================*/
#include "knp.h"

/*
  FEATUREの処理には次の３種類がある

  	(1) ファイル(S式または文字列) ==コピー==> ルール構造体

	(2) ルール構造体 ==付与==> 形態素または文節構造体
        	<○:□>は<○:…>というFEATUREへの上書き (なければ新規)
                <^○>は<○:…>の削除 (なければ無視)
		<&○>は関数呼出
			&表層:付与 -- 辞書引きによる表層格付与
			&表層:削除 -- すべての表層格削除
			&表層:○格 -- ○格付与
			&表層:^○格 -- ○格削除
			&MEMO:○ -- MEMOへの書き込み

	(3) ルール構造体 <==照合==> 形態素または文節構造体
	       	<○>は<○:…>というFEATUREがあればOK
	    	<^○>は<○:…>というFEATUREがなければOK
	    	<&○>は関数呼出
			&記英数カ -- 表記が記号,英文字,数字,カタカナ (形態素)
			&漢字 -- 表記が漢字 (形態素)
	    		&表層:○格 -- ○格がある (文節)
	    		&表層:照合 -- 係の表層格が受にある (係受)
			&D:n -- 構造体間が距離n以内 (係受)
			&レベル:強 -- 受が係以上 (係受)
			&レベル:l -- 自身がl以上 (係受)
			&係側:○ -- 係に○ (係受)

	※ プログラム内で形態素または文節構造体にFEATUREを与える
	場合は(2)のなかの assign_cfeature を用いる．

	※ プログラム内で形態素または文節構造体があるFEATUREを持つ
	かどうかを調べる場合は(3)のなかの check_feature を用いる．
*/

char feature_buffer[DATA_LEN];

/*==================================================================*/
	    void print_one_feature(char *cp, FILE *filep)
/*==================================================================*/
{
    if (!strncmp(cp, "仮付与:", strlen("仮付与:"))) { /* 仮付与したものを表示するとき用(-nbest) */
	if (OptExpress == OPT_TABLE)
	    fprintf(filep, "＜%s＞", cp + strlen("仮付与:")); 
	else
	    fprintf(filep, "<%s>", cp + strlen("仮付与:")); 
    }
    else {
	if (OptExpress == OPT_TABLE)
	    fprintf(filep, "＜%s＞", cp);
	else
	    fprintf(filep, "<%s>", cp);
    }
}

/*==================================================================*/
               int check_important_feature(FEATURE *fp)
/*==================================================================*/
{
    /* -simpletab でも出力する重要なfeature */

    if (comp_feature(fp->cp, "用言") || 
        comp_feature(fp->cp, "体言") || 
        comp_feature(fp->cp, "格解析結果") || 
        comp_feature(fp->cp, "格構造") || 
        comp_feature(fp->cp, "Wikipediaエントリ") || 
        comp_feature(fp->cp, "NE内") || 
        comp_feature(fp->cp, "NE")) {
        return TRUE;
    }
    else {
        return FALSE;
    }
}

/*==================================================================*/
	      void print_feature(FEATURE *fp, FILE *filep)
/*==================================================================*/
{
    /* <f1><f2> ... <f3> という形式の出力 
       (ただしＴではじまるfeatureは表示しない) */

    while (fp) {
	if (fp->cp && 
	    ((OptDisplay == OPT_SIMPLE && check_important_feature(fp)) || 
	     (OptDisplay != OPT_SIMPLE && strncmp(fp->cp, "Ｔ", strlen("Ｔ"))) || 
	     OptDisplay == OPT_DEBUG))
	    print_one_feature(fp->cp, filep);
	fp = fp->next;
    }
}

/*==================================================================*/
	  void print_some_feature(FEATURE *fp, FILE *filep)
/*==================================================================*/
{
    /* <f1><f2> ... <f3> という形式の出力 
       指定したものだけを表示 */

    while (fp) {
        if (fp->cp && check_important_feature(fp)) {
            print_one_feature(fp->cp, filep);
        }
	fp = fp->next;
    }
}
/*==================================================================*/
	      void print_feature2(FEATURE *fp, FILE *filep)
/*==================================================================*/
{
    /* (f1 f2 ... f3) という形式の出力
       (ただしＴではじまるfeatureは表示しない) */
    if (fp) {
	fprintf(filep, "("); 
	while (fp) {
	    if (fp->cp && strncmp(fp->cp, "Ｔ", strlen("Ｔ"))) {
		fprintf(filep, "%s", fp->cp); 
		if (fp->next) fprintf(filep, " "); 		
	    }
	    fp = fp->next;
	}
	fprintf(filep, ")"); 
    } else {
	fprintf(filep, "NIL"); 
    }
}

/*==================================================================*/
		   void clear_feature(FEATURE **fpp)
/*==================================================================*/
{
    FEATURE *fp, *next;

    fp = *fpp;
    *fpp = NULL;

    while (fp) {
	next = fp->next;
	free(fp->cp);
	free(fp);
	fp = next;
    }
}

/*==================================================================*/
	   void delete_cfeature(FEATURE **fpp, char *type)
/*==================================================================*/
{
    FEATURE *prep = NULL;

    while (*fpp) {
	if (comp_feature((*fpp)->cp, type) == TRUE) {
	    FEATURE *next;
	    free((*fpp)->cp);
	    if (prep == NULL) {
		next = (*fpp)->next;
		free(*fpp);
		*fpp = next;
	    }
	    else {
		next = (*fpp)->next;
		free(*fpp);
		prep->next = next;
	    }
	    return;
	}
	prep = *fpp;
	fpp = &(prep->next);
    }
}

/*==================================================================*/
		void delete_alt_feature(FEATURE **fpp)
/*==================================================================*/
{
    /* <ALT-...>を削除 */

    FEATURE *prep = NULL;

    while (*fpp) {
	if (!strncmp((*fpp)->cp, "ALT-", 4)) {
	    FEATURE *next;
	    free((*fpp)->cp);
	    if (prep == NULL) {
		next = (*fpp)->next;
		free(*fpp);
		*fpp = next; /* prepはNULLのまま */
	    }
	    else { /* prepがあるとき */
		next = (*fpp)->next;
		free(*fpp);
		prep->next = next; /* prepは現状維持 */
		fpp = &(prep->next);
	    }
	}
	else {
	    prep = *fpp;
	    fpp = &(prep->next);
	}
    }
}

/*==================================================================*/
void delete_cfeature_from_mrphs(MRPH_DATA *m_ptr, int length, char *type)
/*==================================================================*/
{
    int i;

    for (i = 0; i < length; i++) {
	delete_cfeature(&((m_ptr + i)->f), type);
    }
}

/*==================================================================*/
	       void delete_temp_feature(FEATURE **fpp)
/*==================================================================*/
{
    /* 仮付与したfeatureを削除 */

    FEATURE *prep = NULL;

    while (*fpp) {
	if (comp_feature((*fpp)->cp, "仮付与") == TRUE) {
	    FEATURE *next;
	    free((*fpp)->cp);
	    if (prep == NULL) {
		next = (*fpp)->next;
		free(*fpp);
		*fpp = next;
	    }
	    else {
		next = (*fpp)->next;
		free(*fpp);
		prep->next = next;
	    }
	    fpp = &(prep->next);
	    continue;
	}
	prep = *fpp;
	fpp = &(prep->next);
    }
}    

/*
 *
 *  ファイル(S式または文字列) ==コピー==> ルール構造体
 *
 */

/*==================================================================*/
	   void copy_cfeature(FEATURE **fpp, char *fname)
/*==================================================================*/
{
    while (*fpp) fpp = &((*fpp)->next);

    if (!((*fpp) = (FEATURE *)(malloc(sizeof(FEATURE)))) ||
	!((*fpp)->cp = (char *)(malloc(strlen(fname) + 1)))) {
	fprintf(stderr, "Can't allocate memory for FEATURE\n");
	exit(-1);
    }
    strcpy((*fpp)->cp, fname);
    (*fpp)->next = NULL;
}

/*==================================================================*/
	      void list2feature(CELL *cp, FEATURE **fpp)
/*==================================================================*/
{
    while (!Null(car(cp))) {
	copy_cfeature(fpp, _Atom(car(cp)));
	fpp = &((*fpp)->next);
	cp = cdr(cp);	    
    }
}

/*==================================================================*/
      void list2feature_pattern(FEATURE_PATTERN *f, CELL *cell)
/*==================================================================*/
{
    /* リスト ((文頭)(体言)(提題)) などをFEATURE_PATTERNに変換 */

    int nth = 0;

    while (!Null(car(cell))) {
	clear_feature(f->fp+nth);		/* ?? &(f->fp[nth]) */ 
	list2feature(car(cell), f->fp+nth);	/* ?? &(f->fp[nth]) */ 
	cell = cdr(cell);
	nth++;
    }
    f->fp[nth] = NULL;
}

/*==================================================================*/
      void string2feature_pattern_OLD(FEATURE_PATTERN *f, char *cp)
/*==================================================================*/
{
    /* 文字列 "文頭|体言|提題" などをFEATURE_PATTERNに変換
       本来list2feature_patternに対応するものだが,
       ORだけでANDはサポートしていない */

    int nth = 0;
    char *scp, *ecp;

    if (cp == NULL || cp[0] == '\0') {
	f->fp[nth] = NULL;
	return;
    }

    strcpy(feature_buffer, cp);
    scp = ecp = feature_buffer;
    while (*ecp) {
	if (*ecp == '|') {
	    *ecp = '\0';
	    clear_feature(f->fp+nth);		/* ?? &(f->fp[nth]) */
	    copy_cfeature(f->fp+nth, scp);	/* ?? &(f->fp[nth]) */
	    nth++;
	    scp = ecp + 1; 
	}
	ecp ++;
    }
    
    clear_feature(f->fp+nth);			/* ?? &(f->fp[nth]) */ 
    copy_cfeature(&(f->fp[nth]), scp);
    nth++;

    f->fp[nth] = NULL;
}

/*==================================================================*/
      void string2feature_pattern(FEATURE_PATTERN *f, char *cp)
/*==================================================================*/
{
    /* 文字列 "文頭|体言|提題" などをFEATURE_PATTERNに変換
       本来list2feature_patternに対応するものだが,
       ORだけでANDはサポートしていない */

    int nth;
    char *start_cp, *loop_cp;
    FEATURE **fpp;
    
    if (!*cp) {
	f->fp[0] = NULL;
	return;
    }

    strcpy(feature_buffer, cp);
    nth = 0;
    clear_feature(f->fp+nth);
    fpp = f->fp+nth;
    loop_cp = feature_buffer;
    start_cp = loop_cp;
    while (*loop_cp) {
	if (*loop_cp == '&' && *(loop_cp+1) == '&') {
	    *loop_cp = '\0';
	    copy_cfeature(fpp, start_cp);
	    fpp = &((*fpp)->next);
	    loop_cp += 2;
	    start_cp = loop_cp;
	}
	else if (*loop_cp == '|' && *(loop_cp+1) == '|') {
	    *loop_cp = '\0';
	    copy_cfeature(fpp, start_cp);
	    nth++;
	    clear_feature(f->fp+nth);
	    fpp = f->fp+nth;
	    loop_cp += 2;
	    start_cp = loop_cp;
	}
	else {
	    loop_cp ++;
	}
    }
    copy_cfeature(fpp, start_cp);

    nth++;
    f->fp[nth] = NULL;
}

/*
 *
 * ルール構造体 ==付与==> 形態素または文節構造体
 *
 */

/*==================================================================*/
	   void append_feature(FEATURE **fpp, FEATURE *afp)
/*==================================================================*/
{
    while (*fpp) {
	fpp = &((*fpp)->next);
    }
    *fpp = afp;
}    

/*==================================================================*/
void assign_cfeature(FEATURE **fpp, char *fname, int temp_assign_flag)
/*==================================================================*/
{
    /* temp_assign_flag: TRUEのとき「仮付与」を頭につける */

    /* 上書きの可能性をチェック */

    sscanf(fname, "%[^:]", feature_buffer);	/* ※ fnameに":"がない場合は
						   feature_bufferはfname全体になる */

    /* quote('"')中の":"で切っていれば、もとに戻す */
    if (strcmp(feature_buffer, fname)) {
	int i, count = 0;

	for (i = 0; i < strlen(feature_buffer); i++) {
	    if (feature_buffer[i] == '"') {
		count++;
	    }
	}
	if (count % 2 == 1) { /* '"'が奇数 */
	    strcpy(feature_buffer, fname);
	}
    }

    while (*fpp) {
	if (comp_feature((*fpp)->cp, feature_buffer) == TRUE) {
	    free((*fpp)->cp);
	    if (!((*fpp)->cp = (char *)(malloc(strlen(fname) + 1)))) {
		fprintf(stderr, "Can't allocate memory for FEATURE\n");
		exit(-1);
	    }
	    strcpy((*fpp)->cp, fname);
	    return;	/* 上書きで終了 */
	}
	fpp = &((*fpp)->next);
    }

    /* 上書きできなければ末尾に追加 */

    if (!((*fpp) = (FEATURE *)(malloc(sizeof(FEATURE)))) ||
	!((*fpp)->cp = (char *)(malloc(strlen(fname) + strlen("仮付与:") + 1)))) {
	fprintf(stderr, "Can't allocate memory for FEATURE\n");
	exit(-1);
    }
    if (temp_assign_flag) {
	strcpy((*fpp)->cp, "仮付与:");
	strcat((*fpp)->cp, fname);
    }
    else {
	strcpy((*fpp)->cp, fname);
    }
    (*fpp)->next = NULL;
}    

/*==================================================================*/
	       char *str_delete_last_column(char *str)
/*==================================================================*/
{
    /* ':'区切りとみなし、2つ目以降のカラムを削除
       例: Wikipedia上位語:企業/きぎょう:0-3 → Wikipedia上位語:企業/きぎょう */

    if (str) {
        int count = 0;
	char *ret = strdup(str), *cp = ret;
	while (cp = strchr(cp, ':')) { /* 前から2つ目の':'を探す */
            if (count == 1) { /* 2つ目の':' */
                *cp = '\0'; /* あれば終端 */
                return ret;
            }
            cp++; /* 次のstrchrのために一つ進める */
            count++;
	}
	return ret;
    }
    else {
	return NULL;
    }
}

/*==================================================================*/
void assign_feature(FEATURE **fpp1, FEATURE **fpp2, void *ptr, int offset, int length, int temp_assign_flag)
/*==================================================================*/
{
    /*
     *  ルールを適用の結果，ルールから構造体にFEATUREを付与する
     *  構造体自身に対する処理も可能としておく
     */

    int i, assign_pos;
    char *cp, *cp2, *pat, buffer[DATA_LEN];
    FEATURE **fpp, *next;

    while (*fpp2) {

	if (*((*fpp2)->cp) == '^') {	/* 削除の場合 */
	    
	    fpp = fpp1;
	    
	    while (*fpp) {
		if (comp_feature((*fpp)->cp, &((*fpp2)->cp[1])) == TRUE) {
		    free((*fpp)->cp);
		    next = (*fpp)->next;
		    free(*fpp);
		    *fpp = next;
		} else {
		    fpp = &((*fpp)->next);
		}
	    }
	
	} else if (*((*fpp2)->cp) == '&') {	/* 関数の場合 */

	    if (!strcmp((*fpp2)->cp, "&表層:付与")) {
		set_pred_voice((BNST_DATA *)ptr + offset);	/* ヴォイス */
		get_scase_code((BNST_DATA *)ptr + offset);	/* 表層格 */
	    }
	    else if (!strcmp((*fpp2)->cp, "&表層:削除")) {
		for (i = 0, cp = ((BNST_DATA *)ptr + offset)->SCASE_code; 
		     i < SCASE_CODE_SIZE; i++, cp++) 
		    *cp = 0;		
	    }
	    else if (!strncmp((*fpp2)->cp, "&表層:^", strlen("&表層:^"))) {
		((BNST_DATA *)ptr + offset)->
		    SCASE_code[case2num((*fpp2)->cp + strlen("&表層:^"))] = 0;
	    }
	    else if (!strncmp((*fpp2)->cp, "&表層:", strlen("&表層:"))) {
		((BNST_DATA *)ptr + offset)->
		    SCASE_code[case2num((*fpp2)->cp + strlen("&表層:"))] = 1;
	    }
	    else if (!strncmp((*fpp2)->cp, "&MEMO:", strlen("&MEMO:"))) {
		strcat(PM_Memo, " ");
		strcat(PM_Memo, (*fpp2)->cp + strlen("&MEMO:"));
	    }
	    else if (!strncmp((*fpp2)->cp, "&品詞変更:", strlen("&品詞変更:"))) {
                if (OptPosModification && !(OptInput & OPT_PARSED)) /* 解析済みではない場合に品詞変更を実行 */
                    change_mrph((MRPH_DATA *)ptr + offset, *fpp2);
	    }
	    else if (!strncmp((*fpp2)->cp, "&代表表記変更:", strlen("&代表表記変更:"))) {
		change_one_mrph_rep((MRPH_DATA *)ptr + offset, 1, *((*fpp2)->cp + strlen("&代表表記変更:")));
	    }
	    else if (!strncmp((*fpp2)->cp, "&意味素付与:", strlen("&意味素付与:"))) {
		assign_sm((BNST_DATA *)ptr + offset, (*fpp2)->cp + strlen("&意味素付与:"));
	    }
	    else if (!strncmp((*fpp2)->cp, "&複合辞格解析", strlen("&複合辞格解析"))) {
		cp = make_fukugoji_case_string((TAG_DATA *)ptr + offset + 1);
		if (cp) {
		    assign_cfeature(&(((TAG_DATA *)ptr + offset)->f), cp, temp_assign_flag);
		}
	    }
	    else if (!strncmp((*fpp2)->cp, "&複合辞ID付与", strlen("&複合辞ID付与"))) {
		cp = make_fukugoji_id((BNST_DATA *)ptr + offset);
		if (cp) {
		    assign_cfeature(&(((BNST_DATA *)ptr + offset)->f), cp, temp_assign_flag);
		}
	    }
	    else if (!strncmp((*fpp2)->cp, "&記憶語彙付与:", strlen("&記憶語彙付与:"))) {
		sprintf(buffer, "%s:%s", 
			(*fpp2)->cp + strlen("&記憶語彙付与:"), 
			((MRPH_DATA *)matched_ptr)->Goi);
		assign_cfeature(&(((BNST_DATA *)ptr + offset)->f), buffer, temp_assign_flag);
	    }
	    /* &記憶FEATURE昇格 : 記憶した形態素からFEATUREを探し、それを基本句に付与 */
	    else if (!strncmp((*fpp2)->cp, "&記憶FEATURE昇格:", strlen("&記憶FEATURE昇格:"))) {
		if (cp = check_feature(((MRPH_DATA *)matched_ptr)->f, (*fpp2)->cp + strlen("&記憶FEATURE付与:"))) {
		    if (cp2 = str_delete_last_column(cp)) { /* ':'区切りの最後のカラムを削除 */
			assign_cfeature(&(((TAG_DATA *)ptr + offset)->f), cp2, temp_assign_flag);
			free(cp2);
		    }
		}
	    }
	    /* &伝搬:n:FEATURE : FEATUREの伝搬  */
	    else if (!strncmp((*fpp2)->cp, "&伝搬:", strlen("&伝搬:"))) {
		pat = (*fpp2)->cp + strlen("&伝搬:");
		sscanf(pat, "%d", &i);
		pat = strchr(pat, ':');
		pat++;
		if ((cp = check_feature(((TAG_DATA *)ptr + offset)->f, pat))) {
		    assign_cfeature(&(((TAG_DATA *)ptr + offset + i)->f), cp, temp_assign_flag);
		}
		else { /* ないなら、もとからあるものを削除 */
		    delete_cfeature(&(((TAG_DATA *)ptr + offset + i)->f), pat);
		}
		if (((TAG_DATA *)ptr + offset)->bnum >= 0) { /* 文節区切りでもあるとき */
		    if ((cp = check_feature((((TAG_DATA *)ptr + offset)->b_ptr)->f, pat))) {
			assign_cfeature(&((((TAG_DATA *)ptr + offset)->b_ptr + i)->f), cp, temp_assign_flag);
		    }
		    else {
			delete_cfeature(&((((TAG_DATA *)ptr + offset)->b_ptr + i)->f), pat);
		    }
		}
	    }
	    /* 形態素付属化 : 属する形態素列をすべて<付属>にする
	       本来は、&形態素feature:^自立 のように引き数をとるべき */
	    else if (!strncmp((*fpp2)->cp, "&形態素付属化", strlen("&形態素付属化"))) {
		for (i = 0; i < ((TAG_DATA *)ptr + offset)->mrph_num; i++) {
		    delete_cfeature(&((((TAG_DATA *)ptr + offset)->mrph_ptr + i)->f), "自立");
		    delete_cfeature(&((((TAG_DATA *)ptr + offset)->mrph_ptr + i)->f), "内容語");
		    delete_cfeature(&((((TAG_DATA *)ptr + offset)->mrph_ptr + i)->f), "準内容語");
		    assign_cfeature(&((((TAG_DATA *)ptr + offset)->mrph_ptr + i)->f), "付属", temp_assign_flag);
		}
	    }
	    /* 自動辞書 : 自動獲得した辞書をチェック (マッチ部分全体) */
	    else if (!strncmp((*fpp2)->cp, "&自動辞書:", strlen("&自動辞書:"))) {
		if (offset == 0) {
		    if (!strncmp((*fpp2)->cp + strlen("&自動辞書:"), "先頭:", strlen("先頭:"))) {
			assign_pos = 0;
		    }
		    else if (!strncmp((*fpp2)->cp + strlen("&自動辞書:"), "末尾:", strlen("末尾:"))) {
			assign_pos = length - 1;
		    }
		    else {
			fprintf(stderr, ";; Invalid feature: %s\n", (*fpp2)->cp);
			exit(-1);
		    }
		    check_auto_dic((MRPH_DATA *)ptr, assign_pos, length, (*fpp2)->cp + strlen("&自動辞書:先頭:"), temp_assign_flag);
		}
	    }
	} else {			/* 追加の場合 */
	    assign_cfeature(fpp1, (*fpp2)->cp, temp_assign_flag);	
	}

	fpp2 = &((*fpp2)->next);
    }
}

/*==================================================================*/
	void copy_feature(FEATURE **dst_fpp, FEATURE *src_fp)
/*==================================================================*/
{
    while (src_fp) {
	assign_cfeature(dst_fpp, src_fp->cp, FALSE);
	src_fp = src_fp->next;
    }
}

/*
 *
 * ルール構造体 <==照合==> 形態素または文節構造体
 *
 */

/*==================================================================*/
	     int comp_feature(char *data, char *pattern)
/*==================================================================*/
{
    /* 
     *  完全一致 または 部分一致(patternが短く,次の文字が':')ならマッチ
     */
    if (data && !strcmp(data, pattern)) {
	return TRUE;
    } else if (data && !strncmp(data, pattern, strlen(pattern)) &&
	       data[strlen(pattern)] == ':') {
	return TRUE;
    } else {
	return FALSE;
    }
}

/*==================================================================*/
	     int comp_feature_NE(char *data, char *pattern)
/*==================================================================*/
{
    char decision[9];

    decision[0] = '\0';
    sscanf(data, "%*[^:]:%*[^:]:%s", decision);

    if (decision[0] && !strcmp(decision, pattern))
	return TRUE;
    else
	return FALSE;
}

/*==================================================================*/
	    char *check_feature(FEATURE *fp, char *fname)
/*==================================================================*/
{
    while (fp) {
	if (comp_feature(fp->cp, fname) == TRUE) {
	    return fp->cp;
	}
	fp = fp->next;
    }
    return NULL;
}

/*==================================================================*/
	    char *check_feature_NE(FEATURE *fp, char *fname)
/*==================================================================*/
{
    while (fp) {
	if (comp_feature_NE(fp->cp, fname) == TRUE) {
	    return fp->cp;
	}
	fp = fp->next;
    }
    return NULL;
}

/*==================================================================*/
    int check_category(FEATURE *fp, char *fname, int strict_flag)
/*==================================================================*/
{
    /* strict_flag == TRUE: カテゴリが複数ある(曖昧な)ときはFALSEを返す */

    char *cp;

    if (0 && strlen(fname) == 1) {
	/* fnameが'a'または'v'の場合 */
	/* <代表表記:...[av]>もカテゴリの一種として扱う */
	if (!check_feature(fp, "疑似代表表記") && (cp = check_feature(fp, "代表表記")) &&
	    *(cp + strlen(cp) - 1) == *fname) {
	    return TRUE;
	}
    }
    else if ((cp = check_feature(fp, "カテゴリ"))) {
        if (strict_flag && strchr(cp, ';')) { /* strict_flag時: カテゴリが複数あるときはFALSE */
            return FALSE;
        }
        cp += strlen("カテゴリ"); /* ":"の分はdoの中で足す */
        do {
            cp++; /* ":"もしくは";"の分 */
            if (!strcmp(cp, fname) ||
		!strncmp(cp, fname, strlen(fname)) && 
		*(cp + strlen(fname)) == ';')
		return TRUE;
        } while ((cp = strchr(cp, ';'))); /* 複数ある場合は";"で区切られている */
    }

    return FALSE;
}

/*==================================================================*/
	int compare_threshold(int value, int threshold, char *eq)
/*==================================================================*/
{
    if (str_eq(eq, "lt")) {
	if (value < threshold)
	    return TRUE;
	else
	    return FALSE;
    }
    else if (str_eq(eq, "le")) {
	if (value <= threshold)
	    return TRUE;
	else
	    return FALSE;
    }
    else if (str_eq(eq, "gt")) {
	if (value > threshold)
	    return TRUE;
	else
	    return FALSE;
    }
    else if (str_eq(eq, "ge")) {
	if (value >= threshold)
	    return TRUE;
	else
	    return FALSE;
    }
    return FALSE;
}

/*==================================================================*/
      int check_Bunrui(MRPH_DATA *mp, char *class, int flag)
/*==================================================================*/
{
    char string[14];

    if (str_eq(Class[6][mp->Bunrui].id, class))
	return flag;

    sprintf(string, "品曖-%s", class);
    if (check_feature(mp->f, string))
	return flag;

    return 1-flag;
}

/*==================================================================*/
		    int check_char_type(int code)
/*==================================================================*/
{
#ifdef IO_ENCODING_EUC
    /* カタカナと "ー" */
    if ((0xa5a0 < code && code < 0xa6a0) || code == 0xa1bc) {
	return TYPE_KATAKANA;
    }
    /* ひらがな */
    else if (0xa4a0 < code && code < 0xa5a0) {
	return TYPE_HIRAGANA;
    }
    /* 漢字 and "々" */
    else if (0xb0a0 < code || code == 0xa1b9) {
	return TYPE_KANJI;
    }
    /* 数字(０-９)のみ ("・"(0xa1a6)と"．"(0xa1a5)は対象外) */
    else if (0xa3af < code && code < 0xa3ba) {
	return TYPE_SUUJI;
    }
    /* アルファベット */
    else if (0xa3c0 < code && code < 0xa3fb) {
	return TYPE_EIGO;
    }
    /* 句読点 */
    else if (0xa1a0 < code && code < 0xa1a6) {
	return TYPE_PUNC;
    }
    /* 記号 */
    else {
	return TYPE_KIGOU;
    }
#else
#ifdef IO_ENCODING_SJIS
    /* カタカナと "ー" */
    if ((0x833f < code && code < 0x8397) || code == 0x815b) {
	return TYPE_KATAKANA;
    }
    /* ひらがな */
    else if (0x829e < code && code < 0x82f2) {
	return TYPE_HIRAGANA;
    }
    /* 漢字 and "々" */
    else if (0x889e < code || code == 0x8158) {
	return TYPE_KANJI;
    }
    /* 数字(０-９)のみ */
    else if (0x824e < code && code < 0x8259) {
	return TYPE_SUUJI;
    }
    /* アルファベット */
    else if (0x825f < code && code < 0x829b) {
	return TYPE_EIGO;
    }
    /* 句読点 */
    else if (0x813f < code && code < 0x8145) {
	return TYPE_PUNC;
    }
    /* 記号 */
    else {
	return TYPE_KIGOU;
    }
#else /* for Unicode */
    /* HIRAGANA */
    if (code > 0x303f && code < 0x30a0) {
	return TYPE_HIRAGANA;
    }
    /* KATAKANA and "ー"(0x30fc) */
    else if ((code > 0x309f && code < 0x30fb) || code == 0x30fc) {
	return TYPE_KATAKANA;
    }
    /* PUNCTUATIONS (　、。，．) */
    else if ((code > 0x2fff && code < 0x3003) || code == 0xff0c || code == 0xff0e) {
	return TYPE_PUNC;
    }
    /* FIGURE (only ０-９) */
    else if (code > 0xff0f && code < 0xff1a) {
	return TYPE_SUUJI;
    }
    /* ALPHABET (Ａ-Ｚ, ａ-ｚ) */
    else if ((code > 0xff20 && code < 0xff3b) || 
	     (code > 0xff40 && code < 0xff5b)) {
	return TYPE_EIGO;
    }
    /* CJK Unified Ideographs and "々" */
    else if ((code > 0x4dff && code < 0xa000) || code == 0x3005) {
	return TYPE_KANJI;
    }
    else {
	return TYPE_KIGOU;
    }
#endif
#endif
}

/*==================================================================*/
 int check_str_type(unsigned char *ucp, int allowed_type, int length)
/*==================================================================*/
{
    int code = 0, precode = 0;
    int i = 0, unicode;
    if (length == 0)
        length = strlen(ucp);

#if defined(IO_ENCODING_EUC) || defined(IO_ENCODING_SJIS)
    while (*ucp) {
	code = (*ucp << 8) + *(ucp + 1);
	code = check_char_type(code);
	if (allowed_type) {
	    if (!(code & allowed_type) ) {
		return FALSE;
	    }
	}
	else if (precode && precode != code) {
	    return FALSE; /* code is mixed */
	}
	precode = code;
	ucp += BYTES4CHAR;
        i += BYTES4CHAR;
        if (i >= length)
            break;
    }
#else
    while (i < length) {
	unsigned char c = *(ucp + i);
	if (c > 0xfb) { // 6 bytes
	    code = 0;
	    i += 6;
	}
	else if (c > 0xf7) { // 5 bytes
	    code = 0;
	    i += 5;
	}
	else if (c > 0xef) { // 4 bytes
	    code = 0;
	    i += 4;
	}
	else if (c > 0xdf) { // 3 bytes
	    unicode = (c & 0x0f) << 12;
	    c = *(ucp + i + 1);
	    unicode += (c & 0x3f) << 6;
	    c = *(ucp + i + 2);
	    unicode += c & 0x3f;
	    code = check_char_type(unicode);
	    i += 3;
	}
	else if (c > 0x7f) { // 2 bytes
	    unicode = (c & 0x1f) << 6;
	    c = *(ucp + i + 1);
	    unicode += c & 0x3f;
	    code = check_char_type(unicode);
	    i += 2;
	}
	else { // 1 byte
	    code = check_char_type(c);
	    i++;
	}

	if (allowed_type) {
	    if (!(code & allowed_type) ) {
		return FALSE;
	    }
	}
	else if (precode && precode != code) {
	    return FALSE; /* code is mixed */
	}
	precode = code;
    }
#endif
    return TRUE;
}

/*==================================================================*/
 int check_function(char *rule, FEATURE *fd, void *ptr1, void *ptr2)
/*==================================================================*/
{
    /* rule : ルール
       fd : データ側のFEATURE
       p1 : 係り受けの場合，係り側の構造体(MRPH_DATA,BNST_DATAなど)
       p2 : データの構造体(MRPH_DATA,BNST_DATAなど)
    */

    int i, code, type, pretype, flag, length;
    char *cp;
    unsigned char *ucp; 

    /* &記英数カ : 記英数カ チェック (句読点以外) (形態素レベル) */

    if (!strcmp(rule, "&記英数カ")) {
	return check_str_type(((MRPH_DATA *)ptr2)->Goi2, TYPE_KIGOU | TYPE_EIGO | TYPE_SUUJI | TYPE_KATAKANA, 0);
    }

    /* &漢字 : 漢字 チェック (形態素レベル) */

    else if (!strcmp(rule, "&漢字")) { /* euc-jp */
	/* 先頭の「か」「カ」「ヶ」は OK */
	if (!strncmp(((MRPH_DATA *)ptr2)->Goi2, "か", BYTES4CHAR) || 
	    !strncmp(((MRPH_DATA *)ptr2)->Goi2, "カ", BYTES4CHAR) || 
	    !strncmp(((MRPH_DATA *)ptr2)->Goi2, "ヶ", BYTES4CHAR)) {
	    if (strlen(((MRPH_DATA *)ptr2)->Goi2) == BYTES4CHAR)
		return TRUE;
	    else
		return check_str_type(((MRPH_DATA *)ptr2)->Goi2 + BYTES4CHAR, TYPE_KANJI, 0);
	}
	else {
	    return check_str_type(((MRPH_DATA *)ptr2)->Goi2, TYPE_KANJI, 0);
	}
    }

    /* &かな漢字 : かな漢字チェック (形態素レベル) */

    else if (!strcmp(rule, "&かな漢字")) {
	return check_str_type(((MRPH_DATA *)ptr2)->Goi2, TYPE_KANJI | TYPE_HIRAGANA, 0);
    }

    /* &ひらがな : ひらがな チェック (形態素レベル) */

    else if (!strcmp(rule, "&ひらがな")) {
	return check_str_type(((MRPH_DATA *)ptr2)->Goi2, TYPE_HIRAGANA, 0);
    }

    /* &末尾ひらがな : 末尾の一文字がひらがなか チェック (形態素レベル) */

    else if (!strcmp(rule, "&末尾ひらがな")) {
	ucp = ((MRPH_DATA *)ptr2)->Goi2;	/* 表記をチェック */
	ucp += strlen(ucp) - BYTES4CHAR;
	return check_str_type(ucp, TYPE_HIRAGANA, 0);
    }

    /* &末尾文字列 : 末尾の文字列を チェック (形態素レベル) */

    else if (!strncmp(rule, "&末尾文字列:", strlen("&末尾文字列:"))) {
	cp = rule + strlen("&末尾文字列:");

	/* パターンの方が大きければFALSE */
	if (strlen(cp) > strlen(((MRPH_DATA *)ptr2)->Goi2))
	    return FALSE;

	ucp = ((MRPH_DATA *)ptr2)->Goi2;	/* 表記をチェック */
	ucp += strlen(ucp)-strlen(cp);
	if (strcmp(ucp, cp))
	    return FALSE;
	return TRUE;
    }

    /* &カタカナ : カタカナ チェック (形態素レベル) */

    else if (!strcmp(rule, "&カタカナ")) {
	return check_str_type(((MRPH_DATA *)ptr2)->Goi2, TYPE_KATAKANA, 0);
    }

    /* &数字 : 数字 チェック (形態素レベル) */

    else if (!strcmp(rule, "&数字")) {
	return check_str_type(((MRPH_DATA *)ptr2)->Goi2, TYPE_SUUJI, 0);
    }

    /* &英記号 : 英記号 チェック (形態素レベル) */

    else if (!strcmp(rule, "&英記号")) {
	return check_str_type(((MRPH_DATA *)ptr2)->Goi2, TYPE_EIGO | TYPE_KIGOU | TYPE_PUNC, 0);
    }

    /* &記号 : 記号 チェック (形態素レベル) */

    else if (!strcmp(rule, "&記号")) {
	return check_str_type(((MRPH_DATA *)ptr2)->Goi2, TYPE_KIGOU | TYPE_PUNC, 0);
    }

    /* &混合 : 混合 (漢字+...) チェック (形態素レベル) */

    else if (!strcmp(rule, "&混合")) {
	if (check_str_type(((MRPH_DATA *)ptr2)->Goi2, 0, 0) == 0) /* mixed */
	    return TRUE;
	else
	    return FALSE;
    }

    /* &一文字 : 文字数 チェック (形態素レベル) */

    else if (!strcmp(rule, "&一文字")) {
	if (strlen(((MRPH_DATA *)ptr2)->Goi2) <= BYTES4CHAR)
	    return TRUE;
	else 
	    return FALSE;
    }

    /* &カテゴリ : カテゴリ チェック (形態素レベル) */
    
    else if (!strncmp(rule, "&カテゴリ:", strlen("&カテゴリ:"))) {
	cp = rule + strlen("&カテゴリ:");
        if (!strcmp(cp, "時間")) { /* 「時間」のときは、strict_flag == TRUE (曖昧なときはFALSEを返す) */
            return check_category(((MRPH_DATA *)ptr2)->f, cp, TRUE);
        }
        else {
            return check_category(((MRPH_DATA *)ptr2)->f, cp, FALSE);
        }
    }

    /* &意味素: 意味素チェック (形態素) */

    else if (!strncmp(rule, "&意味素:", strlen("&意味素:"))) {
	if (Thesaurus != USE_NTT || ((MRPH_DATA *)ptr2)->SM == NULL) {
	    return FALSE;
	}

	cp = rule + strlen("&意味素:");
	/* 漢字だったら意味属性名, それ以外ならコードそのまま */
	if (*cp & 0x80) {
	    if (SM2CODEExist == TRUE)
		cp = sm2code(cp);
	    else
		cp = NULL;
	    flag = SM_NO_EXPAND_NE;
	}
	else {
	    flag = SM_CHECK_FULL;
	}

	if (cp) {
	    for (i = 0; ((MRPH_DATA *)ptr2)->SM[i]; i+=SM_CODE_SIZE) {
		if (_sm_match_score(cp, 
				    &(((MRPH_DATA *)ptr2)->SM[i]), flag))
		    return TRUE;
	    }
	}
	return FALSE;
    }

    /* &文節意味素: 意味素チェック (文節) */

    else if (!strncmp(rule, "&文節意味素:", strlen("&文節意味素:"))) {
	if (Thesaurus != USE_NTT && Thesaurus != USE_BGH) {
	    return FALSE;
	}

	cp = rule + strlen("&文節意味素:");
	/* 漢字だったら意味属性名, それ以外ならコードそのまま */
	if (*cp & 0x80) {
	    if (SM2CODEExist == TRUE)
		cp = sm2code(cp);
	    else
		cp = NULL;
	    flag = SM_NO_EXPAND_NE;
	}
	else {
	    flag = SM_CHECK_FULL;
	}

	if (cp) {
	    if (Thesaurus == USE_NTT) {	
		if (sm_match_check(cp, (((BNST_DATA *)ptr2)->SM_code), flag))
		    return TRUE;
	    }
	    else if (Thesaurus == USE_BGH) {
		if (bgh_match_check(cp, ((BNST_DATA *)ptr2)->BGH_code))
		    return TRUE;
	    }
	}
	return FALSE;
    }

    /* &文節全意味素: 文節のすべての意味素が指定意味素以下にあるかどうか */

    else if (!strncmp(rule, "&文節全意味素:", strlen("&文節全意味素:"))) {
	if (Thesaurus != USE_NTT && Thesaurus != USE_BGH) {
	    return FALSE;
	}

	cp = rule + strlen("&文節全意味素:");
	/* 漢字だったら意味属性名, それ以外ならコードそのまま */
	if (*cp & 0x80) {
	    if (SM2CODEExist == TRUE)
		cp = sm2code(cp);
	    else
		cp = NULL;
	}

	if (Thesaurus == USE_NTT) {	
	    if (cp && ((BNST_DATA *)ptr2)->SM_code[0] && 
		sm_all_match(((BNST_DATA *)ptr2)->SM_code, cp)) {
		return TRUE;
	    }
	}
	else if (Thesaurus == USE_BGH) {
	    if (cp && ((BNST_DATA *)ptr2)->BGH_code[0] && 
		sm_all_match(((BNST_DATA *)ptr2)->BGH_code, cp)) {
		return TRUE;
	    }
	}
	return FALSE;
    }

    /* 形態素の長さ */
    
    else if (!strncmp(rule, "&形態素長:", strlen("&形態素長:"))) {
	cp = rule + strlen("&形態素長:");
	if (*(cp + strlen(cp) - 1) == '-') { /* 数字の後に"-"がついていれば */
	    flag = 1; /* 指定長さ以上でOK */
	    *(cp + strlen(cp) - 1) = '\0';
	}
	else {
	    flag = 0;
	}
	code = atoi(cp);

	length = strlen(((MRPH_DATA *)ptr2)->Goi2);
	if (length == code * BYTES4CHAR || (flag && length > code * BYTES4CHAR)) {
	    return TRUE;
	}
	return FALSE;
    }

    /* &数字長 : 先頭の数字の長さ チェック (形態素レベル) */

    else if (!strncmp(rule, "&数字長:", strlen("&数字長:"))) {
        cp = rule + strlen("&数字長:");
        code = atoi(cp) * BYTES4CHAR;
	length = strlen(((MRPH_DATA *)ptr2)->Goi2);
        if (length < code) /* 形態素が指定長より短いとき */
            return FALSE;
        else
            return check_str_type(((MRPH_DATA *)ptr2)->Goi2, TYPE_SUUJI, code);
    }

    else if (!strncmp(rule, "&形態素末尾:", strlen("&形態素末尾:"))) {
	cp = rule + strlen("&形態素末尾:");
	i = strlen(((MRPH_DATA *)ptr2)->Goi2) - strlen(cp);
	if (*cp && i >= 0 && !strcmp((((MRPH_DATA *)ptr2)->Goi2)+i, cp)) {
	    return TRUE;
	}
	return FALSE;
    }

    /* &表層: 表層格チェック (文節レベル,係受レベル) */

    else if (!strncmp(rule, "&表層:", strlen("&表層:"))) {
	if (!strcmp(rule + strlen("&表層:"), "照合")) {
	    if ((cp = check_feature(((BNST_DATA *)ptr1)->f, "係")) == NULL) {
		return FALSE;
	    }
	    if (((BNST_DATA *)ptr2)->
		SCASE_code[case2num(cp + strlen("係:"))]) {
		return TRUE;
	    }
	    else {
		return FALSE;
	    }
	}
	else if (((BNST_DATA *)ptr2)->
		 SCASE_code[case2num(rule + strlen("&表層:"))]) {
	    return TRUE;
	}
	else {
	    return FALSE;
 	}
    }

    /* &D : 距離比較 (係受レベル) */

    else if (!strncmp(rule, "&D:", strlen("&D:"))) {
	if (((BNST_DATA *)ptr2 - (BNST_DATA *)ptr1)
	    <= atoi(rule + strlen("&D:"))) {
	    return TRUE;
	}
	else {
	    return FALSE;
	}
    }

    /* &レベル:強 : 用言のレベル比較 (係受レベル) */

    else if (!strcmp(rule, "&レベル:強")) {
	return subordinate_level_comp((BNST_DATA *)ptr1, 
				      (BNST_DATA *)ptr2);
    }

    /* &レベル:X : 用言がレベルX以上であるかどうか */

    else if (!strncmp(rule, "&レベル:", strlen("&レベル:"))) {
	return subordinate_level_check(rule + strlen("&レベル:"), fd);
	/* (BNST_DATA *)ptr2); */
    }

    /* &係側 : 係側のFEATUREチェック (係受レベル) */
    
    else if (!strncmp(rule, "&係側:", strlen("&係側:"))) {
	cp = rule + strlen("&係側:");
	if ((*cp != '^' && check_feature(((BNST_DATA *)ptr1)->f, cp)) ||
	    (*cp == '^' && !check_feature(((BNST_DATA *)ptr1)->f, cp + 1))) {
	    return TRUE;
	} else {
	    return FALSE;
	}
    }

    /* &係側チェック : 係側のFEATUREチェック (文節ルール) */
    
    else if (!strncmp(rule, "&係側チェック:", strlen("&係側チェック:"))) {
	cp = rule + strlen("&係側チェック:");
	for (i = 0; ((BNST_DATA *)ptr2)->child[i]; i++) {
	    if (check_feature(((BNST_DATA *)ptr2)->child[i]->f, cp)) {
		return TRUE;
	    }
	}
	return FALSE;
    }

    /* &受側チェック : 受側のFEATUREチェック (文節ルール) */
    
    else if (!strncmp(rule, "&受側チェック:", strlen("&受側チェック:"))) {
	cp = rule + strlen("&受側チェック:");
	if (((BNST_DATA *)ptr2)->parent &&
	    check_feature(((BNST_DATA *)ptr2)->parent->f, cp)) {
	    return TRUE;
	} else {
	    return FALSE;
	}
    }

    /* &自立語一致 : 自立語が同じかどうか */
    
    else if (!strncmp(rule, "&自立語一致", strlen("&自立語一致"))) {
	/* if (!strcmp(((BNST_DATA *)ptr1)->head_ptr->Goi, 
	   ((BNST_DATA *)ptr2)->head_ptr->Goi)) { */
	if (!strcmp(((BNST_DATA *)ptr1)->Jiritu_Go, 
		    ((BNST_DATA *)ptr2)->Jiritu_Go)) {
	    return TRUE;
	} else {
	    return FALSE;
	}
    }


    /* &文字列照合 : 原形との文字列部分マッチ by kuro 00/12/28 */
    
    else if (!strncmp(rule, "&文字列照合:", strlen("&文字列照合:"))) {
      	cp = rule + strlen("&文字列照合:");
	if (strstr(((MRPH_DATA *)ptr2)->Goi, cp)) {
	    return TRUE;
	} else {
	    return FALSE;
	}
    }

    /* &ST : 並列構造解析での類似度の閾値 (ここでは無視) */
    
    else if (!strncmp(rule, "&ST", strlen("&ST"))) {
	return TRUE;
    }

    /* &OPTCHECK : オプションのチェック */
    
    else if (!strncmp(rule, "&OptCheck:", strlen("&OptCheck:"))) {
	char **opt;

	cp = rule + strlen("&OptCheck:");
	if (*cp == '-') { /* '-'を含んでいたら飛ばす */
	    cp++;
	}

	for (opt = Options; *opt != NULL; opt++) {
	    if (!strcasecmp(cp, *opt)) {
		return TRUE;	    
	    }
	}
	return FALSE;
    }

    /*
    else if (!strncmp(rule, "&時間", strlen("&時間"))) {
	if (sm_all_match(((BNST_DATA *)ptr2)->SM_code, "1128********")) {
	    return TRUE;
	}
	else {
	    return FALSE;
	}
    } */

    /* &態 : 態をチェック */

    else if (!strncmp(rule, "&態:", strlen("&態:"))) {
	cp = rule + strlen("&態:");
	if ((!strcmp(cp, "能動") && ((BNST_DATA *)ptr2)->voice == 0) || 
	    (!strcmp(cp, "受動") && (((BNST_DATA *)ptr2)->voice & VOICE_UKEMI || 
				     ((BNST_DATA *)ptr2)->voice & VOICE_SHIEKI_UKEMI)) || 
	    (!strcmp(cp, "使役") && (((BNST_DATA *)ptr2)->voice & VOICE_SHIEKI || 
				     ((BNST_DATA *)ptr2)->voice & VOICE_SHIEKI_UKEMI))) {
	    return TRUE;
	}
	else {
	    return FALSE;
	}
    }

    /* &記憶 : 形態素または文節のポインタを記憶 */

    else if (!strcmp(rule, "&記憶")) {
	matched_ptr = ptr2;
	return TRUE;
    }

    /* &名動相互情報量 : 名詞動詞間の相互情報量のチェック (基本句ルール) */
    
    else if (!strncmp(rule, "&名動相互情報量:", strlen("&名動相互情報量:"))) {
	cp = rule + strlen("&名動相互情報量:");
	code = atoi(cp);
	return check_nv_mi_parent_and_children((TAG_DATA *)ptr2, code);
    }

    else {
#ifdef DEBUG
	fprintf(stderr, "Invalid Feature-Function (%s)\n", rule);
#endif
	return TRUE;
    }
}

/*==================================================================*/
 int feature_AND_match(FEATURE *fp, FEATURE *fd, void *p1, void *p2)
/*==================================================================*/
{
    int value;

    while (fp) {
	if (fp->cp[0] == '^' && fp->cp[1] == '&') {
	    value = check_function(fp->cp+1, fd, p1, p2);
	    if (value == TRUE) {
		return FALSE;
	    }
	} else if (fp->cp[0] == '&') {
	    value = check_function(fp->cp, fd, p1, p2);
	    if (value == FALSE) {
		return FALSE;
	    }
	} else if (fp->cp[0] == '^') {
	    if (check_feature(fd, fp->cp+1)) {
		return FALSE;
	    }
	} else {
	    if (!check_feature(fd, fp->cp)) {
		return FALSE;
	    }
	}
	fp = fp->next;
    }
    return TRUE;
}

/*==================================================================*/
int feature_pattern_match(FEATURE_PATTERN *fr, FEATURE *fd,
			  void *p1, void *p2)
/*==================================================================*/
{
    /* fr : ルール側のFEATURE_PATTERN,
       fd : データ側のFEATURE
       p1 : 係り受けの場合，係り側の構造体(MRPH_DATA,BNST_DATAなど)
       p2 : データ側の構造体(MRPH_DATA,BNST_DATAなど)
    */

    int i, value;

    /* PATTERNがなければマッチ */
    if (fr->fp[0] == NULL) return TRUE;

    /* ORの各条件を調べる */
    for (i = 0; fr->fp[i]; i++) {
	value = feature_AND_match(fr->fp[i], fd, p1, p2);
	if (value == TRUE) 
	    return TRUE;
    }
    return FALSE;
}

/*====================================================================*/
                 char* get_feature_for_chi (BNST_DATA *p_ptr)
/*====================================================================*/
{
    char* feature;
    if (check_feature(p_ptr->f, "AD")) {
	feature = "AD";
    }
    else if (check_feature(p_ptr->f, "AS")) {
	feature = "AS";
    }
    else if (check_feature(p_ptr->f, "BA")) {
	feature = "BA";
    }
    else if (check_feature(p_ptr->f, "CC")) {
	feature = "CC";
    }
    else if (check_feature(p_ptr->f, "CD")) {
	feature = "CD";
    }
    else if (check_feature(p_ptr->f, "CS")) {
	feature = "CS";
    }
    else if (check_feature(p_ptr->f, "DEC")) {
	feature = "DEC";
    }
    else if (check_feature(p_ptr->f, "DEG")) {
	feature = "DEG";
    }
    else if (check_feature(p_ptr->f, "DER")) {
	feature = "DER";
    }
    else if (check_feature(p_ptr->f, "DEV")) {
	feature = "DEV";
    }
    else if (check_feature(p_ptr->f, "DT")) {
	feature = "DT";
    }
    else if (check_feature(p_ptr->f, "ETC")) {
	feature = "ETC";
    }
    else if (check_feature(p_ptr->f, "FW")) {
	feature = "FW";
    }
    else if (check_feature(p_ptr->f, "IJ")) {
	feature = "IJ";
    }
    else if (check_feature(p_ptr->f, "JJ")) {
	feature = "JJ";
    }
    else if (check_feature(p_ptr->f, "LB")) {
	feature = "LB";
    }
    else if (check_feature(p_ptr->f, "LC")) {
	feature = "LC";
    }
    else if (check_feature(p_ptr->f, "M")) {
	feature = "M";
    }
    else if (check_feature(p_ptr->f, "MSP")) {
	feature = "MSP";
    }
    else if (check_feature(p_ptr->f, "NN")) {
	feature = "NN";
    }
    else if (check_feature(p_ptr->f, "NR")) {
	feature = "NR";
    }
    else if (check_feature(p_ptr->f, "NT")) {
	feature = "NT";
    }
    else if (check_feature(p_ptr->f, "OD")) {
	feature = "OD";
    }
    else if (check_feature(p_ptr->f, "ON")) {
	feature = "ON";
    }
    else if (check_feature(p_ptr->f, "P")) {
	feature = "P";
    }
    else if (check_feature(p_ptr->f, "PN")) {
	feature = "PN";
    }
    else if (check_feature(p_ptr->f, "PU")) {
	feature = "PU";
    }
    else if (check_feature(p_ptr->f, "SB")) {
	feature = "SB";
    }
    else if (check_feature(p_ptr->f, "SP")) {
	feature = "SP";
    }
    else if (check_feature(p_ptr->f, "VV")) {
	feature = "VV";
    }
    else if (check_feature(p_ptr->f, "VA")) {
	feature = "VA";
    }
    else if (check_feature(p_ptr->f, "VC")) {
	feature = "VC";
    }
    else if (check_feature(p_ptr->f, "VE")) {
	feature = "VE";
    }
    else {
	feature = "";
    }
    
    return feature;
}

/*====================================================================
                               END
====================================================================*/
