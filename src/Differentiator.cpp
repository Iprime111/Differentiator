#include "FileIO.h"
#include "Differentiator.h"
#include "CustomAssert.h"

static DifferentiatorError EvalInternal (Tree::Node <DifferentiatorError> rootNode);

DifferentiatorError Eval (Differentiator *differentiator) {
    PushLog (1);

    custom_assert (differentiator, pointer_is_null, DIFFERENTIATOR_NULL_POINTER);

    EvalInternal (differentiator->expressionTree.root);

    RETURN NO_DIFFERENTIATOR_ERRORS;
}

static DifferentiatorError EvalInternal (Tree::Node <DifferentiatorError> rootNode) {

}
