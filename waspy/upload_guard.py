"""Checks every /upload request before anything touches the disk.

/upload used to take the client's filename verbatim and save it from a root
process, so `filename="../../root/.ssh/authorized_keys"` wrote anywhere on the
filesystem. Now: a secret token is required (uploads are off if none is
configured), the saved name is derived only from a validated lambda name, and
the body must be a real WASM module of bounded size.
"""
import hmac
import os
import re

LAMBDAS_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)), "lambdas")
TOKEN_FILE = os.path.join(os.path.dirname(os.path.abspath(__file__)), ".upload-token")
TOKEN_ENV = "WASP_UPLOAD_TOKEN"
LAMBDA_NAME = re.compile(r"^[A-Za-z][A-Za-z0-9_-]{0,63}$")
WASM_MAGIC = b"\0asm"
MAX_WASM_BYTES = 32 * 1024 * 1024


class UploadRejected(Exception):
	def __init__(self, status, reason):
		super().__init__(reason)
		self.status = status


def configured_token():
	token = os.environ.get(TOKEN_ENV, "")
	if not token and os.path.exists(TOKEN_FILE):
		with open(TOKEN_FILE) as fh:
			token = fh.read().strip()
	return token


def presented_token(request):
	bearer = request.headers.get("Authorization", "")
	if bearer.startswith("Bearer "):
		return bearer[len("Bearer "):]
	return request.headers.get("X-Upload-Token") or request.form.get("token") or ""


def check_token(request):
	expected = configured_token()
	if not expected:
		raise UploadRejected(403, f"uploads are disabled: no {TOKEN_ENV} or {TOKEN_FILE} configured")
	if not hmac.compare_digest(presented_token(request).encode(), expected.encode()):
		raise UploadRejected(401, "missing or wrong upload token")


def lambda_name(request, file):
	name = request.form.get("name") or request.args.get("name") or (file.filename or "").rsplit(".", 1)[0]
	if not LAMBDA_NAME.match(name):
		raise UploadRejected(400, "lambda name must match " + LAMBDA_NAME.pattern)
	return name


def read_wasm(file):
	wasm = file.stream.read(MAX_WASM_BYTES + 1)
	if len(wasm) > MAX_WASM_BYTES:
		raise UploadRejected(413, f"wasm larger than {MAX_WASM_BYTES} bytes")
	if not wasm.startswith(WASM_MAGIC):
		raise UploadRejected(400, "not a wasm module")
	return wasm


def save_upload(request):
	"""Returns (name, path) of the stored module or raises UploadRejected."""
	check_token(request)
	file = request.files.get("file")
	if file is None:
		raise UploadRejected(400, "no file")
	name = lambda_name(request, file)
	wasm = read_wasm(file)
	path = os.path.join(LAMBDAS_DIR, name + ".wasm")
	with open(path, "wb") as fh:
		fh.write(wasm)
	return name, path
