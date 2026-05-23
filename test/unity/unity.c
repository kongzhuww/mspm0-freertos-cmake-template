#include "unity.h"
#include <stdio.h>

Unity_t Unity;

static const char* UnityTestFile = NULL;
static const char* UnityCurrentTestName = NULL;
static int UnityCurrentTestLine = 0;
static bool UnityCurrentTestFailed = false;

void UnityBegin(const char* filename)
{
    UnityTestFile = filename;
    Unity.NumberOfTests = 0;
    Unity.TestFailures = 0;
    Unity.TestIgnores = 0;
    printf("\n--- Test Suite Started ---\n");
}

int UnityEnd(void)
{
    printf("\n-----------------------\n");
    printf("%d Tests %d Failures %d Ignored\n", 
           (int)Unity.NumberOfTests, (int)Unity.TestFailures, (int)Unity.TestIgnores);
    if (Unity.TestFailures == 0) {
        printf("OK\n");
    } else {
        printf("FAIL\n");
    }
    return (int)Unity.TestFailures;
}

void UnityConcludeTest(void)
{
    if (UnityCurrentTestFailed) {
        Unity.TestFailures++;
    }
}

void UnityDefaultTestRun(void (*Func)(void), const char* FuncName, const int FuncLineNum)
{
    Unity.NumberOfTests++;
    UnityCurrentTestName = FuncName;
    UnityCurrentTestLine = FuncLineNum;
    UnityCurrentTestFailed = false;

    if (Func != NULL) {
        Func();
    }

    UnityConcludeTest();
}

void UnityAssertEqualNumber(const int32_t expected, const int32_t actual, const char* msg, const uint16_t line)
{
    if (expected != actual) {
        UnityCurrentTestFailed = true;
        printf("%s:%d:%s:FAIL: Expected %d Was %d", 
               UnityTestFile ? UnityTestFile : "unknown", 
               (int)line, 
               UnityCurrentTestName ? UnityCurrentTestName : "test", 
               (int)expected, 
               (int)actual);
        if (msg != NULL) {
            printf(" (%s)", msg);
        }
        printf("\n");
    } else {
        printf("%s:%d:%s:PASS\n", 
               UnityTestFile ? UnityTestFile : "unknown", 
               (int)line, 
               UnityCurrentTestName ? UnityCurrentTestName : "test");
    }
}
