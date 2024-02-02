#include <map>
#include <string>
#include <vector>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <cmath>
#include <optional>

#include "OperatoreBinario.hpp"
#include "Sequente.hpp"
#include "Shell.hpp"

using namespace std;

vector<string> Shell::parse(string s){
	vector<string> res;
	istringstream is(s+" "); //Viene aggiunto spazio alla fine per non perdere l'ultimo componente del comando
	istringstream js;
	ostringstream os;
	string arg;

	bool (*is_isolated_backslash)(const string&) = [](const string& arg){ return arg.size() == 1 && arg.at(0) == '\\';};
	bool (*is_ending_unescaped_backslash)(const string&) = [](const string& arg){
		return arg.size() >= 2 && arg.back() == '\\' && *(arg.end()-2) != '\\';
	};

	while(!(is >> arg).eof()){
		os.str(string());
		os << arg;
		while(is_isolated_backslash(arg) || is_ending_unescaped_backslash(arg)){
			os.seekp(-1,os.end) << (char) is.get();
			is >> arg;
			os << arg;
		}
		js.seekg(0,js.beg);
		js.str(os.str());
		os.str(string());
		while(!getline(js,arg,'\\').eof()){
			os << arg << (char) js.get();
		}
		os << arg;
		res.push_back(os.str());
	}
	return res;
}

Shell::Context Shell::build_context(const std::vector<std::string>& cmd, const std::map<std::string, Shell::Collectible>& elements){
	using context_function = bool(*)(const std::map<std::string, Shell::Collectible>&, Context& );
	context_function CONTAINS_SEQUENT = [](const std::map<std::string, Shell::Collectible>& elements, const std::string& arg, Context& c){
		auto e = elements.find(arg);
		if(e == elements.end()){
			c.set_exit_code(MISSING_SEQUENT);
			return false;
		}else{
			c.set_active_element(*e);
			c.set_exit_code(SUCCESS);
			return true;
		}
	};
	context_function NOT_CONTAINS_SEQUENT = [](const std::map<std::string, Shell::Collectible>& elements, const std::string& arg, Context& c){
		auto e = elements.find(arg);
		if(e != elements.end()){
			c.set_exit_code(SEQUENT_ALREADY_EXISTS);
			return false;
		}else{
			c.set_new_element(parse(arg));
			c.set_exit_code(SUCCESS);
			return true;
		}
	};
	context_function IS_FILE_VALID = [](const std::map<std::string, Shell::Collectible>& elements, const std::string& arg, Context& c){
		auto e = elements.find(arg);
		if(e != elements.end()){
			c.set_exit_code(SEQUENT_ALREADY_EXISTS);
			return false;
		}else{
			c.set_new_element(parse(arg));
			c.set_exit_code(SUCCESS);
			return true;
		}
	};
	const std::map<std::string, std::vector<context_function>> grammatica = {
		{CMD_SINTASSI   , {} },
		
		{CMD_POP        , {CONTAINS_SEQUENT} },
		{CMD_BUILD_TREE , {CONTAINS_SEQUENT} },
		{CMD_NEGATE     , {CONTAINS_SEQUENT} },
		{CMD_EVAL       , {CONTAINS_SEQUENT} },
		{CMD_EVAL_ALL   , {CONTAINS_SEQUENT} },

		{CMD_SET_ENV    , {CONTAINS_SEQUENT, } },
		{CMD_PUSH_AX    , {CONTAINS_SEQUENT, } },
		{CMD_POP_AX     , {CONTAINS_SEQUENT, } },
		{CMD_LIST_AX    , {CONTAINS_SEQUENT, } },

		{CMD_PUSH       , {NOT_CONTAINS_SEQUENT} },

		{CMD_LOAD       , {IS_FILE_VALID} },
		{CMD_RESET      , {IS_FILE_VALID} },
		{CMD_EXEC       , {IS_FILE_VALID} },
		{CMD_SAVE       , {IS_FILE_VALID} },

		{CMD_HELP       , {} },
		{CMD_LIST       , {} }
	};
	return Context(nullptr,nullptr,nullptr,"");
}

//class Collectible{
//	public:
//		upob expression;
//		std::map<char, bool> environment;
//		Sequente sequente;
//		std::map<std::string,upob> axioms;
//		Collectible(upob&);
//};

using shell_func = std::string (*)(const std::vector<std::string>&,std::map<std::string,Shell::Collectible>&);
using name_shell_func = std::pair<std::string,shell_func>;
using name_upob = std::pair<std::string,upob>;

Shell::Collectible::Collectible(upob& expr) {
	expression = std::move(expr);
	sequente = expression;
	environment = make_env(expression);
}

ostream& operator<<(ostream& os,const map<char,bool>& v){
	if(v.size() == 0){ return os << "()"; }
	os << "(";
	auto j = v.begin(), i = j++;
	for(; j != v.end(); i++,j++){ os << i->first << ":" << i->second << ", "; }
	return os << i->first << ":" << i->second << ")" ;
}

bool contains(const map<string,Shell::Collectible>&        m,const string& k){ return m.find(k) != m.end(); }
bool contains(const map<string,string>&             m,const string& k){ return m.find(k) != m.end(); }
bool contains(const map<string,upob>&               m,const string& k){ return m.find(k) != m.end(); }
bool contains(const map<char,bool>&                 m,const char&   k){ return m.find(k) != m.end(); }

string FUNC_LIST(const vector<string>& args,map<std::string,Shell::Collectible>& elements){
	ostringstream os;
	if(args.size() < 2){ 
		for(const auto& [k,v] : elements ){ os << k << endl; }
	}else{
		for(const auto& [k,v] : elements ){ if( (int) k.find(args.at(1)) > -1 ){ os << k << endl; } }
	}
	string ot = os.str();
	if(ot.size()) { ot.erase(ot.size()-1,ot.size()); }
	return ot;
}
string FUNC_LIST_AX(const vector<string>& args,map<std::string,Shell::Collectible>& elements){
	if(args.size() < 2){ return Shell::eval({CMD_HELP,args.at(0)}); }
	if(!contains(elements,args.at(1))){ return string("SEQUENTE NON PRESENTE"); }
	Shell::Collectible& c = elements.at(args.at(1));
	ostringstream os;
	if(args.size() < 3){ 
		for(const auto& [k,v] : c.axioms ){ os << k << endl; }
	}else{
		for(const auto& [k,v] : c.axioms ){ if( (int) k.find(args.at(2)) > -1 ){ os << k << endl; } }
	}
	string ot = os.str();
	if(ot.size()) { ot.erase(ot.size()-1,ot.size()); }
	return ot;
}

string FUNC_PUSH(const vector<string>& args,map<std::string,Shell::Collectible>& elements){
	if(args.size() < 2){ return Shell::eval({CMD_HELP,args.at(0)}); }
	if(contains(elements,args.at(1))){ return string("SEQUENTE GIÀ PRESENTE"); }
	optional<upob> expr = parse(args.at(1));
	if(!expr.has_value()){ return string("SEQUENTE NON PARSABILE CORRETTAMENTE"); }
	if((*expr)->type() != IMP_TYPE){ return string("UN ELEMENTE DEVE COMINCIARE CON UN IMPLICAZIONE ( > )"); }
	elements.emplace(args.at(1),*expr);
	ostringstream os;
	os << "+ " << elements.at(args.at(1)).expression;
	return os.str();
}
string FUNC_PUSH_AX(const vector<string>& args,map<std::string,Shell::Collectible>& elements){
	if(args.size() < 3){ return Shell::eval({CMD_HELP,args.at(0)}); }
	if(!contains(elements,args.at(1))){ return string("SEQUENTE NON PRESENTE"); }
	Shell::Collectible& c = elements.at(args.at(1));
	if(contains(c.axioms,args.at(2))){ return string("ASSIOMA GIÀ PRESENTE"); }
	optional<upob> expr = parse(args.at(2));
	if(!expr.has_value()){ return string("ASSIOMA NON PARSABILE CORRETTAMENTE"); }
	c.axioms.emplace(args.at(2),std::move(*expr));
	push_ax(c.sequente,c.axioms.at(args.at(2)));
	ostringstream os;
	os << "+ " << c.axioms.at(args.at(2)) << " @ " << c.expression ;
	return os.str();
}
string FUNC_POP(const vector<string>& args,map<std::string,Shell::Collectible>& elements){
	if(args.size() < 2){ return Shell::eval({CMD_HELP,args.at(0)}); }
	if(!contains(elements,args.at(1))){ return string("SEQUENTE NON PRESENTE"); }
	elements.erase(args.at(1));
	ostringstream os;
	os << "- " + args.at(1);
	return os.str();
}
string FUNC_POP_AX(const vector<string>& args,map<std::string,Shell::Collectible>& elements){
	if(args.size() < 3){ return Shell::eval({CMD_HELP,args.at(0)}); }
	if(!contains(elements,args.at(1))){ return string("SEQUENTE NON PRESENTE"); }
	Shell::Collectible& c = elements.at(args.at(1));
	if(!contains(c.axioms,args.at(2))){ return string("ASSIOMA NON PRESENTE"); }
	pop_ax(c.sequente,c.axioms.at(args.at(2)));
	c.axioms.erase(args.at(2));
	ostringstream os;
	os << "- " << args.at(2) << " @ " << args.at(1);
	return os.str();
}

string FUNC_NEGATE(const vector<string>& args,map<std::string,Shell::Collectible>&){
	if(args.size() < 2){ return Shell::eval({CMD_HELP,args.at(0)}); }
	static auto isNegated = [](const string& e){ return e.find(">V!") == 0; };
	return isNegated(args.at(1)) ? args.at(1).substr(3) : string(">V!") + args.at(1);
}

string FUNC_BUILD_TREE(const vector<string>& args,map<std::string,Shell::Collectible>& elements){
	if(args.size() < 2){ return Shell::eval({CMD_HELP,args.at(0)}); }
	if(!contains(elements,args.at(1))){ return string("SEQUENTE NON PRESENTE"); }
	Shell::Collectible& c = elements.at(args.at(1));
	ostringstream os;
	buildTree(c.sequente);
	beautify(os,c.sequente,"") << endl << endl;
	bool check_tautology_of_childs = true;
	for(const Sequente& l : get_leaves(c.sequente)){
		bool check_tautology = isTautology(l);
		check_tautology_of_childs = check_tautology_of_childs && check_tautology;
		os << l << " <- " <<( check_tautology ? "TAUTOLOGIA" : "OPINIONE" )<< endl;
	}
	if(check_tautology_of_childs) { os << "TAUTOLOGIA"; return os.str(); }
	string sequente_negato = Shell::eval({CMD_NEGATE,args.at(1)}) ;
	os << endl;
	os << "Negazione" << endl;
	os << Shell::eval({CMD_PUSH,sequente_negato}) << endl ;
	Shell::Collectible& anti_c = elements.at(sequente_negato);
	buildTree(anti_c.sequente);
	beautify(os,anti_c.sequente,"") << endl << endl;
	check_tautology_of_childs = true;
	for(const Sequente& l : get_leaves(anti_c.sequente)){
		bool check_tautology = isTautology(l);
		check_tautology_of_childs = check_tautology_of_childs && check_tautology;
		os << l << " <- " <<( check_tautology ? "TAUTOLOGIA" : "OPINIONE" )<< endl;
	}
	os << Shell::eval({CMD_POP,sequente_negato}) << endl;
	os << ((check_tautology_of_childs) ? "PARADOSSO" : "OPINIONE");
	return os.str() ;
}

string FUNC_SET_ENV(const vector<string>& args,map<std::string,Shell::Collectible>& elements){
	if(args.size() < 4){ return Shell::eval({CMD_HELP,args.at(0)}); }
	if(!contains(elements,args.at(1))){ return string("SEQUENTE NON PRESENTE"); }
	Shell::Collectible& c = elements.at(args.at(1));
	if(!contains(c.environment,args.at(2).at(0))){ return string("VARIABILE NON PRESENTE"); }
	c.environment.at(args.at(2).at(0)) = (args.at(3).at(0) == '1' ? 1 : 0);
	ostringstream os;
	os << args.at(2) << " -> " << c.environment.at(args.at(2).at(0)) << " @ " << args.at(1);
	return os.str();
}
string FUNC_EVAL(const vector<string>& args,map<std::string,Shell::Collectible>& elements){
	if(args.size() < 2){ return Shell::eval({CMD_HELP,args.at(0)}); }
	if(!contains(elements,args.at(1))){ return string("SEQUENTE NON PRESENTE"); }
	Shell::Collectible& c = elements.at(args.at(1));
	ostringstream os;
	os << c.environment << " = " << c.expression->eval(c.environment);
	return os.str();
}

string FUNC_EVAL_ALL(const vector<string>& args,map<std::string,Shell::Collectible>& elements){
	if(args.size() < 2){ return Shell::eval({CMD_HELP,args.at(0)}); }
	if(!contains(elements,args.at(1))){ return string("SEQUENTE NON PRESENTE"); }
	ostringstream os;
	Shell::Collectible& c = elements.at(args.at(1));
	vector<int> results(pow(2,c.environment.size()));
	for(size_t i = 0; i < results.size(); i++){
		size_t j = 0;
		for(auto& [k,v] : c.environment){ v = (i >> j++)%2; }
		results.at(i) = c.expression->eval(c.environment);
		os << c.environment << " = " << results.at(i) << endl;
	}
	for(auto& [k,v] : c.environment){v = 0;};
	if(all_of(results.begin(),results.end(),[](const bool& i){return i;})){ os << "TAUTOLOGIA";
	}else if(none_of(results.begin(),results.end(),[](const bool& i){return i;})){ os << "PARADOSSO";
	}else { os << "OPINIONE"; }
	return os.str();
}

string FUNC_EXEC(const vector<string>& args,map<std::string,Shell::Collectible>&){
	if(args.size() < 2){ return Shell::eval({CMD_HELP,args.at(0)}); }
	ifstream input_file(args.at(1));
	if(!input_file.is_open()){ return string("FILE INVALIDO"); }
	ostringstream os;
	string line;
	while(getline(input_file,line)){
		os << ">>> " << line << endl;
		line = Shell::eval(Shell::parse(line)); 
		if(line.size()){ os << line << endl; }
	}
	string ot = os.str();
	ot.erase(ot.end()-1,ot.end());
	return ot;
}
string FUNC_RESET(const vector<string>&,map<std::string,Shell::Collectible>& elements){
	elements.clear();
	return string("STATO SVUOTATO");
}
string FUNC_SAVE(const vector<string>& args,map<std::string,Shell::Collectible>& elements){
	if(args.size() < 2){ return Shell::eval({CMD_HELP,args.at(0)}); }
	ofstream os(args.at(1));
	if(!os.is_open()){ return string("FILE INVALIDO"); }
	for(const auto& [k,v] : elements){
		os << CMD_PUSH << " " << k << endl;
		for(const auto& [ak,av] : v.axioms){ os << CMD_PUSH_AX << " " << k << " " << ak << endl; }
		for(const auto& [ek,ev] : v.environment){ if(ev){ os << CMD_SET_ENV << " " << k << " " << ek << " " << ev << endl; } }
	}
	return string("SALAVTO CON SUCCESSO");
}
string FUNC_LOAD(const vector<string>& args,map<std::string,Shell::Collectible>&){
	if(args.size() < 2){ return Shell::eval({CMD_HELP,args.at(0)}); }
	ostringstream os;
	os << Shell::eval({CMD_RESET}) << endl;
	os << Shell::eval({CMD_EXEC,args.at(1)}) << endl;
	os << "CARICATO IL NUOVO STATO DELLA MACCHINA";
	return os.str();
}

string FUNC_SINTASSI(const vector<string>&,map<std::string,Shell::Collectible>&){
	return Shell::eval({CMD_HELP,CMD_SINTASSI});
}

string FUNC_HELP(const vector<string>& args,map<std::string,Shell::Collectible>&){
	static const vector<pair<string,string>>& man_pages = {
		{CMD_HELP      ,"SINTASSI: "+CMD_HELP       +" <CMD>\nStampa come utilizzare il comando\nSenza argomenti stampa tutti i comandi"},
		{CMD_PUSH      ,"SINTASSI: "+CMD_PUSH       +" <SEQUENTE>\nAggiunge l'elemento"},
		{CMD_POP       ,"SINTASSI: "+CMD_POP        +" <SEQUENTE>\nRimuove l'elemento"},
		{CMD_PUSH_AX   ,"SINTASSI: "+CMD_PUSH_AX    +" <SEQUENTE> <ASSIOMA>\nAggiunge l'assioma all'elemento"},
		{CMD_POP_AX    ,"SINTASSI: "+CMD_POP_AX     +" <SEQUENTE> <ASSIOMA>\nRimuove l'assioma dall'elemento"},
		{CMD_NEGATE    ,"SINTASSI: "+CMD_NEGATE     +" <SEQUENTE>\nRestituisce la negazione del sequente. Non modifica lo stato del processo"},
		{CMD_BUILD_TREE,"SINTASSI: "+CMD_BUILD_TREE +" <SEQUENTE>\nSviluppa il sequente e la negazione (se necessario) e restituisce il risultato"},
		{CMD_SET_ENV   ,"SINTASSI: "+CMD_SET_ENV    +" <SEQUENTE> <VARIABILE> <VALORE>\nImposta la variabile d'ambiente dell'elemento\nI valori consentiti solo 0 o 1, in caso di errore la variabile sarà imposata a 0"},
		{CMD_EVAL      ,"SINTASSI: "+CMD_EVAL       +" <SEQUENTE>\nRisolve l'elemento dato l'ambiente"},
		{CMD_EVAL_ALL  ,"SINTASSI: "+CMD_EVAL_ALL   +" <SEQUENTE>\nEsegue un eval per ogni ambiente possibile ( 2^(Numbero di variabili) )"},
		{CMD_LIST      ,"SINTASSI: "+CMD_LIST       +" <TESTO>\nStampa tutti i sequenti che contengono TESTO.\nSe TESTO non viene specificato, vengono stampati tutti i sequenti."},
		{CMD_LIST_AX   ,"SINTASSI: "+CMD_LIST_AX    +" <SEQUENTE> <TESTO>\nStampa tutti gli assiomi del SEQUENTE che contengono TESTO.\nSe TESTO non viene specificato, vengono stampati tutti gli assiomi."},
		{CMD_RESET     ,"SINTASSI: "+CMD_RESET      +" \nRipristina il processo ad uno stato vuoto"},
		{CMD_EXEC      ,"SINTASSI: "+CMD_EXEC       +" <FILE>\nEsegue il file linea per linea e modifica lo stato corrente"},
		{CMD_SAVE      ,"SINTASSI: "+CMD_SAVE       +" <FILE>\nSalva su un file lo stato attuale"},
		{CMD_LOAD      ,"SINTASSI: "+CMD_LOAD       +" <FILE>\nResetta allo stato iniziale ed esegue il file"},
		{CMD_SINTASSI  ,"SINTASSI\nGli operatori logici sono not : !, and : &, or : |, implica : >.\nI valori vero e falso sono rispettivamente: V e F.\nOgni lettera minuscola è considerata una variabile.\nI sequenti e gli assiomi sono scritti in notazione polacca (Es. a -> !( b & a ) : >a!&ba ).\nI sequenti devono iniziare con un implica ( > )."}
	};


	if(args.size() == 1){
		ostringstream os;
		os << endl;
		for(const auto& [v,k] : man_pages){ os << k << endl << endl; }
		return os.str();
	}

	for(auto& [k,v] : man_pages){ if(args.at(1).compare(k)==0){ return v;} }
	return string("COMANDO INESISTENTE");
}

std::string Shell::eval(const std::vector<std::string>& args){

	static map<string,Shell::Collectible> elements;
	static map<string,shell_func> commands = {
		{CMD_HELP      ,FUNC_HELP      },
		{CMD_LIST      ,FUNC_LIST      },
		{CMD_LIST_AX   ,FUNC_LIST_AX   },
		{CMD_RESET     ,FUNC_RESET     },
		{CMD_EXEC      ,FUNC_EXEC      },
		{CMD_LOAD      ,FUNC_LOAD      },
		{CMD_SAVE      ,FUNC_SAVE      },
		{CMD_PUSH      ,FUNC_PUSH      },
		{CMD_POP       ,FUNC_POP       },
		{CMD_PUSH_AX   ,FUNC_PUSH_AX   },
		{CMD_POP_AX    ,FUNC_POP_AX    },
		{CMD_NEGATE    ,FUNC_NEGATE    },
		{CMD_BUILD_TREE,FUNC_BUILD_TREE},
		{CMD_SET_ENV   ,FUNC_SET_ENV   },
		{CMD_EVAL_ALL  ,FUNC_EVAL_ALL  },
		{CMD_EVAL      ,FUNC_EVAL      },
		{CMD_SINTASSI  ,FUNC_SINTASSI  },
	};

	if(args.size() == 0){ return ""; }
	if(commands.find(args.at(0)) == commands.end()){
		return string("COMANDO INVALIDO: ") + args.at(0);
	}
	return commands.at(args.at(0))(args,elements);
}
