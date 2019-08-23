#include <sstream>
#include "Assets.h"

/*
 * http://artho.com/age/drs.html
 * https://github.com/SFTtech/openage/blob/master/doc/media/slp-files.md
*/

#define TRANSPARENT {0, 0, 0, 0}
#define PALLETE_FILE 50500

using namespace std;

// TODO:
// - Frame cache
// - Load all terrain sprites

Assets::Assets(/* args */)
{
    _load_palette(string("../Assets/Origin/Interfac.drs"));
    _load_terrain(string("../Assets/Origin/Terrain.drs"));
    _load_texture(string("../Assets/Origin/graphics.drs"));
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
                << "File offset: " << header->file_offset << endl;
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

void Assets::_read_bin_files_list(fstream &fh, list<DRSTableInfo>::iterator it) {
    DRSFileInfo drs_file_info = {0};
    fh.seekg(it->file_info_offset);
    // for(int k = 0; k < table_info.num_files; k++){
    for(int k = 0; k < it->num_files; ++k){
        fh.read((char*)&drs_file_info, sizeof(DRSFileInfo));
        m_file_info_map_bin.insert(make_pair(drs_file_info.file_id, drs_file_info.file_data_offset));

        // TODO: use logging library
        // cout    << "- File information: file id: " << drs_file_info.file_id
        //         << " at " << drs_file_info.file_data_offset << " size " << drs_file_info.file_size << endl;
    }
}

AssetFrames Assets::_read_slp_frames_by_id(fstream &fh, const int32_t id, map<int32_t, int32_t> slp_map) {
    AssetFrames frames;
    SLPHeader slp_file_header;
    SLPFrameInfo slp_file_frame_info;
    SLPFrameRowEdge slp_file_frame_row_edge;
    SLPCommandOffset slp_file_command_offset;

    auto it = slp_map.find(id);

    if(it != slp_map.end()) {
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

            fh_next_frame_pos = fh.tellg();
            // Draw single frame
            AssetFrame frame;

            frame.width = slp_file_frame_info.width;
            frame.height = slp_file_frame_info.height;
            frame.hotspot_x = slp_file_frame_info.hotspot_x;
            frame.hotspot_y = slp_file_frame_info.hotspot_y;

            // Read and save outline offset into vector
            fh.seekg(it->second + slp_file_frame_info.outline_table_offset);
            vector<SLPFrameRowEdge> rows_edge;
            for(uint32_t i = 0; i < frame.height; ++i) {
                fh.read((char*)&slp_file_frame_row_edge, sizeof(SLPFrameRowEdge));
                rows_edge.push_back({slp_file_frame_row_edge.left_space, slp_file_frame_row_edge.right_space});
            }

            fh.seekg(it->second + slp_file_frame_info.cmd_table_offset);
            vector<uint32_t> command_offsets;
            for(uint32_t i = 0; i < frame.height; ++i) {
                fh.read((char*)&slp_file_command_offset, sizeof(SLPCommandOffset));
                command_offsets.push_back(it->second + slp_file_command_offset.offset);
                // cout << "Command offset: " << it->second + slp_file_command_offset.offset << std::endl;
            }

            uint32_t counter = 0;
            for(uint32_t i = 0; i < frame.height; ++i) {
                uint8_t command;
                uint8_t command_low_nibble;
                uint8_t command_high_nibble;
                uint32_t pixel_count;
                uint32_t pixel_drawed_count = 0;
                bool eol = false;

                fh.seekg(command_offsets[i]);
                // cout << "Command offset: " << command_offsets[i] << std::endl;
                for(int32_t j = 0; j < rows_edge[i].left_space; j++) {
                    frame.pixels.push_back(TRANSPARENT);
                    pixel_drawed_count++;
                }
                while(!eol) {
                    fh.read((char*)&command , sizeof(command ));
                    command_high_nibble = 0xF0 & command;
                    command_low_nibble = 0x0F & command;

                    switch (command_low_nibble) {
                    case 0x0F:
                        eol = true;
                        break;
                    case 0b0000:
                    case 0b0100:
                    case 0b1000:
                    case 0b1100:
                        // Lesser block copy
                        pixel_count = static_cast<uint32_t>(command >> 2);
                        // cout << "Lesser block copy by " << (int)pixel_count << std::endl;
                        for(uint32_t j = 0; j < pixel_count; j ++) {
                            fh.read((char*)&command , sizeof(command));
                            frame.pixels.push_back(m_palletes.at(command));
                            pixel_drawed_count++;
                        }
                        break;
                    case 0b0001:
                    case 0b0101:
                    case 0b1001:
                    case 0b1101:
                        // Lesser skip
                        pixel_count = static_cast<uint32_t>(command >> 2);
                        if (pixel_count == 0) {
                            fh.read((char*)&command , sizeof(command));
                            pixel_count = static_cast<uint32_t>(command);
                        }
                        // cout << "Lesser skip by " << (int)pixel_count << std::endl;
                        for(uint32_t j = 0; j < pixel_count; j ++) {
                            frame.pixels.push_back(TRANSPARENT);
                            pixel_drawed_count++;
                        }
                        break;
                    case 0b0010:
                        // Greater block copy
                        fh.read((char*)&command , sizeof(command));
                        pixel_count = static_cast<uint32_t>(command);

                        // cout << "Greater block copy by " << (int)pixel_count << std::endl;

                        for(uint32_t j = 0; j < pixel_count; j ++) {
                            fh.read((char*)&command , sizeof(command));
                            frame.pixels.push_back(m_palletes.at(command));
                            pixel_drawed_count++;
                        }
                        break;
                    case 0b0011:
                        // Greater skip
                        fh.read((char*)&command , sizeof(command));
                        pixel_count = static_cast<uint32_t>(command);

                        // cout << "Greater skip by " << pixel_count << std::endl;

                        for(uint32_t j = 0; j < pixel_count; j ++) {
                            frame.pixels.push_back(TRANSPARENT);
                            pixel_drawed_count++;
                        }
                        break;
                    case 0b0110:
                        // Player color block copy
                        pixel_count = command_high_nibble >> 4;
                        if (pixel_count == 0) {
                            fh.read((char*)&command , sizeof(command));
                            pixel_count = command;
                        }

                        // cout << "Player color block copy by " << (int)pixel_count << std::endl;

                        for(uint32_t j = 0; j < pixel_count; j ++) {
                            fh.read((char*)&command , sizeof(command));
                            frame.pixels.push_back({0, 0, 255, 255});
                            pixel_drawed_count++;
                        }
                        break;
                    case 0b0111:
                        // Fill
                        pixel_count = command_high_nibble >> 4;
                        if (pixel_count == 0) {
                            fh.read((char*)&command , sizeof(command));
                            pixel_count = command;
                        }

                        // cout << "Fill color by " << (int)pixel_count << std::endl;

                        fh.read((char*)&command , sizeof(command));
                        for(uint32_t j = 0; j < pixel_count; j ++) {
                            // cout << (int)command << std::endl;
                            frame.pixels.push_back(m_palletes.at(command));
                            pixel_drawed_count++;
                        }
                        break;
                    case 0b1010:
                        // Fill player color
                        pixel_count = command_high_nibble >> 4;
                        if (pixel_count == 0) {
                            fh.read((char*)&command , sizeof(command));
                            pixel_count = command;
                        }

                        // cout << "Fill player color by " << (int)pixel_count << std::endl;

                        fh.read((char*)&command , sizeof(command));
                        for(uint32_t j = 0; j < pixel_count; j ++) {
                            frame.pixels.push_back({20, 10, 255, 255});
                            pixel_drawed_count++;
                        }
                        break;
                    case 0b1011:
                        pixel_count = command_high_nibble >> 4;
                        if (pixel_count == 0) {
                            fh.read((char*)&command , sizeof(command));
                            pixel_count = command;
                        }

                        // cout << "Shadows by " << (int)pixel_count << std::endl;

                        for(uint32_t j = 0; j < pixel_count; j ++) {
                            frame.pixels.push_back({0,0,0,100});
                            pixel_drawed_count++;
                        }
                        break;
                    case 0b1110:
                        if (command_high_nibble == 0x00) {
                        } else if (command_high_nibble == 0x40) {
                            frame.pixels.push_back({0, 255, 0, 255});
                            pixel_drawed_count++;
                        } else if (command_high_nibble == 0x50) {
                            fh.read((char*)&command , sizeof(command));
                            pixel_count = command;
                            frame.pixels.push_back({0, 255, 0, 255});
                            pixel_drawed_count++;
                        } else if (command_high_nibble == 0x60) {
                            frame.pixels.push_back({0, 255,  0,255});
                            pixel_drawed_count++;
                        } else if (command_high_nibble == 0x70) {
                            fh.read((char*)&command , sizeof(command));
                            pixel_count = command;
                            frame.pixels.push_back({0, 255, 0, 255});
                            pixel_drawed_count++;
                        } else {
                            cout << "Unsupported extended commands " << (int)command_high_nibble << std::endl;
                        }
                        break;
                    default:
                        cout << "Unknown commands" << std::endl;
                        break;
                    }
                }
                counter ++;
                // cout << "EOL " << counter << " w " << frame.width << " drawed " << (int)pixel_drawed_count << " Add " << frame.width - pixel_drawed_count << std::endl;
                for(uint32_t j = pixel_drawed_count; j < frame.width; j++) {
                    frame.pixels.push_back(TRANSPARENT);
                }
            }
            frames.frame_width = _max_width;
            frames.frame_height = _max_heigh;
            frames.frames.push_back(frame);
        }
    } else {
        cout << "Item not found, could not read frames with id " << id << endl;
        frames.num_frame = 1;
        frames.frame_width = 10;
        frames.frame_height = 10;
        AssetFrame frame;
        frame.width = 10;
        frame.height = 10;
        frame.hotspot_x = 5;
        frame.hotspot_y = 5;
        for (uint32_t i = 0; i < 100; i++) {
            frame.pixels.push_back({0, 0, 0, 255});
        }
        frames.frames.push_back(frame);
    }
    return frames;
}

AssetFrames Assets::get_sprite_by_id(int32_t id) {
    fstream fh;
    AssetFrames frame;
    fh.open("../Assets/Origin/graphics.drs", fstream::in | fstream::binary);
    frame = this->_read_slp_frames_by_id(fh, id, m_file_info_map_slp_sprite);
    fh.close();
    return frame;
}

AssetFrames Assets::get_terrain_by_id(int32_t id) {
    fstream fh;
    AssetFrames frame;
    fh.open("../Assets/Origin/terrain.drs", fstream::in | fstream::binary);
    frame = this->_read_slp_frames_by_id(fh, id, m_file_info_map_slp_terrain);
    fh.close();
    return frame;
}

void Assets::_load_texture(std::string const& path)
{
    DRSHeader header;
    list<DRSTableInfo> drs_tables_info;
    list<DRSTableInfo>::iterator drs_tables_info_it;

    cout << "-----------------------------------------------------" << std::endl;
    cout << "Loading texture: " << path << std::endl;

    fstream fh;

    fh.open(path, fstream::in | fstream::binary);
    // fh.open("../Assets/Origin/Interfac.drs", fstream::in | fstream::binary);
    _read_drs_header(fh, &header);
    _read_drs_table_info(fh, &drs_tables_info, header.table_count);

    // Read the tables one by one
    for(drs_tables_info_it = drs_tables_info.begin(); drs_tables_info_it != drs_tables_info.end(); ++drs_tables_info_it) {
        // drs_file_info tables start at position drs_table_info->file_info_offset for the corresponding table
        if(strncmp(drs_tables_info_it->file_extension, " pls", 4) == 0) {
            DRSFileInfo drs_file_info = {0};
            fh.seekg(drs_tables_info_it->file_info_offset);
            for(int k = 0; k < drs_tables_info_it->num_files; ++k){
                fh.read((char*)&drs_file_info, sizeof(DRSFileInfo));
                m_file_info_map_slp_sprite.insert(make_pair(drs_file_info.file_id, drs_file_info.file_data_offset));
            }
        }
    }
    fh.close();
}

void Assets::_load_terrain(std::string const& path) {
    fstream terrain_fh;
    DRSHeader header = {0};
    list<DRSTableInfo> drs_tables_info;
    list<DRSTableInfo>::iterator drs_tables_info_it;

    cout << "-----------------------------------------------------" << std::endl;
    cout << "Loading terrain: " << path << std::endl;

    /*
    15000: Desert
    15001: Grass
    15002: Sea
    15003: Deep sea
    */

    terrain_fh.open(path, fstream::in | fstream::binary);
    _read_drs_header(terrain_fh, &header);
    _read_drs_table_info(terrain_fh, &drs_tables_info, header.table_count);

    // Read the tables one by one
    for(drs_tables_info_it = drs_tables_info.begin(); drs_tables_info_it != drs_tables_info.end(); ++drs_tables_info_it) {
        // drs_file_info tables start at position drs_table_info->file_info_offset for the corresponding table
        // cout << drs_tables_info_it->file_extension << std::endl;
        if(strncmp(drs_tables_info_it->file_extension, " pls", 4) == 0) {
            DRSFileInfo drs_file_info = {0};
            terrain_fh.seekg(drs_tables_info_it->file_info_offset);
            for(int k = 0; k < drs_tables_info_it->num_files; ++k){
                terrain_fh.read((char*)&drs_file_info, sizeof(DRSFileInfo));
                m_file_info_map_slp_terrain.insert(make_pair(drs_file_info.file_id, drs_file_info.file_data_offset));
                cout << drs_file_info.file_id << std::endl;
            }
        }
    }
}

void Assets::_load_palette(std::string const& path) {
    fstream palette_fh;

    DRSHeader header = {0};
    list<DRSTableInfo> drs_tables_info;
    list<DRSTableInfo>::iterator drs_tables_info_it;

    cout << "-----------------------------------------------------" << std::endl;
    cout << "Loading pallete: " << path << std::endl;

    palette_fh.open(path, fstream::in | fstream::binary);
    _read_drs_header(palette_fh, &header);
    _read_drs_table_info(palette_fh, &drs_tables_info, header.table_count);

    // Read the tables one by one
    for(drs_tables_info_it = drs_tables_info.begin(); drs_tables_info_it != drs_tables_info.end(); ++drs_tables_info_it) {
        // drs_file_info tables start at position drs_table_info->file_info_offset for the corresponding table
        if(strncmp(drs_tables_info_it->file_extension, "anib", 4) == 0) {
            _read_bin_files_list(palette_fh, drs_tables_info_it);
        }
    }

    auto it = m_file_info_map_bin.find(PALLETE_FILE);
    if (it != m_file_info_map_bin.end()) {
        uint32_t palletes_count = 0;
        std::string jascPAL_text;
        palette_fh.seekg(it->second);
        std::getline(palette_fh, jascPAL_text);
        // cout << "File header " << jascPAL_text << std::endl;
        std::getline(palette_fh, jascPAL_text);
        // cout << "File version " << jascPAL_text << std::endl;
        std::getline(palette_fh, jascPAL_text);
        palletes_count = static_cast<uint32_t>(std::stoi(jascPAL_text));
        // cout << "File palletes count " << palletes_count << std::endl;

        for(uint32_t i = 0; i < palletes_count; i++) {
            std::getline(palette_fh, jascPAL_text);
            std::stringstream ss(jascPAL_text);
            std::string buf;
            RGBAPixel pixel;
            ss >> buf;
            pixel.r = static_cast<uint8_t>(std::stoi(buf));
            ss >> buf;
            pixel.g = static_cast<uint8_t>(std::stoi(buf));
            ss >> buf;
            pixel.b = static_cast<uint8_t>(std::stoi(buf));
            pixel.a = 255;
            m_palletes.push_back(pixel);
        }

    }
    palette_fh.close();
}
