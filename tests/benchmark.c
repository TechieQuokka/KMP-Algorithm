#include "../include/kmp.h"

typedef struct {
    char* name;
    char* pattern;
    char* text;
    int iterations;
    double expected_time_ms;
} Benchmark;

char* generate_random_string(int length, int alphabet_size) {
    char* str = (char*)malloc((length + 1) * sizeof(char));
    if (!str) return NULL;

    srand(time(NULL));
    for (int i = 0; i < length; i++) {
        str[i] = 'A' + (rand() % alphabet_size);
    }
    str[length] = '\0';

    return str;
}

char* generate_worst_case_text(const char* pattern, int text_length) {
    int pattern_len = strlen(pattern);
    char* text = (char*)malloc((text_length + 1) * sizeof(char));
    if (!text) return NULL;

    for (int i = 0; i < text_length; i++) {
        if (i < pattern_len - 1) {
            text[i] = pattern[i];
        } else {
            text[i] = pattern[0];
        }
    }
    text[text_length] = '\0';

    return text;
}

int naive_search(const char* text, const char* pattern) {
    int n = strlen(text);
    int m = strlen(pattern);

    for (int i = 0; i <= n - m; i++) {
        int j;
        for (j = 0; j < m; j++) {
            if (text[i + j] != pattern[j]) {
                break;
            }
        }
        if (j == m) {
            return i;
        }
    }
    return -1;
}

double benchmark_kmp(const char* pattern, const char* text, int iterations) {
    KMPMatcher* matcher = kmp_create(pattern);
    if (!matcher) return -1.0;

    clock_t start = clock();

    for (int i = 0; i < iterations; i++) {
        kmp_search(matcher, text);
    }

    clock_t end = clock();
    double total_time = measure_time(start, end);

    kmp_destroy(matcher);
    return total_time / iterations;
}

double benchmark_naive(const char* pattern, const char* text, int iterations) {
    clock_t start = clock();

    for (int i = 0; i < iterations; i++) {
        naive_search(text, pattern);
    }

    clock_t end = clock();
    double total_time = measure_time(start, end);

    return total_time / iterations;
}

void run_single_benchmark(const char* name, const char* pattern,
                         const char* text, int iterations) {
    printf("\n=== %s ===\n", name);
    printf("Pattern: \"%s\" (length: %d)\n", pattern, (int)strlen(pattern));
    printf("Text length: %d\n", (int)strlen(text));
    printf("Iterations: %d\n", iterations);

    double kmp_time = benchmark_kmp(pattern, text, iterations);
    double naive_time = benchmark_naive(pattern, text, iterations);

    if (kmp_time >= 0) {
        printf("KMP average time: %.6f ms\n", kmp_time);
    } else {
        printf("KMP benchmark failed\n");
    }

    if (naive_time >= 0) {
        printf("Naive average time: %.6f ms\n", naive_time);
    } else {
        printf("Naive benchmark failed\n");
    }

    if (kmp_time >= 0 && naive_time >= 0) {
        double speedup = naive_time / kmp_time;
        printf("Speedup: %.2fx\n", speedup);
    }
}

void benchmark_varying_text_size() {
    printf("\n=== Benchmark: Varying Text Size ===\n");

    const char* pattern = "ABABCAB";
    int pattern_len = strlen(pattern);
    int sizes[] = {1000, 10000, 100000, 1000000};
    int num_sizes = sizeof(sizes) / sizeof(sizes[0]);

    printf("Pattern: %s\n", pattern);
    printf("%-10s %-15s %-15s %-10s\n", "Size", "KMP (ms)", "Naive (ms)", "Speedup");
    printf("----------------------------------------------------\n");

    for (int i = 0; i < num_sizes; i++) {
        char* text = generate_random_string(sizes[i], 4);
        if (!text) continue;

        double kmp_time = benchmark_kmp(pattern, text, 10);
        double naive_time = benchmark_naive(pattern, text, 10);

        if (kmp_time >= 0 && naive_time >= 0) {
            double speedup = naive_time / kmp_time;
            printf("%-10d %-15.6f %-15.6f %-10.2f\n",
                   sizes[i], kmp_time, naive_time, speedup);
        }

        free(text);
    }
}

void benchmark_varying_pattern_size() {
    printf("\n=== Benchmark: Varying Pattern Size ===\n");

    int pattern_sizes[] = {5, 10, 50, 100};
    int num_sizes = sizeof(pattern_sizes) / sizeof(pattern_sizes[0]);
    int text_size = 100000;

    printf("Text size: %d\n", text_size);
    printf("%-12s %-15s %-15s %-10s\n", "Pattern Size", "KMP (ms)", "Naive (ms)", "Speedup");
    printf("----------------------------------------------------\n");

    for (int i = 0; i < num_sizes; i++) {
        char* pattern = generate_random_string(pattern_sizes[i], 4);
        char* text = generate_random_string(text_size, 4);

        if (!pattern || !text) {
            free(pattern);
            free(text);
            continue;
        }

        double kmp_time = benchmark_kmp(pattern, text, 10);
        double naive_time = benchmark_naive(pattern, text, 10);

        if (kmp_time >= 0 && naive_time >= 0) {
            double speedup = naive_time / kmp_time;
            printf("%-12d %-15.6f %-15.6f %-10.2f\n",
                   pattern_sizes[i], kmp_time, naive_time, speedup);
        }

        free(pattern);
        free(text);
    }
}

void benchmark_worst_case() {
    printf("\n=== Benchmark: Worst Case Scenario ===\n");

    const char* pattern = "AAAAAAB";
    char* text = generate_worst_case_text(pattern, 100000);

    if (!text) {
        printf("Failed to generate worst case text\n");
        return;
    }

    printf("Pattern: %s\n", pattern);
    printf("Text: %s... (truncated, length: %d)\n",
           text, (int)strlen(text));

    double kmp_time = benchmark_kmp(pattern, text, 10);
    double naive_time = benchmark_naive(pattern, text, 10);

    printf("KMP time: %.6f ms\n", kmp_time);
    printf("Naive time: %.6f ms\n", naive_time);

    if (kmp_time >= 0 && naive_time >= 0) {
        double speedup = naive_time / kmp_time;
        printf("Speedup: %.2fx\n", speedup);
    }

    free(text);
}

void benchmark_alphabet_size() {
    printf("\n=== Benchmark: Different Alphabet Sizes ===\n");

    const char* pattern = "ABCDEFG";
    int text_size = 50000;
    int alphabet_sizes[] = {2, 4, 8, 26};
    int num_sizes = sizeof(alphabet_sizes) / sizeof(alphabet_sizes[0]);

    printf("Pattern: %s\n", pattern);
    printf("Text size: %d\n", text_size);
    printf("%-15s %-15s %-15s %-10s\n", "Alphabet Size", "KMP (ms)", "Naive (ms)", "Speedup");
    printf("--------------------------------------------------------\n");

    for (int i = 0; i < num_sizes; i++) {
        char* text = generate_random_string(text_size, alphabet_sizes[i]);
        if (!text) continue;

        double kmp_time = benchmark_kmp(pattern, text, 10);
        double naive_time = benchmark_naive(pattern, text, 10);

        if (kmp_time >= 0 && naive_time >= 0) {
            double speedup = naive_time / kmp_time;
            printf("%-15d %-15.6f %-15.6f %-10.2f\n",
                   alphabet_sizes[i], kmp_time, naive_time, speedup);
        }

        free(text);
    }
}

void memory_usage_analysis() {
    printf("\n=== Memory Usage Analysis ===\n");

    int pattern_sizes[] = {10, 100, 1000, 10000};
    int num_sizes = sizeof(pattern_sizes) / sizeof(pattern_sizes[0]);

    printf("%-15s %-20s %-15s\n", "Pattern Size", "Memory Usage (bytes)", "Per Character");
    printf("---------------------------------------------------\n");

    for (int i = 0; i < num_sizes; i++) {
        char* pattern = generate_random_string(pattern_sizes[i], 4);
        if (!pattern) continue;

        KMPMatcher* matcher = kmp_create(pattern);
        if (matcher) {
            double per_char = (double)matcher->memory_usage / pattern_sizes[i];
            printf("%-15d %-20zu %-15.2f\n",
                   pattern_sizes[i], matcher->memory_usage, per_char);
            kmp_destroy(matcher);
        }

        free(pattern);
    }
}

int main() {
    printf("KMP Algorithm Benchmark Suite\n");
    printf("=============================\n");

    run_single_benchmark("Basic Test", "ABABCAB",
                         "ABABDABACDABABCABCABCABCABC", 1000);

    run_single_benchmark("Single Character", "A",
                         "AAAAAAAAAAAAAAAAAAAAAAAAAAAA", 1000);

    char* large_text = generate_random_string(10000, 4);
    if (large_text) {
        run_single_benchmark("Random Text", "ABCD", large_text, 100);
        free(large_text);
    }

    benchmark_varying_text_size();
    benchmark_varying_pattern_size();
    benchmark_worst_case();
    benchmark_alphabet_size();
    memory_usage_analysis();

    printf("\nBenchmark completed.\n");
    return 0;
}