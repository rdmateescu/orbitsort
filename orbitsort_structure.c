
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>


#include "orbitsort_structures.h"
#include "orbitsort_internal.h"

//============= BLOCKLIST FUNCTIONS =============
//
// Functions for managing a blocklist, which stores the integer
// elements belonging to one block. The blocklist owns its
// integer nodes and maintains pointers to its first and last nodes.

/*
 * FUNCTION: initialize_blocklist
 *
 * PURPOSE:
 * Initialize an empty blocklist before integer nodes are added.
 *
 * PARAMETERS:
 * - blocklist: Pointer to the blocklist that must be initialized.
 *
 * ALGORITHM:
 * 1. Validate the blocklist pointer.
 * 2. Set the first and last node pointers to NULL.
 * 3. Set the number of integer elements to zero.
 *
 * MEMORY:
 * No memory is allocated or released by this function.
 *
 * RETURNS:
 * ORBITSORT_SUCCESS on success.
 * ORBITSORT_ERROR_NULL_POINTER if blocklist is NULL.
 * This function must not be called on a populated vectorlist,
 * because doing so would discard its existing node pointers.
 */
int initialize_blocklist(BlockList* blocklist)
{
    int error_code = ORBITSORT_SUCCESS;
    const char* exit_reason = ORBITSORT_SUCCESS_MESSAGE;

    (void)log_function_enter(
        ORBITSORT_LOG_LEVEL_TRACE,
        "initialize_blocklist");

    /* STEP 1: Validate the blocklist pointer. */
    if (blocklist == NULL) {
        error_code = ORBITSORT_ERROR_NULL_POINTER;

        (void)log_error(
            "initialize_blocklist",
            error_code,
            "The blocklist pointer is NULL.");

        exit_reason = "Initialization stopped because the blocklist pointer is NULL.";
    }

    /* STEP 2: Initialize the blocklist fields. */
    if (error_code == ORBITSORT_SUCCESS) {
        blocklist->first = NULL;
        blocklist->last = NULL;
        blocklist->size = 0;

        exit_reason = "The empty blocklist was initialized successfully.";
    }

    (void)log_function_exit(
        ORBITSORT_LOG_LEVEL_TRACE,
        "initialize_blocklist",
        exit_reason);

    return error_code;
}

//============= APPEND INTEGER =============
//
// Allocates a new IntegerNode and appends it to the end of a
// blocklist.
//
// Parameters:
//   blocklist - Pointer to the BlockList that receives the integer.
//   value     - Integer value stored in the new node.
//
// Returns:
//   ORBITSORT_SUCCESS if the integer is appended successfully.
//   ORBITSORT_ERROR_NULL_POINTER if blocklist is NULL.
//   ORBITSORT_ERROR_TOO_MANY_ELEMENTS if the blocklist size
//   cannot be increased without overflowing size_t.
//   ORBITSORT_ERROR_MEMORY_ALLOCATION if memory allocation fails.
//
// Memory:
//   Allocates memory for one IntegerNode.
//   Ownership of the allocated node belongs to the blocklist.
//
// Important:
//   The new node is always appended at the end of the blocklist.
//   Existing nodes are not moved or copied.

int append_integer(BlockList* blocklist, int value)
{
    int error_code = ORBITSORT_SUCCESS;
    const char* exit_reason = ORBITSORT_SUCCESS_MESSAGE;

    IntegerNode* new_node = NULL;

    (void)log_function_enter(
        ORBITSORT_LOG_LEVEL_TRACE,
        "append_integer");

    // STEP 1: Validate the blocklist pointer.
    if (blocklist == NULL) {
        error_code = ORBITSORT_ERROR_NULL_POINTER;
        exit_reason = "The blocklist pointer is NULL.";

        (void)log_error(
            "append_integer",
            error_code,
            exit_reason);
    }

    // STEP 2: Check whether the blocklist size can be increased.
    if (error_code == ORBITSORT_SUCCESS) {
        if (blocklist->size == SIZE_MAX) {
            error_code = ORBITSORT_ERROR_TOO_MANY_ELEMENTS;
            exit_reason = "The blocklist size cannot be increased without overflow.";

            (void)log_error(
                "append_integer",
                error_code,
                exit_reason);
        }
    }

    // STEP 3: Allocate memory for the new integer node.
    if (error_code == ORBITSORT_SUCCESS) {
        new_node = malloc(sizeof(IntegerNode));

        if (new_node == NULL) {
            error_code = ORBITSORT_ERROR_MEMORY_ALLOCATION;
            exit_reason = "Memory allocation for the new integer node failed.";

            (void)log_error(
                "append_integer",
                error_code,
                exit_reason);
        }
    }

    // STEP 4: Initialize the new node.
    if (error_code == ORBITSORT_SUCCESS) {
        new_node->value = value;
        new_node->previous = blocklist->last;
        new_node->next = NULL;

        // STEP 5: Attach the new node to the end of the blocklist.
        if (blocklist->last != NULL) {
            blocklist->last->next = new_node;
        }
        else {
            // The blocklist was empty, so this is its first node.
            blocklist->first = new_node;
        }

        blocklist->last = new_node;

        // STEP 6: Update the number of integer elements.
        blocklist->size++;

        exit_reason = "The integer was appended successfully.";
    }

    (void)log_function_exit(
        ORBITSORT_LOG_LEVEL_TRACE,
        "append_integer",
        exit_reason);

    return error_code;
}

//============= DESTROY BLOCKLIST =============
//
// Releases all IntegerNode elements belonging to a blocklist
// and resets the BlockList structure to an empty state.
//
// Parameters:
//   blocklist - Pointer to the BlockList structure to destroy.
//
// Returns:
//   ORBITSORT_SUCCESS if destruction succeeds.
//   ORBITSORT_ERROR_NULL_POINTER if blocklist is NULL.
//
// Memory:
//   Releases the memory occupied by every IntegerNode.
//   Does not release the BlockList structure itself.
//
// Important:
//   After successful destruction, the blocklist contains no
//   integer elements and can be initialized or reused.

int destroy_blocklist(BlockList* blocklist)
{
    int error_code = ORBITSORT_SUCCESS;
    const char* exit_reason = ORBITSORT_SUCCESS_MESSAGE;

    IntegerNode* current_node = NULL;
    IntegerNode* next_node = NULL;

    (void)log_function_enter(
        ORBITSORT_LOG_LEVEL_TRACE,
        "destroy_blocklist");

    // STEP 1: Validate the blocklist pointer.
    if (blocklist == NULL) {
        error_code = ORBITSORT_ERROR_NULL_POINTER;
        exit_reason = "The blocklist pointer is NULL.";

        (void)log_error(
            "destroy_blocklist",
            error_code,
            exit_reason);
    }

    // STEP 2: Start releasing nodes from the beginning.
    if (error_code == ORBITSORT_SUCCESS) {
        current_node = blocklist->first;

        // STEP 3: Release every integer node.
        while (current_node != NULL) {

            // Save the next pointer before freeing the current node.
            next_node = current_node->next;

            free(current_node);

            current_node = next_node;
        }

        // STEP 4: Reset the blocklist structure.
        blocklist->first = NULL;
        blocklist->last = NULL;
        blocklist->size = 0;

        exit_reason = "All integer nodes were released and the blocklist was reset.";
    }

    (void)log_function_exit(
        ORBITSORT_LOG_LEVEL_TRACE,
        "destroy_blocklist",
        exit_reason);

    return error_code;
}
//============= VECTORLIST FUNCTIONS =============
//
// Functions for managing the vectorlist, the outer doubly linked
// list containing the active blocks.
//
// Each VectorNode references one BlockList. The vectorlist size
// represents the number of active blocks, not the total number
// of integer elements stored in those blocks.

//============= APPEND BLOCK =============
//
// Appends a new vectorlist node containing the supplied blocklist.
//
// The vectorlist takes ownership of the blocklist only after the
// new vectorlist node has been successfully allocated and attached.
// If the operation fails, the caller retains ownership of the blocklist.

int append_block(VectorList* vectorlist, BlockList* block)
{
    const char* exit_reason = ORBITSORT_SUCCESS_MESSAGE;
    int error_code = ORBITSORT_SUCCESS;

    VectorNode* new_node = NULL;
    VectorNode* current_node = NULL;

    (void)log_function_enter(ORBITSORT_LOG_LEVEL_TRACE, "append_block");

    // STEP 1: Validate the required pointers.
    // Both the vectorlist and the blocklist must exist.

    if (vectorlist == NULL)
    {
        error_code = ORBITSORT_ERROR_NULL_POINTER;
        exit_reason = "Appending stopped because the vectorlist pointer is NULL.";
        (void)log_error("append_block", error_code, exit_reason);
    }
    else if (block == NULL)
    {
        error_code = ORBITSORT_ERROR_NULL_POINTER;
        exit_reason = "Appending stopped because the blocklist pointer is NULL.";
        (void)log_error("append_block", error_code, exit_reason);
    }

    // STEP 2: Check whether the vectorlist size can be increased.
    // Prevent size from overflowing its size_t representation.

    if (error_code == ORBITSORT_SUCCESS)
    {
        if (vectorlist->size == SIZE_MAX)
        {
            error_code = ORBITSORT_ERROR_TOO_MANY_ELEMENTS;
            exit_reason = "Appending stopped because the vectorlist size would overflow.";
            (void)log_error("append_block", error_code, exit_reason);
        }
    }

    // STEP 3: Allocate the new vectorlist node.
    // The blocklist is not transferred to the vectorlist yet.

    if (error_code == ORBITSORT_SUCCESS)
    {
        new_node = (VectorNode*)malloc(sizeof(VectorNode));

        if (new_node == NULL)
        {
            error_code = ORBITSORT_ERROR_MEMORY_ALLOCATION;
            exit_reason = "Appending stopped because memory allocation for the vectorlist node failed.";
            (void)log_error("append_block", error_code, exit_reason);
        }
    }

    // STEP 4: Initialize the new node.
    // It references the supplied blocklist and starts detached.

    if (error_code == ORBITSORT_SUCCESS)
    {
        new_node->block = block;
        new_node->previous = NULL;
        new_node->next = NULL;
    }

    // STEP 5: Find the current last vectorlist node.
    // Since VectorList has no last pointer, traverse from first.

    if (error_code == ORBITSORT_SUCCESS)
    {
        current_node = vectorlist->first;

        while (current_node != NULL && current_node->next != NULL)
        {
            current_node = current_node->next;
        }
    }

    // STEP 6: Attach the new node.
    // An empty vectorlist receives the new node as its first element.
    // Otherwise, link it after the current last node.

    if (error_code == ORBITSORT_SUCCESS)
    {
        if (current_node == NULL)
        {
            vectorlist->first = new_node;
        }
        else
        {
            current_node->next = new_node;
            new_node->previous = current_node;
        }

        vectorlist->size++;
    }

    // STEP 7: Release any node that was allocated but not attached.
    // On failure, the caller still owns the supplied blocklist.

    if (error_code != ORBITSORT_SUCCESS)
    {
        free(new_node);
    }

    (void)log_function_exit(ORBITSORT_LOG_LEVEL_TRACE, "append_block", exit_reason);

    return error_code;
}

//============= INITIALIZE VECTORLIST =============
//
// Initializes an existing VectorList structure as an empty list.
//
// Parameters:
//   vectorlist - Pointer to the VectorList structure to initialize.
//
// Returns:
//   ORBITSORT_SUCCESS if initialization succeeds.
//   ORBITSORT_ERROR_NULL_POINTER if vectorlist is NULL.
//
// Memory:
//   No memory is allocated or released.
//
// Important:
//   The vectorlist starts with no VectorNode elements.
//   Its size represents the number of active blocks.
//
//   This function must not be called on a populated vectorlist,
//   because doing so would discard its existing node pointers.

int initialize_vectorlist(VectorList* vectorlist)
{
    int error_code = ORBITSORT_SUCCESS;
    const char* exit_reason = ORBITSORT_SUCCESS_MESSAGE;

    (void)log_function_enter(
        ORBITSORT_LOG_LEVEL_TRACE,
        "initialize_vectorlist");

    // STEP 1: Validate the vectorlist pointer.
    if (vectorlist == NULL) {
        error_code = ORBITSORT_ERROR_NULL_POINTER;
        exit_reason = "The vectorlist pointer is NULL.";

        (void)log_error(
            "initialize_vectorlist",
            error_code,
            exit_reason);
    }

    // STEP 2: Initialize the vectorlist fields.
    if (error_code == ORBITSORT_SUCCESS) {
        vectorlist->first = NULL;
        vectorlist->size = 0;

        exit_reason = "The empty vectorlist was initialized successfully.";
    }

    (void)log_function_exit(
        ORBITSORT_LOG_LEVEL_TRACE,
        "initialize_vectorlist",
        exit_reason);

    return error_code;
}



//============= DESTROY VECTORLIST =============
//
// Releases all VectorNode elements in the vectorlist and destroys
// the BlockList referenced by each node.
//
// Parameters:
//   vectorlist - Pointer to the VectorList structure to destroy.
//
// Returns:
//   ORBITSORT_SUCCESS if destruction succeeds.
//   ORBITSORT_ERROR_NULL_POINTER if vectorlist is NULL.
//   An error returned by destroy_blocklist() if a blocklist
//   cannot be destroyed.
//
// Memory:
//   Releases every IntegerNode through destroy_blocklist().
//   Releases every VectorNode.
//   Does not release the VectorList structure itself.
//
// Important:
//   The vectorlist owns its VectorNode elements and the BlockList
//   structures referenced by them.
//   PairVector must no longer reference these nodes when this
//
// Releases all VectorNode elements in the vectorlist and destroys
// the BlockList referenced by each node.
//
// The vectorlist owns both the VectorNode elements and their
// associated BlockList structures.

int destroy_vectorlist(VectorList* vectorlist)
{
    int error_code = ORBITSORT_SUCCESS;
    const char* exit_reason = ORBITSORT_SUCCESS_MESSAGE;

    VectorNode* current_node = NULL;
    VectorNode* next_node = NULL;

    (void)log_function_enter(
        ORBITSORT_LOG_LEVEL_TRACE,
        "destroy_vectorlist");

    // STEP 1: Validate the vectorlist pointer.
    if (vectorlist == NULL) {
        error_code = ORBITSORT_ERROR_NULL_POINTER;
        exit_reason = "The vectorlist pointer is NULL.";

        (void)log_error(
            "destroy_vectorlist",
            error_code,
            exit_reason);
    }

    // STEP 2: Start releasing nodes from the beginning.
    if (error_code == ORBITSORT_SUCCESS) {
        current_node = vectorlist->first;

        // STEP 3: Destroy each blocklist and its vector node.
        while (current_node != NULL &&
            error_code == ORBITSORT_SUCCESS) {

            // Save the next vector node before releasing the current one.
            next_node = current_node->next;

            // Destroy the integer nodes belonging to this blocklist.
            // Any failure is already logged by destroy_blocklist().
            error_code = destroy_blocklist(current_node->block);

            if (error_code == ORBITSORT_SUCCESS) {

                // STEP 4: Remove the current vector node from the list.
                vectorlist->first = next_node;

                if (next_node != NULL) {
                    next_node->previous = NULL;
                }

                vectorlist->size--;

                // STEP 5: Release the now-empty BlockList structure.
                free(current_node->block);

                // STEP 6: Release the current vector node.
                free(current_node);

                current_node = next_node;
            }
            else {
                exit_reason = "Destruction stopped because an associated blocklist could not be destroyed.";
            }
        }

        // STEP 7: Confirm that the vectorlist is empty.
        if (error_code == ORBITSORT_SUCCESS) {
            vectorlist->first = NULL;
            vectorlist->size = 0;

            exit_reason = "All blocklists and vector nodes were destroyed successfully.";
        }
    }

    (void)log_function_exit(
        ORBITSORT_LOG_LEVEL_TRACE,
        "destroy_vectorlist",
        exit_reason);

    return error_code;
}

//============= INITIALIZE VECTORLIST FROM ARRAY =============
//
// Creates one blocklist for each input integer and appends
// each blocklist to the vectorlist.
//
// Each blocklist initially contains exactly one integer node.
// Ownership of a blocklist transfers to the vectorlist only
// after append_block() succeeds.
//
// The vectorlist must not already own allocated nodes.

int initialize_vectorlist_from_array(
    VectorList* vectorlist,
    const int* input_array,
    size_t number_of_elements)
{
    const char* exit_reason = ORBITSORT_SUCCESS_MESSAGE;
    int error_code = ORBITSORT_SUCCESS;
    int cleanup_error_code = ORBITSORT_SUCCESS;

    size_t i = 0;

    BlockList* block = NULL;

    (void)log_function_enter(
        ORBITSORT_LOG_LEVEL_TRACE,
        "initialize_vectorlist_from_array");

    // STEP 1: Validate the required pointers.
    if (vectorlist == NULL) {
        error_code = ORBITSORT_ERROR_NULL_POINTER;
        exit_reason = "Initialization stopped because the vectorlist pointer is NULL.";

        (void)log_error(
            "initialize_vectorlist_from_array",
            error_code,
            exit_reason);
    }
    else if (input_array == NULL) {
        error_code = ORBITSORT_ERROR_NULL_POINTER;
        exit_reason = "Initialization stopped because the input array pointer is NULL.";

        (void)log_error(
            "initialize_vectorlist_from_array",
            error_code,
            exit_reason);
    }

    // STEP 2: Validate the number of input elements.
    if (error_code == ORBITSORT_SUCCESS) {
        if (number_of_elements == 0) {
            error_code = ORBITSORT_ERROR_ZERO_ELEMENTS;
            exit_reason = "Initialization stopped because the input array contains zero elements.";

            (void)log_error(
                "initialize_vectorlist_from_array",
                error_code,
                exit_reason);
        }
    }

    // STEP 3: Initialize an empty vectorlist.
    if (error_code == ORBITSORT_SUCCESS) {
        error_code = initialize_vectorlist(vectorlist);

        if (error_code != ORBITSORT_SUCCESS) {
            exit_reason = "Initialization stopped because the vectorlist could not be initialized.";
        }
    }

    // STEP 4: Build one blocklist for each input integer.
    if (error_code == ORBITSORT_SUCCESS) {

        for (i = 0;
            i < number_of_elements && error_code == ORBITSORT_SUCCESS;
            i++) {

            // Allocate the BlockList structure.
            block = (BlockList*)malloc(sizeof(BlockList));

            if (block == NULL) {
                error_code = ORBITSORT_ERROR_MEMORY_ALLOCATION;
                exit_reason = "Initialization stopped because memory allocation for a blocklist failed.";

                (void)log_error(
                    "initialize_vectorlist_from_array",
                    error_code,
                    exit_reason);
            }

            // Initialize the newly allocated blocklist.
            if (error_code == ORBITSORT_SUCCESS) {
                error_code = initialize_blocklist(block);

                if (error_code != ORBITSORT_SUCCESS) {
                    exit_reason = "Initialization stopped because a blocklist could not be initialized.";
                }
            }

            // Insert the current input integer into the blocklist.
            if (error_code == ORBITSORT_SUCCESS) {
                error_code = append_integer(block, input_array[i]);

                if (error_code != ORBITSORT_SUCCESS) {
                    exit_reason = "Initialization stopped because an integer could not be appended to a blocklist.";
                }
            }

            // Attach the completed blocklist to the vectorlist.
            if (error_code == ORBITSORT_SUCCESS) {
                error_code = append_block(vectorlist, block);

                if (error_code == ORBITSORT_SUCCESS) {

                    // Ownership has transferred to the vectorlist.
                    block = NULL;
                }
                else {
                    exit_reason = "Initialization stopped because a blocklist could not be appended to the vectorlist.";
                }
            }

            // STEP 5: Release the current block if ownership
            // was not transferred to the vectorlist.
            if (error_code != ORBITSORT_SUCCESS && block != NULL) {

                cleanup_error_code = destroy_blocklist(block);

                if (cleanup_error_code == ORBITSORT_SUCCESS) {
                    free(block);
                    block = NULL;
                }
            }
        }
    }

    // STEP 6: Destroy all previously appended blocks if
    // initialization failed partway through.
    if (error_code != ORBITSORT_SUCCESS && vectorlist != NULL) {
        cleanup_error_code = destroy_vectorlist(vectorlist);
    }

    // STEP 7: Log the function result.
    if (error_code == ORBITSORT_SUCCESS) {
        exit_reason = "The vectorlist was initialized successfully from the input array.";
    }

    (void)log_function_exit(
        ORBITSORT_LOG_LEVEL_TRACE,
        "initialize_vectorlist_from_array",
        exit_reason);

    return error_code;
}

//============= PAIRVECTOR FUNCTIONS =============
//
// Functions for managing the pairvector, a fixed-capacity array
// containing the VectorNode pairs assigned during an OrbitSort phase.
//
// The pairvector stores references to VectorNode elements.
// It does not own or destroy those nodes.

//============= INITIALIZE PAIRVECTOR =============
//
// Initializes a PairVector and allocates its fixed-capacity array.
//
// Parameters:
//   pairvector - Pointer to the PairVector structure to initialize.
//   capacity   - Maximum number of PairEntry elements.
//
// Returns:
//   ORBITSORT_SUCCESS if initialization succeeds.
//   ORBITSORT_ERROR_NULL_POINTER if pairvector is NULL.
//   ORBITSORT_ERROR_ZERO_ELEMENTS if capacity is zero.
//   ORBITSORT_ERROR_TOO_MANY_ELEMENTS if the allocation size
//   would overflow size_t.
//   ORBITSORT_ERROR_MEMORY_ALLOCATION if memory allocation fails.
//
// Memory:
//   Allocates memory for the PairEntry array.
//   The PairVector owns this allocated array.
//   The VectorNode elements referenced by its entries remain
//   owned by the vectorlist.
//
// Important:
//   Capacity is fixed after initialization.
//   The logical size starts at zero.
//   first_element_pair is initialized to zero and becomes
//   meaningful when the first-element pair is established.
//   This function must not be called on an already initialized
//   PairVector, because that would discard its existing array pointer.

int initialize_pairvector(PairVector* pairvector, size_t capacity)
{
    int error_code = ORBITSORT_SUCCESS;
    const char* exit_reason = ORBITSORT_SUCCESS_MESSAGE;

    (void)log_function_enter(
        ORBITSORT_LOG_LEVEL_TRACE,
        "initialize_pairvector");

    // STEP 1: Validate the pairvector pointer.
    if (pairvector == NULL) {
        error_code = ORBITSORT_ERROR_NULL_POINTER;
        exit_reason = "The pairvector pointer is NULL.";

        (void)log_error(
            "initialize_pairvector",
            error_code,
            exit_reason);
    }

    // STEP 2: Initialize all fields to a safe empty state.
    // This ensures that the structure remains consistent if
    // a later validation or memory allocation step fails.
    if (error_code == ORBITSORT_SUCCESS) {
        pairvector->pairs = NULL;
        pairvector->size = 0;
        pairvector->capacity = 0;
        pairvector->first_element_pair = 0;
    }

    // STEP 3: Validate the requested capacity.
    if (error_code == ORBITSORT_SUCCESS) {
        if (capacity == 0) {
            error_code = ORBITSORT_ERROR_ZERO_ELEMENTS;
            exit_reason = "The requested pairvector capacity is zero.";

            (void)log_error(
                "initialize_pairvector",
                error_code,
                exit_reason);
        }
    }

    // STEP 4: Check the allocation size for overflow.
    if (error_code == ORBITSORT_SUCCESS) {
        if (capacity > SIZE_MAX / sizeof(PairEntry)) {
            error_code = ORBITSORT_ERROR_TOO_MANY_ELEMENTS;
            exit_reason = "The requested pairvector allocation size would overflow size_t.";

            (void)log_error(
                "initialize_pairvector",
                error_code,
                exit_reason);
        }
    }

    // STEP 5: Allocate memory for the PairEntry array.
    if (error_code == ORBITSORT_SUCCESS) {
        pairvector->pairs = malloc(capacity * sizeof(PairEntry));

        if (pairvector->pairs == NULL) {
            error_code = ORBITSORT_ERROR_MEMORY_ALLOCATION;
            exit_reason = "Memory allocation for the pairvector array failed.";

            (void)log_error(
                "initialize_pairvector",
                error_code,
                exit_reason);
        }
    }

    // STEP 6: Store the requested capacity after successful allocation.
    if (error_code == ORBITSORT_SUCCESS) {
        pairvector->capacity = capacity;

        exit_reason = "The pairvector array was allocated and initialized successfully.";
    }

    (void)log_function_exit(
        ORBITSORT_LOG_LEVEL_TRACE,
        "initialize_pairvector",
        exit_reason);

    return error_code;
}

//============= DESTROY PAIRVECTOR =============
//
// Releases the PairEntry array owned by a PairVector and resets
// the PairVector structure to an empty state.
//
// Parameters:
//   pairvector - Pointer to the PairVector structure to destroy.
//
// Returns:
//   ORBITSORT_SUCCESS if destruction succeeds.
//   ORBITSORT_ERROR_NULL_POINTER if pairvector is NULL.
//
// Memory:
//   Releases the memory occupied by the PairEntry array.
//   Does not destroy or release the VectorNode elements referenced
//   by the entries, because those belong to the vectorlist.
//
// Important:
//   The pairvector must no longer be needed by any worker or phase
//   when this function is called.

int destroy_pairvector(PairVector* pairvector)
{
    int error_code = ORBITSORT_SUCCESS;
    const char* exit_reason = ORBITSORT_SUCCESS_MESSAGE;

    (void)log_function_enter(
        ORBITSORT_LOG_LEVEL_TRACE,
        "destroy_pairvector");

    // STEP 1: Validate the pairvector pointer.
    if (pairvector == NULL) {
        error_code = ORBITSORT_ERROR_NULL_POINTER;
        exit_reason = "The pairvector pointer is NULL.";

        (void)log_error(
            "destroy_pairvector",
            error_code,
            exit_reason);
    }

    // STEP 2: Release the allocated PairEntry array.
    if (error_code == ORBITSORT_SUCCESS) {
        free(pairvector->pairs);

        // STEP 3: Reset the PairVector structure.
        pairvector->pairs = NULL;
        pairvector->size = 0;
        pairvector->capacity = 0;
        pairvector->first_element_pair = 0;

        exit_reason = "The pairvector array was released and the structure was reset.";
    }

    (void)log_function_exit(
        ORBITSORT_LOG_LEVEL_TRACE,
        "destroy_pairvector",
        exit_reason);

    return error_code;
}
