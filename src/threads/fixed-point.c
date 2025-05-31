#include "fixed-point.h"
#include <stdint.h>

int int_to_fp(int n)
{
  int temp = n * F;
  return temp;
}
int fp_to_int_round(int x)
{
  if (x >= 0)
  {
    return (x + F / 2) / F;
  }
  else
  {
    return (x - F / 2) / F;
  }
}
int fp_to_int(int x)
{
  return x / F;
}
int add_fp(int x, int y)
{
  return x + y;
}
int add_mixed(int x, int n)
{
  return x + int_to_fp(n);
}
int sub_fp(int x, int y)
{
  return x - y;
}
int sub_mixed(int x, int n)
{
  return x - int_to_fp(n);
}
int mult_fp(int x, int y)
{
  return (((int64_t)x) * y / F);
}
int mult_mixed(int x, int y)
{
  return x * y;
}
int div_fp(int x, int y)
{
  return ((int64_t)x) * F / y;
}
int div_mixed(int x, int n)
{
  return x / n;
}