#include <stdio.h>

#include "Differentiator.h"
#include "DifferentiatorIO.h"
#include "DifferentiatorDump.h"

int main () {
    Differentiator differentiator = {};

    InitDifferentiator (&differentiator);

    ReadExpression (&differentiator, "abc.txt");

    DumpExpressionTree (&differentiator);

    double value = 0;
    EvalTree (&differentiator, &value);

    printf ("%lf\n", value);
    WriteExpressionToStream (&differentiator, stderr, differentiator.expressionTree.root->left, WriteNodeContentToLatex);

    DestroyDifferentiator (&differentiator);

    return 0;
}

