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
#define T_CHILD_MAX	20
#define BROTHER_MAX	20
#define TEIDAI_TYPES	5

#define BGH_CODE_SIZE	10
#define SM_CODE_SIZE	12
#define SM_CODE_MAX	100

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
#define ALL_CASE_FRAME_MAX 	1536
#define IPAL_FRAME_MAX 		512
#else
#define ALL_CASE_FRAME_MAX 	0
#define IPAL_FRAME_MAX 		0
#endif
#define CF_ELEMENT_MAX 		10
#define PP_ELEMENT_MAX		5
#define SM_ELEMENT_MAX		64			/* SM_CODE_MAX�ȤޤȤ��٤� */
#define EX_ELEMENT_MAX		64
#define MAX_MATCH_MAX 		10

#define CMM_MAX 	(IPAL_FRAME_MAX * 5)		/* ��Ŭ�ʥե졼��� */
#define CPM_MAX 	32				/* ʸ��Ҹ�� */
#define TM_MAX 		5				/* ��Ŭ��¸��¤�� */

#ifndef IMI_MAX
	#define IMI_MAX	129	/* defined in "juman.h" */	
#endif

#define DATA_LEN	5120
#define ALLOCATION_STEP	1024
#define DEFAULT_PARSETIMEOUT	180

/*====================================================================
				DEFINE
====================================================================*/
#define OPT_CASE	1
#define OPT_CASE2	6
#define OPT_DPND	2
#define OPT_BNST	3
#define OPT_PM		4
#define OPT_DISC	5
#define OPT_TREE	1
#define OPT_TREEF	2
#define OPT_SEXP	3
#define OPT_TAB		4
#define OPT_NORMAL	1
#define OPT_DETAIL	2
#define OPT_DEBUG	3
#define OPT_NESM	2
#define OPT_NE		3

#define OPT_INHIBIT_CLAUSE		0x0001
#define OPT_INHIBIT_C_CLAUSE		0x0002
#define OPT_INHIBIT_OPTIONAL_CASE	0x0010
#define OPT_INHIBIT_CASE_PREDICATE	0x0100
#define OPT_INHIBIT_BARRIER		0x1000
#define CORPUS_POSSIBILITY_1	2
#define CORPUS_POSSIBILITY_1_FLAG	'l'

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

#define END_M		-1

#define CONTINUE	-1
#define GUARD		'\n'

#define TYPE_KATAKANA	1
#define TYPE_HIRAGANA	2
#define TYPE_KANJI	3
#define TYPE_SUUJI	4
#define TYPE_EIKIGOU	5

#define SM_NO_EXPAND_NE	1
#define SM_EXPAND_NE	2
#define SM_CHECK_FULL	3

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
    char 	SM[SM_CODE_SIZE*SM_CODE_MAX+1];	/* �ɲ� */
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
    char 	BGH_code[500];
    int		BGH_num;
    char 	SM_code[SM_CODE_SIZE*SM_CODE_MAX+1];
    int         SM_num;
  /* �Ѹ��ǡ��� */
    char 	SCASE_code[11];	/* ɽ�س� */
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
    Treeptr_B 	child[T_CHILD_MAX];

    FEATUREptr	f;
    DpndRule	*dpnd_rule;
} BNST_DATA;

/* ����¤�ǡ��� */
typedef struct node_para_manager *Para_M_ptr;
typedef struct tnode_p *Treeptr_P;
typedef struct tnode_p {
    char 	para_char;
    int  	type;
    int  	max_num;
    int         L_B, R, max_path[BNST_MAX];
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

typedef struct sentence {
    int 		Sen_num;	/* ʸ�ֹ� 1�� */
    int			Mrph_num;
    int			Bnst_num;
    int			New_Bnst_num;
    MRPH_DATA		*mrph_data;
    BNST_DATA	 	*bnst_data;
    PARA_DATA		*para_data;
    PARA_MANAGER	*para_manager;
    struct sentence	*next;
} SENTENCE_DATA;

typedef struct _check {
    int num;
    int def;
    int pos[BNST_MAX];
} CHECK_DATA;

struct _optionalcase {
    int flag;
    int weight;
    char *type;
};

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
#define IPAL_FIELD_NUM	27	/* 65 */
#define IPAL_DATA_SIZE	1026	/* 1380 */

typedef struct {
    int point[IPAL_FIELD_NUM];
    unsigned char DATA[IPAL_DATA_SIZE];
} IPAL_TRANS_FRAME;

typedef struct {
    int id;			/* ID */
    int yomi;			/* �ɤ� */
    int hyouki;			/* ɽ�� */
    int imi;			/* ��̣ */
    int jyutugoso;		/* �Ҹ��� */
    int kaku_keishiki[5];	/* �ʷ��� */
    int imisosei[5];		/* ��̣���� */
    int meishiku[5];		/* ̾��� */
    int sase;			/* �֣� */
    int rare;			/* �֣� */
    int tyoku_noudou1;		/* �֣� */
    int tyoku_ukemi1;		/* �֣� */
    int tyoku_noudou2;		/* �֣� */
    int tyoku_ukemi2;		/* �֣� */
    int voice;			/* �֣� */
    unsigned char DATA[IPAL_DATA_SIZE];
} IPAL_FRAME;

/* �ʥե졼�๽¤��
	�� ����ʸ���Ф��ƺ����
	�� �ޤ����ʥե졼�༭��γƥ���ȥ�ˤ�����
		(�֡����פʤɤξ��ϼ���,º�ɤʤɤˤ��줾����)
 */
typedef struct cf_def {
    int 	element_num;				/* �����ǿ� */
    int 	oblig[CF_ELEMENT_MAX]; 			/* ɬ�ܳʤ��ɤ��� */
    int 	pp[CF_ELEMENT_MAX][PP_ELEMENT_MAX]; 	/* �ʽ��� */
    char	sm[CF_ELEMENT_MAX][SM_ELEMENT_MAX*SM_CODE_SIZE]; 	
							/* ��̣�ޡ��� */
    int         sm_flag[CF_ELEMENT_MAX][SM_ELEMENT_MAX];/* ��̣�ޡ����Υե饰 */
    char 	ex[CF_ELEMENT_MAX][EX_ELEMENT_MAX*10];	/* �� */
    int 	voice;					/* �������� */
    int 	ipal_address;				/* IPAL�Υ��ɥ쥹 */
    char 	ipal_id[128];				/* IPAL��ID */
    char 	imi[128];
} CASE_FRAME;

/* ʸ��γ����Ǥȳʥե졼��Υ���åȤȤ��б��դ���Ͽ */
typedef struct {
    int  	flag[CF_ELEMENT_MAX];
} LIST;

/* ʸ�ȳʥե졼����б��դ���̤ε�Ͽ */
typedef struct {
    CASE_FRAME 	*cf_ptr;			/* �ʥե졼��ؤΥݥ��� */
    int 	score;				/* ������ */
    int 	result_num;			/* ���������б��ط��� */
    LIST	result_lists_p[MAX_MATCH_MAX]; 	/* ������������б��ط�
						   (Ʊ���ξ���ʣ��) */
} CF_MATCH_MGR;

/* ʸ��(�Ѹ����Ф���ʣ���β�ǽ��)�ʥե졼����б��դ���̤ε�Ͽ */
typedef struct cpm_def {
    CASE_FRAME 	cf;				/* ����ʸ�γʹ�¤ */
    BNST_DATA	*pred_b_ptr;			/* ����ʸ���Ѹ�ʸ�� */
    BNST_DATA	*elem_b_ptr[CF_ELEMENT_MAX];	/* ����ʸ�γ�����ʸ�� */
    int 	elem_b_num[CF_ELEMENT_MAX];	/* �� */
    int 	score;				/* ������������(=cmm[0].score) */
    int 	result_num;			/* ��������ʥե졼��� */
    CF_MATCH_MGR cmm[CMM_MAX];			/* ����������γʥե졼��Ȥ�
						   �б��դ���Ͽ
						   (Ʊ���ξ���ʣ��) */
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
                               END
====================================================================*/
