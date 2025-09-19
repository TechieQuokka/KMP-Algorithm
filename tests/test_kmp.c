#include "../include/kmp.h"
#include <assert.h>

typedef struct {
    char* pattern;
    char* text;
    int expected_position;
    char* description;
} TestCase;

typedef struct {
    char* pattern;
    char* text;
    int* expected_positions;
    int expected_count;
    char* description;
} MultiTestCase;

int test_count = 0;
int test_passed = 0;

void run_test(const char* test_name, bool condition) {
    test_count++;
    if (condition) {
        test_passed++;
        printf("PASS: %s\n", test_name);
    } else {
        printf("FAIL: %s\n", test_name);
    }
}

void test_lps_computation() {
    printf("\n=== Testing LPS Computation ===\n");

    struct {
        char* pattern;
        int* expected_lps;
        int len;
    } lps_tests[] = {
        {"ABABCAB", (int[]){0, 0, 1, 2, 0, 1, 2}, 7},
        {"AAAA", (int[]){0, 1, 2, 3}, 4},
        {"ABCDE", (int[]){0, 0, 0, 0, 0}, 5},
        {"ABAB", (int[]){0, 0, 1, 2}, 4},
        {"A", (int[]){0}, 1}
    };

    for (int i = 0; i < 5; i++) {
        int* lps = compute_lps_table(lps_tests[i].pattern, lps_tests[i].len);
        bool passed = true;

        if (lps) {
            for (int j = 0; j < lps_tests[i].len; j++) {
                if (lps[j] != lps_tests[i].expected_lps[j]) {
                    passed = false;
                    break;
                }
            }
            free(lps);
        } else {
            passed = false;
        }

        char test_name[100];
        sprintf(test_name, "LPS for pattern '%s'", lps_tests[i].pattern);
        run_test(test_name, passed);
    }
}

void test_basic_search() {
    printf("\n=== Testing Basic Search ===\n");

    TestCase basic_tests[] = {
        {"ABC", "ABCABCABC", 0, "Simple pattern at beginning"},
        {"ABC", "XYZABC", 3, "Simple pattern at end"},
        {"ABC", "XABCYZ", 1, "Simple pattern in middle"},
        {"ABC", "XYZDEF", -1, "Pattern not found"},
        {"ABABCAB", "ABABDABACDABABCABCABCABCABC", 10, "Complex pattern"},
        {"A", "BANANA", 1, "Single character pattern"},
        {"TEST", "TEST", 0, "Pattern equals text"},
        {"LONG", "SHORT", -1, "Pattern longer than text"}
    };

    int num_tests = sizeof(basic_tests) / sizeof(TestCase);

    for (int i = 0; i < num_tests; i++) {
        KMPMatcher* matcher = kmp_create(basic_tests[i].pattern);
        bool passed = false;

        if (matcher) {
            int position = kmp_search(matcher, basic_tests[i].text);
            passed = (position == basic_tests[i].expected_position);
            kmp_destroy(matcher);
        }

        run_test(basic_tests[i].description, passed);
    }
}

void test_multiple_search() {
    printf("\n=== Testing Multiple Search ===\n");

    MultiTestCase multi_tests[] = {
        {"AB", "ABABAB", (int[]){0, 2, 4}, 3, "Overlapping patterns"},
        {"ABC", "ABCABCABC", (int[]){0, 3, 6}, 3, "Non-overlapping patterns"},
        {"A", "BANANA", (int[]){1, 3, 5}, 3, "Single character multiple matches"},
        {"XYZ", "ABCDEF", NULL, 0, "No matches"},
        {"TEST", "TESTTEST", (int[]){0, 4}, 2, "Adjacent patterns"}
    };

    int num_tests = sizeof(multi_tests) / sizeof(MultiTestCase);

    for (int i = 0; i < num_tests; i++) {
        KMPMatcher* matcher = kmp_create(multi_tests[i].pattern);
        bool passed = false;

        if (matcher) {
            int count;
            int* positions = kmp_search_all(matcher, multi_tests[i].text, &count);

            if (count == multi_tests[i].expected_count) {
                if (count == 0) {
                    passed = (positions == NULL);
                } else {
                    passed = true;
                    for (int j = 0; j < count; j++) {
                        if (positions[j] != multi_tests[i].expected_positions[j]) {
                            passed = false;
                            break;
                        }
                    }
                }
            }

            if (positions) {
                free(positions);
            }
            kmp_destroy(matcher);
        }

        run_test(multi_tests[i].description, passed);
    }
}

void test_edge_cases() {
    printf("\n=== Testing Edge Cases ===\n");

    run_test("NULL pattern", kmp_create(NULL) == NULL);
    run_test("Empty pattern", kmp_create("") == NULL);

    KMPMatcher* matcher = kmp_create("TEST");
    if (matcher) {
        run_test("NULL text search", kmp_search(matcher, NULL) == -1);
        run_test("Empty text search", kmp_search(matcher, "") == -1);
        kmp_destroy(matcher);
    }

    run_test("Non-ASCII pattern", kmp_create("\xFF\xFE") == NULL);

    matcher = kmp_create("ABC");
    if (matcher) {
        run_test("Non-ASCII text", kmp_search(matcher, "AB\xFF") == -1);
        kmp_destroy(matcher);
    }
}

void test_memory_management() {
    printf("\n=== Testing Memory Management ===\n");

    const char* pattern = "TESTPATTERN";
    KMPMatcher* matcher = kmp_create(pattern);

    bool creation_ok = (matcher != NULL);
    run_test("Matcher creation", creation_ok);

    if (matcher) {
        bool pattern_copied = (strcmp(matcher->pattern, pattern) == 0);
        run_test("Pattern copied correctly", pattern_copied);

        bool lps_created = (matcher->lps != NULL);
        run_test("LPS table created", lps_created);

        bool is_compiled = matcher->is_compiled;
        run_test("Matcher compiled flag set", is_compiled);

        bool memory_tracked = (matcher->memory_usage > 0);
        run_test("Memory usage tracked", memory_tracked);

        kmp_destroy(matcher);
    }

    kmp_destroy(NULL);
    run_test("Destroy NULL matcher (no crash)", true);
}

void test_ascii_validation() {
    printf("\n=== Testing ASCII Validation ===\n");

    run_test("Valid ASCII string", is_ascii_string("Hello World!"));
    run_test("Empty string is ASCII", is_ascii_string(""));
    run_test("NULL string is not ASCII", !is_ascii_string(NULL));

    char non_ascii[] = {'H', 'e', 'l', 'l', 'o', (char)200, '\0'};
    run_test("Non-ASCII string detected", !is_ascii_string(non_ascii));

    run_test("ASCII boundary (127)", is_ascii_string("\x7F"));
    run_test("Non-ASCII boundary (128)", !is_ascii_string("\x80"));
}

void test_utility_functions() {
    printf("\n=== Testing Utility Functions ===\n");

    run_test("Valid pattern validation", validate_pattern("TEST"));
    run_test("NULL pattern validation", !validate_pattern(NULL));
    run_test("Empty pattern validation", !validate_pattern(""));

    char* copy = safe_string_copy("TEST");
    bool copy_ok = (copy != NULL && strcmp(copy, "TEST") == 0);
    run_test("Safe string copy", copy_ok);
    if (copy) free(copy);

    run_test("Safe copy of NULL", safe_string_copy(NULL) == NULL);

    clock_t start = clock();
    clock_t end = start + CLOCKS_PER_SEC / 1000;
    double time_ms = measure_time(start, end);
    run_test("Time measurement (approximately 1ms)", time_ms >= 0.5 && time_ms <= 2.0);
}

void print_test_summary() {
    printf("\n=== Test Summary ===\n");
    printf("Total tests: %d\n", test_count);
    printf("Passed: %d\n", test_passed);
    printf("Failed: %d\n", test_count - test_passed);
    printf("Success rate: %.1f%%\n", (double)test_passed / test_count * 100.0);

    if (test_passed == test_count) {
        printf("All tests PASSED!\n");
    } else {
        printf("Some tests FAILED!\n");
    }
}

int main() {
    printf("KMP Algorithm Unit Tests\n");
    printf("========================\n");

    test_lps_computation();
    test_basic_search();
    test_multiple_search();
    test_edge_cases();
    test_memory_management();
    test_ascii_validation();
    test_utility_functions();

    print_test_summary();

    return (test_passed == test_count) ? 0 : 1;
}