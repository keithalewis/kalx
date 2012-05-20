// json.h - Lightweight C++ wrappers for mongo C library.
#include <cstdint>
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <xutility>

using namespace std::rel_ops;

typedef enum {
	JSON_STRING,
	JSON_NUMBER, // double
	JSON_OBJECT,
	JSON_ARRAY,
	JSON_TRUE,
	JSON_FALSE,
	JSON_NULL,
	JSON_BYTE,
	JSON_INT32,
	JSON_INT64,
	JSON_DATE
} json_element_type;

namespace json {

	class value;
	struct element;
	typedef std::map<std::string, value> object;

	struct string {
		size_t size;
		const char* data;
	};
	inline string string_(size_t size = 0, const char* data = 0)
	{
		string s;

		s.size = size;
		s.data = data;

		return s;
	}

	struct array {
		size_t size;
		const element* element;
	};
	inline array array_(size_t size, const element* element)
	{
		array e;

		e.size = size;
		e.element = element;
	}

	struct byte {
		size_t size;
		const uint8_t* data;
	};
	inline byte byte_(size_t size, const uint8_t* data)
	{
		byte b;

		b.size = size;
		b.data = data;

		return b;
	}

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
		json_element_type type;
	};

	inline bool operator==(const string& s, const string& t)
	{
		return s.size == t.size && 0 == strcmp(s.data, t.data);
	}
	inline bool operator==(const string& s, const char* t)
	{
		return s == string_(strlen(t), t);
	}
	inline bool operator<(const string& s, const string& t)
	{
		return strcmp(s.data, t.data) < 0;
	}
	inline bool operator<(const string& s, const char* t)
	{
		return s < string_(strlen(t), t);
	}

	inline bool operator==(const array& a, const array& b)
	{
		return a.size == b.size && std::equal(a.element, a.element + a.size, b.element);
	}
	inline bool operator<(const array& a, const array& b)
	{
		return std::lexicographical_compare(a.element, a.element + a.size, b.element, b.element + b.size);
	}

	inline bool operator==(const byte& a, const byte& b)
	{
		return a.size == b.size && 0 == memcmp(a.data, b.data, a.size);
	}
	inline bool operator<(const byte& a, const byte& b)
	{
		return std::lexicographical_compare(a.data, a.data + a.size, b.data, b.data + b.size);
	}

	inline bool operator==(const element& e, const string& s)
	{
		return e.type == JSON_STRING && e.data.string == s;
	}
	inline bool operator<(const element& e, const string& s)
	{
		return e.type == JSON_STRING && e.data.string < s;
	}
	inline bool operator==(const element& e, const char* s)
	{
		return e.type == JSON_STRING && e.data.string == s;
	}
	inline bool operator<(const element& e, const char* s)
	{
		return e.type == JSON_STRING && e.data.string < s;
	}

	inline bool operator==(const element& e, double number)
	{
		return e.type == JSON_NUMBER && e.data.number == number;
	}
	inline bool operator<(const element& e, double number)
	{
		return e.type == JSON_NUMBER && e.data.number < number;
	}

	inline bool operator==(const element& e, const array& array)
	{
		return e.type == JSON_ARRAY && e.data.array == array;
	}
	inline bool operator<(const element& e, const array& array)
	{
		return e.type == JSON_ARRAY && e.data.array < array;
	}

	inline bool operator==(const element& e, const byte& byte)
	{
		return e.type == JSON_BYTE && e.data.byte == byte;
	}
	inline bool operator<(const element& e, const byte& byte)
	{
		return e.type == JSON_BYTE && e.data.byte < byte;
	}

	inline bool operator==(const element& e, bool b)
	{
		return b && e.type == JSON_TRUE || !b && e.type == JSON_FALSE;
	}
	inline bool operator<(const element& e, bool b)
	{
		return e.type == JSON_FALSE && b;
	}

	inline bool operator==(const element& e, time_t date)
	{
		return e.type == JSON_DATE && e.data.date == date;
	}
	inline bool operator<(const element& e, time_t date)
	{
		return e.type == JSON_DATE && e.data.date < date;
	}

	inline bool operator==(const element& a, const element& b)
	{
		return a.type != b.type ? false
			: a.type == JSON_STRING ? a == b.data.string
			: a.type == JSON_NUMBER ? a == b.data.number
			: a.type == JSON_OBJECT ? a.data.object == b.data.object
			: a.type == JSON_ARRAY ? a == b.data.array
			: a.type == JSON_TRUE ? b.type == JSON_TRUE
			: a.type == JSON_FALSE ? b.type == JSON_FALSE
			: a.type == JSON_NULL ? false // just like javascript
			: a.type == JSON_BYTE ? a == b.data.byte
			: a.type == JSON_INT32 ? a.data.int32 == b.data.int32 
			: a.type == JSON_INT64 ? a.data.int64 == b.data.int64
			: a.type == JSON_DATE ? a.data.date == b.data.date
			: false
			;
	}
	inline bool operator<(const element& a, const element& b)
	{
		return a.type < b.type ? true
			: a.type >  b.type ? false
			: a.type == JSON_STRING ? a < b.data.string
			: a.type == JSON_NUMBER ? a < b.data.number
			: a.type == JSON_OBJECT ? a.data.object < b.data.object
			: a.type == JSON_ARRAY ? a < b.data.array
			: a.type == JSON_TRUE ? false
			: a.type == JSON_FALSE ? b.type == JSON_TRUE
			: a.type == JSON_NULL ? false // just like javascript
			: a.type == JSON_BYTE ? a < b.data.byte
			: a.type == JSON_INT32 ? a.data.int32 < b.data.int32 
			: a.type == JSON_INT64 ? a.data.int64 < b.data.int64
			: a.type == JSON_DATE ? a.data.date < b.data.date
			: false
			;
	}

	class value : public element {
	public:
		operator json::element&()
		{
			return *this;
		}
		operator const json::element&() const
		{
			return *this;
		}
		bool operator!() const
		{
			return type == JSON_NULL || type == JSON_FALSE;
		}

		value()
		{
			type = JSON_NULL;
		}
		value(const value& v)
		{
			operator=(v);
		}
		value& operator=(const value& v)
		{
			if (this != &v) {
				switch (v.type) {
				case JSON_STRING:
					operator=(v.data.string);
					break;
				case JSON_ARRAY:
					operator=(v.data.array);
					break;
				case JSON_BYTE:
					operator=(v.data.byte);
					break;
				default: // non pointer types
					type = v.type;
					data = v.data;
				}
			}

			return *this;
		}
		value(const json::element& e)
		{
			operator=(e);
		}
		value& operator=(const json::element& e)
		{
			switch (e.type) {
			case JSON_STRING:
				operator=(e.data.string);
				break;
			case JSON_ARRAY:
				operator=(e.data.array);
				break;
			case JSON_BYTE:
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
		value(value&& v)
		{
			type = v.type;
			data = v.data;

			v.type = JSON_NULL;
		}
		value& operator=(value&& v)
		{
			if (this != &v) {
				type = v.type;
				data = v.data;

				v.type = JSON_NULL;
			}

			return *this;
		}
		value(const char* s)
		{
			type = JSON_STRING;
			construct_string(s);
		}
		value(const string& s)
		{
			type = JSON_STRING;
			construct_string(s.data);
		}
		value& operator=(const char* s)
		{
			delete_value();
			type = JSON_STRING;
			construct_string(s);

			return *this;
		}
		value& operator=(const string& s)
		{
			delete_value();
			type = JSON_STRING;
			construct_string(s.data);

			return *this;
		}
		// specialize for const char*
		bool operator==(const char* s) const
		{
			return operator const json::element&() == s;
		}
		bool operator<(const char* s) const
		{
			return operator const json::element&() < s;
		}

		explicit value(double number)
		{
			type = JSON_NUMBER;
			data.number = number;
		}
		value& operator=(double number)
		{
			delete_value();
			type = JSON_NUMBER;
			data.number = number;

			return *this;
		}
		bool operator==(double number) const
		{
			return operator const json::element&() == number;
		}
		bool operator<(double number) const
		{
			return operator const json::element&() < number;
		}

		// array
		explicit value(int n)
		{
			type = JSON_ARRAY;
			construct_array(n);
		}
		value& operator=(const array& a)
		{
			delete_value();
			type = JSON_ARRAY;
			std::copy(a.element, a.element + a.size, const_cast<json::element*>(data.array.element));

			return *this;
		}
		json::element& operator[](size_t i)
		{
			// ensure type == JSON_ARRAY;
			return const_cast<json::element&>(data.array.element[i]);
		}
		const json::element& operator[](size_t i) const
		{
			// ensure type == JSON_ARRAY;
			return data.array.element[i];
		}

		// byte
		value(size_t size, uint8_t* data)
		{
			type = JSON_BYTE;
			construct_byte(size, data);
		}
		value& operator=(const byte& b)
		{
			delete_value();
			type = JSON_BYTE;
			construct_byte(b.size, b.data);

			return *this;
		}

		explicit value(bool b)
		{
			type = b ? JSON_TRUE : JSON_FALSE;
		}
		value& operator=(bool b)
		{
			delete_value();
			type = b ? JSON_TRUE : JSON_FALSE;

			return *this;
		}
		bool operator==(bool b) const
		{
			return operator const json::element&() == b;
		}
		bool operator<(bool b) const
		{
			return operator const json::element&() < b;
		}

		// int32
		// int64

		// time_t
		explicit value(time_t t)
		{
			type = JSON_DATE;
			data.date = t;
		}
		value& operator=(time_t t)
		{
			delete_value();
			type = JSON_DATE;
			data.date = t;

			return *this;
		}
		bool operator==(time_t date) const
		{
			return operator const json::element&() == date;
		}
		bool operator<(time_t date) const
		{
			return operator const json::element&() < date;
		}
	protected:
		void construct_string(const char* s)
		{
			data.string.size = strlen(s);
			data.string.data = new char[data.string.size + 1];
			strcpy(const_cast<char*>(data.string.data), s);
		}
		void delete_string(void)
		{
			delete [] data.string.data;
		}

		void construct_array(size_t n)
		{
			data.array.size = n;
			data.array.element = new json::element[n];
		}
		void delete_array(void)
		{
			delete [] data.array.element;
		}

		void construct_byte(size_t n, const uint8_t* b)
		{
			data.byte.size = n;
			data.byte.data = new uint8_t[n];
			memcpy(const_cast<uint8_t*>(data.byte.data), b, n);
		}
		void delete_byte(void)
		{
			delete [] data.byte.data;
		}

		void delete_value()
		{
			switch (type) {
			case JSON_STRING:
				delete_string();
				break;
			case JSON_ARRAY:
				delete_array();
				break;
			case JSON_BYTE:
				delete_byte();
				break;
			}
		}
	};

} // namespace bson

std::ostream& operator<<(std::ostream& os, const json::value& v)
{
	switch (v.type) {
	case JSON_STRING: os << '"' << v.data.string.data << '"'; break;
	case JSON_NUMBER: os << v.data.number; break;
	case JSON_OBJECT: os << '(' << v.data.object << ')'; break;
	case JSON_ARRAY: { 
		os << '[';
		for (size_t i = 0; i < v.data.array.size; ++i) {
			if (i) os << ',';
			os << v[i];
		}
		os << ']';
		break;
	}
	case JSON_TRUE:  os << "true"; break;
	case JSON_FALSE: os << "false"; break;
	case JSON_NULL:  os << "null"; break;

	default:
		os << "*undefined*";
	}

	return os;
}
std::ostream& operator<<(std::ostream& os, const json::object& o)
{
	std::map<std::string,json::value>::const_iterator i;

	os << '{';
	for (i = o.begin(); i != o.end(); ++i) {
		if (i != o.begin()) os << ',';
		os << '"' << i->first << "\":" << i->second; 
	}
	os << '}';

	return os;
}


