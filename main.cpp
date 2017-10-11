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
	elf.Load(elfReader);

	return 0;
}
