#!gunicorn waspy_server:application
from flask import Flask, request, redirect, url_for
from flask_cors import CORS # pip install flask-cors
import waspy
app = Flask(__name__)
CORS(app)
binaries = {}

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

@app.route('/', defaults={'path': ''}, methods=['GET', 'POST'])
@app.route('/<path:path>', methods=['GET', 'POST'])
def index(path):
		if "favicon.ico" in path or "favicon.png" in path:
			return ""
		if "wp-includes" in path or "wp-content" in path or "wp-admin" in path:
			return ""
		if "wp-login.php" in path or "wp-config.php" in path or "xmlrpc.php" in path:
			return ""
		params = {}
		if request.method == 'GET':
				name = request.args.get('name') or path.strip('/')
				for arg in request.args:
					key = arg.split('=', 1)[0]
					value = arg.split('=', 1)[1]
					params[key] = value
		else:
				name = request.form.get('name') or path.strip('/')
				for key, value in request.form.items():
					params[key] = value

		print("lambda name:", name)
		if not name or not name in binaries: return form_
		file_path = binaries[name]
		with open(file_path, "rb") as f:
			try:
				wasm_bytes = f.read()
				ok = waspy.run_wasm(wasm_bytes, params)
				return str(ok)
			except Exception as e:
				return f"Error executing {name}: {str(e)}"

@app.route('/upload', methods=['POST'])
def upload():
		file = request.files['file']
		name = request.form.get('name') or file.filename.rsplit('.', 1)[0]
		filename = f"./lambdas/{file.filename}"
		file.save(filename)
		binaries[name] = filename
		return redirect(url_for('index')+name)

application = app
# gunicorn waspy-server:application

if __name__ == '__main__':
		app.run(debug=True, port=1234)
