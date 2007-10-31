/*====================================================================

			     ��¸��¤����

                                               S.Kurohashi 93. 5.31

    $Id$

====================================================================*/
#include "knp.h"

/* DB file for Chinese dpnd rule */
DBM_FILE chi_dpnd_db;
int     CHIDpndExist;
DBM_FILE chi_dpnd_prob_db;
int     CHIDpndProbExist;
DBM_FILE chi_dis_comma_lex_db;
int     CHIDisCommaLexExist;
DBM_FILE chi_dis_comma_bk1_db;
int     CHIDisCommaBK1Exist;
DBM_FILE chi_dis_comma_bk2_db;
int     CHIDisCommaBK2Exist;
DBM_FILE chi_dis_comma_bk_db;
int     CHIDisCommaBKExist;
DBM_FILE chi_dpnd_stru_db;
int     CHIDpndStruExist;

int Possibility;	/* ��¸��¤�β�ǽ���β����ܤ� */
static int dpndID = 0;

/*==================================================================*/
	       void assign_dpnd_rule(SENTENCE_DATA *sp)
/*==================================================================*/
{
    int 	i, j;
    BNST_DATA	*b_ptr;
    DpndRule 	*r_ptr;

    for (i = 0, b_ptr = sp->bnst_data; i < sp->Bnst_num; i++, b_ptr++) {
	for (j = 0, r_ptr = DpndRuleArray; j < CurDpndRuleSize; j++, r_ptr++) {
	    if (feature_pattern_match(&(r_ptr->dependant), b_ptr->f, NULL, b_ptr) 
		== TRUE) {
		b_ptr->dpnd_rule = r_ptr; 
		break;
	    }
	}

	if (b_ptr->dpnd_rule == NULL) {
	    fprintf(stderr, ";; No DpndRule for %dth bnst (", i);
	    print_feature(b_ptr->f, stderr);
	    fprintf(stderr, ")\n");

	    /* DpndRuleArray[0] �ϥޥå����ʤ����� */
	    b_ptr->dpnd_rule = DpndRuleArray;
	}
    }
}

/*==================================================================*/
                      void close_chi_dpnd_db()
/*==================================================================*/
{
    if (!OptChiProb) {
	DB_close(chi_dpnd_db);
    }
    else {
	DB_close(chi_dpnd_prob_db);
	DB_close(chi_dis_comma_lex_db);
	DB_close(chi_dis_comma_bk1_db);
	DB_close(chi_dis_comma_bk2_db);
	DB_close(chi_dis_comma_bk_db);
	DB_close(chi_dpnd_stru_db);
    }
}

/*==================================================================*/
                      void init_chi_dpnd_db()
/*==================================================================*/
{
    if (!OptChiProb) {
	chi_dpnd_db = open_dict(CHI_DPND_DB, CHI_DPND_DB_NAME, &CHIDpndExist);
    }
    else {
	chi_dpnd_prob_db = open_dict(CHI_DPND_PROB_DB, CHI_DPND_PROB_DB_NAME, &CHIDpndProbExist);

	chi_dis_comma_lex_db = open_dict(CHI_DIS_COMMA_LEX_DB, CHI_DIS_COMMA_LEX_DB_NAME, &CHIDisCommaLexExist);
	chi_dis_comma_bk1_db = open_dict(CHI_DIS_COMMA_BK1_DB, CHI_DIS_COMMA_BK1_DB_NAME, &CHIDisCommaBK1Exist);
	chi_dis_comma_bk2_db = open_dict(CHI_DIS_COMMA_BK2_DB, CHI_DIS_COMMA_BK2_DB_NAME, &CHIDisCommaBK2Exist);
	chi_dis_comma_bk_db = open_dict(CHI_DIS_COMMA_BK_DB, CHI_DIS_COMMA_BK_DB_NAME, &CHIDisCommaBKExist);
	chi_dpnd_stru_db = open_dict(CHI_DPND_STRU_DB, CHI_DPND_STRU_DB_NAME, &CHIDpndStruExist);
    }
}
 
/* get dpnd rule for Chinese */
/*==================================================================*/
   char* get_chi_dpnd_rule(char *word1, char *pos1, char *word2, char *pos2, int distance, int label)
/*==================================================================*/
{
    char *key;


    if ((!OptChiProb && CHIDpndExist == FALSE) || (OptChiProb && CHIDpndProbExist == FALSE)) {
	return NULL;
    }

    if (!strcmp(word2, "ROOT") && OptChiProb) {
	key = malloc_db_buf(strlen(word1) + strlen(pos1) + 7);
	sprintf(key, "%s_%s_ROOT", pos1, word1);
    }
    else if (distance == 999999 && OptChiProb) {
	key = malloc_db_buf(strlen(word1) + strlen(word2) + strlen(pos1) + strlen(pos2) + 6);
	sprintf(key, "%s_%s_%s_%s_XX", pos1, word1, pos2, word2);
    }
    else {
	if (distance > 11) {
	    distance = 12;
	}
	else if (distance < -11) {
	    distance = 12;
	}
	key = malloc_db_buf(strlen(word1) + strlen(word2) + strlen(pos1) + strlen(pos2) + 8);
	sprintf(key, "%s_%s_%s_%s_%d", pos1, word1, pos2, word2, distance);
    }
    if (!OptChiProb) {
	return db_get(chi_dpnd_db, key);
    }
    else {
	if (label == 0) {
	    return db_get(chi_dpnd_prob_db, key);
	}
	else if (label == 1) {
	    return db_get(chi_dis_comma_lex_db, key);
	}
	else if (label == 2) {
	    return db_get(chi_dis_comma_bk1_db, key);
	}
	else if (label == 3) {
	    return db_get(chi_dis_comma_bk2_db, key);
	}
	else if (label == 4) {
	    return db_get(chi_dis_comma_bk_db, key);
	}
    }
}

/*==================================================================*/
	       void calc_dpnd_matrix(SENTENCE_DATA *sp)
/*==================================================================*/
{
    int i, j, k, l, s, value, first_uke_flag;
    BNST_DATA *k_ptr, *u_ptr;
    char *lex_rule, *pos_rule_1, *pos_rule_2, *pos_rule;
    char *type, *probL, *probR, *occur, *dpnd;
    int count;
    char *rule;
    char *curRule[CHI_DPND_TYPE_MAX];
    int appear_LtoR_2, appear_RtoL_2, appear_LtoR_3, appear_RtoL_3, total_2, total_3;
    double bkoff_weight_1, bkoff_weight_2;
    int distance;

    /* initialization */
    lex_rule = NULL;
    pos_rule_1 = NULL;
    pos_rule_2 = NULL;
    pos_rule = NULL;
    rule = NULL;
    bkoff_weight_1 = 0.8;
    bkoff_weight_2 = 0.3;

    for (i = 0; i < sp->Bnst_num; i++) {
	k_ptr = sp->bnst_data + i;
	first_uke_flag = 1;
	for (j = i + 1; j < sp->Bnst_num; j++) {
	    u_ptr = sp->bnst_data + j;
	    lex_rule = NULL;
	    pos_rule_1 = NULL;
	    pos_rule_2 = NULL;
	    pos_rule = NULL;

	    Dpnd_matrix[i][j] = 0;
	    Chi_dpnd_matrix[i][j].count = 0;

	    if (Language != CHINESE) {
		for (k = 0; k_ptr->dpnd_rule->dpnd_type[k]; k++) {
		    value = feature_pattern_match(&(k_ptr->dpnd_rule->governor[k]),
					      u_ptr->f, k_ptr, u_ptr);
		    if (value == TRUE) {
			Dpnd_matrix[i][j] = (int)k_ptr->dpnd_rule->dpnd_type[k];
			first_uke_flag = 0;
			break;
		    }
		}
	    }
	    else {
		if (j == i + 1) {
		    distance = 1;
		}
		else {
		    distance = 2;
		}
                /* read dpnd rule from DB for Chinese */
		lex_rule = get_chi_dpnd_rule(k_ptr->head_ptr->Goi, k_ptr->head_ptr->Pos, u_ptr->head_ptr->Goi, u_ptr->head_ptr->Pos, distance, 0);
		pos_rule_1 = get_chi_dpnd_rule(k_ptr->head_ptr->Goi, k_ptr->head_ptr->Pos, "X", u_ptr->head_ptr->Pos, distance, 0);
		pos_rule_2 = get_chi_dpnd_rule("X", k_ptr->head_ptr->Pos, u_ptr->head_ptr->Goi, u_ptr->head_ptr->Pos, distance, 0);
		pos_rule = get_chi_dpnd_rule("X", k_ptr->head_ptr->Pos, "X", u_ptr->head_ptr->Pos,distance, 0);

		if (lex_rule != NULL) {
		    count = 0;
		    rule = NULL;
		    rule = strtok(lex_rule, ":");
		    while (rule) {
			curRule[count] = malloc(strlen(rule) + 1);
			strcpy(curRule[count], rule);
			count++;
			rule = NULL;
			rule = strtok(NULL, ":");
		    }

		    Chi_dpnd_matrix[i][j].count_1 = count;
		    for (k = 0; k < count; k++) {
			type = NULL;
			probL = NULL;
			probR = NULL;
			occur = NULL;
			    
			type = strtok(curRule[k], "_");
			probR = strtok(NULL, "_");
			probL = strtok(NULL, "_");
			occur = strtok(NULL, "_");
			dpnd = strtok(NULL, "_");
			    
			if (!strcmp(type, "R")) {
			    Chi_dpnd_matrix[i][j].direction_1[k] = 'R';
			}
			else if (!strcmp(type, "L")) {
			    Chi_dpnd_matrix[i][j].direction_1[k] = 'L';
			}
			else if (!strcmp(type, "B")) {
			    Chi_dpnd_matrix[i][j].direction_1[k] = 'B';
			}
			Chi_dpnd_matrix[i][j].occur_1[k] = atof(occur);
			strcpy(Chi_dpnd_matrix[i][j].type_1[k], dpnd);
			Chi_dpnd_matrix[i][j].prob_LtoR_1[k] = atof(probR);
			Chi_dpnd_matrix[i][j].prob_RtoL_1[k] = atof(probL);

			if (curRule[k]) {
			    free(curRule[k]);
			}
		    }
		}

		if (pos_rule_1 != NULL) {
		    count = 0;
		    rule = NULL;
		    rule = strtok(pos_rule_1, ":");
		    while (rule) {
			curRule[count] = malloc(strlen(rule) + 1);
			strcpy(curRule[count], rule);
			count++;
			rule = NULL;
			rule = strtok(NULL, ":");
		    }

		    Chi_dpnd_matrix[i][j].count_2 = count;
		    for (k = 0; k < count; k++) {
			type = NULL;
			probL = NULL;
			probR = NULL;
			occur = NULL;
			    
			type = strtok(curRule[k], "_");
			probR = strtok(NULL, "_");
			probL = strtok(NULL, "_");
			occur = strtok(NULL, "_");
			dpnd = strtok(NULL, "_");
			    
			if (!strcmp(type, "R")) {
			    Chi_dpnd_matrix[i][j].direction_2[k] = 'R';
			}
			else if (!strcmp(type, "L")) {
			    Chi_dpnd_matrix[i][j].direction_2[k] = 'L';
			}
			else if (!strcmp(type, "B")) {
			    Chi_dpnd_matrix[i][j].direction_2[k] = 'B';
			}
			Chi_dpnd_matrix[i][j].occur_2[k] = atof(occur);
			strcpy(Chi_dpnd_matrix[i][j].type_2[k], dpnd);
			Chi_dpnd_matrix[i][j].prob_LtoR_2[k] = atof(probR);
			Chi_dpnd_matrix[i][j].prob_RtoL_2[k] = atof(probL);

			if (curRule[k]) {
			    free(curRule[k]);
			}
		    }
		}

		if (pos_rule_2 != NULL) {
		    count = 0;
		    rule = NULL;
		    rule = strtok(pos_rule_2, ":");
		    while (rule) {
			curRule[count] = malloc(strlen(rule) + 1);
			strcpy(curRule[count], rule);
			count++;
			rule = NULL;
			rule = strtok(NULL, ":");
		    }

		    Chi_dpnd_matrix[i][j].count_3 = count;
		    for (k = 0; k < count; k++) {
			type = NULL;
			probL = NULL;
			probR = NULL;
			occur = NULL;
			    
			type = strtok(curRule[k], "_");
			probR = strtok(NULL, "_");
			probL = strtok(NULL, "_");
			occur = strtok(NULL, "_");
			dpnd = strtok(NULL, "_");
			    
			if (!strcmp(type, "R")) {
			    Chi_dpnd_matrix[i][j].direction_3[k] = 'R';
			}
			else if (!strcmp(type, "L")) {
			    Chi_dpnd_matrix[i][j].direction_3[k] = 'L';
			}
			else if (!strcmp(type, "B")) {
			    Chi_dpnd_matrix[i][j].direction_3[k] = 'B';
			}
			Chi_dpnd_matrix[i][j].occur_3[k] = atof(occur);
			strcpy(Chi_dpnd_matrix[i][j].type_3[k], dpnd);
			Chi_dpnd_matrix[i][j].prob_LtoR_3[k] = atof(probR);
			Chi_dpnd_matrix[i][j].prob_RtoL_3[k] = atof(probL);

			if (curRule[k]) {
			    free(curRule[k]);
			}
		    }
		}

		if (pos_rule != NULL) {
		    count = 0;
		    rule = NULL;
		    rule = strtok(pos_rule, ":");
		    while (rule) {
			curRule[count] = malloc(strlen(rule) + 1);
			strcpy(curRule[count], rule);
			count++;
			rule = NULL;
			rule = strtok(NULL, ":");
		    }

		    Chi_dpnd_matrix[i][j].count_4 = count;
		    for (k = 0; k < count; k++) {
			type = NULL;
			probL = NULL;
			probR = NULL;
			occur = NULL;
			    
			type = strtok(curRule[k], "_");
			probR = strtok(NULL, "_");
			probL = strtok(NULL, "_");
			occur = strtok(NULL, "_");
			dpnd = strtok(NULL, "_");
			    
			if (!strcmp(type, "R")) {
			    Chi_dpnd_matrix[i][j].direction_4[k] = 'R';
			}
			else if (!strcmp(type, "L")) {
			    Chi_dpnd_matrix[i][j].direction_4[k] = 'L';
			}
			else if (!strcmp(type, "B")) {
			    Chi_dpnd_matrix[i][j].direction_4[k] = 'B';
			}
			Chi_dpnd_matrix[i][j].occur_4[k] = atof(occur);
			strcpy(Chi_dpnd_matrix[i][j].type_4[k], dpnd);
			Chi_dpnd_matrix[i][j].prob_LtoR_4[k] = atof(probR);
			Chi_dpnd_matrix[i][j].prob_RtoL_4[k] = atof(probL);

			if (curRule[k]) {
			    free(curRule[k]);
			}
		    }
		}

		Chi_dpnd_matrix[i][j].prob_pos_LtoR = 0;
		Chi_dpnd_matrix[i][j].prob_pos_RtoL = 0;
		/* calculate pos probability */
		if (pos_rule != NULL) {
		    for (k = 0; k < Chi_dpnd_matrix[i][j].count_4; k++) {
			Chi_dpnd_matrix[i][j].prob_pos_LtoR += Chi_dpnd_matrix[i][j].prob_LtoR_4[k];
			Chi_dpnd_matrix[i][j].prob_pos_RtoL += Chi_dpnd_matrix[i][j].prob_RtoL_4[k];
			Chi_dpnd_matrix[i][j].occur_pos = Chi_dpnd_matrix[i][j].occur_4[k];
		    }
		}
		Chi_dpnd_matrix[i][j].prob_pos_LtoR = 1.0 * Chi_dpnd_matrix[i][j].prob_pos_LtoR / Chi_dpnd_matrix[i][j].occur_pos;
		Chi_dpnd_matrix[i][j].prob_pos_RtoL = 1.0 * Chi_dpnd_matrix[i][j].prob_pos_RtoL / Chi_dpnd_matrix[i][j].occur_pos;

		/* calculate probability */
		if (lex_rule != NULL) {
		    Chi_dpnd_matrix[i][j].count = Chi_dpnd_matrix[i][j].count_1;
		    for (k = 0; k < Chi_dpnd_matrix[i][j].count; k++) {
			Chi_dpnd_matrix[i][j].lamda1[k] = (1.0 * Chi_dpnd_matrix[i][j].occur_1[k])/(Chi_dpnd_matrix[i][j].occur_1[k] + 1);
			appear_LtoR_2 = 0;
			appear_RtoL_2 = 0;
			appear_LtoR_3 = 0;
			appear_RtoL_3 = 0;
			total_2 = 0;
			total_3 = 0;
			for (l = 0; l < Chi_dpnd_matrix[i][j].count_2; l++) {
			    if (!strcmp(Chi_dpnd_matrix[i][j].type_1[k], Chi_dpnd_matrix[i][j].type_2[l])) {
				appear_LtoR_2 = Chi_dpnd_matrix[i][j].prob_LtoR_2[l];
				appear_RtoL_2 = Chi_dpnd_matrix[i][j].prob_RtoL_2[l];
				total_2 = Chi_dpnd_matrix[i][j].occur_2[l];
				break;
			    }
			}
			for (l = 0; l < Chi_dpnd_matrix[i][j].count_3; l++) {
			    if (!strcmp(Chi_dpnd_matrix[i][j].type_1[k], Chi_dpnd_matrix[i][j].type_3[l])) {
				appear_LtoR_3 = Chi_dpnd_matrix[i][j].prob_LtoR_3[l];
				appear_RtoL_3 = Chi_dpnd_matrix[i][j].prob_RtoL_3[l];
				total_3 = Chi_dpnd_matrix[i][j].occur_3[l];
				break;
			    }
			}
			if (total_2 != 0 || total_3 != 0) {
			    Chi_dpnd_matrix[i][j].prob_LtoR[k] = 1.0 * Chi_dpnd_matrix[i][j].lamda1[k] * (1.0 * Chi_dpnd_matrix[i][j].prob_LtoR_1[k]/Chi_dpnd_matrix[i][j].occur_1[k]) +
				(1.0 * (1 - Chi_dpnd_matrix[i][j].lamda1[k]) * (1.0 * (appear_LtoR_2 + appear_LtoR_3)/(total_2 + total_3)));
			    Chi_dpnd_matrix[i][j].prob_RtoL[k] = 1.0 * Chi_dpnd_matrix[i][j].lamda1[k] * (1.0 * Chi_dpnd_matrix[i][j].prob_RtoL_1[k]/Chi_dpnd_matrix[i][j].occur_1[k]) +
				(1.0 * (1 - Chi_dpnd_matrix[i][j].lamda1[k]) * (1.0 * (appear_RtoL_2 + appear_RtoL_3)/(total_2 + total_3)));
			}
			else {
			    Chi_dpnd_matrix[i][j].prob_LtoR[k] = 1.0 * Chi_dpnd_matrix[i][j].lamda1[k] * (1.0 * Chi_dpnd_matrix[i][j].prob_LtoR_1[k]/Chi_dpnd_matrix[i][j].occur_1[k]);
			    Chi_dpnd_matrix[i][j].prob_RtoL[k] = 1.0 * Chi_dpnd_matrix[i][j].lamda1[k] * (1.0 * Chi_dpnd_matrix[i][j].prob_RtoL_1[k]/Chi_dpnd_matrix[i][j].occur_1[k]);
			}

			Chi_dpnd_matrix[i][j].direction[k] = Chi_dpnd_matrix[i][j].direction_1[k];
			strcpy(Chi_dpnd_matrix[i][j].type[k],Chi_dpnd_matrix[i][j].type_1[k]);
		    }
		}
		else if (pos_rule_1 != NULL || pos_rule_2 != NULL) {
		    if (pos_rule_1 != NULL) {
			Chi_dpnd_matrix[i][j].count = Chi_dpnd_matrix[i][j].count_2;
			for (k = 0; k < Chi_dpnd_matrix[i][j].count; k++) {
			    appear_LtoR_2 = Chi_dpnd_matrix[i][j].prob_LtoR_2[k];
			    appear_RtoL_2 = Chi_dpnd_matrix[i][j].prob_RtoL_2[k];
			    appear_LtoR_3 = 0;
			    appear_RtoL_3 = 0;
			    total_2 = Chi_dpnd_matrix[i][j].occur_2[k];
			    total_3 = 0;
			    for (l = 0; l < Chi_dpnd_matrix[i][j].count_3; l++) {
				if (!strcmp(Chi_dpnd_matrix[i][j].type_2[k], Chi_dpnd_matrix[i][j].type_3[l])) {
				    appear_LtoR_3 = Chi_dpnd_matrix[i][j].prob_LtoR_3[l];
				    appear_RtoL_3 = Chi_dpnd_matrix[i][j].prob_RtoL_3[l];
				    total_3 = Chi_dpnd_matrix[i][j].occur_3[l];
				    break;
				}
			    }
			    Chi_dpnd_matrix[i][j].lamda2[k] = (1.0 * (total_2 + total_3))/(total_2 + total_3 + 1);
			    Chi_dpnd_matrix[i][j].prob_LtoR[k] = 1.0 * Chi_dpnd_matrix[i][j].lamda2[k] * (1.0 * (appear_LtoR_2 + appear_LtoR_3)/(total_2 + total_3));
			    Chi_dpnd_matrix[i][j].prob_RtoL[k] = 1.0 * Chi_dpnd_matrix[i][j].lamda2[k] * (1.0 * (appear_RtoL_2 + appear_RtoL_3)/(total_2 + total_3));
			    strcpy(Chi_dpnd_matrix[i][j].type[k],Chi_dpnd_matrix[i][j].type_2[k]);
			    for (l = 0; l < Chi_dpnd_matrix[i][j].count_4; l++) {
				if (!strcmp(Chi_dpnd_matrix[i][j].type_2[k], Chi_dpnd_matrix[i][j].type_4[l])) {
				    Chi_dpnd_matrix[i][j].prob_LtoR[k] += (1 - Chi_dpnd_matrix[i][j].lamda2[k]) * (1.0 * Chi_dpnd_matrix[i][j].prob_LtoR_4[l] / Chi_dpnd_matrix[i][j].occur_4[l]);
				    Chi_dpnd_matrix[i][j].prob_RtoL[k] += (1 - Chi_dpnd_matrix[i][j].lamda2[k]) * (1.0 * Chi_dpnd_matrix[i][j].prob_RtoL_4[l] / Chi_dpnd_matrix[i][j].occur_4[l]);
				    break;
				}
			    }
			    Chi_dpnd_matrix[i][j].direction[k] = Chi_dpnd_matrix[i][j].direction_2[k];

			    Chi_dpnd_matrix[i][j].prob_LtoR[k] *= bkoff_weight_1;
			    Chi_dpnd_matrix[i][j].prob_RtoL[k] *= bkoff_weight_1;
			}
		    }
		    else if (pos_rule_2 != NULL) {
			Chi_dpnd_matrix[i][j].count = Chi_dpnd_matrix[i][j].count_3;
			for (k = 0; k < Chi_dpnd_matrix[i][j].count; k++) {
			    Chi_dpnd_matrix[i][j].lamda2[k] = (1.0 * Chi_dpnd_matrix[i][j].occur_3[k]) / (Chi_dpnd_matrix[i][j].occur_3[k] + 1);
			    Chi_dpnd_matrix[i][j].prob_LtoR[k] = 1.0 * Chi_dpnd_matrix[i][j].lamda2[k] * (1.0 * Chi_dpnd_matrix[i][j].prob_LtoR_3[k] / Chi_dpnd_matrix[i][j].occur_3[k]);
			    Chi_dpnd_matrix[i][j].prob_RtoL[k] = 1.0 * Chi_dpnd_matrix[i][j].lamda2[k] * (1.0 * Chi_dpnd_matrix[i][j].prob_RtoL_3[k] / Chi_dpnd_matrix[i][j].occur_3[k]);
			    strcpy(Chi_dpnd_matrix[i][j].type[k], Chi_dpnd_matrix[i][j].type_3[k]);
			    for (l = 0; l < Chi_dpnd_matrix[i][j].count_4; l++) {
				if (!strcmp(Chi_dpnd_matrix[i][j].type_3[k], Chi_dpnd_matrix[i][j].type_4[l])) {
				    Chi_dpnd_matrix[i][j].prob_LtoR[k] += (1 - Chi_dpnd_matrix[i][j].lamda2[k]) * (1.0 * Chi_dpnd_matrix[i][j].prob_LtoR_4[l] / Chi_dpnd_matrix[i][j].occur_4[k]);
				    Chi_dpnd_matrix[i][j].prob_RtoL[k] += (1 - Chi_dpnd_matrix[i][j].lamda2[k]) * (1.0 * Chi_dpnd_matrix[i][j].prob_RtoL_4[l] / Chi_dpnd_matrix[i][j].occur_4[k]);
				    break;
				}
			    }
			    Chi_dpnd_matrix[i][j].direction[k] = Chi_dpnd_matrix[i][j].direction_3[k];

			    Chi_dpnd_matrix[i][j].prob_LtoR[k] *= bkoff_weight_1;
			    Chi_dpnd_matrix[i][j].prob_RtoL[k] *= bkoff_weight_1;
			}
		    }
		}
		else {
		    Chi_dpnd_matrix[i][j].count = Chi_dpnd_matrix[i][j].count_4;
		    for (k = 0; k < Chi_dpnd_matrix[i][j].count; k++) {
			Chi_dpnd_matrix[i][j].prob_LtoR[k] = (1.0 * Chi_dpnd_matrix[i][j].prob_LtoR_4[k]) / Chi_dpnd_matrix[i][j].occur_4[k];
			Chi_dpnd_matrix[i][j].prob_RtoL[k] = (1.0 * Chi_dpnd_matrix[i][j].prob_RtoL_4[k]) / Chi_dpnd_matrix[i][j].occur_4[k];
			Chi_dpnd_matrix[i][j].direction[k] = Chi_dpnd_matrix[i][j].direction_4[k];

			Chi_dpnd_matrix[i][j].prob_LtoR[k] *= bkoff_weight_2;
			Chi_dpnd_matrix[i][j].prob_RtoL[k] *= bkoff_weight_2;

			strcpy(Chi_dpnd_matrix[i][j].type[k], Chi_dpnd_matrix[i][j].type_4[k]);
		    }
		}

		if (lex_rule != NULL || pos_rule_1 != NULL || pos_rule_2 != NULL || pos_rule != NULL) {
		    first_uke_flag = 0;
		    if (Chi_dpnd_matrix[i][j].count > 0) {
			Dpnd_matrix[i][j] = 'O';
		    }
		}
	    }
	}
    }

    if (Language == CHINESE) {
	/* free memory */
	if (lex_rule) {
	    free(lex_rule);
	}
	if (pos_rule_1) {
	    free(pos_rule_1);
	}
	if (pos_rule_2) {
	    free(pos_rule_2);
	}
	if (pos_rule) {
	    free(pos_rule);
	}
	if (rule) {
	    free(rule);
	}
    }
}

/* calculate dpnd probability & root probability for Chinese probabilistic model */
/*==================================================================*/
       void calc_chi_dpnd_matrix_forProbModel(SENTENCE_DATA *sp)
/*==================================================================*/
{
    int i, j, k;
    BNST_DATA *k_ptr, *u_ptr;
    char *lex_rule, *pos_rule_1, *pos_rule_2, *pos_rule;
    char *probLtoR, *probRtoL, *occurLtoR, *occurRtoL, *dpnd, *direction, *rule;
    char *curRule[CHI_DPND_TYPE_MAX];
    double bkoff_weight_1, bkoff_weight_2, dpnd_giga_weight, comma_giga_weight, dis_giga_weight, dis_bkoff_weight_1, dis_bkoff_weight_2;
    int count;
    double lex_root_prob[2], pos_root_prob[2], lex_root_occur[2], pos_root_occur[2], lamda_root, lamda_dpnd, lamda_dis, lamda_comma;
    double commaTotal_1, commaTotal_2, commaTotal_3, commaTotal_4;
    char *comma0, *comma1, *dis, *disTotal;

    /* initialization */
    lex_rule = NULL;
    pos_rule_1 = NULL;
    pos_rule_2 = NULL;
    pos_rule = NULL;

    rule = NULL;
    bkoff_weight_1 = 0.8;
    bkoff_weight_2 = 0.5;
    dpnd_giga_weight = 0.8;
    comma_giga_weight = 0.8;
    dis_giga_weight = 0;
    dis_bkoff_weight_1 = 0.8;
    dis_bkoff_weight_2 = 0.5;

    for (i = 0; i < sp->Bnst_num; i++) {
	// initialization
	for (j = 0; j < sp->Bnst_num; j++) {
	    for (k = 0; k < sp->Bnst_num; k++) {
		Chi_dpnd_stru_matrix[i][j][k].prob_vpn = -1;
	    }
	}

	k_ptr = sp->bnst_data + i;

	/* get rule for root */
	/* initialization */
	lex_rule = NULL;
	pos_rule = NULL;
	lex_root_prob[0] = 0;
	lex_root_prob[1] = 0;
	lex_root_occur[0] = 0;
	lex_root_occur[1] = 0;
	pos_root_prob[0] = 0;
	pos_root_prob[1] = 0;
	pos_root_occur[0] = 0;
	pos_root_occur[1] = 0;
	
	/* get root rule */
	lex_rule = get_chi_dpnd_rule(k_ptr->head_ptr->Goi, k_ptr->head_ptr->Pos, "ROOT", "", 0, 0);
	pos_rule = get_chi_dpnd_rule("XX", k_ptr->head_ptr->Pos, "ROOT", "", 0, 0);

	if (lex_rule != NULL) {
	    count = 0;
	    rule = NULL;
	    rule = strtok(lex_rule, ":");
	    while (rule) {
		curRule[count] = malloc(strlen(rule) + 1);
		strcpy(curRule[count], rule);
		count++;
		rule = NULL;
		rule = strtok(NULL, ":");
	    }

	    for (k = 0; k < count; k++) {
		probLtoR = NULL;
		occurLtoR = NULL;
		dpnd = NULL;
			    
		probLtoR = strtok(curRule[k], "_");
		occurLtoR = strtok(NULL, "_");
		dpnd = strtok(NULL, "_");

		if (!strcmp(dpnd, "TRAIN")) {
		    lex_root_prob[0] = atof(probLtoR);
		    lex_root_occur[0] = atof(occurLtoR);
		}
		else if (!strcmp(dpnd, "GIGA")) {
		    lex_root_prob[1] = atof(probLtoR);
		    lex_root_occur[1] = atof(occurLtoR);
		}
		if (curRule[k]) {
		    free(curRule[k]);
		}
	    }
	}

	if (pos_rule != NULL) {
	    count = 0;
	    rule = NULL;
	    rule = strtok(pos_rule, ":");
	    while (rule) {
		curRule[count] = malloc(strlen(rule) + 1);
		strcpy(curRule[count], rule);
		count++;
		rule = NULL;
		rule = strtok(NULL, ":");
	    }

	    for (k = 0; k < count; k++) {
		probLtoR = NULL;
		occurLtoR = NULL;
		dpnd = NULL;
			    
		probLtoR = strtok(curRule[k], "_");
		occurLtoR = strtok(NULL, "_");
		dpnd = strtok(NULL, "_");

		if (!strcmp(dpnd, "TRAIN")) {
		    pos_root_prob[0] = atof(probLtoR);
		    pos_root_occur[0] = atof(occurLtoR);
		}
		else if (!strcmp(dpnd, "GIGA")) {
		    pos_root_prob[1] = atof(probLtoR);
		    pos_root_occur[1] = atof(occurLtoR);
		}
		if (curRule[k]) {
		    free(curRule[k]);
		}
	    }
	}

	/* calculate root prob */
	lamda_root = 0.7;

	if (lex_root_occur[0] != 0 && pos_root_occur[0] != 0) {
	    Chi_root_prob_matrix[i] = (lamda_root * lex_root_prob[0] / lex_root_occur[0]) + ((1 - lamda_root) * (pos_root_prob[0] / pos_root_occur[0]));
	}
	else if (pos_root_occur[0] != 0) {
	    Chi_root_prob_matrix[i] = (1 - lamda_root) * (pos_root_prob[0] / pos_root_occur[0]);
	}
	else {
	    Chi_root_prob_matrix[i] = 0;
	}

	/* get dpnd rule for word pair */
	for (j = i + 1; j < sp->Bnst_num; j++) {
	    u_ptr = sp->bnst_data + j;

	    /* get dis and comma rule */
	    lex_rule = NULL;
	    pos_rule_1 = NULL;
	    pos_rule_2 = NULL;
	    pos_rule = NULL;
	     
	    Chi_dpnd_matrix[i][j].prob_dis_1[0] = 0;
	    Chi_dpnd_matrix[i][j].occur_dis_1[0] = 0;
	    Chi_dpnd_matrix[i][j].prob_neg_dis_1[0] = 0;
	    Chi_dpnd_matrix[i][j].occur_neg_dis_1[0] = 0;
	    Chi_dpnd_matrix[i][j].prob_comma0_1[0] = 0;
	    Chi_dpnd_matrix[i][j].prob_comma1_1[0] = 0;
	    Chi_dpnd_matrix[i][j].prob_neg_comma0_1[0] = 0;
	    Chi_dpnd_matrix[i][j].prob_neg_comma1_1[0] = 0;
	    Chi_dpnd_matrix[i][j].prob_dis_1[1] = 0;
	    Chi_dpnd_matrix[i][j].occur_dis_1[1] = 0;
	    Chi_dpnd_matrix[i][j].prob_neg_dis_1[1] = 0;
	    Chi_dpnd_matrix[i][j].occur_neg_dis_1[1] = 0;
	    Chi_dpnd_matrix[i][j].prob_comma0_1[1] = 0;
	    Chi_dpnd_matrix[i][j].prob_comma1_1[1] = 0;
	    Chi_dpnd_matrix[i][j].prob_neg_comma0_1[1] = 0;
	    Chi_dpnd_matrix[i][j].prob_neg_comma1_1[1] = 0;

	    Chi_dpnd_matrix[i][j].prob_dis_2[0] = 0;
	    Chi_dpnd_matrix[i][j].occur_dis_2[0] = 0;
	    Chi_dpnd_matrix[i][j].prob_neg_dis_2[0] = 0;
	    Chi_dpnd_matrix[i][j].occur_neg_dis_2[0] = 0;
	    Chi_dpnd_matrix[i][j].prob_comma0_2[0] = 0;
	    Chi_dpnd_matrix[i][j].prob_comma1_2[0] = 0;
	    Chi_dpnd_matrix[i][j].prob_neg_comma0_2[0] = 0;
	    Chi_dpnd_matrix[i][j].prob_neg_comma1_2[0] = 0;
	    Chi_dpnd_matrix[i][j].prob_dis_2[1] = 0;
	    Chi_dpnd_matrix[i][j].occur_dis_2[1] = 0;
	    Chi_dpnd_matrix[i][j].prob_neg_dis_2[1] = 0;
	    Chi_dpnd_matrix[i][j].occur_neg_dis_2[1] = 0;
	    Chi_dpnd_matrix[i][j].prob_comma0_2[1] = 0;
	    Chi_dpnd_matrix[i][j].prob_comma1_2[1] = 0;
	    Chi_dpnd_matrix[i][j].prob_neg_comma0_2[1] = 0;
	    Chi_dpnd_matrix[i][j].prob_neg_comma1_2[1] = 0;

	    Chi_dpnd_matrix[i][j].prob_dis_3[0] = 0;
	    Chi_dpnd_matrix[i][j].occur_dis_3[0] = 0;
	    Chi_dpnd_matrix[i][j].prob_neg_dis_3[0] = 0;
	    Chi_dpnd_matrix[i][j].occur_neg_dis_3[0] = 0;
	    Chi_dpnd_matrix[i][j].prob_comma0_3[0] = 0;
	    Chi_dpnd_matrix[i][j].prob_comma1_3[0] = 0;
	    Chi_dpnd_matrix[i][j].prob_neg_comma0_3[0] = 0;
	    Chi_dpnd_matrix[i][j].prob_neg_comma1_3[0] = 0;
	    Chi_dpnd_matrix[i][j].prob_dis_3[1] = 0;
	    Chi_dpnd_matrix[i][j].occur_dis_3[1] = 0;
	    Chi_dpnd_matrix[i][j].prob_neg_dis_3[1] = 0;
	    Chi_dpnd_matrix[i][j].occur_neg_dis_3[1] = 0;
	    Chi_dpnd_matrix[i][j].prob_comma0_3[1] = 0;
	    Chi_dpnd_matrix[i][j].prob_comma1_3[1] = 0;
	    Chi_dpnd_matrix[i][j].prob_neg_comma0_3[1] = 0;
	    Chi_dpnd_matrix[i][j].prob_neg_comma1_3[1] = 0;

	    Chi_dpnd_matrix[i][j].prob_dis_4[0] = 0;
	    Chi_dpnd_matrix[i][j].occur_dis_4[0] = 0;
	    Chi_dpnd_matrix[i][j].prob_neg_dis_4[0] = 0;
	    Chi_dpnd_matrix[i][j].occur_neg_dis_4[0] = 0;
	    Chi_dpnd_matrix[i][j].prob_comma0_4[0] = 0;
	    Chi_dpnd_matrix[i][j].prob_comma1_4[0] = 0;
	    Chi_dpnd_matrix[i][j].prob_neg_comma0_4[0] = 0;
	    Chi_dpnd_matrix[i][j].prob_neg_comma1_4[0] = 0;
	    Chi_dpnd_matrix[i][j].prob_dis_4[1] = 0;
	    Chi_dpnd_matrix[i][j].occur_dis_4[1] = 0;
	    Chi_dpnd_matrix[i][j].prob_neg_dis_4[1] = 0;
	    Chi_dpnd_matrix[i][j].occur_neg_dis_4[1] = 0;
	    Chi_dpnd_matrix[i][j].prob_comma0_4[1] = 0;
	    Chi_dpnd_matrix[i][j].prob_comma1_4[1] = 0;
	    Chi_dpnd_matrix[i][j].prob_neg_comma0_4[1] = 0;
	    Chi_dpnd_matrix[i][j].prob_neg_comma1_4[1] = 0;

	    Chi_dpnd_matrix[i][j].prob_dis = 0;
	    Chi_dpnd_matrix[i][j].prob_neg_dis = 0;
	    Chi_dpnd_matrix[i][j].prob_comma0 = 0;
	    Chi_dpnd_matrix[i][j].prob_comma1 = 0;
	    Chi_dpnd_matrix[i][j].prob_neg_comma0 = 0;
	    Chi_dpnd_matrix[i][j].prob_neg_comma1 = 0;
	    Chi_dpnd_matrix[i][j].prob_dis = 0;
	    Chi_dpnd_matrix[i][j].prob_neg_dis = 0;
	    Chi_dpnd_matrix[i][j].prob_comma0 = 0;
	    Chi_dpnd_matrix[i][j].prob_comma1 = 0;
	    Chi_dpnd_matrix[i][j].prob_neg_comma0 = 0;
	    Chi_dpnd_matrix[i][j].prob_neg_comma1 = 0;

	    lex_rule = get_chi_dpnd_rule(k_ptr->head_ptr->Goi, k_ptr->head_ptr->Pos, u_ptr->head_ptr->Goi, u_ptr->head_ptr->Pos, j - i, 1);
	    pos_rule_1 = get_chi_dpnd_rule(k_ptr->head_ptr->Goi, k_ptr->head_ptr->Pos, "XX", u_ptr->head_ptr->Pos, j - i, 3);
	    pos_rule_2 = get_chi_dpnd_rule("XX", k_ptr->head_ptr->Pos, u_ptr->head_ptr->Goi, u_ptr->head_ptr->Pos, j - i, 2);
	    pos_rule = get_chi_dpnd_rule("XX", k_ptr->head_ptr->Pos, "XX", u_ptr->head_ptr->Pos, j - i, 4);

	    if (lex_rule != NULL) {
		count = 0;
		rule = NULL;
		rule = strtok(lex_rule, ":");
		while (rule) {
		    curRule[count] = malloc(strlen(rule) + 1);
		    strcpy(curRule[count], rule);
		    count++;
		    rule = NULL;
		    rule = strtok(NULL, ":");
		}

		for (k = 0; k < count; k++) {
		    comma0 = NULL;
		    comma1 = NULL;
		    dis = NULL;
		    disTotal = NULL;
		    dpnd = NULL;
			    
		    comma0 = strtok(curRule[k], "_");
		    comma1 = strtok(NULL, "_");
		    dis = strtok(NULL, "_");
		    disTotal = strtok(NULL, "_");
		    dpnd = strtok(NULL, "_");
			    
		    if (!strcmp(dpnd, "TRAIN")) {
			Chi_dpnd_matrix[i][j].prob_comma0_1[0] = atof(comma0);
			Chi_dpnd_matrix[i][j].prob_comma1_1[0] = atof(comma1);
			Chi_dpnd_matrix[i][j].prob_dis_1[0] = atof(dis);
			Chi_dpnd_matrix[i][j].occur_dis_1[0] = atof(disTotal);
		    }
		    else if (!strcmp(dpnd, "GIGA")) {
			Chi_dpnd_matrix[i][j].prob_comma0_1[1] = atof(comma0);
			Chi_dpnd_matrix[i][j].prob_comma1_1[1] = atof(comma1);
			Chi_dpnd_matrix[i][j].prob_dis_1[1] = atof(dis);
			Chi_dpnd_matrix[i][j].occur_dis_1[1] = atof(disTotal);
		    }
		    if (curRule[k]) {
			free(curRule[k]);
		    }
		}
	    }
	    if (pos_rule_1 != NULL) {
		count = 0;
		rule = NULL;
		rule = strtok(pos_rule_1, ":");
		while (rule) {
		    curRule[count] = malloc(strlen(rule) + 1);
		    strcpy(curRule[count], rule);
		    count++;
		    rule = NULL;
		    rule = strtok(NULL, ":");
		}

		for (k = 0; k < count; k++) {
		    comma0 = NULL;
		    comma1 = NULL;
		    dis = NULL;
		    disTotal = NULL;
		    dpnd = NULL;
			    
		    comma0 = strtok(curRule[k], "_");
		    comma1 = strtok(NULL, "_");
		    dis = strtok(NULL, "_");
		    disTotal = strtok(NULL, "_");
		    dpnd = strtok(NULL, "_");
			    
		    if (!strcmp(dpnd, "TRAIN")) {
			Chi_dpnd_matrix[i][j].prob_comma0_2[0] = atof(comma0);
			Chi_dpnd_matrix[i][j].prob_comma1_2[0] = atof(comma1);
			Chi_dpnd_matrix[i][j].prob_dis_2[0] = atof(dis);
			Chi_dpnd_matrix[i][j].occur_dis_2[0] = atof(disTotal);
		    }
		    else if (!strcmp(dpnd, "GIGA")) {
			Chi_dpnd_matrix[i][j].prob_comma0_2[1] = atof(comma0);
			Chi_dpnd_matrix[i][j].prob_comma1_2[1] = atof(comma1);
			Chi_dpnd_matrix[i][j].prob_dis_2[1] = atof(dis);
			Chi_dpnd_matrix[i][j].occur_dis_2[1] = atof(disTotal);
		    }
		    if (curRule[k]) {
			free(curRule[k]);
		    }
		}
	    }

	    if (pos_rule_2 != NULL) {
		count = 0;
		rule = NULL;
		rule = strtok(pos_rule_2, ":");
		while (rule) {
		    curRule[count] = malloc(strlen(rule) + 1);
		    strcpy(curRule[count], rule);
		    count++;
		    rule = NULL;
		    rule = strtok(NULL, ":");
		}

		for (k = 0; k < count; k++) {
		    comma0 = NULL;
		    comma1 = NULL;
		    dis = NULL;
		    disTotal = NULL;
		    dpnd = NULL;
			    
		    comma0 = strtok(curRule[k], "_");
		    comma1 = strtok(NULL, "_");
		    dis = strtok(NULL, "_");
		    disTotal = strtok(NULL, "_");
		    dpnd = strtok(NULL, "_");
			    
		    if (!strcmp(dpnd, "TRAIN")) {
			Chi_dpnd_matrix[i][j].prob_comma0_3[0] = atof(comma0);
			Chi_dpnd_matrix[i][j].prob_comma1_3[0] = atof(comma1);
			Chi_dpnd_matrix[i][j].prob_dis_3[0] = atof(dis);
			Chi_dpnd_matrix[i][j].occur_dis_3[0] = atof(disTotal);
		    }
		    else if (!strcmp(dpnd, "GIGA")) {
			Chi_dpnd_matrix[i][j].prob_comma0_3[1] = atof(comma0);
			Chi_dpnd_matrix[i][j].prob_comma1_3[1] = atof(comma1);
			Chi_dpnd_matrix[i][j].prob_dis_3[1] = atof(dis);
			Chi_dpnd_matrix[i][j].occur_dis_3[1] = atof(disTotal);
		    }
		    if (curRule[k]) {
			free(curRule[k]);
		    }
		}
	    }

	    if (pos_rule != NULL) {
		count = 0;
		rule = NULL;
		rule = strtok(pos_rule, ":");
		while (rule) {
		    curRule[count] = malloc(strlen(rule) + 1);
		    strcpy(curRule[count], rule);
		    count++;
		    rule = NULL;
		    rule = strtok(NULL, ":");
		}

		for (k = 0; k < count; k++) {
		    comma0 = NULL;
		    comma1 = NULL;
		    dis = NULL;
		    disTotal = NULL;
		    dpnd = NULL;
			    
		    comma0 = strtok(curRule[k], "_");
		    comma1 = strtok(NULL, "_");
		    dis = strtok(NULL, "_");
		    disTotal = strtok(NULL, "_");
		    dpnd = strtok(NULL, "_");
			    
		    if (!strcmp(dpnd, "TRAIN")) {
			Chi_dpnd_matrix[i][j].prob_comma0_4[0] = atof(comma0);
			Chi_dpnd_matrix[i][j].prob_comma1_4[0] = atof(comma1);
			Chi_dpnd_matrix[i][j].prob_dis_4[0] = atof(dis);
			Chi_dpnd_matrix[i][j].occur_dis_4[0] = atof(disTotal);
		    }
		    else if (!strcmp(dpnd, "GIGA")) {
			Chi_dpnd_matrix[i][j].prob_comma0_4[1] = atof(comma0);
			Chi_dpnd_matrix[i][j].prob_comma1_4[1] = atof(comma1);
			Chi_dpnd_matrix[i][j].prob_dis_4[1] = atof(dis);
			Chi_dpnd_matrix[i][j].occur_dis_4[1] = atof(disTotal);
		    }
		    if (curRule[k]) {
			free(curRule[k]);
		    }
		}
	    }

	    lex_rule = NULL;
	    pos_rule_1 = NULL;
	    pos_rule_2 = NULL;
	    pos_rule = NULL;

	    lex_rule = get_chi_dpnd_rule(k_ptr->head_ptr->Goi, k_ptr->head_ptr->Pos, u_ptr->head_ptr->Goi, u_ptr->head_ptr->Pos, i - j, 1);
	    pos_rule_1 = get_chi_dpnd_rule(k_ptr->head_ptr->Goi, k_ptr->head_ptr->Pos, "XX", u_ptr->head_ptr->Pos, i - j, 3);
	    pos_rule_2 = get_chi_dpnd_rule("XX", k_ptr->head_ptr->Pos, u_ptr->head_ptr->Goi, u_ptr->head_ptr->Pos, i - j, 2);
	    pos_rule = get_chi_dpnd_rule("XX", k_ptr->head_ptr->Pos, "XX", u_ptr->head_ptr->Pos, i - j, 4);

	    if (lex_rule != NULL) {
		count = 0;
		rule = NULL;
		rule = strtok(lex_rule, ":");
		while (rule) {
		    curRule[count] = malloc(strlen(rule) + 1);
		    strcpy(curRule[count], rule);
		    count++;
		    rule = NULL;
		    rule = strtok(NULL, ":");
		}

		for (k = 0; k < count; k++) {
		    comma0 = NULL;
		    comma1 = NULL;
		    dis = NULL;
		    disTotal = NULL;
		    dpnd = NULL;
			    
		    comma0 = strtok(curRule[k], "_");
		    comma1 = strtok(NULL, "_");
		    dis = strtok(NULL, "_");
		    disTotal = strtok(NULL, "_");
		    dpnd = strtok(NULL, "_");
			    
		    if (!strcmp(dpnd, "TRAIN")) {
			Chi_dpnd_matrix[i][j].prob_neg_comma0_1[0] = atof(comma0);
			Chi_dpnd_matrix[i][j].prob_neg_comma1_1[0] = atof(comma1);
			Chi_dpnd_matrix[i][j].prob_neg_dis_1[0] = atof(dis);
			Chi_dpnd_matrix[i][j].occur_neg_dis_1[0] = atof(disTotal);
		    }
		    else if (!strcmp(dpnd, "GIGA")) {
			Chi_dpnd_matrix[i][j].prob_neg_comma0_1[1] = atof(comma0);
			Chi_dpnd_matrix[i][j].prob_neg_comma1_1[1] = atof(comma1);
			Chi_dpnd_matrix[i][j].prob_neg_dis_1[1] = atof(dis);
			Chi_dpnd_matrix[i][j].occur_neg_dis_1[1] = atof(disTotal);
		    }
		    if (curRule[k]) {
			free(curRule[k]);
		    }
		}
	    }
	    if (pos_rule_1 != NULL) {
		count = 0;
		rule = NULL;
		rule = strtok(pos_rule_1, ":");
		while (rule) {
		    curRule[count] = malloc(strlen(rule) + 1);
		    strcpy(curRule[count], rule);
		    count++;
		    rule = NULL;
		    rule = strtok(NULL, ":");
		}

		for (k = 0; k < count; k++) {
		    comma0 = NULL;
		    comma1 = NULL;
		    dis = NULL;
		    disTotal = NULL;
		    dpnd = NULL;
			    
		    comma0 = strtok(curRule[k], "_");
		    comma1 = strtok(NULL, "_");
		    dis = strtok(NULL, "_");
		    disTotal = strtok(NULL, "_");
		    dpnd = strtok(NULL, "_");
			    
		    if (!strcmp(dpnd, "TRAIN")) {
			Chi_dpnd_matrix[i][j].prob_neg_comma0_2[0] = atof(comma0);
			Chi_dpnd_matrix[i][j].prob_neg_comma1_2[0] = atof(comma1);
			Chi_dpnd_matrix[i][j].prob_neg_dis_2[0] = atof(dis);
			Chi_dpnd_matrix[i][j].occur_neg_dis_2[0] = atof(disTotal);
		    }
		    else if (!strcmp(dpnd, "GIGA")) {
			Chi_dpnd_matrix[i][j].prob_neg_comma0_2[1] = atof(comma0);
			Chi_dpnd_matrix[i][j].prob_neg_comma1_2[1] = atof(comma1);
			Chi_dpnd_matrix[i][j].prob_neg_dis_2[1] = atof(dis);
			Chi_dpnd_matrix[i][j].occur_neg_dis_2[1] = atof(disTotal);
		    }
		    if (curRule[k]) {
			free(curRule[k]);
		    }
		}
	    }

	    if (pos_rule_2 != NULL) {
		count = 0;
		rule = NULL;
		rule = strtok(pos_rule_2, ":");
		while (rule) {
		    curRule[count] = malloc(strlen(rule) + 1);
		    strcpy(curRule[count], rule);
		    count++;
		    rule = NULL;
		    rule = strtok(NULL, ":");
		}

		for (k = 0; k < count; k++) {
		    comma0 = NULL;
		    comma1 = NULL;
		    dis = NULL;
		    disTotal = NULL;
		    dpnd = NULL;
			    
		    comma0 = strtok(curRule[k], "_");
		    comma1 = strtok(NULL, "_");
		    dis = strtok(NULL, "_");
		    disTotal = strtok(NULL, "_");
		    dpnd = strtok(NULL, "_");
			    
		    if (!strcmp(dpnd, "TRAIN")) {
			Chi_dpnd_matrix[i][j].prob_neg_comma0_3[0] = atof(comma0);
			Chi_dpnd_matrix[i][j].prob_neg_comma1_3[0] = atof(comma1);
			Chi_dpnd_matrix[i][j].prob_neg_dis_3[0] = atof(dis);
			Chi_dpnd_matrix[i][j].occur_neg_dis_3[0] = atof(disTotal);
		    }
		    else if (!strcmp(dpnd, "GIGA")) {
			Chi_dpnd_matrix[i][j].prob_neg_comma0_3[1] = atof(comma0);
			Chi_dpnd_matrix[i][j].prob_neg_comma1_3[1] = atof(comma1);
			Chi_dpnd_matrix[i][j].prob_neg_dis_3[1] = atof(dis);
			Chi_dpnd_matrix[i][j].occur_neg_dis_3[1] = atof(disTotal);
		    }
		    if (curRule[k]) {
			free(curRule[k]);
		    }
		}
	    }

	    if (pos_rule != NULL) {
		count = 0;
		rule = NULL;
		rule = strtok(pos_rule, ":");
		while (rule) {
		    curRule[count] = malloc(strlen(rule) + 1);
		    strcpy(curRule[count], rule);
		    count++;
		    rule = NULL;
		    rule = strtok(NULL, ":");
		}

		for (k = 0; k < count; k++) {
		    comma0 = NULL;
		    comma1 = NULL;
		    dis = NULL;
		    disTotal = NULL;
		    dpnd = NULL;
			    
		    comma0 = strtok(curRule[k], "_");
		    comma1 = strtok(NULL, "_");
		    dis = strtok(NULL, "_");
		    disTotal = strtok(NULL, "_");
		    dpnd = strtok(NULL, "_");
			    
		    if (!strcmp(dpnd, "TRAIN")) {
			Chi_dpnd_matrix[i][j].prob_neg_comma0_4[0] = atof(comma0);
			Chi_dpnd_matrix[i][j].prob_neg_comma1_4[0] = atof(comma1);
			Chi_dpnd_matrix[i][j].prob_neg_dis_4[0] = atof(dis);
			Chi_dpnd_matrix[i][j].occur_neg_dis_4[0] = atof(disTotal);
		    }
		    else if (!strcmp(dpnd, "GIGA")) {
			Chi_dpnd_matrix[i][j].prob_neg_comma0_4[1] = atof(comma0);
			Chi_dpnd_matrix[i][j].prob_neg_comma1_4[1] = atof(comma1);
			Chi_dpnd_matrix[i][j].prob_neg_dis_4[1] = atof(dis);
			Chi_dpnd_matrix[i][j].occur_neg_dis_4[1] = atof(disTotal);
		    }
		    if (curRule[k]) {
			free(curRule[k]);
		    }
		}
	    }

	    Chi_dpnd_matrix[i][j].prob_dis_1[0] += dis_giga_weight * Chi_dpnd_matrix[i][j].prob_dis_1[1];
	    Chi_dpnd_matrix[i][j].occur_dis_1[0] += dis_giga_weight * Chi_dpnd_matrix[i][j].occur_dis_1[1];
	    Chi_dpnd_matrix[i][j].prob_neg_dis_1[0] += dis_giga_weight * Chi_dpnd_matrix[i][j].prob_neg_dis_1[1];
	    Chi_dpnd_matrix[i][j].occur_neg_dis_1[0] += dis_giga_weight * Chi_dpnd_matrix[i][j].occur_neg_dis_1[1];
	    Chi_dpnd_matrix[i][j].prob_comma0_1[0] += comma_giga_weight * Chi_dpnd_matrix[i][j].prob_comma0_1[1];
	    Chi_dpnd_matrix[i][j].prob_comma1_1[0] += comma_giga_weight * Chi_dpnd_matrix[i][j].prob_comma1_1[1];
	    Chi_dpnd_matrix[i][j].prob_neg_comma0_1[0] += comma_giga_weight * Chi_dpnd_matrix[i][j].prob_neg_comma0_1[1];
	    Chi_dpnd_matrix[i][j].prob_neg_comma1_1[0] += comma_giga_weight * Chi_dpnd_matrix[i][j].prob_neg_comma1_1[1];

	    Chi_dpnd_matrix[i][j].prob_dis_2[0] += dis_giga_weight * Chi_dpnd_matrix[i][j].prob_dis_2[1];
	    Chi_dpnd_matrix[i][j].occur_dis_2[0] += dis_giga_weight * Chi_dpnd_matrix[i][j].occur_dis_2[1];
	    Chi_dpnd_matrix[i][j].prob_neg_dis_2[0] += dis_giga_weight * Chi_dpnd_matrix[i][j].prob_neg_dis_2[1];
	    Chi_dpnd_matrix[i][j].occur_neg_dis_2[0] += dis_giga_weight * Chi_dpnd_matrix[i][j].occur_neg_dis_2[1];
	    Chi_dpnd_matrix[i][j].prob_comma0_2[0] += comma_giga_weight * Chi_dpnd_matrix[i][j].prob_comma0_2[1];
	    Chi_dpnd_matrix[i][j].prob_comma1_2[0] += comma_giga_weight * Chi_dpnd_matrix[i][j].prob_comma1_2[1];
	    Chi_dpnd_matrix[i][j].prob_neg_comma0_2[0] += comma_giga_weight * Chi_dpnd_matrix[i][j].prob_neg_comma0_2[1];
	    Chi_dpnd_matrix[i][j].prob_neg_comma1_2[0] += comma_giga_weight * Chi_dpnd_matrix[i][j].prob_neg_comma1_2[1];

	    Chi_dpnd_matrix[i][j].prob_dis_3[0] += dis_giga_weight * Chi_dpnd_matrix[i][j].prob_dis_3[1];
	    Chi_dpnd_matrix[i][j].occur_dis_3[0] += dis_giga_weight * Chi_dpnd_matrix[i][j].occur_dis_3[1];
	    Chi_dpnd_matrix[i][j].prob_neg_dis_3[0] += dis_giga_weight * Chi_dpnd_matrix[i][j].prob_neg_dis_3[1];
	    Chi_dpnd_matrix[i][j].occur_neg_dis_3[0] += dis_giga_weight * Chi_dpnd_matrix[i][j].occur_neg_dis_3[1];
	    Chi_dpnd_matrix[i][j].prob_comma0_3[0] += comma_giga_weight * Chi_dpnd_matrix[i][j].prob_comma0_3[1];
	    Chi_dpnd_matrix[i][j].prob_comma1_3[0] += comma_giga_weight * Chi_dpnd_matrix[i][j].prob_comma1_3[1];
	    Chi_dpnd_matrix[i][j].prob_neg_comma0_3[0] += comma_giga_weight * Chi_dpnd_matrix[i][j].prob_neg_comma0_3[1];
	    Chi_dpnd_matrix[i][j].prob_neg_comma1_3[0] += comma_giga_weight * Chi_dpnd_matrix[i][j].prob_neg_comma1_3[1];

	    Chi_dpnd_matrix[i][j].prob_dis_4[0] += dis_giga_weight * Chi_dpnd_matrix[i][j].prob_dis_4[1];
	    Chi_dpnd_matrix[i][j].occur_dis_4[0] += dis_giga_weight * Chi_dpnd_matrix[i][j].occur_dis_4[1];
	    Chi_dpnd_matrix[i][j].prob_neg_dis_4[0] += dis_giga_weight * Chi_dpnd_matrix[i][j].prob_neg_dis_4[1];
	    Chi_dpnd_matrix[i][j].occur_neg_dis_4[0] += dis_giga_weight * Chi_dpnd_matrix[i][j].occur_neg_dis_4[1];
	    Chi_dpnd_matrix[i][j].prob_comma0_4[0] += comma_giga_weight * Chi_dpnd_matrix[i][j].prob_comma0_4[1];
	    Chi_dpnd_matrix[i][j].prob_comma1_4[0] += comma_giga_weight * Chi_dpnd_matrix[i][j].prob_comma1_4[1];
	    Chi_dpnd_matrix[i][j].prob_neg_comma0_4[0] += comma_giga_weight * Chi_dpnd_matrix[i][j].prob_neg_comma0_4[1];
	    Chi_dpnd_matrix[i][j].prob_neg_comma1_4[0] += comma_giga_weight * Chi_dpnd_matrix[i][j].prob_neg_comma1_4[1];

	    /* calculate probability for dis */
	    if (Chi_dpnd_matrix[i][j].prob_dis_1[0] != 0) {
		lamda_dis = Chi_dpnd_matrix[i][j].occur_dis_1[0] / (Chi_dpnd_matrix[i][j].occur_dis_1[0] + 1);
		if (Chi_dpnd_matrix[i][j].prob_dis_2[0] != 0 || Chi_dpnd_matrix[i][j].prob_dis_3[0] != 0) {
		    Chi_dpnd_matrix[i][j].prob_dis = lamda_dis * (Chi_dpnd_matrix[i][j].prob_dis_1[0] / Chi_dpnd_matrix[i][j].occur_dis_1[0]) +
			(1 - lamda_dis) * ((Chi_dpnd_matrix[i][j].prob_dis_2[0] + Chi_dpnd_matrix[i][j].prob_dis_3[0]) / (Chi_dpnd_matrix[i][j].occur_dis_2[0] + Chi_dpnd_matrix[i][j].occur_dis_3[0]));
		}
		else {
		    Chi_dpnd_matrix[i][j].prob_dis = Chi_dpnd_matrix[i][j].prob_dis_1[0] / Chi_dpnd_matrix[i][j].occur_dis_1[0];
		}
	    }
	    else if (Chi_dpnd_matrix[i][j].prob_dis_2[0] != 0 || Chi_dpnd_matrix[i][j].prob_dis_3[0] != 0) {
		lamda_dis = (Chi_dpnd_matrix[i][j].occur_dis_2[0] + Chi_dpnd_matrix[i][j].occur_dis_3[0]) / (Chi_dpnd_matrix[i][j].occur_dis_2[0] + Chi_dpnd_matrix[i][j].occur_dis_3[0] + 1);
		if (Chi_dpnd_matrix[i][j].prob_dis_4[0] != 0) {
		    Chi_dpnd_matrix[i][j].prob_dis = dis_bkoff_weight_1 * lamda_dis * ((Chi_dpnd_matrix[i][j].prob_dis_2[0] + Chi_dpnd_matrix[i][j].prob_dis_3[0]) / (Chi_dpnd_matrix[i][j].occur_dis_2[0] + Chi_dpnd_matrix[i][j].occur_dis_3[0])) + 
			(1 - lamda_dis) * (Chi_dpnd_matrix[i][j].prob_dis_4[0] / Chi_dpnd_matrix[i][j].occur_dis_4[0]);
		}
		else {
		    Chi_dpnd_matrix[i][j].prob_dis = dis_bkoff_weight_1 * (Chi_dpnd_matrix[i][j].prob_dis_2[0] + Chi_dpnd_matrix[i][j].prob_dis_3[0]) / (Chi_dpnd_matrix[i][j].occur_dis_2[0] + Chi_dpnd_matrix[i][j].occur_dis_3[0]);
		}
	    }
	    else if (Chi_dpnd_matrix[i][j].prob_dis_4[0] != 0) {
		Chi_dpnd_matrix[i][j].prob_dis = dis_bkoff_weight_2 * Chi_dpnd_matrix[i][j].prob_dis_4[0] / Chi_dpnd_matrix[i][j].occur_dis_4[0];
	    }

	    if (Chi_dpnd_matrix[i][j].prob_neg_dis_1[0] != 0) {
		lamda_dis = Chi_dpnd_matrix[i][j].occur_neg_dis_1[0] / (Chi_dpnd_matrix[i][j].occur_neg_dis_1[0] + 1);
		if (Chi_dpnd_matrix[i][j].prob_neg_dis_2[0] != 0 || Chi_dpnd_matrix[i][j].prob_neg_dis_3[0] != 0) {
		    Chi_dpnd_matrix[i][j].prob_neg_dis = lamda_dis * (Chi_dpnd_matrix[i][j].prob_neg_dis_1[0] / Chi_dpnd_matrix[i][j].occur_neg_dis_1[0]) +
			(1 - lamda_dis) * ((Chi_dpnd_matrix[i][j].prob_neg_dis_2[0] + Chi_dpnd_matrix[i][j].prob_neg_dis_3[0]) / (Chi_dpnd_matrix[i][j].occur_neg_dis_2[0] + Chi_dpnd_matrix[i][j].occur_neg_dis_3[0]));
		}
		else {
		    Chi_dpnd_matrix[i][j].prob_neg_dis = Chi_dpnd_matrix[i][j].prob_neg_dis_1[0] / Chi_dpnd_matrix[i][j].occur_neg_dis_1[0];
		}
	    }
	    else if (Chi_dpnd_matrix[i][j].prob_neg_dis_2[0] != 0 || Chi_dpnd_matrix[i][j].prob_neg_dis_3[0] != 0) {
		lamda_dis = (Chi_dpnd_matrix[i][j].occur_neg_dis_2[0] + Chi_dpnd_matrix[i][j].occur_neg_dis_3[0]) / (Chi_dpnd_matrix[i][j].occur_neg_dis_2[0] + Chi_dpnd_matrix[i][j].occur_neg_dis_3[0] + 1);
		if (Chi_dpnd_matrix[i][j].prob_neg_dis_4[0] != 0) {
		    Chi_dpnd_matrix[i][j].prob_neg_dis = dis_bkoff_weight_1 * lamda_dis * ((Chi_dpnd_matrix[i][j].prob_neg_dis_2[0] + Chi_dpnd_matrix[i][j].prob_neg_dis_3[0]) / (Chi_dpnd_matrix[i][j].occur_neg_dis_2[0] + Chi_dpnd_matrix[i][j].occur_neg_dis_3[0])) + 
			(1 - lamda_dis) * (Chi_dpnd_matrix[i][j].prob_neg_dis_4[0] / Chi_dpnd_matrix[i][j].occur_neg_dis_4[0]);
		}
		else {
		    Chi_dpnd_matrix[i][j].prob_neg_dis = dis_bkoff_weight_1 * (Chi_dpnd_matrix[i][j].prob_neg_dis_2[0] + Chi_dpnd_matrix[i][j].prob_neg_dis_3[0]) / (Chi_dpnd_matrix[i][j].occur_neg_dis_2[0] + Chi_dpnd_matrix[i][j].occur_neg_dis_3[0]);
		}
	    }
	    else if (Chi_dpnd_matrix[i][j].prob_neg_dis_4[0] != 0) {
		Chi_dpnd_matrix[i][j].prob_neg_dis = dis_bkoff_weight_2 * Chi_dpnd_matrix[i][j].prob_neg_dis_4[0] / Chi_dpnd_matrix[i][j].occur_neg_dis_4[0];
	    }

	    /* calculate probability for comma */
	    commaTotal_1 = Chi_dpnd_matrix[i][j].prob_comma0_1[0] + Chi_dpnd_matrix[i][j].prob_comma1_1[0];
	    commaTotal_2 = Chi_dpnd_matrix[i][j].prob_comma0_2[0] + Chi_dpnd_matrix[i][j].prob_comma1_2[0];
	    commaTotal_3 = Chi_dpnd_matrix[i][j].prob_comma0_3[0] + Chi_dpnd_matrix[i][j].prob_comma1_3[0];
	    commaTotal_4 = Chi_dpnd_matrix[i][j].prob_comma0_4[0] + Chi_dpnd_matrix[i][j].prob_comma1_4[0];

	    if (Chi_dpnd_matrix[i][j].prob_comma0_1[0] != 0) {
		lamda_comma = commaTotal_1 / (commaTotal_1 + 1);
		if (Chi_dpnd_matrix[i][j].prob_comma0_2[0] != 0 || Chi_dpnd_matrix[i][j].prob_comma0_3[0] != 0) {
		    Chi_dpnd_matrix[i][j].prob_comma0 = lamda_comma * (Chi_dpnd_matrix[i][j].prob_comma0_1[0] / commaTotal_1) +
			(1 - lamda_comma) * ((Chi_dpnd_matrix[i][j].prob_comma0_2[0] + Chi_dpnd_matrix[i][j].prob_comma0_3[0]) / (commaTotal_2 + commaTotal_3));
		}
		else {
		    Chi_dpnd_matrix[i][j].prob_comma0 = Chi_dpnd_matrix[i][j].prob_comma0_1[0] / commaTotal_1;
		}
	    }
	    else if (Chi_dpnd_matrix[i][j].prob_comma0_2[0] != 0 || Chi_dpnd_matrix[i][j].prob_comma0_3[0] != 0) {
		lamda_comma = (commaTotal_2 + commaTotal_3) / (commaTotal_2 + commaTotal_3 + 1);
		if (Chi_dpnd_matrix[i][j].prob_comma0_4[0] != 0) {
		    Chi_dpnd_matrix[i][j].prob_comma0 = dis_bkoff_weight_1 * lamda_comma * ((Chi_dpnd_matrix[i][j].prob_comma0_2[0] + Chi_dpnd_matrix[i][j].prob_comma0_3[0]) / (commaTotal_2 + commaTotal_3)) + 
			(1 - lamda_comma) * (Chi_dpnd_matrix[i][j].prob_comma0_4[0] / commaTotal_4);
		}
		else {
		    Chi_dpnd_matrix[i][j].prob_comma0 = dis_bkoff_weight_1 * (Chi_dpnd_matrix[i][j].prob_comma0_2[0] + Chi_dpnd_matrix[i][j].prob_comma0_3[0]) / (commaTotal_2 + commaTotal_3);
		}
	    }
	    else if (Chi_dpnd_matrix[i][j].prob_comma0_4[0] != 0) {
		Chi_dpnd_matrix[i][j].prob_comma0 = dis_bkoff_weight_2 * Chi_dpnd_matrix[i][j].prob_comma0_4[0] / commaTotal_4;
	    }

	    commaTotal_1 = Chi_dpnd_matrix[i][j].prob_neg_comma0_1[0] + Chi_dpnd_matrix[i][j].prob_neg_comma1_1[0];
	    commaTotal_2 = Chi_dpnd_matrix[i][j].prob_neg_comma0_2[0] + Chi_dpnd_matrix[i][j].prob_neg_comma1_2[0];
	    commaTotal_3 = Chi_dpnd_matrix[i][j].prob_neg_comma0_3[0] + Chi_dpnd_matrix[i][j].prob_neg_comma1_3[0];
	    commaTotal_4 = Chi_dpnd_matrix[i][j].prob_neg_comma0_4[0] + Chi_dpnd_matrix[i][j].prob_neg_comma1_4[0];

	    if (Chi_dpnd_matrix[i][j].prob_neg_comma0_1[0] != 0) {
		lamda_comma = commaTotal_1 / (commaTotal_1 + 1);
		if (Chi_dpnd_matrix[i][j].prob_neg_comma0_2[0] != 0 || Chi_dpnd_matrix[i][j].prob_neg_comma0_3[0] != 0) {
		    Chi_dpnd_matrix[i][j].prob_neg_comma0 = lamda_comma * (Chi_dpnd_matrix[i][j].prob_neg_comma0_1[0] / commaTotal_1) +
			(1 - lamda_comma) * ((Chi_dpnd_matrix[i][j].prob_neg_comma0_2[0] + Chi_dpnd_matrix[i][j].prob_neg_comma0_3[0]) / (commaTotal_2 + commaTotal_3));
		}
		else {
		    Chi_dpnd_matrix[i][j].prob_neg_comma0 = Chi_dpnd_matrix[i][j].prob_neg_comma0_1[0] / commaTotal_1;
		}
	    }
	    else if (Chi_dpnd_matrix[i][j].prob_neg_comma0_2[0] != 0 || Chi_dpnd_matrix[i][j].prob_neg_comma0_3[0] != 0) {
		lamda_comma = (commaTotal_2 + commaTotal_3) / (commaTotal_2 + commaTotal_3 + 1);
		if (Chi_dpnd_matrix[i][j].prob_neg_comma0_4[0] != 0) {
		    Chi_dpnd_matrix[i][j].prob_neg_comma0 = dis_bkoff_weight_1 * lamda_comma * ((Chi_dpnd_matrix[i][j].prob_neg_comma0_2[0] + Chi_dpnd_matrix[i][j].prob_neg_comma0_3[0]) / (commaTotal_2 + commaTotal_3)) + 
			(1 - lamda_comma) * (Chi_dpnd_matrix[i][j].prob_neg_comma0_4[0] / commaTotal_4);
		}
		else {
		    Chi_dpnd_matrix[i][j].prob_neg_comma0 = dis_bkoff_weight_1 * (Chi_dpnd_matrix[i][j].prob_neg_comma0_2[0] + Chi_dpnd_matrix[i][j].prob_neg_comma0_3[0]) / (commaTotal_2 + commaTotal_3);
		}
	    }
	    else if (Chi_dpnd_matrix[i][j].prob_neg_comma0_4[0] != 0) {
		Chi_dpnd_matrix[i][j].prob_neg_comma0 = dis_bkoff_weight_2 * Chi_dpnd_matrix[i][j].prob_neg_comma0_4[0] / commaTotal_4;
	    }

	    /* initialization */
	    lex_rule = NULL;
	    pos_rule_1 = NULL;
	    pos_rule_2 = NULL;
	    pos_rule = NULL;

	    Chi_dpnd_matrix[i][j].count = 0;
	    Chi_dpnd_matrix[i][j].count_1 = 0;
	    Chi_dpnd_matrix[i][j].count_2 = 0;
	    Chi_dpnd_matrix[i][j].count_3 = 0;
	    Chi_dpnd_matrix[i][j].count_4 = 0;
	    Chi_dpnd_matrix[i][j].occur_1[0] = 0;
	    Chi_dpnd_matrix[i][j].occur_1[1] = 0;
	    Chi_dpnd_matrix[i][j].occur_RtoL_1[0] = 0;
	    Chi_dpnd_matrix[i][j].occur_RtoL_1[1] = 0;
	    Chi_dpnd_matrix[i][j].occur_2[0] = 0;
	    Chi_dpnd_matrix[i][j].occur_2[1] = 0;
	    Chi_dpnd_matrix[i][j].occur_RtoL_2[0] = 0;
	    Chi_dpnd_matrix[i][j].occur_RtoL_2[1] = 0;
	    Chi_dpnd_matrix[i][j].occur_3[0] = 0;
	    Chi_dpnd_matrix[i][j].occur_3[1] = 0;
	    Chi_dpnd_matrix[i][j].occur_RtoL_3[0] = 0;
	    Chi_dpnd_matrix[i][j].occur_RtoL_3[1] = 0;
	    Chi_dpnd_matrix[i][j].occur_4[0] = 0;
	    Chi_dpnd_matrix[i][j].occur_4[1] = 0;
	    Chi_dpnd_matrix[i][j].occur_RtoL_4[0] = 0;
	    Chi_dpnd_matrix[i][j].occur_RtoL_4[1] = 0;
	    Chi_dpnd_matrix[i][j].prob_LtoR_1[0] = 0;
	    Chi_dpnd_matrix[i][j].prob_LtoR_1[1] = 0;
	    Chi_dpnd_matrix[i][j].prob_RtoL_1[0] = 0;
	    Chi_dpnd_matrix[i][j].prob_RtoL_1[1] = 0;
	    Chi_dpnd_matrix[i][j].prob_LtoR_2[0] = 0;
	    Chi_dpnd_matrix[i][j].prob_LtoR_2[1] = 0;
	    Chi_dpnd_matrix[i][j].prob_RtoL_2[0] = 0;
	    Chi_dpnd_matrix[i][j].prob_RtoL_2[1] = 0;
	    Chi_dpnd_matrix[i][j].prob_LtoR_3[0] = 0;
	    Chi_dpnd_matrix[i][j].prob_LtoR_3[1] = 0;
	    Chi_dpnd_matrix[i][j].prob_RtoL_3[0] = 0;
	    Chi_dpnd_matrix[i][j].prob_RtoL_3[1] = 0;
	    Chi_dpnd_matrix[i][j].prob_LtoR_4[0] = 0;
	    Chi_dpnd_matrix[i][j].prob_LtoR_4[1] = 0;
	    Chi_dpnd_matrix[i][j].prob_RtoL_4[0] = 0;
	    Chi_dpnd_matrix[i][j].prob_RtoL_4[1] = 0;
	    Chi_dpnd_matrix[i][j].prob_LtoR[0] = 0;
	    Chi_dpnd_matrix[i][j].prob_LtoR[1] = 0;
	    Chi_dpnd_matrix[i][j].prob_RtoL[0] = 0;
	    Chi_dpnd_matrix[i][j].prob_RtoL[1] = 0;
	    Chi_dpnd_matrix[i][j].direction_1[0] = 0;
	    Chi_dpnd_matrix[i][j].direction_1[1] = 0;
	    Chi_dpnd_matrix[i][j].direction_2[0] = 0;
	    Chi_dpnd_matrix[i][j].direction_2[1] = 0;
	    Chi_dpnd_matrix[i][j].direction_3[0] = 0;
	    Chi_dpnd_matrix[i][j].direction_3[1] = 0;
	    Chi_dpnd_matrix[i][j].direction_4[0] = 0;
	    Chi_dpnd_matrix[i][j].direction_4[1] = 0;
	    Chi_dpnd_matrix[i][j].direction[0] = 0;
	    Chi_dpnd_matrix[i][j].direction[1] = 0;

	    /* read dpnd rule from DB for Chinese */
	    /* for each pair, [0] store TRAIN, [1] store GIGA */
	    lex_rule = get_chi_dpnd_rule(k_ptr->head_ptr->Goi, k_ptr->head_ptr->Pos, u_ptr->head_ptr->Goi, u_ptr->head_ptr->Pos, 999999, 0);
	    pos_rule_1 = get_chi_dpnd_rule(k_ptr->head_ptr->Goi, k_ptr->head_ptr->Pos, "XX", u_ptr->head_ptr->Pos, 999999, 0);
	    pos_rule_2 = get_chi_dpnd_rule("XX", k_ptr->head_ptr->Pos, u_ptr->head_ptr->Goi, u_ptr->head_ptr->Pos, 999999, 0);
	    pos_rule = get_chi_dpnd_rule("XX", k_ptr->head_ptr->Pos, "XX", u_ptr->head_ptr->Pos, 999999, 0);

	    if (lex_rule != NULL) {
		count = 0;
		rule = NULL;
		rule = strtok(lex_rule, ":");
		while (rule) {
		    curRule[count] = malloc(strlen(rule) + 1);
		    strcpy(curRule[count], rule);
		    count++;
		    rule = NULL;
		    rule = strtok(NULL, ":");
		}

		for (k = 0; k < count; k++) {
		    direction = NULL;
		    probLtoR = NULL;
		    occurLtoR = NULL;
		    probRtoL = NULL;
		    occurRtoL = NULL;
		    dpnd = NULL;
			    
		    direction = strtok(curRule[k], "_");
		    probLtoR = strtok(NULL, "_");
		    occurLtoR = strtok(NULL, "_");
		    probRtoL = strtok(NULL, "_");
		    occurRtoL = strtok(NULL, "_");
		    dpnd = strtok(NULL, "_");
			    
		    if (!strcmp(dpnd, "TRAIN")) {
			if (!strcmp(direction, "R")) {
			    Chi_dpnd_matrix[i][j].direction_1[0] = 'R';
			}
			else if (!strcmp(direction, "L")) {
			    Chi_dpnd_matrix[i][j].direction_1[0] = 'L';
			}
			else if (!strcmp(direction, "B")) {
			    Chi_dpnd_matrix[i][j].direction_1[0] = 'B';
			}
			Chi_dpnd_matrix[i][j].occur_1[0] = atof(occurLtoR);
			Chi_dpnd_matrix[i][j].occur_RtoL_1[0] = atof(occurRtoL);
			strcpy(Chi_dpnd_matrix[i][j].type_1[0], dpnd);
			Chi_dpnd_matrix[i][j].prob_LtoR_1[0] = atof(probLtoR);
			Chi_dpnd_matrix[i][j].prob_RtoL_1[0] = atof(probRtoL);
		    }
		    else if (!strcmp(dpnd, "GIGA")) {
			if (!strcmp(direction, "R")) {
			    Chi_dpnd_matrix[i][j].direction_1[1] = 'R';
			}
			else if (!strcmp(direction, "L")) {
			    Chi_dpnd_matrix[i][j].direction_1[1] = 'L';
			}
			else if (!strcmp(direction, "B")) {
			    Chi_dpnd_matrix[i][j].direction_1[1] = 'B';
			}
			Chi_dpnd_matrix[i][j].occur_1[1] = atof(occurLtoR);
			Chi_dpnd_matrix[i][j].occur_RtoL_1[1] = atof(occurRtoL);
			strcpy(Chi_dpnd_matrix[i][j].type_1[1], dpnd);
			Chi_dpnd_matrix[i][j].prob_LtoR_1[1] = atof(probLtoR);
			Chi_dpnd_matrix[i][j].prob_RtoL_1[1] = atof(probRtoL);
		    }
		    if (curRule[k]) {
			free(curRule[k]);
		    }
		}
	    }

	    if (pos_rule_1 != NULL) {
		count = 0;
		rule = NULL;
		rule = strtok(pos_rule_1, ":");
		while (rule) {
		    curRule[count] = malloc(strlen(rule) + 1);
		    strcpy(curRule[count], rule);
		    count++;
		    rule = NULL;
		    rule = strtok(NULL, ":");
		}

		for (k = 0; k < count; k++) {
		    direction = NULL;
		    probLtoR = NULL;
		    occurLtoR = NULL;
		    probRtoL = NULL;
		    occurRtoL = NULL;
		    dpnd = NULL;
			    
		    direction = strtok(curRule[k], "_");
		    probLtoR = strtok(NULL, "_");
		    occurLtoR = strtok(NULL, "_");
		    probRtoL = strtok(NULL, "_");
		    occurRtoL = strtok(NULL, "_");
		    dpnd = strtok(NULL, "_");
			    
		    if (!strcmp(dpnd, "TRAIN")) {
			if (!strcmp(direction, "R")) {
			    Chi_dpnd_matrix[i][j].direction_2[0] = 'R';
			}
			else if (!strcmp(direction, "L")) {
			    Chi_dpnd_matrix[i][j].direction_2[0] = 'L';
			}
			else if (!strcmp(direction, "B")) {
			    Chi_dpnd_matrix[i][j].direction_2[0] = 'B';
			}
			Chi_dpnd_matrix[i][j].occur_2[0] = atof(occurLtoR);
			Chi_dpnd_matrix[i][j].occur_RtoL_2[0] = atof(occurRtoL);
			strcpy(Chi_dpnd_matrix[i][j].type_2[0], dpnd);
			Chi_dpnd_matrix[i][j].prob_LtoR_2[0] = atof(probLtoR);
			Chi_dpnd_matrix[i][j].prob_RtoL_2[0] = atof(probRtoL);
		    }
		    else if (!strcmp(dpnd, "GIGA")) {
			if (!strcmp(direction, "R")) {
			    Chi_dpnd_matrix[i][j].direction_2[1] = 'R';
			}
			else if (!strcmp(direction, "L")) {
			    Chi_dpnd_matrix[i][j].direction_2[1] = 'L';
			}
			else if (!strcmp(direction, "B")) {
			    Chi_dpnd_matrix[i][j].direction_2[1] = 'B';
			}
			Chi_dpnd_matrix[i][j].occur_2[1] = atof(occurLtoR);
			Chi_dpnd_matrix[i][j].occur_RtoL_2[1] = atof(occurRtoL);
			strcpy(Chi_dpnd_matrix[i][j].type_2[1], dpnd);
			Chi_dpnd_matrix[i][j].prob_LtoR_2[1] = atof(probLtoR);
			Chi_dpnd_matrix[i][j].prob_RtoL_2[1] = atof(probRtoL);
		    }
		    if (curRule[k]) {
			free(curRule[k]);
		    }
		}
	    }

	    if (pos_rule_2 != NULL) {
		count = 0;
		rule = NULL;
		rule = strtok(pos_rule_2, ":");
		while (rule) {
		    curRule[count] = malloc(strlen(rule) + 1);
		    strcpy(curRule[count], rule);
		    count++;
		    rule = NULL;
		    rule = strtok(NULL, ":");
		}

		for (k = 0; k < count; k++) {
		    direction = NULL;
		    probLtoR = NULL;
		    occurLtoR = NULL;
		    probRtoL = NULL;
		    occurRtoL = NULL;
		    dpnd = NULL;
			    
		    direction = strtok(curRule[k], "_");
		    probLtoR = strtok(NULL, "_");
		    occurLtoR = strtok(NULL, "_");
		    probRtoL = strtok(NULL, "_");
		    occurRtoL = strtok(NULL, "_");
		    dpnd = strtok(NULL, "_");
			    
		    if (!strcmp(dpnd, "TRAIN")) {
			if (!strcmp(direction, "R")) {
			    Chi_dpnd_matrix[i][j].direction_3[0] = 'R';
			}
			else if (!strcmp(direction, "L")) {
			    Chi_dpnd_matrix[i][j].direction_3[0] = 'L';
			}
			else if (!strcmp(direction, "B")) {
			    Chi_dpnd_matrix[i][j].direction_3[0] = 'B';
			}
			Chi_dpnd_matrix[i][j].occur_3[0] = atof(occurLtoR);
			Chi_dpnd_matrix[i][j].occur_RtoL_3[0] = atof(occurRtoL);
			strcpy(Chi_dpnd_matrix[i][j].type_3[0], dpnd);
			Chi_dpnd_matrix[i][j].prob_LtoR_3[0] = atof(probLtoR);
			Chi_dpnd_matrix[i][j].prob_RtoL_3[0] = atof(probRtoL);
		    }
		    else if (!strcmp(dpnd, "GIGA")) {
			if (!strcmp(direction, "R")) {
			    Chi_dpnd_matrix[i][j].direction_3[1] = 'R';
			}
			else if (!strcmp(direction, "L")) {
			    Chi_dpnd_matrix[i][j].direction_3[1] = 'L';
			}
			else if (!strcmp(direction, "B")) {
			    Chi_dpnd_matrix[i][j].direction_3[1] = 'B';
			}
			Chi_dpnd_matrix[i][j].occur_3[1] = atof(occurLtoR);
			Chi_dpnd_matrix[i][j].occur_RtoL_3[1] = atof(occurRtoL);
			strcpy(Chi_dpnd_matrix[i][j].type_3[1], dpnd);
			Chi_dpnd_matrix[i][j].prob_LtoR_3[1] = atof(probLtoR);
			Chi_dpnd_matrix[i][j].prob_RtoL_3[1] = atof(probRtoL);
		    }
		    if (curRule[k]) {
			free(curRule[k]);
		    }
		}
	    }

	    if (pos_rule != NULL) {
		count = 0;
		rule = NULL;
		rule = strtok(pos_rule, ":");
		while (rule) {
		    curRule[count] = malloc(strlen(rule) + 1);
		    strcpy(curRule[count], rule);
		    count++;
		    rule = NULL;
		    rule = strtok(NULL, ":");
		}

		for (k = 0; k < count; k++) {
		    direction = NULL;
		    probLtoR = NULL;
		    occurLtoR = NULL;
		    probRtoL = NULL;
		    occurRtoL = NULL;
		    dpnd = NULL;
			    
		    direction = strtok(curRule[k], "_");
		    probLtoR = strtok(NULL, "_");
		    occurLtoR = strtok(NULL, "_");
		    probRtoL = strtok(NULL, "_");
		    occurRtoL = strtok(NULL, "_");
		    dpnd = strtok(NULL, "_");
			    
		    if (!strcmp(dpnd, "TRAIN")) {
			if (!strcmp(direction, "R")) {
			    Chi_dpnd_matrix[i][j].direction_4[0] = 'R';
			}
			else if (!strcmp(direction, "L")) {
			    Chi_dpnd_matrix[i][j].direction_4[0] = 'L';
			}
			else if (!strcmp(direction, "B")) {
			    Chi_dpnd_matrix[i][j].direction_4[0] = 'B';
			}
			Chi_dpnd_matrix[i][j].occur_4[0] = atof(occurLtoR);
			Chi_dpnd_matrix[i][j].occur_RtoL_4[0] = atof(occurRtoL);
			strcpy(Chi_dpnd_matrix[i][j].type_4[0], dpnd);
			Chi_dpnd_matrix[i][j].prob_LtoR_4[0] = atof(probLtoR);
			Chi_dpnd_matrix[i][j].prob_RtoL_4[0] = atof(probRtoL);
		    }
		    else if (!strcmp(dpnd, "GIGA")) {
			if (!strcmp(direction, "R")) {
			    Chi_dpnd_matrix[i][j].direction_4[1] = 'R';
			}
			else if (!strcmp(direction, "L")) {
			    Chi_dpnd_matrix[i][j].direction_4[1] = 'L';
			}
			else if (!strcmp(direction, "B")) {
			    Chi_dpnd_matrix[i][j].direction_4[1] = 'B';
			}
			Chi_dpnd_matrix[i][j].occur_4[1] = atof(occurLtoR);
			Chi_dpnd_matrix[i][j].occur_RtoL_4[1] = atof(occurRtoL);
			strcpy(Chi_dpnd_matrix[i][j].type_4[1], dpnd);
			Chi_dpnd_matrix[i][j].prob_LtoR_4[1] = atof(probLtoR);
			Chi_dpnd_matrix[i][j].prob_RtoL_4[1] = atof(probRtoL);
		    }
		    if (curRule[k]) {
			free(curRule[k]);
		    }
		}
	    }

	    /* calculate probability */
	    Chi_dpnd_matrix[i][j].occur_1[0] += dpnd_giga_weight * Chi_dpnd_matrix[i][j].occur_1[1];
	    Chi_dpnd_matrix[i][j].occur_RtoL_1[0] += dpnd_giga_weight * Chi_dpnd_matrix[i][j].occur_RtoL_1[1];
	    Chi_dpnd_matrix[i][j].prob_LtoR_1[0] += dpnd_giga_weight * Chi_dpnd_matrix[i][j].prob_LtoR_1[1];
	    Chi_dpnd_matrix[i][j].prob_RtoL_1[0] += dpnd_giga_weight * Chi_dpnd_matrix[i][j].prob_RtoL_1[1];

	    Chi_dpnd_matrix[i][j].occur_2[0] += dpnd_giga_weight * Chi_dpnd_matrix[i][j].occur_2[1];
	    Chi_dpnd_matrix[i][j].occur_RtoL_2[0] += dpnd_giga_weight * Chi_dpnd_matrix[i][j].occur_RtoL_2[1];
	    Chi_dpnd_matrix[i][j].prob_LtoR_2[0] += dpnd_giga_weight * Chi_dpnd_matrix[i][j].prob_LtoR_2[1];
	    Chi_dpnd_matrix[i][j].prob_RtoL_2[0] += dpnd_giga_weight * Chi_dpnd_matrix[i][j].prob_RtoL_2[1];

	    Chi_dpnd_matrix[i][j].occur_3[0] += dpnd_giga_weight * Chi_dpnd_matrix[i][j].occur_3[1];
	    Chi_dpnd_matrix[i][j].occur_RtoL_3[0] += dpnd_giga_weight * Chi_dpnd_matrix[i][j].occur_RtoL_3[1];
	    Chi_dpnd_matrix[i][j].prob_LtoR_3[0] += dpnd_giga_weight * Chi_dpnd_matrix[i][j].prob_LtoR_3[1];
	    Chi_dpnd_matrix[i][j].prob_RtoL_3[0] += dpnd_giga_weight * Chi_dpnd_matrix[i][j].prob_RtoL_3[1];

	    Chi_dpnd_matrix[i][j].occur_4[0] += dpnd_giga_weight * Chi_dpnd_matrix[i][j].occur_4[1];
	    Chi_dpnd_matrix[i][j].occur_RtoL_4[0] += dpnd_giga_weight * Chi_dpnd_matrix[i][j].occur_RtoL_4[1];
	    Chi_dpnd_matrix[i][j].prob_LtoR_4[0] += dpnd_giga_weight * Chi_dpnd_matrix[i][j].prob_LtoR_4[1];
	    Chi_dpnd_matrix[i][j].prob_RtoL_4[0] += dpnd_giga_weight * Chi_dpnd_matrix[i][j].prob_RtoL_4[1];

	    Chi_dpnd_matrix[i][j].prob_pos_LtoR = Chi_dpnd_matrix[i][j].prob_LtoR_4[0] / Chi_dpnd_matrix[i][j].occur_4[0];
	    Chi_dpnd_matrix[i][j].prob_pos_RtoL = Chi_dpnd_matrix[i][j].prob_RtoL_4[0] / Chi_dpnd_matrix[i][j].occur_RtoL_4[0];

	    /* calc dpnd_LtoR */
	    if (Chi_dpnd_matrix[i][j].prob_LtoR_1[0] != 0 || Chi_dpnd_matrix[i][j].prob_RtoL_1[0] != 0) {
		lamda_dpnd = (Chi_dpnd_matrix[i][j].prob_LtoR_1[0] + Chi_dpnd_matrix[i][j].prob_RtoL_1[0]) / (Chi_dpnd_matrix[i][j].prob_LtoR_1[0] + Chi_dpnd_matrix[i][j].prob_RtoL_1[0] +1);
		if (Chi_dpnd_matrix[i][j].prob_LtoR_2[0] != 0 || Chi_dpnd_matrix[i][j].prob_RtoL_2[0] != 0 || Chi_dpnd_matrix[i][j].prob_LtoR_3[0] != 0 || Chi_dpnd_matrix[i][j].prob_RtoL_3[0] != 0) {
		    Chi_dpnd_matrix[i][j].dpnd_LtoR = lamda_dpnd * Chi_dpnd_matrix[i][j].prob_LtoR_1[0] / (Chi_dpnd_matrix[i][j].prob_LtoR_1[0] + Chi_dpnd_matrix[i][j].prob_RtoL_1[0]) +
			(1- lamda_dpnd) * ((Chi_dpnd_matrix[i][j].prob_LtoR_2[0] + Chi_dpnd_matrix[i][j].prob_LtoR_3[0]) / (Chi_dpnd_matrix[i][j].prob_LtoR_2[0] + Chi_dpnd_matrix[i][j].prob_LtoR_3[0] + Chi_dpnd_matrix[i][j].prob_RtoL_2[0] + Chi_dpnd_matrix[i][j].prob_RtoL_3[0]));
		}
		else {
		    Chi_dpnd_matrix[i][j].dpnd_LtoR = Chi_dpnd_matrix[i][j].prob_LtoR_1[0] / (Chi_dpnd_matrix[i][j].prob_LtoR_1[0] + Chi_dpnd_matrix[i][j].prob_RtoL_1[0]);
		}
	    }

	    else if (Chi_dpnd_matrix[i][j].prob_LtoR_2[0] != 0 || Chi_dpnd_matrix[i][j].prob_RtoL_2[0] != 0 || Chi_dpnd_matrix[i][j].prob_LtoR_3[0] != 0 || Chi_dpnd_matrix[i][j].prob_RtoL_3[0] != 0) {
		lamda_dpnd = (Chi_dpnd_matrix[i][j].prob_LtoR_2[0] + Chi_dpnd_matrix[i][j].prob_LtoR_3[0] + Chi_dpnd_matrix[i][j].prob_RtoL_2[0] + Chi_dpnd_matrix[i][j].prob_RtoL_3[0]) / (Chi_dpnd_matrix[i][j].prob_LtoR_2[0] + Chi_dpnd_matrix[i][j].prob_LtoR_3[0] + Chi_dpnd_matrix[i][j].prob_RtoL_2[0] + Chi_dpnd_matrix[i][j].prob_RtoL_3[0] + 1);
		if (Chi_dpnd_matrix[i][j].prob_LtoR_4[0] != 0 || Chi_dpnd_matrix[i][j].prob_RtoL_4[0] != 0) {
		    Chi_dpnd_matrix[i][j].dpnd_LtoR = bkoff_weight_1 * lamda_dpnd * ((Chi_dpnd_matrix[i][j].prob_LtoR_2[0] + Chi_dpnd_matrix[i][j].prob_LtoR_3[0]) / (Chi_dpnd_matrix[i][j].prob_LtoR_2[0] + Chi_dpnd_matrix[i][j].prob_LtoR_3[0] + Chi_dpnd_matrix[i][j].prob_RtoL_2[0] + Chi_dpnd_matrix[i][j].prob_RtoL_3[0])) +
			(1- lamda_dpnd) * ((Chi_dpnd_matrix[i][j].prob_LtoR_4[0]) / (Chi_dpnd_matrix[i][j].prob_LtoR_4[0] + Chi_dpnd_matrix[i][j].prob_RtoL_4[0]));
		}
		else {
		    Chi_dpnd_matrix[i][j].dpnd_LtoR = bkoff_weight_1 * (Chi_dpnd_matrix[i][j].prob_LtoR_2[0] + Chi_dpnd_matrix[i][j].prob_LtoR_3[0]) / (Chi_dpnd_matrix[i][j].prob_LtoR_2[0] + Chi_dpnd_matrix[i][j].prob_LtoR_3[0] + Chi_dpnd_matrix[i][j].prob_RtoL_2[0] + Chi_dpnd_matrix[i][j].prob_RtoL_3[0]);
		}
	    }

	    else if (Chi_dpnd_matrix[i][j].prob_LtoR_4[0] != 0 || Chi_dpnd_matrix[i][j].prob_RtoL_4[0] != 0) {
		Chi_dpnd_matrix[i][j].dpnd_LtoR = bkoff_weight_2 * ((Chi_dpnd_matrix[i][j].prob_LtoR_4[0]) / (Chi_dpnd_matrix[i][j].prob_LtoR_4[0] + Chi_dpnd_matrix[i][j].prob_RtoL_4[0]));
	    }

	    else {
		Chi_dpnd_matrix[i][j].dpnd_LtoR = 0;
	    }

	    /* calc dpnd_RtoL */
	    if (Chi_dpnd_matrix[i][j].prob_LtoR_1[0] != 0 || Chi_dpnd_matrix[i][j].prob_RtoL_1[0] != 0) {
		lamda_dpnd = (Chi_dpnd_matrix[i][j].prob_LtoR_1[0] + Chi_dpnd_matrix[i][j].prob_RtoL_1[0]) / (Chi_dpnd_matrix[i][j].prob_LtoR_1[0] + Chi_dpnd_matrix[i][j].prob_RtoL_1[0] +1);
		if (Chi_dpnd_matrix[i][j].prob_LtoR_2[0] != 0 || Chi_dpnd_matrix[i][j].prob_RtoL_2[0] != 0 || Chi_dpnd_matrix[i][j].prob_LtoR_3[0] != 0 || Chi_dpnd_matrix[i][j].prob_RtoL_3[0] != 0) {
		    Chi_dpnd_matrix[i][j].dpnd_RtoL = lamda_dpnd * Chi_dpnd_matrix[i][j].prob_RtoL_1[0] / (Chi_dpnd_matrix[i][j].prob_LtoR_1[0] + Chi_dpnd_matrix[i][j].prob_RtoL_1[0]) +
			(1- lamda_dpnd) * ((Chi_dpnd_matrix[i][j].prob_RtoL_2[0] + Chi_dpnd_matrix[i][j].prob_RtoL_3[0]) / (Chi_dpnd_matrix[i][j].prob_LtoR_2[0] + Chi_dpnd_matrix[i][j].prob_LtoR_3[0] + Chi_dpnd_matrix[i][j].prob_RtoL_2[0] + Chi_dpnd_matrix[i][j].prob_RtoL_3[0]));
		}
		else {
		    Chi_dpnd_matrix[i][j].dpnd_RtoL = Chi_dpnd_matrix[i][j].prob_RtoL_1[0] / (Chi_dpnd_matrix[i][j].prob_LtoR_1[0] + Chi_dpnd_matrix[i][j].prob_RtoL_1[0]);
		}
	    }

	    else if (Chi_dpnd_matrix[i][j].prob_LtoR_2[0] != 0 || Chi_dpnd_matrix[i][j].prob_RtoL_2[0] != 0 || Chi_dpnd_matrix[i][j].prob_LtoR_3[0] != 0 || Chi_dpnd_matrix[i][j].prob_RtoL_3[0] != 0) {
		lamda_dpnd = (Chi_dpnd_matrix[i][j].prob_LtoR_2[0] + Chi_dpnd_matrix[i][j].prob_LtoR_3[0] + Chi_dpnd_matrix[i][j].prob_RtoL_2[0] + Chi_dpnd_matrix[i][j].prob_RtoL_3[0]) / (Chi_dpnd_matrix[i][j].prob_LtoR_2[0] + Chi_dpnd_matrix[i][j].prob_LtoR_3[0] + Chi_dpnd_matrix[i][j].prob_RtoL_2[0] + Chi_dpnd_matrix[i][j].prob_RtoL_3[0] + 1);
		if (Chi_dpnd_matrix[i][j].prob_LtoR_4[0] != 0 || Chi_dpnd_matrix[i][j].prob_RtoL_4[0] != 0) {
		    Chi_dpnd_matrix[i][j].dpnd_RtoL = bkoff_weight_1 * lamda_dpnd * ((Chi_dpnd_matrix[i][j].prob_RtoL_2[0] + Chi_dpnd_matrix[i][j].prob_RtoL_3[0]) / (Chi_dpnd_matrix[i][j].prob_LtoR_2[0] + Chi_dpnd_matrix[i][j].prob_LtoR_3[0] + Chi_dpnd_matrix[i][j].prob_RtoL_2[0] + Chi_dpnd_matrix[i][j].prob_RtoL_3[0])) +
			(1- lamda_dpnd) * ((Chi_dpnd_matrix[i][j].prob_RtoL_4[0]) / (Chi_dpnd_matrix[i][j].prob_LtoR_4[0] + Chi_dpnd_matrix[i][j].prob_RtoL_4[0]));
		}
		else {
		    Chi_dpnd_matrix[i][j].dpnd_RtoL = bkoff_weight_1 * (Chi_dpnd_matrix[i][j].prob_RtoL_2[0] + Chi_dpnd_matrix[i][j].prob_RtoL_3[0]) / (Chi_dpnd_matrix[i][j].prob_LtoR_2[0] + Chi_dpnd_matrix[i][j].prob_LtoR_3[0] + Chi_dpnd_matrix[i][j].prob_RtoL_2[0] + Chi_dpnd_matrix[i][j].prob_RtoL_3[0]);
		}
	    }

	    else if (Chi_dpnd_matrix[i][j].prob_LtoR_4[0] != 0 || Chi_dpnd_matrix[i][j].prob_RtoL_4[0] != 0) {
		Chi_dpnd_matrix[i][j].dpnd_RtoL = bkoff_weight_2 * ((Chi_dpnd_matrix[i][j].prob_RtoL_4[0]) / (Chi_dpnd_matrix[i][j].prob_LtoR_4[0] + Chi_dpnd_matrix[i][j].prob_RtoL_4[0]));
	    }

	    else {
		Chi_dpnd_matrix[i][j].dpnd_RtoL = 0;
	    }

	    /* prob_LtoR */
	    if (Chi_dpnd_matrix[i][j].occur_1[0] != 0) {
		Chi_dpnd_matrix[i][j].lamda1[0] = Chi_dpnd_matrix[i][j].occur_1[0] / (Chi_dpnd_matrix[i][j].occur_1[0] + 1);
		if (Chi_dpnd_matrix[i][j].occur_2[0] != 0 || Chi_dpnd_matrix[i][j].occur_3[0] != 0) {
		    Chi_dpnd_matrix[i][j].prob_LtoR[0] = (Chi_dpnd_matrix[i][j].lamda1[0] * Chi_dpnd_matrix[i][j].prob_LtoR_1[0] / Chi_dpnd_matrix[i][j].occur_1[0]) +
			((1 - Chi_dpnd_matrix[i][j].lamda1[0]) * (Chi_dpnd_matrix[i][j].prob_LtoR_2[0] + Chi_dpnd_matrix[i][j].prob_LtoR_3[0]) / (Chi_dpnd_matrix[i][j].occur_2[0] + Chi_dpnd_matrix[i][j].occur_3[0]));
		}
		else {
		    Chi_dpnd_matrix[i][j].prob_LtoR[0] = Chi_dpnd_matrix[i][j].lamda1[0] * Chi_dpnd_matrix[i][j].prob_LtoR_1[0] / Chi_dpnd_matrix[i][j].occur_1[0];
		}
	    }
	    else if (Chi_dpnd_matrix[i][j].occur_2[0] != 0 || Chi_dpnd_matrix[i][j].occur_3[0] != 0) {
		Chi_dpnd_matrix[i][j].lamda1[0] = (Chi_dpnd_matrix[i][j].occur_2[0] + Chi_dpnd_matrix[i][j].occur_3[0]) /
		    (Chi_dpnd_matrix[i][j].occur_2[0] + Chi_dpnd_matrix[i][j].occur_3[0] + 1);

		if (Chi_dpnd_matrix[i][j].occur_4[0] != 0) {
		    Chi_dpnd_matrix[i][j].prob_LtoR[0] = bkoff_weight_1 * ((Chi_dpnd_matrix[i][j].lamda1[0]) * (Chi_dpnd_matrix[i][j].prob_LtoR_2[0] + Chi_dpnd_matrix[i][j].prob_LtoR_3[0]) / (Chi_dpnd_matrix[i][j].occur_2[0] + Chi_dpnd_matrix[i][j].occur_3[0]) + 
			((1 - Chi_dpnd_matrix[i][j].lamda1[0]) * (Chi_dpnd_matrix[i][j].prob_LtoR_4[0] / Chi_dpnd_matrix[i][j].occur_4[0])));
		}
		else {
		    Chi_dpnd_matrix[i][j].prob_LtoR[0] = bkoff_weight_1 * ((Chi_dpnd_matrix[i][j].lamda1[0]) * (Chi_dpnd_matrix[i][j].prob_LtoR_2[0] + Chi_dpnd_matrix[i][j].prob_LtoR_3[0]) / (Chi_dpnd_matrix[i][j].occur_2[0] + Chi_dpnd_matrix[i][j].occur_3[0]));
		}
	    }
	    else if (Chi_dpnd_matrix[i][j].occur_4[0] != 0) {
		Chi_dpnd_matrix[i][j].prob_LtoR[0] = bkoff_weight_2 * Chi_dpnd_matrix[i][j].prob_LtoR_4[0] / Chi_dpnd_matrix[i][j].occur_4[0];
	    }


	    /* prob_RtoL */
	    if (Chi_dpnd_matrix[i][j].occur_RtoL_1[0] != 0) {
		Chi_dpnd_matrix[i][j].lamda1[1] = Chi_dpnd_matrix[i][j].occur_RtoL_1[0] / (Chi_dpnd_matrix[i][j].occur_RtoL_1[0] + 1);
		if (Chi_dpnd_matrix[i][j].occur_RtoL_2[0] != 0 || Chi_dpnd_matrix[i][j].occur_RtoL_3[0] != 0) {
		    Chi_dpnd_matrix[i][j].prob_RtoL[0] = (Chi_dpnd_matrix[i][j].lamda1[1] * Chi_dpnd_matrix[i][j].prob_RtoL_1[0] / Chi_dpnd_matrix[i][j].occur_RtoL_1[0]) +
			((1 - Chi_dpnd_matrix[i][j].lamda1[1]) * (Chi_dpnd_matrix[i][j].prob_RtoL_2[0] + Chi_dpnd_matrix[i][j].prob_RtoL_3[0]) / (Chi_dpnd_matrix[i][j].occur_RtoL_2[0] + Chi_dpnd_matrix[i][j].occur_RtoL_3[0]));
		}
		else {
		    Chi_dpnd_matrix[i][j].prob_RtoL[0] = Chi_dpnd_matrix[i][j].lamda1[1] * Chi_dpnd_matrix[i][j].prob_RtoL_1[0] / Chi_dpnd_matrix[i][j].occur_RtoL_1[0];
		}
	    }
	    else if (Chi_dpnd_matrix[i][j].occur_RtoL_2[0] != 0 || Chi_dpnd_matrix[i][j].occur_RtoL_3[0] != 0) {
		Chi_dpnd_matrix[i][j].lamda1[1] = (Chi_dpnd_matrix[i][j].occur_RtoL_2[0] + Chi_dpnd_matrix[i][j].occur_RtoL_3[0]) / 
		    (Chi_dpnd_matrix[i][j].occur_RtoL_2[0] + Chi_dpnd_matrix[i][j].occur_RtoL_3[0] + 1);

		if (Chi_dpnd_matrix[i][j].occur_RtoL_4[0] != 0) {
		    Chi_dpnd_matrix[i][j].prob_RtoL[0] = bkoff_weight_1 * ((Chi_dpnd_matrix[i][j].lamda1[1]) * (Chi_dpnd_matrix[i][j].prob_RtoL_2[0] + Chi_dpnd_matrix[i][j].prob_RtoL_3[0]) / (Chi_dpnd_matrix[i][j].occur_RtoL_2[0] + Chi_dpnd_matrix[i][j].occur_RtoL_3[0]) + 
			((1 - Chi_dpnd_matrix[i][j].lamda1[1]) * (Chi_dpnd_matrix[i][j].prob_RtoL_4[0] / Chi_dpnd_matrix[i][j].occur_RtoL_4[0])));
		}
		else {
		    Chi_dpnd_matrix[i][j].prob_RtoL[0] = bkoff_weight_1 * ((Chi_dpnd_matrix[i][j].lamda1[1]) * (Chi_dpnd_matrix[i][j].prob_RtoL_2[0] + Chi_dpnd_matrix[i][j].prob_RtoL_3[0]) / (Chi_dpnd_matrix[i][j].occur_RtoL_2[0] + Chi_dpnd_matrix[i][j].occur_RtoL_3[0]));
		}
	    }
	    else if (Chi_dpnd_matrix[i][j].occur_RtoL_4[0] != 0) {
		Chi_dpnd_matrix[i][j].prob_RtoL[0] = bkoff_weight_2 * Chi_dpnd_matrix[i][j].prob_RtoL_4[0] / Chi_dpnd_matrix[i][j].occur_RtoL_4[0];
	    }

	    /* direction */
	    if (Chi_dpnd_matrix[i][j].prob_LtoR[0] > 0 && Chi_dpnd_matrix[i][j].prob_RtoL[0] > 0) {
		Chi_dpnd_matrix[i][j].direction[0] = 'B';
		Chi_dpnd_matrix[i][j].count = 1;
	    }
	    else if (Chi_dpnd_matrix[i][j].prob_LtoR[0] > 0) {
		Chi_dpnd_matrix[i][j].direction[0] = 'R';
		Chi_dpnd_matrix[i][j].count = 1;
	    }
	    else if (Chi_dpnd_matrix[i][j].prob_RtoL[0] > 0) {
		Chi_dpnd_matrix[i][j].direction[0] = 'L';
		Chi_dpnd_matrix[i][j].count = 1;
	    }
		
	    if (Chi_dpnd_matrix[i][j].count > 0) {
		Dpnd_matrix[i][j] = 'O';
	    }
	}
    }

    /* free memory */
    if (lex_rule) {
	free(lex_rule);
    }
    if (pos_rule_1) {
	free(pos_rule_1);
    }
    if (pos_rule_2) {
	free(pos_rule_2);
    }
    if (pos_rule) {
	free(pos_rule);
    }
    if (rule) {
	free(rule);
    }
}

/*==================================================================*/
	       int relax_dpnd_matrix(SENTENCE_DATA *sp)
/*==================================================================*/
{
    /* �����褬�ʤ����δ���

       ��̤ˤ��ޥ�����ͥ�褷������������������˷����褦���ѹ�

       �� ���šš֡šššššסšţ� (ʸ��)
       �� �ššš֣��ššţ¡סšš� (��̽�)
       �� �ššš֣��ţ¡��šסšš� (��:ʸ��)
       �� ���šššţ¡֡šššţá� (�¤˷�������ȤϤ��ʤ���
                                      �äȤδط��ϲ��Ϥ��н�)
    */

    int i, j, ok_flag, relax_flag, last_possibility;
  
    relax_flag = FALSE;

    for (i = 0; i < sp->Bnst_num - 1  ; i++) {
	ok_flag = FALSE;
	last_possibility = i;
	for (j = i + 1; j < sp->Bnst_num ; j++) {
	    if (Quote_matrix[i][j]) {
		if (Dpnd_matrix[i][j] > 0) {
		    ok_flag = TRUE;
		    break;
		} else if (check_feature(sp->bnst_data[j].f, "��:ʸ��")) {
		    last_possibility = j;
		    break;
		} else {
		    last_possibility = j;
		}
	    }
	}

	if (ok_flag == FALSE) {
	    if (check_feature(sp->bnst_data[last_possibility].f, "ʸ��") ||
		check_feature(sp->bnst_data[last_possibility].f, "��:ʸ��") ||
		check_feature(sp->bnst_data[last_possibility].f, "��̽�")) {
		Dpnd_matrix[i][last_possibility] = 'R';
		relax_flag = TRUE;
	    }
	}
    }

    return relax_flag;
}

/*==================================================================*/
int check_uncertain_d_condition(SENTENCE_DATA *sp, DPND *dp, int gvnr)
/*==================================================================*/
{
    /* ������(���å�)�� d �η��������������

       �� ���β�ǽ�ʷ�����(D)�����İʾ��� ( d - - D �ʤ� )
       �� ���긵��d�θ��Ʊ����	��) ���ܤǺǽ�˵��ԤǹԤ�줿
       �� d(������)��d�θ��Ʊ����	��) ����Ƿײ���˵��Ԥ��ѹ����줿

       �� ��d������������פ��Ȥ�d�򷸤���Ȥ���Τ���Ŭ��
       ��) �֤������Ĥ����ܤ�ľ�Ѥˤʤ�褦�ˡ������Ϥ��碌����Ρ���
    */

    int i, next_D;
    char *dpnd_cp, *gvnr_cp, *next_cp;

    next_D = 0;
    for (i = gvnr + 1; i < sp->Bnst_num ; i++) {
	if (Mask_matrix[dp->pos][i] &&
	    Quote_matrix[dp->pos][i] &&
	    dp->mask[i] &&
	    Dpnd_matrix[dp->pos][i] == 'D') {
	    next_D = i;
	    break;
	}
    }
    dpnd_cp = check_feature(sp->bnst_data[dp->pos].f, "��");
    gvnr_cp = check_feature(sp->bnst_data[gvnr].f, "��");
    if (gvnr < sp->Bnst_num-1) {
	next_cp = check_feature(sp->bnst_data[gvnr+1].f, "��");	
    }
    else {
	next_cp = NULL;
    }

    if (next_D == 0 ||
	gvnr + 2 < next_D ||
	(gvnr + 2 == next_D && gvnr < sp->Bnst_num-1 &&
	 check_feature(sp->bnst_data[gvnr+1].f, "�θ�") &&
	 ((dpnd_cp && next_cp && !strcmp(dpnd_cp, next_cp)) ||
	  (gvnr_cp && next_cp && !strcmp(gvnr_cp, next_cp))))) {
	/* fprintf(stderr, "%d -> %d OK\n", i, j); */
	return 1;
    } else {
	return 0;
    }
}

/*==================================================================*/
int compare_dpnd(SENTENCE_DATA *sp, TOTAL_MGR *new_mgr, TOTAL_MGR *best_mgr)
/*==================================================================*/
{
    int i;

    return FALSE;

    if (Possibility == 1 || new_mgr->dflt < best_mgr->dflt) {
	return TRUE;
    } else {
	for (i = sp->Bnst_num - 2; i >= 0; i--) {
	    if (new_mgr->dpnd.dflt[i] < best_mgr->dpnd.dflt[i]) 
		return TRUE;
	    else if (new_mgr->dpnd.dflt[i] > best_mgr->dpnd.dflt[i]) 
		return FALSE;
	}
    }

    fprintf(stderr, ";; Error in compare_dpnd !!\n");
    exit(1);
}

/*==================================================================*/
	 void dpnd_info_to_bnst(SENTENCE_DATA *sp, DPND *dp)
/*==================================================================*/
{
    /* ��������˴ؤ����ξ���� DPND ���� BNST_DATA �˥��ԡ� */

    int		i;
    BNST_DATA	*b_ptr;

    for (i = 0, b_ptr = sp->bnst_data; i < sp->Bnst_num; i++, b_ptr++) {
	if (Language != CHINESE && (dp->type[i] == 'd' || dp->type[i] == 'R')) {
	    b_ptr->dpnd_head = dp->head[i];
	    b_ptr->dpnd_type = 'D';	/* relax��������D�� */
	} else {
	    b_ptr->dpnd_head = dp->head[i];
	    b_ptr->dpnd_type = dp->type[i];
	}
	b_ptr->dpnd_dflt = dp->dflt[i];
    }
}

/*==================================================================*/
	void dpnd_info_to_tag_raw(SENTENCE_DATA *sp, DPND *dp)
/*==================================================================*/
{
    /* ��������˴ؤ����ξ���� DPND ���� TAG_DATA �˥��ԡ� */

    int		i, j, last_b, offset, score, rep_length;
    char	*cp, *strp, buf[16];
    TAG_DATA	*t_ptr, *ht_ptr;

    for (i = 0, t_ptr = sp->tag_data; i < sp->Tag_num; i++, t_ptr++) {
	/* ��äȤ�ᤤʸ��Ԥ򵭲� */
	if (t_ptr->bnum >= 0) {
	    last_b = t_ptr->bnum;
	}

	/* ʸ�� */
	if (i == sp->Tag_num - 1) {
	    t_ptr->dpnd_head = -1;
	    t_ptr->dpnd_type = 'D';
	}
	/* �٤ˤ����� */
	else if (t_ptr->inum != 0) {
	    t_ptr->dpnd_head = t_ptr->num + 1;
	    t_ptr->dpnd_type = 'D';
	}
	/* ʸ����Ǹ�Υ���ñ�� (inum == 0) */
	else {
	    if ((!check_feature((sp->bnst_data + last_b)->f, "����ñ�̼�̵��")) &&
		(cp = check_feature((sp->bnst_data + dp->head[last_b])->f, "����ñ�̼�"))) {
		offset = atoi(cp + 11);
		if (offset > 0 || (sp->bnst_data + dp->head[last_b])->tag_num <= -1 * offset) {
		    offset = 0;
		}
	    }
	    else {
		offset = 0;
	    }

	    /* ���Σ£äʤɤ����ä����ϡ��äγʥե졼��ˣ���¸�ߤ�����
	       ���ġ��¤γʥե졼��ˣ���¸�ߤ������ϡ������¤ˤ�����ȹͤ��� */
	    if ((!check_feature((sp->bnst_data + last_b)->f, "����ñ�̼�̵��")) &&
		check_feature(t_ptr->f, "��:�γ�") && dp->head[last_b] - last_b == 1) {

		if (OptCaseFlag & OPT_CASE_USE_REP_CF) {
		    strp = get_mrph_rep(t_ptr->head_ptr);
		    rep_length = get_mrph_rep_length(strp);
		}
		else {
		    strp = t_ptr->head_ptr->Goi2;
		    rep_length = strlen(strp);
		}

		/* �֣��ΣáפΥ����� */
		ht_ptr = (sp->bnst_data + dp->head[last_b])->tag_ptr + 
		    (sp->bnst_data + dp->head[last_b])->tag_num - 1 + offset;	
		if (ht_ptr->cf_ptr) {
		    score = check_examples(strp, rep_length,
					   ht_ptr->cf_ptr->ex_list[0],
					   ht_ptr->cf_ptr->ex_num[0]);
		}
		else {
		    score = -1;
		}

		/* �¤�ʣ����������������Τ���Υ롼�� */
		for (j = 0, ht_ptr = (sp->bnst_data + dp->head[last_b])->tag_ptr; 
		     j < (sp->bnst_data + dp->head[last_b])->tag_num - 1 + offset;
		     j++, ht_ptr++) {
		    
		    if (OptCaseFlag & OPT_CASE_USE_REP_CF) {
			strp = get_mrph_rep(t_ptr->head_ptr);
			rep_length = get_mrph_rep_length(strp);
		    }
		    else {
			strp = t_ptr->head_ptr->Goi2;
			rep_length = strlen(strp);
		    }

		    /* �֣��Σ¡פΥ����� */
		    if (score == -1 && ht_ptr->cf_ptr &&
			check_examples(strp, rep_length,
				       ht_ptr->cf_ptr->ex_list[0],
				       ht_ptr->cf_ptr->ex_num[0]) > score) {

			offset = j - ((sp->bnst_data + dp->head[last_b])->tag_num - 1);
			sprintf(buf, "ľ��������:%d", offset);
			assign_cfeature(&((sp->bnst_data + dp->head[last_b])->f), buf, FALSE);
			assign_cfeature(&(ht_ptr->f), buf, FALSE);
			break;
		    }
		}		 
	    }

	    if (dp->head[last_b] == -1) {
		t_ptr->dpnd_head = -1;
	    }
	    else {
		t_ptr->dpnd_head = ((sp->bnst_data + dp->head[last_b])->tag_ptr + 
				    (sp->bnst_data + dp->head[last_b])->tag_num - 1 + offset)->num;
	    }

	    if (Language != CHINESE && (dp->type[last_b] == 'd' || dp->type[last_b] == 'R')) {
		t_ptr->dpnd_type = 'D';
	    }
	    else {
		t_ptr->dpnd_type = dp->type[last_b];
	    }
	}
    }
}

/*==================================================================*/
	  void dpnd_info_to_tag(SENTENCE_DATA *sp, DPND *dp)
/*==================================================================*/
{
    if (OptInput == OPT_RAW || 
	(OptInput & OPT_INPUT_BNST) ||
	OptUseNCF) {
	dpnd_info_to_tag_raw(sp, dp);
    }
    else {
	dpnd_info_to_tag_pm(sp);
    }
}

/*==================================================================*/
void copy_para_info(SENTENCE_DATA *sp, BNST_DATA *dst, BNST_DATA *src)
/*==================================================================*/
{
    dst->para_num = src->para_num;
    dst->para_key_type = src->para_key_type;
    dst->para_top_p = src->para_top_p;
    dst->para_type = src->para_type;
    dst->to_para_p = src->to_para_p;
}

/*==================================================================*/
	void tag_bnst_postprocess(SENTENCE_DATA *sp, int flag)
/*==================================================================*/
{
    /* ����ñ�̡�ʸ����������ơ���ǽŪ�ʥ���ñ�̤�ޡ���
       flag == 0: num, dpnd_head ���ֹ���դ��ؤ��Ϥ��ʤ� */

    int	i, j, count = -1, t_table[TAG_MAX], b_table[BNST_MAX], merge_to;
    TAG_DATA *t_ptr;
    BNST_DATA *b_ptr;
    char *cp;

    /* ����������ѥ롼���Ŭ�� 
       FEATURE�����¤Ϥ�����ǹԤ� */
    assign_general_feature(sp->tag_data, sp->Tag_num, PostProcessTagRuleType, FALSE, FALSE);

    /* �ޡ������륿����ʸ��ν��� */
    for (i = 0, t_ptr = sp->tag_data; i < sp->Tag_num; i++, t_ptr++) {
	/* ��Ȥ�num, mrph_num����¸ */
	t_ptr->preserve_mrph_num = t_ptr->mrph_num;
	if (t_ptr->bnum >= 0) { /* ʸ����ڤ�Ǥ⤢��Ȥ� */
	    t_ptr->b_ptr->preserve_mrph_num = t_ptr->b_ptr->mrph_num;
	}

	if (check_feature(t_ptr->f, "�ԥޡ�����")) {
	    for (merge_to = i - 1; merge_to >= 0; merge_to--) {
		if ((sp->tag_data + merge_to)->num >= 0) {
		    break;
		}
	    }
	    t_ptr->num = -1; /* ̵���ʥ���ñ�̤Ǥ������Ĥ��Ƥ��� */
	    (sp->tag_data + merge_to)->mrph_num += t_ptr->mrph_num;
	    (sp->tag_data + merge_to)->dpnd_head = t_ptr->dpnd_head;
	    (sp->tag_data + merge_to)->dpnd_type = t_ptr->dpnd_type;
	    for (j = 0; j < t_ptr->mrph_num; j++) {
		(sp->tag_data + merge_to)->length += strlen((t_ptr->mrph_ptr + j)->Goi2);
	    }

	    /* feature�ν񤭴����ϻ���Ū�����
	    assign_cfeature(&((sp->tag_data + i - 1)->f), "�����ۼ�");
	    delete_cfeature(&(t_ptr->mrph_ptr->f), "ʸ���");
	    delete_cfeature(&(t_ptr->mrph_ptr->f), "����ñ�̻�");
	    */

	    if (t_ptr->bnum >= 0) { /* ʸ����ڤ�Ǥ⤢��Ȥ� */
		for (merge_to = -1; t_ptr->b_ptr->num + merge_to >= 0; merge_to--) {
		    if ((t_ptr->b_ptr + merge_to)->num >= 0) {
			break;
		    }
		}
		copy_para_info(sp, t_ptr->b_ptr + merge_to, t_ptr->b_ptr);
		t_ptr->b_ptr->num = -1;
		(t_ptr->b_ptr + merge_to)->mrph_num += t_ptr->b_ptr->mrph_num;
		(t_ptr->b_ptr + merge_to)->dpnd_head = t_ptr->b_ptr->dpnd_head;
		(t_ptr->b_ptr + merge_to)->dpnd_type = t_ptr->b_ptr->dpnd_type;
		for (j = 0; j < t_ptr->mrph_num; j++) {
		    (t_ptr->b_ptr + merge_to)->length += strlen((t_ptr->b_ptr->mrph_ptr + j)->Goi2);
		}
	    }
	}
	else {
	    count++;
	}
	t_table[i] = count;
    }

    if (flag == 0) {
	return;
    }

    count = -1;
    for (i = 0, b_ptr = sp->bnst_data; i < sp->Bnst_num; i++, b_ptr++) {
	if (b_ptr->num != -1) {
	    count++;
	}
	b_table[i] = count;
    }

    /* ����ñ���ֹ�ι��� */
    for (i = 0, t_ptr = sp->tag_data; i < sp->Tag_num; i++, t_ptr++) {
	if (t_ptr->num != -1) { /* num�ι��� (���ɤ����� tag_data + num �򤹤�Ȥ���) */
	    t_ptr->num = t_table[i];
	    if (t_ptr->dpnd_head != -1) {
		t_ptr->dpnd_head = t_table[t_ptr->dpnd_head];
	    }
	}
	if (t_ptr->bnum >= 0) { /* bnum�ι��� (���ɤ����� bnst_data + bnum �򤹤�Ȥ���) */
	    t_ptr->bnum = b_table[t_ptr->bnum];
	}
    }

    /* ʸ���ֹ�ι��� */
    for (i = 0, b_ptr = sp->bnst_data; i < sp->Bnst_num; i++, b_ptr++) {
	if (b_ptr->num != -1) {
	    b_ptr->num = b_table[i];
	    if (b_ptr->dpnd_head != -1) {
		b_ptr->dpnd_head = b_table[b_ptr->dpnd_head];
	    }
	}
    }
}

/*==================================================================*/
	  void undo_tag_bnst_postprocess(SENTENCE_DATA *sp)
/*==================================================================*/
{
    int i, b_count = 0;
    TAG_DATA *t_ptr;

    /* nbest���ץ����ʤɤ�print_result()��ʣ����ƤФ��Ȥ��Τ����
       �ѹ�����num, mrph_num, length�򸵤��ᤷ�Ƥ��� */

    for (i = 0, t_ptr = sp->tag_data; i < sp->Tag_num; i++, t_ptr++) {
	t_ptr->num = i;
	t_ptr->mrph_num = t_ptr->preserve_mrph_num;
	calc_bnst_length(sp, (BNST_DATA *)t_ptr);

	if (t_ptr->bnum >= 0) { /* ʸ����ڤ�Ǥ⤢��Ȥ� */
	    t_ptr->b_ptr->num = b_count++;
	    t_ptr->bnum = t_ptr->b_ptr->num;
	    t_ptr->b_ptr->mrph_num = t_ptr->b_ptr->preserve_mrph_num;
	    calc_bnst_length(sp, t_ptr->b_ptr);
	}
    }    
}

/*==================================================================*/
	       void para_postprocess(SENTENCE_DATA *sp)
/*==================================================================*/
{
    int i;

    for (i = 0; i < sp->Bnst_num; i++) {
	if (check_feature((sp->bnst_data + i)->f, "�Ѹ�") &&
	    !check_feature((sp->bnst_data + i)->f, "���Ȥߤ���") &&
	    (sp->bnst_data + i)->para_num != -1 &&
	    sp->para_data[(sp->bnst_data + i)->para_num].status != 'x') {
	    
	    assign_cfeature(&((sp->bnst_data + i)->f), "�����:30", FALSE);
	    assign_cfeature(&(((sp->bnst_data + i)->tag_ptr + 
			       (sp->bnst_data + i)->tag_num - 1)->f), "�����:30", FALSE);
	}
    }
}

/*==================================================================*/
	  void dpnd_evaluation(SENTENCE_DATA *sp, DPND dpnd)
/*==================================================================*/
{
    int i, j, k, one_score, score, rentai, vacant_slot_num;
    int topic_score;
    int scase_check[SCASE_CODE_SIZE], ha_check, un_count, pred_p;
    char *cp, *cp2, *buffer;
    BNST_DATA *g_ptr, *d_ptr;

    /* ��¸��¤������ɾ��������δؿ�
       (��ʸ��ˤĤ��ơ������˷��äƤ���ʸ���ɾ������׻�)

       ɾ�����
       ========
       0. �������default���֤Ȥκ���ڥʥ�ƥ���(kakari_uke.rule)

       1. �֡��ϡ�(����,��:̤��)�η������ͥ�褵����Τ�����
       		(bnst_etc.rule�ǻ��ꡤ����Υ�����������ϸ�ץ����ǻ���)

       2. �֡��ϡפϰ�Ҹ�˰�ķ��뤳�Ȥ�ͥ��(����,���̤���)

       3. ���٤Ƥγ����Ǥ�Ʊ��ɽ�سʤ���Ҹ�˰�ķ��뤳�Ȥ�ͥ��(��������)

       4. ̤�ʡ�Ϣ�ν�����ϥ�,��,�˳ʤ�;�äƤ��륹��åȿ�����������Ϳ
    */

    score = 0;
    for (i = 1; i < sp->Bnst_num; i++) {
	g_ptr = sp->bnst_data + i;

	one_score = 0;
	for (k = 0; k < SCASE_CODE_SIZE; k++) scase_check[k] = 0;
	ha_check = 0;
	un_count = 0;

	if (check_feature(g_ptr->f, "�Ѹ�") ||
	    check_feature(g_ptr->f, "���Ѹ�")) {
	    pred_p = 1;
	} else {
	    pred_p = 0;
	}

	for (j = i-1; j >= 0; j--) {
	    d_ptr = sp->bnst_data + j;

	    if (dpnd.head[j] == i) {

		/* �������DEFAULT�ΰ��֤Ȥκ���ڥʥ�ƥ���
		     �� �����C,B'����Ʊ󤯤˷��뤳�Ȥ����뤬�����줬
		        ¾�η�����˱ƶ����ʤ��褦,�ڥʥ�ƥ��˺���Ĥ��� */

		if (check_feature(d_ptr->f, "����")) {
		    one_score -= dpnd.dflt[j];
		} else {
		    one_score -= dpnd.dflt[j] * 2;
		}
	    
		/* �������Ĥ�Τ��٤ˤ����뤳�Ȥ��ɤ� */

		if (j + 1 == i && check_feature(d_ptr->f, "����")) {
		    one_score -= 5;
		}

		if (pred_p &&
		    (cp = check_feature(d_ptr->f, "��")) != NULL) {
		    
		    /* ̤�� ����(�֡��ϡ�)�ΰ��� */

		    if (check_feature(d_ptr->f, "����") &&
			!strcmp(cp, "��:̤��")) {

			/* ʸ��, �֡����פʤ�, ������, C, B'�˷��뤳�Ȥ�ͥ�� */

			if ((cp2 = check_feature(g_ptr->f, "�����")) 
			    != NULL) {
			    sscanf(cp2, "%*[^:]:%d", &topic_score);
			    one_score += topic_score;
			}
			/* else {one_score -= 15;} */

			/* ��Ĥ������ˤ�������Ϳ���� (����,���̤���)
			     �� ʣ�������꤬Ʊ��Ҹ�˷��뤳�Ȥ��ɤ� */

			if (check_feature(d_ptr->f, "����") ||
			    check_feature(d_ptr->f, "����")) {
			    one_score += 10;
			} else if (ha_check == 0){
			    one_score += 10;
			    ha_check = 1;
			}
		    }

		    k = case2num(cp+3);

		    /* �����ǰ��̤ΰ��� */

		    /* ̤�� : �����Ƥ�������Ƕ�����åȤ�Ĵ�٤� (����,���̤���) */

		    if (!strcmp(cp, "��:̤��")) {
			if (check_feature(d_ptr->f, "����") ||
			    check_feature(d_ptr->f, "����")) {
			    one_score += 10;
			} else {
			    un_count++;
			}
		    }

		    /* �γ� : �θ��ʳ��ʤ� break 
		       	      �� ���������γ����Ǥˤ�����Ϳ���ʤ���
			      �� �γʤ�������Ф��������γʤϤ�����ʤ�

			      �� ���θ��פȤ����Τ�Ƚ���Τ��ȡ�������
			         ʸ���ʤɤǤ��Ѹ�:ư�ȤʤäƤ��뤳�Ȥ�
				 ����Τǡ����θ��פǥ����å� */

		    else if (!strcmp(cp, "��:�γ�")) {
			if (!check_feature(g_ptr->f, "�θ�")) {
			    one_score += 10;
			    break;
			}
		    } 

		    /* ���� : ������ʸ������ΤǾ���ʣ�� */

		    else if (!strcmp(cp, "��:����")) {
			if (g_ptr->SCASE_code[case2num("����")] &&
			    scase_check[case2num("����")] == 0) {
			    one_score += 10;
			    scase_check[case2num("����")] = 1;
			} 
			else if (g_ptr->SCASE_code[case2num("����")] &&
			    scase_check[case2num("����")] == 0) {
			    one_score += 10;
			    scase_check[case2num("����")] = 1;
			}
		    }

		    /* ¾�γ� : �Ƴ�1�Ĥ������򤢤�����
		       �� �˳ʤξ�硤���֤Ȥ���ʳ��϶��̤��������������⡩ */
		    else if (k != -1) {
			if (scase_check[k] == 0) {
			    scase_check[k] = 1;
			    one_score += 10;
			} 
		    }

		    /* �֡�����Τϡ����פ˥ܡ��ʥ� 01/01/11
		       �ۤȤ�ɤξ�������

		       ������)
		       �ֹ��Ĥ����Τ� Ǥ���� ���ݤ���� ��ͳ�� ��Ĥ餷����

		       �ֻȤ��Τ� ������ ���Ȥ�����
		       �ֱ�������� �ʤ뤫�ɤ����� ��̯�� �Ȥ��������
		       		�� ��������ϡ֤���/�Ȥ�����פ˷���Ȱ���

		       ��¾�ͤ� ������Τ� �����ˤʤ� ������Ǥ���
		       		�� �������ۣ�������ʸ̮��������

		       ��������)
		       �֤��줬 �֣ͣФ� ʬ����ʤ� ���Ǥ��礦��
		       �֡� ����ʤ� ���� ��������
		       �֥ӥ��� ���Τ� ���Ѥ� ���塣��
		       ���Ȥ� ��ޤ�Τ� �򤱤�줽���ˤʤ� ���Ԥ�������
		       �֤��ޤ� ��Ω�ĤȤ� �פ��ʤ� ����������
		       �֤ɤ� �ޤ�礦���� ����뤵��Ƥ��� ˡ������
		       ��ǧ����뤫�ɤ����� ���줿 ��Ƚ�ǡ�

		       �����ꢨ
		       �֤�������פ��� �Τ褦�ʾ����Ѹ��Ȥߤʤ����Τ�����
		     */

		    if (check_feature(d_ptr->f, "�Ѹ�") &&
			(check_feature(d_ptr->f, "��:̤��") ||
			 check_feature(d_ptr->f, "��:����")) &&
			check_feature(g_ptr->f, "�Ѹ�:Ƚ")) {
		      one_score += 3;
		    }
		}
	    }
	}

	/* �Ѹ��ξ�硤�ǽ�Ū��̤��,����,���,�˳�,Ϣ�ν������Ф���
	   ����,���,�˳ʤΥ���å�ʬ����������Ϳ���� */

	if (pred_p) {

	    /* Ϣ�ν����ξ�硤���褬
	       ������̾��,����Ū̾��
	       ����ͽ���,�ָ����ߡפʤ�
	       �Ǥʤ���а�Ĥγ����Ǥȹͤ��� */

	    if (check_feature(g_ptr->f, "��:Ϣ��")) {
		if (check_feature(sp->bnst_data[dpnd.head[i]].f, "���δط�") || 
		    check_feature(sp->bnst_data[dpnd.head[i]].f, "�롼�볰�δط�")) {
		    rentai = 0;
		    one_score += 10;	/* ���δط��ʤ餳���ǲ��� */
		} else {
		    rentai = 1;	/* ����ʳ��ʤ��Ƕ�������åȤ�����å� */
		}
	    } else {
		rentai = 0;
	    }

	    /* �����Ƥ��륬��,���,�˳�,���� */

	    vacant_slot_num = 0;
	    if ((g_ptr->SCASE_code[case2num("����")]
		 - scase_check[case2num("����")]) == 1) {
		vacant_slot_num ++;
	    }
	    if ((g_ptr->SCASE_code[case2num("���")]
		 - scase_check[case2num("���")]) == 1) {
		vacant_slot_num ++;
	    }
	    if ((g_ptr->SCASE_code[case2num("�˳�")]
		 - scase_check[case2num("�˳�")]) == 1 &&
		rentai == 1 &&
		check_feature(g_ptr->f, "�Ѹ�:ư")) {
		vacant_slot_num ++;
		/* �˳ʤ�ư���Ϣ�ν����ξ�������θ���Ĥޤ�Ϣ��
		   �����˳�����Ƥ�����ǡ�̤�ʤΥ���åȤȤϤ��ʤ� */
	    }
	    if ((g_ptr->SCASE_code[case2num("����")]
		 - scase_check[case2num("����")]) == 1) {
		vacant_slot_num ++;
	    }

	    /* ��������å�ʬ����Ϣ�ν�����̤�ʤ˥�������Ϳ���� */

	    if ((rentai + un_count) <= vacant_slot_num) 
		one_score += (rentai + un_count) * 10;
	    else 
		one_score += vacant_slot_num * 10;
	}

	score += one_score;

	if (OptDisplay == OPT_DEBUG) {
	    if (i == 1) 
		fprintf(Outfp, "Score:    ");
	    if (pred_p) {
		fprintf(Outfp, "%2d*", one_score);
	    } else {
		fprintf(Outfp, "%2d ", one_score);
	    }
	}
    }

    if (OptDisplay == OPT_DEBUG) {
	fprintf(Outfp, "=%d\n", score);
    }

    if (OptDisplay == OPT_DEBUG || OptNbest == TRUE) {
	dpnd_info_to_bnst(sp, &dpnd);
	if (!(OptExpress & OPT_NOTAG)) {
	    dpnd_info_to_tag(sp, &dpnd); 
	}
	if (make_dpnd_tree(sp)) {
	    if (!(OptExpress & OPT_NOTAG)) {
		bnst_to_tag_tree(sp); /* ����ñ�̤��ڤ� */
	    }

	    if (OptNbest == TRUE) {
		sp->score = score;
		print_result(sp, 0);
	    }
	    else {
		print_kakari(sp, OptExpress & OPT_NOTAG ? OPT_NOTAGTREE : OPT_TREE);
	    }
	}
    }

    if (score > sp->Best_mgr->score) {
	sp->Best_mgr->dpnd = dpnd;
	sp->Best_mgr->score = score;
	sp->Best_mgr->ID = dpndID;
	Possibility++;
    }

    dpndID++;
}

/*==================================================================*/
void count_dpnd_candidates(SENTENCE_DATA *sp, DPND *dpnd, int pos)
/*==================================================================*/
{
    int i, count = 0, d_possibility = 1;
    BNST_DATA *b_ptr = sp->bnst_data + pos;

    if (pos == -1) {
	return;
    }

    if (pos < sp->Bnst_num - 2) {
	for (i = pos + 2; i < dpnd->head[pos + 1]; i++) {
	    dpnd->mask[i] = 0;
	}
    }

    for (i = pos + 1; i < sp->Bnst_num; i++) {
	if (Quote_matrix[pos][i] &&
	    dpnd->mask[i]) {

	    if (d_possibility && Dpnd_matrix[pos][i] == 'd') {
		if (check_uncertain_d_condition(sp, dpnd, i)) {
		    dpnd->check[pos].pos[count] = i;
		    count++;
		}
		d_possibility = 0;
	    }
	    else if (Dpnd_matrix[pos][i] && 
		     Dpnd_matrix[pos][i] != 'd') {
		dpnd->check[pos].pos[count] = i;
		count++;
		d_possibility = 0;
	    }

	    /* �Хꥢ�Υ����å� */
	    if (count && 
		b_ptr->dpnd_rule->barrier.fp[0] && 
		feature_pattern_match(&(b_ptr->dpnd_rule->barrier), 
				      sp->bnst_data[i].f, 
				      b_ptr, sp->bnst_data + i) == TRUE) {
		break;
	    }
	}
    }

    if (count) {
	dpnd->check[pos].num = count;	/* ����� */
	dpnd->check[pos].def = b_ptr->dpnd_rule->preference == -1 ? count : b_ptr->dpnd_rule->preference;	/* �ǥե���Ȥΰ��� */
    }

    count_dpnd_candidates(sp, dpnd, pos - 1);
}

/*==================================================================*/
    void call_count_dpnd_candidates(SENTENCE_DATA *sp, DPND *dpnd)
/*==================================================================*/
{
    int i;

    for (i = 0; i < sp->Bnst_num; i++) {
	dpnd->mask[i] = 1;
	memset(&(dpnd->check[i]), 0, sizeof(CHECK_DATA));
	dpnd->check[i].num = -1;
    }

    count_dpnd_candidates(sp, dpnd, sp->Bnst_num - 1);
}

/*==================================================================*/
	    void decide_dpnd(SENTENCE_DATA *sp, DPND dpnd)
/*==================================================================*/
{
    int i, count, possibilities[BNST_MAX], default_pos, d_possibility;
    int MaskFlag = 0;
    char *cp;
    BNST_DATA *b_ptr;
    
    if (OptDisplay == OPT_DEBUG) {
	if (dpnd.pos == sp->Bnst_num - 1) {
	    fprintf(Outfp, "------");
	    for (i = 0; i < sp->Bnst_num; i++)
	      fprintf(Outfp, "-%02d", i);
	    fputc('\n', Outfp);
	}
	fprintf(Outfp, "In %2d:", dpnd.pos);
	for (i = 0; i < sp->Bnst_num; i++)
	    fprintf(Outfp, " %2d", dpnd.head[i]);
	fputc('\n', Outfp);
    }

    dpnd.pos --;

    /* ʸƬ�ޤǲ��Ϥ�����ä���ɾ���ؿ����� */

    if (dpnd.pos == -1) {
	/* ̵�ʽ�°: ����ʸ��η�������˽������ */
	for (i = 0; i < sp->Bnst_num -1; i++)
	    if (dpnd.head[i] < 0) {
		/* ���ꤨ�ʤ�������� */
		if (i >= dpnd.head[i + dpnd.head[i]]) {
		    return;
		}
		dpnd.head[i] = dpnd.head[i + dpnd.head[i]];
		dpnd.check[i].pos[0] = dpnd.head[i];
	    }

	if (OptAnalysis == OPT_DPND ||
	    OptAnalysis == OPT_CASE2) {
	    dpnd_evaluation(sp, dpnd);
	} 
	else if (OptAnalysis == OPT_CASE) {
	    call_case_analysis(sp, dpnd);
	}
	return;
    }

    b_ptr = sp->bnst_data + dpnd.pos;
    dpnd.f[dpnd.pos] = b_ptr->f;

    /* (���η���ˤ��)��򺹾������� (dpnd.mask �� 0 �ʤ鷸��ʤ�) */

    if (dpnd.pos < sp->Bnst_num - 2)
	for (i = dpnd.pos + 2; i < dpnd.head[dpnd.pos + 1]; i++)
	    dpnd.mask[i] = 0;
    
    /* ����¤�Υ���ʸ��, ��ʬ�����ʸ��<I>
       (���Ǥ˹Ԥ�줿����¤���Ϥη�̤�ޡ����������) */

    for (i = dpnd.pos + 1; i < sp->Bnst_num; i++) {
	if (Mask_matrix[dpnd.pos][i] == 2) {
	    dpnd.head[dpnd.pos] = i;
	    dpnd.type[dpnd.pos] = 'P';
	    /* �����å��� */
	    /* ����ξ��ϰ�դ˷�ޤäƤ���Τǡ������󤲤�Τϰ�̣���ʤ� */
	    dpnd.check[dpnd.pos].num = 1;
	    dpnd.check[dpnd.pos].pos[0] = i;
	    decide_dpnd(sp, dpnd);
	    return;
	} else if (Mask_matrix[dpnd.pos][i] == 3) {
	    dpnd.head[dpnd.pos] = i;
	    dpnd.type[dpnd.pos] = 'I';

	    dpnd.check[dpnd.pos].num = 1;
	    dpnd.check[dpnd.pos].pos[0] = i;
	    decide_dpnd(sp, dpnd);
	    return;
	}
    }

    /* ����ʸ��η�������˽������  ��) �֡������Τϰ��������� */

    if ((cp = check_feature(b_ptr->f, "��:̵�ʽ�°")) != NULL) {
        sscanf(cp, "%*[^:]:%*[^:]:%d", &(dpnd.head[dpnd.pos]));
        dpnd.type[dpnd.pos] = 'D';
        dpnd.dflt[dpnd.pos] = 0;
	dpnd.check[dpnd.pos].num = 1;
        decide_dpnd(sp, dpnd);
        return;
    }

    /* �̾�η���������� */

    /* ������θ����Ĵ�٤� */
    
    count = 0;
    d_possibility = 1;
    for (i = dpnd.pos + 1; i < sp->Bnst_num; i++) {
	if (Mask_matrix[dpnd.pos][i] &&
	    Quote_matrix[dpnd.pos][i] &&
	    dpnd.mask[i]) {

	    if (d_possibility && Dpnd_matrix[dpnd.pos][i] == 'd') {
		if (check_uncertain_d_condition(sp, &dpnd, i)) {
		    possibilities[count] = i;
		    count++;
		}
		d_possibility = 0;
	    }
	    else if (Dpnd_matrix[dpnd.pos][i] && 
		     Dpnd_matrix[dpnd.pos][i] != 'd') {
		possibilities[count] = i;
		count++;
		d_possibility = 0;
	    }

	    /* �Хꥢ�Υ����å� */
	    if (count &&
		b_ptr->dpnd_rule->barrier.fp[0] &&
		feature_pattern_match(&(b_ptr->dpnd_rule->barrier), 
				      sp->bnst_data[i].f,
				      b_ptr, sp->bnst_data + i) == TRUE)
		break;
	}
	else {
	    MaskFlag = 1;
	}
    }

    /* �ºݤ˸����Ĥ��äƤ���(���δؿ��κƵ�Ū�ƤӽФ�) */

    if (count) {

	/* preference �ϰ��ֶ᤯:1, ������:2, �Ǹ�:-1
	   default_pos �ϰ��ֶ᤯:1, ������:2, �Ǹ�:count ���ѹ� */

	default_pos = (b_ptr->dpnd_rule->preference == -1) ?
	    count: b_ptr->dpnd_rule->preference;

	dpnd.check[dpnd.pos].num = count;	/* ����� */
	dpnd.check[dpnd.pos].def = default_pos;	/* �ǥե���Ȥΰ��� */
	for (i = 0; i < count; i++) {
	    dpnd.check[dpnd.pos].pos[i] = possibilities[i];
	}

	/* ��դ˷��ꤹ���� */

	if (b_ptr->dpnd_rule->barrier.fp[0] == NULL || 
	    b_ptr->dpnd_rule->decide) {
	    if (default_pos <= count) {
		dpnd.head[dpnd.pos] = possibilities[default_pos - 1];
	    } else {
		dpnd.head[dpnd.pos] = possibilities[count - 1];
		/* default_pos �� 2 �ʤΤˡ�count�� 1 �����ʤ���� */
	    }
	    dpnd.type[dpnd.pos] = Dpnd_matrix[dpnd.pos][dpnd.head[dpnd.pos]];
	    dpnd.dflt[dpnd.pos] = 0;
	    decide_dpnd(sp, dpnd);
	} 

	/* ���٤Ƥβ�ǽ����Ĥ���Ф���� */
	/* ��֤η�������ξ��ϰ�դ˷���٤� */

	else {
	    for (i = 0; i < count; i++) {
		dpnd.head[dpnd.pos] = possibilities[i];
		dpnd.type[dpnd.pos] = Dpnd_matrix[dpnd.pos][dpnd.head[dpnd.pos]];
		dpnd.dflt[dpnd.pos] = abs(default_pos - 1 - i);
		decide_dpnd(sp, dpnd);
	    }
	}
    } 

    /* �����褬�ʤ����
       ʸ��������˥ޥ�������Ƥ��ʤ���С�ʸ���˷���Ȥ��� */

    else {
	if (Mask_matrix[dpnd.pos][sp->Bnst_num - 1]) {
	    dpnd.head[dpnd.pos] = sp->Bnst_num - 1;
	    dpnd.type[dpnd.pos] = 'D';
	    dpnd.dflt[dpnd.pos] = 10;
	    dpnd.check[dpnd.pos].num = 1;
	    dpnd.check[dpnd.pos].pos[0] = sp->Bnst_num - 1;
	    decide_dpnd(sp, dpnd);
	}
    }
}

/*==================================================================*/
	     void when_no_dpnd_struct(SENTENCE_DATA *sp)
/*==================================================================*/
{
    int i;

    sp->Best_mgr->dpnd.head[sp->Bnst_num - 1] = -1;
    sp->Best_mgr->dpnd.type[sp->Bnst_num - 1] = 'D';

    for (i = sp->Bnst_num - 2; i >= 0; i--) {
	sp->Best_mgr->dpnd.head[i] = i + 1;
	sp->Best_mgr->dpnd.type[i] = 'D';
	sp->Best_mgr->dpnd.check[i].num = 1;
	sp->Best_mgr->dpnd.check[i].pos[0] = i + 1;
    }

    sp->Best_mgr->score = 0;
}

/*==================================================================*/
	       int after_decide_dpnd(SENTENCE_DATA *sp)
/*==================================================================*/
{
    int i, j;
    TAG_DATA *check_b_ptr;
    
    /* ���Ϻ�: ��¤��Ϳ����줿���1�ĤΤ� */
    if (OptInput & OPT_PARSED) {
	Possibility = 1;
    }

    if (Possibility != 0) {
	/* ��¸��¤����� �ʲ��Ϥ�Ԥ���� */
	if (OptAnalysis == OPT_CASE2) {
	    sp->Best_mgr->score = -10000;
	    if (call_case_analysis(sp, sp->Best_mgr->dpnd) == FALSE) {
		return FALSE;
	    }
	}

	/* ��¸��¤���ʹ�¤�����ν��� */

	/* �ʲ��Ϸ�̤ξ����feature�� */
	if (OptAnalysis == OPT_CASE || OptAnalysis == OPT_CASE2) {
	    /* �ʲ��Ϸ�̤��Ѹ����ܶ�feature�� */
	    for (i = 0; i < sp->Best_mgr->pred_num; i++) {
		assign_nil_assigned_components(sp, &(sp->Best_mgr->cpm[i])); /* ̤�б������Ǥν��� */

		assign_case_component_feature(sp, &(sp->Best_mgr->cpm[i]), FALSE);

		/* �ʥե졼��ΰ�̣������Ѹ����ܶ�feature�� */
		for (j = 0; j < sp->Best_mgr->cpm[i].cmm[0].cf_ptr->element_num; j++) {
		    append_cf_feature(&(sp->Best_mgr->cpm[i].pred_b_ptr->f), 
				      &(sp->Best_mgr->cpm[i]), sp->Best_mgr->cpm[i].cmm[0].cf_ptr, j);
		}
	    }
	}

	/* ��¤�����Υ롼��Ŭ�ѽ��� */
	dpnd_info_to_bnst(sp, &(sp->Best_mgr->dpnd));
	if (make_dpnd_tree(sp) == FALSE) {
	    return FALSE;
	}
	bnst_to_tag_tree(sp); /* ����ñ�̤��ڤ� */

	/* ��¤�����Υ롼��Ŭ�� */
	assign_general_feature(sp->tag_data, sp->Tag_num, AfterDpndTagRuleType, FALSE, FALSE);

	if (OptAnalysis == OPT_CASE || OptAnalysis == OPT_CASE2) {
	    /* �ʲ��Ϥη�̤��Ѹ�ʸ��� */
	    for (i = 0; i < sp->Best_mgr->pred_num; i++) {
		sp->Best_mgr->cpm[i].pred_b_ptr->cpm_ptr = &(sp->Best_mgr->cpm[i]);
		/* �� ����Ū
		   ����ΤȤ��� make_dpnd_tree() ��ƤӽФ��� cpm_ptr ���ʤ��ʤ�Τǡ�
		   �����ǥ��ԡ����Ƥ��� */
		check_b_ptr = sp->Best_mgr->cpm[i].pred_b_ptr;
		while (check_b_ptr->parent && check_b_ptr->parent->para_top_p == TRUE && 
		       check_b_ptr->parent->cpm_ptr == NULL) {
		    check_b_ptr->parent->cpm_ptr = &(sp->Best_mgr->cpm[i]);
		    check_b_ptr = check_b_ptr->parent;
		}

		/* �Ƴ����Ǥο��Ѹ�������
		   �� ʸ̮���ϤΤȤ��˳ʥե졼�����ꤷ�Ƥʤ��Ƥ�ʲ��ϤϹԤäƤ���Τ�
		      ������������� */
		for (j = 0; j < sp->Best_mgr->cpm[i].cf.element_num; j++) {
		    /* ��ά���Ϥη�� or Ϣ�ν����Ͻ��� */
		    if (sp->Best_mgr->cpm[i].elem_b_num[j] <= -2 || 
			sp->Best_mgr->cpm[i].elem_b_ptr[j]->num > sp->Best_mgr->cpm[i].pred_b_ptr->num) {
			continue;
		    }
		    sp->Best_mgr->cpm[i].elem_b_ptr[j]->pred_b_ptr = sp->Best_mgr->cpm[i].pred_b_ptr;
		}

		/* �ʥե졼�ब������ */
		if (sp->Best_mgr->cpm[i].result_num != 0 && 
		    sp->Best_mgr->cpm[i].cmm[0].cf_ptr->cf_address != -1 && 
		    (((OptCaseFlag & OPT_CASE_USE_PROBABILITY) && 
		      sp->Best_mgr->cpm[i].cmm[0].score != CASE_MATCH_FAILURE_PROB) || 
		     (!(OptCaseFlag & OPT_CASE_USE_PROBABILITY) && 
		      sp->Best_mgr->cpm[i].cmm[0].score != CASE_MATCH_FAILURE_SCORE))) {
		    /* ʸ̮���ϤΤȤ��ϳʥե졼����ꤷ�Ƥ����Ѹ��ˤĤ��ƤΤ� */
		    if (!OptEllipsis || sp->Best_mgr->cpm[i].decided == CF_DECIDED) {
			if (OptCaseFlag & OPT_CASE_ASSIGN_GA_SUBJ) {
			    assign_ga_subject(sp, &(sp->Best_mgr->cpm[i]));
			}
			fix_sm_place(sp, &(sp->Best_mgr->cpm[i]));

			if (OptUseSmfix == TRUE) {
			    specify_sm_from_cf(sp, &(sp->Best_mgr->cpm[i]));
			}

			/* �ޥå����������feature�˽��� *
			   record_match_ex(sp, &(sp->Best_mgr->cpm[i])); */

			/* ľ���ʤΥޥå���������feature�˽��� *
			   record_closest_cc_match(sp, &(sp->Best_mgr->cpm[i])); */

			/* �ʲ��Ϥη�̤� feature�� */
			record_case_analysis(sp, &(sp->Best_mgr->cpm[i]), NULL, FALSE);

			/* �ʲ��Ϥη�̤��Ѥ��Ʒ�����ۣ�������� */
			verb_lexical_disambiguation_by_case_analysis(&(sp->Best_mgr->cpm[i]));
			noun_lexical_disambiguation_by_case_analysis(&(sp->Best_mgr->cpm[i]));
		    }
		    else if (sp->Best_mgr->cpm[i].decided == CF_CAND_DECIDED) {
			if (OptCaseFlag & OPT_CASE_ASSIGN_GA_SUBJ) {
			    assign_ga_subject(sp, &(sp->Best_mgr->cpm[i]));
			}
		    }

		    if (sp->Best_mgr->cpm[i].decided == CF_DECIDED) {
			assign_cfeature(&(sp->Best_mgr->cpm[i].pred_b_ptr->f), "�ʥե졼�����", FALSE);
		    }
		}
		/* �ʥե졼��ʤ�����ʲ��Ϸ�̤�� */
		else if (!(OptCaseFlag & OPT_CASE_USE_PROBABILITY) && 
			 (sp->Best_mgr->cpm[i].result_num == 0 || 
			  sp->Best_mgr->cpm[i].cmm[0].cf_ptr->cf_address == -1)) {
		    record_case_analysis(sp, &(sp->Best_mgr->cpm[i]), NULL, FALSE);
		}
	    }
	}
	return TRUE;
    }
    else { 
	return FALSE;
    }
}

/*==================================================================*/
	    int detect_dpnd_case_struct(SENTENCE_DATA *sp)
/*==================================================================*/
{
    int i;
    DPND dpnd;

    sp->Best_mgr->score = -10000; /* �������ϡ֤���礭���׻������촹����Τǡ�
				    ����ͤϽ�ʬ���������Ƥ��� */
    sp->Best_mgr->dflt = 0;
    sp->Best_mgr->ID = -1;
    Possibility = 0;
    dpndID = 0;

    /* ������֤ν���� */

    for (i = 0; i < sp->Bnst_num; i++) {
	dpnd.head[i] = -1;
	dpnd.type[i] = 'D';
	dpnd.dflt[i] = 0;
	dpnd.mask[i] = 1;
	memset(&(dpnd.check[i]), 0, sizeof(CHECK_DATA));
	dpnd.check[i].num = -1;
	dpnd.f[i] = NULL;
    }
    dpnd.pos = sp->Bnst_num - 1;

    /* �ʲ��ϥ���å���ν���� */
    if (OptAnalysis == OPT_CASE) {
	InitCPMcache();
    }

    /* ��¸��¤���� --> �ʹ�¤���� */

    decide_dpnd(sp, dpnd);

    /* �ʲ��ϥ���å���ν���� */
    if (OptAnalysis == OPT_CASE) {
	ClearCPMcache();
    }

    return after_decide_dpnd(sp);
}

/*==================================================================*/
	       void check_candidates(SENTENCE_DATA *sp)
/*==================================================================*/
{
    int i, j;
    TOTAL_MGR *tm = sp->Best_mgr;
    char buffer[DATA_LEN], buffer2[SMALL_DATA_LEN], *cp;

    /* ��ʸ�ᤴ�Ȥ˥����å��Ѥ� feature ��Ϳ���� */
    for (i = 0; i < sp->Bnst_num; i++)
	if (tm->dpnd.check[i].num != -1) {
	    /* ����¦ -> ������ */
	    sprintf(buffer, "����");
	    for (j = 0; j < tm->dpnd.check[i].num; j++) {
		/* ���䤿�� */
		sprintf(buffer2, ":%d", tm->dpnd.check[i].pos[j]);
		if (strlen(buffer)+strlen(buffer2) >= DATA_LEN) {
		    fprintf(stderr, ";; Too long string <%s> (%d) in check_candidates. (%s)\n", 
			    buffer, tm->dpnd.check[i].num, sp->KNPSID ? sp->KNPSID+5 : "?");
		    return;
		}
		strcat(buffer, buffer2);
	    }
	    assign_cfeature(&(sp->bnst_data[i].f), buffer, FALSE);
	}
}

/*==================================================================*/
	       void memo_by_program(SENTENCE_DATA *sp)
/*==================================================================*/
{
    /*
     *  �ץ����ˤ����ؤν񤭹���
     */

    /* ���¤���˵�Ͽ������
    int i;

    for (i = 0; i < sp->Bnst_num - 1; i++) {
	if (sp->Best_mgr->dpnd.type[i] == 'd') {
	    strcat(PM_Memo, " ����d");
	    sprintf(PM_Memo+strlen(PM_Memo), "(%d)", i);
	} else if (sp->Best_mgr->dpnd.type[i] == 'R') {
	    strcat(PM_Memo, " ����R");
	    sprintf(PM_Memo+strlen(PM_Memo), "(%d)", i);
	}
    }
    */

    /* �󤤷����������˵�Ͽ������

    for (i = 0; i < sp->Bnst_num - 1; i++) {
	if (sp->Best_mgr->dpnd.head[i] > i + 3 &&
	    !check_feature(sp->bnst_data[i].f, "��") &&
	    !check_feature(sp->bnst_data[i].f, "����") &&
	    !check_feature(sp->bnst_data[i].f, "�Ѹ�") &&
	    !check_feature(sp->bnst_data[i].f, "��:����") &&
	    !check_feature(sp->bnst_data[i].f, "�Ѹ�:̵") &&
	    !check_feature(sp->bnst_data[i].f, "�¥�") &&
	    !check_feature(sp->bnst_data[i+1].f, "��̻�")) {
	    strcat(PM_Memo, " ��");
	    sprintf(PM_Memo+strlen(PM_Memo), "(%d)", i);
	}
    }
    */
}

/* get gigaword pa count for Chinese, for cell (i,j), i is the position of argument, j is the position of predicate */
/*==================================================================*/
	       void calc_gigaword_pa_matrix(SENTENCE_DATA *sp)
/*==================================================================*/
{
    int i, j, dis;
    
    for (i = 0; i < sp->Bnst_num; i++) {
	for (j = 0; j < sp->Bnst_num; j++) {
	    Chi_spec_pa_matrix[i][j] = 0;
	    Chi_pa_matrix[i][j] = 0;
	}
    }

    for (i = 0; i < sp->Bnst_num; i++) {
	for (j = i + 1; j < sp->Bnst_num; j++) {
	    if (check_feature(sp->bnst_data[i].f, "PU") || check_feature(sp->bnst_data[j].f, "PU")) {
		Chi_spec_pa_matrix[i][j] = 0;
		Chi_spec_pa_matrix[j][i] = 0;
		Chi_pa_matrix[i][j] = 0;
		Chi_pa_matrix[j][i] = 0;
	    }
	    else {
		if (j == i + 1) {
		    dis = 1;
		}
		else {
		    dis = 2;
		}

		Chi_pa_matrix[i][j] = get_chi_pa(sp->bnst_data+i, sp->bnst_data+j, dis);
		Chi_pa_matrix[j][i] = get_chi_pa(sp->bnst_data+j, sp->bnst_data+i, dis);

		if ((check_feature(sp->bnst_data[i].f, "NN") || 
		     check_feature(sp->bnst_data[i].f, "NR") || 
		     check_feature(sp->bnst_data[i].f, "NT") ||
		     check_feature(sp->bnst_data[i].f, "PN")) &&
		    (check_feature(sp->bnst_data[j].f, "VV") || 
		     check_feature(sp->bnst_data[j].f, "VC") || 
		     check_feature(sp->bnst_data[j].f, "VE") ||
		     check_feature(sp->bnst_data[j].f, "VA"))) {
		    Chi_spec_pa_matrix[i][j] = get_chi_spec_pa(sp->bnst_data+i, sp->bnst_data+j, dis);
		}
		else if ((check_feature(sp->bnst_data[j].f, "NN") || 
			  check_feature(sp->bnst_data[j].f, "NR") || 
			  check_feature(sp->bnst_data[j].f, "NT") ||
			  check_feature(sp->bnst_data[j].f, "PN")) &&
			 (check_feature(sp->bnst_data[i].f, "VV") || 
			  check_feature(sp->bnst_data[i].f, "VC") || 
			  check_feature(sp->bnst_data[i].f, "VE") ||
			  check_feature(sp->bnst_data[i].f, "VA"))) {
		    Chi_spec_pa_matrix[j][i] = get_chi_spec_pa(sp->bnst_data+i, sp->bnst_data+j, dis);
		}
		else {
		    Chi_spec_pa_matrix[i][j] = get_chi_spec_pa(sp->bnst_data+j, sp->bnst_data+i, dis);
		    Chi_spec_pa_matrix[j][i] = get_chi_spec_pa(sp->bnst_data+i, sp->bnst_data+j, dis);
		}
	    }
	}
    }
}

/*==================================================================*/
   char* get_chi_dpnd_stru_rule(char *verb, char *verb_pos, char *prep, char *noun, char *noun_pos, int disVP, int disNP, int commaVP, int commaNP)
/*==================================================================*/
{
    char *key;

    if (OptChiProb && CHIDpndStruExist == FALSE) {
	return NULL;
    }

    key = malloc_db_buf(strlen(verb) + strlen(prep) + strlen(noun) + 21);
    sprintf(key, "%s_%s_P_%s_%s_%s_%d_%d_%d_%d", verb_pos, verb, prep, noun_pos, noun, disVP, disNP, commaVP, commaNP);
    return db_get(chi_dpnd_stru_db, key);
}

/*==================================================================*/
   double calc_chi_dpnd_stru_prob(SENTENCE_DATA *sp, int verb, int prep, int noun)
/*==================================================================*/
{
    int commaVP = 0, commaNP = 0, disVP, disNP;
    int i, count;
    char *rule, *cat_rule, *occur, *total, *type;
    char *cur_rule[2];
    double lex_occur[2], lex_total[2], bk100_occur[2], bk100_total[2], bk010_occur[2], bk010_total[2], bk001_occur[2], bk001_total[2], bk110_occur[2], bk110_total[2], bk101_occur[2], bk101_total[2], bk011_occur[2], bk011_total[2], bk000_occur[2], bk000_total[2];
    double giga_weight = 0.8;
    double lamda, prob;
    double bk1_weight = 0.8;
    double bk2_weight = 0.6;
    double bk3_weight = 0.5;

    // initialization
    for (i = 0; i < 2; i++) {
	lex_occur[i] = 0.0;
	lex_total[i] = 0.0;
	bk100_occur[i] = 0.0;
	bk100_total[i] = 0.0;
	bk010_occur[i] = 0.0;
	bk010_total[i] = 0.0;
	bk001_occur[i] = 0.0;
	bk001_total[i] = 0.0;
	bk110_occur[i] = 0.0;
	bk110_total[i] = 0.0;
	bk101_occur[i] = 0.0;
	bk101_total[i] = 0.0;
	bk011_occur[i] = 0.0;
	bk011_total[i] = 0.0;
	bk000_occur[i] = 0.0;
	bk000_total[i] = 0.0;
    }

    // get distance and comma
    if (verb < prep) {
	disVP = -1;
	for (i = verb + 1; i < prep; i++) {
	    if (check_feature((sp->bnst_data+i)->f, "PU") &&
		(!strcmp((sp->bnst_data+i)->head_ptr->Goi, ",") ||
		 !strcmp((sp->bnst_data+i)->head_ptr->Goi, "��") ||
		 !strcmp((sp->bnst_data+i)->head_ptr->Goi, ":") ||
		 !strcmp((sp->bnst_data+i)->head_ptr->Goi, "��") ||
		 !strcmp((sp->bnst_data+i)->head_ptr->Goi, "��"))) {
		commaVP = 1;
		break;
	    }
	}
    }
    else {
	disVP = 1;
	for (i = prep + 1; i < verb; i++) {
	    if (check_feature((sp->bnst_data+i)->f, "PU") &&
		(!strcmp((sp->bnst_data+i)->head_ptr->Goi, ",") ||
		 !strcmp((sp->bnst_data+i)->head_ptr->Goi, "��") ||
		 !strcmp((sp->bnst_data+i)->head_ptr->Goi, ":") ||
		 !strcmp((sp->bnst_data+i)->head_ptr->Goi, "��") ||
		 !strcmp((sp->bnst_data+i)->head_ptr->Goi, "��"))) {
		commaVP = 1;
		break;
	    }
	}
    }

    if (noun < prep) {
	disNP = -1;
	for (i = noun + 1; i < prep; i++) {
	    if (check_feature((sp->bnst_data+i)->f, "PU") &&
		(!strcmp((sp->bnst_data+i)->head_ptr->Goi, ",") ||
		 !strcmp((sp->bnst_data+i)->head_ptr->Goi, "��") ||
		 !strcmp((sp->bnst_data+i)->head_ptr->Goi, ":") ||
		 !strcmp((sp->bnst_data+i)->head_ptr->Goi, "��") ||
		 !strcmp((sp->bnst_data+i)->head_ptr->Goi, "��"))) {
		commaNP = 1;
		break;
	    }
	}
    }
    else {
	disNP = 1;
	for (i = prep + 1; i < noun; i++) {
	    if (check_feature((sp->bnst_data+i)->f, "PU") &&
		(!strcmp((sp->bnst_data+i)->head_ptr->Goi, ",") ||
		 !strcmp((sp->bnst_data+i)->head_ptr->Goi, "��") ||
		 !strcmp((sp->bnst_data+i)->head_ptr->Goi, ":") ||
		 !strcmp((sp->bnst_data+i)->head_ptr->Goi, "��") ||
		 !strcmp((sp->bnst_data+i)->head_ptr->Goi, "��"))) {
		commaNP = 1;
		break;
	    }
	}
    }
    
    // read lex rule
    rule = NULL;
    rule = get_chi_dpnd_stru_rule((sp->bnst_data+verb)->head_ptr->Goi, (sp->bnst_data+verb)->head_ptr->Pos, (sp->bnst_data+prep)->head_ptr->Goi, (sp->bnst_data+noun)->head_ptr->Goi, (sp->bnst_data+noun)->head_ptr->Pos, disVP, disNP, commaVP, commaNP);

    if (rule != NULL) {
	count = 0;
	cat_rule = NULL;
	cat_rule = strtok(rule, ":");
	while (cat_rule) {
	    cur_rule[count] = malloc(strlen(cat_rule) + 1);
	    strcpy(cur_rule[count], cat_rule);
	    count++;
	    cat_rule = NULL;
	    cat_rule = strtok(NULL, ":");
	}

	for (i = 0; i < count; i++) {
	    occur = NULL;
	    total = NULL;
	    type = NULL;
			    
	    occur = strtok(cur_rule[i], "_");
	    total = strtok(NULL, "_");
	    type = strtok(NULL, "_");

	    if (!strcmp(type, "TRAIN")) {
		lex_occur[0] = atof(occur);
		lex_total[0] = atof(total);
	    }
	    else if (!strcmp(type, "GIGA")) {
		lex_occur[1] = atof(occur);
		lex_total[1] = atof(total);
	    }
	    if (cur_rule[i]) {
		free(cur_rule[i]);
	    }
	}
    }

    // read backoff rule
    rule = NULL;
    rule = get_chi_dpnd_stru_rule((sp->bnst_data+verb)->head_ptr->Goi, (sp->bnst_data+verb)->head_ptr->Pos, "XX", "XX", (sp->bnst_data+noun)->head_ptr->Pos, disVP, disNP, commaVP, commaNP);

    if (rule != NULL) {
	count = 0;
	cat_rule = NULL;
	cat_rule = strtok(rule, ":");
	while (cat_rule) {
	    cur_rule[count] = malloc(strlen(cat_rule) + 1);
	    strcpy(cur_rule[count], cat_rule);
	    count++;
	    cat_rule = NULL;
	    cat_rule = strtok(NULL, ":");
	}

	for (i = 0; i < count; i++) {
	    occur = NULL;
	    total = NULL;
	    type = NULL;
			    
	    occur = strtok(cur_rule[i], "_");
	    total = strtok(NULL, "_");
	    type = strtok(NULL, "_");

	    if (!strcmp(type, "TRAIN")) {
		bk100_occur[0] = atof(occur);
		bk100_total[0] = atof(total);
	    }
	    else if (!strcmp(type, "GIGA")) {
		bk100_occur[1] = atof(occur);
		bk100_total[1] = atof(total);
	    }
	    if (cur_rule[i]) {
		free(cur_rule[i]);
	    }
	}
    }

    rule = NULL;
    rule = get_chi_dpnd_stru_rule("XX", (sp->bnst_data+verb)->head_ptr->Pos, (sp->bnst_data+prep)->head_ptr->Goi, "XX", (sp->bnst_data+noun)->head_ptr->Pos, disVP, disNP, commaVP, commaNP);

    if (rule != NULL) {
	count = 0;
	cat_rule = NULL;
	cat_rule = strtok(rule, ":");
	while (cat_rule) {
	    cur_rule[count] = malloc(strlen(cat_rule) + 1);
	    strcpy(cur_rule[count], cat_rule);
	    count++;
	    cat_rule = NULL;
	    cat_rule = strtok(NULL, ":");
	}

	for (i = 0; i < count; i++) {
	    occur = NULL;
	    total = NULL;
	    type = NULL;
			    
	    occur = strtok(cur_rule[i], "_");
	    total = strtok(NULL, "_");
	    type = strtok(NULL, "_");

	    if (!strcmp(type, "TRAIN")) {
		bk010_occur[0] = atof(occur);
		bk010_total[0] = atof(total);
	    }
	    else if (!strcmp(type, "GIGA")) {
		bk010_occur[1] = atof(occur);
		bk010_total[1] = atof(total);
	    }
	    if (cur_rule[i]) {
		free(cur_rule[i]);
	    }
	}
    }

    rule = NULL;
    rule = get_chi_dpnd_stru_rule("XX", (sp->bnst_data+verb)->head_ptr->Pos, "XX", (sp->bnst_data+noun)->head_ptr->Goi, (sp->bnst_data+noun)->head_ptr->Pos, disVP, disNP, commaVP, commaNP);

    if (rule != NULL) {
	count = 0;
	cat_rule = NULL;
	cat_rule = strtok(rule, ":");
	while (cat_rule) {
	    cur_rule[count] = malloc(strlen(cat_rule) + 1);
	    strcpy(cur_rule[count], cat_rule);
	    count++;
	    cat_rule = NULL;
	    cat_rule = strtok(NULL, ":");
	}

	for (i = 0; i < count; i++) {
	    occur = NULL;
	    total = NULL;
	    type = NULL;
			    
	    occur = strtok(cur_rule[i], "_");
	    total = strtok(NULL, "_");
	    type = strtok(NULL, "_");

	    if (!strcmp(type, "TRAIN")) {
		bk001_occur[0] = atof(occur);
		bk001_total[0] = atof(total);
	    }
	    else if (!strcmp(type, "GIGA")) {
		bk001_occur[1] = atof(occur);
		bk001_total[1] = atof(total);
	    }
	    if (cur_rule[i]) {
		free(cur_rule[i]);
	    }
	}
    }

    rule = NULL;
    rule = get_chi_dpnd_stru_rule((sp->bnst_data+verb)->head_ptr->Goi, (sp->bnst_data+verb)->head_ptr->Pos, (sp->bnst_data+prep)->head_ptr->Goi, "XX", (sp->bnst_data+noun)->head_ptr->Pos, disVP, disNP, commaVP, commaNP);

    if (rule != NULL) {
	count = 0;
	cat_rule = NULL;
	cat_rule = strtok(rule, ":");
	while (cat_rule) {
	    cur_rule[count] = malloc(strlen(cat_rule) + 1);
	    strcpy(cur_rule[count], cat_rule);
	    count++;
	    cat_rule = NULL;
	    cat_rule = strtok(NULL, ":");
	}

	for (i = 0; i < count; i++) {
	    occur = NULL;
	    total = NULL;
	    type = NULL;
			    
	    occur = strtok(cur_rule[i], "_");
	    total = strtok(NULL, "_");
	    type = strtok(NULL, "_");

	    if (!strcmp(type, "TRAIN")) {
		bk110_occur[0] = atof(occur);
		bk110_total[0] = atof(total);
	    }
	    else if (!strcmp(type, "GIGA")) {
		bk110_occur[1] = atof(occur);
		bk110_total[1] = atof(total);
	    }
	    if (cur_rule[i]) {
		free(cur_rule[i]);
	    }
	}
    }

    rule = NULL;
    rule = get_chi_dpnd_stru_rule((sp->bnst_data+verb)->head_ptr->Goi, (sp->bnst_data+verb)->head_ptr->Pos, "XX", (sp->bnst_data+noun)->head_ptr->Goi, (sp->bnst_data+noun)->head_ptr->Pos, disVP, disNP, commaVP, commaNP);

    if (rule != NULL) {
	count = 0;
	cat_rule = NULL;
	cat_rule = strtok(rule, ":");
	while (cat_rule) {
	    cur_rule[count] = malloc(strlen(cat_rule) + 1);
	    strcpy(cur_rule[count], cat_rule);
	    count++;
	    cat_rule = NULL;
	    cat_rule = strtok(NULL, ":");
	}

	for (i = 0; i < count; i++) {
	    occur = NULL;
	    total = NULL;
	    type = NULL;
			    
	    occur = strtok(cur_rule[i], "_");
	    total = strtok(NULL, "_");
	    type = strtok(NULL, "_");

	    if (!strcmp(type, "TRAIN")) {
		bk101_occur[0] = atof(occur);
		bk101_total[0] = atof(total);
	    }
	    else if (!strcmp(type, "GIGA")) {
		bk101_occur[1] = atof(occur);
		bk101_total[1] = atof(total);
	    }
	    if (cur_rule[i]) {
		free(cur_rule[i]);
	    }
	}
    }

    rule = NULL;
    rule = get_chi_dpnd_stru_rule("XX", (sp->bnst_data+verb)->head_ptr->Pos, (sp->bnst_data+prep)->head_ptr->Goi, (sp->bnst_data+noun)->head_ptr->Goi, (sp->bnst_data+noun)->head_ptr->Pos, disVP, disNP, commaVP, commaNP);

    if (rule != NULL) {
	count = 0;
	cat_rule = NULL;
	cat_rule = strtok(rule, ":");
	while (cat_rule) {
	    cur_rule[count] = malloc(strlen(cat_rule) + 1);
	    strcpy(cur_rule[count], cat_rule);
	    count++;
	    cat_rule = NULL;
	    cat_rule = strtok(NULL, ":");
	}

	for (i = 0; i < count; i++) {
	    occur = NULL;
	    total = NULL;
	    type = NULL;
			    
	    occur = strtok(cur_rule[i], "_");
	    total = strtok(NULL, "_");
	    type = strtok(NULL, "_");

	    if (!strcmp(type, "TRAIN")) {
		bk011_occur[0] = atof(occur);
		bk011_total[0] = atof(total);
	    }
	    else if (!strcmp(type, "GIGA")) {
		bk011_occur[1] = atof(occur);
		bk011_total[1] = atof(total);
	    }
	    if (cur_rule[i]) {
		free(cur_rule[i]);
	    }
	}
    }

    rule = NULL;
    rule = get_chi_dpnd_stru_rule("XX", (sp->bnst_data+verb)->head_ptr->Pos, "XX", "XX", (sp->bnst_data+noun)->head_ptr->Pos, disVP, disNP, commaVP, commaNP);

    if (rule != NULL) {
	count = 0;
	cat_rule = NULL;
	cat_rule = strtok(rule, ":");
	while (cat_rule) {
	    cur_rule[count] = malloc(strlen(cat_rule) + 1);
	    strcpy(cur_rule[count], cat_rule);
	    count++;
	    cat_rule = NULL;
	    cat_rule = strtok(NULL, ":");
	}

	for (i = 0; i < count; i++) {
	    occur = NULL;
	    total = NULL;
	    type = NULL;
			    
	    occur = strtok(cur_rule[i], "_");
	    total = strtok(NULL, "_");
	    type = strtok(NULL, "_");

	    if (!strcmp(type, "TRAIN")) {
		bk000_occur[0] = atof(occur);
		bk000_total[0] = atof(total);
	    }
	    else if (!strcmp(type, "GIGA")) {
		bk000_occur[1] = atof(occur);
		bk000_total[1] = atof(total);
	    }
	    if (cur_rule[i]) {
		free(cur_rule[i]);
	    }
	}
    }

    // calculate probability
    lex_occur[0] += giga_weight * lex_occur[1];
    lex_total[0] += giga_weight * lex_total[1];
    bk100_occur[0] += giga_weight * bk100_occur[1];
    bk100_total[0] += giga_weight * bk100_total[1];
    bk010_occur[0] += giga_weight * bk010_occur[1];
    bk010_total[0] += giga_weight * bk010_total[1];
    bk001_occur[0] += giga_weight * bk001_occur[1];
    bk001_total[0] += giga_weight * bk001_total[1];
    bk110_occur[0] += giga_weight * bk110_occur[1];
    bk110_total[0] += giga_weight * bk110_total[1];
    bk101_occur[0] += giga_weight * bk101_occur[1];
    bk101_total[0] += giga_weight * bk101_total[1];
    bk011_occur[0] += giga_weight * bk011_occur[1];
    bk011_total[0] += giga_weight * bk011_total[1];
    bk000_occur[0] += giga_weight * bk000_occur[1];
    bk000_total[0] += giga_weight * bk000_total[1];

    prob = 0.0;
    lamda = 0.0;
    
    if (lex_total[0] > 0) {
	lamda = lex_total[0] / (lex_total[0] + 1);
	prob = lamda * (lex_occur[0] / lex_total[0]);
	if (bk110_total[0] > 0 || bk011_total[0] > 0 || bk101_total[0] > 0) {
	    prob += (1 - lamda) * (bk101_occur[0] + bk110_occur[0] + bk011_occur[0]) / (bk101_total[0] + bk110_total[0] +  bk011_total[0]);
	}
	if (bk100_total[0] > 0 || bk010_total[0] > 0 || bk001_total[0] > 0) {
	    prob += (1 - lamda) * (bk100_occur[0] + bk010_occur[0] + bk001_occur[0]) / (bk100_total[0] + bk010_total[0] +  bk001_total[0]);
	}
	if (bk000_total[0] > 0) {
	    prob += (1 - lamda) * (bk000_occur[0] / bk000_total[0]);
	}
    }
    else if (bk110_total[0] > 0 || bk011_total[0] > 0 || bk101_total[0] > 0) {
	lamda = (bk110_total[0] + bk011_total[0] + bk101_total[0]) / (bk110_total[0] + bk011_total[0] + bk101_total[0] + 1);
	prob = lamda * (bk101_occur[0] + bk110_occur[0] + bk011_occur[0]) / (bk101_total[0] + bk110_total[0] +  bk011_total[0]);
	if (bk100_total[0] > 0 || bk010_total[0] > 0 || bk001_total[0] > 0) {
	    prob += (1 - lamda) * (bk100_occur[0] + bk010_occur[0] + bk001_occur[0]) / (bk100_total[0] + bk010_total[0] +  bk001_total[0]);
	}
	if (bk000_total[0] > 0) {
	    prob += (1 - lamda) * (bk000_occur[0] / bk000_total[0]);
	}
	prob *= bk1_weight;
    }
    else if (bk100_total[0] > 0 || bk010_total[0] > 0 || bk001_total[0] > 0) {
	lamda = (bk100_total[0] + bk010_total[0] + bk001_total[0]) / (bk100_total[0] + bk010_total[0] + bk001_total[0] + 1);
	prob = lamda * (bk100_occur[0] + bk010_occur[0] + bk001_occur[0]) / (bk100_total[0] + bk010_total[0] +  bk001_total[0]);
	if (bk000_total[0] > 0) {
	    prob += (1 - lamda) * (bk000_occur[0] / bk000_total[0]);
	}
	prob *= bk2_weight;
    }
    else if (bk000_total[0] > 0) {
	prob = bk3_weight * (bk000_occur[0] / bk000_total[0]);
    }

    if (rule) {
	free(rule);
    }

    return prob;
}

/*====================================================================
                               END
====================================================================*/
