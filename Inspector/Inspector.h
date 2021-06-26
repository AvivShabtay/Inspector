#pragma once

#include <ntifs.h>

#pragma warning(push)
#pragma warning(disable:4201)       // unnamed struct/union

#include <fwpsk.h>

#pragma warning(pop)

#include <fwpmk.h>

#define INITGUID
#include <guiddef.h>

// Context structure to be associated with the filters
typedef struct FILTER_CONTEXT_
{
	PVOID memory;// Driver-specific content
} ISPECTOR_FILTER_CONTEXT, * PFILTER_CONTEXT;

const ULONG FILTER_CONTEXT_SIZE = sizeof(ISPECTOR_FILTER_CONTEXT);

#define INSPECTOR_POOL_TAG 'insp'
#define INSPECTOR_DEVICE 0x8000
#define INSPECTOR_DEVICE_NAME L"\\Device\\Inspector"
#define INSPECTOR_DEVICE_SYMBOLIC_LINK_PATH L"\\??\\Inspector"

// 7ec7f7f5-0c55-4121-adc5-5d07d2ac0cef
DEFINE_GUID(
	INSPECTOR_RECV_ACCEPT_CALLOUT_V4,
	0x7ec7f7f5,
	0x0c55,
	0x4121,
	0xad, 0xc5, 0x5d, 0x07, 0xd2, 0xac, 0x0c, 0xef
);

// Prototypes for the callout's callout functions
void classifyFn(
	IN const FWPS_INCOMING_VALUES0* inFixedValues,
	IN const FWPS_INCOMING_METADATA_VALUES0* inMetaValues,
	IN OUT VOID* layerData,
	IN const FWPS_FILTER0* filter,
	IN UINT64  flowContext,
	IN OUT FWPS_CLASSIFY_OUT0* classifyOut
);

NTSTATUS notifyFn(
	IN FWPS_CALLOUT_NOTIFY_TYPE notifyType,
	IN const GUID* filterKey,
	IN FWPS_FILTER0* filter
);
//VOID NTAPI FlowDeleteFn(IN UINT16  layerId, IN UINT32  calloutId, IN UINT64  flowContext);
VOID inspectorUnload(IN PDRIVER_OBJECT DriverObject);