#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>


//LMAX and LMIN are inclusive range boundaries
#define LMAX	4
#define LMIN	3

#define MIN(a,b)	((a) < (b)) ? (a) : (b)

//
//Structures
//

struct data {
    long int size;
    char *data;
};
typedef struct data data_t;

struct node {
    struct node *n;
    unsigned int frequency;
    long int pos;
    int len;
    long int saves;
    char *str;
};
typedef struct node node_t;

//
//Function declarations to follow
//

data_t *new_data();
uint16_t get_symbol(data_t * d);
node_t *new_node(long int pos, long int len);
int is_in_file(uint16_t q, data_t * data);
data_t *load_file(const char *filename);
void print_data(data_t * data);
void output_data(const char *filename, data_t * d);
node_t *get_word_list(data_t * d);
node_t *get_best(node_t * l, data_t * d);
node_t *found(node_t * h, node_t * n, data_t * d);
void append_node(node_t * h, node_t * n);
void print_word_list(node_t * h, data_t * d);
void set_savings(node_t * h, data_t * d);

int main(int argc, char **argv)
{
    if (argc < 2) {
	fprintf(stderr, "Usage: run {x|c} <input_file>\n");
	exit(1);
    }

    data_t *data;
//    data_t *ndata;
    node_t *l;

    data = load_file(argv[1]);
//    print_data(data);
//    printf("%d\n", get_symbol(data));
//    printf("%d\n", UINT16_MAX);
    l = get_word_list(data);
    set_savings(l, data);
//    print_word_list(l,data);
    get_best(l, data);
//    output_data("output", data);
    exit(0);
}

data_t *build_output(data_t * i, node_t * b)
{

}

void set_savings(node_t * h, data_t * d)
{
    while (h != NULL) {

	h->saves =
	    (((long int) h->len) - 2) * ((long int) h->frequency) -
	    ((long int) h->len) - 3;
	/*
	   fwrite(d->data+h->pos,h->len,1,stdout);
	   putchar('\n');
	   printf("Len:\t%d\n",h->len);
	   printf("Saves:\t%ld\tFrequency:\t%u\n",h->saves,h->frequency);
	 */
	h = h->n;
    }
}

node_t *get_best(node_t * l, data_t * d)
{
    node_t *best;

    best = l;

    if (l == NULL) {
	fprintf(stderr, "No most frequent\n");
	return NULL;
    }

    while (l != NULL) {
	if (l->saves > best->saves) {
	    best = l;
	}
	l = l->n;
    }
    best->str = malloc(best->len);
    printf("%s\n", d->data + best->pos);
    memcpy(best->str, d->data + best->pos, best->len);
    fwrite(best->str, best->len, 1, stdout);
    printf("%d\n", best->len);
    putchar('\n');
    return best;
}

void append_node(node_t * h, node_t * n)
{
    while (h->n != NULL)
	h = h->n;
    h->n = n;
}

void print_word_list(node_t * h, data_t * d)
{
    printf("\n\n\nOutputting word list:\n\n");
    while (h != NULL) {
	fwrite(d->data + h->pos, h->len, 1, stdout);
	putchar('\n');
	h = h->n;
    }
    printf("\n\n\n");
}

node_t *get_word_list(data_t * d)
{
    long int dlen;
    long int i;
    long int j;
    node_t *list = NULL;

    dlen = d->size;

    for (i = LMIN; i <= LMAX; i++) {
	printf("Processing Len:\t%ld\n", i);
	for (j = 0; j < dlen - i; j++) {
	    node_t *f;
	    node_t *new;
	    new = new_node(j, i);

	    //Display Progress
	    if (j % 1000 == 0) {
		printf("Percent done:\t%f\n", j * 1.0 / (dlen - i));
	    }
	    //

	    /*
	       fwrite(d->data+j,i,1,stdout);
	       putchar('\n');
	     */

	    if (list == NULL) {
		list = new;
	    } else {
		if ((f = found(list, new, d))) {
		    f->frequency += 1;
		    free(new);
		} else {
		    //fprintf(stderr, "Appending word.\n");
		    append_node(list, new);
		    new->frequency += 1;
		}

	    }



	}
    }

    return list;

}

node_t *found(node_t * h, node_t * n, data_t * d)
{
    if (h == NULL) {
	return NULL;
    }
    while (h != NULL) {

	if (h->len != n->len) {
	    h = h->n;
	    continue;
	}

	if (memcmp
	    ((void *) (d->data + h->pos), (void *) (d->data + n->pos),
	     h->len) == 0) {
	    return h;
	}
	h = h->n;
    }
    return NULL;

}

uint16_t get_symbol(data_t * d)
{
    uint16_t a = (uint16_t) 'a' << 8 | (uint16_t) 'a';
    int none = 1;
    while (a < UINT16_MAX) {
	a++;
	if (is_in_file(a, d) == 0) {
	    none = 0;
	    break;
	}
    }
    if (none == 1)
	return 0;
    return a;
}



int is_in_file(uint16_t q, data_t * data)
{
    int i;
    int size = data->size - 1;
    for (i = 0; i < size; i++) {
	if (*((uint16_t *) (data->data + i)) == q) {
	    //printf("%d\t\t%s\n",i,data->data+i);
	    return 1;
	}
    }
    return 0;
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

node_t *new_node(long int pos, long int len)
{
    node_t *new;
    new = malloc(sizeof(node_t));
    new->frequency = 0;
    new->pos = pos;
    new->len = len;
    new->saves = 0;
    new->n = NULL;
    new->str = NULL;
    return new;
}
