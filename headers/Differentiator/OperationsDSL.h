#ifndef OPERATIONS_MACROS_H_
#define OPERATIONS_MACROS_H_

#include "Differentiator.h"
#include "Tree.h"
#include "TreeDefinitions.h"

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

#define Diff(direction) DifferentiateInternal (newDifferentiator, variableIndex, rootNode->direction)
#define Copy(direction) CopySubtree (rootNode->direction)
#define Const(number)   CreateNode (Tree::Node <DifferentiatorNode> {.left = NULL, .right = NULL, .parent = NULL, .nodeData = {.type = NUMERIC_NODE,  .value = {.numericValue  = number}}})
#define Var(index)      CreateNode (Tree::Node <DifferentiatorNode> {.left = NULL, .right = NULL, .parent = NULL, .nodeData = {.type = VARIABLE_NODE, .value = {.variableIndex = index}}})
#define OperationNode(leftNode, rightNode, operator) \
    CreateNode (Tree::Node <DifferentiatorNode> {.left = leftNode, .right = rightNode, .parent = NULL, .nodeData = {.type = OPERATION_NODE, .value = {.operation = operator}}})

#define Add(leftNode, rightNode) OperationNode (leftNode, rightNode, ADD)
#define Sub(leftNode, rightNode) OperationNode (leftNode, rightNode, SUB)
#define Mul(leftNode, rightNode) OperationNode (leftNode, rightNode, MUL)
#define Div(leftNode, rightNode) OperationNode (leftNode, rightNode, DIV)
#define Pow(leftNode, rightNode) OperationNode (leftNode, rightNode, POW)
#define Sin(leftNode)            OperationNode (leftNode, NULL,      SIN)
#define Cos(leftNode)            OperationNode (leftNode, NULL,      COS)
#define Exp(leftNode)            OperationNode (leftNode, NULL,      EXP)
#define Ln(leftNode)             OperationNode (leftNode, NULL,      LN)
#define Sqrt(leftNode)           OperationNode (leftNode, NULL,      SQRT)

Tree::Node <DifferentiatorNode> *CopySubtree (Tree::Node <DifferentiatorNode> *subtreeRoot);
Tree::Node <DifferentiatorNode> *CreateNode  (Tree::Node <DifferentiatorNode> node);

#endif
