#!/usr/bin/env python3
"""
Setup script for UpVote Arduino BattleBot project.

This script ensures all dependencies are installed and the project is ready to build:
- Initializes and updates git submodules
- Verifies PlatformIO is installed
- Installs PlatformIO dependencies
- Validates the build environment

Safe to run multiple times (idempotent).
"""

from __future__ import annotations

import os
import subprocess
import sys
import shutil
from pathlib import Path
from typing import List, Optional

# Project root is always where this script lives
PROJECT_ROOT = Path(__file__).parent.resolve()


class SetupError(Exception):
    """Custom exception for setup failures."""
    pass


def run_command(
    cmd: List[str],
    description: str,
    check: bool = True,
    cwd: Optional[Path] = None
) -> subprocess.CompletedProcess:
    """Run a command and handle errors.

    Args:
        cmd: Command and arguments to run
        description: Human-readable description for logging
        check: If True, raise exception on non-zero exit
        cwd: Working directory (defaults to PROJECT_ROOT)
    """
    print(f"  → {description}...")
    try:
        result = subprocess.run(
            cmd,
            capture_output=True,
            text=True,
            check=check,
            cwd=cwd or PROJECT_ROOT
        )
        if result.returncode == 0:
            print(f"    ✓ Success")
        return result
    except subprocess.CalledProcessError as e:
        print(f"    ✗ Failed: {e.stderr.strip()}")
        raise SetupError(f"{description} failed") from e


def check_git() -> None:
    """Verify git is installed."""
    if not shutil.which("git"):
        raise SetupError("Git is not installed. Please install git first.")
    print("  ✓ Git is installed")


def check_platformio() -> Optional[str]:
    """Check if PlatformIO is available and return the command path."""
    # Check common locations
    pio_locations = [
        "platformio",
        "pio",
        str(Path.home() / ".platformio" / "penv" / "bin" / "platformio"),
    ]

    for pio in pio_locations:
        found = shutil.which(pio)
        if found:
            print(f"  ✓ PlatformIO found: {found}")
            return found

    return None


def install_platformio() -> str:
    """Attempt to install PlatformIO."""
    print("  → Attempting to install PlatformIO...")

    try:
        subprocess.run(
            [sys.executable, "-m", "pip", "install", "platformio"],
            capture_output=True,
            text=True,
            check=True
        )
        print("    ✓ PlatformIO installed via pip")

        # Verify installation and return actual path
        pio_cmd = check_platformio()
        if pio_cmd:
            return pio_cmd

        # Fallback: might need shell rehash
        return "platformio"
    except subprocess.CalledProcessError:
        raise SetupError(
            "Could not install PlatformIO. Please install it manually:\n"
            "  pip install platformio\n"
            "  or\n"
            "  Install the PlatformIO VS Code extension"
        )


def check_git_repo() -> None:
    """Verify we're in a git repository."""
    git_dir = PROJECT_ROOT / ".git"
    if not git_dir.exists():
        raise SetupError(
            f"Not a git repository: {PROJECT_ROOT}\n"
            "Please clone the repository properly:\n"
            "  git clone --recursive <repo-url>"
        )
    print("  ✓ Git repository detected")


def setup_submodules() -> None:
    """Initialize and update git submodules to their pinned commits."""
    # Check if .gitmodules exists
    gitmodules = PROJECT_ROOT / ".gitmodules"
    if not gitmodules.exists():
        print("  ✓ No submodules configured")
        return

    # Initialize and checkout submodules to the commit pinned in the parent repo
    # Note: We intentionally do NOT use --remote, as that would update to
    # the latest upstream commit, potentially breaking builds.
    run_command(
        ["git", "submodule", "update", "--init", "--recursive"],
        "Initializing git submodules (using pinned commits)"
    )


def verify_submodules() -> None:
    """Verify all submodules are properly checked out."""
    sub_dir = PROJECT_ROOT / "sub"

    if not sub_dir.exists():
        return

    for submodule in sub_dir.iterdir():
        if submodule.is_dir() and not submodule.name.startswith('.'):
            # Check for library manifest files
            has_manifest = any(
                (submodule / f).exists()
                for f in ["library.json", "library.properties", "module.json"]
            )
            if has_manifest:
                print(f"  ✓ Submodule '{submodule.name}' is ready")
            elif not any(submodule.iterdir()):
                # Empty directory - submodule not initialized
                raise SetupError(
                    f"Submodule '{submodule.name}' is empty. "
                    "Try running: git submodule update --init --recursive"
                )
            else:
                # Has files but no manifest - might be a non-library submodule
                print(f"  ⚠ Submodule '{submodule.name}' has no library manifest (may be OK)")


def install_pio_dependencies(pio_cmd: str) -> None:
    """Install PlatformIO library dependencies."""
    # Check platformio.ini exists
    if not (PROJECT_ROOT / "platformio.ini").exists():
        raise SetupError("platformio.ini not found in project root")

    # Install dependencies (idempotent - skips already installed)
    run_command(
        [pio_cmd, "pkg", "install"],
        "Installing PlatformIO dependencies"
    )


def verify_build(pio_cmd: str) -> None:
    """Verify the project can build."""
    run_command(
        [pio_cmd, "run"],
        "Verifying build"
    )


def main() -> int:
    """Main setup routine."""
    print("=" * 60)
    print("UpVote Arduino BattleBot - Project Setup")
    print("=" * 60)
    print()

    try:
        # Step 1: Check prerequisites
        print("[1/6] Checking prerequisites...")
        check_git()

        # Step 2: Check git repository
        print("\n[2/6] Checking git repository...")
        check_git_repo()

        # Step 3: Check/install PlatformIO
        print("\n[3/6] Checking PlatformIO...")
        pio_cmd = check_platformio()
        if not pio_cmd:
            pio_cmd = install_platformio()

        # Step 4: Setup submodules
        print("\n[4/6] Setting up git submodules...")
        setup_submodules()
        verify_submodules()

        # Step 5: Install PlatformIO dependencies
        print("\n[5/6] Installing PlatformIO dependencies...")
        install_pio_dependencies(pio_cmd)

        # Step 6: Verify build
        print("\n[6/6] Verifying build...")
        verify_build(pio_cmd)

        print()
        print("=" * 60)
        print("✓ Setup complete! Project is ready to build.")
        print("=" * 60)
        print()
        print("Next steps:")
        print(f"  • Build:   {pio_cmd} run")
        print(f"  • Upload:  {pio_cmd} run --target upload")
        print(f"  • Monitor: {pio_cmd} device monitor")
        print()
        return 0

    except SetupError as e:
        print()
        print("=" * 60)
        print(f"✗ Setup failed: {e}")
        print("=" * 60)
        return 1
    except KeyboardInterrupt:
        print("\n\nSetup cancelled by user.")
        return 130


if __name__ == "__main__":
    sys.exit(main())
