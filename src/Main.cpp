#define LOG_LEVEL_3

#include "GameData.h"

#include "packet-parser.h"
#include "bincharstream.h"
#include "binstream.h"

#include <iostream>

#include "game_connector.h"

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

int main()
{
	boost::asio::io_service io;
	game_connector connector(io);
	GameData gamedata;

	try
	{

		connector.connect("wgforge-srv.wargaming.net", "443");

		//connector.socket.set_option(tcp::no_delay(true));
		//connector.socket.set_option(boost::asio::socket_base::receive_buffer_size(65536));
		//connector.socket.set_option(boost::asio::socket_base::send_buffer_size(65536));

		{
			LOG_2("Sending Games request...");
			connector.send(Packets::encode::Games::encodeJSON());

			connector.wait_read();

			const auto response = connector.read_packet();
		}

		{
			LOG_2("Sending Login request...");
			connector.send(Packets::encode::Login::encodeJSON({ "test3", "test3", "Game of Thrones", -1, 3 }));

			connector.wait_read();


			const auto response = connector.read_packet();

			GameData::readJSON_Login(gamedata, json::parse(response.second));
		}

		{
			LOG_2("Sending L0 request...");
			connector.send(Packets::encode::Map::encodeJSON({ 0 }));

			connector.wait_read();

			const auto response = connector.read_packet();

			GameData::readJSON_L0(gamedata, json::parse(response.second));
		}

		{
			LOG_2("Sending L10 request...");
			connector.send(Packets::encode::Map::encodeJSON({ 10 }));

			connector.wait_read();

			const auto response = connector.read_packet();

			GameData::readJSON_L10(gamedata, json::parse(response.second));
		}

		{
			LOG_2("Sending L1 request...");
			connector.send(Packets::encode::Map::encodeJSON({ 1 }));

			connector.wait_read();

			const auto response = connector.read_packet();

			GameData::readJSON_L1(gamedata, json::parse(response.second));
		}

		{
			LOG_2("Sending L1 request...");
			connector.send(Packets::encode::Map::encodeJSON({ 1 }));

			connector.wait_read();

			const auto response = connector.read_packet();

			GameData::readJSON_L1(gamedata, json::parse(response.second));
		}

		{
			LOG_2("Sending Games request...");
			connector.send(Packets::encode::Games::encodeJSON());

			connector.wait_read();

			const auto response = connector.read_packet();

			sf::RenderWindow window(sf::VideoMode(gamedata.map_graph.graph_props().size_width + 20, gamedata.map_graph.graph_props().size_height + 20), "graph");

			while (window.isOpen())
			{
				sf::Event event;
				while (window.pollEvent(event))
				{
					// Request for closing the window
					if (event.type == sf::Event::Closed)
						window.close();
				}

				window.clear();

				gamedata.map_graph.for_each_edge_descriptor([&](GraphIdx::edge_descriptor e) {
					const GraphIdx::VertexProperties& es = gamedata.map_graph.graph[boost::source(e, gamedata.map_graph.graph)];
					const GraphIdx::VertexProperties& et = gamedata.map_graph.graph[boost::target(e, gamedata.map_graph.graph)];
					
					sf::Vertex line[2]{
						sf::Vertex(sf::Vector2f(es.pos_x, es.pos_y)),
						sf::Vertex(sf::Vector2f(et.pos_x, et.pos_y))
					};
					window.draw(line, 2, sf::Lines);

					});

				gamedata.map_graph.for_each_vertex_props([&](const GraphIdx::VertexProperties& v) {
					sf::CircleShape dot(5);
					if (v.post_idx == UINT32_MAX)
					{
						dot.setFillColor(sf::Color::White);
					}
					else
					{
						switch (gamedata.posts[v.post_idx]->type())
						{
						case Posts::TOWN: dot.setFillColor(sf::Color::Red); break;
						case Posts::STORAGE: dot.setFillColor(sf::Color::Yellow); break;
						case Posts::MARKET: dot.setFillColor(sf::Color::Green); break;
						}
					}
					
					dot.setPosition(v.pos_x - 5, v.pos_y - 5);
					window.draw(dot);
					});

				window.display();
			}
		}

	}
	catch (const nlohmann::detail::type_error& err)
	{
		std::cout << "ERROR! " << err.what() << std::endl;
	}
	catch (const std::runtime_error& err)
	{
		std::cout << "ERROR! " << err.what() << std::endl;
	}

	connector.close();

	return 0;

	/*GameData gamedata;

	std::cout << "Importing GameData..." << std::endl;

	ptree_from_file("small_graph.json", [&](const ptree& pt) {
		GameData::readJSON_L0(gamedata, pt);
		});
	
	std::cout << "Calculating coordinates..." << std::endl;

	std::cout << "Drawing Graph with coordinates..." << std::endl;

	gamedata.graph.for_each_vertex([&](auto v) {
		const auto& vertex = gamedata.graph.graph[v];
		std::cout << v
			<< " idx:" << vertex.idx
			<< " length:" << (int32_t)vertex.post_idx
			<< " pos:[" << vertex.pos_x << ' ' << vertex.pos_y << ']'
			<< std::endl;
		});*/

	return 0;
}