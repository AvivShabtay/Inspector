#pragma once

#include "AutoBuffer.h"
#include "Value.h"

#include <ntifs.h>

class StringUtils final
{
public:
	static Value<AutoBuffer<UNICODE_STRING>> bufferToString(const PVOID address, ULONG length);
};

