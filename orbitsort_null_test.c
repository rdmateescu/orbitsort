#include <stdio.h>

#include "orbitsort.c"

static int get_error_code_name(
    int error_code,
    const char** error_code_name)
{
    int result_code = ORBITSORT_SUCCESS;

    if (error_code_name == NULL) {
        result_code = ORBITSORT_ERROR_NULL_POINTER;
    }
    else if (error_code == ORBITSORT_SUCCESS) {
        *error_code_name = "ORBITSORT_SUCCESS";
    }
    else if (error_code == ORBITSORT_ERROR_NULL_POINTER) {
        *error_code_name = "ORBITSORT_ERROR_NULL_POINTER";
    }
    else if (error_code == ORBITSORT_ERROR_ZERO_ELEMENTS) {
        *error_code_name = "ORBITSORT_ERROR_ZERO_ELEMENTS";
    }
    else if (error_code == ORBITSORT_ERROR_INDEX_OUT_OF_RANGE) {
        *error_code_name = "ORBITSORT_ERROR_INDEX_OUT_OF_RANGE";
    }
    else if (error_code == ORBITSORT_ERROR_TOO_MANY_ELEMENTS) {
        *error_code_name = "ORBITSORT_ERROR_TOO_MANY_ELEMENTS";
    }
    else if (error_code == ORBITSORT_ERROR_INVALID_LOG_TYPE) {
        *error_code_name = "ORBITSORT_ERROR_INVALID_LOG_TYPE";
    }
    else if (error_code == ORBITSORT_ERROR_INVALID_ERROR_CODE) {
        *error_code_name = "ORBITSORT_ERROR_INVALID_ERROR_CODE";
    }
    else if (error_code == ORBITSORT_ERROR_INVALID_LOG_LEVEL) {
        *error_code_name = "ORBITSORT_ERROR_INVALID_LOG_LEVEL";
    }
    else {
        *error_code_name = "UNKNOWN_ERROR_CODE";
    }

    return result_code;
}

static int verify_expected_error(
    const char* test_name,
    int expected_error_code,
    int received_error_code)
{
    int test_result = 0;
    const char* expected_error_code_name = NULL;
    const char* received_error_code_name = NULL;

    if (test_name == NULL) {
        test_result = 1;
    }
    else {
        (void)get_error_code_name(
            expected_error_code,
            &expected_error_code_name);
        (void)get_error_code_name(
            received_error_code,
            &received_error_code_name);

        if (received_error_code != expected_error_code) {
            fprintf(
                stderr,
                "%s failed: expected %s (%d), received %s (%d).\n",
                test_name,
                expected_error_code_name,
                expected_error_code,
                received_error_code_name,
                received_error_code);
            test_result = 1;
        }
        else {
            fprintf(
                stdout,
                "%s passed: received %s (%d).\n",
                test_name,
                received_error_code_name,
                received_error_code);
        }
    }

    return test_result;
}

int main(void)
{
    int test_result = 0;
    int error_code = ORBITSORT_SUCCESS;
    unsigned long long total_phases_executed = 0;
    int values[] = { 2, 1 };
    int was_exchanged = 0;

    error_code = run_until_sorted(
        NULL,
        1,
        &total_phases_executed);

    test_result |= verify_expected_error(
        "Null-array test",
        ORBITSORT_ERROR_NULL_POINTER,
        error_code);

    error_code = run_until_sorted(
        values,
        2,
        NULL);

    test_result |= verify_expected_error(
        "Null-result test",
        ORBITSORT_ERROR_NULL_POINTER,
        error_code);

    error_code = run_until_sorted(
        values,
        0,
        &total_phases_executed);

    test_result |= verify_expected_error(
        "Zero-elements test",
        ORBITSORT_ERROR_ZERO_ELEMENTS,
        error_code);

    error_code = compare_exchange(
        values,
        2,
        2,
        0,
        &was_exchanged);

    test_result |= verify_expected_error(
        "First-index-out-of-range test",
        ORBITSORT_ERROR_INDEX_OUT_OF_RANGE,
        error_code);

    error_code = compare_exchange(
        values,
        2,
        0,
        2,
        &was_exchanged);

    test_result |= verify_expected_error(
        "Second-index-out-of-range test",
        ORBITSORT_ERROR_INDEX_OUT_OF_RANGE,
        error_code);

    error_code = compare_exchange(
        values,
        2,
        0,
        1,
        NULL);

    test_result |= verify_expected_error(
        "Null-exchange-result test",
        ORBITSORT_ERROR_NULL_POINTER,
        error_code);

    error_code = compare_exchange(
        NULL,
        2,
        0,
        1,
        &was_exchanged);

    test_result |= verify_expected_error(
        "Null-exchange-array test",
        ORBITSORT_ERROR_NULL_POINTER,
        error_code);

    error_code = compare_exchange(
        values,
        0,
        0,
        0,
        &was_exchanged);

    test_result |= verify_expected_error(
        "Exchange-zero-elements test",
        ORBITSORT_ERROR_ZERO_ELEMENTS,
        error_code);

    error_code = compare_int(
        values[0],
        values[1],
        NULL);

    test_result |= verify_expected_error(
        "Null-comparison-result test",
        ORBITSORT_ERROR_NULL_POINTER,
        error_code);

    error_code = min_int(
        values[0],
        values[1],
        NULL);

    test_result |= verify_expected_error(
        "Null-minimum-result test",
        ORBITSORT_ERROR_NULL_POINTER,
        error_code);

    error_code = max_int(
        values[0],
        values[1],
        NULL);

    test_result |= verify_expected_error(
        "Null-maximum-result test",
        ORBITSORT_ERROR_NULL_POINTER,
        error_code);

    return test_result;
}
