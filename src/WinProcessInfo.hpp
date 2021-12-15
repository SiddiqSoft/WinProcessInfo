/*
	WinProcessInfo
	Windows Process Information
	Repo: https://github.com/SiddiqSoft/WinProcessInfo

	BSD 3-Clause License

	Copyright (c) 2021, Siddiq Software LLC
	All rights reserved.

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:

	1. Redistributions of source code must retain the above copyright notice, this
	list of conditions and the following disclaimer.

	2. Redistributions in binary form must reproduce the above copyright notice,
	this list of conditions and the following disclaimer in the documentation
	and/or other materials provided with the distribution.

	3. Neither the name of the copyright holder nor the names of its
	contributors may be used to endorse or promote products derived from
	this software without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
	AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
	IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
	DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
	FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
	DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
	SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
	CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
	OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
	OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#ifndef WINPROCESSINFO_HPP
#define WINPROCESSINFO_HPP 1

#include <atomic>
#include <sstream>
#include <string>
#include <chrono>

#include <Windows.h>
#include <Psapi.h>
#include <tlhelp32.h>

#pragma comment(lib, "psapi.lib")

#if __cpp_lib_format
#include <format>
#endif

namespace siddiqsoft
{
	/// @brief Windows Process Info
	class WinProcessInfo
	{
	public:
		unsigned long                         cpuCores {0};
		unsigned long                         cpuHandles {0};
		unsigned long                         cpuThreads {0};
		size_t                                memPeakWorkingSet {0};
		size_t                                memWorkingSet {0};
		size_t                                memPrivate {0};
		std::string                           nameHostname {};
		std::string                           nameDomainName {};
		std::string                           nameHostnamePhysical {};
		std::string                           nameFqdn {};
		std::chrono::system_clock::time_point timeStartup {std::chrono::system_clock::now()};
		unsigned long                         processId {0};
		HANDLE                                processHandle {NULL};

	private:
		/// @brief Collects the memory information for this service process.
		/// http://msdn.microsoft.com/en-us/library/windows/desktop/ms682050(v=vs.85).aspx
		void getMemoryInfo()
		{
			PROCESS_MEMORY_COUNTERS_EX pmcInfo {};


			GetProcessMemoryInfo(processHandle, (PROCESS_MEMORY_COUNTERS*)&pmcInfo, sizeof(pmcInfo));
			memPeakWorkingSet = pmcInfo.PeakWorkingSetSize / 1024;
			memWorkingSet     = pmcInfo.WorkingSetSize / 1024;
			memPrivate        = pmcInfo.PrivateUsage / 1024;

			cpuHandles = 0;
			GetProcessHandleCount(processHandle, &cpuHandles);
		}


		/// @brief Get the current thread count
		void getCurrentThreadCount()
		{
			HANDLE         snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPALL, 0);
			PROCESSENTRY32 entry    = {0};
			entry.dwSize            = sizeof(entry);

			// get the first process info.
			BOOL ret = Process32First(snapshot, &entry);
			while (ret && entry.th32ProcessID != processId)
			{
				ret = Process32Next(snapshot, &entry);
			}

			CloseHandle(snapshot);

			cpuThreads = ret ? entry.cntThreads : 0;
		}


	public:
		WinProcessInfo()
		{
			SYSTEM_INFO sysInfo {};

			// Store one-time information
			processId = GetCurrentProcessId();
			GetSystemInfo(&sysInfo);
			cpuCores = sysInfo.dwNumberOfProcessors;

			processHandle = GetCurrentProcess();

			// Get the hostname
			DWORD dwSize {MAX_COMPUTERNAME_LENGTH};
			nameHostname.resize(MAX_COMPUTERNAME_LENGTH);
			GetComputerNameExA(COMPUTER_NAME_FORMAT::ComputerNameDnsHostname, nameHostname.data(), &dwSize);
			nameHostname.resize(dwSize);

			// Get the hostname(fully qualified)
			dwSize = MAX_COMPUTERNAME_LENGTH;
			nameFqdn.resize(MAX_COMPUTERNAME_LENGTH);
			GetComputerNameExA(COMPUTER_NAME_FORMAT::ComputerNameDnsFullyQualified, nameFqdn.data(), &dwSize);
			nameFqdn.resize(dwSize);

			// Get the local hostname(fully qualified)
			dwSize = MAX_COMPUTERNAME_LENGTH;
			nameHostnamePhysical.resize(MAX_COMPUTERNAME_LENGTH);
			GetComputerNameExA(COMPUTER_NAME_FORMAT::ComputerNamePhysicalDnsFullyQualified, nameHostnamePhysical.data(), &dwSize);
			nameHostnamePhysical.resize(dwSize);

			// Get the domain name
			dwSize = MAX_COMPUTERNAME_LENGTH;
			nameDomainName.resize(MAX_COMPUTERNAME_LENGTH);
			GetComputerNameExA(COMPUTER_NAME_FORMAT::ComputerNameDnsDomain, nameDomainName.data(), &dwSize);
			nameDomainName.resize(dwSize);
		}

		/// @brief Cleans up the process handle
		~WinProcessInfo()
		{
			if (processHandle != NULL) CloseHandle(processHandle);
		}

		/// @brief Collect a snapshot of the memory, handle, thread count.
		/// Avoid calling this method as it is expensive. Use a low-priority background thread to measure these stats.
		void snapshot()
		{
			getMemoryInfo();
			getCurrentThreadCount();
		}

		/// @brief Calculate the uptime for this application.
		/// @return system_clock::duration. Cast appropriately via duration_cast<>
		std::chrono::system_clock::duration uptime() const { return std::chrono::system_clock::now() - timeStartup; }
	};


#if defined INCLUDE_NLOHMANN_JSON_HPP_
	/// @brief Serializer for nlohmann::json library
	/// @param dest The destination target
	/// @param wpi Our source object
	static void to_json(nlohmann::json& dest, const WinProcessInfo& wpi)
	{
		dest = nlohmann::json
		{
			{"processId", wpi.processId}, {"hostname", wpi.nameHostname}, {"fqdn", wpi.nameFqdn}, {"domain", wpi.nameDomainName},
					 {"localFqdn", wpi.nameHostnamePhysical},
					{"cpuHandles", wpi.cpuHandles}, {"cpuThreads", wpi.cpuThreads},
					{"cpuCores", wpi.cpuCores}, {"memPeakWorkingSet", wpi.memPeakWorkingSet}, {"memWorkingSet", wpi.memWorkingSet},
					{"memPrivateBytes", wpi.memPrivate},
#if __cpp_lib_format
					{"timeStartup", std::format("{:%FT%T}Z", wpi.timeStartup)},
					{"timeCurrent", std::format("{:%FT%T}Z", std::chrono::system_clock::now())},
#endif
			{
				"uptime", std::chrono::duration_cast<std::chrono::microseconds>(wpi.uptime()).count()
			}
		};
	}
#endif
} // namespace siddiqsoft


#if defined __cpp_lib_format
template <> struct std::formatter<siddiqsoft::WinProcessInfo> : std::formatter<std::string>
{
	auto format(const siddiqsoft::WinProcessInfo& wpi, std::format_context& ctx)
	{
#if defined INCLUDE_NLOHMANN_JSON_HPP_
		return std::formatter<std::string>::format(nlohmann::json(wpi).dump(), ctx);
#else
		std::string s;

		// First element
		std::format_to(std::back_inserter(s), "{{\"processId\":{}", wpi.processId);
		std::format_to(std::back_inserter(s), ",\"hostname\":\"{}\",", wpi.nameHostname);
		std::format_to(std::back_inserter(s), ",\"domain\":\"{}\",", wpi.nameDomainName);
		std::format_to(std::back_inserter(s), ",\"fqdn\":\"{}\",", wpi.nameFqdn);
		std::format_to(std::back_inserter(s), ",\"localFqdn\":\"{}\",", wpi.nameHostnamePhysical);
		std::format_to(std::back_inserter(s), ",\"cpuHandles\":{},", wpi.cpuHandles);
		std::format_to(std::back_inserter(s), ",\"cpuThreads\":{},", wpi.cpuThreads);
		std::format_to(std::back_inserter(s), ",\"cpuCores\":{},", wpi.cpuCores);
		std::format_to(std::back_inserter(s), ",\"memPeakWorkingSet\":{},", wpi.memPeakWorkingSet);
		std::format_to(std::back_inserter(s), ",\"memWorkingSet\":{},", wpi.memWorkingSet);
		std::format_to(std::back_inserter(s), ",\"memPrivateBytes\":{},", wpi.memPrivate);
		std::format_to(std::back_inserter(s), ",\"timeStartup\":\"{:%FT%T}Z\"", wpi.timeStartup);
		std::format_to(std::back_inserter(s), ",\"timeCurrent\":\"{:%FT%T}Z\"", std::chrono::system_clock::now());
		// last element
		std::format_to(std::back_inserter(s),
		               ",\"uptime\":{}}}",
		               std::chrono::duration_cast<std::chrono::microseconds>(wpi.uptime()).count());
		return std::formatter<std::string>::format(s, ctx);
#endif
	}
};

#endif

#endif // __WINPROCESSINFO__HPP__
