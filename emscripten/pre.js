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

// DEBUG: Flip debug system for diagnosing coordinate issues.
//
// Number keys 1-4 set flip mode for the SELECTED STAGE:
//   1 = (+X, -Y)  default desktop convention
//   2 = (+X, +Y)  flip Y
//   3 = (-X, -Y)  flip X
//   4 = (-X, +Y)  flip both
//
// Modifier keys select which rendering stage to affect:
//   No modifier  = Final blit (ScreenBuffer → default FB)
//   Shift        = PostProcess scene copy (BackBuffer → PostProcess FBO)
//   Ctrl         = PostProcess Blit8 (8bpp CPU → PostProcess FBO)
//   Alt          = UploadFrame ScreenBlit (PostProcess → ScreenBuffer)
//
// Current state shown in console on each change.
window._ccFlipDebug = {
  finalBlit:   0,  // 0=+X-Y, 1=+X+Y, 2=-X-Y, 3=-X+Y
  ppScene:     0,
  ppBlit8:     0,
  screenBlit:  0,
  inputYFlip:  0,  // 5 key toggles input Y inversion
  guiShaderFlip: 0 // 6 key toggles GUI shader Y flip
};
(function() {
  var stageNames = ['finalBlit', 'ppScene', 'ppBlit8', 'screenBlit'];
  var modeNames = ['+X-Y (desktop default)', '+X+Y (flip Y)', '-X-Y (flip X)', '-X+Y (flip both)'];

  document.addEventListener('keydown', function(e) {
    // Determine which stage based on modifier
    var stage = 'finalBlit';
    if (e.shiftKey) stage = 'ppScene';
    else if (e.ctrlKey) stage = 'ppBlit8';
    else if (e.altKey) stage = 'screenBlit';

    var num = parseInt(e.key);
    if (num >= 1 && num <= 4) {
      e.preventDefault();
      window._ccFlipDebug[stage] = num - 1;
      console.log('[CC] Flip: ' + stage + ' = mode ' + num + ' (' + modeNames[num-1] + ')');
      console.log('[CC] Current flips: ' + JSON.stringify(window._ccFlipDebug));
    }
    // 5 key: toggle input Y inversion
    if (e.key === '5') {
      window._ccFlipDebug.inputYFlip = 1 - window._ccFlipDebug.inputYFlip;
      console.log('[CC] Input Y flip: ' + (window._ccFlipDebug.inputYFlip ? 'INVERTED' : 'normal'));
    }
    // 6 key: toggle GUI shader Y flip
    if (e.key === '6') {
      window._ccFlipDebug.guiShaderFlip = 1 - window._ccFlipDebug.guiShaderFlip;
      console.log('[CC] GUI shader Y flip: ' + (window._ccFlipDebug.guiShaderFlip ? 'DISABLED' : 'normal'));
    }
  });
})();

// ---------------------------------------------------------------------------
// Audio context unlock — browsers require a user gesture before audio plays.
// Resumes the game's _ccAudioCtx and SDL's AudioContext on first interaction.
// ---------------------------------------------------------------------------
(function() {
  var unlocked = false;
  function unlock() {
    if (unlocked) return;
    unlocked = true;
    console.log('[Audio] User gesture detected — resuming audio contexts');

    // Resume the game's Web Audio context
    if (window._ccAudioCtx && window._ccAudioCtx.state === 'suspended') {
      window._ccAudioCtx.resume().then(function() {
        console.log('[Audio] Game AudioContext resumed: ' + window._ccAudioCtx.state);
        // Replay any sounds that were queued while suspended
        if (window._ccAudioQueue && window._ccAudioQueue.length > 0) {
          console.log('[Audio] Playing ' + window._ccAudioQueue.length + ' queued sounds');
          window._ccAudioQueue.forEach(function(fn) { try { fn(); } catch(e) {} });
          window._ccAudioQueue = [];
        }
      });
    }
    window._ccAudioUnlocked = true;

    // Resume SDL's audio context (used by the software mixer)
    if (typeof SDL !== 'undefined' && SDL.audioContext && SDL.audioContext.state === 'suspended') {
      SDL.audioContext.resume();
    }

    // Also try to resume any AudioContext created by Emscripten's SDL3 port
    try {
      var allContexts = document.querySelectorAll('audio');
      allContexts.forEach(function(a) { a.play().catch(function(){}); });
    } catch(e) {}

    document.removeEventListener('click',     unlock);
    document.removeEventListener('keydown',   unlock);
    document.removeEventListener('touchstart', unlock);
    document.removeEventListener('touchend',  unlock);
  }
  document.addEventListener('click',     unlock);
  document.addEventListener('keydown',   unlock);
  document.addEventListener('touchstart', unlock);
  document.addEventListener('touchend',  unlock);
})();
