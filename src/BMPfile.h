#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <sys/types.h>
#include <unordered_map>
#include <utility>

template <typename Alloc = std::allocator<char>> class BMPfile {
private:
#pragma pack(push, 1)
  struct BMPheader {
    u_char ID[2];
    u_int32_t file_size;
    u_char unused[4];
    u_int32_t poffset;
  };
#pragma pack(pop)

#pragma pack(push, 1)
  struct DIBheader {
    u_int32_t DIBheader_size;
    u_int32_t width;
    u_int32_t height;
    u_int16_t color_planes;
    u_int16_t byts_per_pixel;
    u_int32_t comp;
    u_int32_t data_size;
    u_int32_t pwidth;
    u_int32_t pheight;
    u_int32_t colors_count;
    u_int32_t imp_colors_count;
  };
#pragma pack(pop)

private:
  BMPheader _BMPheader;
  DIBheader _DIBheader;
  unsigned char *_pdata = nullptr;

  static constexpr int _SIZE_BMP_h{sizeof(BMPheader)};
  static constexpr int _SIZE_DIB_h{sizeof(DIBheader)};

  [[no_unique_address]] Alloc _alloc;
  using Allocator_traits = std::allocator_traits<decltype(_alloc)>;

  std::unordered_map<unsigned int, const char *> int_background_color{
      {0, "\u001b[40m"}, {255, "\u001b[47m"}};
  std::unordered_map<unsigned int, const char *> int_char{{0, "."}, {255, "@"}};

public:
  void openBMP(const std::string &fileName) {
    std::ifstream BINfile;
    BMPheader *pBMPheader = &_BMPheader;
    DIBheader *pDIBheader = &_DIBheader;
    BINfile.open(std::move(fileName), std::ios::binary);
    if (!BINfile.is_open()) {
      printf("Failed to open file");
      exit(0);
    }
    BINfile.read(reinterpret_cast<char *>(pBMPheader), _SIZE_BMP_h);
    BINfile.read(reinterpret_cast<char *>(pDIBheader), _SIZE_DIB_h);
    BINfile.seekg(_BMPheader.poffset, std::ios::beg);

    _DIBheader.byts_per_pixel /= 8;
    _DIBheader.data_size =
        _DIBheader.width * _DIBheader.height * _DIBheader.byts_per_pixel;
    _pdata = reinterpret_cast<unsigned char *>(
        Allocator_traits::allocate(_alloc, _DIBheader.data_size));

    int row_adding =
        (4 - (_DIBheader.byts_per_pixel * _DIBheader.width % 4)) % 4;
    int row_bytes = _DIBheader.byts_per_pixel * _DIBheader.width;
    unsigned char *p_last_row = _pdata + (row_bytes) * (_DIBheader.height - 1);

    if (_DIBheader.byts_per_pixel == 3 || _DIBheader.byts_per_pixel == 4) {
      for (std::size_t i = 0; i < _DIBheader.height;
           ++i, p_last_row -= row_bytes) {
        BINfile.read(reinterpret_cast<char *>(p_last_row), row_bytes);
        BINfile.seekg(row_adding, std::ios::cur);
        for (std::size_t g = 0; g < _DIBheader.width; ++g)
          std::swap(*(p_last_row + g * _DIBheader.byts_per_pixel),
                    *(p_last_row + 2 + g * _DIBheader.byts_per_pixel));
      }
    } else {
      printf("Failed: 24-bit or 32-bit images only");
      exit(0);
    }
  }

  void displayBMP() {
    printf("ID:  %c%c\n"
           "File Size:  %i.%i KB\n"
           "Offset:  %i\n"
           "DIBh size:  %i\n"
           "Width:  %i\n"
           "Height:  %i\n"
           "Data size:  %i\n"
           "Byts per pixel:  %i\n",
           _BMPheader.ID[0], _BMPheader.ID[1], _BMPheader.file_size / 1024,
           _BMPheader.file_size % 1024, _BMPheader.poffset,
           _DIBheader.DIBheader_size, _DIBheader.width, _DIBheader.height,
           _DIBheader.data_size, _DIBheader.byts_per_pixel);

    printf("\nImage output using ANSI codes:\n\n");
    for (std::size_t i = 0; i < _DIBheader.height; ++i) {
      for (std::size_t g = 0; g < _DIBheader.width; ++g) {
        printf("%s  ",
               int_background_color[_pdata[((i * _DIBheader.width) + g) *
                                           _DIBheader.byts_per_pixel]]);
      }
      printf("\u001b[0m\n");
    }

    printf("\nImage output using ASCII codes:\n\n");
    for (std::size_t i = 0; i < _DIBheader.height; ++i) {
      for (std::size_t g = 0; g < _DIBheader.width; ++g) {
        printf("%s", int_char[_pdata[((i * _DIBheader.width) + g) *
                                     _DIBheader.byts_per_pixel]]);
      }
      printf("\n");
    }
  }

  void closeBMP() {
    if (!_pdata) {
      Allocator_traits::destroy(_alloc, _pdata);
      Allocator_traits::deallocate(_alloc, reinterpret_cast<char *>(_pdata),
                                   _DIBheader.data_size);
    }
  }

  ~BMPfile() { closeBMP(); }
};
