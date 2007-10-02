/*====================================================================

			     ��ͭɽ������

                                               R.SASANO 05. 7. 31

    $Id$
====================================================================*/
#include "knp.h"

#define SIZE               2
#define NE_TAG_NUMBER      9
#define NE_POSITION_NUMBER 4
#define FEATURE_MAX        1024 /* CRL�ǡ����ˤ������Ĺ��272ʸ�� */
#define TAG_POSITION_NAME  20 /* ��Ĺ�� ORGANIZATION:single �ʤ�19ʸ�� */
#define HEAD               0 /* B */
#define MIDDLE             1 /* I */
#define TAIL               2 /* E */
#define SINGLE             3 /* S */

DBM_FILE ne_db;

char *DBforNE;
char TagPosition[NE_MODEL_NUMBER][TAG_POSITION_NAME];
char *Tag_name[] = {
    "ORGANIZATION", "PERSON", "LOCATION", "ARTIFACT",
    "DATE", "TIME", "MONEY", "PERCENT", "OTHER", "\0"};
char *Position_name[] = {
    "head", "middle", "tail", "single", "\0"};
char *Imi_feature[] = {"�ȿ�", "��", "����", "���", "\0"};
char *Chara_name[] = {
    "����", "�Ҥ餬��", "���ʴ���", "��������", "����", "�ѵ���", "����", "����¾", "\0"};

struct NE_MANAGER {
    char feature[FEATURE_MAX];    /* ���� */
    int NEresult;                 /* NE�β��Ϸ�� */
    double prob[NE_MODEL_NUMBER]; /* �ƥ������ݥ������Ȥʤ��Ψ */
    double max[NE_MODEL_NUMBER];  /* �����ޤǤκ��祹���� */
    int parent[NE_MODEL_NUMBER];  /* ���祹�����η�ϩ */
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
    }
    memset(ne_cache, 0, sizeof(NE_CACHE *)*TBLSIZE);
}

/*==================================================================*/
       void register_ne_cache(char *key, int NEresult, int tag)
/*==================================================================*/
{
    /* NE�β��Ϸ�̤���Ͽ����
       NEresult = NE_MODEL_NUMBER�ξ���NE���Τξ�����ݻ�
       ���ξ��Τ�tag��Ϳ����졢TAG�μ���+1�򵭲�����
       ���ξ�硢�Ť��ǡ���������о�񤭤����(���ߤ��Ի���) */

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
		 int get_chara(MRPH_DATA *mrph_data)
/*==================================================================*/
{
    int i;

    if (mrph_data->Goi && !strncmp(mrph_data->Goi, "��", 2)) return 5; /* ���� */
    for (i = 0; strcmp(Chara_name[i], "����¾"); i++)
	if (check_feature(mrph_data->f, Chara_name[i]))
	    break;
    return i + 1;
}

/*==================================================================*/
	     char *get_pos(MRPH_DATA *mrph_data, int num)
/*==================================================================*/
{
    int i, j, flag;
    char *ret, buf[SMALL_DATA_LEN], pos[SMALL_DATA_LEN];

    ret = (char *)malloc_data(SMALL_DATA_LEN, "get_pos");
    ret[0] = '\0'; /* �Ƶ�Ū���������뤿�� */
    flag = 0;

    /* �ʻ�ۣ�����Τ����� */
    for (i = 0; i < CLASSIFY_NO + 1; i++) {    
	for (j = 0; j < CLASSIFY_NO + 1; j++) {
	    if (!Class[i][j].id) continue;
	    sprintf(pos, "��ۣ-%s", Class[i][j].id);   
	    if (check_feature(mrph_data->f, pos)) {
		sprintf(buf, "%s:%s", ret, Class[i][j].id);
		strcpy(ret, buf);
		flag++;
	    }
	}
    }
    if (flag > 1) {
    	return ret;
    }
    
    /* �ʻ�ۣ�����Τʤ���� */
    sprintf(ret, ":%s", Class[mrph_data->Hinshi][mrph_data->Bunrui].id);
    return ret;
}

/*==================================================================*/
		  char *get_cache(char *key, int num)
/*==================================================================*/
{
    int NEresult;
    NE_CACHE *ncp;
    char *ret, buf[SMALL_DATA_LEN2];

    ret = (char *)malloc_data(SMALL_DATA_LEN2, "get_cache");
    ret[0] = '\0'; /* �Ƶ�Ū���������뤿�� */

    for (NEresult = 0; NEresult < NE_MODEL_NUMBER - 1; NEresult++) {
	if (check_ne_cache(key, NEresult)) {
	    sprintf(buf, "%s:%d", ret, NEresult);
	    strcpy(ret, buf);
	}
    }
    return ret;
}

/*==================================================================*/
	     char *get_feature(MRPH_DATA *mrph_data, int num)
/*==================================================================*/
{
    int i, j;
    char *ret, buf[SMALL_DATA_LEN];
    char *feature_name[] = {"��̾����", "�ȿ�̾����", '\0'};

    ret = (char *)malloc_data(SMALL_DATA_LEN, "get_feature");
    ret[0] = '\0'; /* �Ƶ�Ū���������뤿�� */

    /* ʸ������˿�̾�������ȿ�̾�����Ȥ����줬���뤫 */
    for (j = 1;; j++) {
	if (!(mrph_data + j)->f || 
	    check_feature((mrph_data + j)->f, "ʸ���") ||
	    check_feature((mrph_data + j)->f, "����") ||
	    check_feature((mrph_data + j)->f, "���")) break;
	for (i = 0; feature_name[i]; i++) {
	    if (check_feature((mrph_data + j)->f, feature_name[i])) {
		sprintf(ret, ":%d", i + 3);
	    }
	}
    }

    /* ��̾�������ȿ�̾�����Ǥ��뤫 */
    for (i = 0; feature_name[i]; i++) {
	if (check_feature(mrph_data->f, feature_name[i])) {
	    sprintf(buf, "%s:%d", ret, i + 1);
	    strcpy(ret, buf);
	}	
    }   

    return ret;
}

/*==================================================================*/
	     char *get_parent(MRPH_DATA *mrph_data, int num)
/*==================================================================*/
{
    int j, c;
    char *ret, buf[WORD_LEN_MAX * 2], *pcp, *ccp, *ncp;

    ret = (char *)malloc_data(WORD_LEN_MAX * 2, "get_parent");
    ret[0] = '\0'; /* �Ƶ�Ū���������뤿�� */
    if (num != SIZE + 1) return ret;

    if ((pcp = check_feature(mrph_data->f, "�Է�����μ缭"))) {
	strcpy(ret, "D");	
	if ((ccp = check_feature(mrph_data->f, "��"))) {
	    sprintf(buf, "%s:%s", ret, ccp + 3);	
	    strcpy(ret, buf);	    	    
	}
	sprintf(buf, "%s D:%s", ret, pcp + 15);	
	strcpy(ret, buf);	    
    }
    
    /* ʸ������ˤ��뤫 */
    if (!strstr(ret, "D")) {
	for (j = 1;; j++) {
	    if (!(mrph_data + j)->f ||
		check_feature((mrph_data + j)->f, "ʸ���") ||
		check_feature((mrph_data + j)->f, "���")) break;
	    if ((pcp = check_feature((mrph_data + j)->f, "�Է�����μ缭"))) {
		strcpy(ret, "I");	
		if ((ccp = check_feature((mrph_data + j)->f, "��"))) {
		    sprintf(buf, "%s:%s", ret, ccp + 3);
		    strcpy(ret, buf);	    	    
		}
		sprintf(buf, "%s I:%s", ret, pcp + 15);
		strcpy(ret, buf);	    
		break;
	    }
	}
    }

    if (!strstr(ret, "I") && !strstr(ret, "D"))
	strcat(ret, "NONE NONE");
    
    if (!check_feature(mrph_data->f, "��ʸ��缭")) strcat(ret, " H");
    if ((pcp = check_feature(mrph_data->f, "�Լ缭"))) {
	sprintf(buf, "%s:%s", ret, pcp + 7);
	strcpy(ret, buf);	    
    }
    if (check_feature(mrph_data->f, "��ʸ��缭")) {
	if (strlen(mrph_data->Goi2) < WORD_LEN_MAX /2) {
	    sprintf(buf, "%s S:%s", ret, mrph_data->Goi2);
	}
	else {
	    sprintf(buf, "%s S:LONG_WORD", ret);
	}
	strcpy(ret, buf);
    }

    if (check_feature(mrph_data->f, "ʸ���") && check_feature(mrph_data->f, "��ʸ��缭")) {
	strcat(ret, " SINGLE");
    }
    else if (check_feature(mrph_data->f, "ʸ���")) {
	strcat(ret, " HEAD");
    }
    else if (check_feature(mrph_data->f, "��ʸ��缭")) {
	strcat(ret, " TAIL");
    }
    else if (check_feature(mrph_data->f, "�Լ缭")) {
	strcat(ret, " MIDDLE");
    }
    else {
	strcat(ret, " OTHER");
    }

    return ret;
}

/*==================================================================*/
	     char *get_imi(MRPH_DATA *mrph_data, int num)
/*==================================================================*/
{
    int i, j;
    char *ret, buf[SMALL_DATA_LEN2], cp[WORD_LEN_MAX];

    ret = (char *)malloc_data(SMALL_DATA_LEN2, "get_imi");
    ret[0] = '\0'; /* �Ƶ�Ū���������뤿�� */
    if (num != SIZE + 1) return ret;

    /* �ȿ����͡����Ρ���� */
    for (i = 0; i < 4; i++) {
	sprintf (cp, "��̣-%s", Imi_feature[i]);
	/* ��̣���������뤫 */
	if (check_feature(mrph_data->f, cp)) {
	    sprintf(buf, "%s%d180:1 ", ret, i + 1);
	    strcpy(ret, buf);
	}
	/* ʸ������ˤ��뤫 */
	for (j = 1;; j++) {
	    if (!(mrph_data + j)->f ||
		check_feature((mrph_data + j)->f, "ʸ���") ||
		check_feature((mrph_data + j)->f, "���")) break;
	    if (check_feature((mrph_data + j)->f, cp)) {
		sprintf(buf, "%s%d280:1 ", ret, i + 1);
		strcpy(ret, buf);
		break;
	    }
	}
    }

    /* ��ͭɽ�� */
    for (i = 0; i < NE_TAG_NUMBER - 1; i++) {
	sprintf (cp, "��̣-%s", Tag_name[i]);
	/* ��̣���������뤫 */
	if (check_feature(mrph_data->f, cp)) {
	    sprintf(buf, "%s%d380:1 ", ret, i + 1);
	    strcpy(ret, buf);
	}
	/* ʸ������ˤ��뤫 */
	for (j = 1;; j++) {
	    if (!(mrph_data + j)->f ||
		check_feature((mrph_data + j)->f, "ʸ���") ||
		check_feature((mrph_data + j)->f, "���")) break;
	    if (check_feature((mrph_data + j)->f, cp)) {
		sprintf(buf, "%s%d480:1 ", ret, i + 1);
		strcpy(ret, buf);
		break;
	    }
	}
    }

    return ret;
}

/*==================================================================*/
	       void make_crf_feature(SENTENCE_DATA *sp)
/*==================================================================*/
{
    int i, k;
    char *s[4];

    for (i = 0; i < sp->Mrph_num; i++) {

	s[0] = get_pos(sp->mrph_data + i, 0);
	s[1] = get_feature(sp->mrph_data + i, 0);
	s[2] = get_parent(sp->mrph_data + i, SIZE + 1);
	s[3] = get_cache(sp->mrph_data[i].Goi2, 0);
	
	/* ���Ф� �ʻ� �ʻ��ʬ�� �ʻ�ۣ���� ʸ���� ʸ����
	   (ɽ�س� ������μ缭 �缭 ʸ�������) ����å��� */
	/* feature��1024���ޤ� */
	sprintf(NE_mgr[i].feature, "%s %s %s A%s %s L:%d F%s %s C%s",
		(strlen(sp->mrph_data[i].Goi2) < WORD_LEN_MAX /2) ? 
		sp->mrph_data[i].Goi2 : "LONG_WORD", /* MAX 64ʸ�� */
		Class[sp->mrph_data[i].Hinshi][0].id, /* MAX 8+1ʸ��(̤�����) */
		Class[sp->mrph_data[i].Hinshi][sp->mrph_data[i].Bunrui].id,
		/* MAX 18+1ʸ�� (���ƻ����Ҹ�������) */
		s[0], /* MAX 22+2ʸ��? (�ʽ���:��³����:����¾) */
		Chara_name[get_chara(sp->mrph_data + i) - 1], /* MAX 8+1ʸ�� (�Ҥ餬��) */
		strlen(sp->mrph_data[i].Goi2) / 2, /* MAX 3+2ʸ�� */
		s[1], /* MAX 128ʸ�� */ 
		s[2], /* MAX 256�� */
		s[3]  /* MAX 256ʸ�� */
	    );
	for (k = 0; k < 4; k++) {
	    free(s[k]);
	}
    }
}

/*==================================================================*/
	      void output_feature(SENTENCE_DATA *sp)
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
	    /* CRF�γؽ���̤�ʸ���Ȥ��ƤΥ����Ƚ�ȡ������Ȥ��ƤΥ����Ƚ����פ����뤿��
	       code��100��­���Ƥ��� */
	    fprintf(stderr, "%s %d\n", NE_mgr[i].feature, code + 100);
	}
    }
    fprintf(stderr, "\n");
}

/*==================================================================*/
	       void apply_model(SENTENCE_DATA *sp)
/*==================================================================*/
{
    int i, j;

#ifdef USE_CRF
    clear_crf();    
    for (i = 0; i < sp->Mrph_num; i++) {
	if (OptDisplayNE == OPT_DEBUG)
	    fprintf(stderr, "%d %s\t%s\n", i, sp->mrph_data[i].Goi2, NE_mgr[i].feature);
	crf_add(NE_mgr[i].feature);
    }
    crf_parse();
#endif	       
    
    for (i = 0; i < sp->Mrph_num; i++) {
	if (OptDisplayNE == OPT_DEBUG)
	    fprintf(stderr, "%d %s\t%s\n", i, sp->mrph_data[i].Goi2, NE_mgr[i].feature);
	
	for (j = 0; j < NE_MODEL_NUMBER; j++) {
#ifdef USE_CRF
	    get_crf_prob(i, j, &(NE_mgr[i].prob[j]));
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
		NE_mgr[i].max[j] = NE_mgr[i].prob[j];
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
		score = NE_mgr[i-1].max[k]*NE_mgr[i].prob[j];
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
    char cp[WORD_LEN_MAX * 16];
    char cp_nai[WORD_LEN_MAX * 16];

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
	    
	    if (strlen(cp) + strlen((sp->tag_data[j].mrph_ptr + i)->Goi2) >= WORD_LEN_MAX * 16) {
		fprintf(stderr, ";; Too long tag data for %s... .\n", cp);
		exit(1);
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

    /* ��ͭɽ��ǧ����ɬ�פ�feature��Ϳ���� */
    for_ne_analysis(sp);   

    /* ��������� */
    make_crf_feature(sp);

    if (OptNElearn) {
	output_feature(sp);
    }
    else {
	/* ��ǥ��Ŭ�� */
	apply_model(sp);
	/* ʸ���ΤǺ�Ŭ�� */
	viterbi(sp);
	/* ��̤�����Ǥ���Ϳ */
	assign_ne_feature_mrph(sp);
	/* ��̾��ҤȤĤΥ����ˤ��뤿��Υ롼����ɤ� */
	assign_general_feature(sp->mrph_data, sp->Mrph_num, NeMorphRuleType, FALSE, FALSE);
	/* ��̤���ܶ����Ϳ */
	assign_ne_feature_tag(sp);
    }
}

/*==================================================================*/
	    void for_ne_analysis(SENTENCE_DATA *sp)
/*==================================================================*/
{
    /* ��ʸ���ʲ��Ϸ�̤��顢��ͭɽ�������Ѥ�feature����Ϳ���� */
   
    int i, j, k, l, num;
    char cp[WORD_LEN_MAX];
    CF_PRED_MGR *cpm_ptr;

    /* �缭�ξ��� */
    for (j = 0; j < sp->Bnst_num - 1; j++) {
	assign_cfeature(&((sp->bnst_data[j].head_ptr)->f), "��ʸ��缭", FALSE);

	(strlen((sp->bnst_data[j].head_ptr)->Goi) < WORD_LEN_MAX /2) ? 
	    sprintf(cp, "�Լ缭:%s", (sp->bnst_data[j].head_ptr)->Goi) :
	    sprintf(cp, "�Լ缭:LONG_WORD");
	for (i = 1; (sp->bnst_data[j].head_ptr - i)->f; i++) {
	    if (!(sp->bnst_data[j].head_ptr - i)->f ||
		check_feature((sp->bnst_data[j].head_ptr - i + 1)->f, "ʸ���")) break;
	    assign_cfeature(&((sp->bnst_data[j].head_ptr - i)->f), cp, FALSE);
	}
    }

    /* �Ƥξ��� */
    /* ʸ�������������å� */
    for (j = 0; j < sp->Bnst_num - 1; j++) {	    
	(strlen((sp->bnst_data[sp->bnst_data[j].dpnd_head].head_ptr)->Goi) < WORD_LEN_MAX /2) ? 
	    sprintf(cp, "�Է�����μ缭:%s",
		    (sp->bnst_data[sp->bnst_data[j].dpnd_head].head_ptr)->Goi) :
	    sprintf(cp, "�Է�����μ缭:LONG_WORD");
	
	assign_cfeature(&((sp->bnst_data[j].head_ptr)->f), cp, FALSE);
	assign_cfeature(&((sp->bnst_data[j].head_ptr)->f), 
			check_feature(sp->bnst_data[j].f, "��"), FALSE);    
    }
}

/*==================================================================*/
int ne_corefer(SENTENCE_DATA *sp, int i, char *anaphor, char *ne, int yomi_flag)
/*==================================================================*/
{
    /* ��ͭɽ��(ORGANIZATION)�� */
    /* �����ȴط��ˤ����Ƚ�Ǥ��줿��ͭɽ����������Ϳ����Ƥ��ʤ�ɽ���� */
    /* ��ͭɽ����������Ϳ���� */
   
    int start, end, ne_tag, j, k;
    char cp[WORD_LEN_MAX], word[WORD_LEN_MAX];

    if (strlen(anaphor) == 2) return 0;

    for (ne_tag = 0; ne_tag < NE_TAG_NUMBER; ne_tag++) {
	/* �ɤΥ����Ǥ��뤫��"NE:"��³��4ʸ����Ƚ�Ǥ��� */
	if (!strncmp(ne + 3, Tag_name[ne_tag], 4)) break;
    }

    /* ORGANIZATION��PERSON�ξ��Τ� */
    if (strcmp(Tag_name[ne_tag], "ORGANIZATION") &&
	(strcmp(Tag_name[ne_tag], "PERSON") || !yomi_flag)) return 0;

    end = (sp->tag_data + i)->head_ptr - sp->mrph_data;  /* ��������ޤ��Τˤ�̤�б� */
    if (check_feature(((sp->tag_data + i)->head_ptr)->f, "����")) end--;
    
    for (start = end; start >= 0; start--) {
	word[0] = '\0';
	for (k = start; k <= end; k++) {
	    strcat(word, (sp->mrph_data + k)->Goi2); /* ��Ի���� */
	}
	if (!strcmp(word, anaphor)) break;
    }    
    if (strcmp(word, anaphor)) return 0;

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

#ifdef USE_CRF
    /* �Ʒ����Ǥξ���򵭲� */
    for (i = 0; i < sp->Mrph_num; i++) {
	register_ne_cache(sp->mrph_data[i].Goi2, NE_mgr[i].NEresult, 1);
    }
#endif
}
	  
/*====================================================================
                               END
====================================================================*/
