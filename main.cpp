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

	/* specify level min and level max for which the python code will be called */
	/* for the moment, only implemented for the same x and y levels! */
	// std::vector<int> level_1 = {2, 1};
	// std::vector<int> level_2 = {4, 3};
	vec2d levels = {{2, 1}, {4, 3}};
	int dim = levels[0].size();

	/* specify the faults as x and y coordinates; program will check whether the specified constraints are in the problem dictionary */
	vec2d faults = {{1, 1}, {2, 3}, {3, 4}};

	/* output (i.e. c vector) of the optimization problem */
	std::vector<double> new_c;

	/* start the optimization problem */
	lp_opt::LP_OPT_INTERP opt_interp(
		levels, 
		dim,
		GLP_MAX,
		faults);

	lp_opt::LP_OPT_INTERP opt_interp2(opt_interp);
	
	lp_opt::LP_OPT_INTERP opt_interp3 = opt_interp;

	opt_interp3.init_opti_prob(prob_name);
	opt_interp3.set_constr_matrix();
	opt_interp3.solve_opti_problem();
	new_c = opt_interp3.get_results();
	/* end the optimization problem */

	std::cout << std::endl;
	std::cout << "Optimization problem terminated succesfully!" << std::endl;
	return 0;
}