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
    slp_header slp_file_header = {0};
    slp_frame_info slp_file_frame_info = {0};
    slp_frame_row_edge slp_file_frame_row_edge = {0};
    slp_command_offset slp_file_command_offset = {};

    std::fstream fh;

    fh.open("../Assets/Origin/graphics.drs", std::fstream::in | std::fstream::binary);
    // fh.open("../Assets/Origin/Interfac.drs", std::fstream::in | std::fstream::binary);

    fh.read((char*)&header, sizeof(drs_header));
    std::cout << header.copyright << "\n" << header.version << "\n" << header.ftype << "\n" << "Table count: " << header.table_count << "\n" << header.file_offset << std::endl;
    
    int32_t *file_info_offsets = new int[header.table_count];
    int32_t *file_info_numfile = new int[header.table_count];
    for(int i = 0; i < header.table_count; i++) {
        fh.read((char*)&table_info, sizeof(drs_table_info));
        file_info_offsets[i] = table_info.file_info_offset;
        file_info_numfile[i] = table_info.num_files;
        std::cout << "- Table information: " << std::endl;
        std::cout   << "file_extension: " << table_info.file_extension << "\n"
                    << "file_info_offset: " << table_info.file_info_offset << "\n"
                    << "num_files: " << table_info.num_files << std::endl;
    }

    for(int j = 0; j < header.table_count; j++) {
        // drs_file_info tables start at position drs_table_info->file_info_offset for the corresponding table
        fh.seekg(file_info_offsets[j]);
        // for(int k = 0; k < table_info.num_files; k++){
        for(int k = 0; k < 1; k++){
            std::cout << "- File information: " << std::endl;
            fh.read((char*)&file_info, sizeof(drs_file_info));
            std::cout   << "file_id: " << file_info.file_id << "\n"
                        << "file_data_offset: " << file_info.file_data_offset << "\n"
                        << "file_size: " << file_info.file_size << std::endl;
        }

        // TODO: This is for demostrate first item only
        fh.seekg(file_info.file_data_offset);
        std::cout << "-- SLP file header: " << std::endl;
        fh.read((char*)&slp_file_header, sizeof(slp_file_header));
        std::cout   << "version: " << slp_file_header.version << "\n"
                    << "num_frames: " << slp_file_header.num_frames << "\n"
                    << "comment: " << slp_file_header.comment << std::endl;

        fh.read((char*)&slp_file_frame_info, sizeof(slp_frame_info));
        std::cout   << "cmd_table_offset: " << slp_file_frame_info.cmd_table_offset << "\n"
                    << "outline_table_offset: " << slp_file_frame_info.outline_table_offset << "\n"
                    << "palette_offset: " << slp_file_frame_info.palette_offset << "\n"
                    << "properties: " << slp_file_frame_info.properties << "\n"
                    << "width: " << slp_file_frame_info.width << "\n"
                    << "height: " << slp_file_frame_info.height << "\n"
                    << "hotspot_x: " << slp_file_frame_info.hotspot_x << "\n"
                    << "hotspot_y: " << slp_file_frame_info.hotspot_y << "\n" << std::endl;

        for(int l = 0; l < slp_file_frame_info.height; l++){
            fh.read((char*)&slp_file_frame_row_edge, sizeof(slp_frame_row_edge));
            if((slp_file_frame_row_edge.left_space == 0x8000) || slp_file_frame_row_edge.right_space == 0x8000) {
                for(int m = 0; m < slp_file_frame_info.width; m++){
                    std::cout << "-";
                }
            } else {
                for(int ls = 0; ls < slp_file_frame_row_edge.left_space; ls++){
                    std::cout << " ";
                }
                int center = slp_file_frame_info.width - slp_file_frame_row_edge.left_space - slp_file_frame_row_edge.right_space;
                for(int ct = 0; ct < center; ct++){
                    std::cout << "*";
                }
                for(int lr = 0; lr < slp_file_frame_row_edge.right_space; lr++){
                    std::cout << " ";
                }
                std::cout << "\n";
                // std::cout   << "left_space: " << slp_file_frame_row_edge.left_space << "\n"
                //             << "right_space: " << slp_file_frame_row_edge.right_space << std::endl;
            }
        }

        int32_t *drawing_command_offsets = new int[slp_file_frame_info.height];
        fh.seekg(file_info.file_data_offset + slp_file_frame_info.outline_table_offset + slp_file_frame_info.height * 4);
        for(int l = 0; l < slp_file_frame_info.height; l++){
            fh.read((char*)&slp_file_command_offset , sizeof(slp_command_offset ));
            drawing_command_offsets[l] = slp_file_command_offset.offset;
            // std::cout   << "offset: " << slp_file_command_offset.offset << "\n";
        }
        char command;
        for(int l = 0; l < slp_file_frame_info.height; l++){
            std::cout << "- ";
            // fh.seekg(drawing_command_offsets[l]);
            do {
                fh.read((char*)&command , sizeof(command ));
                printf(" 0x%x", command & 0x0F);
            } while (command != 0x0F);

            std::cout << std::endl;
        }

        delete [] drawing_command_offsets;
    }

    delete [] file_info_offsets;
    delete [] file_info_numfile;
    fh.close();
}