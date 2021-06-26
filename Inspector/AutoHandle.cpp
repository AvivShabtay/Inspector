#include "AutoHandle.h"
#include "DebugPrint.h"
#include "Move.h"

AutoHandle::AutoHandle(HANDLE handle)
	: m_handle(handle)
{
}

AutoHandle::~AutoHandle()
{
	__try
	{
		this->closeHandle();
	}
	__except (ExceptionContinueExecution)
	{
		KdPrint((STRINGIFY(~AutoHandle) "Exception thrown in destructor\n"));

	}
}

AutoHandle::AutoHandle(AutoHandle&& other) noexcept
	: m_handle(nullptr)
{
	this->copy(move(other));
}

AutoHandle& AutoHandle::operator=(AutoHandle&& other) noexcept
{
	this->closeHandle();

	this->copy(move(other));

	return *this;
}

const HANDLE& AutoHandle::get() const
{
	return this->m_handle;
}

HANDLE AutoHandle::get()
{
	return this->m_handle;
}

void AutoHandle::reset(HANDLE handle)
{
	this->closeHandle();

	this->m_handle = handle;
}

void AutoHandle::closeHandle() const
{
	if (nullptr != this->m_handle)
	{
		ZwClose(this->m_handle);
	}
}

void AutoHandle::copy(AutoHandle&& other)
{
	this->m_handle = other.m_handle;

	other.m_handle = nullptr;
}