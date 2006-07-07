/*====================================================================

			     ��ͭɽ������

                                               R.SASANO 05. 7. 31

    $Id$
====================================================================*/
#include "knp.h"

#define SIZE               2
#define NE_TAG_NUMBER      9
#define NE_POSITION_NUMBER 4
#define FEATIRE_MAX        1024
#define TAG_POSITION_NAME   20
#define HEAD               0
#define MIDDLE             1
#define TAIL               2
#define SINGLE             3
#define SIGX               10 /* SVM�η�̤��Ψ�˶�����륷���⥤�ɴؿ��η��� */

DBM_FILE ne_db;

char *DBforNE;
char TagPosition[NE_MODEL_NUMBER][TAG_POSITION_NAME];
char *Tag_name[] = {
    "ORGANIZATION", "PERSON", "LOCATION", "ARTIFACT",
    "DATE", "TIME", "MONEY", "PERCENT", "OTHER", "\0"};
char *Position_name[] = {
    "head", "middle", "tail", "single", "\0"};
struct NE_MANAGER {
    char feature[FEATIRE_MAX];          /* ���� */
    int notHEAD;                        /* head, single�ˤϤʤ�ʤ����1 */
    int NEresult;                       /* NE�β��Ϸ�� */
    double SVMscore[NE_MODEL_NUMBER];   /* �ƥ������ݥ������Ȥʤ��Ψ */
    double max[NE_MODEL_NUMBER];        /* �����ޤǤκ��祹���� */
    int parent[NE_MODEL_NUMBER];        /* ���祹�����η�ϩ */
} NE_mgr[MRPH_MAX];

typedef struct ne_cache {
    char            *key;
    int	            ne_result[NE_MODEL_NUMBER + 1];
    struct ne_cache *next;
} NE_CACHE;

NE_CACHE *ne_cache[TBLSIZE];

/*====================================================================
		     �������ݥ������ݥ������б�
====================================================================*/
void init_tagposition()
{
    int i, j;
    for (i = 0; i < NE_TAG_NUMBER - 1; i++) {
	for (j = 0; j < NE_POSITION_NUMBER; j++) {
	    strcpy(TagPosition[i * NE_POSITION_NUMBER + j], Tag_name[i]);
	    strcat(TagPosition[i * NE_POSITION_NUMBER + j], ":");
	    strcat(TagPosition[i * NE_POSITION_NUMBER + j], Position_name[j]);
	}
    }
    strcpy(TagPosition[32], "OTHER:single");
}

/*====================================================================
		   �������ݥ������ݥ������б��ؿ�
====================================================================*/
int ne_tagposition_to_code(char *cp)
{
    int i;
    for (i = 0; TagPosition[i]; i++)
	if (str_eq(TagPosition[i], cp))
	    return i;
    return -1;
}

char *ne_code_to_tagposition(int num)
{
    return TagPosition[num];
}

/*==================================================================*/
			void init_db_for_NE()
/*==================================================================*/
{
    char *db_filename;

    db_filename = check_dict_filename(DBforNE, TRUE);

    if ((ne_db = DB_open(db_filename, O_RDONLY, 0)) == NULL) {
	if (OptDisplay == OPT_DEBUG) {
	    fprintf(Outfp, "Opening %s ... failed.\n", db_filename);
	}
	fprintf(stderr, ";; Cannot open POS table for NE <%s>.\n", db_filename);
	exit(1);
    } 
    else {
	if (OptDisplay == OPT_DEBUG) {
	    fprintf(Outfp, "Opening %s ... done.\n", db_filename);
	}
    }
    free(db_filename);
}

/*==================================================================*/
			void close_db_for_NE()
/*==================================================================*/
{
    DB_close(ne_db);
}

/*==================================================================*/
			  void init_NE_mgr()
/*==================================================================*/
{
    memset(NE_mgr, 0, sizeof(struct NE_MANAGER)*MRPH_MAX);
}

/*==================================================================*/
			 void init_ne_cache()
/*==================================================================*/
{
    memset(ne_cache, 0, sizeof(NE_CACHE *)*TBLSIZE);
}

/*==================================================================*/
		 int get_chara(FEATURE *f, char *Goi)
/*==================================================================*/
{
    int i;
    char *Chara_name[] = {
	"����", "�Ҥ餬��", "���ʴ���", "��������", "����", "�ѵ���", "����", "\0"};

    if (Goi && !strcmp(Goi, "��")) return 5; /* ���� */
    for (i = 0; *Chara_name[i]; i++)
	if (check_feature(f, Chara_name[i]))
	    break;
    return i + 1;
}

/*==================================================================*/
	     char *get_pos(MRPH_DATA *mrph_data, int num)
/*==================================================================*/
{
    int i, j, flag;
    char *ret, *buf, pos[SMALL_DATA_LEN];

    ret = (char *)malloc_data(SMALL_DATA_LEN, "get_pos");
    buf = (char *)malloc_data(SMALL_DATA_LEN, "get_pos");
    ret[0] = '\0'; /* �Ƶ�Ū���������뤿�� */
    flag = 0;

    /* �ʻ�ۣ�����Τ����� */
    for (i = 0; i < CLASSIFY_NO + 1; i++) {    
	for (j = 0; j < CLASSIFY_NO + 1; j++) {
	    if (!Class[i][j].id) continue;
	    sprintf(pos, "��ۣ-%s", Class[i][j].id);   
	    if (check_feature(mrph_data->f, pos)) {
		sprintf(buf, "%s%d%d%d10:1 ", ret, i, j, num);
		strcpy(ret, buf);
		flag = 1;
	    }
	}
    }
    if (flag) return ret;

    /* �ʻ�ۣ�����Τʤ���� */
    if (mrph_data->Bunrui)
	sprintf(ret, "%d%d%d10:1 ", mrph_data->Hinshi, mrph_data->Bunrui, num);
    else
	sprintf(ret, "%d0%d10:1 ", mrph_data->Hinshi, num);
    
    free(buf);
    return ret;
}

/*==================================================================*/
			void clear_ne_cache()
/*==================================================================*/
{
    int i;
    NE_CACHE *ncp, *next;

    for (i = 0; i < TBLSIZE; i++) {
	if (!ne_cache[i]) continue;
	ncp = ne_cache[i];
	if (ncp->key) {
	    free(ncp->key);
	}
	ncp = ncp->next;
	while (ncp) {
	    if (ncp->key) {
		free(ncp->key);
	    }
	    next = ncp->next;
	    free(ncp);
	    ncp = next;
	}
	memset(ne_cache, 0, sizeof(NE_CACHE *)*TBLSIZE);
    }
}

/*==================================================================*/
       void register_ne_cache(char *key, int NEresult, int tag)
/*==================================================================*/
{
    /* NE�β��Ϸ�̤���Ͽ���� */
    /* �ҤȤĤ�ʸ�Ϥ���λ����ޤ��˴����ʤ��Τǥ����ɬ�פȤ����ǽ������ */
    /* NEresult = NE_MODEL_NUMBER�ξ���NE���Τξ�����ݻ� */
    /* ���ξ��Τ�tag��Ϳ����졢TAG�μ���+1�򵭲����� */
    /* ���ξ�硢�Ť��ǡ���������о�񤭤���� */

    NE_CACHE **ncpp;

    ncpp = &(ne_cache[hash(key, strlen(key))]);
    while (*ncpp && (*ncpp)->key && strcmp((*ncpp)->key, key)) {
	ncpp = &((*ncpp)->next);
    }
    if (!(*ncpp)) {
	*ncpp = (NE_CACHE *)malloc_data(sizeof(NE_CACHE), "register_ne_cache");
	memset(*ncpp, 0, sizeof(NE_CACHE));
    }
    if (!(*ncpp)->key) {
	(*ncpp)->key = strdup(key);
	(*ncpp)->next = NULL;
    }
    (*ncpp)->ne_result[NEresult] = tag;
}

/*==================================================================*/
	     int check_ne_cache(char *key, int NEresult)
/*==================================================================*/
{
    NE_CACHE *ncp;

    ncp = ne_cache[hash(key, strlen(key))];
    if (!ncp || !ncp->key) {
	return 0;
    }
    while (ncp) {
	if (!strcmp(ncp->key, key)) {
	    return ncp->ne_result[NEresult];
	}
	ncp = ncp->next;
    }
    return 0;
}

/*==================================================================*/
		  char *get_cache(char *key, int num)
/*==================================================================*/
{
    int NEresult;
    NE_CACHE *ncp;
    char *ret, *buf;

    ret = (char *)malloc_data(SMALL_DATA_LEN2, "get_cache");
    buf = (char *)malloc_data(SMALL_DATA_LEN2, "get_cache");
    ret[0] = '\0'; /* �Ƶ�Ū���������뤿�� */

    for (NEresult = 0; NEresult < NE_MODEL_NUMBER - 1; NEresult++) {
	if (check_ne_cache(key, NEresult)) {
	    sprintf(buf, "%s%d%d30:1 ", ret, NEresult + 1, num);
	    strcpy(ret, buf);
	}
    }
    free(buf);
    return ret;
}

/*==================================================================*/
	     char *get_tail(MRPH_DATA *mrph_data, int num)
/*==================================================================*/
{
    int i, j;
    char *ret, *buf;
    char *feature_name[] = {"��̾����", "�ȿ�̾����", "\0"};

    ret = (char *)malloc_data(SMALL_DATA_LEN, "get_tail");
    buf = (char *)malloc_data(SMALL_DATA_LEN, "get_tail");
    ret[0] = '\0'; /* �Ƶ�Ū���������뤿�� */

    /* ʸ������˿�̾�������ȿ�̾�����Ȥ����줬���뤫 */
    for (j = 1;; j++) {
	if (!(mrph_data + j)->f || 
	    check_feature((mrph_data + j)->f, "ʸ���") ||
	    check_feature((mrph_data + j)->f, "����") ||
	    check_feature((mrph_data + j)->f, "���")) break;
	for (i = 0; i < 2; i++) {
	    if (check_feature((mrph_data + j)->f, feature_name[i]))
		sprintf(ret, "%d%d40:1 ", i + 3, num);
	}
    }

    /* ��̾�������ȿ�̾�����Ǥ��뤫 */
    for (i = 0; i < 2; i++) {
	if (check_feature(mrph_data->f, feature_name[i])) {
	    sprintf(buf, "%d%d40:1 %s", i + 1, num, ret);
	    strcpy(ret, buf);
	}	
    }   

    free(buf);
    return ret;
}

/*==================================================================*/
	     char *get_imi(MRPH_DATA *mrph_data, int num)
/*==================================================================*/
{
    int i, j;
    char *ret, *buf;
    char *feature_name[] = {"��̣-�ȿ�", "��̣-��", "��̣-����", "��̣-���", "\0"};

    ret = (char *)malloc_data(SMALL_DATA_LEN, "get_imi");
    buf = (char *)malloc_data(SMALL_DATA_LEN, "get_imi");
    ret[0] = '\0'; /* �Ƶ�Ū���������뤿�� */

     /* ʸ������ˤ��뤫 */
    for (i = 0; i < 4; i++) {	 
	for (j = 1;; j++) {
	    if (!(mrph_data + j)->f || 
		check_feature((mrph_data + j)->f, "ʸ���") ||
		check_feature((mrph_data + j)->f, "����") ||
		check_feature((mrph_data + j)->f, "���")) break;
	    if (check_feature((mrph_data + j)->f, feature_name[i])) {
		sprintf(buf, "%s%d%d50:1 ", ret, i + 5, num);
		strcpy(ret, buf);
		break;
	    }
	}
    }

    /* ��̣���������뤫 */
    for (i = 0; i < 4; i++) {
	if (check_feature(mrph_data->f, feature_name[i])) {
	    sprintf(buf, "%s%d%d50:1 ", ret, i + 1, num);
	    strcpy(ret, buf);
	}	    
    }

    free(buf);
    return ret;
}

/*==================================================================*/
		 void make_feature(SENTENCE_DATA *sp)
/*==================================================================*/
{
    int i, j, k;
    char *buf, *s[5];
    buf = (char *)malloc_data(FEATIRE_MAX, "make_feature");

    for (i = 0; i < sp->Mrph_num; i++) {
	
	/* ��̻Ϥ��������ϸ�ͭɽ������Ƭ�ˤϤʤ�ʤ�(�롼��)  */
	NE_mgr[i].notHEAD = 0;
	if (get_chara(sp->mrph_data[i].f, sp->mrph_data[i].Goi) == 5 &&
	    !check_feature(sp->mrph_data[i].f, "��̻�"))
	    NE_mgr[i].notHEAD = 1;
	
	for (j = i - SIZE; j <= i + SIZE; j++) {
	    if (j < 0 || j >= sp->Mrph_num)
		continue;
	    
	    s[0] = db_get(ne_db, sp->mrph_data[j].Goi2);
	    s[1] = get_pos(sp->mrph_data + j, i - j + SIZE + 1);       /* ��������*/
	    s[2] = get_cache(sp->mrph_data[j].Goi2, i - j + SIZE + 1); /* ��������*/
	    s[3] = get_tail(sp->mrph_data + j, i - j + SIZE + 1);      /* ��������*/
	    s[4] = get_imi(sp->mrph_data + j, i - j + SIZE + 1);       /* ��������*/
	    k = i - j + SIZE + 1;
	    sprintf(buf, "%s%s%d:1 %s%d%d20:1 %s%s%s%d%d60:1 ",
		    NE_mgr[i].feature, s[0] ? s[0] : "", k,
		    s[1], get_chara(sp->mrph_data[j].f, sp->mrph_data[j].Goi), k,
		    OptNEcache ? "" : s[2], OptNEend ? "" : s[3], OptNEcase ? s[4] : "",
		    strlen(sp->mrph_data[j].Goi2) / 2, k);
	    strcpy(NE_mgr[i].feature, buf);
	    free(s[0]);
	    free(s[1]);
	    free(s[2]);
	    free(s[3]);
	    free(s[4]);
	}
    }       
    free(buf);
}

/*==================================================================*/
	      void output_svm_feature(SENTENCE_DATA *sp)
/*==================================================================*/
{
    int i, j, code;
    char *cp;

    for (i = 0; i < sp->Mrph_num; i++) {
	if ((cp = check_feature(sp->mrph_data[i].f, "NE"))) {
	    code = ne_tagposition_to_code(cp + 3);
	}
	else {
	    code = 32;
	}  
	NE_mgr[i].NEresult = code;
	if (OptDisplay == OPT_DEBUG) {
	    fprintf(stderr, "%d %s\t%s\n", code, sp->mrph_data[i].Goi2, NE_mgr[i].feature);
	}
	else {
	    for (j = 0; j < NE_MODEL_NUMBER; j++) {
		fprintf(stderr, (j == code) ? "+1 " : "-1 ");
	    }
	    fprintf(stderr, "%s\n", NE_mgr[i].feature);
	}
    }
}

/*==================================================================*/
	       void apply_svm_model(SENTENCE_DATA *sp)
/*==================================================================*/
{
    int i, j;
    
    for (i = 0; i < sp->Mrph_num; i++) {
	if (OptDisplayNE == OPT_DEBUG)
	    fprintf(stderr, "%d %s\t%s\n", i, sp->mrph_data[i].Goi2, NE_mgr[i].feature);

	for (j = 0; j < NE_MODEL_NUMBER; j++) {
#ifdef USE_SVM
	    if (NE_mgr[i].notHEAD && 
		j  != NE_MODEL_NUMBER - 1 &&
		(j % 4 == HEAD || j % 4 == SINGLE))
		NE_mgr[i].SVMscore[j] = 0; /* �ҥ塼�ꥹ�ƥ��å��롼�� */
	    else
		NE_mgr[i].SVMscore[j] 
		    = 1/(1+exp(-svm_classify_for_NE(NE_mgr[i].feature, j) * SIGX));

	    if (OptDisplayNE == OPT_DEBUG) {
		fprintf(stderr, "%2d %f\t", j, NE_mgr[i].SVMscore[j]);
		if (j % 4 == SINGLE && j  != NE_MODEL_NUMBER - 2) fprintf(stderr, "\n");
		if (j  == NE_MODEL_NUMBER - 1) fprintf(stderr, "\n\n");
	    }
#endif
	}
    }
}

/*==================================================================*/
		  int constraint(int pre, int self, int last)
/*==================================================================*/
{
    /* ��������륿��������˰�ȿ�����1���֤�  */
    if (pre  == NE_MODEL_NUMBER - 1) pre  += 3;
    if (self == NE_MODEL_NUMBER - 1) self += 3;

    if (pre == -1) {
	if (self % 4 == MIDDLE || self % 4 == TAIL) return 1;
	return 0;
    }

    if (last && (self % 4 == HEAD || self % 4 == MIDDLE)) return 1;
	
    if ((pre  % 4 == HEAD   || pre  % 4 == MIDDLE) &&
	 self % 4 != MIDDLE && self % 4 != TAIL  ) return 1;
    if ( pre  % 4 != HEAD   && pre  % 4 != MIDDLE  &&
	(self % 4 == MIDDLE || self % 4 == TAIL  )) return 1;  
    if ((pre  % 4 == HEAD   || pre  % 4 == MIDDLE) &&
	pre/4 != self/4) return 1;   
    return 0;
}

/*==================================================================*/
		   void viterbi(SENTENCE_DATA *sp)
/*==================================================================*/
{
    int i, j, k;
    double score, max;
  
    for (i = 0; i < sp->Mrph_num; i++) {
	for (j = 0; j < NE_MODEL_NUMBER; j++) {

	    /* ʸƬ�ξ�� */
	    if (i == 0) {
		if (constraint(-1, j, 0)) continue;
		NE_mgr[i].max[j] = NE_mgr[i].SVMscore[j];
		NE_mgr[i].parent[j] = -1; /* ʸƬ */
		continue;
	    }

	    /* ʸƬ��ʸ���ʳ� */
	    NE_mgr[i].max[j] = 0;
	    for (k = 0; k < NE_MODEL_NUMBER; k++) {
		if (i == sp->Mrph_num - 1) {
		    if (constraint(k, j, 1)) continue;
		}
		else {
		    if (constraint(k, j, 0)) continue;
		}
		score = NE_mgr[i-1].max[k]*NE_mgr[i].SVMscore[j];
		if (score > NE_mgr[i].max[j]) { /* Ʊ���ξ���̵�� */
		    NE_mgr[i].max[j] = score;
		    NE_mgr[i].parent[j] = k;
		}
	    }
	}
    }       
    max = 0;
    for (j = 0; j < NE_MODEL_NUMBER; j++) {
	if (NE_mgr[sp->Mrph_num-1].max[j] > max) {
	    max = NE_mgr[sp->Mrph_num-1].max[j];
	    NE_mgr[sp->Mrph_num-1].NEresult = j;
	}
    }
    for (i = sp->Mrph_num - 1; i > 0; i--) {
	NE_mgr[i-1].NEresult = NE_mgr[i].parent[NE_mgr[i].NEresult];
    }
}

/*==================================================================*/
		     int get_mrph_ne(FEATURE *fp)
/*==================================================================*/
{
    int i;
    char cp[32];

    for (i = 0; i < NE_MODEL_NUMBER - 1; i++) {
	sprintf(cp, "NE:%s", ne_code_to_tagposition(i));
	if (check_feature(fp, cp)) return i;
    }
    return i;
}

/*==================================================================*/
	    void assign_ne_feature_mrph(SENTENCE_DATA *sp)
/*==================================================================*/
{
    int i;
    char cp[WORD_LEN_MAX];

    /* �����Ǥ���Ϳ */
    for (i = 0; i < sp->Mrph_num; i++) {
	if (NE_mgr[i].NEresult == NE_MODEL_NUMBER -1) continue; /* OTHER�ξ�� */
	sprintf(cp, "NE:%s", ne_code_to_tagposition(NE_mgr[i].NEresult));
	assign_cfeature(&(sp->mrph_data[i].f), cp, FALSE);
    }
}

/*==================================================================*/
	    void assign_ne_feature_tag(SENTENCE_DATA *sp)
/*==================================================================*/
{
    int i, j;
    char cp[WORD_LEN_MAX];
    char cp_nai[WORD_LEN_MAX];

    /* ��������Ϳ */
    for (j = 0; j < sp->Tag_num; j++) { /* Ʊ�쥿���θ�ͭɽ���ϰ����ޤ� */
	for (i = 0; i < sp->tag_data[j].mrph_num; i++) {
	    if (check_feature((sp->tag_data[j].mrph_ptr + i)->f, "NE")) break;
	}
	/* �оݤΥ����˸�ͭɽ����̵����м��Υ����� */
	if (i == sp->tag_data[j].mrph_num) continue;

	/* ORGANIZATION��PERSON�ξ��ϰ�̣�ǤȤ���Ϳ���� */
	if (!strcmp(Tag_name[get_mrph_ne((sp->tag_data[j].mrph_ptr + i)->f) / 4],
		    "ORGANIZATION")) {
	    assign_sm((BNST_DATA *)(sp->tag_data +j), "�ȿ�");
	}
	else if (!strcmp(Tag_name[get_mrph_ne((sp->tag_data[j].mrph_ptr + i)->f) / 4],
			 "PERSON")) {
	    assign_sm((BNST_DATA *)(sp->tag_data + j), "��");
	}
 
	sprintf(cp, "NE:%s:",
		Tag_name[get_mrph_ne((sp->tag_data[j].mrph_ptr + i)->f) / 4]);
	while(1) {
	    if (get_mrph_ne((sp->tag_data[j].mrph_ptr + i)->f) == NE_MODEL_NUMBER - 1) {
		OptNElearn ?
		    fprintf(stdout, "Illegal NE ending!!\n") :
		    fprintf(stderr, "Illegal NE ending!!\n");
		break;
	    }    
	    strcat(cp, (sp->tag_data[j].mrph_ptr + i)->Goi2);
	    if (get_mrph_ne((sp->tag_data[j].mrph_ptr + i)->f) % 4 == SINGLE ||
		get_mrph_ne((sp->tag_data[j].mrph_ptr + i)->f) % 4 == TAIL) {
		assign_cfeature(&(sp->tag_data[j].f), cp, FALSE);
		break;
	    }
	    /* ʣ���Υ����ˤޤ����äƤ�����ϼ��Υ����˿ʤ� */
	    i++;
	    if (i == sp->tag_data[j].mrph_num) {
		i = 0;
		sprintf(cp_nai, "NE��:%s",
		    Tag_name[get_mrph_ne((sp->tag_data[j].mrph_ptr + i)->f) / 4]);
		assign_cfeature(&(sp->tag_data[j].f), cp_nai, FALSE);
		j++;
	    }
	}
    }
}

/*==================================================================*/
		 void ne_analysis(SENTENCE_DATA *sp)
/*==================================================================*/
{
    init_NE_mgr();
    /* SVM���Ѥ�����ͭɽ������ */
    make_feature(sp);
    if (OptNElearn) {
	output_svm_feature(sp);
    }
    else {
	apply_svm_model(sp);
	viterbi(sp);
	/* ��̤���Ϳ */
	assign_ne_feature_mrph(sp);
	/* ��̾��ҤȤĤΥ����ˤ��뤿��Υ롼����ɤ� */
	assign_general_feature(sp->mrph_data, sp->Mrph_num, NeMorphRuleType, FALSE, FALSE);
    }
}

/*==================================================================*/
	    void for_ne_analysis(SENTENCE_DATA *sp)
/*==================================================================*/
{
    /* �ʲ��Ϸ�̤��顢�ȿ��ȿ�̾�����ΤȤ���feature����Ϳ���� */
    /* �����򡢥˳ʤǤ��Ĥ��γʥե졼���<����>��Ϳ�����Ƥ����� */

    int i, j, num;
    CF_PRED_MGR *cpm_ptr;

    /* ������夫������å� */
    for (j = sp->Tag_num - 1; j > 0 ; j--) {
	if (!(cpm_ptr = sp->tag_data[j].cpm_ptr)) continue;
	for (i = 0; i < cpm_ptr->cf.element_num; i++) {
	    num = cpm_ptr->cmm[0].result_lists_d[0].flag[i];

		if (cf_match_element(cpm_ptr->cmm[0].cf_ptr->sm[num], "�ȿ�", TRUE)) {
		    assign_cfeature(&((cpm_ptr->elem_b_ptr[i])->head_ptr->f), "��̣-�ȿ�", FALSE);
		}		    
		if (cf_match_element(cpm_ptr->cmm[0].cf_ptr->sm[num], "��", TRUE)) {
		    assign_cfeature(&((cpm_ptr->elem_b_ptr[i])->head_ptr->f), "��̣-��", FALSE);
		}		    
		if (cf_match_element(cpm_ptr->cmm[0].cf_ptr->sm[num], "����", TRUE)) {
		    assign_cfeature(&((cpm_ptr->elem_b_ptr[i])->head_ptr->f), "��̣-����", FALSE);
		}
		if (cf_match_element(cpm_ptr->cmm[0].cf_ptr->sm[num], "���", TRUE)) {
		    assign_cfeature(&((cpm_ptr->elem_b_ptr[i])->head_ptr->f), "��̣-���", FALSE);
		}
	}
    }
}

/*==================================================================*/
int ne_corefer(SENTENCE_DATA *sp, int i, char *anaphor, char *ne)
/*==================================================================*/
{
    /* ��ͭɽ��(ORGANIZATION)�� */
    /* �����ȴط��ˤ����Ƚ�Ǥ��줿��ͭɽ����������Ϳ����Ƥ��ʤ�ɽ���� */
    /* ��ͭɽ����������Ϳ���� */
   
    int start, end, ne_tag, j, k;
    char cp[WORD_LEN_MAX], word[WORD_LEN_MAX];

    for (ne_tag = 0; ne_tag < NE_TAG_NUMBER; ne_tag++) {
	/* �ɤΥ����Ǥ��뤫��"NE:"��³��4ʸ����Ƚ�Ǥ��� */
	if (!strncmp(ne + 3, Tag_name[ne_tag], 4)) break;
    }

    end = (sp->tag_data + i)->head_ptr - sp->mrph_data;  /* ��������ޤ��Τˤ�̤�б� */
    for (start = ((sp->tag_data + i)->b_ptr)->mrph_ptr - sp->mrph_data; 
	 start <= end; start++) {
	word[0] = '\0';
	for (k = start; k <= end; k++) {
	    strcat(word, (sp->mrph_data + k)->Goi2); /* ��Ի���� */
	}
	if (!strcmp(word, anaphor)) break;
    }

    /* ORGANIZATION�ξ��Τ� */
    if (strcmp(Tag_name[ne_tag], "ORGANIZATION")) return 0;

    /* �����Ǥ���Ϳ��NEresult�˵�Ͽ */
    if ((j = start) == end) {
	sprintf(cp, "NE:%s:single", Tag_name[ne_tag]);
	assign_cfeature(&(sp->mrph_data[j].f), cp, FALSE);
	NE_mgr[j].NEresult = ne_tag * 4 + 3; /* single */
    }
    else for (j = start; j <= end; j++) {
	if (j == start) {
	    sprintf(cp, "NE:%s:head", Tag_name[ne_tag]);
	    assign_cfeature(&(sp->mrph_data[j].f), cp, FALSE);
	    NE_mgr[j].NEresult = ne_tag * 4; /* head */
	}
	else if (j == end) {
	    sprintf(cp, "NE:%s:tail", Tag_name[ne_tag]);
	    assign_cfeature(&(sp->mrph_data[j].f), cp, FALSE);
	    NE_mgr[j].NEresult = ne_tag * 4 + 2; /* tail */
	}
	else {
	    sprintf(cp, "NE:%s:middle", Tag_name[ne_tag]);
	    assign_cfeature(&(sp->mrph_data[j].f), cp, FALSE);
	    NE_mgr[j].NEresult = ne_tag * 4 + 1; /* middle */
	}
    }

    /* ORGANIZATION��PERSON�ξ��ϰ�̣�ǤȤ���Ϳ���� */
    if (!strcmp(Tag_name[get_mrph_ne((sp->tag_data[i].head_ptr)->f) / 4],
		"ORGANIZATION")) {
	assign_sm((BNST_DATA *)(sp->tag_data + i), "�ȿ�");
    }
    else if (!strcmp(Tag_name[get_mrph_ne((sp->tag_data[i].head_ptr)->f) / 4],
		     "PERSON")) {
	assign_sm((BNST_DATA *)(sp->tag_data + i), "��");
    }

    /* ��������Ϳ */
    sprintf(cp, "NE:%s:%s", Tag_name[ne_tag], anaphor);
    assign_cfeature(&(sp->tag_data[i].f), cp, FALSE);  
    sprintf(cp, "NE��:%s", Tag_name[ne_tag]);
    for (k = 0; start < sp->tag_data[i - k].mrph_ptr - sp->mrph_data;) {
	k++;
	assign_cfeature(&(sp->tag_data[i - k].f), cp, FALSE);
    }
    
    return 1;
}

/*==================================================================*/
		void make_ne_cache(SENTENCE_DATA *sp)
/*==================================================================*/
{
    int i, ne_tag;
    char *cp;

    /* �Ʒ����Ǥξ���򵭲� */
    for (i = 0; i < sp->Mrph_num; i++) {
	register_ne_cache(sp->mrph_data[i].Goi2, NE_mgr[i].NEresult, 1);
    }

    /* ��ͭɽ����ǧ�����줿ʸ����򵭲� */
    for (i = 0; i < sp->Tag_num; i++) {   
	if ((cp = check_feature(sp->tag_data[i].f, "NE"))) {
	    for (ne_tag = 0; ne_tag < NE_TAG_NUMBER; ne_tag++) {
		/* �ɤΥ����Ǥ��뤫��"NE:"��³��4ʸ����Ƚ�Ǥ��� */
		if (!strncmp(cp + 3, Tag_name[ne_tag], 4)) break;
	    }
	    cp += 3; /* "NE:"���ɤ����Ф� */
	    while (strncmp(cp, ":", 1)) cp++;
	    register_ne_cache(cp + 1, NE_MODEL_NUMBER, ne_tag + 1);
	} 
    }
}
	  
/*====================================================================
                               END
====================================================================*/

