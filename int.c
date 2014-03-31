#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>


//LMAX and LMIN are inclusive range boundaries
#define LMAX	3
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
data_t *build_output(data_t * id, node_t * b);
data_t *decompress(data_t * id);

int main(int argc, char **argv)
{
    if (argc < 2) {
	fprintf(stderr, "Usage: run {x|c} <input_file>\n");
	exit(1);
    }
    if (strcmp(argv[1], "c") == 0) {
	data_t *data;
//    data_t *ndata;
	node_t *l;

	data = load_file(argv[2]);
//    print_data(data);
//    printf("%d\n", get_symbol(data));
//    printf("%d\n", UINT16_MAX);
	l = get_word_list(data);
	set_savings(l, data);
//    print_word_list(l,data);
	l = get_best(l, data);
	data = build_output(data, l);
	output_data("output", data);
//    output_data("output", data);
    } else if (strcmp(argv[1], "x") == 0) {
	data_t *data;
	data = load_file(argv[2]);
	data = decompress(data);
	output_data("decompressed", data);
    }
    exit(0);
}

data_t *decompress(data_t * id)
{
    data_t *out;
    long int i;			//source location ptr
    long int j;			//output location ptr
    char *base;
    uint16_t sym;
    int len;
    char *str;
    char *data;

    data = id->data;

    out = new_data();

    out->data = malloc(2 * id->size);

    memcpy(&sym,id->data,2);
    len = *(data + 2);
    str = data + 3;
    printf("//////////////////////\n"
	   "sym:\t%d\tlen\t%d\n" "//////////////////////\n", sym,len);

    base = data + 3;
    base = base + *(base - 1);

    for (i = 0, j = 0; i < id->size-3-len; i++, j++) {
	if (i < id->size-4-len && memcmp(&sym,base+i,2)==0) {
	    memcpy(out->data + j, str, len);
	    i++;		//the for loop makes up the remainder
	    j += len - 1;
	} else {
	    *(out->data + j) = *(base + i);
	}
    }
    out->size = j;
    return out;

}

data_t *build_output(data_t * id, node_t * b)
{
    data_t *od;
    char *base;
    uint16_t sym;
    long int i;
    long int j;
    char *rep;

    od = new_data();
    od->data = malloc(id->size * 2);

    sym = get_symbol(id);
    printf("//////////////////////\n"
	   "sym:\t%d\n" "//////////////////////\n", sym);

    memcpy(od->data,(void *)&sym,2);	//adds symbol to the header

    *(od->data + 2) = (char) b->len;	//adds length of replacement to header

    rep = (od->data + 3);
    memcpy(rep, id->data + b->pos, b->len);	//adds replacement string to header

    base = rep + b->len;


    for (i = 0, j = 0; i < id->size; i++, j++) {
	if (memcmp(id->data + i, rep, b->len) == 0) {
//	    *((uint16_t *) base + j) = sym;
	    memcpy(base+j,&sym,2);
	    j++;		//Only add one; for loop makes up diff
	    i += b->len - 1;	//See above
	} else {
	    *(base + j) = *(id->data + i);
	}
    }

    od->size = b->len + 3 + j;

    return od;
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
    putchar('|');
    fwrite(d->data+best->pos,best->len,1,stdout);
    putchar('|');
    putchar('\n');
    printf("Saves: %ld\n",best->saves);
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
	//printf("Processing Len:\t%ld\n", i);
	for (j = 0; j < dlen - i; j++) {
	    node_t *f;
	    node_t *new;
	    new = new_node(j, i);

	    /*Display Progress
	    if (j % 1000 == 0) {
		printf("Percent done:\t%f\n", j * 1.0 / (dlen - i));
	    }
	    */

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
    if (a >> 8 == 0)
	a = a | 1 << 16;
    if (a << 8 == 0)
	a = a | 1;
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
    return new;
}
