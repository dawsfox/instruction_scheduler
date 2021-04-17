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
	int src;
	int dest;
} edge_t;

typedef struct ddg_s {
	int start;
	int end;
	int num_edges;
	ddg_node_t node_list[32];
	edge_t edge_list[64];
} ddg_t; 

void print_ddg(ddg_t curr) {
	printf("ddg from %d to %d\n", curr.start, curr.end);
	printf("has nodes:\n");
	for (int i=0; i<curr.end-curr.start+1; i++) {
		printf("%d, ", curr.node_list[i].statement);
	}
	printf("\nwith edges:\n");
	for (int i=0; i < curr.num_edges; i++) {
		printf("%d->%d, ", curr.edge_list[i].src, curr.edge_list[i].dest);
	}
}

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

int dependency_check(char *var, char **statement, int write) {
	// write is 1 when the given var is a destination, 0 otherwise
	// need NULL protection, otherwise probably seg fault
	if (var != NULL) {
		int is_var = statement[0] != NULL;
		if (is_var && (strcmp(var, statement[0]) == 0)) {
			return 1;
		}
		is_var = statement[1] != NULL;
		if (write && is_var && (strcmp(var, statement[1]) == 0)) {
			return 1;
		}
		is_var = statement[2] != NULL;
		if (write && is_var && (strcmp(var, statement[2]) == 0)) {
			return 1;
		}
	}
	return 0;
}

/*
int node_weight(ddg_node_t *node) {
	return node->weight;
}
*/

int get_latency(char *line) {
	unsigned length = strlen(line);
	for (int i=0; i<length; i++) {
		if (line[i] == '/') {
			return 4;
		}
		else if (line[i] == '*') {
			if (line[i+1] == '*') {
				return 8;
			}
			else {
				return 4;
			}
		}
		else if (line[i] == '+' || line[i] == '-') {
			return 1;
		}
		else if (line[i] == '\n') {
			return 2;
		}
	}
	return 0;
}

int main(int argc, char *argv[]) {

	FILE *input = fopen(argv[1], "r");
	int latencies[32];
	char str[80];
	char dest[16];
	char src1[16];
	char src2[16];
	int i=0;
	char **statement_list[32];
	char *whole_statement[32];
	/* while loop parses vars of each line and puts them in appropiate spot in statement_list */
	while (fgets(str, 80, input) != NULL) {
		whole_statement[i] = (char *) malloc(strlen(str) + 1);
		strcpy(whole_statement[i], str);
		latencies[i] = get_latency(str);
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
			statement_list[i][0] = (char *) malloc(sizeof(char) * strlen(dest) + 1); // allocate space for dest including null char
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
	
	/* building ddgs */
	ddg_t blocks[8];
	unsigned block_index = 0;;
	blocks[0].start = 0;
	int if_begin = 0;
	int else_begin = 0;
	int else_end = 0;
	unsigned edge_index = 0;
	unsigned node_index = 0;
	// find endpoint of first basic block
	for (int k=0; k<i; k++) {
		char *dest_curr = statement_list[k][0];
		char *src1_curr = statement_list[k][1];
		char *src2_curr = statement_list[k][2];
		if (dest_curr == NULL && src1_curr != NULL && src2_curr == NULL) {
			blocks[0].end = k-1;
		}
	}
	// loop through to build edges and nodes
	for (int j=0; j<i; j++) {
		//ddg_node_t *curr = malloc(sizeof(ddg_node_t));
		blocks[block_index].node_list[node_index].statement = j;
		blocks[block_index].node_list[node_index].weight = 1;
		char *dest_curr = statement_list[j][0];
		char *src1_curr = statement_list[j][1];
		char *src2_curr = statement_list[j][2];
		// if current block is done
		if (j == blocks[block_index].end+1) {
			blocks[block_index].num_edges = edge_index;
			print_ddg(blocks[block_index]);
			block_index++;
			// find starting point of next block
			for (int k=j+1; k<i; k++) {
				if (statement_list[k][1] == NULL && statement_list[k][2] == NULL) { //either else begin or else end
					if (strcmp(statement_list[k][0], "end") == 0) {
						else_end = k;
						break;
					}
				}
			}
			blocks[block_index].start = else_end+1;
			printf("next block starts at %d\n", blocks[block_index].start);
			// jump j to new starting point
			j = else_end+1;
			// find new blocks end point
			blocks[block_index]. end = i-1; //end of program if no more control flow changes
			for (int k=j; k<i; k++) {
				if (statement_list[k][0] == NULL && statement_list[k][1] != NULL && statement_list[k][2] == NULL) { //next if start
					blocks[block_index].end = k - 1;
				}
			}
			dest_curr = statement_list[j][0];
			src1_curr = statement_list[j][1];
			src2_curr = statement_list[j][2];
			node_index = 0;
			edge_index = 0;
			blocks[block_index].node_list[node_index].statement = j;
			blocks[block_index].node_list[node_index].weight = 1;
		}
		for (int k=j+1; k<=blocks[block_index].end; k++) {
			int deps = dependency_check(dest_curr, statement_list[k], 1);
			deps += dependency_check(src1_curr, statement_list[k], 0);
			deps += dependency_check(src2_curr, statement_list[k], 0);
			if (deps > 0) {
				blocks[block_index].edge_list[edge_index].src = j; 
				blocks[block_index].edge_list[edge_index].dest = k;
				blocks[block_index].edge_list[edge_index].weight = latencies[j];
				printf("edge between statement %d and statement %d with weight %d\n", j, k, latencies[j]);
				edge_index++;
			}
		}
		node_index++;
	} //outermost for
	blocks[block_index].num_edges = edge_index;
	print_ddg(blocks[block_index]);
	
	// employ algorithms on each block
	for (int j=0; j<block_index+1; j++) {
		ddg_t curr = blocks[j];
		int est[64]; //earliest starting time
		//for each node
		for (int k=0; k<curr.end-curr.start+1; k++) {
			ddg_node_t curr_node = curr.node_list[k];
			// for each edge
			for (int l=0; l<curr.num_edges; l++) {
				edge_t curr_edge = curr.edge_list[l];
				
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
