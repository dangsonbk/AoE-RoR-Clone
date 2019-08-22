#ifndef ASSETS_HPP
#define ASSETS_HPP

#include <Common.h>

// #include "spdlog/spdlog.h"
using namespace std;
// Struct defined by https://github.com/SFTtech/openage

typedef struct drs_header {
    char copyright[40];
    char version[4];
    char ftype[12];
    int32_t table_count;
    int32_t file_offset;
} DRSHeader;

typedef struct drs_table_raw_info {
  char file_extension[4];
  int32_t file_info_offset;
  int32_t num_files;
} DRSTableInfo;

typedef struct drs_file_info {
  int32_t file_id;
  int32_t file_data_offset;
  int32_t file_size;
} DRSFileInfo;

typedef struct slp_header {
  char  version[4];
  int32_t num_frames;
  char  comment[24];
} SLPHeader;

typedef struct slp_frame_info {
  uint32_t cmd_table_offset;
  uint32_t outline_table_offset;
  uint32_t palette_offset;
  uint32_t properties;
  int32_t  width;
  int32_t  height;
  int32_t  hotspot_x;
  int32_t  hotspot_y;
} SLPFrameInfo;

typedef struct slp_frame_row_edge {
  uint16_t left_space;
  uint16_t right_space;
} SLPFrameRowEdge;

typedef struct slp_command_offset {
  uint32_t offset;
} SLPCommandOffset;

enum slp_cmd
{
  cmd_color_list,
  cmd_skip,
  cmd_big_color_list,
  cmd_big_skip,
  cmd_player_color_list = 0x6,
  cmd_fill,
  cmd_player_color_fill = 0xa,
  cmd_shadow_transparent,
  cmd_shadow_player = 0xe,
  cmd_end_of_row = 0xf,
  cmd_outline = 0x4e,
  cmd_outline_span = 0x5e,
};

class Assets {
private:
  map<int32_t, int32_t> m_file_info_map_slp;
  map<int32_t, int32_t> m_file_info_map_bin;
  vector<RGBAPixel> m_palletes;

private:
  void _load_texture(std::string const& path);
  void _load_palette(std::string const& path);
  void _read_drs_header(std::fstream &fh, drs_header *header);
  void _read_drs_table_info(std::fstream &fh, list<DRSTableInfo> *table_info, int table_count);
  void _read_slp_files_list(fstream &fh, list<DRSTableInfo>::iterator it);
  void _read_bin_files_list(fstream &fh, list<DRSTableInfo>::iterator it);
  AssetFrames _read_slp_frames_by_id(fstream &fh, int32_t id);
public:
  Assets(/* args */);
  ~Assets();
  AssetFrames get_by_id(int32_t id);
};
#endif
