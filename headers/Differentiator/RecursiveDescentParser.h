#ifndef RECURSIVE_DESCENT_PARSER_H_
#define RECURSIVE_DESCENT_PARSER_H_

#include "Differentiator.h"
#include "TextTypes.h"
#include "TreeDefinitions.h"

const size_t MAX_PRIORITY = 4;

enum class ParsingError {
    NO_PARSING_ERRORS         = 0,
    NUMBER_EXPECTED           = 1 << 0,
    BINARY_OPERATION_EXPECTED = 1 << 1,
    UNARY_EXPRESSION_EXPECTED = 1 << 2,
    BRACKET_EXPECTED          = 1 << 3,
};

struct ParsingContext {
    Tree::Node <DifferentiatorNode> *currentNode = NULL;
    Buffer <Tree::Node <DifferentiatorNode> *> tokens = {};

    ParsingError error = ParsingError::NO_PARSING_ERRORS;
};

typedef Tree::Node <DifferentiatorNode> *(* getter_t) (ParsingContext *, Differentiator *, size_t);

DifferentiatorError TreeLexer (ParsingContext *context, Differentiator *differentiator, TextBuffer *stringTokens);

DifferentiatorError ParseFile (Differentiator *differentiator, char *filename);

Tree::Node <DifferentiatorNode> *GetGrammar         (ParsingContext *context, Differentiator *differentiator);
Tree::Node <DifferentiatorNode> *GetNumber          (ParsingContext *context, Differentiator *differentiator);
Tree::Node <DifferentiatorNode> *GetBinaryOperation (ParsingContext *context, Differentiator *differentiator, size_t priority);
Tree::Node <DifferentiatorNode> *GetUnaryOperation  (ParsingContext *context, Differentiator *differentiator, size_t priority);
Tree::Node <DifferentiatorNode> *GetSeparator       (ParsingContext *context, Differentiator *differentiator, size_t priority);
#endif
