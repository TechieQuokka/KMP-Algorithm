#ifndef KMP_H
#define KMP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

typedef enum {
    KMP_SUCCESS = 0,
    KMP_ERROR_NULL_POINTER,
    KMP_ERROR_EMPTY_PATTERN,
    KMP_ERROR_MEMORY_ALLOCATION,
    KMP_ERROR_INVALID_INPUT
} KMPError;

typedef struct {
    char* pattern;
    int pattern_len;
    int* lps;
    bool is_compiled;
    size_t memory_usage;
} KMPMatcher;

typedef struct {
    int* positions;
    int count;
    double search_time;
} SearchResult;

KMPMatcher* kmp_create(const char* pattern);
void kmp_destroy(KMPMatcher* matcher);
int kmp_search(KMPMatcher* matcher, const char* text);
int* kmp_search_all(KMPMatcher* matcher, const char* text, int* count);
SearchResult* kmp_search_with_stats(KMPMatcher* matcher, const char* text);

int* compute_lps_table(const char* pattern, int pattern_len);
void optimize_lps_table(int* lps, int pattern_len);

void print_lps_table(const int* lps, int len);
double measure_time(clock_t start, clock_t end);
bool validate_pattern(const char* pattern);
void kmp_print_stats(const KMPMatcher* matcher);
const char* kmp_error_string(KMPError error);

bool is_ascii_string(const char* str);
char* safe_string_copy(const char* src);

#endif