/*
 * sketch_name.c — implementation of the Docker-style jazz name
 * generator + validator declared in sketch_name.h.
 *
 * Why Docker-style? It's a friendly, recognisable, low-ceremony
 * naming convention that scales to thousands of unique entries
 * without numbering prefixes getting in the way. Docker's own
 * generator picks a random pair at creation time; we pick the
 * lexicographically smallest unused pair (deterministic, easier
 * to test, identical visual experience for the first 192 sketches
 * which is far beyond SKETCHES_MAX = 16).
 *
 * The 12 genres and 16 musicians were chosen so the longest
 * canonical pair ("hardbop_gillespie" = 17 chars) plus the longest
 * collision suffix ("_99" = 3 chars) plus NUL = 21 bytes, well
 * inside SKETCH_NAME_TOTAL_MAX = 24.
 */
#include "sketch_name.h"

#include <string.h>
#include <stdio.h>

/* Curated jazz genres (12). Order is significant: the generator
 * walks the table lexicographically, so the first created sketch
 * is named "bebop_coltrane", the next "bebop_davis", etc. */
static const char *const k_genres[SKETCH_NAME_GENRES_COUNT] = {
    "bebop",    /*  5 */
    "cool",     /*  4 */
    "swing",    /*  5 */
    "hardbop",  /*  7 */
    "modal",    /*  5 */
    "bossa",    /*  5 */
    "fusion",   /*  6 */
    "free",     /*  4 */
    "latin",    /*  5 */
    "smooth",   /*  6 */
    "acid",     /*  4 */
    "nu",       /*  2 */
};

/* Curated jazz-musician surnames (16). Every surname is unique
 * and ASCII-lowercase. The longest is "gillespie" (9 chars), which
 * with "hardbop_" prefix and "_99" suffix and NUL = 9 + 8 + 3 + 1
 * = 21 bytes — still fits SKETCH_NAME_TOTAL_MAX = 24. */
static const char *const k_musicians[SKETCH_NAME_MUSICIANS_COUNT] = {
    "coltrane",   /*  8 */
    "davis",      /*  5 */
    "monk",       /*  4 */
    "parker",     /*  6 */
    "gillespie",  /*  9 */
    "evans",      /*  5 */
    "corea",      /*  5 */
    "wayne",      /*  5 */
    "basie",      /*  5 */
    "powell",     /*  6 */
    "blakey",     /*  6 */
    "jobim",      /*  5 */
    "coleman",    /*  7 */
    "tyner",      /*  5 */
    "dolphy",     /*  6 */
    "mingus",     /*  6 */
};

const char *const *sketch_name_genres(void)
{
    return k_genres;
}

const char *const *sketch_name_musicians(void)
{
    return k_musicians;
}

bool sketch_name_format(char *out, size_t out_size,
                        uint8_t genre_idx, uint8_t musician_idx,
                        uint8_t collision_n)
{
    if (!out || out_size == 0) return false;
    if (genre_idx    >= SKETCH_NAME_GENRES_COUNT)    return false;
    if (musician_idx >= SKETCH_NAME_MUSICIANS_COUNT) return false;

    int n;
    if (collision_n == 0) {
        n = snprintf(out, out_size, "%s_%s",
                     k_genres[genre_idx], k_musicians[musician_idx]);
    } else {
        n = snprintf(out, out_size, "%s_%s_%u",
                     k_genres[genre_idx], k_musicians[musician_idx],
                     (unsigned)collision_n);
    }
    /* snprintf returns the would-be length excluding NUL on
     * truncation. We treat truncation as failure regardless. */
    return n > 0 && (size_t)n < out_size;
}

bool sketch_name_is_valid(const char *s)
{
    if (!s) return false;
    size_t len = strlen(s);
    if (len == 0 || len > SKETCH_NAME_TOTAL_MAX) return false;

    /* Format: <genre>_<musician>[_<n>]
     *   - genre:  1+ lowercase letters
     *   - '_'    separator (exactly one, then optionally a second)
     *   - musician: 1+ lowercase letters
     *   - optional: '_' followed by 1+ digits (collision suffix)
     * Total: exactly 1 or 2 underscores, no more. */

    /* Find the genre/musician separator. */
    const char *first = strchr(s, '_');
    if (!first) return false;

    /* Genre part: non-empty, lowercase letters only. */
    if (first == s) return false;
    for (const char *p = s; p < first; p++) {
        if (*p < 'a' || *p > 'z') return false;
    }

    /* Musician part runs from after first '_' up to the next '_' or
     * end-of-string. */
    const char *m_start = first + 1;
    const char *second  = strchr(m_start, '_');
    const char *m_end;

    if (second) {
        /* Optional collision suffix "_<digit>+" follows. */
        m_end = second;
        if (second[1] == '\0') return false; /* empty suffix */
        /* Reject bare "_0" so collision suffixes never look like
         * canonical names. The generator starts at "_2" for the
         * same reason. */
        if (second[1] == '0' && second[2] == '\0') return false;
        /* Must be 1+ digits, no further '_' allowed. */
        for (const char *p = second + 1; *p; p++) {
            if (*p < '0' || *p > '9') return false;
            if (p > second + 1 && strchr(p + 1, '_') != NULL) return false;
        }
    } else {
        m_end = s + len;
    }

    /* Musician part: non-empty lowercase letters. */
    if (m_end == m_start) return false;
    for (const char *p = m_start; p < m_end; p++) {
        if (*p < 'a' || *p > 'z') return false;
    }
    return true;
}

/* Helper: returns true iff `name` appears anywhere in `taken[]`. */
static bool name_is_taken(const char *name,
                          const char *const *taken, size_t taken_count)
{
    if (!name) return false;
    for (size_t i = 0; i < taken_count; i++) {
        if (taken[i] && strcmp(name, taken[i]) == 0) return true;
    }
    return false;
}

bool sketch_name_next_free(char *out, size_t out_size,
                           const char *const *taken, size_t taken_count)
{
    if (!out || out_size == 0) return false;

    /* Pass 1: walk the canonical genre x musician grid (192 names)
     * in lex order and return the first one not in `taken`. This
     * is the common path: covers SKETCHES_MAX = 16 with 176 to
     * spare. */
    for (uint8_t g = 0; g < SKETCH_NAME_GENRES_COUNT; g++) {
        for (uint8_t m = 0; m < SKETCH_NAME_MUSICIANS_COUNT; m++) {
            if (!sketch_name_format(out, out_size, g, m, 0)) continue;
            if (!name_is_taken(out, taken, taken_count)) return true;
        }
    }

    /* Pass 2: every canonical name is taken. Fall back to collision
     * suffixes "_2" .. "_99", again in lex order (which here is
     * numeric, since the (g,m) prefix is the same for each pass).
     * We iterate the (g,m) grid once more and try suffixes per pair. */
    for (uint8_t g = 0; g < SKETCH_NAME_GENRES_COUNT; g++) {
        for (uint8_t m = 0; m < SKETCH_NAME_MUSICIANS_COUNT; m++) {
            for (uint8_t n = 2; n <= 99; n++) {
                if (!sketch_name_format(out, out_size, g, m, n)) continue;
                if (!name_is_taken(out, taken, taken_count)) return true;
            }
        }
    }

    /* Namespace exhausted (would need >19,008 sketches). */
    return false;
}
