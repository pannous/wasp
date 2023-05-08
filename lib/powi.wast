(module
  (func $powi  (param i32 i32) (result i64)
    (local i64)
    i64.const 1
    local.set 2
    loop  ;; label = @1
      local.get 2
      local.get 0
      i32.const 1
      local.get 1
      i32.const 1
      i32.and
      select
      i64.extend_i32_s
      i64.mul
      local.set 2
      local.get 1
      i32.const 2
      i32.lt_u
      i32.eqz
      if  ;; label = @2
        local.get 1
        i32.const 1
        i32.shr_s
        local.set 1
        local.get 0
        local.get 0
        i32.mul
        local.set 0
        br 1 (;@1;)
      end
    end
    local.get 2)
)