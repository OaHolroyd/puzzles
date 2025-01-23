//
// Created by Oscar Holroyd on 21/01/2025.
//

#include "trie.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>


static_assert('z' - 'a' == 25, "alphabet chars must be contiguous");
static_assert(NULL == 0, "NULL must be 0");


/**
 * Set up a node with blank child information.
 *
 * @param node pointer to the node to set up
 * @param letter the letter to store in the node
 * @param parent the parent node
 */
static void setup_node(Trie *node, char letter, Trie *parent) {
  node->letter = letter;
  node->parent = parent;
  node->is_terminal = 0;
  memset(node->children, 0, sizeof(node->children));
}


Trie trie_root(void) {
  Trie root = {
    .letter = '\0',
    .parent = NULL,
    .is_terminal = 0,
  };
  memset(root.children, 0, sizeof(root.children));

  return root;
}


void trie_free(Trie *root) {
  for (int i = 0; i < 26; i++) {
    // only free if the child exists
    if (root->children[i]) {
      // recursively free any children underneath the ith child
      trie_free(root->children[i]);

      // free the ith child itself
      free(root->children[i]);
      root->children[i] = NULL;
    }
  }
}


int trie_insert(Trie *root, const char *word) {
  Trie *node = root;

  for (int i = 0; i < strlen(word); i++) {
    const char ch = word[i];

    /* add a child to the node if it doesn't araedy have one for ch */
    if (!node->children[TRIE_IDX(ch)]) {
      node->children[ch - 'a'] = malloc(sizeof(Trie));

      if (!node->children[TRIE_IDX(ch)]) {
        return 1;
      }

      setup_node(node->children[TRIE_IDX(ch)], ch, node);
    }

    /* proceed down into the trie */
    node = node->children[TRIE_IDX(ch)];
  }

  /* the final node is at the end of the word */
  node->is_terminal = 1;

  return 0;
}


int trie_contains(Trie *root, const char *word, int prefix) {
  Trie *node = root;

  for (int i = 0; i < strlen(word); i++) {
    const char ch = word[i];

    /* only keep going if the correct child node exists */
    if (node->children[TRIE_IDX(ch)]) {
      node = node->children[TRIE_IDX(ch)];
    } else {
      return 0;
    }
  }

  /* the word is in the trie, but is it a full word? */
  return prefix || node->is_terminal;
}
