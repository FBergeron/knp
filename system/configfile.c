#include "knp.h"
#include <sys/types.h>
#include <sys/stat.h>

/* $Id$ */

extern char Jumangram_Dirname[];
extern int LineNoForError, LineNo;
char *Knprule_Dirname = NULL;
char *Knpdict_Dirname = NULL;

RuleVector *RULE = NULL;
int CurrentRuleNum = 0;
int RuleNumMax = 0;

char *DICT[DICT_MAX];
int knp_dict_file_already_defined = 0;

/*==================================================================*/
			void init_configfile()
/*==================================================================*/
{
    int i;
    for (i = 0; i < DICT_MAX; i++) {
	DICT[i] = NULL;
    }
}

/*==================================================================*/
	    void check_duplicated(int value, char *string)
/*==================================================================*/
{
    /* �ͤ� 0 �Ǥʤ��Ȥ��ϥ��顼 */
    if (value) {
	fprintf(stderr, "%s is duplicately specified in .jumanrc\n", string);
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
			void read_rc(FILE *in)
/*==================================================================*/
{
#ifdef  _WIN32
    /* MS Windws �ΤФ�����,juman.ini �򸫤˹Ԥ��褦���ѹ� 
     dicfile == gramfile */
    GetPrivateProfileString("juman","dicfile","",Jumangram_Dirname,sizeof(Jumangram_Dirname),"juman.ini");
#else
    CELL *cell1,*cell2;
    char *dicttype;

    LineNo = 0 ;
    Jumangram_Dirname[0] = '\0';

    while (!s_feof(in))  {
	LineNoForError = LineNo;
	cell1 = s_read(in);

	if (!strcmp(DEF_GRAM_FILE, _Atom(car(cell1)))) {
	    if (!Atomp(cell2 = car(cdr(cell1)))) {
		fprintf(stderr, "error in .jumanrc\n");
		exit(0);
	    } else 
		strcpy(Jumangram_Dirname, _Atom(cell2));
	}
	/* KNP �롼��ǥ��쥯�ȥ� */
	else if (!strcmp(DEF_KNP_DIR, _Atom(car(cell1)))) {
	    if (!Atomp(cell2 = car(cdr(cell1)))) {
		fprintf(stderr, "error in .jumanrc\n");
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
		    else if (!strcmp(_Atom(car(cell2)), "ʸ��")) {
			check_duplicated((RULE+CurrentRuleNum)->type, "Rule type");
			(RULE+CurrentRuleNum)->type = BnstRuleType;
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
			fprintf(stderr, "%s is invalid in .jumanrc\n", _Atom(car(cell2)));
			exit(0);
		    }
		    cell2 = cdr(cell2);
		}

		/* �롼��Υ����פ����ꤵ��Ƥ��ʤ��Ȥ� */
		if (!(RULE+CurrentRuleNum)->type) {
		    fprintf(stderr, "Rule type for \'%s\' is not specified in .jumanrc\n", 
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
		fprintf(stderr, "error in .jumanrc\n");
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
		    fprintf(stderr, "%s is invalid in .jumanrc\n", _Atom(car(cdr(car(cell1)))));
		    exit(0);
		}
		cell1 = cdr(cell1);
	    }
	}
    }
#endif
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
	    fprintf(stderr, "%s: older than rule file!\n", datapath);
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
	fprintf(stderr, "Please specify rule directory in .jumanrc\n");
	exit(0);
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
		fullname = (char *)malloc_data(strlen(home)+strlen(file), "check_dict_filename");
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
	fprintf(stderr, "Please specify dict directory in .jumanrc\n");
	exit(0);
#endif
    }

    fullname = (char *)malloc_data(strlen(Knpdict_Dirname)+strlen(file)+2, "check_dict_filename");
    sprintf(fullname, "%s/%s", Knpdict_Dirname, file);

    /* flag �� FALSE �ΤȤ��ϥե����뤬¸�ߤ��뤫�ɤ��������å����ʤ� */
    if (flag == FALSE) {
	return fullname;
    }

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
	    fprintf(stderr, "%s: No such file.\n", fullname);
	    exit(1);
	}
    }
    return fullname;
}

/*====================================================================
				 END
====================================================================*/
