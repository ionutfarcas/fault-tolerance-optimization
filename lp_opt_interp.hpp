#ifndef LPOPTINTERP_HPP_
#define LPOPTINTERP_HPP_

#include "lp_opt.hpp"

namespace lp_opt
{
	class LP_OPT_INTERP : public LP_OPT
	{
	private:
		/* name of the python script */
		const std::string script_name = "../python_code/main.py";
		/* python caller */
		std::string get_dict;

		/* levels of grid indices */
		vec2d i_levels;
		/* top level of grid indices*/
		std::vector<int> level_max;
		/* dimension of the problem */
		int i_dim;

		/* new dimensionality after the input levels are checked */
		int new_dim;
		/* new levels based on new_dim */
		vec2d new_levels;
		/* if it is the case, the dimension(s) that is/are ignored */
		std::vector<int> ignored_dimensions;
		/* if it is the case, new faults, based on ignored dimensions */
		vec2d new_faults;

		/* no. of constraints */
		int no_faults;
		/* level max sum */
		int l_max;

		/* total size of the optimization problem */
		int total_size;
		/* down set size */
		int size_downset;

		/* inverse of M s.t. w = Mc*/
		double** inv_M;

		/* given downset from python code */
		combi_grid_dict given_downset;
		/* modified given downset based on ignored dimensions */
		combi_grid_dict new_given_downset;
		/* entire donwset with corresponding indices */
		combi_grid_dict entire_downset;
		/* auxiliary dictionary, used to create M and inv(M) */
		combi_grid_dict aux_entire_dict;
		/* downset indices as a 2d vector*/
		vec2d downset_indices;
		/* faults input by user */
		vec2d input_faults;
		/* faults that are in the given downset from the set of input faults */
		vec2d valid_input_faults;

		void test()
		{
			std::cout << "inv M" << std::endl;
			for(int i = 0 ; i < size_downset ; ++i)
			{
				for(int j = 0 ; j < size_downset ; ++j)
				{
					std::cout << inv_M[i][j] << " ";
				}
				std::cout << std::endl;
			}

			double** M = inv_M_clever(aux_entire_dict, i_dim);
			std::cout << "inv_M_clever" << std::endl;
			for(int i = 0 ; i < size_downset ; ++i)
			{
				for(int j = 0 ; j < size_downset ; ++j)
				{
					std::cout << M[i][j] << " ";
				}
				std::cout << std::endl;
			}
		}

	public:
		LP_OPT_INTERP() 
		{
			get_dict = "";
			i_dim = 0;
			i_levels = {{0}};
			level_max = {0};
			no_faults = 0;
			l_max = 0;
			size_downset = 0;

			inv_M = NULL;

			constr_mat = NULL;
			row_index = NULL;
			col_index = NULL;
		}

		LP_OPT_INTERP(
			const vec2d& _levels,  
			const int& _dim, 
			const int& _opt_type,
			const vec2d& _input_faults)
		{
			assert(_opt_type == GLP_MIN || _opt_type == GLP_MAX);
			i_levels = _levels;
			i_dim = _dim;
			opt_type = _opt_type;
			input_faults = _input_faults;

			get_dict = python_code_caller(script_name, _levels, _dim);
			given_downset = get_python_data(get_dict, _dim);

			new_levels = check_dimensionality(_levels, ignored_dimensions);
			new_faults =  check_faults(_input_faults, ignored_dimensions);
			new_dim = new_levels[0].size();
			new_given_downset = set_new_given_dict(given_downset, ignored_dimensions, _dim);
			
			check_input_levels(new_levels);

			level_max = new_levels.back();
			size_downset = get_size_downset(level_max, new_dim);
			
			l_max = new_levels[1][0];
			for(int i = 1 ; i < new_dim ; ++i)
			{
				l_max += new_levels[0][i];
			}

			valid_input_faults = filter_faults(new_faults, l_max, new_given_downset);
			no_faults = valid_input_faults.size();

			if(no_faults == 0)
			{
				std::cout << "Please introduce valid faults!" << std::endl;
				exit(0);
			}

			total_size = no_faults*size_downset;

			entire_downset = set_entire_downset_dict(level_max, size_downset, new_given_downset, new_dim);
			aux_entire_dict = create_aux_entire_dict(entire_downset, new_dim);
			auto t1 = std::chrono::high_resolution_clock::now();
			inv_M = M_inv(aux_entire_dict, new_dim);
			auto t2 = std::chrono::high_resolution_clock::now();
			test();
			std::cout << "In constructor, inv_M "
			<< std::chrono::duration_cast<std::chrono::seconds>(t2-t1).count()
			<< " seconds\n";
			downset_indices = get_downset_indices(entire_downset, new_dim);

			constr_mat = (double*)malloc((1 + total_size)*sizeof(double));
			row_index = (int*)malloc((1 + total_size)*sizeof(int));
			col_index = (int*)malloc((1 + total_size)*sizeof(int));

			assert(constr_mat!= NULL);
			assert(row_index!= NULL);
			assert(col_index!= NULL);

			i_lp_prob = glp_create_prob();
			assert(i_lp_prob != NULL);
		}

		LP_OPT_INTERP(const LP_OPT_INTERP& obj)
		{
			i_levels = obj.i_levels;
			i_dim = obj.i_dim;
			opt_type = obj.opt_type;
			input_faults = obj.input_faults;

			get_dict = obj.get_dict;
			given_downset = obj.given_downset;

			new_levels = obj.new_levels;
			new_faults = obj.new_faults;
			ignored_dimensions = obj.ignored_dimensions;
			new_dim = obj.new_dim;
			new_given_downset = obj.new_given_downset;

			level_max = obj.level_max;
			size_downset = obj.size_downset;
			
			l_max = obj.l_max;

			valid_input_faults = obj.valid_input_faults;
			no_faults = obj.no_faults;

			total_size = obj.total_size;
			
			entire_downset = obj.entire_downset;
			aux_entire_dict = obj.aux_entire_dict;
			inv_M = obj.inv_M;

			downset_indices = obj.downset_indices;

			constr_mat = (double*)malloc((1 + total_size)*sizeof(double));
			assert(constr_mat!= NULL);
			std::memcpy(constr_mat, obj.constr_mat, 1*sizeof(constr_mat));

			row_index = (int*)malloc((1 + total_size)*sizeof(int));
			assert(row_index!= NULL);
			std::memcpy(row_index, obj.row_index, 1*sizeof(row_index));

			col_index = (int*)malloc((1 + total_size)*sizeof(int));
			assert(col_index!= NULL);
			std::memcpy(col_index, obj.col_index, 1*sizeof(col_index));

			i_lp_prob = glp_create_prob();
			assert(i_lp_prob != NULL);
			std::memcpy(i_lp_prob, obj.i_lp_prob, 1*sizeof(i_lp_prob));
		}

		LP_OPT_INTERP& operator= (const LP_OPT_INTERP& rhs)
		{
			if(&rhs == this)
			{
				return *this;
			}

			i_levels = rhs.i_levels;
			i_dim = rhs.i_dim;
			opt_type = rhs.opt_type;
			input_faults = rhs.input_faults;

			get_dict = rhs.get_dict;
			given_downset = rhs.given_downset;

			new_levels = rhs.new_levels;
			new_faults = rhs.new_faults;
			ignored_dimensions = rhs.ignored_dimensions;
			new_dim = rhs.new_dim;
			new_given_downset = rhs.new_given_downset;

			level_max = rhs.level_max;
			size_downset = rhs.size_downset;
			
			l_max = rhs.l_max;

			valid_input_faults = rhs.valid_input_faults;
			no_faults = rhs.no_faults;

			total_size = rhs.total_size;
			
			entire_downset = rhs.entire_downset;
			aux_entire_dict = rhs.aux_entire_dict;
			inv_M = rhs.inv_M;

			downset_indices = rhs.downset_indices;

			constr_mat = (double*)malloc((1 + total_size)*sizeof(double));
			assert(constr_mat!= NULL);
			std::memcpy(constr_mat, rhs.constr_mat, 1*sizeof(constr_mat));

			row_index = (int*)malloc((1 + total_size)*sizeof(int));
			assert(row_index!= NULL);
			std::memcpy(row_index, rhs.row_index, 1*sizeof(row_index));

			col_index = (int*)malloc((1 + total_size)*sizeof(int));
			assert(col_index!= NULL);
			std::memcpy(col_index, rhs.col_index, 1*sizeof(col_index));

			i_lp_prob = glp_create_prob();
			assert(i_lp_prob != NULL);
			std::memcpy(i_lp_prob, rhs.i_lp_prob, 1*sizeof(i_lp_prob));

			return *this;
		}

		virtual void init_opti_prob(const std::string& prob_name)
		{
			std::string aux_var;
			double neg_norm = 0.0;
			double coeff = 0.0;

			glp_set_prob_name(i_lp_prob, prob_name.c_str());
			glp_set_obj_dir(i_lp_prob, opt_type);

			glp_add_rows(i_lp_prob, no_faults);
			glp_add_cols(i_lp_prob, size_downset);

			for(int i = 0 ; i < no_faults; ++i)
			{
				aux_var = set_aux_var_name("eq_constr_", i + 1);
				glp_set_row_name(i_lp_prob, i + 1, aux_var.c_str());
				glp_set_row_bnds(i_lp_prob, i + 1, GLP_FX, 0.0, 0.0);
			}

			for(int i = 0 ; i < size_downset ; ++i)
			{
				neg_norm = -l1_norm(downset_indices[i]);
				coeff = pow(4.0, neg_norm);

				aux_var = set_aux_var_name("w", i + 1);
				glp_set_col_name(i_lp_prob, i + 1, aux_var.c_str());
				glp_set_col_bnds(i_lp_prob, i + 1, GLP_DB, 0.0, 1.0);
				glp_set_obj_coef(i_lp_prob, i + 1, coeff);
			}
		}

		virtual void set_constr_matrix()
		{
			int inv_M_row_index = 0;
			
			for(int i = 0 ; i < no_faults ; ++i)
			{
				auto it = aux_entire_dict.find(valid_input_faults[i]);

				if(it != aux_entire_dict.end())
				{
					inv_M_row_index = static_cast<int>(it->second);

					for(int j = 0 ; j < size_downset ; ++j)
					{
						constr_mat[j + i*size_downset + 1] = inv_M[inv_M_row_index][j];
					}
				}
			}

			for(int i = 0 ; i < no_faults ; ++i)
			{
				for(int j = 0 ; j < size_downset ; ++j)
				{
					row_index[j + i*size_downset + 1] = i + 1;
					col_index[j + i*size_downset + 1] = j + 1;
				}
			}
		}

		virtual void set_constr_matrix(const std::vector<double>& W)
		{	
			/* TO DO: nothing here */
		}

		virtual void solve_opti_problem() const
		{
			glp_load_matrix(i_lp_prob, total_size, row_index, col_index, constr_mat);
			glp_simplex(i_lp_prob, NULL);	
		}

		virtual std::vector<double> get_results() const
		{
			double w_i = 0.0;
			double c_i = 0.0;
			std::vector<double> w;		
			std::vector<double> c;

			combi_grid_dict input, output;

			for(int i = 0 ; i < size_downset ; ++i)
			{
				w_i = glp_get_col_prim(i_lp_prob, i + 1);
				w.push_back(w_i);
			}

			for(int i = 0 ; i < size_downset ; ++i)
			{
				c_i = 0.0;
				for(int j = 0 ; j < size_downset ; ++j)
				{
					c_i += inv_M[i][j]*w[j];
				}
				c.push_back(c_i);
			}

			std::cout << std::endl;
			std::cout << "Input faults" << std::endl;
			for(unsigned int i = 0 ; i < input_faults.size() ; ++i)
			{
				std::cout << "{";
				for(int j = 0 ; j < i_dim ; ++j)
				{
					std::cout << input_faults[i][j] << " ";	
				}
				std::cout << "} ";
			}
			std::cout << std::endl;
			std::cout << "Ignored dimensions" << std::endl;
			if(ignored_dimensions.size() == 0)
			{
				std::cout << "None!";
			}
			for(unsigned int i = 0 ; i < ignored_dimensions.size() ; ++i)
			{
				std::cout << ignored_dimensions[i] + 1 << " ";
			}
			std::cout << std::endl;
			std::cout << "Valid input faults" << std::endl;
			for(int i = 0 ; i < no_faults ; ++i)
			{
				std::cout << "{";
				for(int j = 0 ; j < new_dim ; ++j)
				{
					std::cout << valid_input_faults[i][j] << " ";	
				}
				std::cout << "} ";
			}
			std::cout << std::endl;

			input = get_python_data(get_dict, i_dim);
			output = create_out_dict(input, c, i_dim);
			std::cout<< "Dictionary before optimization: " << std::endl;
			for(auto it = input.begin(); it != input.end(); ++it)
			{
				std::cout << "{(";
					for(int j = 0 ; j < i_dim ; ++j) 
					{
						std::cout << it->first[j] << " ";
					}
					std::cout << "), " << it->second << "} ";
}
std::cout << std::endl;

std::cout << std::endl;
std::cout<< "Dictionary after optimization: " << std::endl;
for(auto it = output.begin(); it != output.end(); ++it)
{
	std::cout << "{(";
		for(int j = 0 ; j < i_dim ; ++j) 
		{
			std::cout << it->first[j] << " ";
		}
		std::cout << "), " << it->second << "} ";
}
std::cout << std::endl;

return c;
}

virtual ~LP_OPT_INTERP()
{
	free(constr_mat);
	free(row_index);
	free(col_index);

	glp_delete_prob(i_lp_prob);
}
};
}

#endif /* LPOPTINTERP_HPP_ */
