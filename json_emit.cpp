#ifndef JSON_EMIT_CPP
#define JSON_EMIT_CPP

#include "json.cpp"

//WIP? it works, i supose.

namespace emit{
	std::string
	emitNull();
	std::string
	emitBool(json::Bool const& v);
	std::string
	emitNumber(json::Number const& v);
	std::string
	emitString(json::String const& v);
	std::string
	emitArray(json::Array const& v, std::size_t t);
	std::string
	emitObject(json::Object const& v, std::size_t t);
	std::string
	emitValue(json::Value const& v, std::size_t t);
	std::string
	emitJson(json::jsonWrapper const& j, std::size_t t);
}

namespace emit{
	std::size_t
		tab_size	= 2;
	bool
		tab_obj		= true,
		nl_obj		= true,
		tab_arr		= false,
		nl_arr		= false,
		crlf_endl	= false,
		tab_w_space	= true;


	inline
	std::string
	emitTab(std::size_t ntabs=0, bool b = true){
		return (b)? (tab_w_space)?std::string(((tab_size==-1)? 0:tab_size)*ntabs, ' '):std::string(ntabs,'\t') : "";
	}

	inline
	std::string
	emitLineEnd(bool b = true){
		return (b)? (crlf_endl)?"\r\n":"\n" : "";
	}

	std::string
	emitEscapeSequence(std::string::const_iterator& c, std::string::const_iterator const end){
		std::string res;

		while(c != end){
			switch (*c)
			{
			case '\"':	{res += "\\\""; c++; break;}
			case '\\':	{res += "\\\\"; c++; break;}
			case '/':	{res += "\\/" ; c++; break;}
			case '\b':	{res += "\\b"; c++; break;}
			case '\f':	{res += "\\f"; c++; break;}
			case '\n':	{res += "\\n"; c++; break;}
			case '\r':	{res += "\\r"; c++; break;}
			case '\t':	{res += "\\t"; c++; break;}
			default:	{res += *c; c++;}
			}
		}

		return res;
	}
	
	std::string
	emitNull(){
		return "null";
	}

	std::string
	emitBool(json::Bool const& v){
		return (v?"true": "false");
	}

	std::string
	emitNumber(json::Number const& v){
		return std::to_string(v);
	}

	std::string
	emitString(json::String const& v){
		std::string::const_iterator c = v.begin();
		return '"' + emitEscapeSequence(c, v.end()) + '"';
	}

	std::string
	emitArray(json::Array const& v, std::size_t t=0){
		std::string res;
		res += "[ " + emitLineEnd(nl_arr);
		for(size_t i=0; i<v.size(); i++){
			res += emitTab(t+1, tab_arr)
				+ emitJson(*v[i], t+1);
			if(i<v.size()-1) res += ", " + emitLineEnd(nl_arr);
		}
		res += emitLineEnd(nl_arr) + emitTab(t, tab_arr) + "]";
		return res;
	}

	std::string
	emitObject(json::Object const& v, std::size_t t=0){
	  std::string res;
	  res += "{ " + emitLineEnd(nl_obj);
	  size_t i=0;
	  for(auto const& [k, _j]: v){
		res += emitTab(t+1, tab_obj)
			+ emitString(k) + ": "
			+ emitJson(*_j, t+1);
		if(i < v.size()-1) res += ", " + emitLineEnd(nl_obj);
		i++;
	  }
	  res += emitLineEnd(nl_obj) + emitTab(t, tab_obj) + "}";
	  return res;
	}

	std::string
	emitValue(json::Value const& v, std::size_t t=0){
	  switch (v.index())
	  {
	  case 0: {return emitNull();}
	  case 1: {return emitBool(std::get<json::Bool>(v));}
	  case 2: {return emitNumber(std::get<json::Number>(v));}
	  case 3: {return emitString(std::get<json::String>(v));}
	  case 4: {return emitArray(std::get<json::Array>(v), t);}
	  case 5: {return emitObject(std::get<json::Object>(v), t);}
	  default: {throw std::bad_variant_access();}
	  }
	}

	std::string
	emitJson(json::jsonWrapper const& j, std::size_t t=0){
	  return emitValue(*(j.data), t);
	}
  }

#endif