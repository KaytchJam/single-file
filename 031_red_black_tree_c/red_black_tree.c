#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#define RED true
#define BLACK false

/** A node in the red black tree */
typedef struct RBNode {
    bool color;
    int value;
    struct RBNode* parent;
    union {
        struct {
            struct RBNode* left;
            struct RBNode* right;
        };
        struct RBNode* children[2];
    };
} RBNode;

/** An item in the red-black-tree */
typedef struct RBTree {
    RBNode* root;
    size_t items;
} RBTree;

/** Stack allocate an empty Red-Black-Tree */
RBTree new_tree() {
    RBTree tree;
    tree.items = 0;
    tree.root = NULL;
    return tree;
}

/** Heap-allocate a new node */
RBNode* allocate_node(RBNode* parent, int value, bool color, RBNode* left, RBNode* right) {
    RBNode* node = (RBNode*) malloc(sizeof(RBNode));

    node->parent = parent;
    node->left = left;
    node->right = right;
    node->value = value;
    node->color = color;

    return node;
}

/** Returns the sibling node of some node `child` given its parent node */
RBNode* get_sibling(RBNode* child) {
    if (child == NULL || child->parent == NULL) { return NULL; }
    return child->parent->left == child 
        ? child->parent->right 
        : child->parent->left;
}

int get_index(RBNode* child) {
    return child->parent->children[0] == child ? 0 : 1;
}

/** Pulled straight from wikipedia I'm gonna keep it a buck */
RBTree* rotate_tree(RBTree* tree, RBNode* child, int child_index) {
    RBNode* parent = child->parent;
    RBNode* new_root = child->children[1 - child_index];
    RBNode* new_child = new_root->children[child_index];

    child->children[1 - child_index] = new_child;
    if (new_child != NULL) {
        new_child->parent = child;
    }

    new_root->children[child_index] = child;
    new_root->parent = parent;
    child->parent = new_root;

    if (parent != NULL) {
        parent->children[child == parent->right] = new_root;
    } else {
        tree->root = new_root;
    }

    return tree;
}

RBTree* rebalance_tree(RBTree* tree, RBNode* child) {
    while (child->parent != NULL && child->parent->color == RED) {
        RBNode* grandparent = child->parent->parent;
        RBNode* uncle = get_sibling(child->parent);
        
        if (grandparent != NULL) {
            const int parent_index = get_index(child->parent);
            if (uncle == NULL || uncle->color == BLACK) {
                if (child == child->parent->children[1 - parent_index]) {
                    RBNode* parent = child->parent;
                    tree = rotate_tree(tree, child->parent, parent_index);
                    child = parent;
                    child->parent = grandparent->children[parent_index];
                }

                tree = rotate_tree(tree, grandparent, 1 - parent_index);
                child->parent->color = BLACK;
                grandparent->color = RED;
                
                child = tree->root;
            } else {
                child->parent->color = BLACK;
                uncle->color = BLACK;
                grandparent->color = RED;
                child = grandparent;
            }

        } else {
            child->parent->color = BLACK;
            child = tree->root;
        }
    } 

    return tree;
}

/** Checks if the Tree contains a particular value */
bool has_value(RBTree* tree, int value) {
    RBNode* node = tree->root;
    while (node != NULL && node->value != value) {
        node = value >= node->value ? node->right : node->left;
    }

    return !(node == NULL);
}

int max_depth_rec(RBNode* node, int depth) {
    if (node == NULL) { return depth; }
    const int ldepth = max_depth_rec(node->left, depth + 1);
    const int rdepth = max_depth_rec(node->right, depth + 1);
    return ldepth < rdepth ? rdepth : ldepth;
}

/** Returns the max depth of the Tree */
int max_depth(RBTree* tree) {
    return max_depth_rec(tree->root, 0);
}

int count_leaves_rec(RBNode* node) {
    if (node == NULL) { return 0; }
    if (node->left == NULL && node->right == NULL) { return 1; }
    return count_leaves_rec(node->left) + count_leaves_rec(node->right);
}

/** Counts the number of leaves in the Tree */
int count_leaves(RBTree* tree) {
    return count_leaves_rec(tree->root);
}

int span(RBTree* tree, int dir) {
    RBNode* node = tree->root;
    int span = 0;
    
    while (node != NULL && node->children[dir] != NULL) {
        span += 1;
        node = node->children[dir];
    }
    
    return span;
}

void print_tree_diagnostics(RBTree tree) {
    printf("Count: %d. Max depth: %d. Leaves: %d. Span: (L=%d,R=%d).\n", tree.items, max_depth(&tree), count_leaves(&tree), span(&tree, 0), span(&tree, 1));
}

/** Insert a value to the red-black tree */
RBTree* insert(RBTree* tree, int value) {
    RBNode* node = tree->root;
    RBNode* parent = NULL;
    RBNode** link_ptr = &tree->root;

    while (node != NULL) {
        link_ptr = node->value >= value ? &node->left : &node->right;
        parent = node;
        node = *link_ptr;
    }

    *link_ptr = allocate_node(parent, value, RED, NULL, NULL);
    node = *link_ptr;
    tree->items += 1;

    printf("Before rebalancing --> ");
    print_tree_diagnostics(*tree);
    return rebalance_tree(tree, node);
}

int main() {
    int items[] = { 
        10, 5, 2, 13, 11, 19, 7, 4, 3, 1,
        -8, 0, 12, 14, 18, 100, 55, 48, 33, 67,
        34, 88, 92, 101, 102, 103, 104, 105, 106, 107,
        108, 109, 110, 111, 112, 113, 114, 115, 116, -100,
        -150, -125, -186, -108, -114, -160, -190
    };

    int checks[] = { 19, -7, 5, 12, 14 };
    
    const int num_items = sizeof(items) / sizeof(int);
    const int num_checks = sizeof(checks) / sizeof(int);
    
    RBTree tree = new_tree();
    for (int i = 0; i < num_items; i++) {
        printf("Inserting: %d.\n", items[i]);
        insert(&tree, items[i]);
        printf("After rebalancing --> ");
        print_tree_diagnostics(tree);
        printf("\n");
    }
    
    for (int i = 0; i < num_checks; i++) {
        printf("Is %d in the Tree? %s.\n", checks[i], has_value(&tree, checks[i]) ? "Yes" : "No");
    }
}