#include "FileIO.h"
#include "Differentiator.h"
#include "CustomAssert.h"
#include "Logger.h"
#include "OperationsMacros.h"

static double EvalInternal (Differentiator *differentiator, const Tree::Node <DifferentiatorNode> *rootNode);

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
            if (rootNode->nodeData.value.variableIndex >= differentiator->nameTable.currentIndex) {
                WriteError (differentiator, NAME_TABLE_ERROR);
                RETURN NAN;
            }

            RETURN differentiator->nameTable.data [rootNode->nodeData.value.variableIndex].value;

        default:
            WriteError (differentiator, WRONG_OPERATION);
            RETURN NAN;
    }
    
    RETURN NAN;
}


