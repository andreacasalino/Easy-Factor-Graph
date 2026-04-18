from argparse import ArgumentParser
import subprocess
import os
import re

def get_cwd():
    res = os.path.dirname(__file__)
    res = os.path.join(res, '..')
    return os.path.abspath(res)

CWD = get_cwd()

def gather_files(args):
    cmd = """git diff-tree --no-commit-id --name-only -r {}""".format("""$(git rev-parse HEAD)""" if args.head else "master..HEAD")
    res = subprocess.run(cmd, shell=True, capture_output=True, text=True, check=True, cwd=CWD)
    for file in res.stdout.splitlines():
        file = file.strip()
        if len(file) == 0:
            continue
        yield file

def gather_cpp_files(args):
    for file in gather_files(args):
        if not os.path.exists(file):
            continue
        if any(re.search("(.*?).{}".format(p), file) for p in ['h', 'cpp']):
            yield file

def prune_includes(args):
    for file in gather_cpp_files(args):
        if args.verbose:
            print("pruning includes of {}".format(file))
        cmd = """clang-tidy -checks='-*,misc-include-cleaner' -fix {}""".format(file)
        subprocess.run(cmd, shell=True, stdout=None, stderr=None, cwd=CWD)

def apply_format(args):
    for file in gather_cpp_files(args):
        if args.verbose:
            print("formatting {}".format(file))
        cmd = """clang-format -i {}""".format(file)
        subprocess.run(cmd, shell=True, stdout=None, stderr=None, cwd=CWD)

def main():
    parser = ArgumentParser()
    parser.add_argument("--avoid_prune_includes", action='store_true')
    parser.add_argument("--avoid_apply_format", action='store_true')
    parser.add_argument("--head", default=None, help="If not specified, the set of files changed from the HEAD commit till the one in master from which this branch branched is assumed. Otherwise only the files changed in the HEAD commit")
    parser.add_argument("--verbose", action='store_true')
    parser.add_argument("--dry", action='store_true')
    parser.add_argument("--dry_cpp", action='store_true')
    args = parser.parse_args()

    if args.dry:
        for file in gather_files(args):
            print("-- {}".format(file))
        return
    
    if args.dry_cpp:
        for file in gather_cpp_files(args):
            print("-- {}".format(file))
        return

    if not args.avoid_prune_includes:
        prune_includes(args)
    if not args.avoid_apply_format:
        apply_format(args)

if __name__ == '__main__':
    main()
