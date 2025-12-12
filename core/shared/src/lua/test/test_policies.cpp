// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#if 0


static void test_pair_policy_param(const std::pair<float,std::string> &pair)
{
	std::cout<<"Pair: ("<<pair.first<<","<<pair.second<<")"<<std::endl;
}

static std::pair<float,std::string> test_pair_policy_ret()
{
	return {3.f,"String return value"};
}

static void test_tuple_policy_param(const std::tuple<float,std::string,Vector3,bool> &pair)
{
	std::cout<<"Tuple: ("<<std::get<0>(pair)<<","<<std::get<1>(pair)<<","<<std::get<2>(pair)<<","<<std::get<3>(pair)<<std::endl;
}

static std::tuple<float,std::string,bool> test_tuple_policy_ret()
{
	return {1313.f,"TestString",true};
}

static void test_vector_policy_param(const std::vector<float> &vec)
{
	std::cout<<"Vector:"<<std::endl;
	for(auto &v : vec)
		std::cout<<v<<std::endl;
}

static std::vector<float> test_vector_policy_ret()
{
	return {1.f,8.f,5.f,164.f,666.f};
}

static void test_array_policy_param(const std::array<std::string,4> &vec)
{
	std::cout<<"Array:"<<std::endl;
	for(auto &v : vec)
		std::cout<<v<<std::endl;
}

static std::array<std::string,4> test_array_policy_ret()
{
	return {"a","b","d","c"};
}

static std::string_view test_string_view_policy_ret()
{
	static std::string s = "some_string";
	return s;
}

struct TestSharedFromThis
	: public std::enable_shared_from_this<TestSharedFromThis>
{
	float x = 5.f;
};

static std::shared_ptr<TestSharedFromThis> g_shared_from_this = pragma::util::make_shared<TestSharedFromThis>();
static TestSharedFromThis *test_shared_from_this_policy_ret()
{
	return g_shared_from_this.get();
}

static pragma::util::PFloatProperty test_property_policy_ret()
{
	return pragma::util::FloatProperty::Create(111.f);
}

static std::optional<std::string> test_optional_policy_ret(int i)
{
	if(i == 0)
		return {};
	return "String return value";
}

static void test_default_parameter_policy_param(const Vector3 &v={1,2,3},float f = 5.f)
{
	std::cout<<"Default string: "<<v<<std::endl;
	std::cout<<"Default float: "<<f<<std::endl;
}

__declspec(dllexport) void test_lua_policies(lua::State *l)
{
	auto modTest = luabind::module_(l,"test_policies");

	// Pair policy
	modTest[
		luabind::def("test_pair_policy_param",&test_pair_policy_param,luabind::pair_policy<1,float,std::string>{}),
		luabind::def("test_pair_policy_ret",&test_pair_policy_ret,luabind::pair_policy<0>{})
	];

	auto r = Lua::RunString(l,"test_policies.test_pair_policy_param({7.0,\"Hello\"})","test");
	Lua::HandleSyntaxError(l,r);

	r = Lua::RunString(l,"print(\"test_pair_policy_ret: \",test_policies.test_pair_policy_ret())","test");
	Lua::HandleSyntaxError(l,r);

	// Vector policy
	modTest[
		luabind::def("test_vector_policy_param",&test_vector_policy_param,luabind::vector_policy<1,float>{}),
		luabind::def("test_vector_policy_ret",&test_vector_policy_ret,luabind::vector_policy<0,float>{})
	];

	r = Lua::RunString(l,"test_policies.test_vector_policy_param({8.0,654.0,547,11})","test");
	Lua::HandleSyntaxError(l,r);

	r = Lua::RunString(l,"print(\"test_vector_policy_ret: \") console.print_table(test_policies.test_vector_policy_ret())","test");
	Lua::HandleSyntaxError(l,r);

	// Array policy
	modTest[
		luabind::def("test_array_policy_param",&test_array_policy_param,luabind::array_policy<1,std::string,4>{}),
		luabind::def("test_array_policy_ret",&test_array_policy_ret,luabind::array_policy<0,std::string,4>{})
	];

	r = Lua::RunString(l,"test_policies.test_array_policy_param({\"q\",\"w\",\"e\",\"r\"})","test");
	Lua::HandleSyntaxError(l,r);

	r = Lua::RunString(l,"print(\"test_array_policy_ret: \") console.print_table(test_policies.test_array_policy_ret())","test");
	Lua::HandleSyntaxError(l,r);

	// Should throw error (number of items mismatch)
	r = Lua::RunString(l,"test_policies.test_array_policy_param({\"q\",\"w\",\"e\",\"r\",\"t\"})","test");
	Lua::HandleSyntaxError(l,r);

	// string_view policy
	modTest[
		luabind::def("test_string_view_policy_ret",&test_string_view_policy_ret,luabind::string_view_policy<0>{})
	];

	r = Lua::RunString(l,"print(\"test_string_view_policy_ret: \",test_policies.test_string_view_policy_ret())","test");
	Lua::HandleSyntaxError(l,r);

	// shared_from_this policy

	auto defTestSharedFromThis = luabind::class_<TestSharedFromThis>("TestSharedFromThis");
	defTestSharedFromThis.def_readwrite("x",&TestSharedFromThis::x);
	modTest[defTestSharedFromThis];
	modTest[
		luabind::def("test_shared_from_this_policy_ret",&test_shared_from_this_policy_ret,luabind::shared_from_this_policy<0>{})
	];

	r = Lua::RunString(l,"local v = test_policies.test_shared_from_this_policy_ret() print(\"test_shared_from_this_policy_ret: \",(v ~= nil) and v.x or \"nil\")","test");
	Lua::HandleSyntaxError(l,r);

	// Should print 'nil'
	g_shared_from_this = nullptr;
	r = Lua::RunString(l,"local v = test_policies.test_shared_from_this_policy_ret() print(\"test_shared_from_this_policy_ret: \",(v ~= nil) and v.x or \"nil\")","test");
	Lua::HandleSyntaxError(l,r);

	// property policy
	modTest[
		luabind::def("test_property_policy_ret",&test_property_policy_ret,luabind::property_policy<0>{})
	];

	r = Lua::RunString(l,"print(\"test_property_policy_ret: \",test_policies.test_property_policy_ret():Get())","test");
	Lua::HandleSyntaxError(l,r);

	// Optional policy
	modTest[
		luabind::def("test_optional_policy_ret",&test_optional_policy_ret,luabind::optional_policy<0>{})
	];

	r = Lua::RunString(l,"print(\"test_optional_policy_ret: \",test_policies.test_optional_policy_ret(0),test_policies.test_optional_policy_ret(1))","test");
	Lua::HandleSyntaxError(l,r);

	// Tuple policy
	modTest[
		//luabind::def("test_tuple_policy_param",&test_tuple_policy_param,luabind::tuple_policy<1>{}),
		luabind::def("test_tuple_policy_ret",&test_tuple_policy_ret,luabind::tuple_policy<0>{})
	];

	//r = Lua::RunString(l,"test_policies.test_tuple_policy_param({555,\"XYZ\",Vector(5,4,5),true})","test");
	//Lua::HandleSyntaxError(l,r);

	r = Lua::RunString(l,"print(\"test_tuple_policy_ret: \",test_policies.test_tuple_policy_ret())","test");
	Lua::HandleSyntaxError(l,r);

	// default parameter policy
	modTest[
		luabind::def("test_default_parameter_policy_param",&test_default_parameter_policy_param),
		luabind::def("test_default_parameter_policy_param",&test_default_parameter_policy_param,luabind::meta::join<luabind::default_parameter_policy<1,Vector3{4,5,6}>,luabind::default_parameter_policy<2,3.f>>::type{})
	];

	r = Lua::RunString(l,"test_policies.test_default_parameter_policy_param(Vector(7,8,9),123) test_policies.test_default_parameter_policy_param()","test");
	Lua::HandleSyntaxError(l,r);

}
#endif
