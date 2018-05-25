;; wasmx foo.wast
;; Wasm Lisp
;; See schism https://github.com/google/schism for a complete lisp on WebAssembly
;; https://github.com/sunfishcode/wasm-reference-manual/blob/master/WebAssembly.md
;; http://webassembly.org/docs/js/
;; https://github.com/WebAssembly/host-bindings
(module

 (table 1 1 anyfunc)
 (import "env" "memory" (memory $0 1)) ;; use 
 (export "memory" (memory $0))
 (start $test) ;; start must have 0 params

	(export "main" (func $main))

 ;; If you try to call a exported wasm function that takes or returns an i64 type value, it currently throws
 ;; This may change — a new int64 type is being considered for future standards, which could then be used by wasm.
 (import "console" "log" (func $logp (param i32 i32))) ;;  pointer of any type to anything / string!
 (import "console" "logs" (func $logs (param i32))) ;;  pointer to string!
 (import "console" "logc" (func $logc (param i32))) ;; char
 (import "console" "logi" (func $logi (param i32))) ;; int
 (import "console" "logx" (func $logx (param i32))) ;; hex
 (import "console" "logi" (func $logf (param f64))) 
 (import "console" "logi" (func $logn (param f64))) ;;
 (import "console" "raise" (func $imports.raise)) ;; (param i64)
 (import "global" "NaN" (global $NaN f64))

 ;;(global $a-mutable-global (mut f32) (f32.const 7.5))
 ;;(elem (i32.const 0) "$kitchen()sinker")

 ;;(global $data i32 (i32.const 1))
	;;(export "data" (global $data))

 ;;(global $MEMORY_BASE i32 (i32.const 100))
  ;;(global $HEAP_BASE i32 (i32.const 100))
 (global $offset (mut i32) (i32.const 100));; current heap pointer (after last object)


(func $get_offset (param i32) (result i32)
 (set_global $offset (i32.add (get_local $0) (get_global $offset) ))
 ;;(if (get_local $0) (set_global (i32.add (get_local $0) (get_global $offset) )) )
 (get_global $offset)
)
(export "offset" (func $get_offset (param i32) (result i32))) ;; cannot export a mutable global

 (global $OK i32 (i32.const 1))
 (data (i32.const 1) "OK\00")

 (global $Hello i32 (i32.const 70))
 (data (i32.const 70) "hello wasm!\00") ;; no identifier here :(
 ;;(data (get_global $Hello) "Hello global!\00")
 ;;(data (global $Hello) "Hello global!\00")


 (data (i32.const 20) "Hello!\00")


  (global $type.error i64 (i64.const -1))
  (global $type.null 	i64 (i64.const 0))
  (global $type.bool 	i64 (i64.const 0))
  (global $type.int 	i64 (i64.const 0))
  (global $type.byte 	i64 (i64.const 1))
  (global $type.short i64 (i64.const 2))
  (global $type.type 	i64 (i64.const 3))
  (global $type.word 	i64 (i64.const 4)) ;; vs int32 float32 !
  (global $type.json5 i64 (i64.const 5))
  (global $type.json 	i64 (i64.const 6))
  (global $type.object 	i64 (i64.const 7)) ;; serialized
  (global $type.pointer i64 (i64.const 8));; vs int64, float64
  (global $type.string i64 (i64.const 9))
  (global $type.array  i64 (i64.const 0xA))
  (global $type.binary i64 (i64.const 0xB)) ;; ??
  (global $type.code   i64 (i64.const 0xC)) ;; ??
  (global $type.data	 i64 (i64.const 0xD)) ;; ??
  ;;(global $type.objecte i64 (i64.const 0xE)) ;; entity
  (global $type.char i64 (i64.const 20))

	(global $null  i64 (i64.const 0))
	(global $false i32 (i32.const 0))
	(global $true  i32 (i32.const 1))
  (global $error   i64 (i64.const 0xFFFFFFFFFFFFFFFF))
  (global $mask.value    i64 (i64.const 0xFFFFFFFF00000000)) ;; shift right 32
  (global $mask.type     i64 (i64.const 0x00000000FFFFFFFF))


(func $ok 
	(call $print (get_global $OK) ) 
) 

(func $assert_eq32 (param i32) (param i32) 
 (if (i32.eq (get_local $0) (get_local $1)) (return))
 (call $logi (get_local $0))
 (call $logi (get_local $1))
 (call $raise)
) 

(func $log (param i64) 
 (local $type i32) 
 (local $pointer i32) 
 (set_local $type (i32.wrap/i64 (call $type.of (get_local $0) ) ))
 (set_local $pointer (call $type.raw (get_local $0) )) 
 (call $logx (i32.const 0xFFFFFFFF)) ;; -1 why?
 (call $logx (get_local $type) ) 
 (call $logx (get_local $pointer) ) 
 (call $logp (get_local $type) (get_local $pointer)  ) 
) 

(func $val (param i64) (result i32)
 (i32.wrap/i64 (i64.shr_u (get_local $0) (i64.const 32) )) ;; shift
) ;; ==
(func $type.raw (param i64)  (result i32) 
 (i32.wrap/i64 (i64.shr_u (get_local $0) (i64.const 32) )) ;; shift
) 

(func $type.of (param i64)  (result i64)
 (i64.and (get_local $0) (i64.const 0x00000000FFFFFFFF))
) 
(func $type.of32 (param i64)  (result i32)
 (i32.wrap/i64 (get_local $0))
) 

(func $type.to (param i32) (param i64) (result i64)
 (i64.or 
    (i64.shl_u (i64.extend_u (get_local $0)) (i64.const 32))
    (get_local $1)
  )
)

(func $type.to32 (param i32) (param i32) (result i64)
 (i64.or 
    (i64.extend_u (get_local $0)) 
    (i64.extend_u (get_local $1)) 
  )
)
;;(func $type.to (param i32) (param i32) (result i64)
;; (i64.or (i64.extend_u (get_local $0)) (get_global $type.string))
;;)

(func $string.is (param i64) (result i32)
 (i32.wrap/i64 (i64.and (get_local $0) (get_global $type.string)))
)

(func $raise
 (call $imports.raise)
 (call $logf (f64.div (f64.const 0) (f64.const 0)))
)

(func $assert (param i32) ;;(result i32)
	(if (get_local $0) (nop) (call $raise))
	;;(i32.const 1)
)

(func $string.pointer (param i32) (result i64) ;; stringify
 (return (call $type.to (get_local $0) (get_global $type.string)))
)
;;(func $string.const (param i32) (result i64)
;;)

;;(func $type.of (param i64) (result i32)
;; (return )
;;)

(func $print (param i32)
   (local $char i32)
  (loop $while
  	 (set_local $char (i32.load8 (get_local $0)))
     (set_local $0 (i32.add (get_local $0) (i32.const 1) ) )     
   	 (call $logc (get_local $char))
     (if (get_local $char) (br $while))
  )
)

(func $is (param i64) (param i64) (result i32)
 (i32.wrap/i64 (i64.and (get_local $0) (get_local $1)))
)
(func $is.string (param i64) (result i32)
 (i32.wrap/i64 (i64.and (get_local $0) (get_global $type.string)))
)
(func $todo
 (call $raise)
)

(func $atoi (param i32) (result i32)
 (local $i i32)
 (local $t i32)
 (local $char i32)
 (loop $while
	 (set_local $0 (i32.add (get_local $0) (i32.const 1)))
	 (set_local $char (i32.load8 offset=0 align=1 (get_local $0)))
	 (if (get_local $char) (br $while))
	 (set_local $t 	(i32.sub (get_local $char) (i32.const 48))) 
	 (if (i32.gt_u (get_local $t) (i32.const 9)) (br $while))
	 (if (i32.lt_u (get_local $t) (i32.const 0)) (br $while))
	 (set_local $i (i32.add (get_local $i) (get_local $t)))
 )
 (return (get_local $i))
)


(func $int.multiply (param i64) (param i64) (result i64)
 (if (call $is.int (get_local $0)) (nop) (call $raise))
  (return (get_global $error))
)



(func $is.int (param i64) (result i32)
 (i64.eq (call $type.of (get_local $0)) (get_global $type.int))
)

;;(func $no.int (param i64) (result i32)
;; (i64.neq (call $type.of (get_local $0)) (get_global $type.int))
;;)

(func $multiply (param i64) (param i64) (result i64)
 (if (call $is.int (get_local $0)) (return (call $int.multiply (get_local $0) (get_local $1))))
  (call $raise) 
  (return (get_global $error))
)

;;(func $type.raw (param i64) (result i32)
;; (i32.wrap/i64 (i64.and (get_local $0) (i64.const 0x00000000FFFFFFFF) ) )
;;)
(func $string.raw (param i64) (result i32)
 (i32.wrap/i64 (i64.and (get_local $0) (i64.const 0x00000000FFFFFFFF) ) )
)

(func $string.add_raw (param i32) (param i32) (result i32)
 (local $len i32)
 (local $new i32)
 (set_local $new (get_global $offset))
  (set_local $len (call $string.copy (get_local $0) (get_global $offset) ))
  (set_global $offset (i32.add (get_local $len) (get_global $offset)))
  (set_local $len (call $string.copy (get_local $1) (get_global $offset) ))
  (set_local $len (i32.add (get_local $len) (i32.const 1))) ;; '\0'
  (set_global $offset (i32.add (get_local $len) (get_global $offset)))
  (get_local $new)
)

(func $string.add (param i64) (param i64) (result i64)
 (local $len i32)
 (local $new i32)
 (set_local $new (get_global $offset))
 (call $assert (call $is.string (get_local $0)))
 (call $assert (call $is.string (get_local $1)))
 ;;(if  (call $is.string (get_local $0)) (nop) (set_local $0 (call $string.from (get_local $0)))) ;; cast
 ;;(if (call $is.string (get_local $1)) (nop) (set_local $1 (call $string.from (get_local $1)))) ;; cast
    (set_local $len (call $string.copy (get_global $offset) (call $val (get_local $0))))
    (set_global $offset (i32.add (get_local $len) (get_global $offset)))
    (drop (call $string.copy (get_global $offset) (call $val (get_local $0))))
    (return (call $string.pointer (get_local $new)))
)

(func $int.parse (param i64) (result i32)
 ;;(call $assert (i32.not (i32.const 0)))
 (call $logi (call $is (get_local $0) (get_global $type.string)))
 (call $assert (call $is (get_local $0) (get_global $type.string)))
 (call $atoi (call $val (get_local $0)))
 ;;(call $todo);;
 ;;(i32.const -1)
)


(func $int (param i64) (result i32)
 (if (call $is.string (get_local $0)) (return (call $int.parse (get_local $0))))
 ;;(if (call $string.is (get_local $0) (call $atoi (get_local $0))))
 (call $raise) ;; cannot cast type to int
 (i32.const -1)
)

(func $string.copy (param i32) (param i32) (result i32) ;; from to !
  (local $char i32)
  (local $start i32)
  (set_local $start (get_local $0))
  (loop $while
  	 (set_local $char (i32.load8 (get_local $0)))
   	 (i32.store8 (get_local $1) (get_local $char))
     (set_local $0 (i32.add (get_local $0) (i32.const 1) ) )     
     (set_local $1 (i32.add (get_local $1) (i32.const 1) ) )     
   	 ;;(call $logc (get_local $char))
     (if (get_local $char) (br $while))
  )
  (i32.sub (i32.sub (get_local $0) (get_local $start)) (i32.const 1))
)
	

(func $string.len (param i32) (result i32)
  (local $offset i32)
  (local $char i32)
  (set_local $offset (get_local $0))
;; 	(return (i32.load8 offset=0 align=1 (get_local $0))) safe len in first byte
  (loop $while
  	 (set_local $char (i32.load8 (get_local $offset)))
     (set_local $offset (i32.add (get_local $offset) (i32.const 1) ) )     
   	 ;;(call $logc (get_local $char))
     (if (get_local $char) (br $while))
  )
  (set_local $offset (i32.sub (get_local $offset) (get_local $0)))
  (i32.sub (get_local $offset) (i32.const 1))
)

(func $string.reverse! (param i32) (result i32)
 (local $len i32)
 (local $char i32)
 (local $char_x i32)
 (local $offset i32) ;; flip len-1-i with i
 (set_local $len (call $string.len (get_local $0)))
 (set_local $offset (i32.sub (get_local $len) (i32.const 1)))
 ;;(call $assert (i32.eq (call $type.of $0) (get_global $type.string)))
  (loop $while
     (set_local $char (i32.load8 (get_local $offset)))
     (set_local $char_x (i32.load8 (get_local $0)))
     (i32.store (get_local $offset) (get_local $char_x))
     (i32.store (get_local $0) (get_local $char))
    (set_local $0 (i32.add (get_local $0) (i32.const 1)))
    (set_local $offset (i32.sub (get_local $offset) (i32.const 1)))
    (if (i32.const 1) (br $while)) 
    ;;(if (i32.lt (get_local $offset) (get_local $0)) (br $while)) 
  )
  (return (get_local $0))
)

(func $main (param i32) (result i32) 
 (local $result i32)
 (call $logi (get_local $0))
 (call $logs (get_local $0))
  (drop (call $string.copy (get_local $0) (i32.const 20)))

  ;;(call $string.copy (i32.const 20) (get_local $0))
  (call $logi (call $string.len (i32.const 20)))
  (call $logs (call $string.reverse! (i32.const 20)))
  ;;(call $string.add_raw (get_local $0) (i32.const 20))
;;(set_local $result (call $string.add_raw (get_global $Hello) (get_local $0)))
;;(set_local $result (call $string.add_raw (i32.const 20) (i32.const 20)))
;;(set_local $result (call $string.add_raw (i32.const 20) (i32.const 100)));; cant copy to self!!
(set_local $result (call $string.add_raw (i32.const 20) (get_global $Hello)))
;;(set_local $result (call $string.add_raw (i32.const 20) (get_local $0)))
(call $logi (get_local $0))
;;(call $string.len (get_global $offset))
(call $logi (get_global $offset))
(call $print (get_global $offset))
;;(call $print (get_global $Hello))
;;(call $string.len (get_global $offset))
;;(return (get_global $offset))
(return (get_local $result))
)
	

(func $test
 ;;(call $assert (i32.eq (i32.const 1) (i32.const 0))) throws, ok
	(local $s i64)  
 ;;(call $log (i64.const 0xFFFFDDDD11112222))
 (call $logf (f64.const 0.123))
 (call $log (i64.reinterpret/f64 (get_global $NaN))) ;; (f64.const 0.123)))
 (call $log (i64.const 0x1111222255556666))
 ;;(call $logx (get_global $NaN))
 (call $ok) 
 (call $assert (i32.eq (i32.const 1) (i32.const 1)))
	(set_local $s (call $string.pointer (get_global $OK))) 
 (call $log (get_local $s)) 
  (call $assert (i64.eq (call $type.of (get_local $s)) (get_global $type.string) ) )
	(call $assert (i64.eq (call $type.of (get_local $s)) (i64.const 9) ) )
	(call $logi (call $type.raw (get_local $s))) 
	(call $assert (i32.eq (call $type.raw (get_local $s)) (get_global $OK) ) )
	(call $assert (call $is.string (get_local $s)))
	(call $assert (call $string.is (get_local $s)))
	(call $assert (call $string.raw (get_local $s)))
 (call $ok) 
	(call $assert (call $type.raw (get_local $s)))
 (call $ok) 	
	;;(call $assert (i32.eq (call $type.pointer (get_local $s) (get_global $OK) ) ) )
 (call $logi (i32.const 42)  ) 
 (call $ok) 	
 ;;(call $print (get_global $DONE))
)

)
