#pragma once
#include <string>
#include <vector>
#include <map>
#include <fstream>

#include "OperatoreBinario.hpp"
#include "Sequente.hpp"

const std::string CMD_PUSH       = "push"      ;
const std::string CMD_POP        = "pop"       ;
const std::string CMD_BUILD_TREE = "buildtree" ;
const std::string CMD_NEGATE     = "negate"    ;
const std::string CMD_PUSH_AX    = "pushax"    ;
const std::string CMD_POP_AX     = "popax"     ;
const std::string CMD_SET_ENV    = "setenv"    ;
const std::string CMD_EVAL       = "eval"      ;
const std::string CMD_EVAL_ALL   = "evalall"   ;
const std::string CMD_LOAD       = "load"      ;
const std::string CMD_RESET      = "reset"     ;
const std::string CMD_EXEC       = "exec"      ;
const std::string CMD_SAVE       = "save"      ;
const std::string CMD_HELP       = "help"      ;
const std::string CMD_SINTASSI   = "sintassi"  ;
const std::string CMD_LIST       = "list"      ;
const std::string CMD_LIST_AX    = "listax"    ;

class Shell{
	public:
	class Collectible{
		public:
			upob expression;
			std::map<char, bool> environment;
			Sequente sequente;
			std::map<std::string,upob> axioms;
			Collectible(upob&);
	};
	class Context{
		public:
			upob new_element;
			const Collectible* active_element;
			std::fstream* file;
			const std::string& exit_code;
			Context(const upob&, const Collectible*, const std::fstream*, const std::string&);
	};
	static Context build_context(const std::vector<std::string>&, const std::map<std::string,Collectible>&);
	static std::vector<std::string> parse(std::string);
	static std::string eval(const std::vector<std::string>& args);
};
