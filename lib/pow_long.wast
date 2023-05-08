(module
  (func $pow_long (param i64 i64) (result i64)
    (local i64)
    i64.const 1
    local.set 2
    loop  ;; label = @1
      i64.const 1
      local.get 0
      local.get 1
      i64.const 1
      i64.and
      i64.eqz
      select
      local.get 2
      i64.mul
      local.set 2
      local.get 1
      i64.const 2
      i64.lt_u
      i32.eqz
      if  ;; label = @2
        local.get 1
        i64.const 1
        i64.shr_s
        local.set 1
        local.get 0
        local.get 0
        i64.mul
        local.set 0
        br 1 (;@1;)
      end
    end
    local.get 2)
  )