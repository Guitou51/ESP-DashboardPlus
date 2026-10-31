#!/usr/bin/env python3
"""Build and run the native dashboard contract tests with MSVC or a C++17 compiler."""

import argparse
import os
from pathlib import Path
import shutil
import subprocess
import sys


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--environment', default='esp32dev', help='PlatformIO environment providing ArduinoJson')
    args = parser.parse_args()
    root = Path(__file__).resolve().parent.parent
    build = root / '.pio' / 'native-tests'
    build.mkdir(parents=True, exist_ok=True)
    json_include = root / '.pio' / 'libdeps' / args.environment / 'ArduinoJson' / 'src'
    if not (json_include / 'ArduinoJson.h').is_file():
        sys.exit(f'Build the {args.environment} example first to install ArduinoJson.')
    includes = [root / 'tests' / 'native' / 'stubs', root / 'src', json_include]
    sources = sorted((root / 'src').rglob('*.cpp')) + sorted((root / 'tests' / 'native').glob('*.cpp'))
    executable = build / ('contracts.exe' if os.name == 'nt' else 'contracts')
    if os.name == 'nt':
        vswhere = Path(os.environ.get('ProgramFiles(x86)', 'C:/Program Files (x86)')) / 'Microsoft Visual Studio/Installer/vswhere.exe'
        if not vswhere.is_file():
            sys.exit('Install Visual Studio C++ Build Tools to run the native tests on Windows.')
        installation = subprocess.check_output([
            str(vswhere), '-latest', '-products', '*', '-requires',
            'Microsoft.VisualStudio.Component.VC.Tools.x86.x64', '-property', 'installationPath',
        ], text=True).strip()
        if not installation:
            sys.exit('No MSVC C++ toolchain found.')
        vcvars = Path(installation) / 'VC/Auxiliary/Build/vcvars64.bat'
        command = ['cl', '/nologo', '/std:c++17', '/EHsc', '/DARDUINO_ARCH_ESP32', '/Fe:contracts.exe']
        command += ['/I' + str(path) for path in includes] + list(map(str, sources))
        batch = build / 'build.cmd'
        batch.write_text('@echo off\ncall "' + str(vcvars) + '" >nul\n' + subprocess.list2cmdline(command) + '\n', encoding='utf-8')
        subprocess.run(['cmd', '/d', '/c', str(batch)], cwd=build, check=True)
    else:
        compiler = os.environ.get('CXX') or shutil.which('c++')
        if not compiler:
            sys.exit('Install a C++17 compiler or set CXX.')
        command = [compiler, '-std=c++17', '-DARDUINO_ARCH_ESP32']
        command += ['-I' + str(path) for path in includes] + list(map(str, sources))
        subprocess.run(command + ['-o', str(executable)], cwd=build, check=True)
    subprocess.run([str(executable), str(root / 'tests/native/expected_messages.jsonl')], cwd=root, check=True)


if __name__ == '__main__':
    main()
