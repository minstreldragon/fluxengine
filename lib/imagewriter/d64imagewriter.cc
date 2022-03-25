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

static int sectors_per_track(int track)
{
    if (track < 17)
        return 21;
    if (track < 24)
        return 19;
    if (track < 30)
        return 18;
    return 17;
}

class D64ImageWriter : public ImageWriter
{
public:
	D64ImageWriter(const ImageWriterProto& config):
		ImageWriter(config)
	{}

	void writeImage(const Image& image)
	{
		Logger() << "D64: writing triangular image";

		std::ofstream outputFile(_config.filename(), std::ios::out | std::ios::binary);
		if (!outputFile.is_open())
			Error() << "cannot open output file";

        bool any_error = false;
        int track_count = 0;
        uint32_t disk_size = 0;
        uint32_t offset = 0;
		for (int track = 0; track < 40; track++)
		{
            int sectorCount = sectors_per_track(track);
            for (int sectorId = 0; sectorId < sectorCount; sectorId++)
            {
                const auto& sector = image.get(track, 0, sectorId);
                if (sector)
                {
                    outputFile.seekp(offset);
                    outputFile.write((const char*) sector->data.cbegin(), 256);
                    track_count = track + 1;
                    disk_size = offset + 256;
                }
                else
                {
                    any_error = true;
                }

                offset += 256;
            }
		}

        if (any_error)
        {
            offset = disk_size;
            for (int track = 0; track < track_count; track++)
	    	{
                int sectorCount = sectors_per_track(track);
                {
                    for (int sectorId = 0; sectorId < sectorCount; sectorId++)
                    {
                        char code;
                        const auto& sector = image.get(track, 0, sectorId); 
                        if (!sector) 
                            code = 2; // header descriptor not found
                        else if (sector->status == Sector::HEADER_BAD_CHECKSUM)
                            code = 9; // checksum error in header 
                        else if (sector->status == Sector::MISSING)
                            code = 4; // data descriptor byte not found
                        else if (sector->status == Sector::BAD_CHECKSUM)
                            code = 5; // checksum error in data block
                        else
                            code = 1; // block OK
                        outputFile.seekp(offset);
                        outputFile.write(&code, 1);
                        offset += 1;
                    }
                }
		    }
        }
    }
};

std::unique_ptr<ImageWriter> ImageWriter::createD64ImageWriter(const ImageWriterProto& config)
{
    return std::unique_ptr<ImageWriter>(new D64ImageWriter(config));
}

