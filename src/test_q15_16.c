#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <inttypes.h>
#include "q15_16.h"

#define TOL 0.001f

static int total = 0;
static int passed = 0;

static void check_int(const char *name, int32_t expected, int32_t got) {
    total++;
    int32_t abs_error = expected - got;
    if (abs_error < 0) abs_error = -abs_error;
    if (abs_error == 0) {
        passed++;
        printf("[SUCCESS] %s\n", name);
    } else {
        printf("[FAIL] %s | expected=%" PRId32 " got=%" PRId32 " | abs_error=%" PRId32 "\n",
               name, expected, got, abs_error);
    }
}

static void check_float(const char *name, float expected, float got) {
    total++;
    float abs_error = fabsf(expected - got);
    if (abs_error < TOL) {
        passed++;
        printf("[SUCCESS] %s\n", name);
    } else {
        printf("[FAIL] %s | expected=%f got=%f | abs_error=%f\n",
               name, expected, got, abs_error);
    }
}

static void test_q15_16_pair(float a_float, float b_float) {
    printf("--------------------------------------------------\n");
    q15_16_t a = float_to_q15_16(a_float);
    q15_16_t b = float_to_q15_16(b_float);

    q15_16_t sum = q15_16_add(a, b);
    q15_16_t sub = q15_16_sub(a, b);
    q15_16_t mul = q15_16_mul(a, b);

    float max = 32768.0f - 1.0f/65536.0f;
    float min = -32768.0f;

    float expected_sum = a_float + b_float;
    if (expected_sum > max) expected_sum = max;
    if (expected_sum < min) expected_sum = min;

    float expected_sub = a_float - b_float;
    if (expected_sub > max) expected_sub = max;
    if (expected_sub < min) expected_sub = min;

    float expected_mul = a_float * b_float;
    if (expected_mul > max) expected_mul = max;
    if (expected_mul < min) expected_mul = min;

    char label[64];
    snprintf(label, sizeof(label), "%.2f + %.2f", a_float, b_float);
    check_float(label, expected_sum, q15_16_to_float(sum));

    snprintf(label, sizeof(label), "%.2f - %.2f", a_float, b_float);
    check_float(label, expected_sub, q15_16_to_float(sub));

    snprintf(label, sizeof(label), "%.2f * %.2f", a_float, b_float);
    check_float(label, expected_mul, q15_16_to_float(mul));
}

static void test_q15_16_saturation() {
    q15_16_t max_val = Q15_16_MAX;
    q15_16_t min_val = Q15_16_MIN;
    q15_16_t one = float_to_q15_16(1.0f);

    q15_16_t sum_max = q15_16_add(max_val, one);
    check_int("add (saturação superior)", Q15_16_MAX, sum_max);

    q15_16_t sub_min = q15_16_sub(min_val, one);
    check_int("sub (saturação inferior)", Q15_16_MIN, sub_min);

    q15_16_t mul_max = q15_16_mul(max_val, float_to_q15_16(2.0f));
    check_int("mul (saturação superior)", Q15_16_MAX, mul_max);
}

static float random_float_range(float min, float max) {
    return min + ((float)rand() / RAND_MAX) * (max - min);
}

static void test_conversion() {
    struct { float f; int32_t expected; } conv_tests[] = {
        {32768.00001f,   Q15_16_MAX},
        {40000.0f,       Q15_16_MAX},
        {1000000000.0f,  Q15_16_MAX},
        {-32768.00001f,  Q15_16_MIN},
        {0.2f,           13107},
        {0.1f,           6554},
        {3.14159265f,    205887},
        {10.1234567f,    663451},
        {-0.1f,          -6554},
        {32767.9961f,    (int32_t)(32767.9961f * 65536.0f + 0.5f)}
    };
    int n = sizeof(conv_tests) / sizeof(conv_tests[0]);
    char test_name[100];

    for (int i = 0; i < n; i++) {
        q15_16_t fixed = float_to_q15_16(conv_tests[i].f);
        snprintf(test_name, sizeof(test_name), "float_to_q15_16(%2f)", conv_tests[i].f);
        check_int(test_name, conv_tests[i].expected, fixed);

        float back = q15_16_to_float(fixed);
        float expected_back = conv_tests[i].f;
        if (expected_back > 32768.0f - 1.0f/65536.0f) expected_back = 32768.0f - 1.0f/65536.0f;
        if (expected_back < -32768.0f) expected_back = -32768.0f;
        snprintf(test_name, sizeof(test_name), "q15_16_to_float(%d)", fixed);
        check_float(test_name, expected_back, back);
    }
}

static void test_logarithms() {
    printf("\n=== Testes de Logaritmo ===\n");

    struct { float input; float expected; } log2_tests[] = {
        {2.0f, 1.0f}, {8.0f, 3.0f}, {0.5f, -1.0f}, {1.0f, 0.0f},
        {3.14159265f, 1.651496f}, {0.1f, -3.321928f}
    };
    int n_log2 = sizeof(log2_tests) / sizeof(log2_tests[0]);
    for (int i = 0; i < n_log2; i++) {
        q15_16_t fixed = float_to_q15_16(log2_tests[i].input);
        q15_16_t res = q15_16_log2(fixed);
        char name[100];
        snprintf(name, sizeof(name), "log2(%.3f)", log2_tests[i].input);
        check_float(name, log2_tests[i].expected, q15_16_to_float(res));
    }

    struct { float input; float expected; } ln_tests[] = {
        {2.0f, 0.693147f}, {M_E, 1.0f}, {8.0f, 2.079442f},
        {0.5f, -0.693147f}, {1.0f, 0.0f}, {3.14159265f, 1.144730f},
        {0.1f, -2.302585f}
    };
    int n_ln = sizeof(ln_tests) / sizeof(ln_tests[0]);
    printf("\n");
    for (int i = 0; i < n_ln; i++) {
        q15_16_t fixed = float_to_q15_16(ln_tests[i].input);
        q15_16_t res = q15_16_ln(fixed);
        char name[100];
        snprintf(name, sizeof(name), "ln(%.3f)", ln_tests[i].input);
        check_float(name, ln_tests[i].expected, q15_16_to_float(res));
    }

    struct { float input; float expected; } log10_tests[] = {
        {2.0f, 0.30103f}, {10.0f, 1.0f}, {100.0f, 2.0f},
        {8.0f, 0.90309f}, {0.5f, -0.30103f}, {1.0f, 0.0f},
        {3.14159265f, 0.49715f}, {0.1f, -1.0f}
    };
    int n_log10 = sizeof(log10_tests) / sizeof(log10_tests[0]);
    printf("\n");
    for (int i = 0; i < n_log10; i++) {
        q15_16_t fixed = float_to_q15_16(log10_tests[i].input);
        q15_16_t res = q15_16_log10(fixed);
        char name[100];
        snprintf(name, sizeof(name), "log10(%.3f)", log10_tests[i].input);
        check_float(name, log10_tests[i].expected, q15_16_to_float(res));
    }
}

static void test_complex() {
    printf("\n=== Testes de Números Complexos ===\n");

    struct { float real; float imag; } test_vals[] = {
        {2.0f, 3.0f},
        {-1.5f, 2.5f},
        {32760.0f, 100.0f},   
        {-30000.0f, -5000.0f}
    };
    int n = sizeof(test_vals) / sizeof(test_vals[0]);
    char label[100];
    float max = 32768.0f - 1.0f/65536.0f;
    float min = -32768.0f;

    #define SATURATE(x) ((x) > max ? max : ((x) < min ? min : (x)))

    for (int i = 0; i < n; i++) {
        float ar = test_vals[i].real;
        float ai = test_vals[i].imag;
        for (int j = 0; j < n; j++) {
            float br = test_vals[j].real;
            float bi = test_vals[j].imag;

            complex_q15_16 a = { float_to_q15_16(ar), float_to_q15_16(ai) };
            complex_q15_16 b = { float_to_q15_16(br), float_to_q15_16(bi) };

            complex_q15_16 sum = q15_16_complex_add(a, b);
            float expected_sum_r = SATURATE(ar + br);
            float expected_sum_i = SATURATE(ai + bi);
            snprintf(label, sizeof(label), "(%.2f+%.2fi) + (%.2f+%.2fi) real", ar, ai, br, bi);
            check_float(label, expected_sum_r, q15_16_to_float(sum.real));
            snprintf(label, sizeof(label), "(%.2f+%.2fi) + (%.2f+%.2fi) imag", ar, ai, br, bi);
            check_float(label, expected_sum_i, q15_16_to_float(sum.imag));

            complex_q15_16 sub = q15_16_complex_sub(a, b);
            float expected_sub_r = SATURATE(ar - br);
            float expected_sub_i = SATURATE(ai - bi);
            snprintf(label, sizeof(label), "(%.2f+%.2fi) - (%.2f+%.2fi) real", ar, ai, br, bi);
            check_float(label, expected_sub_r, q15_16_to_float(sub.real));
            snprintf(label, sizeof(label), "(%.2f+%.2fi) - (%.2f+%.2fi) imag", ar, ai, br, bi);
            check_float(label, expected_sub_i, q15_16_to_float(sub.imag));

            complex_q15_16 mul = q15_16_complex_mul(a, b);
            float expected_mul_r = SATURATE(ar*br - ai*bi);
            float expected_mul_i = SATURATE(ar*bi + ai*br);
            snprintf(label, sizeof(label), "(%.2f+%.2fi) * (%.2f+%.2fi) real", ar, ai, br, bi);
            check_float(label, expected_mul_r, q15_16_to_float(mul.real));
            snprintf(label, sizeof(label), "(%.2f+%.2fi) * (%.2f+%.2fi) imag", ar, ai, br, bi);
            check_float(label, expected_mul_i, q15_16_to_float(mul.imag));
        }
    }
    #undef SATURATE
}

int main() {
    test_conversion();

    printf("\n=== Testes de saturação ===\n");
    test_q15_16_saturation();

    test_q15_16_pair(123.456f, -45.678f);
    for (int i = 0; i < 3; i++) {
        float a = random_float_range(-30000.0f, 30000.0f);
        float b = random_float_range(-30000.0f, 30000.0f);
        test_q15_16_pair(a, b);
    }

    test_logarithms();
    test_complex();

    printf("\n=====================\n");
    printf("Passed %d / %d tests\n", passed, total);
    if (passed == total)
        printf("ALL TESTS PASSED\n");
    else
        printf("SOME TESTS FAILED\n");

    return 0;
}