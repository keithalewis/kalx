// tbon.cpp - test bson
#include <cassert>
#include "bson.h"

using namespace bson;

void test_value(void)
{
	value v;
	assert (v.type() == BSON_NULL);
	assert (v != v);

	value w("string");
	v = w;
	assert (v.type() == BSON_STRING);
	assert (v == "string");

	v = 1.23;
	assert (v.type() == BSON_NUMBER);
	assert (v == 1.23);

	value u(v);
	assert (u.type() == v.type());
	assert (u == v);
	assert (u != w);
}

void test_string(void)
{
	value s("a string");
	assert (s.type() == BSON_STRING);
	assert (s == "a string");
	s = "another string";
	assert (s.type() == BSON_STRING);
	assert (s == "another string");
}

void test_number()
{
	value n(1.23);
	assert (n.type() == BSON_NUMBER);
	assert (n == 1.23);
	n = 3.21;
	assert (n.type() == BSON_NUMBER);
	assert (n == 3.21);
}

void test_array(void)
{
	value a2[2];
	a2[0] = value("string");
	a2[1] = value(1.23);
/*	value a(2, a2);
	assert (a.type() == BSON_ARRAY);
	assert (a[0] == a2[0]);
	assert (a[0] == value("string"));
	assert (a[1] == a2[1]);
	assert (a[1] == value(1.23));
*/
}

int main()
{
	assert (sizeof(value) == sizeof(element));

	test_value();
	test_string();
	test_number();
	test_array();

	return 0;
} 