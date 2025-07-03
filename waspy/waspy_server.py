#!gunicorn waspy_server:application
from flask import Flask, request, redirect, url_for
from flask_cors import CORS  # pip install flask-cors
import waspy

app = Flask(__name__)
CORS(app)
wasm_files = {}

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
	blocked = ["robots.txt", ".ico", ".png", ".jpg", ".jpeg", ".gif", "wp-", ".php", "setup/"]
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
	file_path = wasm_files[lib]
	with open(file_path, "rb") as f:
		try:
			wasm_bytes = f.read()
			ok = waspy.run_wasm(wasm_bytes, params, func=func)
			return str(ok)
		except Exception as e:
			return f"Error executing {lib}: {str(e)}"


@app.route('/upload', methods=['POST'])
def upload():
	file = request.files['file']
	name = request.form.get('name') or file.filename.rsplit('.', 1)[0]
	filename = f"./lambdas/{file.filename}"
	file.save(filename)
	wasm_files[name] = filename
	return redirect(url_for('index') + name)


application = app

if __name__ == '__main__':
	print("for LIVE environment, use `gunicorn waspy_server:application`")
	app.run(debug=True, port=8000)
