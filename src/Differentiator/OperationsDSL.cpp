#include "OperationsDSL.h"
#include "Differentiator.h"
#include "TreeDefinitions.h"


Tree::Node <DifferentiatorNode> *CopySubtree (Tree::Node <DifferentiatorNode> *subtreeRoot) {

    Tree::Node <DifferentiatorNode> *rootCopy = {};
    Tree::InitNode (&rootCopy);

    rootCopy->nodeData = subtreeRoot->nodeData;

    #define CopyRootSubtree(direction)                                      \
        if (subtreeRoot->direction) {                                       \
            rootCopy->direction = CopySubtree (subtreeRoot->direction);     \
            rootCopy->direction->parent = rootCopy;                         \
        }

    CopyRootSubtree (left);
    CopyRootSubtree (right);

    #undef CopyRootSubtree

    return rootCopy;
}

Tree::Node <DifferentiatorNode> *CreateNode  (Tree::Node <DifferentiatorNode> node) {
    Tree::Node <DifferentiatorNode> *newNode = NULL;

    if (Tree::InitNode (&newNode) != Tree::NO_TREE_ERRORS) {
        return NULL;
    }

    newNode->nodeData = node.nodeData;
    newNode->right    = node.right;
    newNode->left     = node.left;
    newNode->parent   = node.parent;

    if (newNode->left)
        newNode->left->parent = newNode;

    if (newNode->right)
        newNode->right->parent = newNode;

    return newNode;
}
