#pragma once

#include <iostream>
#include <vector>
#include <memory>
#include <algorithm>
#include <string>
#include "OperatoreBinario.hpp"

using upob = std::unique_ptr<OperatoreBinario>;

class Sequente{
		std::vector<const upob* > ands;
		std::vector<const upob* > ors;
		std::vector<Sequente> nexts;
	public:
		Sequente();
		Sequente(const upob& s);
		friend std::ostream& operator<<(std::ostream& os, const Sequente& seq);
		friend std::ostream& beautify(std::ostream&, const Sequente&,const std::string&);
		friend std::vector<Sequente> next(const Sequente&);
		friend std::vector<Sequente> get_leaves(const Sequente& seq);
		friend std::string to_pars_str(const Sequente&);
		friend void push_ax(Sequente&,const upob&);
		friend void pop_ax( Sequente&,const upob&);
		friend void buildTree(Sequente&);
		friend bool isTautology(const Sequente&);
};
