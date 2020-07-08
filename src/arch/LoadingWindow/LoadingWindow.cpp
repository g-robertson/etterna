#include "Etterna/Globals/global.h"
#include "LoadingWindow.h"
#include "Etterna/Singletons/PrefsManager.h"
#include "RageUtil/Misc/RageLog.h"
#include "arch/arch_default.h"

#include <Tracy.hpp>

LoadingWindow*
LoadingWindow::Create()
{
	ZoneScoped;

	if (!PREFSMAN->m_bShowLoadingWindow)
		return new LoadingWindow_Null;
#if defined(__unix__) && !defined(HAVE_GTK)
	return new LoadingWindow_Null;
#else
	// Don't load NULL by default.
	const std::string drivers = "win32,macosx,gtk";
	vector<std::string> DriversToTry;
	split(drivers, ",", DriversToTry, true);

	ASSERT(DriversToTry.size() != 0);

	std::string Driver;
	LoadingWindow* ret = nullptr;

	for (unsigned i = 0; ret == nullptr && i < DriversToTry.size(); ++i) {
		Driver = DriversToTry[i];

#ifdef USE_LOADING_WINDOW_MACOSX
		if (!CompareNoCase(DriversToTry[i], "MacOSX")) {
			if (ret != nullptr)
				delete ret;
			ret = new LoadingWindow_MacOSX;
		}
#endif
#ifdef USE_LOADING_WINDOW_GTK
		if (!CompareNoCase(DriversToTry[i], "Gtk")) {
			if (ret != nullptr)
				delete ret;
			ret = new LoadingWindow_Gtk;
		}
#endif
#ifdef USE_LOADING_WINDOW_WIN32
		if (!CompareNoCase(DriversToTry[i], "Win32")) {
			if (ret != nullptr)
				delete ret;
			ret = new LoadingWindow_Win32;
		}

#endif
		if (!CompareNoCase(DriversToTry[i], "Null")) {
			if (ret != nullptr)
				delete ret;
			ret = new LoadingWindow_Null;
		}

		if (ret == nullptr)
			continue;

		std::string sError = ret->Init();
		if (sError != "") {
			LOG->Info("Couldn't load driver %s: %s",
					  DriversToTry[i].c_str(),
					  sError.c_str());
			SAFE_DELETE(ret);
		}
	}

	if (ret) {
		if (PREFSMAN->m_verbose_log > 1)
			LOG->Info("Loading window: %s", Driver.c_str());

		ret->SetIndeterminate(true);
	}

	return ret;
#endif
}
