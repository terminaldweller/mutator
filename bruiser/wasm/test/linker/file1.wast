(module
 (type $FUNCSIG$ii (func (param i32) (result i32)))
 (type $FUNCSIG$iii (func (param i32 i32) (result i32)))
 (import "env" "printf" (func $printf (param i32 i32) (result i32)))
 (table 0 anyfunc)
 (memory $0 1)
 (data (i32.const 16) "external symbol\00")
 (data (i32.const 32) "\n\00\00\00")
 (data (i32.const 36) "\0b\00\00\00")
 (export "memory" (memory $0))
 (export "print" (func $print))
 (export "dummy_f_1_0" (func $dummy_f_1_0))
 (export "dymmy_f_1_1" (func $dymmy_f_1_1))
 (func $print (; 1 ;)
  (drop
   (call $printf
    (i32.const 16)
    (i32.const 0)
   )
  )
 )
 (func $dummy_f_1_0 (; 2 ;) (param $0 i32) (result i32)
  (local $1 i32)
  (set_local $1
   (i32.const 1)
  )
  (block $label$0
   (loop $label$1
    (br_if $label$0
     (i32.lt_s
      (get_local $0)
      (i32.const 1)
     )
    )
    (set_local $1
     (i32.mul
      (get_local $0)
      (get_local $1)
     )
    )
    (set_local $0
     (i32.add
      (get_local $0)
      (i32.const -1)
     )
    )
    (br $label$1)
   )
  )
  (get_local $1)
 )
 (func $dymmy_f_1_1 (; 3 ;) (param $0 i32) (result i32)
  (i32.shl
   (get_local $0)
   (i32.const 1)
  )
 )
)
