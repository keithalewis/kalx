// tbon.cpp - test bson
#include <cassert>
#include <iostream>
#include "bson.h"

//using namespace std;
using namespace bson;
using json::string;
using json::string_;

const char* hw ="\x16\x00\x00\x00\x02hello\x00\x06\x00\x00\x00world\x00\x00";

void test_read(void)
{

	const char* t = hw + 4;
	bson_type bt = type(t);
	assert (bt == BSON_STRING);
	t++;
	string k = key(t);
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

	const char* t = buf;
	assert (key(t) == "hello");
	assert (value<string>(t) == "world");
	assert (key(t) == "number");
	assert (value<double>(t) == 1.23);
	assert (key(t) == "boolean");
	assert (value<bool>(t) == false);
}

int main()
{
	test_read();

	test_write();

	return 0;
} 