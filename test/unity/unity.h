#ifndef UNITY_H
#define UNITY_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void UnityBegin(const char* filename);
int UnityEnd(void);
void UnityConcludeTest(void);
void UnityDefaultTestRun(void (*Func)(void), const char* FuncName, const int FuncLineNum);

typedef struct {
    uint32_t NumberOfTests;
    uint32_t TestFailures;
    uint32_t TestIgnores;
} Unity_t;

extern Unity_t Unity;

void UnityAssertEqualNumber(const int32_t expected, const int32_t actual, const char* msg, const uint16_t line);

#define UNITY_BEGIN() UnityBegin(__FILE__)
#define UNITY_END() UnityEnd()

#define TEST_ASSERT_EQUAL(expected, actual) \
    UnityAssertEqualNumber((int32_t)(expected), (int32_t)(actual), NULL, __LINE__)

#define TEST_ASSERT_TRUE(condition) \
    UnityAssertEqualNumber(1, (condition) ? 1 : 0, "Expected True", __LINE__)

#define TEST_ASSERT_FALSE(condition) \
    UnityAssertEqualNumber(0, (condition) ? 1 : 0, "Expected False", __LINE__)

#define RUN_TEST(TestFunc) \
    UnityDefaultTestRun(TestFunc, #TestFunc, __LINE__)

#ifdef __cplusplus
}
#endif

#endif /* UNITY_H */
