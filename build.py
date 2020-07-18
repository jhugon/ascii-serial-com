#!/usr/bin/env python3

import argparse
import os
import subprocess
import sys


def run_make(arch, CC, build_type, args):
    env = os.environ.copy()
    env.update({"arch": arch, "CC": CC, "build_type": build_type})

    try:
        subprocess.run(["make", "clean"], env=env, check=True)
    except subprocess.CalledProcessError as e:
        sys.exit(1)
    try:
        subprocess.run(["make", "install"], env=env, check=True)
    except subprocess.CalledProcessError as e:
        sys.exit(1)
    if args.unittest:
        cmpltProc = subprocess.run(
            ["make", "test"],
            env=env,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
        )
        return cmpltProc.returncode == 0, cmpltProc.stdout
    return None, None


def main():

    available_targets = ["all", "native_gcc", "native_clang"]

    parser = argparse.ArgumentParser(
        description="Build tool for ASCII-Serial-Com. Will stop and exit with status 1 if build fails, but continue if tests fail."
    )
    parser.add_argument(
        "targets",
        help="Targets to build",
        nargs="*",
        default="all",
        choices=available_targets,
    )
    parser.add_argument(
        "--unittest", "-u", help="Targets to build", action="store_true"
    )
    args = parser.parse_args()

    targets = args.targets
    if "all" in targets:
        targets = available_targets
        targets.remove("all")

    testOutBuffer = ""
    for target in targets:
        target_list = target.split("_")
        assert len(target_list) == 2
        arch = target_list[0]
        CC = target_list[1]
        for build_type in ["debug", "opt"]:
            testPass, testOutput = run_make(arch, CC, build_type, args)
            if args.unittest:
                testOutBuffer += testOutput

    if args.unittest:
        print()
        print("------------------------------------------")
        print("------------ Test Results ----------------")
        print("------------------------------------------")
        print()
        print(testOutBuffer)


if __name__ == "__main__":
    main()
