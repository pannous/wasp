(module 
  (func $sin (param f64) (result f64)
    (local f64)
    local.get 0
    local.get 0
    f64.const 0x1.921fb54442d18p+2 (;=6.28319;)
    f64.div
    f64.trunc
    f64.const 0x1.921fb54442d18p+2 (;=6.28319;)
    f64.mul
    f64.sub
    local.tee 0
    f64.const 3.141592653589793
    local.tee 1
    f64.ge
    i32.const 1
    i32.xor
    i32.eqz
    if  ;; label = @1
      local.get 0
      local.get 0
      local.get 1
      f64.div
      f64.trunc
      local.get 1
      f64.mul
      f64.sub
      call $sin
      f64.neg
      return
    end
    local.get 0
    local.get 0
    local.get 0
    local.get 0
    f64.mul
    local.tee 0
    f64.mul
    local.get 0
    local.get 0
    local.get 0
    local.get 0
    f64.mul
    f64.mul
    local.get 0
    f64.const 0x1.5d93a5acfd57cp-33 (;=1.58969e-10;)
    f64.mul
    f64.const -0x1.ae5e68a2b9cebp-26 (;=-2.50508e-08;)
    f64.add
    f64.mul
    local.get 0
    local.get 0
    f64.const 0x1.71de357b1fe7dp-19 (;=2.75573e-06;)
    f64.mul
    f64.const -0x1.a01a019c161d5p-13 (;=-0.000198413;)
    f64.add
    f64.mul
    f64.const 0x1.111111110f8a6p-7 (;=0.00833333;)
    f64.add
    f64.add
    f64.mul
    f64.const -0x1.5555555555549p-3 (;=-0.166667;)
    f64.add
    f64.mul
    f64.add)
  (func $cos (param f64) (result f64)
    ;; pi/2
    local.get 0
    f64.const  1.5707963267948966 
    f64.add
    call $sin)
;;  (func $main (export "main") (param i32) (result i32)
;;   f64.const 1.5707963267948966 
;;(call $sin)
;;	i32.trunc_f64_s
;;   )
)