#include "Inspector.h"

// Variable for the run-time callout identifier
UINT32 g_calloutId;
PDEVICE_OBJECT g_deviceObject;

NTSTATUS
DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPath)
{
	UNREFERENCED_PARAMETER(RegistryPath);

	KdPrint((STRINGIFY(InspectorDriverEntry) "started\n"));

	NTSTATUS status = STATUS_SUCCESS;
	UNICODE_STRING deviceName = RTL_CONSTANT_STRING(INSPECTOR_DEVICE_NAME);
	UNICODE_STRING symbolicLinkPath = RTL_CONSTANT_STRING(INSPECTOR_DEVICE_SYMBOLIC_LINK_PATH);
	bool symbolicLinkCreated = false;
	bool registerCallout = false;

	do
	{
		status = IoCreateDevice(DriverObject, 0, &deviceName, FILE_DEVICE_UNKNOWN,
			FILE_DEVICE_SECURE_OPEN, FALSE, &g_deviceObject);
		if (STATUS_SUCCESS != status)
		{
			KdPrint((STRINGIFY(InspectorDriverEntry) "Could not create device object\n"));
			break;
		}

		status = IoCreateSymbolicLink(&symbolicLinkPath, &deviceName);
		if (!NT_SUCCESS(status))
		{
			KdPrint((STRINGIFY(InspectorDriverEntry) "Could not create symbolic link\n"));
			break;
		}

		symbolicLinkCreated = true;

		// Callout registration structure
		FWPS_CALLOUT0 callout =
		{
			INSPECTOR_RECV_ACCEPT_CALLOUT_V4,
			0,
			(FWPS_CALLOUT_CLASSIFY_FN0)(classifyFn),
			(FWPS_CALLOUT_NOTIFY_FN0)(notifyFn),
			nullptr
		};

		status = FwpsCalloutRegister0(g_deviceObject, &callout, &g_calloutId);
		if (STATUS_SUCCESS != status)
		{
			KdPrint((STRINGIFY(InspectorDriverEntry) "Could not register callout\n"));
			break;
		}

		registerCallout = true;

	} while (false);

	if (STATUS_SUCCESS != status)
	{
		if (registerCallout)
		{
			FwpsCalloutUnregisterById(g_calloutId);
		}
		if (symbolicLinkCreated)
		{
			IoDeleteSymbolicLink(&symbolicLinkPath);
		}
		if (g_deviceObject)
		{
			IoDeleteDevice(g_deviceObject);
		}

		KdPrint((STRINGIFY(InspectorDriverEntry) "Driver entry failed with status: 0x%08X\n", status));

		return status;
	}

	// Specify the callout driver's Unload function
	DriverObject->DriverUnload = inspectorUnload;

	KdPrint((STRINGIFY(InspectorDriverEntry) "ended successfuly\n"));

	return status;
}

NTSTATUS notifyFn(
	IN FWPS_CALLOUT_NOTIFY_TYPE notifyType,
	IN const GUID* filterKey,
	IN FWPS_FILTER0* filter
)
{
	UNREFERENCED_PARAMETER(filterKey);

	KdPrint((STRINGIFY(InspectorNotifyFn) "started\n"));

	PFILTER_CONTEXT context;

	ASSERT(filter != NULL);

	KdPrint((STRINGIFY(InspectorNotifyFn) "Received notification of type: " STRINGIFY(FWPS_CALLOUT_NOTIFY_ADD_FILTER) "\n"));

	switch (notifyType)
	{
	case FWPS_CALLOUT_NOTIFY_ADD_FILTER:
	{
		// A filter is being added to the filter engine

		// Allocate the filter context structure
		context = static_cast<PFILTER_CONTEXT>(ExAllocatePoolWithTag(NonPagedPool, sizeof(ISPECTOR_FILTER_CONTEXT),
			INSPECTOR_POOL_TAG));
		if (nullptr == context)
		{
			KdPrint((STRINGIFY(InspectorNotifyFn) "Could not allocate callout context\n"));
			return STATUS_INSUFFICIENT_RESOURCES;
		}

		// Initialize the filter context structure
		context->memory = nullptr;

		// Associate the filter context structure with the filter
		filter->context = reinterpret_cast<UINT64>(context);

		break;
	}
	case FWPS_CALLOUT_NOTIFY_DELETE_FILTER:
	{
		// A filter is being removed from the filter engine

		// Get the filter context structure from the filter
		context = reinterpret_cast<PFILTER_CONTEXT>(filter->context);

		// Check whether the filter has a context
		if (nullptr != context)
		{
			// Cleanup the filter context structure
			context->memory = nullptr;

			// Free the memory for the filter context structure
			ExFreePoolWithTag(context, INSPECTOR_POOL_TAG);
		}

		break;
	}
	case FWPS_CALLOUT_NOTIFY_ADD_FILTER_POST_COMMIT: break;
	case FWPS_CALLOUT_NOTIFY_TYPE_MAX: break;
	default: break;
	}

	KdPrint((STRINGIFY(InspectorNotifyFn) "ended successfuly\n"));

	return STATUS_SUCCESS;
}

void classifyFn(
	IN const FWPS_INCOMING_VALUES0* inFixedValues,
	IN const FWPS_INCOMING_METADATA_VALUES0* inMetaValues,
	IN OUT VOID* layerData,
	IN const FWPS_FILTER0* filter,
	IN UINT64  flowContext,
	IN OUT FWPS_CLASSIFY_OUT0* classifyOut
)
{
	UNREFERENCED_PARAMETER(inFixedValues);
	UNREFERENCED_PARAMETER(inMetaValues);
	UNREFERENCED_PARAMETER(layerData);
	UNREFERENCED_PARAMETER(filter);
	UNREFERENCED_PARAMETER(flowContext);
	UNREFERENCED_PARAMETER(classifyOut);
}

VOID inspectorUnload(IN PDRIVER_OBJECT DriverObject)
{
	UNICODE_STRING symbolicLinkPath = RTL_CONSTANT_STRING(INSPECTOR_DEVICE_SYMBOLIC_LINK_PATH);
	NTSTATUS status = IoDeleteSymbolicLink(&symbolicLinkPath);
	if (STATUS_SUCCESS != status)
	{
		KdPrint((STRINGIFY(InspectorUnload) "Could not delete symbolic link, status: 0x%08X\n", status));
	}

	auto* device = DriverObject->DeviceObject;
	if (nullptr == device)
	{
		KdPrint((STRINGIFY(InspectorUnload) "Invalid device object\n"));
		return;
	}

	status = FwpsCalloutUnregisterById(g_calloutId);
	if (STATUS_DEVICE_BUSY == status)
	{
		// Finish unregistering the callout
		while (STATUS_DEVICE_BUSY == status)
		{
			status = FwpsCalloutUnregisterById(g_calloutId);
		}
	}

	// Handle error:
	if (status != STATUS_SUCCESS)
	{
		KdPrint((STRINGIFY(InspectorUnload) "Could not unregister callout\n"));
		return;
	}

	// Delete the device object
	IoDeleteDevice(device);
}