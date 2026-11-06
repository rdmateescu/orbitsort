#include <stddef.h>
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>

/**
 * =================================================================================================
 * FILE HEADER: ORBITSORT
 * =================================================================================================
 *
 * PURPOSE
 * -------
 * This file demonstrates OrbitSort, a rotating-pair sorting algorithm for an array of integers.
 * The array is sorted in place, which means that the original array memory is directly modified
 * instead of creating a second sorted array.
 *
 * BASIC IDEA
 * ----------
 * The array is viewed as a circle. During every phase, the algorithm creates disjoint neighboring
 * pairs. Each pair is compared and, when necessary, exchanged so that the smaller value occupies
 * the lower array index and the larger value occupies the higher array index.
 *
 * For an array containing n elements, a phase uses floor(n / 2) pairs. The first pair starts at the
 * current rotation offset, and every following pair starts two positions later. The pair at the
 * end of the circle wraps around from index n - 1 to index 0.
 *
 * After a phase is complete, the rotation offset advances by one position. This causes the next
 * phase to compare the neighboring positions that were not compared in the previous phase.
 * Because the pairs in one phase never share an index, all pairs in that phase can be processed in
 * parallel without two workers modifying the same array element.
 *
 * WRAP-AROUND PAIR
 * ----------------
 * The pair (n - 1, 0) is canonicalized before comparison. Index 0 is treated as the lower index,
 * and index n - 1 is treated as the higher index. Therefore, the smaller value always moves to the
 * lower numerical index, even for the pair that crosses the end of the array.
 *
 * WHEN SORTING STOPS
 * ------------------
 * The algorithm counts consecutive phases that perform no exchanges. For an even-sized array,
 * two consecutive zero-exchange phases cover every circular neighboring pair. For an odd-sized
 * array, three consecutive zero-exchange phases cover every circular neighboring pair. When that
 * condition is reached, every neighboring pair is in the required order and the array is sorted.
 *
 * RELATIONSHIP TO OTHER SORTING ALGORITHMS
 * ----------------------------------------
 * OrbitSort resembles odd-even transposition sort and compare-exchange sorting networks because it
 * repeatedly compares disjoint neighboring pairs. It is not exactly the same algorithm, however:
 *
 * 1. Odd-even transposition sort normally alternates fixed even and odd linear pair layouts.
 * 2. OrbitSort rotates one circular pair layout by one position after every phase.
 * 3. OrbitSort explicitly includes the circular wrap-around pair (n - 1, 0).
 * 4. OrbitSort canonicalizes the wrap-around pair by numerical index before exchanging values.
 * 5. OrbitSort uses consecutive zero-exchange phases as its stopping proof.
 *
 * Therefore, OrbitSort shares the compare-exchange idea with related algorithms, but its rotating
 * circular pairing, wrap-around handling, and stopping condition make it a distinct demonstration.
 * It should not be described as identical to odd-even transposition sort or any other existing
 * sorting algorithm.
 *
 * PARALLEL AND SEQUENTIAL EXECUTION
 * ---------------------------------
 * When compiled with OpenMP enabled, each phase distributes its disjoint pairs among worker
 * threads. When OpenMP is unavailable, the same phases and pair order are executed sequentially.
 * Both execution paths use the same comparison and exchange rules.
 *
 * ERROR HANDLING
 * --------------
 * Functions do not use C++ exceptions because this is a C implementation. They return zero for
 * success or a documented negative error code for failure. Calculated results are returned through
 * output parameters. Functions use one final return statement and do not use early returns.
 *
 * LOGGING
 * -------
 * The logging functions are placed before the sorting functions. They provide function-entry,
 * function-exit, error, and normal-message logging through one display function. The display
 * function is the single place where output behavior can be changed.
 *
 * =================================================================================================
 */

#ifdef _OPENMP
#include <omp.h>
#endif

/**
 * ERROR CODES:
 * -------------
 *  0: The operation completed successfully.
 * -1: A required pointer parameter was NULL.
 * -2: The array contains zero elements.
 * -3: An array index is outside the valid array range.
 * -4: The array is too large for the OpenMP loop counter.
 * -5: A log type is invalid.
 * -6: A reported error code is not negative.
 */
#define ORBITSORT_SUCCESS 0
#define ORBITSORT_ERROR_NULL_POINTER -1
#define ORBITSORT_ERROR_ZERO_ELEMENTS -2
#define ORBITSORT_ERROR_INDEX_OUT_OF_RANGE -3
#define ORBITSORT_ERROR_TOO_MANY_ELEMENTS -4
#define ORBITSORT_ERROR_INVALID_LOG_TYPE -5
#define ORBITSORT_ERROR_INVALID_ERROR_CODE -6

typedef enum
{
    ORBITSORT_LOG_FUNCTION_ENTER = 0,
    ORBITSORT_LOG_FUNCTION_EXIT = 1,
    ORBITSORT_LOG_ERROR = 2,
    ORBITSORT_LOG_MESSAGE = 3
} orbitsort_log_type;

/**
 * FUNCTION: display_log_message
 *
 * PARAMETERS:
 * - log_type: Type of message to display.
 * - function_name: Name of the function associated with the message.
 * - message: Text to display.
 *
 * RETURNS:
 *  0: The message was displayed successfully.
 * -1: function_name or message was NULL.
 * -5: log_type was invalid.
 */
static int display_log_message(
    orbitsort_log_type log_type,
    const char* function_name,
    const char* message)
{
    int error_code = ORBITSORT_SUCCESS;

    if (function_name == NULL || message == NULL) {
        error_code = ORBITSORT_ERROR_NULL_POINTER;
    }
    else if (log_type < ORBITSORT_LOG_FUNCTION_ENTER ||
        log_type > ORBITSORT_LOG_MESSAGE) {
        error_code = ORBITSORT_ERROR_INVALID_LOG_TYPE;
    }
    else {
#ifdef _OPENMP
#pragma omp critical(orbitsort_logging)
        {
#endif
            const char* log_label = "MESSAGE";

            if (log_type == ORBITSORT_LOG_FUNCTION_ENTER) {
                log_label = "ENTER";
            }
            else if (log_type == ORBITSORT_LOG_FUNCTION_EXIT) {
                log_label = "EXIT";
            }
            else if (log_type == ORBITSORT_LOG_ERROR) {
                log_label = "ERROR";
            }

            fprintf(
                stderr,
                "[ORBITSORT][%s][%s] %s\n",
                log_label,
                function_name,
                message);
#ifdef _OPENMP
        }
#endif
    }

    return error_code;
}

/**
 * FUNCTION: log_function_enter
 *
 * PARAMETERS:
 * - function_name: Name of the function being entered.
 *
 * RETURNS:
 *  0: The entry message was displayed successfully.
 * -1: function_name was NULL.
 */
static int log_function_enter(const char* function_name)
{
    int error_code = ORBITSORT_SUCCESS;

    if (function_name == NULL) {
        error_code = ORBITSORT_ERROR_NULL_POINTER;
    }
    else {
        error_code = display_log_message(
            ORBITSORT_LOG_FUNCTION_ENTER,
            function_name,
            "Function entered.");
    }

    return error_code;
}

/**
 * FUNCTION: log_function_exit
 *
 * PARAMETERS:
 * - function_name: Name of the function being exited.
 *
 * RETURNS:
 *  0: The exit message was displayed successfully.
 * -1: function_name was NULL.
 */
static int log_function_exit(const char* function_name)
{
    int error_code = ORBITSORT_SUCCESS;

    if (function_name == NULL) {
        error_code = ORBITSORT_ERROR_NULL_POINTER;
    }
    else {
        error_code = display_log_message(
            ORBITSORT_LOG_FUNCTION_EXIT,
            function_name,
            "Function exited.");
    }

    return error_code;
}

/**
 * FUNCTION: log_error
 *
 * PARAMETERS:
 * - function_name: Name of the function reporting the error.
 * - error_code: Negative error code being reported.
 * - message: Explanation of the error.
 *
 * RETURNS:
 *  0: The error message was displayed successfully.
 * -1: function_name or message was NULL.
 * -6: error_code was not negative.
 */
static int log_error(
    const char* function_name,
    int error_code,
    const char* message)
{
    int logging_error_code = ORBITSORT_SUCCESS;

    if (function_name == NULL || message == NULL) {
        logging_error_code = ORBITSORT_ERROR_NULL_POINTER;
    }
    else if (error_code >= ORBITSORT_SUCCESS) {
        logging_error_code = ORBITSORT_ERROR_INVALID_ERROR_CODE;
    }
    else {
        logging_error_code = display_log_message(
            ORBITSORT_LOG_ERROR,
            function_name,
            message);
    }

    return logging_error_code;
}

/**
 * FUNCTION: log_message
 *
 * PARAMETERS:
 * - function_name: Name of the function reporting the message.
 * - message: Normal informational text to display.
 *
 * RETURNS:
 *  0: The normal message was displayed successfully.
 * -1: function_name or message was NULL.
 */
static int log_message(
    const char* function_name,
    const char* message)
{
    int error_code = ORBITSORT_SUCCESS;

    if (function_name == NULL || message == NULL) {
        error_code = ORBITSORT_ERROR_NULL_POINTER;
    }
    else {
        error_code = display_log_message(
            ORBITSORT_LOG_MESSAGE,
            function_name,
            message);
    }

    return error_code;
}

/**
 * FUNCTION: compare_int
 *
 * PARAMETERS:
 * - first_value: First integer value to compare.
 * - second_value: Second integer value to compare.
 * - comparison_result: Output pointer receiving -1 when first_value is
 *                      smaller, 0 when both values are equal, or 1 when
 *                      first_value is larger.
 *
 * RETURNS:
 *  0: The comparison completed successfully.
 * -1: comparison_result was NULL.
 */
static int compare_int(
    int first_value,
    int second_value,
    int* comparison_result)
{
    int error_code = ORBITSORT_SUCCESS;

    (void)log_function_enter("compare_int");

    if (comparison_result == NULL) {
        error_code = ORBITSORT_ERROR_NULL_POINTER;
    }
    else if (first_value < second_value) {
        *comparison_result = -1;
    }
    else if (first_value > second_value) {
        *comparison_result = 1;
    }
    else {
        *comparison_result = 0;
    }

    if (error_code != ORBITSORT_SUCCESS) {
        (void)log_error(
            "compare_int",
            error_code,
            "The comparison result pointer was NULL.");
    }

    (void)log_function_exit("compare_int");

    return error_code;
}

/**
 * FUNCTION: min_int
 *
 * PARAMETERS:
 * - first_value: First integer value to compare.
 * - second_value: Second integer value to compare.
 * - minimum_value: Output pointer receiving the smaller value.
 *
 * RETURNS:
 *  0: The minimum value was written successfully.
 * -1: minimum_value was NULL.
 */
static int min_int(
    int first_value,
    int second_value,
    int* minimum_value)
{
    int comparison_result = 0;
    int error_code = ORBITSORT_SUCCESS;

    (void)log_function_enter("min_int");

    if (minimum_value == NULL) {
        error_code = ORBITSORT_ERROR_NULL_POINTER;
    }
    else {
        error_code = compare_int(
            first_value,
            second_value,
            &comparison_result);
    }

    if (error_code == ORBITSORT_SUCCESS) {
        *minimum_value = comparison_result <= 0
            ? first_value
            : second_value;
    }

    if (error_code != ORBITSORT_SUCCESS) {
        (void)log_error(
            "min_int",
            error_code,
            "The minimum-value output pointer was NULL.");
    }

    (void)log_function_exit("min_int");

    return error_code;
}

/**
 * FUNCTION: max_int
 *
 * PARAMETERS:
 * - first_value: First integer value to compare.
 * - second_value: Second integer value to compare.
 * - maximum_value: Output pointer receiving the larger value.
 *
 * RETURNS:
 *  0: The maximum value was written successfully.
 * -1: maximum_value was NULL.
 */
static int max_int(
    int first_value,
    int second_value,
    int* maximum_value)
{
    int comparison_result = 0;
    int error_code = ORBITSORT_SUCCESS;

    (void)log_function_enter("max_int");

    if (maximum_value == NULL) {
        error_code = ORBITSORT_ERROR_NULL_POINTER;
    }
    else {
        error_code = compare_int(
            first_value,
            second_value,
            &comparison_result);
    }

    if (error_code == ORBITSORT_SUCCESS) {
        *maximum_value = comparison_result >= 0
            ? first_value
            : second_value;
    }

    if (error_code != ORBITSORT_SUCCESS) {
        (void)log_error(
            "max_int",
            error_code,
            "The maximum-value output pointer was NULL.");
    }

    (void)log_function_exit("max_int");

    return error_code;
}

/**
 * FUNCTION: compare_exchange
 *
 * PARAMETERS:
 * - array_to_compare: Mutable array containing the values to compare.
 * - number_of_elements: Number of elements in array_to_compare.
 * - first_index: First array index.
 * - second_index: Second array index.
 * - was_exchanged: Output pointer receiving 1 when values were exchanged,
 *                  or 0 when they were already ordered.
 *
 * RETURNS:
 *  0: The comparison completed successfully.
 * -1: array_to_compare or was_exchanged was NULL.
 * -2: number_of_elements was zero.
 * -3: An array index was outside the valid array range.
 */
static int compare_exchange(
    int* array_to_compare,
    size_t number_of_elements,
    size_t first_index,
    size_t second_index,
    int* was_exchanged)
{
    int error_code = ORBITSORT_SUCCESS;

    (void)log_function_enter("compare_exchange");

    if (array_to_compare == NULL || was_exchanged == NULL) {
        error_code = ORBITSORT_ERROR_NULL_POINTER;
    }
    else if (number_of_elements == 0) {
        error_code = ORBITSORT_ERROR_ZERO_ELEMENTS;
    }
    else if (first_index >= number_of_elements ||
        second_index >= number_of_elements) {
        error_code = ORBITSORT_ERROR_INDEX_OUT_OF_RANGE;
    }
    else {
        *was_exchanged = 0;

        size_t lower_index = first_index < second_index
            ? first_index
            : second_index;
        size_t higher_index = first_index < second_index
            ? second_index
            : first_index;
        int value_at_lower_index = array_to_compare[lower_index];
        int value_at_higher_index = array_to_compare[higher_index];
        int comparison_result = 0;

        error_code = compare_int(
            value_at_lower_index,
            value_at_higher_index,
            &comparison_result);

        if (error_code == ORBITSORT_SUCCESS && comparison_result > 0) {
            int minimum_value = 0;
            int maximum_value = 0;

            error_code = min_int(
                value_at_lower_index,
                value_at_higher_index,
                &minimum_value);

            if (error_code == ORBITSORT_SUCCESS) {
                error_code = max_int(
                    value_at_lower_index,
                    value_at_higher_index,
                    &maximum_value);
            }

            if (error_code == ORBITSORT_SUCCESS) {
                array_to_compare[lower_index] = minimum_value;
                array_to_compare[higher_index] = maximum_value;
                *was_exchanged = 1;
            }
        }
    }

    if (error_code != ORBITSORT_SUCCESS) {
        (void)log_error(
            "compare_exchange",
            error_code,
            "The pair could not be compared or exchanged.");
    }

    (void)log_function_exit("compare_exchange");

    return error_code;
}

/**
 * FUNCTION: run_until_sorted
 *
 * PARAMETERS:
 * - array_to_sort: Mutable array to sort in ascending order.
 * - number_of_elements: Number of elements in array_to_sort.
 * - total_phases_executed: Output pointer receiving the number of phases.
 *
 * RETURNS:
 *  0: The array was sorted successfully.
 * -1: array_to_sort or total_phases_executed was NULL.
 * -2: number_of_elements was zero.
 * -3: An internally generated array index was invalid.
 * -4: The array is too large for the OpenMP loop counter.
 */
static int run_until_sorted(
    int* array_to_sort,
    size_t number_of_elements,
    unsigned long long* total_phases_executed)
{
    int error_code = ORBITSORT_SUCCESS;

    (void)log_function_enter("run_until_sorted");

    if (array_to_sort == NULL || total_phases_executed == NULL) {
        error_code = ORBITSORT_ERROR_NULL_POINTER;
    }
    else if (number_of_elements == 0) {
        error_code = ORBITSORT_ERROR_ZERO_ELEMENTS;
    }
    else {
        *total_phases_executed = 0;
        (void)log_message(
            "run_until_sorted",
            "Sorting started.");

        size_t number_of_workers = number_of_elements / 2;

        if (number_of_workers > (size_t)PTRDIFF_MAX) {
            error_code = ORBITSORT_ERROR_TOO_MANY_ELEMENTS;
        }

        if (error_code == ORBITSORT_SUCCESS) {
            size_t number_of_phases_needed_to_prove_sorted =
                number_of_elements % 2 == 0 ? 2 : 3;
            size_t consecutive_phases_with_zero_swaps = 0;
            size_t current_rotation_offset = 0;

#ifdef _OPENMP
            int sorting_not_finished = 1;
            int sorting_error_code = ORBITSORT_SUCCESS;
            unsigned long long swaps_in_this_phase = 0;

#pragma omp parallel shared(array_to_sort, number_of_elements, \
    number_of_workers, number_of_phases_needed_to_prove_sorted, \
    total_phases_executed, consecutive_phases_with_zero_swaps, \
    current_rotation_offset, sorting_not_finished, \
    sorting_error_code, swaps_in_this_phase)
            {
                while (sorting_not_finished) {
#pragma omp single
                    {
                        swaps_in_this_phase = 0;
                        sorting_error_code = ORBITSORT_SUCCESS;
                    }

#pragma omp barrier

#pragma omp for reduction(+:swaps_in_this_phase) schedule(static)
                    for (ptrdiff_t worker_id = 0;
                         worker_id < (ptrdiff_t)number_of_workers;
                         worker_id++) {
                        size_t pair_offset =
                            (2 * (size_t)worker_id) % number_of_elements;
                        size_t first_index_of_pair =
                            current_rotation_offset >=
                                number_of_elements - pair_offset
                                ? current_rotation_offset -
                                    (number_of_elements - pair_offset)
                                : current_rotation_offset + pair_offset;
                        size_t second_index_of_pair =
                            first_index_of_pair + 1 == number_of_elements
                                ? 0
                                : first_index_of_pair + 1;
                        int was_exchanged = 0;
                        int phase_error_code = compare_exchange(
                            array_to_sort,
                            number_of_elements,
                            first_index_of_pair,
                            second_index_of_pair,
                            &was_exchanged);

                        if (phase_error_code != ORBITSORT_SUCCESS) {
#pragma omp critical
                            {
                                if (sorting_error_code == ORBITSORT_SUCCESS) {
                                    sorting_error_code = phase_error_code;
                                }
                            }
                        }
                        else {
                            swaps_in_this_phase +=
                                (unsigned long long)was_exchanged;
                        }
                    }

#pragma omp single
                    {
                        if (sorting_error_code != ORBITSORT_SUCCESS) {
                            sorting_not_finished = 0;
                        }
                        else {
                            (*total_phases_executed)++;

                            if (swaps_in_this_phase == 0) {
                                consecutive_phases_with_zero_swaps++;
                            }
                            else {
                                consecutive_phases_with_zero_swaps = 0;
                            }

                            if (consecutive_phases_with_zero_swaps >=
                                number_of_phases_needed_to_prove_sorted) {
                                sorting_not_finished = 0;
                            }
                            else {
                                current_rotation_offset =
                                    current_rotation_offset + 1 ==
                                        number_of_elements
                                        ? 0
                                        : current_rotation_offset + 1;
                            }
                        }
                    }

#pragma omp barrier
                }
            }

            error_code = sorting_error_code;
#else
            while (error_code == ORBITSORT_SUCCESS &&
                consecutive_phases_with_zero_swaps <
                    number_of_phases_needed_to_prove_sorted) {
                unsigned long long swaps_in_this_phase = 0;

                for (size_t worker_id = 0;
                     worker_id < number_of_workers &&
                         error_code == ORBITSORT_SUCCESS;
                     worker_id++) {
                    size_t pair_offset =
                        (2 * worker_id) % number_of_elements;
                    size_t first_index_of_pair =
                        current_rotation_offset >=
                            number_of_elements - pair_offset
                            ? current_rotation_offset -
                                (number_of_elements - pair_offset)
                            : current_rotation_offset + pair_offset;
                    size_t second_index_of_pair =
                        first_index_of_pair + 1 == number_of_elements
                            ? 0
                            : first_index_of_pair + 1;
                    int was_exchanged = 0;
                    int phase_error_code = compare_exchange(
                        array_to_sort,
                        number_of_elements,
                        first_index_of_pair,
                        second_index_of_pair,
                        &was_exchanged);

                    if (phase_error_code != ORBITSORT_SUCCESS) {
                        error_code = phase_error_code;
                    }
                    else {
                        swaps_in_this_phase +=
                            (unsigned long long)was_exchanged;
                    }
                }

                if (error_code == ORBITSORT_SUCCESS) {
                    (*total_phases_executed)++;

                    if (swaps_in_this_phase == 0) {
                        consecutive_phases_with_zero_swaps++;
                    }
                    else {
                        consecutive_phases_with_zero_swaps = 0;
                    }

                    if (consecutive_phases_with_zero_swaps <
                        number_of_phases_needed_to_prove_sorted) {
                        current_rotation_offset =
                            current_rotation_offset + 1 == number_of_elements
                                ? 0
                                : current_rotation_offset + 1;
                    }
                }
            }
#endif
        }
    }

    if (error_code != ORBITSORT_SUCCESS) {
        (void)log_error(
            "run_until_sorted",
            error_code,
            "Sorting could not be completed.");
    }
    else {
        (void)log_message(
            "run_until_sorted",
            "Sorting completed successfully.");
    }

    (void)log_function_exit("run_until_sorted");

    return error_code;
}
