#include "parser.h"
#include <iostream>
#include <sstream>
#include <string>
#include <map>
#include <vector>

std::string types[] = {
	"String","Int","Float",
	"Boolean","Map","Sequence",
	"ObjMap","ObjSequence","Reference","Link"
};

std::string tokentypes[] = {
	"CBRACE_L","CBRACE_R","SBRACE_L","SBRACE_R","RBRACE_L","RBRACE_R",
	"BANG","ASTERISK","AT","AMPERSAND","COLON","COMMA",
	"IDENTIFIER","STRING","INT","FLOAT","BOOL"
};

Parser::Parser(std::istream& is):
	_is(is), _iit(_is), _eos(), _skip_comments(true),
	_document(), _generated(false), _cur_line(1)
{
	_is >> std::noskipws;
}

Node& Parser::get_document() {
	if (!_generated) {
		lex();
		print_tokens();
		parse();
		interpret();
		_generated = true;
	}
	return *_document;
}

void Parser::lex() {
	static std::string char_tokens = "{}[]()!*@&:,";

	while (_iit != _eos) {

		if (char_tokens.find(*_iit) != std::string::npos) {
			Token t;
			t.line = _cur_line;
			t.type = *_iit == '{' ? TOK_CBRACE_L :
	 				 *_iit == '}' ? TOK_CBRACE_R :
					 *_iit == '[' ? TOK_SBRACE_L :
					 *_iit == ']' ? TOK_SBRACE_R :
					 *_iit == '(' ? TOK_RBRACE_L :
					 *_iit == ')' ? TOK_RBRACE_R :
					 *_iit == '!' ? TOK_BANG :
					 *_iit == '*' ? TOK_ASTERISK :
					 *_iit == '@' ? TOK_AT :
					 *_iit == '&' ? TOK_AMPERSAND :
					 *_iit == ':' ? TOK_COLON :
									TOK_COMMA;
			t.contents += *_iit;
			_token_list.push_back(t);
			next_sym();

		} else if (*_iit == '"') {
			next_sym();
			Token t;
			t.line = _cur_line;
			t.type = TOK_STRING;
			bool escape = false, closed = false;
			while (_iit != _eos && !closed) {
				if (*_iit == '"') { // string termination
					if (escape) { // unless escaped
						escape = false;
						t.contents += '"';
					} else {
						closed = true;
					}
				} else if (*_iit == '\\') { // escape character
					if (escape) {
						escape = false;
						t.contents += '\\';
					} else {
						escape = true;
					}
				} else if (escape && *_iit == '#') { // comments can be escaped
					t.contents += '#';
					escape = false;
				} else { // other characters
					if (escape) {
						escape = false;
						t.contents += '\\';
					}
					t.contents += *_iit;
				}
				set_skip_comments(!escape);
				next_sym();
			}
			if (!closed) {
				throw LexException(t.line, std::string("unterminated string"));
			}
			_token_list.push_back(t);

		} else if (is_an_identifier_letter(*_iit)) { // identifier
			Token t;
			t.type = TOK_IDENTIFIER;
			t.line = _cur_line;
			while (_iit != _eos && (is_an_identifier_letter(*_iit) || is_a_digit(*_iit))) {
				t.contents += *_iit;
				next_sym();
			};
			if (t.contents == "true" || t.contents == "false") {
				t.type = TOK_BOOL;
			}
			_token_list.push_back(t);

		} else if (is_a_digit(*_iit) || is_a_sign(*_iit) || *_iit == '.') {
			Token t;
			t.type = TOK_INT;
			t.line = _cur_line;
			if (_iit != _eos && is_a_sign(*_iit)) { // sign
				t.contents += *_iit;
				next_sym();
			}
			while (_iit != _eos && is_a_digit(*_iit)) { // integral part
				t.contents += *_iit;
				next_sym();
			}
			if (_iit != _eos && *_iit == '.') { // fractional part
				t.type = TOK_FLOAT;
				t.contents += *_iit;
				next_sym();
				while (_iit != _eos && is_a_digit(*_iit)) {
					t.contents += *_iit;
					next_sym();
				}
			}
			if (_iit != _eos && (*_iit == 'e' || *_iit == 'E')) { // exponential part
				t.contents += *_iit;
				next_sym();
				if (_iit != _eos && is_a_sign(*_iit)) {
					t.contents += *_iit;
					next_sym();
				}
				while (_iit != _eos && is_a_digit(*_iit)) {
					t.contents += *_iit;
					next_sym();
				}
			}
			_token_list.push_back(t);

		} else if (is_whitespace(*_iit)) {
			next_sym();
		} else {
			throw LexException(_cur_line, std::string("invalid token")); // unknown token!
		}
	}

	std::cout << "Lexing A-OK!" << std::endl;
}

void Parser::set_skip_comments(bool mode) {
	_skip_comments = mode;
}

void Parser::next_sym() {
	if (_iit != _eos) {
		if (*_iit == '\n') ++_cur_line;
		++_iit;
	}
	if (_skip_comments && _iit != _eos && *_iit == '#') {
		do {
			++_iit;
		} while (_iit != _eos && *_iit != '\n');
		if (*_iit == '\n') ++_cur_line;
		++_iit;
	}
}

bool Parser::is_a_digit(char c) {
	return c >= '0' && c <= '9';
}

bool Parser::is_a_sign(char c) {
	return c == '+' || c == '-';
}

bool Parser::is_an_identifier_letter(char c) {
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

bool Parser::is_whitespace(char c) {
	return c == 0x09 || (c >= 0x0A && c <= 0x0D) || c == 0x20;
}

void Parser::print_tokens() {
	for (std::vector<Token>::iterator it = _token_list.begin(); it != _token_list.end(); it++) {
		std::cout << tokentypes[(*it).type] << " | " << (*it).contents << std::endl;
	}
}

void Parser::parse() {
	_cur_token = _token_list.begin();
	parse_value();
	std::cout << "Parsing A-OK!" << std::endl;
}

bool Parser::expect(TokenType t) {
	if (accept(t)) {
		return true;
	} else {
		std::string str("unexpected token ");
		str = str + tokentypes[(*_cur_token).type] + ", expected " + tokentypes[t];
		throw ParseException((*_cur_token).line, str);
	}
}

bool Parser::accept(TokenType t) {
	if (_cur_token != _token_list.end() && (*_cur_token).type == t) {
		++_cur_token;
		return true;
	}
	return false;
}

void Parser::parse_value() {
	bool has_alias = false, is_ref = false;
	if (accept(TOK_AMPERSAND)) {
		expect(TOK_IDENTIFIER);
		has_alias = true;
	}
	if (accept(TOK_CBRACE_L)) {
		parse_map();
	} else if (accept(TOK_SBRACE_L)) {
		parse_seq();
	} else if (accept(TOK_BANG)) {
		parse_obj();
	} else if (accept(TOK_ASTERISK)) {
		if (has_alias) {
			throw ParseException((*_cur_token).line, std::string("reference or link is aliased"));
		}
		expect(TOK_IDENTIFIER);
		is_ref = true;
	} else if (accept(TOK_AT)) {
		if (has_alias) {
			throw ParseException((*_cur_token).line, std::string("reference or link is aliased"));
		}
		expect(TOK_IDENTIFIER);
		is_ref = true;
	} else if (accept(TOK_BOOL) || accept(TOK_INT) || accept(TOK_FLOAT) || accept(TOK_STRING)) {
	} else {
		throw ParseException((*_cur_token).line, std::string("invalid value"));
	}
	if (!has_alias && accept(TOK_AMPERSAND)) {
		if (is_ref) {
			throw ParseException((*_cur_token).line, std::string("reference or link is aliased"));
		}
		expect(TOK_IDENTIFIER);
	}
}

void Parser::parse_map() {
	if (!accept(TOK_CBRACE_R)) {
		do {
			parse_pair();
		} while (accept(TOK_COMMA));
		expect(TOK_CBRACE_R);
	}
}

void Parser::parse_seq() {
	if (!accept(TOK_SBRACE_R)) {
		do {
			parse_value();
		} while (accept(TOK_COMMA));
		expect(TOK_SBRACE_R);
	}
}

void Parser::parse_pair() {
	expect(TOK_IDENTIFIER);
	expect(TOK_COLON);
	parse_value();
}

void Parser::parse_obj() {
	expect(TOK_IDENTIFIER);
	expect(TOK_RBRACE_L);
	if (!accept(TOK_RBRACE_R)) {
		if ((*_cur_token).type == TOK_IDENTIFIER) {
			do {
				parse_pair();
			} while (accept(TOK_COMMA));
		} else {
			do {
				parse_value();
			} while (accept(TOK_COMMA));
		}
		expect(TOK_RBRACE_R);
	}
}

void Parser::interpret() {
	_cur_token = _token_list.begin();
	_document = interpret_value();
	check_for_cycles();
	std::cout << "Interpreting A-OK!" << std::endl;
}

Node* Parser::interpret_value() {
	Node* result;
	bool has_alias = false;
	std::string alias;
	if (accept(TOK_AMPERSAND)) {
		alias = (*_cur_token).contents;
		has_alias = true;
		accept(TOK_IDENTIFIER);
	}
	if (accept(TOK_CBRACE_L)) {
		result = interpret_map();
	} else if (accept(TOK_SBRACE_L)) {
		result = interpret_seq();
	} else if (accept(TOK_BANG)) {
		result = interpret_obj();
	} else if (accept(TOK_ASTERISK)) {
		result = interpret_ref();
	} else if (accept(TOK_AT)) {
		result = interpret_link();
	} else {
		TokenType t = (*_cur_token).type;
		switch(t) {
			case TOK_FLOAT: result = new NodeFloat(); break;
			case TOK_INT: result = new NodeInt(); break;
			case TOK_BOOL: result = new NodeBool(); break;
			case TOK_STRING:
			default: result = new NodeString(); break;
		}
		result->set_contents((*_cur_token).contents);
		_cur_token++;
	}
	if (!has_alias && accept(TOK_AMPERSAND)) {
		has_alias = true;
		alias = (*_cur_token).contents;
		accept(TOK_IDENTIFIER);
	}
	if (has_alias) {
		result->add_as_anchor(alias);
	}
	return result;
}

Node* Parser::interpret_map() {
	Node* result = new NodeMap();
	if (!accept(TOK_CBRACE_R)) {
			std::string key;
		do {
			key = (*_cur_token).contents;
			accept(TOK_IDENTIFIER);
			accept(TOK_COLON);
			result->add_to_map(key, interpret_value());
		} while (accept(TOK_COMMA));
		accept(TOK_CBRACE_R);
	}
	return result;
}

Node* Parser::interpret_seq() {
	Node* result = new NodeSeq();
	if (!accept(TOK_SBRACE_R)) {
		do {
			result->add_to_seq(interpret_value());
		} while (accept(TOK_COMMA));
		accept(TOK_SBRACE_R);
	}
	return result;
}

Node* Parser::interpret_obj() {
	Node* result;
	std::string class_name = (*_cur_token).contents;
	accept(TOK_IDENTIFIER);
	accept(TOK_RBRACE_L);
	if (!accept(TOK_RBRACE_R)) {
		if ((*_cur_token).type == TOK_IDENTIFIER) {
			result = new NodeObjMap();
			std::string key;
			do {
				key = (*_cur_token).contents;
				accept(TOK_IDENTIFIER);
				accept(TOK_COLON);
				result->add_to_map(key, interpret_value());
			} while (accept(TOK_COMMA));
		} else {
			result = new NodeObjSeq();
			do {
				result->add_to_seq(interpret_value());
			} while (accept(TOK_COMMA));
		}
		accept(TOK_RBRACE_R);
	}
	result->set_class_name(class_name);
	return result;
}

Node* Parser::interpret_link() {
	Node* result = new NodeLink();
	result->set_target((*_cur_token).contents);
	accept(TOK_IDENTIFIER);
	return result;
}

Node* Parser::interpret_ref() {
	Node* result = new NodeRef();
	result->set_target((*_cur_token).contents);
	accept(TOK_IDENTIFIER);
	return result;
}

void Parser::check_for_cycles() {
	unsigned int alias_count = Node::_alias_table.size();
	_graph = new bool*[alias_count];
	for (unsigned int i = 0; i < alias_count; ++i) {
		_graph[i] = new bool[alias_count];
		for (unsigned int j = 0; j < alias_count; ++j) {
			_graph[i][j] = false;
		}
	}
	unsigned int cnt = 0;
	for (std::map<std::string,Node*>::iterator it = Node::_alias_table.begin();
		it != Node::_alias_table.end(); ++it) {
		_ordering.insert(std::pair<std::string,unsigned int>((*it).first,cnt));
		cnt++;
	}
	for (std::map<std::string,Node*>::iterator it = Node::_alias_table.begin();
		it != Node::_alias_table.end(); ++it) {
		std::cout << "checking " << (*it).first << " for cyclical references..." << std::endl;
		recursive_check((*it).second, (*it).first);
	}
	for (unsigned int i = 0; i < alias_count; ++i) {
		for (unsigned int j = 0; j < alias_count; ++j) {
			std::cout << _graph[i][j] << " ";
		}
		std::cout << std::endl;
	}
	_color = new int[alias_count];
	for (unsigned int i = 0; i < alias_count; ++i) {
		_color[i] = 0;
	}
	for (unsigned int i = 0; i < alias_count; ++i) {
		if (_color[i] == 0) {
			dfs_visit(i);
		}
	}
	std::cout << "No cycles detected!" << std::endl;
}

void Parser::recursive_check(Node* n, std::string alias) {
	std::cout << "searching node..." << std::endl;
	n->print(3);
	switch (n->get_type()) {
		case Node::Map:
		case Node::ObjMap:
		case Node::Sequence:
		case Node::ObjSequence:
			for (Node::iterator it = n->begin(); it != n->end(); ++it) {
				recursive_check((*it),alias);
			};
			break;
		case Node::Reference:
		case Node::Link:
			std::cout << "found a link to " << n->get_target() << " from " << alias << "!" << std::endl;
			_graph[_ordering[alias]][_ordering[n->get_target()]] = true;
			break;
		default:
			break;
	}
}

void Parser::dfs_visit(int i) {
	_color[i] = 1;
	for (unsigned int j = 0; j < Node::_alias_table.size(); ++j) {
		if (_graph[i][j]) {
			if (_color[j] == 0) {
				dfs_visit(j);
			} else if (_color[j] == 1) {
				throw ValidateException(std::string("cyclical reference or link"));
			}
		}
	}
	_color[i] = 2;
}