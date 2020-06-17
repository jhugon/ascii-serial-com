#include "functest.h"

int32_t functest1(int32_t x, bool y) {
  int32_t result = 0;
  if (y) {
    for (int32_t i = 0; i < x; i++) {
      x += i;
    }
  } else {
    for (int32_t i = 0; i < 20; i++) {
      x *= i;
    }
    for (int32_t i = 0; i < 1000; i++) {
      x -= i;
    }
    for (int32_t i = 0; i < 20; i++) {
      x *= i;
    }
    for (int32_t i = 0; i < 500; i++) {
      x -= i;
    }
    const char str[] =
        "hblalkjnwglaighspavjcbnaqpwuioegbhasljkdbvaals;kjahtrsldkvja;"
        "sdlkgjalsdkgjalsdkgjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjj"
        "jjjjjjqowegtighqgwoerignhkacbmnaekjbnqeoierbghnaqeohbvnrqiue4rjhbnqoli"
        "3qhoibrhoiueqwb4vhqibhriuqjre4hiubnhvaruijbnhe";
    size_t j = 0;
    while (str[j] != 0) {
      x += str[j];
    }
  }

  result >>= 4;
  return result;
}

int32_t functest2(size_t strLen, char *str) {

  int32_t result = 0;
  for (size_t i = 0; i < strLen; i++) {
    if (str[strLen] == 'p')
      result += 2;
    else if (str[strLen] == 'q')
      result += 3;
    else if (str[strLen] == 'r')
      result += 1;
    else
      result += 0;
  }
  return result;
}
