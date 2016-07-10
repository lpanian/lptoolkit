#include "toolkit/imageutils.hh"

#include "toolkit/memstream.hh"
#include "toolkit/image.hh"
#include "toolkit/mathcommon.hh"
#include "toolkit/filehelper.hh"
#include "toolkit/endian.hh"

namespace lptk
{
    ////////////////////////////////////////////////////////////////////////////////
    // TGA loading
	struct TGAHeader
	{
		uint8_t m_idLength;
		uint8_t m_colorMapType;
		uint8_t m_imageType;

		uint16_t m_colorMapOffset;
		uint16_t m_colorMapCount;
		uint8_t m_colorMapBpp;

		uint16_t m_xOrigin;
		uint16_t m_yOrigin;
		uint16_t m_width;
		uint16_t m_height;
		uint8_t m_bpp;
		uint8_t m_desc;
	};
	static constexpr auto kTGAHeaderSize = 18u;

	enum TGAImageTypeFlags 
	{
		TGA_IMAGE_TYPE_TRUE_COLOR = 0x02,
		TGA_IMAGE_TYPE_RLE = 0x08,
	};

	enum TGAImageDescFlags
	{
		TGA_IMAGE_DESC_TOP = 0x20,
		TGA_IMAGE_DESC_RIGHT = 0x10
	};

	struct TGAFooter
	{
		uint32_t m_extensionOffset;
		uint32_t m_devOffset;
		uint8_t m_signature[18];
	};
	static constexpr auto kTGAFooterSize = 26u;

    bool LoadTGA(Image& result, const char* filename)
    {
        const auto fileData = lptk::ReadFile<lptk::DynAry<char>>(filename);
        if (fileData.empty())
            return false;

        return LoadTGA(result, fileData.data(), fileData.size());
    }

    bool LoadTGA(Image& result, const char* memory, size_t n)
    {
        TGAHeader header;
        memset(&header, 0, sizeof(header));
        {
            MemReader reader((const char*)memory, n,
                MemFormatFlag::FLAG_LittleEndianData);
            reader.Get(&header.m_idLength, sizeof(uint8_t));
            reader.Get(&header.m_colorMapType, sizeof(uint8_t));
            reader.Get(&header.m_imageType, sizeof(uint8_t));
            reader.Get(&header.m_colorMapOffset, sizeof(uint16_t));
            reader.Get(&header.m_colorMapCount, sizeof(uint16_t));
            reader.Get(&header.m_colorMapBpp, sizeof(uint8_t));
            reader.Get(&header.m_xOrigin, sizeof(uint16_t));
            reader.Get(&header.m_yOrigin, sizeof(uint16_t));
            reader.Get(&header.m_width, sizeof(uint16_t));
            reader.Get(&header.m_height, sizeof(uint16_t));
            reader.Get(&header.m_bpp, sizeof(uint8_t));
            reader.Get(&header.m_desc, sizeof(uint8_t));
        }

        if (header.m_colorMapType != 0 ||
            (header.m_imageType & 0x7) != TGA_IMAGE_TYPE_TRUE_COLOR)
        {
            fprintf(stderr, "LoadTGA: only true type images are supported.\n");
            return false;
        }

        if (header.m_bpp != 24 && header.m_bpp != 32)
        {
            fprintf(stderr, "LoadTGA: only 24-bit and 32-bit TGAs are supported.\n");
            return false;
        }

        const int bytesPerValue = header.m_bpp / 8;
        result.Init(header.m_width, header.m_height, bytesPerValue);
        {
            MemReader reader((const char*)memory, n, 0);
            reader.Consume(kTGAHeaderSize);
            reader.Consume(header.m_idLength);

            const int numPixels = header.m_width * header.m_height;

            int incWidth = 1;
            int incHeight = 1;
            unsigned yPos = 0;
            unsigned startXpos = 0;
            if ((header.m_desc & TGA_IMAGE_DESC_TOP) == 0)
            {
                incHeight = -incHeight;
                yPos = header.m_height - 1;
            }

            if ((header.m_desc & TGA_IMAGE_DESC_RIGHT) != 0)
            {
                incWidth = -1;
                startXpos = header.m_width - 1;
            }

            if ((header.m_imageType & TGA_IMAGE_TYPE_RLE) != 0)
            {
                int32_t countPixels = 0;
                for (; yPos >= 0 && yPos < header.m_height; yPos += incHeight)
                {
                    auto xPos = startXpos;

                    int32_t countScanline = 0;
                    while (countScanline < (int32_t)header.m_width)
                    {
                        int8_t repCount;
                        reader.Get(&repCount, sizeof(int8_t));
                        int32_t count = (repCount & 0x7F) + 1;
                        countScanline += count;
                        if ((0x80 & repCount) != 0)
                        {
                            // RLE packet
                            // read count + 1 pixels
                            uint8_t r, g, b;
                            uint8_t a = 0xff;
                            reader.Get(&b, 1, false);
                            reader.Get(&g, 1, false);
                            reader.Get(&r, 1, false);
                            if (bytesPerValue == 4) reader.Get(&a, 1, false);
                            const auto fr = r / float(0xff);
                            const auto fg = g / float(0xff);
                            const auto fb = b / float(0xff);
                            const auto fa = a / float(0xff);

                            while (count-- > 0)
                            {
                                result.Set(xPos, yPos, 0, fr);
                                result.Set(xPos, yPos, 1, fg);
                                result.Set(xPos, yPos, 2, fb);
                                if (bytesPerValue == 4)
                                {
                                    result.Set(xPos, yPos, 3, fa);
                                }
                                xPos += incWidth;
                            }
                        }
                        else
                        {
                            // Raw packet
                            // repeat the pixel count times + 1
                            while (count-- > 0)
                            {
                                uint8_t r, g, b;
                                reader.Get(&b, 1, false);
                                reader.Get(&g, 1, false);
                                reader.Get(&r, 1, false);
                                const auto fr = r / float(0xff);
                                const auto fg = g / float(0xff);
                                const auto fb = b / float(0xff);
                                
                                result.Set(xPos, yPos, 0, fr);
                                result.Set(xPos, yPos, 1, fg);
                                result.Set(xPos, yPos, 2, fb);
                                if (bytesPerValue == 4)
                                {
                                    uint8_t a;
                                    reader.Get(&a, 1, false);
                                    const auto fa = a / float(0xff);
                                    result.Set(xPos, yPos, 3, fa);
                                }
                                xPos += incWidth;
                            }
                        }
                    }
                    ASSERT(countScanline == (int32_t)header.m_width);
                    countPixels += countScanline;
                }
                ASSERT(countPixels == numPixels);
            }
            else // NOT run-length encoded.
            {
                for (; yPos >= 0 && yPos < header.m_height; yPos += incHeight)
                {
                    for (auto xPos = startXpos; xPos >= 0 && xPos < header.m_width; xPos += incWidth)
                    {
                        uint32_t pixelValue = 0xFF000000;
                        reader.Get(&pixelValue, bytesPerValue, false);
                        const uint8_t r = (pixelValue >> 16) & 0xFF;
                        const uint8_t g = (pixelValue >> 8) & 0xFF;
                        const uint8_t b = (pixelValue)& 0xFF;
                        const uint8_t a = (pixelValue >> 24) & 0xFF;
                        const auto fr = r / float(0xff);
                        const auto fg = g / float(0xff);
                        const auto fb = b / float(0xff);
                        result.Set(xPos, yPos, 0, fr);
                        result.Set(xPos, yPos, 1, fg);
                        result.Set(xPos, yPos, 2, fb);
                        if (bytesPerValue == 4)
                        {
                            const auto fa = a / float(0xff);
                            result.Set(xPos, yPos, 3, fa);
                        }
                    }
                }
            }
        }

#if 0
        TGAFooter footer;
        bool validFooter = false;
        memset(&footer, 0, sizeof(footer));
        if (size >= kTGAFooterSize)
        {
            BufferReader reader((const char*)memory + size - kTGAFooterSize, kTGAFooterSize);
            reader.Get(&footer.m_extensionOffset, sizeof(uint32_t));
            reader.Get(&footer.m_devOffset, sizeof(uint32_t));
            reader.Get(footer.m_signature, ARRAY_SIZE(footer.m_signature), false);

            if (strcmp("TRUEVISION-XFILE.", (char*)(&footer.m_signature[0])) == 0 &&
                footer.m_signature[17] == '\0')
                validFooter = true;
        }
#endif 
        return true;
    }
    
    bool SaveTGA(const Image& image, const char* filename)
    {
        lptk::DynAry<char> mem;
        if (!SaveTGA(image, mem))
            return false;

        return lptk::WriteFile(filename, mem);
    }

    bool SaveTGA(const Image& srcImage, lptk::DynAry<char> mem)
    {
        TGAHeader header;
        bzero(&header, sizeof(header));

        header.m_imageType = TGA_IMAGE_TYPE_TRUE_COLOR;
        header.m_width = static_cast<uint16_t>(srcImage.GetWidth());
        header.m_height = static_cast<uint16_t>(srcImage.GetHeight());
        header.m_bpp = 32;
        header.m_desc = 0;

        mem.resize(kTGAHeaderSize + header.m_width * header.m_height * sizeof(uint32_t));

        MemWriter writer(reinterpret_cast<char*>(&mem[0]), mem.size());
        writer.Put(&header.m_idLength, sizeof(header.m_idLength), true);
        writer.Put(&header.m_colorMapType, sizeof(header.m_colorMapType), true);
        writer.Put(&header.m_imageType, sizeof(header.m_imageType), true);
        writer.Put(&header.m_colorMapOffset, sizeof(header.m_colorMapOffset), true);
        writer.Put(&header.m_colorMapCount, sizeof(header.m_colorMapCount), true);
        writer.Put(&header.m_colorMapBpp, sizeof(header.m_colorMapBpp), true);
        writer.Put(&header.m_xOrigin, sizeof(header.m_xOrigin), true);
        writer.Put(&header.m_yOrigin, sizeof(header.m_yOrigin), true);
        writer.Put(&header.m_width, sizeof(header.m_width), true);
        writer.Put(&header.m_height, sizeof(header.m_height), true);
        writer.Put(&header.m_bpp, sizeof(header.m_bpp), true);
        writer.Put(&header.m_desc, sizeof(header.m_desc), true);

        for (int y = header.m_height - 1; y >= 0; --y)
        {
            for (int x = 0; x < header.m_width; ++x)
            {
                const auto fr = srcImage.Get(x, y, 0);
                const auto fg = srcImage.Get(x, y, 1);
                const auto fb = srcImage.Get(x, y, 2);
                const auto fa = srcImage.GetNumChannels() > 3 ?
                    srcImage.Get(x, y, 3) : 1.f;
                const uint8_t r = static_cast<uint8_t>(Clamp(int(fr * 255), 0, 255));
                const uint8_t g = static_cast<uint8_t>(Clamp(int(fg * 255), 0, 255));
                const uint8_t b = static_cast<uint8_t>(Clamp(int(fb * 255), 0, 255));
                const uint8_t a = static_cast<uint8_t>(Clamp(int(fa * 255), 0, 255));
                const uint32_t data = (r << 16) | (g << 8) | b | (a << 24);
                writer.Put(&data, sizeof(data), true);
            }
        }
        return true;
    }

    //////////////////////////////////////////////////////////////////////////////// 
    namespace tiff
    {
        enum CompressionType
        {
            NoCompression = 1,
            ModifiedHuffman = 2,
            PackBits = 32773,
        };

        enum ValueType
        {
            Byte = 1,
            Ascii = 2,
            Short = 3,
            Long = 4,
            Rational = 5,
            SByte = 6,
            Undefined = 7,
            SShort = 8,
            SLong = 9,
            SRational = 10,
            Float = 11,
            Double = 12,
        };

        unsigned ByteSizeFromValueType(ValueType type)
        {
            switch (type)
            {
                // single byte values
            case tiff::ValueType::Ascii:
            case tiff::ValueType::Byte:
            case tiff::ValueType::SByte:
            case tiff::ValueType::Undefined:
                return 1;

                // two byte values
            case tiff::ValueType::Short:
            case tiff::ValueType::SShort:
                return 2;

                // 4 byte values
            case tiff::ValueType::Long:
            case tiff::ValueType::SLong:
            case tiff::ValueType::Float:
                return 4;

                // 8 byte values
            case tiff::ValueType::Rational:
            case tiff::ValueType::SRational:
            case tiff::ValueType::Double:
                return 8;
            default:
                return 0;
            }

        }

        struct ImageSettings
        {
            unsigned width = 0;
            unsigned height = 0;
            unsigned compression = 0;
            unsigned photometricInterpretation = 0;
            lptk::MemReader stripOffsetsReader;
            ValueType stripOffsetsType = ValueType::Short;
            unsigned stripsPerImage = 0;
            unsigned rowsPerStrip = 0;
            lptk::MemReader stripByteCountsReader;
            ValueType stripByteCountsType = ValueType::Short;
            lptk::MemReader bitsPerSampleReader;
            unsigned samplesPerPixel = 1;
            lptk::MemReader sampleFormatReader;

            lptk::MemReader colorMapReader;

            // we read these but don't use them
            unsigned resolutionUnit = 0;
            std::pair<unsigned, unsigned> xRes;
            std::pair<unsigned, unsigned> yRes;
        };


        enum TagType
        {
            ImageWidth = 256,
            ImageLength = 257,
            BitsPerSample = 258,
            Compression = 259,
            PhotometricInterpretation = 262,
            StripOffsets = 273,
            SamplePerPixel = 277,
            RowsPerStrip = 278,
            StripByteCounts = 279,
            XResolution = 282,
            YResolution = 283,
            ResolutionUnit = 296,
            ColorMap = 320,
            SampleFormat = 339,
        };

        struct IFDEntryReader
        {
            struct ReadInfo {
                ImageSettings* settings = nullptr;
                unsigned entryType = 0;
                unsigned entryCount = 0;
                const char* baseMem = nullptr;
                size_t baseSize = 0;
                lptk::MemReader reader;

            };
            using ReadFunc = void(*)(const ReadInfo& readInfo);

            uint16_t m_tag;
            ReadFunc m_func;
        };
        static const IFDEntryReader s_entryReaders[] = {
            { TagType::ImageWidth, [](const IFDEntryReader::ReadInfo& readInfo) {
                if (readInfo.entryCount != 1)
                    return;
                auto reader = readInfo.reader;
                if (readInfo.entryType == ValueType::Short) {
                    readInfo.settings->width = static_cast<unsigned>(reader.Get<uint16_t>());
                }
                else if (readInfo.entryType == ValueType::Long) {
                    readInfo.settings->width = static_cast<unsigned>(reader.Get<uint32_t>());
                }
            }},
            { TagType::ImageLength, [](const IFDEntryReader::ReadInfo& readInfo) {
                if (readInfo.entryCount != 1)
                    return;
                auto reader = readInfo.reader;
                if (readInfo.entryType == ValueType::Short) {
                    readInfo.settings->height = static_cast<unsigned>(reader.Get<uint16_t>());
                }
                else if (readInfo.entryType == ValueType::Long) {
                    readInfo.settings->height = static_cast<unsigned>(reader.Get<uint32_t>());
                }
            }},
            { TagType::BitsPerSample, [](const IFDEntryReader::ReadInfo& readInfo) {
                if (readInfo.entryType != ValueType::Short)
                    return;
                auto reader = readInfo.reader;

                const auto entrySize = ByteSizeFromValueType(ValueType(readInfo.entryType)) * readInfo.entryCount;
                if (entrySize > 4)
                {
                    const auto offset = reader.Get<uint32_t>();
                    readInfo.settings->bitsPerSampleReader = readInfo.reader.GetTopReader(offset, entrySize);
                }
                else
                {
                    readInfo.settings->bitsPerSampleReader = reader;
                }
            }},
            { TagType::Compression, [](const IFDEntryReader::ReadInfo& readInfo) {
                if (readInfo.entryType != ValueType::Short || readInfo.entryCount != 1)
                    return;
                auto reader = readInfo.reader;
                readInfo.settings->compression = reader.Get<uint16_t>();
            }},
            { TagType::PhotometricInterpretation, [](const IFDEntryReader::ReadInfo& readInfo) {
                if (readInfo.entryType != ValueType::Short || readInfo.entryCount != 1)
                    return;
                auto reader = readInfo.reader;
                readInfo.settings->photometricInterpretation = reader.Get<uint16_t>();
            }},
            { TagType::StripOffsets, [](const IFDEntryReader::ReadInfo& readInfo) {
                if (readInfo.entryType != ValueType::Short && readInfo.entryType != ValueType::Long)
                    return;
                auto reader = readInfo.reader;
                const auto entrySize = ByteSizeFromValueType(ValueType(readInfo.entryType)) * readInfo.entryCount;
                readInfo.settings->stripOffsetsType = ValueType(readInfo.entryType);
                readInfo.settings->stripsPerImage = readInfo.entryCount;
                if (entrySize > 4)
                {
                    const auto offset = reader.Get<uint32_t>();
                    readInfo.settings->stripOffsetsReader = readInfo.reader.GetTopReader(offset, entrySize);
                }
                else
                {
                    readInfo.settings->stripOffsetsReader = reader;
                }
            }},
            { TagType::SamplePerPixel, [](const IFDEntryReader::ReadInfo& readInfo) {
                if (readInfo.entryType != ValueType::Short || readInfo.entryCount != 1)
                    return;
                auto reader = readInfo.reader;
                readInfo.settings->samplesPerPixel = reader.Get<uint16_t>();
            }},
            { TagType::RowsPerStrip, [](const IFDEntryReader::ReadInfo& readInfo) {
                if (readInfo.entryCount != 1)
                    return;
                auto reader = readInfo.reader;
                if (readInfo.entryType == ValueType::Short) {
                    readInfo.settings->rowsPerStrip = static_cast<unsigned>(reader.Get<uint16_t>());
                }
                else if (readInfo.entryType == ValueType::Long) {
                    readInfo.settings->rowsPerStrip = static_cast<unsigned>(reader.Get<uint32_t>());
                }
            }},
            { TagType::StripByteCounts, [](const IFDEntryReader::ReadInfo& readInfo) {
                if (readInfo.entryType != ValueType::Short && readInfo.entryType != ValueType::Long)
                    return;
                if (readInfo.entryCount != readInfo.settings->stripsPerImage)
                    return;

                auto reader = readInfo.reader;
                const auto entrySize = ByteSizeFromValueType(ValueType(readInfo.entryType)) * readInfo.entryCount;
                readInfo.settings->stripByteCountsType = ValueType(readInfo.entryType);
                if (entrySize > 4)
                {
                    const auto offset = reader.Get<uint32_t>();
                    readInfo.settings->stripByteCountsReader = readInfo.reader.GetTopReader(offset, entrySize);
                }
                else
                {
                    readInfo.settings->stripByteCountsReader = reader;
                }
            }},
            { TagType::XResolution, [](const IFDEntryReader::ReadInfo& readInfo) {
                if (readInfo.entryCount != 1)
                    return;
                if (readInfo.entryType != ValueType::Rational)
                    return;
                auto reader = readInfo.reader;
                const auto offset = reader.Get<uint32_t>();
                auto rationalReader = readInfo.reader.GetTopReader(offset, ByteSizeFromValueType(ValueType(readInfo.entryType)));
                readInfo.settings->xRes.first = rationalReader.Get<uint32_t>();
                readInfo.settings->xRes.second = rationalReader.Get<uint32_t>();
            }},
            { TagType::YResolution, [](const IFDEntryReader::ReadInfo& readInfo) {
                if (readInfo.entryCount != 1)
                    return;
                if (readInfo.entryType != ValueType::Rational)
                    return;
                auto reader = readInfo.reader;
                const auto offset = reader.Get<uint32_t>();
                auto rationalReader = readInfo.reader.GetTopReader(offset, ByteSizeFromValueType(ValueType(readInfo.entryType)));
                readInfo.settings->yRes.first = rationalReader.Get<uint32_t>();
                readInfo.settings->yRes.second = rationalReader.Get<uint32_t>();
            }},
            { TagType::ResolutionUnit, [](const IFDEntryReader::ReadInfo& readInfo) {
                if (readInfo.entryCount != 1)
                    return;
                auto reader = readInfo.reader;
                if (readInfo.entryType == ValueType::Short) {
                    readInfo.settings->resolutionUnit = reader.Get<uint16_t>();
                }
            }},
            { TagType::ColorMap, [](const IFDEntryReader::ReadInfo& readInfo) {
                if (readInfo.entryType != ValueType::Short)
                    return;

                auto reader = readInfo.reader;
                const auto entrySize = readInfo.entryCount * ByteSizeFromValueType(ValueType(readInfo.entryType));
                if (entrySize > 4)
                {
                    const auto offset = reader.Get<uint32_t>();
                    readInfo.settings->colorMapReader = readInfo.reader.GetTopReader(offset, entrySize);
                }
                else
                {
                    readInfo.settings->colorMapReader = reader;
                }
            }},
            { TagType::SampleFormat, [](const IFDEntryReader::ReadInfo& readInfo) {
                if (readInfo.entryType != ValueType::Short)
                    return;

                if (readInfo.entryCount != readInfo.settings->samplesPerPixel)
                    return;

                auto reader = readInfo.reader;

                if (readInfo.entryCount > 2)
                {
                    auto const sizeOfEntries = sizeof(uint16_t) * readInfo.entryCount;
                    auto const offset = reader.Get<uint32_t>();
                    readInfo.settings->sampleFormatReader = readInfo.reader.GetTopReader(offset, sizeOfEntries);
                }
                else
                {
                    readInfo.settings->sampleFormatReader = reader;
                }
            }},
        };


        ////////////////////////////////////////////////////////////////////////////////
        unsigned GetShortOrLong(MemReader& reader, ValueType type)
        {
            if (type == ValueType::Short)
                return reader.Get<uint16_t>();
            else
                return reader.Get<uint32_t>();
        }

        inline void WrapPos(unsigned& row, unsigned& col, const unsigned w)
        {
            ++col;
            if (col >= w)
            {
                ++row;
                col = 0;
            }
        }



        ////////////////////////////////////////////////////////////////////////////////
        bool DecompressStrip(
            lptk::DynAry<char>& result,
            const size_t unpackedSize,
            MemReader& stripReader,
            CompressionType compType)
        {
            if (compType == CompressionType::PackBits)
            {
                result.resize(unpackedSize);

                auto curBytes = 0u;
                while (curBytes < unpackedSize)
                {
                    const auto n = int(stripReader.Get<int8_t>());

                    if (n >= 0 && n <= 127)
                    {
                        const auto copyCount = n + 1;
                        for (int i = 0; i < copyCount; ++i)
                        {
                            const auto byte = char(stripReader.Get<uint8_t>());
                            result.push_back(byte);
                        }

                        curBytes += copyCount;
                    }
                    else if (n >= -127 && n <= -1)
                    {
                        const auto copyCount = -n + 1;
                        const auto byte = char(stripReader.Get<uint8_t>());

                        for (int i = 0; i < copyCount; ++i)
                        {
                            result.push_back(byte);
                        }

                        curBytes += copyCount;
                    }
                }

                // nothing went wrong, return true!
                if (curBytes == unpackedSize)
                    return true;
            }
            else if (compType == CompressionType::ModifiedHuffman)
            {
                result.resize(unpackedSize);
                // TODO
                return false;
            }

            return false;
        }
        
        
        
        ////////////////////////////////////////////////////////////////////////////////
        template<typename UnpackPixelFn> 
        bool UnpackImage(
            Image& result,
            unsigned numChannels,
            const ImageSettings& settings,
            UnpackPixelFn&& unpack)
        {
            result.Init(settings.width, settings.height, numChannels);

            MemReader offsetsReader = settings.stripOffsetsReader;
            const auto offsetsType = settings.stripOffsetsType;

            MemReader byteCountReader = settings.stripByteCountsReader;
            const auto byteCountType = settings.stripByteCountsType;
                
            auto row = 0u;
            auto col = 0u;
            lptk::DynAry<char> uncompressed;
            for (auto strip = 0u; strip < settings.stripsPerImage; ++strip)
            {
                const auto stripOffset = GetShortOrLong(offsetsReader, offsetsType);
                const auto unpackedSize = GetShortOrLong(byteCountReader, byteCountType);

                auto stripReader = offsetsReader.GetTopReader(stripOffset,
                    stripOffset < offsetsReader.GetTopSize() ?
                        offsetsReader.GetTopSize() - stripOffset :
                        0);

                uncompressed.clear();
                if (settings.compression != NoCompression)
                {
                    uncompressed.reserve(unpackedSize);
                    if (!DecompressStrip(uncompressed, unpackedSize, stripReader, CompressionType(settings.compression)))
                        return false;
                    // NOTE: This is already in correct endian format, so does not need flags.
                    stripReader = lptk::MemReader(uncompressed.data(), uncompressed.size());
                }

                // at this point strip reader is uncompressed and can be read from 'stripReader'
                while (stripReader)
                {
                    unpack(result, row, col, stripReader);
                    WrapPos(row, col, settings.width);
                }
            }

            return true;
        }
        
        
        
        ////////////////////////////////////////////////////////////////////////////////
        bool UnpackRGBImage(
            Image& result,
            const ImageSettings& settings)
        {
            if (settings.photometricInterpretation != 2)
                return false;

            if (settings.colorMapReader)
                return false;
            
            if (settings.samplesPerPixel < 3)
                return false;
            
            if (settings.compression != NoCompression &&
                settings.compression != PackBits)
                return false;
            
            auto bitsPerSampleReader = settings.bitsPerSampleReader;
            const auto bitsPerSample = bitsPerSampleReader.Get<uint16_t>();
            for (unsigned ch = 1u; ch < settings.samplesPerPixel; ++ch)
            {
                const auto nextVal = bitsPerSampleReader.Get<uint16_t>();
                if(nextVal != bitsPerSample)
                    return false;
            }

            bool isFloat = false;
            if (settings.sampleFormatReader)
            {
                auto sampleFormatReader = settings.sampleFormatReader;
                const auto sampleFormat = sampleFormatReader.Get<uint16_t>();
                for (unsigned ch = 1u; ch < settings.samplesPerPixel; ++ch)
                {
                    const auto nextVal = sampleFormatReader.Get<uint16_t>();
                    if (nextVal != sampleFormat)
                        return false;
                }

                if (sampleFormat == 3)
                    isFloat = true;
            }


            if (!isFloat)
            {

                if (bitsPerSample == 8)
                {
                    return UnpackImage(result, settings.samplesPerPixel, settings, [](
                        Image& result, unsigned row, unsigned col, MemReader& stripReader) {
                        const auto numCh = result.GetNumChannels();
                        for (auto ch = 0u; ch < numCh; ++ch)
                        {
                            const auto byteVal = stripReader.Get<uint8_t>();
                            result.Set(col, row, ch, lptk::Clamp(
                                float(byteVal) / float(0xff), 0.f, 1.f));
                        }
                    });
                }
            }
            else
            {
                // TODO
                //if (bitsPerSample == 16)
                //{
                //    return UnpackImage(result, settings.samplesPerPixel, settings, [](
                //        Image& result, unsigned row, unsigned col, MemReader& stripReader) {
                //        const auto numCh = result.GetNumChannels();
                //        for (auto ch = 0u; ch < numCh; ++ch)
                //        {
                //            const auto val = stripReader.Get<uint16_t>();
                //            // TODO
                //            result.Set(col, row, ch, val);
                //        }
                //    });
                //}
                //else if (bitsPerSample == 24)
                //{
                //    return UnpackImage(result, settings.samplesPerPixel, settings, [](
                //        Image& result, unsigned row, unsigned col, MemReader& stripReader) {
                //        const auto numCh = result.GetNumChannels();
                //        for (auto ch = 0u; ch < numCh; ++ch)
                //        {
                //            uint32_t val = 0;
                //            val = stripReader.Get<uint8_t>();
                //            val |= stripReader.Get<uint8_t>() << 8;
                //            val |= stripReader.Get<uint8_t>() << 16;
                //            // TODO;
                //            result.Set(col, row, ch, val);
                //        }
                //    });
                //}
                if (bitsPerSample == 32)
                {
                    return UnpackImage(result, settings.samplesPerPixel, settings, [](
                        Image& result, unsigned row, unsigned col, MemReader& stripReader) {
                        const auto numCh = result.GetNumChannels();
                        for (auto ch = 0u; ch < numCh; ++ch)
                        {
                            const auto val = stripReader.Get<float>();
                            result.Set(col, row, ch, val);
                        }
                    });
                }
            }
            return false;
        }



        ////////////////////////////////////////////////////////////////////////////////
        bool UnpackColorMappedRGBImage(
            Image& result,
            const ImageSettings& settings)
        {
            if (settings.photometricInterpretation != 3)
                return false;

            if (!settings.colorMapReader)
                return false;
            
            if (settings.compression != NoCompression &&
                settings.compression != PackBits)
                return false;

            auto bitsPerSampleReader = settings.bitsPerSampleReader;
            const auto bitsPerSample = bitsPerSampleReader.Get<uint16_t>();
            if (bitsPerSample != 4 && bitsPerSample != 8)
                return false;

            const auto colorMapReader = settings.colorMapReader;
            const auto stride = (size_t(1) << bitsPerSample) * sizeof(uint16_t);
            if (bitsPerSample == 8)
            {
                return UnpackImage(result, 3, settings, [&colorMapReader, &stride](
                    Image& result, unsigned row, unsigned col, MemReader& stripReader) {
                    const auto byte = stripReader.Get<uint8_t>();
                    for (auto ch = 0u; ch < 3; ++ch)
                    {
                        auto colorReader = colorMapReader.GetSubReader(ch * stride + 
                            sizeof(uint16_t) * byte, sizeof(uint16_t));
                        result.Set(col, row, ch, lptk::Clamp(
                            float(colorReader.Get<uint16_t>()) / float(65536), 0.f, 1.f));
                    }
                });
            }
            else if (bitsPerSample == 4)
            {
                auto curVal = 0u;
                auto shift = 0u;
                return UnpackImage(result, 3, settings, [&curVal, &shift, &colorMapReader, &stride](
                    Image& result, unsigned row, unsigned col, MemReader& stripReader) {
                    // Couldn't find any documentation on this, so reading lsb to msb for 4 bit versions.
                    auto byte = 0u;
                    if (shift == 0u)
                    {
                        curVal = stripReader.Get<uint8_t>();
                        byte = curVal & 0xf;
                        shift = 4u;
                    }
                    else
                    {
                        curVal >>= shift;
                        byte = curVal & 0xf;
                        shift = 0u;
                    }
                    
                    for (auto ch = 0u; ch < 3; ++ch)
                    {
                        auto colorReader = colorMapReader.GetSubReader(ch * stride + 
                            sizeof(uint16_t) * byte, sizeof(uint16_t));
                        result.Set(col, row, ch, lptk::Clamp(
                            float(colorReader.Get<uint16_t>()) / float(65536), 0.f, 1.f));
                    }
                });
            }
            return false;
        }



        ////////////////////////////////////////////////////////////////////////////////
        bool UnpackGrayscaleImage(
            Image& result,
            const ImageSettings& settings)
        {
            auto bitsPerSampleReader = settings.bitsPerSampleReader;
            const auto bitsPerSample = bitsPerSampleReader.Get<uint16_t>();
            if (bitsPerSample != 4 && bitsPerSample != 8)
                return false;

            if (settings.compression != NoCompression &&
                settings.compression != PackBits)
                return false;

            if (bitsPerSample == 8)
            {
                return UnpackImage(result, 1, settings, [](
                    Image& result, unsigned row, unsigned col, MemReader& stripReader) {
                    const auto byte = stripReader.Get<uint8_t>();
                    const auto val = lptk::Clamp(float(byte) / float(0xff), 0.f, 1.f);
                    result.Set(col, row, 0, val);
                });
            }
            else if (bitsPerSample == 4)
            {
                auto curVal = 0u;
                auto shift = 0u;
                return UnpackImage(result, 1, settings, [&curVal, &shift](
                    Image& result, unsigned row, unsigned col, MemReader& stripReader) {
                    // Couldn't find any documentation on this, so reading lsb to msb for 4 bit versions.
                    auto byte = 0u;
                    if (shift == 0u)
                    {
                        curVal = stripReader.Get<uint8_t>();
                        byte = curVal & 0xf;
                        shift = 4u;
                    }
                    else
                    {
                        curVal >>= shift;
                        byte = curVal & 0xf;
                        shift = 0u;
                    }
                        
                    const auto val = lptk::Clamp(float(byte) / float(0xf), 0.f, 1.f);
                    result.Set(col, row, 0, val);
                });

            }
            return false;
        }



        ////////////////////////////////////////////////////////////////////////////////
        bool UnpackBilevelImage(
            Image& result,
            const ImageSettings& settings)
        {
            if (settings.photometricInterpretation >= 2)
                return false;

            if (settings.compression != CompressionType::NoCompression &&
                settings.compression != CompressionType::PackBits &&
                settings.compression != CompressionType::ModifiedHuffman)
                return false;

            const auto zeroVal = settings.photometricInterpretation == 0 ? 1.f : 0.f;
            const auto nonzeroVal = 1.f - zeroVal;

            return UnpackImage(result, 1, settings, [zeroVal, nonzeroVal](
                Image& result, unsigned row, unsigned col, MemReader& stripReader) {
                const auto byte = stripReader.Get<uint8_t>();
                const auto val = byte == 0 ? zeroVal : nonzeroVal;
                result.Set(col, row, 0, val);
            });
        }

        
        
        ////////////////////////////////////////////////////////////////////////////////
        bool LoadTIFFSubimage(
            Image& result,
            const unsigned ifdCount,
            const lptk::MemReader& ifdReader)
        {
            ImageSettings settings;
            IFDEntryReader::ReadInfo readInfo;
            readInfo.settings = &settings;

            auto formatEntryIndex = 0;
            for (auto entryIndex = 0u; entryIndex < ifdCount; ++entryIndex)
            {
                auto entryReader = ifdReader.GetSubReader(12 * entryIndex, 12u);
                if (entryReader.Error())
                {
                    fprintf(stderr, "LoadTIFFSubimage: IFD entry at index %u is too small\n", unsigned(entryIndex));
                    return false;
                }
                const auto entryTag = entryReader.Get<uint16_t>();

                while (formatEntryIndex < ARRAY_SIZE(s_entryReaders) && s_entryReaders[formatEntryIndex].m_tag < entryTag)
                    ++formatEntryIndex;

                if (formatEntryIndex < ARRAY_SIZE(s_entryReaders) && s_entryReaders[formatEntryIndex].m_tag == entryTag)
                {
                    const auto entryType = entryReader.Get<uint16_t>();
                    const auto entryCount = entryReader.Get<uint32_t>();
                    // only use the leftmost bytes of the value but cast to a uint32_t for simplicity

                    readInfo.entryType = entryType;
                    readInfo.entryCount = entryCount;
                    readInfo.reader = entryReader;

                    s_entryReaders[formatEntryIndex].m_func(readInfo);
                }
            }

            if (settings.width == 0)
            {
                fprintf(stderr, "LoadTIFFSubimage ERROR: width is 0\n");
                return false;
            }
            if (settings.height == 0)
            {
                fprintf(stderr, "LoadTIFFSubimage ERROR: height is 0\n");
                return false;
            }
            
            if (UnpackRGBImage(result, settings))
                return true;
            if (UnpackColorMappedRGBImage(result, settings))
                return true;
            if (UnpackGrayscaleImage(result, settings))
                return true;
            if (UnpackBilevelImage(result, settings))
                return true;

            fprintf(stderr, "LoadTIFFSubimage ERROR: don't know how to read sub image\n");
            return false;
        }
    }

    
    
    ////////////////////////////////////////////////////////////////////////////////
    bool LoadTIFF(Image& result, const char* filename)
    {
        const auto fileData = lptk::ReadFile<lptk::DynAry<char>>(filename);
        if (fileData.empty())
            return false;

        return LoadTIFF(result, fileData.data(), fileData.size());
    }


    
    ////////////////////////////////////////////////////////////////////////////////
    bool LoadTIFF(Image& result, const char* memory, size_t n)
    {
        lptk::MemReader topReader(memory, n);

        // image file header is 8 bytes
        const auto byteOrder = topReader.Get<uint16_t>(false);
        if (byteOrder != 0x4949 && byteOrder != 0x4d4d)
        {
            fprintf(stderr, "Unknown TIFF byte ordering %x\n", unsigned(byteOrder));
            return false;
        }

        const bool srcLittleEndian = byteOrder == 0x4949;
        auto headerReader = topReader.GetSubReader(srcLittleEndian ? MemFormatFlag::FLAG_LittleEndianData : MemFormatFlag::FLAG_BigEndianData);

        const auto fileID = headerReader.Get<uint16_t>();
        if (fileID != 42)
        {
            fprintf(stderr, "unknown TIFF ID %x\n", unsigned(fileID));
            return false;
        }

        const auto firstIFDOffset = headerReader.Get<uint32_t>();
        auto ifdReader = headerReader.GetTopReader(firstIFDOffset, n - firstIFDOffset);

        const auto numDirEntries = ifdReader.Get<uint16_t>();
        return tiff::LoadTIFFSubimage(
            result,
            numDirEntries,
            ifdReader);
    }



    ////////////////////////////////////////////////////////////////////////////////
    bool LoadPFM(Image& result, const char* filename)
    {
        const auto fileData = lptk::ReadFile<lptk::DynAry<char>>(filename);
        if (fileData.empty())
            return false;

        return LoadPFM(result, fileData.data(), fileData.size());
    }

    bool LoadPFM(Image& result, const char* memory, size_t n)
    {
        lptk::MemReader reader(memory, n);
        char typeBuf[2] = {};
        typeBuf[0] = reader.Get<char>();
        typeBuf[1] = reader.Get<char>();
        while (reader && reader.Get<char>() != '\n') {}

        if (typeBuf[0] != 'P') 
        {
            fprintf(stderr, "LoadPFM: Unrecognized PFM prefix\n");
            return false;
        }
        const auto numChannels = typeBuf[1] == 'F' ?  3 :
            typeBuf[1] == 'f' ?  1 :
            0;

        if (!numChannels)
        {
            fprintf(stderr, "LoadPFM: Unrecognized PFM prefix\n");
            return false;
        }
             
        char numBuf[64];
        auto bufIdx = 0;
        while (reader) {
            const auto cur = reader.Get<char>();
            if (cur == ' ')
                break;
            if (isdigit(cur) && bufIdx < ARRAY_SIZE(numBuf))
            {
                numBuf[bufIdx++] = cur;
            }
        }
        numBuf[ARRAY_SIZE(numBuf) - 1] = '\0';
        auto const xRes = unsigned(atoi(numBuf));
       
        bufIdx = 0;
        while (reader) {
            const auto cur = reader.Get<char>();
            if (cur == '\n')
                break;
            if (isdigit(cur) && bufIdx < ARRAY_SIZE(numBuf))
            {
                numBuf[bufIdx++] = cur;
            }
        }
        numBuf[ARRAY_SIZE(numBuf) - 1] = '\0';
        auto const yRes = unsigned(atoi(numBuf));

        if (!xRes || !yRes) 
        {
            fprintf(stderr, "LoadPFM: Invalid resolution: %u x %u\n", unsigned(xRes), unsigned(yRes));
            return false;
        }

        bufIdx = 0;
        while (reader) {
            const auto cur = reader.Get<char>();
            if (cur == '\n')
                break;
            if (bufIdx < ARRAY_SIZE(numBuf))
            {
                numBuf[bufIdx++] = cur;
            }
        }
        numBuf[ARRAY_SIZE(numBuf) - 1] = '\0';

        const auto littleEndian = atof(numBuf) < 0.f;

        auto dataReader = reader.GetSubReader(littleEndian ? lptk::MemFormatFlag::FLAG_LittleEndianData : lptk::MemFormatFlag::FLAG_BigEndianData);

        result.Init(xRes, yRes, numChannels);
        for (auto y = 0u; y < yRes; ++y)
        {
            auto yDest = yRes - y - 1;
            for (auto x = 0u; x < xRes; ++x)
            {
                for (auto ch = 0; ch < numChannels; ++ch)
                {
                    if (!dataReader)
                    {
                        fprintf(stderr, "LoadPFM: Ran out of data at %u x %u ch %u\n", x, y, ch);
                        return false;
                    }

                    result.Set(x, yDest, ch, dataReader.Get<float>());
                }
            }
        }
        return true;
    }
}
