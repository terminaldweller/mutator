
#include <fstream>
#include <iostream>

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
  std::ofstream myfile;
  myfile.open("./touch");
  myfile << "line one.\n";
  myfile.close();

  return 0;
}
