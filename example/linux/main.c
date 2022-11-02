#include <stdint.h>
#include <stdlib.h>

#include "ms.h"

int show_fun(const char *data, uint32_t len) {
  while (len--) {
    putchar(*data++);
  }
}

int main() {
  system("stty -icanon");
  system("stty -echo");

  ms_init(show_fun);

  while (1) {
    ms_input(getchar());
  }
}