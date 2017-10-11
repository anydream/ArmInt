#include "ELFLoader.h"
#include "BinReader.h"

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

bool ELFLoader::Load(BinReader &br)
{
	ELFPreHeader &hdr = Header;
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

	Header.eType = eType;
	Header.eMachine = eMachine;
	Header.eVersion = eVersion;
	Header.eEntry = eEntry;
	Header.ePhOff = ePhOff;
	Header.eShOff = eShOff;
	Header.eFlags = eFlags;
	Header.eEhSize = eEhSize;
	Header.ePhEntSize = ePhEntSize;
	Header.ePhNum = ePhNum;
	Header.eShEntSize = eShEntSize;
	Header.eShNum = eShNum;
	Header.eShStrNdx = eShStrNdx;

	return true;
}
