#pragma once

#include <memory>
#include "ArmIntCfg.h"

class BinReader;

//////////////////////////////////////////////////////////////////////////
// ELF 文件加载器
class ELFLoader
{
public:
	struct ELFHeaderBase
	{
		uint8_t eiMagic[4];
		uint8_t eiClass;
		uint8_t eiData;
		uint8_t eiVersion;
		uint8_t eiOsABI;
		uint8_t eiABIVersion;
		uint8_t eiPad[7];
	};

	struct ELFHeader : ELFHeaderBase
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

	struct SecHeader
	{
		uint32_t shName;
		uint32_t shType;
		uint64_t shFlags;
		uint64_t shAddr;
		uint64_t shOffset;
		uint64_t shSize;
		uint32_t shLink;
		uint32_t shInfo;
		uint64_t shAddrAlign;
		uint64_t shEntSize;

		const char *shNameStr;
		const uint8_t *shDataPtr;
	};

public:
	bool Load(BinReader &br);
	const ELFHeader& GetELFHeader() const;
	const SecHeader& GetSecHeader(size_t idx) const;
	size_t GetSecHeaderCount() const;

private:
	ELFHeader Header_ = {};
	std::unique_ptr<SecHeader[]> SecHdrList_;
	size_t SecHdrCount_ = 0;
};
