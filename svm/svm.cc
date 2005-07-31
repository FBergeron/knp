#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef STDC_HEADERS
#include <stdio.h>
#endif

#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif

#ifdef HAVE_SETJMP_H
#include <setjmp.h>
#endif

#ifdef HAVE_MATH_H
#include <math.h>
#endif

#ifdef HAVE_SYS_FILE_H
#include <sys/file.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif

#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

#include <tinysvm.h>
#include "svm.h"

#define	PP_NUMBER	44
#define NE_MODEL_NUMBER	33

char *SVMFile[PP_NUMBER];	/* model�ե�����λ��� */
char *SVMFileNE[NE_MODEL_NUMBER]; /* ne������model�ե�����λ��� */

TinySVM::Model *model[PP_NUMBER];
TinySVM::Model *modelNE[NE_MODEL_NUMBER];

int init_svm() {
    int i;

    for (i = 0; i < PP_NUMBER; i++) {
	if (SVMFile[i]) {
	    model[i] = new TinySVM::Model;
	    if (!model[i]->read(SVMFile[i])) {
		fprintf(stderr, ";; SVM initialization error (%s is corrupted?).\n", SVMFile[i]);
		exit(1);
	    }

	    if (i == 0) { /* ���٤Ƥγ��� */
		return 0;
	    }
	}
	else {
	    model[i] = NULL;
	}
    }
    return 0;
}

int init_svm_for_NE() {
    int i;

    for (i = 0; i < NE_MODEL_NUMBER; i++) {
	modelNE[i] = new TinySVM::Model;
	if (!modelNE[i]->read(SVMFileNE[i])) {
	    fprintf(stderr, ";; SVM initialization error (%s is corrupted?).\n", SVMFileNE[i]);
	    exit(1);
	}
    }
    return 0;
}

double svm_classify(char *line, int pp) {
    TinySVM::Model *m;
    int i = 0;

    /* 0�Ϥ��٤Ƥγ��� */
    if (model[0]) {
	m = model[0];
    }
    else {
	m = model[pp];
    }

    if (!m) {
	fprintf(stderr, ";; SVM model[%d] cannot be read.\n", i);
	exit(1);
    }

    while (isspace(line[i])) i++;
    return m->classify((const char *)(line + i));
}

double svm_classify_for_NE(char *line, int n) {
    TinySVM::Model *m;
    int i = 0;

    m = modelNE[n];

    if (!m) {
	fprintf(stderr, ";; SVM model for NE [%d] cannot be read.\n", i);
	exit(1);
    }

    while (isspace(line[i])) i++;
    return m->classify((const char *)(line + i));
}

