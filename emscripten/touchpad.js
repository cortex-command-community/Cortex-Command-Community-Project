/**
 * Touch gamepad overlay for Cortex Command on mobile.
 *
 * Creates virtual controls:
 * - Left side: movement joystick (WASD)
 * - Right side: aim joystick (mouse relative motion)
 * - Fire button (left mouse click)
 * - Jump button (W key)
 * - Pie menu button (right mouse click)
 *
 * Injects keyboard/mouse events into the canvas element so SDL picks them up.
 */
(function() {
  'use strict';

  // Only show on touch devices
  if (!('ontouchstart' in window)) return;

  var canvas = null;
  var gamepad = null;

  // Wait for canvas to exist
  function init() {
    canvas = document.getElementById('canvas');
    if (!canvas) { setTimeout(init, 500); return; }
    createGamepad();
  }

  // --- Key event injection ---
  function sendKey(code, key, down) {
    canvas.dispatchEvent(new KeyboardEvent(down ? 'keydown' : 'keyup', {
      code: code, key: key, bubbles: true, cancelable: true
    }));
  }

  function sendMouse(type, button, x, y) {
    var rect = canvas.getBoundingClientRect();
    canvas.dispatchEvent(new PointerEvent(type, {
      clientX: rect.left + (x || rect.width/2),
      clientY: rect.top + (y || rect.height/2),
      button: button || 0,
      bubbles: true,
      pointerId: 9999 + (button || 0)
    }));
  }

  function sendMouseMove(dx, dy) {
    var rect = canvas.getBoundingClientRect();
    canvas.dispatchEvent(new PointerEvent('pointermove', {
      clientX: rect.left + rect.width/2 + dx,
      clientY: rect.top + rect.height/2 + dy,
      movementX: dx,
      movementY: dy,
      bubbles: true,
      pointerId: 9998
    }));
  }

  // --- Gamepad creation ---
  function createGamepad() {
    gamepad = document.createElement('div');
    gamepad.id = 'touch-gamepad';
    gamepad.innerHTML = `
      <style>
        #touch-gamepad {
          position: fixed; bottom: 0; left: 0; right: 0;
          height: 200px; z-index: 10000;
          pointer-events: none;
          user-select: none; -webkit-user-select: none;
        }
        .tp-stick {
          position: absolute; bottom: 20px;
          width: 120px; height: 120px;
          border-radius: 50%;
          background: rgba(255,255,255,0.15);
          border: 2px solid rgba(255,255,255,0.3);
          pointer-events: auto;
          touch-action: none;
        }
        .tp-stick-knob {
          position: absolute;
          width: 50px; height: 50px;
          border-radius: 50%;
          background: rgba(255,255,255,0.5);
          top: 50%; left: 50%;
          transform: translate(-50%, -50%);
          pointer-events: none;
        }
        .tp-btn {
          position: absolute;
          width: 56px; height: 56px;
          border-radius: 50%;
          border: 2px solid rgba(255,255,255,0.4);
          pointer-events: auto;
          touch-action: none;
          display: flex; align-items: center; justify-content: center;
          font-family: monospace; font-size: 12px; font-weight: bold;
          color: rgba(255,255,255,0.7);
        }
        .tp-btn.active { background: rgba(255,255,255,0.4); }
        #tp-move { left: 20px; }
        #tp-aim { right: 20px; }
        #tp-fire { right: 160px; bottom: 30px; background: rgba(255,80,80,0.25); }
        #tp-jump { right: 100px; bottom: 100px; background: rgba(80,180,255,0.25); }
        #tp-pie  { right: 160px; bottom: 110px; background: rgba(255,200,80,0.25); font-size: 10px; width: 48px; height: 48px; }
        #tp-crouch { right: 100px; bottom: 20px; background: rgba(80,255,80,0.25); font-size: 10px; }
      </style>
      <div id="tp-move" class="tp-stick"><div class="tp-stick-knob" id="tp-move-knob"></div></div>
      <div id="tp-aim" class="tp-stick"><div class="tp-stick-knob" id="tp-aim-knob"></div></div>
      <div id="tp-fire" class="tp-btn">FIRE</div>
      <div id="tp-jump" class="tp-btn">JUMP</div>
      <div id="tp-pie" class="tp-btn">PIE</div>
      <div id="tp-crouch" class="tp-btn">DOWN</div>
    `;
    document.body.appendChild(gamepad);

    // --- Movement joystick (left) ---
    setupStick('tp-move', 'tp-move-knob', function(dx, dy) {
      // Map to WASD
      sendKey('KeyA', 'a', dx < -0.3);
      sendKey('KeyD', 'd', dx > 0.3);
      sendKey('KeyW', 'w', dy < -0.3);
      sendKey('KeyS', 's', dy > 0.3);
    }, function() {
      sendKey('KeyA', 'a', false);
      sendKey('KeyD', 'd', false);
      sendKey('KeyW', 'w', false);
      sendKey('KeyS', 's', false);
    });

    // --- Aim joystick (right) ---
    setupStick('tp-aim', 'tp-aim-knob', function(dx, dy) {
      // Send as mouse relative motion for aiming
      sendMouseMove(dx * 8, dy * 8);
    }, function() {});

    // --- Buttons ---
    setupButton('tp-fire', function(down) {
      if (down) sendMouse('pointerdown', 0);
      else sendMouse('pointerup', 0);
    });
    setupButton('tp-jump', function(down) {
      sendKey('KeyW', 'w', down);
    });
    setupButton('tp-pie', function(down) {
      if (down) sendMouse('pointerdown', 2);
      else sendMouse('pointerup', 2);
    });
    setupButton('tp-crouch', function(down) {
      sendKey('ControlLeft', 'Control', down);
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
        knob.style.transform = 'translate(' + (dx * radius * 0.4 - 25) + 'px, ' + (dy * radius * 0.4 - 25) + 'px)';
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

  // Start when DOM is ready
  if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', init);
  } else {
    init();
  }
})();
