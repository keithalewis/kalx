// decode.h - BSON parser
#pragma once
#include <cstdint>
#include <cstring>
#include <io.h>
#include "json.h"

typedef enum {
	BSON_EOO = 0,
	BSON_DOUBLE = 1,
	BSON_STRING = 2,
	BSON_OBJECT = 3,
	BSON_ARRAY = 4,
	BSON_BINDATA = 5,
	BSON_UNDEFINED = 6,
	BSON_OID = 7,
	BSON_BOOL = 8,
	BSON_DATE = 9,
	BSON_NULL = 10,
	BSON_REGEX = 11,
	BSON_DBREF = 12, /**< Deprecated. */
	BSON_CODE = 13,
	BSON_SYMBOL = 14,
	BSON_CODEWSCOPE = 15,
	BSON_INT32 = 16,
	BSON_TIMESTAMP = 17,
	BSON_INT64 = 18
} bson_type;

typedef enum {
	BSON_BIN_BINARY = 0,
	BSON_BIN_FUNC = 1,
	BSON_BIN_BINARY_OLD = 2,
	BSON_BIN_UUID = 3,
	BSON_BIN_MD5 = 5,
	BSON_BIN_USER = 128
} bson_subtype;

namespace bson {

	struct pair {
		json::string key;
		json::element value;
	};
	/*
	inline size_t size(const json::element& e)
	{
		return e.type == JSON_STRING ? e.data.string.size + 1
			: e.type == JSON_ARRAY ? e.data.array.size * sizeof(json::element)
			: e.type == JSON_BYTE ? e.data.byte.size
			: sizeof(json_type<e.type>::type)
			;
	}
	*/
	template<typename T> struct bson_enum { };
	template<> struct bson_enum<double> { static const bson_type type = BSON_DOUBLE; };
	template<> struct bson_enum<char*> { static const bson_type type = BSON_STRING; };
	template<> struct bson_enum<json::string> { static const bson_type type = BSON_STRING; };
	template<> struct bson_enum<json::array> { static const bson_type type = BSON_ARRAY; };
	template<> struct bson_enum<json::byte> { static const bson_type type = BSON_BINDATA; };
	template<> struct bson_enum<bool> { static const bson_type type = BSON_BOOL; };
	template<> struct bson_enum<time_t> { static const bson_type type = BSON_DATE; };

	template<typename T>
	struct bson_traits {
		static const bson_type type = bson_enum<T>::type;
		typedef const T* data_type;
		static size_t size(const T& t)
		{
			return sizeof(T);
		}
		static data_type data(const T& t)
		{
			return &t;
		}
	};
	template<>
	struct bson_traits<json::string>
	{
		typedef const char* data_type;
		static const bson_type type = BSON_STRING;
		static size_t size(const json::string& s)
		{
			return s.size + 1;
		}
		static data_type data(const json::string& s)
		{
			return s.data;
		}
	};
	template<>
	struct bson_traits<json::array>
	{
		typedef const json::element* data_type;
		static const bson_type type = BSON_ARRAY;
		static size_t size(const json::array& a)
		{
			return a.size*sizeof(json::element);
		}
		static data_type data(const json::array& a)
		{
			return a.element;
		}
	};
	template<>
	struct bson_traits<json::byte>
	{
		typedef const uint8_t* data_type;
		static const bson_type type = BSON_BINDATA;
		static size_t size(const json::byte& b)
		{
			return b.size;
		}
		static data_type data(const json::byte& b)
		{
			return b.data;
		}
	};

	inline bson_type type(const char*& s)
	{
		return (bson_type)(*s++);
	}

	template<typename T>
	inline size_t write(const char* key, const T& val, char*&  buf)
	{
		size_t bytes = 1;
		*buf++ = bson_traits<T>::type;

		while (*key) {
			*buf++ = *key++;
			++bytes;
		}
		*buf++ = *key++; // null terminate key
		++bytes;

		size_t size = bson_traits<T>::size(val);
		*(uint32_t*)buf = size; 
		bytes += 4;

		memcpy(buf, bson_traits<T>::data(val), bson_traits<T>::size(val));
		buf += size;
		bytes += size;

		return bytes;
	}
	inline size_t write(const char* key, const char* val, char*&  buf)
	{
		return write<json::string>(key, json::string_(strlen(val), val), buf);
	}
	inline size_t write(const pair& kv, char*& buf)
	{
	}

	inline json::string key(const char*& s)
	{
		json::string key = json::string_(strlen(s), s);

		s += key.size + 1;

		return key;
	}
	
	// read a T off the string
	template<typename T>
	inline T value(const char*& buf)
	{
		// ensure (type(buf) == bson_traits<T>::type);
		++buf;
		T t = *(T*)buf;
		buf += sizeof(T);

		return t;
	}

	template<>
	inline json::string value<json::string> (const char*& buf)
	{
		json::string val;
		// ensure (type(s) == BSON_STRING);
		++buf;
		val.size = *(int32_t*)buf;
		buf += 4;
		val.data = buf;
		buf += val.size + 1;

		return val;
	}

	inline pair read(const char*& buf)
	{
		pair kv;

		kv.value.type = type(buf);

	}

} // namepace bson