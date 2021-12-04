#!/usr/bin/env python3

import argparse
import os
import os.path
import subprocess
import sys


def run_make(platform, CC, build_type, args):
    env = os.environ.copy()
    env.update({"platform": platform, "CC": CC, "build_type": build_type})
    exe_path = env["PATH"]
    exe_path = os.path.abspath("tools/avr-install/bin") + ":" + exe_path
    env["PATH"] = exe_path
    if args.coverage and platform == "native" and build_type == "debug":
        env["coverage"] = "TRUE"

    outdir = "build/{}_{}_{}".format(platform, CC, build_type)
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
        print(outdir, flush=True)
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
                stdout += "build/{}_{}_{}/{}\n\n".format(platform, CC, build_type, fn)
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


def run_integration_tests():
    env = os.environ.copy()

    test_dir = "integration_tests"
    tests = os.listdir(test_dir)
    success = True
    print()
    print("========== Integration Tests =============")
    print()
    stdout = ""
    for test in tests:
        if test[-3:] != ".py":
            continue
        testfn = os.path.join(test_dir, test)
        cmpltProc = subprocess.run(
            ["python3", "-m", "unittest", testfn],
            env=env,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
        )
        success = success and (cmpltProc.returncode == 0)
        print(cmpltProc.stdout, flush=True)
        stdout += cmpltProc.stdout
    if success:
        print("Integration Tests All Pass!")
    else:
        print("Integration Test Failure")
    return success, stdout


def run_python_unit_tests():
    env = os.environ.copy()

    success = True
    stdout = ""
    cmpltProc = subprocess.run(
        ["python3", "-m", "unittest", "tests"],
        env=env,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
        cwd=os.path.abspath("python"),
    )
    success = cmpltProc.returncode == 0
    stdout += cmpltProc.stdout
    print()
    print("========== Python Unit Tests =============")
    print()
    print(stdout)
    if success:
        print("Python Unit Tests All Pass!")
    else:
        print("Python Unit Test Failure")
    return success, stdout


def run_python_mypy():
    env = os.environ.copy()

    success = True
    stdout = ""
    cmpltProc = subprocess.run(
        ["mypy", "asciiserialcom", "tests"],
        env=env,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
        cwd=os.path.abspath("python"),
    )
    success = cmpltProc.returncode == 0
    stdout += cmpltProc.stdout
    print()
    print("========== Python MyPy =============")
    print()
    print(stdout)
    if success:
        print("MyPy Pass!")
    else:
        print("MyPy Failure")
    return success, stdout


def print_FW_size(targets):
    env = os.environ.copy()
    exe_path = env["PATH"]
    exe_path = os.path.abspath("tools/avr-install/bin") + ":" + exe_path
    env["PATH"] = exe_path
    avr_fwfiles = []
    cortex_fwfiles = []
    for target in sorted(targets):
        for build_type in ["debug", "opt"]:
            target_list = target.split("_")
            assert len(target_list) == 2
            platform = target_list[0]
            if "avr" in platform:
                CC = target_list[1]
                outdir = "build/{}_{}_{}".format(platform, CC, build_type)
                outdir = os.path.abspath(outdir)
                outfiles = os.listdir(outdir)
                fwfiles = [os.path.join(outdir, x) for x in outfiles if x[-2:] != ".a"]
                avr_fwfiles += fwfiles
            elif "cortex" in platform:
                CC = target_list[1]
                outdir = "build/{}_{}_{}".format(platform, CC, build_type)
                outdir = os.path.abspath(outdir)
                outfiles = os.listdir(outdir)
                fwfiles = [os.path.join(outdir, x) for x in outfiles if x[-2:] != ".a"]
                cortex_fwfiles += fwfiles
    if len(avr_fwfiles) + len(cortex_fwfiles) == 0:
        return
    print("========= Firmware Size ==========")
    cmpltProc = subprocess.run(["avr-size", "-G"] + avr_fwfiles, env=env, text=True,)
    cmpltProc = subprocess.run(
        ["arm-none-eabi-size", "-G"] + cortex_fwfiles, env=env, text=True,
    )


def run_doxygen():
    cmpltProc = subprocess.run(["doxygen", "doc/Doxyfile"],)


def main():

    available_targets = [
        "all",
        "default",
        "native_gcc",
        "native_clang",
        "avr5_gcc",
        "avr6_gcc",
        "avrxmega3_gcc",
        "cortex-m0_gcc",
        "cortex-m3_gcc",
        "cortex-m4_gcc",
        "cortex-m7_gcc",
    ]
    default_targets = [
        "native_gcc",
        "native_clang",
        "avr5_gcc",
        "cortex-m0_gcc",
    ]

    parser = argparse.ArgumentParser(
        description="Build tool for ASCII-Serial-Com. Will stop and exit with status 1 if build fails, but continue if tests fail."
    )
    parser.add_argument(
        "targets",
        help="Targets to build; defaults: {}".format(default_targets),
        nargs="*",
        default="default",
        choices=available_targets,
    )
    parser.add_argument(
        "--unittest", "-u", help="Perform unittests after building", action="store_true"
    )
    parser.add_argument(
        "--integrationtest",
        "-i",
        help="Perform integration tests after building",
        action="store_true",
    )
    parser.add_argument(
        "--coverage",
        "-c",
        help="Collect coverage info when unittests are performed (sets -u)",
        action="store_true",
    )
    parser.add_argument(
        "--doxygen",
        "-d",
        help="Build Doxygen source documentation",
        action="store_true",
    )
    parser.add_argument(
        "--mypy",
        "-m",
        help="Run mypy on python code (if mypy package is installed)",
        action="store_true",
    )
    args = parser.parse_args()

    targets = args.targets

    print(targets)
    if type(targets) == str:
        targets = default_targets
    elif "all" in targets:
        targets = available_targets
        targets.remove("all")
        targets.remove("default")
    elif "default" in targets:
        targets += default_targets
        targets.remove("default")
    print(targets)

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

    pythonTestsPass = False
    if args.unittest:
        print()
        print("==========================================")
        print("========== Unit Test Results =============")
        print("==========================================")
        print()
        print(testOutBuffer)

        if testsAllPass:
            print("Unit Tests All Pass!")
        else:
            print("Unit Test Failure")

        pythonTestsPass, testOutput = run_python_unit_tests()

    mypyPass = False
    if args.mypy:
        mypyPass, testOutput = run_python_mypy()

    integrationTestsPass = False
    if args.integrationtest:
        integrationTestsPass, testOutput = run_integration_tests()

    if args.doxygen:
        run_doxygen()

    print_FW_size(targets)

    if args.unittest or args.integrationtest or args.mypy:
        print()
        print("============= Test Summary ===============")
        print()
    if args.unittest:
        if testsAllPass:
            print("Unit Tests All Pass!")
        else:
            print("Unit Test Failure")
        if pythonTestsPass:
            print("Python Unit Tests All Pass!")
        else:
            print("Python Unit Test Failure")
    if args.integrationtest:
        if integrationTestsPass:
            print("Integration Tests All Pass!")
        else:
            print("Integration Test Failure")
    if args.mypy:
        if mypyPass:
            print("MyPy Python Static Analysis Pass!")
        else:
            print("MyPy Python Static Analysis Failure")
    if args.unittest or args.integrationtest or args.mypy:
        print()


if __name__ == "__main__":
    main()
