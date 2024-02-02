#include <iostream>
#include <vector>
#include <algorithm>
#include <set>
#include <iterator>
#include <map>
#include <sstream>

#include "OperatoreBinario.hpp"
#include "Sequente.hpp"

using namespace std;
using upob = std::unique_ptr<OperatoreBinario>;

Sequente::Sequente():ands(0),ors(0){};
Sequente::Sequente(const upob& seq) {
	if(seq->type() == IMP_TYPE){
		ands.push_back(&(seq->sx()));
		 ors.push_back(&(seq->dx()));
	}
}

void push_ax(Sequente& e, const upob& o){
	e.ands.push_back(&o);               
}                                     
void pop_ax( Sequente& e, const upob& o){
	e.ands.erase(find(e.ands.begin(),e.ands.end(),&o),e.ands.end());
}

std::ostream& operator<<(std::ostream& os,const upob* expr){
	const upob& r = *expr;
	os << "(";
	if(r->sx() != nullptr){ os << &(r->sx()) ; }
	os << r->name();
	if(r->dx() != nullptr){ os << &(r->dx()) ; }
	return os << ")";
}

std::ostream& operator<<(std::ostream& os, const vector<const upob*>& v){
	if(!v.size()) { return os << "NULL"; }
	os << "[ ";
	for(size_t i = 0; i < v.size()-1; i++){ os << v.at(i) << ", "; }
	return os << v.back() << " ] ";
}

std::ostream& operator<<(std::ostream& os, const Sequente& seq){
	if(seq.ands.size()){ os << seq.ands; }
	else { os << "V"; }
	os << " -> ";
	if(seq.ors.size()){ os << seq.ors; }
	else { os << "F"; }
	return os;
}

ostream& beautify(ostream& os, const Sequente& e, const string& pad){
	if(e.nexts.size() == 0){ return os << pad << e; }

	os << pad << e << "\n";
	auto i = e.nexts.begin();
	for(; i+1 != e.nexts.end(); i++){
		beautify(os,*i,pad+"\t") << "\n";
	}
	return beautify(os,*i,pad+"\t");
}

void buildTree(Sequente& e){
	e.nexts = next(e);
	for(Sequente& i : e.nexts){ buildTree(i); }
}

vector<Sequente> next(const Sequente& seq){
	auto pop_back=[](vector<const upob*>& v){
		const upob * c = v.back();
		v.pop_back();
		return c;
	};
	auto check_validity = [](const map<int, vector<const upob*>>& m,const int type){
		return m.find(type) != m.end() && m.at(type).size();
	};

	vector<Sequente> ris;
	map<int, vector<const upob*>> ands { {VAR_TYPE,{}},{NOT_TYPE,{}},{AND_TYPE,{}},{OR_TYPE,{}},{IMP_TYPE,{}},{V_TYPE,{}},{F_TYPE,{}}};
	map<int, vector<const upob*>>  ors { {VAR_TYPE,{}},{NOT_TYPE,{}},{AND_TYPE,{}},{OR_TYPE,{}},{IMP_TYPE,{}},{V_TYPE,{}},{F_TYPE,{}}};
	for(const auto& i : seq.ands){ ands.at((*i)->type()).push_back(i); }
	for(const auto& i :  seq.ors){  ors.at((*i)->type()).push_back(i); }

	if(check_validity(ands,AND_TYPE)){
		ris.emplace_back();
		const upob * c = pop_back(ands.at(AND_TYPE));
		ands.at((*c)->sx()->type()).push_back(&((*c)->sx()));
		ands.at((*c)->dx()->type()).push_back(&((*c)->dx()));
	}else if(check_validity(ors,OR_TYPE)){
		ris.emplace_back();
		const upob * c = pop_back(ors.at( OR_TYPE));
		 ors.at((*c)->sx()->type()).push_back(&((*c)->sx()));
		 ors.at((*c)->dx()->type()).push_back(&((*c)->dx()));

	}else if(check_validity(ands,NOT_TYPE)){
		ris.emplace_back();
		const upob * c = pop_back(ands.at( NOT_TYPE));
		 ors.at((*c)->type()).push_back(&((*c)->sx()));
	}else if(check_validity(ors,NOT_TYPE)){
		ris.emplace_back();
		const upob * c = pop_back( ors.at( NOT_TYPE));
		ands.at((*c)->type()).push_back(&((*c)->sx()));

	}else if(check_validity(ors,IMP_TYPE)){
		ris.emplace_back();
		const upob * c = pop_back( ors.at( IMP_TYPE));
		ands.at((*c)->sx()->type()).push_back(&((*c)->sx()));
		 ors.at((*c)->dx()->type()).push_back(&((*c)->dx()));

	}else if(check_validity(ors,AND_TYPE)){
		ris.emplace_back();
		ris.emplace_back();
		const upob * c = pop_back(ors.at(AND_TYPE));
		ris.at(0).ors.push_back(&((*c)->sx()));
		ris.at(1).ors.push_back(&((*c)->dx()));


	}else if(check_validity(ands,IMP_TYPE)){
		ris.emplace_back();
		ris.emplace_back();
		const upob * c = pop_back(ands.at(IMP_TYPE));
		ris.at(0).ors.push_back(&((*c)->sx()));
		ris.at(1).ands.push_back(&((*c)->dx()));

	}else if(check_validity(ands,OR_TYPE)){
		ris.emplace_back();
		ris.emplace_back();
		const upob * c = pop_back(ands.at(OR_TYPE));
		ris.at(0).ands.push_back(&((*c)->sx()));
		ris.at(1).ands.push_back(&((*c)->dx()));
	}

	for(Sequente& j : ris){
		for(const auto& i : ands.at(  F_TYPE)){ j.ands.push_back(i); }
		for(const auto& i : ands.at(  V_TYPE)){ j.ands.push_back(i); }
		for(const auto& i : ands.at(VAR_TYPE)){ j.ands.push_back(i); }
		for(const auto& i : ands.at(IMP_TYPE)){ j.ands.push_back(i); }
		for(const auto& i : ands.at( OR_TYPE)){ j.ands.push_back(i); }
		for(const auto& i : ands.at(NOT_TYPE)){ j.ands.push_back(i); }
		for(const auto& i : ands.at(AND_TYPE)){ j.ands.push_back(i); }
								   
		for(const auto& i : ors.at( OR_TYPE)){  j.ors.push_back(i); }
		for(const auto& i : ors.at(NOT_TYPE)){  j.ors.push_back(i); }
		for(const auto& i : ors.at(IMP_TYPE)){  j.ors.push_back(i); }
		for(const auto& i : ors.at(AND_TYPE)){  j.ors.push_back(i); }
		for(const auto& i : ors.at(VAR_TYPE)){  j.ors.push_back(i); }
		for(const auto& i : ors.at(  F_TYPE)){  j.ors.push_back(i); }
		for(const auto& i : ors.at(  V_TYPE)){  j.ors.push_back(i); }
	}

	return ris;
}

vector<Sequente> get_leaves(const Sequente& seq){
	vector<Sequente> leaves;
	vector<const Sequente *> stack;
	stack.push_back(&seq);

	while(stack.size()){
		const Sequente * s = stack.back();
		stack.pop_back();
		for(const Sequente& i : s->nexts){
			if(i.nexts.size() == 0){ leaves.push_back(i);
			}else{ stack.push_back(&i);
			}
		}
	}
	
	if(leaves.size()==0){return {seq};}
	return leaves;
}

bool isTautology(const Sequente& seq){
	auto cmp = [](const upob* i,const upob* j){ return (*i)->name() < (*j)->name() ; };
	set<const upob*,decltype(cmp)> seq_ands(seq.ands.begin(),seq.ands.end(),cmp);
	set<const upob*,decltype(cmp)> seq_ors(seq.ors.begin(),seq.ors.end(),cmp);
	set<const upob*,decltype(cmp)> intersection(cmp);

	set_intersection(seq_ands.begin(),seq_ands.end(),seq_ors.begin(),seq_ors.end(),inserter(intersection,intersection.begin()),cmp);

	return any_of(intersection.begin(),intersection.end(),[](const upob* i){ return (*i)->type() == VAR_TYPE; }) ||
		any_of(seq.ands.begin(),seq.ands.end(),[](const upob* i){ return (*i)->type() == F_TYPE; }) ||
		any_of(seq.ors.begin(),seq.ors.end(),[](const upob* i){ return (*i)->type() == V_TYPE; });
}
