#ifndef ORBITSORT_INTERNAL_H
#define ORBITSORT_INTERNAL_H

#include <stddef.h>

/*
 * Error codes shared by the original and refactored sorting files.
 * Zero means success. Negative values identify specific failures.
 */
#define ORBITSORT_SUCCESS 0
#define ORBITSORT_ERROR_NULL_POINTER -1
#define ORBITSORT_ERROR_ZERO_ELEMENTS -2
#define ORBITSORT_ERROR_INDEX_OUT_OF_RANGE -3
#define ORBITSORT_ERROR_TOO_MANY_ELEMENTS -4
#define ORBITSORT_ERROR_INVALID_LOG_TYPE -5
#define ORBITSORT_ERROR_INVALID_ERROR_CODE -6
#define ORBITSORT_ERROR_INVALID_LOG_LEVEL -7
#define ORBITSORT_ERROR_MEMORY_ALLOCATION -8
/*
*/
#define ORBITSORT_SUCCESS_MESSAGE "The function completed successfully."


/* Logging levels used by the shared logging functions. */
typedef enum
{
    ORBITSORT_LOG_LEVEL_NONE = -1,
    ORBITSORT_LOG_LEVEL_ERROR = 0,
    ORBITSORT_LOG_LEVEL_IMPORTANT = 1,
    ORBITSORT_LOG_LEVEL_DEBUG = 2,
    ORBITSORT_LOG_LEVEL_TRACE = 3
} orbitsort_log_level;

/* Types used by the shared logging functions. */
typedef enum
{
    ORBITSORT_LOG_FUNCTION_ENTER = 0,
    ORBITSORT_LOG_FUNCTION_EXIT = 1,
    ORBITSORT_LOG_ERROR = 2,
    ORBITSORT_LOG_MESSAGE = 3
} orbitsort_log_type;

/*
 * Shared pointer and comparison helpers implemented in orbitsort.c.
 */
int validate_required_pointer(const void* pointer);

int validate_required_pointers(
    const void* first_pointer,
    const void* second_pointer);

int compare_int(
    int first_value,
    int second_value,
    int* comparison_result);

int min_int(
    int first_value,
    int second_value,
    int* minimum_value);

int max_int(
    int first_value,
    int second_value,
    int* maximum_value);

int compare_exchange(
    int* array_to_compare,
    size_t number_of_elements,
    size_t first_index,
    size_t second_index,
    int* was_exchanged);

/*
 * Logging functions implemented in orbitsort_logging.c.
 */
int set_log_level(orbitsort_log_level log_level);

orbitsort_log_level get_log_level(void);

int display_log_message(
    orbitsort_log_level log_level,
    orbitsort_log_type log_type,
    const char* function_name,
    const char* message);

int log_function_enter(
    orbitsort_log_level log_level,
    const char* function_name);

int log_function_exit(
    orbitsort_log_level log_level,
    const char* function_name,
    const char* exit_reason);

int log_error(
    const char* function_name,
    int error_code,
    const char* message);

int log_message(
    const char* function_name,
    const char* message);

#endif
