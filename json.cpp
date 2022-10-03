#ifndef JSON_CPP
#define JSON_CPP

//json parser
#include <string>
#include <vector>
#include <exception>
#include <sstream>
#include <variant>
#include <unordered_map>
#include <memory>
#include <fstream>

namespace json {
	/* Declarations */

	struct jsonWrapper;

	enum json_t{Null_t=0, Bool_t, Number_t, String_t, Array_t, Object_t};
	bool null_to_obj = true; // if Null type is accessed with a key (eg.: json::jsonWrapper jw(json::Null); jw["a"];) it changes to Object type

	//
	using json_ptr  = std::shared_ptr<jsonWrapper>;

	//json types
	using Null   = std::nullptr_t;
	using Bool   = bool;
	using Number = long double;
	using String = std::string;
	using Array  = std::vector<json_ptr>;
	using Object = std::unordered_map<String, json_ptr>;
	using Value  = std::variant<Null, Bool, Number, String, Array, Object>;


	struct jsonWrapper{
		//data
		std::shared_ptr<Value> data;

		//set
		inline void
		set(Value const& v);

		inline void
		operator =(jsonWrapper const& j);

		inline void
		operator =(Value const v);

		//constructors
		jsonWrapper()
			{set(Null());}

		jsonWrapper(std::shared_ptr<Value> const d)
			{data = d;}
		
		jsonWrapper(jsonWrapper const& j)
			{data = j.data;}

		jsonWrapper(std::string const& str, bool parse_file=false){
			if(!parse_file){
				std::string::const_iterator begin = str.begin();
				parse(begin, str.end());
				return;
			}
			parseFile(str);
		}

		jsonWrapper(std::string::const_iterator begin, std::string::const_iterator const end)
			{parse(begin, end);}

		//parse
		void inline
		parse(std::string::const_iterator& c, std::string::const_iterator const end);

		void
		parseFile(std::string const& filepath);

		//get
		inline json_t
		type() const;

		template<typename T>
		inline T&
		get() const;

		jsonWrapper
		at(size_t const n) const;

		jsonWrapper
		at(String const& k) const;

		jsonWrapper&
		operator [](size_t const n);

		jsonWrapper&
		operator [](String const& k);

		bool
		find(std::string const& k) const;

		//add
		void
		emplace(size_t const n, json::Value const v);

		void
		emplace(std::string const& k, json::Value const v);

		void
		emplaceFile(size_t const n, std::string const& filepath);

		void
		emplaceFile(std::string const& k, std::string const& filepath);

		//destroy
		void
		erase(size_t n);

		void
		erase(std::string const& k);

		~jsonWrapper() = default;

		//throw
		inline void
		throwIfItsNotAnObject() const;

		inline void
		throwIfItsNotAnArray() const;
	};

		//parse functions declaration
	namespace parse{
		Null
		parseNull(std::string::const_iterator& c, std::string::const_iterator const end); //not used
		Bool
		parseBool(std::string::const_iterator& c, std::string::const_iterator const end); //not used
		Number
		parseNumber(std::string::const_iterator& c, std::string::const_iterator const end);
		String
		parseString(std::string::const_iterator& c, std::string::const_iterator const end);
		Array
		parseArray(std::string::const_iterator& c, std::string::const_iterator const end);
		Object
		parseObject(std::string::const_iterator& c, std::string::const_iterator const end);
		Value
		parseValue(std::string::const_iterator& c, std::string::const_iterator const end);
		Value
		parseFile(std::string const& filepath);
	}

	/* definitions */

	namespace exception{
		class GenericError: public std::exception{
			public:
			GenericError() {setmsg();}
			GenericError(std::string const str) {setmsg(str);}

			virtual
			const char* what() const throw(){
				return errmsg.c_str();
			}

			protected:
			std::string errmsg;

			virtual
			void setmsg(std::string const msg=""){
				errmsg="[!] json Generic Error";
				if(!msg.empty()) errmsg+=": "+msg;
			};
		};

		class ParseError: public GenericError{
			public:
			ParseError() {setmsg();}
			ParseError(std::string const msg) {setmsg(msg);}

			protected:
			virtual
			void setmsg(std::string const msg=""){
				errmsg="[!] json Parse Error";
				if(!msg.empty()) errmsg+=": "+msg;
			};
		};

		class AccessError: public GenericError{
			public:
			AccessError() {setmsg();}
			AccessError(std::string const msg) {setmsg(msg);}

			protected:
			virtual
			void setmsg(std::string const msg=""){
				errmsg="[!] json Access Error";
				if(!msg.empty()) errmsg+=": "+msg;
			};
		};

	}


  //useful functions
	namespace tools {
		bool
		strMatch(std::string const& sub, std::string::const_iterator& c, std::string::const_iterator const end){
			std::string::const_iterator spc = c;
			std::string::const_iterator sbc = sub.begin();
			bool res   = true;

			while ((sbc!=sub.end()) && (spc!=end)){
				if (*sbc != *spc) {
					res = false;
					break;
				}
				++spc, ++sbc;
			}
			if (res)
				c = spc;
			return res;
		}

		inline bool
		isWhitespace(char const c){
			return (std::string(" \n\t\r").find(c) != std::string::npos);
		}

		bool
		passWhiteChar(std::string::const_iterator& c, std::string::const_iterator const end){
			while( c != end){
				if (!isWhitespace(*c)) return true;
				else c++;
			}
			return false;
		}

		void
		passUnmatch(std::string const& sub, std::string::const_iterator& c, std::string::const_iterator const end){
			while( c != end){
				if (strMatch(sub, c, end)) break;
				c++;
			}
		}

		bool
		passComment(std::string::const_iterator& c, std::string::const_iterator const end){
			if (strMatch("//", c, end)) {passUnmatch("\n", c, end); return true;}
			if (strMatch("/*", c, end)) {passUnmatch("*/", c, end); return true;}
			return false;
		}

		bool
		passWhitespace(std::string::const_iterator& c, std::string::const_iterator const end){
			passComment(c, end);
			while( c != end){
				if (passWhiteChar(c, end)) {
					if(!passComment(c, end)) return true;
				}
			}
			return false;
		}

		char
		parseEscapeSequence(std::string::const_iterator& c, std::string::const_iterator const end){
			char res;
			std::size_t new_n = 0;
			auto shexToChar = [&c, &new_n, end](){return (char) std::stoi(std::string(c, (c+4<end)?c+4:end), &new_n, 16);};

			switch (*c) {
				case '\"':	{res = '\"'; c++; break;}
				case '\\':	{res = '\\'; c++; break;}
				case '/':	{res = '/' ; c++; break;}
				case 'b':	{res = '\b'; c++; break;}
				case 'f':	{res = '\f'; c++; break;}
				case 'n':	{res = '\n'; c++; break;}
				case 'r':	{res = '\r'; c++; break;}
				case 't':	{res = '\t'; c++; break;}
				case 'u':	{c++; res = shexToChar(); c += new_n; break;}
				default:	{throw exception::ParseError("expected valid escape sequence");}
			}

			return res;
		}

		bool
		gettext(std::string const& filepath, std::string& str){
			std::ifstream		file(filepath, std::ifstream::in);
			std::stringstream	text;
			std::string			line;
			std::string			errmsg;

			if (!file.is_open())
				return false;
			while (std::getline(file, line))
				text << line << '\n';
			file.close();

			str = text.str();
			return true;
		}

		std::string
		json_tToStr(size_t t){
			std::vector<std::string> s {"Null", "Bool", "Number", "String", "Array", "Object"};
			return (t!=std::variant_npos? "json::" + s.at(t): "__empty_notype__");
		}
	}

	//jsonWrapper methods
	void inline
	jsonWrapper::parse(std::string::const_iterator& c, std::string::const_iterator const end){
		data = std::make_shared<Value>(parse::parseValue(c, end));
	}

	void
	jsonWrapper::parseFile(std::string const& filepath){
		data = std::make_shared<Value>(parse::parseFile(filepath));
	}

	template<typename T>
	inline T&
	jsonWrapper::get() const{
		return std::get<T>(*data);
	}

	inline json_t
	jsonWrapper::type() const{
		return json_t(data->index());
	}

	jsonWrapper
	jsonWrapper::at(size_t const n) const{
		throwIfItsNotAnArray();
		return (*(get<Array>().at(n)));
	}

	jsonWrapper
	jsonWrapper::at(String const& k) const{
		throwIfItsNotAnObject();
		return (*(get<Object>().at(k)));
	}

	jsonWrapper&
	jsonWrapper::operator [](size_t const n){
		throwIfItsNotAnArray();
		return (*(get<Array>()[n]));
	}

	jsonWrapper&
	jsonWrapper::operator [](String const& k){
		if(type() == Null_t && null_to_obj) {set(Object());}
		else throwIfItsNotAnObject();
		if (!find(k))
			get<Object>().emplace(k, std::make_shared<jsonWrapper>());
		return (*(get<Object>()[k]));
	}

	bool
	jsonWrapper::find(std::string const& k) const{
		throwIfItsNotAnObject();
		return (get<Object>().find(k) != get<Object>().end());
	}
	
	inline void
	jsonWrapper::set(Value const& v){
		data = std::make_shared<Value>(v);
	}

	inline void
	jsonWrapper::operator =(jsonWrapper const& j){
		data = j.data;
	}

	inline void
	jsonWrapper::operator =(Value const v){
		data = std::make_shared<Value>(v);
	}

	void
	jsonWrapper::emplace(size_t const n, json::Value const v){
		throwIfItsNotAnArray();
		get<Array>().emplace(get<Array>().begin()+n, std::make_shared<jsonWrapper>(std::make_shared<Value>(v)));
	}

	void
	jsonWrapper::emplace(std::string const& k, json::Value const v){
		throwIfItsNotAnObject();
		get<Object>().emplace(k, std::make_shared<jsonWrapper>(std::make_shared<Value>(v)));
	}

	void
	jsonWrapper::emplaceFile(size_t const n, std::string const& filepath){
		throwIfItsNotAnArray();
		emplace(n, parse::parseFile(filepath));
	}

	void
	jsonWrapper::emplaceFile(std::string const& k, std::string const& filepath){
		throwIfItsNotAnObject();
		emplace(k, parse::parseFile(filepath));
	}

	void
	jsonWrapper::erase(size_t n){
		throwIfItsNotAnArray();
		get<Array>().erase(get<Array>().begin()+n);
	}

	void
	jsonWrapper::erase(std::string const& k){
		throwIfItsNotAnObject();
		get<Object>().erase(k);
	}

	inline void
	jsonWrapper::throwIfItsNotAnObject() const{
		if(type() != Object_t)
			throw exception::AccessError("not an Object: type()=="+tools::json_tToStr(data->index())+'\n');
	}

	inline void
	jsonWrapper::throwIfItsNotAnArray() const{
		if(type() != Array_t)
			throw exception::AccessError("not an Array: type()=="+tools::json_tToStr(data->index())+'\n');
	}

	//parse functions
	namespace parse{
		Null
		parseNull(std::string::const_iterator& c, std::string::const_iterator const end){
			if (tools::strMatch("null", c, end))
				return Null{};
			else
				throw exception::ParseError("expected Null type: 'null'");
		}

		Bool
		parseBool(std::string::const_iterator& c, std::string::const_iterator const end){
			if (tools::strMatch("false", c, end))
				return Bool{false};
			else if (tools::strMatch("true", c, end))
				return Bool{true};
			else
				throw exception::ParseError("expected Bool type: 'true' or 'false'");
		}

		Number
		parseNumber(std::string::const_iterator& c, std::string::const_iterator const end){
			std::size_t new_n = 0;
			Number res = 0;
			try{
				res = std::stod(std::string(c, end), &new_n);
				c += new_n;
			}
			catch(std::invalid_argument()){
				throw exception::ParseError("expected Number type");
			}

			return res;
		}

		String
		parseString(std::string::const_iterator& c, std::string::const_iterator const end){
			std::string res;
			bool str_ended  = false;

			while( c != end){
				if(!tools::passWhitespace(c, end)) {throw exception::ParseError("expected '\"'");}
				if (*c=='\"'){
					bool str_piece_ended  = false;
					c++;
					while( c != end && !str_piece_ended ){
						switch (*c) {
							case '\\':	{c++; res += tools::parseEscapeSequence(c, end); break;}
							case '\"':	{str_piece_ended = true; c++; break;}
							case '\n':	{throw exception::ParseError("expected '\"'");}
							default:	{res += *c; c++; break;}
						}
					}
					if (!str_piece_ended)
						throw exception::ParseError("expected '\"'");
				}
				else {str_ended = true; break;}
			}

			return res;
		}

		Array
		parseArray(std::string::const_iterator& c, std::string::const_iterator const end){
			Array res;
			bool arr_ended    = false;
			bool expect_value = true;
			bool got_value    = false;
			if (*c=='[') c++;

			while( c != end && !arr_ended ){
				if(!tools::passWhitespace(c, end)) { throw exception::ParseError("expected json::Array");}
				switch (*c){
					case ',':{
						if (!got_value)
							throw exception::ParseError("expected value");
						got_value       = false;
						expect_value    = true;
						c++;
						break;
					}
					case ']':{
						if (!got_value && !res.size())
							throw exception::ParseError("expected Value");
						arr_ended       = true;
						c++;
						break;
					}
					default:{
						if (!expect_value && got_value)
							throw exception::ParseError("expected ',' or ']'");
						got_value       = true;
						expect_value    = false;
						json_ptr new_wrapper = std::make_shared<jsonWrapper>(std::make_shared<Value>(parseValue(c, end)));
						res.push_back(new_wrapper);
						break;
					}
				}
			}

			if (!arr_ended)
				throw exception::ParseError("expected ']'");
			return res;
		}

		Object
		parseObject(std::string::const_iterator& c, std::string::const_iterator const end){
			Object res;
			bool obj_ended    = false;
			bool expect_value = true;
			bool got_value    = false;

			if (*c=='{') c++;
			
			while( c != end && !obj_ended ){
				if(!tools::passWhitespace(c, end)) {throw exception::ParseError("expected json::Object");}
				switch (*c){
					case ',': {
						if (!got_value)
							throw exception::ParseError("expected value");
						got_value		= false;
						expect_value	= true;
						c++;
						break;
					}
					case '}': {
						if (!got_value && !res.size())
							throw exception::ParseError("expected Value");
						obj_ended		= true;
						c++;
						break;
					}
					default:  {
						if (!expect_value && got_value)
							throw exception::ParseError("expected ',' or '}'");
						got_value		= true;
						expect_value	= false;
						std::string name = parseString(c, end);
						if(!tools::passWhitespace(c, end)) {throw exception::ParseError("expected :");}
						if (*c!=':' || c==end)
							throw exception::ParseError("expected ':'");
						c++;
						res[name] = std::make_shared<jsonWrapper>(std::make_shared<Value>(parseValue(c, end)));
						break;
					}
				}
			}

			if (!obj_ended)
				throw exception::ParseError("expected '}'");
			return res;
		}

		Value
		parseValue(std::string::const_iterator& c, std::string::const_iterator const end){
			if (c == end) return {};

			while(c != end) {
				if(!tools::passWhitespace(c, end)) {throw exception::ParseError("expected value");}
				else if (*c == '-' || std::isdigit(*c)) return parseNumber(c, end);
				else{
					switch (*c){
						case '{': 	return parseObject(c, end);
						case '\"':	return parseString(c, end);
						case '[':	return parseArray(c, end);
						case 'f':	return parseBool(c, end);
						case 't':	return parseBool(c, end);
						case 'n':	return parseNull(c, end);
						default:	{throw exception::ParseError("invalid input");}
					}
				}
			}
			return {};
		}

		Value
		parseFile(std::string const& filepath){
			std::string text;
			if(!tools::gettext(filepath, text))
				throw exception::ParseError('"'+filepath+"\" could not be opened.");
			std::string::const_iterator c = text.begin();
			return parseValue(c, text.end());
		}
	}
}

#endif