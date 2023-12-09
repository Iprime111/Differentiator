#ifndef TREE_OPTIMIZATIONS_H_
#define TREE_OPTIMIZATIONS_H_

#include "Differentiator.h"
#include "TreeDefinitions.h"

enum class OptimizationStatus {
    OPTIMIZATION_ERROR = -1,
    TREE_NOT_CHANGED   = 0,
    TREE_CHANGED       = 1,
};

enum class CollapseResult {
    EXPRESSION_NOT_CHANGED = 0,
    COLLAPSED_TO_CONSTANT  = 1,
};

struct NeutralElement {
    double value               = NAN;
    double collapsedValue      = NAN;

    CollapseResult resultingNode  = CollapseResult::COLLAPSED_TO_CONSTANT;
    Tree::TreeEdge validChild     = Tree::NO_EDGE;
};

typedef OptimizationStatus (* optimization_t) (Differentiator *, Tree::Node <DifferentiatorNode> *); 

OptimizationStatus ComputeSubtree          (Differentiator *Differentiator, Tree::Node <DifferentiatorNode> *rootNode, Buffer <Tree::Node <DifferentiatorNode> *> *reassignmentsBuffer);
OptimizationStatus CollapseNeutralElements (Differentiator *Differentiator, Tree::Node <DifferentiatorNode> *rootNode, Buffer <Tree::Node <DifferentiatorNode> *> *reassignmentsBuffer);

#endif
