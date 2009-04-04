/*====================================================================

			   ����ե������Ϣ

                                               S.Kurohashi 1999.11.13

    $Id$
====================================================================*/
#include "knp.h"

extern char Jumangram_Dirname[];
extern int LineNoForError, LineNo;
char *Knprule_Dirname = NULL;
char *Knpdict_Dirname = NULL;
char *KnpNE_Dirname = NULL;

RuleVector *RULE = NULL;
int CurrentRuleNum = 0;
int RuleNumMax = 0;

char *DICT[DICT_MAX];
int knp_dict_file_already_defined = 0;
THESAURUS_FILE THESAURUS[THESAURUS_MAX];


/*==================================================================*/
	    void check_duplicated(int value, char *string)
/*==================================================================*/
{
    /* �ͤ� 0 �Ǥʤ��Ȥ��ϥ��顼 */
    if (value) {
	fprintf(stderr, "%s is duplicately specified in .knprc\n", string);
	exit(0);	
    }
}

/*==================================================================*/
		   void clear_rule_configuration()
/*==================================================================*/
{
    if (CurrentRuleNum) {
	free(RULE);
	RULE = NULL;
	CurrentRuleNum = 0;
	RuleNumMax = 0;
    }
    if (Knprule_Dirname) {
	free(Knprule_Dirname);
	Knprule_Dirname = NULL;
    }
    if (Knpdict_Dirname) {
	free(Knpdict_Dirname);
	Knpdict_Dirname = NULL;
    }
}

/*==================================================================*/
		    char *check_tilde(char *file)
/*==================================================================*/
{
    char *home, *ret;

    if (*file == '~' && (home = getenv("HOME"))) {
	ret = (char *)malloc_data(strlen(home)+strlen(file), "check_tilde");
	sprintf(ret, "%s%s", home, strchr(file, '/'));
    }
    else {
	ret = strdup(file);
    }
    return ret;
}

/*==================================================================*/
		       int *str2ints(char *str)
/*==================================================================*/
{
    char *cp, *start, *token;
    int *ret, ret_size = 1, count = 0;

    if (str[0] == '\"') {
	start = str + 1;
	if (cp = strchr(start, '\"')) {
	    *cp = '\0';
	}
    }
    else {
	start = str;
    }

    ret = (int *)malloc_data(sizeof(int) * ret_size, "str2ints");

    token = strtok(start, ",");
    while (token) {
	if (count >= ret_size - 1) {
	    ret = (int *)realloc_data(ret, sizeof(int) * (ret_size <<= 1), "str2ints");
	}
	*(ret + count++) = atoi(token);
	token = strtok(NULL, ",");
    }
    *(ret + count) = 0;

    return ret;
}

/*==================================================================*/
		   FILE *find_rc_file(char *opfile)
/*==================================================================*/
{
    FILE *fp;

    if (opfile) {
	if ((fp = fopen(opfile, "r")) == NULL) {
	    fprintf(stderr, "not found rc file <%s>.\n", opfile);
	    exit(1);
	}
    }
    else {
	char *user_home, *filename;

	if((user_home = getenv("HOME")) == NULL) {
	    filename = NULL;
	}
	else {
	    filename = (char *)malloc_data(strlen(user_home)+strlen("/.knprc")+1, "find_rc_file");
	    sprintf(filename, "%s/.knprc" , user_home);
	}

	if (filename == NULL || (fp = fopen(filename, "r")) == NULL) {
#ifdef KNP_RC_DEFAULT
	    if ((fp = fopen(KNP_RC_DEFAULT, "r")) == NULL) {
		fprintf(stderr, "not found <.knprc> and KNP_RC_DEFAULT(<%s>).\n", KNP_RC_DEFAULT);
		exit(1);
	    }
#else
	    fprintf(stderr, "not found <.knprc> in your home directory.\n");
	    exit(1);
#endif
	}
	if (filename) {
	    free(filename);
	}
    }
    return fp;
}

/*==================================================================*/
			void read_rc(FILE *in)
/*==================================================================*/
{
#ifdef  _WIN32
    char buf[FILENAME_MAX];
    /* MS Windows �ξ��ϡ�juman.ini, knp.ini �򸫤˹Ԥ�
       dicfile == gramfile */
    GetPrivateProfileString("juman", "dicfile", "", Jumangram_Dirname, FILENAME_MAX, "juman.ini");
    GetPrivateProfileString("knp", "ruledir", "", buf, FILENAME_MAX, "knp.ini");
    if (buf[0]) {
	Knprule_Dirname = strdup(buf);
    }
    GetPrivateProfileString("knp", "dictdir", "", buf, FILENAME_MAX, "knp.ini");
    if (buf[0]) {
	Knpdict_Dirname = strdup(buf);
    }
#else
    CELL *cell1,*cell2;
    char *dicttype;

    LineNo = 0 ;
    Jumangram_Dirname[0] = '\0';

    while (!s_feof(in))  {
	LineNoForError = LineNo;
	cell1 = s_read(in);

	if (!strcmp(DEF_JUMAN_GRAM_FILE, _Atom(car(cell1)))) {
	    if (!Atomp(cell2 = car(cdr(cell1)))) {
		fprintf(stderr, "error in .knprc\n");
		exit(0);
	    } else 
		strcpy(Jumangram_Dirname, _Atom(cell2));
	}
	/* KNP �롼��ǥ��쥯�ȥ� */
	else if (!strcmp(DEF_KNP_DIR, _Atom(car(cell1)))) {
	    if (!Atomp(cell2 = car(cdr(cell1)))) {
		fprintf(stderr, "error in .knprc\n");
		exit(0);
	    }
	    else
		Knprule_Dirname = check_tilde(_Atom(cell2));
	}
	/* KNP �롼��ե����� */
	else if (!strcmp(DEF_KNP_FILE, _Atom(car(cell1)))) {
	    cell1 = cdr(cell1);

	    while (!Null(car(cell1))) {
		if (CurrentRuleNum >= RuleNumMax) {
		    RuleNumMax += RuleIncrementStep;
		    RULE = (RuleVector *)realloc_data(RULE, sizeof(RuleVector)*RuleNumMax, "read_rc");
		}

		/* �ǥե���������� */
		(RULE+CurrentRuleNum)->file = (char *)strdup(_Atom(car(car(cell1))));
		(RULE+CurrentRuleNum)->mode = RLOOP_MRM;
		(RULE+CurrentRuleNum)->breakmode = RLOOP_BREAK_NONE;
		(RULE+CurrentRuleNum)->type = 0;
		(RULE+CurrentRuleNum)->direction = 0;

		cell2 = cdr(car(cell1));

		while (!Null(car(cell2))) {
		    if (!strcmp(_Atom(car(cell2)), "Ʊ���۵���")) {
			check_duplicated((RULE+CurrentRuleNum)->type, "Rule type");
			(RULE+CurrentRuleNum)->type = HomoRuleType;
		    }
		    else if (!strcmp(_Atom(car(cell2)), "������")) {
			check_duplicated((RULE+CurrentRuleNum)->type, "Rule type");
			(RULE+CurrentRuleNum)->type = MorphRuleType;
		    }
		    else if (!strcmp(_Atom(car(cell2)), "������-������")) {
			check_duplicated((RULE+CurrentRuleNum)->type, "Rule type");
			(RULE+CurrentRuleNum)->type = PreProcessMorphRuleType;
		    }
		    else if (!strcmp(_Atom(car(cell2)), "���ܶ�")) {
			check_duplicated((RULE+CurrentRuleNum)->type, "Rule type");
			(RULE+CurrentRuleNum)->type = TagRuleType;
		    }
		    else if (!strcmp(_Atom(car(cell2)), "���ܶ�-��¤�����")) {
			check_duplicated((RULE+CurrentRuleNum)->type, "Rule type");
			(RULE+CurrentRuleNum)->type = AfterDpndTagRuleType;
		    }
		    else if (!strcmp(_Atom(car(cell2)), "���ܶ�-�����")) {
			check_duplicated((RULE+CurrentRuleNum)->type, "Rule type");
			(RULE+CurrentRuleNum)->type = PostProcessTagRuleType;
		    }
		    else if (!strcmp(_Atom(car(cell2)), "ʸ��")) {
			check_duplicated((RULE+CurrentRuleNum)->type, "Rule type");
			(RULE+CurrentRuleNum)->type = BnstRuleType;
		    }
		    else if (!strcmp(_Atom(car(cell2)), "ʸ��-��¤�����")) {
			check_duplicated((RULE+CurrentRuleNum)->type, "Rule type");
			(RULE+CurrentRuleNum)->type = AfterDpndBnstRuleType;
		    }
		    else if (!strcmp(_Atom(car(cell2)), "�������")) {
			check_duplicated((RULE+CurrentRuleNum)->type, "Rule type");
			(RULE+CurrentRuleNum)->type = DpndRuleType;
		    }
		    else if (!strcmp(_Atom(car(cell2)), "�Ʊ�")) {
			check_duplicated((RULE+CurrentRuleNum)->type, "Rule type");
			(RULE+CurrentRuleNum)->type = KoouRuleType;
		    }
		    else if (!strcmp(_Atom(car(cell2)), "��ͭɽ��������")) {
			check_duplicated((RULE+CurrentRuleNum)->type, "Rule type");
			(RULE+CurrentRuleNum)->type = NeMorphRuleType;
		    }
		    else if (!strcmp(_Atom(car(cell2)), "��ͭɽ����-PRE")) {
			check_duplicated((RULE+CurrentRuleNum)->type, "Rule type");
			(RULE+CurrentRuleNum)->type = NePhrasePreRuleType;
		    }
		    else if (!strcmp(_Atom(car(cell2)), "��ͭɽ����")) {
			check_duplicated((RULE+CurrentRuleNum)->type, "Rule type");
			(RULE+CurrentRuleNum)->type = NePhraseRuleType;
		    }
		    else if (!strcmp(_Atom(car(cell2)), "��ͭɽ����-AUX")) {
			check_duplicated((RULE+CurrentRuleNum)->type, "Rule type");
			(RULE+CurrentRuleNum)->type = NePhraseAuxRuleType;
		    }
		    else if (!strcmp(_Atom(car(cell2)), "ʸ̮")) {
			check_duplicated((RULE+CurrentRuleNum)->type, "Rule type");
			(RULE+CurrentRuleNum)->type = ContextRuleType;
		    }
		    else if (!strcmp(_Atom(car(cell2)), "�롼��롼�����")) {
			(RULE+CurrentRuleNum)->mode = RLOOP_RMM;
		    }
		    else if (!strcmp(_Atom(car(cell2)), "BREAK")) {
			/* RLOOP_BREAK_NONE �� 0 �ʤΤǤҤä�����ʤ� */
			check_duplicated((RULE+CurrentRuleNum)->breakmode, "Break mode");
			(RULE+CurrentRuleNum)->breakmode = RLOOP_BREAK_NORMAL;
		    }
		    else if (!strcmp(_Atom(car(cell2)), "BREAKJUMP")) {
			/* RLOOP_BREAK_NONE �� 0 �ʤΤǤҤä�����ʤ� */
			check_duplicated((RULE+CurrentRuleNum)->breakmode, "Break mode");
			(RULE+CurrentRuleNum)->breakmode = RLOOP_BREAK_JUMP;
		    }
		    else if (!strcmp(_Atom(car(cell2)), "������")) {
			check_duplicated((RULE+CurrentRuleNum)->direction, "Direction");
			(RULE+CurrentRuleNum)->direction = LtoR;
		    }
		    else if (!strcmp(_Atom(car(cell2)), "������")) {
			check_duplicated((RULE+CurrentRuleNum)->direction, "Direction");
			(RULE+CurrentRuleNum)->direction = RtoL;
		    }
		    else {
			fprintf(stderr, "%s is invalid in .knprc\n", _Atom(car(cell2)));
			exit(0);
		    }
		    cell2 = cdr(cell2);
		}

		/* �롼��Υ����פ����ꤵ��Ƥ��ʤ��Ȥ� */
		if (!(RULE+CurrentRuleNum)->type) {
		    fprintf(stderr, "Rule type for \'%s\' is not specified in .knprc\n", 
			    (RULE+CurrentRuleNum)->file);
		    exit(0);
		}

		/* �ǥե���Ȥ����� */
		if (!(RULE+CurrentRuleNum)->direction)
		    (RULE+CurrentRuleNum)->direction = LtoR;

		CurrentRuleNum++;
		cell1 = cdr(cell1);
	    }
	}
	/* KNP ����ǥ��쥯�ȥ� */
	else if (!strcmp(DEF_KNP_DICT_DIR, _Atom(car(cell1)))) {
	    if (!Atomp(cell2 = car(cdr(cell1)))) {
		fprintf(stderr, "error in .knprc\n");
		exit(0);
	    }
	    else
		Knpdict_Dirname = check_tilde(_Atom(cell2));
	}
	/* KNP ����ե����� */
	else if (!strcmp(DEF_KNP_DICT_FILE, _Atom(car(cell1))) && 
		 !knp_dict_file_already_defined) {
	    cell1 = cdr(cell1);
	    knp_dict_file_already_defined = 1;

	    while (!Null(car(cell1))) {
		dicttype = _Atom(car(cdr(car(cell1))));
		if (!strcmp(dicttype, "�ʥե졼��INDEXDB")) {
		    DICT[CF_INDEX_DB] = strdup(_Atom(car(car(cell1))));
		}
		else if (!strcmp(dicttype, "�ʥե졼��DATA")) {
		    DICT[CF_DATA] = strdup(_Atom(car(car(cell1))));
		}
		else if (!strcmp(dicttype, "�ʥե졼��SIMDB")) {
		    DICT[CF_SIM_DB] = strdup(_Atom(car(car(cell1))));
		}
		else if (!strcmp(dicttype, "�ʥե졼��CFPDB")) {
		    DICT[CFP_DB] = strdup(_Atom(car(car(cell1))));
		}
		else if (!strcmp(dicttype, "�ʥե졼��CFCASEDB")) {
		    DICT[CF_CASE_DB] = strdup(_Atom(car(car(cell1))));
		}
		else if (!strcmp(dicttype, "�ʥե졼��CASEDB")) {
		    DICT[CASE_DB] = strdup(_Atom(car(car(cell1))));
		}
		else if (!strcmp(dicttype, "�ʥե졼��RENYOUDB")) {
		    DICT[RENYOU_DB] = strdup(_Atom(car(car(cell1))));
		}
		else if (!strcmp(dicttype, "�ʥե졼��ADVERBDB")) {
		    DICT[ADVERB_DB] = strdup(_Atom(car(car(cell1))));
		}
		else if (!strcmp(dicttype, "̾��ʥե졼��INDEXDB")) {
		    DICT[CF_NOUN_INDEX_DB] = strdup(_Atom(car(car(cell1))));
		}
		else if (!strcmp(dicttype, "̾��ʥե졼��DATA")) {
		    DICT[CF_NOUN_DATA] = strdup(_Atom(car(car(cell1))));
		}
		else if (!strcmp(dicttype, "ʬ�����ɽDB")) {
		    DICT[BGH_DB] = strdup(_Atom(car(car(cell1))));
		}
		else if (!strcmp(dicttype, "ɽ�س�DB")) {
		    DICT[SCASE_DB] = strdup(_Atom(car(car(cell1))));
		}
		else if (!strcmp(dicttype, "NTTñ��DB")) {
		    DICT[SM_DB] = strdup(_Atom(car(car(cell1))));
		}
		else if (!strcmp(dicttype, "NTT��̣��DB")) {
		    DICT[SM2CODE_DB] = strdup(_Atom(car(car(cell1))));
		}
		else if (!strcmp(dicttype, "NTT��ͭ̾���Ѵ��ơ��֥�DB")) {
		    DICT[SMP2SMG_DB] = strdup(_Atom(car(car(cell1))));
		}
		else {
		    fprintf(stderr, "%s is invalid in .knprc\n", _Atom(car(cdr(car(cell1)))));
		    exit(0);
		}
		cell1 = cdr(cell1);
	    }
	}
	/* �����ʥ������饹 */
	else if (!strcmp(DEF_THESAURUS, _Atom(car(cell1)))) {
	    int i;
	    cell1 = cdr(cell1);
	    while (!Null(car(cell1))) {
		THESAURUS[0].path = strdup(_Atom(car(car(cell1))));
		THESAURUS[0].name = strdup(_Atom(car(cdr(car(cell1)))));
		THESAURUS[0].format = str2ints(_Atom(car(cdr(cdr(car(cell1))))));
		for (i = 0; THESAURUS[0].format[i]; i++) {
		    THESAURUS[0].code_size += THESAURUS[0].format[i];
		}
		cell1 = cdr(cell1);
	    }
	}
	/* �ʲ����ѥ������饹 */
	else if (!strcmp(DEF_CASE_THESAURUS, _Atom(car(cell1)))) {
	    if (!Atomp(cell2 = car(cdr(cell1)))) {
		fprintf(stderr, "error in .knprc\n");
		exit(0);
	    }
	    else {
		int i;

		Thesaurus = USE_NONE;
		if (strcasecmp(_Atom(cell2), "NONE")) { /* NONE�ǤϤʤ��Ȥ� */
		    for (i = 0; THESAURUS[i].name && i < THESAURUS_MAX; i++) {
			if (!strcasecmp(_Atom(cell2), THESAURUS[i].name)) {
			    Thesaurus = i;
			    if (OptDisplay == OPT_DEBUG) {
				fprintf(Outfp, "Thesaurus for case analysis ... %s\n", THESAURUS[i].name);
			    }
			    break;
			}
		    }
		    if (Thesaurus == USE_NONE) {
			fprintf(stderr, "%s is invalid in .knprc\n", _Atom(cell2));
			exit(0);
		    }
		}
	    }
	}
	/* ��������ѥ������饹 */
	else if (!strcmp(DEF_PARA_THESAURUS, _Atom(car(cell1)))) {
	    if (!Atomp(cell2 = car(cdr(cell1)))) {
		fprintf(stderr, "error in .knprc\n");
		exit(0);
	    }
	    else {
		int i;

		ParaThesaurus = USE_NONE;
		if (strcasecmp(_Atom(cell2), "NONE")) { /* NONE�ǤϤʤ��Ȥ� */
		    for (i = 0; THESAURUS[i].name && i < THESAURUS_MAX; i++) {
			if (!strcasecmp(_Atom(cell2), THESAURUS[i].name)) {
			    ParaThesaurus = i;
			    if (OptDisplay == OPT_DEBUG) {
				fprintf(Outfp, "Thesaurus for para analysis ... %s\n", THESAURUS[i].name);
			    }
			    break;
			}
		    }
		    if (ParaThesaurus == USE_NONE) {
			fprintf(stderr, "%s is invalid in .knprc\n", _Atom(cell2));
			exit(0);
		    }
		}
	    }
	}
	/* ��ά���ϳ� */
	else if (!strcmp(DEF_DISC_CASES, _Atom(car(cell1)))) {
	    int n = 0, cn;

	    if (Null(cdr(cell1))) {
		fprintf(stderr, "error in .knprc: %s\n", _Atom(car(cell1)));
		exit(0);
	    }

	    cell1 = cdr(cell1);
	    while (!Null(car(cell1))) {
		cn = pp_kstr_to_code(_Atom(car(cell1)));
		if (cn == END_M) {
		    fprintf(stderr, "%s is invalid in .knprc\n", _Atom(car(cell1)));
		    exit(0);
		}
		DiscAddedCases[n++] = cn;
		cell1 = cdr(cell1);
	    }
	    DiscAddedCases[n] = END_M;
	}
	/* ��ά����õ���ϰ� */
	else if (!strcmp(DEF_DISC_ORDER, _Atom(car(cell1)))) {
	    int pp;

	    cell1 = cdr(cell1);
	    while (!Null(car(cell1))) {
		dicttype = _Atom(car(car(cell1)));
		pp = pp_kstr_to_code(dicttype);
		if (pp == 0 || pp == END_M) {
		    fprintf(stderr, "%s is invalid in .knprc\n", dicttype);
		    exit(0);
		}

		LocationLimit[pp] = atoi(_Atom(car(cdr(car(cell1)))));
		if (LocationLimit[pp] <= 0) {
		    LocationLimit[pp] = END_M;
		}
		if (OptDisplay == OPT_DEBUG) {
		    fprintf(Outfp, "Location category order limit ... %d for %s\n", LocationLimit[pp], dicttype);
		}

		cell1 = cdr(cell1);
	    }
	}
	/* ��ά����õ������ */
	else if (!strcmp(DEF_ANTECEDENT_DECIDE_TH, _Atom(car(cell1)))) {
	    int pp;

	    cell1 = cdr(cell1);
	    while (!Null(car(cell1))) {
		dicttype = _Atom(car(car(cell1)));
		pp = pp_kstr_to_code(dicttype);
		if (pp == 1) {
		    AntecedentDecideThresholdForGa = atof(_Atom(car(cdr(car(cell1)))));
		}
		else if (pp == 2) {
		    AntecedentDecideThresholdPredGeneral = atof(_Atom(car(cdr(car(cell1)))));
		}
		else if (pp == 3) {
		    AntecedentDecideThresholdForNi = atof(_Atom(car(cdr(car(cell1)))));
		}
		else {
		    fprintf(stderr, "%s is invalid in .knprc\n", dicttype);
		    exit(0);
		}

		if (OptDisplay == OPT_DEBUG) {
		    fprintf(Outfp, "Antecedent dicide th ... %s for %s\n", _Atom(car(cdr(car(cell1)))), dicttype);
		}

		cell1 = cdr(cell1);
	    }
	}
	/* ��ά����õ��ʸ�� */
	else if (!strcmp(DEF_DISC_SEN_NUM, _Atom(car(cell1)))) {
	    if (!Atomp(cell2 = car(cdr(cell1)))) {
		fprintf(stderr, "error in .knprc\n");
		exit(0);
	    }
	    else {
		PrevSentenceLimit = atoi(_Atom(cell2));
		if (PrevSentenceLimit < 0 || PrevSentenceLimit >= SENTENCE_MAX) {
		    fprintf(stderr, "%d is invalid in .knprc\n", PrevSentenceLimit);
		    exit(0);
		}
		if (OptDisplay == OPT_DEBUG) {
		    fprintf(Outfp, "Previous sentence limit ... %d\n", PrevSentenceLimit);
		}
	    }
	}
	/* ��ά����õ����� */
	else if (!strcmp(DEF_DISC_LOC_ORDER, _Atom(car(cell1)))) {
	    int pp, count;

	    cell1 = cdr(cell1);
	    while (!Null(car(cell1))) {
		dicttype = _Atom(car(car(cell1)));
		pp = pp_kstr_to_code(dicttype);
		if (pp == 0 || pp == END_M) {
		    fprintf(stderr, "%s is invalid in .knprc\n", dicttype);
		    exit(0);
		}

		if (OptDisplay == OPT_DEBUG) {
		    fprintf(Outfp, "Location category order for %s:", dicttype);
		}

		count = 0;
		cell2 = cdr(car(cell1));
		while (!Null(car(cell2))) {
		    LocationOrder[pp][count] = loc_name_to_code(_Atom(car(cell2)));
		    if (LocationOrder[pp][count] < 0) {
			LocationOrder[pp][count] = END_M;
		    }
		    if (OptDisplay == OPT_DEBUG) {
			fprintf(Outfp, " %s", loc_code_to_str(LocationOrder[pp][count]));
		    }
		    count++;
		    cell2 = cdr(cell2);
		}

		LocationOrder[pp][count] = END_M;
		if (OptDisplay == OPT_DEBUG) {
		    fputs("\n", Outfp);
		}

		cell1 = cdr(cell1);
	    }
	}
#ifdef USE_SVM
	else if (!strcmp(DEF_SVM_FREQ_SD, _Atom(car(cell1)))) {
	    if (!Atomp(cell2 = car(cdr(cell1)))) {
		fprintf(stderr, "error in .knprc\n");
		exit(0);
	    }
	    else {
		SVM_FREQ_SD = atof(_Atom(cell2));
		if (OptDisplay == OPT_DEBUG) {
		    fprintf(Outfp, "SVM FREQ SD ... %f\n", SVM_FREQ_SD);
		}
		if (SVM_FREQ_SD <= 0) {
		    fprintf(stderr, "%s is invalid in .knprc\n", _Atom(cell2));
		    exit(0);
		}
	    }
	}
	else if (!strcmp(DEF_SVM_FREQ_SD_NO, _Atom(car(cell1)))) {
	    if (!Atomp(cell2 = car(cdr(cell1)))) {
		fprintf(stderr, "error in .knprc\n");
		exit(0);
	    }
	    else {
		SVM_FREQ_SD_NO = atof(_Atom(cell2));
		if (OptDisplay == OPT_DEBUG) {
		    fprintf(Outfp, "SVM FREQ SD NO ... %f\n", SVM_FREQ_SD_NO);
		}
		if (SVM_FREQ_SD_NO <= 0) {
		    fprintf(stderr, "%s is invalid in .knprc\n", _Atom(cell2));
		    exit(0);
		}
	    }
	}
	else if (!strcmp(DEF_SVM_REFERRED_NUM_SURFACE_SD, _Atom(car(cell1)))) {
	    if (!Atomp(cell2 = car(cdr(cell1)))) {
		fprintf(stderr, "error in .knprc\n");
		exit(0);
	    }
	    else {
		SVM_R_NUM_S_SD = atof(_Atom(cell2));
		if (OptDisplay == OPT_DEBUG) {
		    fprintf(Outfp, "SVM REFERRED_NUM_SURFACE SD ... %f\n", SVM_R_NUM_S_SD);
		}
		if (SVM_R_NUM_S_SD <= 0) {
		    fprintf(stderr, "%s is invalid in .knprc\n", _Atom(cell2));
		    exit(0);
		}
	    }
	}
	else if (!strcmp(DEF_SVM_REFERRED_NUM_ELLIPSIS_SD, _Atom(car(cell1)))) {
	    if (!Atomp(cell2 = car(cdr(cell1)))) {
		fprintf(stderr, "error in .knprc\n");
		exit(0);
	    }
	    else {
		SVM_R_NUM_E_SD = atof(_Atom(cell2));
		if (OptDisplay == OPT_DEBUG) {
		    fprintf(Outfp, "SVM REFERRED_NUM_ELLIPSIS SD ... %f\n", SVM_R_NUM_E_SD);
		}
		if (SVM_R_NUM_E_SD <= 0) {
		    fprintf(stderr, "%s is invalid in .knprc\n", _Atom(cell2));
		    exit(0);
		}
	    }
	}
	else if (!strcmp(DEF_SVM_MODEL_FILE, _Atom(car(cell1)))) {
	    int pp;

	    cell1 = cdr(cell1);
	    while (!Null(car(cell1))) {
		dicttype = _Atom(car(cdr(car(cell1))));
		if (!strcmp(dicttype, "ALL")) {
		    pp = 0;
		}
		else {
		    pp = pp_kstr_to_code(dicttype);
		    if (pp == 0 || pp == END_M) {
			fprintf(stderr, "%s is invalid in .knprc\n", dicttype);
			exit(0);
		    }
		}

		SVMFile[pp] = check_tilde(_Atom(car(car(cell1))));
 		if ((OptDiscPredMethod == OPT_SVM || OptDiscNounMethod == OPT_SVM) && 
		    OptDisplay == OPT_DEBUG) {
		    fprintf(Outfp, "SVM model file ... %s for %s\n", SVMFile[pp], dicttype);
		}
		cell1 = cdr(cell1);
	    }
	}
	else if (!strcmp(DEF_NE_MODEL_DIR, _Atom(car(cell1)))) {
	    int i;

	    if (!Atomp(cell2 = car(cdr(cell1)))) {
		fprintf(stderr, "error in .knprc\n");
		exit(0);
	    }
	    else {
		KnpNE_Dirname = check_tilde(_Atom(cell2));
		for (i = 0; i < NE_MODEL_NUMBER; i++) {
		    SVMFileNE[i] = (char *)malloc_data(strlen(KnpNE_Dirname)+strlen(ne_code_to_tagposition(i))+8, "NE_model");
		    sprintf(SVMFileNE[i], "%s/%s.model", 
			    KnpNE_Dirname, ne_code_to_tagposition(i));
		    if (OptNE && !OptNECRF && OptDisplay == OPT_DEBUG) {
			fprintf(Outfp, "NE model file ... %s\n", SVMFileNE[i]);
		    }
		}
		DBforNE = (char *)malloc_data(strlen(KnpNE_Dirname)+10, "NE_db");
		sprintf(DBforNE, "%s/table.db",	KnpNE_Dirname);
		if (OptNE && !OptNECRF && OptDisplay == OPT_DEBUG) {
		    fprintf(Outfp, "NE db file ... %s\n", DBforNE);
		}
	    }
	}
#endif
#ifdef USE_CRF
	else if (!strcmp(DEF_NE_MODEL_FILE, _Atom(car(cell1)))) {
	    int i;

	    if (!Atomp(cell2 = car(cdr(cell1)))) {
		fprintf(stderr, "error in .knprc\n");
		exit(0);
	    }
	    else {
		CRFFileNE = check_tilde(_Atom(cell2));
		if (OptNE && OptNECRF && OptDisplay == OPT_DEBUG) {
		    fprintf(Outfp, "NE model file ... %s\n", CRFFileNE);
		}
	    }
	}
#endif
	else if (!strcmp(DEF_SYNONYM_FILE, _Atom(car(cell1)))) {
	    if (!Atomp(cell2 = car(cdr(cell1)))) {
		fprintf(stderr, "error in .knprc\n");
		exit(0);
	    }
	    else {
		SynonymFile = check_tilde(_Atom(cell2));
		if (OptEllipsis & OPT_COREFER && OptDisplay == OPT_DEBUG) {
		    fprintf(Outfp, "Synonym db file ... %s\n", SynonymFile);
		}
	    }
	}
	else if (!strcmp(DEF_DT_MODEL_FILE, _Atom(car(cell1)))) {
	    int pp;

	    cell1 = cdr(cell1);
	    while (!Null(car(cell1))) {
		dicttype = _Atom(car(cdr(car(cell1))));
		if (!strcmp(dicttype, "ALL")) {
		    pp = 0;
		}
		else {
		    pp = pp_kstr_to_code(dicttype);
		    if (pp == 0 || pp == END_M) {
			fprintf(stderr, "%s is invalid in .knprc\n", dicttype);
			exit(0);
		    }
		}

		DTFile[pp] = check_tilde(_Atom(car(car(cell1))));
		if ((OptDiscPredMethod == OPT_DT || OptDiscNounMethod == OPT_DT) && 
		    OptDisplay == OPT_DEBUG) {
		    fprintf(Outfp, "DT file ... %s for %s\n", DTFile[pp], dicttype);
		}

		cell1 = cdr(cell1);
	    }
	}
    }
#endif

    /* knprc �˥롼�뤬���ꤵ��Ƥ��ʤ����Υǥե���ȥ롼�� */
    if (CurrentRuleNum == 0) {
	if (OptDisplay == OPT_DEBUG) {
	    fprintf(Outfp, "Setting default rules ... ");
	}

	RuleNumMax = 12;
	RULE = (RuleVector *)realloc_data(RULE, sizeof(RuleVector)*RuleNumMax, "read_rc");

	/* mrph_homo Ʊ���۵��� */
	(RULE+CurrentRuleNum)->file = strdup("mrph_homo");
	(RULE+CurrentRuleNum)->mode = RLOOP_MRM;
	(RULE+CurrentRuleNum)->breakmode = RLOOP_BREAK_NONE;
	(RULE+CurrentRuleNum)->type = HomoRuleType;
	(RULE+CurrentRuleNum)->direction = LtoR;
	CurrentRuleNum++;

	/* mrph_filter ������-������ �롼��롼����� */
	(RULE+CurrentRuleNum)->file = strdup("mrph_filter");
	(RULE+CurrentRuleNum)->mode = RLOOP_RMM;
	(RULE+CurrentRuleNum)->breakmode = RLOOP_BREAK_NONE;
	(RULE+CurrentRuleNum)->type = PreProcessMorphRuleType;
	(RULE+CurrentRuleNum)->direction = LtoR;
	CurrentRuleNum++;

	/* mrph_auto_dic ������ �롼��롼����� */
	(RULE+CurrentRuleNum)->file = strdup("mrph_auto_dic");
	(RULE+CurrentRuleNum)->mode = RLOOP_RMM;
	(RULE+CurrentRuleNum)->breakmode = RLOOP_BREAK_NONE;
	(RULE+CurrentRuleNum)->type = MorphRuleType;
	(RULE+CurrentRuleNum)->direction = LtoR;
	CurrentRuleNum++;

	/* mrph_basic ������ �롼��롼����� */
	(RULE+CurrentRuleNum)->file = strdup("mrph_basic");
	(RULE+CurrentRuleNum)->mode = RLOOP_RMM;
	(RULE+CurrentRuleNum)->breakmode = RLOOP_BREAK_NONE;
	(RULE+CurrentRuleNum)->type = MorphRuleType;
	(RULE+CurrentRuleNum)->direction = LtoR;
	CurrentRuleNum++;

	/* bnst_basic ʸ�� ������ �롼��롼����� */
	(RULE+CurrentRuleNum)->file = strdup("bnst_basic");
	(RULE+CurrentRuleNum)->mode = RLOOP_RMM;
	(RULE+CurrentRuleNum)->breakmode = RLOOP_BREAK_NONE;
	(RULE+CurrentRuleNum)->type = BnstRuleType;
	(RULE+CurrentRuleNum)->direction = RtoL;
	CurrentRuleNum++;

	/* bnst_type ʸ�� ������ BREAK */
	(RULE+CurrentRuleNum)->file = strdup("bnst_type");
	(RULE+CurrentRuleNum)->mode = RLOOP_MRM;
	(RULE+CurrentRuleNum)->breakmode = RLOOP_BREAK_NORMAL;
	(RULE+CurrentRuleNum)->type = BnstRuleType;
	(RULE+CurrentRuleNum)->direction = RtoL;
	CurrentRuleNum++;

	/* bnst_etc ʸ�� ������ �롼��롼����� */
	(RULE+CurrentRuleNum)->file = strdup("bnst_etc");
	(RULE+CurrentRuleNum)->mode = RLOOP_RMM;
	(RULE+CurrentRuleNum)->breakmode = RLOOP_BREAK_NONE;
	(RULE+CurrentRuleNum)->type = BnstRuleType;
	(RULE+CurrentRuleNum)->direction = RtoL;
	CurrentRuleNum++;

	/* kakari_uke ������� */
	(RULE+CurrentRuleNum)->file = strdup("kakari_uke");
	(RULE+CurrentRuleNum)->mode = RLOOP_MRM;
	(RULE+CurrentRuleNum)->breakmode = RLOOP_BREAK_NONE;
	(RULE+CurrentRuleNum)->type = DpndRuleType;
	(RULE+CurrentRuleNum)->direction = LtoR;
	CurrentRuleNum++;

	/* koou �Ʊ� */
	(RULE+CurrentRuleNum)->file = strdup("koou");
	(RULE+CurrentRuleNum)->mode = RLOOP_MRM;
	(RULE+CurrentRuleNum)->breakmode = RLOOP_BREAK_NONE;
	(RULE+CurrentRuleNum)->type = KoouRuleType;
	(RULE+CurrentRuleNum)->direction = LtoR;
	CurrentRuleNum++;

	/* bnst_basic ���ܶ� ������ �롼��롼����� */
	(RULE+CurrentRuleNum)->file = strdup("bnst_basic");
	(RULE+CurrentRuleNum)->mode = RLOOP_RMM;
	(RULE+CurrentRuleNum)->breakmode = RLOOP_BREAK_NONE;
	(RULE+CurrentRuleNum)->type = TagRuleType;
	(RULE+CurrentRuleNum)->direction = RtoL;
	CurrentRuleNum++;

	/* case_analysis ���ܶ� ������ */
	(RULE+CurrentRuleNum)->file = strdup("case_analysis");
	(RULE+CurrentRuleNum)->mode = RLOOP_MRM;
	(RULE+CurrentRuleNum)->breakmode = RLOOP_BREAK_NONE;
	(RULE+CurrentRuleNum)->type = TagRuleType;
	(RULE+CurrentRuleNum)->direction = RtoL;
	CurrentRuleNum++;

	/* tag_postprocess ���ܶ�-����� ������ */
	(RULE+CurrentRuleNum)->file = strdup("tag_postprocess");
	(RULE+CurrentRuleNum)->mode = RLOOP_MRM;
	(RULE+CurrentRuleNum)->breakmode = RLOOP_BREAK_NONE;
	(RULE+CurrentRuleNum)->type = PostProcessTagRuleType;
	(RULE+CurrentRuleNum)->direction = RtoL;
	CurrentRuleNum++;

	if (OptDisplay == OPT_DEBUG) {
	    fprintf(Outfp, "done.\n");
	}
    }
}

/*==================================================================*/
		    void server_read_rc(FILE *fp)
/*==================================================================*/
{
    clear_rule_configuration();
    set_cha_getc();
    read_rc(fp);
    unset_cha_getc();
}

/*==================================================================*/
     void check_data_newer_than_rule(time_t data, char *datapath)
/*==================================================================*/
{
    /* �롼��ե�����Ȥλ��֥����å� */

    char *rulename;
    int status;
    struct stat sb;

    rulename = strdup(datapath);
    *(rulename+strlen(rulename)-5) = '\0';
    strcat(rulename, ".rule");
    status = stat(rulename, &sb);
    if (!status) {
	if (data < sb.st_mtime) {
	    fprintf(stderr, ";; %s: older than rule file!\n", datapath);
	}
    }
    free(rulename);
}

/*==================================================================*/
		char *check_rule_filename(char *file)
/*==================================================================*/
{
    /* �롼��ե����� (*.data) �� fullpath ���֤��ؿ� */

    char *fullname, *home;
    int status;
    struct stat sb;

    if (!Knprule_Dirname) {
#ifdef KNP_RULE
	Knprule_Dirname = strdup(KNP_RULE);
#else
	fprintf(stderr, ";; Please specify rule directory in .knprc\n");
	exit(0);
#endif
    }

    fullname = (char *)malloc_data(strlen(Knprule_Dirname)+strlen(file)+7, "check_rule_filename");
    sprintf(fullname, "%s/%s.data", Knprule_Dirname, file);
    /* dir + filename + ".data" */
    status = stat(fullname, &sb);

    if (status < 0) {
	*(fullname+strlen(fullname)-5) = '\0';
	/* dir + filename */
	status = stat(fullname, &sb);
	if (status < 0) {
	    /* filename + ".data" */
	    if (*file == '~' && (home = getenv("HOME"))) {
		free(fullname);
		fullname = (char *)malloc_data(strlen(home)+strlen(file)+6, "check_rule_filename");
		sprintf(fullname, "%s%s.data", home, strchr(file, '/'));
	    }
	    else {
		sprintf(fullname, "%s.data", file);
	    }
	    status = stat(fullname, &sb);
	    if (status < 0) {
		*(fullname+strlen(fullname)-5) = '\0';
		/* filename */
		status = stat(fullname, &sb);
		if (status < 0) {
		    fprintf(stderr, "%s: No such file.\n", fullname);
		    exit(1);
		}
	    }
	}
    }

    /* �롼��ե�����Ȥλ��֥����å� */
    check_data_newer_than_rule(sb.st_mtime, fullname);

    return fullname;
}

/*==================================================================*/
	   char *check_dict_filename(char *file, int flag)
/*==================================================================*/
{
    char *fullname, *home;
    int status;
    struct stat sb;

    if (!Knpdict_Dirname) {
#ifdef KNP_DICT
	Knpdict_Dirname = strdup(KNP_DICT);
#else
	fprintf(stderr, "Please specify dict directory in .knprc\n");
	exit(0);
#endif
    }

    fullname = (char *)malloc_data(strlen(Knpdict_Dirname)+strlen(file)+2, "check_dict_filename");
    sprintf(fullname, "%s/%s", Knpdict_Dirname, file);

    /* dir + filename */
    status = stat(fullname, &sb);

    if (status < 0) {
	free(fullname);
	if (*file == '~' && (home = getenv("HOME"))) {
	    fullname = (char *)malloc_data(strlen(home)+strlen(file), "check_dict_filename");
	    sprintf(fullname, "%s%s", home, strchr(file, '/'));
	}
	else {
	    fullname = strdup(file);
	}
	status = stat(fullname, &sb);
	if (status < 0) {
	    /* flag �� FALSE �ΤȤ��ϥե����뤬¸�ߤ��뤫�ɤ��������å����ʤ� */
	    if (flag == FALSE) {
		return fullname;
	    }
	    fprintf(stderr, "%s: No such file.\n", fullname);
	    exit(1);
	}
    }
    return fullname;
}

/*==================================================================*/
     DBM_FILE open_dict(int dic_num, char *dic_name, int *exist)
/*==================================================================*/
{
    char *index_db_filename;
    DBM_FILE db;

    if (DICT[dic_num]) {
	index_db_filename = check_dict_filename(DICT[dic_num], TRUE);
    }
    else {
	index_db_filename = check_dict_filename(dic_name, FALSE);
    }

    if (OptDisplay == OPT_DEBUG) {
	fprintf(Outfp, "Opening %s ... ", index_db_filename);
    }

    if ((db = DB_open(index_db_filename, O_RDONLY, 0)) == NULL) {
	if (OptDisplay == OPT_DEBUG) {
	    fputs("failed.\n", Outfp);
	}
	*exist = FALSE;
    } 
    else {
	if (OptDisplay == OPT_DEBUG) {
	    fputs("done.\n", Outfp);
	}
	*exist = TRUE;
    }

    free(index_db_filename);
    return db;
}

/*==================================================================*/
		  void init_configfile(char *opfile)
/*==================================================================*/
{
    int i;

    for (i = 0; i < DICT_MAX; i++) {
	DICT[i] = NULL;
    }

    THESAURUS[0].path = NULL;
    THESAURUS[0].name = NULL;
    THESAURUS[0].format = NULL;
    THESAURUS[0].exist = 0;
    THESAURUS[1].path = NULL;
    THESAURUS[1].name = strdup("BGH");
    THESAURUS[1].format = NULL;
    THESAURUS[1].code_size = BGH_CODE_SIZE;
    THESAURUS[2].path = NULL;
    THESAURUS[2].name = strdup("NTT");
    THESAURUS[2].format = NULL;
    THESAURUS[2].code_size = SM_CODE_SIZE;

    for (i = 0; i < PP_NUMBER; i++) {
	DTFile[i] = NULL;
#ifdef USE_SVM
	SVMFile[i] = NULL;
#endif
    }

#ifdef _WIN32
    if (!opfile) {
	read_rc(NULL);
    }
    else
#endif
	read_rc(find_rc_file(opfile));
}

/*====================================================================
				 END
====================================================================*/
