/*====================================================================

		     KNP (Kurohashi-Nagao Parser)

    $Id$
====================================================================*/
#include "knp.h"

/* Server Client Extention */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>

SENTENCE_DATA *sp;
SENTENCE_DATA current_sentence_data;
SENTENCE_DATA sentence_data[256];

MRPH_DATA 	mrph_data[MRPH_MAX];		/* �����ǥǡ��� */
BNST_DATA 	bnst_data[BNST_MAX];		/* ʸ��ǡ��� */
PARA_DATA 	para_data[PARA_MAX]; 		/* ����ǡ��� */
PARA_MANAGER	para_manager[PARA_MAX];		/* ��������ǡ��� */
TOTAL_MGR	Best_mgr;			/* ��¸���ʲ��ϴ����ǡ��� */
TOTAL_MGR	Op_Best_mgr;

int 		Para_num;			/* ����¤�� */
int 		Para_M_num;			/* ��������ޥ͡����㡼�� */
int 		Revised_para_num;			

char		Comment[DATA_LEN];		/* �����ȹ� */
char		KNPSID[256];
char		*ErrorComment = NULL;		/* ���顼������ */
char		PM_Memo[256];			/* �ѥ�����ޥå���� */
char            SID_box[256];

char  		cont_str[DBM_CON_MAX];

int		match_matrix[BNST_MAX][BNST_MAX];
int		path_matrix[BNST_MAX][BNST_MAX];
int		restrict_matrix[BNST_MAX][BNST_MAX];
int 		Dpnd_matrix[BNST_MAX][BNST_MAX]; /* �����ǽ�� 0, D, P, A */
int 		Quote_matrix[BNST_MAX][BNST_MAX];/* ��̥ޥ��� 0, 1 */
int 		Mask_matrix[BNST_MAX][BNST_MAX]; /* ����ޥ���
						    0:��������ػ�
						    1:�������OK
						    2:�����head��,
						    3:�����gap��head�� */
char		G_Feature[100][64];		/* FEATURE���ѿ���Ǽ */

int 		OptAnalysis;
int 		OptInput;
int 		OptExpress;
int 		OptDisplay;
int		OptExpandP;
int		OptInhibit;
int		OptCheck;
int		OptNE;
int		OptLearn;
int		OptCaseFlag;
int		OptCFMode;
char		OptIgnoreChar;
char		*OptOptionalCase = NULL;
VerboseType	VerboseLevel = VERBOSE0;

/* Server Client Extention */
int		OptMode = STAND_ALONE_MODE;
int		OptPort = DEFAULT_PORT;
char		OptHostname[256];

FILE		*Infp;
FILE		*Outfp;

char *Case_name[] = {
    		"����", "���", "�˳�", "�ǳ�", "�����", 
		"�ȳ�", "����", "�س�", "�ޥǳ�", "�γ�",
		"����", ""};

char 		*ProgName;
extern FILE	*Jumanrc_Fileptr;
extern CLASS    Class[CLASSIFY_NO + 1][CLASSIFY_NO + 1];
extern TYPE     Type[TYPE_NO];
extern FORM     Form[TYPE_NO][FORM_NO];
int CLASS_num;

#include "extern.h"

extern void read_bnst_rule(char *file_neme, BnstRule *rp, 
			   int *count, int max);

char *ClauseDBname = NULL;
char *ClauseCDBname = NULL;
char *CasePredicateDBname = NULL;
char *OptionalCaseDBname = NULL;

char *EtcRuleFile = NULL;

jmp_buf timeout;
int	ParseTimeout = DEFAULT_PARSETIMEOUT;
char *Opt_jumanrc = NULL;

extern int	DISTANCE_STEP;
extern int	RENKAKU_STEP;
extern int	STRONG_V_COST;
extern int	ADJACENT_TOUTEN_COST;
extern int	LEVELA_COST;
extern int	TEIDAI_STEP;
extern int	EX_match_qua;
extern int	EX_match_unknown;
extern int	EX_match_sentence;
extern int	EX_match_tim;
extern int	SOTO_SCORE;

/*==================================================================*/
			     void usage()
/*==================================================================*/
{
    fprintf(stderr, "Usage: knp [-case|dpnd|bnst|-disc]\n" 
	    "           [-tree|sexp|-tab]\n" 
	    "           [-normal|detail|debug]\n" 
	    "           [-expand]\n"
	    "           [-C host:port] [-S] [-N port]\n"
	    "           [-timeout second] [-r rcfile]\n"
	    "           [-thesaurus [BGH|NTT]]\n");
    exit(1);    
}

/*==================================================================*/
	       void option_proc(int argc, char **argv)
/*==================================================================*/
{
    /* �������� */

    OptAnalysis = OPT_DPND;
    OptInput = OPT_RAW;
    OptExpress = OPT_TREE;
    OptDisplay = OPT_NORMAL;
    OptExpandP = FALSE;
    OptCFMode = EXAMPLE;
    /* �ǥե���ȤǶػߤ��륪�ץ���� */
    OptInhibit = OPT_INHIBIT_CLAUSE | OPT_INHIBIT_CASE_PREDICATE | OPT_INHIBIT_BARRIER | OPT_INHIBIT_OPTIONAL_CASE | OPT_INHIBIT_C_CLAUSE;
    OptCheck = FALSE;
    OptNE = OPT_NORMAL;
    OptLearn = FALSE;
    OptCaseFlag = 0;
    /*    OptIgnoreChar = (char)NULL;*/
    OptIgnoreChar = '\0';

    while ((--argc > 0) && ((*++argv)[0] == '-')) {
	if (str_eq(argv[0], "-case"))         OptAnalysis = OPT_CASE;
	else if (str_eq(argv[0], "-case2"))   OptAnalysis = OPT_CASE2;
	else if (str_eq(argv[0], "-cfsm"))    OptCFMode   = SEMANTIC_MARKER;
	else if (str_eq(argv[0], "-dpnd"))    OptAnalysis = OPT_DPND;
	else if (str_eq(argv[0], "-bnst"))    OptAnalysis = OPT_BNST;
	else if (str_eq(argv[0], "-assignf")) OptAnalysis = OPT_AssignF;
	else if (str_eq(argv[0], "-disc"))    OptAnalysis = OPT_DISC;
	else if (str_eq(argv[0], "-tree"))    OptExpress = OPT_TREE;
	else if (str_eq(argv[0], "-treef"))   OptExpress = OPT_TREEF;
	else if (str_eq(argv[0], "-sexp"))    OptExpress = OPT_SEXP;
	else if (str_eq(argv[0], "-tab"))     OptExpress = OPT_TAB;
	else if (str_eq(argv[0], "-normal"))  OptDisplay = OPT_NORMAL;
	else if (str_eq(argv[0], "-detail"))  OptDisplay = OPT_DETAIL;
	else if (str_eq(argv[0], "-debug"))   OptDisplay = OPT_DEBUG;
	else if (str_eq(argv[0], "-expand"))  OptExpandP = TRUE;
	else if (str_eq(argv[0], "-S"))       OptMode    = SERVER_MODE;
	else if (str_eq(argv[0], "-check"))   OptCheck = TRUE;
	else if (str_eq(argv[0], "-learn"))   OptLearn = TRUE;
	else if (str_eq(argv[0], "-nesm"))    OptNE = OPT_NESM;
	else if (str_eq(argv[0], "-ne"))      OptNE = OPT_NE;
	else if (str_eq(argv[0], "-cc"))      OptInhibit &= ~OPT_INHIBIT_CLAUSE;
	else if (str_eq(argv[0], "-ck"))      OptInhibit &= ~OPT_INHIBIT_CASE_PREDICATE;
	else if (str_eq(argv[0], "-cb"))      OptInhibit &= ~OPT_INHIBIT_BARRIER;
	else if (str_eq(argv[0], "-co"))      OptInhibit &= ~OPT_INHIBIT_OPTIONAL_CASE;
	else if (str_eq(argv[0], "-i")) {
	    argv++; argc--;
	    if (argc < 1) usage();
	    OptIgnoreChar = *argv[0];
	}
	else if (str_eq(argv[0], "-cdb")) {
	    argv++; argc--;
	    if (argc < 1) usage();
	    ClauseDBname = argv[0];
	    OptInhibit &= ~OPT_INHIBIT_CLAUSE;
	}
	else if (str_eq(argv[0], "-ccdb")) {
	    argv++; argc--;
	    if (argc < 1) usage();
	    ClauseCDBname = argv[0];
	    OptInhibit &= ~OPT_INHIBIT_C_CLAUSE;
	}
	else if (str_eq(argv[0], "-kdb")) {
	    argv++; argc--;
	    if (argc < 1) usage();
	    if (CasePredicateDBname) {
		if (strcmp(CasePredicateDBname, argv[0]))
		    usage();
	    }
	    else
		CasePredicateDBname = argv[0];
	    OptInhibit &= ~OPT_INHIBIT_CASE_PREDICATE;
	}
	else if (str_eq(argv[0], "-bdb")) {
	    argv++; argc--;
	    if (argc < 1) usage();
	    if (CasePredicateDBname) {
		if (strcmp(CasePredicateDBname, argv[0]))
		    usage();
	    }
	    else
		CasePredicateDBname = argv[0];
	    OptInhibit &= ~OPT_INHIBIT_BARRIER;
	}
	else if (str_eq(argv[0], "-odb")) {
	    argv++; argc--;
	    if (argc < 1) usage();
	    OptionalCaseDBname = argv[0];
	    OptInhibit &= ~OPT_INHIBIT_OPTIONAL_CASE;
	} 
	else if (str_eq(argv[0], "-N")) {
	    argv++; argc--;
	    if (argc < 1) usage();
	    OptPort = atol(argv[0]);
	}
	else if (str_eq(argv[0], "-C")) {
	    OptMode = CLIENT_MODE;
	    argv++; argc--;
	    if (argc < 1) usage();
	    strcpy(OptHostname,argv[0]);
	}
	else if (str_eq(argv[0], "-optionalcase")) {
	    argv++; argc--;
	    if (argc < 1) usage();
	    /* 
	    if ((case2num(argv[0])) == -1) {
		fprintf(stderr, "Error: Case %s is invalid!\n", argv[0]);
		usage();
	    }
	    */
	    OptOptionalCase = argv[0];
	}
	else if (str_eq(argv[0], "-timeout")) {
	    argv++; argc--;
	    if (argc < 1) usage();
	    ParseTimeout = atoi(argv[0]);
	}
	else if (str_eq(argv[0], "-thesaurus")) {
	    argv++; argc--;
	    if (argc < 1) usage();
	    if (!strcasecmp(argv[0], "ntt")) {
		Thesaurus = USE_NTT;
	    }
	    else if (!strcasecmp(argv[0], "bgh")) {
		Thesaurus = USE_BGH;
	    }
	    else {
		usage();
	    }
	}
	else if (str_eq(argv[0], "-r")) {
	    argv++; argc--;
	    if (argc < 1) usage();
	    Opt_jumanrc = argv[0];
	}
	else if (str_eq(argv[0], "-v")) {
	    argv++; argc--;
	    if (argc < 1) usage();
	    VerboseLevel = atoi(argv[0]);
	}
	/* �ʲ����ѥ��ץ���� */
	else if (str_eq(argv[0], "-soto")) {
	    OptCaseFlag |= OPT_CASE_SOTO;
	}
	else if (str_eq(argv[0], "-gaga")) {
	    OptCaseFlag |= OPT_CASE_GAGA;
	}
	/* �ʲ�������Ĵ���� */
	else if (str_eq(argv[0], "-dcost")) {
	    argv++; argc--;
	    if (argc < 1) usage();
	    DISTANCE_STEP = atoi(argv[0]);
	}
	else if (str_eq(argv[0], "-rcost")) {
	    argv++; argc--;
	    if (argc < 1) usage();
	    RENKAKU_STEP = atoi(argv[0]);
	}
	else if (str_eq(argv[0], "-svcost")) {
	    argv++; argc--;
	    if (argc < 1) usage();
	    STRONG_V_COST = atoi(argv[0]);
	}
	else if (str_eq(argv[0], "-atcost")) {
	    argv++; argc--;
	    if (argc < 1) usage();
	    ADJACENT_TOUTEN_COST = atoi(argv[0]);
	}
	else if (str_eq(argv[0], "-lacost")) {
	    argv++; argc--;
	    if (argc < 1) usage();
	    LEVELA_COST = atoi(argv[0]);
	}
	else if (str_eq(argv[0], "-tscost")) {
	    argv++; argc--;
	    if (argc < 1) usage();
	    TEIDAI_STEP = atoi(argv[0]);
	}
	else if (str_eq(argv[0], "-quacost")) {
	    argv++; argc--;
	    if (argc < 1) usage();
	    EX_match_qua = atoi(argv[0]);
	}
	else if (str_eq(argv[0], "-unknowncost")) {
	    argv++; argc--;
	    if (argc < 1) usage();
	    EX_match_unknown = atoi(argv[0]);
	}
	else if (str_eq(argv[0], "-sentencecost")) {
	    argv++; argc--;
	    if (argc < 1) usage();
	    EX_match_sentence = atoi(argv[0]);
	}
	else if (str_eq(argv[0], "-timecost")) {
	    argv++; argc--;
	    if (argc < 1) usage();
	    EX_match_tim = atoi(argv[0]);
	}
	else if (str_eq(argv[0], "-sotocost")) {
	    argv++; argc--;
	    if (argc < 1) usage();
	    SOTO_SCORE = atoi(argv[0]);
	}
	else {
	    usage();
	}
    }
    if (argc != 0) {
	usage();
    }
}

/*==================================================================*/
			void init_juman(void)
/*==================================================================*/
{
    int i;

    /* rcfile �򤵤�����
       1. -r �ǻ��ꤵ�줿�ե�����
       2. $HOME/.jumanrc
       3. RC_DEFAULT (Makefile)
       �� rcfile���ʤ���Х��顼
    */

    set_jumanrc_fileptr(Opt_jumanrc, TRUE);
    read_rc(Jumanrc_Fileptr);
    grammar(NULL);				/* ʸˡ���� */
    katuyou(NULL);				/* ���Ѽ��� */

    for (i = 1; Class[i][0].id; i++);
    CLASS_num = i;
}

/*==================================================================*/
			void read_rules(void)
/*==================================================================*/
{
    int i;

    for (i = 0; i < CurrentRuleNum; i++) {
	/* Ʊ���۵���롼�� */
	if ((RULE+i)->type == HomoRuleType) {
	    read_homo_rule((RULE+i)->file);
	}
	/* �����ǥ롼�� or ʸ��롼�� */
	else if ((RULE+i)->type == MorphRuleType || (RULE+i)->type == BnstRuleType) {
	    read_general_rule(RULE+i);
	}
	/* ��������롼�� */
	else if ((RULE+i)->type == DpndRuleType) {
	    read_dpnd_rule((RULE+i)->file);
	}
	/* �Ʊ�ɽ���롼�� */
	else if ((RULE+i)->type == KoouRuleType) {
	    read_koou_rule((RULE+i)->file);
	}
	/* ��ͭ̾��롼�� */
	else if ((RULE+i)->type == NeMorphRuleType) {
	    read_mrph_rule((RULE+i)->file, NERuleArray, &CurNERuleSize, NERule_MAX);
	}
	/* ʣ��̾������롼�� */
	else if ((RULE+i)->type == NePhrasePreRuleType) {
	    read_mrph_rule((RULE+i)->file, CNpreRuleArray, &CurCNpreRuleSize, CNRule_MAX);
	}
	/* ʣ��̾��롼�� */
	else if ((RULE+i)->type == NePhraseRuleType) {
	    read_mrph_rule((RULE+i)->file, CNRuleArray, &CurCNRuleSize, CNRule_MAX);
	}
	/* ʣ��̾������롼�� */
	else if ((RULE+i)->type == NePhraseAuxRuleType) {
	    read_mrph_rule((RULE+i)->file, CNauxRuleArray, &CurCNauxRuleSize, CNRule_MAX);
	}
	/* ʸ̮�����Υ롼�� */
	else if ((RULE+i)->type == ContextRuleType) {
	    read_bnst_rule((RULE+i)->file, ContRuleArray, &ContRuleSize, ContRule_MAX);
	}
    }
}

/*==================================================================*/
	       static void timeout_function(int signal)
/*==================================================================*/
{
    longjmp(timeout, 1);
}

/*==================================================================*/
			   void init_all()
/*==================================================================*/
{
    /* ����� */

    init_configfile();	/* �Ƽ�ե������������� */
    init_juman();	/* JUMAN�ط� */
    init_cf();		/* �ʥե졼�४���ץ� */
    init_bgh();		/* �������饹�����ץ� */
    init_sm();		/* NTT ���񥪡��ץ� */
    init_scase();	/* ɽ�سʼ��񥪡��ץ� */

    if (OptAnalysis == OPT_DISC)
	init_noun();	/* ̾�켭�񥪡��ץ� */

    if (!(OptInhibit & OPT_INHIBIT_CLAUSE))
	init_clause();
    if (!((OptInhibit & OPT_INHIBIT_CASE_PREDICATE) && (OptInhibit & OPT_INHIBIT_BARRIER)))
	init_case_pred();
    if (!(OptInhibit & OPT_INHIBIT_OPTIONAL_CASE) || OptOptionalCase)
	init_optional_case();

    current_sentence_data.mrph_data = mrph_data;
    current_sentence_data.bnst_data = bnst_data;
    current_sentence_data.para_data = para_data;
    current_sentence_data.para_manager = para_manager;
    current_sentence_data.Sen_num = 0;	/* ��������������Ƥ��� */
    current_sentence_data.Mrph_num = 0;
    current_sentence_data.Bnst_num = 0;
    current_sentence_data.New_Bnst_num = 0;
    current_sentence_data.KNPSID = KNPSID;

    /* ��ͭ̾����ϼ��񥪡��ץ� */
    if (OptNE != OPT_NORMAL) {
	init_proper(&current_sentence_data);
    }
}

/*==================================================================*/
		       void knp_main()
/*==================================================================*/
{
    int i, j, flag, success = 1;
    int relation_error, d_struct_error;
    char *code;

    /* sp�򤭤�����Ϥ��褦�ˤ���� sp��local�ˤ��ơ��������
       ����Ǥ褯�ʤ�
       sentence_data *sp = &current_sentence_data; */

    sp = &current_sentence_data;

    /* �ʲ��Ϥν��� */
    init_cf2();
    init_case_analysis();

    /* �롼���ɤ߹��� */
    read_rules();

    while ( 1 ) {

	/* Server Mode �ξ�� ����ν��Ϥ��������Ƥʤ����� 
	   ERROR �ȤϤ� Server/Client �⡼�ɤξ���,���Ϥ�Ʊ���򤳤�ǹԤ� */
	if (!success && OptMode == SERVER_MODE) {
	    fprintf(Outfp,"EOS ERROR\n");
	    fflush(Outfp);
	}

	/********************/
	/* ���β��Ϥθ���� */
	/********************/

	/* �����ॢ���Ȼ� */

	if (setjmp(timeout)) {
#ifdef DEBUG
	    fprintf(stderr, "Parse timeout.\n(");
	    for (i = 0; i < sp->Mrph_num; i++)
		fprintf(stderr, "%s", sp->mrph_data[i].Goi2);
	    fprintf(stderr, ")\n");
#endif
	    ErrorComment = strdup("Parse timeout");
	    when_no_dpnd_struct();
	    dpnd_info_to_bnst(&(Best_mgr.dpnd));
	    if (OptAnalysis != OPT_DISC) print_result();
	    fflush(Outfp);
	}

	/* FEATURE �ν���� */

	if (OptAnalysis == OPT_DISC) {
	    /* ��Ȥ���¸���Ƥ����Τ� */
	    for (i = 0; i < sp->Mrph_num; i++)
		(sp->mrph_data+i)->f = NULL;
	    for (i = 0; i < sp->Bnst_num + sp->New_Bnst_num; i++)
		(sp->bnst_data+i)->f = NULL;
	}
	else {
	    for (i = 0; i < sp->Mrph_num; i++) 
		clear_feature(&(sp->mrph_data[i].f));
	    for (i = 0; i < sp->Bnst_num; i++) 
		clear_feature(&(sp->bnst_data[i].f));
	    /* New_Bnst�Ϥ�Ȥ��pointer */
	    for (i = sp->Bnst_num; i < sp->Bnst_num + sp->New_Bnst_num; i++)
		(sp->bnst_data+i)->f = NULL;
	}

	/**********************/
	/* ������ʸ�β��ϳ��� */
	/**********************/

	success = 0;

	/* �ɤ߹��� */

	sp->Sen_num ++;

	if ((flag = read_mrph(Infp)) == EOF) break;

	if (flag == FALSE) continue;

	/* �����Ǥؤΰ�̣������Ϳ */

	if (OptNE != OPT_NORMAL && SMExist == TRUE) {
	    for (i = 0; i < sp->Mrph_num; i++) {
		code = (char *)get_sm(sp->mrph_data[i].Goi);
		if (code) {
		    strcpy(sp->mrph_data[i].SM, code);
		    free(code);
		}
		assign_ntt_dict(i);
	    }
	}
	else {
	    sp->mrph_data[i].SM = NULL;
	}

	/* �����Ǥؤ�FEATURE��Ϳ */

	assign_cfeature(&(sp->mrph_data[0].f), "ʸƬ");
	assign_cfeature(&(sp->mrph_data[sp->Mrph_num-1].f), "ʸ��");
	assign_general_feature(MorphRuleType);

	/* �����Ǥ�ʸ��ˤޤȤ�� */

	if (OptInput == OPT_RAW) {
	    if (make_bunsetsu() == FALSE) continue;
	} else {
	    if (make_bunsetsu_pm() == FALSE) continue;
	}

	/* ʸ�Ჽ�����ξ�� */

	if (OptAnalysis == OPT_BNST) goto ENDofANALYSIS;

	/* ʸ��ؤΰ�̣������Ϳ */

	for (i = 0; i < sp->Bnst_num; i++) {
	    get_bgh_code(sp->bnst_data+i);		/* �������饹 */
	    get_sm_code(sp->bnst_data+i);		/* ��̣�� */
	}

	/* ʸ��ؤ�FEATURE��Ϳ */

	assign_cfeature(&(sp->bnst_data[0].f), "ʸƬ");
	if (sp->Bnst_num > 0)
	    assign_cfeature(&(sp->bnst_data[sp->Bnst_num-1].f), "ʸ��");
	else
	    assign_cfeature(&(sp->bnst_data[0].f), "ʸ��");
	assign_general_feature(BnstRuleType);

	if (OptDisplay == OPT_DETAIL || OptDisplay == OPT_DEBUG)
	    print_mrphs(0);

	/* ����°�������Ū����Ϳ���� */
	for (i = 0; i < sp->Bnst_num; i++) {
	    if (!check_feature((sp->bnst_data+i)->f, "����") && 
		check_feature((sp->bnst_data+i)->f, "�θ�") && 
		sm_time_match((sp->bnst_data+i)->SM_code)) {
		assign_cfeature(&((sp->bnst_data+i)->f), "����");
	    }
	}

	/* FEATURE��Ϳ�����ξ�� */

	if (OptAnalysis == OPT_AssignF) goto ENDofANALYSIS;


	assign_dpnd_rule();			/* ���������§ */

	if (OptAnalysis == OPT_CASE ||
	    OptAnalysis == OPT_CASE2 ||
	    OptAnalysis == OPT_DISC)
	    set_pred_caseframe();		/* �Ѹ��γʥե졼�� */

	if (OptDisplay == OPT_DETAIL || OptDisplay == OPT_DEBUG)
	    check_bnst();

	/**************/
	/* �ܳ�Ū���� */
	/**************/

	if (OptInput == OPT_PARSED) {
	    dpnd_info_to_bnst(&(Best_mgr.dpnd)); 
	    para_recovery();
	    after_decide_dpnd();
	    goto PARSED;
	}

	calc_dpnd_matrix();			/* ��¸��ǽ���׻� */
	if (OptDisplay == OPT_DEBUG) print_matrix(PRINT_DPND, 0);

	/* �Ʊ�ɽ���ν��� */

	if (koou() == TRUE && OptDisplay == OPT_DEBUG)
	    print_matrix(PRINT_DPND, 0);

	/* ����̤ν��� */

	if ((flag = quote()) == TRUE && OptDisplay == OPT_DEBUG)
	    print_matrix(PRINT_QUOTE, 0);

	if (flag == CONTINUE) continue;

	/* ��������ط����ʤ������д� */
	
	if (relax_dpnd_matrix() == TRUE && OptDisplay == OPT_DEBUG) {
	    fprintf(Outfp, "Relaxation ... \n");
	    print_matrix(PRINT_DPND, 0);
	}

	/****************/
	/* ����¤���� */
	/****************/

	init_mask_matrix();
	Para_num = 0;	
	Para_M_num = 0;
	relation_error = 0;
	d_struct_error = 0;
	Revised_para_num = -1;

	if ((flag = check_para_key()) > 0) {
	    calc_match_matrix();		/* ʸ�������ٷ׻� */
	    detect_all_para_scope();	    	/* ����¤���� */
	    do {
		if (OptDisplay == OPT_DETAIL || OptDisplay == OPT_DEBUG) {
		    print_matrix(PRINT_PARA, 0);
		    /*
		      print_matrix2ps(PRINT_PARA, 0);
		      exit(0);
		      */
		}
		/* ����¤�֤νŤʤ���� */
		if (detect_para_relation() == FALSE) {
		    relation_error++;
		    continue;
		}
		if (OptDisplay == OPT_DEBUG) print_para_relation();
		/* ����¤��ΰ�¸��¤�����å� */
		if (check_dpnd_in_para() == FALSE) {
		    d_struct_error++;
		    continue;
		}
		if (OptDisplay == OPT_DEBUG) print_matrix(PRINT_MASK, 0);
		goto ParaOK;		/* ����¤�������� */
	    } while (relation_error <= 3 &&
		     d_struct_error <= 3 &&
		     detect_para_scope(Revised_para_num, TRUE) == TRUE);
	    ErrorComment = strdup("Cannot detect consistent CS scopes");
	    init_mask_matrix();
	ParaOK:
	}
	else if (flag == CONTINUE)
	    continue;

	/********************/
	/* ��¸���ʹ�¤���� */
	/********************/

	para_postprocess();	/* ��conjunct��head������η������ */

	signal(SIGALRM, timeout_function);
	alarm(ParseTimeout);
	if (detect_dpnd_case_struct() == FALSE) {
	    ErrorComment = strdup("Cannot detect dependency structure");
	    when_no_dpnd_struct();	/* ���������¤����ޤ�ʤ����
					   ���٤�ʸ�᤬�٤˷���Ȱ��� */
	}
	alarm(0);

	/* �����ѥ��١�������ɾ���ͷ׻� */
	if (!(OptInhibit & OPT_INHIBIT_OPTIONAL_CASE))
	    optional_case_evaluation();

    PARSED:

	/* ������������ bnst ��¤�Τ˵��� */
	dpnd_info_to_bnst(&(Best_mgr.dpnd)); 
	para_recovery();

	/* ��ͭ̾��ǧ������ */

	if (OptNE != OPT_NORMAL)
	    NE_analysis();
	else
	    assign_mrph_feature(CNRuleArray, CurCNRuleSize,
				sp->mrph_data, sp->Mrph_num,
				RLOOP_RMM, FALSE, LtoR);

	memo_by_program();	/* ���ؤν񤭹��� */

	/* �����å��� */
	if (OptCheck == TRUE)
	    CheckCandidates();

	if (OptLearn == TRUE)
	    fprintf(Outfp, ";;;OK ���� %d %s %d\n", Best_mgr.ID, sp->KNPSID, Best_mgr.score);

	/* �¸� */
	if (OptCheck == TRUE)
	    CheckChildCaseFrame();

	/* ǧ��������ͭ̾�����¸���Ƥ��� */
	if (OptNE != OPT_NORMAL) {
	    preserveNE();
	    if (OptDisplay == OPT_DEBUG)
		printNE();
	}

	/************/
	/* ʸ̮���� */
	/************/

	if (OptAnalysis == OPT_DISC) {
	    make_dpnd_tree();
	    discourse_analysis();
	}

    ENDofANALYSIS:

	/************/
	/* ���ɽ�� */
	/************/

	if (OptAnalysis == OPT_BNST) {
	    print_mrphs(0);
	} else {
	    print_result();

	    if (!(OptInhibit & OPT_INHIBIT_OPTIONAL_CASE))
		unsupervised_debug_print();
	}
	fflush(Outfp);

	if (OptAnalysis == OPT_CASE || 
	    OptAnalysis == OPT_CASE2 || 
	    OptAnalysis == OPT_DISC) {
	    clear_cf();
	}

	success = 1;	/* OK ���� */
    }

    close_cf();
    close_bgh();
    close_sm();
    close_scase();

    if (OptAnalysis == OPT_DISC)
	close_noun();
    if (OptNE != OPT_NORMAL)
	close_proper();
    if (!(OptInhibit & OPT_INHIBIT_CLAUSE))
	close_clause();
    if (!(OptInhibit & OPT_INHIBIT_CASE_PREDICATE))
	close_case_pred();
    if (!(OptInhibit & OPT_INHIBIT_OPTIONAL_CASE))
	close_optional_case();
    /* close_dic_for_rule(); */
}

/*==================================================================*/
			  void server_mode()
/*==================================================================*/
{
    /* �����Х⡼�� */

    int sfd,fd;
    struct sockaddr_in sin;

    /* �����ʥ���� */
    static void sig_child()
	{
	    int status;
	    while(wait3(&status, WNOHANG, NULL) > 0) {}; 
	    signal(SIGCHLD, sig_child); 
	}

    static void sig_term()
	{
	    shutdown(sfd,2);
	    shutdown(fd, 2);
	    exit(0);
	}

    signal(SIGHUP,  SIG_IGN);
    signal(SIGPIPE, SIG_IGN);
    signal(SIGTERM, sig_term);
    signal(SIGINT,  sig_term);
    signal(SIGQUIT, sig_term);
    signal(SIGCHLD, sig_child);
  
    if((sfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
	fprintf(stderr,"Socket Error\n");
	exit(1);
    }
  
    memset(&sin, 0, sizeof(sin));
    sin.sin_port        = htons(OptPort);
    sin.sin_family      = AF_INET;
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
  
    /* bind */  
    if (bind(sfd, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
	fprintf(stderr,"bind Error\n");
	close(sfd);
	exit(1);
    }
  
    /* listen */  
    if (listen(sfd, SOMAXCONN) < 0) {
	fprintf(stderr,"listen Error\n");
	close(sfd);
	exit(1);
    }

    /* accept loop */
    while(1) {
	int pid;

	if((fd = accept(sfd, NULL, NULL)) < 0) {
	    if (errno == EINTR) 
		continue;
	    fprintf(stderr,"accept Error\n");
	    close(sfd);
	    exit(1);
	}
    
	/* �Һ�꼺�� �������� */
	if((pid = fork()) < 0) {
	    fprintf(stderr, "Fork Error\n");
	    sleep(1);
	    continue;
	}

	/* �Ҷ��Ǥ�����ä� */
	if(pid == 0) {

	    /* �������� ����ʤȤ���� ����Ϥ��ʤ� */
	    chdir("/tmp");

	    close(sfd);
	    Infp  = fdopen(fd, "r");
	    Outfp = fdopen(fd, "w");

	    /* �ɤ��Ҥ˰�Ĥˤ� �������ޤ��礦��.. */
	    fprintf(Outfp, "200 Running KNP Server\n");
	    fflush(Outfp);

	    /* ���ץ������� */
	    while (1) {
		char buf[1024];

		fgets(buf, sizeof(buf), Infp);

		/* QUIT */
		if (strncasecmp(buf, "QUIT", 4) == 0) {
		    fprintf(Outfp, "200 OK Quit\n");
		    fflush(Outfp);
		    exit(0);
		}

		if (strncasecmp(buf, "RC", 2) == 0) {
		    server_read_rc(Infp);
		    fprintf(Outfp, "200 OK\n");
		    fflush(Outfp);
		    continue;
		}

		/* RUN */
		/* Option ���Ϥ� strstr �ʤ󤫤Ǥ��ʤꤨ�������� 
		   �Ĥޤ�ְ�ä����ץ����ϥ��顼�ˤʤ��... */
		if (strncasecmp(buf, "RUN", 3) == 0) {
		    char *p;

		    if (strstr(buf, "-case"))   OptAnalysis = OPT_CASE;
		    if (strstr(buf, "-case2"))  OptAnalysis = OPT_CASE2;
		    if (strstr(buf, "-dpnd"))   OptAnalysis = OPT_DPND;
		    if (strstr(buf, "-bnst"))   OptAnalysis = OPT_BNST;
		    if (strstr(buf, "-tree"))   OptExpress = OPT_TREE;
		    if (strstr(buf, "-sexp"))   OptExpress = OPT_SEXP;
		    if (strstr(buf, "-tab"))    OptExpress = OPT_TAB;
		    if (strstr(buf, "-normal")) OptDisplay = OPT_NORMAL;
		    if (strstr(buf, "-detail")) OptDisplay = OPT_DETAIL;
		    if (strstr(buf, "-debug"))  OptDisplay = OPT_DEBUG;
		    if (strstr(buf, "-expand")) OptExpandP = TRUE;
		    /* ������ �����Ȥ�ΤϺ������ʤ�..
		       �Ȥ��⤤�ĤĤ��ʤ궯��... */
		    if ((p = strstr(buf, "-i")) != NULL) {
			p += 3;
			while(*p != '\0' && (*p == ' ' || *p == '\t')) p++;
			if (*p != '\0') OptIgnoreChar = *p;
		    } 
		    fprintf(Outfp,"200 OK option=[Analysis=%d Express=%d"
			    " Display=%d IgnoreChar=%c]\n",
			    OptAnalysis,OptExpress,OptDisplay,OptIgnoreChar);
		    fflush(Outfp);
		    break;
		} else {
		    fprintf(Outfp,"500 What?\n");
		    fflush(Outfp);
		}
	    }

	    /* ���� */
	    knp_main();

	    /* ����� */
	    shutdown(fd,2);
	    fclose(Infp);
	    fclose(Outfp);
	    close(fd);
	    exit(0); /* ���줷�ʤ������Ѥʤ��Ȥˤʤ뤫�� */
	}

	/* �� */
	close(fd);
    }
}

/*==================================================================*/
			  void client_mode()
/*==================================================================*/
{
    /* ���饤����ȥ⡼�� */

    struct sockaddr_in sin;
    struct hostent *hp;
    int fd;
    FILE *fi,*fo;
    char *p;
    char buf[1024*8];
    char option[1024];
    int  port = DEFAULT_PORT;
    int  strnum = 0;

	/* ʸ��������ä� ���ơ����������ɤ��֤� */  
    int send_string(char *str)
	{
	    int len,result = 0;
	    char buf[1024];
    
	    if (str != NULL){
		fwrite(str,sizeof(char),strlen(str),fo);
		fflush(fo);
	    }

	    while (fgets(buf,sizeof(buf)-1,fi) != NULL){
		len = strlen(buf);
		if (len >= 3 && buf[3] == ' ') {
		    buf[3] = '\0';
		    result = atoi(&buf[0]);
		    break;
		}
	    }

	    return result;
	} 

    /* host:port �äƷ��ξ�� */
    if ((p = strchr(OptHostname, ':')) != NULL) {
	*p++ = '\0';
	port = atoi(p);
    }

    /* ���Ȥ� �Ĥʤ������ */
    if ((hp = gethostbyname(OptHostname)) == NULL) {
	fprintf(stderr,"host unkown\n");
	exit(1);
    }
  
    while ((fd = socket(AF_INET,SOCK_STREAM,0 )) < 0 ){
	fprintf(stderr,"socket error\n");
	exit(1);
    }
  
    sin.sin_family = AF_INET;
    sin.sin_port   = htons(port);
    sin.sin_addr = *((struct in_addr * )hp->h_addr);

    if (connect(fd,(struct sockaddr *)&sin, sizeof(sin)) < 0) {
	fprintf(stderr,"connect error\n");
	exit(1);
    }

    /* Server �ѤȤ� �̿��ϥ�ɥ����� */
    if ((fi = fdopen(fd, "r")) == NULL || (fo = fdopen(fd, "w")) == NULL) {
	close (fd);
	fprintf(stderr,"fd error\n");
	exit(1);
    }

    /* �����ϸ��������� */
    if (send_string(NULL) != 200) {
	fprintf(stderr,"greet error\n");
	exit(1);
    }

    /* ���ץ������� ��ä� ���ޡ��Ȥʤ�����ʤ����ʤ� */
    option[0] = '\0';
    switch (OptAnalysis) {
    case OPT_CASE: strcat(option," -case"); break;
    case OPT_DPND: strcat(option," -dpnd"); break;
    case OPT_BNST: strcat(option," -bnst"); break;
    }

    switch (OptExpress) {
    case OPT_TREE: strcat(option," -tree"); break;
    case OPT_SEXP: strcat(option," -sexp"); break;
    case OPT_TAB:  strcat(option," -tab");  break;
    }

    switch (OptDisplay) {
    case OPT_NORMAL: strcat(option," -normal"); break;
    case OPT_DETAIL: strcat(option," -detail"); break;
    case OPT_DEBUG:  strcat(option," -debug");  break;
    }
    
    if (OptExpandP) strcat(option," -expand");
    if (!OptIgnoreChar) {
	sprintf(buf," -i %c",OptIgnoreChar);
	strcat(option,buf);
    }

    /* ���줫��ư��� */
    sprintf(buf,"RUN%s\n",option);
    if (send_string(buf) != 200) {
	fprintf(stderr,"argument error OK? [%s]\n",option);
	close(fd);
	exit(1);
    }

    /* ���Ȥ� LOOP */
    strnum = 0;
    while (fgets(buf,sizeof(buf),stdin) != NULL) {
	if (strncmp(buf,"EOS",3) == 0) {
	    if (strnum != 0) {
		fwrite(buf,sizeof(char),strlen(buf),fo);
		fflush(fo);
		strnum = 0;
		while (fgets(buf,sizeof(buf),fi) != NULL) {
		    fwrite(buf,sizeof(char),strlen(buf),stdout);
		    fflush(stdout);
		    if (strncmp(buf,"EOS",3) == 0)  break;
		}
	    }
	} else {
	    fwrite(buf,sizeof(char),strlen(buf),fo);
	    fflush(fo);
	    strnum++;
	}
    }

    /* ��λ���� */
    fprintf(fo,"\n%c\nQUIT\n", EOf);
    fclose(fo);
    fclose(fi);
    close(fd);
    exit(0);
}


/*==================================================================*/
		   int main(int argc, char **argv)
/*==================================================================*/
{
    option_proc(argc, argv);

    Infp  = stdin;
    Outfp = stdout;

    /* �⡼�ɤˤ�äƽ�����ʬ�� */
    if (OptMode == STAND_ALONE_MODE) {
	init_all();
	knp_main();
    } else if (OptMode == SERVER_MODE) {
	init_all();
	server_mode();
    } else if (OptMode == CLIENT_MODE) {
	client_mode();
    }
    exit(0);
}

/*====================================================================
                               END
====================================================================*/
