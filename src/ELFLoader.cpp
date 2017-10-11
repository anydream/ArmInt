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

	SecHdrList_.reset(new SecHeader[eShNum]);
	SecHdrCount_ = eShNum;

	uint8_t *secEntry = static_cast<uint8_t*>(alloca(eShEntSize));
	for (int i = 0; i < eShNum; ++i)
	{
		if (!br.ReadBytes(eShEntSize, secEntry))
			return false;

		BinReader secBr(secEntry, eShEntSize);

		uint32_t shName;
		if (!secBr.Read(shName))
			return false;

		uint32_t shType;
		if (!secBr.Read(shType))
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
		if (!secBr.Read(shLink))
			return false;

		uint32_t shInfo;
		if (!secBr.Read(shInfo))
			return false;

		uint64_t shAddrAlign;
		if (!ReadPointer(secBr, shAddrAlign, is64Bit, isReverse))
			return false;

		uint64_t shEntSize;
		if (!ReadPointer(secBr, shEntSize, is64Bit, isReverse))
			return false;

		SecHeader &secHdr = SecHdrList_[i];
		secHdr.shName = shName;
		secHdr.shType = shType;
		secHdr.shFlags = shFlags;
		secHdr.shAddr = shAddr;
		secHdr.shOffset = shOffset;
		secHdr.shSize = shSize;
		secHdr.shLink = shLink;
		secHdr.shInfo = shInfo;
		secHdr.shAddrAlign = shAddrAlign;
		secHdr.shEntSize = shEntSize;
	}

	return true;
}

const ELFLoader::ELFHeader& ELFLoader::GetELFHeader() const
{
	return Header_;
}

const ELFLoader::SecHeader& ELFLoader::GetSecHeader(size_t idx) const
{
	if (idx < SecHdrCount_)
		return SecHdrList_[idx];
	static SecHeader sDummy = {};
	return sDummy;
}
