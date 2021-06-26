#pragma once

#include <ntifs.h>

class AutoHandle
{
public:
	explicit AutoHandle(HANDLE handle);

	virtual ~AutoHandle();

	AutoHandle(AutoHandle&& other) noexcept;

	AutoHandle& operator=(AutoHandle&& other) noexcept;

	// Disable: copyable:
	AutoHandle(AutoHandle& other) = delete;
	AutoHandle& operator=(AutoHandle& other) = delete;

	const HANDLE& get() const;

	HANDLE get();

	void reset(HANDLE handle);

private:
	void closeHandle() const;

	void copy(AutoHandle&& other);

	HANDLE m_handle;
};

