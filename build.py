#!/usr/bin/env python3

import argparse
import os
import os.path
import subprocess
import sys


def run_make(arch, CC, build_type, args):
    env = os.environ.copy()
    env.update({"arch": arch, "CC": CC, "build_type": build_type})

    outdir = "build_{}_{}_{}".format(arch, CC, build_type)
    outdir = os.path.abspath(outdir)

    try:
        subprocess.run(["make", "clean"], env=env, check=True)
    except subprocess.CalledProcessError as e:
        sys.exit(1)
    try:
        subprocess.run(["make", "install"], env=env, check=True)
    except subprocess.CalledProcessError as e:
        sys.exit(1)
    if args.unittest:
        stdout = ""
        success = True
        print(outdir)
        for fn in os.listdir(outdir):
            print(fn)
            if "test_" == fn[:5]:
                fnabs = os.path.join(outdir, fn)
                print(fnabs)
                cmpltProc = subprocess.run(
                    [fnabs],
                    env=env,
                    stdout=subprocess.PIPE,
                    stderr=subprocess.STDOUT,
                    text=True,
                )
                stdout += (
                    "\n==========================================================\n\n"
                )
                stdout += "build_{}_{}_{}/{}\n\n".format(arch, CC, build_type, fn)
                stdout += cmpltProc.stdout
                success = success and (cmpltProc.returncode == 0)
        return success, stdout
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
    testsAllPass = True
    for target in targets:
        target_list = target.split("_")
        assert len(target_list) == 2
        arch = target_list[0]
        CC = target_list[1]
        for build_type in ["debug", "opt"]:
            testPass, testOutput = run_make(arch, CC, build_type, args)
            if args.unittest:
                testOutBuffer += testOutput
                testsAllPass = testsAllPass and testPass

    if args.unittest:
        print()
        print("==========================================")
        print("============ Test Results ================")
        print("==========================================")
        print()
        print(testOutBuffer)

        if testsAllPass:
            print("Tests All Pass!")
        else:
            print("Tests Failure")


if __name__ == "__main__":
    main()
