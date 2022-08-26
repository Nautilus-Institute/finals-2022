#ifdef CFS
#pragma once
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>


enum color {
  RED = 0,
  BLACK = 1
};

// (node->m_pos << 1) | node->parent->m_pos
enum rotate_mask {
  LEFT_ROTATE = 0b11,
  RIGHT_ROTATE = 0b00,
  RIGHTLEFT_ROTATE = 0b01,
  LEFTRIGHT_ROTATE = 0b10
};


enum pos {
  LEFT = 0,
  RIGHT = 1
};


enum child_mask {
  LEAF = 0,
  LEFT_CHILD_RED = 1,
  LEFT_CHILD_BLACK = 2,
  LEFT_CHILD_NULL = 4,
  RIGHT_CHILD_RED = 8,
  RIGHT_CHILD_BLACK = 16,
  RIGHT_CHILD_NULL = 32

};
#define RED_CHILD_MASK (LEFT_CHILD_RED | RIGHT_CHILD_RED)
#define BLACK_CHILD_MASK (LEFT_CHILD_BLACK | RIGHT_CHILD_BLACK)
#define BLACK_OR_NULL_MASK (LEFT_CHILD_BLACK | LEFT_CHILD_NULL | RIGHT_CHILD_BLACK | RIGHT_CHILD_NULL)

template <class K, class V>
class RBNode {
  public:
    K m_key;
    V m_val;
#ifdef FART
    RBNode *parent;
    int64_t se_id;
#endif
    int64_t is_root;
    RBNode *left;
    RBNode* right;
    enum color m_color;
    enum pos m_pos;

    RBNode(K key, V val);
    bool isLeaf(void);
    RBNode<K,V> *sibling(void);
    uint32_t childrenColor(void);
#ifdef DEBUG
    void print(void);
#endif
};

template <class K, class V>
class RBTree {
  public:
    // members
    class RBNode<K,V> *m_root;
    uint64_t m_size;
    uint64_t high_mark;
#ifdef POOL
    class MemPool *m_mpool;

    // methods
    RBTree(void *heap);
    RBNode<K,V> *allocNode(K key, V val);
    void deleteNode(RBNode<K,V> *node);
#endif

#ifdef FART
    int add(K key, V val, int64_t id);
#else
    int add(K key, V val);
#endif
    int add(RBNode<K,V> *parent, RBNode<K,V> *new_node);
    int checkColor(RBNode<K,V> *node);
    uint64_t numBlackNodes(RBNode<K,V> *node);
    int correctTree(RBNode<K,V> *node);
    int rotate(RBNode<K,V> *node);
    int right_rotate(RBNode<K,V> *node);
    int left_rotate(RBNode<K,V> *node);
    int rightleft_rotate(RBNode<K,V> *node);
    int leftright_rotate(RBNode<K,V> *node);
    RBNode<K,V> *getLeftmostNode(void);
    RBNode<K,V> *removeLeftmostNode(void);
    void fixDB(RBNode<K,V> *node);
    uint32_t GetBlockHighMark(void);
};
#endif
