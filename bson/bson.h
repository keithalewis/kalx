// bson.h - Lightweight C++ wrappers for mongo C library.
#include <cstdint>
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <xutility>

using namespace std::rel_ops;

namespace bson {

	class object;
	class value;
	struct element;

	typedef enum {
		BSON_STRING,
		BSON_NUMBER, // double
		BSON_OBJECT,
		BSON_ARRAY,
		BSON_TRUE,
		BSON_FALSE,
		BSON_NULL,
		BSON_BYTE,
		BSON_INT32,
		BSON_INT64,
		BSON_DATE
	} element_t;

	struct string {
		size_t size;
		char* data;
	};

	struct array {
		size_t size;
		element* element;
	};

	struct byte {
		size_t size;
		uint8_t* data;
	};

	struct element {
		union {
			string string;
			double number;
			object* object;
			array array;
			byte byte;
			int32_t int32;
			int64_t int64;
			time_t date;
		} data;
		element_t type;
	};

	inline bool operator==(const element& a, const element& b)
	{
		return a.type == b.type;
	}
	inline bool operator<(const element& a, const element& b)
	{
		return a.type < b.type;
	}

	class value : public element {
	public:
		value()
		{
			type = BSON_NULL;
		}
		value(const value& v)
		{
			operator=(v);
		}
		value& operator=(const value& v)
		{
			if (this != &v) {
				switch (v.type) {
				case BSON_STRING:
					operator=(v.data.string);
					break;
				case BSON_ARRAY:
					operator=(v.data.array);
					break;
				case BSON_BYTE:
					operator=(v.data.byte);
					break;
				default: // non pointer types
					type = v.type;
					data = v.data;
				}
			}

			return *this;
		}
		value(const bson::element& e)
		{
			operator=(e);
		}
		value& operator=(const bson::element& e)
		{
			switch (e.type) {
			case BSON_STRING:
				operator=(e.data.string);
				break;
			case BSON_ARRAY:
				operator=(e.data.array);
				break;
			case BSON_BYTE:
				operator=(e.data.byte);
				break;
			default: // non pointer types
				type = e.type;
				data = e.data;
			}

			return *this;
		}
//		value(const value&& v);
//		value& operator=(const value&& v);
		~value()
		{
			delete_value();
		}

		bool operator==(const value& v) const
		{
			return type != v.type ? false 
				: type == BSON_STRING ? operator==(v.data.string) 
				: type == BSON_NUMBER ? data.number == v.data.number
				: type == BSON_OBJECT ? data.object == v.data.object // point to same object
				: type == BSON_ARRAY ? operator==(v.data.array)
				: type == BSON_TRUE ? true
				: type == BSON_FALSE ? true
				: type == BSON_NULL ? false // null <> null
				: type == BSON_BYTE ? operator==(v.data.byte)
				: type == BSON_INT32 ? data.int32 == v.data.int32
				: type == BSON_INT64 ? data.int64 == v.data.int64
				: type == BSON_DATE ? data.date == v.data.date
				: false // shouldn't happen
				;
		}
		bool operator<(const value& v) const
		{
			return type != v.type ? type < v.type // <--- fix this!!!
				: type == BSON_STRING ? operator<(v.data.string) 
				: type == BSON_NUMBER ? data.number < v.data.number
				: type == BSON_OBJECT ? data.object < v.data.object // meaningless
				: type == BSON_ARRAY ? operator<(v.data.array)
				: type == BSON_TRUE ? true
				: type == BSON_FALSE ? true
				: type == BSON_NULL ? false // null <> null
				: type == BSON_BYTE ? operator<(v.data.byte)
				: type == BSON_INT32 ? data.int32 < v.data.int32
				: type == BSON_INT64 ? data.int64 < v.data.int64
				: type == BSON_DATE ? data.date < v.data.date
				: false // shouldn't happen
				;
		}
		value(value&& v)
		{
			type = v.type;
			data = v.data;

			v.type = BSON_NULL;
		}
		value& operator=(value&& v)
		{
			if (this != &v) {
				type = v.type;
				data = v.data;

				v.type = BSON_NULL;
			}

			return *this;
		}
		value(const char* s)
		{
			type = BSON_STRING;
			construct_string(s);
		}
		value(const string& s)
		{
			type = BSON_STRING;
			construct_string(s.data);
		}
		value& operator=(const char* s)
		{
			delete_value();
			type = BSON_STRING;
			construct_string(s);

			return *this;
		}
		value& operator=(const string& s)
		{
			delete_value();
			type = BSON_STRING;
			construct_string(s.data);

			return *this;
		}
		bool operator==(const char* s) const
		{
			return type == BSON_STRING && strcmp(data.string.data, s) == 0;
		}
		bool operator<(const char* s) const
		{
			return type == BSON_STRING && strcmp(data.string.data, s) < 0;
		}

		explicit value(double number)
		{
			type = BSON_NUMBER;
			data.number = number;
		}
		value& operator=(double number)
		{
			delete_value();
			type = BSON_NUMBER;
			data.number = number;

			return *this;
		}
		bool operator==(double number) const
		{
			return type == BSON_NUMBER && data.number == number;
		}
		bool operator<(double number) const
		{
			return type == BSON_NUMBER && data.number < number;
		}

		// array
		value(size_t n, const bson::element* elements)
		{
			type = BSON_ARRAY;
			construct_array(n, elements);
		}
/*		value(size_t n, const value* value)
		{
			type = BSON_ARRAY;
			construct_array(n, value);
		}
*/		value& operator=(const array& a)
		{
			delete_value();
			type = BSON_ARRAY;
			construct_array(a.size, a.element);

			return *this;
		}
		bool operator==(const array& array) const
		{
			return type == BSON_ARRAY && data.array.size == array.size
				&& std::equal(data.array.element, data.array.element + data.array.size, array.element);
		}
		bool operator<(const array& array) const
		{
			return type == BSON_ARRAY && std::lexicographical_compare(data.array.element, data.array.element + data.array.size, array.element, array.element + array.size);
		}
		bson::element& operator[](size_t i)
		{
			// ensure type == BSON_ARRAY;
			return data.array.element[i];
		}
		const bson::element& operator[](size_t i) const
		{
			// ensure type == BSON_ARRAY;
			return data.array.element[i];
		}

		// byte
		value(size_t size, uint8_t* data)
		{
			type = BSON_BYTE;
			construct_byte(size, data);
		}
		value& operator=(const byte& b)
		{
			delete_value();
			type = BSON_BYTE;
			construct_byte(b.size, b.data);

			return *this;
		}
		bool operator==(const byte& byte) const
		{
			return type == BSON_BYTE && data.byte.size == byte.size
				&& std::equal(data.byte.data, data.byte.data + data.byte.size, byte.data);
		}
		bool operator<(const byte& byte) const
		{
			return type == BSON_BYTE
				&& std::lexicographical_compare(data.byte.data, data.byte.data + data.byte.size, byte.data, byte.data + byte.size);
		}

		explicit value(bool b)
		{
			type = b ? BSON_TRUE : BSON_FALSE;
		}
		value& operator=(bool b)
		{
			delete_value();
			type = b ? BSON_TRUE : BSON_FALSE;
		}
		bool operator==(bool b) const
		{
			return type == (b ? BSON_TRUE : BSON_FALSE);
		}
		bool operator<(bool b) const
		{
			return type == BSON_FALSE ? b : false;
		}

		// int32
		// int64

		// time_t
		value(time_t t)
		{
			type = BSON_DATE;
			data.date = t;
		}
		value& operator=(time_t t)
		{
			delete_value();
			type = BSON_DATE;
			data.date = t;

			return *this;
		}
		bool operator==(time_t date) const
		{
			return type == BSON_DATE && data.date == date;
		}
		bool operator<(time_t date) const
		{
			return type == BSON_DATE && data.date < date;
		}
	protected:
		void construct_string(const char* s)
		{
			data.string.size = strlen(s) + 1;
			data.string.data = new char[data.string.size + 1];
			strcpy(data.string.data, s);
		}
		void delete_string(void)
		{
			delete [] data.string.data;
		}

		void construct_array(size_t n, const bson::element* v)
		{
			data.array.size = n;
			data.array.element = new bson::element[n];
			std::copy(v, v + n, data.array.element);
		}
/*		void construct_array(size_t n, const value* v)
		{
			data.array.size = n;
			data.array.element = new bson::element[n];
			std::copy(v, v + n, data.array.element);
		}
*/		void delete_array(void)
		{
			delete [] data.array.element;
		}

		void construct_byte(size_t n, const uint8_t* b)
		{
			data.byte.size = n;
			data.byte.data = new uint8_t[n];
			memcpy(data.byte.data, b, n);
		}
		void delete_byte(void)
		{
			delete [] data.byte.data;
		}

		void delete_value()
		{
			switch (type) {
			case BSON_STRING:
				delete_string();
				break;
			case BSON_ARRAY:
				delete_array();
				break;
			case BSON_BYTE:
				delete_byte();
				break;
			}
		}
	};

	class object : public std::map<const char*, value> {
	public:
		object();
		object(const char* string);
		object(const object&);
		object& operator=(const object&);
		object(const object&&);
		object& operator=(const object&&);
		~object();
	};

	// read object from string
	inline object recv(const char* s)
	{
		object o;
		// skip length
		s += 4;
		int8_t type = *s++; 
		size_t n = strlen(s);
		const char* key = s;
		s += n;
		
		switch (type) {
		case 0x01:
			o[key] = value(*(double*)s); 
			s += sizeof(double);
			break;
		case 0x02:
			o[key] = value(s);
		}
	}

} // namespace bson
/*
std::ostream& operator(std::ostream& os, const bson::value& v)
{

	return os;
}
*/

