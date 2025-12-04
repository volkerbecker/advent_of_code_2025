#include "read_input.h"
#include <vector>
#include <sstream>
#include <algorithm>
#include <cctype>

std::vector<std::string> read_input(const std::filesystem::path& path)
{
	std::vector<std::string> res;
	std::ifstream in(path);
	if (!in.good())
		std::cout << "Error while open file \n";
	while (!in.eof())
	{
		std::string str;
		in >> str;
		res.push_back(str);
	}
	return res;
}

static inline std::string trim(const std::string& s)
{
	auto start = s.begin();
	while (start != s.end() && std::isspace(static_cast<unsigned char>(*start))) ++start;
	auto end = s.end();
	if (start == end) return {};
	--end;
	while (std::distance(start, end) >= 0 && std::isspace(static_cast<unsigned char>(*end))) --end;
	return std::string(start, end + 1);
}

static bool has_leading_zero(const std::string& s)
{
	if (s.empty()) return false;
	// optional sign
	size_t i = 0;
	if (s[0] == '+' || s[0] == '-')
	{
		if (s.size() <= 1) return false;
		i = 1;
	}
	// single digit is allowed (e.g. "0" or "-1")
	if (s.size() - i <= 1) return false;
	// leading char after optional sign is '0' -> führende Null
	if (s[i] != '0') return false;
	// ensure the rest are digits
	return std::all_of(s.begin() + i, s.end(), [](unsigned char c) { return std::isdigit(c); });
}

std::vector<std::pair<int64_t, int64_t>> read_pairs(const std::filesystem::path& path)
{
	std::vector<std::pair<int64_t, int64_t>> result;
	std::ifstream in(path);
	if (!in.is_open())
	{
		throw std::runtime_error(std::string("Datei konnte nicht geöffnet werden: ") + path.string());
	}

	// Lese gesamte Datei
	std::ostringstream oss;
	oss << in.rdbuf();
	std::string content = oss.str();

	std::stringstream ss(content);
	std::string token;
	while (std::getline(ss, token, ','))
	{
		auto t = trim(token);
		if (t.empty())
			continue;

		auto dashPos = t.find('-');
		if (dashPos == std::string::npos)
		{
			std::cerr << "Ungültiges Feld (kein '-'): \"" << t << "\"\n";
			continue;
		}

		std::string aStr = trim(t.substr(0, dashPos));
		std::string bStr = trim(t.substr(dashPos + 1));

		// Führende Null prüfen (z. B. "01" oder "-01")
		if (has_leading_zero(aStr))
			throw std::runtime_error(std::string("Führende Null in erster Zahl: \"") + aStr + "\" im Feld \"" + t + "\"");
		if (has_leading_zero(bStr))
			throw std::runtime_error(std::string("Führende Null in zweiter Zahl: \"") + bStr + "\" im Feld \"" + t + "\"");

		try
		{
			int64_t a = std::stoll(aStr);
			int64_t b = std::stoll(bStr);
			result.emplace_back(a, b);
		}
		catch (const std::exception& e)
		{
			std::cerr << "Parse-Fehler für Feld \"" << t << "\": " << e.what() << '\n';
		}
	}

	return result;
}
