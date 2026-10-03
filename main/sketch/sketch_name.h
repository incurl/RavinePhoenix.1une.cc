/*
 * sketch_name.h — Docker-style jazz sketch-name generator + validator.
 *
 * Convention:  <genre>_<musician>            (e.g. "bebop_coltrane")
 *              <genre>_<musician>_<n>       (collision suffix,  n>=2)
 *
 * The vocabulary is a curated list of 12 short jazz genres and 16
 * short jazz-musician surnames. Every combination fits within 15
 * characters (well under the SKETCH_NAME_MAX = 24 cap). The
 * collision suffix reserves up to "99" before erroring out, giving
 * 12 * 16 * 99 = 19,008 unique names — comfortably above the
 * 16-entry UI cap (SKETCHES_MAX) by four orders of magnitude.
 *
 * The 4-hex ID (and the folder name on flash) is independent of
 * this human-readable name — see storage.h. This module only deals
 * with the display name.
 *
 * No malloc; everything operates on caller-provided buffers.
 */
#ifndef PO33_SKETCH_NAME_H
#define PO33_SKETCH_NAME_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Number of curated entries in each vocabulary table. */
#define SKETCH_NAME_GENRES_COUNT    12
#define SKETCH_NAME_MUSICIANS_COUNT 16

/* Maximum total sketch-name length, NUL not included.
 * Fits "bebop_coltrane" (14) + "_99" (3) = 17 with headroom.
 * Matches SKETCH_NAME_MAX in config.h. */
#define SKETCH_NAME_TOTAL_MAX       24

/* Read-only access to the curated tables. Used by the docs (and by
 * the test) to verify the contents haven't drifted. */
const char *const *sketch_name_genres   (void);
const char *const *sketch_name_musicians(void);

/* Returns true iff `s` is a valid sketch name:
 *   - lowercase ASCII letters and digits only
 *   - exactly one underscore separator
 *   - non-empty genre prefix
 *   - non-empty musician suffix (digits allowed only after the
 *     separator, i.e. the musician part is letters and the suffix
 *     is "_<digit>+")
 *   - total length <= SKETCH_NAME_TOTAL_MAX
 *
 * Returns false otherwise. */
bool sketch_name_is_valid(const char *s);

/* Pick the next free "<genre>_<musician>[_<n>]" name that isn't in
 * `taken[]`. Writes the result into `out` (which must be at least
 * SKETCH_NAME_TOTAL_MAX + 1 bytes). Returns true on success, false
 * if `taken` exhausts the namespace (effectively impossible —
 * 12 * 16 * 99 = 19,008 names).
 *
 * `taken` is an array of C-strings, `taken_count` long. The chosen
 * name is the lexicographically smallest valid combo that doesn't
 * appear in `taken`. Collision handling: if "bebop_coltrane" is
 * taken, try "bebop_coltrane_2", then "_3", ..., "_99". */
bool sketch_name_next_free(char *out, size_t out_size,
                           const char *const *taken, size_t taken_count);

/* Pure helper: write "<genre>_<musician>" (or ".._n" on collision)
 * into out. Caller must guarantee `out` is at least
 * SKETCH_NAME_TOTAL_MAX + 1 bytes. No dedup — see next_free() for
 * that. Returns false if `genre_idx` or `musician_idx` are out of
 * range. */
bool sketch_name_format(char *out, size_t out_size,
                        uint8_t genre_idx, uint8_t musician_idx,
                        uint8_t collision_n /* 0 = no suffix */);

#ifdef __cplusplus
}
#endif

#endif /* PO33_SKETCH_NAME_H */
