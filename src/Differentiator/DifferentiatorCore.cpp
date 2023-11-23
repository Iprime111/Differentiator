#include "CustomAssert.h"
#include "Differentiator.h"
#include "Logger.h"
#include "TreeDefinitions.h"

DifferentiatorError InitDifferentiator (Differentiator *differentiator) {
    PushLog (3);

    custom_assert (differentiator, pointer_is_null, DIFFERENTIATOR_NULL_POINTER);

    differentiator->errors = NO_DIFFERENTIATOR_ERRORS;
    
    if (Tree::InitTree (&differentiator->expressionTree) != Tree::NO_TREE_ERRORS) {
        ReturnError (differentiator, TREE_ERROR);
    }

    VerificationInternal_ (differentiator);

    RETURN NO_DIFFERENTIATOR_ERRORS;
}

DifferentiatorError DestroyDifferentiator (Differentiator *differentiator) {
    PushLog (3);

    if (!differentiator) {
        RETURN NO_DIFFERENTIATOR_ERRORS; 
    }

    if (Tree::DestroyTree (&differentiator->expressionTree) != Tree::NO_TREE_ERRORS) {
        RETURN TREE_ERROR;
    }

    RETURN NO_DIFFERENTIATOR_ERRORS;
}

DifferentiatorError VerifyDifferentiator (Differentiator *differentiator) {
    PushLog (3);

    if (!differentiator) {
        RETURN DIFFERENTIATOR_NULL_POINTER;
    }

    RETURN differentiator->errors;
}

static DifferentiatorError VerifyDifferentiatorInternal (Tree::Node <DifferentiatorNode> *root) {
    PushLog (3);

    RETURN NO_DIFFERENTIATOR_ERRORS;
}
