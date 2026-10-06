/* Mobile-nav toggle.
 *
 * The mobile menu (#nav-mobile) is hidden by default via the `hidden`
 * utility class. On viewport >= md, the `md:hidden` utility keeps it
 * hidden regardless. This handler:
 *   - toggles `hidden` on the menu when the hamburger button is clicked;
 *   - flips `aria-expanded` and swaps the hamburger / close icon;
 *   - closes the menu when an in-menu link is followed (so the page
 *     transition does not happen with the menu overlaying content);
 *   - closes the menu on Escape;
 *   - resets state cleanly if the viewport grows past md.
 */
(function () {
  const btn = document.getElementById('nav-toggle');
  const menu = document.getElementById('nav-mobile');
  const iconOpen = document.getElementById('nav-toggle-icon-open');
  const iconClose = document.getElementById('nav-toggle-icon-close');
  if (!btn || !menu) return;

  function isOpen() {
    return !menu.classList.contains('hidden');
  }

  function setOpen(open) {
    menu.classList.toggle('hidden', !open);
    btn.setAttribute('aria-expanded', open ? 'true' : 'false');
    if (iconOpen) iconOpen.classList.toggle('hidden', open);
    if (iconClose) iconClose.classList.toggle('hidden', !open);
  }

  btn.addEventListener('click', function () {
    setOpen(!isOpen());
  });

  // Close the menu when the user picks an item.
  menu.querySelectorAll('a').forEach(function (a) {
    a.addEventListener('click', function () {
      setOpen(false);
    });
  });

  // Escape closes the menu and returns focus to the toggle.
  document.addEventListener('keydown', function (ev) {
    if (ev.key === 'Escape' && isOpen()) {
      setOpen(false);
      btn.focus();
    }
  });

  // If the user rotates / resizes to a wide viewport, force the menu
  // closed so the next narrow viewport starts from a known state.
  const mql = window.matchMedia('(min-width: 768px)');
  function onChange(e) { if (e.matches) setOpen(false); }
  if (mql.addEventListener) mql.addEventListener('change', onChange);
  else if (mql.addListener) mql.addListener(onChange);
})();
