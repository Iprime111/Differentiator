#include <stdio.h>

#include "DifferentiatorDump.h"
#include "Buffer.h"
#include "CustomAssert.h"
#include "Differentiator.h"
#include "Logger.h"
#include "TreeDefinitions.h"

static DifferentiatorError EmitDumpHeader      (Differentiator *differentiator, Buffer <char> *dumpBuffer);
static DifferentiatorError EmitNodeData        (Differentiator *differentiator, Tree::Node <DifferentiatorNode> *node, Buffer <char> *dumpBuffer);
static DifferentiatorError EmitNode            (Differentiator *differentiator, Tree::Node <DifferentiatorNode> *node, Buffer <char> *dumpBuffer);
static char               *GetNodeColor        (Tree::Node <DifferentiatorNode> *node);
static DifferentiatorError EmitNodeConnections (Tree::Node <DifferentiatorNode> *node, Buffer <char> *dumpBuffer);

#define WriteToDumpWithErrorCheck(dumpBuffer, data)                                                     \
    do {                                                                                                \
        if (WriteDataToBuffer (dumpBuffer, data, strlen (data)) != BufferErrorCode::NO_BUFFER_ERRORS) { \
            RETURN DUMP_ERROR;                                                                          \
        }                                                                                               \
    } while (0)

DifferentiatorError DumpExpressionTree (Differentiator *differentiator, char *dumpFilename) {
    PushLog (3);

    custom_assert (differentiator, pointer_is_null, DIFFERENTIATOR_NULL_POINTER);

    Buffer <char> dumpBuffer = {};

    #define CatchError(returnValue, ...)    \
        do {                                \
            if (!(__VA_ARGS__)) {           \
                RETURN returnValue;         \
            }                               \
        } while (0)

    CatchError (DUMP_ERROR, InitBuffer (&dumpBuffer) == BufferErrorCode::NO_BUFFER_ERRORS);
    CatchError (DUMP_ERROR, EmitDumpHeader (differentiator, &dumpBuffer) == NO_DIFFERENTIATOR_ERRORS);
    CatchError (DUMP_ERROR, EmitNode (differentiator, differentiator->expressionTree.root, &dumpBuffer) == NO_DIFFERENTIATOR_ERRORS);

    WriteToDumpWithErrorCheck (&dumpBuffer, "}");

    FILE *dumpFile = fopen (dumpFilename, "w");
    CatchError (DUMP_ERROR, dumpFile != NULL);

    fwrite (dumpBuffer.data, sizeof (char), dumpBuffer.currentIndex, dumpFile);

    fclose (dumpFile);
    CatchError (DUMP_ERROR, DestroyBuffer (&dumpBuffer) == BufferErrorCode::NO_BUFFER_ERRORS);

    #undef CatchError

    RETURN NO_DIFFERENTIATOR_ERRORS;
}

static DifferentiatorError EmitNode (Differentiator *differentiator, Tree::Node <DifferentiatorNode> *node, Buffer <char> *dumpBuffer) {
    PushLog (3);

    char indexBuffer [MAX_NODE_INDEX_LENGTH] = "";
    sprintf (indexBuffer, "%lu", (unsigned long) node);

    WriteToDumpWithErrorCheck (dumpBuffer, "\t");
    WriteToDumpWithErrorCheck (dumpBuffer, indexBuffer);
    WriteToDumpWithErrorCheck (dumpBuffer, " [style=\"filled,rounded\" shape=\"record\" fillcolor=\"" DUMP_NODE_COLOR "\" color=\"");

    char *nodeColor = GetNodeColor (node);
    if (!nodeColor) {
        RETURN DUMP_ERROR;
    }

    WriteToDumpWithErrorCheck (dumpBuffer, nodeColor);
    WriteToDumpWithErrorCheck (dumpBuffer, "\" label=\"{ ");

    EmitNodeData (differentiator, node, dumpBuffer);

    WriteToDumpWithErrorCheck (dumpBuffer, " }\"]\n");

    EmitNodeConnections (node, dumpBuffer);

    if (node->left)
        EmitNode (differentiator, node->left, dumpBuffer);

    if (node->right)
        EmitNode (differentiator, node->right, dumpBuffer);

    RETURN NO_DIFFERENTIATOR_ERRORS;
}

static DifferentiatorError EmitNodeData (Differentiator *differentiator, Tree::Node <DifferentiatorNode> *node, Buffer <char> *dumpBuffer) {
    PushLog (3);

    char indexBuffer [MAX_NODE_INDEX_LENGTH] = "";

    switch (node->nodeData.type) {
        case NUMERIC_NODE: {
            snprintf (indexBuffer, MAX_NODE_INDEX_LENGTH, "%lf", node->nodeData.value.numericValue);

            WriteToDumpWithErrorCheck (dumpBuffer, "{");
            WriteToDumpWithErrorCheck (dumpBuffer, indexBuffer);
            break;
        }

        case OPERATION_NODE: {
            #define OPERATOR(NAME, DESIGNATION, PRIORITY, ...)              \
                if (node->nodeData.value.operation == NAME) {               \
                    WriteToDumpWithErrorCheck (dumpBuffer, "{");            \
                    WriteToDumpWithErrorCheck (dumpBuffer, DESIGNATION);    \
                }

            #include "DifferentiatorOperations.def"

            #undef OPERATOR
            break;
        }

        case VARIABLE_NODE: {
            if (node->nodeData.value.variableIndex >= differentiator->nameTable->currentIndex) {
                RETURN NAME_TABLE_ERROR;
            }
            
            snprintf (indexBuffer, MAX_NODE_INDEX_LENGTH, "%lf", differentiator->nameTable->data [node->nodeData.value.variableIndex].value);

            WriteToDumpWithErrorCheck (dumpBuffer, "{");
            WriteToDumpWithErrorCheck (dumpBuffer, differentiator->nameTable->data [node->nodeData.value.variableIndex].name);
            WriteToDumpWithErrorCheck (dumpBuffer, " | ");
            WriteToDumpWithErrorCheck (dumpBuffer, indexBuffer);


            break;
        }

        default: {
            RETURN TREE_ERROR;
        }
    }

    snprintf (indexBuffer, MAX_NODE_INDEX_LENGTH, "%lu", node->nodeData.depth);
    WriteToDumpWithErrorCheck (dumpBuffer, " | ");
    WriteToDumpWithErrorCheck (dumpBuffer, indexBuffer);
    WriteToDumpWithErrorCheck (dumpBuffer, "}");


    RETURN NO_DIFFERENTIATOR_ERRORS;
}



static DifferentiatorError EmitNodeConnections (Tree::Node <DifferentiatorNode> *node, Buffer <char> *dumpBuffer) {
    PushLog (3);

    char currentNodeIndex [MAX_NODE_INDEX_LENGTH] = "";
    char indexBuffer      [MAX_NODE_INDEX_LENGTH] = "";

    snprintf (currentNodeIndex, MAX_NODE_INDEX_LENGTH, "%lu", (unsigned long) node);

    #define ChildConnection(child)                                                                      \
        if (node->child) {                                                                              \
            snprintf (indexBuffer, MAX_NODE_INDEX_LENGTH, "%lu", (unsigned long) node->child);          \
            WriteToDumpWithErrorCheck (dumpBuffer, "\t");                                               \
            WriteToDumpWithErrorCheck (dumpBuffer, currentNodeIndex);                                   \
            WriteToDumpWithErrorCheck (dumpBuffer, " -> ");                                             \
            WriteToDumpWithErrorCheck (dumpBuffer, indexBuffer);                                        \
            WriteToDumpWithErrorCheck (dumpBuffer, " [color=\"" DUMP_NEXT_CONNECTION_COLOR "\"]\n");    \
        }


    ChildConnection (left);
    ChildConnection (right);

    #undef ChildConnection
    
    RETURN NO_DIFFERENTIATOR_ERRORS;
}

static char *GetNodeColor (Tree::Node <DifferentiatorNode> *node) {
    PushLog (4);

    switch (node->nodeData.type) {
        case NUMERIC_NODE:{
            RETURN DUMP_NUMERIC_NODE_OUTLINE_COLOR;
        }

        case OPERATION_NODE: {
            RETURN DUMP_OPERATION_NODE_OUTLINE_COLOR;
        }

        case VARIABLE_NODE: {
            RETURN DUMP_VARIABLE_NODE_OUTLINE_COLOR;
        }

        default: {
            RETURN NULL;
        }
    }
}

static DifferentiatorError EmitDumpHeader (Differentiator *differentiator, Buffer <char> *dumpBuffer) {
    PushLog (4);

    WriteToDumpWithErrorCheck (dumpBuffer, "digraph {\n\tbgcolor=\"" DUMP_BACKGROUND_COLOR "\"\n");

    RETURN NO_DIFFERENTIATOR_ERRORS;
}
