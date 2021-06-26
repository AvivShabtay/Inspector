#include "StringUtils.h"
#include "Move.h"

Value<AutoBuffer<UNICODE_STRING>> StringUtils::bufferToString(const PVOID address, ULONG length)
{
	if (nullptr == address || 0 == length)
	{
		return STATUS_INVALID_PARAMETER;
	}

	AutoBuffer<UNICODE_STRING> buffer(length + 1); // '\0'
	if (nullptr == buffer.get())
	{
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	RtlZeroMemory(buffer.get(), length + 1);

	RtlCopyMemory(buffer.get(), address, length);

	return move(buffer);
}
