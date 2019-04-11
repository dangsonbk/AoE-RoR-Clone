#ifndef COMMON_H
#define COMMON_H

#include <cstdint>
#include <fstream>
#include <iterator>
#include <vector>
#include <list>
#include <string.h>
#include <iostream>
#include <map>
#include <algorithm>

typedef struct rgba {
  uint8_t r, g, b, a;
} RGBAPixel;

typedef struct asset_frame {
  int32_t width;
  int32_t height;
  int32_t hotspot_x;
  int32_t hotspot_y;
  std::vector<RGBAPixel> pixels;
} AssetFrame;

typedef struct asset_frames {
  int32_t frame_width;
  int32_t frame_height;
  int32_t num_frame;
  std::vector<AssetFrame> frames;
} AssetFrames;

#endif
