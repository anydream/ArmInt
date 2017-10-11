#pragma once

#include <unordered_map>
#include "ELFLoader.h"

//////////////////////////////////////////////////////////////////////////
class ELFMapper
{
public:
	explicit ELFMapper(const ELFLoader &elf)
		: ELFLoader_(elf)
	{
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

	const ELFLoader::SectionInfo& GetSectionInfo(const std::string &name) const
	{
		size_t idx = -1;
		const auto &finded = SectionMap_.find(name);
		if (finded != SectionMap_.end())
			idx = finded->second;
		return ELFLoader_.GetSectionInfo(idx);
	}

	const ELFLoader::SymtabInfo& GetSymtabInfo(const std::string &name) const
	{
		size_t idx = -1;
		const auto &finded = SymtabMap_.find(name);
		if (finded != SymtabMap_.end())
			idx = finded->second;
		return ELFLoader_.GetSymtabInfo(idx);
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

private:
	const ELFLoader &ELFLoader_;
	std::unordered_map<std::string, size_t> SectionMap_;
	std::unordered_map<std::string, size_t> SymtabMap_;
};
