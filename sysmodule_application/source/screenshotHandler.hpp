#pragma once

#define SET_BIT(number, bit, loc) (number) ^= (-(unsigned long)(bit) ^ (number)) & (1UL << (loc))
#define GET_BIT(number, loc) ((number) >> (loc)) & 1U

#define JPEG_BUF_SIZE 0x80000

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <iterator>
#include <string>
#include <vector>

#ifdef __SWITCH__
#include <plog/Log.h>
#include <switch.h>
#endif

#include "../../sharedNetworkCode/networkInterface.hpp"

class ScreenshotHandler {
private:
#ifdef __SWITCH__
	Result rc;
#endif

public:
	ScreenshotHandler();

	void writeFramebuffer(std::vector<uint8_t>& buf);

	~ScreenshotHandler();
};