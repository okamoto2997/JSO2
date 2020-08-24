#pragma once

#include <iosfwd>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace JSO2 {

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

	class JSO2 {
		type									_t;
		std::shared_ptr<void> _v;

	public:
		using Object = std::map<std::string, JSO2>;
		using Array	 = std::vector<JSO2>;
		using String = std::string;
		using Number = double;
		using Null	 = nullptr_t;
		type get_type() const;

		static const Object &blank_object();
		static const Array & blank_array();
		static const Null &	 null();

		JSO2();
		JSO2(const Object &);
		JSO2(const Array &);
		JSO2(const String &str);
		JSO2(const Number &x);
		JSO2(const Null &x);
		JSO2(std::istream &src);

		bool load(std::istream &src);

		JSO2 &operator=(const Object &);
		JSO2 &operator=(const Array &);
		JSO2 &operator=(const String &);
		JSO2 &operator=(const char *);
		JSO2 &operator=(const Number &);
		JSO2 &operator=(int);
		JSO2 &operator=(bool);
		JSO2 &operator=(const Null &);

		JSO2 &			operator[](const String &key);
		const JSO2 &operator[](const String &key) const;
		JSO2 &			operator[](const char *key);
		const JSO2 &operator[](const char *key) const;
		JSO2 &			operator[](int index);
		const JSO2 &operator[](int index) const;

		operator const Object &() const;
		operator Object &();
		operator const Array &() const;
		operator Array &();
		operator const String &() const;
		operator String &();
		operator const Number &() const;
		operator Number &();
		operator bool() const;

		template <class T>
		const T &as() const {
			return static_cast<T *>(_v);
		}
	};

	std::ostream &operator<<(std::ostream &dest, const JSO2 &jso2);

}