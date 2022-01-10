#pragma once
#include <Windows.h>

namespace TitleBarTheme {
	enum class PreferredAppMode : int {
		Default = 0,
		AllowDark,
		ForceDark,
		ForceLight,
		Max
	};

	enum class WINDOWCOMPOSITIONATTRIB {
		UNDEFINED = 0,
		NCRENDERING_ENABLED = 1,
		NCRENDERING_POLICY = 2,
		TRANSITIONS_FORCEDISABLED = 3,
		ALLOW_NCPAINT = 4,
		CAPTION_BUTTON_BOUNDS = 5,
		NONCLIENT_RTL_LAYOUT = 6,
		FORCE_ICONIC_REPRESENTATION = 7,
		EXTENDED_FRAME_BOUNDS = 8,
		HAS_ICONIC_BITMAP = 9,
		THEME_ATTRIBUTES = 10,
		NCRENDERING_EXILED = 11,
		NCADORNMENTINFO = 12,
		EXCLUDED_FROM_LIVEPREVIEW = 13,
		VIDEO_OVERLAY_ACTIVE = 14,
		FORCE_ACTIVEWINDOW_APPEARANCE = 15,
		DISALLOW_PEEK = 16,
		CLOAK = 17,
		CLOAKED = 18,
		ACCENT_POLICY = 19,
		FREEZE_REPRESENTATION = 20,
		EVER_UNCLOAKED = 21,
		VISUAL_OWNER = 22,
		HOLOGRAPHIC = 23,
		EXCLUDED_FROM_DDA = 24,
		PASSIVEUPDATEMODE = 25,
		USEDARKMODECOLORS = 26,
		LAST = 27
	};

	struct WINDOWCOMPOSITIONATTRIBDATA {
		WINDOWCOMPOSITIONATTRIB Attrib;
		PVOID pvData;
		SIZE_T cbData;
	};

	using fnAllowThemeForWindow = BOOL(WINAPI*)(HWND hWnd, BOOL allow);
	using fnSetPreferredAppMode = PreferredAppMode(WINAPI*)(PreferredAppMode appMode);
	using fnSetWindowCompAttr = BOOL(WINAPI*)(HWND hwnd, WINDOWCOMPOSITIONATTRIBDATA*);

	inline void update(HWND hwnd)
	{
		static HMODULE hUxtheme = LoadLibraryExW(L"uxtheme.dll", NULL, LOAD_LIBRARY_SEARCH_SYSTEM32);
		static HMODULE hUser32 = GetModuleHandleW(L"user32.dll");
		static auto allowThemeForWindow = (fnAllowThemeForWindow)GetProcAddress(hUxtheme, MAKEINTRESOURCEA(133));
		static auto setPreferredAppMode = (fnSetPreferredAppMode)GetProcAddress(hUxtheme, MAKEINTRESOURCEA(135));
		static auto setWindowCompAttr = (fnSetWindowCompAttr)GetProcAddress(hUser32, "SetWindowCompositionAttribute");

		setPreferredAppMode(PreferredAppMode::AllowDark);

		BOOL dark = TRUE;
		allowThemeForWindow(hwnd, dark);

		WINDOWCOMPOSITIONATTRIBDATA data {
			WINDOWCOMPOSITIONATTRIB::USEDARKMODECOLORS,
			&dark,
			sizeof(dark)
		};
		
		setWindowCompAttr(hwnd, &data);
	}
}