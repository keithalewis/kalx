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

	class value {
	public:
		element element;
		element_t type() const
		{
			return element.type;
		}

		value()
		{
			element.type = BSON_NULL;
		}
		value(const value& v)
		{
			operator=(v);
		}
		value& operator=(const value& v)
		{
			if (this != &v) {
				switch (v.element.type) {
				case BSON_STRING:
					operator=(v.element.data.string);
					break;
				case BSON_ARRAY:
					operator=(v.element.data.array);
					break;
				case BSON_BYTE:
					operator=(v.element.data.byte);
					break;
				default: // non pointer types
					element.type = v.element.type;
					element.data = v.element.data;
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
				element.type = e.type;
				element.data = e.data;
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
			return element.type != v.element.type ? false 
				: element.type == BSON_STRING ? operator==(v.element.data.string) 
				: element.type == BSON_NUMBER ? element.data.number == v.element.data.number
				: element.type == BSON_OBJECT ? element.data.object == v.element.data.object // point to same object
				: element.type == BSON_ARRAY ? operator==(v.element.data.array)
				: element.type == BSON_TRUE ? true
				: element.type == BSON_FALSE ? true
				: element.type == BSON_NULL ? false // null <> null
				: element.type == BSON_BYTE ? operator==(v.element.data.byte)
				: element.type == BSON_INT32 ? element.data.int32 == v.element.data.int32
				: element.type == BSON_INT64 ? element.data.int64 == v.element.data.int64
				: element.type == BSON_DATE ? element.data.date == v.element.data.date
				: false // shouldn't happen
				;
		}
		bool operator<(const value& v) const
		{
			return element.type != v.element.type ? element.type < v.element.type // <--- fix this!!!
				: element.type == BSON_STRING ? operator<(v.element.data.string) 
				: element.type == BSON_NUMBER ? element.data.number < v.element.data.number
				: element.type == BSON_OBJECT ? element.data.object < v.element.data.object // meaningless
				: element.type == BSON_ARRAY ? operator<(v.element.data.array)
				: element.type == BSON_TRUE ? true
				: element.type == BSON_FALSE ? true
				: element.type == BSON_NULL ? false // null <> null
				: element.type == BSON_BYTE ? operator<(v.element.data.byte)
				: element.type == BSON_INT32 ? element.data.int32 < v.element.data.int32
				: element.type == BSON_INT64 ? element.data.int64 < v.element.data.int64
				: element.type == BSON_DATE ? element.data.date < v.element.data.date
				: false // shouldn't happen
				;
		}
		value(value&& v)
		{
			element.type = v.element.type;
			element.data = v.element.data;

			v.element.type = BSON_NULL;
		}
		value& operator=(value&& v)
		{
			if (this != &v) {
				element.type = v.element.type;
				element.data = v.element.data;

				v.element.type = BSON_NULL;
			}

			return *this;
		}
		value(const char* s)
		{
			element.type = BSON_STRING;
			construct_string(s);
		}
		value(const string& s)
		{
			element.type = BSON_STRING;
			construct_string(s.data);
		}
		value& operator=(const char* s)
		{
			delete_value();
			element.type = BSON_STRING;
			construct_string(s);

			return *this;
		}
		value& operator=(const string& s)
		{
			delete_value();
			element.type = BSON_STRING;
			construct_string(s.data);

			return *this;
		}
		bool operator==(const char* s) const
		{
			return element.type == BSON_STRING && strcmp(element.data.string.data, s) == 0;
		}
		bool operator<(const char* s) const
		{
			return element.type == BSON_STRING && strcmp(element.data.string.data, s) < 0;
		}

		explicit value(double number)
		{
			element.type = BSON_NUMBER;
			element.data.number = number;
		}
		value& operator=(double number)
		{
			delete_value();
			element.type = BSON_NUMBER;
			element.data.number = number;

			return *this;
		}
		bool operator==(double number) const
		{
			return element.type == BSON_NUMBER && element.data.number == number;
		}
		bool operator<(double number) const
		{
			return element.type == BSON_NUMBER && element.data.number < number;
		}

		// array
		value(size_t n, const bson::element* elements)
		{
			element.type = BSON_ARRAY;
			construct_array(n, elements);
		}
/*		value(size_t n, const value* value)
		{
			element.type = BSON_ARRAY;
			construct_array(n, value);
		}
*/		value& operator=(const array& a)
		{
			delete_value();
			element.type = BSON_ARRAY;
			construct_array(a.size, a.element);

			return *this;
		}
		bool operator==(const array& array) const
		{
			return element.type == BSON_ARRAY && element.data.array.size == array.size
				&& std::equal(element.data.array.element, element.data.array.element + element.data.array.size, array.element);
		}
		bool operator<(const array& array) const
		{
			return element.type == BSON_ARRAY && std::lexicographical_compare(element.data.array.element, element.data.array.element + element.data.array.size, array.element, array.element + array.size);
		}
		bson::element& operator[](size_t i)
		{
			// ensure type == BSON_ARRAY;
			return element.data.array.element[i];
		}
		const bson::element& operator[](size_t i) const
		{
			// ensure type == BSON_ARRAY;
			return element.data.array.element[i];
		}

		// byte
		value(size_t size, uint8_t* data)
		{
			element.type = BSON_BYTE;
			construct_byte(size, data);
		}
		value& operator=(const byte& b)
		{
			delete_value();
			element.type = BSON_BYTE;
			construct_byte(b.size, b.data);

			return *this;
		}
		bool operator==(const byte& byte) const
		{
			return element.type == BSON_BYTE && element.data.byte.size == byte.size
				&& std::equal(element.data.byte.data, element.data.byte.data + element.data.byte.size, byte.data);
		}
		bool operator<(const byte& byte) const
		{
			return element.type == BSON_BYTE
				&& std::lexicographical_compare(element.data.byte.data, element.data.byte.data + element.data.byte.size, byte.data, byte.data + byte.size);
		}

		explicit value(bool b)
		{
			element.type = b ? BSON_TRUE : BSON_FALSE;
		}
		value& operator=(bool b)
		{
			delete_value();
			element.type = b ? BSON_TRUE : BSON_FALSE;
		}
		bool operator==(bool b) const
		{
			return element.type == (b ? BSON_TRUE : BSON_FALSE);
		}
		bool operator<(bool b) const
		{
			return element.type == BSON_FALSE ? b : false;
		}

		// int32
		// int64

		// time_t
		value(time_t t)
		{
			element.type = BSON_DATE;
			element.data.date = t;
		}
		value& operator=(time_t t)
		{
			delete_value();
			element.type = BSON_DATE;
			element.data.date = t;

			return *this;
		}
		bool operator==(time_t date) const
		{
			return element.type == BSON_DATE && element.data.date == date;
		}
		bool operator<(time_t date) const
		{
			return element.type == BSON_DATE && element.data.date < date;
		}
	protected:
		void construct_string(const char* s)
		{
			element.data.string.size = strlen(s) + 1;
			element.data.string.data = new char[element.data.string.size + 1];
			strcpy(element.data.string.data, s);
		}
		void delete_string(void)
		{
			delete [] element.data.string.data;
		}

		void construct_array(size_t n, const bson::element* v)
		{
			element.data.array.size = n;
			element.data.array.element = new bson::element[n];
			std::copy(v, v + n, element.data.array.element);
		}
/*		void construct_array(size_t n, const value* v)
		{
			element.data.array.size = n;
			element.data.array.element = new bson::element[n];
			std::copy(v, v + n, element.data.array.element);
		}
*/		void delete_array(void)
		{
			delete [] element.data.array.element;
		}

		void construct_byte(size_t n, const uint8_t* b)
		{
			element.data.byte.size = n;
			element.data.byte.data = new uint8_t[n];
			memcpy(element.data.byte.data, b, n);
		}
		void delete_byte(void)
		{
			delete [] element.data.byte.data;
		}

		void delete_value()
		{
			switch (element.type) {
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

