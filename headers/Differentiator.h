#ifndef DIFFERENTIATOR_H_
#define DIFFERENTIATOR_H_

#include <stddef.h>
#include <math.h>

#include "Tree.h"

enum DifferentiatorError {
    NO_DIFFERENTIATOR_ERRORS    = 0,
    DIFFERENTIATOR_NULL_POINTER = 1,
    TREE_ERROR                  = 2,
    NODE_NULL_POINTER           = 3,
};

enum NodeType {
    NUMBER_NODE    = 0,
    OPERATION_NODE = 1,
    VARIABLE_NODE  = 2,
};

enum Operation {
    ADD   = 0,
    SUB   = 1,
    MUL   = 2,
    DIV   = 3,
    SQRT  = 4,
    POWER = 5,
};

union NodeValue {
    double    numericValue  = NAN;
    size_t    variableIndex;
    Operation operation;
};

struct DifferentiatorNode {
    NodeType  type  = NUMBER_NODE;
    NodeValue value = {.numericValue = NAN}; 
};

struct Differentiator {
    Tree::Tree <DifferentiatorNode> expressionTree = {};

    DifferentiatorError errors = NO_DIFFERENTIATOR_ERRORS;
};

DifferentiatorError InitDifferentiator    (Differentiator *differentiator);
DifferentiatorError DestroyDifferentiator (Differentiator *differentiator);
DifferentiatorError VerifyDifferentiator  (Differentiator *differentiator); 
    
#define WriteError(differentiator, error)  (differentiator)->errors = (DifferentiatorError) (error | (differentiator)->errors)
#define ReturnError(differentiator, error)          \
    do {                                            \
        if (error != NO_DIFFERENTIATOR_ERRORS) {    \
            WriteError (differentiator, error);     \
            RETURN error;                           \
        }                                           \
    } while (0)

#define VerificationInternal_(differentiator)                               \
    do {                                                                    \
        DifferentiatorError error_ = VerifyDifferentiator (differentiator); \
        if (error_ != NO_DIFFERENTIATOR_ERRORS) {                           \
            RETURN error_;                                                  \
        }                                                                   \
    } while (0)

#endif
