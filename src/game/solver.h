#pragma once

#include <src/game/solver/train.h>
#include <src/game/solver/collisions_checker.h>
#include <src/utils/network/server_connector.h>


class GameSolver
{
public:

	GameSolver(const GameData& gamedata, server_connector& connector)
		: gamedata(gamedata), 
		connector(connector), 
		pathsolver(gamedata),
		tick(0)
	{
		for (const auto& [train_idx, train_data] : gamedata.self_data().trains)
		{
			trainsolvers.emplace_back(gamedata, train_idx, deltas_market, deltas_storage);
		}
	}

	void reset_deltas()
	{
		deltas_market.init(gamedata.graph());
		deltas_storage.init(gamedata.graph());
	}

	void calculate()
	{
		tick++;

		reset_deltas();

		calculate_upgrades();

		calculate_states();

		for (auto& train_solver : trainsolvers)
		{
			train_solver.calculate_Turn();
		}

		CollisionsChecker::check_and_solve(trainsolvers, gamedata);

		for (auto& train_solver : trainsolvers) {

			if (train_solver.possible_move.has_value())
			{
				connector.send_Move(get<2>(train_solver.possible_move.value()));
			}
		}

	}

	void calculate_states() {
		Types::Epoch epoch = get_epoch();
		if (epoch <= 3)  // 0,1,2,3
		{
			for (TrainSolver& ts : trainsolvers) {
				if (ts.gamedata_train.goods_type == Trains::Armor && ts.gamedata_train.goods > 35) {
					ts.state = TrainSolver::State::RETURN;
				}
				ts.state = TrainSolver::State::NORMAL_ARMOR;
			}
		}
		else if (epoch == 4) { //4 epoch: 

			check_first_free();
			if (food_epoch4_ts_idx != std::numeric_limits<uint32_t>::max()) {

				TrainSolver& ts = trainsolvers[food_epoch4_ts_idx];						   //1 train collexts food

				if (ts.gamedata_train.goods_type == Trains::Product && ts.gamedata_train.goods > 0) {
					ts.state = TrainSolver::State::RETURN;
				}

				ts.state = TrainSolver::State::NORMAL_FOOD;
			}

			for (size_t i = 0; i < trainsolvers.size(); ++i) {								//3 other - armor
				if (i != food_epoch4_ts_idx)
				{
					TrainSolver& ts = trainsolvers[i];
					if (ts.gamedata_train.goods_type == Trains::Armor && ts.gamedata_train.goods > 35)
					{
						ts.state = TrainSolver::State::RETURN;
					}
					ts.state = TrainSolver::State::NORMAL_FOOD;
				}
			}

		}
		else { //epoch 5,6,7 (partly 8) - collect food 
			for (TrainSolver& ts : trainsolvers) {
				if (ts.gamedata_train.goods != 0)		//train has already visited market for food
					ts.state = TrainSolver::State::RETURN;
				else
					ts.state = TrainSolver::State::NORMAL_FOOD;
			}
		}
	}

	void calculate_upgrades() {
		const Posts::Town* town = get_home_town();
		auto armour = town->armor;

		std::set<Types::train_idx_t> toupgrade_trains;
		std::set<Types::post_idx_t> toupgrade_posts;

		bool updated = true;
		while (updated) {
			uint8_t min_level = 4;
			Types::train_idx_t min_train_index = UINT32_MAX;

			for (size_t i = 0; i < trainsolvers.size(); ++i) {

				if (trainsolvers[i].gamedata_train.level < min_level) {
					min_level = trainsolvers[i].gamedata_train.level;
					min_train_index = trainsolvers[i].train_idx;
				}
			}

			updated = false;
			//check for train updates
			if (min_level == 1 && armour >= Trains::TrainTiers[0].next_level_price) {
				armour -= Trains::TrainTiers[0].next_level_price;
				toupgrade_trains.insert(min_train_index);
				//connector.send_Upgrade({ {},{min_train_index} });

				updated = true;
			}
			else if (min_level == 2 && armour >= Trains::TrainTiers[1].next_level_price) {
				armour -= Trains::TrainTiers[1].next_level_price;
				toupgrade_trains.insert(min_train_index);
				updated = true;
			}
			else if (town->level == 2 && armour >= 75) {  //check for city updates
				armour -= 75;
				toupgrade_posts.insert(gamedata.home_idx);
				//connector.send_Upgrade({ {gamedata.home_idx},{} });
				updated = true;
			}
			else if (town->level == 2 && armour >= 185) {
				armour -= 150;
				toupgrade_posts.insert(gamedata.home_idx);
				updated = true;
			}
		}

		std::vector<Types::train_idx_t> upgrade_trains(toupgrade_trains.begin(), toupgrade_trains.end());
		std::vector<Types::post_idx_t> upgrade_posts(toupgrade_posts.begin(), toupgrade_posts.end());


		connector.async_send_Upgrade({upgrade_posts, upgrade_trains});
	}

	const Posts::Town* get_home_town() {
		return (const Posts::Town*) gamedata.posts.at(gamedata.post_idx).get();
	}

	Types::Epoch get_epoch() {
		return tick / 60;
	}

	size_t getTrainSolverIndex(Types::train_idx_t t) {
		for (size_t i = 0; i < trainsolvers.size(); ++i) {
			if (trainsolvers[i].gamedata_train.idx == t)
				return i;
		}
		return std::numeric_limits<uint32_t>::max();
	}

	void check_first_free() {
		if (food_epoch4_ts_idx == std::numeric_limits<uint32_t>::max())
		{
			for (size_t i = 0; i < trainsolvers.size(); ++i) {
				if (is_at_home(trainsolvers[i])) {
					food_epoch4_ts_idx = i;
					return;
				}
			}
		}
	}

	bool is_at_home(TrainSolver& ts) {
		auto v_idx = gamedata.home_idx;
		auto v = gamedata.map_graph.vmap.at(v_idx);
		for (auto& p : gamedata.map_graph.emap) {
			if (ts.gamedata_train.line_idx == p.first) {
				auto e = p.second;
				auto u = boost::source(e, gamedata.map_graph.graph);
				auto t = boost::target(e, gamedata.map_graph.graph);
				if (
					(v == u && ts.gamedata_train.position == 0)||
					(v == t && ts.gamedata_train.position == (gamedata.map_graph.graph[e].length - 1))
					)
				{
					return true;
				}
			}
		}
		return false;
	}


protected:
	const GameData& gamedata;
	server_connector& connector;

	std::vector<TrainSolver> trainsolvers;

	GraphVertexMap<double> deltas_market;
	GraphVertexMap<double> deltas_storage;

	PathSolver pathsolver;
	
	Types::tick_t tick;
	size_t food_epoch4_ts_idx;
};
