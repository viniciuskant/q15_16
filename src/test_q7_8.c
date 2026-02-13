#include <stdio.h>
#include <math.h>
#include "q7_8.h"
#include <math.h>

#define TOL 0.01

int total = 0;
int passed = 0;


void check_int(const char *name, int expected, int got) {
    total++;

    int abs_error = expected - got;
    if (abs_error < 0)
        abs_error = -abs_error;

    if (abs_error == 0) {
        passed++;
        printf("[SUCCESS] %s\n", name);
    } else {
        printf("[FAIL] %s | expected=%d got=%d | abs_error=%d\n",
               name, expected, got, abs_error);
    }
}

void check_float(const char *name, float expected, float got) {
    total++;

    float abs_error = expected - got;
    if (abs_error < 0.0f)
        abs_error = -abs_error;

    if (abs_error < TOL) {
        passed++;
        printf("[SUCCESS] %s\n", name);
    } else {
        printf("[FAIL] %s | expected=%f got=%f | abs_error=%f\n",
               name, expected, got, abs_error);
    }
}


int main() {
    struct {
        float f;
        int expected;
    } tests[] = {
        {127.9961, 32767},
        {-128.0000, -32768},
        {-200.0, -32768}, 
        {200.0, 32767},
        {0.0039063, 1},
        {-0.0039063, -1},
        {-0.05, -13},
        {34.78, 8904},
        {-17.576, -4499},
        {66.666, 17066},
        {39.001, 9984},
        {-39.001, -9984},
    };

    int n = sizeof(tests)/sizeof(tests[0]);
    char test_name[100];

    for (int i = 0; i < n; i++) {
        q7_8 fixed = float_to_q7_8(tests[i].f);
        snprintf(test_name, sizeof(test_name), "float_to_q7_8(%2f)", tests[i].f);
        check_int(test_name, tests[i].expected, fixed);

        float back = q7_8_to_float(fixed);
        snprintf(test_name, sizeof(test_name), "q7_8_to_float(%d)", fixed);
        check_float(test_name, fixed / 256.0f, back);
    }

    float a_float = 34.78f;
    float b_float = -17.576;

    q7_8 a = float_to_q7_8(a_float);
    q7_8 b = float_to_q7_8(b_float);

    q7_8 sum = q7_8_add(a, b);
    q7_8 sub = q7_8_sub(a, b);
    q7_8 mul = q7_8_mul(a, b);

    float expected_sum = a_float + b_float;
    if (expected_sum > 127.99609375f) expected_sum = 127.9961f;
    if (expected_sum < -128.0000f) expected_sum = -128.0000f;

    float expected_sub = a_float - b_float;
    if (expected_sub > 127.9961f) expected_sub = 127.9961f;
    if (expected_sub < -128.0000f) expected_sub = -128.0000f;

    float expected_mul = a_float * b_float;
    if (expected_mul > 127.9961f) expected_mul = 127.9961f;
    if (expected_mul < -128.0000f) expected_mul = -128.0000f;

    check_float("add", expected_sum, q7_8_to_float(sum));
    check_float("sub", expected_sub, q7_8_to_float(sub));
    check_float("mul", expected_mul, q7_8_to_float(mul));

    printf("\n=====================\n");
    printf("Passed %d / %d tests\n", passed, total);

    if (passed == total)
        printf("ALL TESTS PASSED\n");
    else
        printf("SOME TESTS FAILED\n");

    return 0;
}
