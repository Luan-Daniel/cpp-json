#include "json.cpp"
#include "json_emit.cpp"

#include <iostream> //debug

int main(){
	//test
	json::jsonWrapper j("null");
	std::cout << json::tools::json_tToStr(j.type()) << std::endl;
	json::null_to_obj = true;
	j["a"];
	std::cout << json::tools::json_tToStr(j.type()) << std::endl;
	json::jsonWrapper j2(j);
	std::cout << json::tools::json_tToStr(j2.type()) << std::endl;
	return 0;
}