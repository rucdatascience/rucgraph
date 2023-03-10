#pragma once
/*test function


main file content:
----------------------------------------------------------------

#include <iostream>
#include <string>
using namespace std;

// header files in the Boost library: https://www.boost.org/
#include <boost/random.hpp>
boost::random::mt19937 boost_random_time_seed{ static_cast<std::uint32_t>(std::time(0)) };

#include<build_in_progress/HL/MLL/test_MLL.h>

int main()
{
	test_MLL();
}

---------------------------------------------------------------------------------------------

*/
#include <build_in_progress/HL/MLL/graph_hash_of_mixed_weighted_CT_rank.h>
#include <build_in_progress/HL/MLL/graph_hash_of_mixed_weighted_MLL.h>
#include <graph_hash_of_mixed_weighted/common_algorithms/graph_hash_of_mixed_weighted_shortest_paths.h>
#include <graph_hash_of_mixed_weighted/random_graph/graph_hash_of_mixed_weighted_generate_random_graph.h>
#include <graph_hash_of_mixed_weighted/read_save/graph_hash_of_mixed_weighted_read_graph_with_weight.h>
#include <graph_hash_of_mixed_weighted/read_save/graph_hash_of_mixed_weighted_save_graph_with_weight.h>


/*this function somehow cannot run on VS, but is OK on the server; 可能是代码文本问题*/

void test_MLL() {

	/*parameters*/
	int use_mll = 1;
	int iteration_graph_times = 100, iteration_source_times = 100,
		iteration_terminal_times = 100;
	int V = 1000, E = 1500, precision = 1;
	double ec_min = 0.1,
		ec_max = 1; // since ec_min = 0.01, precision should be at least 2!
					// Otherwise ec may be 0, and causes bugs in CT

	double CT_time = 0, avg_PLL_time = 0, avg_PSL_time = 0;
	long long int avg_CT_index_bit_size = 0, avg_PLL_index_bit_size = 0,
		avg_PSL_index_bit_size = 0;

	double MLL_time = 0, MLL_query_time = 0;
	long long int MLL_index_bit_size = 0;

	/*iteration*/
	std::time_t now = std::time(0);
	boost::random::mt19937 gen{ static_cast<std::uint32_t>(now) };
	for (int i = 0; i < iteration_graph_times; i++) {

		cout << i << endl;

		/*input and output*/
		int generate_new_graph = 1;

		std::unordered_set<int> generated_group_vertices;
		graph_hash_of_mixed_weighted instance_graph, generated_group_graph;
		if (generate_new_graph == 1) {
			instance_graph = graph_hash_of_mixed_weighted_generate_random_graph(
				V, E, 0, 0, ec_min, ec_max, precision, boost_random_time_seed);
			graph_hash_of_mixed_weighted_save_graph_with_weight(
				"simple_iterative_tests.txt", instance_graph, 0);
		}
		else {
			double lambda;
			graph_hash_of_mixed_weighted_read_graph_with_weight(
				"simple_iterative_tests.txt", instance_graph, lambda);
		}

		/*CT*/
		graph_hash_of_mixed_weighted_CT_v2_case_info case_info;
		case_info.two_hop_case_info.use_2019R1 = 0;
		case_info.two_hop_case_info.use_2019R2 = 0;
		case_info.two_hop_case_info.use_enhanced2019R2 = 0;
		case_info.two_hop_case_info.use_non_adj_reduc_degree = 0;
		case_info.two_hop_case_info.use_canonical_repair = 1;
		case_info.d = 5;
		case_info.use_PLL = 1;
		case_info.thread_num = 5;

		if (1) {
			auto begin = std::chrono::high_resolution_clock::now();
			CT_rank(instance_graph, V + 1, case_info);
			auto end = std::chrono::high_resolution_clock::now();
			CT_time =
				std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin)
				.count() /
				1e9; // s
			avg_CT_index_bit_size = case_info.compute_label_bit_size();
			if (use_mll) {
				auto begin2 = std::chrono::high_resolution_clock::now();
				ct_mll_index_construction(instance_graph, V + 1, case_info);
				auto end2 = std::chrono::high_resolution_clock::now();
				MLL_time =
					std::chrono::duration_cast<std::chrono::nanoseconds>(end2 - begin2)
					.count() /
					1e9; // s
				MLL_index_bit_size = compute_mll_label_bit_size();
			}
		}

		/*debug*/
		if (0) {
			graph_hash_of_mixed_weighted_print(instance_graph);
			graph_hash_of_mixed_weighted_print(case_info.core_graph);
			case_info.two_hop_case_info.print_f_2019R1();
			case_info.two_hop_case_info.print_reduction_measures_2019R1();
			case_info.two_hop_case_info.print_reduction_measures_2019R2();
			case_info.two_hop_case_info.print_L();
			case_info.print_root();
			case_info.print_isIntree();
		}

		boost::random::uniform_int_distribution<> dist{ static_cast<int>(0),
													   static_cast<int>(V - 1) };

		int iteration_query_times = iteration_source_times * iteration_terminal_times;

		for (int yy = 0; yy < iteration_source_times; yy++) {

			int source = dist(gen);

			// source = 7; cout << "source = " << source << endl;

			std::unordered_map<int, double> distances;
			std::unordered_map<int, int> predecessors;
			graph_hash_of_mixed_weighted_shortest_paths_source_to_all(
				instance_graph, source, distances, predecessors);

			for (int xx = 0; xx < iteration_terminal_times; xx++) {
				int terminal = dist(gen);

				// getchar();
				// terminal = 5; cout << "terminal = " << terminal << endl;

				double dis = CT_extract_distance(case_info, source, terminal);

				if (abs(dis - distances[terminal]) > 1e-5 &&
					(dis < 1e10 || distances[terminal] < 1e10)) {
					cout << "source = " << source << endl;
					cout << "terminal = " << terminal << endl;

					cout << "source vector:" << endl;
					for (auto it = case_info.two_hop_case_info.L[source].begin();
						it != case_info.two_hop_case_info.L[source].end(); it++) {
						cout << "<" << it->vertex << "," << it->distance << ","
							<< it->parent_vertex << ">";
					}
					cout << endl;
					cout << "terminal vector:" << endl;
					for (auto it = case_info.two_hop_case_info.L[terminal].begin();
						it != case_info.two_hop_case_info.L[terminal].end(); it++) {
						cout << "<" << it->vertex << "," << it->distance << ","
							<< it->parent_vertex << ">";
					}
					cout << endl;

					cout << "dis = " << dis << endl;
					cout << "distances[terminal] = " << distances[terminal] << endl;
					cout << "abs(dis - distances[terminal]) > 1e-5!" << endl;
					getchar();
				}
				vector<pair<int, int>> path;


				if (use_mll) {
					auto begin3 = std::chrono::high_resolution_clock::now();
					MLL_extract_path(instance_graph, case_info, source, terminal, path);
					auto end3 = std::chrono::high_resolution_clock::now();
					double runningtime3 =
						std::chrono::duration_cast<std::chrono::nanoseconds>(end3 -
							begin3)
						.count() /
						1e9;
					MLL_query_time =
						MLL_query_time + runningtime3 / iteration_query_times;
				}
				else {
					CT_extract_path(case_info, source, terminal, path);
				}

				double path_dis = 0;
				if (path.size() == 0) {
					if (source != terminal) { // disconnected
						path_dis = std::numeric_limits<double>::max();
					}
				}
				else {
					for (auto it = path.begin(); it != path.end(); it++) {
						path_dis = path_dis + graph_hash_of_mixed_weighted_edge_weight(
							instance_graph, it->first, it->second);
						if (path_dis > std::numeric_limits<double>::max()) {
							path_dis = std::numeric_limits<double>::max();
						}
					}
				}
				if (abs(dis - path_dis) > 1e-5 &&
					(dis < 1e10 || distances[terminal] < 1e10)) {
					cout << "source = " << source << endl;
					cout << "terminal = " << terminal << endl;

					cout << "source vector:" << endl;
					for (auto it = case_info.two_hop_case_info.L[source].begin();
						it != case_info.two_hop_case_info.L[source].end(); it++) {
						cout << "<" << it->vertex << "," << it->distance << ","
							<< it->parent_vertex << ">";
					}
					cout << endl;
					cout << "terminal vector:" << endl;
					for (auto it = case_info.two_hop_case_info.L[terminal].begin();
						it != case_info.two_hop_case_info.L[terminal].end(); it++) {
						cout << "<" << it->vertex << "," << it->distance << ","
							<< it->parent_vertex << ">";
					}
					cout << endl;
					print_vector_pair_int(path);
					for (int i = 0; i < path.size(); i++) {
						if (graph_hash_of_mixed_weighted_contain_edge(
							instance_graph, path[i].first, path[i].second) == false) {
							cout << "not-existing edge: " << path[i].first << ","
								<< path[i].second << endl;
							cout << "isIntree[path[i].first]: "
								<< case_info.isIntree[path[i].first] << endl;
							cout << "isIntree[path[i].second]: "
								<< case_info.isIntree[path[i].second] << endl;
						}
					}
					cout << "dis = " << dis << endl;
					cout << "distances[terminal] = " << distances[terminal] << endl;
					cout << "path_dis = " << path_dis << endl;
					cout << "abs(dis - path_dis) > 1e-5!" << endl;
					getchar();
				}
			}
		}

		/*due to global query values, use PLL and PSL after check*/

		/*PLL*/
		if (0) {
			graph_hash_of_mixed_weighted_two_hop_case_info_v1 mm;
			mm.use_2019R1 = case_info.two_hop_case_info.use_2019R1;
			mm.use_2019R2 = case_info.two_hop_case_info.use_2019R2;
			mm.use_enhanced2019R2 = case_info.two_hop_case_info.use_enhanced2019R2;
			mm.use_non_adj_reduc_degree =
				case_info.two_hop_case_info.use_non_adj_reduc_degree;
			auto begin = std::chrono::high_resolution_clock::now();
			graph_hash_of_mixed_weighted_PLL_v1(instance_graph, V + 1, 1,
				case_info.thread_num, mm);
			auto end = std::chrono::high_resolution_clock::now();
			double runningtime =
				std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin)
				.count() /
				1e9; // s
			avg_PLL_time = avg_PLL_time + runningtime / iteration_graph_times;
			avg_PLL_index_bit_size =
				avg_PLL_index_bit_size +
				mm.compute_label_bit_size() / iteration_graph_times;
		}
		/*PSL*/
		if (0) {
			graph_hash_of_mixed_weighted_two_hop_case_info_v1 mm;
			mm.use_2019R1 = case_info.two_hop_case_info.use_2019R1;
			mm.use_2019R2 = case_info.two_hop_case_info.use_2019R2;
			mm.use_enhanced2019R2 = case_info.two_hop_case_info.use_enhanced2019R2;
			mm.use_non_adj_reduc_degree =
				case_info.two_hop_case_info.use_non_adj_reduc_degree;
			auto begin = std::chrono::high_resolution_clock::now();
			graph_hash_of_mixed_weighted_PSL_v1(instance_graph, V + 1,
				case_info.thread_num, mm);
			auto end = std::chrono::high_resolution_clock::now();
			double runningtime =
				std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin)
				.count() /
				1e9; // s
			avg_PSL_time = avg_PSL_time + runningtime / iteration_graph_times;
			avg_PSL_index_bit_size =
				avg_PSL_index_bit_size +
				mm.compute_label_bit_size() / iteration_graph_times;
		}

		if (use_mll) {
			cout << "CT_time: " << CT_time << "s" << endl;
			cout << "MLL_time: " << MLL_time << "s" << endl;
			print_mll_time();
			cout << "MLL_query_time: " << MLL_query_time << endl;
			cout << "MLL_index_bit_size: " << MLL_index_bit_size << endl;
			vector<vector<pair<int, int>>>().swap(MLL);
		}
	}

	if (!use_mll) {
		cout << "CT_time: " << CT_time << "s" << endl;
		cout << "avg_PLL_time: " << avg_PLL_time << "s" << endl;
		cout << "avg_PSL_time: " << avg_PSL_time << "s" << endl;
		cout << "avg_CT_index_bit_size: " << avg_CT_index_bit_size << endl;
		cout << "avg_PLL_index_bit_size: " << avg_PLL_index_bit_size << endl;
		cout << "avg_PSL_index_bit_size: " << avg_PSL_index_bit_size << endl;
	}
}