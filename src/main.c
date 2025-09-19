#include "../include/kmp.h"

void demo_basic_search() {
    printf("\n=== Basic Search Demo ===\n");

    const char* text = "ABABDABACDABABCABCABCABCABC";
    const char* pattern = "ABABCAB";

    printf("Text: %s\n", text);
    printf("Pattern: %s\n", pattern);

    KMPMatcher* matcher = kmp_create(pattern);
    if (!matcher) {
        printf("Failed to create matcher\n");
        return;
    }

    kmp_print_stats(matcher);

    int position = kmp_search(matcher, text);
    if (position >= 0) {
        printf("Pattern found at position: %d\n", position);
        printf("Match: ");
        for (int i = 0; i < strlen(pattern); i++) {
            printf("%c", text[position + i]);
        }
        printf("\n");
    } else {
        printf("Pattern not found\n");
    }

    kmp_destroy(matcher);
}

void demo_multiple_search() {
    printf("\n=== Multiple Search Demo ===\n");

    const char* text = "ABCABCABCABCABC";
    const char* pattern = "ABC";

    printf("Text: %s\n", text);
    printf("Pattern: %s\n", pattern);

    KMPMatcher* matcher = kmp_create(pattern);
    if (!matcher) {
        printf("Failed to create matcher\n");
        return;
    }

    int count;
    int* positions = kmp_search_all(matcher, text, &count);

    if (positions && count > 0) {
        printf("Pattern found %d times at positions: ", count);
        for (int i = 0; i < count; i++) {
            printf("%d", positions[i]);
            if (i < count - 1) printf(", ");
        }
        printf("\n");
        free(positions);
    } else {
        printf("Pattern not found\n");
    }

    kmp_destroy(matcher);
}

void demo_with_statistics() {
    printf("\n=== Search with Statistics Demo ===\n");

    const char* text = "The quick brown fox jumps over the lazy dog. "
                      "The quick brown fox jumps over the lazy dog again.";
    const char* pattern = "fox";

    printf("Text: %s\n", text);
    printf("Pattern: %s\n", pattern);

    KMPMatcher* matcher = kmp_create(pattern);
    if (!matcher) {
        printf("Failed to create matcher\n");
        return;
    }

    SearchResult* result = kmp_search_with_stats(matcher, text);
    if (result) {
        printf("Search completed in %.3f ms\n", result->search_time);
        if (result->count > 0) {
            printf("Pattern found %d times at positions: ", result->count);
            for (int i = 0; i < result->count; i++) {
                printf("%d", result->positions[i]);
                if (i < result->count - 1) printf(", ");
            }
            printf("\n");
        } else {
            printf("Pattern not found\n");
        }

        free(result->positions);
        free(result);
    }

    kmp_destroy(matcher);
}

void demo_edge_cases() {
    printf("\n=== Edge Cases Demo ===\n");

    KMPMatcher* matcher;

    printf("Test 1: Single character pattern\n");
    matcher = kmp_create("a");
    if (matcher) {
        int pos = kmp_search(matcher, "banana");
        printf("'a' in 'banana': position %d\n", pos);
        kmp_destroy(matcher);
    }

    printf("\nTest 2: Pattern not found\n");
    matcher = kmp_create("xyz");
    if (matcher) {
        int pos = kmp_search(matcher, "abcdefghijk");
        printf("'xyz' in 'abcdefghijk': position %d\n", pos);
        kmp_destroy(matcher);
    }

    printf("\nTest 3: Pattern longer than text\n");
    matcher = kmp_create("verylongpattern");
    if (matcher) {
        int pos = kmp_search(matcher, "short");
        printf("'verylongpattern' in 'short': position %d\n", pos);
        kmp_destroy(matcher);
    }

    printf("\nTest 4: Empty text\n");
    matcher = kmp_create("test");
    if (matcher) {
        int pos = kmp_search(matcher, "");
        printf("'test' in '': position %d\n", pos);
        kmp_destroy(matcher);
    }
}

void print_usage(const char* program_name) {
    printf("Usage: %s [options]\n", program_name);
    printf("Options:\n");
    printf("  -h, --help          Show this help message\n");
    printf("  -d, --demo          Run all demo functions\n");
    printf("  -s, --search PATTERN TEXT  Search for PATTERN in TEXT\n");
    printf("\nExamples:\n");
    printf("  %s --demo\n", program_name);
    printf("  %s --search \"abc\" \"abcdefabcabc\"\n", program_name);
}

int main(int argc, char* argv[]) {
    printf("KMP Algorithm Implementation Demo\n");
    printf("=================================\n");

    if (argc == 1) {
        demo_basic_search();
        demo_multiple_search();
        demo_with_statistics();
        demo_edge_cases();
        return 0;
    }

    if (argc == 2 && (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)) {
        print_usage(argv[0]);
        return 0;
    }

    if (argc == 2 && (strcmp(argv[1], "-d") == 0 || strcmp(argv[1], "--demo") == 0)) {
        demo_basic_search();
        demo_multiple_search();
        demo_with_statistics();
        demo_edge_cases();
        return 0;
    }

    if (argc == 4 && (strcmp(argv[1], "-s") == 0 || strcmp(argv[1], "--search") == 0)) {
        const char* pattern = argv[2];
        const char* text = argv[3];

        if (!is_ascii_string(pattern) || !is_ascii_string(text)) {
            printf("Error: Only ASCII strings are supported\n");
            return 1;
        }

        printf("Searching for pattern: \"%s\"\n", pattern);
        printf("In text: \"%s\"\n", text);

        KMPMatcher* matcher = kmp_create(pattern);
        if (!matcher) {
            printf("Failed to create matcher\n");
            return 1;
        }

        kmp_print_stats(matcher);

        int count;
        int* positions = kmp_search_all(matcher, text, &count);

        if (positions && count > 0) {
            printf("Pattern found %d times at positions: ", count);
            for (int i = 0; i < count; i++) {
                printf("%d", positions[i]);
                if (i < count - 1) printf(", ");
            }
            printf("\n");
            free(positions);
        } else {
            printf("Pattern not found\n");
        }

        kmp_destroy(matcher);
        return 0;
    }

    printf("Invalid arguments. Use --help for usage information.\n");
    return 1;
}