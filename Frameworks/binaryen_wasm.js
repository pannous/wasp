// FIXME: The Emscripten shell requires this variable to be present, even though
// we are building to ES6 (where it doesn't exist) and the .wasm blob is inlined
// see: https://github.com/emscripten-core/emscripten/issues/11792
var __dirname = "";

// FIXME: The Emscripten shell requires this function to be present, even though
// we are building to ESM (where it doesn't exist) and the result is not used
// see: https://github.com/emscripten-core/emscripten/pull/17851
function require() {
  return {}
  ;
}


var Binaryen = (() => {
  var _scriptDir = import.meta.url;
  
  return (
function(Binaryen) {
  Binaryen = Binaryen || {}
  ;


var aa="function"==typeof Object.defineProperties?Object.defineProperty:function(b,d,f){if(b==Array.prototype||b==Object.prototype)return b;b[d]=f.value;return b}
;function ba(b){b=["object"==typeof globalThis&&globalThis,b,"object"==typeof window&&window,"object"==typeof self&&self,"object"==typeof global&&global];for(var d=0;d<b.length;++d){var f=b[d];if(f&&f.Math==Math)return f}
throw Error("Cannot find global object");}
var ca=ba(this);
function da(b,d){if(d)a:{var f=ca;b=b.split(".");for(var c=0;c<b.length-1;c++){var e=b[c];if(!(e in f))break a;f=f[e]}
b=b[b.length-1];c=f[b];d=d(c);d!=c&&null!=d&&aa(f,b,{configurable:!0,writable:!0,value:d}
                                                )}
}
da("Array.prototype.includes",function(b){return b?b:function(d,f){var c=this;c instanceof String&&(c=String(c));var e=c.length;f=f||0;for(0>f&&(f=Math.max(f+e,0));f<e;f++){var g=c[f];if(g===d||Object.is(g,d))return!0}
   return!1}
}
);var a;a||(a=typeof Binaryen !== 'undefined' ? Binaryen : {}
);
var ea,fa;a.ready=new Promise(function(b,d){ea=b;fa=d}
                              );var ha=Object.assign({}
,a),ia="./this.program",ja=(b,d)=>{throw d;}
,ka="object"==typeof window,la="function"==typeof importScripts,ma="object"==typeof process&&"object"==typeof process.versions&&"string"==typeof process.versions.node,k="",na,oa,pa;
if(ma){k=la?require("path").dirname(k)+"/":__dirname+"/";var fs,qa;"function"===typeof require&&(fs=require("fs"),qa=require("path"));na=(b,d)=>{b=qa.normalize(b);return fs.readFileSync(b,d?void 0:"utf8")}
;pa=b=>{b=na(b,!0);b.buffer||(b=new Uint8Array(b));return b}
;oa=(b,d,f)=>{b=qa.normalize(b);fs.readFile(b,function(c,e){c?f(c):d(e.buffer)}
                                            )}
;1<process.argv.length&&(ia=process.argv[1].replace(/\\/g,"/"));process.argv.slice(2);process.on("uncaughtException",function(b){if(!(b instanceof ra))throw b;
}
);process.on("unhandledRejection",function(b){throw b;}
);ja=(b,d)=>{if(noExitRuntime)throw process.exitCode=b,d;d instanceof ra||p("exiting due to exception: "+d);process.exit(b)}
;a.inspect=function(){return"[Emscripten Module object]"}
}
else if(ka||la)la?k=self.location.href:"undefined"!=typeof document&&document.currentScript&&(k=document.currentScript.src),_scriptDir&&(k=_scriptDir),0!==k.indexOf("blob:")?k=k.substr(0,k.replace(/[?#].*/,"").lastIndexOf("/")+1):k="",na=b=>{var d=new XMLHttpRequest;
d.open("GET",b,!1);d.send(null);return d.responseText}
,la&&(pa=b=>{var d=new XMLHttpRequest;d.open("GET",b,!1);d.responseType="arraybuffer";d.send(null);return new Uint8Array(d.response)}
      ),oa=(b,d,f)=>{var c=new XMLHttpRequest;c.open("GET",b,!0);c.responseType="arraybuffer";c.onload=()=>{200==c.status||0==c.status&&c.response?d(c.response):f()}
;c.onerror=f;c.send(null)}
;var q=a.print||console.log.bind(console),p=a.printErr||console.warn.bind(console);Object.assign(a,ha);ha=null;
a.thisProgram&&(ia=a.thisProgram);a.quit&&(ja=a.quit);var sa;a.wasmBinary&&(sa=a.wasmBinary);var noExitRuntime=a.noExitRuntime||!0;"object"!=typeof WebAssembly&&r("no native wasm support detected");var ta,ua=!1,va="undefined"!=typeof TextDecoder?new TextDecoder("utf8"):void 0;
function wa(b,d){for(var f=d+NaN,c=d;b[c]&&!(c>=f);)++c;if(16<c-d&&b.buffer&&va)return va.decode(b.subarray(d,c));for(f="";d<c;){var e=b[d++];if(e&128){var g=b[d++]&63;if(192==(e&224))f+=String.fromCharCode((e&31)<<6|g);else{var h=b[d++]&63;e=224==(e&240)?(e&15)<<12|g<<6|h:(e&7)<<18|g<<12|h<<6|b[d++]&63;65536>e?f+=String.fromCharCode(e):(e-=65536,f+=String.fromCharCode(55296|e>>10,56320|e&1023))}
}
else f+=String.fromCharCode(e)}
  return f}
function t(b){return b?wa(u,b):""}

function xa(b,d,f,c){if(!(0<c))return 0;var e=f;c=f+c-1;for(var g=0;g<b.length;++g){var h=b.charCodeAt(g);if(55296<=h&&57343>=h){var l=b.charCodeAt(++g);h=65536+((h&1023)<<10)|l&1023}
if(127>=h){if(f>=c)break;d[f++]=h}
else{if(2047>=h){if(f+1>=c)break;d[f++]=192|h>>6}
else{if(65535>=h){if(f+2>=c)break;d[f++]=224|h>>12}
else{if(f+3>=c)break;d[f++]=240|h>>18;d[f++]=128|h>>12&63}
d[f++]=128|h>>6&63}
d[f++]=128|h&63}
}
d[f]=0;return f-e}

function ya(b){for(var d=0,f=0;f<b.length;++f){var c=b.charCodeAt(f);127>=c?d++:2047>=c?d+=2:55296<=c&&57343>=c?(d+=4,++f):d+=3}
return d}
var za,v,u,Aa,x,z;function Ba(){var b=ta.buffer;za=b;a.HEAP8=v=new Int8Array(b);a.HEAP16=Aa=new Int16Array(b);a.HEAP32=x=new Int32Array(b);a.HEAPU8=u=new Uint8Array(b);a.HEAPU16=new Uint16Array(b);a.HEAPU32=z=new Uint32Array(b);a.HEAPF32=new Float32Array(b);a.HEAPF64=new Float64Array(b)}
var Ca,Da=[],Ea=[],Fa=[],Ga=!1;
function Ha(){var b=a.preRun.shift();Da.unshift(b)}
var D=0,Ia=null,Ja=null;function r(b){if(a.onAbort)a.onAbort(b);b="Aborted("+b+")";p(b);ua=!0;b=new WebAssembly.RuntimeError(b+". Build with -sASSERTIONS for more info.");fa(b);throw b;}
function Ka(){return E.startsWith("data:application/octet-stream;base64,")}
var E;if(a.locateFile){if(E="binaryen_wasm.wasm",!Ka()){var La=E;E=a.locateFile?a.locateFile(La,k):k+La}
}
else E=(new URL("binaryen_wasm.wasm",import.meta.url)).toString();
function Ma(){var b=E;try{if(b==E&&sa)return new Uint8Array(sa);if(pa)return pa(b);throw"both async and sync fetching of the wasm failed";}
catch(d){r(d)}
}

function Na(){if(!sa&&(ka||la)){if("function"==typeof fetch&&!E.startsWith("file://"))return fetch(E,{credentials:"same-origin"}
                                                                                                   ).then(function(b){if(!b.ok)throw"failed to load wasm binary file at '"+E+"'";return b.arrayBuffer()}
).catch(function(){return Ma()}
);if(oa)return new Promise(function(b,d){oa(E,function(f){b(new Uint8Array(f))}
                                            ,d)}
)}
return Promise.resolve().then(function(){return Ma()}
                              )}
var Oa,Pa;
function ra(b){this.name="ExitStatus";this.message="Program terminated with exit("+b+")";this.status=b}
function Qa(b){for(;0<b.length;)b.shift()(a)}
var Ra=[],Sa=0,G=0;
function Ta(b){this.qB=b;this.dB=b-24;this.WB=function(d){z[this.dB+4>>2]=d}
;this.iB=function(){return z[this.dB+4>>2]}
;this.TB=function(d){z[this.dB+8>>2]=d}
;this.SB=function(){return z[this.dB+8>>2]}
;this.UB=function(){x[this.dB>>2]=0}
;this.zB=function(d){v[this.dB+12>>0]=d?1:0}
;this.PB=function(){return 0!=v[this.dB+12>>0]}
;this.AB=function(d){v[this.dB+13>>0]=d?1:0}
;this.IB=function(){return 0!=v[this.dB+13>>0]}
;this.RB=function(d,f){this.rB(0);this.WB(d);this.TB(f);this.UB();this.zB(!1);this.AB(!1)}
;
this.NB=function(){x[this.dB>>2]+=1}
;this.ZB=function(){var d=x[this.dB>>2];x[this.dB>>2]=d-1;return 1===d}
;this.rB=function(d){z[this.dB+16>>2]=d}
;this.OB=function(){return z[this.dB+16>>2]}
;this.QB=function(){if(Ua(this.iB()))return z[this.qB>>2];var d=this.OB();return 0!==d?d:this.qB}
}
function Va(b){return H((new Ta(b)).dB)}
var Wa=[];function I(b){var d=Wa[b];d||(b>=Wa.length&&(Wa.length=b+1),Wa[b]=d=Ca.get(b));return d}

var Xa=(b,d)=>{for(var f=0,c=b.length-1;0<=c;c--){var e=b[c];"."===e?b.splice(c,1):".."===e?(b.splice(c,1),f++):f&&(b.splice(c,1),f--)}
if(d)for(;f;f--)b.unshift("..");return b}
,Ya=b=>{var d="/"===b.charAt(0),f="/"===b.substr(-1);(b=Xa(b.split("/").filter(c=>!!c),!d).join("/"))||d||(b=".");b&&f&&(b+="/");return(d?"/":"")+b}
,Za=b=>{var d=/^(\/?|)([\s\S]*?)((?:\.{1,2}
|[^\/]+?|)(\.[^.\/]*|))(?:[\/]*)$/.exec(b).slice(1);b=d[0];d=d[1];if(!b&&!d)return".";d&&(d=d.substr(0,d.length-1));return b+d}
,$a=b=>
{if("/"===b)return"/";b=Ya(b);b=b.replace(/\/$/,"");var d=b.lastIndexOf("/");return-1===d?b:b.substr(d+1)}
;function ab(){if("object"==typeof crypto&&"function"==typeof crypto.getRandomValues){var b=new Uint8Array(1);return()=>{crypto.getRandomValues(b);return b[0]}
}
if(ma)try{var d=require("crypto");return()=>d.randomBytes(1)[0]}
catch(f){}
return()=>r("randomDevice")}

function bb(){for(var b="",d=!1,f=arguments.length-1;-1<=f&&!d;f--){d=0<=f?arguments[f]:"/";if("string"!=typeof d)throw new TypeError("Arguments to path.resolve must be strings");if(!d)return"";b=d+"/"+b;d="/"===d.charAt(0)}
b=Xa(b.split("/").filter(c=>!!c),!d).join("/");return(d?"/":"")+b||"."}
function cb(b,d){var f=Array(ya(b)+1);b=xa(b,f,0,f.length);d&&(f.length=b);return f}
var db=[];function eb(b,d){db[b]={input:[],output:[],pB:d}
;fb(b,gb)}

var gb={open:function(b){var d=db[b.node.rdev];if(!d)throw new K(43);b.tty=d;b.seekable=!1}
,close:function(b){b.tty.pB.fsync(b.tty)}
,fsync:function(b){b.tty.pB.fsync(b.tty)}
,read:function(b,d,f,c){if(!b.tty||!b.tty.pB.HB)throw new K(60);for(var e=0,g=0;g<c;g++){try{var h=b.tty.pB.HB(b.tty)}
catch(l){throw new K(29);}
if(void 0===h&&0===e)throw new K(6);if(null===h||void 0===h)break;e++;d[f+g]=h}
e&&(b.node.timestamp=Date.now());return e}
,write:function(b,d,f,c){if(!b.tty||!b.tty.pB.xB)throw new K(60);
try{for(var e=0;e<c;e++)b.tty.pB.xB(b.tty,d[f+e])}
catch(g){throw new K(29);}
c&&(b.node.timestamp=Date.now());return e}
}
,hb={HB:function(b){if(!b.input.length){var d=null;if(ma){var f=Buffer.alloc(256),c=0;try{c=fs.readSync(process.stdin.fd,f,0,256,-1)}
catch(e){if(e.toString().includes("EOF"))c=0;else throw e;}
0<c?d=f.slice(0,c).toString("utf-8"):d=null}
else"undefined"!=typeof window&&"function"==typeof window.prompt?(d=window.prompt("Input: "),null!==d&&(d+="\n")):"function"==typeof readline&&(d=
readline(),null!==d&&(d+="\n"));if(!d)return null;b.input=cb(d,!0)}
  return b.input.shift()}
,xB:function(b,d){null===d||10===d?(q(wa(b.output,0)),b.output=[]):0!=d&&b.output.push(d)}
,fsync:function(b){b.output&&0<b.output.length&&(q(wa(b.output,0)),b.output=[])}
}
,ib={xB:function(b,d){null===d||10===d?(p(wa(b.output,0)),b.output=[]):0!=d&&b.output.push(d)}
,fsync:function(b){b.output&&0<b.output.length&&(p(wa(b.output,0)),b.output=[])}
}
,L={fB:null,jB:function(){return L.createNode(null,"/",16895,0)}
,
createNode:function(b,d,f,c){if(24576===(f&61440)||4096===(f&61440))throw new K(63);L.fB||(L.fB={dir:{node:{kB:L.bB.kB,gB:L.bB.gB,lookup:L.bB.lookup,sB:L.bB.sB,rename:L.bB.rename,unlink:L.bB.unlink,rmdir:L.bB.rmdir,readdir:L.bB.readdir,symlink:L.bB.symlink}
                                                                                           ,stream:{mB:L.cB.mB}
}
,file:{node:{kB:L.bB.kB,gB:L.bB.gB}
,stream:{mB:L.cB.mB,read:L.cB.read,write:L.cB.write,CB:L.cB.CB,JB:L.cB.JB,LB:L.cB.LB}
}
,link:{node:{kB:L.bB.kB,gB:L.bB.gB,readlink:L.bB.readlink}
,stream:{}
}
,DB:{node:{kB:L.bB.kB,gB:L.bB.gB}
,
stream:jb}
}
);f=kb(b,d,f,c);16384===(f.mode&61440)?(f.bB=L.fB.dir.node,f.cB=L.fB.dir.stream,f.aB={}
):32768===(f.mode&61440)?(f.bB=L.fB.file.node,f.cB=L.fB.file.stream,f.eB=0,f.aB=null):40960===(f.mode&61440)?(f.bB=L.fB.link.node,f.cB=L.fB.link.stream):8192===(f.mode&61440)&&(f.bB=L.fB.DB.node,f.cB=L.fB.DB.stream);f.timestamp=Date.now();b&&(b.aB[d]=f,b.timestamp=f.timestamp);return f}
,iC:function(b){return b.aB?b.aB.subarray?b.aB.subarray(0,b.eB):new Uint8Array(b.aB):new Uint8Array(0)}
,EB:function(b,
d){var f=b.aB?b.aB.length:0;f>=d||(d=Math.max(d,f*(1048576>f?2:1.125)>>>0),0!=f&&(d=Math.max(d,256)),f=b.aB,b.aB=new Uint8Array(d),0<b.eB&&b.aB.set(f.subarray(0,b.eB),0))}
,$B:function(b,d){if(b.eB!=d)if(0==d)b.aB=null,b.eB=0;else{var f=b.aB;b.aB=new Uint8Array(d);f&&b.aB.set(f.subarray(0,Math.min(d,b.eB)));b.eB=d}
}
,bB:{kB:function(b){var d={}
;d.dev=8192===(b.mode&61440)?b.id:1;d.ino=b.id;d.mode=b.mode;d.nlink=1;d.uid=0;d.gid=0;d.rdev=b.rdev;16384===(b.mode&61440)?d.size=4096:32768===(b.mode&61440)?
d.size=b.eB:40960===(b.mode&61440)?d.size=b.link.length:d.size=0;d.atime=new Date(b.timestamp);d.mtime=new Date(b.timestamp);d.ctime=new Date(b.timestamp);d.MB=4096;d.blocks=Math.ceil(d.size/d.MB);return d}
,gB:function(b,d){void 0!==d.mode&&(b.mode=d.mode);void 0!==d.timestamp&&(b.timestamp=d.timestamp);void 0!==d.size&&L.$B(b,d.size)}
,lookup:function(){throw lb[44];}
,sB:function(b,d,f,c){return L.createNode(b,d,f,c)}
,rename:function(b,d,f){if(16384===(b.mode&61440)){try{var c=mb(d,f)}
catch(g){}
if(c)for(var e in c.aB)throw new K(55);
}
delete b.parent.aB[b.name];b.parent.timestamp=Date.now();b.name=f;d.aB[f]=b;d.timestamp=b.parent.timestamp;b.parent=d}
,unlink:function(b,d){delete b.aB[d];b.timestamp=Date.now()}
,rmdir:function(b,d){var f=mb(b,d),c;for(c in f.aB)throw new K(55);delete b.aB[d];b.timestamp=Date.now()}
,readdir:function(b){var d=[".",".."],f;for(f in b.aB)b.aB.hasOwnProperty(f)&&d.push(f);return d}
,symlink:function(b,d,f){b=L.createNode(b,d,41471,0);b.link=f;return b}
,readlink:function(b){if(40960!==(b.mode&61440))throw new K(28);
return b.link}
}
,cB:{read:function(b,d,f,c,e){var g=b.node.aB;if(e>=b.node.eB)return 0;b=Math.min(b.node.eB-e,c);if(8<b&&g.subarray)d.set(g.subarray(e,e+b),f);else for(c=0;c<b;c++)d[f+c]=g[e+c];return b}
,write:function(b,d,f,c,e,g){d.buffer===v.buffer&&(g=!1);if(!c)return 0;b=b.node;b.timestamp=Date.now();if(d.subarray&&(!b.aB||b.aB.subarray)){if(g)return b.aB=d.subarray(f,f+c),b.eB=c;if(0===b.eB&&0===e)return b.aB=d.slice(f,f+c),b.eB=c;if(e+c<=b.eB)return b.aB.set(d.subarray(f,f+c),e),c}
L.EB(b,e+
c);if(b.aB.subarray&&d.subarray)b.aB.set(d.subarray(f,f+c),e);else for(g=0;g<c;g++)b.aB[e+g]=d[f+g];b.eB=Math.max(b.eB,e+c);return c}
,mB:function(b,d,f){1===f?d+=b.position:2===f&&32768===(b.node.mode&61440)&&(d+=b.node.eB);if(0>d)throw new K(28);return d}
,CB:function(b,d,f){L.EB(b.node,d+f);b.node.eB=Math.max(b.node.eB,d+f)}
,JB:function(b,d,f,c,e){if(32768!==(b.node.mode&61440))throw new K(43);b=b.node.aB;if(e&2||b.buffer!==za){if(0<f||f+d<b.length)b.subarray?b=b.subarray(f,f+d):b=Array.prototype.slice.call(b,
f,f+d);f=!0;r();d=void 0;if(!d)throw new K(48);v.set(b,d)}
else f=!1,d=b.byteOffset;return{dB:d,gC:f}
}
,LB:function(b,d,f,c){L.cB.write(b,d,0,c,f,!1);return 0}
}
}
,nb=null,ob={}
,pb=[],qb=1,rb=null,sb=!0,K=null,lb={}
,ub=(b,d={}
     )=>{b=bb("/",b);if(!b)return{path:"",node:null}
;d=Object.assign({GB:!0,yB:0}
                 ,d);if(8<d.yB)throw new K(32);b=Xa(b.split("/").filter(h=>!!h),!1);for(var f=nb,c="/",e=0;e<b.length;e++){var g=e===b.length-1;if(g&&d.parent)break;f=mb(f,b[e]);c=Ya(c+"/"+b[e]);f.tB&&(!g||g&&d.GB)&&(f=f.tB.root);
if(!g||d.FB)for(g=0;40960===(f.mode&61440);)if(f=tb(c),c=bb(Za(c),f),f=ub(c,{yB:d.yB+1}
                                                                          ).node,40<g++)throw new K(32);}
return{path:c,node:f}
}
,vb=b=>{for(var d;;){if(b===b.parent)return b=b.jB.KB,d?"/"!==b[b.length-1]?b+"/"+d:b+d:b;d=d?b.name+"/"+d:b.name;b=b.parent}
}
,wb=(b,d)=>{for(var f=0,c=0;c<d.length;c++)f=(f<<5)-f+d.charCodeAt(c)|0;return(b+f>>>0)%rb.length}
,mb=(b,d)=>{var f;if(f=(f=xb(b,"x"))?f:b.bB.lookup?0:2)throw new K(f,b);for(f=rb[wb(b.id,d)];f;f=f.YB){var c=f.name;if(f.parent.id===b.id&&
c===d)return f}
return b.bB.lookup(b,d)}
,kb=(b,d,f,c)=>{b=new yb(b,d,f,c);d=wb(b.parent.id,b.name);b.YB=rb[d];return rb[d]=b}
,zb={r:0,"r+":2,w:577,"w+":578,a:1089,"a+":1090}
,Ab=b=>{var d=["r","w","rw"][b&3];b&512&&(d+="w");return d}
,xb=(b,d)=>{if(sb)return 0;if(!d.includes("r")||b.mode&292){if(d.includes("w")&&!(b.mode&146)||d.includes("x")&&!(b.mode&73))return 2}
else return 2;return 0}
,Bb=(b,d)=>{try{return mb(b,d),20}
catch(f){}
return xb(b,"wx")}
,Cb=(b=0)=>{for(;4096>=b;b++)if(!pb[b])return b;throw new K(33);
}
,Eb=(b,d)=>{Db||(Db=function(){this.iB={}
}
,Db.prototype={}
,Object.defineProperties(Db.prototype,{object:{get:function(){return this.node}
                         ,set:function(f){this.node=f}
}
,flags:{get:function(){return this.iB.flags}
,set:function(f){this.iB.flags=f}
}
,position:{get:function(){return this.iB.position}
,set:function(f){this.iB.position=f}
}
}
));b=Object.assign(new Db,b);d=Cb(d);b.fd=d;return pb[d]=b}
,jb={open:b=>{b.cB=ob[b.node.rdev].cB;b.cB.open&&b.cB.open(b)}
,mB:()=>{throw new K(70);}
}
,fb=(b,d)=>{ob[b]={cB:d}
}
,
Fb=(b,d)=>{var f="/"===d,c=!d;if(f&&nb)throw new K(10);if(!f&&!c){var e=ub(d,{GB:!1}
                                                                           );d=e.path;e=e.node;if(e.tB)throw new K(10);if(16384!==(e.mode&61440))throw new K(54);}
d={type:b,jC:{}
,KB:d,XB:[]}
;b=b.jB(d);b.jB=d;d.root=b;f?nb=b:e&&(e.tB=d,e.jB&&e.jB.XB.push(d))}
,N=(b,d,f)=>{var c=ub(b,{parent:!0}
                      ).node;b=$a(b);if(!b||"."===b||".."===b)throw new K(28);var e=Bb(c,b);if(e)throw new K(e);if(!c.bB.sB)throw new K(63);return c.bB.sB(c,b,d,f)}
,Gb=(b,d,f)=>{"undefined"==typeof f&&(f=d,d=438);N(b,d|8192,
f)}
,Hb=(b,d)=>{if(!bb(b))throw new K(44);var f=ub(d,{parent:!0}
                                               ).node;if(!f)throw new K(44);d=$a(d);var c=Bb(f,d);if(c)throw new K(c);if(!f.bB.symlink)throw new K(63);f.bB.symlink(f,d,b)}
,tb=b=>{b=ub(b).node;if(!b)throw new K(44);if(!b.bB.readlink)throw new K(28);return bb(vb(b.parent),b.bB.readlink(b))}
,Jb=(b,d,f)=>{if(""===b)throw new K(44);if("string"==typeof d){var c=zb[d];if("undefined"==typeof c)throw Error("Unknown file open mode: "+d);d=c}
f=d&64?("undefined"==typeof f?438:f)&4095|32768:0;
if("object"==typeof b)var e=b;else{b=Ya(b);try{e=ub(b,{FB:!(d&131072)}
                                                    ).node}
catch(g){}
}
c=!1;if(d&64)if(e){if(d&128)throw new K(20);}
else e=N(b,f,0),c=!0;if(!e)throw new K(44);8192===(e.mode&61440)&&(d&=-513);if(d&65536&&16384!==(e.mode&61440))throw new K(54);if(!c&&(f=e?40960===(e.mode&61440)?32:16384===(e.mode&61440)&&("r"!==Ab(d)||d&512)?31:xb(e,Ab(d)):44))throw new K(f);if(d&512&&!c){f=e;f="string"==typeof f?ub(f,{FB:!0}
                                                                                                                                                                                                                                                                                              ).node:f;if(!f.bB.gB)throw new K(63);if(16384===(f.mode&61440))throw new K(31);
if(32768!==(f.mode&61440))throw new K(28);if(c=xb(f,"w"))throw new K(c);f.bB.gB(f,{size:0,timestamp:Date.now()}
                                                                                )}
d&=-131713;e=Eb({node:e,path:vb(e),flags:d,seekable:!0,position:0,cB:e.cB,fC:[],error:!1}
                );e.cB.open&&e.cB.open(e);!a.logReadFiles||d&1||(Ib||(Ib={}
                                                      ),b in Ib||(Ib[b]=1));return e}
,Kb=(b,d,f)=>{if(null===b.fd)throw new K(8);if(!b.seekable||!b.cB.mB)throw new K(70);if(0!=f&&1!=f&&2!=f)throw new K(28);b.position=b.cB.mB(b,d,f);b.fC=[]}
,Lb=()=>{K||(K=function(b,d){this.node=d;this.aC=function(f){this.lB=
f}
;this.aC(b);this.message="FS error"}
,K.prototype=Error(),K.prototype.constructor=K,[44].forEach(b=>{lb[b]=new K(b);lb[b].stack="<generic error, no stack>"}
                                                            ))}
,Mb,Nb=(b,d)=>{var f=0;b&&(f|=365);d&&(f|=146);return f}
,Pb=(b,d,f)=>{b=Ya("/dev/"+b);var c=Nb(!!d,!!f);Ob||(Ob=64);var e=Ob++<<8|0;fb(e,{open:g=>{g.seekable=!1}
                                                                               ,close:()=>{f&&f.buffer&&f.buffer.length&&f(10)}
,read:(g,h,l,n)=>{for(var w=0,y=0;y<n;y++){try{var C=d()}
catch(F){throw new K(29);}
if(void 0===C&&0===w)throw new K(6);if(null===C||
void 0===C)break;w++;h[l+y]=C}
w&&(g.node.timestamp=Date.now());return w}
,write:(g,h,l,n)=>{for(var w=0;w<n;w++)try{f(h[l+w])}
catch(y){throw new K(29);}
n&&(g.node.timestamp=Date.now());return w}
}
);Gb(b,c,e)}
,Ob,Qb={}
,Db,Ib,Rb=void 0;function Sb(){Rb+=4;return x[Rb-4>>2]}
function Tb(b){b=pb[b];if(!b)throw new K(8);return b}
var Ub;Ub=ma?()=>{var b=process.hrtime();return 1E3*b[0]+b[1]/1E6}
:()=>performance.now();var Vb={}
;
function Wb(){if(!Xb){var b={USER:"web_user",LOGNAME:"web_user",PATH:"/",PWD:"/",HOME:"/home/web_user",LANG:("object"==typeof navigator&&navigator.languages&&navigator.languages[0]||"C").replace("-","_")+".UTF-8",_:ia||"./this.program"}
,d;for(d in Vb)void 0===Vb[d]?delete b[d]:b[d]=Vb[d];var f=[];for(d in b)f.push(d+"="+b[d]);Xb=f}
return Xb}
var Xb;function Yb(b,d){for(var f=0;f<b.length;++f)v[d++>>0]=b.charCodeAt(f);v[d>>0]=0}
function Zb(b){return 0===b%4&&(0!==b%100||0===b%400)}

var $b=[31,29,31,30,31,30,31,31,30,31,30,31],ac=[31,28,31,30,31,30,31,31,30,31,30,31];
function bc(b,d,f,c){function e(m,B,A){for(m="number"==typeof m?m.toString():m||"";m.length<B;)m=A[0]+m;return m}
function g(m,B){return e(m,B,"0")}
function h(m,B){function A(M){return 0>M?-1:0<M?1:0}
var J;0===(J=A(m.getFullYear()-B.getFullYear()))&&0===(J=A(m.getMonth()-B.getMonth()))&&(J=A(m.getDate()-B.getDate()));return J}
function l(m){switch(m.getDay()){case 0:return new Date(m.getFullYear()-1,11,29);case 1:return m;case 2:return new Date(m.getFullYear(),0,3);case 3:return new Date(m.getFullYear(),
0,2);case 4:return new Date(m.getFullYear(),0,1);case 5:return new Date(m.getFullYear()-1,11,31);case 6:return new Date(m.getFullYear()-1,11,30)}
}
function n(m){var B=m.nB;for(m=new Date((new Date(m.oB+1900,0,1)).getTime());0<B;){var A=m.getMonth(),J=(Zb(m.getFullYear())?$b:ac)[A];if(B>J-m.getDate())B-=J-m.getDate()+1,m.setDate(1),11>A?m.setMonth(A+1):(m.setMonth(0),m.setFullYear(m.getFullYear()+1));else{m.setDate(m.getDate()+B);break}
}
A=new Date(m.getFullYear()+1,0,4);B=l(new Date(m.getFullYear(),
0,4));A=l(A);return 0>=h(B,m)?0>=h(A,m)?m.getFullYear()+1:m.getFullYear():m.getFullYear()-1}
var w=x[c+40>>2];c={dC:x[c>>2],cC:x[c+4>>2],uB:x[c+8>>2],BB:x[c+12>>2],vB:x[c+16>>2],oB:x[c+20>>2],hB:x[c+24>>2],nB:x[c+28>>2],kC:x[c+32>>2],bC:x[c+36>>2],eC:w?t(w):""}
;f=t(f);w={"%c":"%a %b %d %H:%M:%S %Y","%D":"%m/%d/%y","%F":"%Y-%m-%d","%h":"%b","%r":"%I:%M:%S %p","%R":"%H:%M","%T":"%H:%M:%S","%x":"%m/%d/%y","%X":"%H:%M:%S","%Ec":"%c","%EC":"%C","%Ex":"%m/%d/%y","%EX":"%H:%M:%S","%Ey":"%y","%EY":"%Y","%Od":"%d",
"%Oe":"%e","%OH":"%H","%OI":"%I","%Om":"%m","%OM":"%M","%OS":"%S","%Ou":"%u","%OU":"%U","%OV":"%V","%Ow":"%w","%OW":"%W","%Oy":"%y"}
;for(var y in w)f=f.replace(new RegExp(y,"g"),w[y]);var C="Sunday Monday Tuesday Wednesday Thursday Friday Saturday".split(" "),F="January February March April May June July August September October November December".split(" ");w={"%a":function(m){return C[m.hB].substring(0,3)}
,"%A":function(m){return C[m.hB]}
,"%b":function(m){return F[m.vB].substring(0,3)}
,"%B":function(m){return F[m.vB]}
,
"%C":function(m){return g((m.oB+1900)/100|0,2)}
,"%d":function(m){return g(m.BB,2)}
,"%e":function(m){return e(m.BB,2," ")}
,"%g":function(m){return n(m).toString().substring(2)}
,"%G":function(m){return n(m)}
,"%H":function(m){return g(m.uB,2)}
,"%I":function(m){m=m.uB;0==m?m=12:12<m&&(m-=12);return g(m,2)}
,"%j":function(m){for(var B=0,A=0;A<=m.vB-1;B+=(Zb(m.oB+1900)?$b:ac)[A++]);return g(m.BB+B,3)}
,"%m":function(m){return g(m.vB+1,2)}
,"%M":function(m){return g(m.cC,2)}
,"%n":function(){return"\n"}
,"%p":function(m){return 0<=
m.uB&&12>m.uB?"AM":"PM"}
,"%S":function(m){return g(m.dC,2)}
,"%t":function(){return"\t"}
,"%u":function(m){return m.hB||7}
,"%U":function(m){return g(Math.floor((m.nB+7-m.hB)/7),2)}
,"%V":function(m){var B=Math.floor((m.nB+7-(m.hB+6)%7)/7);2>=(m.hB+371-m.nB-2)%7&&B++;if(B)53==B&&(A=(m.hB+371-m.nB)%7,4==A||3==A&&Zb(m.oB)||(B=1));else{B=52;var A=(m.hB+7-m.nB-1)%7;(4==A||5==A&&Zb(m.oB%400-1))&&B++}
return g(B,2)}
,"%w":function(m){return m.hB}
,"%W":function(m){return g(Math.floor((m.nB+7-(m.hB+6)%7)/7),2)}
,
"%y":function(m){return(m.oB+1900).toString().substring(2)}
,"%Y":function(m){return m.oB+1900}
,"%z":function(m){m=m.bC;var B=0<=m;m=Math.abs(m)/60;return(B?"+":"-")+String("0000"+(m/60*100+m%60)).slice(-4)}
,"%Z":function(m){return m.eC}
,"%%":function(){return"%"}
}
;f=f.replace(/%%/g,"\x00\x00");for(y in w)f.includes(y)&&(f=f.replace(new RegExp(y,"g"),w[y](c)));f=f.replace(/\0\0/g,"%");y=cb(f,!1);if(y.length>d)return 0;v.set(y,b);return y.length-1}

function cc(b){var d=ya(b)+1,f=O(d);xa(b,v,f,d);return f}
function yb(b,d,f,c){b||(b=this);this.parent=b;this.jB=b.jB;this.tB=null;this.id=qb++;this.name=d;this.mode=f;this.bB={}
;this.cB={}
;this.rdev=c}
Object.defineProperties(yb.prototype,{read:{get:function(){return 365===(this.mode&365)}
                        ,set:function(b){b?this.mode|=365:this.mode&=-366}
}
,write:{get:function(){return 146===(this.mode&146)}
,set:function(b){b?this.mode|=146:this.mode&=-147}
}
}
);Lb();rb=Array(4096);Fb(L,"/");N("/tmp",16895,0);
N("/home",16895,0);N("/home/web_user",16895,0);(()=>{N("/dev",16895,0);fb(259,{read:()=>0,write:(d,f,c,e)=>e}
                                                                          );Gb("/dev/null",259);eb(1280,hb);eb(1536,ib);Gb("/dev/tty",1280);Gb("/dev/tty1",1536);var b=ab();Pb("random",b);Pb("urandom",b);N("/dev/shm",16895,0);N("/dev/shm/tmp",16895,0)}
)();
(()=>{N("/proc",16895,0);var b=N("/proc/self",16895,0);N("/proc/self/fd",16895,0);Fb({jB:()=>{var d=kb(b,"fd",16895,73);d.bB={lookup:(f,c)=>{var e=pb[+c];if(!e)throw new K(8);f={parent:null,jB:{KB:"fake"}
                                                                                     ,bB:{readlink:()=>e.path}
}
;return f.parent=f}
}
;return d}
}
,"/proc/self/fd")}
)();
var od={b:function(b,d,f,c){r("Assertion failed: "+t(b)+", at: "+[d?t(d):"unknown filename",f,c?t(c):"unknown function"])}
,n:function(b){return dc(b+24)+24}
,q:function(b){b=new Ta(b);b.PB()||(b.zB(!0),Sa--);b.AB(!1);Ra.push(b);b.NB();return b.QB()}
,ya:function(b){p("Unexpected exception thrown, this is not properly supported - aborting");ua=!0;throw b;}
,t:function(){P(0);var b=Ra.pop();if(b.ZB()&&!b.IB()){var d=b.SB();d&&I(d)(b.qB);Va(b.qB)}
G=0}
,a:function(){var b=G;if(!b)return Q(0),0;var d=new Ta(b);
d.rB(b);var f=d.iB();if(!f)return Q(0),b;for(var c=0;c<arguments.length;c++){var e=arguments[c];if(0===e||e===f)break;if(ec(e,f,d.dB+16))return Q(e),b}
Q(f);return b}
,l:function(){var b=G;if(!b)return Q(0),0;var d=new Ta(b);d.rB(b);var f=d.iB();if(!f)return Q(0),b;for(var c=0;c<arguments.length;c++){var e=arguments[c];if(0===e||e===f)break;if(ec(e,f,d.dB+16))return Q(e),b}
Q(f);return b}
,G:function(){var b=G;if(!b)return Q(0),0;var d=new Ta(b);d.rB(b);var f=d.iB();if(!f)return Q(0),b;for(var c=0;c<
arguments.length;c++){var e=arguments[c];if(0===e||e===f)break;if(ec(e,f,d.dB+16))return Q(e),b}
Q(f);return b}
,m:Va,F:function(){var b=Ra.pop();b||r("no exception to throw");var d=b.qB;b.IB()||(Ra.push(b),b.AB(!0),b.zB(!1),Sa++);G=d;throw d;}
,u:function(b,d,f){(new Ta(b)).RB(d,f);G=b;Sa++;throw b;}
,Da:function(){return Sa}
,h:function(b){G||(G=b);throw b;}
,H:function(b,d,f){Rb=f;try{var c=Tb(b);switch(d){case 0:var e=Sb();return 0>e?-28:Eb(c,e).fd;case 1:case 2:return 0;case 3:return c.flags;case 4:return e=
Sb(),c.flags|=e,0;case 5:return e=Sb(),Aa[e+0>>1]=2,0;case 6:case 7:return 0;case 16:case 8:return-28;case 9:return x[fc()>>2]=28,-1;default:return-28}
}
catch(g){if("undefined"==typeof Qb||!(g instanceof K))throw g;return-g.lB}
}
,Ba:function(b,d,f){Rb=f;try{var c=Tb(b);switch(d){case 21509:case 21505:return c.tty?0:-59;case 21510:case 21511:case 21512:case 21506:case 21507:case 21508:return c.tty?0:-59;case 21519:if(!c.tty)return-59;var e=Sb();return x[e>>2]=0;case 21520:return c.tty?-28:-59;case 21531:b=
e=Sb();if(!c.cB.VB)throw new K(59);return c.cB.VB(c,d,b);case 21523:return c.tty?0:-59;case 21524:return c.tty?0:-59;default:return-28}
}
catch(g){if("undefined"==typeof Qb||!(g instanceof K))throw g;return-g.lB}
}
,Ca:function(b,d,f,c){Rb=c;try{d=t(d);var e=d;if("/"===e.charAt(0))d=e;else{var g=-100===b?"/":Tb(b).path;if(0==e.length)throw new K(44);d=Ya(g+"/"+e)}
var h=c?Sb():0;return Jb(d,f,h).fd}
catch(l){if("undefined"==typeof Qb||!(l instanceof K))throw l;return-l.lB}
}
,Ea:function(){return!0}
,r:function(){r("")}
,
Ia:Ub,Ga:function(b,d,f){u.copyWithin(b,d,d+f)}
,Fa:function(b){var d=u.length;b>>>=0;if(2147483648<b)return!1;for(var f=1;4>=f;f*=2){var c=d*(1+.2/f);c=Math.min(c,b+100663296);var e=Math;c=Math.max(b,c);e=e.min.call(e,2147483648,c+(65536-c%65536)%65536);a:{try{ta.grow(e-za.byteLength+65535>>>16);Ba();var g=1;break a}
catch(h){}
g=void 0}
if(g)return!0}
  return!1}
,Ja:function(b,d){var f=0;Wb().forEach(function(c,e){var g=d+f;z[b+4*e>>2]=g;Yb(c,g);f+=c.length+1}
                                       );return 0}
,J:function(b,d){var f=Wb();z[b>>
2]=f.length;var c=0;f.forEach(function(e){c+=e.length+1}
                              );z[d>>2]=c;return 0}
,I:function(b){try{var d=Tb(b);if(null===d.fd)throw new K(8);d.wB&&(d.wB=null);try{d.cB.close&&d.cB.close(d)}
catch(f){throw f;}
finally{pb[d.fd]=null}
d.fd=null;return 0}
catch(f){if("undefined"==typeof Qb||!(f instanceof K))throw f;return f.lB}
}
,Aa:function(b,d,f,c){try{a:{var e=Tb(b);b=d;for(var g=d=0;g<f;g++){var h=z[b>>2],l=z[b+4>>2];b+=8;var n=e,w=h,y=l,C=void 0,F=v;if(0>y||0>C)throw new K(28);if(null===n.fd)throw new K(8);
if(1===(n.flags&2097155))throw new K(8);if(16384===(n.node.mode&61440))throw new K(31);if(!n.cB.read)throw new K(28);var m="undefined"!=typeof C;if(!m)C=n.position;else if(!n.seekable)throw new K(70);var B=n.cB.read(n,F,w,y,C);m||(n.position+=B);var A=B;if(0>A){var J=-1;break a}
d+=A;if(A<l)break}
J=d}
z[c>>2]=J;return 0}
catch(M){if("undefined"==typeof Qb||!(M instanceof K))throw M;return M.lB}
}
,ua:function(b,d,f,c,e){try{d=f+2097152>>>0<4194305-!!d?(d>>>0)+4294967296*f:NaN;if(isNaN(d))return 61;var g=
Tb(b);Kb(g,d,c);Pa=[g.position>>>0,(Oa=g.position,1<=+Math.abs(Oa)?0<Oa?(Math.min(+Math.floor(Oa/4294967296),4294967295)|0)>>>0:~~+Math.ceil((Oa-+(~~Oa>>>0))/4294967296)>>>0:0)];x[e>>2]=Pa[0];x[e+4>>2]=Pa[1];g.wB&&0===d&&0===c&&(g.wB=null);return 0}
catch(h){if("undefined"==typeof Qb||!(h instanceof K))throw h;return h.lB}
}
,Ha:function(b,d,f,c){try{a:{var e=Tb(b);b=d;for(var g=d=0;g<f;g++){var h=z[b>>2],l=z[b+4>>2];b+=8;var n=e,w=h,y=l,C=void 0,F=v;if(0>y||0>C)throw new K(28);if(null===n.fd)throw new K(8);
if(0===(n.flags&2097155))throw new K(8);if(16384===(n.node.mode&61440))throw new K(31);if(!n.cB.write)throw new K(28);n.seekable&&n.flags&1024&&Kb(n,0,2);var m="undefined"!=typeof C;if(!m)C=n.position;else if(!n.seekable)throw new K(70);var B=n.cB.write(n,F,w,y,C,void 0);m||(n.position+=B);var A=B;if(0>A){var J=-1;break a}
d+=A}
J=d}
z[c>>2]=J;return 0}
catch(M){if("undefined"==typeof Qb||!(M instanceof K))throw M;return M.lB}
}
,D:gc,f:hc,xa:ic,c:jc,e:kc,o:lc,va:mc,s:nc,v:oc,B:pc,A:qc,X:rc,L:sc,ra:tc,
ma:uc,ga:vc,N:wc,ta:xc,aa:yc,Y:zc,Z:Ac,_:Bc,W:Cc,ea:Dc,ha:Ec,na:Fc,ia:Gc,M:Hc,pa:Ic,ka:Jc,K:Kc,P:Lc,O:Mc,j:Nc,k:Oc,E:Pc,d:Qc,g:Rc,i:Sc,p:Tc,C:Uc,w:Vc,wa:Wc,y:Xc,z:Yc,qa:Zc,sa:$c,T:ad,$:bd,la:cd,S:dd,R:ed,fa:fd,oa:gd,ca:hd,da:jd,ba:kd,Q:ld,V:md,ja:nd,x:function(b){return b}
,U:function(b){if(!noExitRuntime){if(a.onExit)a.onExit(b);ua=!0}
ja(b,new ra(b))}
,za:function(b,d,f,c){return bc(b,d,f,c)}
}
;
(function(){function b(e){a.asm=e.exports;ta=a.asm.Ka;Ba();Ca=a.asm.jA;Ea.unshift(a.asm.La);D--;a.monitorRunDependencies&&a.monitorRunDependencies(D);0==D&&(null!==Ia&&(clearInterval(Ia),Ia=null),Ja&&(e=Ja,Ja=null,e()))}
 function d(e){b(e.instance)}
function f(e){return Na().then(function(g){return WebAssembly.instantiate(g,c)}
                               ).then(function(g){return g}
).then(e,function(g){p("failed to asynchronously prepare wasm: "+g);r(g)}
)}
var c={a:od}
;D++;a.monitorRunDependencies&&a.monitorRunDependencies(D);
if(a.instantiateWasm)try{return a.instantiateWasm(c,b)}
  catch(e){p("Module.instantiateWasm callback failed with error: "+e),fa(e)}
(function(){return sa||"function"!=typeof WebAssembly.instantiateStreaming||Ka()||E.startsWith("file://")||ma||"function"!=typeof fetch?f(d):fetch(E,{credentials:"same-origin"}
                                                                                                                                                   ).then(function(e){return WebAssembly.instantiateStreaming(e,c).then(d,function(g){p("wasm streaming compile failed: "+g);p("falling back to ArrayBuffer instantiation");return f(d)}
                                                                     )}
)}
)().catch(fa);
return{}
}
)();a.___wasm_call_ctors=function(){return(a.___wasm_call_ctors=a.asm.La).apply(null,arguments)}
;var H=a._free=function(){return(H=a._free=a.asm.Ma).apply(null,arguments)}
;a._BinaryenTypeNone=function(){return(a._BinaryenTypeNone=a.asm.Na).apply(null,arguments)}
;a._BinaryenTypeInt32=function(){return(a._BinaryenTypeInt32=a.asm.Oa).apply(null,arguments)}
;a._BinaryenTypeInt64=function(){return(a._BinaryenTypeInt64=a.asm.Pa).apply(null,arguments)}
;
a._BinaryenTypeFloat32=function(){return(a._BinaryenTypeFloat32=a.asm.Qa).apply(null,arguments)}
;a._BinaryenTypeFloat64=function(){return(a._BinaryenTypeFloat64=a.asm.Ra).apply(null,arguments)}
;a._BinaryenTypeVec128=function(){return(a._BinaryenTypeVec128=a.asm.Sa).apply(null,arguments)}
;a._BinaryenTypeFuncref=function(){return(a._BinaryenTypeFuncref=a.asm.Ta).apply(null,arguments)}
;a._BinaryenTypeExternref=function(){return(a._BinaryenTypeExternref=a.asm.Ua).apply(null,arguments)}
;
a._BinaryenTypeAnyref=function(){return(a._BinaryenTypeAnyref=a.asm.Va).apply(null,arguments)}
;a._BinaryenTypeEqref=function(){return(a._BinaryenTypeEqref=a.asm.Wa).apply(null,arguments)}
;a._BinaryenTypeI31ref=function(){return(a._BinaryenTypeI31ref=a.asm.Xa).apply(null,arguments)}
;a._BinaryenTypeDataref=function(){return(a._BinaryenTypeDataref=a.asm.Ya).apply(null,arguments)}
;a._BinaryenTypeArrayref=function(){return(a._BinaryenTypeArrayref=a.asm.Za).apply(null,arguments)}
;
a._BinaryenTypeStringref=function(){return(a._BinaryenTypeStringref=a.asm._a).apply(null,arguments)}
;a._BinaryenTypeStringviewWTF8=function(){return(a._BinaryenTypeStringviewWTF8=a.asm.$a).apply(null,arguments)}
;a._BinaryenTypeStringviewWTF16=function(){return(a._BinaryenTypeStringviewWTF16=a.asm.ab).apply(null,arguments)}
;a._BinaryenTypeStringviewIter=function(){return(a._BinaryenTypeStringviewIter=a.asm.bb).apply(null,arguments)}
;
a._BinaryenTypeNullref=function(){return(a._BinaryenTypeNullref=a.asm.cb).apply(null,arguments)}
;a._BinaryenTypeNullExternref=function(){return(a._BinaryenTypeNullExternref=a.asm.db).apply(null,arguments)}
;a._BinaryenTypeNullFuncref=function(){return(a._BinaryenTypeNullFuncref=a.asm.eb).apply(null,arguments)}
;a._BinaryenTypeUnreachable=function(){return(a._BinaryenTypeUnreachable=a.asm.fb).apply(null,arguments)}
;a._BinaryenTypeAuto=function(){return(a._BinaryenTypeAuto=a.asm.gb).apply(null,arguments)}
;
a._BinaryenTypeCreate=function(){return(a._BinaryenTypeCreate=a.asm.hb).apply(null,arguments)}
;a._BinaryenTypeArity=function(){return(a._BinaryenTypeArity=a.asm.ib).apply(null,arguments)}
;a._BinaryenTypeExpand=function(){return(a._BinaryenTypeExpand=a.asm.jb).apply(null,arguments)}
;a._BinaryenPackedTypeNotPacked=function(){return(a._BinaryenPackedTypeNotPacked=a.asm.kb).apply(null,arguments)}
;a._BinaryenPackedTypeInt8=function(){return(a._BinaryenPackedTypeInt8=a.asm.lb).apply(null,arguments)}
;
a._BinaryenPackedTypeInt16=function(){return(a._BinaryenPackedTypeInt16=a.asm.mb).apply(null,arguments)}
;a._BinaryenHeapTypeExt=function(){return(a._BinaryenHeapTypeExt=a.asm.nb).apply(null,arguments)}
;a._BinaryenHeapTypeFunc=function(){return(a._BinaryenHeapTypeFunc=a.asm.ob).apply(null,arguments)}
;a._BinaryenHeapTypeAny=function(){return(a._BinaryenHeapTypeAny=a.asm.pb).apply(null,arguments)}
;a._BinaryenHeapTypeEq=function(){return(a._BinaryenHeapTypeEq=a.asm.qb).apply(null,arguments)}
;
a._BinaryenHeapTypeI31=function(){return(a._BinaryenHeapTypeI31=a.asm.rb).apply(null,arguments)}
;a._BinaryenHeapTypeData=function(){return(a._BinaryenHeapTypeData=a.asm.sb).apply(null,arguments)}
;a._BinaryenHeapTypeArray=function(){return(a._BinaryenHeapTypeArray=a.asm.tb).apply(null,arguments)}
;a._BinaryenHeapTypeString=function(){return(a._BinaryenHeapTypeString=a.asm.ub).apply(null,arguments)}
;
a._BinaryenHeapTypeStringviewWTF8=function(){return(a._BinaryenHeapTypeStringviewWTF8=a.asm.vb).apply(null,arguments)}
;a._BinaryenHeapTypeStringviewWTF16=function(){return(a._BinaryenHeapTypeStringviewWTF16=a.asm.wb).apply(null,arguments)}
;a._BinaryenHeapTypeStringviewIter=function(){return(a._BinaryenHeapTypeStringviewIter=a.asm.xb).apply(null,arguments)}
;a._BinaryenHeapTypeNone=function(){return(a._BinaryenHeapTypeNone=a.asm.yb).apply(null,arguments)}
;
a._BinaryenHeapTypeNoext=function(){return(a._BinaryenHeapTypeNoext=a.asm.zb).apply(null,arguments)}
;a._BinaryenHeapTypeNofunc=function(){return(a._BinaryenHeapTypeNofunc=a.asm.Ab).apply(null,arguments)}
;a._BinaryenHeapTypeIsBasic=function(){return(a._BinaryenHeapTypeIsBasic=a.asm.Bb).apply(null,arguments)}
;a._BinaryenHeapTypeIsSignature=function(){return(a._BinaryenHeapTypeIsSignature=a.asm.Cb).apply(null,arguments)}
;
a._BinaryenHeapTypeIsStruct=function(){return(a._BinaryenHeapTypeIsStruct=a.asm.Db).apply(null,arguments)}
;a._BinaryenHeapTypeIsArray=function(){return(a._BinaryenHeapTypeIsArray=a.asm.Eb).apply(null,arguments)}
;a._BinaryenHeapTypeIsBottom=function(){return(a._BinaryenHeapTypeIsBottom=a.asm.Fb).apply(null,arguments)}
;a._BinaryenHeapTypeGetBottom=function(){return(a._BinaryenHeapTypeGetBottom=a.asm.Gb).apply(null,arguments)}
;
a._BinaryenHeapTypeIsSubType=function(){return(a._BinaryenHeapTypeIsSubType=a.asm.Hb).apply(null,arguments)}
;a._BinaryenTypeGetHeapType=function(){return(a._BinaryenTypeGetHeapType=a.asm.Ib).apply(null,arguments)}
;a._BinaryenTypeIsNullable=function(){return(a._BinaryenTypeIsNullable=a.asm.Jb).apply(null,arguments)}
;a._BinaryenTypeFromHeapType=function(){return(a._BinaryenTypeFromHeapType=a.asm.Kb).apply(null,arguments)}
;
a._BinaryenTypeSystemEquirecursive=function(){return(a._BinaryenTypeSystemEquirecursive=a.asm.Lb).apply(null,arguments)}
;a._BinaryenTypeSystemNominal=function(){return(a._BinaryenTypeSystemNominal=a.asm.Mb).apply(null,arguments)}
;a._BinaryenTypeSystemIsorecursive=function(){return(a._BinaryenTypeSystemIsorecursive=a.asm.Nb).apply(null,arguments)}
;a._BinaryenGetTypeSystem=function(){return(a._BinaryenGetTypeSystem=a.asm.Ob).apply(null,arguments)}
;
a._BinaryenSetTypeSystem=function(){return(a._BinaryenSetTypeSystem=a.asm.Pb).apply(null,arguments)}
;a._BinaryenInvalidId=function(){return(a._BinaryenInvalidId=a.asm.Qb).apply(null,arguments)}
;a._BinaryenNopId=function(){return(a._BinaryenNopId=a.asm.Rb).apply(null,arguments)}
;a._BinaryenBlockId=function(){return(a._BinaryenBlockId=a.asm.Sb).apply(null,arguments)}
;a._BinaryenIfId=function(){return(a._BinaryenIfId=a.asm.Tb).apply(null,arguments)}
;
a._BinaryenLoopId=function(){return(a._BinaryenLoopId=a.asm.Ub).apply(null,arguments)}
;a._BinaryenBreakId=function(){return(a._BinaryenBreakId=a.asm.Vb).apply(null,arguments)}
;a._BinaryenSwitchId=function(){return(a._BinaryenSwitchId=a.asm.Wb).apply(null,arguments)}
;a._BinaryenCallId=function(){return(a._BinaryenCallId=a.asm.Xb).apply(null,arguments)}
;a._BinaryenCallIndirectId=function(){return(a._BinaryenCallIndirectId=a.asm.Yb).apply(null,arguments)}
;
a._BinaryenLocalGetId=function(){return(a._BinaryenLocalGetId=a.asm.Zb).apply(null,arguments)}
;a._BinaryenLocalSetId=function(){return(a._BinaryenLocalSetId=a.asm._b).apply(null,arguments)}
;a._BinaryenGlobalGetId=function(){return(a._BinaryenGlobalGetId=a.asm.$b).apply(null,arguments)}
;a._BinaryenGlobalSetId=function(){return(a._BinaryenGlobalSetId=a.asm.ac).apply(null,arguments)}
;a._BinaryenLoadId=function(){return(a._BinaryenLoadId=a.asm.bc).apply(null,arguments)}
;
a._BinaryenStoreId=function(){return(a._BinaryenStoreId=a.asm.cc).apply(null,arguments)}
;a._BinaryenAtomicRMWId=function(){return(a._BinaryenAtomicRMWId=a.asm.dc).apply(null,arguments)}
;a._BinaryenAtomicCmpxchgId=function(){return(a._BinaryenAtomicCmpxchgId=a.asm.ec).apply(null,arguments)}
;a._BinaryenAtomicWaitId=function(){return(a._BinaryenAtomicWaitId=a.asm.fc).apply(null,arguments)}
;a._BinaryenAtomicNotifyId=function(){return(a._BinaryenAtomicNotifyId=a.asm.gc).apply(null,arguments)}
;
a._BinaryenAtomicFenceId=function(){return(a._BinaryenAtomicFenceId=a.asm.hc).apply(null,arguments)}
;a._BinaryenSIMDExtractId=function(){return(a._BinaryenSIMDExtractId=a.asm.ic).apply(null,arguments)}
;a._BinaryenSIMDReplaceId=function(){return(a._BinaryenSIMDReplaceId=a.asm.jc).apply(null,arguments)}
;a._BinaryenSIMDShuffleId=function(){return(a._BinaryenSIMDShuffleId=a.asm.kc).apply(null,arguments)}
;a._BinaryenSIMDTernaryId=function(){return(a._BinaryenSIMDTernaryId=a.asm.lc).apply(null,arguments)}
;
a._BinaryenSIMDShiftId=function(){return(a._BinaryenSIMDShiftId=a.asm.mc).apply(null,arguments)}
;a._BinaryenSIMDLoadId=function(){return(a._BinaryenSIMDLoadId=a.asm.nc).apply(null,arguments)}
;a._BinaryenSIMDLoadStoreLaneId=function(){return(a._BinaryenSIMDLoadStoreLaneId=a.asm.oc).apply(null,arguments)}
;a._BinaryenMemoryInitId=function(){return(a._BinaryenMemoryInitId=a.asm.pc).apply(null,arguments)}
;a._BinaryenDataDropId=function(){return(a._BinaryenDataDropId=a.asm.qc).apply(null,arguments)}
;
a._BinaryenMemoryCopyId=function(){return(a._BinaryenMemoryCopyId=a.asm.rc).apply(null,arguments)}
;a._BinaryenMemoryFillId=function(){return(a._BinaryenMemoryFillId=a.asm.sc).apply(null,arguments)}
;a._BinaryenConstId=function(){return(a._BinaryenConstId=a.asm.tc).apply(null,arguments)}
;a._BinaryenUnaryId=function(){return(a._BinaryenUnaryId=a.asm.uc).apply(null,arguments)}
;a._BinaryenBinaryId=function(){return(a._BinaryenBinaryId=a.asm.vc).apply(null,arguments)}
;
a._BinaryenSelectId=function(){return(a._BinaryenSelectId=a.asm.wc).apply(null,arguments)}
;a._BinaryenDropId=function(){return(a._BinaryenDropId=a.asm.xc).apply(null,arguments)}
;a._BinaryenReturnId=function(){return(a._BinaryenReturnId=a.asm.yc).apply(null,arguments)}
;a._BinaryenMemorySizeId=function(){return(a._BinaryenMemorySizeId=a.asm.zc).apply(null,arguments)}
;a._BinaryenMemoryGrowId=function(){return(a._BinaryenMemoryGrowId=a.asm.Ac).apply(null,arguments)}
;
a._BinaryenUnreachableId=function(){return(a._BinaryenUnreachableId=a.asm.Bc).apply(null,arguments)}
;a._BinaryenPopId=function(){return(a._BinaryenPopId=a.asm.Cc).apply(null,arguments)}
;a._BinaryenRefNullId=function(){return(a._BinaryenRefNullId=a.asm.Dc).apply(null,arguments)}
;a._BinaryenRefIsId=function(){return(a._BinaryenRefIsId=a.asm.Ec).apply(null,arguments)}
;a._BinaryenRefFuncId=function(){return(a._BinaryenRefFuncId=a.asm.Fc).apply(null,arguments)}
;
a._BinaryenRefEqId=function(){return(a._BinaryenRefEqId=a.asm.Gc).apply(null,arguments)}
;a._BinaryenTableGetId=function(){return(a._BinaryenTableGetId=a.asm.Hc).apply(null,arguments)}
;a._BinaryenTableSetId=function(){return(a._BinaryenTableSetId=a.asm.Ic).apply(null,arguments)}
;a._BinaryenTableSizeId=function(){return(a._BinaryenTableSizeId=a.asm.Jc).apply(null,arguments)}
;a._BinaryenTableGrowId=function(){return(a._BinaryenTableGrowId=a.asm.Kc).apply(null,arguments)}
;
a._BinaryenTryId=function(){return(a._BinaryenTryId=a.asm.Lc).apply(null,arguments)}
;a._BinaryenThrowId=function(){return(a._BinaryenThrowId=a.asm.Mc).apply(null,arguments)}
;a._BinaryenRethrowId=function(){return(a._BinaryenRethrowId=a.asm.Nc).apply(null,arguments)}
;a._BinaryenTupleMakeId=function(){return(a._BinaryenTupleMakeId=a.asm.Oc).apply(null,arguments)}
;a._BinaryenTupleExtractId=function(){return(a._BinaryenTupleExtractId=a.asm.Pc).apply(null,arguments)}
;
a._BinaryenI31NewId=function(){return(a._BinaryenI31NewId=a.asm.Qc).apply(null,arguments)}
;a._BinaryenI31GetId=function(){return(a._BinaryenI31GetId=a.asm.Rc).apply(null,arguments)}
;a._BinaryenCallRefId=function(){return(a._BinaryenCallRefId=a.asm.Sc).apply(null,arguments)}
;a._BinaryenRefTestId=function(){return(a._BinaryenRefTestId=a.asm.Tc).apply(null,arguments)}
;a._BinaryenRefCastId=function(){return(a._BinaryenRefCastId=a.asm.Uc).apply(null,arguments)}
;
a._BinaryenBrOnId=function(){return(a._BinaryenBrOnId=a.asm.Vc).apply(null,arguments)}
;a._BinaryenStructNewId=function(){return(a._BinaryenStructNewId=a.asm.Wc).apply(null,arguments)}
;a._BinaryenStructGetId=function(){return(a._BinaryenStructGetId=a.asm.Xc).apply(null,arguments)}
;a._BinaryenStructSetId=function(){return(a._BinaryenStructSetId=a.asm.Yc).apply(null,arguments)}
;a._BinaryenArrayNewId=function(){return(a._BinaryenArrayNewId=a.asm.Zc).apply(null,arguments)}
;
a._BinaryenArrayInitId=function(){return(a._BinaryenArrayInitId=a.asm._c).apply(null,arguments)}
;a._BinaryenArrayGetId=function(){return(a._BinaryenArrayGetId=a.asm.$c).apply(null,arguments)}
;a._BinaryenArraySetId=function(){return(a._BinaryenArraySetId=a.asm.ad).apply(null,arguments)}
;a._BinaryenArrayLenId=function(){return(a._BinaryenArrayLenId=a.asm.bd).apply(null,arguments)}
;a._BinaryenArrayCopyId=function(){return(a._BinaryenArrayCopyId=a.asm.cd).apply(null,arguments)}
;
a._BinaryenRefAsId=function(){return(a._BinaryenRefAsId=a.asm.dd).apply(null,arguments)}
;a._BinaryenStringNewId=function(){return(a._BinaryenStringNewId=a.asm.ed).apply(null,arguments)}
;a._BinaryenStringConstId=function(){return(a._BinaryenStringConstId=a.asm.fd).apply(null,arguments)}
;a._BinaryenStringMeasureId=function(){return(a._BinaryenStringMeasureId=a.asm.gd).apply(null,arguments)}
;a._BinaryenStringEncodeId=function(){return(a._BinaryenStringEncodeId=a.asm.hd).apply(null,arguments)}
;
a._BinaryenStringConcatId=function(){return(a._BinaryenStringConcatId=a.asm.id).apply(null,arguments)}
;a._BinaryenStringEqId=function(){return(a._BinaryenStringEqId=a.asm.jd).apply(null,arguments)}
;a._BinaryenStringAsId=function(){return(a._BinaryenStringAsId=a.asm.kd).apply(null,arguments)}
;a._BinaryenStringWTF8AdvanceId=function(){return(a._BinaryenStringWTF8AdvanceId=a.asm.ld).apply(null,arguments)}
;
a._BinaryenStringWTF16GetId=function(){return(a._BinaryenStringWTF16GetId=a.asm.md).apply(null,arguments)}
;a._BinaryenStringIterNextId=function(){return(a._BinaryenStringIterNextId=a.asm.nd).apply(null,arguments)}
;a._BinaryenStringIterMoveId=function(){return(a._BinaryenStringIterMoveId=a.asm.od).apply(null,arguments)}
;a._BinaryenStringSliceWTFId=function(){return(a._BinaryenStringSliceWTFId=a.asm.pd).apply(null,arguments)}
;
a._BinaryenStringSliceIterId=function(){return(a._BinaryenStringSliceIterId=a.asm.qd).apply(null,arguments)}
;a._BinaryenExternalFunction=function(){return(a._BinaryenExternalFunction=a.asm.rd).apply(null,arguments)}
;a._BinaryenExternalTable=function(){return(a._BinaryenExternalTable=a.asm.sd).apply(null,arguments)}
;a._BinaryenExternalMemory=function(){return(a._BinaryenExternalMemory=a.asm.td).apply(null,arguments)}
;
a._BinaryenExternalGlobal=function(){return(a._BinaryenExternalGlobal=a.asm.ud).apply(null,arguments)}
;a._BinaryenExternalTag=function(){return(a._BinaryenExternalTag=a.asm.vd).apply(null,arguments)}
;a._BinaryenFeatureMVP=function(){return(a._BinaryenFeatureMVP=a.asm.wd).apply(null,arguments)}
;a._BinaryenFeatureAtomics=function(){return(a._BinaryenFeatureAtomics=a.asm.xd).apply(null,arguments)}
;a._BinaryenFeatureBulkMemory=function(){return(a._BinaryenFeatureBulkMemory=a.asm.yd).apply(null,arguments)}
;
a._BinaryenFeatureMutableGlobals=function(){return(a._BinaryenFeatureMutableGlobals=a.asm.zd).apply(null,arguments)}
;a._BinaryenFeatureNontrappingFPToInt=function(){return(a._BinaryenFeatureNontrappingFPToInt=a.asm.Ad).apply(null,arguments)}
;a._BinaryenFeatureSignExt=function(){return(a._BinaryenFeatureSignExt=a.asm.Bd).apply(null,arguments)}
;a._BinaryenFeatureSIMD128=function(){return(a._BinaryenFeatureSIMD128=a.asm.Cd).apply(null,arguments)}
;
a._BinaryenFeatureExceptionHandling=function(){return(a._BinaryenFeatureExceptionHandling=a.asm.Dd).apply(null,arguments)}
;a._BinaryenFeatureTailCall=function(){return(a._BinaryenFeatureTailCall=a.asm.Ed).apply(null,arguments)}
;a._BinaryenFeatureReferenceTypes=function(){return(a._BinaryenFeatureReferenceTypes=a.asm.Fd).apply(null,arguments)}
;a._BinaryenFeatureMultivalue=function(){return(a._BinaryenFeatureMultivalue=a.asm.Gd).apply(null,arguments)}
;
a._BinaryenFeatureGC=function(){return(a._BinaryenFeatureGC=a.asm.Hd).apply(null,arguments)}
;a._BinaryenFeatureMemory64=function(){return(a._BinaryenFeatureMemory64=a.asm.Id).apply(null,arguments)}
;a._BinaryenFeatureRelaxedSIMD=function(){return(a._BinaryenFeatureRelaxedSIMD=a.asm.Jd).apply(null,arguments)}
;a._BinaryenFeatureExtendedConst=function(){return(a._BinaryenFeatureExtendedConst=a.asm.Kd).apply(null,arguments)}
;
a._BinaryenFeatureStrings=function(){return(a._BinaryenFeatureStrings=a.asm.Ld).apply(null,arguments)}
;a._BinaryenFeatureMultiMemories=function(){return(a._BinaryenFeatureMultiMemories=a.asm.Md).apply(null,arguments)}
;a._BinaryenFeatureAll=function(){return(a._BinaryenFeatureAll=a.asm.Nd).apply(null,arguments)}
;a._BinaryenModuleCreate=function(){return(a._BinaryenModuleCreate=a.asm.Od).apply(null,arguments)}
;
a._BinaryenModuleDispose=function(){return(a._BinaryenModuleDispose=a.asm.Pd).apply(null,arguments)}
;a._BinaryenLiteralInt32=function(){return(a._BinaryenLiteralInt32=a.asm.Qd).apply(null,arguments)}
;a._BinaryenLiteralInt64=function(){return(a._BinaryenLiteralInt64=a.asm.Rd).apply(null,arguments)}
;a._BinaryenLiteralFloat32=function(){return(a._BinaryenLiteralFloat32=a.asm.Sd).apply(null,arguments)}
;a._BinaryenLiteralFloat64=function(){return(a._BinaryenLiteralFloat64=a.asm.Td).apply(null,arguments)}
;
a._BinaryenLiteralVec128=function(){return(a._BinaryenLiteralVec128=a.asm.Ud).apply(null,arguments)}
;a._BinaryenLiteralFloat32Bits=function(){return(a._BinaryenLiteralFloat32Bits=a.asm.Vd).apply(null,arguments)}
;a._BinaryenLiteralFloat64Bits=function(){return(a._BinaryenLiteralFloat64Bits=a.asm.Wd).apply(null,arguments)}
;a._BinaryenClzInt32=function(){return(a._BinaryenClzInt32=a.asm.Xd).apply(null,arguments)}
;a._BinaryenCtzInt32=function(){return(a._BinaryenCtzInt32=a.asm.Yd).apply(null,arguments)}
;
a._BinaryenPopcntInt32=function(){return(a._BinaryenPopcntInt32=a.asm.Zd).apply(null,arguments)}
;a._BinaryenNegFloat32=function(){return(a._BinaryenNegFloat32=a.asm._d).apply(null,arguments)}
;a._BinaryenAbsFloat32=function(){return(a._BinaryenAbsFloat32=a.asm.$d).apply(null,arguments)}
;a._BinaryenCeilFloat32=function(){return(a._BinaryenCeilFloat32=a.asm.ae).apply(null,arguments)}
;a._BinaryenFloorFloat32=function(){return(a._BinaryenFloorFloat32=a.asm.be).apply(null,arguments)}
;
a._BinaryenTruncFloat32=function(){return(a._BinaryenTruncFloat32=a.asm.ce).apply(null,arguments)}
;a._BinaryenNearestFloat32=function(){return(a._BinaryenNearestFloat32=a.asm.de).apply(null,arguments)}
;a._BinaryenSqrtFloat32=function(){return(a._BinaryenSqrtFloat32=a.asm.ee).apply(null,arguments)}
;a._BinaryenEqZInt32=function(){return(a._BinaryenEqZInt32=a.asm.fe).apply(null,arguments)}
;a._BinaryenClzInt64=function(){return(a._BinaryenClzInt64=a.asm.ge).apply(null,arguments)}
;
a._BinaryenCtzInt64=function(){return(a._BinaryenCtzInt64=a.asm.he).apply(null,arguments)}
;a._BinaryenPopcntInt64=function(){return(a._BinaryenPopcntInt64=a.asm.ie).apply(null,arguments)}
;a._BinaryenNegFloat64=function(){return(a._BinaryenNegFloat64=a.asm.je).apply(null,arguments)}
;a._BinaryenAbsFloat64=function(){return(a._BinaryenAbsFloat64=a.asm.ke).apply(null,arguments)}
;a._BinaryenCeilFloat64=function(){return(a._BinaryenCeilFloat64=a.asm.le).apply(null,arguments)}
;
a._BinaryenFloorFloat64=function(){return(a._BinaryenFloorFloat64=a.asm.me).apply(null,arguments)}
;a._BinaryenTruncFloat64=function(){return(a._BinaryenTruncFloat64=a.asm.ne).apply(null,arguments)}
;a._BinaryenNearestFloat64=function(){return(a._BinaryenNearestFloat64=a.asm.oe).apply(null,arguments)}
;a._BinaryenSqrtFloat64=function(){return(a._BinaryenSqrtFloat64=a.asm.pe).apply(null,arguments)}
;a._BinaryenEqZInt64=function(){return(a._BinaryenEqZInt64=a.asm.qe).apply(null,arguments)}
;
a._BinaryenExtendSInt32=function(){return(a._BinaryenExtendSInt32=a.asm.re).apply(null,arguments)}
;a._BinaryenExtendUInt32=function(){return(a._BinaryenExtendUInt32=a.asm.se).apply(null,arguments)}
;a._BinaryenWrapInt64=function(){return(a._BinaryenWrapInt64=a.asm.te).apply(null,arguments)}
;a._BinaryenTruncSFloat32ToInt32=function(){return(a._BinaryenTruncSFloat32ToInt32=a.asm.ue).apply(null,arguments)}
;
a._BinaryenTruncSFloat32ToInt64=function(){return(a._BinaryenTruncSFloat32ToInt64=a.asm.ve).apply(null,arguments)}
;a._BinaryenTruncUFloat32ToInt32=function(){return(a._BinaryenTruncUFloat32ToInt32=a.asm.we).apply(null,arguments)}
;a._BinaryenTruncUFloat32ToInt64=function(){return(a._BinaryenTruncUFloat32ToInt64=a.asm.xe).apply(null,arguments)}
;a._BinaryenTruncSFloat64ToInt32=function(){return(a._BinaryenTruncSFloat64ToInt32=a.asm.ye).apply(null,arguments)}
;
a._BinaryenTruncSFloat64ToInt64=function(){return(a._BinaryenTruncSFloat64ToInt64=a.asm.ze).apply(null,arguments)}
;a._BinaryenTruncUFloat64ToInt32=function(){return(a._BinaryenTruncUFloat64ToInt32=a.asm.Ae).apply(null,arguments)}
;a._BinaryenTruncUFloat64ToInt64=function(){return(a._BinaryenTruncUFloat64ToInt64=a.asm.Be).apply(null,arguments)}
;a._BinaryenReinterpretFloat32=function(){return(a._BinaryenReinterpretFloat32=a.asm.Ce).apply(null,arguments)}
;
a._BinaryenReinterpretFloat64=function(){return(a._BinaryenReinterpretFloat64=a.asm.De).apply(null,arguments)}
;a._BinaryenExtendS8Int32=function(){return(a._BinaryenExtendS8Int32=a.asm.Ee).apply(null,arguments)}
;a._BinaryenExtendS16Int32=function(){return(a._BinaryenExtendS16Int32=a.asm.Fe).apply(null,arguments)}
;a._BinaryenExtendS8Int64=function(){return(a._BinaryenExtendS8Int64=a.asm.Ge).apply(null,arguments)}
;
a._BinaryenExtendS16Int64=function(){return(a._BinaryenExtendS16Int64=a.asm.He).apply(null,arguments)}
;a._BinaryenExtendS32Int64=function(){return(a._BinaryenExtendS32Int64=a.asm.Ie).apply(null,arguments)}
;a._BinaryenConvertSInt32ToFloat32=function(){return(a._BinaryenConvertSInt32ToFloat32=a.asm.Je).apply(null,arguments)}
;a._BinaryenConvertSInt32ToFloat64=function(){return(a._BinaryenConvertSInt32ToFloat64=a.asm.Ke).apply(null,arguments)}
;
a._BinaryenConvertUInt32ToFloat32=function(){return(a._BinaryenConvertUInt32ToFloat32=a.asm.Le).apply(null,arguments)}
;a._BinaryenConvertUInt32ToFloat64=function(){return(a._BinaryenConvertUInt32ToFloat64=a.asm.Me).apply(null,arguments)}
;a._BinaryenConvertSInt64ToFloat32=function(){return(a._BinaryenConvertSInt64ToFloat32=a.asm.Ne).apply(null,arguments)}
;a._BinaryenConvertSInt64ToFloat64=function(){return(a._BinaryenConvertSInt64ToFloat64=a.asm.Oe).apply(null,arguments)}
;
a._BinaryenConvertUInt64ToFloat32=function(){return(a._BinaryenConvertUInt64ToFloat32=a.asm.Pe).apply(null,arguments)}
;a._BinaryenConvertUInt64ToFloat64=function(){return(a._BinaryenConvertUInt64ToFloat64=a.asm.Qe).apply(null,arguments)}
;a._BinaryenPromoteFloat32=function(){return(a._BinaryenPromoteFloat32=a.asm.Re).apply(null,arguments)}
;a._BinaryenDemoteFloat64=function(){return(a._BinaryenDemoteFloat64=a.asm.Se).apply(null,arguments)}
;
a._BinaryenReinterpretInt32=function(){return(a._BinaryenReinterpretInt32=a.asm.Te).apply(null,arguments)}
;a._BinaryenReinterpretInt64=function(){return(a._BinaryenReinterpretInt64=a.asm.Ue).apply(null,arguments)}
;a._BinaryenAddInt32=function(){return(a._BinaryenAddInt32=a.asm.Ve).apply(null,arguments)}
;a._BinaryenSubInt32=function(){return(a._BinaryenSubInt32=a.asm.We).apply(null,arguments)}
;a._BinaryenMulInt32=function(){return(a._BinaryenMulInt32=a.asm.Xe).apply(null,arguments)}
;
a._BinaryenDivSInt32=function(){return(a._BinaryenDivSInt32=a.asm.Ye).apply(null,arguments)}
;a._BinaryenDivUInt32=function(){return(a._BinaryenDivUInt32=a.asm.Ze).apply(null,arguments)}
;a._BinaryenRemSInt32=function(){return(a._BinaryenRemSInt32=a.asm._e).apply(null,arguments)}
;a._BinaryenRemUInt32=function(){return(a._BinaryenRemUInt32=a.asm.$e).apply(null,arguments)}
;a._BinaryenAndInt32=function(){return(a._BinaryenAndInt32=a.asm.af).apply(null,arguments)}
;
a._BinaryenOrInt32=function(){return(a._BinaryenOrInt32=a.asm.bf).apply(null,arguments)}
;a._BinaryenXorInt32=function(){return(a._BinaryenXorInt32=a.asm.cf).apply(null,arguments)}
;a._BinaryenShlInt32=function(){return(a._BinaryenShlInt32=a.asm.df).apply(null,arguments)}
;a._BinaryenShrUInt32=function(){return(a._BinaryenShrUInt32=a.asm.ef).apply(null,arguments)}
;a._BinaryenShrSInt32=function(){return(a._BinaryenShrSInt32=a.asm.ff).apply(null,arguments)}
;
a._BinaryenRotLInt32=function(){return(a._BinaryenRotLInt32=a.asm.gf).apply(null,arguments)}
;a._BinaryenRotRInt32=function(){return(a._BinaryenRotRInt32=a.asm.hf).apply(null,arguments)}
;a._BinaryenEqInt32=function(){return(a._BinaryenEqInt32=a.asm.jf).apply(null,arguments)}
;a._BinaryenNeInt32=function(){return(a._BinaryenNeInt32=a.asm.kf).apply(null,arguments)}
;a._BinaryenLtSInt32=function(){return(a._BinaryenLtSInt32=a.asm.lf).apply(null,arguments)}
;
a._BinaryenLtUInt32=function(){return(a._BinaryenLtUInt32=a.asm.mf).apply(null,arguments)}
;a._BinaryenLeSInt32=function(){return(a._BinaryenLeSInt32=a.asm.nf).apply(null,arguments)}
;a._BinaryenLeUInt32=function(){return(a._BinaryenLeUInt32=a.asm.of).apply(null,arguments)}
;a._BinaryenGtSInt32=function(){return(a._BinaryenGtSInt32=a.asm.pf).apply(null,arguments)}
;a._BinaryenGtUInt32=function(){return(a._BinaryenGtUInt32=a.asm.qf).apply(null,arguments)}
;
a._BinaryenGeSInt32=function(){return(a._BinaryenGeSInt32=a.asm.rf).apply(null,arguments)}
;a._BinaryenGeUInt32=function(){return(a._BinaryenGeUInt32=a.asm.sf).apply(null,arguments)}
;a._BinaryenAddInt64=function(){return(a._BinaryenAddInt64=a.asm.tf).apply(null,arguments)}
;a._BinaryenSubInt64=function(){return(a._BinaryenSubInt64=a.asm.uf).apply(null,arguments)}
;a._BinaryenMulInt64=function(){return(a._BinaryenMulInt64=a.asm.vf).apply(null,arguments)}
;
a._BinaryenDivSInt64=function(){return(a._BinaryenDivSInt64=a.asm.wf).apply(null,arguments)}
;a._BinaryenDivUInt64=function(){return(a._BinaryenDivUInt64=a.asm.xf).apply(null,arguments)}
;a._BinaryenRemSInt64=function(){return(a._BinaryenRemSInt64=a.asm.yf).apply(null,arguments)}
;a._BinaryenRemUInt64=function(){return(a._BinaryenRemUInt64=a.asm.zf).apply(null,arguments)}
;a._BinaryenAndInt64=function(){return(a._BinaryenAndInt64=a.asm.Af).apply(null,arguments)}
;
a._BinaryenOrInt64=function(){return(a._BinaryenOrInt64=a.asm.Bf).apply(null,arguments)}
;a._BinaryenXorInt64=function(){return(a._BinaryenXorInt64=a.asm.Cf).apply(null,arguments)}
;a._BinaryenShlInt64=function(){return(a._BinaryenShlInt64=a.asm.Df).apply(null,arguments)}
;a._BinaryenShrUInt64=function(){return(a._BinaryenShrUInt64=a.asm.Ef).apply(null,arguments)}
;a._BinaryenShrSInt64=function(){return(a._BinaryenShrSInt64=a.asm.Ff).apply(null,arguments)}
;
a._BinaryenRotLInt64=function(){return(a._BinaryenRotLInt64=a.asm.Gf).apply(null,arguments)}
;a._BinaryenRotRInt64=function(){return(a._BinaryenRotRInt64=a.asm.Hf).apply(null,arguments)}
;a._BinaryenEqInt64=function(){return(a._BinaryenEqInt64=a.asm.If).apply(null,arguments)}
;a._BinaryenNeInt64=function(){return(a._BinaryenNeInt64=a.asm.Jf).apply(null,arguments)}
;a._BinaryenLtSInt64=function(){return(a._BinaryenLtSInt64=a.asm.Kf).apply(null,arguments)}
;
a._BinaryenLtUInt64=function(){return(a._BinaryenLtUInt64=a.asm.Lf).apply(null,arguments)}
;a._BinaryenLeSInt64=function(){return(a._BinaryenLeSInt64=a.asm.Mf).apply(null,arguments)}
;a._BinaryenLeUInt64=function(){return(a._BinaryenLeUInt64=a.asm.Nf).apply(null,arguments)}
;a._BinaryenGtSInt64=function(){return(a._BinaryenGtSInt64=a.asm.Of).apply(null,arguments)}
;a._BinaryenGtUInt64=function(){return(a._BinaryenGtUInt64=a.asm.Pf).apply(null,arguments)}
;
a._BinaryenGeSInt64=function(){return(a._BinaryenGeSInt64=a.asm.Qf).apply(null,arguments)}
;a._BinaryenGeUInt64=function(){return(a._BinaryenGeUInt64=a.asm.Rf).apply(null,arguments)}
;a._BinaryenAddFloat32=function(){return(a._BinaryenAddFloat32=a.asm.Sf).apply(null,arguments)}
;a._BinaryenSubFloat32=function(){return(a._BinaryenSubFloat32=a.asm.Tf).apply(null,arguments)}
;a._BinaryenMulFloat32=function(){return(a._BinaryenMulFloat32=a.asm.Uf).apply(null,arguments)}
;
a._BinaryenDivFloat32=function(){return(a._BinaryenDivFloat32=a.asm.Vf).apply(null,arguments)}
;a._BinaryenCopySignFloat32=function(){return(a._BinaryenCopySignFloat32=a.asm.Wf).apply(null,arguments)}
;a._BinaryenMinFloat32=function(){return(a._BinaryenMinFloat32=a.asm.Xf).apply(null,arguments)}
;a._BinaryenMaxFloat32=function(){return(a._BinaryenMaxFloat32=a.asm.Yf).apply(null,arguments)}
;a._BinaryenEqFloat32=function(){return(a._BinaryenEqFloat32=a.asm.Zf).apply(null,arguments)}
;
a._BinaryenNeFloat32=function(){return(a._BinaryenNeFloat32=a.asm._f).apply(null,arguments)}
;a._BinaryenLtFloat32=function(){return(a._BinaryenLtFloat32=a.asm.$f).apply(null,arguments)}
;a._BinaryenLeFloat32=function(){return(a._BinaryenLeFloat32=a.asm.ag).apply(null,arguments)}
;a._BinaryenGtFloat32=function(){return(a._BinaryenGtFloat32=a.asm.bg).apply(null,arguments)}
;a._BinaryenGeFloat32=function(){return(a._BinaryenGeFloat32=a.asm.cg).apply(null,arguments)}
;
a._BinaryenAddFloat64=function(){return(a._BinaryenAddFloat64=a.asm.dg).apply(null,arguments)}
;a._BinaryenSubFloat64=function(){return(a._BinaryenSubFloat64=a.asm.eg).apply(null,arguments)}
;a._BinaryenMulFloat64=function(){return(a._BinaryenMulFloat64=a.asm.fg).apply(null,arguments)}
;a._BinaryenDivFloat64=function(){return(a._BinaryenDivFloat64=a.asm.gg).apply(null,arguments)}
;a._BinaryenCopySignFloat64=function(){return(a._BinaryenCopySignFloat64=a.asm.hg).apply(null,arguments)}
;
a._BinaryenMinFloat64=function(){return(a._BinaryenMinFloat64=a.asm.ig).apply(null,arguments)}
;a._BinaryenMaxFloat64=function(){return(a._BinaryenMaxFloat64=a.asm.jg).apply(null,arguments)}
;a._BinaryenEqFloat64=function(){return(a._BinaryenEqFloat64=a.asm.kg).apply(null,arguments)}
;a._BinaryenNeFloat64=function(){return(a._BinaryenNeFloat64=a.asm.lg).apply(null,arguments)}
;a._BinaryenLtFloat64=function(){return(a._BinaryenLtFloat64=a.asm.mg).apply(null,arguments)}
;
a._BinaryenLeFloat64=function(){return(a._BinaryenLeFloat64=a.asm.ng).apply(null,arguments)}
;a._BinaryenGtFloat64=function(){return(a._BinaryenGtFloat64=a.asm.og).apply(null,arguments)}
;a._BinaryenGeFloat64=function(){return(a._BinaryenGeFloat64=a.asm.pg).apply(null,arguments)}
;a._BinaryenAtomicRMWAdd=function(){return(a._BinaryenAtomicRMWAdd=a.asm.qg).apply(null,arguments)}
;a._BinaryenAtomicRMWSub=function(){return(a._BinaryenAtomicRMWSub=a.asm.rg).apply(null,arguments)}
;
a._BinaryenAtomicRMWAnd=function(){return(a._BinaryenAtomicRMWAnd=a.asm.sg).apply(null,arguments)}
;a._BinaryenAtomicRMWOr=function(){return(a._BinaryenAtomicRMWOr=a.asm.tg).apply(null,arguments)}
;a._BinaryenAtomicRMWXor=function(){return(a._BinaryenAtomicRMWXor=a.asm.ug).apply(null,arguments)}
;a._BinaryenAtomicRMWXchg=function(){return(a._BinaryenAtomicRMWXchg=a.asm.vg).apply(null,arguments)}
;
a._BinaryenTruncSatSFloat32ToInt32=function(){return(a._BinaryenTruncSatSFloat32ToInt32=a.asm.wg).apply(null,arguments)}
;a._BinaryenTruncSatSFloat32ToInt64=function(){return(a._BinaryenTruncSatSFloat32ToInt64=a.asm.xg).apply(null,arguments)}
;a._BinaryenTruncSatUFloat32ToInt32=function(){return(a._BinaryenTruncSatUFloat32ToInt32=a.asm.yg).apply(null,arguments)}
;a._BinaryenTruncSatUFloat32ToInt64=function(){return(a._BinaryenTruncSatUFloat32ToInt64=a.asm.zg).apply(null,arguments)}
;
a._BinaryenTruncSatSFloat64ToInt32=function(){return(a._BinaryenTruncSatSFloat64ToInt32=a.asm.Ag).apply(null,arguments)}
;a._BinaryenTruncSatSFloat64ToInt64=function(){return(a._BinaryenTruncSatSFloat64ToInt64=a.asm.Bg).apply(null,arguments)}
;a._BinaryenTruncSatUFloat64ToInt32=function(){return(a._BinaryenTruncSatUFloat64ToInt32=a.asm.Cg).apply(null,arguments)}
;a._BinaryenTruncSatUFloat64ToInt64=function(){return(a._BinaryenTruncSatUFloat64ToInt64=a.asm.Dg).apply(null,arguments)}
;
a._BinaryenSplatVecI8x16=function(){return(a._BinaryenSplatVecI8x16=a.asm.Eg).apply(null,arguments)}
;a._BinaryenExtractLaneSVecI8x16=function(){return(a._BinaryenExtractLaneSVecI8x16=a.asm.Fg).apply(null,arguments)}
;a._BinaryenExtractLaneUVecI8x16=function(){return(a._BinaryenExtractLaneUVecI8x16=a.asm.Gg).apply(null,arguments)}
;a._BinaryenReplaceLaneVecI8x16=function(){return(a._BinaryenReplaceLaneVecI8x16=a.asm.Hg).apply(null,arguments)}
;
a._BinaryenSplatVecI16x8=function(){return(a._BinaryenSplatVecI16x8=a.asm.Ig).apply(null,arguments)}
;a._BinaryenExtractLaneSVecI16x8=function(){return(a._BinaryenExtractLaneSVecI16x8=a.asm.Jg).apply(null,arguments)}
;a._BinaryenExtractLaneUVecI16x8=function(){return(a._BinaryenExtractLaneUVecI16x8=a.asm.Kg).apply(null,arguments)}
;a._BinaryenReplaceLaneVecI16x8=function(){return(a._BinaryenReplaceLaneVecI16x8=a.asm.Lg).apply(null,arguments)}
;
a._BinaryenSplatVecI32x4=function(){return(a._BinaryenSplatVecI32x4=a.asm.Mg).apply(null,arguments)}
;a._BinaryenExtractLaneVecI32x4=function(){return(a._BinaryenExtractLaneVecI32x4=a.asm.Ng).apply(null,arguments)}
;a._BinaryenReplaceLaneVecI32x4=function(){return(a._BinaryenReplaceLaneVecI32x4=a.asm.Og).apply(null,arguments)}
;a._BinaryenSplatVecI64x2=function(){return(a._BinaryenSplatVecI64x2=a.asm.Pg).apply(null,arguments)}
;
a._BinaryenExtractLaneVecI64x2=function(){return(a._BinaryenExtractLaneVecI64x2=a.asm.Qg).apply(null,arguments)}
;a._BinaryenReplaceLaneVecI64x2=function(){return(a._BinaryenReplaceLaneVecI64x2=a.asm.Rg).apply(null,arguments)}
;a._BinaryenSplatVecF32x4=function(){return(a._BinaryenSplatVecF32x4=a.asm.Sg).apply(null,arguments)}
;a._BinaryenExtractLaneVecF32x4=function(){return(a._BinaryenExtractLaneVecF32x4=a.asm.Tg).apply(null,arguments)}
;
a._BinaryenReplaceLaneVecF32x4=function(){return(a._BinaryenReplaceLaneVecF32x4=a.asm.Ug).apply(null,arguments)}
;a._BinaryenSplatVecF64x2=function(){return(a._BinaryenSplatVecF64x2=a.asm.Vg).apply(null,arguments)}
;a._BinaryenExtractLaneVecF64x2=function(){return(a._BinaryenExtractLaneVecF64x2=a.asm.Wg).apply(null,arguments)}
;a._BinaryenReplaceLaneVecF64x2=function(){return(a._BinaryenReplaceLaneVecF64x2=a.asm.Xg).apply(null,arguments)}
;
a._BinaryenEqVecI8x16=function(){return(a._BinaryenEqVecI8x16=a.asm.Yg).apply(null,arguments)}
;a._BinaryenNeVecI8x16=function(){return(a._BinaryenNeVecI8x16=a.asm.Zg).apply(null,arguments)}
;a._BinaryenLtSVecI8x16=function(){return(a._BinaryenLtSVecI8x16=a.asm._g).apply(null,arguments)}
;a._BinaryenLtUVecI8x16=function(){return(a._BinaryenLtUVecI8x16=a.asm.$g).apply(null,arguments)}
;a._BinaryenGtSVecI8x16=function(){return(a._BinaryenGtSVecI8x16=a.asm.ah).apply(null,arguments)}
;
a._BinaryenGtUVecI8x16=function(){return(a._BinaryenGtUVecI8x16=a.asm.bh).apply(null,arguments)}
;a._BinaryenLeSVecI8x16=function(){return(a._BinaryenLeSVecI8x16=a.asm.ch).apply(null,arguments)}
;a._BinaryenLeUVecI8x16=function(){return(a._BinaryenLeUVecI8x16=a.asm.dh).apply(null,arguments)}
;a._BinaryenGeSVecI8x16=function(){return(a._BinaryenGeSVecI8x16=a.asm.eh).apply(null,arguments)}
;a._BinaryenGeUVecI8x16=function(){return(a._BinaryenGeUVecI8x16=a.asm.fh).apply(null,arguments)}
;
a._BinaryenEqVecI16x8=function(){return(a._BinaryenEqVecI16x8=a.asm.gh).apply(null,arguments)}
;a._BinaryenNeVecI16x8=function(){return(a._BinaryenNeVecI16x8=a.asm.hh).apply(null,arguments)}
;a._BinaryenLtSVecI16x8=function(){return(a._BinaryenLtSVecI16x8=a.asm.ih).apply(null,arguments)}
;a._BinaryenLtUVecI16x8=function(){return(a._BinaryenLtUVecI16x8=a.asm.jh).apply(null,arguments)}
;a._BinaryenGtSVecI16x8=function(){return(a._BinaryenGtSVecI16x8=a.asm.kh).apply(null,arguments)}
;
a._BinaryenGtUVecI16x8=function(){return(a._BinaryenGtUVecI16x8=a.asm.lh).apply(null,arguments)}
;a._BinaryenLeSVecI16x8=function(){return(a._BinaryenLeSVecI16x8=a.asm.mh).apply(null,arguments)}
;a._BinaryenLeUVecI16x8=function(){return(a._BinaryenLeUVecI16x8=a.asm.nh).apply(null,arguments)}
;a._BinaryenGeSVecI16x8=function(){return(a._BinaryenGeSVecI16x8=a.asm.oh).apply(null,arguments)}
;a._BinaryenGeUVecI16x8=function(){return(a._BinaryenGeUVecI16x8=a.asm.ph).apply(null,arguments)}
;
a._BinaryenEqVecI32x4=function(){return(a._BinaryenEqVecI32x4=a.asm.qh).apply(null,arguments)}
;a._BinaryenNeVecI32x4=function(){return(a._BinaryenNeVecI32x4=a.asm.rh).apply(null,arguments)}
;a._BinaryenLtSVecI32x4=function(){return(a._BinaryenLtSVecI32x4=a.asm.sh).apply(null,arguments)}
;a._BinaryenLtUVecI32x4=function(){return(a._BinaryenLtUVecI32x4=a.asm.th).apply(null,arguments)}
;a._BinaryenGtSVecI32x4=function(){return(a._BinaryenGtSVecI32x4=a.asm.uh).apply(null,arguments)}
;
a._BinaryenGtUVecI32x4=function(){return(a._BinaryenGtUVecI32x4=a.asm.vh).apply(null,arguments)}
;a._BinaryenLeSVecI32x4=function(){return(a._BinaryenLeSVecI32x4=a.asm.wh).apply(null,arguments)}
;a._BinaryenLeUVecI32x4=function(){return(a._BinaryenLeUVecI32x4=a.asm.xh).apply(null,arguments)}
;a._BinaryenGeSVecI32x4=function(){return(a._BinaryenGeSVecI32x4=a.asm.yh).apply(null,arguments)}
;a._BinaryenGeUVecI32x4=function(){return(a._BinaryenGeUVecI32x4=a.asm.zh).apply(null,arguments)}
;
a._BinaryenEqVecI64x2=function(){return(a._BinaryenEqVecI64x2=a.asm.Ah).apply(null,arguments)}
;a._BinaryenNeVecI64x2=function(){return(a._BinaryenNeVecI64x2=a.asm.Bh).apply(null,arguments)}
;a._BinaryenLtSVecI64x2=function(){return(a._BinaryenLtSVecI64x2=a.asm.Ch).apply(null,arguments)}
;a._BinaryenGtSVecI64x2=function(){return(a._BinaryenGtSVecI64x2=a.asm.Dh).apply(null,arguments)}
;a._BinaryenLeSVecI64x2=function(){return(a._BinaryenLeSVecI64x2=a.asm.Eh).apply(null,arguments)}
;
a._BinaryenGeSVecI64x2=function(){return(a._BinaryenGeSVecI64x2=a.asm.Fh).apply(null,arguments)}
;a._BinaryenEqVecF32x4=function(){return(a._BinaryenEqVecF32x4=a.asm.Gh).apply(null,arguments)}
;a._BinaryenNeVecF32x4=function(){return(a._BinaryenNeVecF32x4=a.asm.Hh).apply(null,arguments)}
;a._BinaryenLtVecF32x4=function(){return(a._BinaryenLtVecF32x4=a.asm.Ih).apply(null,arguments)}
;a._BinaryenGtVecF32x4=function(){return(a._BinaryenGtVecF32x4=a.asm.Jh).apply(null,arguments)}
;
a._BinaryenLeVecF32x4=function(){return(a._BinaryenLeVecF32x4=a.asm.Kh).apply(null,arguments)}
;a._BinaryenGeVecF32x4=function(){return(a._BinaryenGeVecF32x4=a.asm.Lh).apply(null,arguments)}
;a._BinaryenEqVecF64x2=function(){return(a._BinaryenEqVecF64x2=a.asm.Mh).apply(null,arguments)}
;a._BinaryenNeVecF64x2=function(){return(a._BinaryenNeVecF64x2=a.asm.Nh).apply(null,arguments)}
;a._BinaryenLtVecF64x2=function(){return(a._BinaryenLtVecF64x2=a.asm.Oh).apply(null,arguments)}
;
a._BinaryenGtVecF64x2=function(){return(a._BinaryenGtVecF64x2=a.asm.Ph).apply(null,arguments)}
;a._BinaryenLeVecF64x2=function(){return(a._BinaryenLeVecF64x2=a.asm.Qh).apply(null,arguments)}
;a._BinaryenGeVecF64x2=function(){return(a._BinaryenGeVecF64x2=a.asm.Rh).apply(null,arguments)}
;a._BinaryenNotVec128=function(){return(a._BinaryenNotVec128=a.asm.Sh).apply(null,arguments)}
;a._BinaryenAndVec128=function(){return(a._BinaryenAndVec128=a.asm.Th).apply(null,arguments)}
;
a._BinaryenOrVec128=function(){return(a._BinaryenOrVec128=a.asm.Uh).apply(null,arguments)}
;a._BinaryenXorVec128=function(){return(a._BinaryenXorVec128=a.asm.Vh).apply(null,arguments)}
;a._BinaryenAndNotVec128=function(){return(a._BinaryenAndNotVec128=a.asm.Wh).apply(null,arguments)}
;a._BinaryenBitselectVec128=function(){return(a._BinaryenBitselectVec128=a.asm.Xh).apply(null,arguments)}
;a._BinaryenAnyTrueVec128=function(){return(a._BinaryenAnyTrueVec128=a.asm.Yh).apply(null,arguments)}
;
a._BinaryenAbsVecI8x16=function(){return(a._BinaryenAbsVecI8x16=a.asm.Zh).apply(null,arguments)}
;a._BinaryenNegVecI8x16=function(){return(a._BinaryenNegVecI8x16=a.asm._h).apply(null,arguments)}
;a._BinaryenAllTrueVecI8x16=function(){return(a._BinaryenAllTrueVecI8x16=a.asm.$h).apply(null,arguments)}
;a._BinaryenBitmaskVecI8x16=function(){return(a._BinaryenBitmaskVecI8x16=a.asm.ai).apply(null,arguments)}
;a._BinaryenPopcntVecI8x16=function(){return(a._BinaryenPopcntVecI8x16=a.asm.bi).apply(null,arguments)}
;
a._BinaryenShlVecI8x16=function(){return(a._BinaryenShlVecI8x16=a.asm.ci).apply(null,arguments)}
;a._BinaryenShrSVecI8x16=function(){return(a._BinaryenShrSVecI8x16=a.asm.di).apply(null,arguments)}
;a._BinaryenShrUVecI8x16=function(){return(a._BinaryenShrUVecI8x16=a.asm.ei).apply(null,arguments)}
;a._BinaryenAddVecI8x16=function(){return(a._BinaryenAddVecI8x16=a.asm.fi).apply(null,arguments)}
;a._BinaryenAddSatSVecI8x16=function(){return(a._BinaryenAddSatSVecI8x16=a.asm.gi).apply(null,arguments)}
;
a._BinaryenAddSatUVecI8x16=function(){return(a._BinaryenAddSatUVecI8x16=a.asm.hi).apply(null,arguments)}
;a._BinaryenSubVecI8x16=function(){return(a._BinaryenSubVecI8x16=a.asm.ii).apply(null,arguments)}
;a._BinaryenSubSatSVecI8x16=function(){return(a._BinaryenSubSatSVecI8x16=a.asm.ji).apply(null,arguments)}
;a._BinaryenSubSatUVecI8x16=function(){return(a._BinaryenSubSatUVecI8x16=a.asm.ki).apply(null,arguments)}
;a._BinaryenMinSVecI8x16=function(){return(a._BinaryenMinSVecI8x16=a.asm.li).apply(null,arguments)}
;
a._BinaryenMinUVecI8x16=function(){return(a._BinaryenMinUVecI8x16=a.asm.mi).apply(null,arguments)}
;a._BinaryenMaxSVecI8x16=function(){return(a._BinaryenMaxSVecI8x16=a.asm.ni).apply(null,arguments)}
;a._BinaryenMaxUVecI8x16=function(){return(a._BinaryenMaxUVecI8x16=a.asm.oi).apply(null,arguments)}
;a._BinaryenAvgrUVecI8x16=function(){return(a._BinaryenAvgrUVecI8x16=a.asm.pi).apply(null,arguments)}
;a._BinaryenAbsVecI16x8=function(){return(a._BinaryenAbsVecI16x8=a.asm.qi).apply(null,arguments)}
;
a._BinaryenNegVecI16x8=function(){return(a._BinaryenNegVecI16x8=a.asm.ri).apply(null,arguments)}
;a._BinaryenAllTrueVecI16x8=function(){return(a._BinaryenAllTrueVecI16x8=a.asm.si).apply(null,arguments)}
;a._BinaryenBitmaskVecI16x8=function(){return(a._BinaryenBitmaskVecI16x8=a.asm.ti).apply(null,arguments)}
;a._BinaryenShlVecI16x8=function(){return(a._BinaryenShlVecI16x8=a.asm.ui).apply(null,arguments)}
;a._BinaryenShrSVecI16x8=function(){return(a._BinaryenShrSVecI16x8=a.asm.vi).apply(null,arguments)}
;
a._BinaryenShrUVecI16x8=function(){return(a._BinaryenShrUVecI16x8=a.asm.wi).apply(null,arguments)}
;a._BinaryenAddVecI16x8=function(){return(a._BinaryenAddVecI16x8=a.asm.xi).apply(null,arguments)}
;a._BinaryenAddSatSVecI16x8=function(){return(a._BinaryenAddSatSVecI16x8=a.asm.yi).apply(null,arguments)}
;a._BinaryenAddSatUVecI16x8=function(){return(a._BinaryenAddSatUVecI16x8=a.asm.zi).apply(null,arguments)}
;a._BinaryenSubVecI16x8=function(){return(a._BinaryenSubVecI16x8=a.asm.Ai).apply(null,arguments)}
;
a._BinaryenSubSatSVecI16x8=function(){return(a._BinaryenSubSatSVecI16x8=a.asm.Bi).apply(null,arguments)}
;a._BinaryenSubSatUVecI16x8=function(){return(a._BinaryenSubSatUVecI16x8=a.asm.Ci).apply(null,arguments)}
;a._BinaryenMulVecI16x8=function(){return(a._BinaryenMulVecI16x8=a.asm.Di).apply(null,arguments)}
;a._BinaryenMinSVecI16x8=function(){return(a._BinaryenMinSVecI16x8=a.asm.Ei).apply(null,arguments)}
;a._BinaryenMinUVecI16x8=function(){return(a._BinaryenMinUVecI16x8=a.asm.Fi).apply(null,arguments)}
;
a._BinaryenMaxSVecI16x8=function(){return(a._BinaryenMaxSVecI16x8=a.asm.Gi).apply(null,arguments)}
;a._BinaryenMaxUVecI16x8=function(){return(a._BinaryenMaxUVecI16x8=a.asm.Hi).apply(null,arguments)}
;a._BinaryenAvgrUVecI16x8=function(){return(a._BinaryenAvgrUVecI16x8=a.asm.Ii).apply(null,arguments)}
;a._BinaryenQ15MulrSatSVecI16x8=function(){return(a._BinaryenQ15MulrSatSVecI16x8=a.asm.Ji).apply(null,arguments)}
;
a._BinaryenExtMulLowSVecI16x8=function(){return(a._BinaryenExtMulLowSVecI16x8=a.asm.Ki).apply(null,arguments)}
;a._BinaryenExtMulHighSVecI16x8=function(){return(a._BinaryenExtMulHighSVecI16x8=a.asm.Li).apply(null,arguments)}
;a._BinaryenExtMulLowUVecI16x8=function(){return(a._BinaryenExtMulLowUVecI16x8=a.asm.Mi).apply(null,arguments)}
;a._BinaryenExtMulHighUVecI16x8=function(){return(a._BinaryenExtMulHighUVecI16x8=a.asm.Ni).apply(null,arguments)}
;
a._BinaryenAbsVecI32x4=function(){return(a._BinaryenAbsVecI32x4=a.asm.Oi).apply(null,arguments)}
;a._BinaryenNegVecI32x4=function(){return(a._BinaryenNegVecI32x4=a.asm.Pi).apply(null,arguments)}
;a._BinaryenAllTrueVecI32x4=function(){return(a._BinaryenAllTrueVecI32x4=a.asm.Qi).apply(null,arguments)}
;a._BinaryenBitmaskVecI32x4=function(){return(a._BinaryenBitmaskVecI32x4=a.asm.Ri).apply(null,arguments)}
;a._BinaryenShlVecI32x4=function(){return(a._BinaryenShlVecI32x4=a.asm.Si).apply(null,arguments)}
;
a._BinaryenShrSVecI32x4=function(){return(a._BinaryenShrSVecI32x4=a.asm.Ti).apply(null,arguments)}
;a._BinaryenShrUVecI32x4=function(){return(a._BinaryenShrUVecI32x4=a.asm.Ui).apply(null,arguments)}
;a._BinaryenAddVecI32x4=function(){return(a._BinaryenAddVecI32x4=a.asm.Vi).apply(null,arguments)}
;a._BinaryenSubVecI32x4=function(){return(a._BinaryenSubVecI32x4=a.asm.Wi).apply(null,arguments)}
;a._BinaryenMulVecI32x4=function(){return(a._BinaryenMulVecI32x4=a.asm.Xi).apply(null,arguments)}
;
a._BinaryenMinSVecI32x4=function(){return(a._BinaryenMinSVecI32x4=a.asm.Yi).apply(null,arguments)}
;a._BinaryenMinUVecI32x4=function(){return(a._BinaryenMinUVecI32x4=a.asm.Zi).apply(null,arguments)}
;a._BinaryenMaxSVecI32x4=function(){return(a._BinaryenMaxSVecI32x4=a.asm._i).apply(null,arguments)}
;a._BinaryenMaxUVecI32x4=function(){return(a._BinaryenMaxUVecI32x4=a.asm.$i).apply(null,arguments)}
;
a._BinaryenDotSVecI16x8ToVecI32x4=function(){return(a._BinaryenDotSVecI16x8ToVecI32x4=a.asm.aj).apply(null,arguments)}
;a._BinaryenExtMulLowSVecI32x4=function(){return(a._BinaryenExtMulLowSVecI32x4=a.asm.bj).apply(null,arguments)}
;a._BinaryenExtMulHighSVecI32x4=function(){return(a._BinaryenExtMulHighSVecI32x4=a.asm.cj).apply(null,arguments)}
;a._BinaryenExtMulLowUVecI32x4=function(){return(a._BinaryenExtMulLowUVecI32x4=a.asm.dj).apply(null,arguments)}
;
a._BinaryenExtMulHighUVecI32x4=function(){return(a._BinaryenExtMulHighUVecI32x4=a.asm.ej).apply(null,arguments)}
;a._BinaryenAbsVecI64x2=function(){return(a._BinaryenAbsVecI64x2=a.asm.fj).apply(null,arguments)}
;a._BinaryenNegVecI64x2=function(){return(a._BinaryenNegVecI64x2=a.asm.gj).apply(null,arguments)}
;a._BinaryenAllTrueVecI64x2=function(){return(a._BinaryenAllTrueVecI64x2=a.asm.hj).apply(null,arguments)}
;
a._BinaryenBitmaskVecI64x2=function(){return(a._BinaryenBitmaskVecI64x2=a.asm.ij).apply(null,arguments)}
;a._BinaryenShlVecI64x2=function(){return(a._BinaryenShlVecI64x2=a.asm.jj).apply(null,arguments)}
;a._BinaryenShrSVecI64x2=function(){return(a._BinaryenShrSVecI64x2=a.asm.kj).apply(null,arguments)}
;a._BinaryenShrUVecI64x2=function(){return(a._BinaryenShrUVecI64x2=a.asm.lj).apply(null,arguments)}
;a._BinaryenAddVecI64x2=function(){return(a._BinaryenAddVecI64x2=a.asm.mj).apply(null,arguments)}
;
a._BinaryenSubVecI64x2=function(){return(a._BinaryenSubVecI64x2=a.asm.nj).apply(null,arguments)}
;a._BinaryenMulVecI64x2=function(){return(a._BinaryenMulVecI64x2=a.asm.oj).apply(null,arguments)}
;a._BinaryenExtMulLowSVecI64x2=function(){return(a._BinaryenExtMulLowSVecI64x2=a.asm.pj).apply(null,arguments)}
;a._BinaryenExtMulHighSVecI64x2=function(){return(a._BinaryenExtMulHighSVecI64x2=a.asm.qj).apply(null,arguments)}
;
a._BinaryenExtMulLowUVecI64x2=function(){return(a._BinaryenExtMulLowUVecI64x2=a.asm.rj).apply(null,arguments)}
;a._BinaryenExtMulHighUVecI64x2=function(){return(a._BinaryenExtMulHighUVecI64x2=a.asm.sj).apply(null,arguments)}
;a._BinaryenAbsVecF32x4=function(){return(a._BinaryenAbsVecF32x4=a.asm.tj).apply(null,arguments)}
;a._BinaryenNegVecF32x4=function(){return(a._BinaryenNegVecF32x4=a.asm.uj).apply(null,arguments)}
;
a._BinaryenSqrtVecF32x4=function(){return(a._BinaryenSqrtVecF32x4=a.asm.vj).apply(null,arguments)}
;a._BinaryenAddVecF32x4=function(){return(a._BinaryenAddVecF32x4=a.asm.wj).apply(null,arguments)}
;a._BinaryenSubVecF32x4=function(){return(a._BinaryenSubVecF32x4=a.asm.xj).apply(null,arguments)}
;a._BinaryenMulVecF32x4=function(){return(a._BinaryenMulVecF32x4=a.asm.yj).apply(null,arguments)}
;a._BinaryenDivVecF32x4=function(){return(a._BinaryenDivVecF32x4=a.asm.zj).apply(null,arguments)}
;
a._BinaryenMinVecF32x4=function(){return(a._BinaryenMinVecF32x4=a.asm.Aj).apply(null,arguments)}
;a._BinaryenMaxVecF32x4=function(){return(a._BinaryenMaxVecF32x4=a.asm.Bj).apply(null,arguments)}
;a._BinaryenPMinVecF32x4=function(){return(a._BinaryenPMinVecF32x4=a.asm.Cj).apply(null,arguments)}
;a._BinaryenCeilVecF32x4=function(){return(a._BinaryenCeilVecF32x4=a.asm.Dj).apply(null,arguments)}
;a._BinaryenFloorVecF32x4=function(){return(a._BinaryenFloorVecF32x4=a.asm.Ej).apply(null,arguments)}
;
a._BinaryenTruncVecF32x4=function(){return(a._BinaryenTruncVecF32x4=a.asm.Fj).apply(null,arguments)}
;a._BinaryenNearestVecF32x4=function(){return(a._BinaryenNearestVecF32x4=a.asm.Gj).apply(null,arguments)}
;a._BinaryenPMaxVecF32x4=function(){return(a._BinaryenPMaxVecF32x4=a.asm.Hj).apply(null,arguments)}
;a._BinaryenAbsVecF64x2=function(){return(a._BinaryenAbsVecF64x2=a.asm.Ij).apply(null,arguments)}
;a._BinaryenNegVecF64x2=function(){return(a._BinaryenNegVecF64x2=a.asm.Jj).apply(null,arguments)}
;
a._BinaryenSqrtVecF64x2=function(){return(a._BinaryenSqrtVecF64x2=a.asm.Kj).apply(null,arguments)}
;a._BinaryenAddVecF64x2=function(){return(a._BinaryenAddVecF64x2=a.asm.Lj).apply(null,arguments)}
;a._BinaryenSubVecF64x2=function(){return(a._BinaryenSubVecF64x2=a.asm.Mj).apply(null,arguments)}
;a._BinaryenMulVecF64x2=function(){return(a._BinaryenMulVecF64x2=a.asm.Nj).apply(null,arguments)}
;a._BinaryenDivVecF64x2=function(){return(a._BinaryenDivVecF64x2=a.asm.Oj).apply(null,arguments)}
;
a._BinaryenMinVecF64x2=function(){return(a._BinaryenMinVecF64x2=a.asm.Pj).apply(null,arguments)}
;a._BinaryenMaxVecF64x2=function(){return(a._BinaryenMaxVecF64x2=a.asm.Qj).apply(null,arguments)}
;a._BinaryenPMinVecF64x2=function(){return(a._BinaryenPMinVecF64x2=a.asm.Rj).apply(null,arguments)}
;a._BinaryenPMaxVecF64x2=function(){return(a._BinaryenPMaxVecF64x2=a.asm.Sj).apply(null,arguments)}
;a._BinaryenCeilVecF64x2=function(){return(a._BinaryenCeilVecF64x2=a.asm.Tj).apply(null,arguments)}
;
a._BinaryenFloorVecF64x2=function(){return(a._BinaryenFloorVecF64x2=a.asm.Uj).apply(null,arguments)}
;a._BinaryenTruncVecF64x2=function(){return(a._BinaryenTruncVecF64x2=a.asm.Vj).apply(null,arguments)}
;a._BinaryenNearestVecF64x2=function(){return(a._BinaryenNearestVecF64x2=a.asm.Wj).apply(null,arguments)}
;a._BinaryenExtAddPairwiseSVecI8x16ToI16x8=function(){return(a._BinaryenExtAddPairwiseSVecI8x16ToI16x8=a.asm.Xj).apply(null,arguments)}
;
a._BinaryenExtAddPairwiseUVecI8x16ToI16x8=function(){return(a._BinaryenExtAddPairwiseUVecI8x16ToI16x8=a.asm.Yj).apply(null,arguments)}
;a._BinaryenExtAddPairwiseSVecI16x8ToI32x4=function(){return(a._BinaryenExtAddPairwiseSVecI16x8ToI32x4=a.asm.Zj).apply(null,arguments)}
;a._BinaryenExtAddPairwiseUVecI16x8ToI32x4=function(){return(a._BinaryenExtAddPairwiseUVecI16x8ToI32x4=a.asm._j).apply(null,arguments)}
;
a._BinaryenTruncSatSVecF32x4ToVecI32x4=function(){return(a._BinaryenTruncSatSVecF32x4ToVecI32x4=a.asm.$j).apply(null,arguments)}
;a._BinaryenTruncSatUVecF32x4ToVecI32x4=function(){return(a._BinaryenTruncSatUVecF32x4ToVecI32x4=a.asm.ak).apply(null,arguments)}
;a._BinaryenConvertSVecI32x4ToVecF32x4=function(){return(a._BinaryenConvertSVecI32x4ToVecF32x4=a.asm.bk).apply(null,arguments)}
;a._BinaryenConvertUVecI32x4ToVecF32x4=function(){return(a._BinaryenConvertUVecI32x4ToVecF32x4=a.asm.ck).apply(null,arguments)}
;
a._BinaryenLoad8SplatVec128=function(){return(a._BinaryenLoad8SplatVec128=a.asm.dk).apply(null,arguments)}
;a._BinaryenLoad16SplatVec128=function(){return(a._BinaryenLoad16SplatVec128=a.asm.ek).apply(null,arguments)}
;a._BinaryenLoad32SplatVec128=function(){return(a._BinaryenLoad32SplatVec128=a.asm.fk).apply(null,arguments)}
;a._BinaryenLoad64SplatVec128=function(){return(a._BinaryenLoad64SplatVec128=a.asm.gk).apply(null,arguments)}
;
a._BinaryenLoad8x8SVec128=function(){return(a._BinaryenLoad8x8SVec128=a.asm.hk).apply(null,arguments)}
;a._BinaryenLoad8x8UVec128=function(){return(a._BinaryenLoad8x8UVec128=a.asm.ik).apply(null,arguments)}
;a._BinaryenLoad16x4SVec128=function(){return(a._BinaryenLoad16x4SVec128=a.asm.jk).apply(null,arguments)}
;a._BinaryenLoad16x4UVec128=function(){return(a._BinaryenLoad16x4UVec128=a.asm.kk).apply(null,arguments)}
;
a._BinaryenLoad32x2SVec128=function(){return(a._BinaryenLoad32x2SVec128=a.asm.lk).apply(null,arguments)}
;a._BinaryenLoad32x2UVec128=function(){return(a._BinaryenLoad32x2UVec128=a.asm.mk).apply(null,arguments)}
;a._BinaryenLoad32ZeroVec128=function(){return(a._BinaryenLoad32ZeroVec128=a.asm.nk).apply(null,arguments)}
;a._BinaryenLoad64ZeroVec128=function(){return(a._BinaryenLoad64ZeroVec128=a.asm.ok).apply(null,arguments)}
;
a._BinaryenLoad8LaneVec128=function(){return(a._BinaryenLoad8LaneVec128=a.asm.pk).apply(null,arguments)}
;a._BinaryenLoad16LaneVec128=function(){return(a._BinaryenLoad16LaneVec128=a.asm.qk).apply(null,arguments)}
;a._BinaryenLoad32LaneVec128=function(){return(a._BinaryenLoad32LaneVec128=a.asm.rk).apply(null,arguments)}
;a._BinaryenLoad64LaneVec128=function(){return(a._BinaryenLoad64LaneVec128=a.asm.sk).apply(null,arguments)}
;
a._BinaryenStore8LaneVec128=function(){return(a._BinaryenStore8LaneVec128=a.asm.tk).apply(null,arguments)}
;a._BinaryenStore16LaneVec128=function(){return(a._BinaryenStore16LaneVec128=a.asm.uk).apply(null,arguments)}
;a._BinaryenStore32LaneVec128=function(){return(a._BinaryenStore32LaneVec128=a.asm.vk).apply(null,arguments)}
;a._BinaryenStore64LaneVec128=function(){return(a._BinaryenStore64LaneVec128=a.asm.wk).apply(null,arguments)}
;
a._BinaryenNarrowSVecI16x8ToVecI8x16=function(){return(a._BinaryenNarrowSVecI16x8ToVecI8x16=a.asm.xk).apply(null,arguments)}
;a._BinaryenNarrowUVecI16x8ToVecI8x16=function(){return(a._BinaryenNarrowUVecI16x8ToVecI8x16=a.asm.yk).apply(null,arguments)}
;a._BinaryenNarrowSVecI32x4ToVecI16x8=function(){return(a._BinaryenNarrowSVecI32x4ToVecI16x8=a.asm.zk).apply(null,arguments)}
;a._BinaryenNarrowUVecI32x4ToVecI16x8=function(){return(a._BinaryenNarrowUVecI32x4ToVecI16x8=a.asm.Ak).apply(null,arguments)}
;
a._BinaryenExtendLowSVecI8x16ToVecI16x8=function(){return(a._BinaryenExtendLowSVecI8x16ToVecI16x8=a.asm.Bk).apply(null,arguments)}
;a._BinaryenExtendHighSVecI8x16ToVecI16x8=function(){return(a._BinaryenExtendHighSVecI8x16ToVecI16x8=a.asm.Ck).apply(null,arguments)}
;a._BinaryenExtendLowUVecI8x16ToVecI16x8=function(){return(a._BinaryenExtendLowUVecI8x16ToVecI16x8=a.asm.Dk).apply(null,arguments)}
;
a._BinaryenExtendHighUVecI8x16ToVecI16x8=function(){return(a._BinaryenExtendHighUVecI8x16ToVecI16x8=a.asm.Ek).apply(null,arguments)}
;a._BinaryenExtendLowSVecI16x8ToVecI32x4=function(){return(a._BinaryenExtendLowSVecI16x8ToVecI32x4=a.asm.Fk).apply(null,arguments)}
;a._BinaryenExtendHighSVecI16x8ToVecI32x4=function(){return(a._BinaryenExtendHighSVecI16x8ToVecI32x4=a.asm.Gk).apply(null,arguments)}
;
a._BinaryenExtendLowUVecI16x8ToVecI32x4=function(){return(a._BinaryenExtendLowUVecI16x8ToVecI32x4=a.asm.Hk).apply(null,arguments)}
;a._BinaryenExtendHighUVecI16x8ToVecI32x4=function(){return(a._BinaryenExtendHighUVecI16x8ToVecI32x4=a.asm.Ik).apply(null,arguments)}
;a._BinaryenExtendLowSVecI32x4ToVecI64x2=function(){return(a._BinaryenExtendLowSVecI32x4ToVecI64x2=a.asm.Jk).apply(null,arguments)}
;
a._BinaryenExtendHighSVecI32x4ToVecI64x2=function(){return(a._BinaryenExtendHighSVecI32x4ToVecI64x2=a.asm.Kk).apply(null,arguments)}
;a._BinaryenExtendLowUVecI32x4ToVecI64x2=function(){return(a._BinaryenExtendLowUVecI32x4ToVecI64x2=a.asm.Lk).apply(null,arguments)}
;a._BinaryenExtendHighUVecI32x4ToVecI64x2=function(){return(a._BinaryenExtendHighUVecI32x4ToVecI64x2=a.asm.Mk).apply(null,arguments)}
;
a._BinaryenConvertLowSVecI32x4ToVecF64x2=function(){return(a._BinaryenConvertLowSVecI32x4ToVecF64x2=a.asm.Nk).apply(null,arguments)}
;a._BinaryenConvertLowUVecI32x4ToVecF64x2=function(){return(a._BinaryenConvertLowUVecI32x4ToVecF64x2=a.asm.Ok).apply(null,arguments)}
;a._BinaryenTruncSatZeroSVecF64x2ToVecI32x4=function(){return(a._BinaryenTruncSatZeroSVecF64x2ToVecI32x4=a.asm.Pk).apply(null,arguments)}
;
a._BinaryenTruncSatZeroUVecF64x2ToVecI32x4=function(){return(a._BinaryenTruncSatZeroUVecF64x2ToVecI32x4=a.asm.Qk).apply(null,arguments)}
;a._BinaryenDemoteZeroVecF64x2ToVecF32x4=function(){return(a._BinaryenDemoteZeroVecF64x2ToVecF32x4=a.asm.Rk).apply(null,arguments)}
;a._BinaryenPromoteLowVecF32x4ToVecF64x2=function(){return(a._BinaryenPromoteLowVecF32x4ToVecF64x2=a.asm.Sk).apply(null,arguments)}
;a._BinaryenSwizzleVecI8x16=function(){return(a._BinaryenSwizzleVecI8x16=a.asm.Tk).apply(null,arguments)}
;
a._BinaryenRefIsNull=function(){return(a._BinaryenRefIsNull=a.asm.Uk).apply(null,arguments)}
;a._BinaryenRefIsFunc=function(){return(a._BinaryenRefIsFunc=a.asm.Vk).apply(null,arguments)}
;a._BinaryenRefIsData=function(){return(a._BinaryenRefIsData=a.asm.Wk).apply(null,arguments)}
;a._BinaryenRefIsI31=function(){return(a._BinaryenRefIsI31=a.asm.Xk).apply(null,arguments)}
;a._BinaryenRefAsNonNull=function(){return(a._BinaryenRefAsNonNull=a.asm.Yk).apply(null,arguments)}
;
a._BinaryenRefAsFunc=function(){return(a._BinaryenRefAsFunc=a.asm.Zk).apply(null,arguments)}
;a._BinaryenRefAsData=function(){return(a._BinaryenRefAsData=a.asm._k).apply(null,arguments)}
;a._BinaryenRefAsI31=function(){return(a._BinaryenRefAsI31=a.asm.$k).apply(null,arguments)}
;a._BinaryenRefAsExternInternalize=function(){return(a._BinaryenRefAsExternInternalize=a.asm.al).apply(null,arguments)}
;
a._BinaryenRefAsExternExternalize=function(){return(a._BinaryenRefAsExternExternalize=a.asm.bl).apply(null,arguments)}
;a._BinaryenBrOnNull=function(){return(a._BinaryenBrOnNull=a.asm.cl).apply(null,arguments)}
;a._BinaryenBrOnNonNull=function(){return(a._BinaryenBrOnNonNull=a.asm.dl).apply(null,arguments)}
;a._BinaryenBrOnCast=function(){return(a._BinaryenBrOnCast=a.asm.el).apply(null,arguments)}
;a._BinaryenBrOnCastFail=function(){return(a._BinaryenBrOnCastFail=a.asm.fl).apply(null,arguments)}
;
a._BinaryenBrOnFunc=function(){return(a._BinaryenBrOnFunc=a.asm.gl).apply(null,arguments)}
;a._BinaryenBrOnNonFunc=function(){return(a._BinaryenBrOnNonFunc=a.asm.hl).apply(null,arguments)}
;a._BinaryenBrOnData=function(){return(a._BinaryenBrOnData=a.asm.il).apply(null,arguments)}
;a._BinaryenBrOnNonData=function(){return(a._BinaryenBrOnNonData=a.asm.jl).apply(null,arguments)}
;a._BinaryenBrOnI31=function(){return(a._BinaryenBrOnI31=a.asm.kl).apply(null,arguments)}
;
a._BinaryenBrOnNonI31=function(){return(a._BinaryenBrOnNonI31=a.asm.ll).apply(null,arguments)}
;a._BinaryenStringNewUTF8=function(){return(a._BinaryenStringNewUTF8=a.asm.ml).apply(null,arguments)}
;a._BinaryenStringNewWTF8=function(){return(a._BinaryenStringNewWTF8=a.asm.nl).apply(null,arguments)}
;a._BinaryenStringNewReplace=function(){return(a._BinaryenStringNewReplace=a.asm.ol).apply(null,arguments)}
;a._BinaryenStringNewWTF16=function(){return(a._BinaryenStringNewWTF16=a.asm.pl).apply(null,arguments)}
;
a._BinaryenStringNewUTF8Array=function(){return(a._BinaryenStringNewUTF8Array=a.asm.ql).apply(null,arguments)}
;a._BinaryenStringNewWTF8Array=function(){return(a._BinaryenStringNewWTF8Array=a.asm.rl).apply(null,arguments)}
;a._BinaryenStringNewReplaceArray=function(){return(a._BinaryenStringNewReplaceArray=a.asm.sl).apply(null,arguments)}
;a._BinaryenStringNewWTF16Array=function(){return(a._BinaryenStringNewWTF16Array=a.asm.tl).apply(null,arguments)}
;
a._BinaryenStringMeasureUTF8=function(){return(a._BinaryenStringMeasureUTF8=a.asm.ul).apply(null,arguments)}
;a._BinaryenStringMeasureWTF8=function(){return(a._BinaryenStringMeasureWTF8=a.asm.vl).apply(null,arguments)}
;a._BinaryenStringMeasureWTF16=function(){return(a._BinaryenStringMeasureWTF16=a.asm.wl).apply(null,arguments)}
;a._BinaryenStringMeasureIsUSV=function(){return(a._BinaryenStringMeasureIsUSV=a.asm.xl).apply(null,arguments)}
;
a._BinaryenStringMeasureWTF16View=function(){return(a._BinaryenStringMeasureWTF16View=a.asm.yl).apply(null,arguments)}
;a._BinaryenStringEncodeUTF8=function(){return(a._BinaryenStringEncodeUTF8=a.asm.zl).apply(null,arguments)}
;a._BinaryenStringEncodeWTF8=function(){return(a._BinaryenStringEncodeWTF8=a.asm.Al).apply(null,arguments)}
;a._BinaryenStringEncodeWTF16=function(){return(a._BinaryenStringEncodeWTF16=a.asm.Bl).apply(null,arguments)}
;
a._BinaryenStringEncodeUTF8Array=function(){return(a._BinaryenStringEncodeUTF8Array=a.asm.Cl).apply(null,arguments)}
;a._BinaryenStringEncodeWTF8Array=function(){return(a._BinaryenStringEncodeWTF8Array=a.asm.Dl).apply(null,arguments)}
;a._BinaryenStringEncodeWTF16Array=function(){return(a._BinaryenStringEncodeWTF16Array=a.asm.El).apply(null,arguments)}
;a._BinaryenStringAsWTF8=function(){return(a._BinaryenStringAsWTF8=a.asm.Fl).apply(null,arguments)}
;
a._BinaryenStringAsWTF16=function(){return(a._BinaryenStringAsWTF16=a.asm.Gl).apply(null,arguments)}
;a._BinaryenStringAsIter=function(){return(a._BinaryenStringAsIter=a.asm.Hl).apply(null,arguments)}
;a._BinaryenStringIterMoveAdvance=function(){return(a._BinaryenStringIterMoveAdvance=a.asm.Il).apply(null,arguments)}
;a._BinaryenStringIterMoveRewind=function(){return(a._BinaryenStringIterMoveRewind=a.asm.Jl).apply(null,arguments)}
;
a._BinaryenStringSliceWTF8=function(){return(a._BinaryenStringSliceWTF8=a.asm.Kl).apply(null,arguments)}
;a._BinaryenStringSliceWTF16=function(){return(a._BinaryenStringSliceWTF16=a.asm.Ll).apply(null,arguments)}
;a._BinaryenBlock=function(){return(a._BinaryenBlock=a.asm.Ml).apply(null,arguments)}
;a._BinaryenIf=function(){return(a._BinaryenIf=a.asm.Nl).apply(null,arguments)}
;a._BinaryenLoop=function(){return(a._BinaryenLoop=a.asm.Ol).apply(null,arguments)}
;
a._BinaryenBreak=function(){return(a._BinaryenBreak=a.asm.Pl).apply(null,arguments)}
;a._BinaryenSwitch=function(){return(a._BinaryenSwitch=a.asm.Ql).apply(null,arguments)}
;a._BinaryenCall=function(){return(a._BinaryenCall=a.asm.Rl).apply(null,arguments)}
;a._BinaryenReturnCall=function(){return(a._BinaryenReturnCall=a.asm.Sl).apply(null,arguments)}
;a._BinaryenCallIndirect=function(){return(a._BinaryenCallIndirect=a.asm.Tl).apply(null,arguments)}
;
a._BinaryenReturnCallIndirect=function(){return(a._BinaryenReturnCallIndirect=a.asm.Ul).apply(null,arguments)}
;a._BinaryenLocalGet=function(){return(a._BinaryenLocalGet=a.asm.Vl).apply(null,arguments)}
;a._BinaryenLocalSet=function(){return(a._BinaryenLocalSet=a.asm.Wl).apply(null,arguments)}
;a._BinaryenLocalTee=function(){return(a._BinaryenLocalTee=a.asm.Xl).apply(null,arguments)}
;a._BinaryenGlobalGet=function(){return(a._BinaryenGlobalGet=a.asm.Yl).apply(null,arguments)}
;
a._BinaryenGlobalSet=function(){return(a._BinaryenGlobalSet=a.asm.Zl).apply(null,arguments)}
;a._BinaryenLoad=function(){return(a._BinaryenLoad=a.asm._l).apply(null,arguments)}
;a._BinaryenStore=function(){return(a._BinaryenStore=a.asm.$l).apply(null,arguments)}
;a._BinaryenConst=function(){return(a._BinaryenConst=a.asm.am).apply(null,arguments)}
;a._BinaryenUnary=function(){return(a._BinaryenUnary=a.asm.bm).apply(null,arguments)}
;
a._BinaryenBinary=function(){return(a._BinaryenBinary=a.asm.cm).apply(null,arguments)}
;a._BinaryenSelect=function(){return(a._BinaryenSelect=a.asm.dm).apply(null,arguments)}
;a._BinaryenDrop=function(){return(a._BinaryenDrop=a.asm.em).apply(null,arguments)}
;a._BinaryenReturn=function(){return(a._BinaryenReturn=a.asm.fm).apply(null,arguments)}
;a._BinaryenMemorySize=function(){return(a._BinaryenMemorySize=a.asm.gm).apply(null,arguments)}
;
a._BinaryenMemoryGrow=function(){return(a._BinaryenMemoryGrow=a.asm.hm).apply(null,arguments)}
;a._BinaryenNop=function(){return(a._BinaryenNop=a.asm.im).apply(null,arguments)}
;a._BinaryenUnreachable=function(){return(a._BinaryenUnreachable=a.asm.jm).apply(null,arguments)}
;a._BinaryenAtomicLoad=function(){return(a._BinaryenAtomicLoad=a.asm.km).apply(null,arguments)}
;a._BinaryenAtomicStore=function(){return(a._BinaryenAtomicStore=a.asm.lm).apply(null,arguments)}
;
a._BinaryenAtomicRMW=function(){return(a._BinaryenAtomicRMW=a.asm.mm).apply(null,arguments)}
;a._BinaryenAtomicCmpxchg=function(){return(a._BinaryenAtomicCmpxchg=a.asm.nm).apply(null,arguments)}
;a._BinaryenAtomicWait=function(){return(a._BinaryenAtomicWait=a.asm.om).apply(null,arguments)}
;a._BinaryenAtomicNotify=function(){return(a._BinaryenAtomicNotify=a.asm.pm).apply(null,arguments)}
;a._BinaryenAtomicFence=function(){return(a._BinaryenAtomicFence=a.asm.qm).apply(null,arguments)}
;
a._BinaryenSIMDExtract=function(){return(a._BinaryenSIMDExtract=a.asm.rm).apply(null,arguments)}
;a._BinaryenSIMDReplace=function(){return(a._BinaryenSIMDReplace=a.asm.sm).apply(null,arguments)}
;a._BinaryenSIMDShuffle=function(){return(a._BinaryenSIMDShuffle=a.asm.tm).apply(null,arguments)}
;a._BinaryenSIMDTernary=function(){return(a._BinaryenSIMDTernary=a.asm.um).apply(null,arguments)}
;a._BinaryenSIMDShift=function(){return(a._BinaryenSIMDShift=a.asm.vm).apply(null,arguments)}
;
a._BinaryenSIMDLoad=function(){return(a._BinaryenSIMDLoad=a.asm.wm).apply(null,arguments)}
;a._BinaryenSIMDLoadStoreLane=function(){return(a._BinaryenSIMDLoadStoreLane=a.asm.xm).apply(null,arguments)}
;a._BinaryenMemoryInit=function(){return(a._BinaryenMemoryInit=a.asm.ym).apply(null,arguments)}
;a._BinaryenDataDrop=function(){return(a._BinaryenDataDrop=a.asm.zm).apply(null,arguments)}
;a._BinaryenMemoryCopy=function(){return(a._BinaryenMemoryCopy=a.asm.Am).apply(null,arguments)}
;
a._BinaryenMemoryFill=function(){return(a._BinaryenMemoryFill=a.asm.Bm).apply(null,arguments)}
;a._BinaryenTupleMake=function(){return(a._BinaryenTupleMake=a.asm.Cm).apply(null,arguments)}
;a._BinaryenTupleExtract=function(){return(a._BinaryenTupleExtract=a.asm.Dm).apply(null,arguments)}
;a._BinaryenPop=function(){return(a._BinaryenPop=a.asm.Em).apply(null,arguments)}
;a._BinaryenRefNull=function(){return(a._BinaryenRefNull=a.asm.Fm).apply(null,arguments)}
;
a._BinaryenRefIs=function(){return(a._BinaryenRefIs=a.asm.Gm).apply(null,arguments)}
;a._BinaryenRefAs=function(){return(a._BinaryenRefAs=a.asm.Hm).apply(null,arguments)}
;a._BinaryenRefFunc=function(){return(a._BinaryenRefFunc=a.asm.Im).apply(null,arguments)}
;a._BinaryenRefEq=function(){return(a._BinaryenRefEq=a.asm.Jm).apply(null,arguments)}
;a._BinaryenTableGet=function(){return(a._BinaryenTableGet=a.asm.Km).apply(null,arguments)}
;
a._BinaryenTableSet=function(){return(a._BinaryenTableSet=a.asm.Lm).apply(null,arguments)}
;a._BinaryenTableSize=function(){return(a._BinaryenTableSize=a.asm.Mm).apply(null,arguments)}
;a._BinaryenTableGrow=function(){return(a._BinaryenTableGrow=a.asm.Nm).apply(null,arguments)}
;a._BinaryenTry=function(){return(a._BinaryenTry=a.asm.Om).apply(null,arguments)}
;a._BinaryenThrow=function(){return(a._BinaryenThrow=a.asm.Pm).apply(null,arguments)}
;
a._BinaryenRethrow=function(){return(a._BinaryenRethrow=a.asm.Qm).apply(null,arguments)}
;a._BinaryenI31New=function(){return(a._BinaryenI31New=a.asm.Rm).apply(null,arguments)}
;a._BinaryenI31Get=function(){return(a._BinaryenI31Get=a.asm.Sm).apply(null,arguments)}
;a._BinaryenCallRef=function(){return(a._BinaryenCallRef=a.asm.Tm).apply(null,arguments)}
;a._BinaryenRefTest=function(){return(a._BinaryenRefTest=a.asm.Um).apply(null,arguments)}
;
a._BinaryenRefCast=function(){return(a._BinaryenRefCast=a.asm.Vm).apply(null,arguments)}
;a._BinaryenBrOn=function(){return(a._BinaryenBrOn=a.asm.Wm).apply(null,arguments)}
;a._BinaryenStructNew=function(){return(a._BinaryenStructNew=a.asm.Xm).apply(null,arguments)}
;a._BinaryenStructGet=function(){return(a._BinaryenStructGet=a.asm.Ym).apply(null,arguments)}
;a._BinaryenStructSet=function(){return(a._BinaryenStructSet=a.asm.Zm).apply(null,arguments)}
;
a._BinaryenArrayNew=function(){return(a._BinaryenArrayNew=a.asm._m).apply(null,arguments)}
;a._BinaryenArrayInit=function(){return(a._BinaryenArrayInit=a.asm.$m).apply(null,arguments)}
;a._BinaryenArrayGet=function(){return(a._BinaryenArrayGet=a.asm.an).apply(null,arguments)}
;a._BinaryenArraySet=function(){return(a._BinaryenArraySet=a.asm.bn).apply(null,arguments)}
;a._BinaryenArrayLen=function(){return(a._BinaryenArrayLen=a.asm.cn).apply(null,arguments)}
;
a._BinaryenArrayCopy=function(){return(a._BinaryenArrayCopy=a.asm.dn).apply(null,arguments)}
;a._BinaryenStringNew=function(){return(a._BinaryenStringNew=a.asm.en).apply(null,arguments)}
;a._BinaryenStringConst=function(){return(a._BinaryenStringConst=a.asm.fn).apply(null,arguments)}
;a._BinaryenStringMeasure=function(){return(a._BinaryenStringMeasure=a.asm.gn).apply(null,arguments)}
;a._BinaryenStringEncode=function(){return(a._BinaryenStringEncode=a.asm.hn).apply(null,arguments)}
;
a._BinaryenStringConcat=function(){return(a._BinaryenStringConcat=a.asm.jn).apply(null,arguments)}
;a._BinaryenStringEq=function(){return(a._BinaryenStringEq=a.asm.kn).apply(null,arguments)}
;a._BinaryenStringAs=function(){return(a._BinaryenStringAs=a.asm.ln).apply(null,arguments)}
;a._BinaryenStringWTF8Advance=function(){return(a._BinaryenStringWTF8Advance=a.asm.mn).apply(null,arguments)}
;a._BinaryenStringWTF16Get=function(){return(a._BinaryenStringWTF16Get=a.asm.nn).apply(null,arguments)}
;
a._BinaryenStringIterNext=function(){return(a._BinaryenStringIterNext=a.asm.on).apply(null,arguments)}
;a._BinaryenStringIterMove=function(){return(a._BinaryenStringIterMove=a.asm.pn).apply(null,arguments)}
;a._BinaryenStringSliceWTF=function(){return(a._BinaryenStringSliceWTF=a.asm.qn).apply(null,arguments)}
;a._BinaryenStringSliceIter=function(){return(a._BinaryenStringSliceIter=a.asm.rn).apply(null,arguments)}
;
a._BinaryenExpressionGetId=function(){return(a._BinaryenExpressionGetId=a.asm.sn).apply(null,arguments)}
;a._BinaryenExpressionGetType=function(){return(a._BinaryenExpressionGetType=a.asm.tn).apply(null,arguments)}
;a._BinaryenExpressionSetType=function(){return(a._BinaryenExpressionSetType=a.asm.un).apply(null,arguments)}
;a._BinaryenExpressionPrint=function(){return(a._BinaryenExpressionPrint=a.asm.vn).apply(null,arguments)}
;
a._BinaryenExpressionFinalize=function(){return(a._BinaryenExpressionFinalize=a.asm.wn).apply(null,arguments)}
;a._BinaryenExpressionCopy=function(){return(a._BinaryenExpressionCopy=a.asm.xn).apply(null,arguments)}
;a._BinaryenBlockGetName=function(){return(a._BinaryenBlockGetName=a.asm.yn).apply(null,arguments)}
;a._BinaryenBlockSetName=function(){return(a._BinaryenBlockSetName=a.asm.zn).apply(null,arguments)}
;
a._BinaryenBlockGetNumChildren=function(){return(a._BinaryenBlockGetNumChildren=a.asm.An).apply(null,arguments)}
;a._BinaryenBlockGetChildAt=function(){return(a._BinaryenBlockGetChildAt=a.asm.Bn).apply(null,arguments)}
;a._BinaryenBlockSetChildAt=function(){return(a._BinaryenBlockSetChildAt=a.asm.Cn).apply(null,arguments)}
;a._BinaryenBlockAppendChild=function(){return(a._BinaryenBlockAppendChild=a.asm.Dn).apply(null,arguments)}
;
a._BinaryenBlockInsertChildAt=function(){return(a._BinaryenBlockInsertChildAt=a.asm.En).apply(null,arguments)}
;a._BinaryenBlockRemoveChildAt=function(){return(a._BinaryenBlockRemoveChildAt=a.asm.Fn).apply(null,arguments)}
;a._BinaryenIfGetCondition=function(){return(a._BinaryenIfGetCondition=a.asm.Gn).apply(null,arguments)}
;a._BinaryenIfSetCondition=function(){return(a._BinaryenIfSetCondition=a.asm.Hn).apply(null,arguments)}
;
a._BinaryenIfGetIfTrue=function(){return(a._BinaryenIfGetIfTrue=a.asm.In).apply(null,arguments)}
;a._BinaryenIfSetIfTrue=function(){return(a._BinaryenIfSetIfTrue=a.asm.Jn).apply(null,arguments)}
;a._BinaryenIfGetIfFalse=function(){return(a._BinaryenIfGetIfFalse=a.asm.Kn).apply(null,arguments)}
;a._BinaryenIfSetIfFalse=function(){return(a._BinaryenIfSetIfFalse=a.asm.Ln).apply(null,arguments)}
;a._BinaryenLoopGetName=function(){return(a._BinaryenLoopGetName=a.asm.Mn).apply(null,arguments)}
;
a._BinaryenLoopSetName=function(){return(a._BinaryenLoopSetName=a.asm.Nn).apply(null,arguments)}
;a._BinaryenLoopGetBody=function(){return(a._BinaryenLoopGetBody=a.asm.On).apply(null,arguments)}
;a._BinaryenLoopSetBody=function(){return(a._BinaryenLoopSetBody=a.asm.Pn).apply(null,arguments)}
;a._BinaryenBreakGetName=function(){return(a._BinaryenBreakGetName=a.asm.Qn).apply(null,arguments)}
;a._BinaryenBreakSetName=function(){return(a._BinaryenBreakSetName=a.asm.Rn).apply(null,arguments)}
;
a._BinaryenBreakGetCondition=function(){return(a._BinaryenBreakGetCondition=a.asm.Sn).apply(null,arguments)}
;a._BinaryenBreakSetCondition=function(){return(a._BinaryenBreakSetCondition=a.asm.Tn).apply(null,arguments)}
;a._BinaryenBreakGetValue=function(){return(a._BinaryenBreakGetValue=a.asm.Un).apply(null,arguments)}
;a._BinaryenBreakSetValue=function(){return(a._BinaryenBreakSetValue=a.asm.Vn).apply(null,arguments)}
;
a._BinaryenSwitchGetNumNames=function(){return(a._BinaryenSwitchGetNumNames=a.asm.Wn).apply(null,arguments)}
;a._BinaryenSwitchGetNameAt=function(){return(a._BinaryenSwitchGetNameAt=a.asm.Xn).apply(null,arguments)}
;a._BinaryenSwitchSetNameAt=function(){return(a._BinaryenSwitchSetNameAt=a.asm.Yn).apply(null,arguments)}
;a._BinaryenSwitchAppendName=function(){return(a._BinaryenSwitchAppendName=a.asm.Zn).apply(null,arguments)}
;
a._BinaryenSwitchInsertNameAt=function(){return(a._BinaryenSwitchInsertNameAt=a.asm._n).apply(null,arguments)}
;a._BinaryenSwitchRemoveNameAt=function(){return(a._BinaryenSwitchRemoveNameAt=a.asm.$n).apply(null,arguments)}
;a._BinaryenSwitchGetDefaultName=function(){return(a._BinaryenSwitchGetDefaultName=a.asm.ao).apply(null,arguments)}
;a._BinaryenSwitchSetDefaultName=function(){return(a._BinaryenSwitchSetDefaultName=a.asm.bo).apply(null,arguments)}
;
a._BinaryenSwitchGetCondition=function(){return(a._BinaryenSwitchGetCondition=a.asm.co).apply(null,arguments)}
;a._BinaryenSwitchSetCondition=function(){return(a._BinaryenSwitchSetCondition=a.asm.eo).apply(null,arguments)}
;a._BinaryenSwitchGetValue=function(){return(a._BinaryenSwitchGetValue=a.asm.fo).apply(null,arguments)}
;a._BinaryenSwitchSetValue=function(){return(a._BinaryenSwitchSetValue=a.asm.go).apply(null,arguments)}
;
a._BinaryenCallGetTarget=function(){return(a._BinaryenCallGetTarget=a.asm.ho).apply(null,arguments)}
;a._BinaryenCallSetTarget=function(){return(a._BinaryenCallSetTarget=a.asm.io).apply(null,arguments)}
;a._BinaryenCallGetNumOperands=function(){return(a._BinaryenCallGetNumOperands=a.asm.jo).apply(null,arguments)}
;a._BinaryenCallGetOperandAt=function(){return(a._BinaryenCallGetOperandAt=a.asm.ko).apply(null,arguments)}
;
a._BinaryenCallSetOperandAt=function(){return(a._BinaryenCallSetOperandAt=a.asm.lo).apply(null,arguments)}
;a._BinaryenCallAppendOperand=function(){return(a._BinaryenCallAppendOperand=a.asm.mo).apply(null,arguments)}
;a._BinaryenCallInsertOperandAt=function(){return(a._BinaryenCallInsertOperandAt=a.asm.no).apply(null,arguments)}
;a._BinaryenCallRemoveOperandAt=function(){return(a._BinaryenCallRemoveOperandAt=a.asm.oo).apply(null,arguments)}
;
a._BinaryenCallIsReturn=function(){return(a._BinaryenCallIsReturn=a.asm.po).apply(null,arguments)}
;a._BinaryenCallSetReturn=function(){return(a._BinaryenCallSetReturn=a.asm.qo).apply(null,arguments)}
;a._BinaryenCallIndirectGetTarget=function(){return(a._BinaryenCallIndirectGetTarget=a.asm.ro).apply(null,arguments)}
;a._BinaryenCallIndirectSetTarget=function(){return(a._BinaryenCallIndirectSetTarget=a.asm.so).apply(null,arguments)}
;
a._BinaryenCallIndirectGetTable=function(){return(a._BinaryenCallIndirectGetTable=a.asm.to).apply(null,arguments)}
;a._BinaryenCallIndirectSetTable=function(){return(a._BinaryenCallIndirectSetTable=a.asm.uo).apply(null,arguments)}
;a._BinaryenCallIndirectGetNumOperands=function(){return(a._BinaryenCallIndirectGetNumOperands=a.asm.vo).apply(null,arguments)}
;a._BinaryenCallIndirectGetOperandAt=function(){return(a._BinaryenCallIndirectGetOperandAt=a.asm.wo).apply(null,arguments)}
;
a._BinaryenCallIndirectSetOperandAt=function(){return(a._BinaryenCallIndirectSetOperandAt=a.asm.xo).apply(null,arguments)}
;a._BinaryenCallIndirectAppendOperand=function(){return(a._BinaryenCallIndirectAppendOperand=a.asm.yo).apply(null,arguments)}
;a._BinaryenCallIndirectInsertOperandAt=function(){return(a._BinaryenCallIndirectInsertOperandAt=a.asm.zo).apply(null,arguments)}
;a._BinaryenCallIndirectRemoveOperandAt=function(){return(a._BinaryenCallIndirectRemoveOperandAt=a.asm.Ao).apply(null,arguments)}
;
a._BinaryenCallIndirectIsReturn=function(){return(a._BinaryenCallIndirectIsReturn=a.asm.Bo).apply(null,arguments)}
;a._BinaryenCallIndirectSetReturn=function(){return(a._BinaryenCallIndirectSetReturn=a.asm.Co).apply(null,arguments)}
;a._BinaryenCallIndirectGetParams=function(){return(a._BinaryenCallIndirectGetParams=a.asm.Do).apply(null,arguments)}
;a._BinaryenCallIndirectSetParams=function(){return(a._BinaryenCallIndirectSetParams=a.asm.Eo).apply(null,arguments)}
;
a._BinaryenCallIndirectGetResults=function(){return(a._BinaryenCallIndirectGetResults=a.asm.Fo).apply(null,arguments)}
;a._BinaryenCallIndirectSetResults=function(){return(a._BinaryenCallIndirectSetResults=a.asm.Go).apply(null,arguments)}
;a._BinaryenLocalGetGetIndex=function(){return(a._BinaryenLocalGetGetIndex=a.asm.Ho).apply(null,arguments)}
;a._BinaryenLocalGetSetIndex=function(){return(a._BinaryenLocalGetSetIndex=a.asm.Io).apply(null,arguments)}
;
a._BinaryenLocalSetIsTee=function(){return(a._BinaryenLocalSetIsTee=a.asm.Jo).apply(null,arguments)}
;a._BinaryenLocalSetGetIndex=function(){return(a._BinaryenLocalSetGetIndex=a.asm.Ko).apply(null,arguments)}
;a._BinaryenLocalSetSetIndex=function(){return(a._BinaryenLocalSetSetIndex=a.asm.Lo).apply(null,arguments)}
;a._BinaryenLocalSetGetValue=function(){return(a._BinaryenLocalSetGetValue=a.asm.Mo).apply(null,arguments)}
;
a._BinaryenLocalSetSetValue=function(){return(a._BinaryenLocalSetSetValue=a.asm.No).apply(null,arguments)}
;a._BinaryenGlobalGetGetName=function(){return(a._BinaryenGlobalGetGetName=a.asm.Oo).apply(null,arguments)}
;a._BinaryenGlobalGetSetName=function(){return(a._BinaryenGlobalGetSetName=a.asm.Po).apply(null,arguments)}
;a._BinaryenGlobalSetGetName=function(){return(a._BinaryenGlobalSetGetName=a.asm.Qo).apply(null,arguments)}
;
a._BinaryenGlobalSetSetName=function(){return(a._BinaryenGlobalSetSetName=a.asm.Ro).apply(null,arguments)}
;a._BinaryenGlobalSetGetValue=function(){return(a._BinaryenGlobalSetGetValue=a.asm.So).apply(null,arguments)}
;a._BinaryenGlobalSetSetValue=function(){return(a._BinaryenGlobalSetSetValue=a.asm.To).apply(null,arguments)}
;a._BinaryenTableGetGetTable=function(){return(a._BinaryenTableGetGetTable=a.asm.Uo).apply(null,arguments)}
;
a._BinaryenTableGetSetTable=function(){return(a._BinaryenTableGetSetTable=a.asm.Vo).apply(null,arguments)}
;a._BinaryenTableGetGetIndex=function(){return(a._BinaryenTableGetGetIndex=a.asm.Wo).apply(null,arguments)}
;a._BinaryenTableGetSetIndex=function(){return(a._BinaryenTableGetSetIndex=a.asm.Xo).apply(null,arguments)}
;a._BinaryenTableSetGetTable=function(){return(a._BinaryenTableSetGetTable=a.asm.Yo).apply(null,arguments)}
;
a._BinaryenTableSetSetTable=function(){return(a._BinaryenTableSetSetTable=a.asm.Zo).apply(null,arguments)}
;a._BinaryenTableSetGetIndex=function(){return(a._BinaryenTableSetGetIndex=a.asm._o).apply(null,arguments)}
;a._BinaryenTableSetSetIndex=function(){return(a._BinaryenTableSetSetIndex=a.asm.$o).apply(null,arguments)}
;a._BinaryenTableSetGetValue=function(){return(a._BinaryenTableSetGetValue=a.asm.ap).apply(null,arguments)}
;
a._BinaryenTableSetSetValue=function(){return(a._BinaryenTableSetSetValue=a.asm.bp).apply(null,arguments)}
;a._BinaryenTableSizeGetTable=function(){return(a._BinaryenTableSizeGetTable=a.asm.cp).apply(null,arguments)}
;a._BinaryenTableSizeSetTable=function(){return(a._BinaryenTableSizeSetTable=a.asm.dp).apply(null,arguments)}
;a._BinaryenTableGrowGetTable=function(){return(a._BinaryenTableGrowGetTable=a.asm.ep).apply(null,arguments)}
;
a._BinaryenTableGrowSetTable=function(){return(a._BinaryenTableGrowSetTable=a.asm.fp).apply(null,arguments)}
;a._BinaryenTableGrowGetValue=function(){return(a._BinaryenTableGrowGetValue=a.asm.gp).apply(null,arguments)}
;a._BinaryenTableGrowSetValue=function(){return(a._BinaryenTableGrowSetValue=a.asm.hp).apply(null,arguments)}
;a._BinaryenTableGrowGetDelta=function(){return(a._BinaryenTableGrowGetDelta=a.asm.ip).apply(null,arguments)}
;
a._BinaryenTableGrowSetDelta=function(){return(a._BinaryenTableGrowSetDelta=a.asm.jp).apply(null,arguments)}
;a._BinaryenMemoryGrowGetDelta=function(){return(a._BinaryenMemoryGrowGetDelta=a.asm.kp).apply(null,arguments)}
;a._BinaryenMemoryGrowSetDelta=function(){return(a._BinaryenMemoryGrowSetDelta=a.asm.lp).apply(null,arguments)}
;a._BinaryenLoadIsAtomic=function(){return(a._BinaryenLoadIsAtomic=a.asm.mp).apply(null,arguments)}
;
a._BinaryenLoadSetAtomic=function(){return(a._BinaryenLoadSetAtomic=a.asm.np).apply(null,arguments)}
;a._BinaryenLoadIsSigned=function(){return(a._BinaryenLoadIsSigned=a.asm.op).apply(null,arguments)}
;a._BinaryenLoadSetSigned=function(){return(a._BinaryenLoadSetSigned=a.asm.pp).apply(null,arguments)}
;a._BinaryenLoadGetBytes=function(){return(a._BinaryenLoadGetBytes=a.asm.qp).apply(null,arguments)}
;a._BinaryenLoadSetBytes=function(){return(a._BinaryenLoadSetBytes=a.asm.rp).apply(null,arguments)}
;
a._BinaryenLoadGetOffset=function(){return(a._BinaryenLoadGetOffset=a.asm.sp).apply(null,arguments)}
;a._BinaryenLoadSetOffset=function(){return(a._BinaryenLoadSetOffset=a.asm.tp).apply(null,arguments)}
;a._BinaryenLoadGetAlign=function(){return(a._BinaryenLoadGetAlign=a.asm.up).apply(null,arguments)}
;a._BinaryenLoadSetAlign=function(){return(a._BinaryenLoadSetAlign=a.asm.vp).apply(null,arguments)}
;a._BinaryenLoadGetPtr=function(){return(a._BinaryenLoadGetPtr=a.asm.wp).apply(null,arguments)}
;
a._BinaryenLoadSetPtr=function(){return(a._BinaryenLoadSetPtr=a.asm.xp).apply(null,arguments)}
;a._BinaryenStoreIsAtomic=function(){return(a._BinaryenStoreIsAtomic=a.asm.yp).apply(null,arguments)}
;a._BinaryenStoreSetAtomic=function(){return(a._BinaryenStoreSetAtomic=a.asm.zp).apply(null,arguments)}
;a._BinaryenStoreGetBytes=function(){return(a._BinaryenStoreGetBytes=a.asm.Ap).apply(null,arguments)}
;a._BinaryenStoreSetBytes=function(){return(a._BinaryenStoreSetBytes=a.asm.Bp).apply(null,arguments)}
;
a._BinaryenStoreGetOffset=function(){return(a._BinaryenStoreGetOffset=a.asm.Cp).apply(null,arguments)}
;a._BinaryenStoreSetOffset=function(){return(a._BinaryenStoreSetOffset=a.asm.Dp).apply(null,arguments)}
;a._BinaryenStoreGetAlign=function(){return(a._BinaryenStoreGetAlign=a.asm.Ep).apply(null,arguments)}
;a._BinaryenStoreSetAlign=function(){return(a._BinaryenStoreSetAlign=a.asm.Fp).apply(null,arguments)}
;a._BinaryenStoreGetPtr=function(){return(a._BinaryenStoreGetPtr=a.asm.Gp).apply(null,arguments)}
;
a._BinaryenStoreSetPtr=function(){return(a._BinaryenStoreSetPtr=a.asm.Hp).apply(null,arguments)}
;a._BinaryenStoreGetValue=function(){return(a._BinaryenStoreGetValue=a.asm.Ip).apply(null,arguments)}
;a._BinaryenStoreSetValue=function(){return(a._BinaryenStoreSetValue=a.asm.Jp).apply(null,arguments)}
;a._BinaryenStoreGetValueType=function(){return(a._BinaryenStoreGetValueType=a.asm.Kp).apply(null,arguments)}
;
a._BinaryenStoreSetValueType=function(){return(a._BinaryenStoreSetValueType=a.asm.Lp).apply(null,arguments)}
;a._BinaryenConstGetValueI32=function(){return(a._BinaryenConstGetValueI32=a.asm.Mp).apply(null,arguments)}
;a._BinaryenConstSetValueI32=function(){return(a._BinaryenConstSetValueI32=a.asm.Np).apply(null,arguments)}
;a._BinaryenConstGetValueI64=function(){return(a._BinaryenConstGetValueI64=a.asm.Op).apply(null,arguments)}
;
a._BinaryenConstSetValueI64=function(){return(a._BinaryenConstSetValueI64=a.asm.Pp).apply(null,arguments)}
;a._BinaryenConstGetValueI64Low=function(){return(a._BinaryenConstGetValueI64Low=a.asm.Qp).apply(null,arguments)}
;a._BinaryenConstSetValueI64Low=function(){return(a._BinaryenConstSetValueI64Low=a.asm.Rp).apply(null,arguments)}
;a._BinaryenConstGetValueI64High=function(){return(a._BinaryenConstGetValueI64High=a.asm.Sp).apply(null,arguments)}
;
a._BinaryenConstSetValueI64High=function(){return(a._BinaryenConstSetValueI64High=a.asm.Tp).apply(null,arguments)}
;a._BinaryenConstGetValueF32=function(){return(a._BinaryenConstGetValueF32=a.asm.Up).apply(null,arguments)}
;a._BinaryenConstSetValueF32=function(){return(a._BinaryenConstSetValueF32=a.asm.Vp).apply(null,arguments)}
;a._BinaryenConstGetValueF64=function(){return(a._BinaryenConstGetValueF64=a.asm.Wp).apply(null,arguments)}
;
a._BinaryenConstSetValueF64=function(){return(a._BinaryenConstSetValueF64=a.asm.Xp).apply(null,arguments)}
;a._BinaryenConstGetValueV128=function(){return(a._BinaryenConstGetValueV128=a.asm.Yp).apply(null,arguments)}
;a._BinaryenConstSetValueV128=function(){return(a._BinaryenConstSetValueV128=a.asm.Zp).apply(null,arguments)}
;a._BinaryenUnaryGetOp=function(){return(a._BinaryenUnaryGetOp=a.asm._p).apply(null,arguments)}
;
a._BinaryenUnarySetOp=function(){return(a._BinaryenUnarySetOp=a.asm.$p).apply(null,arguments)}
;a._BinaryenUnaryGetValue=function(){return(a._BinaryenUnaryGetValue=a.asm.aq).apply(null,arguments)}
;a._BinaryenUnarySetValue=function(){return(a._BinaryenUnarySetValue=a.asm.bq).apply(null,arguments)}
;a._BinaryenBinaryGetOp=function(){return(a._BinaryenBinaryGetOp=a.asm.cq).apply(null,arguments)}
;a._BinaryenBinarySetOp=function(){return(a._BinaryenBinarySetOp=a.asm.dq).apply(null,arguments)}
;
a._BinaryenBinaryGetLeft=function(){return(a._BinaryenBinaryGetLeft=a.asm.eq).apply(null,arguments)}
;a._BinaryenBinarySetLeft=function(){return(a._BinaryenBinarySetLeft=a.asm.fq).apply(null,arguments)}
;a._BinaryenBinaryGetRight=function(){return(a._BinaryenBinaryGetRight=a.asm.gq).apply(null,arguments)}
;a._BinaryenBinarySetRight=function(){return(a._BinaryenBinarySetRight=a.asm.hq).apply(null,arguments)}
;
a._BinaryenSelectGetIfTrue=function(){return(a._BinaryenSelectGetIfTrue=a.asm.iq).apply(null,arguments)}
;a._BinaryenSelectSetIfTrue=function(){return(a._BinaryenSelectSetIfTrue=a.asm.jq).apply(null,arguments)}
;a._BinaryenSelectGetIfFalse=function(){return(a._BinaryenSelectGetIfFalse=a.asm.kq).apply(null,arguments)}
;a._BinaryenSelectSetIfFalse=function(){return(a._BinaryenSelectSetIfFalse=a.asm.lq).apply(null,arguments)}
;
a._BinaryenSelectGetCondition=function(){return(a._BinaryenSelectGetCondition=a.asm.mq).apply(null,arguments)}
;a._BinaryenSelectSetCondition=function(){return(a._BinaryenSelectSetCondition=a.asm.nq).apply(null,arguments)}
;a._BinaryenDropGetValue=function(){return(a._BinaryenDropGetValue=a.asm.oq).apply(null,arguments)}
;a._BinaryenDropSetValue=function(){return(a._BinaryenDropSetValue=a.asm.pq).apply(null,arguments)}
;
a._BinaryenReturnGetValue=function(){return(a._BinaryenReturnGetValue=a.asm.qq).apply(null,arguments)}
;a._BinaryenReturnSetValue=function(){return(a._BinaryenReturnSetValue=a.asm.rq).apply(null,arguments)}
;a._BinaryenAtomicRMWGetOp=function(){return(a._BinaryenAtomicRMWGetOp=a.asm.sq).apply(null,arguments)}
;a._BinaryenAtomicRMWSetOp=function(){return(a._BinaryenAtomicRMWSetOp=a.asm.tq).apply(null,arguments)}
;
a._BinaryenAtomicRMWGetBytes=function(){return(a._BinaryenAtomicRMWGetBytes=a.asm.uq).apply(null,arguments)}
;a._BinaryenAtomicRMWSetBytes=function(){return(a._BinaryenAtomicRMWSetBytes=a.asm.vq).apply(null,arguments)}
;a._BinaryenAtomicRMWGetOffset=function(){return(a._BinaryenAtomicRMWGetOffset=a.asm.wq).apply(null,arguments)}
;a._BinaryenAtomicRMWSetOffset=function(){return(a._BinaryenAtomicRMWSetOffset=a.asm.xq).apply(null,arguments)}
;
a._BinaryenAtomicRMWGetPtr=function(){return(a._BinaryenAtomicRMWGetPtr=a.asm.yq).apply(null,arguments)}
;a._BinaryenAtomicRMWSetPtr=function(){return(a._BinaryenAtomicRMWSetPtr=a.asm.zq).apply(null,arguments)}
;a._BinaryenAtomicRMWGetValue=function(){return(a._BinaryenAtomicRMWGetValue=a.asm.Aq).apply(null,arguments)}
;a._BinaryenAtomicRMWSetValue=function(){return(a._BinaryenAtomicRMWSetValue=a.asm.Bq).apply(null,arguments)}
;
a._BinaryenAtomicCmpxchgGetBytes=function(){return(a._BinaryenAtomicCmpxchgGetBytes=a.asm.Cq).apply(null,arguments)}
;a._BinaryenAtomicCmpxchgSetBytes=function(){return(a._BinaryenAtomicCmpxchgSetBytes=a.asm.Dq).apply(null,arguments)}
;a._BinaryenAtomicCmpxchgGetOffset=function(){return(a._BinaryenAtomicCmpxchgGetOffset=a.asm.Eq).apply(null,arguments)}
;a._BinaryenAtomicCmpxchgSetOffset=function(){return(a._BinaryenAtomicCmpxchgSetOffset=a.asm.Fq).apply(null,arguments)}
;
a._BinaryenAtomicCmpxchgGetPtr=function(){return(a._BinaryenAtomicCmpxchgGetPtr=a.asm.Gq).apply(null,arguments)}
;a._BinaryenAtomicCmpxchgSetPtr=function(){return(a._BinaryenAtomicCmpxchgSetPtr=a.asm.Hq).apply(null,arguments)}
;a._BinaryenAtomicCmpxchgGetExpected=function(){return(a._BinaryenAtomicCmpxchgGetExpected=a.asm.Iq).apply(null,arguments)}
;a._BinaryenAtomicCmpxchgSetExpected=function(){return(a._BinaryenAtomicCmpxchgSetExpected=a.asm.Jq).apply(null,arguments)}
;
a._BinaryenAtomicCmpxchgGetReplacement=function(){return(a._BinaryenAtomicCmpxchgGetReplacement=a.asm.Kq).apply(null,arguments)}
;a._BinaryenAtomicCmpxchgSetReplacement=function(){return(a._BinaryenAtomicCmpxchgSetReplacement=a.asm.Lq).apply(null,arguments)}
;a._BinaryenAtomicWaitGetPtr=function(){return(a._BinaryenAtomicWaitGetPtr=a.asm.Mq).apply(null,arguments)}
;a._BinaryenAtomicWaitSetPtr=function(){return(a._BinaryenAtomicWaitSetPtr=a.asm.Nq).apply(null,arguments)}
;
a._BinaryenAtomicWaitGetExpected=function(){return(a._BinaryenAtomicWaitGetExpected=a.asm.Oq).apply(null,arguments)}
;a._BinaryenAtomicWaitSetExpected=function(){return(a._BinaryenAtomicWaitSetExpected=a.asm.Pq).apply(null,arguments)}
;a._BinaryenAtomicWaitGetTimeout=function(){return(a._BinaryenAtomicWaitGetTimeout=a.asm.Qq).apply(null,arguments)}
;a._BinaryenAtomicWaitSetTimeout=function(){return(a._BinaryenAtomicWaitSetTimeout=a.asm.Rq).apply(null,arguments)}
;
a._BinaryenAtomicWaitGetExpectedType=function(){return(a._BinaryenAtomicWaitGetExpectedType=a.asm.Sq).apply(null,arguments)}
;a._BinaryenAtomicWaitSetExpectedType=function(){return(a._BinaryenAtomicWaitSetExpectedType=a.asm.Tq).apply(null,arguments)}
;a._BinaryenAtomicNotifyGetPtr=function(){return(a._BinaryenAtomicNotifyGetPtr=a.asm.Uq).apply(null,arguments)}
;a._BinaryenAtomicNotifySetPtr=function(){return(a._BinaryenAtomicNotifySetPtr=a.asm.Vq).apply(null,arguments)}
;
a._BinaryenAtomicNotifyGetNotifyCount=function(){return(a._BinaryenAtomicNotifyGetNotifyCount=a.asm.Wq).apply(null,arguments)}
;a._BinaryenAtomicNotifySetNotifyCount=function(){return(a._BinaryenAtomicNotifySetNotifyCount=a.asm.Xq).apply(null,arguments)}
;a._BinaryenAtomicFenceGetOrder=function(){return(a._BinaryenAtomicFenceGetOrder=a.asm.Yq).apply(null,arguments)}
;a._BinaryenAtomicFenceSetOrder=function(){return(a._BinaryenAtomicFenceSetOrder=a.asm.Zq).apply(null,arguments)}
;
a._BinaryenSIMDExtractGetOp=function(){return(a._BinaryenSIMDExtractGetOp=a.asm._q).apply(null,arguments)}
;a._BinaryenSIMDExtractSetOp=function(){return(a._BinaryenSIMDExtractSetOp=a.asm.$q).apply(null,arguments)}
;a._BinaryenSIMDExtractGetVec=function(){return(a._BinaryenSIMDExtractGetVec=a.asm.ar).apply(null,arguments)}
;a._BinaryenSIMDExtractSetVec=function(){return(a._BinaryenSIMDExtractSetVec=a.asm.br).apply(null,arguments)}
;
a._BinaryenSIMDExtractGetIndex=function(){return(a._BinaryenSIMDExtractGetIndex=a.asm.cr).apply(null,arguments)}
;a._BinaryenSIMDExtractSetIndex=function(){return(a._BinaryenSIMDExtractSetIndex=a.asm.dr).apply(null,arguments)}
;a._BinaryenSIMDReplaceGetOp=function(){return(a._BinaryenSIMDReplaceGetOp=a.asm.er).apply(null,arguments)}
;a._BinaryenSIMDReplaceSetOp=function(){return(a._BinaryenSIMDReplaceSetOp=a.asm.fr).apply(null,arguments)}
;
a._BinaryenSIMDReplaceGetVec=function(){return(a._BinaryenSIMDReplaceGetVec=a.asm.gr).apply(null,arguments)}
;a._BinaryenSIMDReplaceSetVec=function(){return(a._BinaryenSIMDReplaceSetVec=a.asm.hr).apply(null,arguments)}
;a._BinaryenSIMDReplaceGetIndex=function(){return(a._BinaryenSIMDReplaceGetIndex=a.asm.ir).apply(null,arguments)}
;a._BinaryenSIMDReplaceSetIndex=function(){return(a._BinaryenSIMDReplaceSetIndex=a.asm.jr).apply(null,arguments)}
;
a._BinaryenSIMDReplaceGetValue=function(){return(a._BinaryenSIMDReplaceGetValue=a.asm.kr).apply(null,arguments)}
;a._BinaryenSIMDReplaceSetValue=function(){return(a._BinaryenSIMDReplaceSetValue=a.asm.lr).apply(null,arguments)}
;a._BinaryenSIMDShuffleGetLeft=function(){return(a._BinaryenSIMDShuffleGetLeft=a.asm.mr).apply(null,arguments)}
;a._BinaryenSIMDShuffleSetLeft=function(){return(a._BinaryenSIMDShuffleSetLeft=a.asm.nr).apply(null,arguments)}
;
a._BinaryenSIMDShuffleGetRight=function(){return(a._BinaryenSIMDShuffleGetRight=a.asm.or).apply(null,arguments)}
;a._BinaryenSIMDShuffleSetRight=function(){return(a._BinaryenSIMDShuffleSetRight=a.asm.pr).apply(null,arguments)}
;a._BinaryenSIMDShuffleGetMask=function(){return(a._BinaryenSIMDShuffleGetMask=a.asm.qr).apply(null,arguments)}
;a._BinaryenSIMDShuffleSetMask=function(){return(a._BinaryenSIMDShuffleSetMask=a.asm.rr).apply(null,arguments)}
;
a._BinaryenSIMDTernaryGetOp=function(){return(a._BinaryenSIMDTernaryGetOp=a.asm.sr).apply(null,arguments)}
;a._BinaryenSIMDTernarySetOp=function(){return(a._BinaryenSIMDTernarySetOp=a.asm.tr).apply(null,arguments)}
;a._BinaryenSIMDTernaryGetA=function(){return(a._BinaryenSIMDTernaryGetA=a.asm.ur).apply(null,arguments)}
;a._BinaryenSIMDTernarySetA=function(){return(a._BinaryenSIMDTernarySetA=a.asm.vr).apply(null,arguments)}
;
a._BinaryenSIMDTernaryGetB=function(){return(a._BinaryenSIMDTernaryGetB=a.asm.wr).apply(null,arguments)}
;a._BinaryenSIMDTernarySetB=function(){return(a._BinaryenSIMDTernarySetB=a.asm.xr).apply(null,arguments)}
;a._BinaryenSIMDTernaryGetC=function(){return(a._BinaryenSIMDTernaryGetC=a.asm.yr).apply(null,arguments)}
;a._BinaryenSIMDTernarySetC=function(){return(a._BinaryenSIMDTernarySetC=a.asm.zr).apply(null,arguments)}
;
a._BinaryenSIMDShiftGetOp=function(){return(a._BinaryenSIMDShiftGetOp=a.asm.Ar).apply(null,arguments)}
;a._BinaryenSIMDShiftSetOp=function(){return(a._BinaryenSIMDShiftSetOp=a.asm.Br).apply(null,arguments)}
;a._BinaryenSIMDShiftGetVec=function(){return(a._BinaryenSIMDShiftGetVec=a.asm.Cr).apply(null,arguments)}
;a._BinaryenSIMDShiftSetVec=function(){return(a._BinaryenSIMDShiftSetVec=a.asm.Dr).apply(null,arguments)}
;
a._BinaryenSIMDShiftGetShift=function(){return(a._BinaryenSIMDShiftGetShift=a.asm.Er).apply(null,arguments)}
;a._BinaryenSIMDShiftSetShift=function(){return(a._BinaryenSIMDShiftSetShift=a.asm.Fr).apply(null,arguments)}
;a._BinaryenSIMDLoadGetOp=function(){return(a._BinaryenSIMDLoadGetOp=a.asm.Gr).apply(null,arguments)}
;a._BinaryenSIMDLoadSetOp=function(){return(a._BinaryenSIMDLoadSetOp=a.asm.Hr).apply(null,arguments)}
;
a._BinaryenSIMDLoadGetOffset=function(){return(a._BinaryenSIMDLoadGetOffset=a.asm.Ir).apply(null,arguments)}
;a._BinaryenSIMDLoadSetOffset=function(){return(a._BinaryenSIMDLoadSetOffset=a.asm.Jr).apply(null,arguments)}
;a._BinaryenSIMDLoadGetAlign=function(){return(a._BinaryenSIMDLoadGetAlign=a.asm.Kr).apply(null,arguments)}
;a._BinaryenSIMDLoadSetAlign=function(){return(a._BinaryenSIMDLoadSetAlign=a.asm.Lr).apply(null,arguments)}
;
a._BinaryenSIMDLoadGetPtr=function(){return(a._BinaryenSIMDLoadGetPtr=a.asm.Mr).apply(null,arguments)}
;a._BinaryenSIMDLoadSetPtr=function(){return(a._BinaryenSIMDLoadSetPtr=a.asm.Nr).apply(null,arguments)}
;a._BinaryenSIMDLoadStoreLaneGetOp=function(){return(a._BinaryenSIMDLoadStoreLaneGetOp=a.asm.Or).apply(null,arguments)}
;a._BinaryenSIMDLoadStoreLaneSetOp=function(){return(a._BinaryenSIMDLoadStoreLaneSetOp=a.asm.Pr).apply(null,arguments)}
;
a._BinaryenSIMDLoadStoreLaneGetOffset=function(){return(a._BinaryenSIMDLoadStoreLaneGetOffset=a.asm.Qr).apply(null,arguments)}
;a._BinaryenSIMDLoadStoreLaneSetOffset=function(){return(a._BinaryenSIMDLoadStoreLaneSetOffset=a.asm.Rr).apply(null,arguments)}
;a._BinaryenSIMDLoadStoreLaneGetAlign=function(){return(a._BinaryenSIMDLoadStoreLaneGetAlign=a.asm.Sr).apply(null,arguments)}
;a._BinaryenSIMDLoadStoreLaneSetAlign=function(){return(a._BinaryenSIMDLoadStoreLaneSetAlign=a.asm.Tr).apply(null,arguments)}
;
a._BinaryenSIMDLoadStoreLaneGetIndex=function(){return(a._BinaryenSIMDLoadStoreLaneGetIndex=a.asm.Ur).apply(null,arguments)}
;a._BinaryenSIMDLoadStoreLaneSetIndex=function(){return(a._BinaryenSIMDLoadStoreLaneSetIndex=a.asm.Vr).apply(null,arguments)}
;a._BinaryenSIMDLoadStoreLaneGetPtr=function(){return(a._BinaryenSIMDLoadStoreLaneGetPtr=a.asm.Wr).apply(null,arguments)}
;a._BinaryenSIMDLoadStoreLaneSetPtr=function(){return(a._BinaryenSIMDLoadStoreLaneSetPtr=a.asm.Xr).apply(null,arguments)}
;
a._BinaryenSIMDLoadStoreLaneGetVec=function(){return(a._BinaryenSIMDLoadStoreLaneGetVec=a.asm.Yr).apply(null,arguments)}
;a._BinaryenSIMDLoadStoreLaneSetVec=function(){return(a._BinaryenSIMDLoadStoreLaneSetVec=a.asm.Zr).apply(null,arguments)}
;a._BinaryenSIMDLoadStoreLaneIsStore=function(){return(a._BinaryenSIMDLoadStoreLaneIsStore=a.asm._r).apply(null,arguments)}
;a._BinaryenMemoryInitGetSegment=function(){return(a._BinaryenMemoryInitGetSegment=a.asm.$r).apply(null,arguments)}
;
a._BinaryenMemoryInitSetSegment=function(){return(a._BinaryenMemoryInitSetSegment=a.asm.as).apply(null,arguments)}
;a._BinaryenMemoryInitGetDest=function(){return(a._BinaryenMemoryInitGetDest=a.asm.bs).apply(null,arguments)}
;a._BinaryenMemoryInitSetDest=function(){return(a._BinaryenMemoryInitSetDest=a.asm.cs).apply(null,arguments)}
;a._BinaryenMemoryInitGetOffset=function(){return(a._BinaryenMemoryInitGetOffset=a.asm.ds).apply(null,arguments)}
;
a._BinaryenMemoryInitSetOffset=function(){return(a._BinaryenMemoryInitSetOffset=a.asm.es).apply(null,arguments)}
;a._BinaryenMemoryInitGetSize=function(){return(a._BinaryenMemoryInitGetSize=a.asm.fs).apply(null,arguments)}
;a._BinaryenMemoryInitSetSize=function(){return(a._BinaryenMemoryInitSetSize=a.asm.gs).apply(null,arguments)}
;a._BinaryenDataDropGetSegment=function(){return(a._BinaryenDataDropGetSegment=a.asm.hs).apply(null,arguments)}
;
a._BinaryenDataDropSetSegment=function(){return(a._BinaryenDataDropSetSegment=a.asm.is).apply(null,arguments)}
;a._BinaryenMemoryCopyGetDest=function(){return(a._BinaryenMemoryCopyGetDest=a.asm.js).apply(null,arguments)}
;a._BinaryenMemoryCopySetDest=function(){return(a._BinaryenMemoryCopySetDest=a.asm.ks).apply(null,arguments)}
;a._BinaryenMemoryCopyGetSource=function(){return(a._BinaryenMemoryCopyGetSource=a.asm.ls).apply(null,arguments)}
;
a._BinaryenMemoryCopySetSource=function(){return(a._BinaryenMemoryCopySetSource=a.asm.ms).apply(null,arguments)}
;a._BinaryenMemoryCopyGetSize=function(){return(a._BinaryenMemoryCopyGetSize=a.asm.ns).apply(null,arguments)}
;a._BinaryenMemoryCopySetSize=function(){return(a._BinaryenMemoryCopySetSize=a.asm.os).apply(null,arguments)}
;a._BinaryenMemoryFillGetDest=function(){return(a._BinaryenMemoryFillGetDest=a.asm.ps).apply(null,arguments)}
;
a._BinaryenMemoryFillSetDest=function(){return(a._BinaryenMemoryFillSetDest=a.asm.qs).apply(null,arguments)}
;a._BinaryenMemoryFillGetValue=function(){return(a._BinaryenMemoryFillGetValue=a.asm.rs).apply(null,arguments)}
;a._BinaryenMemoryFillSetValue=function(){return(a._BinaryenMemoryFillSetValue=a.asm.ss).apply(null,arguments)}
;a._BinaryenMemoryFillGetSize=function(){return(a._BinaryenMemoryFillGetSize=a.asm.ts).apply(null,arguments)}
;
a._BinaryenMemoryFillSetSize=function(){return(a._BinaryenMemoryFillSetSize=a.asm.us).apply(null,arguments)}
;a._BinaryenRefIsGetOp=function(){return(a._BinaryenRefIsGetOp=a.asm.vs).apply(null,arguments)}
;a._BinaryenRefIsSetOp=function(){return(a._BinaryenRefIsSetOp=a.asm.ws).apply(null,arguments)}
;a._BinaryenRefIsGetValue=function(){return(a._BinaryenRefIsGetValue=a.asm.xs).apply(null,arguments)}
;a._BinaryenRefIsSetValue=function(){return(a._BinaryenRefIsSetValue=a.asm.ys).apply(null,arguments)}
;
a._BinaryenRefAsGetOp=function(){return(a._BinaryenRefAsGetOp=a.asm.zs).apply(null,arguments)}
;a._BinaryenRefAsSetOp=function(){return(a._BinaryenRefAsSetOp=a.asm.As).apply(null,arguments)}
;a._BinaryenRefAsGetValue=function(){return(a._BinaryenRefAsGetValue=a.asm.Bs).apply(null,arguments)}
;a._BinaryenRefAsSetValue=function(){return(a._BinaryenRefAsSetValue=a.asm.Cs).apply(null,arguments)}
;a._BinaryenRefFuncGetFunc=function(){return(a._BinaryenRefFuncGetFunc=a.asm.Ds).apply(null,arguments)}
;
a._BinaryenRefFuncSetFunc=function(){return(a._BinaryenRefFuncSetFunc=a.asm.Es).apply(null,arguments)}
;a._BinaryenRefEqGetLeft=function(){return(a._BinaryenRefEqGetLeft=a.asm.Fs).apply(null,arguments)}
;a._BinaryenRefEqSetLeft=function(){return(a._BinaryenRefEqSetLeft=a.asm.Gs).apply(null,arguments)}
;a._BinaryenRefEqGetRight=function(){return(a._BinaryenRefEqGetRight=a.asm.Hs).apply(null,arguments)}
;a._BinaryenRefEqSetRight=function(){return(a._BinaryenRefEqSetRight=a.asm.Is).apply(null,arguments)}
;
a._BinaryenTryGetName=function(){return(a._BinaryenTryGetName=a.asm.Js).apply(null,arguments)}
;a._BinaryenTrySetName=function(){return(a._BinaryenTrySetName=a.asm.Ks).apply(null,arguments)}
;a._BinaryenTryGetBody=function(){return(a._BinaryenTryGetBody=a.asm.Ls).apply(null,arguments)}
;a._BinaryenTrySetBody=function(){return(a._BinaryenTrySetBody=a.asm.Ms).apply(null,arguments)}
;a._BinaryenTryGetNumCatchTags=function(){return(a._BinaryenTryGetNumCatchTags=a.asm.Ns).apply(null,arguments)}
;
a._BinaryenTryGetNumCatchBodies=function(){return(a._BinaryenTryGetNumCatchBodies=a.asm.Os).apply(null,arguments)}
;a._BinaryenTryGetCatchTagAt=function(){return(a._BinaryenTryGetCatchTagAt=a.asm.Ps).apply(null,arguments)}
;a._BinaryenTrySetCatchTagAt=function(){return(a._BinaryenTrySetCatchTagAt=a.asm.Qs).apply(null,arguments)}
;a._BinaryenTryAppendCatchTag=function(){return(a._BinaryenTryAppendCatchTag=a.asm.Rs).apply(null,arguments)}
;
a._BinaryenTryInsertCatchTagAt=function(){return(a._BinaryenTryInsertCatchTagAt=a.asm.Ss).apply(null,arguments)}
;a._BinaryenTryRemoveCatchTagAt=function(){return(a._BinaryenTryRemoveCatchTagAt=a.asm.Ts).apply(null,arguments)}
;a._BinaryenTryGetCatchBodyAt=function(){return(a._BinaryenTryGetCatchBodyAt=a.asm.Us).apply(null,arguments)}
;a._BinaryenTrySetCatchBodyAt=function(){return(a._BinaryenTrySetCatchBodyAt=a.asm.Vs).apply(null,arguments)}
;
a._BinaryenTryAppendCatchBody=function(){return(a._BinaryenTryAppendCatchBody=a.asm.Ws).apply(null,arguments)}
;a._BinaryenTryInsertCatchBodyAt=function(){return(a._BinaryenTryInsertCatchBodyAt=a.asm.Xs).apply(null,arguments)}
;a._BinaryenTryRemoveCatchBodyAt=function(){return(a._BinaryenTryRemoveCatchBodyAt=a.asm.Ys).apply(null,arguments)}
;a._BinaryenTryHasCatchAll=function(){return(a._BinaryenTryHasCatchAll=a.asm.Zs).apply(null,arguments)}
;
a._BinaryenTryGetDelegateTarget=function(){return(a._BinaryenTryGetDelegateTarget=a.asm._s).apply(null,arguments)}
;a._BinaryenTrySetDelegateTarget=function(){return(a._BinaryenTrySetDelegateTarget=a.asm.$s).apply(null,arguments)}
;a._BinaryenTryIsDelegate=function(){return(a._BinaryenTryIsDelegate=a.asm.at).apply(null,arguments)}
;a._BinaryenThrowGetTag=function(){return(a._BinaryenThrowGetTag=a.asm.bt).apply(null,arguments)}
;
a._BinaryenThrowSetTag=function(){return(a._BinaryenThrowSetTag=a.asm.ct).apply(null,arguments)}
;a._BinaryenThrowGetNumOperands=function(){return(a._BinaryenThrowGetNumOperands=a.asm.dt).apply(null,arguments)}
;a._BinaryenThrowGetOperandAt=function(){return(a._BinaryenThrowGetOperandAt=a.asm.et).apply(null,arguments)}
;a._BinaryenThrowSetOperandAt=function(){return(a._BinaryenThrowSetOperandAt=a.asm.ft).apply(null,arguments)}
;
a._BinaryenThrowAppendOperand=function(){return(a._BinaryenThrowAppendOperand=a.asm.gt).apply(null,arguments)}
;a._BinaryenThrowInsertOperandAt=function(){return(a._BinaryenThrowInsertOperandAt=a.asm.ht).apply(null,arguments)}
;a._BinaryenThrowRemoveOperandAt=function(){return(a._BinaryenThrowRemoveOperandAt=a.asm.it).apply(null,arguments)}
;a._BinaryenRethrowGetTarget=function(){return(a._BinaryenRethrowGetTarget=a.asm.jt).apply(null,arguments)}
;
a._BinaryenRethrowSetTarget=function(){return(a._BinaryenRethrowSetTarget=a.asm.kt).apply(null,arguments)}
;a._BinaryenTupleMakeGetNumOperands=function(){return(a._BinaryenTupleMakeGetNumOperands=a.asm.lt).apply(null,arguments)}
;a._BinaryenTupleMakeGetOperandAt=function(){return(a._BinaryenTupleMakeGetOperandAt=a.asm.mt).apply(null,arguments)}
;a._BinaryenTupleMakeSetOperandAt=function(){return(a._BinaryenTupleMakeSetOperandAt=a.asm.nt).apply(null,arguments)}
;
a._BinaryenTupleMakeAppendOperand=function(){return(a._BinaryenTupleMakeAppendOperand=a.asm.ot).apply(null,arguments)}
;a._BinaryenTupleMakeInsertOperandAt=function(){return(a._BinaryenTupleMakeInsertOperandAt=a.asm.pt).apply(null,arguments)}
;a._BinaryenTupleMakeRemoveOperandAt=function(){return(a._BinaryenTupleMakeRemoveOperandAt=a.asm.qt).apply(null,arguments)}
;a._BinaryenTupleExtractGetTuple=function(){return(a._BinaryenTupleExtractGetTuple=a.asm.rt).apply(null,arguments)}
;
a._BinaryenTupleExtractSetTuple=function(){return(a._BinaryenTupleExtractSetTuple=a.asm.st).apply(null,arguments)}
;a._BinaryenTupleExtractGetIndex=function(){return(a._BinaryenTupleExtractGetIndex=a.asm.tt).apply(null,arguments)}
;a._BinaryenTupleExtractSetIndex=function(){return(a._BinaryenTupleExtractSetIndex=a.asm.ut).apply(null,arguments)}
;a._BinaryenI31NewGetValue=function(){return(a._BinaryenI31NewGetValue=a.asm.vt).apply(null,arguments)}
;
a._BinaryenI31NewSetValue=function(){return(a._BinaryenI31NewSetValue=a.asm.wt).apply(null,arguments)}
;a._BinaryenI31GetGetI31=function(){return(a._BinaryenI31GetGetI31=a.asm.xt).apply(null,arguments)}
;a._BinaryenI31GetSetI31=function(){return(a._BinaryenI31GetSetI31=a.asm.yt).apply(null,arguments)}
;a._BinaryenI31GetIsSigned=function(){return(a._BinaryenI31GetIsSigned=a.asm.zt).apply(null,arguments)}
;a._BinaryenI31GetSetSigned=function(){return(a._BinaryenI31GetSetSigned=a.asm.At).apply(null,arguments)}
;
a._BinaryenCallRefGetNumOperands=function(){return(a._BinaryenCallRefGetNumOperands=a.asm.Bt).apply(null,arguments)}
;a._BinaryenCallRefGetOperandAt=function(){return(a._BinaryenCallRefGetOperandAt=a.asm.Ct).apply(null,arguments)}
;a._BinaryenCallRefSetOperandAt=function(){return(a._BinaryenCallRefSetOperandAt=a.asm.Dt).apply(null,arguments)}
;a._BinaryenCallRefAppendOperand=function(){return(a._BinaryenCallRefAppendOperand=a.asm.Et).apply(null,arguments)}
;
a._BinaryenCallRefInsertOperandAt=function(){return(a._BinaryenCallRefInsertOperandAt=a.asm.Ft).apply(null,arguments)}
;a._BinaryenCallRefRemoveOperandAt=function(){return(a._BinaryenCallRefRemoveOperandAt=a.asm.Gt).apply(null,arguments)}
;a._BinaryenCallRefGetTarget=function(){return(a._BinaryenCallRefGetTarget=a.asm.Ht).apply(null,arguments)}
;a._BinaryenCallRefSetTarget=function(){return(a._BinaryenCallRefSetTarget=a.asm.It).apply(null,arguments)}
;
a._BinaryenCallRefIsReturn=function(){return(a._BinaryenCallRefIsReturn=a.asm.Jt).apply(null,arguments)}
;a._BinaryenCallRefSetReturn=function(){return(a._BinaryenCallRefSetReturn=a.asm.Kt).apply(null,arguments)}
;a._BinaryenRefTestGetRef=function(){return(a._BinaryenRefTestGetRef=a.asm.Lt).apply(null,arguments)}
;a._BinaryenRefTestSetRef=function(){return(a._BinaryenRefTestSetRef=a.asm.Mt).apply(null,arguments)}
;
a._BinaryenRefTestGetIntendedType=function(){return(a._BinaryenRefTestGetIntendedType=a.asm.Nt).apply(null,arguments)}
;a._BinaryenRefTestSetIntendedType=function(){return(a._BinaryenRefTestSetIntendedType=a.asm.Ot).apply(null,arguments)}
;a._BinaryenRefCastGetRef=function(){return(a._BinaryenRefCastGetRef=a.asm.Pt).apply(null,arguments)}
;a._BinaryenRefCastSetRef=function(){return(a._BinaryenRefCastSetRef=a.asm.Qt).apply(null,arguments)}
;
a._BinaryenRefCastGetIntendedType=function(){return(a._BinaryenRefCastGetIntendedType=a.asm.Rt).apply(null,arguments)}
;a._BinaryenRefCastSetIntendedType=function(){return(a._BinaryenRefCastSetIntendedType=a.asm.St).apply(null,arguments)}
;a._BinaryenBrOnGetOp=function(){return(a._BinaryenBrOnGetOp=a.asm.Tt).apply(null,arguments)}
;a._BinaryenBrOnSetOp=function(){return(a._BinaryenBrOnSetOp=a.asm.Ut).apply(null,arguments)}
;
a._BinaryenBrOnGetName=function(){return(a._BinaryenBrOnGetName=a.asm.Vt).apply(null,arguments)}
;a._BinaryenBrOnSetName=function(){return(a._BinaryenBrOnSetName=a.asm.Wt).apply(null,arguments)}
;a._BinaryenBrOnGetRef=function(){return(a._BinaryenBrOnGetRef=a.asm.Xt).apply(null,arguments)}
;a._BinaryenBrOnSetRef=function(){return(a._BinaryenBrOnSetRef=a.asm.Yt).apply(null,arguments)}
;a._BinaryenBrOnGetIntendedType=function(){return(a._BinaryenBrOnGetIntendedType=a.asm.Zt).apply(null,arguments)}
;
a._BinaryenBrOnSetIntendedType=function(){return(a._BinaryenBrOnSetIntendedType=a.asm._t).apply(null,arguments)}
;a._BinaryenStructNewGetNumOperands=function(){return(a._BinaryenStructNewGetNumOperands=a.asm.$t).apply(null,arguments)}
;a._BinaryenStructNewGetOperandAt=function(){return(a._BinaryenStructNewGetOperandAt=a.asm.au).apply(null,arguments)}
;a._BinaryenStructNewSetOperandAt=function(){return(a._BinaryenStructNewSetOperandAt=a.asm.bu).apply(null,arguments)}
;
a._BinaryenStructNewAppendOperand=function(){return(a._BinaryenStructNewAppendOperand=a.asm.cu).apply(null,arguments)}
;a._BinaryenStructNewInsertOperandAt=function(){return(a._BinaryenStructNewInsertOperandAt=a.asm.du).apply(null,arguments)}
;a._BinaryenStructNewRemoveOperandAt=function(){return(a._BinaryenStructNewRemoveOperandAt=a.asm.eu).apply(null,arguments)}
;a._BinaryenStructGetGetIndex=function(){return(a._BinaryenStructGetGetIndex=a.asm.fu).apply(null,arguments)}
;
a._BinaryenStructGetSetIndex=function(){return(a._BinaryenStructGetSetIndex=a.asm.gu).apply(null,arguments)}
;a._BinaryenStructGetGetRef=function(){return(a._BinaryenStructGetGetRef=a.asm.hu).apply(null,arguments)}
;a._BinaryenStructGetSetRef=function(){return(a._BinaryenStructGetSetRef=a.asm.iu).apply(null,arguments)}
;a._BinaryenStructGetIsSigned=function(){return(a._BinaryenStructGetIsSigned=a.asm.ju).apply(null,arguments)}
;
a._BinaryenStructGetSetSigned=function(){return(a._BinaryenStructGetSetSigned=a.asm.ku).apply(null,arguments)}
;a._BinaryenStructSetGetIndex=function(){return(a._BinaryenStructSetGetIndex=a.asm.lu).apply(null,arguments)}
;a._BinaryenStructSetSetIndex=function(){return(a._BinaryenStructSetSetIndex=a.asm.mu).apply(null,arguments)}
;a._BinaryenStructSetGetRef=function(){return(a._BinaryenStructSetGetRef=a.asm.nu).apply(null,arguments)}
;
a._BinaryenStructSetSetRef=function(){return(a._BinaryenStructSetSetRef=a.asm.ou).apply(null,arguments)}
;a._BinaryenStructSetGetValue=function(){return(a._BinaryenStructSetGetValue=a.asm.pu).apply(null,arguments)}
;a._BinaryenStructSetSetValue=function(){return(a._BinaryenStructSetSetValue=a.asm.qu).apply(null,arguments)}
;a._BinaryenArrayNewGetInit=function(){return(a._BinaryenArrayNewGetInit=a.asm.ru).apply(null,arguments)}
;
a._BinaryenArrayNewSetInit=function(){return(a._BinaryenArrayNewSetInit=a.asm.su).apply(null,arguments)}
;a._BinaryenArrayNewGetSize=function(){return(a._BinaryenArrayNewGetSize=a.asm.tu).apply(null,arguments)}
;a._BinaryenArrayNewSetSize=function(){return(a._BinaryenArrayNewSetSize=a.asm.uu).apply(null,arguments)}
;a._BinaryenArrayInitGetNumValues=function(){return(a._BinaryenArrayInitGetNumValues=a.asm.vu).apply(null,arguments)}
;
a._BinaryenArrayInitGetValueAt=function(){return(a._BinaryenArrayInitGetValueAt=a.asm.wu).apply(null,arguments)}
;a._BinaryenArrayInitSetValueAt=function(){return(a._BinaryenArrayInitSetValueAt=a.asm.xu).apply(null,arguments)}
;a._BinaryenArrayInitAppendValue=function(){return(a._BinaryenArrayInitAppendValue=a.asm.yu).apply(null,arguments)}
;a._BinaryenArrayInitInsertValueAt=function(){return(a._BinaryenArrayInitInsertValueAt=a.asm.zu).apply(null,arguments)}
;
a._BinaryenArrayInitRemoveValueAt=function(){return(a._BinaryenArrayInitRemoveValueAt=a.asm.Au).apply(null,arguments)}
;a._BinaryenArrayGetGetRef=function(){return(a._BinaryenArrayGetGetRef=a.asm.Bu).apply(null,arguments)}
;a._BinaryenArrayGetSetRef=function(){return(a._BinaryenArrayGetSetRef=a.asm.Cu).apply(null,arguments)}
;a._BinaryenArrayGetGetIndex=function(){return(a._BinaryenArrayGetGetIndex=a.asm.Du).apply(null,arguments)}
;
a._BinaryenArrayGetSetIndex=function(){return(a._BinaryenArrayGetSetIndex=a.asm.Eu).apply(null,arguments)}
;a._BinaryenArrayGetIsSigned=function(){return(a._BinaryenArrayGetIsSigned=a.asm.Fu).apply(null,arguments)}
;a._BinaryenArrayGetSetSigned=function(){return(a._BinaryenArrayGetSetSigned=a.asm.Gu).apply(null,arguments)}
;a._BinaryenArraySetGetRef=function(){return(a._BinaryenArraySetGetRef=a.asm.Hu).apply(null,arguments)}
;
a._BinaryenArraySetSetRef=function(){return(a._BinaryenArraySetSetRef=a.asm.Iu).apply(null,arguments)}
;a._BinaryenArraySetGetIndex=function(){return(a._BinaryenArraySetGetIndex=a.asm.Ju).apply(null,arguments)}
;a._BinaryenArraySetSetIndex=function(){return(a._BinaryenArraySetSetIndex=a.asm.Ku).apply(null,arguments)}
;a._BinaryenArraySetGetValue=function(){return(a._BinaryenArraySetGetValue=a.asm.Lu).apply(null,arguments)}
;
a._BinaryenArraySetSetValue=function(){return(a._BinaryenArraySetSetValue=a.asm.Mu).apply(null,arguments)}
;a._BinaryenArrayLenGetRef=function(){return(a._BinaryenArrayLenGetRef=a.asm.Nu).apply(null,arguments)}
;a._BinaryenArrayLenSetRef=function(){return(a._BinaryenArrayLenSetRef=a.asm.Ou).apply(null,arguments)}
;a._BinaryenArrayCopyGetDestRef=function(){return(a._BinaryenArrayCopyGetDestRef=a.asm.Pu).apply(null,arguments)}
;
a._BinaryenArrayCopySetDestRef=function(){return(a._BinaryenArrayCopySetDestRef=a.asm.Qu).apply(null,arguments)}
;a._BinaryenArrayCopyGetDestIndex=function(){return(a._BinaryenArrayCopyGetDestIndex=a.asm.Ru).apply(null,arguments)}
;a._BinaryenArrayCopySetDestIndex=function(){return(a._BinaryenArrayCopySetDestIndex=a.asm.Su).apply(null,arguments)}
;a._BinaryenArrayCopyGetSrcRef=function(){return(a._BinaryenArrayCopyGetSrcRef=a.asm.Tu).apply(null,arguments)}
;
a._BinaryenArrayCopySetSrcRef=function(){return(a._BinaryenArrayCopySetSrcRef=a.asm.Uu).apply(null,arguments)}
;a._BinaryenArrayCopyGetSrcIndex=function(){return(a._BinaryenArrayCopyGetSrcIndex=a.asm.Vu).apply(null,arguments)}
;a._BinaryenArrayCopySetSrcIndex=function(){return(a._BinaryenArrayCopySetSrcIndex=a.asm.Wu).apply(null,arguments)}
;a._BinaryenArrayCopyGetLength=function(){return(a._BinaryenArrayCopyGetLength=a.asm.Xu).apply(null,arguments)}
;
a._BinaryenArrayCopySetLength=function(){return(a._BinaryenArrayCopySetLength=a.asm.Yu).apply(null,arguments)}
;a._BinaryenStringNewGetOp=function(){return(a._BinaryenStringNewGetOp=a.asm.Zu).apply(null,arguments)}
;a._BinaryenStringNewSetOp=function(){return(a._BinaryenStringNewSetOp=a.asm._u).apply(null,arguments)}
;a._BinaryenStringNewGetPtr=function(){return(a._BinaryenStringNewGetPtr=a.asm.$u).apply(null,arguments)}
;
a._BinaryenStringNewSetPtr=function(){return(a._BinaryenStringNewSetPtr=a.asm.av).apply(null,arguments)}
;a._BinaryenStringNewGetLength=function(){return(a._BinaryenStringNewGetLength=a.asm.bv).apply(null,arguments)}
;a._BinaryenStringNewSetLength=function(){return(a._BinaryenStringNewSetLength=a.asm.cv).apply(null,arguments)}
;a._BinaryenStringNewGetStart=function(){return(a._BinaryenStringNewGetStart=a.asm.dv).apply(null,arguments)}
;
a._BinaryenStringNewSetStart=function(){return(a._BinaryenStringNewSetStart=a.asm.ev).apply(null,arguments)}
;a._BinaryenStringNewGetEnd=function(){return(a._BinaryenStringNewGetEnd=a.asm.fv).apply(null,arguments)}
;a._BinaryenStringNewSetEnd=function(){return(a._BinaryenStringNewSetEnd=a.asm.gv).apply(null,arguments)}
;a._BinaryenStringConstGetString=function(){return(a._BinaryenStringConstGetString=a.asm.hv).apply(null,arguments)}
;
a._BinaryenStringConstSetString=function(){return(a._BinaryenStringConstSetString=a.asm.iv).apply(null,arguments)}
;a._BinaryenStringMeasureGetOp=function(){return(a._BinaryenStringMeasureGetOp=a.asm.jv).apply(null,arguments)}
;a._BinaryenStringMeasureSetOp=function(){return(a._BinaryenStringMeasureSetOp=a.asm.kv).apply(null,arguments)}
;a._BinaryenStringMeasureGetRef=function(){return(a._BinaryenStringMeasureGetRef=a.asm.lv).apply(null,arguments)}
;
a._BinaryenStringMeasureSetRef=function(){return(a._BinaryenStringMeasureSetRef=a.asm.mv).apply(null,arguments)}
;a._BinaryenStringEncodeGetOp=function(){return(a._BinaryenStringEncodeGetOp=a.asm.nv).apply(null,arguments)}
;a._BinaryenStringEncodeSetOp=function(){return(a._BinaryenStringEncodeSetOp=a.asm.ov).apply(null,arguments)}
;a._BinaryenStringEncodeGetRef=function(){return(a._BinaryenStringEncodeGetRef=a.asm.pv).apply(null,arguments)}
;
a._BinaryenStringEncodeSetRef=function(){return(a._BinaryenStringEncodeSetRef=a.asm.qv).apply(null,arguments)}
;a._BinaryenStringEncodeGetPtr=function(){return(a._BinaryenStringEncodeGetPtr=a.asm.rv).apply(null,arguments)}
;a._BinaryenStringEncodeSetPtr=function(){return(a._BinaryenStringEncodeSetPtr=a.asm.sv).apply(null,arguments)}
;a._BinaryenStringEncodeGetStart=function(){return(a._BinaryenStringEncodeGetStart=a.asm.tv).apply(null,arguments)}
;
a._BinaryenStringEncodeSetStart=function(){return(a._BinaryenStringEncodeSetStart=a.asm.uv).apply(null,arguments)}
;a._BinaryenStringConcatGetLeft=function(){return(a._BinaryenStringConcatGetLeft=a.asm.vv).apply(null,arguments)}
;a._BinaryenStringConcatSetLeft=function(){return(a._BinaryenStringConcatSetLeft=a.asm.wv).apply(null,arguments)}
;a._BinaryenStringConcatGetRight=function(){return(a._BinaryenStringConcatGetRight=a.asm.xv).apply(null,arguments)}
;
a._BinaryenStringConcatSetRight=function(){return(a._BinaryenStringConcatSetRight=a.asm.yv).apply(null,arguments)}
;a._BinaryenStringEqGetLeft=function(){return(a._BinaryenStringEqGetLeft=a.asm.zv).apply(null,arguments)}
;a._BinaryenStringEqSetLeft=function(){return(a._BinaryenStringEqSetLeft=a.asm.Av).apply(null,arguments)}
;a._BinaryenStringEqGetRight=function(){return(a._BinaryenStringEqGetRight=a.asm.Bv).apply(null,arguments)}
;
a._BinaryenStringEqSetRight=function(){return(a._BinaryenStringEqSetRight=a.asm.Cv).apply(null,arguments)}
;a._BinaryenStringAsGetOp=function(){return(a._BinaryenStringAsGetOp=a.asm.Dv).apply(null,arguments)}
;a._BinaryenStringAsSetOp=function(){return(a._BinaryenStringAsSetOp=a.asm.Ev).apply(null,arguments)}
;a._BinaryenStringAsGetRef=function(){return(a._BinaryenStringAsGetRef=a.asm.Fv).apply(null,arguments)}
;
a._BinaryenStringAsSetRef=function(){return(a._BinaryenStringAsSetRef=a.asm.Gv).apply(null,arguments)}
;a._BinaryenStringWTF8AdvanceGetRef=function(){return(a._BinaryenStringWTF8AdvanceGetRef=a.asm.Hv).apply(null,arguments)}
;a._BinaryenStringWTF8AdvanceSetRef=function(){return(a._BinaryenStringWTF8AdvanceSetRef=a.asm.Iv).apply(null,arguments)}
;a._BinaryenStringWTF8AdvanceGetPos=function(){return(a._BinaryenStringWTF8AdvanceGetPos=a.asm.Jv).apply(null,arguments)}
;
a._BinaryenStringWTF8AdvanceSetPos=function(){return(a._BinaryenStringWTF8AdvanceSetPos=a.asm.Kv).apply(null,arguments)}
;a._BinaryenStringWTF8AdvanceGetBytes=function(){return(a._BinaryenStringWTF8AdvanceGetBytes=a.asm.Lv).apply(null,arguments)}
;a._BinaryenStringWTF8AdvanceSetBytes=function(){return(a._BinaryenStringWTF8AdvanceSetBytes=a.asm.Mv).apply(null,arguments)}
;a._BinaryenStringWTF16GetGetRef=function(){return(a._BinaryenStringWTF16GetGetRef=a.asm.Nv).apply(null,arguments)}
;
a._BinaryenStringWTF16GetSetRef=function(){return(a._BinaryenStringWTF16GetSetRef=a.asm.Ov).apply(null,arguments)}
;a._BinaryenStringWTF16GetGetPos=function(){return(a._BinaryenStringWTF16GetGetPos=a.asm.Pv).apply(null,arguments)}
;a._BinaryenStringWTF16GetSetPos=function(){return(a._BinaryenStringWTF16GetSetPos=a.asm.Qv).apply(null,arguments)}
;a._BinaryenStringIterNextGetRef=function(){return(a._BinaryenStringIterNextGetRef=a.asm.Rv).apply(null,arguments)}
;
a._BinaryenStringIterNextSetRef=function(){return(a._BinaryenStringIterNextSetRef=a.asm.Sv).apply(null,arguments)}
;a._BinaryenStringIterMoveGetOp=function(){return(a._BinaryenStringIterMoveGetOp=a.asm.Tv).apply(null,arguments)}
;a._BinaryenStringIterMoveSetOp=function(){return(a._BinaryenStringIterMoveSetOp=a.asm.Uv).apply(null,arguments)}
;a._BinaryenStringIterMoveGetRef=function(){return(a._BinaryenStringIterMoveGetRef=a.asm.Vv).apply(null,arguments)}
;
a._BinaryenStringIterMoveSetRef=function(){return(a._BinaryenStringIterMoveSetRef=a.asm.Wv).apply(null,arguments)}
;a._BinaryenStringIterMoveGetNum=function(){return(a._BinaryenStringIterMoveGetNum=a.asm.Xv).apply(null,arguments)}
;a._BinaryenStringIterMoveSetNum=function(){return(a._BinaryenStringIterMoveSetNum=a.asm.Yv).apply(null,arguments)}
;a._BinaryenStringSliceWTFGetOp=function(){return(a._BinaryenStringSliceWTFGetOp=a.asm.Zv).apply(null,arguments)}
;
a._BinaryenStringSliceWTFSetOp=function(){return(a._BinaryenStringSliceWTFSetOp=a.asm._v).apply(null,arguments)}
;a._BinaryenStringSliceWTFGetRef=function(){return(a._BinaryenStringSliceWTFGetRef=a.asm.$v).apply(null,arguments)}
;a._BinaryenStringSliceWTFSetRef=function(){return(a._BinaryenStringSliceWTFSetRef=a.asm.aw).apply(null,arguments)}
;a._BinaryenStringSliceWTFGetStart=function(){return(a._BinaryenStringSliceWTFGetStart=a.asm.bw).apply(null,arguments)}
;
a._BinaryenStringSliceWTFSetStart=function(){return(a._BinaryenStringSliceWTFSetStart=a.asm.cw).apply(null,arguments)}
;a._BinaryenStringSliceWTFGetEnd=function(){return(a._BinaryenStringSliceWTFGetEnd=a.asm.dw).apply(null,arguments)}
;a._BinaryenStringSliceWTFSetEnd=function(){return(a._BinaryenStringSliceWTFSetEnd=a.asm.ew).apply(null,arguments)}
;a._BinaryenStringSliceIterGetRef=function(){return(a._BinaryenStringSliceIterGetRef=a.asm.fw).apply(null,arguments)}
;
a._BinaryenStringSliceIterSetRef=function(){return(a._BinaryenStringSliceIterSetRef=a.asm.gw).apply(null,arguments)}
;a._BinaryenStringSliceIterGetNum=function(){return(a._BinaryenStringSliceIterGetNum=a.asm.hw).apply(null,arguments)}
;a._BinaryenStringSliceIterSetNum=function(){return(a._BinaryenStringSliceIterSetNum=a.asm.iw).apply(null,arguments)}
;a._BinaryenAddFunction=function(){return(a._BinaryenAddFunction=a.asm.jw).apply(null,arguments)}
;
a._BinaryenGetFunction=function(){return(a._BinaryenGetFunction=a.asm.kw).apply(null,arguments)}
;a._BinaryenRemoveFunction=function(){return(a._BinaryenRemoveFunction=a.asm.lw).apply(null,arguments)}
;a._BinaryenGetNumFunctions=function(){return(a._BinaryenGetNumFunctions=a.asm.mw).apply(null,arguments)}
;a._BinaryenGetFunctionByIndex=function(){return(a._BinaryenGetFunctionByIndex=a.asm.nw).apply(null,arguments)}
;a._BinaryenAddGlobal=function(){return(a._BinaryenAddGlobal=a.asm.ow).apply(null,arguments)}
;
a._BinaryenGetGlobal=function(){return(a._BinaryenGetGlobal=a.asm.pw).apply(null,arguments)}
;a._BinaryenRemoveGlobal=function(){return(a._BinaryenRemoveGlobal=a.asm.qw).apply(null,arguments)}
;a._BinaryenGetNumGlobals=function(){return(a._BinaryenGetNumGlobals=a.asm.rw).apply(null,arguments)}
;a._BinaryenGetGlobalByIndex=function(){return(a._BinaryenGetGlobalByIndex=a.asm.sw).apply(null,arguments)}
;a._BinaryenAddTag=function(){return(a._BinaryenAddTag=a.asm.tw).apply(null,arguments)}
;
a._BinaryenGetTag=function(){return(a._BinaryenGetTag=a.asm.uw).apply(null,arguments)}
;a._BinaryenRemoveTag=function(){return(a._BinaryenRemoveTag=a.asm.vw).apply(null,arguments)}
;a._BinaryenAddFunctionImport=function(){return(a._BinaryenAddFunctionImport=a.asm.ww).apply(null,arguments)}
;a._BinaryenAddTableImport=function(){return(a._BinaryenAddTableImport=a.asm.xw).apply(null,arguments)}
;a._BinaryenAddMemoryImport=function(){return(a._BinaryenAddMemoryImport=a.asm.yw).apply(null,arguments)}
;
a._BinaryenAddGlobalImport=function(){return(a._BinaryenAddGlobalImport=a.asm.zw).apply(null,arguments)}
;a._BinaryenAddTagImport=function(){return(a._BinaryenAddTagImport=a.asm.Aw).apply(null,arguments)}
;a._BinaryenAddFunctionExport=function(){return(a._BinaryenAddFunctionExport=a.asm.Bw).apply(null,arguments)}
;a._BinaryenAddTableExport=function(){return(a._BinaryenAddTableExport=a.asm.Cw).apply(null,arguments)}
;
a._BinaryenAddMemoryExport=function(){return(a._BinaryenAddMemoryExport=a.asm.Dw).apply(null,arguments)}
;a._BinaryenAddGlobalExport=function(){return(a._BinaryenAddGlobalExport=a.asm.Ew).apply(null,arguments)}
;a._BinaryenAddTagExport=function(){return(a._BinaryenAddTagExport=a.asm.Fw).apply(null,arguments)}
;a._BinaryenGetExport=function(){return(a._BinaryenGetExport=a.asm.Gw).apply(null,arguments)}
;a._BinaryenRemoveExport=function(){return(a._BinaryenRemoveExport=a.asm.Hw).apply(null,arguments)}
;
a._BinaryenGetNumExports=function(){return(a._BinaryenGetNumExports=a.asm.Iw).apply(null,arguments)}
;a._BinaryenGetExportByIndex=function(){return(a._BinaryenGetExportByIndex=a.asm.Jw).apply(null,arguments)}
;a._BinaryenAddTable=function(){return(a._BinaryenAddTable=a.asm.Kw).apply(null,arguments)}
;a._BinaryenRemoveTable=function(){return(a._BinaryenRemoveTable=a.asm.Lw).apply(null,arguments)}
;a._BinaryenGetNumTables=function(){return(a._BinaryenGetNumTables=a.asm.Mw).apply(null,arguments)}
;
a._BinaryenGetTable=function(){return(a._BinaryenGetTable=a.asm.Nw).apply(null,arguments)}
;a._BinaryenGetTableByIndex=function(){return(a._BinaryenGetTableByIndex=a.asm.Ow).apply(null,arguments)}
;a._BinaryenAddActiveElementSegment=function(){return(a._BinaryenAddActiveElementSegment=a.asm.Pw).apply(null,arguments)}
;a._BinaryenAddPassiveElementSegment=function(){return(a._BinaryenAddPassiveElementSegment=a.asm.Qw).apply(null,arguments)}
;
a._BinaryenRemoveElementSegment=function(){return(a._BinaryenRemoveElementSegment=a.asm.Rw).apply(null,arguments)}
;a._BinaryenGetElementSegment=function(){return(a._BinaryenGetElementSegment=a.asm.Sw).apply(null,arguments)}
;a._BinaryenGetElementSegmentByIndex=function(){return(a._BinaryenGetElementSegmentByIndex=a.asm.Tw).apply(null,arguments)}
;a._BinaryenGetNumElementSegments=function(){return(a._BinaryenGetNumElementSegments=a.asm.Uw).apply(null,arguments)}
;
a._BinaryenElementSegmentGetOffset=function(){return(a._BinaryenElementSegmentGetOffset=a.asm.Vw).apply(null,arguments)}
;a._BinaryenElementSegmentGetLength=function(){return(a._BinaryenElementSegmentGetLength=a.asm.Ww).apply(null,arguments)}
;a._BinaryenElementSegmentGetData=function(){return(a._BinaryenElementSegmentGetData=a.asm.Xw).apply(null,arguments)}
;a._BinaryenSetMemory=function(){return(a._BinaryenSetMemory=a.asm.Yw).apply(null,arguments)}
;
a._BinaryenGetNumMemorySegments=function(){return(a._BinaryenGetNumMemorySegments=a.asm.Zw).apply(null,arguments)}
;a._BinaryenGetMemorySegmentByteOffset=function(){return(a._BinaryenGetMemorySegmentByteOffset=a.asm._w).apply(null,arguments)}
;a._BinaryenHasMemory=function(){return(a._BinaryenHasMemory=a.asm.$w).apply(null,arguments)}
;a._BinaryenMemoryGetInitial=function(){return(a._BinaryenMemoryGetInitial=a.asm.ax).apply(null,arguments)}
;
a._BinaryenMemoryHasMax=function(){return(a._BinaryenMemoryHasMax=a.asm.bx).apply(null,arguments)}
;a._BinaryenMemoryGetMax=function(){return(a._BinaryenMemoryGetMax=a.asm.cx).apply(null,arguments)}
;a._BinaryenMemoryImportGetModule=function(){return(a._BinaryenMemoryImportGetModule=a.asm.dx).apply(null,arguments)}
;a._BinaryenMemoryImportGetBase=function(){return(a._BinaryenMemoryImportGetBase=a.asm.ex).apply(null,arguments)}
;
a._BinaryenMemoryIsShared=function(){return(a._BinaryenMemoryIsShared=a.asm.fx).apply(null,arguments)}
;a._BinaryenMemoryIs64=function(){return(a._BinaryenMemoryIs64=a.asm.gx).apply(null,arguments)}
;a._BinaryenGetMemorySegmentByteLength=function(){return(a._BinaryenGetMemorySegmentByteLength=a.asm.hx).apply(null,arguments)}
;a._BinaryenGetMemorySegmentPassive=function(){return(a._BinaryenGetMemorySegmentPassive=a.asm.ix).apply(null,arguments)}
;
a._BinaryenCopyMemorySegmentData=function(){return(a._BinaryenCopyMemorySegmentData=a.asm.jx).apply(null,arguments)}
;a._BinaryenSetStart=function(){return(a._BinaryenSetStart=a.asm.kx).apply(null,arguments)}
;a._BinaryenModuleGetFeatures=function(){return(a._BinaryenModuleGetFeatures=a.asm.lx).apply(null,arguments)}
;a._BinaryenModuleSetFeatures=function(){return(a._BinaryenModuleSetFeatures=a.asm.mx).apply(null,arguments)}
;
a._BinaryenModuleParse=function(){return(a._BinaryenModuleParse=a.asm.nx).apply(null,arguments)}
;a._BinaryenModulePrint=function(){return(a._BinaryenModulePrint=a.asm.ox).apply(null,arguments)}
;a._BinaryenModulePrintStackIR=function(){return(a._BinaryenModulePrintStackIR=a.asm.px).apply(null,arguments)}
;a._BinaryenModulePrintAsmjs=function(){return(a._BinaryenModulePrintAsmjs=a.asm.qx).apply(null,arguments)}
;
a._BinaryenModuleValidate=function(){return(a._BinaryenModuleValidate=a.asm.rx).apply(null,arguments)}
;a._BinaryenModuleOptimize=function(){return(a._BinaryenModuleOptimize=a.asm.sx).apply(null,arguments)}
;a._BinaryenModuleUpdateMaps=function(){return(a._BinaryenModuleUpdateMaps=a.asm.tx).apply(null,arguments)}
;a._BinaryenGetOptimizeLevel=function(){return(a._BinaryenGetOptimizeLevel=a.asm.ux).apply(null,arguments)}
;
a._BinaryenSetOptimizeLevel=function(){return(a._BinaryenSetOptimizeLevel=a.asm.vx).apply(null,arguments)}
;a._BinaryenGetShrinkLevel=function(){return(a._BinaryenGetShrinkLevel=a.asm.wx).apply(null,arguments)}
;a._BinaryenSetShrinkLevel=function(){return(a._BinaryenSetShrinkLevel=a.asm.xx).apply(null,arguments)}
;a._BinaryenGetDebugInfo=function(){return(a._BinaryenGetDebugInfo=a.asm.yx).apply(null,arguments)}
;a._BinaryenSetDebugInfo=function(){return(a._BinaryenSetDebugInfo=a.asm.zx).apply(null,arguments)}
;
a._BinaryenGetLowMemoryUnused=function(){return(a._BinaryenGetLowMemoryUnused=a.asm.Ax).apply(null,arguments)}
;a._BinaryenSetLowMemoryUnused=function(){return(a._BinaryenSetLowMemoryUnused=a.asm.Bx).apply(null,arguments)}
;a._BinaryenGetZeroFilledMemory=function(){return(a._BinaryenGetZeroFilledMemory=a.asm.Cx).apply(null,arguments)}
;a._BinaryenSetZeroFilledMemory=function(){return(a._BinaryenSetZeroFilledMemory=a.asm.Dx).apply(null,arguments)}
;
a._BinaryenGetFastMath=function(){return(a._BinaryenGetFastMath=a.asm.Ex).apply(null,arguments)}
;a._BinaryenSetFastMath=function(){return(a._BinaryenSetFastMath=a.asm.Fx).apply(null,arguments)}
;a._BinaryenGetPassArgument=function(){return(a._BinaryenGetPassArgument=a.asm.Gx).apply(null,arguments)}
;a._BinaryenSetPassArgument=function(){return(a._BinaryenSetPassArgument=a.asm.Hx).apply(null,arguments)}
;
a._BinaryenClearPassArguments=function(){return(a._BinaryenClearPassArguments=a.asm.Ix).apply(null,arguments)}
;a._BinaryenGetAlwaysInlineMaxSize=function(){return(a._BinaryenGetAlwaysInlineMaxSize=a.asm.Jx).apply(null,arguments)}
;a._BinaryenSetAlwaysInlineMaxSize=function(){return(a._BinaryenSetAlwaysInlineMaxSize=a.asm.Kx).apply(null,arguments)}
;a._BinaryenGetFlexibleInlineMaxSize=function(){return(a._BinaryenGetFlexibleInlineMaxSize=a.asm.Lx).apply(null,arguments)}
;
a._BinaryenSetFlexibleInlineMaxSize=function(){return(a._BinaryenSetFlexibleInlineMaxSize=a.asm.Mx).apply(null,arguments)}
;a._BinaryenGetOneCallerInlineMaxSize=function(){return(a._BinaryenGetOneCallerInlineMaxSize=a.asm.Nx).apply(null,arguments)}
;a._BinaryenSetOneCallerInlineMaxSize=function(){return(a._BinaryenSetOneCallerInlineMaxSize=a.asm.Ox).apply(null,arguments)}
;
a._BinaryenGetAllowInliningFunctionsWithLoops=function(){return(a._BinaryenGetAllowInliningFunctionsWithLoops=a.asm.Px).apply(null,arguments)}
;a._BinaryenSetAllowInliningFunctionsWithLoops=function(){return(a._BinaryenSetAllowInliningFunctionsWithLoops=a.asm.Qx).apply(null,arguments)}
;a._BinaryenModuleRunPasses=function(){return(a._BinaryenModuleRunPasses=a.asm.Rx).apply(null,arguments)}
;a._BinaryenModuleAutoDrop=function(){return(a._BinaryenModuleAutoDrop=a.asm.Sx).apply(null,arguments)}
;
a._BinaryenModuleWrite=function(){return(a._BinaryenModuleWrite=a.asm.Tx).apply(null,arguments)}
;a._BinaryenModuleWriteText=function(){return(a._BinaryenModuleWriteText=a.asm.Ux).apply(null,arguments)}
;a._BinaryenModuleWriteStackIR=function(){return(a._BinaryenModuleWriteStackIR=a.asm.Vx).apply(null,arguments)}
;a._BinaryenModuleWriteWithSourceMap=function(){return(a._BinaryenModuleWriteWithSourceMap=a.asm.Wx).apply(null,arguments)}
;
a._BinaryenModuleAllocateAndWrite=function(){return(a._BinaryenModuleAllocateAndWrite=a.asm.Xx).apply(null,arguments)}
;var dc=a._malloc=function(){return(dc=a._malloc=a.asm.Yx).apply(null,arguments)}
;a._BinaryenModuleAllocateAndWriteText=function(){return(a._BinaryenModuleAllocateAndWriteText=a.asm.Zx).apply(null,arguments)}
;a._BinaryenModuleAllocateAndWriteStackIR=function(){return(a._BinaryenModuleAllocateAndWriteStackIR=a.asm._x).apply(null,arguments)}
;
a._BinaryenModuleRead=function(){return(a._BinaryenModuleRead=a.asm.$x).apply(null,arguments)}
;a._BinaryenModuleInterpret=function(){return(a._BinaryenModuleInterpret=a.asm.ay).apply(null,arguments)}
;a._BinaryenModuleAddDebugInfoFileName=function(){return(a._BinaryenModuleAddDebugInfoFileName=a.asm.by).apply(null,arguments)}
;a._BinaryenModuleGetDebugInfoFileName=function(){return(a._BinaryenModuleGetDebugInfoFileName=a.asm.cy).apply(null,arguments)}
;
a._BinaryenFunctionGetName=function(){return(a._BinaryenFunctionGetName=a.asm.dy).apply(null,arguments)}
;a._BinaryenFunctionGetParams=function(){return(a._BinaryenFunctionGetParams=a.asm.ey).apply(null,arguments)}
;a._BinaryenFunctionGetResults=function(){return(a._BinaryenFunctionGetResults=a.asm.fy).apply(null,arguments)}
;a._BinaryenFunctionGetNumVars=function(){return(a._BinaryenFunctionGetNumVars=a.asm.gy).apply(null,arguments)}
;
a._BinaryenFunctionGetVar=function(){return(a._BinaryenFunctionGetVar=a.asm.hy).apply(null,arguments)}
;a._BinaryenFunctionGetNumLocals=function(){return(a._BinaryenFunctionGetNumLocals=a.asm.iy).apply(null,arguments)}
;a._BinaryenFunctionHasLocalName=function(){return(a._BinaryenFunctionHasLocalName=a.asm.jy).apply(null,arguments)}
;a._BinaryenFunctionGetLocalName=function(){return(a._BinaryenFunctionGetLocalName=a.asm.ky).apply(null,arguments)}
;
a._BinaryenFunctionSetLocalName=function(){return(a._BinaryenFunctionSetLocalName=a.asm.ly).apply(null,arguments)}
;a._BinaryenFunctionGetBody=function(){return(a._BinaryenFunctionGetBody=a.asm.my).apply(null,arguments)}
;a._BinaryenFunctionSetBody=function(){return(a._BinaryenFunctionSetBody=a.asm.ny).apply(null,arguments)}
;a._BinaryenFunctionOptimize=function(){return(a._BinaryenFunctionOptimize=a.asm.oy).apply(null,arguments)}
;
a._BinaryenFunctionRunPasses=function(){return(a._BinaryenFunctionRunPasses=a.asm.py).apply(null,arguments)}
;a._BinaryenFunctionSetDebugLocation=function(){return(a._BinaryenFunctionSetDebugLocation=a.asm.qy).apply(null,arguments)}
;a._BinaryenTableGetName=function(){return(a._BinaryenTableGetName=a.asm.ry).apply(null,arguments)}
;a._BinaryenTableSetName=function(){return(a._BinaryenTableSetName=a.asm.sy).apply(null,arguments)}
;
a._BinaryenTableGetInitial=function(){return(a._BinaryenTableGetInitial=a.asm.ty).apply(null,arguments)}
;a._BinaryenTableSetInitial=function(){return(a._BinaryenTableSetInitial=a.asm.uy).apply(null,arguments)}
;a._BinaryenTableHasMax=function(){return(a._BinaryenTableHasMax=a.asm.vy).apply(null,arguments)}
;a._BinaryenTableGetMax=function(){return(a._BinaryenTableGetMax=a.asm.wy).apply(null,arguments)}
;a._BinaryenTableSetMax=function(){return(a._BinaryenTableSetMax=a.asm.xy).apply(null,arguments)}
;
a._BinaryenElementSegmentGetName=function(){return(a._BinaryenElementSegmentGetName=a.asm.yy).apply(null,arguments)}
;a._BinaryenElementSegmentSetName=function(){return(a._BinaryenElementSegmentSetName=a.asm.zy).apply(null,arguments)}
;a._BinaryenElementSegmentGetTable=function(){return(a._BinaryenElementSegmentGetTable=a.asm.Ay).apply(null,arguments)}
;a._BinaryenElementSegmentSetTable=function(){return(a._BinaryenElementSegmentSetTable=a.asm.By).apply(null,arguments)}
;
a._BinaryenElementSegmentIsPassive=function(){return(a._BinaryenElementSegmentIsPassive=a.asm.Cy).apply(null,arguments)}
;a._BinaryenGlobalGetName=function(){return(a._BinaryenGlobalGetName=a.asm.Dy).apply(null,arguments)}
;a._BinaryenGlobalGetType=function(){return(a._BinaryenGlobalGetType=a.asm.Ey).apply(null,arguments)}
;a._BinaryenGlobalIsMutable=function(){return(a._BinaryenGlobalIsMutable=a.asm.Fy).apply(null,arguments)}
;
a._BinaryenGlobalGetInitExpr=function(){return(a._BinaryenGlobalGetInitExpr=a.asm.Gy).apply(null,arguments)}
;a._BinaryenTagGetName=function(){return(a._BinaryenTagGetName=a.asm.Hy).apply(null,arguments)}
;a._BinaryenTagGetParams=function(){return(a._BinaryenTagGetParams=a.asm.Iy).apply(null,arguments)}
;a._BinaryenTagGetResults=function(){return(a._BinaryenTagGetResults=a.asm.Jy).apply(null,arguments)}
;
a._BinaryenFunctionImportGetModule=function(){return(a._BinaryenFunctionImportGetModule=a.asm.Ky).apply(null,arguments)}
;a._BinaryenTableImportGetModule=function(){return(a._BinaryenTableImportGetModule=a.asm.Ly).apply(null,arguments)}
;a._BinaryenGlobalImportGetModule=function(){return(a._BinaryenGlobalImportGetModule=a.asm.My).apply(null,arguments)}
;a._BinaryenTagImportGetModule=function(){return(a._BinaryenTagImportGetModule=a.asm.Ny).apply(null,arguments)}
;
a._BinaryenFunctionImportGetBase=function(){return(a._BinaryenFunctionImportGetBase=a.asm.Oy).apply(null,arguments)}
;a._BinaryenTableImportGetBase=function(){return(a._BinaryenTableImportGetBase=a.asm.Py).apply(null,arguments)}
;a._BinaryenGlobalImportGetBase=function(){return(a._BinaryenGlobalImportGetBase=a.asm.Qy).apply(null,arguments)}
;a._BinaryenTagImportGetBase=function(){return(a._BinaryenTagImportGetBase=a.asm.Ry).apply(null,arguments)}
;
a._BinaryenExportGetKind=function(){return(a._BinaryenExportGetKind=a.asm.Sy).apply(null,arguments)}
;a._BinaryenExportGetName=function(){return(a._BinaryenExportGetName=a.asm.Ty).apply(null,arguments)}
;a._BinaryenExportGetValue=function(){return(a._BinaryenExportGetValue=a.asm.Uy).apply(null,arguments)}
;a._BinaryenAddCustomSection=function(){return(a._BinaryenAddCustomSection=a.asm.Vy).apply(null,arguments)}
;
a._BinaryenSideEffectNone=function(){return(a._BinaryenSideEffectNone=a.asm.Wy).apply(null,arguments)}
;a._BinaryenSideEffectBranches=function(){return(a._BinaryenSideEffectBranches=a.asm.Xy).apply(null,arguments)}
;a._BinaryenSideEffectCalls=function(){return(a._BinaryenSideEffectCalls=a.asm.Yy).apply(null,arguments)}
;a._BinaryenSideEffectReadsLocal=function(){return(a._BinaryenSideEffectReadsLocal=a.asm.Zy).apply(null,arguments)}
;
a._BinaryenSideEffectWritesLocal=function(){return(a._BinaryenSideEffectWritesLocal=a.asm._y).apply(null,arguments)}
;a._BinaryenSideEffectReadsGlobal=function(){return(a._BinaryenSideEffectReadsGlobal=a.asm.$y).apply(null,arguments)}
;a._BinaryenSideEffectWritesGlobal=function(){return(a._BinaryenSideEffectWritesGlobal=a.asm.az).apply(null,arguments)}
;a._BinaryenSideEffectReadsMemory=function(){return(a._BinaryenSideEffectReadsMemory=a.asm.bz).apply(null,arguments)}
;
a._BinaryenSideEffectWritesMemory=function(){return(a._BinaryenSideEffectWritesMemory=a.asm.cz).apply(null,arguments)}
;a._BinaryenSideEffectReadsTable=function(){return(a._BinaryenSideEffectReadsTable=a.asm.dz).apply(null,arguments)}
;a._BinaryenSideEffectWritesTable=function(){return(a._BinaryenSideEffectWritesTable=a.asm.ez).apply(null,arguments)}
;a._BinaryenSideEffectImplicitTrap=function(){return(a._BinaryenSideEffectImplicitTrap=a.asm.fz).apply(null,arguments)}
;
a._BinaryenSideEffectTrapsNeverHappen=function(){return(a._BinaryenSideEffectTrapsNeverHappen=a.asm.gz).apply(null,arguments)}
;a._BinaryenSideEffectIsAtomic=function(){return(a._BinaryenSideEffectIsAtomic=a.asm.hz).apply(null,arguments)}
;a._BinaryenSideEffectThrows=function(){return(a._BinaryenSideEffectThrows=a.asm.iz).apply(null,arguments)}
;a._BinaryenSideEffectDanglingPop=function(){return(a._BinaryenSideEffectDanglingPop=a.asm.jz).apply(null,arguments)}
;
a._BinaryenSideEffectAny=function(){return(a._BinaryenSideEffectAny=a.asm.kz).apply(null,arguments)}
;a._BinaryenExpressionGetSideEffects=function(){return(a._BinaryenExpressionGetSideEffects=a.asm.lz).apply(null,arguments)}
;a._RelooperCreate=function(){return(a._RelooperCreate=a.asm.mz).apply(null,arguments)}
;a._RelooperAddBlock=function(){return(a._RelooperAddBlock=a.asm.nz).apply(null,arguments)}
;a._RelooperAddBranch=function(){return(a._RelooperAddBranch=a.asm.oz).apply(null,arguments)}
;
a._RelooperAddBlockWithSwitch=function(){return(a._RelooperAddBlockWithSwitch=a.asm.pz).apply(null,arguments)}
;a._RelooperAddBranchForSwitch=function(){return(a._RelooperAddBranchForSwitch=a.asm.qz).apply(null,arguments)}
;a._RelooperRenderAndDispose=function(){return(a._RelooperRenderAndDispose=a.asm.rz).apply(null,arguments)}
;a._ExpressionRunnerFlagsDefault=function(){return(a._ExpressionRunnerFlagsDefault=a.asm.sz).apply(null,arguments)}
;
a._ExpressionRunnerFlagsPreserveSideeffects=function(){return(a._ExpressionRunnerFlagsPreserveSideeffects=a.asm.tz).apply(null,arguments)}
;a._ExpressionRunnerFlagsTraverseCalls=function(){return(a._ExpressionRunnerFlagsTraverseCalls=a.asm.uz).apply(null,arguments)}
;a._ExpressionRunnerCreate=function(){return(a._ExpressionRunnerCreate=a.asm.vz).apply(null,arguments)}
;a._ExpressionRunnerSetLocalValue=function(){return(a._ExpressionRunnerSetLocalValue=a.asm.wz).apply(null,arguments)}
;
a._ExpressionRunnerSetGlobalValue=function(){return(a._ExpressionRunnerSetGlobalValue=a.asm.xz).apply(null,arguments)}
;a._ExpressionRunnerRunAndDispose=function(){return(a._ExpressionRunnerRunAndDispose=a.asm.yz).apply(null,arguments)}
;a._TypeBuilderErrorReasonSelfSupertype=function(){return(a._TypeBuilderErrorReasonSelfSupertype=a.asm.zz).apply(null,arguments)}
;a._TypeBuilderErrorReasonInvalidSupertype=function(){return(a._TypeBuilderErrorReasonInvalidSupertype=a.asm.Az).apply(null,arguments)}
;
a._TypeBuilderErrorReasonForwardSupertypeReference=function(){return(a._TypeBuilderErrorReasonForwardSupertypeReference=a.asm.Bz).apply(null,arguments)}
;a._TypeBuilderErrorReasonForwardChildReference=function(){return(a._TypeBuilderErrorReasonForwardChildReference=a.asm.Cz).apply(null,arguments)}
;a._TypeBuilderCreate=function(){return(a._TypeBuilderCreate=a.asm.Dz).apply(null,arguments)}
;a._TypeBuilderGrow=function(){return(a._TypeBuilderGrow=a.asm.Ez).apply(null,arguments)}
;
a._TypeBuilderGetSize=function(){return(a._TypeBuilderGetSize=a.asm.Fz).apply(null,arguments)}
;a._TypeBuilderSetBasicHeapType=function(){return(a._TypeBuilderSetBasicHeapType=a.asm.Gz).apply(null,arguments)}
;a._TypeBuilderSetSignatureType=function(){return(a._TypeBuilderSetSignatureType=a.asm.Hz).apply(null,arguments)}
;a._TypeBuilderSetStructType=function(){return(a._TypeBuilderSetStructType=a.asm.Iz).apply(null,arguments)}
;
a._TypeBuilderSetArrayType=function(){return(a._TypeBuilderSetArrayType=a.asm.Jz).apply(null,arguments)}
;a._TypeBuilderIsBasic=function(){return(a._TypeBuilderIsBasic=a.asm.Kz).apply(null,arguments)}
;a._TypeBuilderGetBasic=function(){return(a._TypeBuilderGetBasic=a.asm.Lz).apply(null,arguments)}
;a._TypeBuilderGetTempHeapType=function(){return(a._TypeBuilderGetTempHeapType=a.asm.Mz).apply(null,arguments)}
;
a._TypeBuilderGetTempTupleType=function(){return(a._TypeBuilderGetTempTupleType=a.asm.Nz).apply(null,arguments)}
;a._TypeBuilderGetTempRefType=function(){return(a._TypeBuilderGetTempRefType=a.asm.Oz).apply(null,arguments)}
;a._TypeBuilderSetSubType=function(){return(a._TypeBuilderSetSubType=a.asm.Pz).apply(null,arguments)}
;a._TypeBuilderCreateRecGroup=function(){return(a._TypeBuilderCreateRecGroup=a.asm.Qz).apply(null,arguments)}
;
a._TypeBuilderBuildAndDispose=function(){return(a._TypeBuilderBuildAndDispose=a.asm.Rz).apply(null,arguments)}
;a._BinaryenModuleSetTypeName=function(){return(a._BinaryenModuleSetTypeName=a.asm.Sz).apply(null,arguments)}
;a._BinaryenModuleSetFieldName=function(){return(a._BinaryenModuleSetFieldName=a.asm.Tz).apply(null,arguments)}
;a._BinaryenSetColorsEnabled=function(){return(a._BinaryenSetColorsEnabled=a.asm.Uz).apply(null,arguments)}
;
a._BinaryenAreColorsEnabled=function(){return(a._BinaryenAreColorsEnabled=a.asm.Vz).apply(null,arguments)}
;var pd=a._BinaryenSizeofLiteral=function(){return(pd=a._BinaryenSizeofLiteral=a.asm.Wz).apply(null,arguments)}
,qd=a._BinaryenSizeofAllocateAndWriteResult=function(){return(qd=a._BinaryenSizeofAllocateAndWriteResult=a.asm.Xz).apply(null,arguments)}
;a.__i32_store8=function(){return(a.__i32_store8=a.asm.Yz).apply(null,arguments)}
;
a.__i32_store16=function(){return(a.__i32_store16=a.asm.Zz).apply(null,arguments)}
;a.__i32_store=function(){return(a.__i32_store=a.asm._z).apply(null,arguments)}
;a.__f32_store=function(){return(a.__f32_store=a.asm.$z).apply(null,arguments)}
;a.__f64_store=function(){return(a.__f64_store=a.asm.aA).apply(null,arguments)}
;a.__i32_load8_s=function(){return(a.__i32_load8_s=a.asm.bA).apply(null,arguments)}
;a.__i32_load8_u=function(){return(a.__i32_load8_u=a.asm.cA).apply(null,arguments)}
;
a.__i32_load16_s=function(){return(a.__i32_load16_s=a.asm.dA).apply(null,arguments)}
;a.__i32_load16_u=function(){return(a.__i32_load16_u=a.asm.eA).apply(null,arguments)}
;a.__i32_load=function(){return(a.__i32_load=a.asm.fA).apply(null,arguments)}
;a.__f32_load=function(){return(a.__f32_load=a.asm.gA).apply(null,arguments)}
;a.__f64_load=function(){return(a.__f64_load=a.asm.hA).apply(null,arguments)}
;
var fc=a.___errno_location=function(){return(fc=a.___errno_location=a.asm.iA).apply(null,arguments)}
,P=a._setThrew=function(){return(P=a._setThrew=a.asm.kA).apply(null,arguments)}
,Q=a.setTempRet0=function(){return(Q=a.setTempRet0=a.asm.lA).apply(null,arguments)}
,R=a.stackSave=function(){return(R=a.stackSave=a.asm.mA).apply(null,arguments)}
,S=a.stackRestore=function(){return(S=a.stackRestore=a.asm.nA).apply(null,arguments)}
,O=a.stackAlloc=function(){return(O=a.stackAlloc=a.asm.oA).apply(null,arguments)}
,
ec=a.___cxa_can_catch=function(){return(ec=a.___cxa_can_catch=a.asm.pA).apply(null,arguments)}
,Ua=a.___cxa_is_pointer_type=function(){return(Ua=a.___cxa_is_pointer_type=a.asm.qA).apply(null,arguments)}
,rd=a.dynCall_iiiiij=function(){return(rd=a.dynCall_iiiiij=a.asm.rA).apply(null,arguments)}
,sd=a.dynCall_viij=function(){return(sd=a.dynCall_viij=a.asm.sA).apply(null,arguments)}
,td=a.dynCall_iij=function(){return(td=a.dynCall_iij=a.asm.tA).apply(null,arguments)}
,ud=a.dynCall_viiij=function(){return(ud=
a.dynCall_viiij=a.asm.uA).apply(null,arguments)}
,vd=a.dynCall_iiij=function(){return(vd=a.dynCall_iiij=a.asm.vA).apply(null,arguments)}
,wd=a.dynCall_viijj=function(){return(wd=a.dynCall_viijj=a.asm.wA).apply(null,arguments)}
,xd=a.dynCall_viiiiij=function(){return(xd=a.dynCall_viiiiij=a.asm.xA).apply(null,arguments)}
,yd=a.dynCall_ji=function(){return(yd=a.dynCall_ji=a.asm.yA).apply(null,arguments)}
,zd=a.dynCall_vij=function(){return(zd=a.dynCall_vij=a.asm.zA).apply(null,arguments)}
,Ad=a.dynCall_ijiii=
function(){return(Ad=a.dynCall_ijiii=a.asm.AA).apply(null,arguments)}
,Bd=a.dynCall_iiiji=function(){return(Bd=a.dynCall_iiiji=a.asm.BA).apply(null,arguments)}
,Cd=a.dynCall_viiji=function(){return(Cd=a.dynCall_viiji=a.asm.CA).apply(null,arguments)}
,Dd=a.dynCall_jii=function(){return(Dd=a.dynCall_jii=a.asm.DA).apply(null,arguments)}
,Ed=a.dynCall_vjii=function(){return(Ed=a.dynCall_vjii=a.asm.EA).apply(null,arguments)}
,Fd=a.dynCall_ijiiii=function(){return(Fd=a.dynCall_ijiiii=a.asm.FA).apply(null,arguments)}
,
Gd=a.dynCall_jiii=function(){return(Gd=a.dynCall_jiii=a.asm.GA).apply(null,arguments)}
,Hd=a.dynCall_iji=function(){return(Hd=a.dynCall_iji=a.asm.HA).apply(null,arguments)}
,Id=a.dynCall_iiijii=function(){return(Id=a.dynCall_iiijii=a.asm.IA).apply(null,arguments)}
,Jd=a.dynCall_iijj=function(){return(Jd=a.dynCall_iijj=a.asm.JA).apply(null,arguments)}
,Kd=a.dynCall_j=function(){return(Kd=a.dynCall_j=a.asm.KA).apply(null,arguments)}
,Ld=a.dynCall_vijii=function(){return(Ld=a.dynCall_vijii=a.asm.LA).apply(null,
arguments)}
,Md=a.dynCall_viji=function(){return(Md=a.dynCall_viji=a.asm.MA).apply(null,arguments)}
,Nd=a.dynCall_vijiii=function(){return(Nd=a.dynCall_vijiii=a.asm.NA).apply(null,arguments)}
,Od=a.dynCall_iiji=function(){return(Od=a.dynCall_iiji=a.asm.OA).apply(null,arguments)}
,Pd=a.dynCall_iijiiii=function(){return(Pd=a.dynCall_iijiiii=a.asm.PA).apply(null,arguments)}
,Qd=a.dynCall_iijiii=function(){return(Qd=a.dynCall_iijiii=a.asm.QA).apply(null,arguments)}
,Rd=a.dynCall_iijii=function(){return(Rd=
a.dynCall_iijii=a.asm.RA).apply(null,arguments)}
,Sd=a.dynCall_iiiiiiij=function(){return(Sd=a.dynCall_iiiiiiij=a.asm.SA).apply(null,arguments)}
,Td=a.dynCall_iijiiiij=function(){return(Td=a.dynCall_iijiiiij=a.asm.TA).apply(null,arguments)}
,Ud=a.dynCall_vijji=function(){return(Ud=a.dynCall_vijji=a.asm.UA).apply(null,arguments)}
,Vd=a.dynCall_viijiijj=function(){return(Vd=a.dynCall_viijiijj=a.asm.VA).apply(null,arguments)}
,Wd=a.dynCall_vijiijj=function(){return(Wd=a.dynCall_vijiijj=a.asm.WA).apply(null,
arguments)}
,Xd=a.dynCall_jiiiij=function(){return(Xd=a.dynCall_jiiiij=a.asm.XA).apply(null,arguments)}
,Yd=a.dynCall_jiji=function(){return(Yd=a.dynCall_jiji=a.asm.YA).apply(null,arguments)}
,Zd=a.dynCall_iiijj=function(){return(Zd=a.dynCall_iiijj=a.asm.ZA).apply(null,arguments)}
,$d=a.dynCall_viiiji=function(){return($d=a.dynCall_viiiji=a.asm._A).apply(null,arguments)}
,ae=a.dynCall_viijii=function(){return(ae=a.dynCall_viijii=a.asm.$A).apply(null,arguments)}
;
function jc(b,d,f){var c=R();try{return I(b)(d,f)}
catch(e){S(c);if(e!==e+0)throw e;P(1,0)}
}
function Sc(b,d,f,c,e){var g=R();try{I(b)(d,f,c,e)}
catch(h){S(g);if(h!==h+0)throw h;P(1,0)}
}
function Qc(b,d,f){var c=R();try{I(b)(d,f)}
catch(e){S(c);if(e!==e+0)throw e;P(1,0)}
}
function oc(b,d,f,c,e,g,h){var l=R();try{return I(b)(d,f,c,e,g,h)}
catch(n){S(l);if(n!==n+0)throw n;P(1,0)}
}
function Oc(b,d){var f=R();try{I(b)(d)}
catch(c){S(f);if(c!==c+0)throw c;P(1,0)}
}

function Nc(b){var d=R();try{I(b)()}
catch(f){S(d);if(f!==f+0)throw f;P(1,0)}
}
function kc(b,d,f,c){var e=R();try{return I(b)(d,f,c)}
catch(g){S(e);if(g!==g+0)throw g;P(1,0)}
}
function hc(b,d){var f=R();try{return I(b)(d)}
catch(c){S(f);if(c!==c+0)throw c;P(1,0)}
}
function Rc(b,d,f,c){var e=R();try{I(b)(d,f,c)}
catch(g){S(e);if(g!==g+0)throw g;P(1,0)}
}
function Tc(b,d,f,c,e,g){var h=R();try{I(b)(d,f,c,e,g)}
catch(l){S(h);if(l!==l+0)throw l;P(1,0)}
}

function lc(b,d,f,c,e){var g=R();try{return I(b)(d,f,c,e)}
catch(h){S(g);if(h!==h+0)throw h;P(1,0)}
}
function nc(b,d,f,c,e,g){var h=R();try{return I(b)(d,f,c,e,g)}
catch(l){S(h);if(l!==l+0)throw l;P(1,0)}
}
function ic(b,d,f){var c=R();try{return I(b)(d,f)}
catch(e){S(c);if(e!==e+0)throw e;P(1,0)}
}
function Uc(b,d,f,c,e,g,h){var l=R();try{I(b)(d,f,c,e,g,h)}
catch(n){S(l);if(n!==n+0)throw n;P(1,0)}
}
function Vc(b,d,f,c,e,g,h,l){var n=R();try{I(b)(d,f,c,e,g,h,l)}
catch(w){S(n);if(w!==w+0)throw w;P(1,0)}
}

function pc(b,d,f,c,e,g,h,l){var n=R();try{return I(b)(d,f,c,e,g,h,l)}
catch(w){S(n);if(w!==w+0)throw w;P(1,0)}
}
function Pc(b,d,f,c){var e=R();try{I(b)(d,f,c)}
catch(g){S(e);if(g!==g+0)throw g;P(1,0)}
}
function gc(b){var d=R();try{return I(b)()}
catch(f){S(d);if(f!==f+0)throw f;P(1,0)}
}
function Wc(b,d,f,c,e,g,h,l,n){var w=R();try{I(b)(d,f,c,e,g,h,l,n)}
catch(y){S(w);if(y!==y+0)throw y;P(1,0)}
}
function mc(b,d,f,c,e,g){var h=R();try{return I(b)(d,f,c,e,g)}
catch(l){S(h);if(l!==l+0)throw l;P(1,0)}
}

function qc(b,d,f,c,e,g,h,l,n,w,y,C){var F=R();try{return I(b)(d,f,c,e,g,h,l,n,w,y,C)}
catch(m){S(F);if(m!==m+0)throw m;P(1,0)}
}
function Xc(b,d,f,c,e,g,h,l,n,w,y){var C=R();try{I(b)(d,f,c,e,g,h,l,n,w,y)}
catch(F){S(C);if(F!==F+0)throw F;P(1,0)}
}
function Yc(b,d,f,c,e,g,h,l,n,w,y,C,F,m,B,A){var J=R();try{I(b)(d,f,c,e,g,h,l,n,w,y,C,F,m,B,A)}
catch(M){S(J);if(M!==M+0)throw M;P(1,0)}
}
function xc(b,d,f,c){var e=R();try{return td(b,d,f,c)}
catch(g){S(e);if(g!==g+0)throw g;P(1,0)}
}

function $c(b,d,f,c,e,g){var h=R();try{ud(b,d,f,c,e,g)}
catch(l){S(h);if(l!==l+0)throw l;P(1,0)}
}
function tc(b,d,f,c,e){var g=R();try{return vd(b,d,f,c,e)}
catch(h){S(g);if(h!==h+0)throw h;P(1,0)}
}
function Zc(b,d,f,c,e,g,h,l){var n=R();try{xd(b,d,f,c,e,g,h,l)}
catch(w){S(n);if(w!==w+0)throw w;P(1,0)}
}
function Ic(b,d){var f=R();try{return yd(b,d)}
catch(c){S(f);if(c!==c+0)throw c;P(1,0)}
}
function gd(b,d,f,c){var e=R();try{zd(b,d,f,c)}
catch(g){S(e);if(g!==g+0)throw g;P(1,0)}
}

function Fc(b,d,f,c,e,g){var h=R();try{return Ad(b,d,f,c,e,g)}
catch(l){S(h);if(l!==l+0)throw l;P(1,0)}
}
function uc(b,d,f,c,e,g){var h=R();try{return Bd(b,d,f,c,e,g)}
catch(l){S(h);if(l!==l+0)throw l;P(1,0)}
}
function cd(b,d,f,c,e,g){var h=R();try{Cd(b,d,f,c,e,g)}
catch(l){S(h);if(l!==l+0)throw l;P(1,0)}
}
function Jc(b,d,f){var c=R();try{return Dd(b,d,f)}
catch(e){S(c);if(e!==e+0)throw e;P(1,0)}
}
function nd(b,d,f,c,e){var g=R();try{Ed(b,d,f,c,e)}
catch(h){S(g);if(h!==h+0)throw h;P(1,0)}
}

function Gc(b,d,f,c,e,g,h){var l=R();try{return Fd(b,d,f,c,e,g,h)}
catch(n){S(l);if(n!==n+0)throw n;P(1,0)}
}
function Ec(b,d,f,c){var e=R();try{return Hd(b,d,f,c)}
catch(g){S(e);if(g!==g+0)throw g;P(1,0)}
}
function vc(b,d,f,c,e,g,h){var l=R();try{return Id(b,d,f,c,e,g,h)}
catch(n){S(l);if(n!==n+0)throw n;P(1,0)}
}
function fd(b,d,f,c,e,g,h){var l=R();try{wd(b,d,f,c,e,g,h)}
catch(n){S(l);if(n!==n+0)throw n;P(1,0)}
}

function Dc(b,d,f,c,e,g){var h=R();try{return Jd(b,d,f,c,e,g)}
catch(l){S(h);if(l!==l+0)throw l;P(1,0)}
}
function jd(b,d,f,c,e,g){var h=R();try{Ld(b,d,f,c,e,g)}
catch(l){S(h);if(l!==l+0)throw l;P(1,0)}
}
function hd(b,d,f,c,e){var g=R();try{Md(b,d,f,c,e)}
catch(h){S(g);if(h!==h+0)throw h;P(1,0)}
}
function kd(b,d,f,c,e,g,h){var l=R();try{Nd(b,d,f,c,e,g,h)}
catch(n){S(l);if(n!==n+0)throw n;P(1,0)}
}
function yc(b,d,f,c,e){var g=R();try{return Od(b,d,f,c,e)}
catch(h){S(g);if(h!==h+0)throw h;P(1,0)}
}

function bd(b,d,f,c,e){var g=R();try{sd(b,d,f,c,e)}
catch(h){S(g);if(h!==h+0)throw h;P(1,0)}
}
function Bc(b,d,f,c,e,g,h,l){var n=R();try{return Pd(b,d,f,c,e,g,h,l)}
catch(w){S(n);if(w!==w+0)throw w;P(1,0)}
}
function Ac(b,d,f,c,e,g,h){var l=R();try{return Qd(b,d,f,c,e,g,h)}
catch(n){S(l);if(n!==n+0)throw n;P(1,0)}
}
function zc(b,d,f,c,e,g){var h=R();try{return Rd(b,d,f,c,e,g)}
catch(l){S(h);if(l!==l+0)throw l;P(1,0)}
}

function rc(b,d,f,c,e,g,h,l,n){var w=R();try{return Sd(b,d,f,c,e,g,h,l,n)}
catch(y){S(w);if(y!==y+0)throw y;P(1,0)}
}
function Cc(b,d,f,c,e,g,h,l,n,w){var y=R();try{return Td(b,d,f,c,e,g,h,l,n,w)}
catch(C){S(y);if(C!==C+0)throw C;P(1,0)}
}
function md(b,d,f,c,e,g,h){var l=R();try{Ud(b,d,f,c,e,g,h)}
catch(n){S(l);if(n!==n+0)throw n;P(1,0)}
}
function ad(b,d,f,c,e,g,h){var l=R();try{$d(b,d,f,c,e,g,h)}
catch(n){S(l);if(n!==n+0)throw n;P(1,0)}
}

function dd(b,d,f,c,e,g,h){var l=R();try{ae(b,d,f,c,e,g,h)}
catch(n){S(l);if(n!==n+0)throw n;P(1,0)}
}
function ed(b,d,f,c,e,g,h,l,n,w,y){var C=R();try{Vd(b,d,f,c,e,g,h,l,n,w,y)}
catch(F){S(C);if(F!==F+0)throw F;P(1,0)}
}
function ld(b,d,f,c,e,g,h,l,n,w){var y=R();try{Wd(b,d,f,c,e,g,h,l,n,w)}
catch(C){S(y);if(C!==C+0)throw C;P(1,0)}
}
function Lc(b,d,f,c,e,g,h){var l=R();try{return Xd(b,d,f,c,e,g,h)}
catch(n){S(l);if(n!==n+0)throw n;P(1,0)}
}

function Mc(b,d,f,c,e){var g=R();try{return Yd(b,d,f,c,e)}
catch(h){S(g);if(h!==h+0)throw h;P(1,0)}
}
function wc(b,d,f,c,e,g,h){var l=R();try{return Zd(b,d,f,c,e,g,h)}
catch(n){S(l);if(n!==n+0)throw n;P(1,0)}
}
function Hc(b){var d=R();try{return Kd(b)}
catch(f){S(d);if(f!==f+0)throw f;P(1,0)}
}
function sc(b,d,f,c,e,g,h){var l=R();try{return rd(b,d,f,c,e,g,h)}
catch(n){S(l);if(n!==n+0)throw n;P(1,0)}
}
function Kc(b,d,f,c){var e=R();try{return Gd(b,d,f,c)}
catch(g){S(e);if(g!==g+0)throw g;P(1,0)}
}

a.allocateUTF8OnStack=cc;var be;Ja=function ce(){be||de();be||(Ja=ce)}
;
function de(){function b(){if(!be&&(be=!0,a.calledRun=!0,!ua)){Ga=!0;a.noFSInit||Mb||(Mb=!0,Lb(),a.stdin=a.stdin,a.stdout=a.stdout,a.stderr=a.stderr,a.stdin?Pb("stdin",a.stdin):Hb("/dev/tty","/dev/stdin"),a.stdout?Pb("stdout",null,a.stdout):Hb("/dev/tty","/dev/stdout"),a.stderr?Pb("stderr",null,a.stderr):Hb("/dev/tty1","/dev/stderr"),Jb("/dev/stdin",0),Jb("/dev/stdout",1),Jb("/dev/stderr",1));sb=!1;Qa(Ea);ea(a);if(a.onRuntimeInitialized)a.onRuntimeInitialized();if(a.postRun)for("function"==typeof a.postRun&&
(a.postRun=[a.postRun]);a.postRun.length;){var d=a.postRun.shift();Fa.unshift(d)}
Qa(Fa)}
}
if(!(0<D)){if(a.preRun)for("function"==typeof a.preRun&&(a.preRun=[a.preRun]);a.preRun.length;)Ha();Qa(Da);0<D||(a.setStatus?(a.setStatus("Running..."),setTimeout(function(){setTimeout(function(){a.setStatus("")}
                                                                                                                                                                                         ,1);b()}
,1)):b())}
}
if(a.preInit)for("function"==typeof a.preInit&&(a.preInit=[a.preInit]);0<a.preInit.length;)a.preInit.pop()();de();function T(b){try{var d=R();return b()}
finally{S(d)}
}

function U(b){return b?cc(b):0}
function V(b){const d=O(b.length<<2);x.set(b,d>>>2);return d}
function ee(b){const d=O(b.length);v.set(b,d);return d}

function fe(){[["none","None"],["i32","Int32"],["i64","Int64"],["f32","Float32"],["f64","Float64"],["v128","Vec128"],["funcref","Funcref"],["externref","Externref"],["anyref","Anyref"],["eqref","Eqref"],["i31ref","I31ref"],["dataref","Dataref"],["stringref","Stringref"],["stringview_wtf8","StringviewWTF8"],["stringview_wtf16","StringviewWTF16"],["stringview_iter","StringviewIter"],["unreachable","Unreachable"],["auto","Auto"]].forEach(b=>{a[b[0]]=a["_BinaryenType"+b[1]]()}
                                                                                                                                                                                                                                                                                                                                                                                                                                                       );a.ExpressionIds={}
;"Invalid Block If Loop Break Switch Call CallIndirect LocalGet LocalSet GlobalGet GlobalSet Load Store Const Unary Binary Select Drop Return MemorySize MemoryGrow Nop Unreachable AtomicCmpxchg AtomicRMW AtomicWait AtomicNotify AtomicFence SIMDExtract SIMDReplace SIMDShuffle SIMDTernary SIMDShift SIMDLoad SIMDLoadStoreLane MemoryInit DataDrop MemoryCopy MemoryFill RefNull RefIs RefFunc RefEq TableGet TableSet TableSize TableGrow Try Throw Rethrow TupleMake TupleExtract Pop I31New I31Get CallRef RefTest RefCast BrOn StructNew StructGet StructSet ArrayNew ArrayInit ArrayGet ArraySet ArrayLen ArrayCopy RefAs StringNew StringConst StringMeasure StringEncode StringConcat StringEq StringAs StringWTF8Advance StringWTF16Get StringIterNext StringIterMove StringSliceWTF StringSliceIter".split(" ").forEach(b=>
{a.ExpressionIds[b]=a[b+"Id"]=a["_Binaryen"+b+"Id"]()}
);a.ExternalKinds={}
;["Function","Table","Memory","Global","Tag"].forEach(b=>{a.ExternalKinds[b]=a["External"+b]=a["_BinaryenExternal"+b]()}
                                                      );a.Features={}
;"MVP Atomics BulkMemory MutableGlobals NontrappingFPToInt SignExt SIMD128 ExceptionHandling TailCall ReferenceTypes Multivalue GC Memory64 RelaxedSIMD ExtendedConst Strings MultiMemories All".split(" ").forEach(b=>{a.Features[b]=a["_BinaryenFeature"+b]()}
                                                                                                                                                                                                                    );a.Operations={}
;"ClzInt32 CtzInt32 PopcntInt32 NegFloat32 AbsFloat32 CeilFloat32 FloorFloat32 TruncFloat32 NearestFloat32 SqrtFloat32 EqZInt32 ClzInt64 CtzInt64 PopcntInt64 NegFloat64 AbsFloat64 CeilFloat64 FloorFloat64 TruncFloat64 NearestFloat64 SqrtFloat64 EqZInt64 ExtendSInt32 ExtendUInt32 WrapInt64 TruncSFloat32ToInt32 TruncSFloat32ToInt64 TruncUFloat32ToInt32 TruncUFloat32ToInt64 TruncSFloat64ToInt32 TruncSFloat64ToInt64 TruncUFloat64ToInt32 TruncUFloat64ToInt64 TruncSatSFloat32ToInt32 TruncSatSFloat32ToInt64 TruncSatUFloat32ToInt32 TruncSatUFloat32ToInt64 TruncSatSFloat64ToInt32 TruncSatSFloat64ToInt64 TruncSatUFloat64ToInt32 TruncSatUFloat64ToInt64 ReinterpretFloat32 ReinterpretFloat64 ConvertSInt32ToFloat32 ConvertSInt32ToFloat64 ConvertUInt32ToFloat32 ConvertUInt32ToFloat64 ConvertSInt64ToFloat32 ConvertSInt64ToFloat64 ConvertUInt64ToFloat32 ConvertUInt64ToFloat64 PromoteFloat32 DemoteFloat64 ReinterpretInt32 ReinterpretInt64 ExtendS8Int32 ExtendS16Int32 ExtendS8Int64 ExtendS16Int64 ExtendS32Int64 AddInt32 SubInt32 MulInt32 DivSInt32 DivUInt32 RemSInt32 RemUInt32 AndInt32 OrInt32 XorInt32 ShlInt32 ShrUInt32 ShrSInt32 RotLInt32 RotRInt32 EqInt32 NeInt32 LtSInt32 LtUInt32 LeSInt32 LeUInt32 GtSInt32 GtUInt32 GeSInt32 GeUInt32 AddInt64 SubInt64 MulInt64 DivSInt64 DivUInt64 RemSInt64 RemUInt64 AndInt64 OrInt64 XorInt64 ShlInt64 ShrUInt64 ShrSInt64 RotLInt64 RotRInt64 EqInt64 NeInt64 LtSInt64 LtUInt64 LeSInt64 LeUInt64 GtSInt64 GtUInt64 GeSInt64 GeUInt64 AddFloat32 SubFloat32 MulFloat32 DivFloat32 CopySignFloat32 MinFloat32 MaxFloat32 EqFloat32 NeFloat32 LtFloat32 LeFloat32 GtFloat32 GeFloat32 AddFloat64 SubFloat64 MulFloat64 DivFloat64 CopySignFloat64 MinFloat64 MaxFloat64 EqFloat64 NeFloat64 LtFloat64 LeFloat64 GtFloat64 GeFloat64 AtomicRMWAdd AtomicRMWSub AtomicRMWAnd AtomicRMWOr AtomicRMWXor AtomicRMWXchg SplatVecI8x16 ExtractLaneSVecI8x16 ExtractLaneUVecI8x16 ReplaceLaneVecI8x16 SplatVecI16x8 ExtractLaneSVecI16x8 ExtractLaneUVecI16x8 ReplaceLaneVecI16x8 SplatVecI32x4 ExtractLaneVecI32x4 ReplaceLaneVecI32x4 SplatVecI64x2 ExtractLaneVecI64x2 ReplaceLaneVecI64x2 SplatVecF32x4 ExtractLaneVecF32x4 ReplaceLaneVecF32x4 SplatVecF64x2 ExtractLaneVecF64x2 ReplaceLaneVecF64x2 EqVecI8x16 NeVecI8x16 LtSVecI8x16 LtUVecI8x16 GtSVecI8x16 GtUVecI8x16 LeSVecI8x16 LeUVecI8x16 GeSVecI8x16 GeUVecI8x16 EqVecI16x8 NeVecI16x8 LtSVecI16x8 LtUVecI16x8 GtSVecI16x8 GtUVecI16x8 LeSVecI16x8 LeUVecI16x8 GeSVecI16x8 GeUVecI16x8 EqVecI32x4 NeVecI32x4 LtSVecI32x4 LtUVecI32x4 GtSVecI32x4 GtUVecI32x4 LeSVecI32x4 LeUVecI32x4 GeSVecI32x4 GeUVecI32x4 EqVecI64x2 NeVecI64x2 LtSVecI64x2 GtSVecI64x2 LeSVecI64x2 GeSVecI64x2 EqVecF32x4 NeVecF32x4 LtVecF32x4 GtVecF32x4 LeVecF32x4 GeVecF32x4 EqVecF64x2 NeVecF64x2 LtVecF64x2 GtVecF64x2 LeVecF64x2 GeVecF64x2 NotVec128 AndVec128 OrVec128 XorVec128 AndNotVec128 BitselectVec128 AnyTrueVec128 PopcntVecI8x16 AbsVecI8x16 NegVecI8x16 AllTrueVecI8x16 BitmaskVecI8x16 ShlVecI8x16 ShrSVecI8x16 ShrUVecI8x16 AddVecI8x16 AddSatSVecI8x16 AddSatUVecI8x16 SubVecI8x16 SubSatSVecI8x16 SubSatUVecI8x16 MinSVecI8x16 MinUVecI8x16 MaxSVecI8x16 MaxUVecI8x16 AvgrUVecI8x16 AbsVecI16x8 NegVecI16x8 AllTrueVecI16x8 BitmaskVecI16x8 ShlVecI16x8 ShrSVecI16x8 ShrUVecI16x8 AddVecI16x8 AddSatSVecI16x8 AddSatUVecI16x8 SubVecI16x8 SubSatSVecI16x8 SubSatUVecI16x8 MulVecI16x8 MinSVecI16x8 MinUVecI16x8 MaxSVecI16x8 MaxUVecI16x8 AvgrUVecI16x8 Q15MulrSatSVecI16x8 ExtMulLowSVecI16x8 ExtMulHighSVecI16x8 ExtMulLowUVecI16x8 ExtMulHighUVecI16x8 DotSVecI16x8ToVecI32x4 ExtMulLowSVecI32x4 ExtMulHighSVecI32x4 ExtMulLowUVecI32x4 ExtMulHighUVecI32x4 AbsVecI32x4 NegVecI32x4 AllTrueVecI32x4 BitmaskVecI32x4 ShlVecI32x4 ShrSVecI32x4 ShrUVecI32x4 AddVecI32x4 SubVecI32x4 MulVecI32x4 MinSVecI32x4 MinUVecI32x4 MaxSVecI32x4 MaxUVecI32x4 AbsVecI64x2 NegVecI64x2 AllTrueVecI64x2 BitmaskVecI64x2 ShlVecI64x2 ShrSVecI64x2 ShrUVecI64x2 AddVecI64x2 SubVecI64x2 MulVecI64x2 ExtMulLowSVecI64x2 ExtMulHighSVecI64x2 ExtMulLowUVecI64x2 ExtMulHighUVecI64x2 AbsVecF32x4 NegVecF32x4 SqrtVecF32x4 AddVecF32x4 SubVecF32x4 MulVecF32x4 DivVecF32x4 MinVecF32x4 MaxVecF32x4 PMinVecF32x4 PMaxVecF32x4 CeilVecF32x4 FloorVecF32x4 TruncVecF32x4 NearestVecF32x4 AbsVecF64x2 NegVecF64x2 SqrtVecF64x2 AddVecF64x2 SubVecF64x2 MulVecF64x2 DivVecF64x2 MinVecF64x2 MaxVecF64x2 PMinVecF64x2 PMaxVecF64x2 CeilVecF64x2 FloorVecF64x2 TruncVecF64x2 NearestVecF64x2 ExtAddPairwiseSVecI8x16ToI16x8 ExtAddPairwiseUVecI8x16ToI16x8 ExtAddPairwiseSVecI16x8ToI32x4 ExtAddPairwiseUVecI16x8ToI32x4 TruncSatSVecF32x4ToVecI32x4 TruncSatUVecF32x4ToVecI32x4 ConvertSVecI32x4ToVecF32x4 ConvertUVecI32x4ToVecF32x4 Load8SplatVec128 Load16SplatVec128 Load32SplatVec128 Load64SplatVec128 Load8x8SVec128 Load8x8UVec128 Load16x4SVec128 Load16x4UVec128 Load32x2SVec128 Load32x2UVec128 Load32ZeroVec128 Load64ZeroVec128 Load8LaneVec128 Load16LaneVec128 Load32LaneVec128 Load64LaneVec128 Store8LaneVec128 Store16LaneVec128 Store32LaneVec128 Store64LaneVec128 NarrowSVecI16x8ToVecI8x16 NarrowUVecI16x8ToVecI8x16 NarrowSVecI32x4ToVecI16x8 NarrowUVecI32x4ToVecI16x8 ExtendLowSVecI8x16ToVecI16x8 ExtendHighSVecI8x16ToVecI16x8 ExtendLowUVecI8x16ToVecI16x8 ExtendHighUVecI8x16ToVecI16x8 ExtendLowSVecI16x8ToVecI32x4 ExtendHighSVecI16x8ToVecI32x4 ExtendLowUVecI16x8ToVecI32x4 ExtendHighUVecI16x8ToVecI32x4 ExtendLowSVecI32x4ToVecI64x2 ExtendHighSVecI32x4ToVecI64x2 ExtendLowUVecI32x4ToVecI64x2 ExtendHighUVecI32x4ToVecI64x2 ConvertLowSVecI32x4ToVecF64x2 ConvertLowUVecI32x4ToVecF64x2 TruncSatZeroSVecF64x2ToVecI32x4 TruncSatZeroUVecF64x2ToVecI32x4 DemoteZeroVecF64x2ToVecF32x4 PromoteLowVecF32x4ToVecF64x2 SwizzleVecI8x16 RefIsNull RefIsFunc RefIsData RefIsI31 RefAsNonNull RefAsFunc RefAsData RefAsI31 RefAsExternInternalize RefAsExternExternalize BrOnNull BrOnNonNull BrOnCast BrOnCastFail BrOnFunc BrOnNonFunc BrOnData BrOnNonData BrOnI31 BrOnNonI31 StringNewUTF8 StringNewWTF8 StringNewReplace StringNewWTF16 StringNewUTF8Array StringNewWTF8Array StringNewReplaceArray StringNewWTF16Array StringMeasureUTF8 StringMeasureWTF8 StringMeasureWTF16 StringMeasureIsUSV StringMeasureWTF16View StringEncodeUTF8 StringEncodeWTF8 StringEncodeWTF16 StringEncodeUTF8Array StringEncodeWTF8Array StringEncodeWTF16Array StringAsWTF8 StringAsWTF16 StringAsIter StringIterMoveAdvance StringIterMoveRewind StringSliceWTF8 StringSliceWTF16".split(" ").forEach(b=>
{a.Operations[b]=a[b]=a["_Binaryen"+b]()}
);a.SideEffects={}
;"None Branches Calls ReadsLocal WritesLocal ReadsGlobal WritesGlobal ReadsMemory WritesMemory ReadsTable WritesTable ImplicitTrap IsAtomic Throws DanglingPop TrapsNeverHappen Any".split(" ").forEach(b=>{a.SideEffects[b]=a["_BinaryenSideEffect"+b]()}
                                                                                                                                                                                                        );a.ExpressionRunner.Flags={Default:a._ExpressionRunnerFlagsDefault(),PreserveSideeffects:a._ExpressionRunnerFlagsPreserveSideeffects(),TraverseCalls:a._ExpressionRunnerFlagsTraverseCalls()}
}

a.Module=function(b){!b||r();ge(a._BinaryenModuleCreate(),this)}
;
function ge(b,d={}
            ){b||r();d.ptr=b;const f=pd();d.block=function(c,e,g){return T(()=>a._BinaryenBlock(b,c?U(c):0,V(e),e.length,"undefined"!==typeof g?g:a.none))}
;d["if"]=function(c,e,g){return a._BinaryenIf(b,c,e,g)}
;d.loop=function(c,e){return T(()=>a._BinaryenLoop(b,U(c),e))}
;d["break"]=d.br=function(c,e,g){return T(()=>a._BinaryenBreak(b,U(c),e,g))}
;d.br_if=function(c,e,g){return d.br(c,e,g)}
;d["switch"]=function(c,e,g,h){return T(()=>a._BinaryenSwitch(b,V(c.map(U)),c.length,U(e),g,h))}
;d.call=
function(c,e,g){return T(()=>a._BinaryenCall(b,U(c),V(e),e.length,g))}
;d.callIndirect=d.call_indirect=function(c,e,g,h,l){return T(()=>a._BinaryenCallIndirect(b,U(c),e,V(g),g.length,h,l))}
;d.returnCall=d.return_call=function(c,e,g){return T(()=>a._BinaryenReturnCall(b,U(c),V(e),e.length,g))}
;d.returnCallIndirect=d.return_call_indirect=function(c,e,g,h,l){return T(()=>a._BinaryenReturnCallIndirect(b,U(c),e,V(g),g.length,h,l))}
;d.local={get:function(c,e){return a._BinaryenLocalGet(b,c,e)}
,set:function(c,
e){return a._BinaryenLocalSet(b,c,e)}
,tee:function(c,e,g){if("undefined"===typeof g)throw Error("local.tee's type should be defined");return a._BinaryenLocalTee(b,c,e,g)}
}
;d.global={get:function(c,e){return a._BinaryenGlobalGet(b,U(c),e)}
,set:function(c,e){return a._BinaryenGlobalSet(b,U(c),e)}
}
;d.table={get:function(c,e,g){return a._BinaryenTableGet(b,U(c),e,g)}
,set:function(c,e,g){return a._BinaryenTableSet(b,U(c),e,g)}
,size:function(c){return a._BinaryenTableSize(b,U(c))}
,grow:function(c,e,g){return a._BinaryenTableGrow(b,
U(c),e,g)}
}
;d.memory={size:function(c,e){return a._BinaryenMemorySize(b,U(c),e)}
,grow:function(c,e,g){return a._BinaryenMemoryGrow(b,c,U(e),g)}
,init:function(c,e,g,h,l){return a._BinaryenMemoryInit(b,c,e,g,h,U(l))}
,copy:function(c,e,g,h,l){return a._BinaryenMemoryCopy(b,c,e,g,U(h),U(l))}
,fill:function(c,e,g,h){return a._BinaryenMemoryFill(b,c,e,g,U(h))}
,atomic:{notify:function(c,e,g){return a._BinaryenAtomicNotify(b,c,e,U(g))}
,wait32:function(c,e,g,h){return a._BinaryenAtomicWait(b,c,e,g,a.i32,U(h))}
,
wait64:function(c,e,g,h){return a._BinaryenAtomicWait(b,c,e,g,a.i64,U(h))}
}
}
;d.data={drop:function(c){return a._BinaryenDataDrop(b,c)}
}
;d.i32={load:function(c,e,g,h){return a._BinaryenLoad(b,4,!0,c,e,a.i32,g,U(h))}
,load8_s:function(c,e,g,h){return a._BinaryenLoad(b,1,!0,c,e,a.i32,g,U(h))}
,load8_u:function(c,e,g,h){return a._BinaryenLoad(b,1,!1,c,e,a.i32,g,U(h))}
,load16_s:function(c,e,g,h){return a._BinaryenLoad(b,2,!0,c,e,a.i32,g,U(h))}
,load16_u:function(c,e,g,h){return a._BinaryenLoad(b,2,!1,c,e,
a.i32,g,U(h))}
,store:function(c,e,g,h,l){return a._BinaryenStore(b,4,c,e,g,h,a.i32,U(l))}
,store8:function(c,e,g,h,l){return a._BinaryenStore(b,1,c,e,g,h,a.i32,U(l))}
,store16:function(c,e,g,h,l){return a._BinaryenStore(b,2,c,e,g,h,a.i32,U(l))}
,["const"](c){return T(()=>{const e=O(f);a._BinaryenLiteralInt32(e,c);return a._BinaryenConst(b,e)}
                       )}
,clz:function(c){return a._BinaryenUnary(b,a.ClzInt32,c)}
,ctz:function(c){return a._BinaryenUnary(b,a.CtzInt32,c)}
,popcnt:function(c){return a._BinaryenUnary(b,
a.PopcntInt32,c)}
,eqz:function(c){return a._BinaryenUnary(b,a.EqZInt32,c)}
,trunc_s:{f32:function(c){return a._BinaryenUnary(b,a.TruncSFloat32ToInt32,c)}
,f64:function(c){return a._BinaryenUnary(b,a.TruncSFloat64ToInt32,c)}
}
,trunc_u:{f32:function(c){return a._BinaryenUnary(b,a.TruncUFloat32ToInt32,c)}
,f64:function(c){return a._BinaryenUnary(b,a.TruncUFloat64ToInt32,c)}
}
,trunc_s_sat:{f32:function(c){return a._BinaryenUnary(b,a.TruncSatSFloat32ToInt32,c)}
,f64:function(c){return a._BinaryenUnary(b,a.TruncSatSFloat64ToInt32,
c)}
}
,trunc_u_sat:{f32:function(c){return a._BinaryenUnary(b,a.TruncSatUFloat32ToInt32,c)}
,f64:function(c){return a._BinaryenUnary(b,a.TruncSatUFloat64ToInt32,c)}
}
,reinterpret:function(c){return a._BinaryenUnary(b,a.ReinterpretFloat32,c)}
,extend8_s:function(c){return a._BinaryenUnary(b,a.ExtendS8Int32,c)}
,extend16_s:function(c){return a._BinaryenUnary(b,a.ExtendS16Int32,c)}
,wrap:function(c){return a._BinaryenUnary(b,a.WrapInt64,c)}
,add:function(c,e){return a._BinaryenBinary(b,a.AddInt32,c,e)}
,sub:function(c,
e){return a._BinaryenBinary(b,a.SubInt32,c,e)}
,mul:function(c,e){return a._BinaryenBinary(b,a.MulInt32,c,e)}
,div_s:function(c,e){return a._BinaryenBinary(b,a.DivSInt32,c,e)}
,div_u:function(c,e){return a._BinaryenBinary(b,a.DivUInt32,c,e)}
,rem_s:function(c,e){return a._BinaryenBinary(b,a.RemSInt32,c,e)}
,rem_u:function(c,e){return a._BinaryenBinary(b,a.RemUInt32,c,e)}
,and:function(c,e){return a._BinaryenBinary(b,a.AndInt32,c,e)}
,or:function(c,e){return a._BinaryenBinary(b,a.OrInt32,c,e)}
,xor:function(c,
e){return a._BinaryenBinary(b,a.XorInt32,c,e)}
,shl:function(c,e){return a._BinaryenBinary(b,a.ShlInt32,c,e)}
,shr_u:function(c,e){return a._BinaryenBinary(b,a.ShrUInt32,c,e)}
,shr_s:function(c,e){return a._BinaryenBinary(b,a.ShrSInt32,c,e)}
,rotl:function(c,e){return a._BinaryenBinary(b,a.RotLInt32,c,e)}
,rotr:function(c,e){return a._BinaryenBinary(b,a.RotRInt32,c,e)}
,eq:function(c,e){return a._BinaryenBinary(b,a.EqInt32,c,e)}
,ne:function(c,e){return a._BinaryenBinary(b,a.NeInt32,c,e)}
,lt_s:function(c,
e){return a._BinaryenBinary(b,a.LtSInt32,c,e)}
,lt_u:function(c,e){return a._BinaryenBinary(b,a.LtUInt32,c,e)}
,le_s:function(c,e){return a._BinaryenBinary(b,a.LeSInt32,c,e)}
,le_u:function(c,e){return a._BinaryenBinary(b,a.LeUInt32,c,e)}
,gt_s:function(c,e){return a._BinaryenBinary(b,a.GtSInt32,c,e)}
,gt_u:function(c,e){return a._BinaryenBinary(b,a.GtUInt32,c,e)}
,ge_s:function(c,e){return a._BinaryenBinary(b,a.GeSInt32,c,e)}
,ge_u:function(c,e){return a._BinaryenBinary(b,a.GeUInt32,c,e)}
,atomic:{load:function(c,
e,g){return a._BinaryenAtomicLoad(b,4,c,a.i32,e,U(g))}
,load8_u:function(c,e,g){return a._BinaryenAtomicLoad(b,1,c,a.i32,e,U(g))}
,load16_u:function(c,e,g){return a._BinaryenAtomicLoad(b,2,c,a.i32,e,U(g))}
,store:function(c,e,g,h){return a._BinaryenAtomicStore(b,4,c,e,g,a.i32,U(h))}
,store8:function(c,e,g,h){return a._BinaryenAtomicStore(b,1,c,e,g,a.i32,U(h))}
,store16:function(c,e,g,h){return a._BinaryenAtomicStore(b,2,c,e,g,a.i32,U(h))}
,rmw:{add:function(c,e,g,h){return a._BinaryenAtomicRMW(b,a.AtomicRMWAdd,
4,c,e,g,a.i32,U(h))}
,sub:function(c,e,g,h){return a._BinaryenAtomicRMW(b,a.AtomicRMWSub,4,c,e,g,a.i32,U(h))}
,and:function(c,e,g,h){return a._BinaryenAtomicRMW(b,a.AtomicRMWAnd,4,c,e,g,a.i32,U(h))}
,or:function(c,e,g,h){return a._BinaryenAtomicRMW(b,a.AtomicRMWOr,4,c,e,g,a.i32,U(h))}
,xor:function(c,e,g,h){return a._BinaryenAtomicRMW(b,a.AtomicRMWXor,4,c,e,g,a.i32,U(h))}
,xchg:function(c,e,g,h){return a._BinaryenAtomicRMW(b,a.AtomicRMWXchg,4,c,e,g,a.i32,U(h))}
,cmpxchg:function(c,e,g,h,l){return a._BinaryenAtomicCmpxchg(b,
4,c,e,g,h,a.i32,U(l))}
}
,rmw8_u:{add:function(c,e,g,h){return a._BinaryenAtomicRMW(b,a.AtomicRMWAdd,1,c,e,g,a.i32,U(h))}
,sub:function(c,e,g,h){return a._BinaryenAtomicRMW(b,a.AtomicRMWSub,1,c,e,g,a.i32,U(h))}
,and:function(c,e,g,h){return a._BinaryenAtomicRMW(b,a.AtomicRMWAnd,1,c,e,g,a.i32,U(h))}
,or:function(c,e,g,h){return a._BinaryenAtomicRMW(b,a.AtomicRMWOr,1,c,e,g,a.i32,U(h))}
,xor:function(c,e,g,h){return a._BinaryenAtomicRMW(b,a.AtomicRMWXor,1,c,e,g,a.i32,U(h))}
,xchg:function(c,e,g,h){return a._BinaryenAtomicRMW(b,
a.AtomicRMWXchg,1,c,e,g,a.i32,U(h))}
,cmpxchg:function(c,e,g,h,l){return a._BinaryenAtomicCmpxchg(b,1,c,e,g,h,a.i32,U(l))}
}
,rmw16_u:{add:function(c,e,g,h){return a._BinaryenAtomicRMW(b,a.AtomicRMWAdd,2,c,e,g,a.i32,U(h))}
,sub:function(c,e,g,h){return a._BinaryenAtomicRMW(b,a.AtomicRMWSub,2,c,e,g,a.i32,U(h))}
,and:function(c,e,g,h){return a._BinaryenAtomicRMW(b,a.AtomicRMWAnd,2,c,e,g,a.i32,U(h))}
,or:function(c,e,g,h){return a._BinaryenAtomicRMW(b,a.AtomicRMWOr,2,c,e,g,a.i32,U(h))}
,xor:function(c,e,g,
h){return a._BinaryenAtomicRMW(b,a.AtomicRMWXor,2,c,e,g,a.i32,U(h))}
,xchg:function(c,e,g,h){return a._BinaryenAtomicRMW(b,a.AtomicRMWXchg,2,c,e,g,a.i32,U(h))}
,cmpxchg:function(c,e,g,h,l){return a._BinaryenAtomicCmpxchg(b,2,c,e,g,h,a.i32,U(l))}
}
}
,pop:function(){return a._BinaryenPop(b,a.i32)}
}
;d.i64={load:function(c,e,g,h){return a._BinaryenLoad(b,8,!0,c,e,a.i64,g,U(h))}
,load8_s:function(c,e,g,h){return a._BinaryenLoad(b,1,!0,c,e,a.i64,g,U(h))}
,load8_u:function(c,e,g,h){return a._BinaryenLoad(b,1,
!1,c,e,a.i64,g,U(h))}
,load16_s:function(c,e,g,h){return a._BinaryenLoad(b,2,!0,c,e,a.i64,g,U(h))}
,load16_u:function(c,e,g,h){return a._BinaryenLoad(b,2,!1,c,e,a.i64,g,U(h))}
,load32_s:function(c,e,g,h){return a._BinaryenLoad(b,4,!0,c,e,a.i64,g,U(h))}
,load32_u:function(c,e,g,h){return a._BinaryenLoad(b,4,!1,c,e,a.i64,g,U(h))}
,store:function(c,e,g,h,l){return a._BinaryenStore(b,8,c,e,g,h,a.i64,U(l))}
,store8:function(c,e,g,h,l){return a._BinaryenStore(b,1,c,e,g,h,a.i64,U(l))}
,store16:function(c,e,g,h,
l){return a._BinaryenStore(b,2,c,e,g,h,a.i64,U(l))}
,store32:function(c,e,g,h,l){return a._BinaryenStore(b,4,c,e,g,h,a.i64,U(l))}
,["const"](c,e){return T(()=>{const g=O(f);a._BinaryenLiteralInt64(g,c,e);return a._BinaryenConst(b,g)}
                         )}
,clz:function(c){return a._BinaryenUnary(b,a.ClzInt64,c)}
,ctz:function(c){return a._BinaryenUnary(b,a.CtzInt64,c)}
,popcnt:function(c){return a._BinaryenUnary(b,a.PopcntInt64,c)}
,eqz:function(c){return a._BinaryenUnary(b,a.EqZInt64,c)}
,trunc_s:{f32:function(c){return a._BinaryenUnary(b,
a.TruncSFloat32ToInt64,c)}
,f64:function(c){return a._BinaryenUnary(b,a.TruncSFloat64ToInt64,c)}
}
,trunc_u:{f32:function(c){return a._BinaryenUnary(b,a.TruncUFloat32ToInt64,c)}
,f64:function(c){return a._BinaryenUnary(b,a.TruncUFloat64ToInt64,c)}
}
,trunc_s_sat:{f32:function(c){return a._BinaryenUnary(b,a.TruncSatSFloat32ToInt64,c)}
,f64:function(c){return a._BinaryenUnary(b,a.TruncSatSFloat64ToInt64,c)}
}
,trunc_u_sat:{f32:function(c){return a._BinaryenUnary(b,a.TruncSatUFloat32ToInt64,c)}
,f64:function(c){return a._BinaryenUnary(b,
a.TruncSatUFloat64ToInt64,c)}
}
,reinterpret:function(c){return a._BinaryenUnary(b,a.ReinterpretFloat64,c)}
,extend8_s:function(c){return a._BinaryenUnary(b,a.ExtendS8Int64,c)}
,extend16_s:function(c){return a._BinaryenUnary(b,a.ExtendS16Int64,c)}
,extend32_s:function(c){return a._BinaryenUnary(b,a.ExtendS32Int64,c)}
,extend_s:function(c){return a._BinaryenUnary(b,a.ExtendSInt32,c)}
,extend_u:function(c){return a._BinaryenUnary(b,a.ExtendUInt32,c)}
,add:function(c,e){return a._BinaryenBinary(b,a.AddInt64,
c,e)}
,sub:function(c,e){return a._BinaryenBinary(b,a.SubInt64,c,e)}
,mul:function(c,e){return a._BinaryenBinary(b,a.MulInt64,c,e)}
,div_s:function(c,e){return a._BinaryenBinary(b,a.DivSInt64,c,e)}
,div_u:function(c,e){return a._BinaryenBinary(b,a.DivUInt64,c,e)}
,rem_s:function(c,e){return a._BinaryenBinary(b,a.RemSInt64,c,e)}
,rem_u:function(c,e){return a._BinaryenBinary(b,a.RemUInt64,c,e)}
,and:function(c,e){return a._BinaryenBinary(b,a.AndInt64,c,e)}
,or:function(c,e){return a._BinaryenBinary(b,a.OrInt64,
c,e)}
,xor:function(c,e){return a._BinaryenBinary(b,a.XorInt64,c,e)}
,shl:function(c,e){return a._BinaryenBinary(b,a.ShlInt64,c,e)}
,shr_u:function(c,e){return a._BinaryenBinary(b,a.ShrUInt64,c,e)}
,shr_s:function(c,e){return a._BinaryenBinary(b,a.ShrSInt64,c,e)}
,rotl:function(c,e){return a._BinaryenBinary(b,a.RotLInt64,c,e)}
,rotr:function(c,e){return a._BinaryenBinary(b,a.RotRInt64,c,e)}
,eq:function(c,e){return a._BinaryenBinary(b,a.EqInt64,c,e)}
,ne:function(c,e){return a._BinaryenBinary(b,a.NeInt64,
c,e)}
,lt_s:function(c,e){return a._BinaryenBinary(b,a.LtSInt64,c,e)}
,lt_u:function(c,e){return a._BinaryenBinary(b,a.LtUInt64,c,e)}
,le_s:function(c,e){return a._BinaryenBinary(b,a.LeSInt64,c,e)}
,le_u:function(c,e){return a._BinaryenBinary(b,a.LeUInt64,c,e)}
,gt_s:function(c,e){return a._BinaryenBinary(b,a.GtSInt64,c,e)}
,gt_u:function(c,e){return a._BinaryenBinary(b,a.GtUInt64,c,e)}
,ge_s:function(c,e){return a._BinaryenBinary(b,a.GeSInt64,c,e)}
,ge_u:function(c,e){return a._BinaryenBinary(b,a.GeUInt64,
c,e)}
,atomic:{load:function(c,e,g){return a._BinaryenAtomicLoad(b,8,c,a.i64,e,U(g))}
,load8_u:function(c,e,g){return a._BinaryenAtomicLoad(b,1,c,a.i64,e,U(g))}
,load16_u:function(c,e,g){return a._BinaryenAtomicLoad(b,2,c,a.i64,e,U(g))}
,load32_u:function(c,e,g){return a._BinaryenAtomicLoad(b,4,c,a.i64,e,U(g))}
,store:function(c,e,g,h){return a._BinaryenAtomicStore(b,8,c,e,g,a.i64,U(h))}
,store8:function(c,e,g,h){return a._BinaryenAtomicStore(b,1,c,e,g,a.i64,U(h))}
,store16:function(c,e,g,h){return a._BinaryenAtomicStore(b,
2,c,e,g,a.i64,U(h))}
,store32:function(c,e,g,h){return a._BinaryenAtomicStore(b,4,c,e,g,a.i64,U(h))}
,rmw:{add:function(c,e,g,h){return a._BinaryenAtomicRMW(b,a.AtomicRMWAdd,8,c,e,g,a.i64,U(h))}
,sub:function(c,e,g,h){return a._BinaryenAtomicRMW(b,a.AtomicRMWSub,8,c,e,g,a.i64,U(h))}
,and:function(c,e,g,h){return a._BinaryenAtomicRMW(b,a.AtomicRMWAnd,8,c,e,g,a.i64,U(h))}
,or:function(c,e,g,h){return a._BinaryenAtomicRMW(b,a.AtomicRMWOr,8,c,e,g,a.i64,U(h))}
,xor:function(c,e,g,h){return a._BinaryenAtomicRMW(b,
a.AtomicRMWXor,8,c,e,g,a.i64,U(h))}
,xchg:function(c,e,g,h){return a._BinaryenAtomicRMW(b,a.AtomicRMWXchg,8,c,e,g,a.i64,U(h))}
,cmpxchg:function(c,e,g,h,l){return a._BinaryenAtomicCmpxchg(b,8,c,e,g,h,a.i64,U(l))}
}
,rmw8_u:{add:function(c,e,g,h){return a._BinaryenAtomicRMW(b,a.AtomicRMWAdd,1,c,e,g,a.i64,U(h))}
,sub:function(c,e,g,h){return a._BinaryenAtomicRMW(b,a.AtomicRMWSub,1,c,e,g,a.i64,U(h))}
,and:function(c,e,g,h){return a._BinaryenAtomicRMW(b,a.AtomicRMWAnd,1,c,e,g,a.i64,U(h))}
,or:function(c,e,g,
h){return a._BinaryenAtomicRMW(b,a.AtomicRMWOr,1,c,e,g,a.i64,U(h))}
,xor:function(c,e,g,h){return a._BinaryenAtomicRMW(b,a.AtomicRMWXor,1,c,e,g,a.i64,U(h))}
,xchg:function(c,e,g,h){return a._BinaryenAtomicRMW(b,a.AtomicRMWXchg,1,c,e,g,a.i64,U(h))}
,cmpxchg:function(c,e,g,h,l){return a._BinaryenAtomicCmpxchg(b,1,c,e,g,h,a.i64,U(l))}
}
,rmw16_u:{add:function(c,e,g,h){return a._BinaryenAtomicRMW(b,a.AtomicRMWAdd,2,c,e,g,a.i64,U(h))}
,sub:function(c,e,g,h){return a._BinaryenAtomicRMW(b,a.AtomicRMWSub,2,c,e,
g,a.i64,U(h))}
,and:function(c,e,g,h){return a._BinaryenAtomicRMW(b,a.AtomicRMWAnd,2,c,e,g,a.i64,U(h))}
,or:function(c,e,g,h){return a._BinaryenAtomicRMW(b,a.AtomicRMWOr,2,c,e,g,a.i64,U(h))}
,xor:function(c,e,g,h){return a._BinaryenAtomicRMW(b,a.AtomicRMWXor,2,c,e,g,a.i64,U(h))}
,xchg:function(c,e,g,h){return a._BinaryenAtomicRMW(b,a.AtomicRMWXchg,2,c,e,g,a.i64,U(h))}
,cmpxchg:function(c,e,g,h,l){return a._BinaryenAtomicCmpxchg(b,2,c,e,g,h,a.i64,U(l))}
}
,rmw32_u:{add:function(c,e,g,h){return a._BinaryenAtomicRMW(b,
a.AtomicRMWAdd,4,c,e,g,a.i64,U(h))}
,sub:function(c,e,g,h){return a._BinaryenAtomicRMW(b,a.AtomicRMWSub,4,c,e,g,a.i64,U(h))}
,and:function(c,e,g,h){return a._BinaryenAtomicRMW(b,a.AtomicRMWAnd,4,c,e,g,a.i64,U(h))}
,or:function(c,e,g,h){return a._BinaryenAtomicRMW(b,a.AtomicRMWOr,4,c,e,g,a.i64,U(h))}
,xor:function(c,e,g,h){return a._BinaryenAtomicRMW(b,a.AtomicRMWXor,4,c,e,g,a.i64,U(h))}
,xchg:function(c,e,g,h){return a._BinaryenAtomicRMW(b,a.AtomicRMWXchg,4,c,e,g,a.i64,U(h))}
,cmpxchg:function(c,e,g,h,
l){return a._BinaryenAtomicCmpxchg(b,4,c,e,g,h,a.i64,U(l))}
}
}
,pop:function(){return a._BinaryenPop(b,a.i64)}
}
;d.f32={load:function(c,e,g,h){return a._BinaryenLoad(b,4,!0,c,e,a.f32,g,U(h))}
,store:function(c,e,g,h,l){return a._BinaryenStore(b,4,c,e,g,h,a.f32,U(l))}
,["const"](c){return T(()=>{const e=O(f);a._BinaryenLiteralFloat32(e,c);return a._BinaryenConst(b,e)}
                       )}
,const_bits:function(c){return T(()=>{const e=O(f);a._BinaryenLiteralFloat32Bits(e,c);return a._BinaryenConst(b,e)}
                                 )}
,neg:function(c){return a._BinaryenUnary(b,
a.NegFloat32,c)}
,abs:function(c){return a._BinaryenUnary(b,a.AbsFloat32,c)}
,ceil:function(c){return a._BinaryenUnary(b,a.CeilFloat32,c)}
,floor:function(c){return a._BinaryenUnary(b,a.FloorFloat32,c)}
,trunc:function(c){return a._BinaryenUnary(b,a.TruncFloat32,c)}
,nearest:function(c){return a._BinaryenUnary(b,a.NearestFloat32,c)}
,sqrt:function(c){return a._BinaryenUnary(b,a.SqrtFloat32,c)}
,reinterpret:function(c){return a._BinaryenUnary(b,a.ReinterpretInt32,c)}
,convert_s:{i32:function(c){return a._BinaryenUnary(b,
a.ConvertSInt32ToFloat32,c)}
,i64:function(c){return a._BinaryenUnary(b,a.ConvertSInt64ToFloat32,c)}
}
,convert_u:{i32:function(c){return a._BinaryenUnary(b,a.ConvertUInt32ToFloat32,c)}
,i64:function(c){return a._BinaryenUnary(b,a.ConvertUInt64ToFloat32,c)}
}
,demote:function(c){return a._BinaryenUnary(b,a.DemoteFloat64,c)}
,add:function(c,e){return a._BinaryenBinary(b,a.AddFloat32,c,e)}
,sub:function(c,e){return a._BinaryenBinary(b,a.SubFloat32,c,e)}
,mul:function(c,e){return a._BinaryenBinary(b,a.MulFloat32,
c,e)}
,div:function(c,e){return a._BinaryenBinary(b,a.DivFloat32,c,e)}
,copysign:function(c,e){return a._BinaryenBinary(b,a.CopySignFloat32,c,e)}
,min:function(c,e){return a._BinaryenBinary(b,a.MinFloat32,c,e)}
,max:function(c,e){return a._BinaryenBinary(b,a.MaxFloat32,c,e)}
,eq:function(c,e){return a._BinaryenBinary(b,a.EqFloat32,c,e)}
,ne:function(c,e){return a._BinaryenBinary(b,a.NeFloat32,c,e)}
,lt:function(c,e){return a._BinaryenBinary(b,a.LtFloat32,c,e)}
,le:function(c,e){return a._BinaryenBinary(b,
a.LeFloat32,c,e)}
,gt:function(c,e){return a._BinaryenBinary(b,a.GtFloat32,c,e)}
,ge:function(c,e){return a._BinaryenBinary(b,a.GeFloat32,c,e)}
,pop:function(){return a._BinaryenPop(b,a.f32)}
}
;d.f64={load:function(c,e,g,h){return a._BinaryenLoad(b,8,!0,c,e,a.f64,g,U(h))}
,store:function(c,e,g,h,l){return a._BinaryenStore(b,8,c,e,g,h,a.f64,U(l))}
,["const"](c){return T(()=>{const e=O(f);a._BinaryenLiteralFloat64(e,c);return a._BinaryenConst(b,e)}
                       )}
,const_bits:function(c,e){return T(()=>{const g=O(f);a._BinaryenLiteralFloat64Bits(g,
c,e);return a._BinaryenConst(b,g)}
)}
,neg:function(c){return a._BinaryenUnary(b,a.NegFloat64,c)}
,abs:function(c){return a._BinaryenUnary(b,a.AbsFloat64,c)}
,ceil:function(c){return a._BinaryenUnary(b,a.CeilFloat64,c)}
,floor:function(c){return a._BinaryenUnary(b,a.FloorFloat64,c)}
,trunc:function(c){return a._BinaryenUnary(b,a.TruncFloat64,c)}
,nearest:function(c){return a._BinaryenUnary(b,a.NearestFloat64,c)}
,sqrt:function(c){return a._BinaryenUnary(b,a.SqrtFloat64,c)}
,reinterpret:function(c){return a._BinaryenUnary(b,
a.ReinterpretInt64,c)}
,convert_s:{i32:function(c){return a._BinaryenUnary(b,a.ConvertSInt32ToFloat64,c)}
,i64:function(c){return a._BinaryenUnary(b,a.ConvertSInt64ToFloat64,c)}
}
,convert_u:{i32:function(c){return a._BinaryenUnary(b,a.ConvertUInt32ToFloat64,c)}
,i64:function(c){return a._BinaryenUnary(b,a.ConvertUInt64ToFloat64,c)}
}
,promote:function(c){return a._BinaryenUnary(b,a.PromoteFloat32,c)}
,add:function(c,e){return a._BinaryenBinary(b,a.AddFloat64,c,e)}
,sub:function(c,e){return a._BinaryenBinary(b,
a.SubFloat64,c,e)}
,mul:function(c,e){return a._BinaryenBinary(b,a.MulFloat64,c,e)}
,div:function(c,e){return a._BinaryenBinary(b,a.DivFloat64,c,e)}
,copysign:function(c,e){return a._BinaryenBinary(b,a.CopySignFloat64,c,e)}
,min:function(c,e){return a._BinaryenBinary(b,a.MinFloat64,c,e)}
,max:function(c,e){return a._BinaryenBinary(b,a.MaxFloat64,c,e)}
,eq:function(c,e){return a._BinaryenBinary(b,a.EqFloat64,c,e)}
,ne:function(c,e){return a._BinaryenBinary(b,a.NeFloat64,c,e)}
,lt:function(c,e){return a._BinaryenBinary(b,
a.LtFloat64,c,e)}
,le:function(c,e){return a._BinaryenBinary(b,a.LeFloat64,c,e)}
,gt:function(c,e){return a._BinaryenBinary(b,a.GtFloat64,c,e)}
,ge:function(c,e){return a._BinaryenBinary(b,a.GeFloat64,c,e)}
,pop:function(){return a._BinaryenPop(b,a.f64)}
}
;d.v128={load:function(c,e,g,h){return a._BinaryenLoad(b,16,!1,c,e,a.v128,g,U(h))}
,load8_splat:function(c,e,g,h){return a._BinaryenSIMDLoad(b,a.Load8SplatVec128,c,e,g,U(h))}
,load16_splat:function(c,e,g,h){return a._BinaryenSIMDLoad(b,a.Load16SplatVec128,
c,e,g,U(h))}
,load32_splat:function(c,e,g,h){return a._BinaryenSIMDLoad(b,a.Load32SplatVec128,c,e,g,U(h))}
,load64_splat:function(c,e,g,h){return a._BinaryenSIMDLoad(b,a.Load64SplatVec128,c,e,g,U(h))}
,load8x8_s:function(c,e,g,h){return a._BinaryenSIMDLoad(b,a.Load8x8SVec128,c,e,g,U(h))}
,load8x8_u:function(c,e,g,h){return a._BinaryenSIMDLoad(b,a.Load8x8UVec128,c,e,g,U(h))}
,load16x4_s:function(c,e,g,h){return a._BinaryenSIMDLoad(b,a.Load16x4SVec128,c,e,g,U(h))}
,load16x4_u:function(c,e,g,h){return a._BinaryenSIMDLoad(b,
a.Load16x4UVec128,c,e,g,U(h))}
,load32x2_s:function(c,e,g,h){return a._BinaryenSIMDLoad(b,a.Load32x2SVec128,c,e,g,U(h))}
,load32x2_u:function(c,e,g,h){return a._BinaryenSIMDLoad(b,a.Load32x2UVec128,c,e,g,U(h))}
,load32_zero:function(c,e,g,h){return a._BinaryenSIMDLoad(b,a.Load32ZeroVec128,c,e,g,U(h))}
,load64_zero:function(c,e,g,h){return a._BinaryenSIMDLoad(b,a.Load64ZeroVec128,c,e,g,U(h))}
,load8_lane:function(c,e,g,h,l,n){return a._BinaryenSIMDLoadStoreLane(b,a.Load8LaneVec128,c,e,g,h,l,U(n))}
,load16_lane:function(c,
e,g,h,l,n){return a._BinaryenSIMDLoadStoreLane(b,a.Load16LaneVec128,c,e,g,h,l,U(n))}
,load32_lane:function(c,e,g,h,l,n){return a._BinaryenSIMDLoadStoreLane(b,a.Load32LaneVec128,c,e,g,h,l,U(n))}
,load64_lane:function(c,e,g,h,l,n){return a._BinaryenSIMDLoadStoreLane(b,a.Load64LaneVec128,c,e,g,h,l,U(n))}
,store8_lane:function(c,e,g,h,l,n){return a._BinaryenSIMDLoadStoreLane(b,a.Store8LaneVec128,c,e,g,h,l,U(n))}
,store16_lane:function(c,e,g,h,l,n){return a._BinaryenSIMDLoadStoreLane(b,a.Store16LaneVec128,
c,e,g,h,l,U(n))}
,store32_lane:function(c,e,g,h,l,n){return a._BinaryenSIMDLoadStoreLane(b,a.Store32LaneVec128,c,e,g,h,l,U(n))}
,store64_lane:function(c,e,g,h,l,n){return a._BinaryenSIMDLoadStoreLane(b,a.Store64LaneVec128,c,e,g,h,l,U(n))}
,store:function(c,e,g,h,l){return a._BinaryenStore(b,16,c,e,g,h,a.v128,U(l))}
,["const"](c){return T(()=>{const e=O(f);a._BinaryenLiteralVec128(e,ee(c));return a._BinaryenConst(b,e)}
                       )}
,not:function(c){return a._BinaryenUnary(b,a.NotVec128,c)}
,any_true:function(c){return a._BinaryenUnary(b,
a.AnyTrueVec128,c)}
,and:function(c,e){return a._BinaryenBinary(b,a.AndVec128,c,e)}
,or:function(c,e){return a._BinaryenBinary(b,a.OrVec128,c,e)}
,xor:function(c,e){return a._BinaryenBinary(b,a.XorVec128,c,e)}
,andnot:function(c,e){return a._BinaryenBinary(b,a.AndNotVec128,c,e)}
,bitselect:function(c,e,g){return a._BinaryenSIMDTernary(b,a.BitselectVec128,c,e,g)}
,pop:function(){return a._BinaryenPop(b,a.v128)}
}
;d.i8x16={shuffle:function(c,e,g){return T(()=>a._BinaryenSIMDShuffle(b,c,e,ee(g)))}
,swizzle:function(c,
e){return a._BinaryenBinary(b,a.SwizzleVecI8x16,c,e)}
,splat:function(c){return a._BinaryenUnary(b,a.SplatVecI8x16,c)}
,extract_lane_s:function(c,e){return a._BinaryenSIMDExtract(b,a.ExtractLaneSVecI8x16,c,e)}
,extract_lane_u:function(c,e){return a._BinaryenSIMDExtract(b,a.ExtractLaneUVecI8x16,c,e)}
,replace_lane:function(c,e,g){return a._BinaryenSIMDReplace(b,a.ReplaceLaneVecI8x16,c,e,g)}
,eq:function(c,e){return a._BinaryenBinary(b,a.EqVecI8x16,c,e)}
,ne:function(c,e){return a._BinaryenBinary(b,a.NeVecI8x16,
c,e)}
,lt_s:function(c,e){return a._BinaryenBinary(b,a.LtSVecI8x16,c,e)}
,lt_u:function(c,e){return a._BinaryenBinary(b,a.LtUVecI8x16,c,e)}
,gt_s:function(c,e){return a._BinaryenBinary(b,a.GtSVecI8x16,c,e)}
,gt_u:function(c,e){return a._BinaryenBinary(b,a.GtUVecI8x16,c,e)}
,le_s:function(c,e){return a._BinaryenBinary(b,a.LeSVecI8x16,c,e)}
,le_u:function(c,e){return a._BinaryenBinary(b,a.LeUVecI8x16,c,e)}
,ge_s:function(c,e){return a._BinaryenBinary(b,a.GeSVecI8x16,c,e)}
,ge_u:function(c,e){return a._BinaryenBinary(b,
a.GeUVecI8x16,c,e)}
,abs:function(c){return a._BinaryenUnary(b,a.AbsVecI8x16,c)}
,neg:function(c){return a._BinaryenUnary(b,a.NegVecI8x16,c)}
,all_true:function(c){return a._BinaryenUnary(b,a.AllTrueVecI8x16,c)}
,bitmask:function(c){return a._BinaryenUnary(b,a.BitmaskVecI8x16,c)}
,popcnt:function(c){return a._BinaryenUnary(b,a.PopcntVecI8x16,c)}
,shl:function(c,e){return a._BinaryenSIMDShift(b,a.ShlVecI8x16,c,e)}
,shr_s:function(c,e){return a._BinaryenSIMDShift(b,a.ShrSVecI8x16,c,e)}
,shr_u:function(c,e){return a._BinaryenSIMDShift(b,
a.ShrUVecI8x16,c,e)}
,add:function(c,e){return a._BinaryenBinary(b,a.AddVecI8x16,c,e)}
,add_saturate_s:function(c,e){return a._BinaryenBinary(b,a.AddSatSVecI8x16,c,e)}
,add_saturate_u:function(c,e){return a._BinaryenBinary(b,a.AddSatUVecI8x16,c,e)}
,sub:function(c,e){return a._BinaryenBinary(b,a.SubVecI8x16,c,e)}
,sub_saturate_s:function(c,e){return a._BinaryenBinary(b,a.SubSatSVecI8x16,c,e)}
,sub_saturate_u:function(c,e){return a._BinaryenBinary(b,a.SubSatUVecI8x16,c,e)}
,min_s:function(c,e){return a._BinaryenBinary(b,
a.MinSVecI8x16,c,e)}
,min_u:function(c,e){return a._BinaryenBinary(b,a.MinUVecI8x16,c,e)}
,max_s:function(c,e){return a._BinaryenBinary(b,a.MaxSVecI8x16,c,e)}
,max_u:function(c,e){return a._BinaryenBinary(b,a.MaxUVecI8x16,c,e)}
,avgr_u:function(c,e){return a._BinaryenBinary(b,a.AvgrUVecI8x16,c,e)}
,narrow_i16x8_s:function(c,e){return a._BinaryenBinary(b,a.NarrowSVecI16x8ToVecI8x16,c,e)}
,narrow_i16x8_u:function(c,e){return a._BinaryenBinary(b,a.NarrowUVecI16x8ToVecI8x16,c,e)}
}
;d.i16x8={splat:function(c){return a._BinaryenUnary(b,
a.SplatVecI16x8,c)}
,extract_lane_s:function(c,e){return a._BinaryenSIMDExtract(b,a.ExtractLaneSVecI16x8,c,e)}
,extract_lane_u:function(c,e){return a._BinaryenSIMDExtract(b,a.ExtractLaneUVecI16x8,c,e)}
,replace_lane:function(c,e,g){return a._BinaryenSIMDReplace(b,a.ReplaceLaneVecI16x8,c,e,g)}
,eq:function(c,e){return a._BinaryenBinary(b,a.EqVecI16x8,c,e)}
,ne:function(c,e){return a._BinaryenBinary(b,a.NeVecI16x8,c,e)}
,lt_s:function(c,e){return a._BinaryenBinary(b,a.LtSVecI16x8,c,e)}
,lt_u:function(c,e){return a._BinaryenBinary(b,
a.LtUVecI16x8,c,e)}
,gt_s:function(c,e){return a._BinaryenBinary(b,a.GtSVecI16x8,c,e)}
,gt_u:function(c,e){return a._BinaryenBinary(b,a.GtUVecI16x8,c,e)}
,le_s:function(c,e){return a._BinaryenBinary(b,a.LeSVecI16x8,c,e)}
,le_u:function(c,e){return a._BinaryenBinary(b,a.LeUVecI16x8,c,e)}
,ge_s:function(c,e){return a._BinaryenBinary(b,a.GeSVecI16x8,c,e)}
,ge_u:function(c,e){return a._BinaryenBinary(b,a.GeUVecI16x8,c,e)}
,abs:function(c){return a._BinaryenUnary(b,a.AbsVecI16x8,c)}
,neg:function(c){return a._BinaryenUnary(b,
a.NegVecI16x8,c)}
,all_true:function(c){return a._BinaryenUnary(b,a.AllTrueVecI16x8,c)}
,bitmask:function(c){return a._BinaryenUnary(b,a.BitmaskVecI16x8,c)}
,shl:function(c,e){return a._BinaryenSIMDShift(b,a.ShlVecI16x8,c,e)}
,shr_s:function(c,e){return a._BinaryenSIMDShift(b,a.ShrSVecI16x8,c,e)}
,shr_u:function(c,e){return a._BinaryenSIMDShift(b,a.ShrUVecI16x8,c,e)}
,add:function(c,e){return a._BinaryenBinary(b,a.AddVecI16x8,c,e)}
,add_saturate_s:function(c,e){return a._BinaryenBinary(b,a.AddSatSVecI16x8,
c,e)}
,add_saturate_u:function(c,e){return a._BinaryenBinary(b,a.AddSatUVecI16x8,c,e)}
,sub:function(c,e){return a._BinaryenBinary(b,a.SubVecI16x8,c,e)}
,sub_saturate_s:function(c,e){return a._BinaryenBinary(b,a.SubSatSVecI16x8,c,e)}
,sub_saturate_u:function(c,e){return a._BinaryenBinary(b,a.SubSatUVecI16x8,c,e)}
,mul:function(c,e){return a._BinaryenBinary(b,a.MulVecI16x8,c,e)}
,min_s:function(c,e){return a._BinaryenBinary(b,a.MinSVecI16x8,c,e)}
,min_u:function(c,e){return a._BinaryenBinary(b,a.MinUVecI16x8,
c,e)}
,max_s:function(c,e){return a._BinaryenBinary(b,a.MaxSVecI16x8,c,e)}
,max_u:function(c,e){return a._BinaryenBinary(b,a.MaxUVecI16x8,c,e)}
,avgr_u:function(c,e){return a._BinaryenBinary(b,a.AvgrUVecI16x8,c,e)}
,q15mulr_sat_s:function(c,e){return a._BinaryenBinary(b,a.Q15MulrSatSVecI16x8,c,e)}
,extmul_low_i8x16_s:function(c,e){return a._BinaryenBinary(b,a.ExtMulLowSVecI16x8,c,e)}
,extmul_high_i8x16_s:function(c,e){return a._BinaryenBinary(b,a.ExtMulHighSVecI16x8,c,e)}
,extmul_low_i8x16_u:function(c,
e){return a._BinaryenBinary(b,a.ExtMulLowUVecI16x8,c,e)}
,extmul_high_i8x16_u:function(c,e){return a._BinaryenBinary(b,a.ExtMulHighUVecI16x8,c,e)}
,extadd_pairwise_i8x16_s:function(c){return a._BinaryenUnary(b,a.ExtAddPairwiseSVecI8x16ToI16x8,c)}
,extadd_pairwise_i8x16_u:function(c){return a._BinaryenUnary(b,a.ExtAddPairwiseUVecI8x16ToI16x8,c)}
,narrow_i32x4_s:function(c,e){return a._BinaryenBinary(b,a.NarrowSVecI32x4ToVecI16x8,c,e)}
,narrow_i32x4_u:function(c,e){return a._BinaryenBinary(b,a.NarrowUVecI32x4ToVecI16x8,
c,e)}
,extend_low_i8x16_s:function(c){return a._BinaryenUnary(b,a.ExtendLowSVecI8x16ToVecI16x8,c)}
,extend_high_i8x16_s:function(c){return a._BinaryenUnary(b,a.ExtendHighSVecI8x16ToVecI16x8,c)}
,extend_low_i8x16_u:function(c){return a._BinaryenUnary(b,a.ExtendLowUVecI8x16ToVecI16x8,c)}
,extend_high_i8x16_u:function(c){return a._BinaryenUnary(b,a.ExtendHighUVecI8x16ToVecI16x8,c)}
}
;d.i32x4={splat:function(c){return a._BinaryenUnary(b,a.SplatVecI32x4,c)}
,extract_lane:function(c,e){return a._BinaryenSIMDExtract(b,
a.ExtractLaneVecI32x4,c,e)}
,replace_lane:function(c,e,g){return a._BinaryenSIMDReplace(b,a.ReplaceLaneVecI32x4,c,e,g)}
,eq:function(c,e){return a._BinaryenBinary(b,a.EqVecI32x4,c,e)}
,ne:function(c,e){return a._BinaryenBinary(b,a.NeVecI32x4,c,e)}
,lt_s:function(c,e){return a._BinaryenBinary(b,a.LtSVecI32x4,c,e)}
,lt_u:function(c,e){return a._BinaryenBinary(b,a.LtUVecI32x4,c,e)}
,gt_s:function(c,e){return a._BinaryenBinary(b,a.GtSVecI32x4,c,e)}
,gt_u:function(c,e){return a._BinaryenBinary(b,a.GtUVecI32x4,
c,e)}
,le_s:function(c,e){return a._BinaryenBinary(b,a.LeSVecI32x4,c,e)}
,le_u:function(c,e){return a._BinaryenBinary(b,a.LeUVecI32x4,c,e)}
,ge_s:function(c,e){return a._BinaryenBinary(b,a.GeSVecI32x4,c,e)}
,ge_u:function(c,e){return a._BinaryenBinary(b,a.GeUVecI32x4,c,e)}
,abs:function(c){return a._BinaryenUnary(b,a.AbsVecI32x4,c)}
,neg:function(c){return a._BinaryenUnary(b,a.NegVecI32x4,c)}
,all_true:function(c){return a._BinaryenUnary(b,a.AllTrueVecI32x4,c)}
,bitmask:function(c){return a._BinaryenUnary(b,
a.BitmaskVecI32x4,c)}
,shl:function(c,e){return a._BinaryenSIMDShift(b,a.ShlVecI32x4,c,e)}
,shr_s:function(c,e){return a._BinaryenSIMDShift(b,a.ShrSVecI32x4,c,e)}
,shr_u:function(c,e){return a._BinaryenSIMDShift(b,a.ShrUVecI32x4,c,e)}
,add:function(c,e){return a._BinaryenBinary(b,a.AddVecI32x4,c,e)}
,sub:function(c,e){return a._BinaryenBinary(b,a.SubVecI32x4,c,e)}
,mul:function(c,e){return a._BinaryenBinary(b,a.MulVecI32x4,c,e)}
,min_s:function(c,e){return a._BinaryenBinary(b,a.MinSVecI32x4,c,e)}
,min_u:function(c,
e){return a._BinaryenBinary(b,a.MinUVecI32x4,c,e)}
,max_s:function(c,e){return a._BinaryenBinary(b,a.MaxSVecI32x4,c,e)}
,max_u:function(c,e){return a._BinaryenBinary(b,a.MaxUVecI32x4,c,e)}
,dot_i16x8_s:function(c,e){return a._BinaryenBinary(b,a.DotSVecI16x8ToVecI32x4,c,e)}
,extmul_low_i16x8_s:function(c,e){return a._BinaryenBinary(b,a.ExtMulLowSVecI32x4,c,e)}
,extmul_high_i16x8_s:function(c,e){return a._BinaryenBinary(b,a.ExtMulHighSVecI32x4,c,e)}
,extmul_low_i16x8_u:function(c,e){return a._BinaryenBinary(b,
a.ExtMulLowUVecI32x4,c,e)}
,extmul_high_i16x8_u:function(c,e){return a._BinaryenBinary(b,a.ExtMulHighUVecI32x4,c,e)}
,extadd_pairwise_i16x8_s:function(c){return a._BinaryenUnary(b,a.ExtAddPairwiseSVecI16x8ToI32x4,c)}
,extadd_pairwise_i16x8_u:function(c){return a._BinaryenUnary(b,a.ExtAddPairwiseUVecI16x8ToI32x4,c)}
,trunc_sat_f32x4_s:function(c){return a._BinaryenUnary(b,a.TruncSatSVecF32x4ToVecI32x4,c)}
,trunc_sat_f32x4_u:function(c){return a._BinaryenUnary(b,a.TruncSatUVecF32x4ToVecI32x4,c)}
,extend_low_i16x8_s:function(c){return a._BinaryenUnary(b,
a.ExtendLowSVecI16x8ToVecI32x4,c)}
,extend_high_i16x8_s:function(c){return a._BinaryenUnary(b,a.ExtendHighSVecI16x8ToVecI32x4,c)}
,extend_low_i16x8_u:function(c){return a._BinaryenUnary(b,a.ExtendLowUVecI16x8ToVecI32x4,c)}
,extend_high_i16x8_u:function(c){return a._BinaryenUnary(b,a.ExtendHighUVecI16x8ToVecI32x4,c)}
,trunc_sat_f64x2_s_zero:function(c){return a._BinaryenUnary(b,a.TruncSatZeroSVecF64x2ToVecI32x4,c)}
,trunc_sat_f64x2_u_zero:function(c){return a._BinaryenUnary(b,a.TruncSatZeroUVecF64x2ToVecI32x4,
c)}
}
;d.i64x2={splat:function(c){return a._BinaryenUnary(b,a.SplatVecI64x2,c)}
,extract_lane:function(c,e){return a._BinaryenSIMDExtract(b,a.ExtractLaneVecI64x2,c,e)}
,replace_lane:function(c,e,g){return a._BinaryenSIMDReplace(b,a.ReplaceLaneVecI64x2,c,e,g)}
,eq:function(c,e){return a._BinaryenBinary(b,a.EqVecI64x2,c,e)}
,ne:function(c,e){return a._BinaryenBinary(b,a.NeVecI64x2,c,e)}
,lt_s:function(c,e){return a._BinaryenBinary(b,a.LtSVecI64x2,c,e)}
,gt_s:function(c,e){return a._BinaryenBinary(b,a.GtSVecI64x2,
c,e)}
,le_s:function(c,e){return a._BinaryenBinary(b,a.LeSVecI64x2,c,e)}
,ge_s:function(c,e){return a._BinaryenBinary(b,a.GeSVecI64x2,c,e)}
,abs:function(c){return a._BinaryenUnary(b,a.AbsVecI64x2,c)}
,neg:function(c){return a._BinaryenUnary(b,a.NegVecI64x2,c)}
,all_true:function(c){return a._BinaryenUnary(b,a.AllTrueVecI64x2,c)}
,bitmask:function(c){return a._BinaryenUnary(b,a.BitmaskVecI64x2,c)}
,shl:function(c,e){return a._BinaryenSIMDShift(b,a.ShlVecI64x2,c,e)}
,shr_s:function(c,e){return a._BinaryenSIMDShift(b,
a.ShrSVecI64x2,c,e)}
,shr_u:function(c,e){return a._BinaryenSIMDShift(b,a.ShrUVecI64x2,c,e)}
,add:function(c,e){return a._BinaryenBinary(b,a.AddVecI64x2,c,e)}
,sub:function(c,e){return a._BinaryenBinary(b,a.SubVecI64x2,c,e)}
,mul:function(c,e){return a._BinaryenBinary(b,a.MulVecI64x2,c,e)}
,extmul_low_i32x4_s:function(c,e){return a._BinaryenBinary(b,a.ExtMulLowSVecI64x2,c,e)}
,extmul_high_i32x4_s:function(c,e){return a._BinaryenBinary(b,a.ExtMulHighSVecI64x2,c,e)}
,extmul_low_i32x4_u:function(c,e){return a._BinaryenBinary(b,
a.ExtMulLowUVecI64x2,c,e)}
,extmul_high_i32x4_u:function(c,e){return a._BinaryenBinary(b,a.ExtMulHighUVecI64x2,c,e)}
,extend_low_i32x4_s:function(c){return a._BinaryenUnary(b,a.ExtendLowSVecI32x4ToVecI64x2,c)}
,extend_high_i32x4_s:function(c){return a._BinaryenUnary(b,a.ExtendHighSVecI32x4ToVecI64x2,c)}
,extend_low_i32x4_u:function(c){return a._BinaryenUnary(b,a.ExtendLowUVecI32x4ToVecI64x2,c)}
,extend_high_i32x4_u:function(c){return a._BinaryenUnary(b,a.ExtendHighUVecI32x4ToVecI64x2,c)}
}
;d.f32x4={splat:function(c){return a._BinaryenUnary(b,
a.SplatVecF32x4,c)}
,extract_lane:function(c,e){return a._BinaryenSIMDExtract(b,a.ExtractLaneVecF32x4,c,e)}
,replace_lane:function(c,e,g){return a._BinaryenSIMDReplace(b,a.ReplaceLaneVecF32x4,c,e,g)}
,eq:function(c,e){return a._BinaryenBinary(b,a.EqVecF32x4,c,e)}
,ne:function(c,e){return a._BinaryenBinary(b,a.NeVecF32x4,c,e)}
,lt:function(c,e){return a._BinaryenBinary(b,a.LtVecF32x4,c,e)}
,gt:function(c,e){return a._BinaryenBinary(b,a.GtVecF32x4,c,e)}
,le:function(c,e){return a._BinaryenBinary(b,a.LeVecF32x4,
c,e)}
,ge:function(c,e){return a._BinaryenBinary(b,a.GeVecF32x4,c,e)}
,abs:function(c){return a._BinaryenUnary(b,a.AbsVecF32x4,c)}
,neg:function(c){return a._BinaryenUnary(b,a.NegVecF32x4,c)}
,sqrt:function(c){return a._BinaryenUnary(b,a.SqrtVecF32x4,c)}
,add:function(c,e){return a._BinaryenBinary(b,a.AddVecF32x4,c,e)}
,sub:function(c,e){return a._BinaryenBinary(b,a.SubVecF32x4,c,e)}
,mul:function(c,e){return a._BinaryenBinary(b,a.MulVecF32x4,c,e)}
,div:function(c,e){return a._BinaryenBinary(b,a.DivVecF32x4,
c,e)}
,min:function(c,e){return a._BinaryenBinary(b,a.MinVecF32x4,c,e)}
,max:function(c,e){return a._BinaryenBinary(b,a.MaxVecF32x4,c,e)}
,pmin:function(c,e){return a._BinaryenBinary(b,a.PMinVecF32x4,c,e)}
,pmax:function(c,e){return a._BinaryenBinary(b,a.PMaxVecF32x4,c,e)}
,ceil:function(c){return a._BinaryenUnary(b,a.CeilVecF32x4,c)}
,floor:function(c){return a._BinaryenUnary(b,a.FloorVecF32x4,c)}
,trunc:function(c){return a._BinaryenUnary(b,a.TruncVecF32x4,c)}
,nearest:function(c){return a._BinaryenUnary(b,
a.NearestVecF32x4,c)}
,convert_i32x4_s:function(c){return a._BinaryenUnary(b,a.ConvertSVecI32x4ToVecF32x4,c)}
,convert_i32x4_u:function(c){return a._BinaryenUnary(b,a.ConvertUVecI32x4ToVecF32x4,c)}
,demote_f64x2_zero:function(c){return a._BinaryenUnary(b,a.DemoteZeroVecF64x2ToVecF32x4,c)}
}
;d.f64x2={splat:function(c){return a._BinaryenUnary(b,a.SplatVecF64x2,c)}
,extract_lane:function(c,e){return a._BinaryenSIMDExtract(b,a.ExtractLaneVecF64x2,c,e)}
,replace_lane:function(c,e,g){return a._BinaryenSIMDReplace(b,
a.ReplaceLaneVecF64x2,c,e,g)}
,eq:function(c,e){return a._BinaryenBinary(b,a.EqVecF64x2,c,e)}
,ne:function(c,e){return a._BinaryenBinary(b,a.NeVecF64x2,c,e)}
,lt:function(c,e){return a._BinaryenBinary(b,a.LtVecF64x2,c,e)}
,gt:function(c,e){return a._BinaryenBinary(b,a.GtVecF64x2,c,e)}
,le:function(c,e){return a._BinaryenBinary(b,a.LeVecF64x2,c,e)}
,ge:function(c,e){return a._BinaryenBinary(b,a.GeVecF64x2,c,e)}
,abs:function(c){return a._BinaryenUnary(b,a.AbsVecF64x2,c)}
,neg:function(c){return a._BinaryenUnary(b,
a.NegVecF64x2,c)}
,sqrt:function(c){return a._BinaryenUnary(b,a.SqrtVecF64x2,c)}
,add:function(c,e){return a._BinaryenBinary(b,a.AddVecF64x2,c,e)}
,sub:function(c,e){return a._BinaryenBinary(b,a.SubVecF64x2,c,e)}
,mul:function(c,e){return a._BinaryenBinary(b,a.MulVecF64x2,c,e)}
,div:function(c,e){return a._BinaryenBinary(b,a.DivVecF64x2,c,e)}
,min:function(c,e){return a._BinaryenBinary(b,a.MinVecF64x2,c,e)}
,max:function(c,e){return a._BinaryenBinary(b,a.MaxVecF64x2,c,e)}
,pmin:function(c,e){return a._BinaryenBinary(b,
a.PMinVecF64x2,c,e)}
,pmax:function(c,e){return a._BinaryenBinary(b,a.PMaxVecF64x2,c,e)}
,ceil:function(c){return a._BinaryenUnary(b,a.CeilVecF64x2,c)}
,floor:function(c){return a._BinaryenUnary(b,a.FloorVecF64x2,c)}
,trunc:function(c){return a._BinaryenUnary(b,a.TruncVecF64x2,c)}
,nearest:function(c){return a._BinaryenUnary(b,a.NearestVecF64x2,c)}
,convert_low_i32x4_s:function(c){return a._BinaryenUnary(b,a.ConvertLowSVecI32x4ToVecF64x2,c)}
,convert_low_i32x4_u:function(c){return a._BinaryenUnary(b,a.ConvertLowUVecI32x4ToVecF64x2,
c)}
,promote_low_f32x4:function(c){return a._BinaryenUnary(b,a.PromoteLowVecF32x4ToVecF64x2,c)}
}
;d.funcref={pop:function(){return a._BinaryenPop(b,a.funcref)}
}
;d.externref={pop:function(){return a._BinaryenPop(b,a.externref)}
}
;d.anyref={pop:function(){return a._BinaryenPop(b,a.anyref)}
}
;d.eqref={pop:function(){return a._BinaryenPop(b,a.eqref)}
}
;d.i31ref={pop:function(){return a._BinaryenPop(b,a.i31ref)}
}
;d.dataref={pop:function(){return a._BinaryenPop(b,a.dataref)}
}
;d.stringref={pop:function(){return a._BinaryenPop(b,
a.stringref)}
}
;d.stringview_wtf8={pop:function(){return a._BinaryenPop(b,a.stringview_wtf8)}
}
;d.stringview_wtf16={pop:function(){return a._BinaryenPop(b,a.stringview_wtf16)}
}
;d.stringview_iter={pop:function(){return a._BinaryenPop(b,a.stringview_iter)}
}
;d.ref={["null"](c){return a._BinaryenRefNull(b,c)}
,is_null:function(c){return a._BinaryenRefIs(b,a.RefIsNull,c)}
,is_func:function(c){return a._BinaryenRefIs(b,a.RefIsFunc,c)}
,is_data:function(c){return a._BinaryenRefIs(b,a.RefIsData,c)}
,is_i31:function(c){return a._BinaryenRefIs(b,
a.RefIsI31,c)}
,as_non_null:function(c){return a._BinaryenRefAs(b,a.RefAsNonNull,c)}
,as_func:function(c){return a._BinaryenRefAs(b,a.RefAsFunc,c)}
,as_data:function(c){return a._BinaryenRefAs(b,a.RefAsData,c)}
,as_i31:function(c){return a._BinaryenRefAs(b,a.RefAsI31,c)}
,func:function(c,e){return T(()=>a._BinaryenRefFunc(b,U(c),e))}
,eq:function(c,e){return a._BinaryenRefEq(b,c,e)}
}
;d.select=function(c,e,g,h){return a._BinaryenSelect(b,c,e,g,"undefined"!==typeof h?h:a.auto)}
;d.drop=function(c){return a._BinaryenDrop(b,
c)}
;d["return"]=function(c){return a._BinaryenReturn(b,c)}
;d.nop=function(){return a._BinaryenNop(b)}
;d.unreachable=function(){return a._BinaryenUnreachable(b)}
;d.atomic={fence:function(){return a._BinaryenAtomicFence(b)}
}
;d["try"]=function(c,e,g,h,l){return T(()=>a._BinaryenTry(b,c?U(c):0,e,V(g.map(U)),g.length,V(h),h.length,l?U(l):0))}
;d["throw"]=function(c,e){return T(()=>a._BinaryenThrow(b,U(c),V(e),e.length))}
;d.rethrow=function(c){return a._BinaryenRethrow(b,U(c))}
;d.tuple={make:function(c){return T(()=>
a._BinaryenTupleMake(b,V(c),c.length))}
,extract:function(c,e){return a._BinaryenTupleExtract(b,c,e)}
}
;d.i31={["new"](c){return a._BinaryenI31New(b,c)}
,get_s:function(c){return a._BinaryenI31Get(b,c,1)}
,get_u:function(c){return a._BinaryenI31Get(b,c,0)}
}
;d.addFunction=function(c,e,g,h,l){return T(()=>a._BinaryenAddFunction(b,U(c),e,g,V(h),h.length,l))}
;d.getFunction=function(c){return T(()=>a._BinaryenGetFunction(b,U(c)))}
;d.removeFunction=function(c){return T(()=>a._BinaryenRemoveFunction(b,U(c)))}
;
d.addGlobal=function(c,e,g,h){return T(()=>a._BinaryenAddGlobal(b,U(c),e,g,h))}
;d.getGlobal=function(c){return T(()=>a._BinaryenGetGlobal(b,U(c)))}
;d.addTable=function(c,e,g,h=a._BinaryenTypeFuncref()){return T(()=>a._BinaryenAddTable(b,U(c),e,g,h))}
;d.getTable=function(c){return T(()=>a._BinaryenGetTable(b,U(c)))}
;d.addActiveElementSegment=function(c,e,g,h=d.i32["const"](0)){return T(()=>a._BinaryenAddActiveElementSegment(b,U(c),U(e),V(g.map(U)),g.length,h))}
;d.addPassiveElementSegment=function(c,
e){return T(()=>a._BinaryenAddPassiveElementSegment(b,U(c),V(e.map(U)),e.length))}
;d.getElementSegment=function(c){return T(()=>a._BinaryenGetElementSegment(b,U(c)))}
;d.getTableSegments=function(c){var e=a._BinaryenGetNumElementSegments(b);c=t(a._BinaryenTableGetName(c));for(var g=[],h=0;h<e;h++){var l=a._BinaryenGetElementSegmentByIndex(b,h),n=t(a._BinaryenElementSegmentGetTable(l));c===n&&g.push(l)}
return g}
;d.removeGlobal=function(c){return T(()=>a._BinaryenRemoveGlobal(b,U(c)))}
;d.removeTable=
function(c){return T(()=>a._BinaryenRemoveTable(b,U(c)))}
;d.removeElementSegment=function(c){return T(()=>a._BinaryenRemoveElementSegment(b,U(c)))}
;d.addTag=function(c,e,g){return T(()=>a._BinaryenAddTag(b,U(c),e,g))}
;d.getTag=function(c){return T(()=>a._BinaryenGetTag(b,U(c)))}
;d.removeTag=function(c){return T(()=>a._BinaryenRemoveTag(b,U(c)))}
;d.addFunctionImport=function(c,e,g,h,l){return T(()=>a._BinaryenAddFunctionImport(b,U(c),U(e),U(g),h,l))}
;d.addTableImport=function(c,e,g){return T(()=>a._BinaryenAddTableImport(b,
U(c),U(e),U(g)))}
;d.addMemoryImport=function(c,e,g,h){return T(()=>a._BinaryenAddMemoryImport(b,U(c),U(e),U(g),h))}
;d.addGlobalImport=function(c,e,g,h,l){return T(()=>a._BinaryenAddGlobalImport(b,U(c),U(e),U(g),h,l))}
;d.addTagImport=function(c,e,g,h,l){return T(()=>a._BinaryenAddTagImport(b,U(c),U(e),U(g),h,l))}
;d.addExport=d.addFunctionExport=function(c,e){return T(()=>a._BinaryenAddFunctionExport(b,U(c),U(e)))}
;d.addTableExport=function(c,e){return T(()=>a._BinaryenAddTableExport(b,U(c),U(e)))}
;
d.addMemoryExport=function(c,e){return T(()=>a._BinaryenAddMemoryExport(b,U(c),U(e)))}
;d.addGlobalExport=function(c,e){return T(()=>a._BinaryenAddGlobalExport(b,U(c),U(e)))}
;d.addTagExport=function(c,e){return T(()=>a._BinaryenAddTagExport(b,U(c),U(e)))}
;d.removeExport=function(c){return T(()=>a._BinaryenRemoveExport(b,U(c)))}
;d.setMemory=function(c,e,g,h=[],l=!1,n=!1,w=null){return T(()=>{const y=h.length,C=Array(y),F=Array(y),m=Array(y),B=Array(y);for(let A=0;A<y;A++){const {data:J,offset:M,passive:je}
                                                            =
h[A];C[A]=O(J.length);v.set(J,C[A]);F[A]=J.length;m[A]=je;B[A]=M}
return a._BinaryenSetMemory(b,c,e,U(g),V(C),ee(m),V(B),V(F),y,l,n,U(w))}
)}
;d.hasMemory=function(){return!!a._BinaryenHasMemory(b)}
;d.getMemoryInfo=function(c){var e={module:t(a._BinaryenMemoryImportGetModule(b,U(c))),base:t(a._BinaryenMemoryImportGetBase(b,U(c))),initial:a._BinaryenMemoryGetInitial(b,U(c)),shared:!!a._BinaryenMemoryIsShared(b,U(c)),is64:!!a._BinaryenMemoryIs64(b,U(c))}
;a._BinaryenMemoryHasMax(b,U(c))&&(e.max=a._BinaryenMemoryGetMax(b,
U(c)));return e}
;d.getNumMemorySegments=function(){return a._BinaryenGetNumMemorySegments(b)}
;d.getMemorySegmentInfoByIndex=function(c){const e=!!a._BinaryenGetMemorySegmentPassive(b,c);var g=null;e||(g=a._BinaryenGetMemorySegmentByteOffset(b,c));const h=a._BinaryenGetMemorySegmentByteLength(b,c),l=dc(h);a._BinaryenCopyMemorySegmentData(b,c,l);c=new Uint8Array(h);c.set(new Uint8Array(za,l,h));H(l);return{offset:g,data:c.buffer,passive:e}
}
;d.setStart=function(c){return a._BinaryenSetStart(b,c)}
;d.getFeatures=
function(){return a._BinaryenModuleGetFeatures(b)}
;d.setFeatures=function(c){a._BinaryenModuleSetFeatures(b,c)}
;d.addCustomSection=function(c,e){return T(()=>a._BinaryenAddCustomSection(b,U(c),ee(e),e.length))}
;d.getExport=function(c){return T(()=>a._BinaryenGetExport(b,U(c)))}
;d.getNumExports=function(){return a._BinaryenGetNumExports(b)}
;d.getExportByIndex=function(c){return a._BinaryenGetExportByIndex(b,c)}
;d.getNumFunctions=function(){return a._BinaryenGetNumFunctions(b)}
;d.getFunctionByIndex=
function(c){return a._BinaryenGetFunctionByIndex(b,c)}
;d.getNumGlobals=function(){return a._BinaryenGetNumGlobals(b)}
;d.getNumTables=function(){return a._BinaryenGetNumTables(b)}
;d.getNumElementSegments=function(){return a._BinaryenGetNumElementSegments(b)}
;d.getGlobalByIndex=function(c){return a._BinaryenGetGlobalByIndex(b,c)}
;d.getTableByIndex=function(c){return a._BinaryenGetTableByIndex(b,c)}
;d.getElementSegmentByIndex=function(c){return a._BinaryenGetElementSegmentByIndex(b,c)}
;d.emitText=function(){let c=
a._BinaryenModuleAllocateAndWriteText(b),e=t(c);c&&H(c);return e}
;d.emitStackIR=function(c){c=a._BinaryenModuleAllocateAndWriteStackIR(b,c);let e=t(c);c&&H(c);return e}
;d.emitAsmjs=function(){const c=q;let e="";q=g=>{e+=g+"\n"}
;a._BinaryenModulePrintAsmjs(b);q=c;return e}
;d.validate=function(){return a._BinaryenModuleValidate(b)}
;d.optimize=function(){return a._BinaryenModuleOptimize(b)}
;d.optimizeFunction=function(c){"string"===typeof c&&(c=d.getFunction(c));return a._BinaryenFunctionOptimize(c,
b)}
;d.runPasses=function(c){return T(()=>a._BinaryenModuleRunPasses(b,V(c.map(U)),c.length))}
;d.runPassesOnFunction=function(c,e){"string"===typeof c&&(c=d.getFunction(c));return T(()=>a._BinaryenFunctionRunPasses(c,b,V(e.map(U)),e.length))}
;d.autoDrop=function(){return a._BinaryenModuleAutoDrop(b)}
;d.dispose=function(){a._BinaryenModuleDispose(b)}
;d.emitBinary=function(c){return T(()=>{var e=O(qd());a._BinaryenModuleAllocateAndWrite(e,b,U(c));const g=z[e>>>2],h=z[(e>>>2)+1];e=z[(e>>>2)+2];try{const l=
new Uint8Array(h);l.set(u.subarray(g,g+h));return"undefined"===typeof c?l:{binary:l,sourceMap:t(e)}
}
finally{H(g),e&&H(e)}
}
)}
;d.interpret=function(){return a._BinaryenModuleInterpret(b)}
;d.addDebugInfoFileName=function(c){return T(()=>a._BinaryenModuleAddDebugInfoFileName(b,U(c)))}
;d.getDebugInfoFileName=function(c){return t(a._BinaryenModuleGetDebugInfoFileName(b,c))}
;d.setDebugLocation=function(c,e,g,h,l){return a._BinaryenFunctionSetDebugLocation(c,e,g,h,l)}
;d.copyExpression=function(c){return a._BinaryenExpressionCopy(c,
b)}
;return d}
a.wrapModule=ge;
a.Relooper=function(b){b&&"object"===typeof b&&b.ptr&&b.block&&b["if"]||r();const d=a._RelooperCreate(b.ptr);this.ptr=d;this.addBlock=function(f){return a._RelooperAddBlock(d,f)}
;this.addBranch=function(f,c,e,g){return a._RelooperAddBranch(f,c,e,g)}
;this.addBlockWithSwitch=function(f,c){return a._RelooperAddBlockWithSwitch(d,f,c)}
;this.addBranchForSwitch=function(f,c,e,g){return T(()=>a._RelooperAddBranchForSwitch(f,c,V(e),e.length,g))}
;this.renderAndDispose=function(f,c){return a._RelooperRenderAndDispose(d,
f,c)}
}
;a.ExpressionRunner=function(b,d,f,c){const e=a._ExpressionRunnerCreate(b.ptr,d,f,c);this.ptr=e;this.setLocalValue=function(g,h){return!!a._ExpressionRunnerSetLocalValue(e,g,h)}
;this.setGlobalValue=function(g,h){return T(()=>!!a._ExpressionRunnerSetGlobalValue(e,U(g),h))}
;this.runAndDispose=function(g){return a._ExpressionRunnerRunAndDispose(e,g)}
}
;function W(b,d,f){d=d(b);const c=Array(d);for(let e=0;e<d;++e)c[e]=f(b,e);return c}

function X(b,d,f,c,e,g){const h=d.length;f=f(b);let l=0;for(;l<h;)l<f?c(b,l,d[l]):e(b,d[l]),++l;for(;f>l;)g(b,--f)}
a.getExpressionId=function(b){return a._BinaryenExpressionGetId(b)}
;a.getExpressionType=function(b){return a._BinaryenExpressionGetType(b)}
;
a.getExpressionInfo=function(b){const d=a._BinaryenExpressionGetId(b),f=a._BinaryenExpressionGetType(b);switch(d){case a.BlockId:return{id:d,type:f,name:t(a._BinaryenBlockGetName(b)),children:W(b,a._BinaryenBlockGetNumChildren,a._BinaryenBlockGetChildAt)}
;case a.IfId:return{id:d,type:f,condition:a._BinaryenIfGetCondition(b),ifTrue:a._BinaryenIfGetIfTrue(b),ifFalse:a._BinaryenIfGetIfFalse(b)}
;case a.LoopId:return{id:d,type:f,name:t(a._BinaryenLoopGetName(b)),body:a._BinaryenLoopGetBody(b)}
;case a.BreakId:return{id:d,
type:f,name:t(a._BinaryenBreakGetName(b)),condition:a._BinaryenBreakGetCondition(b),value:a._BinaryenBreakGetValue(b)}
;case a.SwitchId:return{id:d,type:f,names:W(b,a._BinaryenSwitchGetNumNames,a._BinaryenSwitchGetNameAt).map(e=>t(e)),defaultName:t(a._BinaryenSwitchGetDefaultName(b)),condition:a._BinaryenSwitchGetCondition(b),value:a._BinaryenSwitchGetValue(b)}
;case a.CallId:return{id:d,type:f,isReturn:!!a._BinaryenCallIsReturn(b),target:t(a._BinaryenCallGetTarget(b)),operands:W(b,a._BinaryenCallGetNumOperands,
a._BinaryenCallGetOperandAt)}
;case a.CallIndirectId:return{id:d,type:f,isReturn:!!a._BinaryenCallIndirectIsReturn(b),target:a._BinaryenCallIndirectGetTarget(b),table:a._BinaryenCallIndirectGetTable(b),operands:W(b,a._BinaryenCallIndirectGetNumOperands,a._BinaryenCallIndirectGetOperandAt)}
;case a.LocalGetId:return{id:d,type:f,index:a._BinaryenLocalGetGetIndex(b)}
;case a.LocalSetId:return{id:d,type:f,isTee:!!a._BinaryenLocalSetIsTee(b),index:a._BinaryenLocalSetGetIndex(b),value:a._BinaryenLocalSetGetValue(b)}
;
case a.GlobalGetId:return{id:d,type:f,name:t(a._BinaryenGlobalGetGetName(b))}
;case a.GlobalSetId:return{id:d,type:f,name:t(a._BinaryenGlobalSetGetName(b)),value:a._BinaryenGlobalSetGetValue(b)}
;case a.TableGetId:return{id:d,type:f,table:t(a._BinaryenTableGetGetTable(b)),index:a._BinaryenTableGetGetIndex(b)}
;case a.TableSetId:return{id:d,type:f,table:t(a._BinaryenTableSetGetTable(b)),index:a._BinaryenTableSetGetIndex(b),value:a._BinaryenTableSetGetValue(b)}
;case a.TableSizeId:return{id:d,type:f,table:t(a._BinaryenTableSizeGetTable(b))}
;
case a.TableGrowId:return{id:d,type:f,table:t(a._BinaryenTableGrowGetTable(b)),value:a._BinaryenTableGrowGetValue(b),delta:a._BinaryenTableGrowGetDelta(b)}
;case a.LoadId:return{id:d,type:f,isAtomic:!!a._BinaryenLoadIsAtomic(b),isSigned:!!a._BinaryenLoadIsSigned(b),offset:a._BinaryenLoadGetOffset(b),bytes:a._BinaryenLoadGetBytes(b),align:a._BinaryenLoadGetAlign(b),ptr:a._BinaryenLoadGetPtr(b)}
;case a.StoreId:return{id:d,type:f,isAtomic:!!a._BinaryenStoreIsAtomic(b),offset:a._BinaryenStoreGetOffset(b),
bytes:a._BinaryenStoreGetBytes(b),align:a._BinaryenStoreGetAlign(b),ptr:a._BinaryenStoreGetPtr(b),value:a._BinaryenStoreGetValue(b)}
;case a.ConstId:let c;switch(f){case a.i32:c=a._BinaryenConstGetValueI32(b);break;case a.i64:c={low:a._BinaryenConstGetValueI64Low(b),high:a._BinaryenConstGetValueI64High(b)}
;break;case a.f32:c=a._BinaryenConstGetValueF32(b);break;case a.f64:c=a._BinaryenConstGetValueF64(b);break;case a.v128:T(()=>{const e=O(16);a._BinaryenConstGetValueV128(b,e);c=Array(16);for(let g=
0;16>g;g++)c[g]=u[e+g]}
);break;default:throw Error("unexpected type: "+f);}
return{id:d,type:f,value:c}
;case a.UnaryId:return{id:d,type:f,op:a._BinaryenUnaryGetOp(b),value:a._BinaryenUnaryGetValue(b)}
;case a.BinaryId:return{id:d,type:f,op:a._BinaryenBinaryGetOp(b),left:a._BinaryenBinaryGetLeft(b),right:a._BinaryenBinaryGetRight(b)}
;case a.SelectId:return{id:d,type:f,ifTrue:a._BinaryenSelectGetIfTrue(b),ifFalse:a._BinaryenSelectGetIfFalse(b),condition:a._BinaryenSelectGetCondition(b)}
;case a.DropId:return{id:d,
type:f,value:a._BinaryenDropGetValue(b)}
;case a.ReturnId:return{id:d,type:f,value:a._BinaryenReturnGetValue(b)}
;case a.NopId:case a.UnreachableId:case a.PopId:return{id:d,type:f}
;case a.MemorySizeId:return{id:d,type:f}
;case a.MemoryGrowId:return{id:d,type:f,delta:a._BinaryenMemoryGrowGetDelta(b)}
;case a.AtomicRMWId:return{id:d,type:f,op:a._BinaryenAtomicRMWGetOp(b),bytes:a._BinaryenAtomicRMWGetBytes(b),offset:a._BinaryenAtomicRMWGetOffset(b),ptr:a._BinaryenAtomicRMWGetPtr(b),value:a._BinaryenAtomicRMWGetValue(b)}
;
case a.AtomicCmpxchgId:return{id:d,type:f,bytes:a._BinaryenAtomicCmpxchgGetBytes(b),offset:a._BinaryenAtomicCmpxchgGetOffset(b),ptr:a._BinaryenAtomicCmpxchgGetPtr(b),expected:a._BinaryenAtomicCmpxchgGetExpected(b),replacement:a._BinaryenAtomicCmpxchgGetReplacement(b)}
;case a.AtomicWaitId:return{id:d,type:f,ptr:a._BinaryenAtomicWaitGetPtr(b),expected:a._BinaryenAtomicWaitGetExpected(b),timeout:a._BinaryenAtomicWaitGetTimeout(b),expectedType:a._BinaryenAtomicWaitGetExpectedType(b)}
;case a.AtomicNotifyId:return{id:d,
type:f,ptr:a._BinaryenAtomicNotifyGetPtr(b),notifyCount:a._BinaryenAtomicNotifyGetNotifyCount(b)}
;case a.AtomicFenceId:return{id:d,type:f,order:a._BinaryenAtomicFenceGetOrder(b)}
;case a.SIMDExtractId:return{id:d,type:f,op:a._BinaryenSIMDExtractGetOp(b),vec:a._BinaryenSIMDExtractGetVec(b),index:a._BinaryenSIMDExtractGetIndex(b)}
;case a.SIMDReplaceId:return{id:d,type:f,op:a._BinaryenSIMDReplaceGetOp(b),vec:a._BinaryenSIMDReplaceGetVec(b),index:a._BinaryenSIMDReplaceGetIndex(b),value:a._BinaryenSIMDReplaceGetValue(b)}
;
case a.SIMDShuffleId:return T(()=>{const e=O(16);a._BinaryenSIMDShuffleGetMask(b,e);const g=Array(16);for(let h=0;16>h;h++)g[h]=u[e+h];return{id:d,type:f,left:a._BinaryenSIMDShuffleGetLeft(b),right:a._BinaryenSIMDShuffleGetRight(b),mask:g}
}
);case a.SIMDTernaryId:return{id:d,type:f,op:a._BinaryenSIMDTernaryGetOp(b),a:a._BinaryenSIMDTernaryGetA(b),b:a._BinaryenSIMDTernaryGetB(b),c:a._BinaryenSIMDTernaryGetC(b)}
;case a.SIMDShiftId:return{id:d,type:f,op:a._BinaryenSIMDShiftGetOp(b),vec:a._BinaryenSIMDShiftGetVec(b),
shift:a._BinaryenSIMDShiftGetShift(b)}
;case a.SIMDLoadId:return{id:d,type:f,op:a._BinaryenSIMDLoadGetOp(b),offset:a._BinaryenSIMDLoadGetOffset(b),align:a._BinaryenSIMDLoadGetAlign(b),ptr:a._BinaryenSIMDLoadGetPtr(b)}
;case a.SIMDLoadStoreLaneId:return{id:d,type:f,op:a._BinaryenSIMDLoadStoreLaneGetOp(b),offset:a._BinaryenSIMDLoadStoreLaneGetOffset(b),align:a._BinaryenSIMDLoadStoreLaneGetAlign(b),index:a._BinaryenSIMDLoadStoreLaneGetIndex(b),ptr:a._BinaryenSIMDLoadStoreLaneGetPtr(b),vec:a._BinaryenSIMDLoadStoreLaneGetVec(b)}
;
case a.MemoryInitId:return{id:d,segment:a._BinaryenMemoryInitGetSegment(b),dest:a._BinaryenMemoryInitGetDest(b),offset:a._BinaryenMemoryInitGetOffset(b),size:a._BinaryenMemoryInitGetSize(b)}
;case a.DataDropId:return{id:d,segment:a._BinaryenDataDropGetSegment(b)}
;case a.MemoryCopyId:return{id:d,dest:a._BinaryenMemoryCopyGetDest(b),source:a._BinaryenMemoryCopyGetSource(b),size:a._BinaryenMemoryCopyGetSize(b)}
;case a.MemoryFillId:return{id:d,dest:a._BinaryenMemoryFillGetDest(b),value:a._BinaryenMemoryFillGetValue(b),
size:a._BinaryenMemoryFillGetSize(b)}
;case a.RefNullId:return{id:d,type:f}
;case a.RefIsId:return{id:d,type:f,op:a._BinaryenRefIsGetOp(b),value:a._BinaryenRefIsGetValue(b)}
;case a.RefAsId:return{id:d,type:f,op:a._BinaryenRefAsGetOp(b),value:a._BinaryenRefAsGetValue(b)}
;case a.RefFuncId:return{id:d,type:f,func:t(a._BinaryenRefFuncGetFunc(b))}
;case a.RefEqId:return{id:d,type:f,left:a._BinaryenRefEqGetLeft(b),right:a._BinaryenRefEqGetRight(b)}
;case a.TryId:return{id:d,type:f,name:t(a._BinaryenTryGetName(b)),
body:a._BinaryenTryGetBody(b),catchTags:W(b,a._BinaryenTryGetNumCatchTags,a._BinaryenTryGetCatchTagAt),catchBodies:W(b,a._BinaryenTryGetNumCatchBodies,a._BinaryenTryGetCatchBodyAt),hasCatchAll:a._BinaryenTryHasCatchAll(b),delegateTarget:t(a._BinaryenTryGetDelegateTarget(b)),isDelegate:a._BinaryenTryIsDelegate(b)}
;case a.ThrowId:return{id:d,type:f,tag:t(a._BinaryenThrowGetTag(b)),operands:W(b,a._BinaryenThrowGetNumOperands,a._BinaryenThrowGetOperandAt)}
;case a.RethrowId:return{id:d,type:f,target:t(a._BinaryenRethrowGetTarget(b))}
;
case a.TupleMakeId:return{id:d,type:f,operands:W(b,a._BinaryenTupleMakeGetNumOperands,a._BinaryenTupleMakeGetOperandAt)}
;case a.TupleExtractId:return{id:d,type:f,tuple:a._BinaryenTupleExtractGetTuple(b),index:a._BinaryenTupleExtractGetIndex(b)}
;case a.I31NewId:return{id:d,type:f,value:a._BinaryenI31NewGetValue(b)}
;case a.I31GetId:return{id:d,type:f,i31:a._BinaryenI31GetGetI31(b),isSigned:!!a._BinaryenI31GetIsSigned(b)}
;default:throw Error("unexpected id: "+d);}
}
;
a.getSideEffects=function(b,d){d||r();return a._BinaryenExpressionGetSideEffects(b,d.ptr)}
;a.createType=function(b){return T(()=>a._BinaryenTypeCreate(V(b),b.length))}
;a.expandType=function(b){return T(()=>{const d=a._BinaryenTypeArity(b),f=O(d<<2);a._BinaryenTypeExpand(b,f);const c=Array(d);for(let e=0;e<d;e++)c[e]=z[(f>>>2)+e];return c}
                                   )}
;
a.getFunctionInfo=function(b){return{name:t(a._BinaryenFunctionGetName(b)),module:t(a._BinaryenFunctionImportGetModule(b)),base:t(a._BinaryenFunctionImportGetBase(b)),params:a._BinaryenFunctionGetParams(b),results:a._BinaryenFunctionGetResults(b),vars:W(b,a._BinaryenFunctionGetNumVars,a._BinaryenFunctionGetVar),body:a._BinaryenFunctionGetBody(b)}
}
;
a.getGlobalInfo=function(b){return{name:t(a._BinaryenGlobalGetName(b)),module:t(a._BinaryenGlobalImportGetModule(b)),base:t(a._BinaryenGlobalImportGetBase(b)),type:a._BinaryenGlobalGetType(b),mutable:!!a._BinaryenGlobalIsMutable(b),init:a._BinaryenGlobalGetInitExpr(b)}
}
;
a.getTableInfo=function(b){var d=!!a._BinaryenTableHasMax(b),f={name:t(a._BinaryenTableGetName(b)),module:t(a._BinaryenTableImportGetModule(b)),base:t(a._BinaryenTableImportGetBase(b)),initial:a._BinaryenTableGetInitial(b)}
;d&&(f.max=a._BinaryenTableGetMax(b));return f}
;
a.getElementSegmentInfo=function(b){var d=a._BinaryenElementSegmentGetLength(b),f=Array(d);for(let e=0;e!==d;++e){var c=a._BinaryenElementSegmentGetData(b,e);f[e]=t(c)}
return{name:t(a._BinaryenElementSegmentGetName(b)),table:t(a._BinaryenElementSegmentGetTable(b)),offset:a._BinaryenElementSegmentGetOffset(b),data:f}
}
;a.getTagInfo=function(b){return{name:t(a._BinaryenTagGetName(b)),module:t(a._BinaryenTagImportGetModule(b)),base:t(a._BinaryenTagImportGetBase(b)),params:a._BinaryenTagGetParams(b),results:a._BinaryenTagGetResults(b)}
}
;
a.getExportInfo=function(b){return{kind:a._BinaryenExportGetKind(b),name:t(a._BinaryenExportGetName(b)),value:t(a._BinaryenExportGetValue(b))}
}
;a.emitText=function(b){if("object"===typeof b)return b.hC();const d=q;let f="";q=c=>{f+=c+"\n"}
;a._BinaryenExpressionPrint(b);q=d;return f}
;Object.defineProperty(a,"readBinary",{writable:!0}
                       );a.readBinary=function(b){const d=dc(b.length);v.set(b,d);b=a._BinaryenModuleRead(d,b.length);H(d);return ge(b)}
;
a.parseText=function(b){const d=dc(b.length+1);Yb(b,d);b=a._BinaryenModuleParse(d);H(d);return ge(b)}
;a.getOptimizeLevel=function(){return a._BinaryenGetOptimizeLevel()}
;a.setOptimizeLevel=function(b){a._BinaryenSetOptimizeLevel(b)}
;a.getShrinkLevel=function(){return a._BinaryenGetShrinkLevel()}
;a.setShrinkLevel=function(b){a._BinaryenSetShrinkLevel(b)}
;a.getDebugInfo=function(){return!!a._BinaryenGetDebugInfo()}
;a.setDebugInfo=function(b){a._BinaryenSetDebugInfo(b)}
;a.getLowMemoryUnused=function(){return!!a._BinaryenGetLowMemoryUnused()}
;
a.setLowMemoryUnused=function(b){a._BinaryenSetLowMemoryUnused(b)}
;a.getZeroFilledMemory=function(){return!!a._BinaryenGetZeroFilledMemory()}
;a.setZeroFilledMemory=function(b){a._BinaryenSetZeroFilledMemory(b)}
;a.getFastMath=function(){return!!a._BinaryenGetFastMath()}
;a.setFastMath=function(b){a._BinaryenSetFastMath(b)}
;a.getPassArgument=function(b){return T(()=>{const d=a._BinaryenGetPassArgument(U(b));return 0!==d?t(d):null}
                                        )}
;
a.setPassArgument=function(b,d){T(()=>{a._BinaryenSetPassArgument(U(b),U(d))}
                                  )}
;a.clearPassArguments=function(){a._BinaryenClearPassArguments()}
;a.getAlwaysInlineMaxSize=function(){return a._BinaryenGetAlwaysInlineMaxSize()}
;a.setAlwaysInlineMaxSize=function(b){a._BinaryenSetAlwaysInlineMaxSize(b)}
;a.getFlexibleInlineMaxSize=function(){return a._BinaryenGetFlexibleInlineMaxSize()}
;a.setFlexibleInlineMaxSize=function(b){a._BinaryenSetFlexibleInlineMaxSize(b)}
;a.getOneCallerInlineMaxSize=function(){return a._BinaryenGetOneCallerInlineMaxSize()}
;
a.setOneCallerInlineMaxSize=function(b){a._BinaryenSetOneCallerInlineMaxSize(b)}
;a.getAllowInliningFunctionsWithLoops=function(){return!!a._BinaryenGetAllowInliningFunctionsWithLoops()}
;a.setAllowInliningFunctionsWithLoops=function(b){a._BinaryenSetAllowInliningFunctionsWithLoops(b)}
;const he=Symbol();
function Y(b){function d(f){if(!(this instanceof d))return f?new d(f):null;Z.call(this,f)}
Object.assign(d,Z);Object.assign(d,b);(d.prototype=Object.create(Z.prototype)).constructor=d;ie(d.prototype,b);return d}

function ie(b,d){Object.keys(d).forEach(f=>{const c=d[f];if("function"===typeof c){b[f]=function(...g){return this.constructor[f](this[he],...g)}
                                        ;var e;if(1===c.length&&(e=f.match(/^(get|is)/))){e=e[1].length;const g=f.charAt(e).toLowerCase()+f.substring(e+1),h=d["set"+f.substring(e)];Object.defineProperty(b,g,{get(){return c(this[he])}
                                                                                                                                                                   ,set(l){if(h)h(this[he],l);else throw Error("property '"+g+"' has no setter");}
}
)}
}
}
)}

function Z(b){if(!b)throw Error("expression reference must not be null");this[he]=b}
Z.getId=function(b){return a._BinaryenExpressionGetId(b)}
;Z.getType=function(b){return a._BinaryenExpressionGetType(b)}
;Z.setType=function(b,d){a._BinaryenExpressionSetType(b,d)}
;Z.finalize=function(b){return a._BinaryenExpressionFinalize(b)}
;Z.toText=function(b){return a.emitText(b)}
;ie(Z.prototype,Z);Z.prototype.valueOf=function(){return this[he]}
;a.Expression=Z;
a.Block=Y({getName:function(b){return(b=a._BinaryenBlockGetName(b))?t(b):null}
          ,setName:function(b,d){T(()=>{a._BinaryenBlockSetName(b,U(d))}
                         )}
,getNumChildren:function(b){return a._BinaryenBlockGetNumChildren(b)}
,getChildren:function(b){return W(b,a._BinaryenBlockGetNumChildren,a._BinaryenBlockGetChildAt)}
,setChildren:function(b,d){X(b,d,a._BinaryenBlockGetNumChildren,a._BinaryenBlockSetChildAt,a._BinaryenBlockAppendChild,a._BinaryenBlockRemoveChildAt)}
,getChildAt:function(b,d){return a._BinaryenBlockGetChildAt(b,
d)}
,setChildAt:function(b,d,f){a._BinaryenBlockSetChildAt(b,d,f)}
,appendChild:function(b,d){return a._BinaryenBlockAppendChild(b,d)}
,insertChildAt:function(b,d,f){a._BinaryenBlockInsertChildAt(b,d,f)}
,removeChildAt:function(b,d){return a._BinaryenBlockRemoveChildAt(b,d)}
}
);
a.If=Y({getCondition:function(b){return a._BinaryenIfGetCondition(b)}
       ,setCondition:function(b,d){a._BinaryenIfSetCondition(b,d)}
,getIfTrue:function(b){return a._BinaryenIfGetIfTrue(b)}
,setIfTrue:function(b,d){a._BinaryenIfSetIfTrue(b,d)}
,getIfFalse:function(b){return a._BinaryenIfGetIfFalse(b)}
,setIfFalse:function(b,d){a._BinaryenIfSetIfFalse(b,d)}
}
);
a.Loop=Y({getName:function(b){return(b=a._BinaryenLoopGetName(b))?t(b):null}
         ,setName:function(b,d){T(()=>{a._BinaryenLoopSetName(b,U(d))}
                         )}
,getBody:function(b){return a._BinaryenLoopGetBody(b)}
,setBody:function(b,d){a._BinaryenLoopSetBody(b,d)}
}
);
a.Break=Y({getName:function(b){return(b=a._BinaryenBreakGetName(b))?t(b):null}
          ,setName:function(b,d){T(()=>{a._BinaryenBreakSetName(b,U(d))}
                         )}
,getCondition:function(b){return a._BinaryenBreakGetCondition(b)}
,setCondition:function(b,d){a._BinaryenBreakSetCondition(b,d)}
,getValue:function(b){return a._BinaryenBreakGetValue(b)}
,setValue:function(b,d){a._BinaryenBreakSetValue(b,d)}
}
);
a.Switch=Y({getNumNames:function(b){return a._BinaryenSwitchGetNumNames(b)}
           ,getNames:function(b){return W(b,a._BinaryenSwitchGetNumNames,a._BinaryenSwitchGetNameAt).map(d=>t(d))}
,setNames:function(b,d){T(()=>{X(b,d.map(U),a._BinaryenSwitchGetNumNames,a._BinaryenSwitchSetNameAt,a._BinaryenSwitchAppendName,a._BinaryenSwitchRemoveNameAt)}
                          )}
,getDefaultName:function(b){return(b=a._BinaryenSwitchGetDefaultName(b))?t(b):null}
,setDefaultName:function(b,d){T(()=>{a._BinaryenSwitchSetDefaultName(b,U(d))}
                                )}
,
getCondition:function(b){return a._BinaryenSwitchGetCondition(b)}
,setCondition:function(b,d){a._BinaryenSwitchSetCondition(b,d)}
,getValue:function(b){return a._BinaryenSwitchGetValue(b)}
,setValue:function(b,d){a._BinaryenSwitchSetValue(b,d)}
,getNameAt:function(b,d){return t(a._BinaryenSwitchGetNameAt(b,d))}
,setNameAt:function(b,d,f){T(()=>{a._BinaryenSwitchSetNameAt(b,d,U(f))}
                             )}
,appendName:function(b,d){T(()=>a._BinaryenSwitchAppendName(b,U(d)))}
,insertNameAt:function(b,d,f){T(()=>{a._BinaryenSwitchInsertNameAt(b,
d,U(f))}
)}
,removeNameAt:function(b,d){return t(a._BinaryenSwitchRemoveNameAt(b,d))}
}
);
a.Call=Y({getTarget:function(b){return t(a._BinaryenCallGetTarget(b))}
         ,setTarget:function(b,d){T(()=>{a._BinaryenCallSetTarget(b,U(d))}
                           )}
,getNumOperands:function(b){return a._BinaryenCallGetNumOperands(b)}
,getOperands:function(b){return W(b,a._BinaryenCallGetNumOperands,a._BinaryenCallGetOperandAt)}
,setOperands:function(b,d){X(b,d,a._BinaryenCallGetNumOperands,a._BinaryenCallSetOperandAt,a._BinaryenCallAppendOperand,a._BinaryenCallRemoveOperandAt)}
,getOperandAt:function(b,d){return a._BinaryenCallGetOperandAt(b,
d)}
,setOperandAt:function(b,d,f){a._BinaryenCallSetOperandAt(b,d,f)}
,appendOperand:function(b,d){return a._BinaryenCallAppendOperand(b,d)}
,insertOperandAt:function(b,d,f){a._BinaryenCallInsertOperandAt(b,d,f)}
,removeOperandAt:function(b,d){return a._BinaryenCallRemoveOperandAt(b,d)}
,isReturn:function(b){return!!a._BinaryenCallIsReturn(b)}
,setReturn:function(b,d){a._BinaryenCallSetReturn(b,d)}
}
);
a.CallIndirect=Y({getTarget:function(b){return a._BinaryenCallIndirectGetTarget(b)}
                 ,setTarget:function(b,d){a._BinaryenCallIndirectSetTarget(b,d)}
,getTable:function(b){return t(a._BinaryenCallIndirectGetTable(b))}
,setTable:function(b,d){T(()=>{a._BinaryenCallIndirectSetTable(b,U(d))}
                          )}
,getNumOperands:function(b){return a._BinaryenCallIndirectGetNumOperands(b)}
,getOperands:function(b){return W(b,a._BinaryenCallIndirectGetNumOperands,a._BinaryenCallIndirectGetOperandAt)}
,setOperands:function(b,d){X(b,
d,a._BinaryenCallIndirectGetNumOperands,a._BinaryenCallIndirectSetOperandAt,a._BinaryenCallIndirectAppendOperand,a._BinaryenCallIndirectRemoveOperandAt)}
,getOperandAt:function(b,d){return a._BinaryenCallIndirectGetOperandAt(b,d)}
,setOperandAt:function(b,d,f){a._BinaryenCallIndirectSetOperandAt(b,d,f)}
,appendOperand:function(b,d){return a._BinaryenCallIndirectAppendOperand(b,d)}
,insertOperandAt:function(b,d,f){a._BinaryenCallIndirectInsertOperandAt(b,d,f)}
,removeOperandAt:function(b,d){return a._BinaryenCallIndirectRemoveOperandAt(b,
d)}
,isReturn:function(b){return!!a._BinaryenCallIndirectIsReturn(b)}
,setReturn:function(b,d){a._BinaryenCallIndirectSetReturn(b,d)}
,getParams:function(b){return a._BinaryenCallIndirectGetParams(b)}
,setParams:function(b,d){a._BinaryenCallIndirectSetParams(b,d)}
,getResults:function(b){return a._BinaryenCallIndirectGetResults(b)}
,setResults:function(b,d){a._BinaryenCallIndirectSetResults(b,d)}
}
);
a.LocalGet=Y({getIndex:function(b){return a._BinaryenLocalGetGetIndex(b)}
             ,setIndex:function(b,d){a._BinaryenLocalGetSetIndex(b,d)}
}
);a.LocalSet=Y({getIndex:function(b){return a._BinaryenLocalSetGetIndex(b)}
,setIndex:function(b,d){a._BinaryenLocalSetSetIndex(b,d)}
,isTee:function(b){return!!a._BinaryenLocalSetIsTee(b)}
,getValue:function(b){return a._BinaryenLocalSetGetValue(b)}
,setValue:function(b,d){a._BinaryenLocalSetSetValue(b,d)}
}
);
a.GlobalGet=Y({getName:function(b){return t(a._BinaryenGlobalGetGetName(b))}
              ,setName:function(b,d){T(()=>{a._BinaryenGlobalGetSetName(b,U(d))}
                         )}
}
);a.GlobalSet=Y({getName:function(b){return t(a._BinaryenGlobalSetGetName(b))}
,setName:function(b,d){T(()=>{a._BinaryenGlobalSetSetName(b,U(d))}
                         )}
,getValue:function(b){return a._BinaryenGlobalSetGetValue(b)}
,setValue:function(b,d){a._BinaryenGlobalSetSetValue(b,d)}
}
);
a.TableGet=Y({getTable:function(b){return t(a._BinaryenTableGetGetTable(b))}
             ,setTable:function(b,d){T(()=>{a._BinaryenTableGetSetTable(b,U(d))}
                          )}
,getIndex:function(b){return a._BinaryenTableGetGetIndex(b)}
,setIndex:function(b,d){a._BinaryenTableGetSetIndex(b,d)}
}
);
a.TableSet=Y({getTable:function(b){return t(a._BinaryenTableSetGetTable(b))}
             ,setTable:function(b,d){T(()=>{a._BinaryenTableSetSetTable(b,U(d))}
                          )}
,getIndex:function(b){return a._BinaryenTableSetGetIndex(b)}
,setIndex:function(b,d){a._BinaryenTableSetSetIndex(b,d)}
,getValue:function(b){return a._BinaryenTableSetGetValue(b)}
,setValue:function(b,d){a._BinaryenTableSetSetValue(b,d)}
}
);
a.TableSize=Y({getTable:function(b){return t(a._BinaryenTableSizeGetTable(b))}
              ,setTable:function(b,d){T(()=>{a._BinaryenTableSizeSetTable(b,U(d))}
                          )}
}
);
a.TableGrow=Y({getTable:function(b){return t(a._BinaryenTableGrowGetTable(b))}
              ,setTable:function(b,d){T(()=>{a._BinaryenTableGrowSetTable(b,U(d))}
                          )}
,getValue:function(b){return a._BinaryenTableGrowGetValue(b)}
,setValue:function(b,d){a._BinaryenTableGrowSetValue(b,d)}
,getDelta:function(b){return a._BinaryenTableGrowGetDelta(b)}
,setDelta:function(b,d){a._BinaryenTableGrowSetDelta(b,d)}
}
);a.MemorySize=Y({}
);
a.MemoryGrow=Y({getDelta:function(b){return a._BinaryenMemoryGrowGetDelta(b)}
               ,setDelta:function(b,d){a._BinaryenMemoryGrowSetDelta(b,d)}
}
);
a.Load=Y({isAtomic:function(b){return!!a._BinaryenLoadIsAtomic(b)}
         ,setAtomic:function(b,d){a._BinaryenLoadSetAtomic(b,d)}
,isSigned:function(b){return!!a._BinaryenLoadIsSigned(b)}
,setSigned:function(b,d){a._BinaryenLoadSetSigned(b,d)}
,getOffset:function(b){return a._BinaryenLoadGetOffset(b)}
,setOffset:function(b,d){a._BinaryenLoadSetOffset(b,d)}
,getBytes:function(b){return a._BinaryenLoadGetBytes(b)}
,setBytes:function(b,d){a._BinaryenLoadSetBytes(b,d)}
,getAlign:function(b){return a._BinaryenLoadGetAlign(b)}
,
setAlign:function(b,d){a._BinaryenLoadSetAlign(b,d)}
,getPtr:function(b){return a._BinaryenLoadGetPtr(b)}
,setPtr:function(b,d){a._BinaryenLoadSetPtr(b,d)}
}
);
a.Store=Y({isAtomic:function(b){return!!a._BinaryenStoreIsAtomic(b)}
          ,setAtomic:function(b,d){a._BinaryenStoreSetAtomic(b,d)}
,getBytes:function(b){return a._BinaryenStoreGetBytes(b)}
,setBytes:function(b,d){a._BinaryenStoreSetBytes(b,d)}
,getOffset:function(b){return a._BinaryenStoreGetOffset(b)}
,setOffset:function(b,d){a._BinaryenStoreSetOffset(b,d)}
,getAlign:function(b){return a._BinaryenStoreGetAlign(b)}
,setAlign:function(b,d){a._BinaryenStoreSetAlign(b,d)}
,getPtr:function(b){return a._BinaryenStoreGetPtr(b)}
,
setPtr:function(b,d){a._BinaryenStoreSetPtr(b,d)}
,getValue:function(b){return a._BinaryenStoreGetValue(b)}
,setValue:function(b,d){a._BinaryenStoreSetValue(b,d)}
,getValueType:function(b){return a._BinaryenStoreGetValueType(b)}
,setValueType:function(b,d){a._BinaryenStoreSetValueType(b,d)}
}
);
a.Const=Y({getValueI32:function(b){return a._BinaryenConstGetValueI32(b)}
          ,setValueI32:function(b,d){a._BinaryenConstSetValueI32(b,d)}
,getValueI64Low:function(b){return a._BinaryenConstGetValueI64Low(b)}
,setValueI64Low:function(b,d){a._BinaryenConstSetValueI64Low(b,d)}
,getValueI64High:function(b){return a._BinaryenConstGetValueI64High(b)}
,setValueI64High:function(b,d){a._BinaryenConstSetValueI64High(b,d)}
,getValueF32:function(b){return a._BinaryenConstGetValueF32(b)}
,setValueF32:function(b,d){a._BinaryenConstSetValueF32(b,
d)}
,getValueF64:function(b){return a._BinaryenConstGetValueF64(b)}
,setValueF64:function(b,d){a._BinaryenConstSetValueF64(b,d)}
,getValueV128:function(b){let d;T(()=>{const f=O(16);a._BinaryenConstGetValueV128(b,f);d=Array(16);for(let c=0;16>c;++c)d[c]=u[f+c]}
                                  );return d}
,setValueV128:function(b,d){T(()=>{const f=O(16);for(let c=0;16>c;++c)u[f+c]=d[c];a._BinaryenConstSetValueV128(b,f)}
                              )}
}
);
a.Unary=Y({getOp:function(b){return a._BinaryenUnaryGetOp(b)}
          ,setOp:function(b,d){a._BinaryenUnarySetOp(b,d)}
,getValue:function(b){return a._BinaryenUnaryGetValue(b)}
,setValue:function(b,d){a._BinaryenUnarySetValue(b,d)}
}
);
a.Binary=Y({getOp:function(b){return a._BinaryenBinaryGetOp(b)}
           ,setOp:function(b,d){a._BinaryenBinarySetOp(b,d)}
,getLeft:function(b){return a._BinaryenBinaryGetLeft(b)}
,setLeft:function(b,d){a._BinaryenBinarySetLeft(b,d)}
,getRight:function(b){return a._BinaryenBinaryGetRight(b)}
,setRight:function(b,d){a._BinaryenBinarySetRight(b,d)}
}
);
a.Select=Y({getIfTrue:function(b){return a._BinaryenSelectGetIfTrue(b)}
           ,setIfTrue:function(b,d){a._BinaryenSelectSetIfTrue(b,d)}
,getIfFalse:function(b){return a._BinaryenSelectGetIfFalse(b)}
,setIfFalse:function(b,d){a._BinaryenSelectSetIfFalse(b,d)}
,getCondition:function(b){return a._BinaryenSelectGetCondition(b)}
,setCondition:function(b,d){a._BinaryenSelectSetCondition(b,d)}
}
);a.Drop=Y({getValue:function(b){return a._BinaryenDropGetValue(b)}
,setValue:function(b,d){a._BinaryenDropSetValue(b,d)}
}
);
a.Return=Y({getValue:function(b){return a._BinaryenReturnGetValue(b)}
           ,setValue:function(b,d){a._BinaryenReturnSetValue(b,d)}
}
);
a.AtomicRMW=Y({getOp:function(b){return a._BinaryenAtomicRMWGetOp(b)}
              ,setOp:function(b,d){a._BinaryenAtomicRMWSetOp(b,d)}
,getBytes:function(b){return a._BinaryenAtomicRMWGetBytes(b)}
,setBytes:function(b,d){a._BinaryenAtomicRMWSetBytes(b,d)}
,getOffset:function(b){return a._BinaryenAtomicRMWGetOffset(b)}
,setOffset:function(b,d){a._BinaryenAtomicRMWSetOffset(b,d)}
,getPtr:function(b){return a._BinaryenAtomicRMWGetPtr(b)}
,setPtr:function(b,d){a._BinaryenAtomicRMWSetPtr(b,d)}
,getValue:function(b){return a._BinaryenAtomicRMWGetValue(b)}
,
setValue:function(b,d){a._BinaryenAtomicRMWSetValue(b,d)}
}
);
a.AtomicCmpxchg=Y({getBytes:function(b){return a._BinaryenAtomicCmpxchgGetBytes(b)}
                  ,setBytes:function(b,d){a._BinaryenAtomicCmpxchgSetBytes(b,d)}
,getOffset:function(b){return a._BinaryenAtomicCmpxchgGetOffset(b)}
,setOffset:function(b,d){a._BinaryenAtomicCmpxchgSetOffset(b,d)}
,getPtr:function(b){return a._BinaryenAtomicCmpxchgGetPtr(b)}
,setPtr:function(b,d){a._BinaryenAtomicCmpxchgSetPtr(b,d)}
,getExpected:function(b){return a._BinaryenAtomicCmpxchgGetExpected(b)}
,setExpected:function(b,d){a._BinaryenAtomicCmpxchgSetExpected(b,
d)}
,getReplacement:function(b){return a._BinaryenAtomicCmpxchgGetReplacement(b)}
,setReplacement:function(b,d){a._BinaryenAtomicCmpxchgSetReplacement(b,d)}
}
);
a.AtomicWait=Y({getPtr:function(b){return a._BinaryenAtomicWaitGetPtr(b)}
               ,setPtr:function(b,d){a._BinaryenAtomicWaitSetPtr(b,d)}
,getExpected:function(b){return a._BinaryenAtomicWaitGetExpected(b)}
,setExpected:function(b,d){a._BinaryenAtomicWaitSetExpected(b,d)}
,getTimeout:function(b){return a._BinaryenAtomicWaitGetTimeout(b)}
,setTimeout:function(b,d){a._BinaryenAtomicWaitSetTimeout(b,d)}
,getExpectedType:function(b){return a._BinaryenAtomicWaitGetExpectedType(b)}
,setExpectedType:function(b,d){a._BinaryenAtomicWaitSetExpectedType(b,
d)}
}
);a.AtomicNotify=Y({getPtr:function(b){return a._BinaryenAtomicNotifyGetPtr(b)}
,setPtr:function(b,d){a._BinaryenAtomicNotifySetPtr(b,d)}
,getNotifyCount:function(b){return a._BinaryenAtomicNotifyGetNotifyCount(b)}
,setNotifyCount:function(b,d){a._BinaryenAtomicNotifySetNotifyCount(b,d)}
}
);a.AtomicFence=Y({getOrder:function(b){return a._BinaryenAtomicFenceGetOrder(b)}
,setOrder:function(b,d){a._BinaryenAtomicFenceSetOrder(b,d)}
}
);
a.SIMDExtract=Y({getOp:function(b){return a._BinaryenSIMDExtractGetOp(b)}
                ,setOp:function(b,d){a._BinaryenSIMDExtractSetOp(b,d)}
,getVec:function(b){return a._BinaryenSIMDExtractGetVec(b)}
,setVec:function(b,d){a._BinaryenSIMDExtractSetVec(b,d)}
,getIndex:function(b){return a._BinaryenSIMDExtractGetIndex(b)}
,setIndex:function(b,d){a._BinaryenSIMDExtractSetIndex(b,d)}
}
);
a.SIMDReplace=Y({getOp:function(b){return a._BinaryenSIMDReplaceGetOp(b)}
                ,setOp:function(b,d){a._BinaryenSIMDReplaceSetOp(b,d)}
,getVec:function(b){return a._BinaryenSIMDReplaceGetVec(b)}
,setVec:function(b,d){a._BinaryenSIMDReplaceSetVec(b,d)}
,getIndex:function(b){return a._BinaryenSIMDReplaceGetIndex(b)}
,setIndex:function(b,d){a._BinaryenSIMDReplaceSetIndex(b,d)}
,getValue:function(b){return a._BinaryenSIMDReplaceGetValue(b)}
,setValue:function(b,d){a._BinaryenSIMDReplaceSetValue(b,d)}
}
);
a.SIMDShuffle=Y({getLeft:function(b){return a._BinaryenSIMDShuffleGetLeft(b)}
                ,setLeft:function(b,d){a._BinaryenSIMDShuffleSetLeft(b,d)}
,getRight:function(b){return a._BinaryenSIMDShuffleGetRight(b)}
,setRight:function(b,d){a._BinaryenSIMDShuffleSetRight(b,d)}
,getMask:function(b){let d;T(()=>{const f=O(16);a._BinaryenSIMDShuffleGetMask(b,f);d=Array(16);for(let c=0;16>c;++c)d[c]=u[f+c]}
                             );return d}
,setMask:function(b,d){T(()=>{const f=O(16);for(let c=0;16>c;++c)u[f+c]=d[c];a._BinaryenSIMDShuffleSetMask(b,
f)}
)}
}
);a.SIMDTernary=Y({getOp:function(b){return a._BinaryenSIMDTernaryGetOp(b)}
,setOp:function(b,d){a._BinaryenSIMDTernarySetOp(b,d)}
,getA:function(b){return a._BinaryenSIMDTernaryGetA(b)}
,setA:function(b,d){a._BinaryenSIMDTernarySetA(b,d)}
,getB:function(b){return a._BinaryenSIMDTernaryGetB(b)}
,setB:function(b,d){a._BinaryenSIMDTernarySetB(b,d)}
,getC:function(b){return a._BinaryenSIMDTernaryGetC(b)}
,setC:function(b,d){a._BinaryenSIMDTernarySetC(b,d)}
}
);
a.SIMDShift=Y({getOp:function(b){return a._BinaryenSIMDShiftGetOp(b)}
              ,setOp:function(b,d){a._BinaryenSIMDShiftSetOp(b,d)}
,getVec:function(b){return a._BinaryenSIMDShiftGetVec(b)}
,setVec:function(b,d){a._BinaryenSIMDShiftSetVec(b,d)}
,getShift:function(b){return a._BinaryenSIMDShiftGetShift(b)}
,setShift:function(b,d){a._BinaryenSIMDShiftSetShift(b,d)}
}
);
a.SIMDLoad=Y({getOp:function(b){return a._BinaryenSIMDLoadGetOp(b)}
             ,setOp:function(b,d){a._BinaryenSIMDLoadSetOp(b,d)}
,getOffset:function(b){return a._BinaryenSIMDLoadGetOffset(b)}
,setOffset:function(b,d){a._BinaryenSIMDLoadSetOffset(b,d)}
,getAlign:function(b){return a._BinaryenSIMDLoadGetAlign(b)}
,setAlign:function(b,d){a._BinaryenSIMDLoadSetAlign(b,d)}
,getPtr:function(b){return a._BinaryenSIMDLoadGetPtr(b)}
,setPtr:function(b,d){a._BinaryenSIMDLoadSetPtr(b,d)}
}
);
a.SIMDLoadStoreLane=Y({getOp:function(b){return a._BinaryenSIMDLoadStoreLaneGetOp(b)}
                      ,setOp:function(b,d){a._BinaryenSIMDLoadStoreLaneSetOp(b,d)}
,getOffset:function(b){return a._BinaryenSIMDLoadStoreLaneGetOffset(b)}
,setOffset:function(b,d){a._BinaryenSIMDLoadStoreLaneSetOffset(b,d)}
,getAlign:function(b){return a._BinaryenSIMDLoadStoreLaneGetAlign(b)}
,setAlign:function(b,d){a._BinaryenSIMDLoadStoreLaneSetAlign(b,d)}
,getIndex:function(b){return a._BinaryenSIMDLoadStoreLaneGetIndex(b)}
,setIndex:function(b,
d){a._BinaryenSIMDLoadStoreLaneSetIndex(b,d)}
,getPtr:function(b){return a._BinaryenSIMDLoadStoreLaneGetPtr(b)}
,setPtr:function(b,d){a._BinaryenSIMDLoadStoreLaneSetPtr(b,d)}
,getVec:function(b){return a._BinaryenSIMDLoadStoreLaneGetVec(b)}
,setVec:function(b,d){a._BinaryenSIMDLoadStoreLaneSetVec(b,d)}
,isStore:function(b){return!!a._BinaryenSIMDLoadStoreLaneIsStore(b)}
}
);
a.MemoryInit=Y({getSegment:function(b){return a._BinaryenMemoryInitGetSegment(b)}
               ,setSegment:function(b,d){a._BinaryenMemoryInitSetSegment(b,d)}
,getDest:function(b){return a._BinaryenMemoryInitGetDest(b)}
,setDest:function(b,d){a._BinaryenMemoryInitSetDest(b,d)}
,getOffset:function(b){return a._BinaryenMemoryInitGetOffset(b)}
,setOffset:function(b,d){a._BinaryenMemoryInitSetOffset(b,d)}
,getSize:function(b){return a._BinaryenMemoryInitGetSize(b)}
,setSize:function(b,d){a._BinaryenMemoryInitSetSize(b,d)}
}
);
a.DataDrop=Y({getSegment:function(b){return a._BinaryenDataDropGetSegment(b)}
             ,setSegment:function(b,d){a._BinaryenDataDropSetSegment(b,d)}
}
);
a.MemoryCopy=Y({getDest:function(b){return a._BinaryenMemoryCopyGetDest(b)}
               ,setDest:function(b,d){a._BinaryenMemoryCopySetDest(b,d)}
,getSource:function(b){return a._BinaryenMemoryCopyGetSource(b)}
,setSource:function(b,d){a._BinaryenMemoryCopySetSource(b,d)}
,getSize:function(b){return a._BinaryenMemoryCopyGetSize(b)}
,setSize:function(b,d){a._BinaryenMemoryCopySetSize(b,d)}
}
);
a.MemoryFill=Y({getDest:function(b){return a._BinaryenMemoryFillGetDest(b)}
               ,setDest:function(b,d){a._BinaryenMemoryFillSetDest(b,d)}
,getValue:function(b){return a._BinaryenMemoryFillGetValue(b)}
,setValue:function(b,d){a._BinaryenMemoryFillSetValue(b,d)}
,getSize:function(b){return a._BinaryenMemoryFillGetSize(b)}
,setSize:function(b,d){a._BinaryenMemoryFillSetSize(b,d)}
}
);
a.RefIs=Y({getOp:function(b){return a._BinaryenRefIsGetOp(b)}
          ,setOp:function(b,d){a._BinaryenRefIsSetOp(b,d)}
,getValue:function(b){return a._BinaryenRefIsGetValue(b)}
,setValue:function(b,d){a._BinaryenRefIsSetValue(b,d)}
}
);a.RefAs=Y({getOp:function(b){return a._BinaryenRefAsGetOp(b)}
,setOp:function(b,d){a._BinaryenRefAsSetOp(b,d)}
,getValue:function(b){return a._BinaryenRefAsGetValue(b)}
,setValue:function(b,d){a._BinaryenRefAsSetValue(b,d)}
}
);
a.RefFunc=Y({getFunc:function(b){return t(a._BinaryenRefFuncGetFunc(b))}
            ,setFunc:function(b,d){T(()=>{a._BinaryenRefFuncSetFunc(b,U(d))}
                         )}
}
);a.RefEq=Y({getLeft:function(b){return a._BinaryenRefEqGetLeft(b)}
,setLeft:function(b,d){return a._BinaryenRefEqSetLeft(b,d)}
,getRight:function(b){return a._BinaryenRefEqGetRight(b)}
,setRight:function(b,d){return a._BinaryenRefEqSetRight(b,d)}
}
);
a.Try=Y({getName:function(b){return(b=a._BinaryenTryGetName(b))?t(b):null}
        ,setName:function(b,d){T(()=>{a._BinaryenTrySetName(b,U(d))}
                         )}
,getBody:function(b){return a._BinaryenTryGetBody(b)}
,setBody:function(b,d){a._BinaryenTrySetBody(b,d)}
,getNumCatchTags:function(b){return a._BinaryenTryGetNumCatchTags(b)}
,getCatchTags:function(b){return W(b,a._BinaryenTryGetNumCatchTags,a._BinaryenTryGetCatchTagAt).map(d=>t(d))}
,setCatchTags:function(b,d){T(()=>{X(b,d.map(U),a._BinaryenTryGetNumCatchTags,a._BinaryenTrySetCatchTagAt,
a._BinaryenTryAppendCatchTag,a._BinaryenTryRemoveCatchTagAt)}
)}
,getCatchTagAt:function(b,d){return t(a._BinaryenTryGetCatchTagAt(b,d))}
,setCatchTagAt:function(b,d,f){T(()=>{a._BinaryenTrySetCatchTagAt(b,d,U(f))}
                                 )}
,appendCatchTag:function(b,d){T(()=>a._BinaryenTryAppendCatchTag(b,U(d)))}
,insertCatchTagAt:function(b,d,f){T(()=>{a._BinaryenTryInsertCatchTagAt(b,d,U(f))}
                                    )}
,removeCatchTagAt:function(b,d){return t(a._BinaryenTryRemoveCatchTagAt(b,d))}
,getNumCatchBodies:function(b){return a._BinaryenTryGetNumCatchBodies(b)}
,
getCatchBodies:function(b){return W(b,a._BinaryenTryGetNumCatchBodies,a._BinaryenTryGetCatchBodyAt)}
,setCatchBodies:function(b,d){X(b,d,a._BinaryenTryGetNumCatchBodies,a._BinaryenTrySetCatchBodyAt,a._BinaryenTryAppendCatchBody,a._BinaryenTryRemoveCatchBodyAt)}
,getCatchBodyAt:function(b,d){return a._BinaryenTryGetCatchBodyAt(b,d)}
,setCatchBodyAt:function(b,d,f){a._BinaryenTrySetCatchBodyAt(b,d,f)}
,appendCatchBody:function(b,d){return a._BinaryenTryAppendCatchBody(b,d)}
,insertCatchBodyAt:function(b,
d,f){a._BinaryenTryInsertCatchBodyAt(b,d,f)}
,removeCatchBodyAt:function(b,d){return a._BinaryenTryRemoveCatchBodyAt(b,d)}
,hasCatchAll:function(b){return!!a._BinaryenTryHasCatchAll(b)}
,getDelegateTarget:function(b){return(b=a._BinaryenTryGetDelegateTarget(b))?t(b):null}
,setDelegateTarget:function(b,d){T(()=>{a._BinaryenTrySetDelegateTarget(b,U(d))}
                                   )}
,isDelegate:function(b){return!!a._BinaryenTryIsDelegate(b)}
}
);
a.Throw=Y({getTag:function(b){return t(a._BinaryenThrowGetTag(b))}
          ,setTag:function(b,d){T(()=>{a._BinaryenThrowSetTag(b,U(d))}
                        )}
,getNumOperands:function(b){return a._BinaryenThrowGetNumOperands(b)}
,getOperands:function(b){return W(b,a._BinaryenThrowGetNumOperands,a._BinaryenThrowGetOperandAt)}
,setOperands:function(b,d){X(b,d,a._BinaryenThrowGetNumOperands,a._BinaryenThrowSetOperandAt,a._BinaryenThrowAppendOperand,a._BinaryenThrowRemoveOperandAt)}
,getOperandAt:function(b,d){return a._BinaryenThrowGetOperandAt(b,
d)}
,setOperandAt:function(b,d,f){a._BinaryenThrowSetOperandAt(b,d,f)}
,appendOperand:function(b,d){return a._BinaryenThrowAppendOperand(b,d)}
,insertOperandAt:function(b,d,f){a._BinaryenThrowInsertOperandAt(b,d,f)}
,removeOperandAt:function(b,d){return a._BinaryenThrowRemoveOperandAt(b,d)}
}
);a.Rethrow=Y({getTarget:function(b){return(b=a._BinaryenRethrowGetTarget(b))?t(b):null}
,setTarget:function(b,d){T(()=>{a._BinaryenRethrowSetTarget(b,U(d))}
                           )}
}
);
a.TupleMake=Y({getNumOperands:function(b){return a._BinaryenTupleMakeGetNumOperands(b)}
              ,getOperands:function(b){return W(b,a._BinaryenTupleMakeGetNumOperands,a._BinaryenTupleMakeGetOperandAt)}
,setOperands:function(b,d){X(b,d,a._BinaryenTupleMakeGetNumOperands,a._BinaryenTupleMakeSetOperandAt,a._BinaryenTupleMakeAppendOperand,a._BinaryenTupleMakeRemoveOperandAt)}
,getOperandAt:function(b,d){return a._BinaryenTupleMakeGetOperandAt(b,d)}
,setOperandAt:function(b,d,f){a._BinaryenTupleMakeSetOperandAt(b,
d,f)}
,appendOperand:function(b,d){return a._BinaryenTupleMakeAppendOperand(b,d)}
,insertOperandAt:function(b,d,f){a._BinaryenTupleMakeInsertOperandAt(b,d,f)}
,removeOperandAt:function(b,d){return a._BinaryenTupleMakeRemoveOperandAt(b,d)}
}
);
a.TupleExtract=Y({getTuple:function(b){return a._BinaryenTupleExtractGetTuple(b)}
                 ,setTuple:function(b,d){a._BinaryenTupleExtractSetTuple(b,d)}
,getIndex:function(b){return a._BinaryenTupleExtractGetIndex(b)}
,setIndex:function(b,d){a._BinaryenTupleExtractSetIndex(b,d)}
}
);a.I31New=Y({getValue:function(b){return a._BinaryenI31NewGetValue(b)}
,setValue:function(b,d){a._BinaryenI31NewSetValue(b,d)}
}
);
a.I31Get=Y({getI31:function(b){return a._BinaryenI31GetGetI31(b)}
           ,setI31:function(b,d){a._BinaryenI31GetSetI31(b,d)}
,isSigned:function(b){return!!a._BinaryenI31GetIsSigned(b)}
,setSigned:function(b,d){a._BinaryenI31GetSetSigned(b,d)}
}
);
a.Function=(()=>{function b(d){if(!(this instanceof b))return d?new b(d):null;if(!d)throw Error("function reference must not be null");this[he]=d}
            b.getName=function(d){return t(a._BinaryenFunctionGetName(d))}
;b.getParams=function(d){return a._BinaryenFunctionGetParams(d)}
;b.getResults=function(d){return a._BinaryenFunctionGetResults(d)}
;b.getNumVars=function(d){return a._BinaryenFunctionGetNumVars(d)}
;b.getVar=function(d,f){return a._BinaryenFunctionGetVar(d,f)}
;b.getNumLocals=function(d){return a._BinaryenFunctionGetNumLocals(d)}
;
b.hasLocalName=function(d,f){return!!a._BinaryenFunctionHasLocalName(d,f)}
;b.getLocalName=function(d,f){return t(a._BinaryenFunctionGetLocalName(d,f))}
;b.setLocalName=function(d,f,c){T(()=>{a._BinaryenFunctionSetLocalName(d,f,U(c))}
                                  )}
;b.getBody=function(d){return a._BinaryenFunctionGetBody(d)}
;b.setBody=function(d,f){a._BinaryenFunctionSetBody(d,f)}
;ie(b.prototype,b);b.prototype.valueOf=function(){return this[he]}
;return b}
)();a.exit=function(b){if(0!=b)throw Error("exiting due to error: "+b);}
;
Ga?fe():a.onRuntimeInitialized=(b=>()=>{fe();b&&b()}
                                )(a.onRuntimeInitialized);


  return Binaryen.ready
}

);
}
)();
export default Binaryen;