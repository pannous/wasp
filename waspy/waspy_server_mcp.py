#!gunicorn waspy_server:application
from flask import Flask, request, redirect, url_for
from flask_cors import CORS  # pip install flask-cors
from mcp.server.fastmcp import FastMCP  # pip install "mcp[cli]"
import threading, queue
import waspy

register_queue = queue.Queue()
mcp = FastMCP("waspy", host="0.0.0.0", port=3001, debug=True)  # FastMCP instance for managing tools

app = Flask(__name__)
CORS(app)

wasm_files = {}  # name -> filename
binaries = {}  # name -> wasm bytes

form_ = '''
<form method="post" action="/">
	Lambda name: <input type="text" name="name">
	<input type="submit" value="Execute">
</form>
<form method="post" enctype="multipart/form-data" action="/upload">
	Upload WASM: <input type="file" name="file">
	Lambda name: <input type="text" name="name">
	<input type="submit" value="Upload">
</form>
'''


@app.route('/', defaults={'lib': None, 'func': '_start', 'args': ''})
@app.route('/<lib>/', defaults={'func': '_start', 'args': ''})
@app.route('/<lib>/<func>/', defaults={'args': ''})
@app.route('/<lib>/<func>/<args>')
def index(lib, func, args):
	path = request.path
	if not path or not lib:
		return form_
	blocked = ["robots.txt", ".ico", ".png", ".jpg", ".jpeg", ".gif", "wp-", ".php", "setup/", "sitemap.xml"]
	for b in blocked:
		if b in path: return ""
	params = {}
	args = args or request.query_string.decode()
	for i, val in enumerate(args.split(',')):
		if '=' in val:
			k, v = val.split('=', 1)
			params[k] = v
		else:
			params[f"${i}"] = val
	for key in request.values:
		params[key] = request.values[key]
	print("lambda name:", lib)
	print("function name:", func)
	print("params:", params)
	if not lib or not lib in wasm_files: return form_
	wasm_bytes = None
	if lib in binaries:
		wasm_bytes = binaries[lib]
	else:
		file_path = wasm_files[lib]
		with open(file_path, "rb") as f:
			wasm_bytes = f.read()
	try:
		ok = waspy.run_wasm(wasm_bytes, params, func=func)
		return str(ok)
	except Exception as e:
		return f"Error executing {lib}: {str(e)}"


def fix_params(params, binary=None, func=""):
	if not params: return {}
	if not isinstance(params, dict):
		return {"value2": params}
	return params


@app.route('/upload', methods=['POST'])
def upload():
	file = request.files['file']
	name = request.form.get('name') or file.filename.rsplit('.', 1)[0]
	filename = f"./lambdas/{file.filename}"
	file.save(filename)
	wasm_files[name] = filename
	register_exports_as_mcp(name, filename)
	return redirect(url_for('index') + name)


def register_exports_as_mcp(lib, filename):
	with open(filename, "rb") as fbin:
		wasm = fbin.read()
		binaries[lib] = wasm
		module, store = waspy.instantiate(wasm)
		for func in module.exports:
			if "Func" in str(type(func.type)):  # skip isinstance(export, wasmtime._types.MemoryType)
				register_mcp(lib, func.name)


known_mcps = []
def register_mcp(lib, func):
	print("registering mcp", lib, func)

	@mcp.tool(f"{lib}.{func}")
	def inner(arguments={}):
		# argument_dict= fix_params(arguments, binaries[lib], func)
		return waspy.run_wasm(binaries[lib], arguments, func=func)

	description = f"Tool {lib}-{func} registered without description, must be inferred from name and function signature."
	register_queue.put((inner, f"{lib}-{func}", description))
	known_mcps.append(f"{lib}-{func}")


def register_test():
	@mcp.tool("test-hallo", description="send greetings to a user with provided name")
	def hallo(name="World"):
		return f"Hallo {name}!"

	register_queue.put((lambda: "pong", f"ping", "Respond with 'pong' to a ping request."))
	register_queue.put((lambda echo: echo, f"echo", "Echo the input back to the user."))
	register_queue.put((hallo, f"hallo", "Send greetings to a user with provided name."))


def mcp_server():
	def poll_registers():  # needs to be run in a separate thread
		while True:
			func, name, description = register_queue.get()
			print(f"Registering tool: {name} with description: {description}")
			# mcp.add_tool(mcp.tool(name,description=description)(func), name, description=description)
			mcp.add_tool(func, name, description=description)
			known_mcps.append(name)

	threading.Thread(target=poll_registers, daemon=True).start()
	register_test()  # OK
	# mcp.run(transport="sse") can only use one
	mcp.run(transport="streamable-http")


# 		https://mcp.pannous.com/mcp/ or local http://localhost:3001/mcp/


# Start thread (non-daemon so it stays alive unless cleanly shut down)
thread = threading.Thread(target=mcp_server)
thread.start()

application = app  # !gunicorn waspy_server:application
if __name__ == '__main__':
	print("for LIVE environment, use `gunicorn waspy_server:application`")
	# threading.Thread(target=lambda: mcp_server(), daemon=True).start()
	# app.run(debug=True, port=9000)
	app.run(debug=True, port=9000, use_reloader=False)
