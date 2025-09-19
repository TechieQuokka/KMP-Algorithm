#include "../include/kmp.h"

void print_lps_table(const int* lps, int len) {
    if (!lps || len <= 0) {
        printf("Invalid LPS table\n");
        return;
    }

    printf("LPS Table: [");
    for (int i = 0; i < len; i++) {
        printf("%d", lps[i]);
        if (i < len - 1) {
            printf(", ");
        }
    }
    printf("]\n");
}

double measure_time(clock_t start, clock_t end) {
    return ((double)(end - start)) / CLOCKS_PER_SEC * 1000.0;
}

bool validate_pattern(const char* pattern) {
    if (!pattern) {
        return false;
    }

    if (strlen(pattern) == 0) {
        return false;
    }

    return is_ascii_string(pattern);
}

void kmp_print_stats(const KMPMatcher* matcher) {
    if (!matcher) {
        printf("Matcher is NULL\n");
        return;
    }

    printf("=== KMP Matcher Statistics ===\n");
    printf("Pattern: \"%s\"\n", matcher->pattern);
    printf("Pattern Length: %d\n", matcher->pattern_len);
    printf("Is Compiled: %s\n", matcher->is_compiled ? "Yes" : "No");
    printf("Memory Usage: %zu bytes\n", matcher->memory_usage);

    if (matcher->is_compiled && matcher->lps) {
        printf("LPS Table: ");
        print_lps_table(matcher->lps, matcher->pattern_len);
    }
    printf("==============================\n");
}

const char* kmp_error_string(KMPError error) {
    switch (error) {
        case KMP_SUCCESS:
            return "Success";
        case KMP_ERROR_NULL_POINTER:
            return "Null pointer error";
        case KMP_ERROR_EMPTY_PATTERN:
            return "Empty pattern error";
        case KMP_ERROR_MEMORY_ALLOCATION:
            return "Memory allocation error";
        case KMP_ERROR_INVALID_INPUT:
            return "Invalid input error";
        default:
            return "Unknown error";
    }
}

bool is_ascii_string(const char* str) {
    if (!str) {
        return false;
    }

    while (*str) {
        if ((unsigned char)*str > 127) {
            return false;
        }
        str++;
    }

    return true;
}

char* safe_string_copy(const char* src) {
    if (!src) {
        return NULL;
    }

    int len = strlen(src);
    char* dest = (char*)malloc((len + 1) * sizeof(char));
    if (!dest) {
        return NULL;
    }

    strcpy(dest, src);
    return dest;
}