#include <cstdio>
#include <stddef.h>
#include <ctype.h>

#include "Buffer.h"
#include "CustomAssert.h"
#include "Differentiator.h"
#include "RecursiveDescentParser.h"
#include "TextTypes.h"
#include "OperationsDSL.h"

#define WriteNodePointer(node)                                                                              \
    do {                                                                                                    \
        Tree::Node <DifferentiatorNode> *nodePointer = node;                                                \
        if (WriteDataToBuffer (&context->tokens, &nodePointer, 1) != BufferErrorCode::NO_BUFFER_ERRORS) {   \
            RETURN ParsingError::TOKEN_ARRAY_ERROR;                                                         \
        }                                                                                                   \
    } while (0)

#define SyntaxAssert(EXPRESSION, ERROR)                                             \
    do {                                                                            \
        if (!(EXPRESSION)) {                                                        \
            context->error = (ParsingError) ((int) context->error | (int) (ERROR)); \
            RETURN ERROR;                                                           \
        }                                                                           \
    } while (0)

ParsingError TreeLexer (ParsingContext *context, Differentiator *differentiator, FileBuffer *fileContent) {
    PushLog (3);
    
    custom_assert (context,        pointer_is_null, ParsingError::NO_CONTEXT);
    custom_assert (differentiator, pointer_is_null, ParsingError::NO_CONTEXT);
    custom_assert (fileContent,    pointer_is_null, ParsingError::FILE_ERROR);

    char *symbol = fileContent->buffer;
    char *word = (char *) calloc (MAX_WORD_LENGTH + 1, sizeof (char));

    while (symbol < fileContent->buffer + fileContent->buffer_size) {

        if (isspace (*symbol)) {
            symbol++;
            continue;
        }

        if (isdigit (*symbol)) {
            double number     = NAN;
            int  numberLength = 0;

            if (sscanf (symbol, "%lf%n", &number, &numberLength) > 0) {
                WriteNodePointer (Const (number));

                fprintf (stderr, "Const: %lf\n", number);

                symbol += numberLength;
                continue;
            }
        }

        size_t letterNumber = 0;
        bool isOperation    = false;

        while (symbol < fileContent->buffer + fileContent->buffer_size && (isalpha (*symbol) || ispunct (*symbol))) {
            SyntaxAssert (letterNumber < MAX_WORD_LENGTH, ParsingError::WORD_TOO_LONG);

            word [letterNumber]     = *symbol;
            word [letterNumber + 1] = '\0';

            symbol++;

            const OperationData *operation = findOperationByDesignation (word);

            if (operation) {
                WriteNodePointer (OperationNode (NULL, NULL, operation->name));
                isOperation = true;
                fprintf (stderr, "Operation: %s\n", operation->designation);

                break;
            }

            letterNumber++;

            if ((isalpha (*symbol) && ispunct (*(symbol - 1))) || (ispunct (*symbol) && isalpha (*(symbol - 1)))) {
                break;
            }
        }

        if (isOperation) {
            continue;
        }
       
        NameTableRecord patternRecord = {.name = word};
        NameTableRecord *foundRecord  = FindValueInBuffer (differentiator->nameTable, &patternRecord, CompareNames);
        
        if (!foundRecord) {
            NameTableRecord newRecord = {.name = (char *) calloc (MAX_WORD_LENGTH + 1, sizeof (char)), .value = NAN};
            if (!newRecord.name) {
                RETURN ParsingError::NAME_TABLE_ERROR;
            }
        
            strncpy (newRecord.name, word, MAX_WORD_LENGTH);
            if (WriteDataToBuffer (differentiator->nameTable, &newRecord, 1) != BufferErrorCode::NO_BUFFER_ERRORS) {
                RETURN ParsingError::NAME_TABLE_ERROR;
            } 
            foundRecord = &differentiator->nameTable->data [differentiator->nameTable->currentIndex - 1];
        }
        
        size_t nameIndex = (size_t) (foundRecord - differentiator->nameTable->data) / sizeof (NameTableRecord);
        
        WriteNodePointer (Var (nameIndex));
        fprintf (stderr, "Var: (%lu) %s\n", nameIndex, word);
    }
    
    free (word);

    WriteNodePointer (OperationNode (NULL, NULL, TERMINATOR));

    RETURN ParsingError::NO_PARSING_ERRORS;
}

#undef WriteNodePointer


