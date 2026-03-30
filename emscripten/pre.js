/**
 * Cortex Command Community Project — Emscripten pre.js
 *
 * Loaded before the compiled WASM module starts. Sets up helpers used by the
 * C++ ↔ JS bridge (emscripten/shell.html and Source/System/WebPlatform.cpp).
 */

// ---------------------------------------------------------------------------
// Keyboard focus — ensure the canvas gets keyboard events immediately
// ---------------------------------------------------------------------------

Module['postRun'] = Module['postRun'] || [];
Module['postRun'].push(function() {
  var canvas = document.getElementById('canvas');
  if (canvas) {
    canvas.focus();
    // Prevent the browser from consuming arrow keys, space, and tab.
    canvas.addEventListener('keydown', function(e) {
      var consumed = [32, 37, 38, 39, 40, 9];
      if (consumed.indexOf(e.keyCode) !== -1) e.preventDefault();
    });
  }
});

// ---------------------------------------------------------------------------
// Audio context unlock — browsers require a user gesture before audio plays.
// ---------------------------------------------------------------------------
(function() {
  var unlocked = false;
  function unlock() {
    if (unlocked) return;
    unlocked = true;
    if (typeof AudioContext !== 'undefined') {
      var ac = new AudioContext();
      if (ac.state === 'suspended') ac.resume();
    }
    document.removeEventListener('click',    unlock);
    document.removeEventListener('keydown',  unlock);
    document.removeEventListener('touchend', unlock);
  }
  document.addEventListener('click',    unlock);
  document.addEventListener('keydown',  unlock);
  document.addEventListener('touchend', unlock);
})();
