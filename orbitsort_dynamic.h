#ifndef ORBITSORT_DYNAMIC_H
#define ORBITSORT_DYNAMIC_H

#include <stddef.h>
#include "orbitsort_structures.h"

/*
 * ======================================================================
 * ORBITSORT DYNAMIC BLOCKS
 * ======================================================================
 *
 * Public interface for the Dynamic Blocks implementation.
 *
 * The caller supplies:
 * - The input array.
 * - The number of input elements.
 * - minDiff, the minimum absolute difference between any two input
 *   elements, calculated before calling the sorting function.
 *
 * The function creates and manages the required dynamic structures.
 */

 /*
  * FUNCTION: run_dynamic_until_sorted
  *
  * PURPOSE:
  * Sort the input elements using the OrbitSort Dynamic Blocks algorithm.
  *
  * PARAMETERS:
  * - array_to_sort: Input array.
  * - number_of_elements: Number of input elements.
  * - minDiff: Minimum absolute difference between any two input elements.
  * - total_phases_executed: Output receiving the number of completed phases.
  *
  * RETURNS:
  * ORBITSORT_SUCCESS when sorting completes successfully.
  * An appropriate error code if the operation fails.
  */
int run_dynamic_until_sorted(
    const int* array_to_sort,
    size_t number_of_elements,
    int minDiff,
    unsigned long long* total_phases_executed);

#endif /* ORBITSORT_DYNAMIC_H */
