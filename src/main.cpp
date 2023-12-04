#include <stdio.h>

#include "Differentiator.h"
#include "DifferentiatorIO.h"
#include "DifferentiatorDump.h"
#include "RecursiveDescentParser.h"

int main () {
    Differentiator differentiator      = {};
    Buffer <NameTableRecord> nameTable = {};

    InitNameTable      (&nameTable);
    InitDifferentiator (&differentiator, &nameTable);

    //ReadExpression (&differentiator, "abc.txt");
    ParseFile (&differentiator, "abc.txt");
    
    DumpExpressionTree (&differentiator, "dump_init.dot");

    Differentiator firstDerivative = {};
    Differentiate (&differentiator, &firstDerivative, 0);

    OptimizeTree (&firstDerivative);

    DumpExpressionTree (&firstDerivative, "dump.dot");
    WriteExpressionToStream (&firstDerivative, stdout, firstDerivative.expressionTree.root->left, WriteNodeContentToLatex);

    DestroyDifferentiator (&differentiator);
    DestroyDifferentiator (&firstDerivative);

    DestroyNameTable (&nameTable);

    return 0;
}

