#pragma once

#include <unordered_map>
#include "ELFLoader.h"

//////////////////////////////////////////////////////////////////////////
class ELFMapper
{
public:
	void Map(const ELFLoader &elf)
	{
		Loader = &elf;
		SectionMap_.clear();
		SymtabMap_.clear();

		const size_t secCount = elf.GetSectionInfoCount();
		for (size_t i = 0; i < secCount; ++i)
		{
			const auto &secInfo = elf.GetSectionInfo(i);
			SectionMap_[secInfo.shNameStr] = i;
		}

		const size_t symCount = elf.GetSymtabInfoCount();
		for (size_t i = 0; i < symCount; ++i)
		{
			const auto &symInfo = elf.GetSymtabInfo(i);
			SymtabMap_[symInfo.stNameStr] = i;
		}
	}

	bool GetSectionInfo(const std::string &name, ELFLoader::SectionInfo &secInfo) const
	{
		const auto &finded = SectionMap_.find(name);
		if (finded != SectionMap_.end())
		{
			secInfo = Loader->GetSectionInfo(finded->second);
			return true;
		}
		return false;
	}

	bool GetSymtabInfo(const std::string &name, ELFLoader::SymtabInfo &symInfo) const
	{
		const auto &finded = SymtabMap_.find(name);
		if (finded != SymtabMap_.end())
		{
			symInfo = Loader->GetSymtabInfo(finded->second);
			return true;
		}
		return false;
	}

	std::vector<std::string> GetSectionNames() const
	{
		std::vector<std::string> result;
		for (auto &kv : SectionMap_)
			result.push_back(kv.first);
		return result;
	}

	std::vector<std::string> GetSymtabNames() const
	{
		std::vector<std::string> result;
		for (auto &kv : SymtabMap_)
			result.push_back(kv.first);
		return result;
	}

	static const uint8_t* GetSectionData(const ELFLoader::SectionInfo &secInfo, BinReader &reader, size_t &sz)
	{
		sz = static_cast<size_t>(secInfo.shSize);
		return reader.Data(static_cast<size_t>(secInfo.shOffset));
	}

public:
	const ELFLoader *Loader = nullptr;

private:
	std::unordered_map<std::string, size_t> SectionMap_;
	std::unordered_map<std::string, size_t> SymtabMap_;
};
