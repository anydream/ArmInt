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

	struct SectionInfo
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

	struct SymtabInfo
	{
		uint32_t stName;
		uint8_t stInfo;
		uint8_t stOther;
		uint16_t stShNdx;
		uint64_t stValue;
		uint64_t stSize;

		const char *stNameStr;
	};

public:
	bool Load(BinReader &br);

	const ELFHeader& GetELFHeader() const;

	const SectionInfo& GetSectionInfo(size_t idx) const;
	size_t GetSectionInfoCount() const;

	const SymtabInfo& GetSymtabInfo(size_t idx) const;
	size_t GetSymtabInfoCount() const;

private:
	// 文件头
	ELFHeader Header_ = {};
	// 段列表
	std::unique_ptr<SectionInfo[]> SecInfoList_;
	size_t SecInfoCount_ = 0;
	// 符号列表
	std::unique_ptr<SymtabInfo[]> SymInfoList_;
	size_t SymInfoCount_ = 0;
};
