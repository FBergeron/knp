/*====================================================================

			     �����ѥ���Ϣ

                                             S.Kurohashi 1999. 4. 1

    $Id$
====================================================================*/
#include "knp.h"

static char buffer[DATA_LEN];
char CorpusComment[BNST_MAX][DATA_LEN];
static DBM_FILE c_db, cc_db, op_db, op_sm_db, cp_db, c_temp_db;

extern char *ClauseDBname;
extern char *ClauseCDBname;
extern char *CasePredicateDBname;
extern char *OptionalCaseDBname;

DBM_FILE db_read_open(char *filename);
char *db_get(DBM_FILE db, char *buf);
void DB_close(DBM_FILE db);

typedef struct {
    int Value[BNST_MAX][BNST_MAX];
    char *Type1[BNST_MAX];
    char *Type2[BNST_MAX];
} EtcMatrix;

EtcMatrix BarrierMatrix;

typedef struct {
    char *sm;
    int frequency;
} SMwithFrequency;

#define SM_ALLOCATION_STEP 100

typedef struct {
    char *relation;
    SMwithFrequency *list;
    int num;
    int maxnum;
} _SMCaseFrame;

#define CASE_ALLOCATION_STEP 10

typedef struct {
    _SMCaseFrame *frame;
    int num;
    int maxnum;
    char *predicate;
} SMCaseFrame;

SMCaseFrame smcf;

/* DB �� search �������ͤȤ����֤��ؿ� */
int dbfetch_num(DBM_FILE db, char *buf)
{
    int count = 0;
    char *cp;

    cp = db_get(db, buf);

    if (cp) {
	if (strlen(cp) >= DATA_LEN) {
	    fprintf(stderr, "dbfetch_num: content length overflow.\n");
	    exit(1);
	}

	strcpy(buffer, cp);
	free(cp);
	count = atoi(buffer);
    }
    return count;
}

#define CLAUSE_TEMP_DB_NAME KNP_DICT "/clause/clause-strength.gdbm"

int init_clause()
{
    int i;
    buffer[DATA_LEN-1] = '\n';
    for (i = 0; i < BNST_MAX; i++)
	CorpusComment[i][DATA_LEN-1] = '\n';

    if (!(OptInhibit & OPT_INHIBIT_C_CLAUSE)) {
	if (ClauseCDBname)
	    cc_db = db_read_open(ClauseCDBname);
	else
	    cc_db = db_read_open(CLAUSE_CDB_NAME);
    }
    if (ClauseDBname)
	c_db = db_read_open(ClauseDBname);
    else
	c_db = db_read_open(CLAUSE_DB_NAME);

    /* c_temp_db = db_read_open(CLAUSE_TEMP_DB_NAME); */

    return TRUE;
}

void close_clause()
{
    DB_close(c_db);
    /* db_close(c_temp_db); */
    if (!(OptInhibit & OPT_INHIBIT_C_CLAUSE))
	DB_close(cc_db);
}

/* �Ҹ���֤η���������٤�Ĵ�٤�ؿ� */
int corpus_clause_comp(BNST_DATA *ptr1, BNST_DATA *ptr2, int para_flag)
{
    char *type1, *type2, *cp, *token, *type, *level1, *level2, *sparse;
    char parallel1, parallel2, touten1, touten2, touten;
    int score, offset, i, score2;
    BNST_DATA *bnst_data;

    bnst_data = ptr1-ptr1->num;

    /* para_flag == TRUE  : ������θ (������Ϥޤ��θƤӽФ��Ǥϰ�̣���ʤ�)
       para_flag == FALSE : �����̵�� */

    /* ����� */
    parallel1 = ' ';
    parallel2 = ' ';
    touten1 = ' ';
    touten2 = ' ';

    /* ��������� */
    if (para_flag == TRUE) {
	if (ptr1->para_key_type != PARA_KEY_O)
	    parallel1 = 'P';
	/* if (ptr2->para_key_type != PARA_KEY_O)
	    parallel2 = 'P'; */
    }

    /* �Ҹ쥿���פ����� */
    type1 = (char *)check_feature(ptr1->f, "ID");
    type2 = (char *)check_feature(ptr2->f, "ID");

    /* level ������(ɽ�����Ѥ������) */
    level1 = (char *)check_feature(ptr1->f, "��٥�");
    level2 = (char *)check_feature(ptr2->f, "��٥�");

    if (!type1) return TRUE;
    if (!type2) return FALSE;

    /* ������������򸫤뤿��Υ��ե��å� */
    if (!strcmp(type2+3, "���ʤ����"))
	offset = 1;
    else
	offset = 0;

    /* ���������� */
    if ((char *)check_feature(ptr1->f, "����"))
	touten1 = ',';
    if ((char *)check_feature((ptr2+offset)->f, "����"))
	touten2 = ',';

    /* �ǡ����١����θ��� */
    sprintf(buffer, "%s%c%c %s%c%c", 
	    type1+3, parallel1, touten1, 
	    type2+3, parallel2, touten2);
    if (buffer[DATA_LEN-1] != '\n') {
	fprintf(stderr, "corpus_clause_comp: data length overflow.\n");
	exit(1);
    }
    score = dbfetch_num(c_db, buffer);

    if (!(OptInhibit & OPT_INHIBIT_C_CLAUSE) && (para_flag == TRUE) && 
	parallel1 == ' ') {
	cp = db_get(cc_db, buffer);
	token = strtok(cp, "|");
	while (token) {
	    for (i = ptr1->num+1; i < ptr2->num; i++) {
		type = (char *)check_feature((bnst_data+i)->f, "ID");
		if (type) {
		    if ((char *)check_feature((bnst_data+i)->f, "����"))
			touten = ',';
		    else
			touten = ' ';
		    sprintf(buffer, "%s %c", type+3, touten);
		    if (!strcmp(token, buffer)) {
			fprintf(Outfp, ";;;(D) %2d %2d %s\n", ptr1->num, i, buffer);
			Dpnd_matrix[ptr1->num][i] = 0;
		    }
		}
	    }
	    token = strtok(NULL, "|");
	}
    }
    /* free(cp); */

    sprintf(buffer, "!%s%c%c %s%c%c", 
	    type1+3, parallel1, touten1, 
	    type2+3, parallel2, touten2);
    if (buffer[DATA_LEN-1] != '\n') {
	fprintf(stderr, "corpus_clause_comp: data length overflow.\n");
	exit(1);
    }
    score2 = dbfetch_num(c_db, buffer);

    /* �ǥХå����� */
    if (para_flag == TRUE) {
	if (level1 && level2) {
	    fprintf(Outfp, ";;; %2d %2d %s%c%c(%s) %s%c%c(%s) -> %d (%d)\n", 
		    ptr1->num, ptr2->num, type1+3, parallel1, touten1, level1+7, 
		    type2+3, parallel2, touten2, level2+7, score, score2);
	}
	else {
	    fprintf(Outfp, ";;; %2d %2d %s%c%c %s%c%c -> %d (%d)\n", 
		    ptr1->num, ptr2->num, type1+3, parallel1, touten1, 
		    type2+3, parallel2, touten2, score, score2);
	}
    }
    else {
	fprintf(Outfp, ";;;(R) %s %c %s %c->%d (%d)\n", type1+3, touten1, 
		type2+3, touten2, score, score2);
    }

    /* ���٤����뤫���䴰����Ƥ���Ȥ� */
    if (score > 0 || score == -2) {
	return TRUE;
    }
    else {
	/* ����ΤȤ��ǥ��������ʤ��ʤ�����¤��������̵��
	if (parallel1 == 'P')
	    return corpus_clause_comp((BNST_DATA *)ptr1, 
				      (BNST_DATA *)ptr2, 
				      FALSE);
	else */

	if (!score && !score2) {
	    if ((sparse = (char *)check_feature(ptr1->f, "SPARSE")))
		sprintf(buffer, "%s:%d%c", sparse, ptr2->num, parallel1);
	    else
		sprintf(buffer, "SPARSE:%d%c", ptr2->num, parallel1);
	    assign_cfeature(&(ptr1->f), buffer);
	}
	return FALSE;
    }
}

int init_case_pred()
{
    int i, j;
    buffer[DATA_LEN-1] = '\n';
    for (i = 0; i < BNST_MAX; i++) {
	BarrierMatrix.Type1[i] = NULL;
	BarrierMatrix.Type2[i] = NULL;
	CorpusComment[i][DATA_LEN-1] = '\n';
	for (j = i+1; j < BNST_MAX; j++) {
	    BarrierMatrix.Value[i][j] = -1;
	}
    }

    if (CasePredicateDBname)
	cp_db = db_read_open(CasePredicateDBname);
    else
	cp_db = db_read_open(CASE_PRED_DB_NAME);
    return TRUE;
}

void close_case_pred()
{
    DB_close(cp_db);
}

/* ��Ω�� : ʸ�� feature ������ */
int check_feature_for_optional_case(FEATURE *f)
{
    if ((char *)check_feature(f, "�ؼ���") || 
	(char *)check_feature(f, "����̾��"))
	return TRUE;
    return FALSE;
}

/* ��Ω�� : ��Ω������� */
int check_JiritsuGo_for_optional_case(char *cp)
{
     if (!strcmp(cp, "�ʤ�") || 
	 !strcmp(cp, "�ʤ�") || 
	 !strcmp(cp, "����") ||
	 !strcmp(cp, "����")) {
	return TRUE;
    }
    return FALSE;
}

/* ��Ω�� : �����Ǥ����� */
int check_Morph_for_optional_case(MRPH_DATA *m)
{
    /* ����Ū̾�� */
    if (m->Hinshi == 6 && m->Bunrui == 9)
	return TRUE;
    /* ����̾�� */
    else if (m->Hinshi == 6 && m->Bunrui == 8)
	return TRUE;
    return FALSE;
}

/* �ʤ���Ҹ�ؤη���������٤�Ĵ�٤�ؿ� */
int corpus_case_predicate_check(BNST_DATA *ptr1, BNST_DATA *ptr2)
{
    char *type1, *type2, *level2;
    char parallel1, touten1, touten2;
    int score, scorep, offset, para_flag = 0;

    /* ����� */
    parallel1 = ' ';
    touten1 = ' ';
    touten2 = ' ';

    /* ��������� */
    if (ptr1->para_key_type != PARA_KEY_O)
	parallel1 = 'P';

    /* �ʤȽҸ쥿���פ����� */
    type1 = (char *)check_feature(ptr1->f, "��");
    type2 = (char *)check_feature(ptr2->f, "ID");

    /* level ������(ɽ�����Ѥ������) */
    level2 = (char *)check_feature(ptr2->f, "��٥�");

    if (!type1) return FALSE;
    if (!type2) return FALSE;

    /* ������������򸫤뤿��Υ��ե��å� */
    if (!strcmp(type2+3, "���ʤ����"))
	offset = 1;
    else
	offset = 0;

    /* ���������� */
    if ((char *)check_feature(ptr1->f, "����"))
	touten1 = ',';
    if ((char *)check_feature((ptr2+offset)->f, "����"))
	touten2 = ',';

    /* �ǡ����١����θ���(�ۤ�����) */
    sprintf(buffer, "!%s%c%c %s %c", type1+3, parallel1, touten1, type2+3, touten2);
    if (buffer[DATA_LEN-1] != '\n') {
	fprintf(stderr, "corpus_case_predicate_check: data length overflow.\n");
	exit(1);
    }
    score = dbfetch_num(cp_db, buffer);

    /* ������θ���ʤ��Ȥ� */
    if (!para_flag) {
	if (parallel1 == 'P')
	    sprintf(buffer, "!%s %c %s %c", type1+3, touten1, type2+3, touten2);
	else
	    sprintf(buffer, "!%sP%c %s %c", type1+3, touten1, type2+3, touten2);
	scorep = dbfetch_num(cp_db, buffer);
	score += scorep;
    }

#ifdef DEBUGMORE
    fprintf(Outfp, ";;;(K) %2d %2d %s%c%c %s %c->!%d", ptr1->num, ptr2->num, type1+3, parallel1, touten1, type2+3, touten2, score);
#endif

    /* �ǡ����١����θ���(������) */
    sprintf(buffer, "%s%c%c %s %c", type1+3, parallel1, touten1, type2+3, touten2);
    if (buffer[DATA_LEN-1] != '\n') {
	fprintf(stderr, "corpus_case_predicate_check: data length overflow.\n");
	exit(1);
    }
    score = dbfetch_num(cp_db, buffer);

    /* ������θ���ʤ��Ȥ� */
    if (!para_flag) {
	if (parallel1 == 'P')
	    sprintf(buffer, "%s %c %s %c", type1+3, touten1, type2+3, touten2);
	else
	    sprintf(buffer, "%sP%c %s %c", type1+3, touten1, type2+3, touten2);
	scorep = dbfetch_num(cp_db, buffer);
	score += scorep;
    }

#ifdef DEBUGMORE
    fprintf(Outfp, "  %d\n", score);
#endif

    if (score)
	return TRUE;
    else
	return FALSE;
}

/* ��֤ΥХꥢ��Ĵ�٤�ؿ� */
int corpus_clause_barrier_check(BNST_DATA *ptr1, BNST_DATA *ptr2)
{
    char *type1, *type2;
    char parallel1, touten1, touten2;
    int score, scorep, offset, para_flag = 0;
    int pos1, pos2;

    /* ʸ���ֹ� */
    pos1 = ptr1->num;
    pos2 = ptr2->num;

    /* ����� */
    parallel1 = ' ';
    touten1 = ' ';
    touten2 = ' ';

    /* ��������� */
    if (ptr1->para_key_type != PARA_KEY_O)
	/* if (ptr1->dpnd_type == 'P') */
	parallel1 = 'P';

    /* �ʤȽҸ쥿���פ����� */
    type1 = (char *)check_feature(ptr1->f, "ID");
    type2 = (char *)check_feature(ptr2->f, "ID");

    if (!type1) return FALSE;
    if (!type2) return FALSE;

    /* ������������򸫤뤿��Υ��ե��å� */
    if (!strcmp(type2+3, "���ʤ����"))
	offset = 1;
    else
	offset = 0;

    /* ���������� */
    if ((char *)check_feature(ptr1->f, "����"))
	touten1 = ',';
    if ((char *)check_feature((ptr2+offset)->f, "����"))
	touten2 = ',';

    /* �ǡ����١����θ���(�ۤ�����) */
    sprintf(buffer, "!%s %c %s %c", type1+3, touten1, type2+3, touten2);
    if (buffer[DATA_LEN-1] != '\n') {
	fprintf(stderr, "corpus_barrier_check: data length overflow.\n");
	exit(1);
    }
    score = dbfetch_num(c_db, buffer);

    /* ������θ���ʤ��Ȥ� */
    if (!para_flag) {
	if (parallel1 == 'P')
	    sprintf(buffer, "!%s %c %s %c", type1+3, touten1, type2+3, touten2);
	else
	    sprintf(buffer, "!%sP%c %s %c", type1+3, touten1, type2+3, touten2);
	scorep = dbfetch_num(c_db, buffer);
	score += scorep;
    }

    /* �ۤ������Ȥ�����ȥХꥢ�ǤϤʤ� */
    if (score) {
#ifdef DEBUG
	/* �ǥХå����� */
	fprintf(Outfp, ";;; (C��) %2d %2d %s %c %s %c-> !%d\n", pos1, pos2, type1+3, touten1, type2+3, touten2, score);
#endif
	return FALSE;
    }
#ifdef DEBUG
    else
	/* �ǥХå����� */
	fprintf(Outfp, ";;; (C��) %2d %2d %s %c %s %c-> !%d", pos1, pos2, type1+3, touten1, type2+3, touten2, score);
#endif

    /* �ǡ����١����θ��� (������) */
    sprintf(buffer, "%s%c%c %s %c", type1+3, parallel1, touten1, type2+3, touten2);
    if (buffer[DATA_LEN-1] != '\n') {
	fprintf(stderr, "corpus_barrier_check: data length overflow.\n");
	exit(1);
    }
    score = dbfetch_num(c_db, buffer);

    /* �䴰���줿�Ȥ��ȡ���ϣ�ˤ�������줿�Ȥ����ɤȤ��ʤ����Ȥˤ��� */

    if (score >= 0) {
	/* ������θ���ʤ��Ȥ� */
	if (!para_flag) {
	    if (parallel1 == 'P')
		sprintf(buffer, "%s %c %s %c", type1+3, touten1, type2+3, touten2);
	    else
		sprintf(buffer, "%sP%c %s %c", type1+3, touten1, type2+3, touten2);
	    scorep = dbfetch_num(c_db, buffer);
	    score += scorep;
	}
    }

#ifdef DEBUG
    /* �ǥХå����� */
    fprintf(Outfp, "  %d\n", score);
#endif

    /* ���ȤϷ��ä����Ȥ�����Ф褤 */
    if (score > 0)
	return TRUE;
    else
	return FALSE;
}

/* �ʤ���Ҹ���Ф���Хꥢ��Ĵ�٤�ؿ� */
int corpus_barrier_check(BNST_DATA *ptr1, BNST_DATA *ptr2)
{
    char *type1, *type2;
    char parallel1, touten1, touten2;
    int score, scorep, offset, para_flag = 0;
    int pos1, pos2;

    /* ʸ���ֹ� */
    pos1 = ptr1->num;
    pos2 = ptr2->num;

    /* ����� */
    parallel1 = ' ';
    touten1 = ' ';
    touten2 = ' ';

    /* ��������� */
    if (ptr1->para_key_type != PARA_KEY_O)
	/* if (ptr1->dpnd_type == 'P') */
	parallel1 = 'P';

    /* �ʤȽҸ쥿���פ����� */
    type1 = (char *)check_feature(ptr1->f, "��");
    type2 = (char *)check_feature(ptr2->f, "ID");

    if (!type1) return FALSE;
    if (!type2) return FALSE;

    /* ������������򸫤뤿��Υ��ե��å� */
    if (!strcmp(type2+3, "���ʤ����"))
	offset = 1;
    else
	offset = 0;

    /* ���������� */
    if ((char *)check_feature(ptr1->f, "����"))
	touten1 = ',';
    if ((char *)check_feature((ptr2+offset)->f, "����"))
	touten2 = ',';

    /* �ǡ����١����θ���(�ۤ�����) */
    sprintf(buffer, "!%s %c %s %c", type1+3, touten1, type2+3, touten2);
    if (buffer[DATA_LEN-1] != '\n') {
	fprintf(stderr, "corpus_barrier_check: data length overflow.\n");
	exit(1);
    }
    score = dbfetch_num(cp_db, buffer);

    /* ������θ���ʤ��Ȥ� */
    if (!para_flag) {
	if (parallel1 == 'P')
	    sprintf(buffer, "!%s %c %s %c", type1+3, touten1, type2+3, touten2);
	else
	    sprintf(buffer, "!%sP%c %s %c", type1+3, touten1, type2+3, touten2);
	scorep = dbfetch_num(cp_db, buffer);
	score += scorep;
    }

    /* �ۤ������Ȥ�����ȥХꥢ�ǤϤʤ� */
    if (score) {
	/* �ǥХå�����
	fprintf(Outfp, ";;;(B) %2d %2d %s %c %s %c->!%d\n", pos1, pos2, type1+3, touten1, type2+3, touten2, score); */
	return FALSE;
    }
    /* else
	�ǥХå�����
	fprintf(Outfp, ";;;(B) %2d %2d %s %c %s %c->!%d", pos1, pos2, type1+3, touten1, type2+3, touten2, score); */

    /* �ǡ����١����θ���(������) */
    sprintf(buffer, "%s%c%c %s %c", type1+3, parallel1, touten1, type2+3, touten2);
    if (buffer[DATA_LEN-1] != '\n') {
	fprintf(stderr, "corpus_barrier_check: data length overflow.\n");
	exit(1);
    }
    score = dbfetch_num(cp_db, buffer);

    /* ������θ���ʤ��Ȥ� */
    if (!para_flag) {
	if (parallel1 == 'P')
	    sprintf(buffer, "%s %c %s %c", type1+3, touten1, type2+3, touten2);
	else
	    sprintf(buffer, "%sP%c %s %c", type1+3, touten1, type2+3, touten2);
	scorep = dbfetch_num(cp_db, buffer);
	score += scorep;
    }

    /* ���Ϥ��뤿��˵�Ͽ���Ƥ��� */
    BarrierMatrix.Value[pos1][pos2] = score;
    if (BarrierMatrix.Type1[pos1] == NULL)
	BarrierMatrix.Type1[pos1] = strdup(type1+3);
    if (BarrierMatrix.Type2[pos2] == NULL)
	BarrierMatrix.Type2[pos2] = strdup(type2+3);

    /* �ǥХå�����
    fprintf(Outfp, "  %d\n", score); */

    /* ���ȤϷ��ä����Ȥ�����Ф褤 */
    if (score)
	return TRUE;
    else
	return FALSE;
}

/* ����ɽ������ؿ� */
void print_barrier(int bnum)
{
    int i, j;

    fprintf(Outfp, ";;;(B)   ");
    for (i = 0; i < bnum; i++)
	fprintf(Outfp, " %2d", i);
    fprintf(Outfp, "\n");
    for (i = 0; i < bnum-1; i++) {
	fprintf(Outfp, ";;;(B) %2d   ", i);
	for (j = 0; j < i; j++) {
	    fprintf(Outfp, "   ");
	}
	for (j = i+1; j < bnum; j++) {
	    /* fprintf(Outfp, " %2c", BarrierMatrix.Value[i][j] ? 'o' : '-'); */
	    if (BarrierMatrix.Value[i][j] > 99)
		fprintf(Outfp, "  *");
	    else if (BarrierMatrix.Value[i][j] == -1)
		fprintf(Outfp, "  -");
	    else
		fprintf(Outfp, " %2d", BarrierMatrix.Value[i][j]);
	    
	}
	fprintf(Outfp, "\n");
    }

    for (i = 0; i < bnum; i++)
	fprintf(Outfp, ";;;(B) %2d %10s %20s\n", i, BarrierMatrix.Type1[i] ? BarrierMatrix.Type1[i] : " ", BarrierMatrix.Type2[i] ? BarrierMatrix.Type2[i] : " ");

    /* ����� */
    for (i = 0; i < bnum; i++) {
	if (BarrierMatrix.Type1[i]) {
	    free(BarrierMatrix.Type1[i]);
	    BarrierMatrix.Type1[i] = NULL;
	}
	if (BarrierMatrix.Type2[i]) {
	    free(BarrierMatrix.Type2[i]);
	    BarrierMatrix.Type2[i] = NULL;
	}
	for (j = i+1; j < bnum; j++)
	    BarrierMatrix.Value[i][j] = -1;
    }
}

int init_optional_case() {
    int i;
    buffer[DATA_LEN-1] = '\n';
    for (i = 0; i < BNST_MAX; i++)
	CorpusComment[i][DATA_LEN-1] = '\n';

    if (OptionalCaseDBname)
	op_db = db_read_open(OptionalCaseDBname);
    else
	op_db = db_read_open(OP_DB_NAME);
    /* wc_db = db_read_open(WC_DB_NAME); */
    op_sm_db = db_read_open(OP_SM_DB_NAME);

    smcf.maxnum = 0;
    smcf.frame = NULL;

    return TRUE;
}

void close_optional_case()
{
    DB_close(op_db);
    DB_close(op_sm_db);
}

/* Ǥ�ճʤ���η���������٤�Ĵ�٤�ؿ� */
int corpus_optional_case_comp(SENTENCE_DATA *sp, BNST_DATA *ptr1, char *case1, BNST_DATA *ptr2, CORPUS_DATA *corpus)
{
    int i, j, k, score, flag, pos1, pos2, firstscore = 0, special = 0;
    int fukugojiflag = 0;
    char *cp1 = NULL, *cp2 = NULL, *cp;

    /* ʸ���ֹ� */
    pos1 = ptr1->num;
    pos2 = ptr2->num;

    /* ʸ�� feature �Υ����å� */
    if (check_feature_for_optional_case(ptr1->f) == TRUE)
	return 0;

    /* ����¦����ư�֤ξ�� */
    if (check_feature(ptr2->f, "�����") || check_feature(ptr2->f, "������"))
	return 0;

    /* ����ξ�� */
    if (check_feature(ptr2->f, "������") || check_feature(ptr2->f, "��������"))
	return 0;

    /* ��ʸ */
    if (check_feature(ptr1->f, "ID:���ȡʰ��ѡ�")) {
	cp1 = strdup("C:��ʸ");
	special = 1;
    }
    /* ���� */
    else if (check_feature(ptr1->f, "����")) {
	cp1 = strdup("C:����");
	special = 1;
    }
    /* ʣ�缭 */
    else if (check_feature(ptr1->f, "ʣ�缭") && ptr1->num > 0) {
	cp = ptr1->Jiritu_Go;

	/* �ҤȤ�����ʸ��μ�Ω���ߤ� */
	pos1 = ptr1->num - 1;
	ptr1 = sp->bnst_data + pos1;

	/* ���δط��Ȥ��Ƥϡ�������°��+��ʬ�μ�Ω�� */
	case1 = (char *)malloc_data(strlen((ptr1->fuzoku_ptr + ptr1->fuzoku_num - 1)->Goi) + 
				    strlen(cp) + 1, "optional_case");
	sprintf(case1, "%s%s", (ptr1->fuzoku_ptr + ptr1->fuzoku_num - 1)->Goi, cp);
	fukugojiflag = 1;
    }

    /* Memory ���� */
    if (!cp1) {
	cp1 = (char *)malloc_data(strlen(ptr1->Jiritu_Go)+1, "optional case");
	cp1[0] = '\0';
    }
    if (!cp2) {
	cp2 = (char *)malloc_data(strlen(ptr2->Jiritu_Go)+1, "optional case");
	cp2[0] = '\0';
    }

    /* ����¦��Ω�������û�����Ƥ��� */
    for (i = 0; i < ptr1->jiritu_num; i++) {
	if (!special) {
	    cp1[0] = '\0';
	    flag = 1;

	    /* ����¦��Ω����κ��� */
	    for (k = i; k < ptr1->jiritu_num; k++) {
		/* ����Ū̾��, ����̾���ޤ�Ǥ�������� */
		if (check_Morph_for_optional_case(ptr1->jiritu_ptr+k) == TRUE) {
		    flag = 0;
		    break;
		}
		strcat(cp1, (ptr1->jiritu_ptr+k)->Goi);
	    }

	    if (!flag)
		continue;

	    /* �֤ʤ��, �֤ʤ���, �֤����, �֤���� �Ͻ��� */
	    if (check_JiritsuGo_for_optional_case(cp1) == TRUE)
		continue;
	}

	/* ����¦��Ω�������û�����Ƥ��� */
	for (j = 0; j < ptr2->jiritu_num; j++) {
	    cp2[0] = '\0';
	    flag = 1;

	    /* ����¦��Ω����κ��� */
	    for (k = j; k < ptr2->jiritu_num; k++) {
		/* ����Ū̾��, ����̾���ޤ�Ǥ�������� */
		if (check_Morph_for_optional_case(ptr2->jiritu_ptr+k) == TRUE) {
		    flag = 0;
		    break;
		}
		strcat(cp2, (ptr2->jiritu_ptr+k)->Goi);
	    }

	    if (!flag)
		continue;

	    /* �֤ʤ��, �֤ʤ���, �֤����, �֤���� �Ͻ��� */
	    if (check_JiritsuGo_for_optional_case(cp2) == TRUE)
		continue;

	    /* DB �򸡺����뤿��Υ�������� */
	    sprintf(buffer, "%s:%s %s", cp1, case1, cp2);
	    if (buffer[DATA_LEN-1] != '\n') {
		fprintf(stderr, "corpus_optional_case_comp: data length overflow.\n");
		exit(1);
	    }

	    /* DB ���� */
	    score = dbfetch_num(op_db, buffer);

	    sprintf(buffer, "Unsupervised:%s:%s %s -> %d", cp1, case1, cp2, score);
	    if (buffer[DATA_LEN-1] != '\n') {
		fprintf(stderr, "corpus_optional_case_comp: data length overflow.\n");
		exit(1);
	    }
#ifdef DEBUGMORE
	    fprintf(Outfp, ";;;(O) %d %d %s:%s %s ->%d\n", pos1, pos2, cp1, case1, cp2, score);
#endif

	    if (score && !firstscore) {
		if (!CorpusComment[ptr1->num][0]) {
		    sprintf(CorpusComment[ptr1->num], "%s:%s %s %d", cp1, case1, cp2, score);
		    if (CorpusComment[ptr1->num][DATA_LEN-1] != '\n') {
			fprintf(stderr, "corpus_optional_case_comp: data length overflow.\n");
			exit(1);
		    }
		}

		if (corpus)
		    corpus->data = strdup(buffer);

		/* full match */
		if (!i && !j)
		    firstscore = 2;
		/* part match */
		else
		    firstscore = 1;
	    }
	}
	if (special)
	    break;
    }

    free(cp1);
    free(cp2);

    if (fukugojiflag)
	free(case1);

    return firstscore*5;
}

/* �����٤�Ǥ�ճʤǤ���п����֤��ؿ� */
int check_optional_case(char *scase)
{

    /* ���ץ�����Ϳ����줿�� */
    if (OptOptionalCase) {
	if (str_eq(scase, OptOptionalCase))
	    return TRUE;
	else
	    return FALSE;
    }
    else {
	/* ��, ����, �ޥ�, �ȳʤ��� */
	if (str_eq(scase, "�����") || 
	    str_eq(scase, "����") || 
	    str_eq(scase, "�ǳ�") || 
	    str_eq(scase, "�ȳ�") || 
	    str_eq(scase, "�˳�") || 
	    str_eq(scase, "�س�") || 
	    str_eq(scase, "�ޥǳ�") || 
	    str_eq(scase, "����"))
	    return TRUE;
	else
	    return FALSE;
    }
}

/* ���������Ѥ�����ʸ�ڤ����򤹤뤫�ݤ� */
void optional_case_evaluation(SENTENCE_DATA *sp)
{
    int i;
    int appropriate = 0;

    /* ���̤� Best ��ȡ�������Ѥ������� Best ��Ʊ���ʤ�� return */
    if (Op_Best_mgr.ID < 0 || sp->Best_mgr->ID == Op_Best_mgr.ID)
	return;

    /* �ؽ����Ǥʤ���� */
    if (!OptLearn) {
	for (i = 0;i < sp->Bnst_num; i++) {
	    /* ������Ѥ���ʸ�� */
	    if (Op_Best_mgr.dpnd.op[i].flag && sp->Best_mgr->dpnd.head[i] != Op_Best_mgr.dpnd.head[i]) {

		/* �����褬�ۤʤꡢ������Ѥ��Ƥ���Хǡ������� */
		if (Op_Best_mgr.dpnd.op[i].data)
		    assign_cfeature(&(Op_Best_mgr.dpnd.f[i]), Op_Best_mgr.dpnd.op[i].data);

		/* ����������� */
		if (check_feature(Op_Best_mgr.dpnd.f[i], "����")) {
		    appropriate++;
		}
		/* �������ʤ���� */
		else {
		    /* ������Ѥ����Ȥ��Τۤ����ᤤ�Ȥ� */
		    if (Op_Best_mgr.dpnd.head[i] < sp->Best_mgr->dpnd.head[i])
			appropriate++;
		    /* ������Ѥ����Ȥ��Τۤ����󤤤Ȥ� */
		    else
			appropriate--;
		}
	    }
	}

	/* ������Ѥ����Ȥ��Υ��������礭����
	   ���� appropriate �� 0 �ʾ���ä���Ŭ�� */
	/* 
	   if (Op_Best_mgr.score > sp->Best_mgr->score && appropriate >= 0) {
	   */
	if (Op_Best_mgr.score > sp->Best_mgr->score) {
	    *(sp->Best_mgr) = Op_Best_mgr;
	    return;
	}
    }
}

/*==================================================================*/
    int subordinate_level_check_special(char *cp, BNST_DATA *ptr2)
/*==================================================================*/
{
    char *level1, *level2;

    level1 = cp;
    level2 = (char *)check_feature(ptr2->f, "��٥�");

    /* Ϣ�Τ� FALSE */
    if (check_feature(ptr2->f, "��:Ϣ��") || check_feature(ptr2->f, "��:Ϣ��"))
	return FALSE;

    if (level1 == NULL) return TRUE;		/* ����ʤ� --> ���Ǥ�OK */
    else if (level2 == NULL) return FALSE;	/* ���Ǥ�� --> ����ʤ� */
    else if (levelcmp(level1, level2 + strlen("��٥�:")) <= 0)
	return TRUE;				/* ptr1 <= ptr2 �ʤ�OK */
    else return FALSE;
}

/* �¸� (����ط���켡���ꥹ�Ȥˤ������)*/
int temp_corpus_clause_comp(BNST_DATA *ptr1, BNST_DATA *ptr2, int para_flag)
{
    char *type1, *type2, *level1, *level2;
    char parallel1, parallel2, touten1, touten2;
    int score1, score2, offset;

    /* para_flag == TRUE  : ������θ (������Ϥޤ��θƤӽФ��Ǥϰ�̣���ʤ�)
       para_flag == FALSE : �����̵�� */

    /* ����� */
    parallel1 = ' ';
    parallel2 = ' ';
    touten1 = ' ';
    touten2 = ' ';

    /* ���������
    if (para_flag == TRUE) {
	if (ptr1->para_key_type != PARA_KEY_O)
	    parallel1 = 'P';
    }
    */

    /* �Ҹ쥿���פ����� */
    type1 = (char *)check_feature(ptr1->f, "ID");
    type2 = (char *)check_feature(ptr2->f, "ID");

    /* level ������(ɽ�����Ѥ������) */
    level1 = (char *)check_feature(ptr1->f, "��٥�");
    level2 = (char *)check_feature(ptr2->f, "��٥�");

    if (!type1) return TRUE;
    if (!type2) return FALSE;

    /* ������������򸫤뤿��Υ��ե��å� */
    if (!strcmp(type2+3, "���ʤ����"))
	offset = 1;
    else
	offset = 0;

    /* ���������� */
    if ((char *)check_feature(ptr1->f, "����"))
	touten1 = ',';
    if ((char *)check_feature((ptr2+offset)->f, "����"))
	touten2 = ',';

    /* �ǡ����١����θ��� */

    /* ����¦ */
    sprintf(buffer, "%s%c%c", type1+3, parallel1, touten1);
    if (buffer[DATA_LEN-1] != '\n') {
	fprintf(stderr, "corpus_clause_comp: data length overflow.\n");
	exit(1);
    }

    /*  fprintf(stdout, ";;; K�� %s ", buffer); */
    score1 = dbfetch_num(c_temp_db, buffer);
    /*  fprintf(stdout, "%d\n", score1); */

    /* ����¦ */
    sprintf(buffer, "%s%c%c", type2+3, parallel2, touten2);
    if (buffer[DATA_LEN-1] != '\n') {
	fprintf(stderr, "corpus_clause_comp: data length overflow.\n");
	exit(1);
    }

    /*  fprintf(stdout, ";;; U�� %s ", buffer); */
    score2 = dbfetch_num(c_temp_db, buffer);
    /*  fprintf(stdout, "%d\n", score2); */

    /*
    fprintf(stderr, ";;; �� %d %d\n", score1, score2);
    */

    /* ������������������ TRUE */
    /* if (!score2 || score1 <= score2) */
    if (!score1 || score1 <= score2)
	return TRUE;
    else
	return FALSE;
}

void CheckChildCaseFrame(SENTENCE_DATA *sp) {
    int i, j;
    TOTAL_MGR *tm = sp->Best_mgr;

    for (i = sp->Bnst_num-1; i > 0; i--) {
	if (!check_feature((sp->bnst_data+i)->f, "�Ѹ�"))
	    continue;
	for (j = 0; j < i; j++) {
	    if (tm->dpnd.head[j] == i) {
		assign_cfeature(&((sp->bnst_data+i)->f), "�ҡ�");
		break;
	    }
	}
    }
}

/* �����겼 Unsupervised ��Ϣ (̤����) */

void _make_sm_frame(char *buf, char *delimiter, _SMCaseFrame* cf) {
    char *token;

    token = strtok_r(buf, delimiter, &buf);
    while (token) {
	if (cf->num >= cf->maxnum) {
	    cf->maxnum += SM_ALLOCATION_STEP;
	    cf->list = (SMwithFrequency *)realloc(cf->list, sizeof(SMwithFrequency)*cf->maxnum);
	}
	(cf->list+cf->num)->sm = (char *)malloc_data(SM_CODE_SIZE+1, "_make_sm_frame");
	strncpy((cf->list+cf->num)->sm, token, SM_CODE_SIZE);
	*((cf->list+cf->num)->sm+SM_CODE_SIZE) = '\0';
	(cf->list+cf->num)->frequency = atoi(token+SM_CODE_SIZE+1);
	/* printf("====> <%s>\n", token); */
	token = strtok_r(NULL, delimiter, &buf);
	cf->num++;
    }
}

void make_sm_frame(char *buf, char *delimiter, SMCaseFrame* cf) {
    char *token, *cp;

    token = strtok_r(buf, delimiter, &buf);
    while (token) {
	cp = strchr(token, ':');
	if (cp) {
	    *cp = '\0';
	    if (cf->num >= cf->maxnum) {
		cf->maxnum += CASE_ALLOCATION_STEP;
		cf->frame = (_SMCaseFrame *)realloc(cf->frame, sizeof(_SMCaseFrame)*cf->maxnum);
	    }
	    (cf->frame+cf->num)->relation = strdup(token);	/* ���ط������� */
	    (cf->frame+cf->num)->list = NULL;
	    (cf->frame+cf->num)->num = 0;
	    (cf->frame+cf->num)->maxnum = 0;
	    token = cp+1;
	}
	/* printf("==> <%s>\n", token); */
	_make_sm_frame(token, " ", cf->frame+cf->num);
	token = strtok_r(NULL, delimiter, &buf);
	cf->num++;
    }
}

float match_sm_tree(char *code, char *rel, SMCaseFrame* cf) {
    char *cp, *sm, backup, *cp1;
    int i, n = -1, value = 0, count = 0;

    if (!*code)
	return 0;

    for (i = 0; i < cf->num; i++) {
	if (str_eq((cf->frame+i)->relation, rel)) {
	    n = i;
	    break;
	}
    }

    /* �ʤ��ޥå����ʤ��Ȥ� */
    if (n < 0)
	return 0;

    for (cp = sm = code; *cp; cp += SM_CODE_SIZE) {
	count++;
	if (cp == sm)
	    continue;
	backup = *cp;
	*cp = '\0';
	cp1 = strdup(sm);
	sm = cp;
	*cp = backup;
	*cp1 = '1';

	for (i = 0; i < (cf->frame+n)->num; i++) {
	    if (comp_sm(((cf->frame+n)->list+i)->sm, cp1, 0) > 0) {
		value += ((cf->frame+n)->list+i)->frequency;
		/* fprintf(stdout, "%s: %s %s * %d\n", cf->predicate, cp1, ((cf->frame+n)->list+i)->sm, ((cf->frame+n)->list+i)->frequency); */
	    }
	}
	/* fprintf(stdout, "V:%d N:%d\n", value, count); */
    }
    if (count)
	return value/count;
    else
	return 0;
}

char *get_unsupervised_data(DBM_FILE db, char *key, char c, char p) {

    /* DB �򸡺����뤿��Υ�������� */
    if (c && p) {
	sprintf(buffer, "%s%c%c", key, c, p);
    }
    else if (c) {
	sprintf(buffer, "%s%c", key, c);
    }
    else if (p) {
	sprintf(buffer, "%s%c", key, p);
    }
    else {
	sprintf(buffer, "%s", key);
    }
    if (buffer[DATA_LEN-1] != '\n') {
	fprintf(stderr, "CorpusExampleDependencyFrequency: data length overflow.\n");
	exit(1);
    }

    /* DB ���� */
    return db_get(db, buffer);
}

/* ��̣�Ǥη���������٤�Ĵ�٤�ؿ� */
float CorpusSMDependencyFrequency(SENTENCE_DATA *sp, BNST_DATA *ptr1, char *case1, BNST_DATA *ptr2, CORPUS_DATA *corpus, int target)
{
    int i, j, k, pos1, pos2;
    int fukugojiflag = 0;
    char *cp2 = NULL, *cp, *string = NULL, causative = 0, passive = 0;
    float score = 0;

    /* ʸ���ֹ� */
    pos1 = ptr1->num;
    pos2 = ptr2->num;

    /* ʸ�� feature �Υ����å� */
    if (check_feature_for_optional_case(ptr1->f) == TRUE)
	return 0;

    /* ����¦����ư�֤ξ�� */
    if (check_feature(ptr2->f, "�����") || check_feature(ptr2->f, "������"))
	passive = 'P';

    /* ����ξ�� */
    if (check_feature(ptr2->f, "������") || check_feature(ptr2->f, "��������"))
	causative = 'C';

    /* ��ʸ */
    if (str_eq(case1, "�ȳ�") && (check_feature(ptr1->f, "ID:���ȡʰ��ѡ�") || 
				  check_feature(ptr1->f, "ID:�ʶ��ڡ�"))) {
	return 0;
    }

    /* ���� */
    else if (check_feature(ptr1->f, "����")) {
	return 0;
    }
    /* ʣ�缭 */
    else if (check_feature(ptr1->f, "ʣ�缭") && ptr1->num > 0) {
	cp = ptr1->Jiritu_Go;

	/* �ҤȤ�����ʸ��μ�Ω���ߤ� */
	pos1 = ptr1->num - 1;
	ptr1 = sp->bnst_data + pos1;

	/* ���δط��Ȥ��Ƥϡ�������°��+��ʬ�μ�Ω�� */
	case1 = (char *)malloc_data(strlen((ptr1->fuzoku_ptr + ptr1->fuzoku_num - 1)->Goi) + 
				    strlen(cp) + 1, "CorpusSMDependencyFrequency");
	sprintf(case1, "%s%s", (ptr1->fuzoku_ptr + ptr1->fuzoku_num - 1)->Goi, cp);
	fukugojiflag = 1;
    }

    /* Memory ���� */
    if (!cp2) {
	cp2 = (char *)malloc_data(strlen(ptr2->Jiritu_Go)+1, "CorpusSMDependencyFrequency");
	cp2[0] = '\0';
    }

    /* DB ��������ᡢ����¦��Ω���Ĵ�� */
    for (j = 0; j < ptr2->jiritu_num; j++) {
	cp2[0] = '\0';

	/* ����¦��Ω����κ��� */
	for (k = j; k < ptr2->jiritu_num; k++)
	    strcat(cp2, (ptr2->jiritu_ptr+k)->Goi);

	/* DB ���� */
	if ((string = get_unsupervised_data(op_sm_db, cp2, causative, passive)))
	    break; /* ����� break */
    }

    /* ��̣�� */
    if (string) {
	/* ����� */
	smcf.num = 0;
	smcf.predicate = strdup(cp2);

	make_sm_frame(string, ";", &smcf);
	score = match_sm_tree(ptr1->SM_code, case1, &smcf);

	/* ����� */
	if (smcf.frame) {
	    for (i = 0; i < smcf.num; i++) {
		if ((smcf.frame+i)->list) {
		    for (j = 0; j < (smcf.frame+i)->num; j++)
			if (((smcf.frame+i)->list+j)->sm)
			    free(((smcf.frame+i)->list+j)->sm);
		    free((smcf.frame+i)->list);
		}
		(smcf.frame+i)->num = 0;
		(smcf.frame+i)->maxnum = 0;
		if ((smcf.frame+i)->relation)
		    free((smcf.frame+i)->relation);
	    }
	    free(smcf.predicate);
	}
    }

    free(string);
    free(cp2);

    if (fukugojiflag)
	free(case1);

    return score;
}

float get_unsupervised_num(DBM_FILE db, char *cp1, char *case1, char *cp2, char c, char p) {

    /* DB �򸡺����뤿��Υ�������� */
    if (c && p) {
	sprintf(buffer, "%s:%s %s%c%c", cp1, case1, cp2, c, p);
    }
    else if (c) {
	sprintf(buffer, "%s:%s %s%c", cp1, case1, cp2, c);
    }
    else if (p) {
	sprintf(buffer, "%s:%s %s%c", cp1, case1, cp2, p);
    }
    else {
	sprintf(buffer, "%s:%s %s", cp1, case1, cp2);
    }
    if (buffer[DATA_LEN-1] != '\n') {
	fprintf(stderr, "CorpusExampleDependencyFrequency: data length overflow.\n");
	exit(1);
    }

    /* DB ���� */
    return dbfetch_num(db, buffer);
}

/* ����η���������٤��֤��ؿ� */
float CorpusExampleDependencyFrequency(SENTENCE_DATA *sp, BNST_DATA *ptr1, char *case1, BNST_DATA *ptr2, CORPUS_DATA *corpus, int target) {
    int i, k, score, flag, pos1, pos2, special = 0;
    int fukugojiflag = 0;
    char *cp1, *cp2, *cp;
    float maxscore = 0, tempscore;
    char *pbuffer = NULL, causative = 0, passive = 0;

    /* ʸ���ֹ� */
    pos1 = ptr1->num;
    pos2 = ptr2->num;

    /* ʸ�� feature �Υ����å� */
    if (check_feature_for_optional_case(ptr1->f) == TRUE)
	return 0;

    /* ����¦����ư�֤ξ�� */
    if (check_feature(ptr2->f, "�����") || check_feature(ptr2->f, "������"))
	passive = 'P';

    /* ����ξ�� */
    if (check_feature(ptr2->f, "������") || check_feature(ptr2->f, "��������"))
	causative = 'C';

    /* ��ʸ */
    if (str_eq(case1, "�ȳ�") && (check_feature(ptr1->f, "ID:���ȡʰ��ѡ�") || 
				  check_feature(ptr1->f, "ID:�ʶ��ڡ�"))) {
	cp1 = strdup("C:��ʸ");
	special = 1;
    }
    /* ���� */
    else if (check_feature(ptr1->f, "����")) {
	cp1 = strdup("C:����");
	special = 1;
    }
    /* ʣ�缭 */
    else if (check_feature(ptr1->f, "ʣ�缭") && ptr1->num > 0) {
	cp = ptr1->Jiritu_Go;

	/* �ҤȤ�����ʸ��μ�Ω���ߤ� */
	pos1 = ptr1->num - 1;
	ptr1 = sp->bnst_data + pos1;

	/* ���δط��Ȥ��Ƥϡ�������°��+��ʬ�μ�Ω�� */
	case1 = (char *)malloc_data(strlen((ptr1->fuzoku_ptr + ptr1->fuzoku_num - 1)->Goi) + 
				    strlen(cp) + 1, "optional_case");
	sprintf(case1, "%s%s", (ptr1->fuzoku_ptr + ptr1->fuzoku_num - 1)->Goi, cp);
	fukugojiflag = 1;
    }

    /* Memory ���� */
    cp1 = (char *)malloc_data(strlen(ptr1->Jiritu_Go), "optional case");
    cp1[0] = '\0';
    cp2 = (char *)malloc_data(strlen(ptr2->Jiritu_Go), "optional case");
    cp2[0] = '\0';

    /* ����¦��Ω�� */
    cp2[0] = '\0';
    flag = 1;

    /* ����¦��Ω����κ��� */
    for (k = 0; k < ptr2->jiritu_num; k++) {
	/* ����Ū̾��, ����̾���ޤ�Ǥ�������� */
	if (check_Morph_for_optional_case(ptr2->jiritu_ptr+k) == TRUE) {
	    flag = 0;
	    break;
	}
	strcat(cp2, (ptr2->jiritu_ptr+k)->Goi);
    }

    if (!flag)
	return 0;

    /* �֤ʤ��, �֤ʤ���, �֤����, �֤���� �Ͻ��� */
    if (check_JiritsuGo_for_optional_case(cp2) == TRUE)
	return 0;

    /* count = dbfetch_num(wc_db, cp2); */

    /* ����¦��Ω�������û�����Ƥ��� */
    for (i = 0; i < ptr1->jiritu_num; i++) {
	cp1[0] = '\0';
	flag = 1;

	/* ����¦��Ω����κ��� */
	for (k = i; k < ptr1->jiritu_num; k++) {
	    /* ����Ū̾��, ����̾���ޤ�Ǥ�������� */
	    if (check_Morph_for_optional_case(ptr1->jiritu_ptr+k) == TRUE) {
		flag = 0;
		break;
	    }
	    strcat(cp1, (ptr1->jiritu_ptr+k)->Goi);
	}

	if (!flag)
	    continue;

	/* �֤ʤ��, �֤ʤ���, �֤����, �֤���� �Ͻ��� */
	if (check_JiritsuGo_for_optional_case(cp1) == TRUE)
	    continue;

	score = get_unsupervised_num(op_db, cp1, case1, cp2, causative, passive);

	if (target) {
	    sprintf(buffer, "Unsupervised:%s:%s %s -> %d", cp1, case1, cp2, score);
	    if (buffer[DATA_LEN-1] != '\n') {
		fprintf(stderr, "corpus_optional_case_comp: data length overflow.\n");
		exit(1);
	    }
	}

#ifdef DEBUGMORE
	fprintf(Outfp, ";;;(O) %d %d %s:%s %s(%d) ->%d\n", pos1, pos2, cp1, case1, cp2, score);
#endif

	if (score) {
	    /* tempscore = (float)score/count; */
	    tempscore = (float)score;
	    if (maxscore < tempscore) {
		maxscore = tempscore;
		if (target) {
		    if (pbuffer)
			free(pbuffer);
		    pbuffer = strdup(buffer);
		}
	    }
	}
	if (special)
	    break;
    }

    /* Feature �ؤΥǡ������� */
    if (target && corpus && pbuffer) {
	if (corpus->data) {
	    char *newstr;
	    newstr = (char *)malloc_data(strlen(corpus->data)+strlen(pbuffer)+2, "CorpusExampleDependencyFrequency");
	    sprintf(newstr, "%s %s", corpus->data, pbuffer);
	    free(corpus->data);
	    corpus->data = newstr;
	}
	else {
	    corpus->data = strdup(pbuffer);
	}
    }

    free(cp1);
    free(cp2);

    if (fukugojiflag)
	free(case1);

    return maxscore;	/* ���祹�����򤫤��� */
}

/* ������ꥹ�Ȥ�Ϳ����줿�Ȥ��ˡ����ꤵ�줿������Υ�������׻�����ؿ� */
int CorpusExampleDependencyCalculation(SENTENCE_DATA *sp, BNST_DATA *ptr1, char *case1, int h, CHECK_DATA *list, CORPUS_DATA *corpus)
{
    int i, flag;
    float score, currentscore = -1, totalscore = 0;
    float smscore, currentsmscore = -1, totalsmscore = 0;
    float lastscore, ratio, jiprob = 0, smprob = 0;
    float *candidates_score, *candidates_smscore;
    char *newstr;

    if (list->num < 0)
	return 0;

    candidates_score = (float *)malloc_data(sizeof(float)*list->num, "CorpusExampleDependencyCalculation");
    candidates_smscore = (float *)malloc_data(sizeof(float)*list->num, "CorpusExampleDependencyCalculation");

    /* fprintf(Outfp, "�� %2d ==>\n", ptr1->num); */

    /* ���䤹�٤ƤΥ�������׻����� */
    for (i = 0; i < list->num; i++) {
	if (list->pos[i] == h)
	    flag = 1;
	else
	    flag = 0;

	/* ��Ω��Υ�������׻����� */
	score = CorpusExampleDependencyFrequency(sp, ptr1, case1, sp->bnst_data+list->pos[i], corpus, flag);
	totalscore += score;
	*(candidates_score+i) = score;

	/* ��̣�ǤΥ�������׻����� */
	smscore = CorpusSMDependencyFrequency(sp, ptr1, case1, sp->bnst_data+list->pos[i], corpus, flag);
	totalsmscore += smscore;
	*(candidates_smscore+i) = smscore;

	/* fprintf(Outfp, "           %2d ��Ω:%f ��̣:%f\n", list->pos[i], score, smscore); */

	/* ��Ĵ�٤Ƥ��� Head �Ǥ���Ȥ� */
	if (flag) {
	    currentscore = score;
	    currentsmscore = smscore;
	}
    }

    if (currentscore < 0 || currentsmscore < 0) {
	fprintf(stderr, "A contradiction occured.\n");
	exit(1);
    }

    if (totalscore)
	jiprob = currentscore/totalscore;
    if (totalsmscore)
	smprob = currentsmscore/totalsmscore;

    ratio = currentscore/(currentscore+1);
    if (totalscore && totalsmscore)
	lastscore = ratio*jiprob+(1-ratio)*smprob;
    else if (totalscore)
	lastscore = jiprob;
    else if (totalsmscore)
	lastscore = smprob;
    else
	lastscore = 0;

    sprintf(buffer, "��Ω:%.2f,��̣:%.2f,��:%.2f", jiprob, smprob, lastscore);
    if (buffer[DATA_LEN-1] != '\n') {
	fprintf(stderr, "CorpusExampleDependencyCalculation: data length overflow.\n");
	exit(1);
    }

    if (corpus) {
	if (corpus->data) {
	    newstr = (char *)malloc_data(strlen(corpus->data)+strlen(buffer)+2, "CorpusExampleDependencyCalculation");
	    sprintf(newstr, "%s %s", corpus->data, buffer);
	    free(corpus->data);
	    corpus->data = newstr;
	}
	else {
	    newstr = (char *)malloc_data(strlen(buffer)+14, "CorpusExampleDependencyCalculation");
	    sprintf(newstr, "Unsupervised:%s", buffer);
	    corpus->data = newstr;
	}

	corpus->candidatesdata = (char *)malloc_data(strlen(buffer)*list->num+DATA_LEN, "CorpusExampleDependencyCalculation");
	*(corpus->candidatesdata) = '\0';

	/* ���䤽�줾��Υ����� */
	for (i = 0; i < list->num; i++) {
	    if (list->pos[i] == h)
		continue;

	    if (totalscore)
		jiprob = candidates_score[i]/totalscore;
	    if (totalsmscore)
		smprob = candidates_smscore[i]/totalsmscore;

	    ratio = candidates_score[i]/(candidates_score[i]+1);
	    if (totalscore && totalsmscore)
		score = ratio*jiprob+(1-ratio)*smprob;
	    else if (totalscore)
		score = jiprob;
	    else if (totalsmscore)
		score = smprob;
	    else
		score = 0;

	    sprintf(buffer, " %s(%d) ��Ω:%.2f,��̣:%.2f,��:%.2f", (sp->bnst_data+list->pos[i])->Jiritu_Go, list->pos[i], jiprob, smprob, score);
	    if (buffer[DATA_LEN-1] != '\n') {
		fprintf(stderr, "CorpusExampleDependencyCalculation: data length overflow.\n");
		exit(1);
	    }
	    strcat(corpus->candidatesdata, buffer);
	}
    }

    free(candidates_score);
    free(candidates_smscore);

    return (int)(lastscore*10);
}

void unsupervised_debug_print(SENTENCE_DATA *sp) {
    int i;

    for (i = 0; i < sp->Bnst_num; i++) {
	/* ������Ѥ���ʸ�� */
	if (sp->Best_mgr->dpnd.op[i].flag) {
	    /* ���Ϥ��� free */
	    if (sp->Best_mgr->dpnd.op[i].data) {
		fprintf(Outfp, "; %s(%d) %s\n", (sp->bnst_data+i)->Jiritu_Go, i, sp->Best_mgr->dpnd.op[i].data);
		if (*(sp->Best_mgr->dpnd.op[i].candidatesdata))
		    fprintf(Outfp, ";        %s\n", sp->Best_mgr->dpnd.op[i].candidatesdata);
		free(sp->Best_mgr->dpnd.op[i].data);
		sp->Best_mgr->dpnd.op[i].data = NULL;
	    }
	    if (sp->Best_mgr->dpnd.op[i].candidatesdata) {
		free(sp->Best_mgr->dpnd.op[i].candidatesdata);
		sp->Best_mgr->dpnd.op[i].candidatesdata = NULL;
	    }
	}
    }
}
