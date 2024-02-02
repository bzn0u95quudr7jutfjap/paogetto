#pragma once
#include <string>
#include <memory>
#include <map>
#include <optional>

const char NOT_NAME='!';
const char AND_NAME='&';
const char  OR_NAME='|';
const char IMP_NAME='>';
const char   V_NAME='V';
const char   F_NAME='F';

const int VAR_TYPE=0;
const int NOT_TYPE=1;
const int AND_TYPE=2;
const int  OR_TYPE=3;
const int IMP_TYPE=4;
const int   V_TYPE=5;
const int   F_TYPE=6;


class OperatoreBinario{
	protected:
	const char _name;
	const int  _type;
	const std::unique_ptr<OperatoreBinario> _sx;
	const std::unique_ptr<OperatoreBinario> _dx;

	public:
	using upob = std::unique_ptr<OperatoreBinario>;
	char name() const;
	int type() const;
	const upob& sx() const;
	const upob& dx() const;
	friend std::ostream& operator<<(std::ostream&, const upob&);
	friend std::ostream& operator<<(std::ostream&, const upob*);
	OperatoreBinario(char n,upob s,upob d,int);
	virtual bool eval(const std::map<char,bool>&) = 0;
	virtual ~OperatoreBinario(){}
};

using upob = std::unique_ptr<OperatoreBinario>;

class Var : public OperatoreBinario{
	public:
		Var(char n);
		bool eval(const std::map<char,bool>&);
};

class Not : public OperatoreBinario{
	public:
		Not(upob o);
		bool eval(const std::map<char,bool>&);
};

class And : public OperatoreBinario{
	public:
		And(upob o,upob p);
		bool eval(const std::map<char,bool>&);
};

class Or : public OperatoreBinario{
	public:
		Or(upob o,upob p);
		bool eval(const std::map<char,bool>&);
};

class Imp : public OperatoreBinario{
	public:
		Imp(upob o,upob p);
		bool eval(const std::map<char,bool>&);
};

class Vero : public OperatoreBinario {
	public:
		Vero();
		bool eval(const std::map<char,bool>&);
};

class Falso : public OperatoreBinario {
	public:
		Falso();
		bool eval(const std::map<char,bool>&);
};

std::optional<upob> parse(const std::string&);
std::map<char,bool> make_env(const upob&);
