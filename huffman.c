#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define 	LEFT 	0
#define 	RIGHT 	0

//
//Structures
//

struct table {
    int size;
    char *key;
    char *val;
    int *counts;
    int *bits;
};
typedef struct table table_t;

struct data {
    long int size;
    char *data;
};
typedef struct data data_t;

struct node {
    int weight;
    int bits;
    char bstring;
    int index;			//index when sorted by freq
    struct node *l;
    struct node *r;
    struct node *n;
    struct node *p;
};
typedef struct node node_t;

//
//Function declarations to follow
//

data_t *new_data();
table_t *new_table();
data_t *load_file(const char *filename);
void print_data(data_t * data);
void output_data(const char *filename, data_t * d);
void count_bytes(data_t * d, table_t * t);
void print_table(table_t * t);
void value_table(table_t * t);
void swap_table_element(int a, int b, table_t * t);
void clean_table(table_t * t);
void resize_table(table_t * t);	//Only to be used on clean tables.
int match_first_n_bits(int n, char a, char b);
int match_last_n_bits(int n, char a, char b);
void sort_table_by_freq(table_t * t);
void sort_table_by_char(table_t * t);
node_t *build_huffman_queue(table_t * t);
node_t *new_node(int weight, int index);
void print_huffman_queue(node_t * q);
node_t *pop_node(node_t * n);
node_t *pop_lowest(node_t * q);
int get_queue_length(node_t * q);
void append_node(node_t * q, node_t * n);
void tree_transform(node_t * q);
node_t *build_huffman_tree(node_t * q,table_t *t);
void free_node_struct_struct(node_t * n);	//frees a structure made of nodes.
void populate_bstrings(node_t * t, table_t * l, int d, char str);

int main(int argc, char **argv)
{
    if (argc < 5) {
	fprintf(stderr,
		"Usage: huffman {c <depth>|x} <infile> <outfile>\n");
	exit(1);
    }

    data_t *d;
    table_t *t;
    node_t *q;

    t = new_table();
    d = load_file(argv[1]);
    //print_data(d);
    count_bytes(d, t);
    clean_table(t);
    resize_table(t);
    sort_table_by_freq(t);
    print_table(t);
    q = build_huffman_queue(t);
    print_huffman_queue(q);
	build_huffman_tree(q,t);
	print_table(t);
    output_data("output", d);
    exit(0);
}

//
//The code to follow has trees and I am thoroughly ashamed of it.
//
node_t *new_node(int weight, int index)
{
    node_t *new;
    new = malloc(sizeof(node_t));
    new->weight = weight;
    new->index = index;
    new->bits = 0;
    new->bstring = 0;
    new->l = NULL;
    new->r = NULL;
    new->n = NULL;
    new->p = NULL;
    return new;
}

void free_node_struct(node_t * n)
{
    free_node_struct(n->l);
    free_node_struct(n->r);
    free_node_struct(n->p);
    free_node_struct(n->n);
    free(n);
}

node_t *build_huffman_queue(table_t * t)
{
    node_t *branches;
    node_t *prev;
    int i;
    int size;
    int *c;

    c = t->counts;
    size = t->size;

    //
    //Build a linked list of nodes to work with.
    //

    //The first node doesn't matter.
    //This makes things easier in transformations.
    branches = new_node(-1, -1);
    prev = branches;

    for (i = 0; i < size; i++) {
	node_t *new;
	new = new_node(c[i], i);
	prev->n = new;
	new->p = prev;
	prev = new;
    }
    return branches;
}

void print_huffman_queue(node_t * q)
{
    q = q->n;
    if (q == NULL)
	return;
    while (q->n != NULL) {
	printf("%d\n", q->weight);
	q = q->n;
    }
    printf("%d\n", q->weight);

}

node_t *pop_node(node_t * n)
{
    if (n->n != NULL)
	n->n->p = n->p;
    if (n->p != NULL)
	n->p->n = n->n;
    return n;
}

int get_queue_length(node_t * q)
{
    int l = 0;
    while (q->n != NULL) {
	l++;
	q = q->n;
    }
    return l;
}

node_t *build_huffman_tree(node_t * q,table_t *t)
{

    while (get_queue_length(q) > 1) {
	tree_transform(q);
    }
    populate_bstrings(q->n,t, -1,0 );
    return q->n;
}

void tree_transform(node_t * q)
{
    node_t *a;
    node_t *b;
    node_t *new;

    a = pop_lowest(q);
    b = pop_lowest(q);

    new = malloc(sizeof(node_t));

    new->weight = a->weight + b->weight;
    new->l = a;
    new->r = b;

    append_node(q, new);
}

void append_node(node_t * q, node_t * n)
{
    while (q->n != NULL) {
	q = q->n;
    }
    n->p = q;
    q->n = n;
    return;
}


node_t *pop_lowest(node_t * q)
{
    node_t *l;

    q = q->n;

    l = q;

    if (q == NULL)
	return NULL;

    while (1) {
	if (q->weight < l->weight)
	    l = q;
	if (q->n == NULL)
	    break;
	q = q->n;
    }

    return pop_node(l);
}


void populate_bstrings(node_t * t, table_t * l, int d, char str)
{
    if (t == NULL)
	return;
    int i;

    i = t->index;
    if(i != -1)
    	l->key[i] = str;

    if (t->r != NULL)
	populate_bstrings(t->r, l, d + 1, str ^ 1 << (8 - d));
    if (t->l != NULL)
	populate_bstrings(t->l, l, d + 1, str);
}

//
//The code to follow is tree free.
//

int match_last_n_bits(int n, char a, char b)
{
    if ((a << (8 - n) ^ b << (8 - n)) == 0)
	return 1;
    return 0;
}

int match_first_n_bits(int n, char a, char b)
{
    if ((a >> (8 - n) ^ b >> (8 - n)) == 0)
	return 1;
    return 0;
}

//
//Sort table by frequency for building a huffman tree
//

void sort_table_by_freq(table_t * t)
{
    int i;			//index
    int b;			//base
    int best;
    int size;
    int *c;			//counts

    c = t->counts;
    size = t->size;

    for (i = 0; i < size; i++) {

	b = i;
	best = i;

	for (; i < size; i++) {
	    if (c[i] > c[best]) {
		best = i;
	    }
	}
	swap_table_element(best, b, t);
	i = b;
    }
}

//
//Sort the table by character to use it as a hash table.
//

void sort_table_by_char(table_t * t)
{
    int i;			//index
    int b;			//base
    int best;
    char *c;			//char

    c = t->val;

    for (i = 0; i < 256; i++) {

	b = i;
	best = i;

	for (; i < 256; i++) {
	    if (c[i] < c[best]) {
		best = i;
	    }
	}
	swap_table_element(best, b, t);
	i = b;
    }
}

void count_bytes(data_t * d, table_t * t)
{
    char c;
    long int i;
    char *data;
    int *counts;

    counts = t->counts;
    data = d->data;

    for (i = 0; i < d->size; i++) {
	c = data[i];
	counts[(int) c]++;
    }
}

void print_table(table_t * t)
{
    int i;
    for (i = 0; i < 256; i++) {
	if (t->counts[i] != 0) {
	    printf("%d\t%u\t%u\t%c\n", t->counts[i], (uint8_t) t->key[i],
		   (uint8_t) t->val[i], (char) t->val[i]);
	}
    }

}

void print_data(data_t * data)
{
    fwrite(data->data, 1, data->size, stdout);
    return;
}

void output_data(const char *filename, data_t * d)
{
    FILE *fp;
    fp = fopen(filename, "w");
    fwrite(d->data, 1, d->size, fp);
    fclose(fp);
    return;
}

data_t *load_file(const char *filename)
{
    data_t *new;
    FILE *fp;
    long int fsize;

    fp = fopen(filename, "r");
    fseek(fp, 0L, SEEK_END);
    fsize = ftell(fp);
    rewind(fp);
    printf("Filesize: %ld\n", fsize);

    new = new_data();
    new->size = fsize;
    new->data = malloc(fsize);
    fread(new->data, 1, fsize, fp);
    fclose(fp);
    return new;

}

data_t *new_data()
{
    data_t *new;
    new = malloc(sizeof(data_t));
    new->size = 0;
    new->data = NULL;
    return new;
}

void clean_table(table_t * t)
{
    int i;
    int last_zero;
    int *c;

    c = t->counts;

    for (i = 0; i < 256; i++) {
	if (c[i] == 0) {
	    last_zero = i;
	    for (; c[i] == 0 && i < 256; i++)
		continue;
	    swap_table_element(last_zero, i, t);
	    i = last_zero;
	}
    }
}

void swap_table_element(int a, int b, table_t * t)
{
    char ht;
    char vt;
    int ct;

    ht = t->key[a];
    vt = t->val[a];
    ct = t->counts[a];

    t->key[a] = t->key[b];
    t->val[a] = t->val[b];
    t->counts[a] = t->counts[b];

    t->key[b] = ht;
    t->val[b] = vt;
    t->counts[b] = ct;
}

void value_table(table_t * t)
{
    int i;
    for (i = 0; i < 256; i++) {
	t->val[i] = i;
    }

}

void resize_table(table_t * t)
{
    int i;
    int *c;

    c = t->counts;

    for (i = 0; i < 256; i++) {
	if (c[i] == 0) {
	    break;
	}
    }

    t->size = i;
}

table_t *new_table()
{
    table_t *new;
    new = malloc(sizeof(table_t));
    new->size = 0;
    new->key = calloc(256, sizeof(char));
    new->val = calloc(256, sizeof(char));
    new->counts = calloc(256, sizeof(int));
    new->bits = calloc(256, sizeof(int));
    value_table(new);
    return new;
}
