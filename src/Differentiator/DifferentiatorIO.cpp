#include <stdlib.h>
#include <stdio.h>

#include "Buffer.h"
#include "CustomAssert.h"
#include "Differentiator.h"
#include "DifferentiatorIO.h"
#include "Logger.h"
#include "FileIO.h"
#include "TextTypes.h"
#include "TreeDefinitions.h"

static DifferentiatorError ReadExpressionInternal (Differentiator *differentiator, Tree::Node <DifferentiatorNode> *rootNode, TextBuffer *fileText, size_t *tokenIndex);
static DifferentiatorError ReadNodeContent        (Differentiator *differentiator, Tree::Node <DifferentiatorNode> *node, TextBuffer *fileText, size_t *tokenIndex);

DifferentiatorError ReadExpression (Differentiator *differentiator, char *filename) {
    PushLog (3);
    
    custom_assert (differentiator, pointer_is_null, DIFFERENTIATOR_NULL_POINTER);

    FileBuffer fileContent = {};
    TextBuffer fileText    = {};

    if (!CreateFileBuffer (&fileContent, filename)){ //TODO: rewrite library, so it can read not only from files 
        RETURN INPUT_FILE_ERROR;
    }

    if (!ReadFileLines (filename, &fileContent, &fileText, ' ')){
        DestroyFileBuffer (&fileContent);
        RETURN INPUT_FILE_ERROR;
    }

    ChangeNewLinesToZeroes (&fileText);

    size_t tokenIndex = 0;
    DifferentiatorError error = ReadExpressionInternal (differentiator, differentiator->expressionTree.root, &fileText, &tokenIndex);
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
    NameTableRecord *foundRecord = FindValueInBuffer (&differentiator->nameTable, &patternRecord, CompareNames);
    
    if (!foundRecord) {
        NameTableRecord newRecord = {.name = (char *) calloc (MAX_NAME_LENGTH + 1, sizeof (char)), .value = NAN};
        strncpy (newRecord.name, fileText->lines [*tokenIndex].pointer, MAX_NAME_LENGTH);
        WriteDataToBuffer (&differentiator->nameTable, &newRecord, 1); // TODO: error check
        foundRecord = &differentiator->nameTable.data [differentiator->nameTable.currentIndex - 1];
    }

    (*tokenIndex)++;

    size_t nameIndex = (size_t) (foundRecord - differentiator->nameTable.data) / sizeof (NameTableRecord);
    node->nodeData.value.variableIndex = nameIndex;

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

static void SkipWhitespaces (TextBuffer *fileText, size_t *tokenIndex) {
    PushLog (4);
    
    //TODO:

    RETURN;
}
