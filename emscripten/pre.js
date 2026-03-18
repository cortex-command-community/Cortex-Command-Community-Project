/**
 * Cortex Command Community Project — Emscripten pre.js
 *
 * Loaded before the compiled WASM module starts. Sets up helpers used by the
 * C++ ↔ JS bridge (emscripten/shell.html and Source/System/WebPlatform.cpp).
 */

// ---------------------------------------------------------------------------
// File system helpers — allow lazy HTTP fetching of non-base-rte data modules
// ---------------------------------------------------------------------------

/**
 * Fetch a data module (.rte or .rte.zip) from the server and mount it into
 * Emscripten's MEMFS at the expected path. Called from C++ via EM_ASM when
 * PresetMan encounters an unloaded module.
 *
 * @param {string} modulePath  e.g. "Data/Missions.rte"
 * @param {function} onDone    Called with (success: bool) when complete
 */
function ccFetchModule(modulePath, onDone) {
  // The server must serve .rte directories as .zip files for lazy fetch.
  var zipUrl = modulePath + '.zip';
  fetch(zipUrl)
    .then(function(r) {
      if (!r.ok) throw new Error('HTTP ' + r.status + ' for ' + zipUrl);
      return r.arrayBuffer();
    })
    .then(function(buf) {
      // Write raw bytes to MEMFS under the module path.
      // The game's DataModule loader uses minizip-ng to extract .rte.zip.
      var bytes = new Uint8Array(buf);
      FS.writeFile(modulePath + '.zip', bytes);
      if (onDone) onDone(true);
    })
    .catch(function(err) {
      console.error('[CC] Failed to fetch module', modulePath, err);
      if (onDone) onDone(false);
    });
}

// Expose to C++ via emscripten_run_script / EM_ASM
Module['ccFetchModule'] = ccFetchModule;

// ---------------------------------------------------------------------------
// Persistent save data via IndexedDB (IDBFS)
// ---------------------------------------------------------------------------

/**
 * Called once after the WASM runtime is ready. Mounts the Userdata directory
 * on IDBFS so save games and settings persist across page reloads.
 */
Module['onRuntimeInitialized_pre'] = function() {
  // Mount IDBFS for persistent data. The C++ side must call
  // WebPlatform::SyncSavesToDisk() after writing saves.
  try {
    FS.mkdir('/Userdata');
  } catch (e) { /* already exists */ }
  FS.mount(IDBFS, {}, '/Userdata');

  // Populate MEMFS from IndexedDB on startup (true = populate from IDB)
  FS.syncfs(true, function(err) {
    if (err) console.warn('[CC] IDBFS initial sync error:', err);
    else console.log('[CC] IDBFS loaded persistent save data.');
  });
};

// ---------------------------------------------------------------------------
// Keyboard focus — ensure the canvas gets keyboard events immediately
// ---------------------------------------------------------------------------
Module['postRun'] = Module['postRun'] || [];
Module['postRun'].push(function() {
  var canvas = document.getElementById('canvas');
  if (canvas) {
    canvas.focus();
    // Prevent browser from stealing arrow keys / space / etc.
    canvas.addEventListener('keydown', function(e) {
      var consumed = [32,37,38,39,40,9]; // space, arrows, tab
      if (consumed.indexOf(e.keyCode) !== -1) e.preventDefault();
    });
  }
});

// ---------------------------------------------------------------------------
// Audio context unlock — browsers require a user gesture before audio plays.
// SDL2/Emscripten handles this automatically; this is a belt-and-suspenders
// handler for the overlay canvas path.
// ---------------------------------------------------------------------------
(function() {
  var unlocked = false;
  function unlock() {
    if (unlocked) return;
    unlocked = true;
    // If the AudioContext is in suspended state, resume it.
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
