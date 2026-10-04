/* ──────────────────────────────────────────────────────────────────
   Reader.js — small behaviors for the /reader/* chapter pages.

   The reader is rendered entirely as static HTML at build time
   (each chapter is its own URL). On the client, we only need:

     1. Scroll-progress bar at the top of the viewport.
     2. Sidebar toggle on mobile (the ☰ button).
     3. Keyboard shortcuts: j / k / ? / [ / g.
     4. Persistent theme: already handled by theme.js; we just
        make sure the <html> class flips when the toggle is
        clicked on the reader (which it does, but the progress
        bar's colour was hardcoded in Tailwind; this script
        re-styles it on theme change).

   All chapter content is pre-rendered — no markdown parsing here.
   ────────────────────────────────────────────────────────────────── */

(function () {
  "use strict";

  /* ── 1. Progress bar ────────────────────────────────────────── */
  const bar = document.getElementById("reader-progress");
  if (bar) {
    let ticking = false;
    function update() {
      const scrollTop =
        window.pageYOffset ||
        document.documentElement.scrollTop ||
        document.body.scrollTop;
      const docHeight =
        document.documentElement.scrollHeight - window.innerHeight;
      const pct = docHeight > 0 ? (scrollTop / docHeight) * 100 : 0;
      bar.style.width = Math.max(0, Math.min(100, pct)) + "%";
      ticking = false;
    }
    window.addEventListener(
      "scroll",
      function () {
        if (!ticking) {
          window.requestAnimationFrame(update);
          ticking = true;
        }
      },
      { passive: true }
    );
    update();
  }

  /* ── 2. Sidebar toggle (mobile) ─────────────────────────────── */
  const sidebar = document.getElementById("reader-sidebar");
  const sidebarToggle = document.getElementById("reader-sidebar-toggle");
  if (sidebar && sidebarToggle) {
    function closeSidebar() {
      sidebar.classList.add("-translate-x-full");
      sidebar.classList.remove("translate-x-0");
    }
    function openSidebar() {
      sidebar.classList.remove("-translate-x-full");
      sidebar.classList.add("translate-x-0");
    }
    sidebarToggle.addEventListener("click", function () {
      const isOpen = sidebar.classList.contains("translate-x-0");
      if (isOpen) closeSidebar();
      else openSidebar();
    });
    /* Close on link click (mobile) */
    sidebar.addEventListener("click", function (e) {
      if (e.target.tagName === "A") closeSidebar();
    });
    /* Close on Escape */
    document.addEventListener("keydown", function (e) {
      if (e.key === "Escape") closeSidebar();
    });
  }

  /* ── 3. Keyboard shortcuts ──────────────────────────────────── */
  /* We get prev/next URLs from data attributes that the chapter
   * template emits onto <body>. This avoids re-parsing the sidebar
   * for the prev/next links. */
  const prevUrl = document.body.dataset && document.body.dataset.prev;
  const nextUrl = document.body.dataset && document.body.dataset.next;

  document.addEventListener("keydown", function (e) {
    /* Skip when typing in form fields */
    const tag = (e.target && e.target.tagName) || "";
    if (tag === "INPUT" || tag === "TEXTAREA" || e.target.isContentEditable) {
      return;
    }
    /* Skip when modifier keys are held (don't hijack Cmd-R, etc.) */
    if (e.metaKey || e.ctrlKey || e.altKey) return;

    switch (e.key) {
      case "j":
        if (nextUrl) {
          e.preventDefault();
          window.location.href = nextUrl;
        }
        break;
      case "k":
        if (prevUrl) {
          e.preventDefault();
          window.location.href = prevUrl;
        }
        break;
      case "g":
        e.preventDefault();
        window.scrollTo({ top: 0, behavior: "smooth" });
        break;
      case "[":
        e.preventDefault();
        if (sidebar) {
          const isOpen = sidebar.classList.contains("translate-x-0");
          if (isOpen) sidebar.classList.add("-translate-x-full"), sidebar.classList.remove("translate-x-0");
          else sidebar.classList.remove("-translate-x-full"), sidebar.classList.add("translate-x-0");
        }
        break;
      case "?":
        e.preventDefault();
        const d = document.querySelector("aside details");
        if (d) d.open = !d.open;
        break;
    }
  });

  /* ── 4. Update progress-bar color on theme change ───────────── */
  /* theme.js flips the .dark class on <html>; we re-render the bar
   * color via a tiny style override. Tailwind's `dark:bg-po33-400`
   * handles the rest via CSS. No JS needed here. */
})();