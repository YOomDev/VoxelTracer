#pragma once

#include <SDL.h>
#include <fstream>

static void save_surface_as_bmp(SDL_Surface* surface, const char* filename) {
    uint32_t* array = (uint32_t*) surface->pixels;
    uint32_t size = surface->w * surface->h;
    uint32_t datasize = size * 3;
    char* data = (char*) calloc(datasize, sizeof(char));
    std::ofstream ofs;
    ofs.open(filename, std::ios_base::out | std::ios_base::binary);
    if (!ofs.is_open()) {
        printf_s("ERROR: could not open file\n");
        return;
    }

    if (data == nullptr) { exit(-3); }

    // convert uint32_t to 3 uint8_t before writing to file
    for (int i = 0; i < size; i++) {
        data[i * 3] = (array[i] & surface->format->Rmask) >> surface->format->Rshift;
        data[i * 3 + 1] = (array[i] & surface->format->Gmask) >> surface->format->Gshift;
        data[i * 3 + 2] = (array[i] & surface->format->Bmask) >> surface->format->Bshift;
    }

    // ready up for file writing
    unsigned char bmpPad[3]{ 0 };
    const int paddingAmount = ((4 - (surface->w * 3) % 4) % 4);
    const int fileHeaderSize = 14;
    const int informationHeaderSize = 40;
    const int fileSize = fileHeaderSize + informationHeaderSize + datasize + paddingAmount * surface->h;

    // creating the file header
    unsigned char fileHeader[fileHeaderSize];

    // file type
    fileHeader[0] = 'B';
    fileHeader[1] = 'M';

    // file size
    fileHeader[2] = fileSize;
    fileHeader[3] = fileSize >> 8;
    fileHeader[4] = fileSize >> 16;
    fileHeader[5] = fileSize >> 24;

    // reserved
    fileHeader[6] = 0;
    fileHeader[7] = 0;
    fileHeader[8] = 0;
    fileHeader[9] = 0;

    // pixel data offset
    fileHeader[10] = fileHeaderSize + informationHeaderSize;
    fileHeader[11] = 0;
    fileHeader[12] = 0;
    fileHeader[13] = 0;

    // creating the information header
    unsigned char informationHeader[informationHeaderSize];

    // header size
    informationHeader[0] = informationHeaderSize;
    informationHeader[1] = 0;
    informationHeader[2] = 0;
    informationHeader[3] = 0;

    // image width
    informationHeader[4] = surface->w;
    informationHeader[5] = surface->w >> 8;
    informationHeader[6] = surface->w >> 16;
    informationHeader[7] = surface->w >> 24;

    // image height
    informationHeader[8] = surface->h;
    informationHeader[9] = surface->h >> 8;
    informationHeader[10] = surface->h >> 16;
    informationHeader[11] = surface->h >> 24;

    // planes
    informationHeader[12] = 1;
    informationHeader[13] = 0;

    // bits per pixel (RGB)
    informationHeader[14] = 24;
    informationHeader[15] = 0;

    // compression (no compression)
    informationHeader[16] = 0;
    informationHeader[17] = 0;
    informationHeader[18] = 0;
    informationHeader[19] = 0;

    // image size (no compression)
    informationHeader[20] = 0;
    informationHeader[21] = 0;
    informationHeader[22] = 0;
    informationHeader[23] = 0;

    // X pixels per meter (not specified)
    informationHeader[24] = 0;
    informationHeader[25] = 0;
    informationHeader[26] = 0;
    informationHeader[27] = 0;

    // X pixels per meter (not specified)
    informationHeader[28] = 0;
    informationHeader[29] = 0;
    informationHeader[30] = 0;
    informationHeader[31] = 0;

    // Total colors (color palette not used)
    informationHeader[32] = 0;
    informationHeader[33] = 0;
    informationHeader[34] = 0;
    informationHeader[35] = 0;

    // important colors (generally ignored)
    informationHeader[36] = 0;
    informationHeader[37] = 0;
    informationHeader[38] = 0;
    informationHeader[39] = 0;

    /// start writing file ///
    // write headers
    ofs.write(reinterpret_cast<char*>(fileHeader), fileHeaderSize);
    ofs.write(reinterpret_cast<char*>(informationHeader), informationHeaderSize);
    ofs.write(reinterpret_cast<char*>(data), datasize);

    // write image from buffer to file
    for (int y = 0; y < surface->w; y++) {
        int offset = y * surface->w * 3;
        for (int x = 0; x < surface->w; x++) {
            unsigned char c[3] = { data[offset + x * 3], data[offset + x * 3 + 1], data[offset + x * 3 + 2] };
            ofs.write(reinterpret_cast<char*>(c), 3);
        }
        ofs.write(reinterpret_cast<char*>(bmpPad), paddingAmount);
    }

    // close file
    ofs.close();
}