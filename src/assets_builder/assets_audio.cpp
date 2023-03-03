#include "dummy.h"
#include "assets.h"

// Little-Endian
#define RIFF 'FFIR'
#define WAVE 'EVAW'
#define FMT ' tmf'
#define DATA 'atad'

#define WAVE_FORMAT_PCM 0x0001	        // PCM
#define WAVE_FORMAT_IEEE_FLOAT 0x0003   // IEEE float

#pragma pack(push, 1)

struct wave_chunk
{
    u32 ChunkId;
    u32 ChunkSize;
};

struct wave_chunk_riff
{
    u32 Format;
};

struct wave_chunk_fmt
{
    u16 wFormatTag;             // Format code
    u16 nChannels;              // Number of interleaved channels
    u32 nSamplesPerSec;         // Sampling rate(blocks per second)
    u32 nAvgBytesPerSec;        // Data rate
    u16 nBlockAlign;            // Data block size(bytes)
    u16 wBitsPerSample;         // Bits per sample
    u16 cbSize;                 // Size of the extension(0 or 22)
    u16 wValidBitsPerSample;    // Number of valid bits
    u32 dwChannelMask;          // Speaker position mask
    u16 SubFormat;              // GUID, including the data format code
};

#pragma pack(pop)

inline wave_chunk *
GetChunk(u8 *At)
{
    wave_chunk *Result = (wave_chunk *) At;
    return Result;
}

inline u8 *
GetChunkData(u8 *At)
{
    u8 *Result = At + sizeof(wave_chunk);
    return Result;
}

struct wave_chunk_iterator
{
    u8 *At;
    u8 *End;
};

inline wave_chunk_iterator
GetChunkIterator(u8 *Buffer, u32 BufferSize)
{
    wave_chunk_iterator Result;

    Result.At = Buffer + sizeof(wave_chunk) + sizeof(wave_chunk_riff);
    Result.End = Buffer + BufferSize;

    return Result;
}

inline void
NextChunk(wave_chunk_iterator *Iterator)
{
    wave_chunk *Chunk = GetChunk(Iterator->At);
    Iterator->At += sizeof(wave_chunk) + Chunk->ChunkSize;
}

inline bool32
HasNextChunk(wave_chunk_iterator *Iterator)
{
    bool32 Result = Iterator->At < Iterator->End;
    return Result;
}

dummy_internal void
LoadAudioAsset(const char *FilePath, audio_clip_asset *Asset)
{
    FILE *File = fopen(FilePath, "rb");
    u32 BufferSize = GetFileSize(File);

    u8 *Buffer = AllocateMemory<u8>(BufferSize);
    fread(Buffer, 1, BufferSize, File);

    fclose(File);

    wave_chunk_riff *RiffChunk = (wave_chunk_riff *) GetChunkData(Buffer);

    Assert(RiffChunk->Format == WAVE);

    wave_chunk_iterator Iterator = GetChunkIterator(Buffer, BufferSize);

    while (HasNextChunk(&Iterator))
    {
        wave_chunk *Chunk = GetChunk(Iterator.At);

        if (Chunk->ChunkId == FMT)
        {
            wave_chunk_fmt *FmtChunk = (wave_chunk_fmt *) GetChunkData(Iterator.At);

            Asset->Format = FmtChunk->wFormatTag;
            Asset->Channels = FmtChunk->nChannels;
            Asset->SamplesPerSecond = FmtChunk->nSamplesPerSec;
            Asset->BitsPerSample = FmtChunk->wBitsPerSample;
        }

        if (Chunk->ChunkId == DATA)
        {
            u32 AudioBytes = Chunk->ChunkSize;
            u8 *AudioData = GetChunkData(Iterator.At);

            Asset->AudioBytes = AudioBytes;
            Asset->AudioData = AudioData;
        }

        NextChunk(&Iterator);
    }
}

dummy_internal void
WriteAudioAsset(const char *FilePath, audio_clip_asset *Asset)
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
    Header.Type = AssetType_AudioClip;
    CopyString("Dummy audio clip asset file", Header.Description);

    u64 CurrentStreamPosition = 0;

    fwrite(&Header, sizeof(asset_header), 1, AssetFile);

    CurrentStreamPosition = ftell(AssetFile);

    audio_clip_asset_header AudioAssetHeader = {};

    AudioAssetHeader.Format = Asset->Format;
    AudioAssetHeader.Channels = Asset->Channels;
    AudioAssetHeader.SamplesPerSecond = Asset->SamplesPerSecond;
    AudioAssetHeader.BitsPerSample = Asset->BitsPerSample;
    AudioAssetHeader.AudioBytes = Asset->AudioBytes;

    fwrite(&AudioAssetHeader, sizeof(audio_clip_asset_header), 1, AssetFile);

    CurrentStreamPosition = ftell(AssetFile);
    AudioAssetHeader.AudioDataOffset = CurrentStreamPosition;
    fwrite(Asset->AudioData, sizeof(u8), Asset->AudioBytes, AssetFile);

    fseek(AssetFile, (long) Header.DataOffset, SEEK_SET);
    fwrite(&AudioAssetHeader, sizeof(audio_clip_asset_header), 1, AssetFile);

    fclose(AssetFile);
}

// For testing
dummy_internal void
ReadAudioAsset(const char *FilePath, audio_clip_asset *Asset, audio_clip_asset *OriginalAsset)
{
    FILE *AssetFile = fopen(FilePath, "rb");

    u32 FileSize = GetFileSize(AssetFile);

    u8 *Buffer = AllocateMemory<u8>(FileSize);

    fread(Buffer, FileSize, 1, AssetFile);

    asset_header *Header = (asset_header *) Buffer;

    audio_clip_asset_header *AudioHeader = (audio_clip_asset_header *) (Buffer + Header->DataOffset);

    u8 *AudioData = (u8 *) (Buffer + AudioHeader->AudioDataOffset);

    fclose(AssetFile);
}

dummy_internal void
ProcessAudioAsset(const char *FilePath, const char *OutputPath)
{
    audio_clip_asset Asset;
    LoadAudioAsset(FilePath, &Asset);
    WriteAudioAsset(OutputPath, &Asset);

#if 1
    audio_clip_asset TestAsset = {};
    ReadAudioAsset(OutputPath, &TestAsset, &Asset);
#endif
}
