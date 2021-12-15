/*
	WinProcessInfo : tests
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

#include "gtest/gtest.h"
#include <format>
#include "nlohmann/json.hpp"
#include "../src/WinProcessInfo.hpp"


TEST(examples, Example1)
{
	try
	{
		siddiqsoft::WinProcessInfo procInfo;

		procInfo.snapshot();
		EXPECT_EQ(GetCurrentProcessId(), procInfo.processId);
	}
	catch (...)
	{
		EXPECT_TRUE(false); // if we throw then the test fails.
	}
}

TEST(examples, Example2)
{
	try
	{
		siddiqsoft::WinProcessInfo procInfo;

		// We must perform the snapshot to obtain memory and thread usage
		procInfo.snapshot();

		// tip: do not use brace-init/assignment as it will create an array instead of object!
		nlohmann::json info(procInfo);

		std::cerr << info.dump(4) << std::endl;
		EXPECT_EQ(GetCurrentProcessId(), info.value("processId", 0));
		std::cerr << std::format("{} - Contents:{}\n", __func__, procInfo);
	}
	catch (std::exception& e)
	{
		EXPECT_TRUE(false) << e.what(); // if we throw then the test fails.
	}
}

TEST(examples, Example3)
{
	try
	{
		siddiqsoft::WinProcessInfo procInfo;

		// We must perform the snapshot to obtain memory and thread usage
		procInfo.snapshot();

		std::cerr << nlohmann::json(procInfo).dump(4) << std::endl;
		EXPECT_EQ(GetCurrentProcessId(), nlohmann::json(procInfo).value("processId", 0));
		std::cerr << std::format("{} - Contents:{}\n", __func__, procInfo);
	}
	catch (std::exception& e)
	{
		EXPECT_TRUE(false) << e.what(); // if we throw then the test fails.
	}
}
