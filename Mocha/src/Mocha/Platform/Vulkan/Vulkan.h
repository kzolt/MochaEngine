#pragma once

#include <vulkan/vulkan.h>

#define VK_CHECK_RESULT(f)											             \
{																	             \
	VkResult res = (f);												             \
	if (res != VK_SUCCESS)											             \
	{																             \
		MC_CORE_ERROR("VkResult is '{0}' in {1}:{2}", res, __FILE__ , __LINE__); \
		MC_CORE_ASSERT(res == VK_SUCCESS);										 \
	}																			 \
}