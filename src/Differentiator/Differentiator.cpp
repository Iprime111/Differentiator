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
        case NUMBER_NODE:
            RETURN rootNode->nodeData.value.numericValue;
        case OPERATION_NODE:
            #define OPERATOR(NAME, DESIGNATION, ...)                \
                if (rootNode->nodeData.value.operation == NAME) {   \
                    __VA_ARGS__                                     \
                }
            
            #include "DifferentiatorOperations.def"

            #undef OPERATOR

            WriteError (differentiator, WRONG_OPERATION);
            RETURN NAN;
            break;

        case VARIABLE_NODE:
            //FIXME: do a name table lookup 
            RETURN NO_DIFFERENTIATOR_ERRORS;
    }
    
    RETURN NAN;
}


