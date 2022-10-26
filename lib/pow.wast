(module
 (memory 0 10)
  (func $pow  (param f64 f64) (result f64)
    (local i32 i32 i32 i32 i32 i32 i32 i32 i32 i64 i64 f64 f64 f64 f64 f64)
    f64.const 0x1p+0 (;=1;)
    local.set 13
    block  ;; label = @1
      local.get 1
      i64.reinterpret_f64
      local.tee 11
      i64.const 32
      i64.shr_u
      i32.wrap_i64
      local.tee 2
      i32.const 2147483647
      i32.and
      local.tee 3
      local.get 11
      i32.wrap_i64
      local.tee 4
      i32.or
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      i64.reinterpret_f64
      local.tee 12
      i32.wrap_i64
      local.set 5
      block  ;; label = @2
        local.get 12
        i64.const 32
        i64.shr_u
        i32.wrap_i64
        local.tee 6
        i32.const 1072693248
        i32.ne
        br_if 0 (;@2;)
        local.get 5
        i32.eqz
        br_if 1 (;@1;)
      end
      block  ;; label = @2
        block  ;; label = @3
          local.get 6
          i32.const 2147483647
          i32.and
          local.tee 7
          i32.const 2146435072
          i32.gt_u
          br_if 0 (;@3;)
          local.get 7
          i32.const 2146435072
          i32.eq
          local.get 5
          i32.const 0
          i32.ne
          i32.and
          br_if 0 (;@3;)
          local.get 3
          i32.const 2146435072
          i32.gt_u
          br_if 0 (;@3;)
          local.get 3
          i32.const 2146435072
          i32.ne
          br_if 1 (;@2;)
          local.get 4
          i32.eqz
          br_if 1 (;@2;)
        end
        local.get 0
        local.get 1
        f64.add
        return
      end
      block  ;; label = @2
        block  ;; label = @3
          block  ;; label = @4
            block  ;; label = @5
              local.get 12
              i64.const -1
              i64.gt_s
              br_if 0 (;@5;)
              i32.const 2
              local.set 8
              local.get 3
              i32.const 1128267775
              i32.gt_u
              br_if 1 (;@4;)
              local.get 3
              i32.const 1072693248
              i32.lt_u
              br_if 0 (;@5;)
              local.get 3
              i32.const 20
              i32.shr_u
              local.set 9
              block  ;; label = @6
                local.get 3
                i32.const 1094713344
                i32.lt_u
                br_if 0 (;@6;)
                i32.const 0
                local.set 8
                local.get 4
                i32.const 1075
                local.get 9
                i32.sub
                local.tee 9
                i32.shr_u
                local.tee 10
                local.get 9
                i32.shl
                local.get 4
                i32.ne
                br_if 2 (;@4;)
                i32.const 2
                local.get 10
                i32.const 1
                i32.and
                i32.sub
                local.set 8
                br 2 (;@4;)
              end
              i32.const 0
              local.set 8
              local.get 4
              br_if 3 (;@2;)
              i32.const 0
              local.set 8
              local.get 3
              i32.const 1043
              local.get 9
              i32.sub
              local.tee 4
              i32.shr_u
              local.tee 9
              local.get 4
              i32.shl
              local.get 3
              i32.ne
              br_if 2 (;@3;)
              i32.const 2
              local.get 9
              i32.const 1
              i32.and
              i32.sub
              local.set 8
              br 2 (;@3;)
            end
            i32.const 0
            local.set 8
          end
          local.get 4
          br_if 1 (;@2;)
        end
        block  ;; label = @3
          local.get 3
          i32.const 2146435072
          i32.ne
          br_if 0 (;@3;)
          local.get 7
          i32.const -1072693248
          i32.add
          local.get 5
          i32.or
          i32.eqz
          br_if 2 (;@1;)
          block  ;; label = @4
            local.get 7
            i32.const 1072693248
            i32.lt_u
            br_if 0 (;@4;)
            local.get 1
            f64.const 0x0p+0 (;=0;)
            local.get 11
            i64.const -1
            i64.gt_s
            select
            return
          end
          f64.const 0x0p+0 (;=0;)
          local.get 1
          f64.neg
          local.get 11
          i64.const -1
          i64.gt_s
          select
          return
        end
        block  ;; label = @3
          local.get 3
          i32.const 1072693248
          i32.ne
          br_if 0 (;@3;)
          block  ;; label = @4
            local.get 11
            i64.const -1
            i64.le_s
            br_if 0 (;@4;)
            local.get 0
            return
          end
          f64.const 0x1p+0 (;=1;)
          local.get 0
          f64.div
          return
        end
        block  ;; label = @3
          local.get 2
          i32.const 1073741824
          i32.ne
          br_if 0 (;@3;)
          local.get 0
          local.get 0
          f64.mul
          return
        end
        local.get 2
        i32.const 1071644672
        i32.ne
        br_if 0 (;@2;)
        local.get 12
        i64.const 0
        i64.lt_s
        br_if 0 (;@2;)
        local.get 0
        f64.sqrt
        return
      end
      local.get 0
      f64.abs
      local.set 13
      block  ;; label = @2
        local.get 5
        br_if 0 (;@2;)
        block  ;; label = @3
          local.get 6
          i32.const 1073741823
          i32.and
          i32.const 1072693248
          i32.eq
          br_if 0 (;@3;)
          local.get 7
          br_if 1 (;@2;)
        end
        f64.const 0x1p+0 (;=1;)
        local.get 13
        f64.div
        local.get 13
        local.get 11
        i64.const 0
        i64.lt_s
        select
        local.set 13
        local.get 12
        i64.const -1
        i64.gt_s
        br_if 1 (;@1;)
        block  ;; label = @3
          local.get 8
          local.get 7
          i32.const -1072693248
          i32.add
          i32.or
          br_if 0 (;@3;)
          local.get 13
          local.get 13
          f64.sub
          local.tee 1
          local.get 1
          f64.div
          return
        end
        local.get 13
        f64.neg
        local.get 13
        local.get 8
        i32.const 1
        i32.eq
        select
        return
      end
      f64.const 0x1p+0 (;=1;)
      local.set 14
      block  ;; label = @2
        local.get 12
        i64.const -1
        i64.gt_s
        br_if 0 (;@2;)
        block  ;; label = @3
          block  ;; label = @4
            local.get 8
            br_table 0 (;@4;) 1 (;@3;) 2 (;@2;)
          end
          local.get 0
          local.get 0
          f64.sub
          local.tee 1
          local.get 1
          f64.div
          return
        end
        f64.const -0x1p+0 (;=-1;)
        local.set 14
      end
      block  ;; label = @2
        block  ;; label = @3
          local.get 3
          i32.const 1105199105
          i32.lt_u
          br_if 0 (;@3;)
          block  ;; label = @4
            local.get 3
            i32.const 1139802113
            i32.lt_u
            br_if 0 (;@4;)
            block  ;; label = @5
              local.get 7
              i32.const 1072693247
              i32.gt_u
              br_if 0 (;@5;)
              f64.const inf (;=inf;)
              f64.const 0x0p+0 (;=0;)
              local.get 11
              i64.const 0
              i64.lt_s
              select
              return
            end
            f64.const inf (;=inf;)
            f64.const 0x0p+0 (;=0;)
            local.get 2
            i32.const 0
            i32.gt_s
            select
            return
          end
          block  ;; label = @4
            local.get 7
            i32.const 1072693246
            i32.gt_u
            br_if 0 (;@4;)
            local.get 14
            f64.const 0x1.7e43c8800759cp+996 (;=1e+300;)
            f64.mul
            f64.const 0x1.7e43c8800759cp+996 (;=1e+300;)
            f64.mul
            local.get 14
            f64.const 0x1.56e1fc2f8f359p-997 (;=1e-300;)
            f64.mul
            f64.const 0x1.56e1fc2f8f359p-997 (;=1e-300;)
            f64.mul
            local.get 11
            i64.const 0
            i64.lt_s
            select
            return
          end
          block  ;; label = @4
            local.get 7
            i32.const 1072693249
            i32.lt_u
            br_if 0 (;@4;)
            local.get 14
            f64.const 0x1.7e43c8800759cp+996 (;=1e+300;)
            f64.mul
            f64.const 0x1.7e43c8800759cp+996 (;=1e+300;)
            f64.mul
            local.get 14
            f64.const 0x1.56e1fc2f8f359p-997 (;=1e-300;)
            f64.mul
            f64.const 0x1.56e1fc2f8f359p-997 (;=1e-300;)
            f64.mul
            local.get 2
            i32.const 0
            i32.gt_s
            select
            return
          end
          local.get 13
          f64.const -0x1p+0 (;=-1;)
          f64.add
          local.tee 0
          f64.const 0x1.4ae0bf85ddf44p-26 (;=1.92596e-08;)
          f64.mul
          local.get 0
          local.get 0
          f64.mul
          f64.const 0x1p-1 (;=0.5;)
          local.get 0
          local.get 0
          f64.const -0x1p-2 (;=-0.25;)
          f64.mul
          f64.const 0x1.5555555555555p-2 (;=0.333333;)
          f64.add
          f64.mul
          f64.sub
          f64.mul
          f64.const -0x1.71547652b82fep+0 (;=-1.4427;)
          f64.mul
          f64.add
          local.tee 13
          local.get 13
          local.get 0
          f64.const 0x1.715476p+0 (;=1.4427;)
          f64.mul
          local.tee 15
          f64.add
          i64.reinterpret_f64
          i64.const -4294967296
          i64.and
          f64.reinterpret_i64
          local.tee 0
          local.get 15
          f64.sub
          f64.sub
          local.set 15
          br 1 (;@2;)
        end
        local.get 13
        f64.const 0x1p+53 (;=9.0072e+15;)
        f64.mul
        local.tee 0
        local.get 13
        local.get 7
        i32.const 1048576
        i32.lt_u
        local.tee 3
        select
        local.set 13
        local.get 0
        i64.reinterpret_f64
        i64.const 32
        i64.shr_u
        i32.wrap_i64
        local.get 7
        local.get 3
        select
        local.tee 2
        i32.const 1048575
        i32.and
        local.tee 4
        i32.const 1072693248
        i32.or
        local.set 5
        i32.const -1076
        i32.const -1023
        local.get 3
        select
        local.get 2
        i32.const 20
        i32.shr_s
        i32.add
        local.set 2
        i32.const 0
        local.set 3
        block  ;; label = @3
          local.get 4
          i32.const 235663
          i32.lt_u
          br_if 0 (;@3;)
          block  ;; label = @4
            local.get 4
            i32.const 767610
            i32.ge_u
            br_if 0 (;@4;)
            i32.const 1
            local.set 3
            br 1 (;@3;)
          end
          local.get 4
          i32.const 1071644672
          i32.or
          local.set 5
          local.get 2
          i32.const 1
          i32.add
          local.set 2
        end
        local.get 3
        i32.const 3
        i32.shl
        local.tee 4
        i32.const 1040
        i32.add
        f64.load
        f64.const 0x1p+0 (;=1;)
        local.get 4
        i32.const 1024
        i32.add
        f64.load
        local.tee 0
        local.get 5
        i64.extend_i32_u
        i64.const 32
        i64.shl
        local.get 13
        i64.reinterpret_f64
        i64.const 4294967295
        i64.and
        i64.or
        f64.reinterpret_i64
        local.tee 16
        f64.add
        f64.div
        local.tee 13
        local.get 16
        local.get 0
        f64.sub
        local.tee 15
        local.get 3
        i32.const 18
        i32.shl
        local.get 5
        i32.const 1
        i32.shr_u
        i32.add
        i32.const 537395200
        i32.add
        i64.extend_i32_u
        i64.const 32
        i64.shl
        f64.reinterpret_i64
        local.tee 17
        local.get 15
        local.get 13
        f64.mul
        local.tee 15
        i64.reinterpret_f64
        i64.const -4294967296
        i64.and
        f64.reinterpret_i64
        local.tee 13
        f64.mul
        f64.sub
        local.get 16
        local.get 17
        local.get 0
        f64.sub
        f64.sub
        local.get 13
        f64.mul
        f64.sub
        f64.mul
        local.tee 0
        local.get 13
        local.get 13
        f64.mul
        local.tee 16
        f64.const 0x1.8p+1 (;=3;)
        f64.add
        local.get 0
        local.get 15
        local.get 13
        f64.add
        f64.mul
        local.get 15
        local.get 15
        f64.mul
        local.tee 0
        local.get 0
        f64.mul
        local.get 0
        local.get 0
        local.get 0
        local.get 0
        local.get 0
        f64.const 0x1.a7e284a454eefp-3 (;=0.206975;)
        f64.mul
        f64.const 0x1.d864a93c9db65p-3 (;=0.230661;)
        f64.add
        f64.mul
        f64.const 0x1.17460a91d4101p-2 (;=0.272728;)
        f64.add
        f64.mul
        f64.const 0x1.55555518f264dp-2 (;=0.333333;)
        f64.add
        f64.mul
        f64.const 0x1.b6db6db6fabffp-2 (;=0.428571;)
        f64.add
        f64.mul
        f64.const 0x1.3333333333303p-1 (;=0.6;)
        f64.add
        f64.mul
        f64.add
        local.tee 17
        f64.add
        i64.reinterpret_f64
        i64.const -4294967296
        i64.and
        f64.reinterpret_i64
        local.tee 0
        f64.mul
        local.get 15
        local.get 17
        local.get 0
        f64.const -0x1.8p+1 (;=-3;)
        f64.add
        local.get 16
        f64.sub
        f64.sub
        f64.mul
        f64.add
        local.tee 15
        local.get 15
        local.get 13
        local.get 0
        f64.mul
        local.tee 13
        f64.add
        i64.reinterpret_f64
        i64.const -4294967296
        i64.and
        f64.reinterpret_i64
        local.tee 0
        local.get 13
        f64.sub
        f64.sub
        f64.const 0x1.ec709dc3a03fdp-1 (;=0.961797;)
        f64.mul
        local.get 0
        f64.const -0x1.e2fe0145b01f5p-28 (;=-7.02846e-09;)
        f64.mul
        f64.add
        f64.add
        local.tee 13
        local.get 4
        i32.const 1056
        i32.add
        f64.load
        local.tee 15
        local.get 13
        local.get 0
        f64.const 0x1.ec709ep-1 (;=0.961797;)
        f64.mul
        local.tee 16
        f64.add
        f64.add
        local.get 2
        f64.convert_i32_s
        local.tee 13
        f64.add
        i64.reinterpret_f64
        i64.const -4294967296
        i64.and
        f64.reinterpret_i64
        local.tee 0
        local.get 13
        f64.sub
        local.get 15
        f64.sub
        local.get 16
        f64.sub
        f64.sub
        local.set 15
      end
      local.get 0
      local.get 11
      i64.const -4294967296
      i64.and
      f64.reinterpret_i64
      local.tee 16
      f64.mul
      local.tee 13
      local.get 15
      local.get 1
      f64.mul
      local.get 1
      local.get 16
      f64.sub
      local.get 0
      f64.mul
      f64.add
      local.tee 1
      f64.add
      local.tee 0
      i64.reinterpret_f64
      local.tee 11
      i32.wrap_i64
      local.set 3
      block  ;; label = @2
        block  ;; label = @3
          local.get 11
          i64.const 32
          i64.shr_u
          i32.wrap_i64
          local.tee 5
          i32.const 1083179008
          i32.lt_s
          br_if 0 (;@3;)
          block  ;; label = @4
            local.get 5
            i32.const -1083179008
            i32.add
            local.get 3
            i32.or
            i32.eqz
            br_if 0 (;@4;)
            local.get 14
            f64.const 0x1.7e43c8800759cp+996 (;=1e+300;)
            f64.mul
            f64.const 0x1.7e43c8800759cp+996 (;=1e+300;)
            f64.mul
            return
          end
          local.get 1
          f64.const 0x1.71547652b82fep-54 (;=8.00857e-17;)
          f64.add
          local.get 0
          local.get 13
          f64.sub
          f64.gt
          i32.eqz
          br_if 1 (;@2;)
          local.get 14
          f64.const 0x1.7e43c8800759cp+996 (;=1e+300;)
          f64.mul
          f64.const 0x1.7e43c8800759cp+996 (;=1e+300;)
          f64.mul
          return
        end
        local.get 5
        i32.const 2147482624
        i32.and
        i32.const 1083231232
        i32.lt_u
        br_if 0 (;@2;)
        block  ;; label = @3
          local.get 5
          i32.const 1064252416
          i32.add
          local.get 3
          i32.or
          i32.eqz
          br_if 0 (;@3;)
          local.get 14
          f64.const 0x1.56e1fc2f8f359p-997 (;=1e-300;)
          f64.mul
          f64.const 0x1.56e1fc2f8f359p-997 (;=1e-300;)
          f64.mul
          return
        end
        local.get 1
        local.get 0
        local.get 13
        f64.sub
        f64.le
        i32.eqz
        br_if 0 (;@2;)
        local.get 14
        f64.const 0x1.56e1fc2f8f359p-997 (;=1e-300;)
        f64.mul
        f64.const 0x1.56e1fc2f8f359p-997 (;=1e-300;)
        f64.mul
        return
      end
      i32.const 0
      local.set 3
      block  ;; label = @2
        local.get 5
        i32.const 2147483647
        i32.and
        local.tee 4
        i32.const 1071644673
        i32.lt_u
        br_if 0 (;@2;)
        i32.const 0
        i32.const 1048576
        local.get 4
        i32.const 20
        i32.shr_u
        i32.const -1022
        i32.add
        i32.shr_u
        local.get 5
        i32.add
        local.tee 5
        i32.const 1048575
        i32.and
        i32.const 1048576
        i32.or
        i32.const 1043
        local.get 5
        i32.const 20
        i32.shr_u
        i32.const 2047
        i32.and
        local.tee 4
        i32.sub
        i32.shr_u
        local.tee 3
        i32.sub
        local.get 3
        local.get 11
        i64.const 0
        i64.lt_s
        select
        local.set 3
        local.get 1
        local.get 13
        i32.const -1048576
        local.get 4
        i32.const -1023
        i32.add
        i32.shr_s
        local.get 5
        i32.and
        i64.extend_i32_u
        i64.const 32
        i64.shl
        f64.reinterpret_i64
        f64.sub
        local.tee 13
        f64.add
        i64.reinterpret_f64
        local.set 11
      end
      block  ;; label = @2
        block  ;; label = @3
          local.get 3
          i32.const 20
          i32.shl
          local.get 11
          i64.const -4294967296
          i64.and
          f64.reinterpret_i64
          local.tee 0
          f64.const 0x1.62e43p-1 (;=0.693147;)
          f64.mul
          local.tee 15
          local.get 1
          local.get 0
          local.get 13
          f64.sub
          f64.sub
          f64.const 0x1.62e42fefa39efp-1 (;=0.693147;)
          f64.mul
          local.get 0
          f64.const -0x1.05c610ca86c39p-29 (;=-1.90465e-09;)
          f64.mul
          f64.add
          local.tee 13
          f64.add
          local.tee 1
          local.get 1
          local.get 1
          local.get 1
          local.get 1
          f64.mul
          local.tee 0
          local.get 0
          local.get 0
          local.get 0
          local.get 0
          f64.const 0x1.6376972bea4dp-25 (;=4.13814e-08;)
          f64.mul
          f64.const -0x1.bbd41c5d26bf1p-20 (;=-1.65339e-06;)
          f64.add
          f64.mul
          f64.const 0x1.1566aaf25de2cp-14 (;=6.61376e-05;)
          f64.add
          f64.mul
          f64.const -0x1.6c16c16bebd93p-9 (;=-0.00277778;)
          f64.add
          f64.mul
          f64.const 0x1.555555555553ep-3 (;=0.166667;)
          f64.add
          f64.mul
          f64.sub
          local.tee 0
          f64.mul
          local.get 0
          f64.const -0x1p+1 (;=-2;)
          f64.add
          f64.div
          local.get 13
          local.get 1
          local.get 15
          f64.sub
          f64.sub
          local.tee 0
          local.get 1
          local.get 0
          f64.mul
          f64.add
          f64.sub
          f64.sub
          f64.const 0x1p+0 (;=1;)
          f64.add
          local.tee 1
          i64.reinterpret_f64
          local.tee 11
          i64.const 32
          i64.shr_u
          i32.wrap_i64
          i32.add
          local.tee 5
          i32.const 1048575
          i32.gt_s
          br_if 0 (;@3;)
          local.get 1
          local.get 3
          call $pow_helper1
          local.set 1
          br 1 (;@2;)
        end
        local.get 5
        i64.extend_i32_u
        i64.const 32
        i64.shl
        local.get 11
        i64.const 4294967295
        i64.and
        i64.or
        f64.reinterpret_i64
        local.set 1
      end
      local.get 14
      local.get 1
      f64.mul
      local.set 13
    end
    local.get 13)
  (func $pow_helper1 (param f64 i32) (result f64)
    block  ;; label = @1
      block  ;; label = @2
        local.get 1
        i32.const 1024
        i32.lt_s
        br_if 0 (;@2;)
        local.get 0
        f64.const 0x1p+1023 (;=8.98847e+307;)
        f64.mul
        local.set 0
        block  ;; label = @3
          local.get 1
          i32.const 2047
          i32.ge_s
          br_if 0 (;@3;)
          local.get 1
          i32.const -1023
          i32.add
          local.set 1
          br 2 (;@1;)
        end
        local.get 0
        f64.const 0x1p+1023 (;=8.98847e+307;)
        f64.mul
        local.set 0
        local.get 1
        i32.const 3069
        local.get 1
        i32.const 3069
        i32.lt_s
        select
        i32.const -2046
        i32.add
        local.set 1
        br 1 (;@1;)
      end
      local.get 1
      i32.const -1023
      i32.gt_s
      br_if 0 (;@1;)
      local.get 0
      f64.const 0x1p-1022 (;=2.22507e-308;)
      f64.mul
      local.set 0
      block  ;; label = @2
        local.get 1
        i32.const -2045
        i32.le_s
        br_if 0 (;@2;)
        local.get 1
        i32.const 1022
        i32.add
        local.set 1
        br 1 (;@1;)
      end
      local.get 0
      f64.const 0x1p-1022 (;=2.22507e-308;)
      f64.mul
      local.set 0
      local.get 1
      i32.const -3066
      local.get 1
      i32.const -3066
      i32.gt_s
      select
      i32.const 2044
      i32.add
      local.set 1
    end
    local.get 0
    local.get 1
    i32.const 1023
    i32.add
    i64.extend_i32_u
    i64.const 52
    i64.shl
    f64.reinterpret_i64
    f64.mul)

  (table (;0;) 1 1 funcref)
  (data (;0;) (i32.const 1024) "\00\00\00\00\00\00\f0?\00\00\00\00\00\00\f8?\00\00\00\00\00\00\00\00\06\d0\cfC\eb\fdL>\00\00\00\00\00\00\00\00\00\00\00@\03\b8\e2?")
  )
