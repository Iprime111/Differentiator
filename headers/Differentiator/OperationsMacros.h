#ifndef OPERATIONS_MACROS_H_
#define OPERATIONS_MACROS_H_

#define WriteWithErrorCheck(buffer, data)                                           \
    if (WriteStringToBuffer (buffer, data) != BufferErrorCode::NO_BUFFER_ERRORS) {  \
        RETURN OUTPUT_FILE_ERROR;                                                   \
    }

#define EvalSubtree(subtree) EvalInternal (differentiator, rootNode->subtree)
#define WriteExpression(branch, emitter)                                                        \
    do {                                                                                        \
        if (rootNode->branch) {                                                                 \
            WriteExpressionInternal (differentiator, rootNode->branch, printBuffer, emitter);   \
        } else {                                                                                \
            RETURN TREE_ERROR;                                                                  \
        }                                                                                       \
    } while (0)

#define WriteUnaryOperation(designator)                     \
    do {                                                    \
        WriteWithErrorCheck (printBuffer, designator);      \
        WriteExpression (left, WriteNodeContentToLatex);    \
    } while (0)

#define WriteBinaryOperation(designator)                    \
    do {                                                    \
        WriteExpression (left,  WriteNodeContentToLatex);   \
        WriteWithErrorCheck (printBuffer, designator);      \
        WriteExpression (right, WriteNodeContentToLatex);   \
    } while (0)


#endif
