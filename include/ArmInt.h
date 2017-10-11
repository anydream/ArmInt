#pragma once

#include <stdint.h>

//////////////////////////////////////////////////////////////////////////
// ARM 解释器
class ArmInt
{
public:
	ArmInt();
	~ArmInt();

	void Step();

private:
	void DispatchInst();

	struct OpCode
	{
		uint16_t Code;
		uint16_t Op1;
		uint16_t Op2;
		uint16_t Op3;
	};

	OpCode DecodeInst();
	OpCode DecodeThumb();
};
