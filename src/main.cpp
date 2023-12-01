#include <stdio.h>

#include "Differentiator.h"
#include "DifferentiatorIO.h"
#include "DifferentiatorDump.h"

int main () {
    Differentiator differentiator      = {};
    Buffer <NameTableRecord> nameTable = {};

    InitNameTable      (&nameTable);
    InitDifferentiator (&differentiator, &nameTable);

    ReadExpression (&differentiator, "abc.txt");
    
    Differentiator firstDerivative = {};
    Differentiate (&differentiator, &firstDerivative, 0);

    DumpExpressionTree (&firstDerivative);
    WriteExpressionToStream (&firstDerivative, stdout, firstDerivative.expressionTree.root->left, WriteNodeContentToLatex);

    DestroyDifferentiator (&differentiator);
    DestroyDifferentiator (&firstDerivative);

    DestroyNameTable (&nameTable);

    return 0;
}

