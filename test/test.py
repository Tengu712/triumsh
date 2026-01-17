import subprocess
import sys
from pathlib import Path

TRISH = Path("../build/trish")
error = False

def test_success(name):
	global error
	result = subprocess.run([TRISH, name], capture_output=True)
	if result.returncode == 2:
		error = True
		print(f"[test] fail: {name} (internal)")
	elif result.returncode != 0:
		error = True
		print(f"[test] fail: {name}")

def test_error(name):
	global error
	result = subprocess.run([TRISH, name], capture_output=True)
	if result.returncode == 2:
		error = True
		print(f"[test] fail: {name} (internal)")
	elif result.returncode != 1:
		error = True
		print(f"[test] fail: {name}")

def test_stdout(name):
	global error
	result = subprocess.run([TRISH, name], capture_output=True, text=True)
	if result.returncode == 2:
		error = True
		print(f"[test] fail: {name} (internal)")
		return
	expected = Path(f"{name}.txt").read_text()
	if result.returncode != 0 or result.stdout != expected:
		error = True
		print(f"[test] fail: {name}")

test_success("empty.trish")
test_success("simple.trish")

test_error("err-toplevel-space.trish")
test_error("err-unclosed-double.trish")
test_error("err-unclosed-double.trish")
test_error("err-command-not-found.trish")
test_error("err-command-exit-not-0.trish")

test_stdout("echo.trish")

if error:
	sys.exit(1)
else:
	print("[test] all tests passed.")
