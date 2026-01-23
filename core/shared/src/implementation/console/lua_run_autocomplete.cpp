// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :console.commands;

// We have to access some members of luabind::detail::class_rep which are inaccessable,
// so we'll force them to be accessable.
class Luaclass_rep {
  public:
	luabind::type_id m_type;
	std::vector<luabind::detail::class_rep::base_info> m_bases;
	const char *m_name;
	luabind::detail::lua_reference m_self_ref;
	luabind::handle m_table;
	luabind::handle m_default_table;
	luabind::detail::class_rep::class_type m_class_type;
	int m_instance_metatable;
	std::map<const char *, int, luabind::detail::ltstr> m_static_constants;
	int m_operator_cache;
	luabind::detail::cast_graph *m_casts;
	luabind::detail::class_id_map *m_classes;
};
static_assert(sizeof(Luaclass_rep) == sizeof(luabind::detail::class_rep));
static Luaclass_rep &access_class_rep(luabind::detail::class_rep &rep) { return reinterpret_cast<Luaclass_rep &>(rep); }

static void check_autocomplete(const std::string &arg, std::vector<std::string> &autoCompleteOptions, const std::string_view &candidate)
{
	auto caseSensitive = true;
	if(!pragma::string::compare(candidate.data(), arg.c_str(), caseSensitive, arg.length()))
		return;
	autoCompleteOptions.push_back(std::string {candidate});
}

static void get_crep_autocomplete(lua::State *l, luabind::detail::class_rep *crep, const std::string &arg, std::vector<std::string> &autoCompleteOptions)
{
	crep->get_table(l);
	luabind::object table(luabind::from_stack(l, -1));
	Lua::Pop(l, 1);

	for(luabind::iterator i(table), e; i != e; ++i) {
		if(static_cast<Lua::Type>(luabind::type(*i)) != Lua::Type::Function)
			continue;

		// We have to create a temporary `object` here, otherwise the proxy
		// returned by operator->() will mess up the stack. This is a known
		// problem that probably doesn't show up in real code very often.
		luabind::object member(*i);
		member.push(l);
		luabind::detail::stack_pop pop(l, 1);

		auto attr = i.key();
		attr.push(l);
		if(Lua::IsString(l, -1)) {
			std::string name = Lua::CheckString(l, -1);
			// TODO: Find argument names using LuaDoc system
			check_autocomplete(arg, autoCompleteOptions, name + "(");
			Lua::Pop(l, 1);
			//ParseLuaProperty(name, *i, result, true);
		}
	}

	auto &staticConstants = access_class_rep(*crep).m_static_constants;
	for(auto &[name, ltstr] : staticConstants) {
		check_autocomplete(arg, autoCompleteOptions, name);
	}
}

// simple trim (whitespace only)
static std::string trim(const std::string &s)
{
	auto l = s.find_first_not_of(" \t\n\r");
	if(l == std::string::npos)
		return "";
	auto r = s.find_last_not_of(" \t\n\r");
	return s.substr(l, r - l + 1);
}

static std::vector<std::pair<std::string, size_t>> split_chain(const std::string &expr)
{
	auto eq = expr.find_last_of("=");
	if(eq != std::string::npos) {
		auto res = split_chain(expr.substr(eq + 1));
		for(auto &[str, startPos] : res)
			startPos += eq + 1;
		return res;
	}
	std::vector<std::pair<std::string, size_t>> tokens;
	std::string cur;
	int depth = 0;
	size_t segment_start = 0; // index in expr where `cur` began

	for(size_t i = 0; i < expr.size(); ++i) {
		char c = expr[i];

		if(depth == 0 && cur.empty()) {
			// mark the start of a new segment
			segment_start = i;
		}

		// track paren depth
		if(c == '(') {
			depth++;
			cur += c;
		}
		else if(c == ')') {
			depth--;
			cur += c;
		}
		// split on . or : only at top level
		else if((c == '.' || c == ':') && depth == 0) {
			auto t = trim(cur);
			// strip trailing "()"
			if(t.size() > 2 && t.substr(t.size() - 2) == "()")
				t.erase(t.size() - 2);
			tokens.emplace_back(t, segment_start);
			cur.clear();
		}
		else {
			cur += c;
		}
	}

	// handle the final segment
	if(!cur.empty()) {
		auto t = trim(cur);
		if(t.size() > 2 && t.substr(t.size() - 2) == "()")
			t.erase(t.size() - 2);
		tokens.emplace_back(t, segment_start);
	}

	if(!expr.empty() && (expr.back() == '.' || expr.back() == ':'))
		tokens.push_back({"", expr.length()});

	return tokens;
}

void pragma::console::commands::lua_run_autocomplete(lua::State *l, const std::string &arg, std::vector<std::string> &autoCompleteOptions)
{
	auto chain = split_chain(arg);
	if(chain.empty())
		return;
	auto lastSegment = chain.back();
	chain.erase(chain.end() - 1);
	luabind::object o {luabind::globals(l)};
	for(auto &[el, startIdx] : chain) {
		o = o[el];
		if(!o)
			break;
		auto type = static_cast<Lua::Type>(luabind::type(o));
		if(type == Lua::Type::Function) {
			luabind::detail::function_object *fobj = scripting::lua_core::util::get_function_object(o);
			if(fobj) {
				auto *functionName = fobj->name.c_str();
				for(luabind::detail::function_object const *f = fobj; f != 0; f = f->next) {
					std::vector<luabind::detail::TypeInfo> types;
					f->get_signature_info(l, functionName, types);
					if(types.empty())
						continue;
					auto &type = types.front();
					//type.crep->
				}
				//for(luabind::detail::function_object const *f = fobj; f != 0; f = f->next) {
				//	std::vector<luabind::detail::TypeInfo> types;
				//	f->get_signature_info(L, function_name, types);
			}
		}
	}

	/*if (o) {
		o.push(l);
		print_methods_of_top(l);
		Lua::Pop(l);
	}*/

	if(!o)
		;
	else {
		auto *crep = Lua::get_crep(o);
		if(crep) {
			get_crep_autocomplete(l, crep, lastSegment.first, autoCompleteOptions);
		}
		else {
			auto t = static_cast<Lua::Type>(luabind::type(o));
			if(t == Lua::Type::Table) {
				for(luabind::iterator it {o}, end; it != end; ++it) {
					auto strKey = luabind::object_cast_nothrow<std::string>(it.key(), std::string {});
					if(strKey.empty())
						continue;
					check_autocomplete(lastSegment.first, autoCompleteOptions, strKey);
					//if (pragma::string::compare(strKey.c_str(), lastSegment.first.c_str(), false, lastSegment.first.length())) {
					//	autoCompleteOptions.push_back(arg.substr(0, lastSegment.second) +strKey);
					//}
				}
			}
		}
		for(auto &opt : autoCompleteOptions) {
			auto pos = opt.find(lastSegment.first);
			if(pos != std::string::npos)
				opt = opt.substr(pos);
		}
		//auto o = tmp;
		//dump_methods(l,o);
		/*
		for(luabind::iterator it {o}, end; it != end; ++it) {
			auto strKey = luabind::object_cast_nothrow<std::string>(it.key(), std::string {});
			if (strKey.empty())
				continue;
			if (pragma::string::compare(strKey.c_str(), lastSegment.first.c_str(), false, lastSegment.first.length())) {
				autoCompleteOptions.push_back(arg.substr(0, lastSegment.second) +strKey);
			}
		}
		*/

		/*auto type = luabind::type(o);
		if (type == LUA_TUSERDATA) {
			auto *crep = Lua::get_crep(o);
			if(crep) {
				//crep->classes()
			}
		}*/
	}

	for(auto &opt : autoCompleteOptions)
		opt = arg.substr(0, lastSegment.second) + opt;
}
