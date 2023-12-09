#include <cstdio>
#include <cstdlib>
#include <string.h>

#include "Differentiator.h"
#include "FileIO.h"
#include "Logger.h"
#include "RecursiveDescentParser.h"
#include "OperationsDSL.h"
#include "Buffer.h"
#include "TreeDefinitions.h"

#define SyntaxAssert(EXPRESSION, ERROR)                                         \
    if (!(EXPRESSION)) {                                                        \
        context->error = (ParsingError) ((int) context->error | (int) ERROR);   \
        RETURN NULL;                                                            \
    }

static Tree::Node <DifferentiatorNode> *GetGrammar         (ParsingContext *context, Differentiator *differentiator);
static Tree::Node <DifferentiatorNode> *GetNumber          (ParsingContext *context, Differentiator *differentiator);
static Tree::Node <DifferentiatorNode> *GetBinaryOperation (ParsingContext *context, Differentiator *differentiator, size_t priority);
static Tree::Node <DifferentiatorNode> *GetUnaryOperation  (ParsingContext *context, Differentiator *differentiator, size_t priority);
static Tree::Node <DifferentiatorNode> *GetSeparator       (ParsingContext *context, Differentiator *differentiator, size_t priority);

static const getter_t NextFunction [] = {GetSeparator, GetBinaryOperation, GetUnaryOperation, GetBinaryOperation, GetBinaryOperation};

DifferentiatorError ParseFile (Differentiator *differentiator, FILE *stream, Buffer <Tree::Node <DifferentiatorNode> *> *reassignmentsBuffer) {
    PushLog (2);

    FileBuffer fileContent = {};

    size_t bufferSize = 0;
    getline (&fileContent.buffer, &bufferSize, stream);
    fileContent.buffer_size = (ssize_t) strlen (fileContent.buffer);

    ParsingContext context = {.error = ParsingError::NO_PARSING_ERRORS};

    if (InitBuffer (&context.tokens) != BufferErrorCode::NO_BUFFER_ERRORS) {
        DestroyFileBuffer (&fileContent);
        RETURN INPUT_FILE_ERROR;
    }

    TreeLexer (&context, differentiator, &fileContent);

    differentiator->expressionTree.root->left = GetGrammar (&context, differentiator);
    GetReassignments (differentiator, reassignmentsBuffer, differentiator->expressionTree.root->left);

    DestroyFileBuffer (&fileContent); 
   
    DifferentiatorError error = NO_DIFFERENTIATOR_ERRORS;

    if (differentiator->expressionTree.root->left == NULL) {
        for (size_t tokenIndex = 0; tokenIndex < context.tokens.currentIndex; tokenIndex++) {
            Tree::DestroySingleNode (context.tokens.data [tokenIndex]);
        }

        error = INPUT_FILE_ERROR;
    }
    
    DestroyBuffer (&context.tokens);
    RETURN error;
}

static Tree::Node <DifferentiatorNode> *GetGrammar (ParsingContext *context, Differentiator *differentiator) {
    PushLog (3);

    if (context->error != ParsingError::NO_PARSING_ERRORS) {
        RETURN NULL;
    }

    context->currentNode = 0;

    Tree::Node <DifferentiatorNode> *treeRoot = NextFunction [MAX_PRIORITY] (context, differentiator, MAX_PRIORITY);

    if (context->tokens.data [context->currentNode]->nodeData.type == OPERATION_NODE && context->tokens.data [context->currentNode]->nodeData.value.operation == TERMINATOR) {
        context->error = ParsingError::NO_PARSING_ERRORS;
        Tree::DestroySingleNode (context->tokens.data [context->currentNode]);
        context->tokens.data [context->currentNode] = NULL;
        RETURN treeRoot;
    }

    RETURN NULL;
}

static Tree::Node <DifferentiatorNode> *GetNumber (ParsingContext *context, Differentiator *differentiator) {
    PushLog (4);

    SyntaxAssert (context->tokens.data [context->currentNode]->nodeData.type & (NUMERIC_NODE | VARIABLE_NODE), ParsingError::NUMBER_EXPECTED);

    context->currentNode++;
    return context->tokens.data [context->currentNode - 1];
}

static Tree::Node <DifferentiatorNode> *GetUnaryOperation (ParsingContext *context, Differentiator *differentiator, size_t priority) {
    PushLog (3);

    Tree::Node <DifferentiatorNode> *value = NextFunction [priority - 1] (context, differentiator, priority - 1);

    if (value) {
        RETURN value;
    }
    
    context->error = ParsingError::NO_PARSING_ERRORS;
    const OperationData *operation = FindOperationByName (context->tokens.data [context->currentNode]->nodeData.value.operation);

    if (context->tokens.data [context->currentNode]->nodeData.type == OPERATION_NODE) {
        SyntaxAssert (context->tokens.data [context->currentNode]->nodeData.value.operation != CLOSE_BRACKET && 
                      context->tokens.data [context->currentNode]->nodeData.value.operation != TERMINATOR    &&
                      context->tokens.data [context->currentNode]->nodeData.value.operation != OPEN_BRACKET, ParsingError::UNARY_EXPRESSION_EXPECTED);

        SyntaxAssert (operation->priority == priority, ParsingError::UNARY_EXPRESSION_EXPECTED);
    }

    Tree::Node <DifferentiatorNode> *operationNode = context->tokens.data [context->currentNode];

    context->currentNode++;

    operationNode->left         = NextFunction [priority - 1] (context, differentiator, priority - 1);
    SyntaxAssert (operationNode->left, ParsingError::NUMBER_EXPECTED);
    operationNode->left->parent = operationNode;

    operationNode->nodeData.depth = operationNode->left->nodeData.depth;

    RETURN operationNode; 
}

static Tree::Node <DifferentiatorNode> *GetBinaryOperation (ParsingContext *context, Differentiator *differentiator, size_t priority) {
    PushLog (3);

    Tree::Node <DifferentiatorNode> *value = NextFunction [priority - 1] (context, differentiator, priority - 1);

    SyntaxAssert (value, context->error);

    while (context->tokens.data [context->currentNode]->nodeData.type == OPERATION_NODE) {
        const OperationData *operation = FindOperationByName (context->tokens.data [context->currentNode]->nodeData.value.operation); 

        SyntaxAssert (context->tokens.data [context->currentNode]->nodeData.value.operation != OPEN_BRACKET,  ParsingError::BINARY_OPERATION_EXPECTED);

        if (context->tokens.data [context->currentNode]->nodeData.value.operation == TERMINATOR || context->tokens.data [context->currentNode]->nodeData.value.operation == CLOSE_BRACKET || operation->priority != priority) {
            RETURN value;
        }

        Tree::Node <DifferentiatorNode> *operationNode = context->tokens.data [context->currentNode];
        context->currentNode++;

        operationNode->left = value;
        value->parent       = operationNode; 

        Tree::Node <DifferentiatorNode> *value2 = NextFunction [priority - 1] (context, differentiator, priority - 1);
        SyntaxAssert (value2, context->error);

        operationNode->right = value2;
        value2->parent       = operationNode;

        operationNode->nodeData.depth = value2->nodeData.depth + value->nodeData.depth;

        value = operationNode;
    }

    RETURN value;
}
static Tree::Node <DifferentiatorNode> *GetSeparator (ParsingContext *context, Differentiator *differentiator, size_t priority) {
    PushLog (3);

    Tree::Node <DifferentiatorNode> *value = GetNumber (context, differentiator);

    if (value) {
        RETURN value;
    }

    context->error = ParsingError::NO_PARSING_ERRORS;

    if (context->tokens.data [context->currentNode]->nodeData.type == OPERATION_NODE && context->tokens.data [context->currentNode]->nodeData.value.operation == OPEN_BRACKET) {
        Tree::DestroySingleNode (context->tokens.data [context->currentNode]);
        context->tokens.data [context->currentNode] = NULL;
        context->currentNode++;

        value = NextFunction [MAX_PRIORITY] (context, differentiator, MAX_PRIORITY);
        
        SyntaxAssert (value, context->error);
        SyntaxAssert(context->tokens.data [context->currentNode]->nodeData.type == OPERATION_NODE && context->tokens.data [context->currentNode]->nodeData.value.operation == CLOSE_BRACKET, ParsingError::BRACKET_EXPECTED);
        Tree::DestroySingleNode (context->tokens.data [context->currentNode]);
        context->tokens.data [context->currentNode] = NULL;
        context->currentNode++;

        RETURN value;
    }

    SyntaxAssert (false, ParsingError::NUMBER_EXPECTED);
}

