#pragma once

#ifdef MC_DEBUG
	#define MC_ENABLE_ASSERTS
#endif

#ifdef MC_ENABLE_ASSERTS
	#define MC_ASSERT_NO_MESSAGE(condition) { if(!(condition)) {MC_ERROR("Assertion Failed"); __debugbreak(); } }
	#define MC_ASSERT_MESSAGE(condition, ...) { if(!(condition)) {MC_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak;}}

	#define MC_ASSERT_RESOLVE(arg1, arg2, macro, ...) macro
	#define MC_GET_ASSERT_MACRO(...) MC_EXPAND_VARGS(MC_ASSERT_RESOLVE(__VA_ARGS__, MC_ASSERT_MESSAGE, MC_ASSERT_NO_MESSAGE))

	#define MC_ASSERT(...) MC_EXPAND_VARGS(MC_GET_ASSERT_MACRO(__VA_ARGS__)(__VA_ARGS__))
	#define MC_CORE_ASSERT(...) MC_EXPAND_VARGS(MC_GET_ASSERT_MACRO(__VA_ARGS__)(__VA_ARGS__))

#else
	#define MC_ASSERT(...)
	#define MC_CORE_ASSERT(...)
#endif