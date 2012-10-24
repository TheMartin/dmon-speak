#include "parser.h"
#include <iostream>
#include <string>
#include <map>
#include <vector>

Node::iterator::iterator(): _impl(0) {}
Node::iterator::iterator(std::map<std::string,Node*>::iterator iter): _impl(new iterNodeMapImpl(iter)) {}
Node::iterator::iterator(std::vector<Node*>::iterator iter): _impl(new iterNodeSeqImpl(iter)) {}
Node::iterator::iterator(const Node::iterator &rhs): _impl(rhs._impl->clone()) {}

Node::iterator& Node::iterator::operator =(const Node::iterator &rhs) {
	delete _impl;
	_impl = rhs._impl->clone();
	return *this;
}

Node::iterator::~iterator() {
	delete _impl;
}

Node::iterator& Node::iterator::operator ++() {
	_impl->increment();
	return *this;
}

Node::iterator Node::iterator::operator ++(int) {
	Node::iterator tmp(*this);
	_impl->increment();
	return tmp;
}

bool Node::iterator::operator ==(const Node::iterator &rhs) {
	return _impl->equals(*(rhs._impl));
}

bool Node::iterator::operator !=(const Node::iterator &rhs) {
	return !(_impl->equals(*(rhs._impl)));
}

Node* Node::iterator::operator *() {
	return _impl->dereference();
}

iterNodeMapImpl::iterNodeMapImpl(std::map<std::string,Node*>::iterator iter): it(iter) {}
iterNodeMapImpl::iterNodeMapImpl(const iterNodeMapImpl& rhs): it(rhs.it) {}

iterNodeImpl* iterNodeMapImpl::clone() {
	return new iterNodeMapImpl(*this);
}

void iterNodeMapImpl::increment() {
	++it;
}
bool iterNodeMapImpl::equals(iterNodeImpl& rhs) {
	return rhs.equal_accept(*this);
}

bool iterNodeMapImpl::equal_accept(iterNodeMapImpl& rhs) {
	return it == rhs.it;
}

bool iterNodeMapImpl::equal_accept(iterNodeSeqImpl& rhs) {
	return false;
}

Node* iterNodeMapImpl::dereference() {
	return (*it).second;
}

iterNodeSeqImpl::iterNodeSeqImpl(std::vector<Node*>::iterator iter): it(iter) {}
iterNodeSeqImpl::iterNodeSeqImpl(const iterNodeSeqImpl& rhs): it(rhs.it) {}

iterNodeImpl* iterNodeSeqImpl::clone() {
	return new iterNodeSeqImpl(*this);
}

void iterNodeSeqImpl::increment() {
	++it;
}
bool iterNodeSeqImpl::equals(iterNodeImpl& rhs) {
	return rhs.equal_accept(*this);
}

bool iterNodeSeqImpl::equal_accept(iterNodeMapImpl& rhs) {
	return false;
}

bool iterNodeSeqImpl::equal_accept(iterNodeSeqImpl& rhs) {
	return it == rhs.it;
}

Node* iterNodeSeqImpl::dereference() {
	return *it;
}