#ifndef DIFFERENTIATOR_H_
#define DIFFERENTIATOR_H_

#include <stddef.h>
#include <math.h>

#include "Buffer.h"
#include "Tree.h"

const size_t MAX_NAME_LENGTH = 256;
const double EPS = 1e-5;

#define ON_DEBUG(...) __VA_ARGS__

enum DifferentiatorError {
    NO_DIFFERENTIATOR_ERRORS    = 0,
    DIFFERENTIATOR_NULL_POINTER = 1 << 0,
    TREE_ERROR                  = 1 << 1,
    NODE_NULL_POINTER           = 1 << 2,
    WRONG_OPERATION             = 1 << 3,
    NO_VALUE                    = 1 << 4,
    OUTPUT_FILE_ERROR           = 1 << 5,
    INPUT_FILE_ERROR            = 1 << 6,
    DUMP_ERROR                  = 1 << 7,
    NAME_TABLE_ERROR            = 1 << 8,
    NO_PARSING_CONTEXT          = 1 << 9,
};

enum NodeType {
    NUMERIC_NODE   = 1 << 0,
    OPERATION_NODE = 1 << 1,
    VARIABLE_NODE  = 1 << 2,
};

#define OPERATOR(NAME, DESIGNATION, ...) NAME,

enum Operation {
    TERMINATOR,
    #include "DifferentiatorOperations.def"
};

#undef OPERATOR

#define OPERATOR(...) 1 +

const size_t OPERATIONS_COUNT =
    #include "DifferentiatorOperations.def" 
    0;

#undef OPERATOR

struct OperationData {
    Operation name;
    char     *designation = NULL;
    size_t    priority    = 0;
};

union NodeValue {
    double    numericValue  = NAN;
    size_t    variableIndex;
    Operation operation;
};

struct DifferentiatorNode {
    NodeType  type  = NUMERIC_NODE;
    NodeValue value = {.numericValue = NAN}; 
};

struct NameTableRecord {
    char *name   = NULL;
    double value = NAN;  
};

struct Differentiator {
    Tree::Tree <DifferentiatorNode> expressionTree = {};

    Buffer <NameTableRecord> *nameTable = NULL;

    DifferentiatorError errors = NO_DIFFERENTIATOR_ERRORS;
};

typedef double (* operation_t) (double, double);

DifferentiatorError InitDifferentiator    (Differentiator *differentiator, Buffer <NameTableRecord> *nameTable);
DifferentiatorError DestroyDifferentiator (Differentiator *differentiator);
DifferentiatorError VerifyDifferentiator  (Differentiator *differentiator);
DifferentiatorError InitNameTable    (Buffer <NameTableRecord> *nameTable);
DifferentiatorError DestroyNameTable (Buffer <NameTableRecord> *nameTable);

DifferentiatorError EvalTree      (Differentiator *differentiator, double *value);
DifferentiatorError Differentiate (Differentiator *differentiator, Differentiator *newDifferentiator, size_t variableIndex, FILE *stream);
DifferentiatorError OptimizeTree  (Differentiator *differentiator);

const OperationData *findOperationByName        (const Operation name);
const OperationData *findOperationByDesignation (const char     *designation);

long long CompareNames (void *value1, void *value2);

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
