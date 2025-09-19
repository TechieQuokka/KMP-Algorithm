#include "../include/kmp.h"

KMPMatcher* kmp_create(const char* pattern) {
    if (!pattern) {
        return NULL;
    }

    if (!is_ascii_string(pattern)) {
        return NULL;
    }

    int pattern_len = strlen(pattern);
    if (pattern_len == 0) {
        return NULL;
    }

    KMPMatcher* matcher = (KMPMatcher*)malloc(sizeof(KMPMatcher));
    if (!matcher) {
        return NULL;
    }

    matcher->pattern = safe_string_copy(pattern);
    if (!matcher->pattern) {
        free(matcher);
        return NULL;
    }

    matcher->pattern_len = pattern_len;
    matcher->lps = compute_lps_table(pattern, pattern_len);
    if (!matcher->lps) {
        free(matcher->pattern);
        free(matcher);
        return NULL;
    }

    matcher->is_compiled = true;
    matcher->memory_usage = sizeof(KMPMatcher) +
                           (pattern_len + 1) * sizeof(char) +
                           pattern_len * sizeof(int);

    return matcher;
}

void kmp_destroy(KMPMatcher* matcher) {
    if (matcher) {
        free(matcher->pattern);
        free(matcher->lps);
        free(matcher);
    }
}

int kmp_search(KMPMatcher* matcher, const char* text) {
    if (!matcher || !matcher->is_compiled || !text) {
        return -1;
    }

    if (!is_ascii_string(text)) {
        return -1;
    }

    int n = strlen(text);
    int m = matcher->pattern_len;
    int i = 0;
    int j = 0;

    while (i < n) {
        if (matcher->pattern[j] == text[i]) {
            i++;
            j++;
        }

        if (j == m) {
            return i - j;
        } else if (i < n && matcher->pattern[j] != text[i]) {
            if (j != 0) {
                j = matcher->lps[j - 1];
            } else {
                i++;
            }
        }
    }

    return -1;
}

int* kmp_search_all(KMPMatcher* matcher, const char* text, int* count) {
    if (!matcher || !matcher->is_compiled || !text || !count) {
        if (count) *count = 0;
        return NULL;
    }

    if (!is_ascii_string(text)) {
        *count = 0;
        return NULL;
    }

    int n = strlen(text);
    int m = matcher->pattern_len;
    int capacity = 10;
    int* positions = (int*)malloc(capacity * sizeof(int));
    if (!positions) {
        *count = 0;
        return NULL;
    }

    *count = 0;
    int i = 0;
    int j = 0;

    while (i < n) {
        if (matcher->pattern[j] == text[i]) {
            i++;
            j++;
        }

        if (j == m) {
            if (*count >= capacity) {
                capacity *= 2;
                int* new_positions = (int*)realloc(positions, capacity * sizeof(int));
                if (!new_positions) {
                    free(positions);
                    *count = 0;
                    return NULL;
                }
                positions = new_positions;
            }

            positions[*count] = i - j;
            (*count)++;
            j = matcher->lps[j - 1];
        } else if (i < n && matcher->pattern[j] != text[i]) {
            if (j != 0) {
                j = matcher->lps[j - 1];
            } else {
                i++;
            }
        }
    }

    if (*count == 0) {
        free(positions);
        return NULL;
    }

    int* result = (int*)realloc(positions, (*count) * sizeof(int));
    return result ? result : positions;
}

SearchResult* kmp_search_with_stats(KMPMatcher* matcher, const char* text) {
    if (!matcher || !matcher->is_compiled || !text) {
        return NULL;
    }

    SearchResult* result = (SearchResult*)malloc(sizeof(SearchResult));
    if (!result) {
        return NULL;
    }

    clock_t start = clock();
    result->positions = kmp_search_all(matcher, text, &result->count);
    clock_t end = clock();

    result->search_time = measure_time(start, end);

    return result;
}