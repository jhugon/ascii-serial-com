#!/usr/bin/env python3

import argparse
import os
import subprocess


def run_make(arch, CC, build_type, args):
    env = os.environ.copy()
    env.update({"arch": arch, "CC": CC, "build_type": build_type})

    subprocess.run(["make", "clean"], env=env)
    subprocess.run(["make", "install"], env=env)
    if args.unittest:
        subprocess.run(["make", "test"], env=env)


def main():

    available_targets = ["all", "native_gcc", "native_clang"]
    available_targets.sort()

    parser = argparse.ArgumentParser(description="Build tool for ASCII-Serial-Com")
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

    for target in targets:
        target_list = target.split("_")
        assert len(target_list) == 2
        arch = target_list[0]
        CC = target_list[1]
        for build_type in ["debug", "opt"]:
            run_make(arch, CC, build_type, args)


if __name__ == "__main__":
    main()
