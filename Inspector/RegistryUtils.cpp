#include "RegistryUtils.h"
#include "AutoHandle.h"
#include "Move.h"

Value<ULONG> RegistryUtils::getKeyInfoSize(HANDLE registryKey, PUNICODE_STRING valueName)
{
	ULONG keySize = 0;
	const NTSTATUS status = ZwQueryValueKey(registryKey, valueName, KeyValueFullInformation,
		nullptr, 0, &keySize);

	// ZwQueryValueKey fails with one of this statuses, if return size is needed:
	if (status == STATUS_BUFFER_TOO_SMALL || status == STATUS_BUFFER_OVERFLOW)
	{
		return keySize;
	}

	return STATUS_INVALID_BUFFER_SIZE;
}

Value<AutoBuffer<KEY_VALUE_FULL_INFORMATION>> RegistryUtils::getValueInformationFromKey(PUNICODE_STRING registryKey,
	PUNICODE_STRING valueName)
{
	if (nullptr == registryKey || nullptr == valueName || 0 == registryKey->Length || 0 == valueName->Length)
	{
		return STATUS_INVALID_PARAMETER;
	}

	OBJECT_ATTRIBUTES objectAttributes = { 0 };
	InitializeObjectAttributes(&objectAttributes, registryKey, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, nullptr, nullptr);

	HANDLE registryKeyHandle;

	NTSTATUS status = ZwOpenKey(&registryKeyHandle, KEY_QUERY_VALUE, &objectAttributes);
	if (!NT_SUCCESS(status))
	{
		return status;
	}

	AutoHandle registryHandle(registryKeyHandle);

	Value<ULONG> keyInfoSize = RegistryUtils::getKeyInfoSize(registryHandle.get(), valueName);
	if (keyInfoSize.isError())
	{
		return keyInfoSize.getError();
	}

	AutoBuffer<KEY_VALUE_FULL_INFORMATION > keyInformation(keyInfoSize.get());
	if (nullptr == keyInformation.get())
	{
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	ULONG keyInfoReturned = 0;

	status = ZwQueryValueKey(registryHandle.get(), valueName, KeyValueFullInformation,
		keyInformation.get(), keyInfoSize.get(), &keyInfoReturned);

	if (!NT_SUCCESS(status) || (keyInfoSize.get() != keyInfoReturned))
	{
		return status;
	}

	return move(keyInformation);
}
