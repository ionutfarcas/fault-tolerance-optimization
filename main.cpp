#include "opt_combi_technique.hpp"

int main(int argv, char** argc)
{
	std::cout << "Optimization problem started................" << std::endl;
	std::cout << "Please wait................................." << std::endl;
	std::cout << "Computing..................................." << std::endl;
	std::cout << "Results....................................." << std::endl;
	std::cout << std::endl;

	/* specify the name of the optimization problem */
	const std::string prob_name = "interp_based_optimization";

	/* specify levels for which the python code will be called */
	/* the order should be from lower to higher */
	vec2d levels = {{1, 1, 1}, {10, 10, 10}};
	assert(levels.front().size() == levels.back().size());
	
	int dim = levels[0].size();

	/* specify the faults as x and y coordinates; program will check whether the specified constraints are in the problem dictionary */
	vec2d faults = {{8, 2, 1}};
	assert(faults[0].size() == static_cast<unsigned int>(dim));

	/* output (i.e. c vector) of the optimization problem */
	std::vector<double> new_c;

	/* start the optimization problem */
	auto start_time_constructor = std::chrono::high_resolution_clock::now();
	lp_opt::LP_OPT_INTERP opt_interp(
		levels, 
		dim,
		GLP_MAX,
		faults);
	auto end_time_constructor = std::chrono::high_resolution_clock::now();
    std::cout << "Constructor call "
              << std::chrono::duration_cast<std::chrono::seconds>(end_time_constructor - start_time_constructor).count()
              << " seconds\n";

    auto start_time_init = std::chrono::high_resolution_clock::now();
	opt_interp.init_opti_prob(prob_name);
	auto end_time_init = std::chrono::high_resolution_clock::now();
    std::cout << "init_opti_prob call "
              << std::chrono::duration_cast<std::chrono::seconds>(end_time_init - start_time_init).count()
              << " seconds\n";

    auto start_time_set_matrix = std::chrono::high_resolution_clock::now();
	opt_interp.set_constr_matrix();
	auto end_time_set_matrix = std::chrono::high_resolution_clock::now();
    std::cout << "set_constr_matrix "
              << std::chrono::duration_cast<std::chrono::seconds>(end_time_set_matrix - start_time_set_matrix).count()
              << " seconds\n";

    auto start_time_solve_prob = std::chrono::high_resolution_clock::now();
	opt_interp.solve_opti_problem();
	auto end_time_solve_prob = std::chrono::high_resolution_clock::now();
    std::cout << "solve_opti_problem "
              << std::chrono::duration_cast<std::chrono::seconds>(end_time_solve_prob - start_time_solve_prob).count()
              << " seconds\n";
	new_c = opt_interp.get_results();
	/* end the optimization problem */

	std::cout << std::endl;
	std::cout << "Optimization problem terminated succesfully!" << std::endl;
	
	return 0;
}