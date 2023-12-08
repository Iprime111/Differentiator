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

    ParseFile (&differentiator, stdin); // TODO: add reading from stdout
    
    DumpExpressionTree (&differentiator, "dump_init.dot");

    Differentiator firstDerivative = {};
    FILE *reportFile = fopen ("report.tex", "w");

    if (reportFile) {
        DifferentiateAndGenerateLatexReport (&differentiator, &firstDerivative, 0, reportFile);
    }

    DumpExpressionTree (&firstDerivative, "dump.dot");

    DestroyDifferentiator (&differentiator);
    DestroyDifferentiator (&firstDerivative);

    DestroyNameTable (&nameTable);

    return 0;
}

