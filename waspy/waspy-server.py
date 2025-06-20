from flask import Flask, request, redirect, url_for
import waspy

app = Flask(__name__)
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
        if not ok:
            return "Error executing the lambda function."
        return ok

@app.route('/upload', methods=['POST'])
def upload():
    file = request.files['file']
    name = request.form.get('name') or file.filename.rsplit('.', 1)[0]
    filename = f"./lambdas/{file.filename}"
    file.save(filename)
    binaries[name] = filename
    return redirect(url_for('index')+name)

if __name__ == '__main__':
    app.run(debug=True, port=1234)