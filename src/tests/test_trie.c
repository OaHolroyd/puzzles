#include "testing.h"

#include "src/core/trie.h"


int main(void) {
  START_TEST("trie");

  /* check insertion works as expected */
  SUBTEST("insertion") {
    Trie root = trie_root();

    /* insert some words */
    trie_insert(&root, "a");
    trie_insert(&root, "aa");
    trie_insert(&root, "ab");
    trie_insert(&root, "bc");

    // This should create the following trie:
    //        ROOT
    //        / \
    //       /   \
    //      a     b
    //     / \    |
    //    a   b   c

    /* root node should have children for 'a' and 'b' and nothing else */
    REQUIRE_BARRIER(root.children[TRIE_IDX('a')]);
    REQUIRE_BARRIER(root.children[TRIE_IDX('b')]);
    for (char c = 'c'; c <= 'z'; c++) {
      REQUIRE(!root.children[TRIE_IDX(c)]);
    }
    REQUIRE(root.is_terminal == 0);
    REQUIRE(root.parent == NULL);
    REQUIRE(root.letter == '\0');

    /* the first 'a' node should be terminal and have two children */
    Trie *a1 = root.children[TRIE_IDX('a')];
    REQUIRE_BARRIER(a1->children[TRIE_IDX('a')]);
    REQUIRE_BARRIER(a1->children[TRIE_IDX('b')]);
    for (char c = 'c'; c <= 'z'; c++) {
      REQUIRE(!a1->children[TRIE_IDX(c)]);
    }
    REQUIRE(a1->is_terminal == 1);
    REQUIRE(a1->parent == &root);
    REQUIRE(a1->letter == 'a');

    /* the first 'b' node should not be terminal and have one child */
    Trie *b1 = root.children[TRIE_IDX('b')];
    REQUIRE(!b1->children[TRIE_IDX('a')]);
    REQUIRE(!b1->children[TRIE_IDX('b')]);
    REQUIRE_BARRIER(b1->children[TRIE_IDX('c')]);
    for (char c = 'd'; c <= 'z'; c++) {
      REQUIRE(!b1->children[TRIE_IDX(c)]);
    }
    REQUIRE(b1->is_terminal == 0);
    REQUIRE(b1->parent == &root);
    REQUIRE(b1->letter == 'b');

    /* the second 'a' node should be terminal and have no children */
    Trie *a2 = a1->children[TRIE_IDX('a')];
    for (char c = 'a'; c <= 'z'; c++) {
      REQUIRE(!a2->children[TRIE_IDX(c)]);
    }
    REQUIRE(a2->is_terminal == 1);
    REQUIRE(a2->parent == a1);
    REQUIRE(a2->letter == 'a');

    /* the second 'b' node should be terminal and have no children */
    Trie *b2 = a1->children[TRIE_IDX('b')];
    for (char c = 'a'; c <= 'z'; c++) {
      REQUIRE(!b2->children[TRIE_IDX(c)]);
    }
    REQUIRE(b2->is_terminal == 1);
    REQUIRE(b2->parent == a1);
    REQUIRE(b2->letter == 'b');

    /* the 'c' node should be terminal and have no children */
    Trie *c2 = b1->children[TRIE_IDX('c')];
    for (char c = 'a'; c <= 'z'; c++) {
      REQUIRE(!c2->children[TRIE_IDX(c)]);
    }
    REQUIRE(c2->is_terminal == 1);
    REQUIRE(c2->parent == b1);

    trie_free(&root);
  }

  /* check contains works as expected */
  SUBTEST("contains") {
    Trie root = trie_root();

    /* insert some words */
    trie_insert(&root, "a");
    trie_insert(&root, "aa");
    trie_insert(&root, "ab");
    trie_insert(&root, "bc");

    /* check the words are in the trie */
    REQUIRE(trie_contains(&root, "a", 0));
    REQUIRE(trie_contains(&root, "a", 1));

    REQUIRE(trie_contains(&root, "aa", 0));
    REQUIRE(trie_contains(&root, "aa", 1));

    REQUIRE(trie_contains(&root, "ab", 0));
    REQUIRE(trie_contains(&root, "ab", 1));

    REQUIRE(!trie_contains(&root, "b", 0));
    REQUIRE(trie_contains(&root, "b", 1));

    REQUIRE(trie_contains(&root, "bc", 0));
    REQUIRE(trie_contains(&root, "bc", 1));

    REQUIRE(!trie_contains(&root, "c", 0));
    REQUIRE(!trie_contains(&root, "c", 1));

    REQUIRE(!trie_contains(&root, "z", 0));
    REQUIRE(!trie_contains(&root, "z", 1));

    REQUIRE(!trie_contains(&root, "zab", 0));
    REQUIRE(!trie_contains(&root, "zab", 1));

    trie_free(&root);
  }

  END_TEST();
}
