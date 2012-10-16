#ifndef _PARSER_H_
#define _PARSER_H_

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <iterator>
#include <stdexcept>

class LexException: public std::runtime_error {
public:
	LexException(std::string &t): std::runtime_error("LexException: " + t) {}
};

class ParseException: public std::runtime_error {
public:
	ParseException(std::string &t): std::runtime_error("ParseException: " + t) {}
};

class ValidateException: public std::runtime_error {
public:
	ValidateException(std::string &t): std::runtime_error("ValidateException: " + t) {}
};

class NodeException: public std::runtime_error {
public:
	NodeException(std::string &t): std::runtime_error("NodeException: " + t) {}
};

class Node {
public:
	enum NodeType {
		String,Int,Float,Boolean,Map,Sequence,ObjMap,ObjSequence,Reference,Link
	};
	typedef unsigned int size_type;
private:
	friend class Parser;
	std::map<std::string,Node> _map;
	std::vector<Node> _seq;
	std::string _str; // value / class name / anchor
	NodeType _type;
	static std::map<std::string,Node> _alias_table;
public:
	Node(NodeType type=Node::String): _type(type) {}
	Node& operator[](size_type n);
	Node& operator[](std::string key);
	size_type size();
	std::string get_class_name();
	void operator>>(int &n);
	void operator>>(double &x);
	void operator>>(std::string &s);
	void operator>>(bool &b);
	void print(int indent);
private:
	void add_to_map(std::string key, Node& n);
	void add_to_seq(Node& n);
	static Node& get_anchor(std::string alias);
	void add_as_anchor(std::string alias);
};

class Parser {
	enum TokenType {
		TOK_CBRACE_L,
		TOK_CBRACE_R,
		TOK_SBRACE_L,
		TOK_SBRACE_R,
		TOK_RBRACE_L,
		TOK_RBRACE_R,
		TOK_INTERROBANG,
		TOK_ASTERISK,
		TOK_AT,
		TOK_AMPERSAND,
		TOK_COLON,
		TOK_COMMA,
		TOK_IDENTIFIER,
		TOK_STRING,
		TOK_INT,
		TOK_FLOAT,
		TOK_BOOL
	};

	struct Token {
		TokenType type;
		std::string contents;
	};

	bool _generated;
	std::istream& _is;
	std::vector<Token> _token_list;
	std::istream_iterator<char> _iit, _eos;
	std::vector<Token>::iterator _cur_token;
	bool _skip_comments;
	Node _document;
	bool **_graph;
	std::map<std::string,int> _ordering;
	int *_color;
public:
	Parser(std::istream& is);
	Node& get_document();
	void print_tokens();
private:
	void lex();
	void parse();
	void interpret();
	void set_skip_comments(bool mode);
	void next_sym();
	bool is_a_digit(char c);
	bool is_an_identifier_letter(char c);
	bool is_a_sign(char c);
	bool is_whitespace(char c);
	bool expect(TokenType t);
	bool accept(TokenType t);
	void parse_value();
	void parse_pair();
	void parse_map();
	void parse_seq();
	void parse_obj();
	Node interpret_value();
	Node interpret_map();
	Node interpret_seq();
	Node interpret_obj();
	Node interpret_ref();
	Node interpret_link();
	void check_for_cycles();
	void recursive_check(Node& n, std::string alias);
	void dfs_visit(int i);
};

std::string tokentypes[];
std::string types[];

#endif