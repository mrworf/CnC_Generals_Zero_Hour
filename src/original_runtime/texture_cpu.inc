#include <cstring>
#include <stdexcept>

namespace {
unsigned next_texture_id = 0;
[[noreturn]] void unavailable()
{
	throw std::runtime_error("texture device operation requires an installed WW3D device translator");
}
}

TextureBaseClass::TextureBaseClass(unsigned width, unsigned height, MipCountType mip_level_count,
	PoolType pool, bool, bool reducible) :
	MipLevelCount(mip_level_count), Initialized(false), IsLightmap(false),
	IsCompressionAllowed(false), IsProcedural(false), IsReducible(reducible),
	InactivationTime(0), ExtendedInactivationTime(0), LastInactivationSyncTime(0),
	LastAccessed(0), HSVShift(0.0f, 0.0f, 0.0f), Width(width), Height(height),
	D3DTexture(nullptr), Name(""), FullPath(""), texture_id(next_texture_id++),
	Pool(pool), Dirty(false), TextureLoadTask(nullptr), ThumbnailLoadTask(nullptr)
{
}

TextureBaseClass::~TextureBaseClass()
{
	zh::original_runtime::OriginalGpuEdge::release_texture_if_owned(this);
	// CPU-only objects never create background/device tasks.
	WWASSERT(TextureLoadTask == nullptr && ThumbnailLoadTask == nullptr && D3DTexture == nullptr);
}

void TextureBaseClass::Set_Texture_Name(const char *name) { Name = name; }
unsigned int TextureBaseClass::Get_Priority() { return 0; }
unsigned int TextureBaseClass::Set_Priority(unsigned int) { return 0; }
IDirect3DBaseTexture8 *TextureBaseClass::Peek_D3D_Base_Texture() const { return D3DTexture; }
void TextureBaseClass::Set_D3D_Base_Texture(IDirect3DBaseTexture8 *tex)
{
	if (tex != nullptr) unavailable();
	D3DTexture = nullptr;
}
bool TextureBaseClass::Is_Missing_Texture()
{
	return zh::original_runtime::OriginalGpuEdge::is_missing_texture(this);
}
void TextureBaseClass::Invalidate()
{
	if (TextureLoadTask || ThumbnailLoadTask || IsProcedural) return;
	zh::original_runtime::OriginalGpuEdge::release_texture_if_owned(this);
	Initialized = false; D3DTexture = nullptr;
}
void TextureBaseClass::Set_HSV_Shift(const Vector3 &shift) { HSVShift = shift; Invalidate(); }
unsigned TextureBaseClass::Get_Reduction() const
{
	if (MipLevelCount == MIP_LEVELS_1 || Width <= 32 || Height <= 32) return 0;
	int reduction = WW3D::Get_Texture_Reduction();
	if (WW3D::Is_Large_Texture_Extra_Reduction_Enabled() && (Width > 256 || Height > 256))
		++reduction;
	if (MipLevelCount && reduction > MipLevelCount) reduction = MipLevelCount;
	return reduction;
}
void TextureBaseClass::Apply_Null(unsigned int stage)
{
	// Canonical texture.cpp clears the selected DX8 texture at this stage.
	zh::original_runtime::OriginalGpuEdge::required().select_texture(stage, nullptr);
}
int TextureBaseClass::_Get_Total_Locked_Surface_Size() { return 0; }
int TextureBaseClass::_Get_Total_Texture_Size() { return 0; }
int TextureBaseClass::_Get_Total_Lightmap_Texture_Size() { return 0; }
int TextureBaseClass::_Get_Total_Procedural_Texture_Size() { return 0; }
int TextureBaseClass::_Get_Total_Locked_Surface_Count() { return 0; }
int TextureBaseClass::_Get_Total_Texture_Count() { return 0; }
int TextureBaseClass::_Get_Total_Lightmap_Texture_Count() { return 0; }
int TextureBaseClass::_Get_Total_Procedural_Texture_Count() { return 0; }

TextureClass::TextureClass(const char *name, const char *full_path, MipCountType mip_count,
	WW3DFormat format, bool compression, bool reduction) :
	TextureBaseClass(0, 0, mip_count), TextureFormat(format), Filter(mip_count)
{
	IsCompressionAllowed = compression;
	IsReducible = reduction;
	InactivationTime = 20000;
	Set_Texture_Name(name);
	if (full_path != nullptr) Set_Full_Path(full_path);
	for (const char *p = name; p != nullptr && *p != '\0'; ++p) {
		if (*p == '+') { IsLightmap = true; break; }
	}
}

TextureClass::TextureClass(unsigned width, unsigned height, WW3DFormat format,
	MipCountType mip_count, PoolType pool, bool, bool reduction) :
	TextureBaseClass(width, height, mip_count, pool, false, reduction),
	TextureFormat(format), Filter(mip_count)
{
	IsProcedural = true;
}

void TextureClass::Init()
{
	if (Initialized) return;
	if (InactivationTime && LastInactivationSyncTime) {
		if ((WW3D::Get_Sync_Time()-LastInactivationSyncTime)<InactivationTime)
			ExtendedInactivationTime=3*InactivationTime;
		LastInactivationSyncTime=0;
	}
	if (!Peek_D3D_Base_Texture()) {
		if (!WW3D::Get_Thumbnail_Enabled() || MipLevelCount==MIP_LEVELS_1)
			TextureLoader::Request_Foreground_Loading(this);
		else
			throw std::runtime_error("original thumbnail load requires an unavailable Win32 mix-file route");
	}
	if (!Initialized) TextureLoader::Request_Background_Loading(this);
	LastAccessed=WW3D::Get_Sync_Time();
}
void TextureClass::Apply_New_Surface(IDirect3DBaseTexture8 *, bool, bool) { unavailable(); }
void TextureClass::Apply_Gpu_Texture(WW3DFormat format, unsigned width, unsigned height)
{
	TextureFormat=format; Width=width; Height=height; Initialized=true;
}
#include "texture_apply.inc"
unsigned TextureClass::Get_Texture_Memory_Usage() const { return 0; }
