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

SENTENCE_DATA	current_sentence_data;
SENTENCE_DATA	sentence_data[256];

MRPH_DATA 	mrph_data[MRPH_MAX];		/* �����ǥǡ��� */
BNST_DATA 	bnst_data[BNST_MAX];		/* ʸ��ǡ��� */
PARA_DATA 	para_data[PARA_MAX]; 		/* ����ǡ��� */
PARA_MANAGER	para_manager[PARA_MAX];		/* ��������ǡ��� */
TOTAL_MGR	Best_mgr;			/* ��¸���ʲ��ϴ����ǡ��� */
TOTAL_MGR	Op_Best_mgr;

int 		Revised_para_num;			

char		*ErrorComment = NULL;		/* ���顼������ */
char		PM_Memo[256];			/* �ѥ�����ޥå���� */

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
int		OptNE;
int		OptCFMode;
char		OptIgnoreChar;
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

jmp_buf timeout;
int	ParseTimeout = DEFAULT_PARSETIMEOUT;
char *Opt_jumanrc = NULL;


/*==================================================================*/
			     void usage()
/*==================================================================*/
{
    fprintf(stderr, "Usage: knp [-case|dpnd|bnst]\n" 
	    "           [-tree|sexp|-tab]\n" 
	    "           [-normal|detail|debug]\n" 
	    "           [-expand]\n"
	    "           [-C host:port] [-S] [-N port]\n"
	    "           [-timeout second] [-r rcfile]\n"
	    "           [-thesaurus [BGH|NTT]] (Default:NTT)\n"
	    "           [-para [BGH|NTT]] (Default:NTT)\n");
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
    OptNE = OPT_NORMAL;
    OptIgnoreChar = '\0';

    while ((--argc > 0) && ((*++argv)[0] == '-')) {
	if (str_eq(argv[0], "-case"))         OptAnalysis = OPT_CASE;
	else if (str_eq(argv[0], "-case2"))   OptAnalysis = OPT_CASE2;
	else if (str_eq(argv[0], "-cfsm"))    OptCFMode   = SEMANTIC_MARKER;
	else if (str_eq(argv[0], "-dpnd"))    OptAnalysis = OPT_DPND;
	else if (str_eq(argv[0], "-bnst"))    OptAnalysis = OPT_BNST;
	else if (str_eq(argv[0], "-assignf")) OptAnalysis = OPT_AssignF;
	else if (str_eq(argv[0], "-tree"))    OptExpress  = OPT_TREE;
	else if (str_eq(argv[0], "-treef"))   OptExpress  = OPT_TREEF;
	else if (str_eq(argv[0], "-sexp"))    OptExpress  = OPT_SEXP;
	else if (str_eq(argv[0], "-tab"))     OptExpress  = OPT_TAB;
	else if (str_eq(argv[0], "-normal"))  OptDisplay  = OPT_NORMAL;
	else if (str_eq(argv[0], "-detail"))  OptDisplay  = OPT_DETAIL;
	else if (str_eq(argv[0], "-debug"))   OptDisplay  = OPT_DEBUG;
	else if (str_eq(argv[0], "-expand"))  OptExpandP  = TRUE;
	else if (str_eq(argv[0], "-S"))       OptMode     = SERVER_MODE;
	else if (str_eq(argv[0], "-nesm"))    OptNE       = OPT_NESM;
	else if (str_eq(argv[0], "-ne"))      OptNE       = OPT_NE;
	else if (str_eq(argv[0], "-i")) {
	    argv++; argc--;
	    if (argc < 1) usage();
	    OptIgnoreChar = *argv[0];
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
	else if (str_eq(argv[0], "-para")) {
	    argv++; argc--;
	    if (argc < 1) usage();
	    if (!strcasecmp(argv[0], "ntt")) {
		ParaThesaurus = USE_NTT;
	    }
	    else if (!strcasecmp(argv[0], "bgh")) {
		ParaThesaurus = USE_BGH;
	    }
	    else if (!strcasecmp(argv[0], "none")) {
		ParaThesaurus = USE_NONE;
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
    int i;

    /* ����� */

    init_configfile();	/* �Ƽ�ե������������� */
    init_juman();	/* JUMAN�ط� */
    init_cf();		/* �ʥե졼�४���ץ� */
    init_bgh();		/* �������饹�����ץ� */
    init_sm();		/* NTT ���񥪡��ץ� */
    init_scase();	/* ɽ�سʼ��񥪡��ץ� */

    current_sentence_data.mrph_data = mrph_data;
    current_sentence_data.bnst_data = bnst_data;
    current_sentence_data.para_data = para_data;
    current_sentence_data.para_manager = para_manager;
    current_sentence_data.Sen_num = 0;	/* ��������������Ƥ��� */
    current_sentence_data.Mrph_num = 0;
    current_sentence_data.Bnst_num = 0;
    current_sentence_data.New_Bnst_num = 0;
    current_sentence_data.Best_mgr = &Best_mgr;
    current_sentence_data.KNPSID = NULL;
    current_sentence_data.Comment = NULL;

    for (i = 0; i < BNST_MAX; i++) {
	 current_sentence_data.bnst_data[i].internal_num = 0;
	 current_sentence_data.bnst_data[i].f = NULL;
    }

    /* ��ͭ̾����ϼ��񥪡��ץ� */
    if (OptNE != OPT_NORMAL) {
	init_proper(&current_sentence_data);
    }
}

/*==================================================================*/
	  int main_analysis(SENTENCE_DATA *sp, FILE *input)
/*==================================================================*/
{
    int flag, i;
    int relation_error, d_struct_error;
    char *code;

    sp->Sen_num ++;

    if ((flag = read_mrph(sp, input)) == EOF) return EOF;
    if (flag == FALSE) return FALSE;

    /* �����Ǥؤΰ�̣������Ϳ */

    if (OptNE != OPT_NORMAL && SMExist == TRUE) {
	for (i = 0; i < sp->Mrph_num; i++) {
	    code = (char *)get_sm(sp->mrph_data[i].Goi);
	    if (code) {
		strcpy(sp->mrph_data[i].SM, code);
		free(code);
	    }
	    assign_ntt_dict(sp, i);
	}
    }

    /* �����Ǥؤ�FEATURE��Ϳ */

    assign_cfeature(&(sp->mrph_data[0].f), "ʸƬ");
    assign_cfeature(&(sp->mrph_data[sp->Mrph_num-1].f), "ʸ��");
    assign_general_feature(sp, MorphRuleType);

    /* �����Ǥ�ʸ��ˤޤȤ�� */

    if (OptInput == OPT_RAW) {
	if (make_bunsetsu(sp) == FALSE) return FALSE;
    } else {
	if (make_bunsetsu_pm(sp) == FALSE) return FALSE;
    }

    /* ʸ�Ჽ�����ξ�� */

    if (OptAnalysis == OPT_BNST) return TRUE;

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
    assign_general_feature(sp, BnstRuleType);

    if (OptDisplay == OPT_DETAIL || OptDisplay == OPT_DEBUG)
	print_mrphs(sp, 0);

	/* ����°�������Ū����Ϳ���� */
    for (i = 0; i < sp->Bnst_num; i++) {
	if (!check_feature((sp->bnst_data+i)->f, "����") && 
	    check_feature((sp->bnst_data+i)->f, "�θ�") && 
	    sm_time_match((sp->bnst_data+i)->SM_code)) {
	    assign_cfeature(&((sp->bnst_data+i)->f), "����");
	}
    }

    /* FEATURE��Ϳ�����ξ�� */

    if (OptAnalysis == OPT_AssignF) return TRUE;

    assign_dpnd_rule(sp);			/* ���������§ */

    if (OptAnalysis == OPT_CASE ||
	OptAnalysis == OPT_CASE2) {

	/* �ʲ��Ϥ�Ԥ�����̾���ޤ�ʸ��� feature ��Ϳ����
	   ʣ��̾���Ф餷�Ƴ����ǤȤ���ǧ������ */
	MakeInternalBnst(sp);

	/* ���줾����Ѹ��γʥե졼������ */
	set_pred_caseframe(sp);
    }

    if (OptDisplay == OPT_DETAIL || OptDisplay == OPT_DEBUG)
	check_bnst(sp);

    /**************/
    /* �ܳ�Ū���� */
    /**************/

    if (OptInput == OPT_PARSED) {
	dpnd_info_to_bnst(sp, &(sp->Best_mgr->dpnd)); 
	para_recovery(sp);
	after_decide_dpnd(sp);
	goto PARSED;
    }

    calc_dpnd_matrix(sp);			/* ��¸��ǽ���׻� */
    if (OptDisplay == OPT_DEBUG) print_matrix(sp, PRINT_DPND, 0);

    /* �Ʊ�ɽ���ν��� */

    if (koou(sp) == TRUE && OptDisplay == OPT_DEBUG)
	print_matrix(sp, PRINT_DPND, 0);

	/* ����̤ν��� */

    if ((flag = quote(sp)) == TRUE && OptDisplay == OPT_DEBUG)
	print_matrix(sp, PRINT_QUOTE, 0);

    if (flag == CONTINUE) return FALSE;

    /* ��������ط����ʤ������д� */
	
    if (relax_dpnd_matrix(sp) == TRUE && OptDisplay == OPT_DEBUG) {
	fprintf(Outfp, "Relaxation ... \n");
	print_matrix(sp, PRINT_DPND, 0);
    }

    /****************/
    /* ����¤���� */
    /****************/

    init_mask_matrix(sp);
    sp->Para_num = 0;
    sp->Para_M_num = 0;
    relation_error = 0;
    d_struct_error = 0;
    Revised_para_num = -1;

    if ((flag = check_para_key(sp)) > 0) {
	calc_match_matrix(sp);		/* ʸ�������ٷ׻� */
	detect_all_para_scope(sp);    	/* ����¤���� */
	do {
	    assign_para_similarity_feature(sp);
	    if (OptDisplay == OPT_DETAIL || OptDisplay == OPT_DEBUG) {
		print_matrix(sp, PRINT_PARA, 0);
		/*
		  print_matrix2ps(sp, PRINT_PARA, 0);
		  exit(0);
		*/
	    }
	    /* ����¤�֤νŤʤ���� */
	    if (detect_para_relation(sp) == FALSE) {
		relation_error++;
		continue;
	    }
	    if (OptDisplay == OPT_DEBUG) print_para_relation(sp);
	    /* ����¤��ΰ�¸��¤�����å� */
	    if (check_dpnd_in_para(sp) == FALSE) {
		d_struct_error++;
		continue;
	    }
	    if (OptDisplay == OPT_DEBUG) print_matrix(sp, PRINT_MASK, 0);
	    goto ParaOK;		/* ����¤�������� */
	} while (relation_error <= 3 &&
		 d_struct_error <= 3 &&
		 detect_para_scope(sp, Revised_para_num, TRUE) == TRUE);
	ErrorComment = strdup("Cannot detect consistent CS scopes");
	init_mask_matrix(sp);
    ParaOK:
    }
    else if (flag == CONTINUE)
	return FALSE;

    /********************/
    /* ��¸���ʹ�¤���� */
    /********************/

    para_postprocess(sp);	/* ��conjunct��head������η������ */

    signal(SIGALRM, timeout_function);
    alarm(ParseTimeout);
    if (detect_dpnd_case_struct(sp) == FALSE) {
	ErrorComment = strdup("Cannot detect dependency structure");
	when_no_dpnd_struct(sp);	/* ���������¤����ޤ�ʤ����
					   ���٤�ʸ�᤬�٤˷���Ȱ��� */
    }
    alarm(0);

PARSED:

    /* ������������ bnst ��¤�Τ˵��� */
    dpnd_info_to_bnst(sp, &(sp->Best_mgr->dpnd)); 
    para_recovery(sp);

	/* ��ͭ̾��ǧ������ */

    if (OptNE != OPT_NORMAL)
	NE_analysis(sp);
    else
	assign_mrph_feature(CNRuleArray, CurCNRuleSize,
			    sp->mrph_data, sp->Mrph_num,
			    RLOOP_RMM, FALSE, LtoR);

    memo_by_program(sp);	/* ���ؤν񤭹��� */

    /* ǧ��������ͭ̾�����¸���Ƥ��� */
    if (OptNE != OPT_NORMAL) {
	preserveNE(sp);
	if (OptDisplay == OPT_DEBUG)
	    printNE();
    }

    return TRUE;
}

/*==================================================================*/
			   void knp_main()
/*==================================================================*/
{
    int i, success = 1, flag;

    SENTENCE_DATA *sp = &current_sentence_data;

    /* �ʲ��Ϥν��� */
    init_cf2(sp);
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
	    when_no_dpnd_struct(sp);
	    dpnd_info_to_bnst(sp, &(sp->Best_mgr->dpnd));
	    print_result(sp);
	    fflush(Outfp);
	}

	/* �ʥե졼��ν���� */
	if (OptAnalysis == OPT_CASE || 
	    OptAnalysis == OPT_CASE2) {
	    clear_cf();
	}

	/* ����� */
	if (sp->KNPSID) {
	    free(sp->KNPSID);
	    sp->KNPSID = NULL;
	}
	if (sp->Comment) {
	    free(sp->Comment);
	    sp->Comment = NULL;
	}

	/* FEATURE �ν���� */
	for (i = 0; i < sp->Mrph_num; i++) 
	    clear_feature(&(sp->mrph_data[i].f));
	for (i = 0; i < sp->Bnst_num; i++) {
	    clear_feature(&(sp->bnst_data[i].f));
	    if (sp->bnst_data[i].internal_num) {
		sp->bnst_data[i].internal_num = 0;
		sp->bnst_data[i].internal_max = 0;
		free(sp->bnst_data[i].internal);
	    }
	}
	/* New_Bnst�Ϥ�Ȥ��pointer */
	for (i = sp->Bnst_num; i < sp->Bnst_num + sp->New_Bnst_num; i++)
	    (sp->bnst_data+i)->f = NULL;

	/**************/
	/* �ᥤ����� */
	/**************/

	success = 0;
	if ((flag = main_analysis(sp, Infp)) == EOF) break;
	if (flag == FALSE) continue;

	/************/
	/* ���ɽ�� */
	/************/

	if (OptAnalysis == OPT_BNST) {
	    print_mrphs(sp, 0);
	} else {
	    print_result(sp);
	}
	fflush(Outfp);

	success = 1;	/* OK ���� */
    }

    close_cf();
    close_bgh();
    close_sm();
    close_scase();

    if (OptNE != OPT_NORMAL)
	close_proper();
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
