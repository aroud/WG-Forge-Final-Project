#pragma once

#include <string>
#include <vector>
#include <memory>


struct GameList
{

	//------------------------------          ------------------------------//
	//------------------------------          ------------------------------//
	//------------------------------ TYPEDEFS ------------------------------//
	//------------------------------          ------------------------------//
	//------------------------------          ------------------------------//

	using tick_t = uint64_t;

	//------------------------------ GAME ------------------------------//

	enum GameState
	{
		INIT = 1,
		RUN = 2,
		FINISHED = 3
	};

	struct GameConfig
	{
		std::string name;
		uint32_t num_players;
		tick_t num_turns;
		GameState state;
	};

	//------------------------------          ------------------------------//
	//------------------------------          ------------------------------//
	//------------------------------   IMPL   ------------------------------//
	//------------------------------          ------------------------------//
	//------------------------------          ------------------------------//

	std::vector<GameConfig> games;

};