#include "Differentiator.h"

#define OPERATOR(NAME, DESIGNATION, PRIORITY, ...) {.name = NAME, .designation = DESIGNATION, .priority = PRIORITY},

static const OperationData operations [] = {
    #include "DifferentiatorOperations.def"
};

#undef OPERATOR

#define FindOperation(...)                                                                      \
    do {                                                                                        \
        for (size_t operationIndex = 0; operationIndex < OPERATIONS_COUNT; operationIndex++) {  \
            if (__VA_ARGS__) {                                                                  \
                return operations + operationIndex;                                             \
            }                                                                                   \
        }                                                                                       \
        return NULL;                                                                            \
    } while (0)

const OperationData *findOperationByName (const Operation name) {
    FindOperation (operations [operationIndex].name == name);
}

const OperationData *findOperationByDesignation (const char *designation) {
    FindOperation(strcmp (operations [operationIndex].designation, designation) == 0); 
}
