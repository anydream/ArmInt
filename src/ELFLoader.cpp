#include <malloc.h>
#include "ELFLoader.h"
#include "BinReader.h"

//////////////////////////////////////////////////////////////////////////
static bool ReadPointer(BinReader &br, uint64_t &result, bool is64Bit, bool isReverse)
{
	if (!is64Bit)
	{
		uint32_t ptr;
		if (!br.Read(ptr, isReverse))
			return false;
		result = ptr;
	}
	else
	{
		uint64_t ptr;
		if (!br.Read(ptr, isReverse))
			return false;
		result = ptr;
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool ELFLoader::Load(BinReader &br)
{
	// 读取文件头
	ELFHeaderBase &hdr = Header_;
	if (!br.Read(hdr))
		return false;

	if (hdr.eiMagic[0] != 0x7F ||
		hdr.eiMagic[1] != 0x45 ||
		hdr.eiMagic[2] != 0x4C ||
		hdr.eiMagic[3] != 0x46)
		return false;

	if (hdr.eiVersion != 1)
		return false;

	if (hdr.eiClass != 1 &&
		hdr.eiClass != 2)
		return false;

	if (hdr.eiData != 1 &&
		hdr.eiData != 2)
		return false;

	bool is64Bit = hdr.eiClass == 2;
	bool isBigEndian = hdr.eiData == 2;
	bool isReverse = isBigEndian != (!!HOST_BIG_ENDIAN);

	uint16_t eType;
	if (!br.Read(eType, isReverse))
		return false;

	uint16_t eMachine;
	if (!br.Read(eMachine, isReverse))
		return false;

	uint32_t eVersion;
	if (!br.Read(eVersion, isReverse))
		return false;

	if (eVersion != 1)
		return false;

	uint64_t eEntry;
	if (!ReadPointer(br, eEntry, is64Bit, isReverse))
		return false;

	uint64_t ePhOff;
	if (!ReadPointer(br, ePhOff, is64Bit, isReverse))
		return false;

	uint64_t eShOff;
	if (!ReadPointer(br, eShOff, is64Bit, isReverse))
		return false;

	uint32_t eFlags;
	if (!br.Read(eFlags, isReverse))
		return false;

	uint16_t eEhSize;
	if (!br.Read(eEhSize, isReverse))
		return false;

	if (eEhSize != (is64Bit ? 64 : 52))
		return false;

	uint16_t ePhEntSize;
	if (!br.Read(ePhEntSize, isReverse))
		return false;

	uint16_t ePhNum;
	if (!br.Read(ePhNum, isReverse))
		return false;

	uint16_t eShEntSize;
	if (!br.Read(eShEntSize, isReverse))
		return false;

	uint16_t eShNum;
	if (!br.Read(eShNum, isReverse))
		return false;

	uint16_t eShStrNdx;
	if (!br.Read(eShStrNdx, isReverse))
		return false;

	Header_.eType = eType;
	Header_.eMachine = eMachine;
	Header_.eVersion = eVersion;
	Header_.eEntry = eEntry;
	Header_.ePhOff = ePhOff;
	Header_.eShOff = eShOff;
	Header_.eFlags = eFlags;
	Header_.eEhSize = eEhSize;
	Header_.ePhEntSize = ePhEntSize;
	Header_.ePhNum = ePhNum;
	Header_.eShEntSize = eShEntSize;
	Header_.eShNum = eShNum;
	Header_.eShStrNdx = eShStrNdx;

	// 读取段信息
	if (!br.Seek(static_cast<size_t>(eShOff)))
		return false;

	SecInfoList_.reset(new SectionInfo[eShNum]);
	SecInfoCount_ = eShNum;

	uint8_t *secEntry = static_cast<uint8_t*>(alloca(eShEntSize));
	for (int i = 0; i < eShNum; ++i)
	{
		if (!br.ReadBytes(eShEntSize, secEntry))
			return false;

		BinReader secBr(secEntry, eShEntSize);

		uint32_t shName;
		if (!secBr.Read(shName, isReverse))
			return false;

		uint32_t shType;
		if (!secBr.Read(shType, isReverse))
			return false;

		uint64_t shFlags;
		if (!ReadPointer(secBr, shFlags, is64Bit, isReverse))
			return false;

		uint64_t shAddr;
		if (!ReadPointer(secBr, shAddr, is64Bit, isReverse))
			return false;

		uint64_t shOffset;
		if (!ReadPointer(secBr, shOffset, is64Bit, isReverse))
			return false;

		uint64_t shSize;
		if (!ReadPointer(secBr, shSize, is64Bit, isReverse))
			return false;

		uint32_t shLink;
		if (!secBr.Read(shLink, isReverse))
			return false;

		uint32_t shInfo;
		if (!secBr.Read(shInfo, isReverse))
			return false;

		uint64_t shAddrAlign;
		if (!ReadPointer(secBr, shAddrAlign, is64Bit, isReverse))
			return false;

		uint64_t shEntSize;
		if (!ReadPointer(secBr, shEntSize, is64Bit, isReverse))
			return false;

		SectionInfo &secInfo = SecInfoList_[i];
		secInfo.shName = shName;
		secInfo.shType = shType;
		secInfo.shFlags = shFlags;
		secInfo.shAddr = shAddr;
		secInfo.shOffset = shOffset;
		secInfo.shSize = shSize;
		secInfo.shLink = shLink;
		secInfo.shInfo = shInfo;
		secInfo.shAddrAlign = shAddrAlign;
		secInfo.shEntSize = shEntSize;
	}

	// 构造段信息数据
	const SectionInfo &secStrInfo = GetSectionInfo(eShStrNdx);
	for (int i = 0; i < eShNum; ++i)
	{
		SectionInfo &secInfo = SecInfoList_[i];
		if (secInfo.shOffset + secInfo.shSize > br.Size())
			return false;

		secInfo.shNameStr = reinterpret_cast<const char*>(br.Data(static_cast<size_t>(secStrInfo.shOffset + secInfo.shName)));
		secInfo.shDataPtr = br.Data(static_cast<size_t>(secInfo.shOffset));
	}

	// 构造符号表
	const char strSymtab[] = ".symtab";
	for (int i = 0; i < eShNum; ++i)
	{
		const SectionInfo &secInfo = SecInfoList_[i];
		if (strncmp(secInfo.shNameStr, strSymtab, sizeof(strSymtab)) != 0)
			continue;

		const size_t symInfoCount = static_cast<size_t>(secInfo.shSize / secInfo.shEntSize);
		SymInfoList_.reset(new SymtabInfo[symInfoCount]);
		SymInfoCount_ = symInfoCount;

		BinReader brSt(secInfo.shDataPtr, static_cast<size_t>(secInfo.shSize));
		for (size_t j = 0; j < symInfoCount; ++j)
		{
			SymtabInfo &symInfo = SymInfoList_[j];
			if (!is64Bit)
			{
				uint32_t stName;
				if (!brSt.Read(stName, isReverse))
					return false;

				uint32_t stValue;
				if (!brSt.Read(stValue, isReverse))
					return false;

				uint32_t stSize;
				if (!brSt.Read(stSize, isReverse))
					return false;

				uint8_t stInfo;
				if (!brSt.Read(stInfo))
					return false;

				uint8_t stOther;
				if (!brSt.Read(stOther))
					return false;

				uint16_t stShNdx;
				if (!brSt.Read(stShNdx, isReverse))
					return false;

				symInfo.stName = stName;
				symInfo.stInfo = stInfo;
				symInfo.stOther = stOther;
				symInfo.stShNdx = stShNdx;
				symInfo.stValue = stValue;
				symInfo.stSize = stSize;
			}
			else
			{
				uint32_t stName;
				if (!brSt.Read(stName, isReverse))
					return false;

				uint8_t stInfo;
				if (!brSt.Read(stInfo))
					return false;

				uint8_t stOther;
				if (!brSt.Read(stOther))
					return false;

				uint16_t stShNdx;
				if (!brSt.Read(stShNdx, isReverse))
					return false;

				uint64_t stValue;
				if (!brSt.Read(stValue, isReverse))
					return false;

				uint64_t stSize;
				if (!brSt.Read(stSize, isReverse))
					return false;

				symInfo.stName = stName;
				symInfo.stInfo = stInfo;
				symInfo.stOther = stOther;
				symInfo.stShNdx = stShNdx;
				symInfo.stValue = stValue;
				symInfo.stSize = stSize;
			}

			symInfo.stNameStr = reinterpret_cast<const char*>(br.Data(static_cast<size_t>(secStrInfo.shOffset + symInfo.stName)));
		}
		break;
	}

	return true;
}

const ELFLoader::ELFHeader& ELFLoader::GetELFHeader() const
{
	return Header_;
}

const ELFLoader::SectionInfo& ELFLoader::GetSectionInfo(size_t idx) const
{
	if (idx < SecInfoCount_)
		return SecInfoList_[idx];
	static SectionInfo sDummy = {};
	return sDummy;
}

size_t ELFLoader::GetSectionInfoCount() const
{
	return SecInfoCount_;
}

const ELFLoader::SymtabInfo& ELFLoader::GetSymtabInfo(size_t idx) const
{
	if (idx < SymInfoCount_)
		return SymInfoList_[idx];
	static SymtabInfo sDummy = {};
	return sDummy;
}

size_t ELFLoader::GetSymtabInfoCount() const
{
	return SymInfoCount_;
}
