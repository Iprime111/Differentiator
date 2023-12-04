#include "TreeOptimizations.h"
#include "CustomAssert.h"
#include "Differentiator.h"
#include "Logger.h"
#include "OperationsDSL.h"
#include "TreeDefinitions.h"

#undef  EvalSubtree
#define EvalSubtree(subtree) ComputeSubtreeInternal (differentiator, rootNode->subtree, status)

static double ComputeSubtreeInternal (Differentiator *differentiator, Tree::Node<DifferentiatorNode> *rootNode, OptimizationStatus *status);

static OptimizationStatus CollapseNeutralElementsInternal (Differentiator *differentiator, Tree::Node <DifferentiatorNode> *rootNode);
static Tree::TreeEdge     GetChildNeutralElementEdge      (const NeutralElement *element, Tree::Node <DifferentiatorNode> *node);
static OptimizationStatus ProcessNeutralElements          (Differentiator *differentiator, NeutralElement *elements,      Tree::Node <DifferentiatorNode> *node, size_t neutralElementsCount);
static OptimizationStatus ProcessNeutralElement           (Differentiator *differentiator, const NeutralElement *element, Tree::Node <DifferentiatorNode> *node, Tree::Node <DifferentiatorNode> *nextNode);

OptimizationStatus ComputeSubtree (Differentiator *differentiator, Tree::Node <DifferentiatorNode> *rootNode) {
    PushLog (3);

    custom_assert (differentiator, pointer_is_null, OptimizationStatus::OPTIMIZATION_ERROR);
    custom_assert (rootNode,       pointer_is_null, OptimizationStatus::OPTIMIZATION_ERROR);

    OptimizationStatus status = OptimizationStatus::TREE_NOT_CHANGED;
    ComputeSubtreeInternal (differentiator, rootNode, &status);

    RETURN status;
}

static double ComputeSubtreeInternal (Differentiator *differentiator, Tree::Node<DifferentiatorNode> *rootNode, OptimizationStatus *status) {
    PushLog (3);

    if (!rootNode) {
        *status = OptimizationStatus::OPTIMIZATION_ERROR;
        RETURN NAN;
    }

    #define OPERATOR(NAME, DESIGNATION, PRIORITY, EVAL_CALLBACK, LATEX_CALLBACK, DIFF_CALLBACK, ...)    \
        if (rootNode->nodeData.value.operation == NAME) {                                               \
            EVAL_CALLBACK                                                                               \
        }                                                                                               

    switch (rootNode->nodeData.type) {
        case NUMERIC_NODE: {
            RETURN rootNode->nodeData.value.numericValue;
        }

        case OPERATION_NODE: {
            double evalValue = NAN;

            #include "DifferentiatorOperations.def"

            if (isnan (evalValue)) {
                RETURN NAN;
            }

            if (rootNode == rootNode->parent->left) {
                rootNode->parent->left  = Const (evalValue);
            } else if (rootNode == rootNode->parent->right) {
                rootNode->parent->right = Const (evalValue);
            }

            Tree::DestroySubtreeNode (&differentiator->expressionTree, rootNode);

            *status = OptimizationStatus::TREE_CHANGED;
            RETURN evalValue;
        }

        case VARIABLE_NODE:
        default: {
            RETURN NAN;
        }
    }

    #undef OPERATOR
}

OptimizationStatus CollapseNeutralElements (Differentiator *differentiator, Tree::Node <DifferentiatorNode> *rootNode) {
    PushLog (3);

    custom_assert (differentiator, pointer_is_null, OptimizationStatus::OPTIMIZATION_ERROR);
    custom_assert (rootNode,       pointer_is_null, OptimizationStatus::OPTIMIZATION_ERROR);

    RETURN CollapseNeutralElementsInternal (differentiator, rootNode);
}

static OptimizationStatus CollapseNeutralElementsInternal (Differentiator *differentiator, Tree::Node <DifferentiatorNode> *rootNode) {
    PushLog (3);

    if (!rootNode) {
        RETURN OptimizationStatus::OPTIMIZATION_ERROR;
    }

    if (rootNode->nodeData.type != OPERATION_NODE) {
        RETURN OptimizationStatus::TREE_NOT_CHANGED;
    }

    OptimizationStatus status = OptimizationStatus::TREE_NOT_CHANGED;

    #define CollapseChild(direction)                                                                                                    \
        if (rootNode->direction) {                                                                                                      \
            status = (OptimizationStatus) ((int) status | (int) CollapseNeutralElementsInternal (differentiator, rootNode->direction)); \
        }

    #define CollapseChildren()      \
        do {                        \
            CollapseChild (left);   \
            CollapseChild (right);  \
        } while (0)

    #define OPERATOR(NAME, DESIGNATION, PRIORITY, EVAL_CALLBACK, LATEX_CALLBACK, DIFF_CALLBACK, ...)                                                \
        if (rootNode->nodeData.value.operation == NAME) {                                                                                           \
            NeutralElement elements [] = __VA_ARGS__;                                                                                               \
            size_t neutralElementsCount = sizeof (elements) / sizeof (NeutralElement);                                                              \
            if (neutralElementsCount == 0) {                                                                                                        \
                CollapseChildren ();                                                                                                                \
                RETURN status;                                                                                                                      \
            }                                                                                                                                       \
            CollapseChildren ();                                                                                                                    \
            status = (OptimizationStatus) ((int) status | (int) ProcessNeutralElements (differentiator, elements, rootNode, neutralElementsCount)); \
            RETURN status;                                                                                                                          \
        }

    #include "DifferentiatorOperations.def"

    #undef OPERATOR

    RETURN OptimizationStatus::TREE_NOT_CHANGED;

}

static OptimizationStatus ProcessNeutralElements (Differentiator *differentiator, NeutralElement *elements, Tree::Node <DifferentiatorNode> *node, size_t neutralElementsCount) {
     PushLog (3);

    for (size_t neutralElement = 0; neutralElement < neutralElementsCount; neutralElement++) {
        Tree::TreeEdge neutralElementEdge = GetChildNeutralElementEdge (&elements [neutralElement], node);

        if (neutralElementEdge == Tree::NO_EDGE)
            continue;

        Tree::TreeEdge nonNeutralEdge = (Tree::TreeEdge) ((Tree::LEFT_CHILD | Tree::RIGHT_CHILD) ^ neutralElementEdge);

        Tree::Node <DifferentiatorNode> *nonNeutralNode = Tree::NextNode (node, nonNeutralEdge);

        RETURN ProcessNeutralElement (differentiator, &elements [neutralElement], node, nonNeutralNode);
    }

    RETURN OptimizationStatus::TREE_NOT_CHANGED;

}

static OptimizationStatus ProcessNeutralElement (Differentiator *differentiator, const NeutralElement *element, Tree::Node <DifferentiatorNode> *node, Tree::Node <DifferentiatorNode> *nextNode) {

    Tree::Node <DifferentiatorNode> **nextNodePointer = NULL;
    Tree::Node <DifferentiatorNode> **nodePointer     = NULL;

    #define CheckChild(currentNode, childNode, direction, variable) \
        if (childNode == currentNode->direction) {                  \
            variable = &(currentNode->direction);                   \
        }

    #define FindChild(currentNode, childNode, variable)             \
        do {                                                        \
            CheckChild (currentNode, childNode, left,  variable);   \
            CheckChild (currentNode, childNode, right, variable);   \
        } while (0)

    FindChild (node, nextNode,     nextNodePointer);
    FindChild (node->parent, node, nodePointer);

    if (element->resultingNode == CollapseResult::COLLAPSED_TO_CONSTANT) {
        *nodePointer = Const (element->collapsedValue);

        Tree::DestroySubtreeNode (&differentiator->expressionTree, node);
    }

    if (element->resultingNode == CollapseResult::EXPRESSION_NOT_CHANGED) {
        *nodePointer     = nextNode;
        *nextNodePointer = NULL;
        nextNode->parent = node->parent;
        
        Tree::DestroySubtreeNode (&differentiator->expressionTree, node);
    }

    #undef CheckChild
    #undef FindChild

    return OptimizationStatus::TREE_CHANGED;
}

static Tree::TreeEdge GetChildNeutralElementEdge (const NeutralElement *element, Tree::Node <DifferentiatorNode> *node) {
        
    #define CheckForChild(child, edge)                                                                                                      \
        do {                                                                                                                                \
            if ((element->validChild & edge) && node->child) {                                                                              \
                if (node->child->nodeData.type == NUMERIC_NODE && abs (node->child->nodeData.value.numericValue - element->value) < EPS) {  \
                    return edge;                                                                                                            \
                }                                                                                                                           \
            }                                                                                                                               \
        } while (0)

    CheckForChild (left,  Tree::LEFT_CHILD);
    CheckForChild (right, Tree::RIGHT_CHILD);

    return Tree::NO_EDGE;
}
