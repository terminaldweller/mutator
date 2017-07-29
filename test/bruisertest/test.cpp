
#if 1
#include <fstream>
#include <iostream>
#endif

namespace devi
{
  class LOCO
  {
    public:
      LOCO () {}

      void lupo (void)
      {
        int a = 1;
        int b = 2;
        int c = 3;
      }
  };
}

int main(int argc, const char **argv)
{
#if 1
  std::ofstream myfile;
  myfile.open("./touch");
  myfile << "line one.\n";
  myfile.close();
#endif

  return 0;
}
