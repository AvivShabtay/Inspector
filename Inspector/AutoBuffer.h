#pragma once

#include "DriverSettings.h"
#include "DebugPrint.h"

#include <ntifs.h>

template<class Type>
class AutoBuffer
{
public:
	explicit AutoBuffer(ULONG bufferSize = sizeof(Type))
		:m_size(bufferSize), m_buffer(nullptr)
	{
		this->allocate();
	}

	AutoBuffer(AutoBuffer&& other) noexcept
	{
		this->copy(move(other));
	}

	AutoBuffer& operator=(AutoBuffer&& other) noexcept
	{
		this->release();
		this->copy(move(other));

		return *this;
	}

	AutoBuffer(AutoBuffer& other)
	{
		this->clone(other);
	}

	AutoBuffer& operator=(const AutoBuffer& other)
	{
		this->release();
		this->clone(other);

		return *this;
	}

	virtual ~AutoBuffer()
	{
		__try
		{
			this->release();
		}
		__except (ExceptionContinueExecution)
		{
			KdPrint((STRINGIFY(~AutoBuffer) "Exception thrown in destructor\n"));

		}
	}

	const Type& get() const
	{
		return static_cast<Type*>(this->m_buffer);
	}

	Type* get()
	{
		return static_cast<Type*>(this->m_buffer);
	}

	void reset(ULONG bufferSize = sizeof(Type))
	{
		this->release();

		this->m_size = bufferSize;
		this->allocate();
	}

private:
	void allocate()
	{
		this->m_buffer = ExAllocatePoolWithTag(NonPagedPool, this->m_size, DriverSettings::DRIVER_TAG);
	}

	void release()
	{
		if (nullptr != this->m_buffer)
		{
			ExFreePoolWithTag(this->m_buffer, DriverSettings::DRIVER_TAG);
			this->m_buffer = nullptr;
		}
	}

	void copy(AutoBuffer&& other)
	{
		this->m_size = other.m_size;
		this->m_buffer = other.m_buffer;

		other.m_size = 0;
		other.m_buffer = nullptr;
	}

	void clone(AutoBuffer& other)
	{
		this->m_size = other.m_size;

		this->allocate();

		memcpy(this->m_buffer, other.m_buffer, this->m_size);
	}

	ULONG m_size;
	PVOID m_buffer;
};

