#ifndef ASSETS_HPP
#define ASSETS_HPP

#include <cstdint>
#include <fstream>
#include <iterator>
#include <vector>

struct drs_header {
    char copyright[40];
    char version[4];
    char ftype[12];
    int32_t table_count;
    int32_t file_offset;
};

struct drs_table_info {
	char file_extension[4];
	int32_t file_info_offset;
	int32_t num_files;
};

struct drs_file_info {
	int32_t file_id;
	int32_t file_data_offset;
	int32_t file_size;
};

struct slp_header {
  char  version[4];
  int32_t num_frames;
  char  comment[24];
};

struct slp_frame_info {
  uint32_t cmd_table_offset;
  uint32_t outline_table_offset;
  uint32_t palette_offset;
  uint32_t properties;
  int32_t  width;
  int32_t  height;
  int32_t  hotspot_x;
  int32_t  hotspot_y;
};

struct slp_frame_row_edge {
  uint16_t left_space;
  uint16_t right_space;
};

struct slp_command_offset {
  uint32_t offset;
};

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

class Assets
{
private:
public:
    Assets(/* args */);
    ~Assets();
    void load();
};
#endif