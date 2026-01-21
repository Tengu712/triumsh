import os
import subprocess
import sys
from pathlib import Path

TRISH = Path("../build/trish")
error = False

def test_unit(name):
	global error
	if subprocess.run([name]).returncode != 0:
		error = True

def test_trish(name, expected):
	global error
	print(f"test: {name} ... ", end="")
	result = subprocess.run([TRISH, name], capture_output=True, encoding="utf-8", env={**os.environ, "TEST_ENV": "hoge"})
	if result.returncode == 2:
		error = True
		print("fail (internal)")
		return None
	elif result.returncode != expected:
		error = True
		print("fail")
		return None
	else:
		print("ok")
		return result

def test_success(name):
	test_trish(name, 0)

def test_error(name):
	test_trish(name, 1)

def test_stdout(name):
	result = test_trish(name, 0)
	if result:
		global error
		print(f"test: {name}.txt ... ", end="")
		if result.stdout != Path(f"{name}.txt").read_text(encoding="utf-8"):
			error = True
			print("fail")
		else:
			print("ok")

test_unit("../build/test_cursor")

test_success("empty.trish")

test_error("err-toplevel-space.trish")
test_error("err-unclosed-double.trish")
test_error("err-unclosed-double.trish")
test_error("err-command-not-found.trish")
test_error("err-command-exit-not-0.trish")
test_error("err-invalid-escape.trish")
test_error("err-invalid-env.trish")

test_stdout("simple.trish")
test_stdout("echo.trish")
test_stdout("escape.trish")
test_stdout("env.trish")

if error:
	print("some tests failed.")
	sys.exit(1)
else:
	print("all tests passed.")
