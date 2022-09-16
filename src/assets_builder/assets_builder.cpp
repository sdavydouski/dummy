#define _CRT_SECURE_NO_WARNINGS

#include <string>
#include <vector>
#include <unordered_map>
#include <filesystem>
#include <stdlib.h>

#define STB_RECT_PACK_IMPLEMENTATION
#include "stb_rect_pack.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <assimp/cimport.h>        // Plain-C interface
#include <assimp/scene.h>          // Output data structure
#include <assimp/postprocess.h>    // Post processing flags

#include "rapidjson/document.h"

#include "dummy_defs.h"
#include "dummy_math.h"
#include "dummy_string.h"
#include "dummy_container.h"
#include "dummy_collision.h"
#include "dummy_animation.h"
#include "dummy_assets.h"

#include "assets_utils.cpp"
#include "assets_models.cpp"
#include "assets_fonts.cpp"

// https://nilooy.github.io/character-animation-combiner/
internal void
BuildModelAssets()
{
    char *Path = (char *) "models/";

    for (const fs::directory_entry &Entry : fs::directory_iterator(Path))
    {
        if (Entry.is_directory())
        {
            fs::path DirectoryPath = Entry.path();
            fs::path DirectoryName = Entry.path().filename();

            char FilePath[256];
            FormatString(FilePath, "%s/%s.fbx", DirectoryPath.generic_string().c_str(), DirectoryName.generic_string().c_str());

            char AnimationConfigPath[256];
            // todo: move to yaml configs
            FormatString(AnimationConfigPath, "%s/animation_graph.json", DirectoryPath.generic_string().c_str());

            char AnimationClipsPath[256];
            FormatString(AnimationClipsPath, "%s/clips", DirectoryPath.generic_string().c_str());

            char OutputPath[256];
            FormatString(OutputPath, "assets/%s.asset", DirectoryName.generic_string().c_str());

            // todo: multithreading (std::thread?)
            ProcessAsset(FilePath, AnimationConfigPath, AnimationClipsPath, OutputPath);
        }
    }
}

internal void
BuildFontAssets()
{
    char *Path = (char *) "fonts/";

    for (const fs::directory_entry &Entry : fs::directory_iterator(Path))
    {
        fs::path FilePath = Entry.path();
        fs::path FileName = Entry.path().stem();

        char OutputPath[256];
        FormatString(OutputPath, "assets/%s.asset", FileName.generic_string().c_str());

        // todo: multithreading (std::thread?)
        ProcessFont(FilePath.generic_string().c_str(), OutputPath);
    }
}

i32 main(i32 ArgCount, char **Args)
{
    BuildModelAssets();
    BuildFontAssets();
}
