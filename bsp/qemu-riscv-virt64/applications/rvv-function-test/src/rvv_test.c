#include <rtthread.h>
#include <rthw.h>

extern char _rvv_function_test_start;

void call_rvv_function_test() {
  rt_kprintf("rvv_function_test\n");
  void (*fn)() = (void *)(&_rvv_function_test_start);
  fn();
  return ;
}