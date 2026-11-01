#!/usr/bin/env python3
"""Run the example build with the repository's pinned Python environment."""

import importlib.metadata
import os
from pathlib import Path
import subprocess
import sys


def main():
    root = Path(__file__).resolve().parent.parent
    expected_python = root.joinpath('.python-version').read_text().strip()
    actual_python = '.'.join(map(str, sys.version_info[:3]))
    if actual_python != expected_python:
        sys.exit(f'Use Python {expected_python}; found {actual_python}. See README.md.')

    requirements = root.joinpath('requirements-build.txt').read_text().splitlines()
    for line in requirements:
        if not line or line.startswith(('#', '-')):
            continue
        name, expected = line.split('==')
        try:
            actual = importlib.metadata.version(name)
        except importlib.metadata.PackageNotFoundError:
            actual = 'not installed'
        if actual != expected:
            sys.exit(f'Install requirements-build.txt: {name} needs {expected}, found {actual}.')

    for line in root.joinpath('constraints-build.txt').read_text().splitlines():
        if not line or line.startswith('#'):
            continue
        name, expected = line.split('==')
        try:
            actual = importlib.metadata.version(name)
        except importlib.metadata.PackageNotFoundError:
            continue  # Platform-only tools are installed by pioarduino as needed.
        if actual != expected:
            sys.exit(f'Use a clean build environment: {name} needs {expected}, found {actual}.')

    build_env = os.environ.copy()
    constraints = str(root / 'constraints-build.txt')
    for name in ('PIP_CONSTRAINT', 'PIP_BUILD_CONSTRAINT', 'UV_CONSTRAINT', 'UV_BUILD_CONSTRAINT'):
        build_env[name] = constraints
    build_env['PYTHONUTF8'] = '1'
    return subprocess.call(
        [sys.executable, '-m', 'platformio', 'run', *sys.argv[1:]],
        cwd=root,
        env=build_env,
    )


if __name__ == '__main__':
    sys.exit(main())
