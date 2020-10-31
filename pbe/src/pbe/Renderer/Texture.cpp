#include "pch.h"
#include "Texture.h"


namespace pbe {

	Ref<Texture2D> Texture2D::Create(TextureFormat format, unsigned int width, unsigned int height, TextureWrap wrap)
	{

		return nullptr;
	}

	Ref<Texture2D> Texture2D::Create(const std::string& path, bool srgb)
	{

		return nullptr;
	}

	Ref<TextureCube> TextureCube::Create(TextureFormat format, uint32_t width, uint32_t height)
	{
		return nullptr;
	}

	Ref<TextureCube> TextureCube::Create(const std::string& path)
	{

		return nullptr;
	}

	uint32_t Texture::GetBPP(TextureFormat format)
	{
		switch (format)
		{
			case TextureFormat::RGB:    return 3;
			case TextureFormat::RGBA:   return 4;
		}
		return 0;
	}

	uint32_t Texture::CalculateMipMapCount(uint32_t width, uint32_t height)
	{
		uint32_t levels = 1;
		while ((width | height) >> levels)
			levels++;

		return levels;
	}

}
