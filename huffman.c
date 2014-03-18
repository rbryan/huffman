#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

struct table {
    int size;
    char *key;
    char *val;
    int *counts;
};
typedef struct table table_t;

struct data {
    long int size;
    char *data;
};
typedef struct data data_t;

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
void sort_table(table_t * t);
int match_first_n_bits(int n, char a, char b);
int match_last_n_bits(int n, char a, char b);

int main(int argc, char **argv)
{
    if (argc < 5) {
	fprintf(stderr,
		"Usage: huffman {c <depth>|x} <infile> <outfile>\n");
	exit(1);
    }

    data_t *d;
    table_t *t;

    t = new_table();
    d = load_file(argv[1]);
    print_data(d);
    count_bytes(d, t);
    clean_table(t);
    resize_table(t);
    sort_table(t);
    print_table(t);
    output_data("output", d);
    exit(0);
}

int match_last_n_bits(int n, char a, char b){
	if((a<<(8-n) ^ b <<(8-n)) == 0) return 1;
	return 0;
}

int match_first_n_bits(int n, char a, char b){
	if((a>>(8-n) ^ b>>(8-n)) == 0) return 1;
	return 0;
}

void set_keys(table_t *t){
		
}

void sort_table(table_t * t)
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
	    printf("%d\t%u\t%u\t%c\n", t->counts[i], (uint8_t) t->key[i], (uint8_t) t->val[i],
		   (char) t->val[i]);
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
    new->key = calloc(256,sizeof(char));
    new->val = calloc(256,sizeof(char));
    new->counts = calloc(256, sizeof(int));
    value_table(new);
    return new;
}
