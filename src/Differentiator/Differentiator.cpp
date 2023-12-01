#include "Buffer.h"
#include "FileIO.h"
#include "Differentiator.h"
#include "CustomAssert.h"
#include "Logger.h"
#include "OperationsDSL.h"
#include "TreeDefinitions.h"

static double EvalInternal (Differentiator *differentiator, const Tree::Node <DifferentiatorNode> *rootNode);
static Tree::Node <DifferentiatorNode> *DifferentiateInternal (Differentiator *differentiator, size_t variableIndex, Tree::Node <DifferentiatorNode> *rootNode);
static DifferentiatorError SetParentConnections (Differentiator *differentiator, Tree::Node <DifferentiatorNode> *rootNode);

DifferentiatorError EvalTree (Differentiator *differentiator, double *value) {
    PushLog (1);

    custom_assert (value, pointer_is_null, DIFFERENTIATOR_NULL_POINTER);
    VerificationInternal_ (differentiator);

    *value = EvalInternal (differentiator, differentiator->expressionTree.root);

    RETURN differentiator->errors;
}

static double EvalInternal (Differentiator *differentiator, const Tree::Node <DifferentiatorNode> *rootNode) {
    PushLog (2);

    switch (rootNode->nodeData.type) {
        case NUMERIC_NODE:
            RETURN rootNode->nodeData.value.numericValue;
        case OPERATION_NODE:
            #define OPERATOR(NAME, DESIGNATION, PRIORITY, EVAL_CALLBACK, ...)   \
                if (rootNode->nodeData.value.operation == NAME) {               \
                    EVAL_CALLBACK                                               \
                }
            
            #include "DifferentiatorOperations.def"

            #undef OPERATOR

            WriteError (differentiator, WRONG_OPERATION);
            RETURN NAN;

        case VARIABLE_NODE:
            if (rootNode->nodeData.value.variableIndex >= differentiator->nameTable->currentIndex) {
                WriteError (differentiator, NAME_TABLE_ERROR);
                RETURN NAN;
            }

            RETURN differentiator->nameTable->data [rootNode->nodeData.value.variableIndex].value;

        default:
            WriteError (differentiator, WRONG_OPERATION);
            RETURN NAN;
    }
    
    RETURN NAN;
}

DifferentiatorError Differentiate (Differentiator *differentiator, Differentiator *newDifferentiator, size_t variableIndex) {
    PushLog (2);

    custom_assert (differentiator, pointer_is_null, DIFFERENTIATOR_NULL_POINTER);
    VerificationInternal_ (differentiator);

    InitDifferentiator (newDifferentiator, differentiator->nameTable);
    Tree::DestroySingleNode (newDifferentiator->expressionTree.root);

    newDifferentiator->expressionTree.root = DifferentiateInternal (newDifferentiator, variableIndex, differentiator->expressionTree.root);

    SetParentConnections (newDifferentiator, newDifferentiator->expressionTree.root);

    RETURN NO_DIFFERENTIATOR_ERRORS;
}

static Tree::Node <DifferentiatorNode> *DifferentiateInternal (Differentiator *newDifferentiator, size_t variableIndex, Tree::Node <DifferentiatorNode> *rootNode) {
    PushLog (2);

    if (rootNode->nodeData.type == NUMERIC_NODE || (rootNode->nodeData.type == VARIABLE_NODE && rootNode->nodeData.value.variableIndex != variableIndex)) {
        RETURN Const (0);
    }

    if (rootNode->nodeData.type == VARIABLE_NODE && rootNode->nodeData.value.variableIndex == variableIndex) {
        RETURN Const (1);
    }

    #define OPERATOR(NAME, DESIGNATION, PRIORITY, EVAL_CALLBACK, LATEX_CALLBACK, DIFF_CALLBACK, ...)    \
        if (rootNode->nodeData.value.operation == NAME) {                                               \
            DIFF_CALLBACK                                                                               \
        }

    #include "DifferentiatorOperations.def"

    #undef OPERATOR

    RETURN NULL;
}

static DifferentiatorError SetParentConnections (Differentiator *differentiator, Tree::Node <DifferentiatorNode> *rootNode) {
    PushLog (3);

    custom_assert (differentiator, pointer_is_null, DIFFERENTIATOR_NULL_POINTER);
    custom_assert (rootNode,       pointer_is_null, NODE_NULL_POINTER);

    #define SetForChild(direction)                                          \
        do {                                                                \
            if (rootNode->direction) {                                      \
                rootNode->direction->parent = rootNode;                     \
                SetParentConnections (differentiator, rootNode->direction); \
            }                                                               \
        } while (0)

    SetForChild (left);
    SetForChild (right);

    RETURN NO_DIFFERENTIATOR_ERRORS;
}

DifferentiatorError OptimizeTree (Differentiator *differentiator) {

}

static DifferentiatorError OptimizeSubtree (Differentiator *differentiator, Tree::Node <DifferentiatorNode> *rootNode) {
    PushLog (3);

    // Use tree optimizations
    
    RETURN NO_DIFFERENTIATOR_ERRORS;
}
