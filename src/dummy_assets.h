#pragma once

#define GET_DATA_AT(Buffer, Offset, Type) (Type *) (((u8 *) Buffer) + (Offset))

enum material_property_type
{
    MaterialProperty_Float_Shininess,

    MaterialProperty_Float_Metalness,
    MaterialProperty_Float_Roughness,
    
    MaterialProperty_Color_Ambient,
    MaterialProperty_Color_Diffuse,
    MaterialProperty_Color_Specular,
    
    MaterialProperty_Texture_Diffuse,
    MaterialProperty_Texture_Specular,
    MaterialProperty_Texture_Shininess,

    MaterialProperty_Texture_Albedo,
    MaterialProperty_Texture_Metalness,
    MaterialProperty_Texture_Roughness,

    MaterialProperty_Texture_Normal
};

struct bitmap
{
    i32 Width;
    i32 Height;
    i32 Channels;
    bool32 IsHDR;
    void *Pixels;
};

enum material_shading_mode
{
    ShadingMode_Flat,
    ShadingMode_Phong,
    ShadingMode_PBR
};

struct material_property
{
    material_property_type Type;
    union
    {
        f32 Value;
        vec4 Color;
        struct
        {
            u32 TextureId;
            bitmap Bitmap;
        };
    };
};

struct mesh_material
{
    material_shading_mode ShadingMode;

    u32 PropertyCount;
    material_property *Properties;
};

struct mesh
{
    u32 MeshId;
    u32 MaterialIndex;
    bool32 Visible;

    u32 VertexCount;
    vec3 *Positions;
    vec3 *Normals;
    vec3 *Tangents;
    vec3 *Bitangents;
    vec2 *TextureCoords;
    vec4 *Weights;
    ivec4 *JointIndices;

    u32 IndexCount;
    u32 *Indices;
};

struct animation_graph_asset;

struct model
{
    char Key[64];
    u32 SkinningBufferId;

    aabb Bounds;
    obb BoundsOBB;

    skeleton *Skeleton;
    skeleton_pose *BindPose;
    animation_graph_asset *AnimationGraph;

    u32 MeshCount;
    mesh *Meshes;

    u32 MaterialCount;
    mesh_material *Materials;

    u32 AnimationCount;
    animation_clip *Animations;
};

struct animation_state_asset
{
    char AnimationClipName[256];
    bool32 IsLooping;
    bool32 EnableRootMotion;
};

struct additive_animation_asset
{
    char TargetClipName[256];
    char BaseClipName[256];
    u32 BaseKeyFrameIndex;
    bool32 IsLooping;
};

struct blend_space_1d_value_asset
{
    char AnimationClipName[256];
    f32 Value;
    bool32 EnableRootMotion;
};

struct animation_reference_asset
{
    char NodeName[256];
};

struct blend_space_1d_asset {
    u32 ValueCount;
    blend_space_1d_value_asset *Values;
};

struct animation_transition_asset
{
    char From[256];
    char To[256];

    animation_transition_type Type;
    union
    {
        f32 Duration;

        struct
        {
            f32 Duration;
            char TransitionNode[256];
        };
    };
};

struct animation_graph_asset;

struct animation_node_asset
{
    animation_node_type Type;
    char Name[256];

    union
    {
        animation_state_asset *Animation;
        blend_space_1d_asset *Blendspace;
        animation_reference_asset *Reference;
        animation_graph_asset *Graph;
    };

    u32 TransitionCount;
    animation_transition_asset *Transitions;

    u32 AdditiveAnimationCount;
    additive_animation_asset *AdditiveAnimations;
};

struct animation_graph_asset
{
    char Name[256];
    char Entry[256];
    char Animator[256];

    u32 NodeCount;
    animation_node_asset *Nodes;
};

struct model_asset
{
    aabb Bounds;
    obb BoundsOBB;

    skeleton Skeleton;
    skeleton_pose BindPose;
    animation_graph_asset AnimationGraph;

    u32 MeshCount;
    mesh *Meshes;

    u32 MaterialCount;
    mesh_material *Materials;

    u32 AnimationCount;
    animation_clip *Animations;
};

struct glyph
{
    vec2 SpriteSize;
    vec2 CharacterSize;
    vec2 UV;
    vec2 Alignment;
};

struct codepoints_range
{
    u32 Start;
    u32 End;
    u32 Count;
};

struct font_asset
{
    bitmap TextureAtlas;

    i32 VerticalAdvance;
    i32 Ascent;
    i32 Descent;

    u32 CodepointsRangeCount;
    codepoints_range *CodepointsRanges;

    u32 HorizontalAdvanceTableCount;
    f32 *HorizontalAdvanceTable;

    u32 GlyphCount;
    glyph *Glyphs;
};

struct font
{
    char Key[64];
    u32 TextureId;

    bitmap TextureAtlas;

    i32 VerticalAdvance;
    i32 Ascent;
    i32 Descent;

    u32 CodepointsRangeCount;
    codepoints_range *CodepointsRanges;

    // todo: should be per codepoint range?
    u32 HorizontalAdvanceTableCount;
    f32 *HorizontalAdvanceTable;

    u32 GlyphCount;
    glyph *Glyphs;
};

struct audio_clip
{
    char Key[64];

    u32 Format;
    u32 Channels;
    u32 SamplesPerSecond;
    u32 BitsPerSample;

    // Channel-interleaved samples
    u32 AudioBytes;
    u8 *AudioData;
};

struct audio_clip_asset
{
    u32 Format;
    u32 Channels;
    u32 SamplesPerSecond;
    u32 BitsPerSample;

    // Channel-interleaved samples
    u32 AudioBytes;
    u8 *AudioData;
};

struct texture
{
    char Key[64];
    u32 TextureId;

    bitmap Bitmap;
};

struct texture_asset
{
    bitmap Bitmap;
};

#pragma pack(push, 1)

enum asset_type
{
    AssetType_Model = 0x1,
    AssetType_Font = 0x2,
    AssetType_AudioClip = 0x3,
    AssetType_Texture = 0x4,
};

struct asset_header
{
    u32 MagicValue;
    u32 Version;
    u64 DataOffset;
    asset_type Type;
    char Description[32];
};

struct model_asset_header
{
    aabb Bounds;
    obb BoundsOBB;

    u64 SkeletonHeaderOffset;
    u64 SkeletonPoseHeaderOffset;
    u64 AnimationGraphHeaderOffset;
    u64 MeshesHeaderOffset;
    u64 MaterialsHeaderOffset;
    u64 AnimationsHeaderOffset;
};

struct model_asset_skeleton_header
{
    u32 JointCount;
    u64 JointsOffset;
};

struct model_asset_skeleton_pose_header
{
    u64 LocalJointPosesOffset;
    u64 GlobalJointPosesOffset;
};

struct model_asset_animation_graph_header
{
    char Name[256];
    char Entry[256];
    char Animator[256];
    u32 NodeCount;
    u64 NodesOffset;
};

struct model_asset_animation_node_header
{
    animation_node_type Type;
    char Name[256];

    u32 TransitionCount;
    u64 TransitionsOffset;

    u32 AdditiveAnimationCount;
    u64 AdditiveAnimationsOffset;

    u64 Offset;
};

struct model_asset_animation_state_header
{
    char AnimationClipName[256];
    bool32 IsLooping;
    bool32 EnableRootMotion;
};

struct model_asset_additive_animation_header
{
    char TargetClipName[256];
    char BaseClipName[256];
    u32 BaseFrameIndex;
    bool32 IsLooping;
};

struct model_asset_blend_space_1d_header
{
    u32 ValueCount;
    u64 ValuesOffset;
};

struct model_asset_animation_reference_header
{
    char NodeName[256];
};

struct model_asset_meshes_header
{
    u32 MeshCount;
    u64 MeshesOffset;
};

struct model_asset_mesh_header
{
    u32 MaterialIndex;
    u32 VertexCount;
    u32 IndexCount;

    bool32 HasPositions;
    bool32 HasNormals;
    bool32 HasTangents;
    bool32 HasBitangets;
    bool32 HasTextureCoords;
    bool32 HasWeights;
    bool32 HasJointIndices;

    u64 VerticesOffset;
    u64 IndicesOffset;
};

struct model_asset_materials_header
{
    u32 MaterialCount;
    u64 MaterialsOffset;
};

struct model_asset_material_header
{
    material_shading_mode ShadingMode;
    u32 PropertyCount;
    u64 PropertiesOffset;
};

struct model_asset_material_property_header
{
    material_property_type Type;
    union
    {
        f32 Value;
        vec4 Color;
        bitmap Bitmap;
    };
    u64 BitmapOffset;
};

struct model_asset_animations_header
{
    u32 AnimationCount;
    u64 AnimationsOffset;
};

struct model_asset_animation_header
{
    char Name[MAX_ANIMATION_NAME_LENGTH];
    f32 Duration;

    u32 PoseSampleCount;
    u64 PoseSamplesOffset;

    u32 EventCount;
    u64 EventsOffset;
};

struct model_asset_animation_sample_header
{
    u32 JointIndex;
    u32 KeyFrameCount;
    u64 KeyFramesOffset;
};

struct font_asset_header
{
    i32 TextureAtlasWidth;
    i32 TextureAtlasHeight;
    i32 TextureAtlasChannels;
    u64 TextureAtlasOffset;

    i32 VerticalAdvance;
    i32 Ascent;
    i32 Descent;

    u32 CodepointsRangeCount;
    u64 CodepointsRangesOffset;

    u32 HorizontalAdvanceTableCount;
    u64 HorizontalAdvanceTableOffset;

    u32 GlyphCount;
    u64 GlyphsOffset;
};

struct audio_clip_asset_header
{
    u32 Format;
    u32 Channels;
    u32 SamplesPerSecond;
    u32 BitsPerSample;
    u32 AudioBytes;
    u64 AudioDataOffset;
};

struct texture_asset_header
{
    i32 Width;
    i32 Height;
    i32 Channels;
    bool32 IsHDR;
    u64 PixelsOffset;
};

#pragma pack(pop)
