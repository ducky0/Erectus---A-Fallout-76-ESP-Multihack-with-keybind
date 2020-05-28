#pragma once

#include <Windows.h>
#include "imgui.h"
#include <string>
#include <unordered_map>

class HotkeyListener
{
public:
	virtual void OnHotkeyPressed() = 0;
};

struct HotkeyButton
{
	HotkeyButton(UINT defaultKey, HotkeyListener* callback);

	UINT key;
	bool waitingForKey = false;
	HotkeyListener* cb;
};

static std::unordered_map<std::string, HotkeyButton> g_ExistingHotkeys;

namespace ImGui
{
	void HandleKeyPress(UINT key);

	void Hotkey(const char* id, const char* msg, UINT defaultKey, const ImVec2& size, HotkeyListener* cb);
}

/*** CPP FILE ***/


HotkeyButton::HotkeyButton(UINT defaultKey, HotkeyListener* callback)
	:
	key(defaultKey),
	cb(callback)
{}

void ImGui::HandleKeyPress(UINT key)
{
	for (auto& hotkey : g_ExistingHotkeys)
	{
		if (hotkey.second.waitingForKey)
		{
			hotkey.second.key = key;
			hotkey.second.waitingForKey = false;
		}
		else if (hotkey.second.key == key)
		{
			hotkey.second.cb->OnHotkeyPressed();
		}
	}
}

void ImGui::Hotkey(const char* id, const char* msg, UINT defaultKey, const ImVec2 & size, HotkeyListener* cb)
{
	auto pair = g_ExistingHotkeys.emplace(id, HotkeyButton{ defaultKey, cb });
	auto hotkey = pair.first;

	UINT key = hotkey->second.key;

	LONG sc = MapVirtualKey(key, 0);

	char buf[256];
	memset(buf, 0, 256);

	unsigned short int temp;
	int asc = (key <= 32) ? 1 : 0;
	if (!asc && key != VK_DIVIDE)
		asc = ToAscii(key, sc, reinterpret_cast<BYTE*>(buf), &temp, 1);

	// Set bits
	sc <<= 16;
	sc |= 0x1 << 25;  // <- don't care
	if (!asc)
		sc |= 0x1 << 24; // <- extended bit

						 // Convert to ANSI string
	if (GetKeyNameText(sc, buf, 256) > 0)
	{
		Text(msg);
		SameLine();
		if (Button(buf, size))
		{
			hotkey->second.waitingForKey = true;
		}
	}
}