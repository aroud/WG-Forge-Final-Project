#pragma once

#include <nlohmann/json.hpp>
using nlohmann::json;

#include <src/Types.h>

struct LobbyData
{

	enum GameState : uint8_t
	{
		INIT = 1,
		RUN = 2,
		FINISHED = 3
	};

	std::string name;
	uint8_t num_players;
	Types::tick_t num_turns;
	GameState state;

	static void readJSON_Games(LobbyData& val, const json& j)
	{
		j["name"].get_to(val.name);
		j["num_players"].get_to(val.num_players);
		j["num_turns"].get_to(val.num_turns);
		j["state"].get_to(val.state);
	}

	static void readJSON_Games_vector(std::vector<LobbyData>& vec, const json& j)
	{
		vec.clear();
		vec.reserve(j["games"].size());
		for (const json& ji : j["games"])
		{
			readJSON_Games(vec.emplace_back(), ji);
		}
	}
};
