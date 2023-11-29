#ifndef DIFFERENTIATOR_IO_H_
#define DIFFERENTIATOR_IO_H_

#include "Differentiator.h"

typedef DifferentiatorError (* nodeContentEmitter_t) (Differentiator *, Tree::Node <DifferentiatorNode> *, Buffer <char> *, Operation, const char *);

DifferentiatorError WriteNodeContentToLatex  (Differentiator *differentiator, Tree::Node <DifferentiatorNode> *rootNode, Buffer <char> *printBuffer, Operation operation, const char *designation);
DifferentiatorError WriteNodeContentToStream (Differentiator *differentiator, Tree::Node <DifferentiatorNode> *rootNode, Buffer <char> *printBuffer, Operation operation, const char *designation);

DifferentiatorError WriteExpressionToStream (Differentiator *differentiator, FILE *stream, Tree::Node <DifferentiatorNode> *rootNode, nodeContentEmitter_t emitter);
DifferentiatorError ReadExpression          (Differentiator *differentiator, char *filename);

#endif
