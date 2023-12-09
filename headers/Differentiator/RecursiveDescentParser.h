#ifndef RECURSIVE_DESCENT_PARSER_H_
#define RECURSIVE_DESCENT_PARSER_H_

#include "Differentiator.h"
#include "TextTypes.h"
#include "TreeDefinitions.h"

const size_t MAX_PRIORITY    = 4;
const size_t MAX_WORD_LENGTH = 256;

enum class ParsingError {
    NO_PARSING_ERRORS         = 0,
    NO_CONTEXT                = 1 << 0,
    NUMBER_EXPECTED           = 1 << 1,
    BINARY_OPERATION_EXPECTED = 1 << 2,
    UNARY_EXPRESSION_EXPECTED = 1 << 3,
    BRACKET_EXPECTED          = 1 << 4,
    NAME_TABLE_ERROR          = 1 << 5,
    FILE_ERROR                = 1 << 6,
    TOKEN_ARRAY_ERROR         = 1 << 7,
    WORD_TOO_LONG             = 1 << 8,
};

struct ParsingContext {
    size_t currentNode = 0;
    Buffer <Tree::Node <DifferentiatorNode> *> tokens = {};

    ParsingError error = ParsingError::NO_PARSING_ERRORS;
};

typedef Tree::Node <DifferentiatorNode> *(* getter_t) (ParsingContext *, Differentiator *, size_t);

DifferentiatorError ParseFile (Differentiator *differentiator, FILE *stream, Buffer <Tree::Node <DifferentiatorNode> *> *reassignmentsBuffer);
ParsingError TreeLexer (ParsingContext *context, Differentiator *differentiator, FileBuffer *content);

#endif
