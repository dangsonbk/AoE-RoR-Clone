#include "Assets.h"

/*
 * http://artho.com/age/drs.html
 * https://github.com/SFTtech/openage/blob/master/doc/media/slp-files.md
*/
using namespace std;

// TODO:
// - Frame cache

Assets::Assets(/* args */)
{
    _load();
}

Assets::~Assets()
{}

void Assets::_read_drs_header(fstream &fh, DRSHeader *header) {
    fh.read((char*)header, sizeof(drs_header));

    // TODO: use loging library
    cout   << header->copyright << "\n"
                << "Version: " << header->version << "\n"
                << "File type: " << header->ftype << "\n"
                << "Table count: " << header->table_count << "\n"
                << "File offset: " << header->file_offset << "\n"
                << "-----------------------------------------------------" << endl;
}

void Assets::_read_drs_table_info(fstream &fh, list<DRSTableInfo> *tables_info, int table_count) {
    DRSTableInfo table_info;
    for(int i = 0; i < table_count; i++) {
        fh.read((char*)&table_info, sizeof(DRSTableInfo));
        tables_info->push_back(table_info);

        // TODO: use logging library
        // TODO: remove on production, this is for debug only
        char _file_ext[5];
        strncpy(_file_ext, table_info.file_extension, 4);
        _file_ext[4] = '\0';
        cout    << "- Table information: " << table_info.num_files << " " << _file_ext
                << " file(s) at offset " << table_info.file_info_offset << endl;
    }
}

/*
    SLP file structure in ASCII form by https://github.com/ChariotEngine/Slp
    +-----------------------------+
    |          SlpHeader          |
    +-----------------------------+
    |SlpShapeHeader|SlpShapeHeader|
    +-----------------------------+
    |                             |
    | Array of u16 padding pairs  | <-+ Each SlpShapeHeader has a "shape_outline_offset"
    |                             |     that points to a pair in this array
    +-----------------------------+
    |                             |
    | Arrays of u32 offsets to    | <-+ Each SlpShapeHeader has a "shape_data_offsets"
    |  first command in each row  |     that points to an array
    |                             |
    +-----------------------------+
    |                             |
    | Drawing commands used to    |
    |  produce indexed image data |
    |                             |
    +-----------------------------+
*/

void Assets::_read_slp_files_list(fstream &fh, list<DRSTableInfo>::iterator it) {
    DRSFileInfo drs_file_info = {0};
    fh.seekg(it->file_info_offset);
    // for(int k = 0; k < table_info.num_files; k++){
    for(int k = 0; k < it->num_files; ++k){
        fh.read((char*)&drs_file_info, sizeof(DRSFileInfo));
        m_file_info_map_slp.insert(make_pair(drs_file_info.file_id, drs_file_info.file_data_offset));

        // TODO: use logging library
        // cout    << "- File information: file id: " << drs_file_info.file_id
        //         << " at " << drs_file_info.file_data_offset << endl;
    }
}

AssetFrames Assets::_read_slp_frames_by_id(fstream &fh, int32_t id) {
    AssetFrames frames;
    SLPHeader slp_file_header;
    SLPFrameInfo slp_file_frame_info;
    SLPFrameRowEdge slp_file_frame_row_edge;
    SLPCommandOffset slp_file_command_offset;

    auto it = m_file_info_map_slp.find(id);

    if(it != m_file_info_map_slp.end()){
        fh.seekg(it->second);
        fh.read((char*)&slp_file_header, sizeof(SLPHeader));

        cout << "-- SLP file header: " << slp_file_header.num_frames << " frames of " << slp_file_header.comment << endl;

        int32_t _max_width = 0;
        int32_t _max_heigh = 0;

        frames.num_frame = slp_file_header.num_frames;
        streampos fh_next_frame_pos = fh.tellg();
        for(int i = 0; i < slp_file_header.num_frames; ++i) {
            fh.seekg(fh_next_frame_pos);
            fh.read((char*)&slp_file_frame_info, sizeof(SLPFrameInfo));
            _max_width = max(_max_width, slp_file_frame_info.width);
            _max_heigh = max(_max_heigh, slp_file_frame_info.height);
            // cout    << "cmd_table_offset: " << slp_file_frame_info.cmd_table_offset << "\n"
            //         << "outline_table_offset: " << slp_file_frame_info.outline_table_offset << "\n"
            //         << "palette_offset: " << slp_file_frame_info.palette_offset << "\n"
            //         << "properties: " << slp_file_frame_info.properties << "\n"
            //         << "width: " << slp_file_frame_info.width << "\n"
            //         << "height: " << slp_file_frame_info.height << "\n"
            //         << "hotspot_x: " << slp_file_frame_info.hotspot_x << "\n"
            //         << "hotspot_y: " << slp_file_frame_info.hotspot_y << "\n" << endl;
            fh_next_frame_pos = fh.tellg();
            // Draw single frame
            AssetFrame frame;

            frame.width = slp_file_frame_info.width;
            frame.height = slp_file_frame_info.height;
            frame.hotspot_x = slp_file_frame_info.hotspot_x;
            frame.hotspot_y = slp_file_frame_info.hotspot_y;

            fh.seekg(it->second + slp_file_frame_info.outline_table_offset);
            for(int32_t i = 0; i < slp_file_frame_info.height; ++i) {
                fh.read((char*)&slp_file_frame_row_edge, sizeof(SLPFrameRowEdge));
                for(int32_t j = 0; j < slp_file_frame_info.width; ++j) {
                    if((j<slp_file_frame_row_edge.left_space) || (j>slp_file_frame_info.width-slp_file_frame_row_edge.right_space)) {
                        frame.pixels.push_back({0,0,0,0});
                    } else {
                        frame.pixels.push_back({255,255,255,255});
                    }
                }
            }
            frames.frame_width = _max_width;
            frames.frame_height = _max_heigh;
            frames.frames.push_back(frame);
        }
    } else {
        cout << "Item not found, could not read frames info" << endl;
    }
    return frames;
}

AssetFrames Assets::get_by_id(int32_t id) {
    fstream fh;
    fh.open("../Assets/Origin/graphics.drs", fstream::in | fstream::binary);
    cout << "Opening file ..." << endl;
    return this->_read_slp_frames_by_id(fh, id);
}

void Assets::_load()
{
    DRSHeader header = {0};
    list<DRSTableInfo> drs_tables_info;
    list<DRSTableInfo>::iterator drs_tables_info_it;

    fstream fh;

    fh.open("../Assets/Origin/graphics.drs", fstream::in | fstream::binary);
    // fh.open("../Assets/Origin/Interfac.drs", fstream::in | fstream::binary);
    _read_drs_header(fh, &header);
    _read_drs_table_info(fh, &drs_tables_info, header.table_count);

    // Read the tables one by one
    for(drs_tables_info_it = drs_tables_info.begin(); drs_tables_info_it != drs_tables_info.end(); ++drs_tables_info_it) {
        // drs_file_info tables start at position drs_table_info->file_info_offset for the corresponding table
        if(strncmp(drs_tables_info_it->file_extension, " pls", 4) == 0) {
            _read_slp_files_list(fh, drs_tables_info_it);
        } else {
            cout << "File not support" << endl;
        }
    }

            // int32_t *drawing_command_offsets = new int[slp_file_frame_info.height];
            // fh.seekg(file_info.file_data_offset + slp_file_frame_info.outline_table_offset + slp_file_frame_info.height * 4);
            // for(int l = 0; l < slp_file_frame_info.height; l++){
            //     fh.read((char*)&slp_file_command_offset , sizeof(slp_command_offset ));
            //     drawing_command_offsets[l] = slp_file_command_offset.offset;
            //     // cout   << "offset: " << slp_file_command_offset.offset << "\n";
            // }
            // char command;
            // for(int l = 0; l < slp_file_frame_info.height; l++){
            //     cout << "- ";
            //     // fh.seekg(drawing_command_offsets[l]);
            //     do {
            //         fh.read((char*)&command , sizeof(command ));
            //         printf(" 0x%x", command & 0x0F);
            //     } while (command != 0x0F);

            //     cout << endl;
            // }
        // delete [] drawing_command_offsets;

    fh.close();
}
