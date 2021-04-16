#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct ddg_node_s {
	int statement;
	int weight;
} ddg_node_t;

typedef struct edge_s {
	int weight;
	ddg_node_t *src;
	ddg_node_t *dest;
} edge_t;

typedef struct ddg_s {
	ddg_node_t *start;
	ddg_node_t *end;
	ddg_node_t *node_list;
	edge_t edge_list[64];
} ddg_t; 

int retrieve_dest(char *three_address, char *dest) {
	unsigned length = strlen(three_address);
	unsigned tab = 0; int if_flag = 0;
	unsigned left_bound;
	if (three_address[0] == '\t') {
		tab = 1;
	}
	else if (three_address[0] == 'I' && three_address[1] == 'f') {
		if_flag = 1;
	}
	for (int i=0; i<length; i++) {
		if (three_address[i] == ' ') {
			strncpy(dest, three_address+tab, i+1);
			dest[i-tab] = '\0';
			return 0;
		}
		else if (if_flag && three_address[i] == '(') {
			left_bound = i+1;
		}
		else if (if_flag && three_address[i] == ')') {
			strncpy(dest, three_address+left_bound, i-left_bound);
			dest[i-left_bound] = '\0';
			return 2;
		}
		else if (three_address[i] == '}') { //line is involved in if statement
			if (three_address[i+1] == ' ' && three_address[i+2] == 'e') { //else beginning
				sprintf(dest, "%s", "else");
			}
			else { //end of if statement
				sprintf(dest, "%s", "end");
			}
			return 1;
		}
	}
	return -1;
}

int retrieve_first_src(char *three_address, char *src1) {
	unsigned length = strlen(three_address);
	int left_bound = 0;
	int single_flag = 0;
	for (int i=0; i<length; i++) {
		char curr = three_address[i];
		if (curr == '=') {
			left_bound = i+2;
		}
		else if ((curr == '/') || (curr == '+') || (curr == '-')) { // all typical cases
			strncpy(src1, three_address+left_bound, i-left_bound);
			src1[i-left_bound] = '\0';
			return i+1;
		}
		else if (curr == '*') {
			strncpy(src1, three_address+left_bound, i-left_bound);
			src1[i-left_bound] = '\0';
			if (three_address[i+1] == '*') { //if pow
				return(i+2);
			}
			else { //just multiplication
				return i+1;
			}
		}
		else if (curr == '!') {
			left_bound = i+1;
		}
		else if (curr == '\n') {
			strncpy(src1, three_address+left_bound, i - left_bound);
			src1[i-left_bound] = '\0';
			return -1; //only one source for the statement
			
		} 
	}
	return -2; //error or unexpected input
}

void retrieve_second_src(char *three_address, char *src2, int left_bound) {
	unsigned length = strlen(three_address);
	for (int i=left_bound; i<length; i++) {
		if (three_address[i] == '\n') {
			three_address[i] = '\0';
			break;
		}
	}
	sprintf(src2, "%s", three_address+left_bound);
}

int is_number(char *src) {
	int i=0;
	char curr = src[0];
	while(isdigit(curr)) {
		i++;
		curr = src[i];
		if (curr == '\0') {
			return 1;
		}
	}
	return 0;
}

int dependency_check(char *first, char *second) {

/*
int node_weight(ddg_node_t *node) {
	return node->weight;
}
*/

int main(int argc, char *argv[]) {

	FILE *input = fopen(argv[1], "r");
	char str[80];
	char dest[16];
	char src1[16];
	char src2[16];
	int i=0;
	char **statement_list[32];
	/* while loop parses vars of each line and puts them in appropiate spot in statement_list */
	while (fgets(str, 80, input) != NULL) {
		statement_list[i] = (char **) malloc(sizeof(char *) * 3); //allocate space for dest, src1, src2 for this statement
		int result = retrieve_dest(str, dest);
		if(result == 0) {
			statement_list[i][0] = (char *) malloc(sizeof(char) * strlen(dest) + 1); // allocate space for dest including null char
			strcpy(statement_list[i][0], dest);
			printf("statement %d has dest: %s, ", i, statement_list[i][0]);
			result = retrieve_first_src(str, src1);
			if (result == -1) {
				if (!is_number(src1)) {
					statement_list[i][1] = (char *) malloc(sizeof(char) * strlen(src1) + 1);
					strcpy(statement_list[i][1], src1);
					printf("src1: %s, no second source\n", statement_list[i][1]);
				}
				else {
					statement_list[i][1] = NULL;
				}
				statement_list[i][2] = NULL;
			}
			else {
				if(!is_number(src1)) {
					statement_list[i][1] = (char *) malloc(sizeof(char) * strlen(src1) + 1);
					strcpy(statement_list[i][1], src1);
					printf("src1: %s, ", statement_list[i][1]);
				}
				else {
					statement_list[i][1] = NULL;
				}
				retrieve_second_src(str, src2, result);
				if(!is_number(src2)) {
					statement_list[i][2] = (char *) malloc(sizeof(char) * strlen(src2) + 1);
					strcpy(statement_list[i][2], src2);
					printf("src2: %s", statement_list[i][2]);
				}
				else {
					statement_list[i][2] = NULL;
				}
				printf(" (%d%d%d)\n", is_number(dest), is_number(src1), is_number(src2));
			}
		}
		else if (result == 1) {
			printf("statement %d is if-statement overhead\n", i);
			strcpy(statement_list[i][0], dest);
			statement_list[i][1] = NULL;
			statement_list[i][2] = NULL;
		}
		else if (result == 2) { //note in this case dest is actually src1
			statement_list[i][0] = NULL;
			statement_list[i][1] = (char *) malloc(sizeof(char) * strlen(dest) + 1); // allocate space for dest including null char
			statement_list[i][2] = NULL;
			strcpy(statement_list[i][1], dest);
			printf("statement %d is if-statement reading %s\n", i, statement_list[i][1]);
		}
		else {
			printf("error retrieving dest\n");
		}
		i++;
	}
	
	/* building ddg */
	ddg_t dd_graph;
	int if_begin = 0;
	int else_begin = 0;
	int else_end = 0;
	for (int j=0; j<i; j++) {
		ddg_node_t *curr = malloc(sizeof(ddg_node_t));
		curr->statement = j;
		curr->weight = 0;
		char *dest_curr = statement_list[j][0];
		char *src1_curr = statement_list[j][1];
		char *src2_curr = statement_list[j][2];
		if (dest_curr == NULL && src1_curr != NULL && src2_curr == NULL) {
			if_begin = j+1;
		}
		else if (src1_curr == NULL && src2_curr == NULL) { //either else begin or else end
			if (strcmp(dest_curr, "end") == 0) {
				else_end = j;
			}
			else if (strcmp(dest_curr, "else") == 0) {
				else_begin = j;
			}
		}
		else {
			for (int k=j; k<i; k++) {
				int dep;
			}
		}
	}
	
	/* freeing memory used by statement_list */
	for (int j=i-1; j>=0; j--) {
		for (int k=0; k<3; k++) {
			if (statement_list[j][k] != NULL) {
				free(statement_list[j][k]);
			}
		}
		free(statement_list[j]);
	}
	fclose(input);
	//FILE *output = fopen(argv[2], "w");
}