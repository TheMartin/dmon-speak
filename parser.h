#ifndef _PARSER_H_
#define _PARSER_H_

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <iterator>
#include <stdexcept>

class LexException: public std::runtime_error {
	unsigned int _line;
public:
	LexException(unsigned int l, std::string &t): std::runtime_error("LexException: " + t), _line(l) {}
	unsigned int line() {return _line;}
};

class ParseException: public std::runtime_error {
	unsigned int _line;
public:
	ParseException(unsigned int l, std::string &t): std::runtime_error("ParseException: " + t), _line(l) {}
	unsigned int line() {return _line;}
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
	class iterator;
	friend class Parser;
	NodeType _type;
protected:
	static std::map<std::string,Node*> _alias_table;
public:
	Node(NodeType type=Node::String): _type(type) {}
	NodeType get_type() {return _type;}
	virtual Node& operator[](size_type n);
	virtual Node& operator[](std::string key);
	virtual size_type size();
	virtual std::string get_class_name();
	virtual void set_class_name(std::string name);
	virtual void operator>>(int &n);
	virtual void operator>>(double &x);
	virtual void operator>>(std::string &s);
	virtual void operator>>(bool &b);
	virtual void print(int indent);
	virtual iterator begin();
	virtual iterator end();
protected:
	virtual void set_contents(std::string contents);
	virtual void set_target(std::string target);
	virtual std::string get_target();
	virtual void add_to_map(std::string key, Node* n);
	virtual void add_to_seq(Node* n);
	static Node& get_anchor(std::string alias);
	void add_as_anchor(std::string alias);
};

class iterNodeImpl;
class iterNodeMapImpl;
class iterNodeSeqImpl;

class iterNodeImpl {
public:
	virtual iterNodeImpl* clone() = 0;
	virtual void increment() = 0;
	virtual bool equals(iterNodeImpl& rhs) = 0;
	virtual bool equal_accept(iterNodeMapImpl& rhs) = 0;
	virtual bool equal_accept(iterNodeSeqImpl& rhs) = 0;
	virtual Node* dereference() = 0;
};

class iterNodeMapImpl: public iterNodeImpl {
	std::map<std::string,Node*>::iterator it;
public:
	iterNodeMapImpl(std::map<std::string,Node*>::iterator iter);
	iterNodeMapImpl(const iterNodeMapImpl& rhs);
	iterNodeImpl* clone();
	void increment();
	bool equals(iterNodeImpl& rhs);
	bool equal_accept(iterNodeMapImpl& rhs);
	bool equal_accept(iterNodeSeqImpl& rhs);
	Node* dereference();
};

class iterNodeSeqImpl: public iterNodeImpl {
	std::vector<Node*>::iterator it;
public:
	iterNodeSeqImpl(std::vector<Node*>::iterator iter);
	iterNodeSeqImpl(const iterNodeSeqImpl& rhs);
	iterNodeImpl* clone();
	void increment();
	bool equals(iterNodeImpl& rhs);
	bool equal_accept(iterNodeMapImpl& rhs);
	bool equal_accept(iterNodeSeqImpl& rhs);
	Node* dereference();
};

class Node::iterator {
	iterNodeImpl *_impl;
public:
	iterator();
	iterator(std::map<std::string,Node*>::iterator iter);
	iterator(std::vector<Node*>::iterator iter);
	iterator(const iterator& rhs);
	iterator& operator=(const iterator& rhs);
	~iterator();
	iterator& operator++();
	iterator operator++(int);
	bool operator==(const iterator& rhs);
	bool operator!=(const iterator& rhs);
	Node* operator*();
};

class NodeMap: public Node {
protected:
	std::map<std::string,Node*> _map;
public:
	NodeMap(Node::NodeType type = Node::Map): Node(type) {}
	Node& operator[](std::string key);
	size_type size();
	void add_to_map(std::string key, Node* n);
	iterator begin();
	iterator end();
	virtual void print(int indent);
};

class NodeSeq: public Node {
protected:
	std::vector<Node*> _seq;
public:
	NodeSeq(Node::NodeType type = Node::Sequence): Node(type) {}
	Node& operator[](size_type n);
	size_type size();
	void add_to_seq(Node *n);
	iterator begin();
	iterator end();
	virtual void print(int indent);
};

class NodeObjMap: public NodeMap {
protected:
	std::string _name;
public:
	NodeObjMap(): NodeMap(Node::ObjMap) {}
	std::string get_class_name();
	void set_class_name(std::string name);
	void print(int indent);
};

class NodeObjSeq: public NodeSeq {
protected:
	std::string _name;
public:
	NodeObjSeq(): NodeSeq(Node::ObjSequence) {}
	std::string get_class_name();
	void set_class_name(std::string name);
	void print(int indent);
};

class NodeRef: public Node {
protected:
	std::string _target;
	void set_target(std::string target);
	std::string get_target();
public:
	NodeRef(Node::NodeType type = Node::Reference): Node(type) {}
	Node& operator[](size_type n);
	Node& operator[](std::string key);
	size_type size();
	std::string get_class_name();
	void set_class_name(std::string name);
	void operator>>(int &n);
	void operator>>(double &x);
	void operator>>(std::string &s);
	void operator>>(bool &b);
	void print(int indent);
};

class NodeLink: public NodeRef {
public:
	NodeLink(): NodeRef(Node::Link) {}
};

class NodeLiteral: public Node {
protected:
	std::string _str;
	void set_contents(std::string contents);
public:
	NodeLiteral(Node::NodeType type = Node::String): Node(type) {}
	virtual void print(int indent);
};

class NodeBool: public NodeLiteral {
public:
	NodeBool(): NodeLiteral(Node::Boolean) {}
	void operator>>(bool &b);
};

class NodeInt: public NodeLiteral {
public:
	NodeInt(): NodeLiteral(Node::Int) {}
	void operator>>(int &n);
};

class NodeFloat: public NodeLiteral {
public:
	NodeFloat(): NodeLiteral(Node::Float) {}
	void operator>>(double &x);
};

class NodeString: public NodeLiteral {
public:
	NodeString(): NodeLiteral(Node::String) {}
	void operator>>(std::string &s);
	void print(int indent);
};

class Parser {
	enum TokenType {
		TOK_CBRACE_L,
		TOK_CBRACE_R,
		TOK_SBRACE_L,
		TOK_SBRACE_R,
		TOK_RBRACE_L,
		TOK_RBRACE_R,
		TOK_BANG,
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
		unsigned int line;
	};

	unsigned int _cur_line;
	bool _generated;
	std::istream& _is;
	std::vector<Token> _token_list;
	std::istream_iterator<char> _iit, _eos;
	std::vector<Token>::iterator _cur_token;
	bool _skip_comments;
	Node *_document;
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
	Node* interpret_value();
	Node* interpret_map();
	Node* interpret_seq();
	Node* interpret_obj();
	Node* interpret_ref();
	Node* interpret_link();
	void check_for_cycles();
	void recursive_check(Node* n, std::string alias);
	void dfs_visit(int i);
};

std::string tokentypes[];
std::string types[];

#endif