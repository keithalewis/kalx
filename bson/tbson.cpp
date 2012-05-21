// tbon.cpp - test bson
#include <cassert>
#include <iostream>
#include "bson.h"

//using namespace std;
using namespace bson;
using bson::read;
using json::string;
using json::string_;

const char* hw ="\x16\x00\x00\x00\x02hello\x00\x06\x00\x00\x00world\x00\x00";

void test_read(void)
{

	const char* t = hw + 4;
	bson_type bt = type(t);
	assert (bt == BSON_STRING);
	std::string k = key(t);
	assert (k == "hello");
}

void test_write(void)
{
	char buf[1024];
	char* s = buf + 4;

	size_t n = bson::write("hello", string_(5, "world"), s);
	*s = 0;
	*(uint32_t*)buf = n + 5;

	assert (0 == strcmp(buf, hw));

	s = buf;
	n = write("hello", "world", s);
	n += write("number", 1.23, s);
	n += write("boolean", false, s);
	json::value a(3);
	a[0] = json::value("string");
	a[1] = json::value(1.23);
	a[2] = json::value(true);
	n += write("array", a, s);
	

	const char* t = buf;
	json::pair kv;
	
	kv = read(t);
	assert (kv.first == "hello");
	assert (kv.second == "world");
	
	kv = read(t);
	assert (kv.first == "number");
	assert (kv.second == 1.23);
	
	kv = read(t);
	assert (kv.first == "boolean");
	assert (kv.second == false);
}

int main()
{
	test_read();

	test_write();

	return 0;
} 