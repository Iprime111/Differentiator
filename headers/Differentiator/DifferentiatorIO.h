#ifndef DIFFERENTIATOR_IO_H_
#define DIFFERENTIATOR_IO_H_

#include "Differentiator.h"

const double PHRASE_RATE = 0.35;

typedef DifferentiatorError (* nodeContentEmitter_t) (Differentiator *, Tree::Node <DifferentiatorNode> *, Buffer <char> *, const OperationData *, const OperationData *);

DifferentiatorError WriteNodeContentToLatex  (Differentiator *differentiator, Tree::Node <DifferentiatorNode> *rootNode, Buffer <char> *printBuffer, const OperationData *operation, const OperationData *parentOperation);
DifferentiatorError WriteNodeContentToStream (Differentiator *differentiator, Tree::Node <DifferentiatorNode> *rootNode, Buffer <char> *printBuffer, const OperationData *operation, const OperationData *parentOperation); 

DifferentiatorError WriteExpressionToStream             (Differentiator *differentiator, FILE *stream, Tree::Node <DifferentiatorNode> *rootNode, nodeContentEmitter_t emitter);
DifferentiatorError DifferentiateAndGenerateLatexReport (Differentiator *differentiator, Differentiator *newDifferentiator, size_t variableIndex, FILE *stream);

DifferentiatorError PrintPhrase (FILE *stream);

#endif
