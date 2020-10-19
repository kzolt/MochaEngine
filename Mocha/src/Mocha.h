#pragma once

#include "Mocha/Core/Base.h"
#include "Mocha/Core/Log.h"

#include "Mocha/Core/Application.h"

#include "Mocha/Core/Events/ApplicationEvent.h"
#include "Mocha/Core/Events/KeyEvent.h"
#include "Mocha/Core/Events/MouseEvent.h"

#ifdef MC_PLATFORM_WINDOWS
	#include "Mocha/Platform/Windows/WindowsWindow.h"
#endif

//////////////////////////////////////////////////
// Entry Point
//////////////////////////////////////////////////
#include "Mocha/Core/Entrypoint.h"