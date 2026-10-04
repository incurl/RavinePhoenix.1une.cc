/* Metadata for the two long-form books that ship with the reader app.
 * Each entry maps a book "slug" (used by collections + permalinks) to
 * its display name, blurb, cover color, and the chapter count we
 * expect from the source markdown. The chapter count is informational
 * only; the real chapter list comes from the .eleventy.js collection
 * at build time. */

module.exports = [
  {
    slug: "makers",
    title: "The Maker's Companion",
    subtitle:
      "A study guide for Make: Electronic Music from Scratch × RavinePhoenix",
    blurb:
      "Fifteen chapters from the project's own study guide. Pairs the " +
      "Teenage Engineering PO-33 with the upstream makers book by the " +
      "same publisher. History, soldering, schematics, oscillators, " +
      "filters, modulation, sequencers, percussion, the Dogbotophone.",
    cover_color: "po33",
    badge: "Hardware × music",
    accent: "from-po33-700 to-po33-500",
    icon: "🔧",
    chapters_expected: 15,
  },
  {
    slug: "music-course",
    title: "The PO-33 Musical Companion",
    subtitle: "Beat, melody, groove, and genre for the novice operator",
    blurb:
      "A complete self-study music course. Seventeen chapters and " +
      "five appendices covering rhythm, sampling, slicing, scales, " +
      "chords, basslines, four genres (hip-hop, house, techno, lo-fi), " +
      "song structure, pattern chaining, parameter locks, and live " +
      "performance.",
    cover_color: "sky",
    badge: "Music theory",
    accent: "from-sky-700 to-sky-500",
    icon: "🎵",
    chapters_expected: 23, /* 1 front + 1 intro + 17 chapters + 5 appendices */
  },
];