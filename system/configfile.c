#include "knp.h"
#include <sys/types.h>
#include <sys/stat.h>

/* $Id$ */

extern FILE *Jumanrc_Fileptr;
extern char Jumangram_Dirname[];
extern int LineNoForError, LineNo;
char Knprule_Dirname[FILENAME_MAX];

RuleVector *RULE;
int CurrentRuleNum = 0;
int RuleNumMax = 0;

/*==================================================================*/
			   void init_knp()
/*==================================================================*/
{
#ifdef  _WIN32
    /* MS Windws �ΤФ�����,juman.ini �򸫤˹Ԥ��褦���ѹ� 
     dicfile == gramfile */
    GetPrivateProfileString("juman","dicfile","",Jumangram_Dirname,sizeof(Jumangram_Dirname),"juman.ini");
#else
    CELL *cell1,*cell2;

    LineNo = 0 ;
    Jumangram_Dirname[0]='\0';

    while (!s_feof(Jumanrc_Fileptr))  {
	LineNoForError = LineNo;
	cell1 = s_read(Jumanrc_Fileptr);

	if (!strcmp(DEF_GRAM_FILE, _Atom(car(cell1)))) {
	    if (!Atomp(cell2 = car(cdr(cell1)))) {
		fprintf(stderr, "error in .jumanrc");
		exit(0);
	    } else 
		strcpy(Jumangram_Dirname, _Atom(cell2));
	}
	/* KNP �롼��ǥ��쥯�ȥ� */
	else if (!strcmp(DEF_KNP_DIR, _Atom(car(cell1)))) {
	    if (!Atomp(cell2 = car(cdr(cell1)))) {
		fprintf(stderr, "error in .jumanrc");
		exit(0);
	    }
	    else
		strcpy(Knprule_Dirname, _Atom(cell2));
	}
	/* KNP �롼��ե����� */
	else if (!strcmp(DEF_KNP_FILE, _Atom(car(cell1)))) {
	    cell1 = cdr(cell1);

	    while (!Null(car(cell1))) {
		if (CurrentRuleNum >= RuleNumMax) {
		    RuleNumMax += RuleIncrementStep;
		    RULE = (RuleVector *)realloc(RULE, sizeof(RuleVector)*RuleNumMax);
		}

		/* �ǥե���������� */
		(RULE+CurrentRuleNum)->file = (char *)strdup(_Atom(car(car(cell1))));
		(RULE+CurrentRuleNum)->mode = RLOOP_MRM;
		(RULE+CurrentRuleNum)->breakmode = RLOOP_BREAK_NONE;
		(RULE+CurrentRuleNum)->direction = 0;

		cell2 = cdr(car(cell1));

		while (!Null(car(cell2))) {
		    if (!strcmp(_Atom(car(cell2)), "Ʊ��¿����")) {
			(RULE+CurrentRuleNum)->type = HomoRuleType;
		    }
		    else if (!strcmp(_Atom(car(cell2)), "������")) {
			(RULE+CurrentRuleNum)->type = MorphRuleType;
		    }
		    else if (!strcmp(_Atom(car(cell2)), "ʸ��")) {
			(RULE+CurrentRuleNum)->type = BnstRuleType;
		    }
		    else if (!strcmp(_Atom(car(cell2)), "�������")) {
			(RULE+CurrentRuleNum)->type = DpndRuleType;
		    }
		    else if (!strcmp(_Atom(car(cell2)), "�Ʊ�")) {
			(RULE+CurrentRuleNum)->type = KoouRuleType;
		    }
		    else if (!strcmp(_Atom(car(cell2)), "��ͭɽ��������")) {
			(RULE+CurrentRuleNum)->type = NeMorphRuleType;
		    }
		    else if (!strcmp(_Atom(car(cell2)), "��ͭɽ����-PRE")) {
			(RULE+CurrentRuleNum)->type = NePhrasePreRuleType;
		    }
		    else if (!strcmp(_Atom(car(cell2)), "��ͭɽ����")) {
			(RULE+CurrentRuleNum)->type = NePhraseRuleType;
		    }
		    else if (!strcmp(_Atom(car(cell2)), "��ͭɽ����-AUX")) {
			(RULE+CurrentRuleNum)->type = NePhraseAuxRuleType;
		    }
		    else if (!strcmp(_Atom(car(cell2)), "ʸ̮")) {
			(RULE+CurrentRuleNum)->type = ContextRuleType;
		    }
		    else if (!strcmp(_Atom(car(cell2)), "�롼��롼�����")) {
			(RULE+CurrentRuleNum)->mode = RLOOP_RMM;
		    }
		    else if (!strcmp(_Atom(car(cell2)), "BREAK")) {
			(RULE+CurrentRuleNum)->breakmode = RLOOP_BREAK_NORMAL;
		    }
		    else {
			fprintf(stderr, "%s is invalid in .jumanrc\n", _Atom(car(cell2)));
			exit(0);
		    }
		    cell2 = cdr(cell2);
		}
		CurrentRuleNum++;
		cell1 = cdr(cell1);
	    }
	}
    }
#endif
}

/*==================================================================*/
		   char *check_filename(char *file)
/*==================================================================*/
{
    /* �롼��ե����� (*.data) �� fullpath ���֤��ؿ� */

    char *fullname, *rulename;
    int status;
    time_t data;
    struct stat sb;

    fullname = (char *)malloc_data(strlen(Knprule_Dirname)+strlen(file)+7, "check_filename");
    sprintf(fullname, "%s/%s.data", Knprule_Dirname, file);
    /* dir + filename + ".data" */
    status = stat(fullname, &sb);

    if (status < 0) {
	*(fullname+strlen(fullname)-5) = '\0';
	/* dir + filename */
	status = stat(fullname, &sb);
	if (status < 0) {
	    sprintf(fullname, "%s.data", file);
	    /* filename + ".data" */
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

    data = sb.st_mtime;

    /* �롼��ե�����Ȥλ��֥����å� */
    rulename = strdup(fullname);
    *(rulename+strlen(rulename)-5) = '\0';
    strcat(rulename, ".rule");
    status = stat(rulename, &sb);
    if (!status) {
	if (data < sb.st_mtime) {
	    fprintf(stderr, "%s: rule file is newer!\n", fullname);
	}
    }
    free(rulename);

    return fullname;
}

/*====================================================================
				 END
====================================================================*/
