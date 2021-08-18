#define WIN32_LEAN_AND_MEAN
#define FMT_HEADER_ONLY

#include <fmt/core.h>
#include <fmt/color.h>

#include "discord_register.h"
#include "discord_rpc.h"

#include "offsets.hpp"
#include "memory.hpp"
#include "utilities.hpp"
#include "state.hpp"

#include "lazy_importer.hpp"
#include "skCrypter.h"

static int64_t startTime;

static void handleDiscordReady(const DiscordUser* connectedUser)
{
	fmt::print(fg(fmt::color::light_green) | fmt::emphasis::bold, skCrypt("\n[+] connected to discord: {}#{} ({})").decrypt(), connectedUser->username, connectedUser->discriminator, connectedUser->userId);
}

static void handleDiscordDisconnected(int errcode, const char* message)
{
	fmt::print(fg(fmt::color::light_green) | fmt::emphasis::bold, skCrypt("\n[-] disconnected from discord ({}): {}").decrypt(), errcode, message);
}

static void handleDiscordError(int errcode, const char* message)
{
	fmt::print(fg(fmt::color::light_green) | fmt::emphasis::bold, skCrypt("\n[-] error from discord ({}): {}").decrypt(), errcode, message);
}

__forceinline auto discordInit() -> void
{
	DiscordEventHandlers handlers;
	memset(&handlers, 0, sizeof(handlers));
	handlers.ready = handleDiscordReady;
	handlers.disconnected = handleDiscordDisconnected;
	handlers.errored = handleDiscordError;
	Discord_Initialize(skCrypt("877608785122824202"), &handlers, 1, NULL);
	startTime = time(0);
}

__forceinline auto discordRPC() -> void
{
	DiscordRichPresence discordPresence;
	memset(&discordPresence, 0, sizeof(discordPresence));
	discordPresence.state = skCrypt("Cheating in CS:GO");
	discordPresence.largeImageKey = skCrypt("logo");
	discordPresence.startTimestamp = startTime;
	discordPresence.instance = 0;
	Discord_UpdatePresence(&discordPresence);
}

auto main() -> int
{
	LI_FN(LoadLibraryA).get()(skCrypt("user32.dll").decrypt());
	LI_FN(LoadLibraryA).get()(skCrypt("kernel32.dll").decrypt());

	LI_FN(SetConsoleTitle).get()(skCrypt("privacier - open-source kernel-mode cheat"));
	HANDLE hOut = LI_FN(GetStdHandle).get()(STD_OUTPUT_HANDLE);
	DWORD dwMode = 0;
	LI_FN(GetConsoleMode).get()(hOut, &dwMode);
	dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
	LI_FN(SetConsoleMode).get()(hOut, dwMode);

	auto logoText = skCrypt(R"(
privacier (cs:go kernel-mode cheat)
made by raywave
	)");
	fmt::print(fg(fmt::color::snow) | fmt::emphasis::bold, logoText.decrypt());
	logoText.clear();
	DWORD pId = utilities::getPid(skCrypt("csgo.exe"));
	if (!pId)
	{
		auto pIdText = skCrypt("\n[-] pid not found, please, start cs:go.");
		fmt::print(fg(fmt::color::crimson) | fmt::emphasis::bold, pIdText.decrypt());
		pIdText.clear();

		while (!pId)
		{
			pId = utilities::getPid(skCrypt("csgo.exe"));
			LI_FN(Sleep).get()(128);
		}
	}

	fmt::print(fg(fmt::color::light_green) | fmt::emphasis::bold, skCrypt("\n[+] found pid: {0:x}").decrypt(), pId);

	if (!memory::initialize(pId))
	{
		fmt::print(fg(fmt::color::crimson) | fmt::emphasis::bold, skCrypt("\n[-] failed to initialize a driver, try re-map driver.").decrypt());
		LI_FN(Sleep).get()(5420);
		return 0xDEAD;
	}

	DWORD client = memory::getModuleAddress();

	if (!client)
	{
		fmt::print(fg(fmt::color::crimson) | fmt::emphasis::bold, skCrypt("\n[-] failed to find module, retrying...").decrypt());

		while (!client)
		{
			client = memory::getModuleAddress();
			LI_FN(Sleep).get()(198);
		}
	}

	bool bStopHack = false;
	DWORD dwCurrentEntity;
	float flSensorTime;

	fmt::print(fg(fmt::color::light_green) | fmt::emphasis::bold, skCrypt("\n[+] found module: {0:x}").decrypt(), client);

	fmt::print(fg(fmt::color::light_green) | fmt::emphasis::bold, skCrypt("\n[+] cheat is working now.").decrypt());
	fmt::print(fg(fmt::color::snow) | fmt::emphasis::bold, skCrypt("\n[+] keybinds: f1 - glow / f2 - radar / f3 - bhop / end - exit.").decrypt());

	discordInit();

	while (!bStopHack)
	{
		static bool glowJustToggled = false;
		if (LI_FN(GetAsyncKeyState).safe_cached()(VK_F1) & 1) [[unlikely]]
		{
			state::glow = !state::glow;
			glowJustToggled = true;
		}
		else if (LI_FN(GetAsyncKeyState).safe_cached()(VK_F2) & 1) [[unlikely]]
		{
			state::radar = !state::radar;
		}
		else if (LI_FN(GetAsyncKeyState).safe_cached()(VK_F3) & 1) [[unlikely]]
		{
			state::bhop = !state::bhop;
		}
		else if (LI_FN(GetAsyncKeyState).safe_cached()(VK_END) & 1) [[unlikely]]
		{
			bStopHack = 1;
		}

		for (int i = 0; i < 65; i++)
		{
			DWORD dwCurrentEntity = memory::read<DWORD>((client + hazedumper::signatures::dwEntityList + (i * 0x10)));
			if (memory::isvalidptr(dwCurrentEntity)) [[unlikely]]
			{
				flSensorTime = (bStopHack || !state::glow) ? 0.f : 86400.f;
				if (state::radar) memory::write<bool>(dwCurrentEntity + hazedumper::netvars::m_bSpotted, true);
				if (state::glow || glowJustToggled) memory::write<float>(dwCurrentEntity + hazedumper::netvars::m_flDetectedByEnemySensorTime, flSensorTime);
			}
		}

		if (state::bhop)
		{
			DWORD dwLocalPlayer = memory::read<DWORD>((client + hazedumper::signatures::dwLocalPlayer));
			if (memory::isvalidptr(dwLocalPlayer)) [[likely]]
			{
				DWORD m_fFlags = memory::read<ULONG>(dwLocalPlayer + hazedumper::netvars::m_fFlags);
				if ((LI_FN(GetAsyncKeyState).safe_cached()(VK_SPACE) & 0x8000) && ((m_fFlags & 1) == 1)) [[unlikely]]
				{
					memory::write<int>(client + hazedumper::signatures::dwForceJump, 0x5);
					LI_FN(Sleep).safe_cached()(5);
					memory::write<int>(client + hazedumper::signatures::dwForceJump, 0x4);
				}
			}
		}

		glowJustToggled = false;
		discordRPC();

		Discord_RunCallbacks();

		LI_FN(Sleep).safe_cached()(5);
	}

	Discord_Shutdown();

	return 0xDEAD;
}