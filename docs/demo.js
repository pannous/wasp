// <!-- code specific to the demo site https://wasp.pannous.com/ -->

// Module for wasm_to_wat functionality
import * as binaryen from "./assets/script/binaryen-latest.js"

// Deploy function
function deploy() {
  let server;
  let name = lambda_name.value || "test";
  if (LIVE)
    server = "https://lambda.pannous.com/";
  else
    server = "http://localhost:9000/";

  // upload wasm_buffer to
  if (wasm_buffer) {
    const blob = new Blob([wasm_buffer], {type: 'application/wasm'});
    const formData = new FormData();
    formData.append('file', blob, 'compiled.wasm');
    formData.append('name', name);
    fetch(server + "upload?name=" + name, {
      method: 'POST',
      body: formData
    }).then(response => response.text())
      .then(data => {
        console.log('Success:', data);
        results.value = data;
      })
      .catch((error) => {
        console.error('Error:', error);
        results.value = error;
      });
  } else {
    alert("No wasm compiled yet!");
  }
  if (!auto_deploy.checked)
    window.open(server + name, '_blank');
}

// Window load handler and initialization
function initEditor() {
  try {
    const codeElement = document.getElementById('code');
    window.editor = CodeMirror.fromTextArea(codeElement, {
      lineNumbers: true,
      mode: "wasp",
      extraKeys: {
        "Ctrl-Space": "autocomplete", "Cmd-Enter": function () {
          compile_and_run(editor.getValue())
        }
      },
    });
    editor.on('change', function () {
      if (auto_run.checked)
        compile_and_run(editor.getValue());
      if (auto_deploy.checked) // todo and error free
        deploy();
      // editor.save();
      editor.focus();
    });
  } catch (error) {
    console.error('Error initializing CodeMirror:', error);
  }

  // fill from examples.js
  for (let key in exampleCode) {
    const optionExists = (val) => Array.from(examples.options).some(option => option.value === val);
    if (!optionExists(key)) {
      const newOption = document.createElement('option');
      newOption.value = key;
      newOption.text = key; // use exampleCode[key] on select!!
      examples.appendChild(newOption);
    }
  }
  // select example from URL parameter ^^
  // https://wasp.pannous.com/?example=lambda -
  const urlParams = new URLSearchParams(window.location.search);
  const exampleParam = urlParams.get('example');
  if (exampleParam && exampleCode[exampleParam]) {
    examples.value = exampleParam;
    editor.setValue(exampleCode[exampleParam]);
  }
}

// wasm_to_wat function
export function wasm_to_wat(buffer) {
  try {
    const module = binaryen.default.readBinary(buffer);
    const wat = module.emitText();
    results.value += wat;
    console.log("WAT output:\n", wat);
  } catch (e) {
    console.error(e);
    results.value = e;
  }
}

// Make functions available globally and set up event listener
window.deploy = deploy;
window.wasm_to_wat = wasm_to_wat;

// Use DOMContentLoaded instead of load, and add fallback
if (document.readyState === 'loading') {
  document.addEventListener('DOMContentLoaded', function() {
    initEditor();
    window.ctx = canvas.getContext('2d');
  });
} else {
  console.log('Initializing editor...');
  initEditor();
  window.ctx = canvas.getContext('2d');
}