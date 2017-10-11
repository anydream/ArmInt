#pragma once

#include "ArmIntCfg.h"

class BinReader;

//////////////////////////////////////////////////////////////////////////
// ELF 文件加载器
class ELFLoader
{
public:
	struct ELFPreHeader
	{
		uint8_t eiMagic[4];
		uint8_t eiClass;
		uint8_t eiData;
		uint8_t eiVersion;
		uint8_t eiOsABI;
		uint8_t eiABIVersion;
		uint8_t eiPad[7];
	};

	struct ELFHeader : ELFPreHeader
	{
		uint16_t eType;
		uint16_t eMachine;
		uint32_t eVersion;
		uint64_t eEntry;
		uint64_t ePhOff;
		uint64_t eShOff;
		uint32_t eFlags;
		uint16_t eEhSize;
		uint16_t ePhEntSize;
		uint16_t ePhNum;
		uint16_t eShEntSize;
		uint16_t eShNum;
		uint16_t eShStrNdx;
	};

public:
	bool Load(BinReader &br);

public:
	ELFHeader Header = {};
};
