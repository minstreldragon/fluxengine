#include "globals.h"
#include "flags.h"
#include "sector.h"
#include "imagewriter/imagewriter.h"
#include "fmt/format.h"
#include "image.h"
#include "ldbs.h"
#include "logger.h"
#include "lib/config.pb.h"
#include <algorithm>
#include <iostream>
#include <fstream>


class AtrImageWriter : public ImageWriter
{
public:
    AtrImageWriter(const ImageWriterProto& config):
        ImageWriter(config)
    {}

    void writeImage(const Image& image)
    {
        Logger() << "ATR: writing ATARI 8 bit disk image";
        const char hdr[] = "\x96\x02\x80\x16\x80\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";
        char invsector[128];

        std::ofstream outputFile(_config.filename(), std::ios::out | std::ios::binary);
        if (!outputFile.is_open())
            Error() << "cannot open output file";

        // TODO: fill ATR header with correct data
        outputFile.write((const char*) hdr, 16);

        uint32_t offset = 16;
        for (int track = 0; track < 40; track++)
        {
            for (int sectorId = 0; sectorId < 18; sectorId++)
            {
                const auto& sector = image.get(track, 0, sectorId+1);
                if (sector)
                {
                    for (int i = 0; i < sector->data.size(); i++) {
                       invsector[i] = sector->data[i] ^ 0xff;
                    }

                    outputFile.seekp(offset);
                    outputFile.write((const char*) invsector, 128);
                }
                else {
                }

                offset += 128;
            }
        }
    }
};

std::unique_ptr<ImageWriter> ImageWriter::createAtrImageWriter(const ImageWriterProto& config)
{
    return std::unique_ptr<ImageWriter>(new AtrImageWriter(config));
}

