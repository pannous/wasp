// NEEDS http-server to serve files! FF and chrome no longer support local development without server

const canvas = document.createElement('canvas');
canvas.width = 500
canvas.height = 500
document.body.appendChild(canvas)

const gl = canvas.getContext('webgl');
gl.clearColor(1, 1, 1, 1) // background white
gl.enable(gl.DEPTH_TEST) // occlusion


model = './f16-model.obj'
// model='./cube.obj'
texture = 'f16-texture.bmp'
// texture='tree-texture.jpg'
fetch(model).then(response => response.text().then(obj => {
    load(parseWavefrontObj(obj));// takes several seconds to parse in js! not in json!
}))

// fetch('./f16-model.json').then(response=>response.json().then(json=>{
//   load(json); // much faster in json! todo: binary wasm!!
// }))


let loaded3dModel;

function load(modelJSON) {
    console.log(modelJSON);
    const image = new window.Image();
    image.crossOrigin = 'anonymous'
    // Once our image downloads we buffer our 3d model data for the GPU
    image.onload = x => {
        console.log("Model texture loaded")
        loaded3dModel = LoadWavefrontObj(gl, modelJSON, {textureImage: image})
        window.requestAnimationFrame(draw);
    }
    image.src = texture
}

// // Our model's x-axis rotation in radians
let xRotation = 0;

function draw(dt) {
    gl.viewport(0, 0, canvas.width, canvas.height)
    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT)

    // Once we've loaded our model we draw it every frame
    if (loaded3dModel) {
        loaded3dModel.draw({
            position: [0, 0, -3.1],
            rotateX: xRotation,
            rotateY: xRotation
        })
    } else console.log("Model not loaded (yet)")
    xRotation += 1 / 100
    window.requestAnimationFrame(draw);
}


// Map .obj vertex info line names to our returned property names
const vertexInfoNameMap = {v: 'vertex', vt: 'uv', vn: 'normal'};
// The returned properties that we will populate
const parsedProperties = ['normal', 'uv', 'vertex', 'normalIndex', 'uvIndex', 'vertexIndex'];

function parseWavefrontObj(wavefrontString) {
    const parsedJSON = {normal: [], uv: [], vertex: [], normalIndex: [], uvIndex: [], vertexIndex: []};

    const linesInWavefrontObj = wavefrontString.split('\n');

    // Loop through and parse every line in our .obj file
    linesInWavefrontObj.forEach(function (currentLine) {
        // Tokenize our current line
        const currentLineTokens = currentLine.split(' ');
        // vertex position, vertex texture, or vertex normal
        const vertexInfoType = vertexInfoNameMap[currentLineTokens[0]];
        if (vertexInfoType) {
            parsedJSON[vertexInfoType] = parsedJSON[vertexInfoType].concat(currentLineTokens.slice(1))
            return
        }
        if (currentLineTokens[0] === 'f') {
            // Get our 4 sets of vertex, uv, and normal indices for this face
            for (let i = 1; i < 5; i++) {
                // If there is no fourth face entry then this is specifying a triangle
                // in this case we push `-1`
                // Consumers of this module should check for `-1` before expanding face data
                if (i === 4 && !currentLineTokens[4]) {
                    parsedJSON.vertexIndex.push(-1)
                    parsedJSON.uvIndex.push(-1)
                    parsedJSON.normalIndex.push(-1)
                } else {
                    const indices = currentLineTokens[i].split('/');
                    parsedJSON.vertexIndex.push(Number(indices[0]) - 1) // We zero index
                    parsedJSON.uvIndex.push(Number(indices[1]) - 1) // our face indices
                    parsedJSON.normalIndex.push(Number(indices[2]) - 1) // by subtracting 1
                }
            }
        }
    })
    // Convert our parsed strings into floats (trims trailing insignificant `0`s)
    parsedProperties.forEach(function (property) {
        parsedJSON[property] = parsedJSON[property].reduce(function (accumulator, nextValue) {
            return accumulator.concat(parseFloat(nextValue))
        }, [])
    })
    return parsedJSON
}


/**
 * Calculates a 3x3 normal matrix (transpose inverse) from the 4x4 matrix
 *
 * @alias mat3.normalFromMat4
 * @param {mat3} out mat3 receiving operation result
 * @param {mat4} a Mat4 to derive the normal matrix from
 *
 * @returns {mat3} out
 */
function normalFromMat4(out, a) {
    const a00 = a[0], a01 = a[1], a02 = a[2], a03 = a[3];
    const a10 = a[4], a11 = a[5], a12 = a[6], a13 = a[7];
    const a20 = a[8], a21 = a[9], a22 = a[10], a23 = a[11];
    const a30 = a[12], a31 = a[13], a32 = a[14], a33 = a[15];

    const b00 = a00 * a11 - a01 * a10;
    const b01 = a00 * a12 - a02 * a10;
    const b02 = a00 * a13 - a03 * a10;
    const b03 = a01 * a12 - a02 * a11;
    const b04 = a01 * a13 - a03 * a11;
    const b05 = a02 * a13 - a03 * a12;
    const b06 = a20 * a31 - a21 * a30;
    const b07 = a20 * a32 - a22 * a30;
    const b08 = a20 * a33 - a23 * a30;
    const b09 = a21 * a32 - a22 * a31;
    const b10 = a21 * a33 - a23 * a31;
    const b11 = a22 * a33 - a23 * a32;

    // Calculate the determinant
    let det = b00 * b11
        - b01 * b10
        + b02 * b09
        + b03 * b08
        - b04 * b07
        + b05 * b06;

    if (!det) return null
    det = 1.0 / det

    out[0] = (a11 * b11 - a12 * b10 + a13 * b09) * det
    out[1] = (a12 * b08 - a10 * b11 - a13 * b07) * det
    out[2] = (a10 * b10 - a11 * b08 + a13 * b06) * det

    out[3] = (a02 * b10 - a01 * b11 - a03 * b09) * det
    out[4] = (a00 * b11 - a02 * b08 + a03 * b07) * det
    out[5] = (a01 * b08 - a00 * b10 - a03 * b06) * det

    out[6] = (a31 * b05 - a32 * b04 + a33 * b03) * det
    out[7] = (a32 * b02 - a30 * b05 - a33 * b01) * det
    out[8] = (a30 * b04 - a31 * b02 + a33 * b00) * det

    return out
}

/**
 * Creates a new identity mat4
 *
 * @returns {mat4} a new 4x4 matrix
 */
function create() {
    const out = new Float32Array(16);
    out[0] = 1;
    out[1] = 0;
    out[2] = 0;
    out[3] = 0;
    out[4] = 0;
    out[5] = 1;
    out[6] = 0;
    out[7] = 0;
    out[8] = 0;
    out[9] = 0;
    out[10] = 1;
    out[11] = 0;
    out[12] = 0;
    out[13] = 0;
    out[14] = 0;
    out[15] = 1;
    return out;
}


/**
 * Multiplies two mat4's
 *
 * @param {mat4} out the receiving matrix
 * @param {mat4} a the first operand
 * @param {mat4} b the second operand
 * @returns {mat4} out
 */
function multiply(out, a, b) {
    const a00 = a[0], a01 = a[1], a02 = a[2], a03 = a[3],
        a10 = a[4], a11 = a[5], a12 = a[6], a13 = a[7],
        a20 = a[8], a21 = a[9], a22 = a[10], a23 = a[11],
        a30 = a[12], a31 = a[13], a32 = a[14], a33 = a[15];

    // Cache only the current line of the second matrix
    let b0 = b[0], b1 = b[1], b2 = b[2], b3 = b[3];
    out[0] = b0 * a00 + b1 * a10 + b2 * a20 + b3 * a30;
    out[1] = b0 * a01 + b1 * a11 + b2 * a21 + b3 * a31;
    out[2] = b0 * a02 + b1 * a12 + b2 * a22 + b3 * a32;
    out[3] = b0 * a03 + b1 * a13 + b2 * a23 + b3 * a33;

    b0 = b[4];
    b1 = b[5];
    b2 = b[6];
    b3 = b[7];
    out[4] = b0 * a00 + b1 * a10 + b2 * a20 + b3 * a30;
    out[5] = b0 * a01 + b1 * a11 + b2 * a21 + b3 * a31;
    out[6] = b0 * a02 + b1 * a12 + b2 * a22 + b3 * a32;
    out[7] = b0 * a03 + b1 * a13 + b2 * a23 + b3 * a33;

    b0 = b[8];
    b1 = b[9];
    b2 = b[10];
    b3 = b[11];
    out[8] = b0 * a00 + b1 * a10 + b2 * a20 + b3 * a30;
    out[9] = b0 * a01 + b1 * a11 + b2 * a21 + b3 * a31;
    out[10] = b0 * a02 + b1 * a12 + b2 * a22 + b3 * a32;
    out[11] = b0 * a03 + b1 * a13 + b2 * a23 + b3 * a33;

    b0 = b[12];
    b1 = b[13];
    b2 = b[14];
    b3 = b[15];
    out[12] = b0 * a00 + b1 * a10 + b2 * a20 + b3 * a30;
    out[13] = b0 * a01 + b1 * a11 + b2 * a21 + b3 * a31;
    out[14] = b0 * a02 + b1 * a12 + b2 * a22 + b3 * a32;
    out[15] = b0 * a03 + b1 * a13 + b2 * a23 + b3 * a33;
    return out;
}


/**
 * Generates a perspective projection matrix with the given bounds
 *
 * @param {mat4} out mat4 frustum matrix will be written into
 * @param {number} fovy Vertical field of view in radians
 * @param {number} aspect Aspect ratio. typically viewport width/height
 * @param {number} near Near bound of the frustum
 * @param {number} far Far bound of the frustum
 * @returns {mat4} out
 */
function perspective(out, fovy, aspect, near, far) {
    const f = 1.0 / Math.tan(fovy / 2),
        nf = 1 / (near - far);
    out[0] = f / aspect;
    out[1] = 0;
    out[2] = 0;
    out[3] = 0;
    out[4] = 0;
    out[5] = f;
    out[6] = 0;
    out[7] = 0;
    out[8] = 0;
    out[9] = 0;
    out[10] = (far + near) * nf;
    out[11] = -1;
    out[12] = 0;
    out[13] = 0;
    out[14] = (2 * far * near) * nf;
    out[15] = 0;
    return out;
}


/**
 * Rotates a matrix by the given angle around the X axis
 *
 * @param {mat4} out the receiving matrix
 * @param {mat4} a the matrix to rotate
 * @param {Number} rad the angle to rotate the matrix by
 * @returns {mat4} out
 */
function rotateX(out, a, rad) {
    const s = Math.sin(rad),
        c = Math.cos(rad),
        a10 = a[4],
        a11 = a[5],
        a12 = a[6],
        a13 = a[7],
        a20 = a[8],
        a21 = a[9],
        a22 = a[10],
        a23 = a[11];

    if (a !== out) { // If the source and destination differ, copy the unchanged rows
        out[0] = a[0];
        out[1] = a[1];
        out[2] = a[2];
        out[3] = a[3];
        out[12] = a[12];
        out[13] = a[13];
        out[14] = a[14];
        out[15] = a[15];
    }

    // Perform axis-specific matrix multiplication
    out[4] = a10 * c + a20 * s;
    out[5] = a11 * c + a21 * s;
    out[6] = a12 * c + a22 * s;
    out[7] = a13 * c + a23 * s;
    out[8] = a20 * c - a10 * s;
    out[9] = a21 * c - a11 * s;
    out[10] = a22 * c - a12 * s;
    out[11] = a23 * c - a13 * s;
    return out;
}


/**
 * Rotates a matrix by the given angle around the Y axis
 *
 * @param {mat4} out the receiving matrix
 * @param {mat4} a the matrix to rotate
 * @param {Number} rad the angle to rotate the matrix by
 * @returns {mat4} out
 */
function rotateY(out, a, rad) {
    const s = Math.sin(rad),
        c = Math.cos(rad),
        a00 = a[0],
        a01 = a[1],
        a02 = a[2],
        a03 = a[3],
        a20 = a[8],
        a21 = a[9],
        a22 = a[10],
        a23 = a[11];

    if (a !== out) { // If the source and destination differ, copy the unchanged rows
        out[4] = a[4];
        out[5] = a[5];
        out[6] = a[6];
        out[7] = a[7];
        out[12] = a[12];
        out[13] = a[13];
        out[14] = a[14];
        out[15] = a[15];
    }

    // Perform axis-specific matrix multiplication
    out[0] = a00 * c - a20 * s;
    out[1] = a01 * c - a21 * s;
    out[2] = a02 * c - a22 * s;
    out[3] = a03 * c - a23 * s;
    out[8] = a00 * s + a20 * c;
    out[9] = a01 * s + a21 * c;
    out[10] = a02 * s + a22 * c;
    out[11] = a03 * s + a23 * c;
    return out;
}


/**
 * Rotates a matrix by the given angle around the Z axis
 *
 * @param {mat4} out the receiving matrix
 * @param {mat4} a the matrix to rotate
 * @param {Number} rad the angle to rotate the matrix by
 * @returns {mat4} out
 */
function rotateZ(out, a, rad) {
    const s = Math.sin(rad),
        c = Math.cos(rad),
        a00 = a[0],
        a01 = a[1],
        a02 = a[2],
        a03 = a[3],
        a10 = a[4],
        a11 = a[5],
        a12 = a[6],
        a13 = a[7];

    if (a !== out) { // If the source and destination differ, copy the unchanged last row
        out[8] = a[8];
        out[9] = a[9];
        out[10] = a[10];
        out[11] = a[11];
        out[12] = a[12];
        out[13] = a[13];
        out[14] = a[14];
        out[15] = a[15];
    }

    // Perform axis-specific matrix multiplication
    out[0] = a00 * c + a10 * s;
    out[1] = a01 * c + a11 * s;
    out[2] = a02 * c + a12 * s;
    out[3] = a03 * c + a13 * s;
    out[4] = a10 * c - a00 * s;
    out[5] = a11 * c - a01 * s;
    out[6] = a12 * c - a02 * s;
    out[7] = a13 * c - a03 * s;
    return out;
}


/**
 * Translate a mat4 by the given vector
 *
 * @param {mat4} out the receiving matrix
 * @param {mat4} a the matrix to translate
 * @param {vec3} v vector to translate by
 * @returns {mat4} out
 */
function translate(out, a, v) {
    const x = v[0], y = v[1], z = v[2];
    let a00, a01, a02, a03,
        a10, a11, a12, a13,
        a20, a21, a22, a23;

    if (a === out) {
        out[12] = a[0] * x + a[4] * y + a[8] * z + a[12];
        out[13] = a[1] * x + a[5] * y + a[9] * z + a[13];
        out[14] = a[2] * x + a[6] * y + a[10] * z + a[14];
        out[15] = a[3] * x + a[7] * y + a[11] * z + a[15];
    } else {
        a00 = a[0];
        a01 = a[1];
        a02 = a[2];
        a03 = a[3];
        a10 = a[4];
        a11 = a[5];
        a12 = a[6];
        a13 = a[7];
        a20 = a[8];
        a21 = a[9];
        a22 = a[10];
        a23 = a[11];

        out[0] = a00;
        out[1] = a01;
        out[2] = a02;
        out[3] = a03;
        out[4] = a10;
        out[5] = a11;
        out[6] = a12;
        out[7] = a13;
        out[8] = a20;
        out[9] = a21;
        out[10] = a22;
        out[11] = a23;

        out[12] = a00 * x + a10 * y + a20 * z + a[12];
        out[13] = a01 * x + a11 * y + a21 * z + a[13];
        out[14] = a02 * x + a12 * y + a22 * z + a[14];
        out[15] = a03 * x + a13 * y + a23 * z + a[15];
    }

    return out;
}


// TODO: Pull into own repo. The collada loader will use this also
// TODO: The way that we're doing this is inefficient and so we'll want to write a
//        few tests when we pull this out into it's own repo
//        ex: We don't need to dedupe indices that are pointing to the same exact data
//
// Dedupe vertex indices and duplicate the associated position / uv / normal data
// Since we can't have multiple VBO indexes we need to expand out our indexed data
function dedupeVertexIndices(modelJSON) {
    const expandedVertexPositions = modelJSON.vertex;
    const expandedVertexUVs = [];
    const expandedVertexNormals = [];
    const expandedVertexPositionIndices = [];
    const encounteredIndices = {};
    let largestPositionIndex = 0;

    const decodedIndices = expandVertexIndices(modelJSON);

    decodedIndices.decodedPositionIndices.forEach(function (vertexIndex, counter) {
        largestPositionIndex = Math.max(largestPositionIndex, vertexIndex)
        // If this is our first time seeing the vertex index we add it
        // Later we'll add all of the duplicate indices into the end of the array
        if (!encounteredIndices[vertexIndex]) {
            expandedVertexPositionIndices[counter] = vertexIndex
            // Push the appropriate UV coordinates
            for (let i = 0; i < 3; i++) {
                if (i < 2) {
                    expandedVertexUVs[vertexIndex * 2 + i] = modelJSON.uv[decodedIndices.decodedUVIndices[counter] * 2 + i]
                }
                expandedVertexNormals[vertexIndex * 3 + i] = modelJSON.normal[decodedIndices.decodedNormalIndices[counter] * 3 + i]
            }
            encounteredIndices[vertexIndex] = true
        }
    })
    decodedIndices.decodedPositionIndices.forEach(function (vertexIndex, counter) {
        // Add all of the duplicate indices that we skipped over above
        if (encounteredIndices[vertexIndex]) {
            expandedVertexPositionIndices[counter] = ++largestPositionIndex
            for (let i = 0; i < 3; i++) {
                if (i < 2) {
                    expandedVertexUVs[largestPositionIndex * 2 + i] = modelJSON.uv[decodedIndices.decodedUVIndices[counter] * 2 + i]
                }
                expandedVertexPositions[largestPositionIndex * 3 + i] = modelJSON.vertex[vertexIndex * 3 + i]
                expandedVertexNormals[largestPositionIndex * 3 + i] = modelJSON.normal[decodedIndices.decodedNormalIndices[counter] * 3 + i]
            }
        }
    })

    return {
        positions: expandedVertexPositions,
        positionIndices: expandedVertexPositionIndices,
        normals: expandedVertexNormals,
        uvs: expandedVertexUVs
    }
}

// TODO: Pull out into own repo. The collada loader will use this also
//
// Expands .obj faces with two triangles into 6 indices instead of the decoded 4 indices
//  example: f 1/20/30 2/30/40 3/40/50 4/50/60
//           gets expanded into [1, 2, 3, 1, 2, 4]
//
//           but 1/20/30 2/30/40 3/40/50 would not get expanded
//           it becomes [1, 2, 3]
//

// function ExpandVertexPositionIndices (modelJSON) {
function expandVertexIndices(modelJSON) {

    const decodedVertexPositionIndices = [];
    const decodedVertexUVIndices = [];
    const decodedVertexNormalIndices = [];

    for (let i = 0; i < modelJSON.vertexIndex.length() / 4; i++) {
        decodedVertexPositionIndices.push(modelJSON.vertexIndex[i * 4])
        decodedVertexPositionIndices.push(modelJSON.vertexIndex[i * 4 + 1])
        decodedVertexPositionIndices.push(modelJSON.vertexIndex[i * 4 + 2])
        decodedVertexUVIndices.push(modelJSON.uvIndex[i * 4])
        decodedVertexUVIndices.push(modelJSON.uvIndex[i * 4 + 1])
        decodedVertexUVIndices.push(modelJSON.uvIndex[i * 4 + 2])
        decodedVertexNormalIndices.push(modelJSON.normalIndex[i * 4])
        decodedVertexNormalIndices.push(modelJSON.normalIndex[i * 4 + 1])
        decodedVertexNormalIndices.push(modelJSON.normalIndex[i * 4 + 2])
        // If this is a face with 4 vertices we push a second triangle
        if (decodedVertexPositionIndices[i * 4 + 3] !== -1) {
            decodedVertexPositionIndices.push(modelJSON.vertexIndex[i * 4])
            decodedVertexPositionIndices.push(modelJSON.vertexIndex[i * 4 + 2])
            decodedVertexPositionIndices.push(modelJSON.vertexIndex[i * 4 + 3])
            decodedVertexUVIndices.push(modelJSON.uvIndex[i * 4])
            decodedVertexUVIndices.push(modelJSON.uvIndex[i * 4 + 2])
            decodedVertexUVIndices.push(modelJSON.uvIndex[i * 4 + 3])
            decodedVertexNormalIndices.push(modelJSON.normalIndex[i * 4])
            decodedVertexNormalIndices.push(modelJSON.normalIndex[i * 4 + 2])
            decodedVertexNormalIndices.push(modelJSON.normalIndex[i * 4 + 3])
        }
    }
    return {
        decodedPositionIndices: decodedVertexPositionIndices,
        decodedUVIndices: decodedVertexUVIndices,
        decodedNormalIndices: decodedVertexNormalIndices
    }
}


const mat4Create = create;
const mat4Multiply = multiply;
const mat4RotateX = rotateX;
const mat4RotateY = rotateY;
const mat4RotateZ = rotateZ;
const mat4Translate = translate;
const mat4Perspective = perspective;
const mat3NormalFromMat4 = normalFromMat4;

function LoadWavefrontObj(gl, modelJSON, opts) {
    const expandedVertexData = dedupeVertexIndices(modelJSON);
    const vertexPositionBuffer = createBuffer(gl, 'ARRAY_BUFFER', Float32Array, expandedVertexData.positions);
    const vertexPositionIndexBuffer = createBuffer(gl, 'ELEMENT_ARRAY_BUFFER', Uint16Array, expandedVertexData.positionIndices);
    const vertexTextureBuffer = createBuffer(gl, 'ARRAY_BUFFER', Float32Array, expandedVertexData.uvs);
    const vertexNormalBuffer = createBuffer(gl, 'ARRAY_BUFFER', Float32Array, expandedVertexData.normals);

    const numIndices = expandedVertexData.positionIndices.length;

    const shaderObj = initShader(gl);
    const modelTexture = initTexture(gl, opts);

    const defaults = {
        ambient: [1.0, 1.0, 1.0],
        perspective: mat4Perspective([], Math.PI / 4, 256 / 256, 0.1, 100),
        position: [0.0, 0.0, -5.0],
        // Rotate the model in place
        rotateX: 0.0,
        rotateY: 0.0,
        rotateZ: 0.0,
        viewMatrix: mat4Create()
    };

    return {
        draw: draw.bind(null, gl)
    }

    // TODO: Pull out into own file
    // TODO: Add test for drawing 2 models with different number of vertex attributeNodeKind::List
    //        Make sure we enable and disable properly. Adding this in without tests for now..
    function draw(gl, opts) {
        opts = extend(defaults, opts)

        const modelMatrix = mat4Create();
        mat4Translate(modelMatrix, modelMatrix, opts.position)

        // We rotate the model in place. If you want to rotate it about an axis
        //  you can handle that by letting the consumer of this module manipulate
        //  the model's position occordingly
        mat4RotateX(modelMatrix, modelMatrix, opts.rotateX)
        mat4RotateY(modelMatrix, modelMatrix, opts.rotateY)
        mat4RotateZ(modelMatrix, modelMatrix, opts.rotateZ)

        mat4Multiply(modelMatrix, opts.viewMatrix, modelMatrix)

        // TODO: Should the consumer be in charge of `useProgram` and we just return the shaderProgram during model init?
        gl.useProgram(shaderObj.program)

        gl.bindBuffer(gl.ARRAY_BUFFER, vertexPositionBuffer)
        gl.enableVertexAttribArray(shaderObj.vertexPositionAttribute)
        gl.vertexAttribPointer(shaderObj.vertexPositionAttribute, 3, gl.FLOAT, false, 0, 0)

        // Textures
        gl.bindBuffer(gl.ARRAY_BUFFER, vertexTextureBuffer)
        gl.enableVertexAttribArray(shaderObj.textureCoordAttribute)
        gl.vertexAttribPointer(shaderObj.textureCoordAttribute, 2, gl.FLOAT, false, 0, 0)

        gl.activeTexture(gl.TEXTURE0)
        gl.bindTexture(gl.TEXTURE_2D, modelTexture)
        gl.uniform1i(shaderObj.samplerUniform, 0)

        // Normals
        gl.bindBuffer(gl.ARRAY_BUFFER, vertexNormalBuffer)
        gl.enableVertexAttribArray(shaderObj.vertexNormalAttribute)
        gl.vertexAttribPointer(shaderObj.vertexNormalAttribute, 3, gl.FLOAT, false, 0, 0)

        const normalMatrix = [];
        mat3NormalFromMat4(normalMatrix, modelMatrix)
        gl.uniformMatrix3fv(shaderObj.nMatrixUniform, false, normalMatrix)

        // Lighting
        gl.uniform3fv(shaderObj.ambientColorUniform, opts.ambient)

        // Drawing the model
        gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, vertexPositionIndexBuffer)

        gl.uniformMatrix4fv(shaderObj.pMatrixUniform, false, opts.perspective)
        gl.uniformMatrix4fv(shaderObj.mvMatrixUniform, false, modelMatrix)

        gl.drawElements(gl.TRIANGLES, numIndices, gl.UNSIGNED_SHORT, 0)

        // Clean up
        gl.disableVertexAttribArray(shaderObj.vertexPositionAttribute)
        gl.disableVertexAttribArray(shaderObj.textureCoordAttribute)
        gl.disableVertexAttribArray(shaderObj.vertexNormalAttribute)
    }
}

function createBuffer(gl, bufferType, DataType, data) {
    const buffer = gl.createBuffer();
    gl.bindBuffer(gl[bufferType], buffer)
    gl.bufferData(gl[bufferType], new DataType(data), gl.STATIC_DRAW)
    return buffer
}

function createFragmentShader(opts) {
    return `
    precision mediump float;

    varying vec2 vTextureCoord;
    varying vec3 vLightWeighting;

    uniform sampler2D uSampler;

    void main(void) {
      vec4 textureColor = texture2D(uSampler, vec2(vTextureCoord.s, vTextureCoord.t));
      gl_FragColor = vec4(textureColor.rgb * vLightWeighting, textureColor.a);
    }
  `
}

function createVertexShader(opts) {
    return `
    attribute vec3 aVertexPosition;

    attribute vec3 aVertexNormal;
    uniform mat3 uNMatrix;

    attribute vec2 aTextureCoord;
    varying vec2 vTextureCoord;

    uniform vec3 uAmbientColor;
    varying vec3 vLightWeighting;

    uniform mat4 uMVMatrix;
    uniform mat4 uPMatrix;

    void main (void) {
      gl_Position = uPMatrix * uMVMatrix * vec4(aVertexPosition, 1.0);
      vTextureCoord = aTextureCoord;

      vec3 transformedNormal = uNMatrix * aVertexNormal;
      vLightWeighting = uAmbientColor;
    }
  `
}


// TODO: Pull out into separate, tested shader generator repository
function initShader(gl, opts) {
    const fragmentShader = gl.createShader(gl.FRAGMENT_SHADER);
    gl.shaderSource(fragmentShader, createFragmentShader(opts))
    gl.compileShader(fragmentShader)

    const vertexShader = gl.createShader(gl.VERTEX_SHADER);
    gl.shaderSource(vertexShader, createVertexShader(opts))
    gl.compileShader(vertexShader)

    const shaderProgram = gl.createProgram();
    gl.attachShader(shaderProgram, fragmentShader)
    gl.attachShader(shaderProgram, vertexShader)
    gl.linkProgram(shaderProgram)

    // Return our shader data object
    return {
        ambientColorUniform: gl.getUniformLocation(shaderProgram, 'uAmbientColor'),
        mvMatrixUniform: gl.getUniformLocation(shaderProgram, 'uMVMatrix'),
        nMatrixUniform: gl.getUniformLocation(shaderProgram, 'uNMatrix'),
        pMatrixUniform: gl.getUniformLocation(shaderProgram, 'uPMatrix'),
        program: shaderProgram,
        samplerUniform: gl.getUniformLocation(shaderProgram, 'uSampler'),
        textureCoordAttribute: gl.getAttribLocation(shaderProgram, 'aTextureCoord'),
        vertexPositionAttribute: gl.getAttribLocation(shaderProgram, 'aVertexPosition'),
        vertexNormalAttribute: gl.getAttribLocation(shaderProgram, 'aVertexNormal')
    }
}

function initTexture(gl, opts) {
    const modelTexture = gl.createTexture();
    handleLoadedTexture(gl, modelTexture, opts.textureImage)

    return modelTexture

    function handleLoadedTexture(gl, modelTexture, textureImage) {
        gl.bindTexture(gl.TEXTURE_2D, modelTexture)
        // If we're passing in a Uint8Array of image data
        if (textureImage.length) {
            gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE)
            gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE)
            gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, 256, 256, 0, gl.RGBA, gl.UNSIGNED_BYTE, textureImage)
        } else {
            // If we're passing in an HTML image element
            gl.pixelStorei(gl.UNPACK_FLIP_Y_WEBGL, true)
            gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, textureImage)
        }

        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST)
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST)
        gl.bindTexture(gl.TEXTURE_2D, null)
    }
}

const hasOwnProperty = Object.prototype.hasOwnProperty;

function extend() {
    const target = {};
    for (let i = 0; i < arguments.length; i++) {
        const source = arguments[i];
        for (let key in source)
            if (hasOwnProperty.call(source, key))
                target[key] = source[key]
    }
    return target
}

