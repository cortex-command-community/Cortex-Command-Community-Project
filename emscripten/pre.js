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

// DEBUG: F1-F4 keys toggle flip modes for diagnosing coordinate issues.
// F1=default(-Y), F2=(+Y), F3=(-X,-Y), F4=(-X,+Y)
// Read from C++ via window._ccFlipKeys
window._ccFlipKeys = {up:false, down:false, left:false, right:false};
document.addEventListener('keydown', function(e) {
  if (e.key === 'F1') { window._ccFlipKeys = {up:true, down:false, left:false, right:false}; console.log('[CC] Flip: F1 = default -Y'); }
  if (e.key === 'F2') { window._ccFlipKeys = {up:false, down:true, left:false, right:false}; console.log('[CC] Flip: F2 = +Y'); }
  if (e.key === 'F3') { window._ccFlipKeys = {up:false, down:false, left:true, right:false}; console.log('[CC] Flip: F3 = -X,-Y'); }
  if (e.key === 'F4') { window._ccFlipKeys = {up:false, down:false, left:false, right:true}; console.log('[CC] Flip: F4 = -X,+Y'); }
});
document.addEventListener('keyup', function(e) {
  // Keep the last pressed mode (don't reset on keyup)
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
