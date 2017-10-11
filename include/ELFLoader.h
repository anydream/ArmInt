#pragma once

#include "ArmIntCfg.h"

class BinReader;

//////////////////////////////////////////////////////////////////////////
// ELF 文件加载器
class ELFLoader
{
public:
	struct ELFHeader
	{
		uint8_t eiMagic[4];
		uint8_t eiClass;
		uint8_t eiData;
		uint8_t eiVersion;
		uint8_t eiOsABI;
		uint8_t eiABIVersion;
		uint8_t eiPad[7];
		uint16_t eType;
		uint16_t eMachine;
		uint32_t eVersion;
		Ptr_t eEntry;
		Ptr_t ePHOff;
		Ptr_t eSHOff;
		uint32_t eFlags;
		uint16_t eEHSize;
		uint16_t ePHEntSize;
		uint16_t ePHNum;
		uint16_t eSHEntSize;
		uint16_t eSHNum;
		uint16_t eSHStrNdx;
	};

public:
	bool Load(BinReader &br);
};
