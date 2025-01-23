//
// Created by Oscar Holroyd on 21/01/2025.
//

#ifndef TRIE_H
#define TRIE_H

#include <stdbool.h>

typedef struct Trie {
  /* the letter held by this node */
  char letter;

  /* pointer to the parent node (if one exists) */
  struct Trie *parent;

  /* array of pointers to potential child nodes */
  struct Trie *children[26]; // can only handle the 26 lowercase letters

  /* whether this represents the end of a word */
  bool is_terminal;
} Trie;


#define TRIE_IDX(c) ((c) - 'a') // convert a lowercase letter to an index


/**
 * Creates a new trie root node.
 *
 * @return the root node
 */
Trie trie_root(void);


/**
 * Frees the memory used by the trie, not including this node.
 *
 * @param root pointer to the root node
 */
void trie_free(Trie *root);


/**
 * Create a new trie node.
 *
 * @param root pointer to the root node
 * @param word the word to add (null-terminated string)
 * @return 0 on success, 1 on failure (due to failed malloc)
 */
int trie_insert(Trie *root, const char *word);


/**
 * Check if a word is in the trie.
 *
 * @param root pointer to the root node
 * @param word the word to find (null-terminated string)
 * @param prefix whether to count a prefix as valid (1) or a require full word match (0)
 * @return 1 if the word is in the trie, 0 otherwise
 */
int trie_contains(Trie *root, const char *word, int prefix);


/**
 * Get the word from a node and all of its parents.
 *
 * @param node pointer to the node at the end of the word
 * @param word pointer to the buffer to store the word (must have space for n+1 characters)
 * @param n the length of the buffer
 */
// void trie_get_word(Trie *node, char *word, int n);

#endif //TRIE_H
