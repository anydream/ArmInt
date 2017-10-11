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
		return 1;

	BinReader elfReader(elfBuf.get(), elfSize);
	ELFLoader elf;
	if (!elf.Load(elfReader))
		return 2;

	auto &elfHdr = elf.GetELFHeader();
	// ARM
	if (elfHdr.eMachine != 0x28)
		return 3;
	// Relocatable
	if (elfHdr.eType != 1)
		return 4;

	const size_t secCount = elf.GetSecHeaderCount();
	for (size_t i = 0; i < secCount; ++i)
	{
		auto &secHdr = elf.GetSecHeader(i);
		printf("[%s] %llu\n", secHdr.shNameStr, secHdr.shSize);
	}

	return 0;
}
