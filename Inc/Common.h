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
  uint32_t width;
  uint32_t height;
  uint32_t hotspot_x;
  uint32_t hotspot_y;
  std::vector<RGBAPixel> pixels;
} AssetFrame;

typedef struct asset_frames {
  uint32_t frame_width;
  uint32_t frame_height;
  uint32_t num_frame;
  std::vector<AssetFrame> frames;
} AssetFrames;

#endif
