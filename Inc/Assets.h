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

class Assets
{
private:
public:
    Assets(/* args */);
    ~Assets();
    void load();
};
#endif