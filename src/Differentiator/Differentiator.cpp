#include "stdio.h"

#include "Buffer.h"
#include "FileIO.h"
#include "Differentiator.h"
#include "CustomAssert.h"
#include "Logger.h"
#include "OperationsDSL.h"
#include "TreeDefinitions.h"
#include "TreeOptimizations.h"
#include "DifferentiatorIO.h"

static double EvalInternal (Differentiator *differentiator, const Tree::Node <DifferentiatorNode> *rootNode);

static Tree::Node <DifferentiatorNode> *DifferentiateInternal (Differentiator *differentiator, Differentiator *newDifferentiator, size_t variableIndex, Tree::Node <DifferentiatorNode> *rootNode,
        FILE *stream, Buffer <Tree::Node <DifferentiatorNode> *> *reassignmentsBuffer);



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
        case OPERATION_NODE: {
            double evalValue = NAN;

            #define OPERATOR(NAME, DESIGNATION, PRIORITY, EVAL_CALLBACK, ...)   \
                if (rootNode->nodeData.value.operation == NAME) {               \
                    EVAL_CALLBACK                                               \
                }
            
            #include "DifferentiatorOperations.def"

            #undef OPERATOR

            RETURN evalValue;
        }

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

DifferentiatorError Differentiate (Differentiator *differentiator, Differentiator *newDifferentiator, size_t variableIndex, FILE *stream, Buffer <Tree::Node <DifferentiatorNode> *> *reassignmentsBuffer) {
    PushLog (2);

    custom_assert (differentiator, pointer_is_null, DIFFERENTIATOR_NULL_POINTER);
    VerificationInternal_ (differentiator);

    InitDifferentiator (newDifferentiator, differentiator->nameTable);
    Tree::DestroySingleNode (newDifferentiator->expressionTree.root);

    newDifferentiator->expressionTree.root = DifferentiateInternal (differentiator, newDifferentiator, variableIndex, differentiator->expressionTree.root,
            stream, reassignmentsBuffer);

    RETURN NO_DIFFERENTIATOR_ERRORS;
}

static Tree::Node <DifferentiatorNode> *DifferentiateInternal (Differentiator *differentiator, Differentiator *newDifferentiator, 
        size_t variableIndex, Tree::Node <DifferentiatorNode> *rootNode, FILE *stream, Buffer <Tree::Node <DifferentiatorNode> *> *reassignmentsBuffer) {
    PushLog (2);

    if (!rootNode) {
        RETURN NULL;
    }

    if (rootNode->nodeData.type == NUMERIC_NODE || (rootNode->nodeData.type == VARIABLE_NODE && rootNode->nodeData.value.variableIndex != variableIndex)) {
        RETURN Const (0);
    }

    if (rootNode->nodeData.type == VARIABLE_NODE && rootNode->nodeData.value.variableIndex == variableIndex) {
        RETURN Const (1);
    }

    Tree::Node <DifferentiatorNode> *currentNode = NULL;

    #define SetParent(direction)                                                                            \
        do {                                                                                                \
            if (currentNode->direction) {                                                                   \
                currentNode->direction->parent = currentNode;                                               \
            }                                                                                               \
        } while (0)

    #define OPERATOR(NAME, DESIGNATION, PRIORITY, EVAL_CALLBACK, LATEX_CALLBACK, DIFF_CALLBACK, ...)                                    \
        if (rootNode->nodeData.value.operation == NAME) {                                                                               \
            DIFF_CALLBACK                                                                                                               \
            SetParent (left);                                                                                                           \
            SetParent (right);                                                                                                          \
            if (rootNode->parent) {                                                                                                     \
                GetReassignments (newDifferentiator, reassignmentsBuffer, currentNode);                                                                    \
                PrintPhrase (stream);                                                                                                   \
                fprintf (stream, "$$(");                                                                                                \
                WriteExpressionToStream (differentiator,    stream, rootNode,    WriteNodeContentToLatex, reassignmentsBuffer, false);  \
                fprintf (stream, ")^{'} = ");                                                                                           \
                WriteExpressionToStream (newDifferentiator, stream, currentNode, WriteNodeContentToLatex, reassignmentsBuffer, false);  \
                fprintf (stream, "$$\n");                                                                                               \
            }                                                                                                                           \
            RETURN currentNode;                                                                                                         \
        }

    #include "DifferentiatorOperations.def"

    #undef OPERATOR

    RETURN NULL;
}

DifferentiatorError OptimizeTree (Differentiator *differentiator, Buffer <Tree::Node <DifferentiatorNode> *> *reassignmentsBuffer) {
    PushLog (4);

    custom_assert (differentiator, pointer_is_null, DIFFERENTIATOR_NULL_POINTER);
    VerificationInternal_ (differentiator);

    OptimizationStatus status = OptimizationStatus::TREE_CHANGED;

    while (status == OptimizationStatus::TREE_CHANGED) {
        status = OptimizationStatus::TREE_NOT_CHANGED;

        status = (OptimizationStatus) ((int) status | (int) ComputeSubtree          (differentiator, differentiator->expressionTree.root->left, reassignmentsBuffer));
        status = (OptimizationStatus) ((int) status | (int) CollapseNeutralElements (differentiator, differentiator->expressionTree.root->left, reassignmentsBuffer));
    }
    
    RETURN NO_DIFFERENTIATOR_ERRORS;
}

size_t GetReassignments (Differentiator *differentiator, Buffer <Tree::Node <DifferentiatorNode> *> *reassignmentsBuffer, Tree::Node <DifferentiatorNode> *root) {
    PushLog (3);

    if (!root) {
        RETURN 0;
    }

    size_t depth = 1;

    if (root->nodeData.type == NUMERIC_NODE || root->nodeData.type == VARIABLE_NODE || root->nodeData.depth == 1) {
        RETURN 1;
    }

    if (root->left)
        depth += GetReassignments (differentiator, reassignmentsBuffer, root->left);

    if (root->right)
        depth += GetReassignments (differentiator, reassignmentsBuffer, root->right);

    if (depth > MAX_DEPTH) {
        WriteDataToBuffer (reassignmentsBuffer, &root, 1);
        depth = 1;
    }

    root->nodeData.depth = depth;
    RETURN depth;
}

