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

i32 main(i32 ArgCount, char **Args)
{
    BuildModelAssets();
    BuildFontAssets();
}
