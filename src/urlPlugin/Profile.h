#pragma once
#include <string>
#include "Define.h"

class Profile
{
	std::wstring m_ProfileFilePath;

public:
	Profile();
	virtual ~Profile() = default;

protected:
	bool ReadValue(const std::wstring& section, const std::wstring& key, std::wstring& retVal) const;
	bool WriteValue(const std::wstring& section, const std::wstring& key, const std::wstring& value) const;

private:
	void Init();
};


class ProfileEncode : public Profile
{
public:
	ProfileEncode() = default;
	~ProfileEncode() = default;

	bool GetInfo(EncodeInfo& info) const;
	bool SetInfo(const EncodeInfo& info) const;
};


class ProfileDecode : public Profile
{
public:
	ProfileDecode() = default;
	~ProfileDecode() = default;

	bool GetInfo(DecodeInfo & info) const;
	bool SetInfo(const DecodeInfo& info) const;
};