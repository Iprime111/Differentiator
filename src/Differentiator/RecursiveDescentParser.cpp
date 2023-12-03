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

const getter_t NextFunction [] = {GetSeparator, GetBinaryOperation, GetUnaryOperation, GetBinaryOperation, GetBinaryOperation};

static void RemoveExcessWhitespaces (TextBuffer *fileText);

#define WriteNodePointer(node)                                                                              \
    do {                                                                                                    \
        Tree::Node <DifferentiatorNode> *nodePointer = node;                                                \
        if (WriteDataToBuffer (&context->tokens, &nodePointer, 1) != BufferErrorCode::NO_BUFFER_ERRORS) {   \
            RETURN INPUT_FILE_ERROR;                                                                        \
        }                                                                                                   \
    } while (0)

DifferentiatorError TreeLexer (ParsingContext *context, Differentiator *differentiator, TextBuffer *stringTokens) {
    PushLog (3);

    fprintf (stderr, "Tokens count: %lu\n", stringTokens->line_count);
    for (size_t tokenIndex = 0; tokenIndex < stringTokens->line_count; tokenIndex++) {
        double number = NAN;

        #define ReadBracket(symbol, name)                                                   \
            if (stringTokens->lines [tokenIndex].pointer [0] == symbol) {                   \
                if (strlen (stringTokens->lines [tokenIndex].pointer) != 1)                 \
                    {RETURN INPUT_FILE_ERROR;}                                              \
                WriteNodePointer (OperationNode (NULL, NULL, name));  \
                continue;                                                                   \
            }

        ReadBracket ('(', OPEN_BRACKET);
        ReadBracket (')', CLOSE_BRACKET);

        #undef ReadBracket

        if (sscanf (stringTokens->lines [tokenIndex].pointer, "%lf", &number) > 0) {
            WriteNodePointer (Const (number));
            continue;
        }

        //TODO: use find function

        #define OPERATOR(NAME, DESIGNATION, PRIORITY, ...)                                   \
            if (strcmp (DESIGNATION, stringTokens->lines [tokenIndex].pointer) == 0) {       \
                WriteNodePointer (OperationNode (NULL, NULL, NAME));  \
                continue;                                                                    \
            }
        
        #include "DifferentiatorOperations.def"

        #undef OPERATOR

        NameTableRecord patternRecord = {.name = stringTokens->lines [tokenIndex].pointer};
        NameTableRecord *foundRecord = FindValueInBuffer (differentiator->nameTable, &patternRecord, CompareNames);

        if (!foundRecord) {

            NameTableRecord newRecord = {.name = (char *) calloc (MAX_NAME_LENGTH, sizeof (char)), .value = NAN};    
            if (!newRecord.name) {
                RETURN NAME_TABLE_ERROR;
            }

            strncpy (newRecord.name, stringTokens->lines [tokenIndex].pointer, MAX_NAME_LENGTH);

            if (WriteDataToBuffer (differentiator->nameTable, &newRecord, 1) != BufferErrorCode::NO_BUFFER_ERRORS) {
                RETURN NAME_TABLE_ERROR;
            } 

            foundRecord = &differentiator->nameTable->data [differentiator->nameTable->currentIndex - 1];
        }

        size_t nameIndex = (size_t) (foundRecord - differentiator->nameTable->data) / sizeof (NameTableRecord);
        WriteNodePointer (Var (nameIndex)); 
    }

    WriteNodePointer (OperationNode (NULL, NULL, TERMINATOR));

    RETURN NO_DIFFERENTIATOR_ERRORS;
}

#undef WriteNodePointer

DifferentiatorError ParseFile (Differentiator *differentiator, char *filename) {
    PushLog (2);

    FileBuffer fileContent = {};
    TextBuffer fileText    = {};

    if (!CreateFileBuffer (&fileContent, filename)) { 
        RETURN INPUT_FILE_ERROR;
    }

    if (!ReadFileLines (filename, &fileContent, &fileText, ' ')){
        DestroyFileBuffer (&fileContent);
        RETURN INPUT_FILE_ERROR;
    }

    ChangeNewLinesToZeroes  (&fileText);
    RemoveExcessWhitespaces (&fileText);

    ParsingContext context = {.error = ParsingError::NO_PARSING_ERRORS};

    if (InitBuffer (&context.tokens) != BufferErrorCode::NO_BUFFER_ERRORS) {
        //TODO: Destroy buffers
        RETURN INPUT_FILE_ERROR;
    }

    TreeLexer (&context, differentiator, &fileText);

    fprintf (stderr, "size: %lu\n", context.tokens.currentIndex);
    for (size_t i = 0; i < context.tokens.currentIndex; i++) {
        if (context.tokens.data [i] == NULL) {
            fprintf (stderr, "NULL\n");
            continue;
        }

        if (context.tokens.data [i]->nodeData.type == NUMERIC_NODE)
            fprintf (stderr, "%lf\n", context.tokens.data [i]->nodeData.value.numericValue);

        if (context.tokens.data [i]->nodeData.type == VARIABLE_NODE)
            fprintf (stderr, "var: %lu\n", context.tokens.data [i]->nodeData.value.variableIndex);

        if (context.tokens.data [i]->nodeData.type == OPERATION_NODE) {
            if (context.tokens.data [i]->nodeData.value.operation == OPEN_BRACKET) {
                fprintf (stderr, "open bracket\n");
            } else if (context.tokens.data [i]->nodeData.value.operation == CLOSE_BRACKET) {
                fprintf (stderr, "close bracket\n");
            } else {
                fprintf (stderr, "operation: %ld\n", (long) context.tokens.data [i]->nodeData.value.operation);
            }
        }
    }

    differentiator->expressionTree.root->left = GetGrammar (&context, differentiator);

    DestroyBuffer (&context.tokens);


    if (context.error != ParsingError::NO_PARSING_ERRORS) {
        RETURN INPUT_FILE_ERROR;
    }
   
    DestroyFileBuffer (&fileContent);
    free (fileText.lines);
    RETURN NO_DIFFERENTIATOR_ERRORS;
}

Tree::Node <DifferentiatorNode> *GetGrammar (ParsingContext *context, Differentiator *differentiator) {
    PushLog (3);

    if (context->error != ParsingError::NO_PARSING_ERRORS) {
        RETURN NULL;
    }

    // FIXME: memory leaks are occuring in case of syntax error
    Tree::Node <DifferentiatorNode> *treeRoot = NextFunction [MAX_PRIORITY] (context, differentiator, MAX_PRIORITY);

    if (context->currentNode->nodeData.type == OPERATION_NODE && context->currentNode->nodeData.value.operation == TERMINATOR) {
        context->error = ParsingError::NO_PARSING_ERRORS;
        RETURN treeRoot;
    }

    RETURN NULL;
}

Tree::Node <DifferentiatorNode> *GetNumber (ParsingContext *context, Differentiator *differentiator) {
    PushLog (4);

    SyntaxAssert (context->currentNode->nodeData.type & (NUMERIC_NODE | VARIABLE_NODE), ParsingError::NUMBER_EXPECTED);

    return context->currentNode++;
}

Tree::Node <DifferentiatorNode> *GetUnaryOperation (ParsingContext *context, Differentiator *differentiator, size_t priority) {
    PushLog (3);

    Tree::Node <DifferentiatorNode> *value = NextFunction [priority - 1] (context, differentiator, priority - 1);

    if (value) {
        RETURN value;
    }
    
    context->error = ParsingError::NO_PARSING_ERRORS;
    const OperationData *operation = findOperationByName(context->currentNode->nodeData.value.operation);

    SyntaxAssert (context->currentNode->nodeData.type == OPERATION_NODE && operation->priority == priority, ParsingError::UNARY_EXPRESSION_EXPECTED);

    context->currentNode->left = value;
    value->parent = context->currentNode;

    RETURN context->currentNode;
}

Tree::Node <DifferentiatorNode> *GetBinaryOperation (ParsingContext *context, Differentiator *differentiator, size_t priority) {
    PushLog (3);

    Tree::Node <DifferentiatorNode> *value = NextFunction [priority - 1] (context, differentiator, priority - 1);

    SyntaxAssert (value, context->error);

    const OperationData *operation = findOperationByName (context->currentNode->nodeData.value.operation); 

    while (context->currentNode->nodeData.type == OPERATION_NODE && operation->priority == priority) {
        Tree::Node <DifferentiatorNode> *operationNode = context->currentNode;
        context->currentNode++;

        operationNode->left = value;
        value->parent  = operationNode; 

        Tree::Node <DifferentiatorNode> *value2 = NextFunction [priority - 1] (context, differentiator, priority - 1);
        SyntaxAssert (value2, context->error);

        operationNode->right = value2;
        value2->parent  = operationNode;

        value = operationNode;
    }

    RETURN value;
}
Tree::Node <DifferentiatorNode> *GetSeparator (ParsingContext *context, Differentiator *differentiator, size_t priority) {
    PushLog (3); 

    Tree::Node <DifferentiatorNode> *value = GetNumber (context, differentiator);

    if (value) {
        RETURN value;
    }

    context->error = ParsingError::NO_PARSING_ERRORS;

    if (context->currentNode->nodeData.type == OPERATION_NODE && context->currentNode->nodeData.value.operation == OPEN_BRACKET) {
        context->currentNode++;

        value = NextFunction [MAX_PRIORITY] (context, differentiator, MAX_PRIORITY);
        
        SyntaxAssert (value, context->error);
        SyntaxAssert(context->currentNode->nodeData.type == OPERATION_NODE && context->currentNode->nodeData.value.operation == CLOSE_BRACKET, ParsingError::BRACKET_EXPECTED);
        context->currentNode++;

        RETURN value;
    }

    SyntaxAssert (false, ParsingError::NUMBER_EXPECTED);
}

static void RemoveExcessWhitespaces (TextBuffer *fileText) {
    PushLog (4);
    
    size_t currentIndex = 0;

    for (size_t lineIndex = 0; lineIndex < fileText->line_count; lineIndex++) {
        if (fileText->lines [lineIndex].length > 0) {
            fileText->lines [currentIndex] = fileText->lines [lineIndex];
            currentIndex++;
        }
    }

    fileText->line_count = currentIndex;

    RETURN;
}
