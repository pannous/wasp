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
		if "favicon.ico" in path:
			return ""
		if request.method == 'GET':
				name = request.args.get('name') or path.strip('/')
		else:
				name = request.form.get('name') or path.strip('/')

		print("lambda name:", name)
		if not name or not name in binaries: return form_
		file_path = binaries[name]
		with open(file_path, "rb") as f:
				wasm_bytes = f.read()
				ok = waspy.run_wasm(wasm_bytes)
				return str(ok)

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
