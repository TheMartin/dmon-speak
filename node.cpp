#include "parser.h"
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <sstream>

std::map<std::string,Node> Node::_alias_table;

Node& Node::operator [](unsigned int n) { // TODO: separate out-of-bounds and type mismatch
	if (_type == Reference || _type == Link) {
		return get_anchor(_str)[n];
	} else if ((_type == Sequence || _type == ObjSequence) && _seq.size() > n) {
		return _seq[n];
	} else throw NodeException(std::string("invalid access or type mismatch"));
}

Node& Node::operator [](std::string key) { // TODO: separate out-of-bounds and type mismatch
	if (_type == Reference || _type == Link) {
		return get_anchor(_str)[key];
	} else if ((_type == Map || _type == ObjMap) && _map.count(key) != 0) {
		return _map[key];
	} else throw NodeException(std::string("invalid access or type mismatch"));
}

Node::size_type Node::size() {
	if (_type == Reference || _type == Link) {
		return get_anchor(_str).size();
	} else if (_type == Map || _type == ObjMap) {
		return (size_type)_map.size();
	} else if (_type == Sequence || _type == ObjSequence) {
		return (size_type)_seq.size();
	} else throw NodeException(std::string("invalid access or type mismatch"));
}

std::string Node::get_class_name() {
	if (_type == Reference || _type == Link) {
		return get_anchor(_str).get_class_name();
	} else if (_type == ObjMap || _type == ObjSequence) {
		return _str;
	} else throw NodeException(std::string("invalid access or type mismatch"));
}

void Node::operator >>(int &n) {
	if (_type == Reference || _type == Link) {
		get_anchor(_str) >> n;
	} else if (_type == Int) {
		std::stringstream stream(_str);
		stream.unsetf(std::ios_base::dec);
		stream >> n;
	} else throw NodeException(std::string("invalid access or type mismatch"));
}

void Node::operator >>(double &x) {
	if (_type == Reference || _type == Link) {
		get_anchor(_str) >> x;
	} else if (_type == Float) {
		std::stringstream stream(_str);
		stream.unsetf(std::ios_base::dec);
		stream >> x;
	} else throw NodeException(std::string("invalid access or type mismatch"));
}

void Node::operator >>(std::string &s) {
	if (_type == Reference || _type == Link) {
		get_anchor(_str) >> s;
	} else if (_type == String) {
		s = _str;
	} else throw NodeException(std::string("invalid access or type mismatch"));
}

void Node::operator >>(bool &b) {
	if (_type == Reference || _type == Link) {
		get_anchor(_str) >> b;
	} else if (_type == Boolean) {
		std::stringstream stream(_str);
		stream.setf(std::ios_base::boolalpha);
		stream >> b;
	} else throw NodeException(std::string("invalid access or type mismatch"));
}

void Node::add_to_map(std::string key, Node &n) {
	if (_type == Map || _type == ObjMap) {
		_map.insert( std::pair<std::string,Node>(key,n) );
	} else throw NodeException(std::string("invalid access or type mismatch"));
}

void Node::add_to_seq(Node &n) {
	if (_type == Sequence || _type == ObjSequence) {
		_seq.push_back(n);
	} else throw NodeException(std::string("invalid access or type mismatch"));
}

Node& Node::get_anchor(std::string alias) {
	if (_alias_table.count(alias) != 0) {
		return _alias_table[alias];
	} else throw NodeException(std::string("unknown alias"));
}

void Node::add_as_anchor(std::string alias) {
	_alias_table.insert( std::pair<std::string, Node> (alias, *this) );
}

void Node::print(int indent) {
	for (int i = 0; i < indent; i++) {
		std::cout << " ";
	}
	std::cout << types[_type] << " ";
	if (_type == ObjMap || _type == ObjSequence) {
		std::cout << "\"" << _str << "\" ";
	}
	if (_type == Map || _type == ObjMap) {
		std::cout << ":" << std::endl;
		for (std::map<std::string,Node>::iterator it = _map.begin(); it != _map.end(); it++) {
			(*it).second.print(indent+1);
		}
	} else if (_type == Sequence || _type == ObjSequence) {
		std::cout << ":" << std::endl;
		for (std::vector<Node>::iterator it = _seq.begin(); it != _seq.end(); it++) {
			(*it).print(indent+1);
		}
	} else if (_type == Reference || _type == Link) {
		std::cout << "->" << _str << std::endl;
	} else if (_type == String) {
		std::cout << "\"" << _str << "\"" << std::endl;
	} else {
		std::cout << _str << std::endl;
	}
}