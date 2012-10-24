#include "parser.h"
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <sstream>

std::map<std::string,Node*> Node::_alias_table;

Node& Node::operator [](size_type n) {
	throw NodeException(std::string("type mismatch"));
}

Node& Node::operator [](std::string key) {
	throw NodeException(std::string("type mismatch"));
}

Node::size_type Node::size() {
	throw NodeException(std::string("type mismatch"));
}

std::string Node::get_class_name() {
	throw NodeException(std::string("type mismatch"));
}

void Node::set_class_name(std::string name) {
	throw NodeException(std::string("type mismatch"));
}

void Node::operator >>(int &n) {
	throw NodeException(std::string("type mismatch"));
}

void Node::operator >>(double &x) {
	throw NodeException(std::string("type mismatch"));
}

void Node::operator >>(std::string &s) {
	throw NodeException(std::string("type mismatch"));
}

void Node::operator >>(bool &b) {
	throw NodeException(std::string("type mismatch"));
}

void Node::print(int indent) {
	throw NodeException(std::string("attempting to print an invalid node"));
}

Node::iterator Node::begin() {
	throw NodeException(std::string("invalid access"));
}

Node::iterator Node::end() {
	throw NodeException(std::string("invalid access"));
}

void Node::set_contents(std::string contents) {
	throw NodeException(std::string("invalid access"));
}

void Node::set_target(std::string target) {
	throw NodeException(std::string("invalid access"));
}

std::string Node::get_target() {
	throw NodeException(std::string("invalid access"));
}

void Node::add_to_map(std::string key, Node *n) {
	throw NodeException(std::string("type mismatch"));
}

void Node::add_to_seq(Node *n) {
	throw NodeException(std::string("type mismatch"));
}

void Node::add_as_anchor(std::string alias) {
	_alias_table.insert( std::pair<std::string, Node*> (alias, this) );
}

Node& Node::get_anchor(std::string alias) {
	if (_alias_table.count(alias) != 0) {
		return *(_alias_table[alias]);
	} else throw NodeException(std::string("unknown alias"));
}

Node& NodeMap::operator [](std::string key) {
	if (_map.count(key) != 0) {
		return *(_map[key]);
	} else {
		throw NodeException(std::string("invalid access"));
	}
}

Node::size_type NodeMap::size() {
	return (size_type)_map.size();
}

void NodeMap::add_to_map(std::string key, Node *n) {
	_map.insert( std::pair<std::string,Node*>(key,n) );
}

Node::iterator NodeMap::begin() {
	return Node::iterator(_map.begin());
}

Node::iterator NodeMap::end() {
	return Node::iterator(_map.end());
}

void NodeMap::print(int indent) {
	for (int i = 0; i < indent; i++) {
		std::cout << " ";
	}
	std::cout << types[_type] << " ";
	std::cout << ":" << std::endl;
	for (std::map<std::string,Node*>::iterator it = _map.begin(); it != _map.end(); it++) {
		(*it).second->print(indent+1);
	}
}

Node& NodeSeq::operator [](size_type n) {
	if (_seq.size() > n) {
		return *(_seq[n]);
	} else {
		throw NodeException(std::string("invalid access"));
	}
}

Node::size_type NodeSeq::size() {
	return _seq.size();
}

void NodeSeq::add_to_seq(Node *n) {
	_seq.push_back(n);
}

Node::iterator NodeSeq::begin() {
	return Node::iterator(_seq.begin());
}

Node::iterator NodeSeq::end() {
	return Node::iterator(_seq.end());
}

void NodeSeq::print(int indent) {
	for (int i = 0; i < indent; i++) {
		std::cout << " ";
	}
	std::cout << types[_type] << " ";
	std::cout << ":" << std::endl;
	for (std::vector<Node*>::iterator it = _seq.begin(); it != _seq.end(); it++) {
		(*it)->print(indent+1);
	}
}

std::string NodeObjMap::get_class_name() {
	return _name;
}

void NodeObjMap::set_class_name(std::string name) {
	_name = name;
}

void NodeObjMap::print(int indent) {
	for (int i = 0; i < indent; i++) {
		std::cout << " ";
	}
	std::cout << types[_type] << " ";
	std::cout << "\"" << _name << "\" ";
	std::cout << ":" << std::endl;
	for (std::map<std::string,Node*>::iterator it = _map.begin(); it != _map.end(); it++) {
		(*it).second->print(indent+1);
	}
}

std::string NodeObjSeq::get_class_name() {
	return _name;
}

void NodeObjSeq::set_class_name(std::string name) {
	_name = name;
}

void NodeObjSeq::print(int indent) {
	for (int i = 0; i < indent; i++) {
		std::cout << " ";
	}
	std::cout << types[_type] << " ";
	std::cout << "\"" << _name << "\" ";
	std::cout << ":" << std::endl;
	for (std::vector<Node*>::iterator it = _seq.begin(); it != _seq.end(); it++) {
		(*it)->print(indent+1);
	}
}

void NodeRef::set_target(std::string target) {
	_target = target;
}

std::string NodeRef::get_target() {
	return _target;
}

Node& NodeRef::operator [](size_type n) {
	return get_anchor(_target)[n];
}

Node& NodeRef::operator [](std::string key) {
	return get_anchor(_target)[key];
}

Node::size_type NodeRef::size() {
	return get_anchor(_target).size();
}

std::string NodeRef::get_class_name() {
	return get_anchor(_target).get_class_name();
}

void NodeRef::set_class_name(std::string name) {
	get_anchor(_target).set_class_name(name);
}

void NodeRef::operator >>(int &n) {
	get_anchor(_target) >> n;
}

void NodeRef::operator >>(double &x) {
	get_anchor(_target) >> x;
}

void NodeRef::operator >>(std::string &s) {
	get_anchor(_target) >> s;
}

void NodeRef::operator >>(bool &b) {
	get_anchor(_target) >> b;
}

void NodeRef::print(int indent) {
	for (int i = 0; i < indent; i++) {
		std::cout << " ";
	}
	std::cout << types[_type] << " ";
	std::cout << "->" << _target << std::endl;
}

void NodeLiteral::set_contents(std::string contents) {
	_str = contents;
}

void NodeLiteral::print(int indent) {
	for (int i = 0; i < indent; i++) {
		std::cout << " ";
	}
	std::cout << types[_type] << " ";
	std::cout << _str << std::endl;
}

void NodeBool::operator >>(bool &b) {
	std::stringstream stream(_str);
	stream.setf(std::ios_base::boolalpha);
	stream >> b;
}

void NodeInt::operator >>(int &n) {
	std::stringstream stream(_str);
	stream.unsetf(std::ios_base::dec);
	stream >> n;
}

void NodeFloat::operator >>(double &x) {
	std::stringstream stream(_str);
	stream.unsetf(std::ios_base::dec);
	stream >> x;
}

void NodeString::operator >>(std::string &s) {
	s = _str;
}

void NodeString::print(int indent) {
	for (int i = 0; i < indent; i++) {
		std::cout << " ";
	}
	std::cout << types[_type] << " ";
	std::cout << "\"" << _str << "\"" << std::endl;
}