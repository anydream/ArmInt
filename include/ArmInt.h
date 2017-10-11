#pragma once

#include "ArmIntCfg.h"

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
		uint8_t Code;
		uint8_t Op1;
		union
		{
			struct
			{
				uint8_t Op2;
				uint8_t Op3;
			};
			uint16_t OpA;
		};
		union
		{
			struct
			{
				uint8_t Op4;
				uint8_t Op5;
			};
			uint16_t OpB;
		};
	};

	OpCode DecodeInst();
	OpCode DecodeThumb();
};
