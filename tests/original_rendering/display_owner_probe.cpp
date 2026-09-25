#include "PreRTS.h"
#include "GameLogic/GameLogic.h"
#include "W3DDevice/GameClient/W3DDisplay.h"
#include "W3DDevice/GameClient/W3DScene.h"
#include "W3DDevice/GameClient/W3DAssetManager.h"
#include "W3DDevice/GameClient/BaseHeightMap.h"
#include "W3DDevice/GameClient/Module/W3DTreeDraw.h"
#include "Common/FileSystem.h"
#include "WW3D2/assetmgr.h"
#include "WW3D2/mesh.h"
#include "WW3D2/meshmdl.h"
#include "WW3D2/ww3d.h"
#include "sphere.h"
#include "WWLib/RAMFILE.H"
#include "original_gpu_edge.h"
#include "zh/platform/bgfx_device.h"
#include "zh/renderer/recording_device.h"

#include <cstdio>
#include <cmath>
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
void require(bool value, const char* message)
{
    if (!value) throw std::runtime_error(message);
}
}

extern "C" void zh_probe_display_owner()
{
    require(TheGameLogic && TheGameLogic->isInGame(),
        "original display owner requires initialized GameClient scenario");
    require(!W3DDisplay::m_3DScene && !W3DDisplay::m_2DScene &&
        !W3DDisplay::m_3DInterfaceScene && !W3DDisplay::m_assetManager,
        "original display owner slots already occupied");
    const char* packet_path = std::getenv("ZH_M22_DISPLAY_OWNER_ASSET");
    require(packet_path, "original display owner packet path missing");
    std::ifstream input(packet_path, std::ios::binary);
    require(input.good(), "original display owner packet unreadable");
    std::vector<char> bytes(std::istreambuf_iterator<char>{input}, std::istreambuf_iterator<char>{});
    require(!bytes.empty(), "original display owner packet empty");
    const bool physical = std::getenv("ZH_M22_DISPLAY_OWNER_PHYSICAL") != nullptr;
    auto run = [&](zh::renderer::GpuDevice& device) {
        auto display = std::make_unique<W3DDisplay>();
        bool no_edge = false;
        try { display->init(); }
        catch (const std::runtime_error& error) {
            no_edge = std::string(error.what()).find("edge") != std::string::npos;
        }
        require(no_edge && !W3DDisplay::m_3DScene && !W3DDisplay::m_assetManager,
            "original display missing-edge init published owners");
        bool no_reset = false;
        try { display->reset(); }
        catch (const std::runtime_error&) { no_reset = true; }
        require(no_reset, "original display reset before bootstrap succeeded");
        display->doSmartAssetPurgeAndPreload("Maps\\Owned\\AssetUsage.txt");
        {
            zh::original_runtime::OriginalGpuEdge edge(device);
            display->init();
            auto* scene3d = W3DDisplay::m_3DScene;
            auto* scene2d = W3DDisplay::m_2DScene;
            auto* interface_scene = W3DDisplay::m_3DInterfaceScene;
            auto* assets = W3DDisplay::m_assetManager;
            require(scene3d && scene2d && interface_scene && assets && WW3D::Is_Initted(),
                "original display init omitted source owners");
            display->init();
            require(W3DDisplay::m_3DScene == scene3d && W3DDisplay::m_2DScene == scene2d &&
                W3DDisplay::m_3DInterfaceScene == interface_scene &&
                W3DDisplay::m_assetManager == assets,
                "original display re-entry replaced source owners");
            bool duplicate = false;
            try { W3DDisplay second; }
            catch (const std::runtime_error&) { duplicate = true; }
            require(duplicate && W3DDisplay::m_3DScene == scene3d,
                "original display duplicate owner construction changed slots");
            bool draw_pending = false;
            try { display->draw(); }
            catch (const std::runtime_error&) { draw_pending = true; }
            require(draw_pending, "original display draw unexpectedly succeeded");
            auto load_packet = [&] {
                RAMFileClass packet(bytes.data(), static_cast<int>(bytes.size()));
                require(static_cast<WW3DAssetManager*>(assets)->Load_3D_Assets(packet),
                    "original display asset manager rejected generated packet");
            };
            auto has_prototype = [&](const char* name) {
                return assets->Find_Prototype(name) != nullptr;
            };
            FileSystem *saved_file_system = TheFileSystem;
            TheFileSystem = nullptr;
            bool missing_provider = false;
            try { display->doSmartAssetPurgeAndPreload("Maps\\Owned\\AssetUsage.txt"); }
            catch (const std::runtime_error&) { missing_provider = true; }
            TheFileSystem = saved_file_system;
            require(missing_provider && W3DDisplay::m_assetManager == assets,
                "original display purge accepted a missing source file-system provider");
            W3DDisplay::m_assetManager = nullptr;
            bool missing_assets = false;
            try { display->doSmartAssetPurgeAndPreload("Maps\\Owned\\AssetUsage.txt"); }
            catch (const std::runtime_error&) { missing_assets = true; }
            W3DDisplay::m_assetManager = assets;
            require(missing_assets, "original display purge accepted a broken published asset owner");

            load_packet();
            require(has_prototype("TEST.ZERO01") && has_prototype("ALT0.ZERO01"),
                "generated display purge prototypes missing");
            W3DTreeDrawModuleData tree_data;
            tree_data.m_modelName = "TEST.ZERO01";
            W3DTreeModelSource tree_model;
            require(tree_model.acquire(&tree_data, assets) && tree_model.mesh() &&
                tree_model.mesh()->Num_Refs() == 1 && tree_model.boundsRadius() > 0 &&
                tree_model.shadowSize() > 0,
                "original tree direct mesh model candidate missing");
            require(!tree_model.acquire(&tree_data, assets),
                "original tree candidate replaced a live source mesh reference");
            tree_model.reset();
            require(!tree_model.mesh() && tree_model.boundsRadius() == 0 &&
                !tree_model.acquire(&tree_data, nullptr),
                "original tree model reset or missing provider retained a reference");
            tree_data.m_modelName = "TEST.MISSING";
            require(!tree_model.acquire(&tree_data, assets) && !tree_model.mesh(),
                "original tree missing asset published a model");
            tree_data.m_modelName = "TEST.EMPTYHLOD";
            RenderObjClass *empty_hlod = assets->Create_Render_Obj("TEST.EMPTYHLOD");
            require(empty_hlod && empty_hlod->Class_ID() == RenderObjClass::CLASSID_HLOD &&
                empty_hlod->Get_Num_Sub_Objects() == 0,
                "generated empty tree HLOD did not load");
            empty_hlod->Release_Ref();
            require(!tree_model.acquire(&tree_data, assets) && !tree_model.mesh(),
                "original tree empty HLOD published a model");
            tree_data.m_modelName = "TEST.NESTEDHLOD";
            RenderObjClass *nested_hlod = assets->Create_Render_Obj("TEST.NESTEDHLOD");
            require(nested_hlod && nested_hlod->Class_ID() == RenderObjClass::CLASSID_HLOD &&
                nested_hlod->Get_Num_Sub_Objects() == 1,
                "generated nested tree HLOD did not load");
            nested_hlod->Release_Ref();
            require(!tree_model.acquire(&tree_data, assets) && !tree_model.mesh(),
                "original tree non-mesh first child published a model");
            tree_data.m_modelName = "TEST.HLOD";
            setenv("ZH_M22_TREE_MODEL_FAIL_AT", "root", 1);
            require(!tree_model.acquire(&tree_data, assets) && !tree_model.mesh(),
                "original tree root-reference fault published a model");
            setenv("ZH_M22_TREE_MODEL_FAIL_AT", "child", 1);
            require(!tree_model.acquire(&tree_data, assets) && !tree_model.mesh(),
                "original tree HLOD child-reference fault published a model");
            unsetenv("ZH_M22_TREE_MODEL_FAIL_AT");
            require(tree_model.acquire(&tree_data, assets) && tree_model.mesh() &&
                tree_model.mesh()->Num_Refs() == 1 && tree_model.boundsRadius() > 0 &&
                tree_model.shadowSize() > 0,
                "original tree HLOD first-child source model missing");
            RenderObjClass *source_hlod = assets->Create_Render_Obj("TEST.HLOD");
            require(source_hlod && source_hlod->Get_Num_Sub_Objects() > 0,
                "generated tree HLOD reference missing first child");
            AABoxClass source_box;
            source_hlod->Get_Obj_Space_Bounding_Box(source_box);
            RenderObjClass *source_child = source_hlod->Get_Sub_Object(0);
            require(source_child && source_child->Class_ID() == RenderObjClass::CLASSID_MESH,
                "generated tree HLOD first child is not source mesh");
            Vector3 source_offset;
            source_child->Get_Bone_Transform(0).Get_Translation(&source_offset);
            auto *source_mesh = static_cast<MeshClass *>(source_child);
            auto *source_model = source_mesh->Peek_Model();
            SphereClass source_bounds(source_model->Get_Vertex_Array(),
                source_model->Get_Vertex_Count());
            source_bounds.Center += source_offset;
            require((tree_model.offset() - source_offset).Length() < 0.001f &&
                (tree_model.boundsCenter() - source_bounds.Center).Length() < 0.001f &&
                std::fabs(tree_model.boundsRadius() - source_bounds.Radius) < 0.001f &&
                std::fabs(tree_model.shadowSize() -
                    (source_box.Extent.X + source_box.Extent.Y)) < 0.001f,
                "original tree HLOD offset, bounds or shadow size differ from source");
            source_child->Release_Ref();
            source_hlod->Release_Ref();
            tree_model.reset();
            W3DAssetManager *published_assets = W3DDisplay::m_assetManager;
            W3DDisplay::m_assetManager = nullptr;
            require(!tree_model.acquire(&tree_data, assets) && !tree_model.mesh(),
                "original tree accepted detached asset provider");
            W3DDisplay::m_assetManager = published_assets;
            require(tree_model.acquire(&tree_data, assets) && tree_model.mesh(),
                "original tree model retry lost the source HLOD");
            tree_model.reset();
            W3DTreeAtlasSource tree_atlas;
            const std::vector<AsciiString> tree_textures = {
                "TreeA.tga", "TreeB.tga", "treea.tga"};
            require(!tree_atlas.prepare(tree_textures, nullptr) &&
                tree_atlas.pixels().empty(),
                "original tree atlas accepted absent file-system provider");
            FileSystem *published_files = TheFileSystem;
            TheFileSystem = nullptr;
            require(!tree_atlas.prepare(tree_textures, published_files) &&
                tree_atlas.pixels().empty(),
                "original tree atlas accepted detached file-system provider");
            TheFileSystem = published_files;
            require(tree_atlas.prepare(tree_textures, published_files) &&
                tree_atlas.width() == 512 && tree_atlas.slots().size() == 3 &&
                tree_atlas.pixels().size() == 512u * 512u * 4u,
                "original tree generated atlas was not prepared");
            const auto &tree_slots = tree_atlas.slots();
            require(tree_slots[0].firstTile == 0 && tree_slots[0].tileWidth == 2 &&
                tree_slots[0].numTiles == 4 && tree_slots[0].origin.x == 0 &&
                tree_slots[0].origin.y == 0 && tree_slots[1].firstTile == 4 &&
                tree_slots[1].tileWidth == 1 && tree_slots[1].numTiles == 1 &&
                tree_slots[1].origin.x == 128 && tree_slots[1].origin.y == 0 &&
                tree_slots[2].numTiles == 0 && tree_slots[2].tileWidth == 2 &&
                tree_slots[2].origin.x == 0 && tree_slots[2].origin.y == 0,
                "original tree atlas source packing or alias identity changed");
            auto tree_pixel = [&](Int x, Int y) {
                return tree_atlas.pixels().data() +
                    (static_cast<std::size_t>(y) * tree_atlas.width() + x) * 4;
            };
            require(std::memcmp(tree_pixel(0, 0), "\x0b\x0a\x09\x0c", 4) == 0 &&
                std::memcmp(tree_pixel(0, 64), "\x03\x02\x01\x04", 4) == 0 &&
                std::memcmp(tree_pixel(64, 0), "\x0f\x0e\x0d\x10", 4) == 0 &&
                std::memcmp(tree_pixel(128, 0), "\x17\x16\x15\xff", 4) == 0 &&
                std::memcmp(tree_pixel(192, 0), "\0\0\0\0", 4) == 0,
                "original tree atlas source TGA format or vertical orientation changed");
            const std::vector<AsciiString> missing_tree_texture = {"TreeMissing.tga"};
            const std::vector<AsciiString> bad_tree_texture = {"TreeBad.tga"};
            const std::vector<AsciiString> truncated_tree_texture = {"TreeTrunc.tga"};
            require(!tree_atlas.prepare(missing_tree_texture, published_files) &&
                !tree_atlas.prepare(bad_tree_texture, published_files) &&
                !tree_atlas.prepare(truncated_tree_texture, published_files) &&
                tree_atlas.width() == 512 && tree_atlas.slots().size() == 3,
                "original tree atlas failure replaced accepted candidate");
            const std::vector<AsciiString> one_tree_texture = {"TreeA.tga"};
            for (const char *fault : {"read", "tile", "pack", "pixels"}) {
                setenv("ZH_M22_TREE_ATLAS_FAIL_AT", fault, 1);
                require(!tree_atlas.prepare(one_tree_texture, published_files) &&
                    tree_atlas.width() == 512 && tree_atlas.slots().size() == 3,
                    "original tree atlas injected fault replaced accepted candidate");
            }
            unsetenv("ZH_M22_TREE_ATLAS_FAIL_AT");
            std::vector<AsciiString> capacity_textures;
            for (Int index = 0; index < 6; ++index) {
                AsciiString name;
                name.format("TreeCap%d.tga", index);
                capacity_textures.push_back(name);
            }
            W3DTreeAtlasSource wide_atlas;
            std::vector<AsciiString> five_capacity_textures(
                capacity_textures.begin(), capacity_textures.begin() + 5);
            require(wide_atlas.prepare(five_capacity_textures, published_files) &&
                wide_atlas.width() == 2048 && wide_atlas.slots().size() == 5,
                "original tree atlas rejected valid 500-tile source budget");
            require(!wide_atlas.prepare(capacity_textures, published_files) &&
                wide_atlas.width() == 2048 && wide_atlas.slots().size() == 5,
                "original tree atlas capacity failure replaced accepted wide candidate");
            wide_atlas.reset();
            require(!tree_atlas.prepare(capacity_textures, published_files) &&
                tree_atlas.width() == 512 && tree_atlas.slots().size() == 3,
                "original tree atlas exceeded the 512-tile budget");
            tree_atlas.reset();
            require(tree_atlas.width() == 0 && tree_atlas.slots().empty() &&
                tree_atlas.pixels().empty() &&
                tree_atlas.prepare({"TreeHalf.tga"}, published_files) &&
                tree_atlas.slots().size() == 1 && tree_atlas.slots()[0].halfTile &&
                std::memcmp(tree_pixel(0, 0), "\0\0\0\0", 4) == 0 &&
                std::memcmp(tree_pixel(0, 32), "\x1f\x20\x21\x22", 4) == 0,
                "original tree half-tile decode or reset retry changed");
            tree_atlas.reset();
            display->doSmartAssetPurgeAndPreload(nullptr);
            display->doSmartAssetPurgeAndPreload("");
            require(has_prototype("TEST.ZERO01") && has_prototype("ALT0.ZERO01"),
                "original display empty usage name changed assets");
            display->doSmartAssetPurgeAndPreload("Maps\\Owned\\AssetUsage.txt");
            require(has_prototype("TEST.ZERO01") && !has_prototype("ALT0.ZERO01"),
                "original display usage list did not preserve exactly the named source assets");
            display->doSmartAssetPurgeAndPreload("Maps\\Owned\\MissingAssetUsage.txt");
            require(!has_prototype("TEST.ZERO01") && !has_prototype("ALT0.ZERO01"),
                "original display absent usage list did not purge all source assets");
            load_packet();
            require(has_prototype("TEST.ZERO01") && has_prototype("ALT0.ZERO01"),
                "original display purge retry did not reload source assets");
            display->doSmartAssetPurgeAndPreload("Maps\\Owned\\CommentsOnly.txt");
            require(!has_prototype("TEST.ZERO01") && !has_prototype("ALT0.ZERO01"),
                "original display comment tokens entered the exclusion list");
            load_packet();
            auto* mesh = assets->Create_Render_Obj("TEST.ZERO01");
            auto* second_mesh = assets->Create_Render_Obj("TEST.ZERO01");
            require(mesh && second_mesh && mesh != second_mesh &&
                mesh->Num_Refs() == 1 && second_mesh->Num_Refs() == 1,
                "original display independent created meshes missing");
            scene3d->Add_Render_Object(mesh);
            scene3d->Add_Render_Object(second_mesh);
            require(mesh->Num_Refs() == 2 && second_mesh->Num_Refs() == 2,
                "original display scene refs missing");
            display->reset();
            require(mesh->Num_Refs() == 1 && second_mesh->Num_Refs() == 1 &&
                !mesh->Peek_Scene() && !second_mesh->Peek_Scene() &&
                W3DDisplay::m_3DScene == scene3d,
                "original display reset failed to detach objects while preserving owners");
            display->reset();
            mesh->Release_Ref();
            second_mesh->Release_Ref();
            display.reset();
            require(!W3DDisplay::m_3DScene && !W3DDisplay::m_2DScene &&
                !W3DDisplay::m_3DInterfaceScene && !W3DDisplay::m_assetManager &&
                !WW3D::Is_Initted(), "original display teardown retained source owners");
        }
    };
    if (physical) {
        for (int generation = 0; generation != 2; ++generation) {
            zh::renderer::BgfxOptions options;
            options.shader_root = ZH_BGFX_SHADER_DIR;
            zh::renderer::BgfxGpuDevice device(options);
            run(device);
            require(device.live_resource_count() == 0,
                "original display bgfx teardown retained resources");
        }
    } else {
        for (int generation = 0; generation != 2; ++generation) {
            zh::renderer::RecordingGpuDevice device;
            run(device);
            require(device.resource_counts().total() == 0,
                "original display Recording teardown retained resources");
        }
    }
    require(!W3DDisplay::m_3DScene && !W3DDisplay::m_2DScene &&
        !W3DDisplay::m_3DInterfaceScene && !W3DDisplay::m_assetManager && !WW3D::Is_Initted(),
        "original display teardown retained owners");
    std::puts(physical ? "original display owner: bgfx generations=2 resources=0" :
        "original display owner: source generations=2 resources=0");
}
