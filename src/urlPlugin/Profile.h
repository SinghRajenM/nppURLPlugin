#pragma once
#include <string>
#include "Define.h"

class Profile
{
	std::wstring m_ProfileFilePath;

public:
	Profile(const std::wstring &path);
	virtual ~Profile() = default;

protected:
	bool ReadValue(const std::wstring& section, const std::wstring& key, std::wstring& retVal, const std::wstring& defaultVal = {}) const;
	bool WriteValue(const std::wstring& section, const std::wstring& key, const std::wstring& value) const;

private:
	void Init();
};


class ProfileEncode : public Profile
{
public:
	ProfileEncode(const std::wstring& path) : Profile(path) {}
	~ProfileEncode() = default;

	bool GetInfo(EncodeInfo& info) const;
	bool SetInfo(const EncodeInfo& info) const;
};


class ProfileDecode : public Profile
{
public:
	ProfileDecode(const std::wstring& path) : Profile(path) {}
	~ProfileDecode() = default;

	bool GetInfo(DecodeInfo & info) const;
	bool SetInfo(const DecodeInfo& info) const;
};