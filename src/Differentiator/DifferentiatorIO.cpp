#include <cstdlib>
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
#include "LatexPhrases.h"

static DifferentiatorError WriteExpressionInternal (Differentiator *differentiator, Tree::Node <DifferentiatorNode> *rootNode, Buffer <char> *printBuffer, nodeContentEmitter_t emitter, Buffer <Tree::Node <DifferentiatorNode> *> *reassignmentsBuffer, bool suppressReassignment);

DifferentiatorError WriteExpressionToStream (Differentiator *differentiator, FILE *stream, Tree::Node <DifferentiatorNode> *rootNode, nodeContentEmitter_t emitter, Buffer <Tree::Node <DifferentiatorNode> *> *reassignmentsBuffer, bool suppressReassignment) {
    PushLog (3);

    custom_assert (stream, pointer_is_null, OUTPUT_FILE_ERROR);
    
    Buffer <char> printBuffer = {};
    InitBuffer (&printBuffer);

    WriteExpressionInternal (differentiator, rootNode, &printBuffer, emitter, reassignmentsBuffer, suppressReassignment);

    fwrite (printBuffer.data, sizeof (char), printBuffer.currentIndex, stream);

    DestroyBuffer (&printBuffer);

    RETURN NO_DIFFERENTIATOR_ERRORS;
}

static DifferentiatorError WriteExpressionInternal (Differentiator *differentiator, Tree::Node <DifferentiatorNode> *rootNode, Buffer <char> *printBuffer, nodeContentEmitter_t emitter, Buffer <Tree::Node <DifferentiatorNode> *> *reassignmentsBuffer, bool suppressReassignment) {
    PushLog (3);

    if (!rootNode) {
        RETURN NODE_NULL_POINTER;
    }

    char numberBuffer [MAX_NODE_INDEX_LENGTH] = "";

    if (!suppressReassignment && rootNode->nodeData.depth == 1 && rootNode->nodeData.type != NUMERIC_NODE && rootNode->nodeData.type != VARIABLE_NODE) {
        Tree::Node <DifferentiatorNode> **reassignment = FindValueInBuffer (reassignmentsBuffer, &rootNode, CompareReassignments);

        size_t reassignmentIndex = (size_t) (reassignment - reassignmentsBuffer->data);

        if (reassignment) {
            snprintf (numberBuffer, MAX_NODE_INDEX_LENGTH, "%lu", reassignmentIndex);
            WriteWithErrorCheck (printBuffer, "R_{");
            WriteWithErrorCheck (printBuffer, numberBuffer);
            WriteWithErrorCheck (printBuffer, "}");
            RETURN NO_DIFFERENTIATOR_ERRORS;
        }
    }

    if (rootNode->nodeData.type == NUMERIC_NODE) {
        snprintf (numberBuffer, MAX_NODE_INDEX_LENGTH, "%lg", rootNode->nodeData.value.numericValue);
        WriteWithErrorCheck (printBuffer, numberBuffer);
        RETURN NO_DIFFERENTIATOR_ERRORS;
    }

    if (rootNode->nodeData.type == VARIABLE_NODE) {
        WriteWithErrorCheck (printBuffer, differentiator->nameTable->data [rootNode->nodeData.value.variableIndex].name);
        RETURN NO_DIFFERENTIATOR_ERRORS;
    }
   
    const OperationData *currentOperation = FindOperationByName (rootNode->nodeData.value.operation);

    if (rootNode->parent) {
        const OperationData *parentOperation  = FindOperationByName (rootNode->parent->nodeData.value.operation);

        emitter (differentiator, rootNode, printBuffer, currentOperation, parentOperation, reassignmentsBuffer);
    } else {
        emitter (differentiator, rootNode, printBuffer, currentOperation, NULL, reassignmentsBuffer);
    }


    RETURN NO_DIFFERENTIATOR_ERRORS;
}

DifferentiatorError WriteNodeContentToLatex (Differentiator *differentiator, Tree::Node <DifferentiatorNode> *rootNode, Buffer <char> *printBuffer, const OperationData *operation, const OperationData *parentOperation, Buffer <Tree::Node <DifferentiatorNode> *> *reassignmentsBuffer) {
    PushLog (4);

    custom_assert (differentiator,  pointer_is_null,   NO_DIFFERENTIATOR_ERRORS);
    custom_assert (rootNode,        pointer_is_null,   NODE_NULL_POINTER);
    custom_assert (printBuffer,     pointer_is_null,   OUTPUT_FILE_ERROR);
    custom_assert (operation,       pointer_is_null,   WRONG_OPERATION);

    bool placeBrackets = false;
    if (parentOperation) {
        placeBrackets = (operation->priority >= parentOperation->priority) && (parentOperation->name != DIV);
    }

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

DifferentiatorError WriteNodeContentToStream (Differentiator *differentiator, Tree::Node <DifferentiatorNode> *rootNode, Buffer <char> *printBuffer, const OperationData *operation, const OperationData *parentOperation, Buffer <Tree::Node <DifferentiatorNode> *> *reassignmentsBuffer) {
    PushLog (4);

    custom_assert (differentiator,  pointer_is_null,   NO_DIFFERENTIATOR_ERRORS);
    custom_assert (rootNode,        pointer_is_null,   NODE_NULL_POINTER);
    custom_assert (printBuffer,     pointer_is_null,   OUTPUT_FILE_ERROR);
    custom_assert (operation,       pointer_is_null,   WRONG_OPERATION);

    if (parentOperation && operation->priority >= parentOperation->priority) {
        WriteWithErrorCheck (printBuffer, "(");
    }

    WriteExpression (left, WriteNodeContentToStream);
    WriteWithErrorCheck (printBuffer, operation->designation);

    if (rootNode->right) {
        WriteExpression (right, WriteNodeContentToStream);
    }

    if (parentOperation && operation->priority >= parentOperation->priority) {
        WriteWithErrorCheck (printBuffer, ")");
    }

    RETURN NO_DIFFERENTIATOR_ERRORS;
}

DifferentiatorError DifferentiateAndGenerateLatexReport (Differentiator *differentiator, Differentiator *newDifferentiator, size_t variableIndex, FILE *stream, Buffer <Tree::Node <DifferentiatorNode> *> *reassignmentsBuffer) {
    PushLog (2);

    custom_assert (newDifferentiator, pointer_is_null, DIFFERENTIATOR_NULL_POINTER);
    custom_assert (differentiator,    pointer_is_null, DIFFERENTIATOR_NULL_POINTER);
    custom_assert (stream,            pointer_is_null, OUTPUT_FILE_ERROR);

    fprintf (stream, "\\documentclass{article}\n\\title{Differentiating report}\n\\usepackage[a4paper,top=1.3cm,bottom=2cm,left=1.5cm,right=1.5cm,marginparwidth=0.75cm]{geometry}\n"
                    "\\usepackage[T2A]{fontenc}\n\\usepackage[utf8]{inputenc}\n\\usepackage[english,russian]{babel}\n\\begin{document}\n\\maketitle\n"
                    "\\section {Отчет о дифференцировании}\n\\subsection {Выражение}\n$$");

    WriteExpressionToStream (differentiator, stream, differentiator->expressionTree.root->left, WriteNodeContentToLatex, reassignmentsBuffer, true);

    fprintf (stream, "$$\n\\subsection {Дифференцирование}\n");

    Differentiate (differentiator, newDifferentiator, variableIndex, stream, reassignmentsBuffer);
    DumpExpressionTree(newDifferentiator, "dump_no_opt.dot");

    fprintf (stream, "\\subsection {После всех упрощений}\n$$(");

    WriteExpressionToStream (differentiator, stream, differentiator->expressionTree.root->left, WriteNodeContentToLatex, reassignmentsBuffer, false);
    OptimizeTree (newDifferentiator, reassignmentsBuffer);

    fprintf (stream, ")^{'}");

    WriteExpressionToStream (newDifferentiator, stream, newDifferentiator->expressionTree.root, WriteNodeContentToLatex, reassignmentsBuffer, false);

    fprintf (stream, "$$\n\\subsection {Обозначения}\n");

    for (size_t reassignmentIndex = 0; reassignmentIndex < reassignmentsBuffer->currentIndex; reassignmentIndex++) {
        fprintf (stream, "$$R_{%lu} = ", reassignmentIndex);
        WriteExpressionToStream (differentiator, stream, reassignmentsBuffer->data [reassignmentIndex], WriteNodeContentToLatex, reassignmentsBuffer, true);
        fprintf (stream, "$$\n");

    }

    for (size_t reassignmentIndex = 0; reassignmentIndex < reassignmentsBuffer->currentIndex; reassignmentIndex++) {
        Tree::Node <DifferentiatorNode> *currentNode = reassignmentsBuffer->data [reassignmentIndex];

        if (reassignmentsBuffer->data [reassignmentIndex]->nodeData.manualDelete) {
            #define DestroyChild(direction)                                                                 \
                do {                                                                                        \
                    if (currentNode->direction && currentNode->direction->parent == currentNode) {          \
                        Tree::DestroySubtreeNode (&differentiator->expressionTree, currentNode->direction); \
                    }                                                                                       \
                } while (0)

            DestroyChild (left);
            DestroyChild (right);

            #undef DestroyChild

            Tree::DestroySingleNode (currentNode);
        }
    }

    fprintf (stream, "\\end{document}");

    RETURN NO_DIFFERENTIATOR_ERRORS;
}

DifferentiatorError PrintPhrase (FILE *stream) {
    PushLog (3);

    if (rand () < RAND_MAX * PHRASE_RATE)
        fprintf (stream, "%s\\\\\n", Phrases [(size_t) rand () % PHRASES_COUNT]);

    RETURN NO_DIFFERENTIATOR_ERRORS;
}
