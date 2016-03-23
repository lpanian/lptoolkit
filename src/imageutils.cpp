#include "toolkit/imageutils.hh"
#include "toolkit/memstream.hh"
#include "toolkit/image.hh"
#include "toolkit/mathcommon.hh"
#include "toolkit/filehelper.hh"

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

}
