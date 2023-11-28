#ifndef DIFFERENTIATOR_IO_H_
#define DIFFERENTIATOR_IO_H_

#include "Differentiator.h"

DifferentiatorError WriteSubtreeToLatex (Differentiator *differentiator, Tree::Node <DifferentiatorNode> rootNode, FILE *stream);
DifferentiatorError WriteToLatex        (Differentiator *differentiator, FILE *stream);
DifferentiatorError ReadExpression      (Differentiator *differentiator, char *filename);

#endif
