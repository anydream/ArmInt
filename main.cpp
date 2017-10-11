#if defined(_DEBUG) && defined(_WIN32) && !defined(_WIN64)
#include <vld.h>
#endif

#include <assert.h>
#include <memory>
#include "ArmInt.h"
#include "BinReader.h"
#include "ELFLoader.h"

//////////////////////////////////////////////////////////////////////////
std::unique_ptr<uint8_t[]> LoadFile(const char *path, size_t &fsize)
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
		return 1;
	}

	BinReader elfReader(elfBuf.get(), elfSize);
	ELFLoader elf;
	if (!elf.Load(elfReader))
	{
		fprintf(stderr, "Bad ELF image\n");
		return 2;
	}

	auto &elfHdr = elf.GetELFHeader();
	// ARM
	if (elfHdr.eMachine != 0x28)
	{
		fprintf(stderr, "Only support ARM instruction set\n");
		return 3;
	}
	// Relocatable
	if (elfHdr.eType != 1)
	{
		fprintf(stderr, "Currently only support relocatable object file\n");
		return 4;
	}

	const size_t secCount = elf.GetSectionInfoCount();
	for (size_t i = 0; i < secCount; ++i)
	{
		auto &secInfo = elf.GetSectionInfo(i);
		printf("[%s] offset(%llu), size(%llu)\n", secInfo.shNameStr, secInfo.shOffset, secInfo.shSize);
	}

	const size_t symCount = elf.GetSymtabInfoCount();
	for (size_t i = 0; i < symCount; ++i)
	{
		auto &symInfo = elf.GetSymtabInfo(i);

		printf("symbol: [%s] secid(%u), size(%llu), value(%llu), type(%u), other(%u)",
			symInfo.stNameStr, symInfo.stShNdx,
			symInfo.stSize, symInfo.stValue,
			symInfo.stInfo, symInfo.stOther);

		if (symInfo.stInfo == 1)
		{
			auto &strSec = elf.GetSectionInfo(symInfo.stShNdx);
			char *strData = static_cast<char*>(alloca(strSec.shSize));
			memcpy(strData, elfReader.Data(strSec.shOffset), strSec.shSize);
			printf(", '%.*s'", static_cast<int>(strSec.shSize), strData);
		}

		printf("\n");
	}

	return 0;
}
