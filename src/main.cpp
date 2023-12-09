#include <stdio.h>

#include "Buffer.h"
#include "Differentiator.h"
#include "DifferentiatorIO.h"
#include "DifferentiatorDump.h"
#include "RecursiveDescentParser.h"

int main () {
    Differentiator differentiator      = {};
    Buffer <NameTableRecord> nameTable = {};

    Buffer <Tree::Node <DifferentiatorNode> *> reassignmentBuffer = {};
    InitBuffer (&reassignmentBuffer);

    InitNameTable      (&nameTable);
    InitDifferentiator (&differentiator, &nameTable);

    ParseFile (&differentiator, stdin, &reassignmentBuffer); 
    
    DumpExpressionTree (&differentiator, "dump_init.dot");

    Differentiator firstDerivative = {};
    FILE *reportFile = fopen ("report.tex", "w");

   
    if (reportFile) {
        DifferentiateAndGenerateLatexReport (&differentiator, &firstDerivative, 0, reportFile, &reassignmentBuffer);
    }

    DumpExpressionTree (&firstDerivative, "dump.dot");
    DestroyBuffer (&reassignmentBuffer);
    DestroyDifferentiator (&differentiator);
    DestroyDifferentiator (&firstDerivative);

    DestroyNameTable (&nameTable);

    return 0;
}

