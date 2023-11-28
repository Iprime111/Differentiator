#include "Buffer.h"
#include "CustomAssert.h"
#include "Differentiator.h"
#include "Logger.h"
#include "TreeDefinitions.h"

static DifferentiatorError VerifyDifferentiatorInternal (Tree::Node <DifferentiatorNode> *root);

DifferentiatorError InitDifferentiator (Differentiator *differentiator) {
    PushLog (3);

    custom_assert (differentiator, pointer_is_null, DIFFERENTIATOR_NULL_POINTER);

    differentiator->errors = NO_DIFFERENTIATOR_ERRORS;
    
    if (Tree::InitTree (&differentiator->expressionTree) != Tree::NO_TREE_ERRORS) {
        ReturnError (differentiator, TREE_ERROR);
    }

    if (InitBuffer (&differentiator->nameTable) != BufferErrorCode::NO_BUFFER_ERRORS) {
        ReturnError (differentiator, NAME_TABLE_ERROR);
    }

    VerificationInternal_ (differentiator);

    RETURN NO_DIFFERENTIATOR_ERRORS;
}

DifferentiatorError DestroyDifferentiator (Differentiator *differentiator) {
    PushLog (3);

    if (!differentiator) {
        RETURN NO_DIFFERENTIATOR_ERRORS; 
    }

    if (Tree::DestroyTree (&differentiator->expressionTree) != Tree::NO_TREE_ERRORS) {
        RETURN TREE_ERROR;
    }

    if (!differentiator->nameTable.data) {
        RETURN NAME_TABLE_ERROR;
    }

    for (size_t nameIndex = 0; nameIndex < differentiator->nameTable.currentIndex; nameIndex++) {
        free (differentiator->nameTable.data [nameIndex].name);
    }

    DestroyBuffer (&differentiator->nameTable);

    RETURN NO_DIFFERENTIATOR_ERRORS;
}

DifferentiatorError VerifyDifferentiator (Differentiator *differentiator) {
    PushLog (3);

    if (!differentiator) {
        RETURN DIFFERENTIATOR_NULL_POINTER;
    }

    ON_DEBUG (VerifyDifferentiatorInternal (differentiator->expressionTree.root));

    RETURN differentiator->errors;
}

static DifferentiatorError VerifyDifferentiatorInternal (Tree::Node <DifferentiatorNode> *root) {
    PushLog (3);  

    switch (root->nodeData.type) {
        case VARIABLE_NODE:
        case NUMERIC_NODE:
            if (root->left != NULL || root->right != NULL) {
                RETURN TREE_ERROR;
            }
            break;

        case OPERATION_NODE:
            break;

        default: {
            RETURN TREE_ERROR;
        }
    }

    DifferentiatorError error = NO_DIFFERENTIATOR_ERRORS;

    if (root->left) {
        error = (DifferentiatorError) (error | VerifyDifferentiatorInternal (root->left));
    }

    if (root->right) {
        error = (DifferentiatorError) (error | VerifyDifferentiatorInternal (root->right));
    }

    RETURN NO_DIFFERENTIATOR_ERRORS;
}

long long CompareNames (void *value1, void *value2) {
    NameTableRecord *record1 = (NameTableRecord *) value1;
    NameTableRecord *record2 = (NameTableRecord *) value2;

    return strcmp (record1->name, record2->name); 
}
