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
	BSON_INT = 16,
	BSON_TIMESTAMP = 17,
	BSON_LONG = 18
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
	template<typename T> struct bson_enum { };
	template<> struct bson_enum<double> { static const bson_type type = BSON_DOUBLE; };
	template<> struct bson_enum<char*> { static const bson_type type = BSON_STRING; };
	template<> struct bson_enum<json::object*> { static const bson_type type = BSON_OBJECT; };
	template<> struct bson_enum<json::string> { static const bson_type type = BSON_STRING; };
	template<> struct bson_enum<json::array> { static const bson_type type = BSON_ARRAY; };
	template<> struct bson_enum<json::byte> { static const bson_type type = BSON_BINDATA; };
	template<> struct bson_enum<bool> { static const bson_type type = BSON_BOOL; };
	template<> struct bson_enum<int32_t> { static const bson_type type = BSON_INT; };
//	template<> struct bson_enum<int64_t> { static const bson_type type = BSON_LONG; };
	template<> struct bson_enum<time_t> { static const bson_type type = BSON_DATE; };

	struct pair {
		json::string key;
		json::element value;
	};

	//
	// writing objects
	//

	template<typename T>
	inline size_t write(const char* key, const T& val, char*&  buf)
	{
		size_t bytes = 1;
		*buf++ = bson_enum<T>::type;

		while (*key) {
			*buf++ = *key++;
			++bytes;
		}
		*buf++ = *key++; // null terminate key
		++bytes;

		size_t size = sizeof(val);
		memcpy(buf, &val, size);
		buf += size;
		bytes += size;

		return bytes;
	}
	// specializations
	inline size_t write(const char* key, const json::element& val, char*& buf)
	{
		return val.type == JSON_NUMBER ? write(key, val.data.number, buf)
			:  val.type == JSON_STRING ? write(key, val.data.string, buf)
			:  val.type == JSON_OBJECT ? write(key, val.data.object, buf)
			:  val.type == JSON_ARRAY ? write(key, val.data.array, buf)
			:  val.type == JSON_BYTE ? write(key, val.data.byte, buf)
//	BSON_UNDEFINED = 6,
//	BSON_OID = 7,
			:  val.type == JSON_TRUE ? write(key, true, buf)
			:  val.type == JSON_FALSE ? write(key, false, buf)
			:  val.type == JSON_DATE ? write(key, val.data.date, buf)
//???			:  val.type == BSON_NULL = 10,
//	BSON_REGEX = 11,
//	BSON_CODE = 13,
//	BSON_SYMBOL = 14,
//	BSON_CODEWSCOPE = 15,
			:  val.type == JSON_INT32 ? write(key, val.data.int32, buf)
//	BSON_TIMESTAMP = 17,
//			:  val.type == JSON_INT64 ? write(key, val.data.int64, buf)
			: 0 // no write
			;
	}
	// none template functions resolved first
	inline size_t write(const char* key, const json::string& val, char*&  buf)
	{
		size_t bytes = 1;
		*buf++ = BSON_STRING;

		while (*key) {
			*buf++ = *key++;
			++bytes;
		}
		*buf++ = *key++; // null terminate key
		++bytes;

		*(uint32_t*)buf = val.size + 1;
		buf += 4;
		bytes += 4;
		
		memcpy(buf, val.data, val.size);
		buf += val.size;
		*buf++ = 0;
		bytes += val.size + 1;

		return bytes;
	}
	inline size_t write(const char* key, const char* val, char*&  buf)
	{
		return write(key, json::string_(strlen(val), val), buf);
	}
	inline size_t write(const char* key, const json::array& val, char*& buf)
	{
		size_t bytes = 1;
		*buf++ = BSON_ARRAY;

		while (*key) {
			*buf++ = *key++;
			++bytes;
		}
		*buf++ = *key++; // null terminate key
		++bytes;

		char i_[1024];
		for (size_t i = 0; i < val.size; ++i) {
			bytes += write(itoa(i, i_, 10), val.element[i], buf);
		}
		
		return bytes;
	}
	inline size_t write(const char* key, const json::byte& val, char*& buf)
	{
		size_t bytes = 1;
		*buf++ = BSON_BINDATA;

		while (*key) {
			*buf++ = *key++;
			++bytes;
		}
		*buf++ = *key++; // null terminate key
		++bytes;

		*(uint32_t*)buf = val.size;
		buf += 4;
		bytes += 4;
		
		memcpy(buf, val.data, val.size);
		buf += val.size;
		bytes += val.size;

		return bytes;
	}

	//
	// reading objects
	//

	inline bson_type type(const char*& s)
	{
		return (bson_type)(*s++);
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
		T t = *(T*)buf;
		buf += sizeof(T);

		return t;
	}
	template<>
	inline json::string value<json::string>(const char*& buf)
	{
		json::string val;
		val.size = *(int32_t*)buf - 1;
		buf += 4;
		val.data = buf;
		buf += val.size + 1;

		return val;
	}
	// overload using type
	inline json::element value(bson_type type, const char*& buf)
	{
		json::element e;

		switch (type) {
		case BSON_DOUBLE:
			e.type = JSON_NUMBER;
			e.data.number = value<double>(buf);
			break;
		case BSON_STRING:
			e.type = JSON_STRING;
			e.data.string = value<json::string>(buf);
			break;
		case BSON_OBJECT:
			e.type = JSON_OBJECT;
			e.data.object = value<json::object*>(buf);
			break;
		case BSON_ARRAY:
			e.type = JSON_ARRAY;
			e.data.array.size = 0;
			break;
		case BSON_BINDATA:
			e.type = JSON_BYTE;
			e.data.byte = value<json::byte>(buf);
			break;
//	BSON_UNDEFINED = 6,
//	BSON_OID = 7,
		case BSON_BOOL:
			value<bool>(buf) ? e.type = JSON_TRUE : e.type = JSON_FALSE;
			break;
		case BSON_DATE:
			e.type = JSON_DATE;
			e.data.date = value<time_t>(buf);
			break;
		case BSON_NULL:
			e.type = JSON_NULL;
			break;
//	BSON_REGEX = 11,
//	BSON_DBREF = 12, /**< Deprecated. */
//	BSON_CODE = 13,
//	BSON_SYMBOL = 14,
//	BSON_CODEWSCOPE = 15,
		case BSON_INT:
			e.type = JSON_INT32;
			e.data.int32 = value<int32_t>(buf);
			break;
//	BSON_TIMESTAMP = 17,
		case BSON_LONG:
			e.type = JSON_INT64;
			e.data.int64 = value<int64_t>(buf);
			break;
		default:
			e.type = JSON_NULL;
		}

		return e;
	}

	inline pair read(const char*& buf)
	{
		pair kv;

		bson_type t = type(buf);
		kv.key = key(buf);
		kv.value = value(t, buf);

		return kv;
	}

} // namepace bson