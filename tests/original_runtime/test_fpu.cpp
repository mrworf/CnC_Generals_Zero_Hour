#include "PreRTS.h"
#include "GameLogic/FPUControl.h"

#include <cfenv>
#include <cstdio>

namespace {

unsigned short x87_control_word()
{
  unsigned short value = 0;
  __asm__ volatile("fnstcw %0" : "=m"(value));
  return value;
}

struct FpuStateGuard {
  int rounding = std::fegetround();
  unsigned short control = x87_control_word();

  ~FpuStateGuard()
  {
    std::fesetround(rounding);
    __asm__ volatile("fldcw %0" : : "m"(control));
  }
};

long double x87_precision_probe()
{
  const long double large = 16777216.0L;
  const long double one = 1.0L;
  long double result = 0.0L;
  __asm__ volatile(
      "fldt %[large]\n\t"
      "fldt %[one]\n\t"
      "faddp\n\t"
      "fldt %[large]\n\t"
      "fsubrp\n\t"
      "fstpt %[result]"
      : [result] "=m"(result)
      : [large] "m"(large), [one] "m"(one)
      : "st");
  return result;
}

}  // namespace

int main()
{
  const FpuStateGuard restore_state;
  std::fesetround(FE_DOWNWARD);
  unsigned short control = x87_control_word();
  control = static_cast<unsigned short>((control & ~0x0300U) | 0x0300U);
  __asm__ volatile("fldcw %0" : : "m"(control));

  setFPMode();
  const unsigned short normalized = x87_control_word();
  if (std::fegetround() != FE_TONEAREST) {
    std::fprintf(stderr, "setFPMode did not select nearest rounding\n");
    return 1;
  }
  if ((normalized & 0x0300U) != 0 || (normalized & 0x0c00U) != 0) {
    std::fprintf(stderr, "setFPMode did not select 24-bit/nearest x87 control\n");
    return 1;
  }
  if (x87_precision_probe() != 0.0L) {
    std::fprintf(stderr, "setFPMode did not produce characterized 24-bit arithmetic\n");
    return 1;
  }
  std::printf("original-process fpu: ok provider=FPUControl.cpp rounding=nearest precision=x87-24\n");
  return 0;
}
