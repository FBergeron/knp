/*====================================================================

			   �����ڥ饤�֥��

                                         Daisuke Kawahara 2002. 7. 31

    $Id$
====================================================================*/
#include "knp.h"

#define DT_EQ		1
#define DT_NE		2
#define DT_GT		3
#define DT_GE		4
#define DT_LT		5
#define DT_LE		6

char *DTFile[PP_NUMBER];	/* �����ڥե����� */

typedef struct _dtcond {
    int   num;
    int   eq;
    float value;
    struct _dtcond *next;
} DTCOND;

typedef struct {
    int class;
    float cf;
    DTCOND *cond;
} DTRULE;

#define	DT_RULE_NUM_MAX	1000

typedef struct {
    DTRULE ContextRules[DT_RULE_NUM_MAX];    
    int ContextRuleNum;
} DT;

DT *DTrule[PP_NUMBER];


/*==================================================================*/
			int trans_eq(char *eq)
/*==================================================================*/
{
    if (!strcmp(eq, "eq")) {
	return DT_EQ;
    }
    else if (!strcmp(eq, "ne")) {
	return DT_NE;
    }
    else if (!strcmp(eq, "gt")) {
	return DT_GT;
    }
    else if (!strcmp(eq, "ge")) {
	return DT_GE;
    }
    else if (!strcmp(eq, "lt")) {
	return DT_LT;
    }
    else if (!strcmp(eq, "le")) {
	return DT_LE;
    }
    else {
	fprintf(stderr, ";; Invalid DT equal (%s)\n", eq);
	exit(1);
    }
}

/*==================================================================*/
	      int read_svm_str(char *buf, DTRULE *rule)
/*==================================================================*/
{
    char *token;
    DTCOND **nc;

    nc = &(rule->cond);
    token = strtok(buf, " ");
    while (token) {
	*nc = (DTCOND *)malloc(sizeof(DTCOND));
	sscanf(token, "%d:%f", &((*nc)->num), &((*nc)->value));
	(*nc)->eq = 0;
	(*nc)->next = NULL;

	token = strtok(NULL, " ");
	nc = &((*nc)->next);
    }

    return 0;
}

/*==================================================================*/
	       int read_dt_str(char *buf, DTRULE *rule)
/*==================================================================*/
{
    char class[3], *token, eq[3];
    DTCOND **nc;

    sscanf(buf, "%s %f %[^\n]", class, &(rule->cf), buf);

    if (strncmp(class, "OK", 2)) {
	return 1;
    }
    else {
	rule->class = 1;
    }

    nc = &(rule->cond);
    token = strtok(buf, " ");
    while (token) {
	*nc = (DTCOND *)malloc_data(sizeof(DTCOND), "read_dt_str");
	sscanf(token, "%d:%[^:]:%f", &((*nc)->num), eq, &((*nc)->value));
	(*nc)->eq = trans_eq(eq);
	(*nc)->next = NULL;

	token = strtok(NULL, " ");
	nc = &((*nc)->next);
    }

    return 0;
}

/*==================================================================*/
	      void read_dt_file(DT *dt, char *filename)
/*==================================================================*/
{
    FILE *fp;
    char buf[DATA_LEN];

    dt->ContextRuleNum = 0;

    if (filename == NULL) {
	fprintf(stderr, ";; DTFile is not specified!!\n");
	exit(1);
    }
    else if ((fp = fopen(filename, "r")) == NULL) {
	fprintf(stderr, ";; Cannot open file (%s) !!\n", filename);
	exit(1);
    }

    while (fgets(buf, DATA_LEN, fp) != NULL) {
	if (read_dt_str(buf, &(dt->ContextRules[dt->ContextRuleNum])) == 0) {
	    dt->ContextRuleNum++;
	}
    }

    fclose(fp);
}

/*==================================================================*/
	 int dt_comp_elem(float r, float d, int eq, int flag)
/*==================================================================*/
{
    /* flag�����äƤ���С�float�Τޤ�ɾ������ */
    if (flag) {
	if (eq == DT_EQ) {
	    return d == r ? 1 : 0;
	}
	else if (eq == DT_NE) {
	    return d != r ? 1 : 0;
	}
	else if (eq == DT_GT) {
	    return d > r ? 1 : 0;
	}
	else if (eq == DT_GE) {
	    return d >= r ? 1 : 0;
	}
	else if (eq == DT_LT) {
	    return d < r ? 1 : 0;
	}
	else if (eq == DT_LE) {
	    return d <= r ? 1 : 0;
	}
    }

    if (eq == DT_EQ) {
	return (int)d == (int)r ? 1 : 0;
    }
    else if (eq == DT_NE) {
	return (int)d != (int)r ? 1 : 0;
    }
    else if (eq == DT_GT) {
	return (int)d > (int)r ? 1 : 0;
    }
    else if (eq == DT_GE) {
	return (int)d >= (int)r ? 1 : 0;
    }
    else if (eq == DT_LT) {
	return (int)d < (int)r ? 1 : 0;
    }
    else if (eq == DT_LE) {
	return (int)d <= (int)r ? 1 : 0;
    }
    return 0;
}

/*==================================================================*/
		float dt_classify(char *data, int pp)
/*==================================================================*/
{
    DT *dt;
    DTRULE d; /* �ƥ�����feature */
    DTCOND *rc, *dc;
    int i, flag;

    /* 0�Ϥ��٤Ƥγ��� */
    if (DTrule[0]) {
	dt = DTrule[0];
    }
    else {
	dt = DTrule[pp];
    }

    read_svm_str(data, &d);

    for (i = 0; i < dt->ContextRuleNum; i++) {
	rc = dt->ContextRules[i].cond;
	flag = 1;
	/* rule���Υ롼�� */
	while (rc) {
	    dc = d.cond;
	    /* rule¦��Ʊ��feature��ǡ�������õ�� */
	    while (rc->num != dc->num) {
		dc = dc->next;
		if (dc == NULL) {
		    fprintf(stderr, ";; DT rule mismatched! (%d)\n", rc->num);
		    exit(1);
		}
	    }

	    /* feature�ֹ椬1�ΤȤ�����٤ʤΤǡ�float����� */
	    if (!dt_comp_elem(rc->value, dc->value, rc->eq, rc->num == 1 ? 1 : 0)) {
		flag = 0;
		break;
	    }
	    rc = rc->next;
	}
	/* ���٤Ƥξ������������Ȥ� */
	if (flag) {
	    return dt->ContextRules[i].cf;
	    /* dc = d.cond;
	    while (dc) {
		* ����٤��֤� *
		if (dc->num == 1) {
		    return dc->value;
		}
		dc = dc->next;
	    } */
	}
    }

    return -1;
}

/*==================================================================*/
			    void init_dt()
/*==================================================================*/
{
    int i;

    for (i = 0; i < PP_NUMBER; i++) {
	if (DTFile[i]) {
	    DTrule[i] = (DT *)malloc_data(sizeof(DT), "init_dt");
	    read_dt_file(DTrule[i], DTFile[i]);
	    if (i == 0) { /* ���٤Ƥγ��� */
		break;
	    }
	}
	else {
	    DTrule[i] = NULL;
	}
    }
}

/*====================================================================
                               END
====================================================================*/
