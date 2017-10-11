#include "ELFLoader.h"
#include "BinReader.h"

bool ELFLoader::Load(BinReader &br)
{
	ELFHeader hdr;
	if (!br.Read(hdr))
		return false;

	return true;
}
