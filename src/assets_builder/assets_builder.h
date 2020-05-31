#pragma once

#include "handmade_defs.h"
#include "handmade_math.h"
#include "handmade_renderer.h"
#include "handmade_assets.h"

#undef PI

#include <assimp/cimport.h>        // Plain-C interface
#include <assimp/scene.h>          // Output data structure
#include <assimp/postprocess.h>    // Post processing flags

#include <string>
#include <vector>
#include <unordered_map>

using std::string;

template <typename TValue>
using dynamic_array = std::vector<TValue>;

template <typename TKey, typename TValue>
using hashtable = std::unordered_map<TKey, TValue>;

struct assimp_node
{
    aiNode *Node;
    aiBone *Bone;
};
