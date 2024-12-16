let exampleCode = {
  // getElementById: '$canvas.style',
  forLoop: `for i in 1 to 3 {print i}`,
  getElementById: '$canvas.tagName', // OK via chars
  invokeRef: `$canvas.getContext('2d')`,
  download: `download http://pannous.com/files/test`,
  math: `1+2*√3^2`,
  hello: `"Hello, World!" // last item in root block is its return value, which is printed"`,
  math2: `√π²`,
  function: `square := it*it; square 3`,
  lists: `a = [1, 2, 3]; a[1] == a#2`,
  lists2: `a = [1, 2] + [3, 4]; a[2] == a#3 == 3`,
  strings: `a = "Hello " +"🌎"; a#7 == '🌎'`,
  fibonacci: `fib := it < 2 ? it : fib(it - 1) + fib(it - 2)\n\nfib(10)`,
  // fibonacci2: `fib(n) = n < 2 ? n : fib(n - 1) + fib(n - 2)\n\nfib(10)`,
  // fibonacci_juliaesque: `fib 0 :=0; fib 1 := 1; fib n := fib(n - 1) + fib(n - 2)\n\nfib(10)`,
  // fibonacci2: `fib n := n < 3 ? 1 : fib(n - 1) + fib(n - 2)\n\n`,
  // paint2d: `ctx = $canvas.getContext('2d');ctx.fillStyle = 'red'; ctx.fillRect(10, 10, 150, 100)`,
  // paint2d: `$ctx.fillStyle = 'red'; $ctx.fillRect(10, 10, 150, 100)`,
  paint2d: `$ctx.fillStyle = 'red'`,
  html: `html{bold{"Hello"}}`,
  // html: `html: h1: "Hello, World!"`,
  // html: `html: <h1>Hello, World!</h1>`, // operator missing: <
  // html2: `html: div id="app"; app.innerHTML = "Hello, World!"`,
  alert: 'script: alert("Hello, World!")',
  // draw_sine: `// sine wave\nfor i in 0..1000\n  ctx.lineTo(i, 100 + Math.sin(i / 10) * 50)\nctx.stroke()`,
  sine: `double sin(double x){
    double
    S1  = -1.66666666666666324348e-01, 
    S2  =  8.33333333332248946124e-03, 
    S3  = -1.98412698298579493134e-04, 
    S4  =  2.75573137070700676789e-06, 
    S5  = -2.50507602534068634195e-08, 
    S6  =  1.58969099521155010221e-10; 
    ;;
    double z,r,w;
    double tau=6.283185307179586;// 2*pi

    x = modulo_double(x,tau);
    // if(x<0) return -sin(-x);
    if(x >= pi) return -sin(modulo_double(x,pi));

    // if(x%tau > pi) return -sin(x%tau);
    z = x*x;
    w = z*z;
    r = S2 + z*(S3 + z*S4) + z*w*(S5 + z*S6);
    return x + z*x*(S1 + z*r);
}
sin(3.1415926)
    `,

  // scene3d: `// draw a red rectangle\nctx.fillStyle = 'red';\nctx.fillRect(10, 10, 150, 100);`
}