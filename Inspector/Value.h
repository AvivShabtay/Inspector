#pragma once

#include "Move.h"
#include <ntifs.h>

template <class Result, class Error = NTSTATUS>
class Value
{
public:
	Value(const Result& result) : m_result(result), m_error(), m_isError(false) { }
	Value(Result&& result) : m_result(move(result)), m_error(), m_isError(false) { }

	Value(const Error& error) : m_result(), m_error(error), m_isError(true) { }
	Value(Error&& error) : m_result(), m_error(move(error)), m_isError(true) { }

	bool isError() const
	{
		return this->m_isError;
	}

	const Result& get() const
	{
		return this->m_result;
	}

	Result get()
	{
		return this->m_result;
	}

	Error getError()
	{
		return this->m_error;
	}

private:
	Result m_result;
	Error m_error;
	bool m_isError;
};

