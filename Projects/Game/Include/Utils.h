#pragma once
#include <filesystem>

namespace NS
{
	inline std::filesystem::path GetResourcePath(const std::string& ResourceRelativePath)
	{
		return std::filesystem::path(std::string(RESOURCE_DIR)) / ResourceRelativePath;
	}
}
