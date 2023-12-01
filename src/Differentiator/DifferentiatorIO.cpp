#include <stdlib.h>
#include <stdio.h>

#include "Buffer.h"
#include "CustomAssert.h"
#include "Differentiator.h"
#include "DifferentiatorDump.h"
#include "DifferentiatorIO.h"
#include "OperationsDSL.h"
#include "Logger.h"
#include "FileIO.h"
#include "TextTypes.h"
#include "TreeDefinitions.h"

static DifferentiatorError ReadExpressionInternal          (Differentiator *differentiator, Tree::Node <DifferentiatorNode> *rootNode, TextBuffer *fileText, size_t *tokenIndex);
static DifferentiatorError ReadNodeContent                 (Differentiator *differentiator, Tree::Node <DifferentiatorNode> *node, TextBuffer *fileText, size_t *tokenIndex);
static DifferentiatorError WriteExpressionInternal         (Differentiator *differentiator, Tree::Node <DifferentiatorNode> *rootNode, Buffer <char> *printBuffer, nodeContentEmitter_t emitter);

static void RemoveExcessWhitespaces (TextBuffer *fileText);

DifferentiatorError ReadExpression (Differentiator *differentiator, char *filename) {
    PushLog (3);
    
    custom_assert (differentiator, pointer_is_null, DIFFERENTIATOR_NULL_POINTER);

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

    if (Tree::AddNode (&differentiator->expressionTree, differentiator->expressionTree.root, Tree::LEFT_CHILD) != Tree::NO_TREE_ERRORS) {
        DestroyFileBuffer (&fileContent);
        free (fileText.lines);
        RETURN TREE_ERROR;
    }

    size_t tokenIndex = 0;
    DifferentiatorError error = ReadExpressionInternal (differentiator, differentiator->expressionTree.root->left, &fileText, &tokenIndex);
    if (error != NO_DIFFERENTIATOR_ERRORS) {
        free (fileText.lines);
        DestroyFileBuffer (&fileContent);    
        RETURN error;
    }

    free (fileText.lines);
    DestroyFileBuffer (&fileContent);

    RETURN NO_DIFFERENTIATOR_ERRORS;
}

static DifferentiatorError ReadExpressionInternal (Differentiator *differentiator, Tree::Node <DifferentiatorNode> *rootNode, TextBuffer *fileText, size_t *tokenIndex) {
    PushLog (3);

    if (fileText->lines [*tokenIndex].pointer [0] != '(') {
        RETURN INPUT_FILE_ERROR;
    }

    (*tokenIndex)++;

    #define ReadNode(direction, node)                                                                                   \
        do {                                                                                                            \
            if (Tree::AddNode (&differentiator->expressionTree, rootNode, direction) != Tree::NO_TREE_ERRORS) {         \
                RETURN TREE_ERROR;                                                                                      \
            }                                                                                                           \
            DifferentiatorError error = ReadExpressionInternal (differentiator, rootNode->node, fileText, tokenIndex);  \
            if (error != NO_DIFFERENTIATOR_ERRORS) {                                                                    \
                RETURN error;                                                                                           \
            }                                                                                                           \
        } while (0)

    if (fileText->lines [*tokenIndex].pointer [0] == '(') {
        ReadNode (Tree::LEFT_CHILD, left);
    }

    if (ReadNodeContent (differentiator, rootNode, fileText, tokenIndex) != NO_DIFFERENTIATOR_ERRORS) {
        RETURN INPUT_FILE_ERROR;
    }

    if (fileText->lines [*tokenIndex].pointer [0] == '(') {
        ReadNode (Tree::RIGHT_CHILD, right); 
    }

    (*tokenIndex)++;

    #undef ReadNode
   
    RETURN NO_DIFFERENTIATOR_ERRORS;
}

static DifferentiatorError ReadNodeContent (Differentiator *differentiator, Tree::Node <DifferentiatorNode> *node, TextBuffer *fileText, size_t *tokenIndex) {
    PushLog (3);

    if (sscanf (fileText->lines [*tokenIndex].pointer, "%lf", &node->nodeData.value.numericValue) > 0) {
        node->nodeData.type = NUMERIC_NODE;
        (*tokenIndex)++;
        RETURN NO_DIFFERENTIATOR_ERRORS;
    }

    #define OPERATOR(NAME, DESIGNATION, PRIORITY, ...)                          \
        if (strcmp (DESIGNATION, fileText->lines [*tokenIndex].pointer) == 0) { \
            node->nodeData.value.operation = NAME;                              \
            node->nodeData.type = OPERATION_NODE;                               \
            (*tokenIndex)++;                                                    \
            RETURN NO_DIFFERENTIATOR_ERRORS;                                    \
        }                                           

    #include "DifferentiatorOperations.def"

    #undef OPERATOR

    node->nodeData.type = VARIABLE_NODE;

    NameTableRecord patternRecord = {.name = fileText->lines [*tokenIndex].pointer};
    NameTableRecord *foundRecord = FindValueInBuffer (differentiator->nameTable, &patternRecord, CompareNames);
    
    if (!foundRecord) {
        NameTableRecord newRecord = {.name = (char *) calloc (MAX_NAME_LENGTH + 1, sizeof (char)), .value = NAN};
        if (!newRecord.name) {
            RETURN NAME_TABLE_ERROR;
        }

        strncpy (newRecord.name, fileText->lines [*tokenIndex].pointer, MAX_NAME_LENGTH);
        if (WriteDataToBuffer (differentiator->nameTable, &newRecord, 1) != BufferErrorCode::NO_BUFFER_ERRORS) {
            RETURN NAME_TABLE_ERROR;
        } 
        foundRecord = &differentiator->nameTable->data [differentiator->nameTable->currentIndex - 1];
    }

    (*tokenIndex)++;

    size_t nameIndex = (size_t) (foundRecord - differentiator->nameTable->data) / sizeof (NameTableRecord);
    node->nodeData.value.variableIndex = nameIndex;

    RETURN NO_DIFFERENTIATOR_ERRORS;
}

DifferentiatorError WriteExpressionToStream (Differentiator *differentiator, FILE *stream, Tree::Node <DifferentiatorNode> *rootNode, nodeContentEmitter_t emitter) {
    PushLog (3);

    custom_assert (stream, pointer_is_null, OUTPUT_FILE_ERROR);
    VerificationInternal_ (differentiator);
    
    Buffer <char> printBuffer = {};
    InitBuffer (&printBuffer);

    WriteExpressionInternal (differentiator, rootNode, &printBuffer, emitter);

    fwrite (printBuffer.data, sizeof (char), printBuffer.currentIndex, stream);

    DestroyBuffer (&printBuffer);

    RETURN NO_DIFFERENTIATOR_ERRORS;
}

static DifferentiatorError WriteExpressionInternal (Differentiator *differentiator, Tree::Node <DifferentiatorNode> *rootNode, Buffer <char> *printBuffer, nodeContentEmitter_t emitter) {
    PushLog (3);

    char numberBuffer [MAX_NODE_INDEX_LENGTH] = "";

    if (rootNode->nodeData.type == NUMERIC_NODE) {
        snprintf (numberBuffer, MAX_NODE_INDEX_LENGTH, "%lf", rootNode->nodeData.value.numericValue);
        WriteWithErrorCheck (printBuffer, numberBuffer);
        RETURN NO_DIFFERENTIATOR_ERRORS;
    }

    if (rootNode->nodeData.type == VARIABLE_NODE) {
        WriteWithErrorCheck (printBuffer, differentiator->nameTable->data [rootNode->nodeData.value.variableIndex].name);
        RETURN NO_DIFFERENTIATOR_ERRORS;
    }

    const OperationData *parentOperation  = findOperationByName (rootNode->parent->nodeData.value.operation);
    const OperationData *currentOperation = findOperationByName (rootNode->nodeData.value.operation);

    emitter (differentiator, rootNode, printBuffer, currentOperation, parentOperation);

    RETURN NO_DIFFERENTIATOR_ERRORS;
}

DifferentiatorError WriteNodeContentToLatex (Differentiator *differentiator, Tree::Node <DifferentiatorNode> *rootNode, Buffer <char> *printBuffer, const OperationData *operation, const OperationData *parentOperation) {
    PushLog (4);

    bool placeBrackets = (operation->priority >= parentOperation->priority) && (parentOperation->name != DIV);

    if (placeBrackets) {
        WriteWithErrorCheck (printBuffer, "(");
    }

    #define OPERATOR(NAME, DESIGNATION, PRIORITY, EVAL_CALLBACK, LATEX_CALLBACK, ...)   \
        if (operation->name == NAME) {                                                  \
            LATEX_CALLBACK                                                              \
        }

    #include "DifferentiatorOperations.def"

    #undef OPERATOR

    if (placeBrackets) {
        WriteWithErrorCheck (printBuffer, ")");
    }

    RETURN NO_DIFFERENTIATOR_ERRORS;
}

DifferentiatorError WriteNodeContentToStream (Differentiator *differentiator, Tree::Node <DifferentiatorNode> *rootNode, Buffer <char> *printBuffer, const OperationData *operation, const OperationData *parentOperation) {
    PushLog (4);

    if (operation->priority >= parentOperation->priority) {
        WriteWithErrorCheck (printBuffer, "(");
    }

    WriteExpression (left, WriteNodeContentToStream);
    WriteWithErrorCheck (printBuffer, operation->designation);

    if (rootNode->right) {
        WriteExpression (right, WriteNodeContentToStream);
    }

    if (operation->priority >= parentOperation->priority) {
        WriteWithErrorCheck (printBuffer, ")");
    }

    RETURN NO_DIFFERENTIATOR_ERRORS;
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
