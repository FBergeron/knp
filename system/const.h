/*====================================================================

			      CONSTANTS

                                               S.Kurohashi 91. 6.25
                                               S.Kurohashi 93. 5.31

    $Id$
====================================================================*/

/*====================================================================
				MACRO
====================================================================*/

#define debug(s, d)    fprintf(stderr, "%s %d\n", s, d)
#define str_eq(c1, c2) ( ! strcmp(c1, c2) )
#define sizeof_char(cp) (sizeof(cp) / sizeof(char *))
#define L_Jiritu_M(ptr)   (ptr->jiritu_ptr + ptr->jiritu_num - 1)

#ifdef _WIN32
#define fprintf sjis_fprintf
#endif 

/*====================================================================
				LENGTH
====================================================================*/
#define	MRPH_MAX	200

#define	BNST_MAX	64
#define	BNST_LENGTH_MAX	256
#define	PARA_MAX	32
#define PARA_PART_MAX	32
#define WORD_LEN_MAX	128
#define SENTENCE_MAX	400
#define PRINT_WIDTH	100
#define PARENT_MAX	20
#define BROTHER_MAX	20
#define TEIDAI_TYPES	5
#define HOMO_MAX	30
#define HOMO_MRPH_MAX	10

#define BGH_CODE_SIZE	10
#define SM_CODE_SIZE	12
#define SCASE_CODE_SIZE	11

#define	HomoRule_MAX	128
#define BonusRule_MAX	16
#define KoouRule_MAX	124
#define DpndRule_MAX	124
#define DpndRule_G_MAX	16
#define ContRule_MAX	256
#define DicForRule_MAX	1024
#define NERule_MAX	512
#define CNRule_MAX	512
#define EtcRule_MAX	1024
#define GeneralRule_MAX	1024

#define IsMrphRule	1
#define IsBnstRule	2
#define IsMrph2Rule	3

#ifndef SMALL
#define ALL_CASE_FRAME_MAX 	1024
#else
#define ALL_CASE_FRAME_MAX 	0
#endif
#define CF_ELEMENT_MAX 		20
#define PP_ELEMENT_MAX		5
#define SM_ELEMENT_MAX		256
#define EX_ELEMENT_MAX		256
#define MAX_MATCH_MAX 		10

#define CMM_MAX 	5				/* ��Ŭ�ʥե졼��� */
#define CPM_MAX 	32				/* ʸ��Ҹ�� */
#define TM_MAX 		5				/* ��Ŭ��¸��¤�� */

#ifndef IMI_MAX
	#define IMI_MAX	129	/* defined in "juman.h" */	
#endif

#define DATA_LEN	5120
#define ALLOCATION_STEP	1024
#define DEFAULT_PARSETIMEOUT	180

#define	TBLSIZE	1024

/*====================================================================
				DEFINE
====================================================================*/
#define OPT_CASE	1
#define OPT_CASE2	6
#define OPT_DPND	2
#define OPT_BNST	3
#define OPT_AssignF	4
#define OPT_DISC	5
#define OPT_RAW		1
#define OPT_PARSED	2
#define OPT_TREE	1
#define OPT_TREEF	2
#define OPT_SEXP	3
#define OPT_TAB		4
#define OPT_NORMAL	1
#define OPT_DETAIL	2
#define OPT_DEBUG	3
#define OPT_NESM	2
#define OPT_NE		3
#define OPT_NE_SIMPLE	4
#define OPT_JUMAN	2
#define OPT_SVM		2

#define OPT_INHIBIT_CLAUSE		0x0001
#define OPT_INHIBIT_C_CLAUSE		0x0002
#define OPT_INHIBIT_OPTIONAL_CASE	0x0010
#define OPT_INHIBIT_CASE_PREDICATE	0x0100
#define OPT_INHIBIT_BARRIER		0x1000

#define	OPT_CASE_SOTO	1
#define	OPT_CASE_GAGA	2

typedef enum {VERBOSE0, VERBOSE1, VERBOSE2, 
	      VERBOSE3, VERBOSE4, VERBOSE5} VerboseType;

#define PARA_KEY_O          0
#define PARA_KEY_N          1	/* �θ������� */
#define PARA_KEY_P          2	/* �Ѹ������� */
#define PARA_KEY_A          4	/* �θ����Ѹ���ʬ����ʤ����� */
#define PARA_KEY_I          3	/* GAP�Τ������� ���� */

#define BNST_RULE_INDP	1
#define BNST_RULE_SUFX	2
#define BNST_RULE_SKIP	3

#define PRINT_PARA	0
#define PRINT_DPND	1
#define PRINT_MASK	2
#define PRINT_QUOTE	3
#define PRINT_RSTR	4
#define PRINT_RSTD	5
#define PRINT_RSTQ	6

#define SEMANTIC_MARKER	1
#define EXAMPLE		2

#define VOICE_SHIEKI 	1
#define VOICE_UKEMI 	2
#define VOICE_MORAU 	3
#define VOICE_UNKNOWN 	4

#define FRAME_ACTIVE		1
#define FRAME_PASSIVE_I		2
#define FRAME_PASSIVE_1		3
#define FRAME_PASSIVE_2		4
#define FRAME_CAUSATIVE_WO_NI	5
#define FRAME_CAUSATIVE_WO	6
#define FRAME_CAUSATIVE_NI	7

#define FRAME_POSSIBLE		8
#define FRAME_POLITE		9
#define FRAME_SPONTANE		10

#define UNASSIGNED	-1
#define NIL_ASSIGNED	-2

#define END_M		-10

#define CONTINUE	-1
#define GUARD		'\n'

#define TYPE_KATAKANA	1
#define TYPE_HIRAGANA	2
#define TYPE_KANJI	3
#define TYPE_SUUJI	4
#define TYPE_EIGO	5
#define TYPE_KIGOU	6

#define SM_NO_EXPAND_NE	1
#define SM_EXPAND_NE	2
#define SM_CHECK_FULL	3
#define	SM_EXPAND_NE_DATA	4

#define RLOOP_MRM	0
#define RLOOP_RMM	1

#define RLOOP_BREAK_NONE	0
#define RLOOP_BREAK_NORMAL	1
#define RLOOP_BREAK_JUMP	2

#define LtoR		1
#define RtoL		-1

/*====================================================================
				  ?
====================================================================*/

#define PARA_NIL 	0
#define PARA_NORMAL 	1	/* <P> */
#define PARA_INCOMP 	2	/* <I> */

#define REL_NOT		0 /* �Ťʤ�ʤ� */
#define REL_BIT 	1 /* �����Ťʤ� */
#define REL_PRE 	2 /* ���ǽŤʤ� */
#define REL_POS 	3 /* ��ǽŤʤ� */
#define REL_PAR 	4 /* ��ʣ 	*/
#define REL_REV 	5 /* �����ν��� */
#define REL_IN1 	6 /* �ޤޤ����	*/
#define REL_IN2 	7 /* �ޤޤ���	*/
#define REL_BAD 	8 /* ��� 	*/

/*====================================================================
		       Client/Server  ư��⡼��
====================================================================*/

#define STAND_ALONE_MODE 0
#define SERVER_MODE      1
#define CLIENT_MODE      2

#define DEFAULT_PORT     31000
#define EOf 0x0b

/*====================================================================
			       FEATURE
====================================================================*/

#define RF_MAX	16

/* FEATURE��¤�� */
typedef struct _FEATURE *FEATUREptr;
typedef struct _FEATURE {
    char	*cp;
    FEATUREptr	next;
} FEATURE;

/* FEATURE�ѥ����� */
typedef struct {
    FEATURE 	*fp[RF_MAX];
} FEATURE_PATTERN;

/*====================================================================
			     ��������ɽ��
====================================================================*/

#define NOT_FLG '^'
#define MAT_FLG NULL
#define AST_FLG '*'
#define QST_FLG '?'
#define NOT_STR "^"
#define AST_STR "*"
#define QST_STR "?"
#define FW_MATCHING 0
#define BW_MATCHING 1
#define ALL_MATCHING 0
#define PART_MATCHING 1
#define SHORT_MATCHING 0
#define LONG_MATCHING 1

#define RM_HINSHI_MAX 64
#define RM_BUNRUI_MAX 64
#define RM_KATA_MAX 64
#define RM_KEI_MAX  64
#define RM_GOI_MAX  64

/* �����ǥѥ����� */
typedef struct {
    char type_flag;	/* '?' or '^' or NULL */
    char ast_flag;	/* '*' or NULL */
    char Hinshi_not;
    int Hinshi[RM_HINSHI_MAX];
    char Bunrui_not;
    int Bunrui[RM_BUNRUI_MAX];
    char Kata_not;
    int Katuyou_Kata[RM_KATA_MAX];
    char Kei_not;
    char *Katuyou_Kei[RM_KEI_MAX];
    char Goi_not;
    char *Goi[RM_GOI_MAX];
    FEATURE_PATTERN f_pattern;
} REGEXPMRPH;

/* ��������ѥ����� */
typedef struct {
    REGEXPMRPH 	*mrph;
    char 	mrphsize;
} REGEXPMRPHS;

/* ʸ��ѥ����� */
typedef struct {
    char 	type_flag;	/* '?' or '^' or NULL */
    char 	ast_flag;	/* '*' or NULL */
    REGEXPMRPHS	*mrphs;
    FEATURE_PATTERN f_pattern;
} REGEXPBNST;

/* ʸ����ѥ����� */
typedef struct {
    REGEXPBNST	*bnst;
    char	bnstsize;
} REGEXPBNSTS;

/*====================================================================
				 ��§
====================================================================*/

#define LOOP_BREAK	0
#define LOOP_ALL	1

/* Ʊ���۵��쵬§ */
typedef struct {
    REGEXPMRPHS *pattern;
    FEATURE	*f;
} HomoRule;

/* ��������§ */
typedef struct {
    REGEXPMRPHS	*pre_pattern;
    REGEXPMRPHS	*self_pattern;
    REGEXPMRPHS	*post_pattern;
    FEATURE	*f;
} MrphRule;

/* ʸ����§ */
typedef struct {
    REGEXPBNSTS	*pre_pattern;
    REGEXPBNSTS	*self_pattern;
    REGEXPBNSTS	*post_pattern;
    FEATURE	*f;
} BnstRule;

/* ���������§ */
typedef struct {
    FEATURE_PATTERN dependant;
    FEATURE_PATTERN governor[DpndRule_G_MAX];
    char	    dpnd_type[DpndRule_G_MAX];
    FEATURE_PATTERN barrier;
    int 	    preference;
    int		    decide;	/* ��դ˷��ꤹ�뤫�ɤ��� */
} DpndRule;

/* �ܡ��ʥ���§ */
typedef struct {
    REGEXPMRPHS *pattern;
    int		type;		/* ����Υ����� */
} BonusRule;

/* �Ʊ���§ */
typedef struct {
    REGEXPMRPHS 	*start_pattern;
    REGEXPMRPHS 	*end_pattern;
    REGEXPMRPHS 	*uke_pattern;
} KoouRule;

#define QUOTE_MAX 40

typedef struct {
    int in_num[QUOTE_MAX];
    int out_num[QUOTE_MAX];
} QUOTE_DATA;

typedef struct {
    char *key;
    FEATUREptr f;
} DicForRule;

/* ��������§, ʸ����§�ν��ޤ�򰷤�����ι�¤�� */
typedef struct {
    void	*RuleArray;
    int		CurRuleSize;
    int		type;
    int		mode;
    int		breakmode;
    int		direction;
} GeneralRuleType;

/* KNP �Υ롼��ե���������� (.jumanrc) */
#ifndef DEF_GRAM_FILE
#define         DEF_GRAM_FILE           "ʸˡ�ե�����"
#endif

#define		DEF_KNP_FILE		"KNP�롼��ե�����"
#define		DEF_KNP_DIR		"KNP�롼��ǥ��쥯�ȥ�"
#define		DEF_KNP_DICT_DIR	"KNP����ǥ��쥯�ȥ�"
#define		DEF_KNP_DICT_FILE	"KNP����ե�����"

typedef struct _RuleVector {
    char	*file;
    int		type;
    int		mode;
    int		breakmode;
    int		direction;
} RuleVector;

#define RuleIncrementStep 10

/* �ɤ߹�����ˡ */
#define MorphRuleType 1
#define BnstRuleType 2
#define HomoRuleType 3
#define DpndRuleType 4
#define KoouRuleType 5
#define NeMorphRuleType 6
#define NePhrasePreRuleType 7
#define NePhraseRuleType 8
#define NePhraseAuxRuleType 9
#define ContextRuleType 10

/* ����κ���� */
#define DICT_MAX	12

/* �������� */
#define	BGH_DB		1
#define	SM_DB		2
#define	SM2CODE_DB	3
#define	SMP2SMG_DB	4
#define	SCASE_DB	5
#define CF_INDEX_DB	6
#define CF_DATA		7
#define	PROPER_DB	8
#define	PROPERC_DB	9
#define	PROPERCASE_DB	10
#define	NOUN_DB		11
#define	CODE2SM_DB	12

/*====================================================================
			     ��ͭ̾�����
====================================================================*/

struct _pos_s {
    int Location;
    int Person;
    int Organization;
    int Artifact;
    int Others;
    char Type[9];
    int Count;
};

typedef struct {
    struct _pos_s XB;
    struct _pos_s AX;
    struct _pos_s AnoX;
    struct _pos_s XnoB;
    struct _pos_s self;
    struct _pos_s selfSM;
    struct _pos_s Case;
} NamedEntity;

/*====================================================================
			      ���ܥǡ���
====================================================================*/

/* �����ǥǡ��� */
typedef struct {
    char 	Goi[WORD_LEN_MAX];	/* ���� */
    char 	Yomi[WORD_LEN_MAX];
    char 	Goi2[WORD_LEN_MAX];
    int  	Hinshi;
    int 	Bunrui;
    int 	Katuyou_Kata;
    int  	Katuyou_Kei;
    char	Imi[IMI_MAX];
    char	type;
    FEATUREptr	f;
    char 	*SM;				/* �ɲ� */
    NamedEntity	NE;				/* �ɲ� */
    NamedEntity	eNE;				/* �ɲ� */
    struct _pos_s	Case[23];		/* �ɲ� */
} MRPH_DATA;

typedef struct cf_def *CF_ptr;
typedef struct cpm_def *CPM_ptr;
/* ʸ��ǡ��� */
typedef struct tnode_b *Treeptr_B;
typedef struct tnode_b {
  /* �ֹ� */
    int 	num;
  /* �����ǥǡ��� */
    int		mrph_num,   settou_num,  jiritu_num,  fuzoku_num;
    MRPH_DATA 	*mrph_ptr,  *settou_ptr, *jiritu_ptr, *fuzoku_ptr;
  /* ��Ω��ǡ��� */
    char 	Jiritu_Go[WORD_LEN_MAX];
  /* ����¤ */
    int 	para_num;	/* �б���������¤�ǡ����ֹ� */
    char   	para_key_type;  /* ̾|��|�� feature���饳�ԡ� */
    char	para_top_p;	/* TRUE -> PARA */
    char	para_type;	/* 0, 1:<P>, 2:<I> */
    				/* ����2�Ĥ�PARA�Ρ��ɤ�Ƴ�����뤿��Τ��
				   dpnd_type�ʤɤȤ���̯�˰ۤʤ� */
    char	to_para_p;	/* ���ԡ� */
    int 	sp_level;	/* ����¤���Ф���Хꥢ */
  /* ��̣���� */
    char 	BGH_code[EX_ELEMENT_MAX*BGH_CODE_SIZE+1];
    int		BGH_num;
    char 	SM_code[SM_ELEMENT_MAX*SM_CODE_SIZE+1];
    int         SM_num;
  /* �Ѹ��ǡ��� */
    char 	SCASE_code[SCASE_CODE_SIZE];	/* ɽ�س� */
    int 	voice;
    int 	cf_num;		/* �����Ѹ����Ф���ʥե졼��ο� */
    CF_ptr 	cf_ptr;		/* �ʥե졼���������(Case_frame_array)
				   �ǤΤ����Ѹ��γʥե졼��ΰ��� */
    CPM_ptr     cpm_ptr;	/* �ʲ��Ϥη�̤��ݻ� */
  /* ����������� (����������女�ԡ�) */
    int		dpnd_head;	/* �������ʸ���ֹ� */
    char 	dpnd_type;	/* ����Υ����� : D, P, I, A */
    int		dpnd_dflt;	/* default�η�����ʸ���ֹ� */
    char 	dpnd_int[32];	/* ʸ�᷸���������ΰ��֡�����̤���� */
    char 	dpnd_ext[32];	/* ¾��ʸ�᷸�����ǽ��������̤���� */
  /* ETC. */
    int  	length;
    int 	space;
  /* �ڹ�¤�ݥ��� */
    Treeptr_B 	parent;
    Treeptr_B 	child[PARA_PART_MAX];

    FEATUREptr	f;
    DpndRule	*dpnd_rule;

    int		internal_num;
    int		internal_max;
    struct tnode_b *internal;
    struct tnode_b *pred_b_ptr;
} BNST_DATA;

/* ����¤�ǡ��� */
typedef struct node_para_manager *Para_M_ptr;
typedef struct tnode_p *Treeptr_P;
typedef struct tnode_p {
    char 	para_char;
    int  	type;
    int  	max_num;
    int         key_pos, iend_pos, jend_pos, max_path[BNST_MAX];
    FEATURE_PATTERN f_pattern;	/* ����ʸ��ξ�� */
    float	max_score;	/* ������κ����� */
    float	pure_score;	/* ����ɽ���Υܡ��ʥ����������,������δ�� */
    char        status;
    Para_M_ptr  manager_ptr;
} PARA_DATA;

typedef struct node_para_manager {
    int 	para_num;
    int 	para_data_num[PARA_PART_MAX];
    int 	part_num;
    int 	start[PARA_PART_MAX];
    int 	end[PARA_PART_MAX];
    Para_M_ptr  parent;
    Para_M_ptr  child[PARA_PART_MAX];
    int 	child_num;
    BNST_DATA	*bnst_ptr;
    char 	status;
} PARA_MANAGER;

typedef struct _check {
    int num;
    int def;
    int pos[BNST_MAX];
} CHECK_DATA;

typedef struct _optionalcase {
    int flag;
    int weight;
    char *type;
    char *data;
    char *candidatesdata;
} CORPUS_DATA;

/* ʸ��γ�ʸ��η�����ʤɤε�Ͽ */
typedef struct {
    int  	head[BNST_MAX];	/* ������ */
    char  	type[BNST_MAX];	/* ���꥿���� */
    int   	dflt[BNST_MAX];	/* ����ε�Υ */
    int 	mask[BNST_MAX];	/* ��򺹾�� */
    int 	pos;		/* ���ߤν������� */
    int         flag;           /* �ƥ�ݥ��ե饰 */
    char        *comment;       /* �ƥ�ݥ�� */
    struct _optionalcase op[BNST_MAX];	/* Ǥ�ճʻ��� */
    CHECK_DATA	check[BNST_MAX];
    FEATURE	*f[BNST_MAX];	/* feature */
} DPND;

/*====================================================================
				�ʲ���
====================================================================*/
#define IPAL_FIELD_NUM	72
#define CASE_MAX_NUM	20

#define	USE_NONE 0
#define USE_BGH	1
#define	USE_NTT	2
#define	STOREtoCF	4
#define	USE_BGH_WITH_STORE	5
#define	USE_NTT_WITH_STORE	6

#define	CF_NORMAL	0
#define	CF_SUM		1	/* OR �γʥե졼�� */
#define	CF_GA_SEMI_SUBJECT	2

#define	CF_UNDECIDED	0
#define	CF_CAND_DECIDED	1
#define	CF_DECIDED	2

#define MATCH_SUBJECT	-1
#define MATCH_NONE	-2

typedef struct {
    int id;				/* ID */
    int yomi;				/* �ɤ� */
    int hyouki;				/* ɽ�� */
    int imi;				/* ��̣ */
    int jyutugoso;			/* �Ҹ��� */
    int kaku_keishiki[CASE_MAX_NUM];	/* �ʷ��� */
    int imisosei[CASE_MAX_NUM];		/* ��̣���� */
    int meishiku[CASE_MAX_NUM];		/* ̾��� */
    int sase;				/* �֣� */
    int rare;				/* �֣� */
    int tyoku_noudou1;			/* �֣� */
    int tyoku_ukemi1;			/* �֣� */
    int tyoku_noudou2;			/* �֣� */
    int tyoku_ukemi2;			/* �֣� */
    int voice;				/* �֣� */
} IPAL_FRAME_INDEX;

typedef struct {
    int id;			/* ID */
    int yomi;			/* �ɤ� */
    int hyouki;			/* ɽ�� */
    int imi;			/* ��̣ */
    int jyutugoso;		/* �Ҹ��� */
    int kaku_keishiki[CASE_MAX_NUM];	/* �ʷ��� */
    int imisosei[CASE_MAX_NUM];		/* ��̣���� */
    int meishiku[CASE_MAX_NUM];		/* ̾��� */
    int sase;			/* �֣� */
    int rare;			/* �֣� */
    int tyoku_noudou1;		/* �֣� */
    int tyoku_ukemi1;		/* �֣� */
    int tyoku_noudou2;		/* �֣� */
    int tyoku_ukemi2;		/* �֣� */
    int voice;			/* �֣� */
    unsigned char *DATA;
} IPAL_FRAME;

/* �ʥե졼�๽¤��
	�� ����ʸ���Ф��ƺ����
	�� �ޤ����ʥե졼�༭��γƥ���ȥ�ˤ�����
		(�֡����פʤɤξ��ϼ���,º�ɤʤɤˤ��줾����)
 */
typedef struct cf_def {
    int 	element_num;				/* �����ǿ� */
    int 	oblig[CF_ELEMENT_MAX]; 			/* ɬ�ܳʤ��ɤ��� */
    int 	adjacent[CF_ELEMENT_MAX];		/* ľ���ʤ��ɤ��� */
    int 	pp[CF_ELEMENT_MAX][PP_ELEMENT_MAX]; 	/* �ʽ��� */
    char	*sm[CF_ELEMENT_MAX]; 			/* ��̣�ޡ��� */
    char	*sm_delete[CF_ELEMENT_MAX];		/* ���Ѷػ߰�̣�ޡ��� */
    int		sm_delete_size[CF_ELEMENT_MAX];
    int		sm_delete_num[CF_ELEMENT_MAX];
    char 	*ex[CF_ELEMENT_MAX];			/* ���� (BGH) */
    char	*ex2[CF_ELEMENT_MAX];			/* ���� (NTT) */
    char	**ex_list[CF_ELEMENT_MAX];
    int		ex_size[CF_ELEMENT_MAX];
    int		ex_num[CF_ELEMENT_MAX];
    char	*examples[CF_ELEMENT_MAX];
    char	*semantics[CF_ELEMENT_MAX];
    int 	voice;					/* �������� */
    int 	ipal_address;				/* IPAL�Υ��ɥ쥹 */
    int 	ipal_size;				/* IPAL�Υ����� */
    char 	ipal_id[128];				/* IPAL��ID */
    char 	*entry;					/* �Ѹ���ɽ�� */
    char 	imi[128];
    char	concatenated_flag;			/* ɽ�������٤�ʸ��ȷ�礷�Ƥ��뤫 */
    int		etcflag;				/* �ʥե졼�ब OR ���ɤ��� */
    int		weight[CF_ELEMENT_MAX];
    BNST_DATA	*pred_b_ptr;
} CASE_FRAME;

/* ʸ��γ����Ǥȳʥե졼��Υ���åȤȤ��б��դ���Ͽ */
typedef struct {
    int  	flag[CF_ELEMENT_MAX];
    int		score[CF_ELEMENT_MAX];
    int		pos[CF_ELEMENT_MAX];
} LIST;

/* ʸ�ȳʥե졼����б��դ���̤ε�Ͽ */
typedef struct {
    CASE_FRAME 	*cf_ptr;			/* �ʥե졼��ؤΥݥ��� */
    float 	score;				/* ������ */
    int		pure_score[MAX_MATCH_MAX];	/* �������������Υ����� */
    float	sufficiency;			/* �ʥե졼�����ޤꤰ���� */
    int 	result_num;			/* ���������б��ط��� */
    LIST	result_lists_p[MAX_MATCH_MAX]; 	/* ������������б��ط�
						   (Ʊ���ξ���ʣ��) */
    LIST	result_lists_d[MAX_MATCH_MAX];

    struct cpm_def	*cpm;
} CF_MATCH_MGR;

/* ʸ��(�Ѹ����Ф���ʣ���β�ǽ��)�ʥե졼����б��դ���̤ε�Ͽ */
typedef struct cpm_def {
    CASE_FRAME 	cf;				/* ����ʸ�γʹ�¤ */
    BNST_DATA	*pred_b_ptr;			/* ����ʸ���Ѹ�ʸ�� */
    BNST_DATA	*elem_b_ptr[CF_ELEMENT_MAX];	/* ����ʸ�γ�����ʸ�� */
    int 	elem_b_num[CF_ELEMENT_MAX];	/* ����ʸ�γ�����ʸ��(Ϣ�ʤη������-1,¾�ϻҤν���) */
    int 	score;				/* ������������(=cmm[0].score) */
    int 	default_score;			/* ���δط������� (�롼���Ϳ�����Ƥ��볰�δط�) */
    int 	result_num;			/* ��������ʥե졼��� */
    int		tie_num;
    CF_MATCH_MGR cmm[CMM_MAX];			/* ����������γʥե졼��Ȥ�
						   �б��դ���Ͽ
						   (Ʊ���ξ���ʣ��) */
    int		decided;
} CF_PRED_MGR;

/* ��ʸ�β��Ϸ�̤�����Ͽ */
typedef struct {
    DPND 	dpnd;		/* ��¸��¤ */
    int		pssb;		/* ��¸��¤�β�ǽ���β����ܤ� */
    int		dflt;		/* �� */
    int 	score;		/* ������ */
    int 	pred_num;	/* ʸ����Ѹ��� */
    CF_PRED_MGR cpm[CPM_MAX];	/* ʸ��γ��Ѹ��γʲ��Ϸ�� */
    int		ID;		/* DPND �� ID */
} TOTAL_MGR;

#define	OPTIONAL_CASE_SCORE	2

/*====================================================================
		      ��ͭ̾����� - ʸ̮������
====================================================================*/

/* �ݻ����Ƥ�������Υǡ��� */

typedef struct _MRPH_P {
    MRPH_DATA data;
    struct _MRPH_P *next;
} MRPH_P;

typedef struct _PreservedNamedEntity {
    MRPH_P *mrph;
    int Type;
    struct _PreservedNamedEntity *next;
} PreservedNamedEntity;

/*====================================================================
			       ʸ̮����
====================================================================*/

typedef struct sentence {
    int 		Sen_num;	/* ʸ�ֹ� 1�� */
    int			Mrph_num;
    int			Bnst_num;
    int			New_Bnst_num;
    int			Para_M_num;	/* ��������ޥ͡������ */
    int			Para_num;	/* ����¤�� */
    MRPH_DATA		*mrph_data;
    BNST_DATA	 	*bnst_data;
    PARA_DATA		*para_data;
    PARA_MANAGER	*para_manager;
    CF_PRED_MGR		*cpm;
    CASE_FRAME		*cf;
    TOTAL_MGR		*Best_mgr;
    char		*KNPSID;
    char		*Comment;
} SENTENCE_DATA;

typedef struct anaphora_list {
    char	*key;
    int		count;
    struct anaphora_list *next;
} ALIST;

#define	CREL	1	/* �ʴط� */
#define	EREL	2	/* ��ά�ط� */

typedef struct case_component {
    char	*word;
    int		count;
    int		flag;
    struct case_component *next;
} CASE_COMPONENT;

/* �Ѹ��ȳ����Ǥ��Ȥι�¤�� */
typedef struct predicate_anaphora_list {
    char	*key;		/* �Ѹ� */
    int		voice;
    int		cf_addr;
    CASE_COMPONENT *cc[CASE_MAX_NUM];	/* �����ǤΥꥹ�� */
    struct predicate_anaphora_list *next;
} PALIST;

typedef struct ellipsis_component {
    SENTENCE_DATA	*s;
    int			bnst;
    float		score;
} ELLIPSIS_COMPONENT;

typedef struct ellipsis_cmm_list {
    CF_MATCH_MGR	cmm;
    CF_PRED_MGR		cpm;
    int			element_num;			/* ����¦ */
} ELLIPSIS_CMM;

typedef struct ellipsis_list {
    CF_PRED_MGR		*cpm;
    float		score;
    ELLIPSIS_COMPONENT cc[CASE_MAX_NUM];	/* ��ά�����ǤΥꥹ�� */
    FEATUREptr		f;
    int			result_num;
    ELLIPSIS_CMM	ecmm[CMM_MAX];
} ELLIPSIS_MGR;

/*====================================================================
                               END
====================================================================*/
