#!/usr/bin/env python3
"""Checks that the simulation is deterministic, by running scenarios several times and comparing the runs.

Each scenario is first run to record a trace of determinism checkpoints, then run again to verify against that trace.
The runs are headless unless asked otherwise. A run that doesn't match reports the first checkpoint it diverged at, as the sim update it was made in,
the hash of the checked state, and a label that says what was checked and where in the update.
The runs use a copy of the game's settings with what they need set in it, so the game's own settings are left alone.
Several scenarios can be run at the same time, to get through many of them sooner.

Run this from the game's working directory. For example:
    python Tools/determinism_test.py
    python Tools/determinism_test.py --activity GAScripted "Wave Defense" "Grasslands" --updates 3000 --verify-windowed
    python Tools/determinism_test.py --jobs 2
"""

import argparse
import os
import subprocess
import sys
import tempfile
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

# What gets run when no scenarios are asked for. Each is the type name and preset name of an Activity, and the preset name of a Scene.
# These are missions where things happen on their own. Most other Activities start out waiting for the player to place their brain.
DEFAULT_ACTIVITIES = [
    ("GAScripted", "Dummy Assault", "Dummy Assault"),
    ("GAScripted", "Decision Day", "Decision Day"),
]

# The settings that the runs need, whatever the game's own settings say.
TEST_SETTINGS = {
    # Pathing requests otherwise complete after however many sim updates the pathfinding threads happen to take.
    "ForceImmediatePathingRequestCompletion": "1",
    # A windowed run that crashes otherwise waits for someone to click its crash dialog away.
    "DisableCrashDialogs": "1",
}


def write_test_settings(path):
    """Writes a copy of the game's settings to the path, with the settings that the runs need."""
    lines = Path("Userdata/Settings.ini").read_text().splitlines()
    lines = [line for line in lines if line.split("=")[0].strip() not in TEST_SETTINGS]
    # The first line is the class name, which the settings have to come after.
    lines[1:1] = [f"\t{name} = {value}" for name, value in TEST_SETTINGS.items()]
    path.write_text("\n".join(lines) + "\n")


def run_game(exe, scenario_args, updates, checker_args, headless, settings, timeout):
    """Runs the game through one scenario. Returns whether it finished properly, and what it reported if not."""
    args = [exe, *scenario_args, "-sim-updates", str(updates), *checker_args]
    if headless:
        args.append("-headless")
    # This is where the game looks for its settings file, when set.
    environment = {**os.environ, "CCCP_SETTINGSPATH": str(settings)}
    try:
        # The game reports a determinism break on stderr.
        result = subprocess.run(args, timeout=timeout, stderr=subprocess.PIPE, text=True, env=environment)
    except subprocess.TimeoutExpired:
        return False, f"It was still going after {timeout} seconds."
    return result.returncode == 0, result.stderr.strip() or f"It exited with code {result.returncode}."


def test_scenario(name, scenario_args, options, trace_dir):
    """Records a trace of the scenario and verifies further runs against it. Returns whether all the runs matched."""
    file_name = name.replace(" ", "_").replace("/", "_")
    trace = trace_dir / (file_name + ".trace")
    # The game can write to its settings file, so each scenario gets one of its own in case several are running at the same time.
    settings = trace_dir / (file_name + ".Settings.ini")
    write_test_settings(settings)

    mode = "windowed" if options.windowed else "headless"

    print(f"{name}: recording {options.updates} sim updates, {mode}...", flush=True)
    finished, report = run_game(options.exe, scenario_args, options.updates, ["-determinism-record", str(trace)], mode == "headless", settings, options.timeout)
    if not finished:
        print(f"{name}: FAILED, the recording run didn't finish properly\n{report}")
        return False
    # A scenario that fails to launch still exits normally, it just doesn't make any checkpoints.
    if trace.stat().st_size == 0:
        print(f"{name}: FAILED, the scenario didn't start")
        return False

    # A windowed run runs in real time and so takes the longest to fail, which is why the extra one goes last.
    verify_runs = [mode] * options.runs + (["windowed"] if options.verify_windowed else [])
    for run, mode in enumerate(verify_runs, start=1):
        print(f"{name}: verifying, {mode} run {run} of {len(verify_runs)}...", flush=True)
        finished, report = run_game(options.exe, scenario_args, options.updates, ["-determinism-verify", str(trace)], mode == "headless", settings, options.timeout)
        if not finished:
            print(f"{name}: FAILED\n{report}")
            return False

    print(f"{name}: passed")
    return True


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--exe", default="Cortex Command.exe", help="the game executable to test (default: %(default)s)")
    parser.add_argument("--updates", type=int, default=1000, help="how many sim updates to run each scenario for (default: %(default)s)")
    parser.add_argument("--runs", type=int, default=2, help="how many runs to verify against the recorded one (default: %(default)s)")
    parser.add_argument("--jobs", type=int, default=1, help="how many scenarios to run at the same time (default: %(default)s)")
    parser.add_argument("--windowed", action="store_true", help="do the runs with the game window, audio and real time pacing, instead of headless")
    parser.add_argument("--verify-windowed", action="store_true", help="also verify a windowed run against the recorded one, to check that the sim doesn't depend on drawing, audio or frame pacing")
    parser.add_argument("--save", action="append", default=[], metavar="NAME", help="a saved game to run, can be given several times")
    parser.add_argument("--activity", action="append", default=[], nargs=3, metavar=("TYPE", "ACTIVITY", "SCENE"), help="an Activity and Scene to run, can be given several times")
    parser.add_argument("--trace-dir", type=Path, help="where to keep the recorded traces, instead of a temporary directory")
    parser.add_argument("--timeout", type=int, default=1800, help="how many seconds a single run can take before it is given up on (default: %(default)s)")
    options = parser.parse_args()
    options.exe = str(Path(options.exe).resolve())

    scenarios =[(f"Save {save}", ["-save", save]) for save in options.save]
    scenarios += [(f"{activity} on {scene}", ["-activity", activity_type, activity, scene]) for activity_type, activity, scene in options.activity or ([] if options.save else DEFAULT_ACTIVITIES)]

    with tempfile.TemporaryDirectory() as temp_dir:
        trace_dir = (options.trace_dir or Path(temp_dir)).resolve()
        trace_dir.mkdir(parents=True, exist_ok=True)
        with ThreadPoolExecutor(options.jobs) as pool:
            results = list(pool.map(lambda scenario: test_scenario(*scenario, options, trace_dir), scenarios))
        failed = [name for (name, scenario_args), passed in zip(scenarios, results) if not passed]

    print(f"\n{len(scenarios) - len(failed)} of {len(scenarios)} scenarios are deterministic.")
    for name in failed:
        print(f"  FAILED: {name}")
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
