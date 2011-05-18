

#ifndef __TOKENIZE_H_GUARD__
#define __TOKENIZE_H_GUARD__

struct tokenized_node {
    char* contents;
    struct tokenized_node* next;
};

struct tokenized_node* tokenize(char* string);

void print_list(struct tokenized_node* node_to_print);

void free_list(struct tokenized_node* node_to_free);

#endif
