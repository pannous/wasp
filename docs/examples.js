let exampleCode = {
  forLoop: `for i in 1 to 3 {print i}`,
  getElementById: '$canvas.tagName', // OK via chars
  // getElementById: '$canvas.style',
  invokeRef: `$canvas.getContext('2d')`,
  download: `download https://pannous.com/files/test`,
  math: `1+2*√3^2`,
  hello: `"Hello, World!" // last item in root block is its return value, which is printed"`,
  math2: `√π²`,
  function: `squared := it*it; squared 3`, // error square already defined
  // lists: `a = [1, 2, 3]; a[1] == a#2`, // TODO!!!
  // lists2: `a = [1, 2] + [3, 4]; a[2] == a#3 == 3`,
  strings: `a = "Hello " +"🌎"; a#7`, // == '🌎'`,
  fibonacci: `fib := it < 2 ? it : fib(it - 1) + fib(it - 2)\n\nfib(10)`,
  lambda: `def addi(x,y){x+y}\n# click to deploy https://lambda.pannous.com/math/addi/1,2 👇`,
  // fibonacci2: `fib(n) = n < 2 ? n : fib(n - 1) + fib(n - 2)\n\nfib(10)`,
  // fibonacci_juliaesque: `fib 0 :=0; fib 1 := 1; fib n := fib(n - 1) + fib(n - 2)\n\nfib(10)`,
  // fibonacci2: `fib n := n < 3 ? 1 : fib(n - 1) + fib(n - 2)\n\n`,
  // paint2d: `$ctx.fillStyle = 'red'; $ctx.fillRect(10, 10, 150, 100)`,
  // paint2d: `$ctx.fillStyle = 'red'`,
  html: `html{bold{"Hello"}}`,
  // html: `html: h1: "Hello, World!"`,
  // html: `html: <h1>Hello, World!</h1>`, // operator missing: <
  // html2: `html: div id="app"; app.innerHTML = "Hello, World!"`,
  alert: 'script: alert("Hello, World!")',
  // draw_sine: `// sine wave\nfor i in 0..1000\n  ctx.lineTo(i, 100 + Math.sin(i / 10) * 50)\nctx.stroke()`,
  sine: `real sin(real x){
  	if(x<0) return -sin(-x)
  		
    # magical taylor constants
    S1 = -1/6. 
    S2 =  1/120.
    S3 = -1/5040. 
    S4 =  1/362880. 
    S5 = -1/39916800.
    S6 =  1/6227020800.
    
    x = x%τ # modulo 2π
    z = x*x
    w = z*z
    r = S2 + z*(S3 + z*S4) + z*w*(S5 + z*S6)
    return x + z*x*(S1 + z*r)
}
sin(π/2)
    `,
  paint2d: `// TODO!\nctx = $canvas.getContext('2d')\nctx.fillStyle = 'red'\nctx.fillRect(10, 10, 150, 100)`,
  scene3d: `// TODO!\nload tree.3ds\nscene.render();`
}
