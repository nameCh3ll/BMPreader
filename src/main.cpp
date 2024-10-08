#include "./BMPfile.h"
#include <cstdio>

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Syntax: BMPreader.exe <path to BMP file>\n");
    return 0;
  }
  BMPfile file;
  file.openBMP(argv[1]);
  file.displayBMP();
  file.closeBMP();
  return 0;
}
