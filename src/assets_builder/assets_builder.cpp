#define _CRT_SECURE_NO_WARNINGS

#include "assets_models.cpp"
#include "assets_fonts.cpp"
#include "assets_audio.cpp"
#include "assets_textures.cpp"

// https://nilooy.github.io/character-animation-combiner/
dummy_internal void
BuildModelAssets(const char *InputPath, const char *OutputPath)
{
    for (const fs::directory_entry &Entry : fs::directory_iterator(InputPath))
    {
        if (Entry.is_directory())
        {
            fs::path DirectoryPath = Entry.path();
            fs::path DirectoryName = Entry.path().filename();

            char FilePath[256];
            FormatString(FilePath, "%s/%s.fbx", DirectoryPath.generic_string().c_str(), DirectoryName.generic_string().c_str());

            char AnimationConfigPath[256];
            FormatString(AnimationConfigPath, "%s/animation_graph.json", DirectoryPath.generic_string().c_str());

            char AnimationClipsPath[256];
            FormatString(AnimationClipsPath, "%s/clips", DirectoryPath.generic_string().c_str());

            char AssetPath[256];
            FormatString(AssetPath, "%s/%s.model.asset", OutputPath, DirectoryName.generic_string().c_str());

            printf("Processing %s...\n", FilePath);
            ProcessModelAsset(FilePath, AnimationConfigPath, AnimationClipsPath, AssetPath);
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
#if 0
    BuildModelAssets("assets/models", "game/assets");
    BuildFontAssets("assets/fonts", "game/assets");
    BuildAudioAssets("assets/audio", "game/assets");
    BuildTextureAssets("assets/textures", "game/assets");
#else
    ProcessModelAsset("assets\\models\\ybot\\yBot.fbx", "assets\\models\\ybot\\animation_graph.json", "assets\\models\\ybot\\clips", "game\\assets\\ybot.model.asset");
#endif
}
