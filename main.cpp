#if defined(_DEBUG) && defined(_WIN32) && !defined(_WIN64)
#include <vld.h>
#endif

#include <assert.h>
#include <memory>
#include "ArmInt.h"
#include "BinReader.h"
#include "ELFLoader.h"
#include <string>
#include <unordered_map>
#include "ELFMapper.h"

//////////////////////////////////////////////////////////////////////////
static std::unique_ptr<uint8_t[]> LoadFile(const char *path, size_t &fsize)
{
	FILE *fp = fopen(path, "rb");
	if (!fp)
		return nullptr;

	fseek(fp, 0, SEEK_END);
	fsize = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	std::unique_ptr<uint8_t[]> buffer(new uint8_t[fsize]);
	size_t szRead = fread(buffer.get(), 1, fsize, fp);
	assert(szRead == fsize);

	fclose(fp);
	return buffer;
}

static int LoadELFFile(ELFLoader &elf, BinReader &elfReader, bool isOutput)
{
	if (!elf.Load(elfReader))
	{
		if (isOutput)
			fprintf(stderr, "Bad ELF image\n");
		return 1;
	}

	const auto &elfHdr = elf.GetELFHeader();
	// AArch64
	if (elfHdr.eMachine != 0xB7)
	{
		if (isOutput)
			fprintf(stderr, "Only support AArch64 instruction set\n");
		return 2;
	}
	// Relocatable
	if (elfHdr.eType != 1)
	{
		if (isOutput)
			fprintf(stderr, "Currently only support relocatable object file\n");
		return 3;
	}

	if (isOutput)
	{
		const size_t secCount = elf.GetSectionInfoCount();
		for (size_t i = 0; i < secCount; ++i)
		{
			const auto &secInfo = elf.GetSectionInfo(i);

			printf("%u: [%s] offset(%llu), size(%llu)\n", i, secInfo.shNameStr, secInfo.shOffset, secInfo.shSize);
		}

		const size_t symCount = elf.GetSymtabInfoCount();
		for (size_t i = 0; i < symCount; ++i)
		{
			const auto &symInfo = elf.GetSymtabInfo(i);

			printf("symbol: [%s] sid(%u), size(%llu), value(%llu), type(%u), other(%u)",
				symInfo.stNameStr, symInfo.stShNdx,
				symInfo.stSize, symInfo.stValue,
				symInfo.stInfo, symInfo.stOther);

			if (symInfo.stInfo == 1 ||
				symInfo.stInfo == 3)
			{
				const auto &strSec = elf.GetSectionInfo(symInfo.stShNdx);
				char *strData = static_cast<char*>(alloca(static_cast<size_t>(strSec.shSize)));
				memcpy(strData, elfReader.Data(static_cast<size_t>(strSec.shOffset)), static_cast<size_t>(strSec.shSize));
				printf(", '%.*s'", static_cast<int>(strSec.shSize), strData);
			}

			printf("\n");
		}
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////
int main(int argc, const char **argv)
{
	const char *elfFile;
	if (argc == 2)
		elfFile = argv[1];
	else
		elfFile = "./testcase/fib.o";

	size_t elfSize;
	auto elfBuf = LoadFile(elfFile, elfSize);
	if (!elfBuf)
	{
		fprintf(stderr, "Cannot open ELF image at '%s'\n", elfFile);
		return -1;
	}

	ELFLoader elf;
	BinReader elfReader(elfBuf.get(), elfSize);
	const int status = LoadELFFile(elf, elfReader, true);
	if (status != 0)
		return status;

	ELFMapper elfMap;
	elfMap.Map(elf);

	ELFLoader::SymtabInfo mainSym;
	if (!elfMap.GetSymtabInfo("main", mainSym))
		return false;

	const auto &mainSec = elf.GetSectionInfo(mainSym.stShNdx);
	size_t mainCodeSize;
	const uint8_t *mainCode = ELFMapper::GetSectionData(mainSec, elfReader, mainCodeSize);

	return 0;
}
