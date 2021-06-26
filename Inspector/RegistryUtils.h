#pragma once

#include "Value.h"
#include "AutoBuffer.h"

#include <ntifs.h>

class RegistryUtils final
{
public:
	static Value<ULONG> getKeyInfoSize(HANDLE registryKey, PUNICODE_STRING valueName);

	static Value<AutoBuffer<KEY_VALUE_FULL_INFORMATION>> getValueInformationFromKey(PUNICODE_STRING registryKey,
		PUNICODE_STRING valueName);
};

