#pragma once
#include <locale>
#include <map>
#include <sstream>
#include <algorithm>
#include <vector>


class EncodeDecodeBase
{
protected:
	std::map <std::string, std::string> _parameters;
	std::string _baseUrl;		// Eg: http://example.com	
	std::string _url;			// Eg: http://example.com?key1=val1&key2=val2
	std::vector<std::string> _exclusionSet;

public:

	EncodeDecodeBase() = default;
	virtual ~EncodeDecodeBase() = default;

	virtual void setParameter(const std::pair<std::string, std::string>& pair)
	{
		setParameter(pair.first, pair.second);
	}

	virtual void setParameter(const std::string& key, const std::string& val)
	{
		_parameters[key] = val;
	}

	virtual void clear()
	{
		_baseUrl.clear();
		_parameters.clear();
		_url.clear();
	}

	virtual size_t removeParameter(const std::string& key)
	{
		return _parameters.erase(key);
	}

	virtual void setBaseUrl(const std::string& baseUrl)
	{
		_baseUrl = baseUrl;
	}

	virtual const std::string& getBaseUrl() const
	{
		return _baseUrl;
	}

	virtual void setUrl(const std::string& url)
	{
		_url = url;
	}

	virtual const std::string& getUrl() const
	{
		return _url;
	}

	virtual void setExclusion(const std::vector<std::string>& exclusion)
	{
		_exclusionSet = exclusion;
	}

	virtual const std::vector<std::string>& getExclusion() const
	{
		return _exclusionSet;
	}

	virtual const std::map<std::string, std::string>& getParameters() const
	{
		return _parameters;
	}
};


class Encode : public EncodeDecodeBase
{
	const std::string& encode(const std::string& aString, bool bRetainLineFeed)
	{
		std::string aEncodedString = "";

		// Does starts with https:// or http://, then find base URL till ?
		std::string https[] = { "http://", "https://" };

		for (const auto& h : https)
		{
			// aString starts with http or https
			if (aString.rfind(h) == 0)
			{
				auto posOfQMark = aString.find_first_of('?');
				if (posOfQMark != std::string::npos)
				{
					// Everything prior to ? is the baseUrl
					_baseUrl = aString.substr(0, posOfQMark);
					aEncodedString = aString.substr(0, posOfQMark + 1);

					_url = aString.substr(posOfQMark + 1);
				}
				break;
			}
		}

		for (char aChar : aString)
		{
			// Ignore below whitespaces and retain space which will be converted
			/*
				'\n' : newline,
				'\t' : horizontal tab,
				'\v' : vertical tab
				'\r' : Carraige return
				'\f' : form feed
			*/
			if (std::isspace(aChar) && aChar != ' ' && bRetainLineFeed == false)
				continue;

			char allowedChars[] = { '-', '_', '.','~','&', '=', '+', };

			if (!std::isalnum(aChar) && !(std::any_of(std::begin(allowedChars), std::end(allowedChars), [aChar](const char c) {return aChar == c; })))
			{
				// Hex encode
				aEncodedString += "%";
				std::stringstream ss;
				ss << std::hex << (int)aChar;
				std::string aTempStr = ss.str();
				/* Apparently two overloaded toupper methods exist.
				* One in the std namespace and one in the global namespace
				* Good going C++, you god damned cryptic esoteric parseltongue
				*/
				std::transform(aTempStr.begin(), aTempStr.end(), aTempStr.begin(),
					[](char c) {return static_cast<char>(std::toupper(c)); });
				aEncodedString += aTempStr;
			}
			else
			{
				//Otherwise, keep it as it is
				aEncodedString += aChar;
			}
		}
		_url = aEncodedString;
		return _url;
	}

public:

	Encode() = default;
	~Encode() = default;

	const std::string& encode(bool bRetainLineFeed)
	{
		return _url.empty() ? _url : encode(_url, bRetainLineFeed);
	}
}; // class Encode


class Decode : public EncodeDecodeBase
{
	std::vector<std::string> split(const std::string& str, const char delim)
	{
		if (str.empty())
			return {};

		std::vector<std::string> tokens;
		std::istringstream iss(str);
		std::string token;

		while (std::getline(iss, token, delim))
		{
			tokens.push_back(token);
		}
		return tokens;
	}

	void buildParameters(const std::string& parameters)
	{
		auto keyValTokens = split(parameters, '&');
		for (const auto& keyValToken : keyValTokens)
		{
			if (keyValToken.size() < 1)
			{
				continue;
			}

			auto keyValVector = split(keyValToken, '=');
			if (keyValVector.size() != 2)
			{
				continue;
			}
			_parameters[keyValVector[0]] = keyValVector[1];
		}
	}

	const std::string& decode(const std::string encodedUrl)
	{
		bool baseUrlAvailable = false;

		auto posOfQMark = encodedUrl.find_first_of('?');
		if (posOfQMark != std::string::npos)
		{
			baseUrlAvailable = true;

			// Everything prior to ? is the baseUrl
			_baseUrl = encodedUrl.substr(0, posOfQMark);
		}

		//If '?' is at n, we need n+1 characters from the start, i.e including the ?.
		_url = encodedUrl.substr(0, baseUrlAvailable ? posOfQMark + 1 : 0);

		std::string parameters;
		for (size_t pos = posOfQMark + 1; pos < encodedUrl.size(); ++pos)
		{
			// Look for %XX
			if ('%' == encodedUrl[pos])
			{
				if (pos + 2 >= encodedUrl.size())
				{
					//We have reached the end of the url. We cannot decode it because there is nothing to decode
					break;
				}
				//Decode next two chars and increase the pos counter accordingly
				//Get the next two characters after % and store it in iss
				std::istringstream iss(encodedUrl.substr(pos + 1, 2));
				int aIntVal;
				iss >> std::hex >> aIntVal;
				parameters += static_cast<char> (aIntVal);
				//We have processed two extra characters
				pos += 2;
			}
			else
			{
				//Nothing to do, just copy
				parameters += encodedUrl[pos];
			}
		}

		buildParameters(parameters);
		_url += parameters;
		return _url;
	}

public:

	Decode() = default;
	~Decode() = default;

	const std::string& decode()
	{
		return _url.empty() ? _url : decode(_url);
	}

}; // class Decode

