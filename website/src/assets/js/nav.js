/* Mobile nav toggle. */
(function () {
  const btn = document.getElementById('nav-toggle');
  const menu = document.getElementById('nav-mobile');
  if (!btn || !menu) return;
  btn.addEventListener('click', function () {
    menu.classList.toggle('hidden');
  });
})();