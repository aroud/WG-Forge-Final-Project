#pragma once

#include <src/game/solver/train.h>
#include <vector>

class CollisionsChecker {

	enum CollisionType {
		Towards, //� ��������� ���� �����, ��� ������ ��������� -> ������� ��� ����� (�����) �� �������� 2��� ������, ����������� �������� ���������

		Sequential, // ���� ����������� �����, �� ������ ����� ��������� � ����� ����������� ->  
		// ���������� ���� �� 2 ������� �� 1 ���, ���� ����������

		None,//-> ��� ��

		Vertex// -> ����� ������� �� ���� (�� �����) ->���������� ���� �� 2 �������, ���� � ��� ������� ���� ��������� ������������
	};

	static CollisionType solve(const GraphDijkstra::path_edges_t& path_edges1, const GraphDijkstra::path_edges_t& path_edges2,
		const GraphDijkstra::path_t& path_1, const GraphDijkstra::path_t& path_2) {

		for (size_t i = 0; i < path_edges1.size(); ++i) {
			for (size_t j = 0; j < path_edges2.size(); ++j) {
				if (path_edges1[i] == path_edges2[j]) {

				}
			}

		}

		std::vector<Graph::edge_descriptor> for_delete;

	}

	static void check_and_solve(TrainSolver& t1, TrainSolver& t2) {
		if (t1.possible_move.has_value() && t2.possible_move.has_value()) {
			auto& tuple1 = t1.possible_move.value();
			auto& tuple2 = t2.possible_move.value();

			auto& path1 = get<0>(tuple1);
			auto& path2 = get<0>(tuple2);

			auto& path_edges1 = get<1>(tuple1);
			auto& path_edges2 = get<1>(tuple2);

			CollisionType type = solve(path_edges1, path_edges2, path1, path2);
		}
	}
public:
	static void check_and_solve(std::vector<TrainSolver>& trainsolvers) {
		for (size_t i = 0; i < trainsolvers.size(); ++i) {
			for (size_t j = i + 1; j < trainsolvers.size(); ++j) {
				check_and_solve(trainsolvers[i], trainsolvers[j]);
			}
		}
	}
};
