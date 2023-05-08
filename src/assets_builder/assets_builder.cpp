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
            char FilePath[256];
            FormatString(FilePath, "%s/%s.fbx", EntryPath.generic_string().c_str(), EntryName.generic_string().c_str());

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

i32 main(i32 ArgCount, char **Args)
{
#if 1
    BuildModelAssets("assets/models", "game/assets");
    BuildFontAssets("assets/fonts", "game/assets");
    BuildAudioAssets("assets/audio", "game/assets");
    BuildTextureAssets("assets/textures", "game/assets");
#else
    ProcessModelAsset("assets\\models\\sphere_rust.gltf", "game\\assets\\sphere_rust.model.asset");
#endif
}
