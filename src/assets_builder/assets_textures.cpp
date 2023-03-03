#include "dummy.h"
#include "assets.h"

dummy_internal void
LoadTextureAsset(const char *FilePath, texture_asset *Asset)
{
    if (stbi_is_hdr(FilePath))
    {
        Asset->Bitmap.IsHDR = true;
        Asset->Bitmap.Pixels = stbi_loadf(FilePath, &Asset->Bitmap.Width, &Asset->Bitmap.Height, &Asset->Bitmap.Channels, 0);
    }
    else
    {
        Asset->Bitmap.IsHDR = false;
        Asset->Bitmap.Pixels = stbi_load(FilePath, &Asset->Bitmap.Width, &Asset->Bitmap.Height, &Asset->Bitmap.Channels, 0);
    }
}

dummy_internal void
WriteTextureAsset(const char *FilePath, texture_asset *Asset)
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
    Header.Type = AssetType_Texture;
    CopyString("Dummy texture asset file", Header.Description);

    u64 CurrentStreamPosition = 0;

    fwrite(&Header, sizeof(asset_header), 1, AssetFile);

    CurrentStreamPosition = ftell(AssetFile);

    texture_asset_header TextureAssetHeader = {};

    TextureAssetHeader.Width = Asset->Bitmap.Width;
    TextureAssetHeader.Height = Asset->Bitmap.Height;
    TextureAssetHeader.Channels = Asset->Bitmap.Channels;
    TextureAssetHeader.IsHDR = Asset->Bitmap.IsHDR;

    fwrite(&TextureAssetHeader, sizeof(texture_asset_header), 1, AssetFile);

    CurrentStreamPosition = ftell(AssetFile);
    TextureAssetHeader.PixelsOffset = CurrentStreamPosition;

    u32 BitmapSize = Asset->Bitmap.Width * Asset->Bitmap.Height * Asset->Bitmap.Channels;

    if (Asset->Bitmap.IsHDR)
    {
        fwrite(Asset->Bitmap.Pixels, sizeof(f32), BitmapSize, AssetFile);
    }
    else
    {
        fwrite(Asset->Bitmap.Pixels, sizeof(u8), BitmapSize, AssetFile);
    }

    fseek(AssetFile, (long)Header.DataOffset, SEEK_SET);
    fwrite(&TextureAssetHeader, sizeof(texture_asset_header), 1, AssetFile);

    fclose(AssetFile);
}

dummy_internal void
ReadTextureAsset(const char *FilePath, texture_asset *Asset, texture_asset *OriginalAsset)
{
    FILE *AssetFile = fopen(FilePath, "rb");

    u32 FileSize = GetFileSize(AssetFile);

    u8 *Buffer = AllocateMemory<u8>(FileSize);

    fread(Buffer, FileSize, 1, AssetFile);

    asset_header *Header = GET_DATA_AT(Buffer, 0, asset_header);

    texture_asset_header *TextureHeader = GET_DATA_AT(Buffer, Header->DataOffset, texture_asset_header);

    u8 *Pixels = (u8 *)(Buffer + TextureHeader->PixelsOffset);

    fclose(AssetFile);
}

dummy_internal void
ProcessTextureAsset(const char *FilePath, const char *OutputPath)
{
    texture_asset Asset;
    LoadTextureAsset(FilePath, &Asset);
    WriteTextureAsset(OutputPath, &Asset);

#if 1
    texture_asset TestAsset = {};
    ReadTextureAsset(OutputPath, &TestAsset, &Asset);
#endif
}
