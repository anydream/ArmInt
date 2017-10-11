#pragma once

#include <stdint.h>

//////////////////////////////////////////////////////////////////////////
// 二进制数据读取器
class BinReader
{
public:
	explicit BinReader(const void *ptr, size_t len)
		: Buffer_(static_cast<const uint8_t*>(ptr))
		, Length_(len)
	{}

	template <class T>
	bool Read(T &result)
	{
		const size_t sz = sizeof(T);
		if (Offset_ + sz <= Length_)
		{
			result = *static_cast<const T*>(Buffer_ + Offset_);
			Offset_ += sz;
			return true;
		}
		return false;
	}

	template <class T>
	const T& Read()
	{
		const size_t sz = sizeof(T);
		if (Offset_ + sz <= Length_)
		{
			const size_t oldPos = Offset_;
			Offset_ += sz;
			return *static_cast<const T*>(Buffer_ + oldPos);
		}
		return T();
	}

	void Advance(size_t sz)
	{
		if (Offset_ + sz <= Length_)
			Offset_ += sz;
	}

	bool Seek(size_t pos)
	{
		if (pos <= Length_)
		{
			Offset_ = pos;
			return true;
		}
		return false;
	}

	size_t Tell() const
	{
		return Offset_;
	}

	size_t Remain() const
	{
		return Length_ - Offset_;
	}

	size_t Size() const
	{
		return Length_;
	}

private:
	const uint8_t *Buffer_ = nullptr;
	size_t Length_ = 0;
	size_t Offset_ = 0;
};
