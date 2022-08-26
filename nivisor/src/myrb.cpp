#ifdef CFS
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <cassert>
#include <iostream>

#include "ctx.hpp"
#include "myrb.h"
#ifdef POOL
#include "pool.h"
#endif
#define INDENT_STEP  4
using namespace std;

// RULES:
// 1) Root is always black
// 2) New nodes are always red
// 3) From the root to any leaf node, the number of black nodes must be the same
// 4) You can never have two consecutive red nodes
// 5) Null nodes are black
// 
// Black aunt --> Rotate
// Red aunt --> Color flipA
//
//
//
//    Color flip:
//
//        RED
//       /   \
//    BLACK  BLACK
//
//
//
//     Rotate:
//
//      BLACK
//     /     \
//   RED     RED
//

#ifdef DEBUG
//const char *getRotationString(int val) {
//  switch (val) {
//    case LEFT_ROTATE:
//      return "LEFT_ROTATE";
//    case RIGHT_ROTATE:
//      return "RIGHT_ROTATE";
//    case LEFTRIGHT_ROTATE:
//      return "LEFTRIGHT_ROTATE";
//    case RIGHTLEFT_ROTATE:
//      return "RIGHTLEFT_ROTATE";
//    default:
//      return "UNKNOWN ROTATION";
//  }
//}
//
template <class K, class V>
void RBNode<K,V>::print(void) {
  DPRINTF("Node: <%d> : <%p>  %p\n", this->m_key, this->m_val, this);
  DPRINTF("Left: %p\n", this->left);
  DPRINTF("Right: %p\n", this->right);
  DPRINTF("Parent: %p\n", this->parent);
  DPRINTF("m_pos: %s\n", this->m_pos == LEFT ? "LEFT" : "RIGHT");
  DPRINTF("m_color: %s\n", this->m_color == BLACK ? "BLACK" : "RED");
  DPRINTF("se_id: 0x%08llx\n", this->se_id);

  DPRINTF("is_root: %p\n", &this->is_root);
  DPRINTF("se_id: %p\n", &this->se_id);
  DPRINTF("Parent: %p\n", &this->parent);
  DPRINTF("left: %p\n", &this->left);
  DPRINTF("right: %p\n", &this->right);
  DPRINTF("color: %p\n", &this->m_color);
  DPRINTF("pos: %p\n", &this->m_pos);
  return;
};
#endif

template <class K, class V>
RBNode<K,V>::RBNode(K key, V val) {
  m_key = key;
  m_val = val;
  m_color = RED;
  m_pos = LEFT;
  parent = NULL;
  left = NULL;
  right = NULL;
#ifdef FART
  is_root = -1;
  se_id = 0;
#endif
};


template <class K, class V>
bool RBNode<K,V>::isLeaf(void) {
  return ( (this->left == NULL) && (this->right == NULL));
};

template <class K, class V>
RBNode<K,V> *RBNode<K,V>::sibling(void) {
  if (this->parent == NULL) {
    return NULL;
  }

  return this->m_pos == LEFT ? this->parent->right : this->parent->left;
}

template <class K, class V>
uint32_t RBNode<K,V>::childrenColor(void) {
  uint32_t children_color = 0;
  if (this->left)
    children_color |= this->left->m_color == RED ? LEFT_CHILD_RED : LEFT_CHILD_BLACK;
  else {
    children_color |= LEFT_CHILD_NULL;
  }

  if (this->right)
    children_color |= this->right->m_color == RED ? RIGHT_CHILD_RED : RIGHT_CHILD_BLACK;
  else {
    children_color |= RIGHT_CHILD_NULL;
  }

  return children_color;
}


template <class K, class V>
int RBTree<K,V>::rotate(RBNode<K,V> *node) {
  // we need to determine what kind of rotation we want to do
  int type = (node->m_pos << 1) | node->parent->m_pos;
  //printf("type of rotation: %s\n", getRotationString(type));

  // remember that for all of these, we operate/expect to be passed the grandparent
  switch (type) {
    case LEFT_ROTATE:
      left_rotate(node->parent->parent);
      node->m_color = RED;
      node->parent->m_color = BLACK;
      node->parent->left->m_color = RED;
      break;

    case RIGHT_ROTATE:
      right_rotate(node->parent->parent);
      node->m_color = RED;
      node->parent->m_color = BLACK;
      node->parent->right->m_color = RED;
      break;

    case LEFTRIGHT_ROTATE:
      leftright_rotate(node->parent->parent);
      node->m_color = BLACK;
      node->left->m_color = RED;
      node->right->m_color = RED;
      break;

    case RIGHTLEFT_ROTATE:
      rightleft_rotate(node->parent->parent);
      node->m_color = BLACK;
      node->left->m_color = RED;
      node->right->m_color = RED;
      break;

    default:
      abort();
  }
  return -1;
};

template <class K, class V>
int RBTree<K,V>::correctTree(RBNode<K,V> *node) {
  // we need to determine the location of our aunt
  // if our parent is a left child, the aunt will be on the grandparents right
  // if our parent is a right child, the aunt will be on the grandparents left
  enum pos aunt_loc = node->parent->m_pos == LEFT ? RIGHT : LEFT;
  if (aunt_loc == RIGHT) {
    // case of a black aunt
    if (node->parent->parent->right == NULL
        || node->parent->parent->right->m_color == BLACK) {
      // rotate
      rotate(node);
    }

    // case of a red aunt
    else {
      // color flip
      // parent gets set to black, gp get sets to red, aunt gets set to black
#ifdef DEBUG
      assert(node->parent->parent->right->m_color == RED);
#endif
      node->parent->m_color = BLACK;
      node->parent->parent->m_color = RED;
      node->parent->parent->right->m_color = BLACK;
    }
  }

  else {
#ifdef DEBUG
    assert(aunt_loc == LEFT);
#endif
    // case of a black aunt
    if (node->parent->parent->left == NULL
        || node->parent->parent->left->m_color == BLACK) {
      rotate(node);
    }

    // case of a red aunt
    else {
#ifdef DEBUG
      assert(node->parent->parent->left->m_color == RED);
#endif
      // color flip
      // parent gets set to black, gp get sets to red, aunt gets set to black
      node->parent->m_color = BLACK;
      node->parent->parent->m_color = RED;
      node->parent->parent->left->m_color = BLACK;
    }

  }

  return 0;
}

template <class K, class V>
uint64_t RBTree<K,V>::numBlackNodes(RBNode<K,V> *node) {
  if (node == NULL)
    return 0;
  uint64_t right_black_nodes = numBlackNodes(node->right);
  uint64_t left_black_nodes = numBlackNodes(node->left);

  if (right_black_nodes != left_black_nodes) {
    abort();
  }

  else {
    if (node->m_color == BLACK)
      left_black_nodes++;
    return left_black_nodes;
  }
  return 0;
}

// recurses up the tree, checking for red color violations along the way
template <class K, class V>
int RBTree<K,V>::checkColor(RBNode<K,V> *node) {
  int err = 0;
  // at the root node
  // TODO: Do we need to check that it's black?
  if (node == m_root) {
    node->m_color = BLACK;
    return 0;
  }

  // TODO 
  // check to make sure node->parent->parent isn't NULL here
  if (node->m_color == RED && node->parent->m_color == RED) {
    err = correctTree(node);
  }

  // if our node is now the root from the correction, do not recurse further
  if (node == m_root) {
    return err;
  }

  err = checkColor(node->parent);
  return err;
}

template <class K, class V>
int RBTree<K,V>::add(RBNode<K,V> *parent, RBNode<K,V> *new_node) {
  // go down the right
  if (new_node->m_key > parent->m_key) {
    // parent has no right child.
    // New node is now the right child
    if (parent->right == NULL) {
      parent->right = new_node;
      new_node->parent = parent;
      new_node->m_pos = RIGHT;
      return 0;
    }

    // parent DOES have a right child
    this->add(parent->right, new_node);
  }

  // go down left
  else {
    // parent has no left child
    // new node is now the left child
    if (parent->left == NULL) {
      parent->left = new_node;
      new_node->parent = parent;
      new_node->m_pos = LEFT;
      return 0;
    }

    // parent DOES have a left child
    this->add(parent->left, new_node);

  }

  checkColor(new_node);
  return 0;
}

template <class K, class V>
#ifdef FART
int RBTree<K,V>::add(K key, V val, int64_t id) {
#else
int RBTree<K,V>::add(K key, V val) {
#endif
  //printf("\n[+] Adding %d\n", key);
  int err = 0;
#ifdef POOL
  RBNode<K,V> *node = allocNode(key, val);
#ifdef FART
  node->se_id = id;
#endif
  if (!node)
    return -1;
#else
  RBNode<K,V> *node = new RBNode<K,V>(key, val);
#endif
  if (m_root == NULL) {
    m_root = node;
    node->is_root = 1;
    node->m_color = BLACK;
    m_size++;
    if (m_size > high_mark)
      high_mark = m_size;
    return 0;
  }
  err = this->add(m_root, node);
  if (!err) {
    m_size++;
    if (m_size > high_mark)
      high_mark = m_size;
  }
  return err;
}


template <class K, class V>
//RBNode<K,V> *RBTree<K, V>::left_rotate(RBNode<K,V> *node) {
int RBTree<K, V>::left_rotate(RBNode<K,V> *node) {

  // set temp to nodes right child
  /*            
   *          a  
   *           \
   *            4 <--- node
   *           / \
   *          x   6  <--- temp
   *             / \
   *            y   8
   *               /
   *              z
   */    
  RBNode<K,V> *temp = node->right;

  // set nodes right child to temps left child
  /*             
   *             
   *        a    
   *         \   \
   *          4   6  <--- temp
   *         / \ / \
   *        x   y   8
   *               /
   *              z
   *
   *      4->left = x
   *      4->right = y
   *
   *      y->parent = 6
   *      y->m_pos = LEFT
   */    
  node->right = temp->left;

  // if temp->left (y here) isn't NULL, we need to update it's parent
  // We also need to update it's position since it's no longer a left child of 6, it's now a right child of 4
  if (node->right != NULL) {
    node->right->parent = node;
    node->right->m_pos = RIGHT;
  }

  // Now we need to update temps parent
  /*             
   *             
   *        a    
   *         \   \
   *          4   6  <--- temp
   *         / \ / \
   *        x   y   8
   *               /
   *              z
   *
   *      4->left = x
   *      4->right = y
   *
   *      y->parent = 4
   *      y->m_pos = RIGHT
   */    
  // was `node` (4 here), the root?
  if (node->parent == NULL) {
    temp->parent = NULL;
    temp->m_color = BLACK;
    //printf("[###] just set the root to: %p\n", temp);
    m_root = temp;
  }

  // else 4 was not the root
  else {
    temp->parent = node->parent;
    // we also need to update the position of temp
    /*             
     *             
     *        a   a
     *         \   \
     *          4   6  <--- temp
     *         / \ / \
     *        x   y   8
     *               /
     *              z
     *
     */

    // node was a left child
    if (node->m_pos == LEFT) {
      // update our position to also be left
      temp->m_pos = LEFT;
      // and update a's `left` to point to us
      temp->parent->left = temp;
    }

    // node was a right child
    else {
#ifdef DEBUG
      assert(node->m_pos == RIGHT);
#endif
      // update our position to also be left
      temp->m_pos = RIGHT;
      // and update a's `right` to point to us
      temp->parent->right = temp;

    }

  }


  // set temp's left child = node
  /*             
   *              
   *            a 
   *             \
   *              6  <--- temp
   *             / \
   *        a   /   \
   *         \ /     \
   *          4       8
   *         / \     /
   *        x   x   z           
   *                            
   *                            
   */    
  temp->left = node;
  // update nodes parent from a to 6
  node->parent = temp;
  // and finally update it's position
  // as it's now the left child of temp
  node->m_pos = LEFT;

  // now treat temp as the node
  /*             
   *              
   *            a 
   *             \
   *              6  <--- new node
   *             / \
   *            /   \
   *           /     \
   *          4       8
   *         / \     /
   *        x   x   z           
   *                            
   *                            
   */    

  return 0;
}


template <class K, class V>
//RBNode<K,V> *RBTree<K, V>::right_rotate(RBNode<K,V> *node) {
int RBTree<K, V>::right_rotate(RBNode<K,V> *node) {
  // set temp to nodes left child
   /*
   *
   *             a
   *              \
   *               8  <--- node
   *              / \
   * temp ---->  6   x
   *            / \
   * imb --->  4   y
   *            \
   *             z
   *
   */
  RBNode<K,V> *temp = node->left;

  // now that we've saved a reference to it,
  // set the nodes left child to temps right child
  node->left = temp->right;

   /*
   *
   *               a
   *                \
   *                 \
   *              /   8 <--- node
   * temp ---->  6   / \
   *            / \ /   \
   * imb --->  4   y     x
   *            \
   *             z
   *
   */

  // Now we need to update y's parent and position
  if (temp->right != NULL) {
    temp->right->parent = node;
    temp->right->m_pos = LEFT;
  }

  // next, update temps parent if node had a parent

  // is the node the root?
  if (node->parent == NULL) {
    temp->parent = NULL;
    temp->m_color = BLACK;
    //printf("[######] just set the root to: %p\n", temp);
    m_root = temp;
  }

  // if not, then the node (8) has a parent, so we need to update links
  else {
    temp->parent = node->parent;
    // we need to know nodes position as well to properly update those links (and positions)
    if (node->m_pos == LEFT) {
      temp->parent->left = temp;
      temp->m_pos = LEFT;
    }
    else {
#ifdef DEBUG
      assert(node->m_pos == RIGHT);
#endif
      temp->parent->right = temp;
      temp->m_pos = RIGHT;
    }
  }

  // Now that we have something pointing to temps right child,
  // we can set temp's right child = node
  temp->right = node;

  // update node's parent to point to temp
  node->parent = temp;
  // and finally set it's position to RIGHT,
  // as it's now the right child of temp
  node->m_pos = RIGHT;

  // now treat temp as the new node
  return 0;
}


/*                          RIGHT LEFT ROTATE
 *
 *               4                     4                        6
 *                \    right rotate     \      left rotate     / \
 *                 8    ========>        6      ========>     4   8
 *                /     parent            \    grandparent   
 *               6                         8     
 *                                    
 *
 */

// assume we are passed in the grandparent
template <class K, class V>
//RBNode<K,V> *RBTree<K, V>::rightleft_rotate(RBNode<K,V> *node) {
int RBTree<K, V>::rightleft_rotate(RBNode<K,V> *node) {
  // since we're passed in the gradparent, the "parent" is the grandparents right child
  right_rotate(node->right);
  return left_rotate(node);
}

/*                          LEFT RIGHT ROTATE
 *
 *               8                     8                     6
 *              /       left rotate   /     right rotate    / \
 *             4        ========>    6       ========>     4   8
 *              \       parent      /       grandparent   
 *               6                 4          
 *                                    
 *
 */

// assume we are passed in the grandparent
template <class K, class V>
//RBNode<K,V> *RBTree<K, V>::leftright_rotate(RBNode<K,V> *node) {
int RBTree<K, V>::leftright_rotate(RBNode<K,V> *node) {
  // since we're passed in the gradparent, the "parent" is the grandparents left child
  left_rotate(node->left);
  return right_rotate(node);
}

template <class K, class V>
RBNode<K,V> *RBTree<K,V>::getLeftmostNode(void) {
  if (m_root == NULL) {
    return NULL;
  }

  RBNode<K,V> *cur = m_root;
  while (cur->left) {
    cur = cur->left;
  }
  return cur;
}
template <class K, class V>
RBNode<K,V> *RBTree<K,V>::removeLeftmostNode(void) {
  RBNode<K,V> *node = getLeftmostNode();
  if (!node) {
    //printf("oh fuck\n");
    return NULL;
  }
#ifdef DEBUG
  assert(node->left == NULL);
#endif

  // let's do the simple cases first

  // ###############
  // red leaf node
  // ###############
  if ( (node->isLeaf())
      && (node->m_color == RED) ) {
    DPRINTF("Red leaf case\n");
    // don't need to do anything special. just remove it

    // SHOULD NEVER ACTUALLY HIT THIS BECAUSE WE SAID WE'RE RED
#ifdef DEBUG
    assert (m_root != node);
#endif
    //// are we the root node? even easier
    //if (m_root == node) {
    //  m_root = NULL;
    //  return node;
    //}
    
    // update the parent pointer accordingly
    if (node->m_pos == LEFT) {
      node->parent->left = NULL;
    }
    else {
#ifdef DEBUG
      assert(node->m_pos == RIGHT);
#endif
      node->parent->right = NULL;
    }

    node->parent = NULL;
    m_size--;
    return node;
  }

  // Let v be the node to be deleted and u be the child that replaces v
  // v <-- to be deleted
  // u <-- child to be deleted (only right for us)

  // #########################
  // either u or v is red
  // #########################
  uint32_t children_color = node->childrenColor();
  if (node->m_color == RED || (children_color & RED_CHILD_MASK) ) {
    DPRINTF("Removing node where either u or v is red (easy)\n");
    // mark the replaced child as black (No change in black height).
    // Note that both u and v cannot be red as v is parent of u and two consecutive reds are not allowed in red-black tree. 
    RBNode<K,V> *u = node->right;
    u->m_color = BLACK;

    // are we the root?
    if (node == m_root) {
      //printf("\n\n[!] Just set the root to: %p\n", u);
      m_root = u;
#ifdef FART
      u->is_root = 1;
#endif
      u->parent = NULL;
      node->right = NULL;
      m_size--;
      return node;
    }

    RBNode<K,V> *parent = node->parent;
    // if not, update our parent pointer accordingly
    if (node->m_pos == LEFT) {
      node->parent->left = u;
    }

    else {
#ifdef DEBUG
      assert(node->m_pos == RIGHT);
#endif
      node->parent->right = u;
    }

    u->parent = node->parent;
    node->parent = NULL;
    u->m_pos = node->m_pos;
    m_size--;
    return node;
  }

  // Okay so now for the fucked up stuff
  // so now we're either a black leaf, or a black node with a black child

  // if we're a black leaf, fix the double black on US (v)
  if (node->isLeaf()) {
    DPRINTF("Removing Black Leaf\n");
    RBNode<K,V> *temp = node;
    fixDB(node);
    if (temp == m_root) {
      //printf("Better see this shit\n");
      //printf("\n\n[!!] Just set the root to NULL\n");
      m_root = NULL;
    }

    else {
      if (temp->m_pos == LEFT) {
        temp->parent->left = NULL;
      }
      else {
        temp->parent->right = NULL;
      }
    }
  }
  // otherwise fix the double black on our kid (u)
  else {
    DPRINTF("Removing black node with children\n");
#ifdef DEBUG
    assert(node != m_root);
    assert(node->right);
#endif
    fixDB(node->right);
    // TODO: needed???
    node->right->m_pos = node->m_pos;
    if (node->m_pos == LEFT) {
      node->parent->left = node->right;
    }
    else {
      node->parent->right = node->right;
    }
    node->right->parent = node->parent;

  }

  m_size--;
  return node;
}

template <class K, class V>
void RBTree<K,V>::fixDB(RBNode<K,V> *node) {
  if (node == m_root || node == NULL) {
    return;
  }

  RBNode<K, V> *s = node->sibling();
  if (s == NULL) {
    fixDB(node->parent);
  }

  // case 5 and 6
  uint32_t children_color = s->childrenColor();
  if ( (s->m_color == BLACK) && (children_color & RED_CHILD_MASK)) {
    RBNode<K, V> *tmp = nullptr;

    if (s->m_pos == LEFT) {
      //printf("3.2 (a) LEFT\n");
      // case 5. Sibling is black, far away child is black, near child is red
      if ( ( (children_color & LEFT_CHILD_BLACK) || (children_color & LEFT_CHILD_NULL) ) ) {
        //printf("case 5 A\n");
        // (a) swap color of sibling with siblings red child
#ifdef DEBUG
        assert(s->right);
#endif
        // Okay so the siblings RED child should be on the RIGHT, because it's NEAR to us (the DB), and since our sibling is on the LEFT, that means WE'RE (DB) on the RIGHT
        // so the RED CHILD NEAR US, is also on the RIGHT
        s->right->m_color = s->m_color;
        s->m_color = RED;
        // (b) perform rotation at sibling node in direction opposite of DB
        DPRINTF("LEFT_ROTATE\n");
        left_rotate(s);
        // (c) apply case 6 (below)
        fixDB(node);
      }

      //children_color = s->childrenColor();
      switch (children_color & RED_CHILD_MASK) {
        case RIGHT_CHILD_RED:
          break;
        // case 6. Sibling is black and far away child is red
        case RED_CHILD_MASK:
        case LEFT_CHILD_RED:
          DPRINTF("case 6 A\n");
          // remember the red child
          tmp = s->left;
          // (a) swap color of DB's parent with DB sibling's color
          s->m_color = node->parent->m_color;
          node->parent->m_color = BLACK;
          // (b) perform rotation at DB's parent in directon of DB
          DPRINTF("RIGHT_ROTATE\n");
          right_rotate(node->parent);
          // (c) remove DB sign and make node a normal black node
          node->m_color = BLACK;
          // (d) change the far red child to black
          tmp->m_color = BLACK;
          break;

        default:
          //printf("VERY NOT AMORE\n");
          abort();
      }
    }

    else {
      //printf("3.2 (a) RIGHT\n");
      //printf("RED_CHILD_MASK: 0x%x\n", RED_CHILD_MASK);
      //printf("children_color: %d\n", children_color);
#ifdef DEBUG
      assert(s->m_pos == RIGHT);
#endif

      // case 5. Sibling is black, far away child is black, near child is red
      if ( ( (children_color & RIGHT_CHILD_BLACK) || (children_color & RIGHT_CHILD_NULL) ) ) {
        DPRINTF("case 5 B\n");
        // (a) swap color of sibling with siblings red child
#ifdef DEBUG
        assert(s->left);
#endif
        // s->right could be NULL here, either way it's blac, so just make it that
        s->left->m_color = s->m_color;
        s->m_color = RED;
        // (b) perform rotation at sibling node in direction opposite of DB
        DPRINTF("RIGHT_ROTATE\n");
        right_rotate(s);
        // (c) apply case 6 (below)
        // TODO TODO: Do I need to grab the variables again because the tree changed?
        fixDB(node);
      }
      

      //children_color = s->childrenColor();
      switch (children_color & RED_CHILD_MASK) {
        case LEFT_CHILD_RED:
          break;
        // case 6. Sibling is black and far away child is red
        case RED_CHILD_MASK:
        case RIGHT_CHILD_RED:
          //printf("case 6 B\n");
          // remember the red child
          tmp = s->right;
          // (a) swap color of DB's parent with DB sibling's color
          s->m_color = node->parent->m_color;
          node->parent->m_color = BLACK;
          // (b) perform rotation at DB's parent in directon of DB
          DPRINTF("LEFT_ROTATE\n");
          left_rotate(node->parent);
          // (c) remove DB sign and make node a normal black node
          node->m_color = BLACK;
          // (d) change the far red child to black
          tmp->m_color = BLACK;
          break;
        default:
          //printf("THAT'S A BAD NEWS\n");
          //printf("children_color: %d\n", children_color);
          abort();
      }
    }
    //printf("mdk\n");
  }

  // 3.2 (b)  If sibling is black and both its children are black
  else if ( (s->m_color == BLACK) && (children_color & BLACK_OR_NULL_MASK) ) {
    //printf("3.2 (b)\n");
    s->m_color = RED;
    if (node->parent->m_color == BLACK)
      fixDB(node->parent);
    else
      node->parent->m_color = BLACK;

    return;
  }

  // 3.2 (c) If sibling is red, perform a rotation to move old sibling up, recolor the old sibling and parent. The new sibling is always black
  else if (s->m_color == RED) {
    //printf("3.2 (c)\n");
    s->m_color = node->parent->m_color;
    node->parent->m_color = RED;
    // (i) Left Case (s is left child of its parent). 
    // Right rotate parent
    if (s->m_pos == LEFT) {
      //printf("MAKE SURE I'M DOING A RIGHT ROTATE\n");
      right_rotate(node->parent);
    }

    // (ii) Right Case (s is right child of its parent).
    else {
#ifdef DEBUG
      assert(s->m_pos == RIGHT);
#endif
      //printf("MAKE SURE I'M DOING A LEFT ROTATE\n");
      left_rotate(node->parent);
    }
    fixDB(node);
  }

  else {
    abort();
  }
  return;
}

#ifdef POOL
template <class K, class V>
RBNode<K,V> *RBTree<K,V>::allocNode(K key, V val) {
  //m_mpool->Print();
  void *node_mem = m_mpool->AllocBlock();
  //printf("node_mem: %p\n", node_mem);
  RBNode<K,V> *new_node = new (node_mem)RBNode<K,V>(key,val);
  return new_node;
}
template <class K, class V>
void RBTree<K,V>::deleteNode(RBNode<K,V> *node) {
  m_mpool->FreeBlock(node);
}

template <class K, class V>
RBTree<K,V>::RBTree(void *heap) {
  m_mpool = create_mempool(heap, HEAP_SIZE);
  m_mpool->m_high_mark = 0;
  m_root = NULL;
}

template <class K, class V>
uint32_t RBTree<K,V>::GetBlockHighMark(void) {
  return m_mpool->m_high_mark;
}
#endif

#ifdef DEBUG
/*
 * Print RBTRee
 */
template <class K, class V>
void print_tree_helper(RBNode<K,V> *n, int indent)
{
    int i;
    if (n == NULL)
    {
        fputs("<empty tree>", stdout);
        return;
    }
    if (n->right != NULL)
    {
        print_tree_helper(n->right, indent + INDENT_STEP);
    }
    for(i = 0; i < indent; i++)
        fputs(" ", stdout);
    if (n->m_color == BLACK)
        cout<<(int)n->m_key<<endl;
    else
        cout<<"<"<<(int)n->m_key<<">"<<endl;
    if (n->left != NULL)
    {
        print_tree_helper(n->left, indent + INDENT_STEP);
    }
}

template <class K, class V>
void print_tree(RBTree<K,V> *tree)
{
    print_tree_helper(tree->m_root, 0);
    puts("");
}
#endif

/*
int main(void) {
  RBTree<int, int> *tree = new RBTree<int,int>();
  //tree->add(0, 0x1337);
  //tree->add(2, 0x1337);
  //tree->add(4, 0x1337);
  //tree->add(5, 0x1337);
  //tree->add(6, 0x1337);
  //tree->add(7, 0x1337);
  //tree->add(8, 0x1337);
  //tree->add(9, 0x1337);
  //tree->add(10, 0x1337);
  //
  tree->add(12, 0x1337);
  tree->add(7, 0x1337);
  tree->add(5, 0x1337);
  tree->add(0, 0x1337);
  tree->add(19, 0x1337);
  tree->add(3, 0x1337);
  tree->add(8, 0x1337);
  tree->add(9, 0x1337);
  tree->add(10, 0x1337);
  tree->numBlackNodes(tree->m_root);
  print_tree(tree);
  printf("\n\n\n");

  printf("Removing node\n");
  tree->removeLeftmostNode();
  print_tree(tree);
  printf("-----------------\n\n\n");

  printf("Removing node\n");
  tree->removeLeftmostNode();
  print_tree(tree);
  printf("-----------------\n\n\n");

  printf("Removing node\n");
  tree->removeLeftmostNode();
  print_tree(tree);
  printf("-----------------\n\n\n");

  printf("Removing node\n");
  tree->removeLeftmostNode();
  print_tree(tree);
  printf("-----------------\n\n\n");

  printf("Removing node\n");
  tree->removeLeftmostNode();
  print_tree(tree);
  printf("-----------------\n\n\n");

  printf("Removing node\n");
  tree->removeLeftmostNode();
  print_tree(tree);
  printf("-----------------\n\n\n");

  printf("Removing node\n");
  tree->removeLeftmostNode();
  print_tree(tree);
  printf("-----------------\n\n\n");

  printf("Removing node\n");
  tree->removeLeftmostNode();
  print_tree(tree);
  printf("-----------------\n\n\n");
}
*/

#ifdef CFS
template class RBTree<uint64_t, SchedEntity *>;
template class RBNode<uint64_t, SchedEntity *>;
#endif
#endif
