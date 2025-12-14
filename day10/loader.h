#pragma once

#include <cctype>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

struct Configuration
{
	uint16_t curr_state{ 0 };
	uint16_t target_state{ 0 };
	std::vector<uint16_t> buttons;
	std::vector<size_t> jolts{};
	void press_button(size_t index)
	{
		if (index < buttons.size())
		{
			curr_state ^= buttons[index];
		}
	}
};

using button_t = std::vector<size_t>;
std::vector<button_t> buttons_from_configuration(const Configuration& config)
{
	std::vector<button_t> buttons;
	for (auto button_mask : config.buttons)
	{
		button_t button{};
		for(size_t bit=0; button_mask > 0; ++bit)
		{
			if( (button_mask & 1) == 1)
				button.push_back(bit);
			button_mask >>= 1;
		}
		buttons.push_back(button);	
	}
	return buttons;
}

namespace loader_detail
{
	inline std::string_view trim(std::string_view value)
	{
		while (!value.empty() && std::isspace(static_cast<unsigned char>(value.front())))
		{
			value.remove_prefix(1);
		}
		while (!value.empty() && std::isspace(static_cast<unsigned char>(value.back())))
		{
			value.remove_suffix(1);
		}
		return value;
	}

	inline uint16_t parse_target_pattern(std::string_view pattern)
	{
		if (pattern.size() > 16)
		{
			throw std::runtime_error("Target pattern exceeds 16 bits");
		}

		uint16_t state = 0;
		for (size_t idx = 0; idx < pattern.size(); ++idx)
		{
			char ch = pattern[idx];
			if (ch == '#')
			{
				state |= static_cast<uint16_t>(1u << idx);
			}
			else if (ch != '.')
			{
				throw std::runtime_error("Invalid character in target pattern");
			}
		}
		return state;
	}

	inline uint16_t parse_button_mask(std::string_view spec)
	{
		uint16_t mask = 0;
		size_t start = 0;
		while (start <= spec.size())
		{
			size_t next = spec.find(',', start);
			std::string_view token = trim(spec.substr(start, next == std::string_view::npos ? spec.size() - start : next - start));
			if (!token.empty())
			{
				int bit = std::stoi(std::string(token));
				if (bit < 0 || bit > 15)
				{
					throw std::runtime_error("Button bit index out of range");
				}
				mask |= static_cast<uint16_t>(1u << bit);
			}
			if (next == std::string_view::npos)
			{
				break;
			}
			start = next + 1;
		}
		return mask;
	}

	inline std::vector<size_t> parse_jolts(std::string_view spec)
	{
		std::vector<size_t> jolts;
		size_t start = 0;
		while (start <= spec.size())
		{
			size_t next = spec.find(',', start);
			std::string_view token = trim(spec.substr(start, next == std::string_view::npos ? spec.size() - start : next - start));
			if (!token.empty())
			{
				jolts.push_back(std::stoi(std::string(token)));
			}
			if (next == std::string_view::npos)
			{
				break;
			}
			start = next + 1;
		}
		return jolts;
	}

	inline Configuration parse_configuration_line(const std::string& line)
	{
		Configuration config{};
		std::string_view view = trim(line);
		if (view.empty())
		{
			return config;
		}

		if (view.front() != '[')
		{
			throw std::runtime_error("Line does not start with target pattern");
		}

		size_t target_end = view.find(']');
		if (target_end == std::string_view::npos)
		{
			throw std::runtime_error("Missing closing bracket for target pattern");
		}

		config.target_state = parse_target_pattern(view.substr(1, target_end - 1));

		size_t pos = target_end + 1;
		bool jolts_assigned = false;
		while (pos < view.size())
		{
			while (pos < view.size() && std::isspace(static_cast<unsigned char>(view[pos])))
			{
				++pos;
			}
			if (pos >= view.size())
			{
				break;
			}

			if (view[pos] == '(')
			{
				size_t closing = view.find(')', pos);
				if (closing == std::string_view::npos)
				{
					throw std::runtime_error("Missing closing parenthesis for button definition");
				}
				config.buttons.push_back(parse_button_mask(view.substr(pos + 1, closing - pos - 1)));
				pos = closing + 1;
			}
			else if (view[pos] == '{')
			{
				size_t closing = view.find('}', pos);
				if (closing == std::string_view::npos)
				{
					throw std::runtime_error("Missing closing brace for jolt list");
				}
				config.jolts = parse_jolts(view.substr(pos + 1, closing - pos - 1));
				jolts_assigned = true;
				pos = closing + 1;
			}
			else
			{
				throw std::runtime_error("Unexpected token while parsing configuration line");
			}
		}

		if (!jolts_assigned)
		{
			throw std::runtime_error("Jolt list missing in configuration line");
		}

		return config;
	}
}

inline std::vector<Configuration> load_configurations(const std::filesystem::path& path)
{
	std::ifstream input(path);
	if (!input)
	{
		throw std::runtime_error("Unable to open configuration file");
	}

	std::vector<Configuration> configurations;
	std::string line;
	while (std::getline(input, line))
	{
		std::string_view trimmed = loader_detail::trim(line);
		if (trimmed.empty())
		{
			continue;
		}

		configurations.push_back(loader_detail::parse_configuration_line(line));
	}

	return configurations;
}
