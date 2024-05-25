#define _CRT_SECURE_NO_WARNINGS

#include "assets_models.cpp"
#include "assets_fonts.cpp"
#include "assets_audio.cpp"
#include "assets_textures.cpp"

dummy_internal void
BuildModelAssets(const char *InputPath, const char *OutputPath)
{
    for (const fs::directory_entry &Entry : fs::directory_iterator(InputPath))
    {
        fs::path EntryPath = Entry.path();
        fs::path EntryName = Entry.path().stem();

        if (Entry.is_directory())
        {
            char GlbFilePath[256];
            FormatString(GlbFilePath, "%s/%s.glb", EntryPath.generic_string().c_str(), EntryName.generic_string().c_str());

            char FbxFilePath[256];
            FormatString(FbxFilePath, "%s/%s.fbx", EntryPath.generic_string().c_str(), EntryName.generic_string().c_str());

            char FilePath[256];

            if (fs::exists(GlbFilePath))
            {
                CopyString(GlbFilePath, FilePath);
            }
            else if (fs::exists(FbxFilePath))
            {
                CopyString(FbxFilePath, FilePath);
            }
            else
            {
                Assert(!"Invalid model extension");
            }

            char AnimationConfigPath[256];
            FormatString(AnimationConfigPath, "%s/animation_graph.json", EntryPath.generic_string().c_str());

            char AnimationClipsPath[256];
            FormatString(AnimationClipsPath, "%s/clips", EntryPath.generic_string().c_str());

            char AssetPath[256];
            FormatString(AssetPath, "%s/%s.model.asset", OutputPath, EntryName.generic_string().c_str());

            printf("Processing %s...\n", FilePath);
            ProcessModelAsset(FilePath, AnimationConfigPath, AnimationClipsPath, AssetPath);
        }
        else
        {
            char AssetPath[256];
            FormatString(AssetPath, "%s/%s.model.asset", OutputPath, EntryName.generic_string().c_str());

            printf("Processing %s...\n", EntryPath.generic_string().c_str());
            ProcessModelAsset(EntryPath.generic_string().c_str(), AssetPath);
        }
    }
}

dummy_internal void
BuildFontAssets(const char *InputPath, const char *OutputPath)
{
    for (const fs::directory_entry &Entry : fs::directory_iterator(InputPath))
    {
        fs::path FilePath = Entry.path();
        fs::path FileName = Entry.path().stem();

        char AssetPath[256];
        FormatString(AssetPath, "%s/%s.font.asset", OutputPath, FileName.generic_string().c_str());

        printf("Processing %s...\n", FilePath.generic_string().c_str());
        ProcessFontAsset(FilePath.generic_string().c_str(), AssetPath);
    }
}

dummy_internal void
BuildAudioAssets(const char *InputPath, const char *OutputPath)
{
    for (const fs::directory_entry &Entry : fs::directory_iterator(InputPath))
    {
        fs::path FilePath = Entry.path();
        fs::path FileName = Entry.path().stem();

        char AssetPath[256];
        FormatString(AssetPath, "%s/%s.audio.asset", OutputPath, FileName.generic_string().c_str());

        printf("Processing %s...\n", FilePath.generic_string().c_str());
        ProcessAudioAsset(FilePath.generic_string().c_str(), AssetPath);
    }
}

dummy_internal void
BuildTextureAssets(const char *InputPath, const char *OutputPath)
{
    for (const fs::directory_entry &Entry : fs::directory_iterator(InputPath))
    {
        fs::path FilePath = Entry.path();
        fs::path FileName = Entry.path().stem();

        char AssetPath[256];
        FormatString(AssetPath, "%s/%s.texture.asset", OutputPath, FileName.generic_string().c_str());

        printf("Processing %s...\n", FilePath.generic_string().c_str());
        ProcessTextureAsset(FilePath.generic_string().c_str(), AssetPath);
    }
}

dummy_internal void
ClearFolder(const char *Path)
{
    for (const fs::directory_entry &Entry : fs::directory_iterator(Path))
    {
        fs::remove_all(Entry.path());
    }
}

i32 main(i32 ArgCount, char **Args)
{
    const char *GameAssetsPath = "game/assets";

    if (ArgCount == 1)
    {
        ClearFolder(GameAssetsPath);

        BuildModelAssets("assets/models", GameAssetsPath);
        BuildFontAssets("assets/fonts", GameAssetsPath);
        BuildAudioAssets("assets/audio", GameAssetsPath);
        BuildTextureAssets("assets/textures", GameAssetsPath);
    }
    else if (ArgCount == 2)
    {
        fs::path EntryPath = Args[1];
        fs::path EntryName = EntryPath.stem();

        // todo: extend to other asset types
        char AssetPath[256];
        FormatString(AssetPath, "%s/%s.model.asset", GameAssetsPath, EntryName.generic_string().c_str());

        printf("Processing %s...\n", EntryPath.generic_string().c_str());
        ProcessModelAsset(EntryPath.generic_string().c_str(), AssetPath);
    }
    else
    {
        printf("Usage: %s <asset:optional>", Args[0]);
    }

    return 1;
}
