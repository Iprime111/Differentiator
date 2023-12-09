#include "OperationsDSL.h"
#include "Differentiator.h"
#include "TreeDefinitions.h"


Tree::Node <DifferentiatorNode> *CopySubtree (Tree::Node <DifferentiatorNode> *subtreeRoot) {

    Tree::Node <DifferentiatorNode> *rootCopy = {};
    Tree::InitNode (&rootCopy);

    rootCopy->nodeData       = subtreeRoot->nodeData;
    rootCopy->nodeData.depth = subtreeRoot->nodeData.depth;

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

    newNode->nodeData.depth = 1;

    if (newNode->left) {
        newNode->left->parent   = newNode;
        newNode->nodeData.depth += newNode->left->nodeData.depth;
    }

    if (newNode->right) {
        newNode->right->parent  = newNode;
        newNode->nodeData.depth += newNode->right->nodeData.depth;
    }

    return newNode;
}
