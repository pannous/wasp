(module ;; @name "math.log"

 ;; test
 ;;(import "env" "put" (func $print (param f64)))
 ;;(func $main (export "main") (param i32) 
 ;;   f64.const 256
 ;;   call $log2
 ;;   call $print
 ;; assert 2⌟256 == 8 because 256 == 2^8

 ;;   f64.const 10000
 ;;   call $log10
 ;;   call $print

 ;;   (f64.const 2.7182818284590)
 ;;   (f64.const 2.7182818284590)
 ;;   f64.mul
 ;;   call $ln
 ;;   call $print
 ;; )


  (func $log (param f64) (result f64)
              local.get 0
              call $ln
              )

  (func $log2 (param f64) (result f64)
              local.get 0
              call $ln
              (f64.const 0.6931471805599453)
              f64.div
   )

  (func $log10 (param f64) (result f64)
              local.get 0
              call $ln
              (f64.const 2.302585092994046)
              f64.div
   )

  (func $ln (param f64) (result f64)
    (local i32 i32 i32 i64 f64 f64)
    block  ;; label = @1
      block  ;; label = @2
        block  ;; label = @3
          block  ;; label = @4
            block  ;; label = @5
              local.get 0
              i64.reinterpret_f64
              local.tee 4
              i64.const 32
              i64.shr_u
              i32.wrap_i64
              local.tee 1
              i32.const 1048576
              i32.lt_u
              br_if 0 (;@5;)
              local.get 4
              i64.const -1
              i64.gt_s
              br_if 1 (;@4;)
            end
            block  ;; label = @5
              local.get 4
              i64.const 9223372036854775807
              i64.and
              i64.const 0
              i64.ne
              br_if 0 (;@5;)
              f64.const -0x1p+0 (;=-1;)
              local.get 0
              local.get 0
              f64.mul
              f64.div
              return
            end
            local.get 4
            i64.const -1
            i64.gt_s
            br_if 1 (;@3;)
            local.get 0
            local.get 0
            f64.sub
            f64.const 0x0p+0 (;=0;)
            f64.div
            return
          end
          local.get 1
          i32.const 2146435071
          i32.gt_u
          br_if 2 (;@1;)
          i32.const 1072693248
          local.set 2
          i32.const -1023
          local.set 3
          block  ;; label = @4
            local.get 1
            i32.const 1072693248
            i32.eq
            br_if 0 (;@4;)
            local.get 1
            local.set 2
            br 2 (;@2;)
          end
          local.get 4
          i32.wrap_i64
          br_if 1 (;@2;)
          f64.const 0x0p+0 (;=0;)
          return
        end
        local.get 0
        f64.const 0x1p+54 (;=1.80144e+16;)
        f64.mul
        i64.reinterpret_f64
        local.tee 4
        i64.const 32
        i64.shr_u
        i32.wrap_i64
        local.set 2
        i32.const -1077
        local.set 3
      end
      local.get 3
      local.get 2
      i32.const 614242
      i32.add
      local.tee 1
      i32.const 20
      i32.shr_u
      i32.add
      f64.convert_i32_s
      local.tee 5
      f64.const 0x1.62e42feep-1 (;=0.693147;)
      f64.mul
      local.get 1
      i32.const 1048575
      i32.and
      i32.const 1072079006
      i32.add
      i64.extend_i32_u
      i64.const 32
      i64.shl
      local.get 4
      i64.const 4294967295
      i64.and
      i64.or
      f64.reinterpret_i64
      f64.const -0x1p+0 (;=-1;)
      f64.add
      local.tee 0
      local.get 5
      f64.const 0x1.a39ef35793c76p-33 (;=1.90821e-10;)
      f64.mul
      local.get 0
      local.get 0
      f64.const 0x1p+1 (;=2;)
      f64.add
      f64.div
      local.tee 5
      local.get 0
      local.get 0
      f64.const 0x1p-1 (;=0.5;)
      f64.mul
      f64.mul
      local.tee 6
      local.get 5
      local.get 5
      f64.mul
      local.tee 5
      local.get 5
      f64.mul
      local.tee 0
      local.get 0
      local.get 0
      f64.const 0x1.39a09d078c69fp-3 (;=0.153138;)
      f64.mul
      f64.const 0x1.c71c51d8e78afp-3 (;=0.222222;)
      f64.add
      f64.mul
      f64.const 0x1.999999997fa04p-2 (;=0.4;)
      f64.add
      f64.mul
      local.get 5
      local.get 0
      local.get 0
      local.get 0
      f64.const 0x1.2f112df3e5244p-3 (;=0.147982;)
      f64.mul
      f64.const 0x1.7466496cb03dep-3 (;=0.181836;)
      f64.add
      f64.mul
      f64.const 0x1.2492494229359p-2 (;=0.285714;)
      f64.add
      f64.mul
      f64.const 0x1.5555555555593p-1 (;=0.666667;)
      f64.add
      f64.mul
      f64.add
      f64.add
      f64.mul
      f64.add
      local.get 6
      f64.sub
      f64.add
      f64.add
      local.set 0
    end
    local.get 0
    )

;; ₂⌞  := log₂  who ever uses this?
;; ₁₀⌞ := log₁₀ 10ʳ Logarithm
;; 10⌞ := log₁₀ prefix operator!  10⌞100 == 2
;; ⌟10 := log₁₀ suffix operator!  100⌟10 == 2
;; ᵦ⌞r := logᵦr the arrow always points to the base
;; r⌟ᵦ := logᵦr
;; ⌞r  := ln r    ⌞ reminiscent of L in log and of "lower" because the value is lowered 

;; NOTES
;; ⌞ℯ = 1
;; ⌞1 = 0
;; ⌞0 = -∞
;; ⌞2 = ln(2) per definition ( ə := ⌞2 as complement to ℯ )
;; 1⌞ = ∞   because log(1)==0 for any base
;; 0⌞ = 0   because 0⌞ = log₀(x) = log(x)/log(0)=log(x)/-∞=0
;; ℯ⌞ = logₑ = log = ln   REDUNDANT by definition ᵦ⌞r := logᵦr ( because log(ℯ)=1 )

;; ⌟ᵦ𝑟 " is reminiscent of "𝑟√ᵦ" ⚠️ lowercase badly visible =>

;; ⌜ reserved  ⌜b == e^b ?
;; ⌝ reserved  ⌝b == e^b ?
;; ⌜ reserved  b⌜a == a^b ?
;; ⌝ reserved  a⌝b == a^b ?
;; ⌝ reserved  ⌝ := ℯ if used without arguments!

;; ⌜x := ℯˣ BAD because useless and unfamiliar, only good for portaking the √ ^ ⌟ log series
;; ⌝x := ℯˣ BAD because confusable with ¬ / ⁻¹
;; ⌝(x=1) := ℯˣ == pow(e,x)  BAD because confusable with ¬ / ⁻¹
;; ⌜(x=1) := ℯˣ == pow(e,x)  OK?? vs  ¬ / ⁻¹
;; ⌜ ≈ ℯ as operator
;; ⌜ ≈ ℯ as number
;; ⌟ ≈ ə as number ə := ln(2) 0.6931471805599453  
;; ⌟ ≈ ə as operator ₂⌞  
;; 1 = I as operator (identity)
;; əₓ := ln(x) ₔx əx Əx Əₓ too similar to derivative ∂ !
;; logᵦy = logᵦ(y) = log(x,y) := ln(x)/ln(y) == ln(x) for y==ℯ since ln(ℯ) = 1
;; ℯ^ə = 2 as number
;; əℯ = 1 as operator
;; ⌜⌟ == ⌞ℯ = 1  
;; ⌜⌟ == 1 == pow(e,log(1,ℯ))  beautyful with default arguments ℯ
;; √^ == 1  with default arguments  x^ := x^2   √x == x^½
;; x√y == x * √y !!
;; x˅y := y^(⅟x)
;; ˄˅ == 1
;; ⚠️ ˄ ˅ has default base 2 while 
;; ⚠️ ⌜ ⌟ has default base ℯ 
;; alias heuler ⌜ 
;; alias lowea  ⌞ lowae  lowaℯ
;; canonical monotonous mapping log vs tanh, sigmoid
;; todo : restrict to signed numbers!

;; does e^e e^π or π^e π^π or log(π) have any significance in this universe??
;; e^π - π^e < 1
;; e^π - π^e - ln(2) < 0.012
;; π^π ≈ 6^2 = (3*2)^(2*1) cheap trick


;; Todo: better alias elsewhere? conflict with Console.log etc?
  (export "log10" (func $log10))
  (export "_alias_log₁₀" (func $log10))
  (export "_alias_₁₀⌞"   (func $log10))
  (export "_alias_10⌞"   (func $log10))

  (export "log2" (func $log2))
  (export "_alias_log₂" (func $log2))
  (export "_alias_₂⌞"   (func $log2))
  (export "_alias_2⌞"   (func $log2))

  (export "ln" (func $ln))
  (export "_alias_log" (func $ln)) ;; careful with Console.log … !
  (export "_alias_ln" (func $ln))
  (export "_alias_⌞"   (func $ln))
  (export "_alias_ₑ⌞"   (func $ln))
  (export "_alias_e⌞"   (func $ln))


)