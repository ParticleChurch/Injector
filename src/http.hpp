#pragma once

#include <string>
#include <vector>
#include <map>
#include <cpr/cpr.h>
#include "json.hpp"

namespace HTTP {
	namespace JSON = nlohmann;
	constexpr const char* ROOT = "https://api.particle.church/";

	enum class Protocol
	{
		HTTP,
		HTTPS
	};
	typedef std::map<std::string, std::string> Dictionary;

	class URLPath {
		std::string path;

		static std::string trimSlashes(const std::string& s)
		{
			std::string cpy = s;

			cpy.erase(s.find_last_not_of('/') + 1);
			cpy.erase(0, s.find_first_not_of('/'));

			return cpy;
		}

	public:
		URLPath(const std::string& s)
		{
			this->path = URLPath::trimSlashes(s);
		}

		URLPath(const URLPath& s) : URLPath(s.path) {}

		URLPath operator / (const std::string& other)
		{
			return this->path + "/" + URLPath::trimSlashes(other);
		}

		std::string render()
		{
			if (this->path.length() > 0)
			{
				return this->path + "/";
			}
			else
			{
				return this->path;
			}
		}
	};

	class URL {
		Protocol protocol;
		std::string domain;
		URLPath path;

	public:
		URL(
			const Protocol& protocol,
			const std::string& domain,
			const URLPath& path = { "" }
		)
			: protocol(protocol), domain(domain), path(path)
		{ }

		URL operator / (std::string const& subpath) {
			return {
				this->protocol,
				this->domain,
				this->path / subpath,
			};
		}

		std::string render()
		{
			return
				(this->protocol == Protocol::HTTPS ? "https://" : "http://") +
				this->domain +
				"/" + this->path.render();
		}

	private:
		constexpr inline char BIN_TO_HEX_DIGIT(const char& x)
		{
			constexpr char HEX_DIGITS[] = {
				'0', '1', '2', '3',
				'4', '5', '6', '7',
				'8', '9', 'A', 'B',
				'C', 'D', 'E', 'F',
			};
			return HEX_DIGITS[x];
		}

		static std::string encode(const std::string& s)
		{
			constexpr char HEX_DIGITS[] = {
				'0', '1', '2', '3',
				'4', '5', '6', '7',
				'8', '9', 'A', 'B',
				'C', 'D', 'E', 'F',
			};

			std::string encoded;
			encoded.reserve(s.size() * 3);

			for (const char& c : s)
			{
				if ('a' <= c && c <= 'z') encoded += c;
				else if ('A' <= c && c <= 'Z') encoded += c;
				else if ('0' <= c && c <= '9') encoded += c;

				const char low = HEX_DIGITS[c & 0b1111], high = HEX_DIGITS[c >> 4];
				encoded += '%';
				encoded += high;
				encoded += low;
			}

			return encoded;
		}
	};

	inline cpr::Url urlFromPath(const std::string& path = "")
	{
		static URL API(Protocol::HTTPS, "api.particle.church");
		
		return { (API / path).render() };
	}

	cpr::Response get(std::string path, Dictionary data = {}, Dictionary headers = {})
	{
		cpr::Session session;
		session.SetUrl(urlFromPath(path));

		for (const auto& [key, value] : headers)
			session.SetHeader(cpr::Header{ key, value });

		cpr::Parameters params;

		for (const auto& [key, value] : data)
			params.Add(cpr::Parameter{ key, value });

		session.SetParameters(params);

		return session.Get();
	}

	cpr::Response post(std::string path, JSON::json data = "{}", Dictionary headers = {})
	{
		cpr::Session session;
		session.SetUrl(urlFromPath(path));

		headers["Content-Type"] = "application/json";
		for (const auto& [key, value] : headers)
			session.SetHeader(cpr::Header{ key, value });

		session.SetBody(data.dump());

		return session.Post();
	}
}