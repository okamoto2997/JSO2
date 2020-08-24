#include "JSONParser.h"

#include <cctype>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <limits>
#include <memory>
#include <sstream>

namespace JSON {
#define fault(name)                                     \
	do {                                                  \
		std::cerr << "# JSON : parse error in " #name "\n"; \
		std::cerr << "# >>>>";                              \
		std::cerr << buffer;                                \
		std::cerr << "<<<<\n";                              \
		return nullptr;                                     \
	} while(false)

#define dequeue(name)                 \
	do {                                \
		if(type::name == type::String) {  \
			if(src)                         \
				buffer.push_back(src.get());  \
			else {                          \
				fault(name);                  \
			}                               \
		} else {                          \
			if(src) {                       \
				if(src.peek() == '#')         \
					while(src.peek() != '\n') { \
						if(src)                   \
							src.get();              \
						else                      \
							fault(name);            \
					}                           \
				buffer.push_back(src.get());  \
			} else {                        \
				fault(name);                  \
			}                               \
		}                                 \
	} while(false)

	bool is_white_space(char c) {
		switch(c) {
			case ' ':		// space
			case '\n':	// linefeed
			case '\r':	// carriage return
			case '\t':	// horizontal tab
				return true;
		}
		return false;
	}

	std::ostream &operator<<(std::ostream &dest, const Value &val) {
		val.print(dest);
		return dest;
	}

	bool is_one_nine(char c) { return '1' <= c && c <= '9'; }

	bool is_digit(char c) { return '0' <= c && c <= '9'; }

	std::shared_ptr<Value> Value::parse(std::istream &src) {
		buffer.clear();
		while(is_white_space(src.peek())) dequeue(Value);
		std::shared_ptr<Value> val;
		switch(src.peek()) {
			case '\"':
				val = String::parse(src);
				break;
			case '{':
				val = Object::parse(src);
				break;
			case '[':
				val = Array::parse(src);
				break;
			case 't':
				val = True::parse(src);
				break;
			case 'f':
				val = False::parse(src);
				break;
			case 'n':
				val = Null::parse(src);
				break;
			case '-':
			default:
				if(is_digit(src.peek())) val = Number::parse(src);
		}
		if(!val) fault(Value);

		while(is_white_space(src.peek())) dequeue(Value);

		return std::shared_ptr<Value>(val);
	}

	std::string Value::buffer;

	std::shared_ptr<String> String::parse(std::istream &src) {
		buffer.clear();

		if(src && src.peek() != '"') return nullptr;
		src.get();

		do {
			dequeue(String);

			if(buffer.back() == '\"') {
				buffer.pop_back();
				return std::make_shared<String>(buffer);
			} else if(buffer.back() == '\\') {
				dequeue(String);
				switch(buffer.back()) {
#define push(c)      \
	buffer.pop_back(); \
	buffer.pop_back(); \
	buffer.push_back(c);
					case '"':
						push('\"');
						break;
					case '\\':
						push('\\');
						break;
					case '/':
						push('/');
						break;
					case 'b':
						push('\b');
						break;
					case 'f':
						push('\f');
						break;
					case 'n':
						push('\n');
						break;
					case 'r':
						push('\r');
						break;
					case 't':
						push('\t');
						break;
#undef push
					default:
						fault(String);
						return nullptr;
				}
			}
		} while(1);
	}

	void String::print(std::ostream &dest, size_t) const {
		dest << "\"" << static_cast<std::string>(*this) << "\"";
	};

	std::shared_ptr<Number> Number::parse(std::istream &src) {
		buffer.clear();

		if(src.peek() == '-') dequeue(Number);

		if(src.peek() == '0')
			dequeue(Number);
		else if(is_one_nine(src.peek())) {
			dequeue(Number);
			while(is_digit(src.peek())) dequeue(Number);
		} else
			fault(Number);

		if(src.peek() == '.') {
			dequeue(Number);
			while(is_digit(src.peek())) dequeue(Number);
		}
		if(src.peek() == 'e' || src.peek() == 'E') {
			dequeue(Number);
			if(src.peek() == '-' || src.peek() == '+') dequeue(Number);
			if(!is_digit(src.peek()))
				fault(Number);
			else
				while(is_digit(src.peek())) dequeue(Number);
		}
		return std::make_shared<Number>(std::stod(buffer));
	}

	void Number::print(std::ostream &dest, size_t) const { dest << val; }

	bool is_hex(char c) {
		return ('0' <= c && c <= '9') || ('A' <= c && c <= 'F');
	}

	int from_hex(char c) { return '0' <= c && c <= '9' ? c - '0' : c - 'A' + 10; }
	char to_hex(int n) {
		n &= 15;
		return n < 10 ? n + '0' : n - 10 + 'A';
	}

	double double_from_binary_string(const std::string &str) {
		uint64_t binary = 0;
		double	 x;

		if(str.size() < 16) return std::numeric_limits<double>::quiet_NaN();

		for(size_t i = 0; i < 16; ++i) {
			if(!is_hex(str[i])) return std::numeric_limits<double>::quiet_NaN();
			binary <<= 4;
			binary |= (from_hex(str[i]) & 15);
		}
		memcpy(&x, &binary, sizeof(double));
		return x;
	}

	std::string binary_string_from_double(double x) {
		uint64_t binary;

		std::memcpy(&binary, &x, sizeof(double));
		char buf[17] = {};
		for(size_t i = 0; i < 16; ++i) {
			buf[15 - i] = to_hex(binary);
			binary >>= 4;
		}
		return buf;
	}

	std::shared_ptr<Object> Object::parse(std::istream &src) {
		buffer.clear();

		if(src.peek() != '{') fault(Object);
		dequeue(Object);

		std::shared_ptr<Object> ret(new Object);

		while(1) {
			while(is_white_space(src.peek())) dequeue(Object);
			if(src.peek() == '}') {
				dequeue(Object);
				return ret;
			}
			std::shared_ptr<String> p_key(String::parse(src));

			if(!p_key) fault(Object);

			while(is_white_space(src.peek())) dequeue(Object);

			if(src.peek() != ':') fault(Object);
			dequeue(Object);

			std::shared_ptr<Value> p_value = Value::parse(src);

			if(!p_value) fault(Object);

			if(ret->contains((std::string)*p_key)) {
				std::cerr << "# warning : key \"" << (std::string)*p_key
									<< "\" already contained.\n";
				std::cerr << "# The previous one is ignored.\n";
			}

			(*ret)[(std::string)*p_key] = p_value;

			if(src.peek() == '}') {
				dequeue(Object);
				return ret;
			}

			if(src.peek() != ',') fault(Object);
			dequeue(Object);
		}
	}

	void Object::print(std::ostream &dest, size_t level) const {
		std::string tab(level, '\t');

		size_t max_key_len = 0;
		for(const auto &[key, _] : *this)
			max_key_len = std::max(max_key_len, key.length());

		dest << "{\n";
		if(size() != 0)
			for(auto iter = begin();;) {
				const auto &[key, p_val] = *iter;
				dest << tab << "\t" << std::setw(max_key_len + 2) << std::left
						 << "\"" + key + "\""
						 << " : ";
				p_val->print(dest, level + 1);
				if(++iter == end()) {
					dest << "\n";
					break;
				} else
					dest << ",\n";
			}
		dest << tab << "}";
	}

	std::shared_ptr<Array> Array::parse(std::istream &src) {
		buffer.clear();

		if(src.peek() != '[') fault(Array);
		dequeue(Array);

		while(is_white_space(src.peek())) dequeue(Array);

		std::shared_ptr<Array> ret(new Array);

		while(1) {
			if(src.peek() == ']') {
				dequeue(Array);
				return ret;
			}

			std::shared_ptr<Value> p_value = Value::parse(src);

			if(!p_value) fault(Array);

			ret->push_back(p_value);

			if(src.peek() == ']') {
				dequeue(Array);
				return ret;
			}

			if(src.peek() != ',') fault(Object);
			dequeue(Array);
		}
	}

	void Array::print(std::ostream &dest, size_t level) const {
		std::string tab(level, '\t');

		dest << "[\n";
		for(auto iter = begin();;) {
			const auto p_val = *iter;
			dest << tab << "\t";
			p_val->print(dest, level + 1);
			if(++iter == end()) {
				dest << "\n";
				break;
			} else
				dest << ",\n";
		}
		dest << tab << "]";
	}

	std::shared_ptr<True> True::parse(std::istream &src) {
		buffer.clear();
		for(size_t i = 0; i < 4; ++i) {
			dequeue(True);
			if(buffer.back() != "true"[i]) fault(True);
		}
		return std::make_shared<True>();
	}

	void True::print(std::ostream &dest, size_t) const { dest << "true"; }

	std::shared_ptr<False> False::parse(std::istream &src) {
		buffer.clear();
		for(size_t i = 0; i < 5; ++i) {
			dequeue(False);
			if(buffer.back() != "false"[i]) fault(False);
		}
		return std::make_shared<False>();
	}

	void False::print(std::ostream &dest, size_t) const { dest << "false"; }

	std::shared_ptr<Null> Null::parse(std::istream &src) {
		buffer.clear();
		for(size_t i = 0; i < 4; ++i) {
			dequeue(True);
			if(buffer.back() != "null"[i]) fault(True);
		}
		return std::make_shared<Null>();
	}

	void Null::print(std::ostream &dest, size_t) const { dest << "null"; }

#undef pop
#undef fault

	/*
	namespace v1 {

	#define fault           \
	do {                \
		next = begin;   \
		return nullptr; \
	} while(false)

	#define advance           \
	do {                  \
		if(++next == end) \
			fault;        \
	} while(false)

		Value *Value::parse(const std::string &str, size_t begin, size_t end, size_t
	&next) { while(isspace(str[next])) advance;

			Value *ret = nullptr;
			if((ret = String::parse(str, next, end, next)))
				;
			else if((ret = Number::parse(str, next, end, next)))
				;
			else if((ret = Object::parse(str, next, end, next)))
				;
			else if((ret = Array::parse(str, next, end, next)))
				;
			else if((ret = True::parse(str, next, end, next)))
				;
			else if((ret = False::parse(str, next, end, next)))
				;
			else if((ret = Null::parse(str, next, end, next)))
				;
			else {
				std::cerr << "# JSON parsing error.\n";
				std::cerr << "# >>>>>>";
				std::cerr << str.substr(begin, end - begin);
				std::cerr << "# <<<<<";
				fault;
			}

			//		std::cerr << ">>>> Parse result : " << (int)ret->type_id() << "\n";
			//		std::cerr << str.substr(begin, next - begin) << "\n";
			//		std::cerr << "Parse result : " << (int)ret->type_id() << "<<<<<\n";

			while(isspace(str[next])) advance;

			return ret;
		}

		String *String::parse(const std::string &str, size_t begin, size_t end,
	size_t &next) { next = begin; if(str[next] != '\"') fault;

			advance;

			while(str[next] != '\"')
				advance;
			advance;

			return new String(str, begin, next);

			return nullptr;
		}

		String::String(const std::string &str, size_t begin, size_t end) : str(str,
	begin, end - begin) {}

		std::string String::to_string() const {
			return str;
		}
		void String::print(std::ostream &dest, size_t) const {
			dest << str;
		}

		bool is_one_nine(char c) {
			return '1' <= c && c <= '9';
		}

		bool is_digit(char c) {
			return '0' <= c && c <= '9';
		}

		Number *Number::parse(const std::string &str, size_t begin, size_t end,
	size_t &next) { next = begin; if(str[next] == '-') advance;

			if(str[next] == '0')
				advance;
			else if(is_one_nine(str[next]))
				do
					advance;
				while(is_digit(str[next]));
			else
				fault;

			if(str[next] == '.')
				do
					advance;
				while(is_digit(str[next]));

			if(str[next] == 'e' || str[next] == 'E') {
				advance;
				if(str[next] == '+' || str[next] == '-')
					advance;
				while(is_digit(str[next]))
					advance;
			}

			return new Number(str, begin, next);
		}

		Number::Number(const std::string &str, size_t begin, size_t end) : str(str,
	begin, end - begin) {}

		std::string Number::to_string() const { return str; }
		void		Number::print(std::ostream &dest, size_t) const {
				 dest << str;
		}

		Object *Object::parse(const std::string &str, size_t begin, size_t end,
	size_t &next) { next = begin;

			if(str[next] != '{')
				fault;
			advance;

			std::unique_ptr<Object> ret(new Object);

			do {
				while(isspace(str[next]))
					advance;

				if(str[next] == '}')
					break;

				std::unique_ptr<String> p_key(String::parse(str, next, end, next));

				if(p_key == nullptr)
					fault;

				while(isspace(str[next]))
					advance;

				if(str[next] != ':')
					fault;

				advance;

				if(Value *p_val = Value::parse(str, next, end, next); p_val)
					ret->data.emplace(p_key->to_string(), p_val);

				if(str[next] == ',')
					advance;
				else if(str[next] == '}')
					break;
				else
					fault;
			} while(true);

			if(++next > end)
				fault;
			return ret.release();
		}

		Object::~Object() {
			for(auto &[key, p_val] : data) delete p_val;
		}

		std::string Object::to_string() const {
			std::stringstream dest;
			print(dest, 0);
			return dest.str();
		}

		void Object::print(std::ostream &dest, size_t level) const {
			size_t max_key_size = 0;
			for(const auto &[key, value] : data)
				max_key_size = std::max(max_key_size, key.size());

			const std::string tab(level, '\t');
			dest << "{\n";
			for(auto iter = data.begin(); iter != data.end();) {
				const auto &[key, value] = *iter;
				dest << tab << "\t" << std::setw(max_key_size) << std::left << key << "
	: "; value->print(dest, level + 1); if(++iter == data.end()) dest << "\n";
	else dest << ",\n";
			}
			dest << tab << "}";
		}

		Array *Array::parse(const std::string &str, size_t begin, size_t end, size_t
	&next) { next = begin; if(str[next] != '[') fault; advance;

			std::unique_ptr<Array> ret(new Array);

			do {
				while(isspace(str[next]))
					advance;
				if(str[next] == ']')
					break;
				else if(Value *val = Value::parse(str, next, end, next); val) {
					ret->data.emplace_back(val);
				} else
					fault;

				if(str[next] == ',')
					advance;
				else if(str[next] == ']')
					break;
				else
					fault;

			} while(true);

			advance;

			return ret.release();
		}

		Array::~Array() {
			for(auto &p_val : data) delete p_val;
		}

		void Array::print(std::ostream &dest, size_t level) const {
			std::string tab(level, '\t');
			dest << "[\n";
			for(auto iter = data.begin(); iter != data.end();) {
				const Value *p_val = *iter;
				dest << tab << "\t";
				p_val->print(dest, level + 1);
				if(++iter == data.end())
					dest << "\n";
				else
					dest << ",\n";
			}
			dest << tab << "]";
		}

		True *True::parse(const std::string &str, size_t begin, size_t end, size_t
	&next) { if(end - begin < 4 || str.substr(begin, 4) != "true") { next = begin;
				return nullptr;
			}
			next = begin + 4;
			return new True;
		}

		void True::print(std::ostream &dest, size_t) const {
			dest << "true";
		}

		False *False::parse(const std::string &str, size_t begin, size_t end, size_t
	&next) { if(end - begin < 4 || str.substr(begin, 5) != "false") { next =
	begin; return nullptr;
			}
			next = begin + 5;
			return new False;
		}

		void False::print(std::ostream &dest, size_t) const {
			dest << "false";
		}

		Null *Null::parse(const std::string &str, size_t begin, size_t end, size_t
	&next) { if(end - begin < 4 || str.substr(begin, 4) != "null") { next = begin;
				return nullptr;
			}
			next = begin + 4;
			return new Null;
		}

		void Null::print(std::ostream &dest, size_t) const {
			dest << "null";
		}
	}
		*/
}