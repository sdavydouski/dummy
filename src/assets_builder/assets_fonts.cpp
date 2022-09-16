struct char_info
{
    u32 CharacterIndex;
    stbtt_packedchar *GlyphInfo;
    codepoints_range *Encoding;
    stbtt_packedchar *CharData;
};

struct unicode_block
{
    u32 Start;
    u32 End;
};

inline u32
GetCodepointsRangeCount(codepoints_range *Range)
{
    u32 Result = (Range->End - Range->Start) + 1;

    assert(Result > 0);

    return Result;
}

inline char_info
GetCharInfo(
    u32 CodepointIndex,
    u32 CodepointsRangeCount,
    codepoints_range *CodepointsRanges,
    stbtt_packedchar **CodepointsCharData
)
{
    char_info Result = {};

    u32 TargetIndex = 0;
    for (u32 CodepointRangeIndex = 0; CodepointRangeIndex < CodepointsRangeCount; ++CodepointRangeIndex)
    {
        codepoints_range *CodepointsRange = CodepointsRanges + CodepointRangeIndex;
        stbtt_packedchar **CharDataForRange = CodepointsCharData + CodepointRangeIndex;

        for (u32 CharacterIndex = 0; CharacterIndex < CodepointsRange->Count; ++CharacterIndex)
        {
            if (CodepointIndex == TargetIndex++)
            {
                Result.CharacterIndex = CharacterIndex;
                Result.GlyphInfo = *CharDataForRange + CharacterIndex;
                Result.Encoding = CodepointsRange;
                Result.CharData = *CharDataForRange;

                break;
            }
        }
    }

    Assert(CodepointIndex < TargetIndex);

    return Result;
}

internal void
LoadFontAsset(const char *FilePath, font_asset *FontAsset)
{
    f32 FontHeight = 72.f;

    FontAsset->TextureAtlas.Width = 1024;
    FontAsset->TextureAtlas.Height = 1024;
    FontAsset->TextureAtlas.Channels = 1;

    unicode_block UnicodeBlocks[] = {
        // Basic Latin
        {
            0x20,
            0x7E
        }
#if 0
        // Greek
        {
            0x370,
            0x3FF
        },
        // Cyrillic
        {
            0x410,
            0x045F
        },
        // Arabic
        {
            0x600,
            0x6FF
        }
#endif
    };

    FILE *FontFile = fopen(FilePath, "rb");
    u32 FileSize = GetFileSize(FontFile);

    u8 *FontBuffer = AllocateMemory<u8>(FileSize);
    fread(FontBuffer, 1, FileSize, FontFile);

    fclose(FontFile);

    stbtt_fontinfo FontInfo;
    stbtt_InitFont(&FontInfo, FontBuffer, 0);

    f32 Scale = stbtt_ScaleForPixelHeight(&FontInfo, FontHeight);

    i32 Ascent, Descent, LineGap;
    stbtt_GetFontVMetrics(&FontInfo, &Ascent, &Descent, &LineGap);
    Ascent = (i32) (Ascent * Scale);
    Descent = (i32) (Descent * Scale);
    LineGap = (i32) (LineGap * Scale);

    FontAsset->Ascent = Ascent;
    FontAsset->Descent = Descent;
    FontAsset->VerticalAdvance = Ascent - Descent + LineGap;

    stbtt_pack_context PackContext;
    FontAsset->TextureAtlas.Pixels = AllocateMemory<u8>(FontAsset->TextureAtlas.Width * FontAsset->TextureAtlas.Height * FontAsset->TextureAtlas.Channels);

    i32 StrideInBytes = 0;
    i32 Padding = 1;

    stbtt_PackBegin(&PackContext, (u8 *) FontAsset->TextureAtlas.Pixels, FontAsset->TextureAtlas.Width, FontAsset->TextureAtlas.Height, StrideInBytes, Padding, 0);

    u32 hOverSample = 2;
    u32 vOverSample = 2;
    stbtt_PackSetOversampling(&PackContext, hOverSample, vOverSample);

    i32 FontIndex = 0;

    FontAsset->CodepointsRangeCount = ArrayCount(UnicodeBlocks);
    FontAsset->CodepointsRanges = AllocateMemory<codepoints_range>(FontAsset->CodepointsRangeCount);
    stbtt_packedchar **CodepointsCharData = AllocateMemory<stbtt_packedchar *>(FontAsset->CodepointsRangeCount);

    u32 TotalCodepointCount = 0;
    for (u32 UnicodeBlockIndex = 0; UnicodeBlockIndex < ArrayCount(UnicodeBlocks); ++UnicodeBlockIndex)
    {
        unicode_block *UnicodeBlock = UnicodeBlocks + UnicodeBlockIndex;
        codepoints_range *CodepointsRange = FontAsset->CodepointsRanges + UnicodeBlockIndex;
        stbtt_packedchar **CharDataForRange = CodepointsCharData + UnicodeBlockIndex;

        CodepointsRange->Start = UnicodeBlock->Start;
        CodepointsRange->End = UnicodeBlock->End;
        CodepointsRange->Count = GetCodepointsRangeCount(CodepointsRange);

        *CharDataForRange = AllocateMemory<stbtt_packedchar>(CodepointsRange->Count);
        stbtt_PackFontRange(&PackContext, FontBuffer, FontIndex, FontHeight, CodepointsRange->Start, CodepointsRange->Count, *CharDataForRange);

        TotalCodepointCount += CodepointsRange->Count;
    }

    stbtt_PackEnd(&PackContext);

    FontAsset->GlyphCount = TotalCodepointCount;
    FontAsset->Glyphs = AllocateMemory<glyph>(TotalCodepointCount);

    for (u32 GlyphIndex = 0; GlyphIndex < TotalCodepointCount; ++GlyphIndex)
    {
        glyph *Glyph = FontAsset->Glyphs + GlyphIndex;

        char_info CharInfo = GetCharInfo(GlyphIndex, FontAsset->CodepointsRangeCount, FontAsset->CodepointsRanges, CodepointsCharData);
        stbtt_packedchar *GlyphInfo = CharInfo.GlyphInfo;

        Glyph->SpriteSize = vec2((f32) (GlyphInfo->x1 - GlyphInfo->x0), (f32) (GlyphInfo->y1 - GlyphInfo->y0));
    }

    FontAsset->HorizontalAdvanceTableCount = TotalCodepointCount * TotalCodepointCount;
    FontAsset->HorizontalAdvanceTable = AllocateMemory<f32>(FontAsset->HorizontalAdvanceTableCount);

    u32 CodepointIndex = 0;
    while (CodepointIndex < TotalCodepointCount)
    {
        char_info CharInfo = GetCharInfo(CodepointIndex, FontAsset->CodepointsRangeCount, FontAsset->CodepointsRanges, CodepointsCharData);

        stbtt_packedchar *CharData = CharInfo.CharData;
        codepoints_range *Encoding = CharInfo.Encoding;

        i32 CharacterIndex = CharInfo.CharacterIndex;

        wchar Character = Encoding->Start + CharacterIndex;

        f32 OffsetX = 0;
        f32 OffsetY = 0;
        stbtt_aligned_quad Quad;
        stbtt_GetPackedQuad(CharData, FontAsset->TextureAtlas.Width, FontAsset->TextureAtlas.Height, CharacterIndex, &OffsetX, &OffsetY, &Quad, 1);

        glyph *Glyph = FontAsset->Glyphs + CodepointIndex;

        Glyph->CharacterSize = vec2(Quad.x1 - Quad.x0, Quad.y1 - Quad.y0);
        Glyph->UV = vec2(Quad.s0, Quad.t0);
        Glyph->Alignment = vec2(Quad.x0, -Quad.y1);

        u32 OtherCodepointIndex = 0;
        while (OtherCodepointIndex < TotalCodepointCount)
        {
            char_info CharInfo = GetCharInfo(OtherCodepointIndex, FontAsset->CodepointsRangeCount, FontAsset->CodepointsRanges, CodepointsCharData);

            codepoints_range *Encoding = CharInfo.Encoding;
            i32 OtherCharacterIndex = CharInfo.CharacterIndex;

            wchar OtherCharacter = Encoding->Start + OtherCharacterIndex;

            f32 *HorizontalAdvance = FontAsset->HorizontalAdvanceTable + CodepointIndex * TotalCodepointCount + OtherCodepointIndex;
            f32 Kerning = (f32) stbtt_GetCodepointKernAdvance(&FontInfo, Character, OtherCharacter);
            Kerning = Kerning * Scale;

            *HorizontalAdvance = OffsetX + Kerning;

            ++OtherCodepointIndex;
        }

        ++CodepointIndex;
    }

#if 0
    // for testing
    char FontTextureAtlasFilePath[256];
    FormatString(FontTextureAtlasFilePath, "assets/%s.png", GetLastAfterDelimiter((char *) FilePath, '/'));
    stbi_write_png(FontTextureAtlasFilePath, FontAsset->TextureAtlas.Width, FontAsset->TextureAtlas.Height, FontAsset->TextureAtlas.Channels, FontAsset->TextureAtlas.Pixels, 0);
#endif
}

internal void
WriteFontAsset(const char *FilePath, font_asset *Asset)
{
    FILE *AssetFile = fopen(FilePath, "wb");

    if (!AssetFile)
    {
        errno_t Error;
        _get_errno(&Error);
        Assert(!"Panic");
    }

    asset_header Header = {};
    Header.MagicValue = 0x451;
    Header.Version = 1;
    Header.DataOffset = sizeof(asset_header);
    Header.Type = AssetType_Font;
    CopyString("Dummy font asset file", Header.Description);

    u64 CurrentStreamPosition = 0;

    fwrite(&Header, sizeof(asset_header), 1, AssetFile);

    CurrentStreamPosition = ftell(AssetFile);

    font_asset_header FontAssetHeader = {};

    i32 PixelCount = Asset->TextureAtlas.Width * Asset->TextureAtlas.Height * Asset->TextureAtlas.Channels;

    FontAssetHeader.TextureAtlasWidth = Asset->TextureAtlas.Width;
    FontAssetHeader.TextureAtlasHeight = Asset->TextureAtlas.Height;
    FontAssetHeader.TextureAtlasChannels = Asset->TextureAtlas.Channels;
    FontAssetHeader.VerticalAdvance = Asset->VerticalAdvance;
    FontAssetHeader.Ascent = Asset->Ascent;
    FontAssetHeader.Descent = Asset->Descent;
    FontAssetHeader.CodepointsRangeCount = Asset->CodepointsRangeCount;
    FontAssetHeader.HorizontalAdvanceTableCount = Asset->HorizontalAdvanceTableCount;
    FontAssetHeader.GlyphCount = Asset->GlyphCount;

    fwrite(&FontAssetHeader, sizeof(FontAssetHeader), 1, AssetFile);

    CurrentStreamPosition = ftell(AssetFile);
    FontAssetHeader.TextureAtlasOffset = CurrentStreamPosition;
    fwrite(Asset->TextureAtlas.Pixels, sizeof(u8), PixelCount, AssetFile);

    CurrentStreamPosition = ftell(AssetFile);
    FontAssetHeader.CodepointsRangesOffset = CurrentStreamPosition;
    fwrite(Asset->CodepointsRanges, sizeof(codepoints_range), Asset->CodepointsRangeCount, AssetFile);

    CurrentStreamPosition = ftell(AssetFile);
    FontAssetHeader.HorizontalAdvanceTableOffset = CurrentStreamPosition;
    fwrite(Asset->HorizontalAdvanceTable, sizeof(f32), Asset->HorizontalAdvanceTableCount, AssetFile);

    CurrentStreamPosition = ftell(AssetFile);
    FontAssetHeader.GlyphsOffset = CurrentStreamPosition;
    fwrite(Asset->Glyphs, sizeof(glyph), Asset->GlyphCount, AssetFile);

    fseek(AssetFile, (long) Header.DataOffset, SEEK_SET);
    fwrite(&FontAssetHeader, sizeof(font_asset_header), 1, AssetFile);

    fclose(AssetFile);
}

// For testing
internal void
ReadFontAsset(const char *FilePath, font_asset *Asset, font_asset *OriginalAsset)
{
    FILE *AssetFile = fopen(FilePath, "rb");

    u32 FileSize = GetFileSize(AssetFile);

    u8 *Buffer = AllocateMemory<u8>(FileSize);

    fread(Buffer, FileSize, 1, AssetFile);

    asset_header *Header = (asset_header *) Buffer;

    font_asset_header *FontHeader = (font_asset_header *) (Buffer + Header->DataOffset);

    u32 PixelCount = FontHeader->TextureAtlasWidth * FontHeader->TextureAtlasHeight * FontHeader->TextureAtlasChannels;
    u8 *TextureAtlas = (u8 *) (Buffer + FontHeader->TextureAtlasOffset);

    codepoints_range *CodepointsRanges = (codepoints_range *) (Buffer + FontHeader->CodepointsRangesOffset);
    f32 *HorizontalAdvanceTable = (f32 *) (Buffer + FontHeader->HorizontalAdvanceTableOffset);

    glyph *Glyphs = (glyph *) (Buffer + FontHeader->GlyphsOffset);

    fclose(AssetFile);
}

internal void
ProcessFont(const char *FilePath, const char *OutputPath)
{
    font_asset Asset;
    LoadFontAsset(FilePath, &Asset);
    WriteFontAsset(OutputPath, &Asset);

#if 1
    font_asset TestAsset = {};
    ReadFontAsset(OutputPath, &TestAsset, &Asset);
#endif
}
