#include "Assets.h"
#include <iostream>

/*
 * http://artho.com/age/drs.html
 * https://github.com/SFTtech/openage/blob/master/doc/media/slp-files.md
*/

Assets::Assets(/* args */)
{
    load();
}

Assets::~Assets()
{}

void Assets::load()
{
    drs_header header = {0};
    drs_table_info table_info = {0};
    drs_file_info file_info = {0};
    std::fstream fh;

    fh.open("../Assets/Origin/graphics.drs", std::fstream::in | std::fstream::binary);

    fh.read((char*)&header, sizeof(drs_header));
    std::cout << header.copyright << "\n" << header.version << "\n" << header.ftype << "\n" << header.table_count << "\n" << header.file_offset << std::endl;
    
    for(int i = 0; i < header.table_count; i++) {
        fh.read((char*)&table_info, sizeof(drs_table_info));
        std::cout << "- Table information" << std::endl;
        std::cout << table_info.file_extension << "\n" << table_info.file_info_offset << "\n" << table_info.num_files << std::endl;
        
        int32_t file_offset = table_info.file_info_offset;
        for(int j = 0; j < table_info.num_files; j++){
            std::cout << "- File information: " << j << std::endl;
            fh.seekg(file_offset);
            fh.read((char*)&file_info, sizeof(drs_file_info));
            file_offset += file_info.file_size;
            std::cout << file_info.file_id << "\n" << file_info.file_data_offset << "\n" << file_info.file_size << std::endl;
        }
    }


    fh.close();


}