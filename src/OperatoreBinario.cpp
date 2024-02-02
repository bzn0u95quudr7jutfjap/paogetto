#include <memory>
#include <vector>
#include <string>
#include <iostream>
#include <optional>

#include "OperatoreBinario.hpp"

using namespace std;
using upob = std::unique_ptr<OperatoreBinario>;

OperatoreBinario::OperatoreBinario(char n,upob s,upob d, int t): _name(n), _type(t), _sx(std::move(s)), _dx(std::move(d)) {}
Var::Var(char n)        : OperatoreBinario(n,nullptr,nullptr,VAR_TYPE) {}
Not::Not(upob o)        : OperatoreBinario(NOT_NAME,std::move(o),nullptr,NOT_TYPE) {} 
And::And(upob o,upob p) : OperatoreBinario(AND_NAME,std::move(o),std::move(p),AND_TYPE) {} 
 Or:: Or(upob o,upob p) : OperatoreBinario( OR_NAME,std::move(o),std::move(p), OR_TYPE) {} 
Imp::Imp(upob o,upob p) : OperatoreBinario(IMP_NAME,std::move(o),std::move(p),IMP_TYPE) {} 
Vero::Vero()            : OperatoreBinario(V_NAME,nullptr,nullptr,V_TYPE){};
Falso::Falso()          : OperatoreBinario(F_NAME,nullptr,nullptr,F_TYPE){};

char        OperatoreBinario::name() const { return _name; }
int         OperatoreBinario::type() const { return _type; }
const upob& OperatoreBinario::sx()   const { return _sx; }
const upob& OperatoreBinario::dx()   const { return _dx; }

bool   Var::eval(const map<char,bool>& env){ return env.at(_name); }
bool   Not::eval(const map<char,bool>& env){ return  !(_sx->eval(env)); }
bool   And::eval(const map<char,bool>& env){ return   (_sx->eval(env)  && (_dx->eval(env))); }
bool    Or::eval(const map<char,bool>& env){ return   (_sx->eval(env)  || (_dx->eval(env))); }
bool   Imp::eval(const map<char,bool>& env){ return (!(_sx->eval(env)) || (_dx->eval(env))); }
bool  Vero::eval(const map<char,bool>& ){ return true;}
bool Falso::eval(const map<char,bool>& ){ return false;}

std::ostream& operator<<(std::ostream& os,const OperatoreBinario& r){
	return os << r.name();
}

std::ostream& operator<<(std::ostream& os,const upob& r){
	os << r->name();
	if(r->sx() != nullptr){ os << r->sx(); }
	if(r->dx() != nullptr){ os << r->dx(); }
	return os;
}

optional<upob> parse(const string& s){
	auto pop_back = [](vector<upob>& v){ 
		upob pop = std::move(v.back());
		v.pop_back();
		return pop;
	};

	vector<upob> stack;
	for(int i=s.size()-1; 0 <= i ; i--){
		if('a' <= s.at(i) && s.at(i) <= 'z'){
			stack.push_back(make_unique<Var>(s.at(i)));
		}else if(s.at(i) == V_NAME){	
			stack.push_back(make_unique<Vero>());
		}else if(s.at(i) == F_NAME){	
			stack.push_back(make_unique<Falso>());
		}else{
			if(s.at(i) == NOT_NAME){
				if(stack.size() < 1){ return optional<upob>(); }
				stack.push_back(make_unique<Not>(pop_back(stack)));
			}else{
				if(stack.size() < 2){ return optional<upob>(); }
				upob a = pop_back(stack);
				upob b = pop_back(stack);
				if(s.at(i) == AND_NAME){
					stack.push_back(make_unique<And>(std::move(a),std::move(b)));
				}else if(s.at(i) ==  OR_NAME){
					stack.push_back(make_unique< Or>(std::move(a),std::move(b)));
				}else if(s.at(i) == IMP_NAME){
					stack.push_back(make_unique<Imp>(std::move(a),std::move(b)));
				}else{
					return optional<upob>();
				}
			}
		}
	}
	if(stack.size() != 1){ return optional<upob>(); }
	return make_optional<upob>(std::move(stack.front()));
}

map<char,bool> make_env(const upob& r){
	map<char, bool> env;
	vector<const upob*> stack;
	stack.push_back(&r);
	while(stack.size()){
		const upob* s = stack.back();
		stack.pop_back();
		if((*s)->type() == VAR_TYPE){ 
			env.emplace((*s)->name(),false);
		}else{
			if((*s)->sx() != nullptr){ stack.push_back(&((*s)->sx())); }
			if((*s)->dx() != nullptr){ stack.push_back(&((*s)->dx())); }
		}
	}
	return env;
}
