#pragma once

#include <cassert>
#include <iosfwd>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace JSON {

	enum class type {
		Value,
		String,
		Number,
		Object,
		Array,
		True,
		False,
		Null,
		TotalTypes
	};

	struct Value {
		static std::shared_ptr<Value> parse(std::istream &src);

		virtual ~Value(){};
		virtual type type_id() const																	 = 0;
		virtual void print(std::ostream &dest, size_t level = 0) const = 0;
		template <class T>
		constexpr T &as() {
			auto *p = dynamic_cast<T *>(this);
			assert(p);
			return *p;
		}

	protected:
		static std::string buffer;
	};

	std::ostream &operator<<(std::ostream &dest, const Value &val);

	struct String : Value, std::string {
		static std::shared_ptr<String> parse(std::istream &src);

		String() : std::string() {}
		String(const std::string &str) : std::string(str) {}
		String(std::string &&str) : std::string(str) {}

		String &operator=(const std::string &str) {
			std::string::operator=(str);
			return *this;
		}
		String &operator=(std::string &&str) {
			std::string::operator=(str);
			return *this;
		}

		virtual ~String(){};

		type type_id() const { return type::String; }
		void print(std::ostream &dest, size_t level) const;
	};

	struct Number : Value {
		static std::shared_ptr<Number> parse(std::istream &src);

		virtual ~Number(){};
		type type_id() const { return type::Number; }
		void print(std::ostream &dest, size_t level) const;

		operator double &() { return val; }
		operator const double &() const { return val; }

		Number(double val) : val(val){};

	private:
		double val;
	};

	double			double_from_binary_string(const std::string &str);
	std::string binary_string_from_double(double x);

	struct Object : Value, std::map<std::string, std::shared_ptr<Value>> {
		static std::shared_ptr<Object> parse(std::istream &src);

		type type_id() const { return type::Object; }
		void print(std::ostream &dest, size_t level = 0) const;
	};

	struct Array : Value, std::vector<std::shared_ptr<Value>> {
		static std::shared_ptr<Array> parse(std::istream &src);

		type type_id() const { return type::Array; }
		void print(std::ostream &dest, size_t level) const;
	};

	struct True : Value {
		static std::shared_ptr<True> parse(std::istream &src);

		~True(){};
		type type_id() const { return type::True; };
		void print(std::ostream &dest, size_t level) const;
	};

	struct False : Value {
		static std::shared_ptr<False> parse(std::istream &src);

		~False(){};
		type type_id() const { return type::False; };
		void print(std::ostream &dest, size_t level) const;
	};

	struct Null : Value {
		static std::shared_ptr<Null> parse(std::istream &src);

		~Null(){};
		type type_id() const { return type::Null; };
		void print(std::ostream &dest, size_t level) const;
	};
}