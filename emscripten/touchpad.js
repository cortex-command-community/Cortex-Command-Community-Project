/**
 * Touch gamepad overlay for Cortex Command on mobile.
 *
 * Left side: D-pad joystick (movement via SDL scancodes)
 * Right side: FIRE (left click), JUMP (spacebar), PIE (right click)
 *
 * Uses SDL scancodes injected directly into Emscripten's input system
 * via Module.SDL3 or falling back to KeyboardEvent dispatch.
 */
(function() {
  'use strict';

  if (!('ontouchstart' in window)) return;

  var canvas = null;
  var gamepad = null;
  var audioUnlocked = false;

  function unlockAudio() {
    if (audioUnlocked) return;
    audioUnlocked = true;
    console.log('[Audio] Touch gamepad unlocking audio...');
    // Resume game's Web Audio context
    if (window._ccAudioCtx) {
      window._ccAudioCtx.resume();
    }
    // Also trigger the pre.js unlock handler
    if (!window._ccAudioUnlocked) {
      window._ccAudioUnlocked = true;
      if (window._ccAudioQueue) {
        window._ccAudioQueue.forEach(function(fn) { try { fn(); } catch(e) {} });
        window._ccAudioQueue = [];
      }
    }
    // Resume any Emscripten SDL audio contexts
    try {
      var allCtx = [window._ccAudioCtx];
      if (typeof Module !== 'undefined' && Module.SDL3 && Module.SDL3.audioContext) {
        allCtx.push(Module.SDL3.audioContext);
      }
      allCtx.forEach(function(ctx) {
        if (ctx && ctx.state === 'suspended') ctx.resume();
      });
    } catch(e) {}
  }

  function init() {
    canvas = document.getElementById('canvas');
    if (!canvas) { setTimeout(init, 500); return; }
    createGamepad();
  }

  // --- SDL key codes (matching SDL_SCANCODE values) ---
  var SDL_KEYS = {
    w: 'KeyW', a: 'KeyA', s: 'KeyS', d: 'KeyD',
    space: 'Space',
  };

  // Track which keys are currently held to avoid duplicate events
  var heldKeys = {};

  function pressKey(code, key) {
    if (heldKeys[code]) return;
    heldKeys[code] = true;
    // Dispatch to the canvas — SDL3 Emscripten listens on the canvas element
    canvas.dispatchEvent(new KeyboardEvent('keydown', {
      code: code, key: key, bubbles: true, cancelable: true, keyCode: keyCodeFor(key)
    }));
  }

  function releaseKey(code, key) {
    if (!heldKeys[code]) return;
    heldKeys[code] = false;
    canvas.dispatchEvent(new KeyboardEvent('keyup', {
      code: code, key: key, bubbles: true, cancelable: true, keyCode: keyCodeFor(key)
    }));
  }

  function keyCodeFor(key) {
    var map = { 'w': 87, 'a': 65, 's': 83, 'd': 68, ' ': 32 };
    return map[key] || 0;
  }

  function sendMouse(type, button) {
    var rect = canvas.getBoundingClientRect();
    var x = rect.left + rect.width / 2;
    var y = rect.top + rect.height / 2;
    canvas.dispatchEvent(new MouseEvent(type, {
      clientX: x, clientY: y, button: button, buttons: button === 0 ? 1 : 2,
      bubbles: true, cancelable: true
    }));
  }

  function createGamepad() {
    gamepad = document.createElement('div');
    gamepad.id = 'touch-gamepad';
    gamepad.innerHTML = `
      <style>
        #touch-gamepad {
          position: fixed; bottom: 0; left: 0; right: 0;
          height: 220px; z-index: 10000;
          pointer-events: none;
          user-select: none; -webkit-user-select: none;
        }
        .tp-stick {
          position: absolute; bottom: 20px;
          width: 130px; height: 130px;
          border-radius: 50%;
          background: rgba(255,255,255,0.12);
          border: 2px solid rgba(255,255,255,0.25);
          pointer-events: auto;
          touch-action: none;
        }
        .tp-stick-knob {
          position: absolute;
          width: 55px; height: 55px;
          border-radius: 50%;
          background: rgba(255,255,255,0.45);
          top: 50%; left: 50%;
          transform: translate(-50%, -50%);
          pointer-events: none;
        }
        .tp-btn {
          position: absolute;
          width: 64px; height: 64px;
          border-radius: 50%;
          border: 2px solid rgba(255,255,255,0.35);
          pointer-events: auto;
          touch-action: none;
          display: flex; align-items: center; justify-content: center;
          font-family: monospace; font-size: 13px; font-weight: bold;
          color: rgba(255,255,255,0.7);
        }
        .tp-btn.active { background: rgba(255,255,255,0.4); }
        #tp-move { left: 15px; }
        #tp-fire { right: 20px; bottom: 20px; background: rgba(255,80,80,0.3); }
        #tp-jump { right: 95px; bottom: 80px; background: rgba(80,180,255,0.3); }
        #tp-pie  { right: 95px; bottom: 15px; background: rgba(255,200,80,0.3); }
      </style>
      <div id="tp-move" class="tp-stick"><div class="tp-stick-knob" id="tp-move-knob"></div></div>
      <div id="tp-fire" class="tp-btn">FIRE</div>
      <div id="tp-jump" class="tp-btn">JUMP</div>
      <div id="tp-pie" class="tp-btn">PIE</div>
    `;
    document.body.appendChild(gamepad);

    // --- Movement joystick (left) ---
    setupStick('tp-move', 'tp-move-knob', function(dx, dy) {
      if (dx < -0.3) pressKey('KeyA', 'a'); else releaseKey('KeyA', 'a');
      if (dx > 0.3)  pressKey('KeyD', 'd'); else releaseKey('KeyD', 'd');
      if (dy < -0.3) pressKey('KeyW', 'w'); else releaseKey('KeyW', 'w');
      if (dy > 0.3)  pressKey('KeyS', 's'); else releaseKey('KeyS', 's');
    }, function() {
      releaseKey('KeyA', 'a');
      releaseKey('KeyD', 'd');
      releaseKey('KeyW', 'w');
      releaseKey('KeyS', 's');
    });

    // --- Buttons ---
    // FIRE = left mouse click (also unlocks audio on first tap)
    setupButton('tp-fire', function(down) {
      unlockAudio();
      sendMouse(down ? 'mousedown' : 'mouseup', 0);
    });

    // JUMP = spacebar
    setupButton('tp-jump', function(down) {
      unlockAudio();
      if (down) pressKey('Space', ' '); else releaseKey('Space', ' ');
    });

    // PIE = right mouse click
    setupButton('tp-pie', function(down) {
      unlockAudio();
      sendMouse(down ? 'mousedown' : 'mouseup', 2);
    });
  }

  // --- Joystick handler ---
  function setupStick(stickId, knobId, onMove, onRelease) {
    var stick = document.getElementById(stickId);
    var knob = document.getElementById(knobId);
    var activeTouch = null;
    var centerX, centerY, radius;

    stick.addEventListener('touchstart', function(e) {
      e.preventDefault();
      if (activeTouch !== null) return;
      var t = e.changedTouches[0];
      activeTouch = t.identifier;
      var rect = stick.getBoundingClientRect();
      centerX = rect.left + rect.width / 2;
      centerY = rect.top + rect.height / 2;
      radius = rect.width / 2;
    }, {passive: false});

    document.addEventListener('touchmove', function(e) {
      if (activeTouch === null) return;
      for (var i = 0; i < e.changedTouches.length; i++) {
        var t = e.changedTouches[i];
        if (t.identifier !== activeTouch) continue;
        var dx = (t.clientX - centerX) / radius;
        var dy = (t.clientY - centerY) / radius;
        var len = Math.sqrt(dx*dx + dy*dy);
        if (len > 1) { dx /= len; dy /= len; }
        knob.style.transform = 'translate(' + (dx * radius * 0.4 - 27) + 'px, ' + (dy * radius * 0.4 - 27) + 'px)';
        onMove(dx, dy);
      }
    }, {passive: true});

    function release() {
      activeTouch = null;
      knob.style.transform = 'translate(-50%, -50%)';
      onRelease();
    }
    document.addEventListener('touchend', function(e) {
      for (var i = 0; i < e.changedTouches.length; i++) {
        if (e.changedTouches[i].identifier === activeTouch) release();
      }
    });
    document.addEventListener('touchcancel', function(e) {
      for (var i = 0; i < e.changedTouches.length; i++) {
        if (e.changedTouches[i].identifier === activeTouch) release();
      }
    });
  }

  // --- Button handler ---
  function setupButton(btnId, callback) {
    var btn = document.getElementById(btnId);
    var active = false;

    btn.addEventListener('touchstart', function(e) {
      e.preventDefault();
      if (!active) { active = true; btn.classList.add('active'); callback(true); }
    }, {passive: false});

    btn.addEventListener('touchend', function(e) {
      e.preventDefault();
      if (active) { active = false; btn.classList.remove('active'); callback(false); }
    }, {passive: false});

    btn.addEventListener('touchcancel', function(e) {
      if (active) { active = false; btn.classList.remove('active'); callback(false); }
    });
  }

  if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', init);
  } else {
    init();
  }
})();
