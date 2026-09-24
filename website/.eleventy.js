const { DateTime } = require("luxon");

module.exports = function (eleventyConfig) {
  /* Pass through binaries & assets that live under public/ */
  eleventyConfig.addPassthroughCopy({ "public": "." });

  /* Watch CSS in dev */
  eleventyConfig.addWatchTarget("src/assets/css/");

  return {
    dir: {
      input: "src",
      output: "_site",
      includes: "_includes",
      data: "_data",
    },
    templateFormats: ["njk", "html", "md"],
    htmlTemplateEngine: "njk",
    markdownTemplateEngine: "njk",
  };
};