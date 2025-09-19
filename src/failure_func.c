#include "../include/kmp.h"

int* compute_lps_table(const char* pattern, int pattern_len) {
    if (!pattern || pattern_len <= 0) {
        return NULL;
    }

    int* lps = (int*)calloc(pattern_len, sizeof(int));
    if (!lps) {
        return NULL;
    }

    int len = 0;
    int i = 1;

    lps[0] = 0;

    while (i < pattern_len) {
        if (pattern[i] == pattern[len]) {
            len++;
            lps[i] = len;
            i++;
        } else {
            if (len != 0) {
                len = lps[len - 1];
            } else {
                lps[i] = 0;
                i++;
            }
        }
    }

    return lps;
}

void optimize_lps_table(int* lps, int pattern_len) {
    if (!lps || pattern_len <= 0) {
        return;
    }

    for (int i = 1; i < pattern_len; i++) {
        if (lps[i] > 0) {
            int j = lps[i];
            while (j > 0 && lps[j] >= lps[i]) {
                j = lps[j - 1];
            }
            if (j > 0) {
                lps[i] = j;
            }
        }
    }
}