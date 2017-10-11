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

	template <class T, bool isReverse = false>
	bool Read(T &result)
	{
		const size_t sz = sizeof(T);
		if (Offset_ + sz <= Length_)
		{
			result = *reinterpret_cast<const T*>(Buffer_ + Offset_);

			if (isReverse)
				ReverseBytes<sz>(reinterpret_cast<uint8_t*>(&result));

			Offset_ += sz;
			return true;
		}
		return false;
	}

	template <class T>
	bool ReadReverse(T &result)
	{
		return Read<T, true>(result);
	}

	template <class T>
	bool Read(T &result, bool isReverse)
	{
		if (!isReverse)
			return Read(result);
		else
			return ReadReverse(result);
	}

	template <class T, bool isReverse = false>
	const T& Read()
	{
		const size_t sz = sizeof(T);
		if (Offset_ + sz <= Length_)
		{
			const size_t oldPos = Offset_;
			Offset_ += sz;

			if (isReverse)
			{
				T result = *reinterpret_cast<const T*>(Buffer_ + oldPos);
				ReverseBytes<sz>(reinterpret_cast<uint8_t*>(&result));
				return result;
			}
			return *static_cast<const T*>(Buffer_ + oldPos);
		}
		return T();
	}

	template <class T>
	const T& ReadReverse()
	{
		return Read<T, true>();
	}

	template <class T>
	const T& ReadReverse(bool isReverse)
	{
		if (!isReverse)
			return Read<T>();
		else
			return ReadReverse<T>();
	}

	template <size_t Size>
	static void ReverseBytes(uint8_t *data)
	{
		for (size_t i = 0; i < Size / 2; ++i)
			data[i] = data[Size - 1 - i];
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
