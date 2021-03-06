/******************************
 * parser.h
 * 
 * Responsible for taking a tokenized string from tokenizer 
 * and turning it into a parse tree.
 *
 * For now, the "parse tree" is a simple tree of things to
 * be executed.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "parser.h"
#include "tokenizer.h"
#include "common.h"

commanditem* create_commanditem();
command* create_command();

/**
 * is_backtick, is_pipe
 *
 * Small helper functions.
 */
bool is_backtick(struct tokenized_node* token)
{
    if (*(token->contents) == '`')
        return true;

    return false;
}
bool is_pipe(struct tokenized_node* token)
{
    if (*(token->contents) == '|')
        return true;
    
    return false;
}


/**************************
 * command* create_parse_tree_recursive(
 *      struct tokenized_node*,
 *
 * Input:
 *  tokenized_command   - Token node to start at
 *  
 * Given a tokenized command, create a parse tree out of it.
 */
command* create_parse_tree_recursive(struct tokenized_node* tokenized_command)
{

    // Used so we don't have to special-case first node in the tree
    command fake_start_node;
    fake_start_node.piped_to = NULL;
    command* current_command = &fake_start_node;
    bool in_command = false;

    commanditem fake_start_item;
    fake_start_item.next = NULL;
    commanditem* current_command_item = &fake_start_item;

    struct tokenized_node* token_iterator = tokenized_command;

    for (iterator(token_iterator, tokenized_command))
    {

        if (!in_command)
        {
            current_command->piped_to = create_command();
            current_command = current_command->piped_to;

            in_command = true;
        }

        if (is_pipe(token_iterator))
        {
            in_command = false;

            current_command->contents = fake_start_item.next;
            current_command_item = &fake_start_item;

            continue;
        }

        if (is_backtick(token_iterator))
        {
            perror("You're fucked.\n");
            exit(1);
        }

        // Input redirect
        if ('<' == *(token_iterator->contents))
        {
            token_iterator = token_iterator->next;
            
            current_command->input
                = malloc(sizeof(char) * (strlen(token_iterator->contents) + 1));
            strcpy(current_command->input, token_iterator->contents);

            continue;
        }

        // Output redirect
        if ('>' == *(token_iterator->contents))
        {
            token_iterator = token_iterator->next;
            
            if (token_iterator && '>' == *(token_iterator->contents))
            {
                current_command->output_append = true;
                token_iterator = token_iterator->next;
            }

            current_command->output
                = malloc(sizeof(char) * (strlen(token_iterator->contents) + 1));
            strcpy(current_command->output, token_iterator->contents);

            continue;

        }

        // Background
        if ('&' == *(token_iterator->contents))
        {
            in_command = false;

            current_command->background = true;
            current_command->contents = fake_start_item.next;
            current_command_item = &fake_start_item;

            continue;
        }

        commanditem* newitem = create_commanditem(ARGUMENT);

        newitem->contents
            = malloc(sizeof(char) * (strlen(token_iterator->contents) + 1));
        strcpy(newitem->contents, token_iterator->contents);

        current_command_item->next = newitem;
        current_command_item = current_command_item->next;

    }
 
    // Make sure last command has contents.
    current_command->contents = fake_start_item.next;

    return fake_start_node.piped_to;

}

/*************************
 * command* create_parse_tree(struct tokenized_node*)
 *
 * Given a tokenized command, turns it into a parse tree.
 * This method is a dummy method to call the recursive method
 * with the default parameters.
 */
command* create_parse_tree(
    struct tokenized_node* tokenized_command)
{
    return create_parse_tree_recursive(tokenized_command);
}



/*****************
 * Allocates and returns a commanditem with a given
 * node_type.
 */
commanditem* create_commanditem(enum node_type type)
{

    commanditem* new_node = malloc(sizeof(commanditem));
    if (new_node == NULL)
        perror("Out of space!");

    new_node->type = type;
    new_node->contents = NULL;
    new_node->next = NULL;

    return new_node;

}

/*****************
 * Allocates and returns a command.
 */
command* create_command()
{

    command* new_node = malloc(sizeof(command));
    if (new_node == NULL)
        perror("Out of space!");

    new_node->piped_to = NULL;
    new_node->input = NULL;
    new_node->output = NULL;
    new_node->output_append = false;
    new_node->background = false;

    return new_node;

}

/*********************
 * Given a constructed parse tree, free it and
 * all of its child nodes.
 */
void free_parse_tree(command* parse_tree)
{

    command* command_iterator = parse_tree;
    while (command_iterator)
    {
        command* next_command = command_iterator->piped_to;

        commanditem* commanditem_iterator = command_iterator->contents;

        while (commanditem_iterator)
        {
            commanditem* next_commanditem = commanditem_iterator->next;

            if (commanditem_iterator->contents != NULL)
                free(commanditem_iterator->contents);
            free(commanditem_iterator);

            commanditem_iterator = next_commanditem;
        }
        if (command_iterator->input != NULL)
            free(command_iterator->input);
        if (command_iterator->output != NULL)
            free(command_iterator->output);
        free(command_iterator);

        command_iterator = next_command;
    }

}
