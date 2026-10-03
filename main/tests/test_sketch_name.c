/*
 * test_sketch_name.c — Unity tests for the Docker-style jazz
 * sketch-name generator + validator (sketch/sketch_name.{c,h}).
 *
 * Uses the repo's TEST_CASE convention (TEST_CASE("name", "[tag]")).
 * Future unified test runner (see main/CMakeLists.txt note) will
 * pick this up automatically.
 *
 * For a host-compile verification that doesn't need Unity or
 * ESP-IDF, see /tmp/test_sketch_name.c which exercises the same
 * surface.
 */
#include "unity.h"
#include "sketch/sketch_name.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

TEST_CASE("vocabulary sizes match the design", "[sketch_name]")
{
    TEST_ASSERT_EQUAL_UINT8(12, SKETCH_NAME_GENRES_COUNT);
    TEST_ASSERT_EQUAL_UINT8(16, SKETCH_NAME_MUSICIANS_COUNT);
}

TEST_CASE("every genre is lowercase ASCII, 2..8 chars", "[sketch_name]")
{
    const char *const *g = sketch_name_genres();
    for (uint8_t i = 0; i < SKETCH_NAME_GENRES_COUNT; i++) {
        TEST_ASSERT_NOT_NULL_MESSAGE(g[i], "genre entry NULL");
        size_t len = strlen(g[i]);
        TEST_ASSERT_GREATER_THAN_MESSAGE(1, len, "genre too short");
        TEST_ASSERT_LESS_THAN_MESSAGE(9, len, "genre too long");
        for (size_t j = 0; j < len; j++) {
            TEST_ASSERT_TRUE_MESSAGE(g[i][j] >= 'a' && g[i][j] <= 'z',
                                     "genre has non-lowercase char");
        }
    }
}

TEST_CASE("every musician is lowercase ASCII, 3..10 chars, unique", "[sketch_name]")
{
    const char *const *m = sketch_name_musicians();
    for (uint8_t i = 0; i < SKETCH_NAME_MUSICIANS_COUNT; i++) {
        TEST_ASSERT_NOT_NULL_MESSAGE(m[i], "musician entry NULL");
        size_t len = strlen(m[i]);
        TEST_ASSERT_GREATER_THAN_MESSAGE(2, len, "musician too short");
        TEST_ASSERT_LESS_THAN_MESSAGE(11, len, "musician too long");
        for (size_t j = 0; j < len; j++) {
            TEST_ASSERT_TRUE_MESSAGE(m[i][j] >= 'a' && m[i][j] <= 'z',
                                     "musician has non-lowercase char");
        }
        for (uint8_t k = i + 1; k < SKETCH_NAME_MUSICIANS_COUNT; k++) {
            TEST_ASSERT_NOT_EQUAL_MESSAGE(0, strcmp(m[i], m[k]),
                                          "duplicate musician");
        }
    }
}

TEST_CASE("format() produces the canonical names", "[sketch_name]")
{
    char out[SKETCH_NAME_TOTAL_MAX + 1];
    TEST_ASSERT_TRUE(sketch_name_format(out, sizeof(out), 0, 0, 0));
    TEST_ASSERT_EQUAL_STRING("bebop_coltrane", out);

    TEST_ASSERT_TRUE(sketch_name_format(out, sizeof(out), 3, 4, 0));
    TEST_ASSERT_EQUAL_STRING("hardbop_gillespie", out);

    TEST_ASSERT_TRUE(sketch_name_format(out, sizeof(out), 0, 0, 2));
    TEST_ASSERT_EQUAL_STRING("bebop_coltrane_2", out);

    TEST_ASSERT_TRUE(sketch_name_format(out, sizeof(out), 0, 0, 99));
    TEST_ASSERT_EQUAL_STRING("bebop_coltrane_99", out);
}

TEST_CASE("format() rejects out-of-range and null inputs", "[sketch_name]")
{
    char out[SKETCH_NAME_TOTAL_MAX + 1];
    TEST_ASSERT_FALSE(sketch_name_format(out, sizeof(out),
                                         SKETCH_NAME_GENRES_COUNT, 0, 0));
    TEST_ASSERT_FALSE(sketch_name_format(out, sizeof(out),
                                         0, SKETCH_NAME_MUSICIANS_COUNT, 0));
    TEST_ASSERT_FALSE(sketch_name_format(NULL, 0, 0, 0, 0));
    TEST_ASSERT_FALSE(sketch_name_format(out, 0, 0, 0, 0));
}

TEST_CASE("format() rejects buffers that would truncate", "[sketch_name]")
{
    char tiny[17];
    TEST_ASSERT_FALSE(sketch_name_format(tiny, sizeof(tiny), 0, 0, 99));
}

TEST_CASE("validator accepts canonical and suffixed names", "[sketch_name]")
{
    TEST_ASSERT_TRUE(sketch_name_is_valid("bebop_coltrane"));
    TEST_ASSERT_TRUE(sketch_name_is_valid("cool_davis"));
    TEST_ASSERT_TRUE(sketch_name_is_valid("hardbop_blakey"));
    TEST_ASSERT_TRUE(sketch_name_is_valid("a_b"));
    TEST_ASSERT_TRUE(sketch_name_is_valid("nu_mingus"));
    TEST_ASSERT_TRUE(sketch_name_is_valid("bebop_coltrane_2"));
    TEST_ASSERT_TRUE(sketch_name_is_valid("bebop_coltrane_99"));
}

TEST_CASE("validator rejects obvious garbage", "[sketch_name]")
{
    TEST_ASSERT_FALSE(sketch_name_is_valid(NULL));
    TEST_ASSERT_FALSE(sketch_name_is_valid(""));
    TEST_ASSERT_FALSE(sketch_name_is_valid("bebop"));
    TEST_ASSERT_FALSE(sketch_name_is_valid("_coltrane"));
    TEST_ASSERT_FALSE(sketch_name_is_valid("bebop_"));
    TEST_ASSERT_FALSE(sketch_name_is_valid("Bebop_Coltrane"));
    TEST_ASSERT_FALSE(sketch_name_is_valid("bebop-coltrane"));
    TEST_ASSERT_FALSE(sketch_name_is_valid("bebop coltrane"));
}

TEST_CASE("validator rejects malformed collision suffix", "[sketch_name]")
{
    TEST_ASSERT_FALSE(sketch_name_is_valid("bebop_coltrane_"));
    TEST_ASSERT_FALSE(sketch_name_is_valid("bebop_coltrane_0"));
    TEST_ASSERT_FALSE(sketch_name_is_valid("bebop_coltrane_2x"));
    TEST_ASSERT_FALSE(sketch_name_is_valid("bebop_coltrane_2_3"));
}

TEST_CASE("validator enforces SKETCH_NAME_TOTAL_MAX", "[sketch_name]")
{
    char too_long[26];
    memset(too_long, 'a', 11);
    too_long[11] = '_';
    memset(too_long + 12, 'a', 12);
    too_long[24] = 'b';
    too_long[25] = '\0';
    TEST_ASSERT_EQUAL_UINT(25, strlen(too_long));
    TEST_ASSERT_FALSE(sketch_name_is_valid(too_long));

    char max_len[25];
    memset(max_len, 'a', 11);
    max_len[11] = '_';
    memset(max_len + 12, 'b', 12);
    max_len[24] = '\0';
    TEST_ASSERT_EQUAL_UINT(24, strlen(max_len));
    TEST_ASSERT_TRUE(sketch_name_is_valid(max_len));
}

TEST_CASE("next_free returns bebop_coltrane when nothing is taken", "[sketch_name]")
{
    char out[SKETCH_NAME_TOTAL_MAX + 1];
    const char *const taken[1] = { NULL };
    TEST_ASSERT_TRUE(sketch_name_next_free(out, sizeof(out), taken, 1));
    TEST_ASSERT_EQUAL_STRING("bebop_coltrane", out);
}

TEST_CASE("next_free skips over already-taken canonical names", "[sketch_name]")
{
    char out[SKETCH_NAME_TOTAL_MAX + 1];
    const char *const taken[] = {
        "bebop_coltrane", "bebop_davis", "bebop_monk",
    };
    TEST_ASSERT_TRUE(sketch_name_next_free(out, sizeof(out), taken, 3));
    TEST_ASSERT_EQUAL_STRING("bebop_parker", out);
}

TEST_CASE("next_free uses _N suffix when canonical grid is full", "[sketch_name]")
{
    char out[SKETCH_NAME_TOTAL_MAX + 1];
    const size_t M = SKETCH_NAME_GENRES_COUNT * SKETCH_NAME_MUSICIANS_COUNT;
    const char **taken = malloc(M * sizeof(const char *));
    size_t n = 0;
    for (uint8_t g = 0; g < SKETCH_NAME_GENRES_COUNT; g++)
        for (uint8_t m = 0; m < SKETCH_NAME_MUSICIANS_COUNT; m++) {
            char tmp[SKETCH_NAME_TOTAL_MAX + 1];
            snprintf(tmp, sizeof(tmp), "%s_%s",
                     sketch_name_genres()[g],
                     sketch_name_musicians()[m]);
            taken[n++] = strdup(tmp);
        }
    TEST_ASSERT_TRUE(sketch_name_next_free(out, sizeof(out), taken, n));
    TEST_ASSERT_EQUAL_STRING("bebop_coltrane_2", out);
    for (size_t i = 0; i < n; i++) free((void *)taken[i]);
    free(taken);
}

TEST_CASE("next_free returns false when namespace is exhausted", "[sketch_name]")
{
    const size_t N = SKETCH_NAME_GENRES_COUNT * SKETCH_NAME_MUSICIANS_COUNT
                     * (1 + 98);
    const char **taken = malloc(N * sizeof(const char *));
    size_t k = 0;
    for (uint8_t g = 0; g < SKETCH_NAME_GENRES_COUNT; g++)
        for (uint8_t m = 0; m < SKETCH_NAME_MUSICIANS_COUNT; m++) {
            char tmp[SKETCH_NAME_TOTAL_MAX + 1];
            snprintf(tmp, sizeof(tmp), "%s_%s",
                     sketch_name_genres()[g],
                     sketch_name_musicians()[m]);
            taken[k++] = strdup(tmp);
        }
    for (uint8_t g = 0; g < SKETCH_NAME_GENRES_COUNT; g++)
        for (uint8_t m = 0; m < SKETCH_NAME_MUSICIANS_COUNT; m++)
            for (uint8_t s = 2; s <= 99; s++) {
                char tmp[SKETCH_NAME_TOTAL_MAX + 1];
                snprintf(tmp, sizeof(tmp), "%s_%s_%u",
                         sketch_name_genres()[g],
                         sketch_name_musicians()[m],
                         (unsigned)s);
                taken[k++] = strdup(tmp);
            }
    char out[SKETCH_NAME_TOTAL_MAX + 1];
    TEST_ASSERT_FALSE(sketch_name_next_free(out, sizeof(out), taken, N));
    for (size_t i = 0; i < N; i++) free((void *)taken[i]);
    free(taken);
}

TEST_CASE("curated grid fits in SKETCH_NAME_TOTAL_MAX", "[sketch_name]")
{
    for (uint8_t g = 0; g < SKETCH_NAME_GENRES_COUNT; g++) {
        for (uint8_t m = 0; m < SKETCH_NAME_MUSICIANS_COUNT; m++) {
            char out[SKETCH_NAME_TOTAL_MAX + 1];
            TEST_ASSERT_TRUE(sketch_name_format(out, sizeof(out), g, m, 0));
            TEST_ASSERT_LESS_OR_EQUAL_UINT_MESSAGE(SKETCH_NAME_TOTAL_MAX,
                                                    strlen(out),
                                                    "name too long for cap");
            TEST_ASSERT_TRUE_MESSAGE(sketch_name_is_valid(out),
                                     "name fails its own validator");
        }
    }
}
