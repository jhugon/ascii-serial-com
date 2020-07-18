#!/usr/bin/env python3

import argparse
import os
import os.path
import subprocess
import sys


def run_make(platform, CC, build_type, args):
    env = os.environ.copy()
    env.update({"platform": platform, "CC": CC, "build_type": build_type})
    if args.coverage and platform == "native" and build_type == "debug":
        env["coverage"] = "TRUE"

    outdir = "build_{}_{}_{}".format(platform, CC, build_type)
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
                stdout += "build_{}_{}_{}/{}\n\n".format(platform, CC, build_type, fn)
                stdout += cmpltProc.stdout
                success = success and (cmpltProc.returncode == 0)
        if args.coverage and platform == "native" and build_type == "debug":
            coverage_dir = os.path.join(outdir, "coverage")
            try:
                os.mkdir(coverage_dir)
            except FileExistsError:
                pass
            coverage_html_base = os.path.join(coverage_dir, "index.html")
            coverage_text = os.path.join(coverage_dir, "coverage.txt")
            subprocess.run(
                ["gcovr", "-o", coverage_text, "-e", "src/externals/.*"],
                env=env,
                check=True,
            )
            subprocess.run(
                [
                    "gcovr",
                    "--html-details",
                    "-o",
                    coverage_html_base,
                    "-e",
                    "src/externals/.*",
                ],
                env=env,
                check=True,
            )
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
        "--unittest", "-u", help="Perform unittests after building", action="store_true"
    )
    parser.add_argument(
        "--coverage",
        "-c",
        help="Collect coverage info when unittests are performed (sets -u)",
        action="store_true",
    )
    args = parser.parse_args()

    targets = args.targets
    if "all" in targets:
        targets = available_targets
        targets.remove("all")

    if args.coverage:
        args.unittest = True

    testOutBuffer = ""
    testsAllPass = True
    for target in targets:
        target_list = target.split("_")
        assert len(target_list) == 2
        platform = target_list[0]
        CC = target_list[1]
        for build_type in ["debug", "opt"]:
            testPass, testOutput = run_make(platform, CC, build_type, args)
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
