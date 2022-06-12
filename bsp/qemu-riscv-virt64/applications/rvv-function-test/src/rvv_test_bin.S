
  .section ".payload","a",@progbits

  .algin 12 + 9
  .globl _rvv_function_test_start, _rvv_function_test_end

_rvv_function_test_start:
  .incbin /nfs/home/zhangzifei/rvv/cgk/rvv/rvv_functional_test/main.bin
_rvv_function_test_end:
  .algin 12 + 3