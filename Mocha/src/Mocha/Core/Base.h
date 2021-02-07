#pragma once

#include <memory>
#include "Mocha/Core/Ref.h"

#define MC_EXPAND_VARGS(x) x

#define BIT(x) (1 << x)

#define MC_BIND_EVENT_FN(fn) std::bind(&##fn, this, std::placeholder::_1)

#include "Mocha/Core/Assert.h"
#include "Mocha/Core/Log.h"

namespace Mocha {

	template<typename T>
	using Scope = std::unique_ptr<T>;

	template<typename T, typename ... Args>
	constexpr Scope<T> CreateScope(Args&& ... args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

	using byte = uint8_t;

}