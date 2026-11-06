
#include <stddef.h>

#ifdef _OPENMP
#include <omp.h>
#endif

/**
 * =================================================================================================
 * FUNCTION HEADER  : min_int
 * =================================================================================================
 * This static inline function compares two integers and returns the smaller one of the two.
 * It is named min_int (and not min) so it never clashes with the min macro of <windows.h>.
 *
 * PARAMETERS EXPLAINED:
 * ---------------------
 * 1. int first_value
 *    - Description: The first integer to compare.
 *    - Role: Candidate for the minimum, returned when it is smaller than the second value.
 *
 * 2. int second_value
 *    - Description: The second integer to compare.
 *    - Role: Candidate for the minimum, returned when it is smaller than or equal to the first value.
 *
 * RETURNS:
 * --------
 * - int: The smaller of first_value and second_value.
 * =================================================================================================
 */
static inline int min_int(int first_value, int second_value)
{
    // Return the first value if it is strictly smaller than the second value, otherwise return the second value
    return first_value < second_value ? first_value : second_value;
}

/**
 * =================================================================================================
 * FUNCTION HEADER  : max_int
 * =================================================================================================
 * This static inline function compares two integers and returns the larger one of the two.
 * It is named max_int (and not max) so it never clashes with the max macro of <windows.h>.
 *
 * PARAMETERS EXPLAINED:
 * ---------------------
 * 1. int first_value
 *    - Description: The first integer to compare.
 *    - Role: Candidate for the maximum, returned when it is larger than the second value.
 *
 * 2. int second_value
 *    - Description: The second integer to compare.
 *    - Role: Candidate for the maximum, returned when it is larger than or equal to the first value.
 *
 * RETURNS:
 * --------
 * - int: The larger of first_value and second_value.
 * =================================================================================================
 */
static inline int max_int(int first_value, int second_value)
{
    // Return the first value if it is strictly larger than the second value, otherwise return the second value
    return first_value > second_value ? first_value : second_value;
}

/**
 * =================================================================================================
 * FUNCTION HEADER  : compare_exchange
 * =================================================================================================
 * This static inline function compares the two array elements located at the two given indices and,
 * if they are out of order, exchanges them so that the SMALLER value always ends up at the LOWER
 * index and the LARGER value always ends up at the HIGHER index, whatever the order of the arguments.
 * For the wrap-around pair (n-1, 0) this means index 0 receives the smallest value and index n-1
 * receives the biggest value, so a single rule covers both the middle pairs and the wrap pair.
 *
 * PARAMETERS EXPLAINED:
 * ---------------------
 * 1. int *array_to_compare
 *    - Description: A pointer to the first element of the mutable integer array being sorted.
 *    - Role: The elements at the two given indices are read and, if needed, rewritten in place.
 *
 * 2. size_t first_index
 *    - Description: The array index of the first element of the pair.
 *    - Role: One of the two positions to compare; it may be the lower or the higher one.
 *
 * 3. size_t second_index
 *    - Description: The array index of the second element of the pair.
 *    - Role: The other position to compare; it may be the lower or the higher one.
 *
 * RETURNS:
 * --------
 * - int: 1 if the two elements were exchanged, 0 if they were already in the correct order.
 * =================================================================================================
 */
static inline int compare_exchange(int* array_to_compare, size_t first_index, size_t second_index)
{
    // Determine the lower of the two indices, which is the position that must receive the smaller value
    size_t lower_index = first_index < second_index ? first_index : second_index;

    // Determine the higher of the two indices, which is the position that must receive the larger value
    size_t higher_index = first_index < second_index ? second_index : first_index;

    // Read the value stored at the lower index once, so the comparison and the writes use the same data
    int value_at_lower_index = array_to_compare[lower_index];

    // Read the value stored at the higher index once, so the comparison and the writes use the same data
    int value_at_higher_index = array_to_compare[higher_index];

    // Check if the pair is inverted, meaning the lower index currently holds a value larger than the higher index
    if (value_at_lower_index > value_at_higher_index) {
        // Write the smaller value to the lower index
        array_to_compare[lower_index] = min_int(value_at_lower_index, value_at_higher_index);

        // Write the larger value to the higher index
        array_to_compare[higher_index] = max_int(value_at_lower_index, value_at_higher_index);

        // Report that exactly one swap was performed
        return 1;
    }

    // The pair was already in the correct order, nothing was modified, so report zero swaps
    return 0;
}

/**
 * =================================================================================================
 * FUNCTION HEADER  : run_until_sorted
 * =================================================================================================
 * This static function sorts an array of integers in place using a parallelized (via OpenMP)
 * or sequential rotating-offset exchange sorting mechanism, running continuous phases until the
 * array is mathematically proven to be fully sorted, and returns the total number of phases executed.
 *
 * PARAMETERS EXPLAINED:
 * ---------------------
 * 1. int *array_to_sort
 *    - Description: A pointer to the memory address of the very first element of a mutable integer array.
 *    - Role: This is the raw data array that will be manipulated, compared, and modified directly
 *            in place (in-memory) until all elements are arranged in ascending sorted order.
 *            Because it is passed by pointer, all sorting modifications performed by worker threads
 *            persist outside this function scope.
 *
 * 2. size_t number_of_elements
 *    - Description: An unsigned integer representing the total count of elements present in the array.
 *    - Role: Defines the operational bounds of the array. It dictates how many worker pairs are
 *            created (`number_of_elements / 2`), how modulo wrapping calculations wrap around boundaries,
 *            and how many consecutive zero-swap phases are strictly required to mathematically prove
 *            that the array has reached a completely sorted state (2 phases if even, 3 if odd).
 *
 * RETURNS:
 * --------
 * - unsigned long long: The grand total count of all sorting phases executed from the moment the
 *                       function started until the termination threshold was successfully met.
 * =================================================================================================
 */
static unsigned long long run_until_sorted(int* array_to_sort, size_t number_of_elements)
{
    // Calculate the number of worker pairs, which is half of the total number of elements (integer division drops remainders)
    size_t number_of_workers = number_of_elements / 2;

    // Determine how many consecutive zero-swap phases are required to prove the array is sorted: 2 if the element count is even, 3 if it is odd
    size_t number_of_phases_needed_to_prove_sorted = (number_of_elements % 2 == 0) ? 2 : 3;

    // Initialize an unsigned long long counter variable to track the total number of sorting phases executed across the entire process from start to finish
    unsigned long long total_phases_executed = 0;

    // Initialize a size_t counter variable to track how many phases in a direct row have resulted in zero element swaps being performed
    size_t consecutive_phases_with_zero_swaps = 0;

    // Initialize the rotation offset variable to zero, which dictates how the pairing window shifts for each subsequent sorting phase
    size_t current_rotation_offset = 0;

    // Check via preprocessor directive if OpenMP is enabled at compile time so we can run the parallel code path instead of the fallback
#ifdef _OPENMP
    // Declare an integer flag shared among all threads to control the main loop execution; 1 means keep looping, 0 means terminate everything
    int sorting_not_finished = 1;

    // Declare a shared accumulator variable to hold the combined total number of swaps that happen across all threads in the current phase
    unsigned long long swaps_in_this_phase = 0;

    // Start a parallel multi-threaded region, explicitly declaring every variable as shared so every thread reads and writes the exact same memory
#pragma omp parallel shared(array_to_sort, number_of_elements, number_of_workers, \
                                number_of_phases_needed_to_prove_sorted, total_phases_executed, \
                                consecutive_phases_with_zero_swaps, current_rotation_offset, \
                                sorting_not_finished, swaps_in_this_phase)
    {
        // Loop continuously as long as the shared sorting_not_finished control flag remains true (meaning it evaluates to a non-zero value)
        while (sorting_not_finished) {

            // Designate precisely one single thread to reset the shared phase swap accumulator variable back to zero before parallel work begins
#pragma omp single
            {
                swaps_in_this_phase = 0;
            } // Implicit thread barrier inserted automatically by OpenMP right here ensures all threads wait until the single thread finishes resetting

            // Distribute the loop iterations across multiple hardware threads in parallel, automatically reducing local swap counts into the shared variable using addition
#pragma omp for reduction(+:swaps_in_this_phase) schedule(static)
            for (long long worker_id = 0; worker_id < (long long)number_of_workers; worker_id++) {

                // Calculate the exact array index for the first element of the pair using the current rotation offset and worker ID multiplied by 2
                size_t first_index_of_pair = (current_rotation_offset + 2 * (size_t)worker_id) % number_of_elements;

                // Calculate the exact array index for the second element of the pair as the immediate next neighbor, wrapping around via modulo if needed
                size_t second_index_of_pair = (first_index_of_pair + 1) % number_of_elements;

                // Call the helper function to compare elements at these two indices, swap them if out of order, and add the result (0 or 1) to the reduction accumulator
                swaps_in_this_phase += compare_exchange(array_to_sort, first_index_of_pair, second_index_of_pair);
            } // Implicit thread barrier inserted automatically here ensures all worker threads finish their comparisons before moving forward

            // Designate precisely one single thread to evaluate the final phase results and update the global tracking state variables safely without race conditions
#pragma omp single
            {
                // Increment the total phases executed tracking counter by exactly one
                total_phases_executed++;

                // Check if the total number of swaps recorded in this specific phase equals zero
                if (swaps_in_this_phase == 0)
                    consecutive_phases_with_zero_swaps++; // If zero swaps occurred, increment our consecutive zero-swap streak counter by one
                else
                    consecutive_phases_with_zero_swaps = 0; // If any swaps occurred, reset the consecutive zero-swap streak counter entirely back to zero

                // Check if our accumulated zero-swap streak has reached or exceeded the required mathematical threshold to prove the array is completely sorted
                if (consecutive_phases_with_zero_swaps >= number_of_phases_needed_to_prove_sorted) {
                    // If the threshold is met, set the shared control flag to 0 so all threads know it is finally time to exit the loop
                    sorting_not_finished = 0;
                }
                else {
                    // If the threshold is not met yet, advance the rotation offset position forward by 1 for the next phase, wrapping around via modulo
                    current_rotation_offset = (current_rotation_offset + 1) % number_of_elements;
                }
            }

            // Force all threads to halt, synchronize, and wait here until the single thread block finishes updating the control flags and the offset variable
#pragma omp barrier
        } // Jump back to the top of the while loop; if sorting_not_finished became 0, all threads break out of the parallel loop together cleanly
    } // End of the OpenMP parallel region; all child worker threads join back into the main execution thread here
#else
    // Fallback sequential execution path if OpenMP is not defined or supported by the compiler
    // Loop continuously one step at a time until our consecutive zero-swap streak reaches the required mathematical threshold
    while (consecutive_phases_with_zero_swaps < number_of_phases_needed_to_prove_sorted) {

        // Initialize a local phase swap counter variable to zero for this single sequential iteration
        unsigned long long swaps_in_this_phase = 0;

        // Loop sequentially through all worker pair positions one by one using a standard single-threaded for loop
        for (size_t worker_id = 0; worker_id < number_of_workers; worker_id++) {

            // Calculate the first index of the pair for this sequential worker iteration
            size_t first_index_of_pair = (current_rotation_offset + 2 * worker_id) % number_of_elements;

            // Calculate the second index of terms sequentially, wrapping around with modulo if it hits the end of the array
            size_t second_index_of_pair = (first_index_of_pair + 1) % number_of_elements;

            // Compare and exchange the array elements sequentially, adding any resulting swap count to our phase counter
            swaps_in_this_phase += compare_exchange(array_to_sort, first_index_of_pair, second_index_of_pair);
        }

        // Increment the total phases executed counter by one after finishing this sequential pass
        total_phases_executed++;

        // Check if zero swaps occurred during this sequential phase
        if (swaps_in_this_phase == 0)
            consecutive_phases_with_zero_swaps++; // Increment the streak counter if no swaps happened
        else
            consecutive_phases_with_zero_swaps = 0; // Reset the streak counter back to zero if any swaps happened

        // If we still haven't met the required threshold to prove the array is fully sorted, advance the rotation offset by 1 position
        if (consecutive_phases_with_zero_swaps < number_of_phases_needed_to_prove_sorted) {
            current_rotation_offset = (current_rotation_offset + 1) % number_of_elements;
        }
    }
#endif // End of the OpenMP conditional compilation block

    // Return the final accumulated count of total sorting phases that were executed before the array achieved a fully sorted state
    return total_phases_executed;
}