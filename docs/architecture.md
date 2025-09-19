# KMP 알고리즘 아키텍처 설계서

## 1. 개요

### 1.1 목적
본 문서는 KMP(Knuth-Morris-Pratt) 문자열 매칭 알고리즘의 C언어 구현을 위한 시스템 아키텍처를 정의한다.

### 1.2 범위
- KMP 알고리즘의 핵심 로직 설계
- 전처리 테이블(실패 함수) 구현 아키텍처
- 메모리 관리 및 성능 최적화 방안
- 테스트 및 검증 체계

### 1.3 용어 정의
- **패턴(Pattern)**: 찾고자 하는 문자열
- **텍스트(Text)**: 패턴을 찾을 대상 문자열
- **실패 함수(Failure Function)**: 패턴의 접두사-접미사 일치 정보를 담은 테이블
- **LPS(Longest Proper Prefix which is also Suffix)**: 가장 긴 진접두사이면서 접미사인 문자열

## 2. 시스템 아키텍처

### 2.1 전체 시스템 구조

```
KMP-Algorithm/
├── include/
│   └── kmp.h           # KMP 알고리즘 헤더 파일
├── src/
│   ├── kmp.c           # KMP 알고리즘 구현
│   ├── failure_func.c  # 실패 함수 구현
│   └── utils.c         # 유틸리티 함수들
├── tests/
│   ├── test_kmp.c      # 단위 테스트
│   └── benchmark.c     # 성능 벤치마크
├── docs/
│   ├── architecture.md # 본 문서
│   └── paper.md        # 연구 논문
└── Makefile            # 빌드 설정
```

### 2.2 모듈별 아키텍처

#### 2.2.1 Core Module (kmp.h, kmp.c)
```c
// 주요 구조체
typedef struct {
    char* pattern;      // 패턴 문자열
    int pattern_len;    // 패턴 길이
    int* lps;          // LPS 배열 (실패 함수)
    bool is_compiled;   // 전처리 완료 여부
} KMPMatcher;

// 주요 함수들
KMPMatcher* kmp_create(const char* pattern);
void kmp_destroy(KMPMatcher* matcher);
int kmp_search(KMPMatcher* matcher, const char* text);
int* kmp_search_all(KMPMatcher* matcher, const char* text, int* count);
```

#### 2.2.2 Preprocessing Module (failure_func.c)
```c
// 실패 함수 구현
int* compute_lps_table(const char* pattern, int pattern_len);
void optimize_lps_table(int* lps, int pattern_len);
```

#### 2.2.3 Utility Module (utils.c)
```c
// 유틸리티 함수들
void print_lps_table(const int* lps, int len);
double measure_time(clock_t start, clock_t end);
bool validate_pattern(const char* pattern);
```

## 3. 데이터 구조 설계

### 3.1 KMPMatcher 구조체
```c
typedef struct {
    char* pattern;          // 동적 할당된 패턴 문자열
    int pattern_len;        // 패턴의 길이
    int* lps;              // LPS 테이블 (동적 할당)
    bool is_compiled;       // 전처리 완료 플래그
    size_t memory_usage;    // 메모리 사용량 추적
} KMPMatcher;
```

### 3.2 LPS 테이블
- **타입**: `int*` (동적 배열)
- **크기**: 패턴 길이와 동일
- **초기화**: 모든 원소를 0으로 초기화
- **메모리 관리**: 매처 생성 시 할당, 소멸 시 해제

### 3.3 검색 결과 구조체
```c
typedef struct {
    int* positions;     // 매칭된 위치들의 배열
    int count;         // 매칭된 개수
    double search_time; // 검색 소요 시간
} SearchResult;
```

## 4. 알고리즘 플로우

### 4.1 전체 실행 흐름
```
1. KMPMatcher 생성
   ├── 패턴 검증
   ├── 메모리 할당
   └── LPS 테이블 계산

2. 문자열 검색
   ├── 텍스트 검증
   ├── KMP 검색 실행
   └── 결과 반환

3. 리소스 정리
   ├── 동적 메모리 해제
   └── 매처 객체 소멸
```

### 4.2 LPS 테이블 계산 알고리즘
```
function compute_lps_table(pattern):
    lps[0] = 0
    len = 0
    i = 1

    while i < pattern_length:
        if pattern[i] == pattern[len]:
            len++
            lps[i] = len
            i++
        else:
            if len != 0:
                len = lps[len - 1]
            else:
                lps[i] = 0
                i++
```

### 4.3 KMP 검색 알고리즘
```
function kmp_search(matcher, text):
    i = 0  // text index
    j = 0  // pattern index

    while i < text_length:
        if pattern[j] == text[i]:
            i++
            j++

        if j == pattern_length:
            // 매칭 발견
            return i - j
            j = lps[j - 1]
        else if i < text_length and pattern[j] != text[i]:
            if j != 0:
                j = lps[j - 1]
            else:
                i++

    return -1  // 매칭 없음
```

## 5. 메모리 관리

### 5.1 메모리 할당 전략
- **패턴 문자열**: `malloc()`을 사용한 동적 할당
- **LPS 테이블**: `calloc()`을 사용한 초기화된 할당
- **검색 결과**: 필요에 따른 동적 할당

### 5.2 메모리 해제 정책
```c
void kmp_destroy(KMPMatcher* matcher) {
    if (matcher) {
        free(matcher->pattern);
        free(matcher->lps);
        free(matcher);
    }
}
```

### 5.3 메모리 누수 방지
- 모든 동적 할당에 대한 대응되는 해제 함수 제공
- 예외 상황에서의 메모리 정리 로직 구현
- Valgrind를 활용한 메모리 누수 검증

## 6. 성능 최적화

### 6.1 시간 복잡도
- **전처리**: O(m) - m은 패턴 길이
- **검색**: O(n) - n은 텍스트 길이
- **전체**: O(n + m)

### 6.2 공간 복잡도
- **LPS 테이블**: O(m)
- **추가 공간**: O(1)

### 6.3 최적화 기법
- **캐시 지역성**: 배열 접근 패턴 최적화
- **분기 예측**: 조건문 순서 최적화
- **컴파일러 최적화**: `-O2` 플래그 활용

## 7. 에러 처리

### 7.1 입력 검증
```c
typedef enum {
    KMP_SUCCESS = 0,
    KMP_ERROR_NULL_POINTER,
    KMP_ERROR_EMPTY_PATTERN,
    KMP_ERROR_MEMORY_ALLOCATION,
    KMP_ERROR_INVALID_INPUT
} KMPError;
```

### 7.2 에러 처리 전략
- 모든 공개 함수에서 입력 매개변수 검증
- 메모리 할당 실패 시 적절한 에러 코드 반환
- 디버그 모드에서 상세한 에러 메시지 제공

## 8. 테스트 아키텍처

### 8.1 단위 테스트
```c
// 테스트 케이스 구조
typedef struct {
    char* pattern;
    char* text;
    int expected_position;
    char* description;
} TestCase;
```

### 8.2 테스트 범위
- LPS 테이블 생성 정확성 검증
- 다양한 패턴-텍스트 조합에서의 검색 정확성
- 경계 조건 (빈 문자열, 단일 문자 등) 테스트
- 성능 벤치마크 테스트

### 8.3 벤치마크 설계
```c
typedef struct {
    char* name;
    char* pattern;
    char* text;
    int iterations;
    double expected_time;
} Benchmark;
```

## 9. 빌드 시스템

### 9.1 Makefile 구조
```makefile
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2
DEBUG_FLAGS = -g -DDEBUG

SRCDIR = src
INCDIR = include
TESTDIR = tests
OBJDIR = obj

# 타겟들
all: kmp_demo test benchmark
debug: CFLAGS += $(DEBUG_FLAGS)
debug: all
```

### 9.2 빌드 타겟
- **kmp_demo**: 기본 데모 프로그램
- **test**: 단위 테스트 실행
- **benchmark**: 성능 벤치마크
- **debug**: 디버그 버전 빌드
- **clean**: 빌드 파일 정리

## 10. API 문서

### 10.1 공개 인터페이스
```c
// 매처 생성 및 소멸
KMPMatcher* kmp_create(const char* pattern);
void kmp_destroy(KMPMatcher* matcher);

// 검색 함수들
int kmp_search(KMPMatcher* matcher, const char* text);
int* kmp_search_all(KMPMatcher* matcher, const char* text, int* count);

// 유틸리티 함수들
void kmp_print_stats(const KMPMatcher* matcher);
const char* kmp_error_string(KMPError error);
```

### 10.2 사용 예제
```c
#include "kmp.h"

int main() {
    // 매처 생성
    KMPMatcher* matcher = kmp_create("ABCDAB");
    if (!matcher) {
        fprintf(stderr, "Failed to create matcher\n");
        return 1;
    }

    // 검색 실행
    const char* text = "ABC ABCDAB ABCDABCDABDE";
    int position = kmp_search(matcher, text);

    if (position >= 0) {
        printf("Pattern found at position: %d\n", position);
    } else {
        printf("Pattern not found\n");
    }

    // 리소스 정리
    kmp_destroy(matcher);
    return 0;
}
```

## 11. 확장성 고려사항

### 11.1 다국어 지원
- UTF-8 인코딩 지원을 위한 확장 가능한 구조
- 와이드 문자(wchar_t) 지원 옵션

### 11.2 병렬 처리
- 멀티 스레딩을 위한 스레드 안전 설계
- SIMD 명령어 활용 가능성

### 11.3 추가 기능
- 대소문자 무시 검색 옵션
- 정규 표현식과의 통합 가능성
- 스트리밍 데이터 처리 지원

이 아키텍처 설계서는 KMP 알고리즘의 효율적이고 유지보수 가능한 C언어 구현을 위한 청사진을 제공한다.