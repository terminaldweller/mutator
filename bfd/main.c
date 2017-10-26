
/*intentionally left blank*/
#include "bfd.h"
#include <stdlib.h>
#include <stdio.h>

bfd* dobfd(const char* __filename, const char* __target) {
  bfd* abfd;
  bfd_init();

  abfd = bfd_openr(__filename, __target);
  if (abfd) return abfd;
  else return NULL;
}

int main(int argv, char** argc) {
  bfd* abfd = dobfd("../mutator-lvl0", "default");
  if (abfd) {
    printf("bfd section count is %d.\n", bfd_count_sections(abfd));
  }
  return 0;
}
/*intentionally left blank*/

