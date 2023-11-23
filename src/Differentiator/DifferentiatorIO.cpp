#include "CustomAssert.h"
#include "Differentiator.h"
#include "Logger.h"

DifferentiatorError ReadExpression (Differentiator *differentiator, FILE *stream) {
    PushLog (3);
    
    custom_assert (stream,         pointer_is_null, OUTPUT_FILE_ERROR);
    custom_assert (differentiator, pointer_is_null, DIFFERENTIATOR_NULL_POINTER);

    RETURN NO_DIFFERENTIATOR_ERRORS;
}

DifferentiatorError WriteToLatex (Differentiator *differentiator, FILE *stream) {
    PushLog (3);

    custom_assert (stream, pointer_is_null, OUTPUT_FILE_ERROR);
    VerificationInternal_ (differentiator);


    RETURN NO_DIFFERENTIATOR_ERRORS;
}

DifferentiatorError WriteSubtreeToLatex (Differentiator *differentiator, Tree::Node <DifferentiatorNode> rootNode, FILE *stream) {
    PushLog (3);

    custom_assert (stream, pointer_is_null, OUTPUT_FILE_ERROR);
    VerificationInternal_ (differentiator);

    RETURN NO_DIFFERENTIATOR_ERRORS;
}
