#ifndef DIFFERENTIATOR_IO_H_
#define DIFFERENTIATOR_IO_H_

#include "Differentiator.h"

typedef DifferentiatorError (* nodeContentEmitter_t) (Differentiator *, Tree::Node <DifferentiatorNode> *, Buffer <char> *, const OperationData *, const OperationData *);

DifferentiatorError WriteNodeContentToLatex  (Differentiator *differentiator, Tree::Node <DifferentiatorNode> *rootNode, Buffer <char> *printBuffer, const OperationData *operation, const OperationData *parentOperation);
DifferentiatorError WriteNodeContentToStream (Differentiator *differentiator, Tree::Node <DifferentiatorNode> *rootNode, Buffer <char> *printBuffer, const OperationData *operation, const OperationData *parentOperation); 

DifferentiatorError WriteExpressionToStream (Differentiator *differentiator, FILE *stream, Tree::Node <DifferentiatorNode> *rootNode, nodeContentEmitter_t emitter);
DifferentiatorError ReadExpression          (Differentiator *differentiator, char *filename);

#endif
