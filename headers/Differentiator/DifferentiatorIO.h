#ifndef DIFFERENTIATOR_IO_H_
#define DIFFERENTIATOR_IO_H_

#include "Differentiator.h"

const double PHRASE_RATE            = 0.45;
const size_t MAX_DEPTH              = 3;

typedef DifferentiatorError (* nodeContentEmitter_t) (Differentiator *, Tree::Node <DifferentiatorNode> *, Buffer <char> *, const OperationData *, const OperationData *, Buffer <Tree::Node <DifferentiatorNode> *> *);

DifferentiatorError WriteNodeContentToLatex  (Differentiator *differentiator, Tree::Node <DifferentiatorNode> *rootNode, Buffer <char> *printBuffer, const OperationData *operation, const OperationData *parentOperation, Buffer <Tree::Node <DifferentiatorNode> *> *reassignmentsBuffer);
DifferentiatorError WriteNodeContentToStream (Differentiator *differentiator, Tree::Node <DifferentiatorNode> *rootNode, Buffer <char> *printBuffer, const OperationData *operation, const OperationData *parentOperation, Buffer <Tree::Node <DifferentiatorNode> *> *reassignmentsBuffer); 

DifferentiatorError WriteExpressionToStream             (Differentiator *differentiator, FILE *stream, Tree::Node <DifferentiatorNode> *rootNode, nodeContentEmitter_t emitter, Buffer <Tree::Node <DifferentiatorNode> *> *reassignmentsBuffer, bool suppressReassignment);
DifferentiatorError DifferentiateAndGenerateLatexReport (Differentiator *differentiator, Differentiator *newDifferentiator, size_t variableIndex, FILE *stream, Buffer <Tree::Node <DifferentiatorNode> *> *reassignmentsBuffer);

DifferentiatorError PrintPhrase (FILE *stream);

#endif
