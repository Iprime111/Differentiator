#ifndef DIFFERENTIATOR_DUMP_H_
#define DIFFERENTIATOR_DUMP_H_

#include <stddef.h>

#include "Differentiator.h"

const size_t MAX_NODE_INDEX_LENGTH = 32;

#define DUMP_NODE_COLOR                     "#5e69db"
#define DUMP_NUMERIC_NODE_OUTLINE_COLOR     "#c95410"
#define DUMP_OPERATION_NODE_OUTLINE_COLOR   "#10c929"
#define DUMP_VARIABLE_NODE_OUTLINE_COLOR    "#c224ce"
#define DUMP_NEXT_CONNECTION_COLOR          "#10c94b"
#define DUMP_BACKGROUND_COLOR               "#393f87"
#define DUMP_HEADER_NODE_COLOR              "#dbd802"

DifferentiatorError DumpExpressionTree (Differentiator *differentiator, char *dumpFilename);

#endif
