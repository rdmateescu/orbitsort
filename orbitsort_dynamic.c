#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "orbitsort_internal.h"
#include "orbitsort_structures.h"

#ifdef _OPENMP
#include <omp.h>
#endif

/**
 * ======================================================================
 * FILE HEADER: ORBITSORT DYNAMIC BLOCKS IMPLEMENTATION
 * ======================================================================
 *
 * PURPOSE
 * -------
 * This file contains the Dynamic Blocks implementation of OrbitSort.
 *
 * Instead of sorting individual array elements directly, the algorithm
 * organizes them into sorted blocklists. Each blocklist is represented
 * by a node in an outer vectorlist.
 *
 * The implementation uses the data structures and memory-management
 * functions declared in orbitsort_structures.h.
 *
 * THE DATA MODEL
 * --------------
 * vectorlist:
 * The outer doubly linked list containing the active blocks.
 *
 * blocklist:
 * A doubly linked list containing the sorted integer elements of one
 * block. Each integer is stored in an IntegerNode.
 *
 * pairvector:
 * A temporary array containing the block pairs processed during a phase.
 * It also accommodates an orphan block when the active block count is
 * odd.
 *
 * PAIR PROCESSING
 * ---------------
 * The algorithm preserves the circular pairing and rotation principles
 * of the original OrbitSort.
 *
 * Each phase processes disjoint pairs of active blocks. Pair processing
 * may exchange their positions or merge their contents, depending on
 * their ranges and the supplied minDiff value.
 *
 * MERGING
 * -------
 * The larger blocklist is the merge destination.
 * When both blocks have equal sizes, the first block in the OrbitSort
 * pair is the destination.
 *
 * IntegerNode elements are transferred by relinking existing nodes.
 * Integer values are not copied, and merging does not allocate new
 * IntegerNode elements.
 *
 * The destination block remains in its original vectorlist position.
 * The source block is consumed and its pairvector pointer becomes NULL.
 *
 * PAIRVECTOR AND RECONSTRUCTION
 * -----------------------------
 * Pairvector preserves the sequence of blocks processed during a phase.
 * Its markers identify the pair containing the first active block.
 *
 * After pair processing is complete, vectorlist is reconstructed from
 * pairvector, respecting pointer order, markers and consumed blocks.
 *
 * Workers do not modify the outer vectorlist links during a phase.
 *
 * PHASE ROTATION
 * --------------
 * Pair selection follows the circular pairing principle of the original
 * OrbitSort. The rotation cursor identifies a live block rather than
 * relying on a numeric array index.
 *
 * If a block referenced by the rotation cursor is consumed by a merge,
 * the cursor is redirected to the surviving destination block.
 *
 * STOPPING CONDITION
 * ------------------
 * The stopping threshold depends on the number of active blocks.
 *
 * The threshold is recalculated after each phase because merges reduce
 * the active block count.
 *
 * The algorithm stops immediately when one or fewer active blocks remain.
 *
 * SEQUENTIAL AND OPENMP VERSIONS
 * ------------------------------
 * The sequential and OpenMP implementations use the same pair-selection,
 * comparison, swapping, merging and reconstruction rules.
 *
 * During an OpenMP phase, workers process independent pairs. Workers
 * operate on their own pairvector entries and associated blocklists.
 * The outer vectorlist is reconstructed only after all workers finish.
 *
 * MEMORY OWNERSHIP
 * ----------------
 * vectorlist owns its VectorNode elements and their associated BlockList
 * structures.
 *
 * Each blocklist owns its IntegerNode elements.
 *
 * pairvector owns its PairEntry array. Its VectorNode pointers are
 * borrowed references and must not be freed by pairvector destruction.
 *
 * The pairvector must no longer be in use before vectorlist destruction.
 *
 * ======================================================================
 */
 
 
 
 /**
  * FUNCTION: validate_dynamic_sorting_parameters
  *
  * PURPOSE:
  * Validate the input parameters required by the Dynamic Blocks
  * implementation and initialize its output parameters.
  */
static int validate_dynamic_sorting_parameters(
    const int* array_to_sort,
    size_t number_of_elements,
    unsigned long long* total_phases_executed,
    size_t* number_of_workers)
{
    int error_code = ORBITSORT_SUCCESS;

    /*
     * STEP 1: Validate the required pointers.
     */

    error_code = validate_required_pointers(
        array_to_sort,
        total_phases_executed);

    if (error_code == ORBITSORT_SUCCESS)
    {
        error_code = validate_required_pointer(number_of_workers);
    }

    /*
     * STEP 2: Validate the number of input elements.
     */

    if (error_code == ORBITSORT_SUCCESS)
    {
        if (number_of_elements == 0)
        {
            error_code = ORBITSORT_ERROR_ZERO_ELEMENTS;
        }
    }

    /*
     * STEP 3: Initialize the output parameters.
     */

    if (error_code == ORBITSORT_SUCCESS)
    {
        *total_phases_executed = 0;
        *number_of_workers = 0;
    }

    /*
     * STEP 4: Calculate the number of workers.
     *
     * Each worker processes one pair of blocks.
     * An odd number of elements leaves one unpaired element.
     */

    if (error_code == ORBITSORT_SUCCESS)
    {
        *number_of_workers = number_of_elements / 2;
    }

    /*
     * STEP 5: Check whether the worker count is within the
     * supported range.
     */

    if (error_code == ORBITSORT_SUCCESS)
    {
        if (*number_of_workers > (size_t)PTRDIFF_MAX)
        {
            error_code = ORBITSORT_ERROR_TOO_MANY_ELEMENTS;
        }
    }

    return error_code;
}

/**
 * FUNCTION: initialize_dynamic_sorting_state
 *
 * PURPOSE:
 * Initialize the vectorlist and pairvector required by the
 * Dynamic Blocks implementation.
 */
static int initialize_dynamic_sorting_state(
    VectorList* vectorlist,
    PairVector* pairvector,
    const int* array_to_sort,
    size_t number_of_elements)
{
    const char* exit_reason = ORBITSORT_SUCCESS_MESSAGE;
    int error_code = ORBITSORT_SUCCESS;

    size_t pairvector_capacity = 0;

    (void)log_function_enter(
        ORBITSORT_LOG_LEVEL_TRACE,
        "initialize_dynamic_sorting_state");

    /*
     * STEP 1: Validate the required pointers.
     */

    error_code = validate_required_pointers(
        vectorlist,
        pairvector);

    if (error_code == ORBITSORT_SUCCESS)
    {
        error_code = validate_required_pointer(array_to_sort);
    }

    /*
     * STEP 2: Initialize the vectorlist from the input array.
     *
     * Initially, every integer belongs to its own block.
     */

    if (error_code == ORBITSORT_SUCCESS)
    {
        error_code = initialize_vectorlist_from_array(
            vectorlist,
            array_to_sort,
            number_of_elements);

        if (error_code != ORBITSORT_SUCCESS)
        {
            exit_reason =
                "Dynamic sorting initialization stopped because the vectorlist could not be initialized.";
        }
    }

    /*
     * STEP 3: Calculate the required pairvector capacity.
     *
     * Every pair occupies one entry.
     * An odd number of blocks requires one additional entry
     * for the orphan block.
     */

    if (error_code == ORBITSORT_SUCCESS)
    {
        pairvector_capacity =
            (number_of_elements / 2) +
            (number_of_elements % 2);
    }

    /*
     * STEP 4: Allocate the pairvector.
     */

    if (error_code == ORBITSORT_SUCCESS)
    {
        error_code = initialize_pairvector(
            pairvector,
            pairvector_capacity);

        if (error_code != ORBITSORT_SUCCESS)
        {
            exit_reason =
                "Dynamic sorting initialization stopped because the pairvector could not be initialized.";
        }
    }

    /*
     * STEP 5: Release the vectorlist if initialization failed.
     *
     * The vectorlist was successfully created, but the complete
     * sorting state could not be prepared.
     */

    if (error_code != ORBITSORT_SUCCESS)
    {
        (void)destroy_vectorlist(vectorlist);
    }

    (void)log_function_exit(
        ORBITSORT_LOG_LEVEL_TRACE,
        "initialize_dynamic_sorting_state",
        exit_reason);

    return error_code;
}
