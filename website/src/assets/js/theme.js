/* Theme toggle: flips dark/light class on <html> and persists choice. */
(function () {
  const btn = document.getElementById('theme-toggle');
  if (!btn) return;
  btn.addEventListener('click', function () {
    const isDark = document.documentElement.classList.toggle('dark');
    localStorage.setItem('po33-theme', isDark ? 'dark' : 'light');
  });
})();