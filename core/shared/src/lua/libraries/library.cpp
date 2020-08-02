/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_shared.h"
#include <pragma/game/game.h>
#include "pragma/lua/classes/lconvar.h"
#include "pragma/lua/libraries/lfile.h"
#include "pragma/lua/libraries/lboundingvolume.h"
#include "pragma/lua/libraries/lintersect.h"
#include "pragma/lua/libraries/lsweep.h"
#include "pragma/lua/libraries/lray.h"
#include "pragma/lua/libraries/ltimer.h"
#include "pragma/lua/libraries/lgeometry.h"
#include "pragma/lua/classes/lphysics.h"
#include "pragma/lua/libraries/lmatrix.h"
#include "pragma/lua/libraries/lasset.hpp"

#include "pragma/lua/classes/ldef_vector.h"
#include "luasystem.h"
#include <pragma/math/angle/wvquaternion.h>
#include "pragma/lua/classes/lvector.h"
#include "pragma/lua/libraries/lrecast.h"
#include "pragma/lua/libraries/lmath.h"
#include "pragma/lua/libraries/lnoise.h"
#include "pragma/lua/libraries/lregex.h"
#include "pragma/lua/libraries/lutil.h"
#include "pragma/lua/libraries/ldebug.h"
#include "pragma/lua/libraries/lmesh.h"
#include "pragma/lua/libraries/ltable.h"
#include "pragma/lua/classes/lcolor.h"
#include "pragma/lua/libraries/los.h"
#include "pragma/lua/libraries/ltime.hpp"
#include "pragma/lua/libraries/lprint.h"
#include "pragma/lua/classes/lerrorcode.h"
#include "pragma/lua/classes/lcallback.h"
#include "pragma/lua/classes/ldatastream.h"
#include "pragma/lua/libraries/limport.hpp"
#include "pragma/math/util_easing.hpp"
#include "pragma/lua/libraries/lnav.hpp"
#include "pragma/lua/lua_call.hpp"
#include "pragma/util/util_handled.hpp"
#include "pragma/util/util_rgbcsv.hpp"
#include "pragma/model/animation/animation.h"
#include "pragma/model/modelmesh.h"
#include "pragma/model/model.h"
#include <pragma/math/vector/util_winding_order.hpp>
#include <pragma/math/util_engine_math.hpp>
#include "pragma/game/game_coordinate_system.hpp"
#include "pragma/util/util_variable_type.hpp"
#include <sharedutils/util_file.h>
#include <pragma/math/intersection.h>
#include <mathutil/camera.hpp>
#include <regex>
#include <complex>
#include <mpParser.h>
#include <luainterface.hpp>
#include <luabind/out_value_policy.hpp>

extern DLLENGINE Engine *engine;

static std::ostream &operator<<(std::ostream &out,const CallbackHandle &hCallback)
{
	out<<"Callback[";
	if(hCallback.IsValid())
		out<<"true";
	else
		out<<"false";
	out<<"]";
	return out;
}

static std::ostream &operator<<(std::ostream &out,const util::HSV &hsv)
{
	out<<hsv.h<<' '<<hsv.s<<' '<<hsv.v;
	return out;
}

static void call_callback(CallbackHandle &cb,std::initializer_list<luabind::object> args)
{
	if(cb.IsValid() == false)
		return;
	cb.Call<void,std::function<Lua::StatusCode(lua_State*)>>([&args](lua_State *l) -> Lua::StatusCode {
		for(auto &arg : args)
			arg.push(l);
		return Lua::StatusCode::Ok;
	});
}

static int32_t parse_math_expression(lua_State *l)
{
	class FunGeneric : public mup::ICallback
	{
	public:

		FunGeneric(const std::string &name,uint32_t numArgs)
			: ICallback(mup::cmFUNC, name.c_str(), numArgs),m_name{name},m_numArgs{numArgs}
		{}

		FunGeneric(const FunGeneric &other)
			: mup::ICallback{other},m_name{other.m_name},
			m_numArgs{other.m_numArgs}
		{}

		virtual void Eval(mup::ptr_val_type &ret, const mup::ptr_val_type * /*a_pArg*/, int /*a_iArgc*/) override {}

		virtual const mup::char_type* GetDesc() const override
		{
			return _T("");
		}

		virtual IToken* Clone() const override {return new FunGeneric(*this);}
	private:
		std::string m_name;
		uint32_t m_numArgs = 0;
	};

	mup::ParserX p;
	p.ClearFun();

	std::string expression = Lua::CheckString(l,1);

	int32_t t = 2;
	Lua::CheckTable(l,t);
	auto n = Lua::GetObjectLength(l,t);
	for(auto i=decltype(n){0u};i<n;++i)
	{
		Lua::PushInt(l,i +1); /* 1 */
		Lua::GetTableValue(l,t); /* 1 */
		Lua::CheckTable(l,-1);

		auto tF = Lua::GetStackTop(l);

		Lua::PushInt(l,1); /* 2 */
		Lua::GetTableValue(l,tF); /* 2 */
		auto *fnName = Lua::CheckString(l,-1);
		Lua::Pop(l,1); /* 1 */

		Lua::PushInt(l,2); /* 2 */
		Lua::GetTableValue(l,tF); /* 2 */
		auto numArgs = Lua::CheckInt(l,-1);
		Lua::Pop(l,1); /* 1 */
		p.DefineFun(new FunGeneric{fnName,static_cast<uint32_t>(numArgs)});

		Lua::Pop(l,1); /* 0 */
	}

	t = 3;
	Lua::CheckTable(l,t);
	n = Lua::GetObjectLength(l,t);
	for(auto i=decltype(n){0u};i<n;++i)
	{
		Lua::PushInt(l,i +1); /* 1 */
		Lua::GetTableValue(l,t); /* 1 */
		auto *varName = Lua::CheckString(l,-1);

		p.DefineVar(varName,0);

		Lua::Pop(l,1); /* 0 */
	}

	p.SetExpr(expression);
	try
	{
		p.GetExprVar();
	}
	catch(const mup::ParserError &err)
	{
		Lua::PushBool(l,false);
		Lua::PushString(l,err.GetMsg());
		return 2;
	}
	auto &rpn = p.GetRPN();
	auto &data = rpn.GetData();
	t = Lua::CreateTable(l);
	auto numTokens = data.size();
	for(auto i=decltype(numTokens){0u};i<numTokens;++i)
	{
		Lua::PushInt(l,i +1); /* 1 */

		auto &pTok = *data[i].Get();
		auto tToken = Lua::CreateTable(l); /* 2 */

		Lua::PushString(l,"identifier"); /* 3 */
		Lua::PushString(l,pTok.GetIdent()); /* 4 */
		Lua::SetTableValue(l,tToken); /* 2 */

		Lua::PushString(l,"code"); /* 3 */
		Lua::PushInt(l,umath::to_integral(pTok.GetCode())); /* 4 */
		Lua::SetTableValue(l,tToken); /* 2 */

		Lua::SetTableValue(l,t); /* 0 */
	}
	return 1;
}

void NetworkState::RegisterSharedLuaLibraries(Lua::Interface &lua)
{
	// Remove sensitive functions and libraries
	lua_pushnil(lua.GetState());
	lua_setglobal(lua.GetState(),"dofile");

	lua_pushnil(lua.GetState());
	lua_setglobal(lua.GetState(),"loadfile");

	std::array<std::string,7> fRemoveOs = {
		"execute",
		"rename",
		"setlocale",
		"getenv",
		"remove",
		"exit",
		"tmpname"
	};
	Lua::GetGlobal(lua.GetState(),"os"); /* 1 */
	auto tOs = Lua::GetStackTop(lua.GetState());
	for(auto &name : fRemoveOs)
	{
		Lua::PushString(lua.GetState(),name); /* 2 */
		Lua::PushNil(lua.GetState()); /* 3 */
		Lua::SetTableValue(lua.GetState(),tOs); /* 1 */
	}
	Lua::Pop(lua.GetState(),1); /* 0 */

	//lua_pushnil(lua.GetState());
	//lua_setglobal(lua.GetState(),"os");
	//

	luabind::globals(lua.GetState())["print"] = luabind::nil;

	auto modVec = luabind::module_(lua.GetState(),"vector");
	modVec[
		luabind::def("to_min_max",static_cast<void(*)(::Vector2&,::Vector2&)>(Lua::vector::to_min_max),luabind::meta::join<luabind::out_value<1>,luabind::out_value<2>>::type{}),
		luabind::def("to_min_max",static_cast<void(*)(::Vector3&,::Vector3&)>(Lua::vector::to_min_max),luabind::meta::join<luabind::out_value<1>,luabind::out_value<2>>::type{}),
		luabind::def("to_min_max",static_cast<void(*)(::Vector4&,::Vector4&)>(Lua::vector::to_min_max),luabind::meta::join<luabind::out_value<1>,luabind::out_value<2>>::type{}),
		luabind::def("to_min_max",static_cast<void(*)(::Vector2&,::Vector2&,const Vector2&)>(Lua::vector::to_min_max),luabind::meta::join<luabind::out_value<1>,luabind::out_value<2>>::type{}),
		luabind::def("to_min_max",static_cast<void(*)(::Vector3&,::Vector3&,const Vector3&)>(Lua::vector::to_min_max),luabind::meta::join<luabind::out_value<1>,luabind::out_value<2>>::type{}),
		luabind::def("to_min_max",static_cast<void(*)(::Vector4&,::Vector4&,const Vector4&)>(Lua::vector::to_min_max),luabind::meta::join<luabind::out_value<1>,luabind::out_value<2>>::type{}),
		luabind::def("get_min_max",static_cast<void(*)(lua_State*,luabind::table<>,::Vector2&,::Vector2&)>(Lua::vector::get_min_max),luabind::meta::join<luabind::pure_out_value<3>,luabind::pure_out_value<4>>::type{}),
		luabind::def("get_min_max",static_cast<void(*)(lua_State*,luabind::table<>,::Vector3&,::Vector3&)>(Lua::vector::get_min_max),luabind::meta::join<luabind::pure_out_value<3>,luabind::pure_out_value<4>>::type{}),
		luabind::def("get_min_max",static_cast<void(*)(lua_State*,luabind::table<>,::Vector4&,::Vector4&)>(Lua::vector::get_min_max),luabind::meta::join<luabind::pure_out_value<3>,luabind::pure_out_value<4>>::type{}),
		luabind::def("random",uvec::create_random_unit_vector),
		luabind::def("random2D",Lua::vector::random_2d),
		luabind::def("create_from_string",static_cast<Vector3(*)(const std::string&)>(uvec::create)),
		luabind::def("calc_average",Lua::vector::calc_average),
		luabind::def("calc_best_fitting_plane",Lua::vector::calc_best_fitting_plane,luabind::meta::join<luabind::pure_out_value<3>,luabind::pure_out_value<4>>::type{}),
		luabind::def("calc_linear_velocity_from_angular",util::angular_velocity_to_linear)
	];

	Lua::RegisterLibraryValue<Vector3>(lua.GetState(),"vector","ORIGIN",uvec::ORIGIN);
	Lua::RegisterLibraryValue<Vector3>(lua.GetState(),"vector","FORWARD",uvec::FORWARD);
	Lua::RegisterLibraryValue<Vector3>(lua.GetState(),"vector","RIGHT",uvec::RIGHT);
	Lua::RegisterLibraryValue<Vector3>(lua.GetState(),"vector","UP",uvec::UP);
	Lua::RegisterLibraryValue<Vector3>(lua.GetState(),"vector","MIN",uvec::MIN);
	Lua::RegisterLibraryValue<Vector3>(lua.GetState(),"vector","MAX",uvec::MAX);

	Lua::RegisterLibrary(lua.GetState(),"angle",{
		{"random",Lua::global::angle_rand},
		{"create_from_string",Lua::global::create_from_string}
	});

	Lua::RegisterLibrary(lua.GetState(),"noise",{
		{"perlin",Lua_noise_perlin},
		{"const",Lua_noise_const},
		{"voronoi",Lua_noise_voronoi},
		{"generate_height_map",Lua_noise_generate_height_map}
	});

	Lua::RegisterLibraryEnums(lua.GetState(),"noise",{
		{"QUALITY_FAST",umath::to_integral(noise::NoiseQuality::QUALITY_FAST)},
		{"QUALITY_STD",umath::to_integral(noise::NoiseQuality::QUALITY_STD)},
		{"QUALITY_BEST",umath::to_integral(noise::NoiseQuality::QUALITY_BEST)}
	});

	//lua_pushtablecfunction(lua.GetState(),"table","has_value",Lua::table::has_value); // Function is incomplete
	lua_pushtablecfunction(lua.GetState(),"table","random",Lua::table::random);
	lua_pushtablecfunction(lua.GetState(),"table","merge",static_cast<int32_t(*)(lua_State*)>([](lua_State *l) -> int32_t {
		auto t0 = 1;
		auto t1 = 2;
		Lua::CheckTable(l,t0);
		Lua::CheckTable(l,t1);
		auto bMergeByKey = false;
		if(Lua::IsSet(l,3))
			bMergeByKey = Lua::CheckBool(l,3);
		if(bMergeByKey == false)
		{
			auto l0 = Lua::GetObjectLength(l,t0);
			auto l1 = Lua::GetObjectLength(l,t1);
			for(auto i=decltype(l1){1};i<=l1;++i)
			{
				Lua::PushInt(l,++l0);

				Lua::PushInt(l,i);
				Lua::GetTableValue(l,t1);

				Lua::SetTableValue(l,t0);
			}
			Lua::PushValue(l,1);
			return 1;
		}

		Lua::PushNil(l);
		while(Lua::GetNextPair(l,t1) != 0)
		{
			Lua::PushValue(l,-2);
			Lua::PushValue(l,-2);
			Lua::SetTableValue(l,t0);

			Lua::Pop(l,1); // We need the key at the top for the next iteration
		}
		Lua::PushValue(l,1);
		return 1;
	}));

	lua_pushtablecfunction(lua.GetState(),"math","randomf",Lua::math::randomf);
	lua_pushtablecfunction(lua.GetState(),"math","approach",Lua::math::approach);
	lua_pushtablecfunction(lua.GetState(),"math","get_angle_difference",Lua::math::get_angle_difference);
	lua_pushtablecfunction(lua.GetState(),"math","approach_angle",Lua::math::approach_angle);
	lua_pushtablecfunction(lua.GetState(),"math","normalize_angle",Lua::math::normalize_angle);
	lua_pushtablecfunction(lua.GetState(),"math","clamp_angle",Lua::math::clamp_angle);
	lua_pushtablecfunction(lua.GetState(),"math","is_angle_in_range",Lua::math::is_angle_in_range);
	lua_pushtablecfunction(lua.GetState(),"math","perlin_noise",Lua::math::perlin_noise);
	lua_pushtablecfunction(lua.GetState(),"math","sign",Lua::math::sign);
	lua_pushtablecfunction(lua.GetState(),"math","clamp",Lua::math::clamp);
	lua_pushtablecfunction(lua.GetState(),"math","lerp",Lua::math::lerp);
	lua_pushtablecfunction(lua.GetState(),"math","get_next_power_of_2",Lua::math::get_next_power_of_2);
	lua_pushtablecfunction(lua.GetState(),"math","get_previous_power_of_2",Lua::math::get_previous_power_of_2);
	lua_pushtablecfunction(lua.GetState(),"math","round",Lua::math::round);
	lua_pushtablecfunction(lua.GetState(),"math","snap_to_grid",Lua::math::snap_to_grid);
	lua_pushtablecfunction(lua.GetState(),"math","calc_hermite_spline",Lua::math::calc_hermite_spline);
	lua_pushtablecfunction(lua.GetState(),"math","calc_hermite_spline_position",Lua::math::calc_hermite_spline_position);
	lua_pushtablecfunction(lua.GetState(),"math","smooth_step",Lua::math::smooth_step);
	lua_pushtablecfunction(lua.GetState(),"math","smoother_step",Lua::math::smoother_step);
	lua_pushtablecfunction(lua.GetState(),"math","is_in_range",Lua::math::is_in_range);
	lua_pushtablecfunction(lua.GetState(),"math","normalize_uv_coordinates",Lua::math::normalize_uv_coordinates);

	lua_pushtablecfunction(lua.GetState(),"math","solve_quadric",Lua::math::solve_quadric);
	lua_pushtablecfunction(lua.GetState(),"math","solve_cubic",Lua::math::solve_cubic);
	lua_pushtablecfunction(lua.GetState(),"math","solve_quartic",Lua::math::solve_quartic);
	lua_pushtablecfunction(lua.GetState(),"math","calc_ballistic_range",Lua::math::calc_ballistic_range);
	lua_pushtablecfunction(lua.GetState(),"math","calc_ballistic_position",Lua::math::calc_ballistic_position);
	lua_pushtablecfunction(lua.GetState(),"math","calc_ballistic_velocity",Lua::math::calc_ballistic_velocity);
	lua_pushtablecfunction(lua.GetState(),"math","calc_ballistic_time_of_flight",Lua::math::calc_ballistic_time_of_flight);
	lua_pushtablecfunction(lua.GetState(),"math","calc_ballistic_angle_of_reach",Lua::math::calc_ballistic_angle_of_reach);
	lua_pushtablecfunction(lua.GetState(),"math","solve_ballistic_arc",Lua::math::solve_ballistic_arc);
	lua_pushtablecfunction(lua.GetState(),"math","solve_ballistic_arc_lateral",Lua::math::solve_ballistic_arc_lateral);

	lua_pushtablecfunction(lua.GetState(),"math","calc_horizontal_fov",Lua::math::calc_horizontal_fov);
	lua_pushtablecfunction(lua.GetState(),"math","calc_vertical_fov",Lua::math::calc_vertical_fov);
	lua_pushtablecfunction(lua.GetState(),"math","calc_diagonal_fov",Lua::math::calc_diagonal_fov);

	lua_pushtablecfunction(lua.GetState(),"math","horizontal_fov_to_vertical_fov",Lua::math::horizontal_fov_to_vertical_fov);
	lua_pushtablecfunction(lua.GetState(),"math","vertical_fov_to_horizontal_fov",Lua::math::vertical_fov_to_horizontal_fov);
	lua_pushtablecfunction(lua.GetState(),"math","diagonal_fov_to_vertical_fov",Lua::math::diagonal_fov_to_vertical_fov);

	lua_pushtablecfunction(lua.GetState(),"math","get_frustum_plane_center",Lua::math::get_frustum_plane_center);
	lua_pushtablecfunction(lua.GetState(),"math","get_frustum_plane_size",Lua::math::get_frustum_plane_size);
	lua_pushtablecfunction(lua.GetState(),"math","get_frustum_plane_boundaries",Lua::math::get_frustum_plane_boundaries);
	lua_pushtablecfunction(lua.GetState(),"math","get_frustum_plane_point",Lua::math::get_frustum_plane_point);

	lua_pushtablecfunction(lua.GetState(),"math","max_abs",Lua::math::abs_max);
	lua_pushtablecfunction(lua.GetState(),"math","ease_in",Lua::math::ease_in);
	lua_pushtablecfunction(lua.GetState(),"math","ease_out",Lua::math::ease_out);
	lua_pushtablecfunction(lua.GetState(),"math","ease_in_out",Lua::math::ease_in_out);
	lua_pushtablecfunction(lua.GetState(),"math","is_nan",static_cast<int32_t(*)(lua_State*)>([](lua_State *l) -> int32_t {
		auto f = Lua::CheckNumber(l,1);
		Lua::PushBool(l,std::isnan(f));
		return 1;
	}));
	lua_pushtablecfunction(lua.GetState(),"math","is_inf",static_cast<int32_t(*)(lua_State*)>([](lua_State *l) -> int32_t {
		auto f = Lua::CheckNumber(l,1);
		Lua::PushBool(l,std::isinf(f));
		return 1;
	}));
	lua_pushtablecfunction(lua.GetState(),"math","is_finite",static_cast<int32_t(*)(lua_State*)>([](lua_State *l) -> int32_t {
		auto f = Lua::CheckNumber(l,1);
		Lua::PushBool(l,std::isfinite(f));
		return 1;
	}));
	lua_pushtablecfunction(lua.GetState(),"math","calc_fov_from_lens",static_cast<int32_t(*)(lua_State*)>([](lua_State *l) -> int32_t {
		auto sensorSize = Lua::CheckNumber(l,1);
		auto focalLength = Lua::CheckNumber(l,1);
		auto aspectRatio = Lua::CheckNumber(l,1);
		Lua::PushNumber(l,::umath::camera::calc_fov_from_lens(sensorSize,focalLength,aspectRatio));
		return 1;
	}));
	lua_pushtablecfunction(lua.GetState(),"math","calc_aperture_size_from_fstop",static_cast<int32_t(*)(lua_State*)>([](lua_State *l) -> int32_t {
		auto fstop = Lua::CheckNumber(l,1);
		auto focalLength = Lua::CheckNumber(l,2);
		auto orthographicCamera = false;
		if(Lua::IsSet(l,3))
			orthographicCamera = Lua::CheckBool(l,3);
		Lua::PushNumber(l,::umath::camera::calc_aperture_size_from_fstop(fstop,focalLength,orthographicCamera));
		return 1;
	}));
	lua_pushtablecfunction(lua.GetState(),"math","calc_focal_length_from_fov",static_cast<int32_t(*)(lua_State*)>([](lua_State *l) -> int32_t {
		auto fov = Lua::CheckNumber(l,1);
		auto sensorSize = Lua::CheckNumber(l,2);
		Lua::PushNumber(l,::umath::camera::calc_focal_length_from_fov(fov,sensorSize));
		return 1;
	}));
	lua_pushtablecfunction(lua.GetState(),"math","calc_fov_from_focal_length",static_cast<int32_t(*)(lua_State*)>([](lua_State *l) -> int32_t {
		auto focalLength = Lua::CheckNumber(l,1);
		auto sensorSize = Lua::CheckNumber(l,2);
		Lua::PushNumber(l,::umath::camera::calc_fov_from_focal_length(focalLength,sensorSize));
		return 1;
	}));
	lua_pushtablecfunction(lua.GetState(),"math","cot",static_cast<int32_t(*)(lua_State*)>([](lua_State *l) -> int32_t {
		Lua::PushNumber(l,umath::cot(Lua::CheckNumber(l,1)));
		return 1;
	}));
	lua_pushtablecfunction(lua.GetState(),"math","float_to_half_float",static_cast<int32_t(*)(lua_State*)>([](lua_State *l) -> int32_t {
		Lua::PushInt(l,umath::float32_to_float16_glm(Lua::CheckNumber(l,1)));
		return 1;
	}));
	lua_pushtablecfunction(lua.GetState(),"math","half_float_to_float",static_cast<int32_t(*)(lua_State*)>([](lua_State *l) -> int32_t {
		Lua::PushNumber(l,umath::float16_to_float32_glm(Lua::CheckInt(l,1)));
		return 1;
	}));
	lua_pushtablecfunction(lua.GetState(),"math","parse_expression",parse_math_expression);
	lua_pushtablecfunction(lua.GetState(),"math","set_flag",static_cast<int32_t(*)(lua_State*)>([](lua_State *l) -> int32_t {
		auto flags = Lua::CheckInt(l,1);
		auto flag = Lua::CheckInt(l,2);
		auto set = Lua::CheckBool(l,3);
		if(set)
			flags |= flag;
		else
			flags &= ~flag;
		Lua::PushInt(l,flags);
		return 1;
	}));

	Lua::RegisterLibraryEnums(lua.GetState(),"math",{
		{"EXPRESSION_CODE_BRACKET_OPENING",mup::ECmdCode::cmBO},
		{"EXPRESSION_CODE_BRACKET_CLOSING",mup::ECmdCode::cmBC},
		{"EXPRESSION_CODE_INDEX_OPERATOR_OPENING",mup::ECmdCode::cmIO},
		{"EXPRESSION_CODE_INDEX_OPERATOR_CLOSING",mup::ECmdCode::cmIC},
		{"EXPRESSION_CODE_CURLY_BRACKET_OPENING",mup::ECmdCode::cmCBO},
		{"EXPRESSION_CODE_CURLY_BRACKET_CLOSING",mup::ECmdCode::cmCBC},
		{"EXPRESSION_CODE_COMMA",mup::ECmdCode::cmARG_SEP},
		{"EXPRESSION_CODE_TERNARY_IF",mup::ECmdCode::cmIF},
		{"EXPRESSION_CODE_TERNARY_ELSE",mup::ECmdCode::cmELSE},
		{"EXPRESSION_CODE_TERNARY_ENDIF",mup::ECmdCode::cmENDIF},
		{"EXPRESSION_CODE_VALUE",mup::ECmdCode::cmVAL},
		{"EXPRESSION_CODE_FUNCTION",mup::ECmdCode::cmFUNC},
		{"EXPRESSION_CODE_BINARY_OPERATOR",mup::ECmdCode::cmOPRT_BIN},
		{"EXPRESSION_CODE_INFIX_OPERATOR",mup::ECmdCode::cmOPRT_INFIX},
		{"EXPRESSION_CODE_POSTFIX_OPERATOR",mup::ECmdCode::cmOPRT_POSTFIX}
	});

	Lua::RegisterLibraryEnums(lua.GetState(),"math",{
		{"EASE_TYPE_BACK",umath::to_integral(umath::EaseType::Back)},
		{"EASE_TYPE_BOUNCE",umath::to_integral(umath::EaseType::Bounce)},
		{"EASE_TYPE_CIRCULAR",umath::to_integral(umath::EaseType::Circular)},
		{"EASE_TYPE_CUBIC",umath::to_integral(umath::EaseType::Cubic)},
		{"EASE_TYPE_ELASTIC",umath::to_integral(umath::EaseType::Elastic)},
		{"EASE_TYPE_EXPONENTIAL",umath::to_integral(umath::EaseType::Exponential)},
		{"EASE_TYPE_LINEAR",umath::to_integral(umath::EaseType::Linear)},
		{"EASE_TYPE_QUADRATIC",umath::to_integral(umath::EaseType::Quadratic)},
		{"EASE_TYPE_QUARTIC",umath::to_integral(umath::EaseType::Quartic)},
		{"EASE_TYPE_QUINTIC",umath::to_integral(umath::EaseType::Quintic)},
		{"EASE_TYPE_SINE",umath::to_integral(umath::EaseType::Sine)},

		{"MAX_SINT8",std::numeric_limits<int8_t>::max()},
		{"MIN_SINT8",std::numeric_limits<int8_t>::lowest()},
		{"MAX_UINT8",std::numeric_limits<uint8_t>::max()},
		{"MIN_UINT8",std::numeric_limits<uint8_t>::lowest()},
		{"MAX_SINT16",std::numeric_limits<int16_t>::max()},
		{"MIN_SINT16",std::numeric_limits<int16_t>::lowest()},
		{"MAX_UINT16",std::numeric_limits<uint16_t>::max()},
		{"MIN_UINT16",std::numeric_limits<uint16_t>::lowest()},
		{"MAX_SINT32",std::numeric_limits<int32_t>::max()},
		{"MIN_SINT32",std::numeric_limits<int32_t>::lowest()},
		{"MAX_UINT32",std::numeric_limits<uint32_t>::max()},
		{"MIN_UINT32",std::numeric_limits<uint32_t>::lowest()},
		{"MAX_SINT64",std::numeric_limits<int64_t>::max()},
		{"MIN_SINT64",std::numeric_limits<int64_t>::lowest()},
		{"MAX_UINT64",std::numeric_limits<uint64_t>::max()},
		{"MIN_UINT64",std::numeric_limits<uint64_t>::lowest()},
		{"MAX_FLOAT",std::numeric_limits<float>::max()},
		{"MIN_FLOAT",std::numeric_limits<float>::lowest()},
		{"MAX_DOUBLE",std::numeric_limits<double>::max()},
		{"MIN_DOUBLE",std::numeric_limits<double>::lowest()},
		{"MAX_LONG_DOUBLE",std::numeric_limits<long double>::max()},
		{"MIN_LONG_DOUBLE",std::numeric_limits<long double>::lowest()},

		{"ROTATION_ORDER_XYZ",umath::to_integral(pragma::RotationOrder::XYZ)},
		{"ROTATION_ORDER_YXZ",umath::to_integral(pragma::RotationOrder::YXZ)},
		{"ROTATION_ORDER_XZX",umath::to_integral(pragma::RotationOrder::XZX)},
		{"ROTATION_ORDER_XYX",umath::to_integral(pragma::RotationOrder::XYX)},
		{"ROTATION_ORDER_YXY",umath::to_integral(pragma::RotationOrder::YXY)},
		{"ROTATION_ORDER_YZY",umath::to_integral(pragma::RotationOrder::YZY)},
		{"ROTATION_ORDER_ZYZ",umath::to_integral(pragma::RotationOrder::ZYZ)},
		{"ROTATION_ORDER_ZXZ",umath::to_integral(pragma::RotationOrder::ZXZ)},
		{"ROTATION_ORDER_XZY",umath::to_integral(pragma::RotationOrder::XZY)},
		{"ROTATION_ORDER_YZX",umath::to_integral(pragma::RotationOrder::YZX)},
		{"ROTATION_ORDER_ZYX",umath::to_integral(pragma::RotationOrder::ZYX)},
		{"ROTATION_ORDER_ZXY",umath::to_integral(pragma::RotationOrder::ZXY)},
		
		{"AXIS_X",umath::to_integral(pragma::Axis::X)},
		{"AXIS_Y",umath::to_integral(pragma::Axis::Y)},
		{"AXIS_Z",umath::to_integral(pragma::Axis::Z)}
	});

	auto &mathMod = lua.RegisterLibrary("math");
	auto complexNumberClassDef = luabind::class_<std::complex<double>>("ComplexNumber");
	complexNumberClassDef.def(luabind::constructor<>());
	complexNumberClassDef.def(luabind::constructor<double,double>());
	complexNumberClassDef.def(luabind::tostring(luabind::self));
	complexNumberClassDef.def(-luabind::const_self);
	complexNumberClassDef.def(luabind::const_self /luabind::const_self);
	complexNumberClassDef.def(luabind::const_self *luabind::const_self);
	complexNumberClassDef.def(luabind::const_self +luabind::const_self);
	complexNumberClassDef.def(luabind::const_self -luabind::const_self);
	complexNumberClassDef.def(luabind::const_self ==luabind::const_self);
	complexNumberClassDef.def(luabind::const_self /double());
	complexNumberClassDef.def(luabind::const_self *double());
	complexNumberClassDef.def(luabind::const_self +double());
	complexNumberClassDef.def(luabind::const_self -double());
	complexNumberClassDef.def(luabind::const_self ==double());
#ifdef _WIN32
	complexNumberClassDef.def("SetReal",static_cast<double(std::complex<double>::*)(const double&)>(&std::complex<double>::real));
	complexNumberClassDef.def("SetImaginary",static_cast<double(std::complex<double>::*)(const double&)>(&std::complex<double>::real));
#else
	complexNumberClassDef.def("SetReal",static_cast<void(std::complex<double>::*)(double)>(&std::complex<double>::real));
	complexNumberClassDef.def("SetImaginary",static_cast<void(std::complex<double>::*)(double)>(&std::complex<double>::real));
#endif
	complexNumberClassDef.def("GetReal",static_cast<double(std::complex<double>::*)() const>(&std::complex<double>::real));
	complexNumberClassDef.def("GetImaginary",static_cast<double(std::complex<double>::*)() const>(&std::complex<double>::imag));
	//mathMod[complexNumberClassDef];

#if 0
	auto expressionClassDef = luabind::class_<exprtk::expression<float>>("Expression");
	expressionClassDef.def(luabind::constructor<>());
	expressionClassDef.def("RegisterSymbolTable",static_cast<void(*)(lua_State*,exprtk::expression<float>&,exprtk::symbol_table<float>&)>([](lua_State *l,exprtk::expression<float> &expr,exprtk::symbol_table<float> &symbolTable) {
		expr.register_symbol_table(symbolTable);
	}));
	expressionClassDef.def("GetValue",static_cast<void(*)(lua_State*,exprtk::expression<float>&)>([](lua_State *l,exprtk::expression<float> &expr) {
		Lua::PushNumber(l,expr.value());
	}));

	auto parserClassDef = luabind::class_<exprtk::parser<float>>("Parser");
	parserClassDef.def(luabind::constructor<>());
	parserClassDef.def("RegisterSymbolTable",static_cast<void(*)(lua_State*,exprtk::parser<float>&,const std::string&,exprtk::symbol_table<float>&)>([](lua_State *l,exprtk::parser<float> &parser,const std::string &expressionString,exprtk::symbol_table<float> &symbolTable) {
		Lua::Push(l,parser.compile(expressionString,symbolTable));
	}));
	expressionClassDef.scope[parserClassDef];

	auto symbolTableClassDef = luabind::class_<exprtk::symbol_table<float>>("SymbolTable");
	symbolTableClassDef.def(luabind::constructor<>());
	symbolTableClassDef.def("AddConstant",static_cast<void(*)(lua_State*,exprtk::symbol_table<float>&,const std::string&,float)>([](lua_State *l,exprtk::symbol_table<float> &symbolTable,const std::string &name,float val) {
		symbolTable.add_constant(name,val);
	}));
	expressionClassDef.scope[symbolTableClassDef];

	mathMod[expressionClassDef];
#endif

	lua_pushtablecfunction(lua.GetState(),"debug","move_state_to_string",Lua::debug::move_state_to_string);
	if(Lua::get_extended_lua_modules_enabled())
	{
		DLLLUA int lua_snapshot(lua_State *L);
		lua_pushtablecfunction(lua.GetState(),"debug","snapshot",lua_snapshot);
	}
	//lua_pushtablecfunction(lua.GetState(),"debug","enable_remote_debugging",Lua::debug::enable_remote_debugging);

	lua_register(lua.GetState(),"print",Lua_print);
	Lua::RegisterLibrary(lua.GetState(),"console",{
		{"print",Lua_print},
		{"printc",Lua_MsgC},
		{"print_table",Lua_PrintTable},
		{"print_message",Lua_Msg},
		{"print_messageln",Lua_MsgN},
		{"print_color",Lua_MsgC},
		{"print_warning",Lua_MsgW},
		{"print_error",Lua_MsgE},

		{"register_variable",Lua_cvar_CreateConVar},
		{"register_command",Lua_cvar_CreateConCommand},
		{"run",Lua_cvar_Run},
		{"get_convar",Lua_cvar_GetConVar},
		{"get_convar_int",Lua_cvar_GetConVarInt},
		{"get_convar_float",Lua_cvar_GetConVarFloat},
		{"get_convar_string",Lua_cvar_GetConVarString},
		{"get_convar_bool",Lua_cvar_GetConVarBool},
		{"get_convar_flags",Lua_cvar_GetConVarFlags},
		{"add_change_callback",Lua_cvar_AddChangeCallback},

		{"register_override",Lua::console::register_override},
		{"clear_override",Lua::console::clear_override},
		{"parse_command_arguments",Lua::console::parse_command_arguments}
	});

	auto consoleMod = luabind::module(lua.GetState(),"console");

	static const auto fGetConVarName = [](lua_State *l,ConVar &cvar) -> std::string {
		auto *nw = engine->GetNetworkState(l);
		auto &conVars = nw->GetConVars();
		auto it = std::find_if(conVars.begin(),conVars.end(),[&cvar](const std::pair<std::string,std::shared_ptr<ConConf>> &pair) {
			return pair.second.get() == &cvar;
		});
		if(it == conVars.end())
			return "";
		return it->first;
	};
	auto classDefConVar = luabind::class_<ConVar>("Var");
	classDefConVar.def("GetString",&Lua_ConVar_GetString);
	classDefConVar.def("GetInt",&Lua_ConVar_GetInt);
	classDefConVar.def("GetFloat",&Lua_ConVar_GetFloat);
	classDefConVar.def("GetBool",&Lua_ConVar_GetBool);
	classDefConVar.def("GetFlags",&Lua_ConVar_GetFlags);
	classDefConVar.def("GetDefault",static_cast<void(*)(lua_State*,ConVar&)>([](lua_State *l,ConVar &cvar) {
		Lua::PushString(l,cvar.GetDefault());
	}));
	classDefConVar.def("GetHelpText",static_cast<void(*)(lua_State*,ConVar&)>([](lua_State *l,ConVar &cvar) {
		Lua::PushString(l,cvar.GetHelpText());
	}));
	classDefConVar.def("AddChangeCallback",static_cast<void(*)(lua_State*,ConVar&,luabind::object)>([](lua_State *l,ConVar &cvar,luabind::object oFunction) {
		Lua::CheckFunction(l,2);
		auto fc = luabind::object(luabind::from_stack(l,2));
		engine->GetNetworkState(l)->GetGameState()->AddConVarCallback(fGetConVarName(l,cvar),fc);
	}));
	classDefConVar.def("GetName",static_cast<void(*)(lua_State*,ConVar&)>([](lua_State *l,ConVar &cvar) {
		auto name = fGetConVarName(l,cvar);
		Lua::PushString(l,name);
	}));
	consoleMod[classDefConVar];

	// util
	Lua::RegisterLibraryEnums(lua.GetState(),"util",{
		{"SIZEOF_CHAR",sizeof(char)},
		{"SIZEOF_BOOL",sizeof(bool)},
		{"SIZEOF_INT",sizeof(int)},
		{"SIZEOF_SHORT",sizeof(int16_t)},
		{"SIZEOF_FLOAT",sizeof(float)},
		{"SIZEOF_DOUBLE",sizeof(double)},
		{"SIZEOF_LONG_LONG",sizeof(long long)},
		{"SIZEOF_LONG_DOUBLE",sizeof(long double)},
		{"SIZEOF_VECTOR3",sizeof(Vector3)},
		{"SIZEOF_VECTOR2",sizeof(Vector2)},
		{"SIZEOF_VECTOR4",sizeof(Vector4)},
		{"SIZEOF_EULER_ANGLES",sizeof(EulerAngles)},
		{"SIZEOF_QUATERNION",sizeof(Quat)},
		{"SIZEOF_MAT2",sizeof(Mat2)},
		{"SIZEOF_MAT2X3",sizeof(Mat2x3)},
		{"SIZEOF_MAT2X4",sizeof(Mat2x4)},
		{"SIZEOF_MAT3X2",sizeof(Mat3x2)},
		{"SIZEOF_MAT3",sizeof(Mat3)},
		{"SIZEOF_MAT3X4",sizeof(Mat3x4)},
		{"SIZEOF_MAT4X2",sizeof(Mat4x2)},
		{"SIZEOF_MAT4X3",sizeof(Mat4x3)},
		{"SIZEOF_MAT4",sizeof(Mat4)},
		{"SIZEOF_VECTOR2I",sizeof(Vector2i)},
		{"SIZEOF_VECTOR3I",sizeof(Vector3i)},
		{"SIZEOF_VECTOR4I",sizeof(Vector4i)},

		{"VAR_TYPE_INVALID",umath::to_integral(util::VarType::Invalid)},
		{"VAR_TYPE_BOOL",umath::to_integral(util::VarType::Bool)},
		{"VAR_TYPE_DOUBLE",umath::to_integral(util::VarType::Double)},
		{"VAR_TYPE_FLOAT",umath::to_integral(util::VarType::Float)},
		{"VAR_TYPE_INT8",umath::to_integral(util::VarType::Int8)},
		{"VAR_TYPE_INT16",umath::to_integral(util::VarType::Int16)},
		{"VAR_TYPE_INT32",umath::to_integral(util::VarType::Int32)},
		{"VAR_TYPE_INT64",umath::to_integral(util::VarType::Int64)},
		{"VAR_TYPE_LONG_DOUBLE",umath::to_integral(util::VarType::LongDouble)},
		{"VAR_TYPE_STRING",umath::to_integral(util::VarType::String)},
		{"VAR_TYPE_UINT8",umath::to_integral(util::VarType::UInt8)},
		{"VAR_TYPE_UINT16",umath::to_integral(util::VarType::UInt16)},
		{"VAR_TYPE_UINT32",umath::to_integral(util::VarType::UInt32)},
		{"VAR_TYPE_UINT64",umath::to_integral(util::VarType::UInt64)},
		{"VAR_TYPE_EULER_ANGLES",umath::to_integral(util::VarType::EulerAngles)},
		{"VAR_TYPE_COLOR",umath::to_integral(util::VarType::Color)},
		{"VAR_TYPE_VECTOR",umath::to_integral(util::VarType::Vector)},
		{"VAR_TYPE_VECTOR2",umath::to_integral(util::VarType::Vector2)},
		{"VAR_TYPE_VECTOR4",umath::to_integral(util::VarType::Vector4)},
		{"VAR_TYPE_ENTITY",umath::to_integral(util::VarType::Entity)},
		{"VAR_TYPE_QUATERNION",umath::to_integral(util::VarType::Quaternion)},

		{"EVENT_REPLY_HANDLED",umath::to_integral(util::EventReply::Handled)},
		{"EVENT_REPLY_UNHANDLED",umath::to_integral(util::EventReply::Unhandled)}
	});

	auto classDefErrorCode = luabind::class_<ErrorCode>("ResultCode");
	classDefErrorCode.def(luabind::constructor<>());
	classDefErrorCode.def(luabind::constructor<const std::string&,int32_t>());
	classDefErrorCode.def(luabind::tostring(luabind::self));
	classDefErrorCode.def("GetMessage",&Lua_ErrorCode_GetMessage);
	classDefErrorCode.def("GetValue",&Lua_ErrorCode_GetValue);
	classDefErrorCode.def("IsError",&Lua_ErrorCode_IsError);

	auto classDefCallback = luabind::class_<CallbackHandle>("Callback");
	classDefCallback.def(luabind::tostring(luabind::self));
	classDefCallback.scope[luabind::def("Create",static_cast<void(*)(lua_State*,luabind::object)>([](lua_State *l,luabind::object o) {
		Lua::CheckFunction(l,1);
		Lua::Push<CallbackHandle>(l,FunctionCallback<void,std::function<Lua::StatusCode(lua_State*)>>::Create([o,l](std::function<Lua::StatusCode(lua_State*)> fPushArgs) {
			Lua::CallFunction(l,[fPushArgs,&o](lua_State *l) -> Lua::StatusCode {
				o.push(l);
				return fPushArgs(l);
			},LUA_MULTRET);
		}));
	}))];
	classDefCallback.def("Call",static_cast<void(*)(lua_State*,CallbackHandle&)>([](lua_State *l,CallbackHandle &cb) {
		call_callback(cb,{});
	}));
	classDefCallback.def("Call",static_cast<void(*)(lua_State*,CallbackHandle&,luabind::object)>([](lua_State *l,CallbackHandle &cb,luabind::object arg0) {
		call_callback(cb,{arg0});
	}));
	classDefCallback.def("Call",static_cast<void(*)(lua_State*,CallbackHandle&,luabind::object,luabind::object)>([](lua_State *l,CallbackHandle &cb,luabind::object arg0,luabind::object arg1) {
		call_callback(cb,{arg0,arg1});
	}));
	classDefCallback.def("Call",static_cast<void(*)(lua_State*,CallbackHandle&,luabind::object,luabind::object,luabind::object)>([](
			lua_State *l,CallbackHandle &cb,luabind::object arg0,luabind::object arg1,luabind::object arg2
		) {
		call_callback(cb,{arg0,arg1,arg2});
	}));
	classDefCallback.def("Call",static_cast<void(*)(lua_State*,CallbackHandle&,luabind::object,luabind::object,luabind::object,luabind::object)>([](
		lua_State *l,CallbackHandle &cb,luabind::object arg0,luabind::object arg1,luabind::object arg2,luabind::object arg3
		) {
			call_callback(cb,{arg0,arg1,arg2,arg3});
	}));
	classDefCallback.def("Call",static_cast<void(*)(lua_State*,CallbackHandle&,luabind::object,luabind::object,luabind::object,luabind::object,luabind::object)>([](
		lua_State *l,CallbackHandle &cb,luabind::object arg0,luabind::object arg1,luabind::object arg2,luabind::object arg3,luabind::object arg4
		) {
			call_callback(cb,{arg0,arg1,arg2,arg3,arg4});
	}));
	classDefCallback.def("Call",static_cast<void(*)(lua_State*,CallbackHandle&,luabind::object,luabind::object,luabind::object,luabind::object,luabind::object,luabind::object)>([](
		lua_State *l,CallbackHandle &cb,luabind::object arg0,luabind::object arg1,luabind::object arg2,luabind::object arg3,luabind::object arg4,luabind::object arg5
		) {
			call_callback(cb,{arg0,arg1,arg2,arg3,arg4,arg5});
	}));
	classDefCallback.def("Call",static_cast<void(*)(
		lua_State*,CallbackHandle&,luabind::object,luabind::object,luabind::object,luabind::object,luabind::object,luabind::object,
		luabind::object
	)>([](
		lua_State *l,CallbackHandle &cb,luabind::object arg0,luabind::object arg1,luabind::object arg2,luabind::object arg3,luabind::object arg4,luabind::object arg5,
		luabind::object arg6
		) {
			call_callback(cb,{arg0,arg1,arg2,arg3,arg4,arg5,arg6});
	}));
	classDefCallback.def("Call",static_cast<void(*)(
		lua_State*,CallbackHandle&,luabind::object,luabind::object,luabind::object,luabind::object,luabind::object,luabind::object,
		luabind::object,luabind::object
		)>([](
			lua_State *l,CallbackHandle &cb,luabind::object arg0,luabind::object arg1,luabind::object arg2,luabind::object arg3,luabind::object arg4,luabind::object arg5,
			luabind::object arg6,luabind::object arg7
			) {
				call_callback(cb,{arg0,arg1,arg2,arg3,arg4,arg5,arg6,arg7});
	}));
	classDefCallback.def("Call",static_cast<void(*)(
		lua_State*,CallbackHandle&,luabind::object,luabind::object,luabind::object,luabind::object,luabind::object,luabind::object,
		luabind::object,luabind::object,luabind::object
		)>([](
			lua_State *l,CallbackHandle &cb,luabind::object arg0,luabind::object arg1,luabind::object arg2,luabind::object arg3,luabind::object arg4,luabind::object arg5,
			luabind::object arg6,luabind::object arg7,luabind::object arg8
			) {
				call_callback(cb,{arg0,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8});
	}));
	classDefCallback.def("Call",static_cast<void(*)(
		lua_State*,CallbackHandle&,luabind::object,luabind::object,luabind::object,luabind::object,luabind::object,luabind::object,
		luabind::object,luabind::object,luabind::object,luabind::object
		)>([](
			lua_State *l,CallbackHandle &cb,luabind::object arg0,luabind::object arg1,luabind::object arg2,luabind::object arg3,luabind::object arg4,luabind::object arg5,
			luabind::object arg6,luabind::object arg7,luabind::object arg8,luabind::object arg9
			) {
				call_callback(cb,{arg0,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9});
	}));
	classDefCallback.def("Call",static_cast<void(*)(
		lua_State*,CallbackHandle&,luabind::object,luabind::object,luabind::object,luabind::object,luabind::object,luabind::object,
		luabind::object,luabind::object,luabind::object,luabind::object,luabind::object
		)>([](
			lua_State *l,CallbackHandle &cb,luabind::object arg0,luabind::object arg1,luabind::object arg2,luabind::object arg3,luabind::object arg4,luabind::object arg5,
			luabind::object arg6,luabind::object arg7,luabind::object arg8,luabind::object arg9,luabind::object arg10
			) {
				call_callback(cb,{arg0,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9,arg10});
	}));
	classDefCallback.def("Call",static_cast<void(*)(
		lua_State*,CallbackHandle&,luabind::object,luabind::object,luabind::object,luabind::object,luabind::object,luabind::object,
		luabind::object,luabind::object,luabind::object,luabind::object,luabind::object,luabind::object
		)>([](
			lua_State *l,CallbackHandle &cb,luabind::object arg0,luabind::object arg1,luabind::object arg2,luabind::object arg3,luabind::object arg4,luabind::object arg5,
			luabind::object arg6,luabind::object arg7,luabind::object arg8,luabind::object arg9,luabind::object arg10,luabind::object arg11
			) {
				call_callback(cb,{arg0,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9,arg10,arg11});
	}));
	classDefCallback.def("Call",static_cast<void(*)(
		lua_State*,CallbackHandle&,luabind::object,luabind::object,luabind::object,luabind::object,luabind::object,luabind::object,
		luabind::object,luabind::object,luabind::object,luabind::object,luabind::object,luabind::object,luabind::object
		)>([](
			lua_State *l,CallbackHandle &cb,luabind::object arg0,luabind::object arg1,luabind::object arg2,luabind::object arg3,luabind::object arg4,luabind::object arg5,
			luabind::object arg6,luabind::object arg7,luabind::object arg8,luabind::object arg9,luabind::object arg10,luabind::object arg11,luabind::object arg12
			) {
				call_callback(cb,{arg0,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9,arg10,arg11,arg12});
	}));
	classDefCallback.def("Call",static_cast<void(*)(
		lua_State*,CallbackHandle&,luabind::object,luabind::object,luabind::object,luabind::object,luabind::object,luabind::object,
		luabind::object,luabind::object,luabind::object,luabind::object,luabind::object,luabind::object,luabind::object,
		luabind::object
		)>([](
			lua_State *l,CallbackHandle &cb,luabind::object arg0,luabind::object arg1,luabind::object arg2,luabind::object arg3,luabind::object arg4,luabind::object arg5,
			luabind::object arg6,luabind::object arg7,luabind::object arg8,luabind::object arg9,luabind::object arg10,luabind::object arg11,luabind::object arg12,
			luabind::object arg13
			) {
				call_callback(cb,{arg0,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9,arg10,arg11,arg12,arg13});
	}));
	classDefCallback.def("Call",static_cast<void(*)(
		lua_State*,CallbackHandle&,luabind::object,luabind::object,luabind::object,luabind::object,luabind::object,luabind::object,
		luabind::object,luabind::object,luabind::object,luabind::object,luabind::object,luabind::object,luabind::object,
		luabind::object,luabind::object
		)>([](
			lua_State *l,CallbackHandle &cb,luabind::object arg0,luabind::object arg1,luabind::object arg2,luabind::object arg3,luabind::object arg4,luabind::object arg5,
			luabind::object arg6,luabind::object arg7,luabind::object arg8,luabind::object arg9,luabind::object arg10,luabind::object arg11,luabind::object arg12,
			luabind::object arg13,luabind::object arg14
			) {
				call_callback(cb,{arg0,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9,arg10,arg11,arg12,arg13,arg14});
	}));
	classDefCallback.def("Call",static_cast<void(*)(
		lua_State*,CallbackHandle&,luabind::object,luabind::object,luabind::object,luabind::object,luabind::object,luabind::object,
		luabind::object,luabind::object,luabind::object,luabind::object,luabind::object,luabind::object,luabind::object,
		luabind::object,luabind::object,luabind::object
		)>([](
			lua_State *l,CallbackHandle &cb,luabind::object arg0,luabind::object arg1,luabind::object arg2,luabind::object arg3,luabind::object arg4,luabind::object arg5,
			luabind::object arg6,luabind::object arg7,luabind::object arg8,luabind::object arg9,luabind::object arg10,luabind::object arg11,luabind::object arg12,
			luabind::object arg13,luabind::object arg14,luabind::object arg15
			) {
				call_callback(cb,{arg0,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9,arg10,arg11,arg12,arg13,arg14,arg15});
		}));
	classDefCallback.def("IsValid",&Lua_Callback_IsValid);
	classDefCallback.def("Remove",&Lua_Callback_Remove);

	auto callbackHandlerClassDef = luabind::class_<CallbackHandler>("CallbackHandler");
	Lua::CallbackHandler::register_class(callbackHandlerClassDef);

	auto utilMod = luabind::module(lua.GetState(),"util");
	utilMod[classDefErrorCode];
	utilMod[classDefCallback];
	utilMod[callbackHandlerClassDef];

	auto defHSV = luabind::class_<util::HSV>("HSVColor");
	defHSV.def(luabind::constructor<>());
	defHSV.def(luabind::constructor<double,double,double>());
	defHSV.def(luabind::tostring(luabind::self));
	defHSV.def(luabind::const_self ==luabind::const_self);
	defHSV.def_readwrite("h",&util::HSV::h);
	defHSV.def_readwrite("s",&util::HSV::s);
	defHSV.def_readwrite("v",&util::HSV::v);
	defHSV.def("ToRGBColor",static_cast<void(*)(lua_State*,const util::HSV&)>([](lua_State *l,const util::HSV &hsv) {
		Lua::Push<Color>(l,util::hsv_to_rgb(hsv));
	}));
	defHSV.def("Lerp",static_cast<void(*)(lua_State*,const util::HSV&,const util::HSV&,float)>([](lua_State *l,const util::HSV &hsv0,const util::HSV &hsv1,float t) {
		Lua::Push<util::HSV>(l,util::lerp_hsv(hsv0,hsv1,t));
	}));
	utilMod[defHSV];

	auto defColor = luabind::class_<Color>("Color");
	defColor.scope[luabind::def("CreateFromHexColor",static_cast<void(*)(lua_State*,const std::string&)>([](lua_State *l,const std::string &hexColor) {
		Lua::Push<Color>(l,Color::CreateFromHexColor(hexColor));
	}))];
	defColor.def(luabind::constructor<>());
	defColor.def(luabind::constructor<short,short,short>());
	defColor.def(luabind::constructor<short,short,short,short>());
	defColor.def(luabind::constructor<const Vector3&>());
	defColor.def(luabind::constructor<const Vector4&>());
	defColor.def(luabind::constructor<const std::string&>());
	defColor.def(luabind::tostring(luabind::self));
	defColor.def_readwrite("r",&Color::r);
	defColor.def_readwrite("g",&Color::g);
	defColor.def_readwrite("b",&Color::b);
	defColor.def_readwrite("a",&Color::a);
	defColor.def(luabind::const_self /float());
	defColor.def(luabind::const_self *float());
	defColor.def(luabind::const_self +luabind::const_self);
	defColor.def(luabind::const_self -luabind::const_self);
	defColor.def(luabind::const_self ==luabind::const_self);
	defColor.def(float() *luabind::const_self);
	defColor.def("Copy",&Lua::Color::Copy);
	defColor.def("Set",static_cast<void(Color::*)(const Color&)>(&Color::Set));
	defColor.def("Set",&Lua::Color::Set);
	defColor.def("Lerp",&Lua::Color::Lerp);
	defColor.def("ToVector4",&Lua::Color::ToVector4);
	defColor.def("ToVector",&Lua::Color::ToVector);
	defColor.def("ToHexColor",static_cast<void(*)(lua_State*,const Color&)>([](lua_State *l,const Color &color) {
		Lua::PushString(l,color.ToHexColor());
	}));
	defColor.def("ToHexColorRGB",static_cast<void(*)(lua_State*,const Color&)>([](lua_State *l,const Color &color) {
		Lua::PushString(l,color.ToHexColorRGB());
	}));
	defColor.def("ToHSVColor",static_cast<void(*)(lua_State*,const Color&)>([](lua_State *l,const Color &color) {
		Lua::Push<util::HSV>(l,util::rgb_to_hsv(color));
	}));
	defColor.def("GetComplementaryColor",static_cast<void(*)(lua_State*,const Color&)>([](lua_State *l,const Color &color) {
		Lua::Push(l,color.GetComplementaryColor());
	}));
	defColor.def("GetContrastColor",static_cast<void(*)(lua_State*,const Color&)>([](lua_State *l,const Color &color) {
		Lua::Push(l,color.GetContrastColor());
	}));
	defColor.def("CalcPerceivedLuminance",static_cast<void(*)(lua_State*,const Color&)>([](lua_State *l,const Color &color) {
		Lua::PushNumber(l,color.CalcPerceivedLuminance());
	}));
	utilMod[defColor];

	auto _G = luabind::globals(lua.GetState());
	_G["Color"] = _G["util"]["Color"]; // Add to global table for quicker access
	_G["util"]["Color"]["Clear"] = Color(0,0,0,0);
	// Pink Colors
	_G["util"]["Color"]["Pink"] = Color::Pink;
	_G["util"]["Color"]["LightPink"] = Color::LightPink;
	_G["util"]["Color"]["HotPink"] = Color::HotPink;
	_G["util"]["Color"]["DeepPink"] = Color::DeepPink;
	_G["util"]["Color"]["PaleVioletRed"] = Color::PaleVioletRed;
	_G["util"]["Color"]["MediumVioletRed"] = Color::MediumVioletRed;

	// Red Colors
	_G["util"]["Color"]["LightSalmon"] = Color::LightSalmon;
	_G["util"]["Color"]["Salmon"] = Color::Salmon;
	_G["util"]["Color"]["DarkSalmon"] = Color::DarkSalmon;
	_G["util"]["Color"]["LightCoral"] = Color::LightCoral;
	_G["util"]["Color"]["IndianRed"] = Color::IndianRed;
	_G["util"]["Color"]["Crimson"] = Color::Crimson;
	_G["util"]["Color"]["FireBrick"] = Color::FireBrick;
	_G["util"]["Color"]["DarkRed"] = Color::DarkRed;
	_G["util"]["Color"]["Red"] = Color::Red;

	// Orange Colors
	_G["util"]["Color"]["OrangeRed"] = Color::OrangeRed;
	_G["util"]["Color"]["Tomato"] = Color::Tomato;
	_G["util"]["Color"]["Coral"] = Color::Coral;
	_G["util"]["Color"]["DarkOrange"] = Color::DarkOrange;
	_G["util"]["Color"]["Orange"] = Color::Orange;

	// Yellow Colors
	_G["util"]["Color"]["Yellow"] = Color::Yellow;
	_G["util"]["Color"]["LightYellow"] = Color::LightYellow;
	_G["util"]["Color"]["LemonChiffon"] = Color::LemonChiffon;
	_G["util"]["Color"]["LightGoldenrodYellow"] = Color::LightGoldenrodYellow;
	_G["util"]["Color"]["PapayaWhip"] = Color::PapayaWhip;
	_G["util"]["Color"]["Moccasin"] = Color::Moccasin;
	_G["util"]["Color"]["PeachPuff"] = Color::PeachPuff;
	_G["util"]["Color"]["PaleGoldenrod"] = Color::PaleGoldenrod;
	_G["util"]["Color"]["Khaki"] = Color::Khaki;
	_G["util"]["Color"]["DarkKhaki"] = Color::DarkKhaki;
	_G["util"]["Color"]["Gold"] = Color::Gold;

	// Brown Colors
	_G["util"]["Color"]["Cornsilk"] = Color::Cornsilk;
	_G["util"]["Color"]["BlanchedAlmond"] = Color::BlanchedAlmond;
	_G["util"]["Color"]["Bisque"] = Color::Bisque;
	_G["util"]["Color"]["NavajoWhite"] = Color::NavajoWhite;
	_G["util"]["Color"]["Wheat"] = Color::Wheat;
	_G["util"]["Color"]["BurlyWood"] = Color::BurlyWood;
	_G["util"]["Color"]["Tan"] = Color::Tan;
	_G["util"]["Color"]["RosyBrown"] = Color::RosyBrown;
	_G["util"]["Color"]["SandyBrown"] = Color::SandyBrown;
	_G["util"]["Color"]["Goldenrod"] = Color::Goldenrod;
	_G["util"]["Color"]["DarkGoldenrod"] = Color::DarkGoldenrod;
	_G["util"]["Color"]["Peru"] = Color::Peru;
	_G["util"]["Color"]["Chocolate"] = Color::Chocolate;
	_G["util"]["Color"]["SaddleBrown"] = Color::SaddleBrown;
	_G["util"]["Color"]["Sienna"] = Color::Sienna;
	_G["util"]["Color"]["Brown"] = Color::Brown;
	_G["util"]["Color"]["Maroon"] = Color::Maroon;

	// Green Colors
	_G["util"]["Color"]["DarkOliveGreen"] = Color::DarkOliveGreen;
	_G["util"]["Color"]["Olive"] = Color::Olive;
	_G["util"]["Color"]["OliveDrab"] = Color::OliveDrab;
	_G["util"]["Color"]["YellowGreen"] = Color::YellowGreen;
	_G["util"]["Color"]["LimeGreen"] = Color::LimeGreen;
	_G["util"]["Color"]["Lime"] = Color::Lime;
	_G["util"]["Color"]["LawnGreen"] = Color::LawnGreen;
	_G["util"]["Color"]["Chartreuse"] = Color::Chartreuse;
	_G["util"]["Color"]["GreenYellow"] = Color::GreenYellow;
	_G["util"]["Color"]["SpringGreen"] = Color::SpringGreen;
	_G["util"]["Color"]["MediumSpringGreen"] = Color::MediumSpringGreen;
	_G["util"]["Color"]["LightGreen"] = Color::LightGreen;
	_G["util"]["Color"]["PaleGreen"] = Color::PaleGreen;
	_G["util"]["Color"]["DarkSeaGreen"] = Color::DarkSeaGreen;
	_G["util"]["Color"]["MediumAquamarine"] = Color::MediumAquamarine;
	_G["util"]["Color"]["MediumSeaGreen"] = Color::MediumSeaGreen;
	_G["util"]["Color"]["SeaGreen"] = Color::SeaGreen;
	_G["util"]["Color"]["ForestGreen"] = Color::ForestGreen;
	_G["util"]["Color"]["Green"] = Color::Green;
	_G["util"]["Color"]["DarkGreen"] = Color::DarkGreen;

	// Cyan Colors
	_G["util"]["Color"]["Aqua"] = Color::Aqua;
	_G["util"]["Color"]["Cyan"] = Color::Cyan;
	_G["util"]["Color"]["LightCyan"] = Color::LightCyan;
	_G["util"]["Color"]["PaleTurquoise"] = Color::PaleTurquoise;
	_G["util"]["Color"]["Aquamarine"] = Color::Aquamarine;
	_G["util"]["Color"]["Turquoise"] = Color::Turquoise;
	_G["util"]["Color"]["MediumTurquoise"] = Color::MediumTurquoise;
	_G["util"]["Color"]["DarkTurquoise"] = Color::DarkTurquoise;
	_G["util"]["Color"]["LightSeaGreen"] = Color::LightSeaGreen;
	_G["util"]["Color"]["CadetBlue"] = Color::CadetBlue;
	_G["util"]["Color"]["DarkCyan"] = Color::DarkCyan;
	_G["util"]["Color"]["Teal"] = Color::Teal;

	// Blue Colors
	_G["util"]["Color"]["LightSteelBlue"] = Color::LightSteelBlue;
	_G["util"]["Color"]["PowderBlue"] = Color::PowderBlue;
	_G["util"]["Color"]["LightBlue"] = Color::LightBlue;
	_G["util"]["Color"]["SkyBlue"] = Color::SkyBlue;
	_G["util"]["Color"]["LightSkyBlue"] = Color::LightSkyBlue;
	_G["util"]["Color"]["DeepSkyBlue"] = Color::DeepSkyBlue;
	_G["util"]["Color"]["DodgerBlue"] = Color::DodgerBlue;
	_G["util"]["Color"]["CornflowerBlue"] = Color::CornflowerBlue;
	_G["util"]["Color"]["SteelBlue"] = Color::SteelBlue;
	_G["util"]["Color"]["RoyalBlue"] = Color::RoyalBlue;
	_G["util"]["Color"]["Blue"] = Color::Blue;
	_G["util"]["Color"]["MediumBlue"] = Color::MediumBlue;
	_G["util"]["Color"]["DarkBlue"] = Color::DarkBlue;
	_G["util"]["Color"]["Navy"] = Color::Navy;
	_G["util"]["Color"]["MidnightBlue"] = Color::MidnightBlue;

	// Purple, Violet and Magenta Colors
	_G["util"]["Color"]["Lavender"] = Color::Lavender;
	_G["util"]["Color"]["Thistle"] = Color::Thistle;
	_G["util"]["Color"]["Plum"] = Color::Plum;
	_G["util"]["Color"]["Violet"] = Color::Violet;
	_G["util"]["Color"]["Orchid"] = Color::Orchid;
	_G["util"]["Color"]["Fuchsia"] = Color::Fuchsia;
	_G["util"]["Color"]["Magenta"] = Color::Magenta;
	_G["util"]["Color"]["MediumOrchid"] = Color::MediumOrchid;
	_G["util"]["Color"]["MediumPurple"] = Color::MediumPurple;
	_G["util"]["Color"]["BlueViolet"] = Color::BlueViolet;
	_G["util"]["Color"]["DarkViolet"] = Color::DarkViolet;
	_G["util"]["Color"]["DarkOrchid"] = Color::DarkOrchid;
	_G["util"]["Color"]["DarkMagenta"] = Color::DarkMagenta;
	_G["util"]["Color"]["Purple"] = Color::Purple;
	_G["util"]["Color"]["Indigo"] = Color::Indigo;
	_G["util"]["Color"]["DarkSlateBlue"] = Color::DarkSlateBlue;
	_G["util"]["Color"]["SlateBlue"] = Color::SlateBlue;
	_G["util"]["Color"]["MediumSlateBlue"] = Color::MediumSlateBlue;

	// White Colors
	_G["util"]["Color"]["White"] = Color::White;
	_G["util"]["Color"]["Snow"] = Color::Snow;
	_G["util"]["Color"]["Honeydew"] = Color::Honeydew;
	_G["util"]["Color"]["MintCream"] = Color::MintCream;
	_G["util"]["Color"]["Azure"] = Color::Azure;
	_G["util"]["Color"]["AliceBlue"] = Color::AliceBlue;
	_G["util"]["Color"]["GhostWhite"] = Color::GhostWhite;
	_G["util"]["Color"]["WhiteSmoke"] = Color::WhiteSmoke;
	_G["util"]["Color"]["Seashell"] = Color::Seashell;
	_G["util"]["Color"]["Beige"] = Color::Beige;
	_G["util"]["Color"]["OldLace"] = Color::OldLace;
	_G["util"]["Color"]["FloralWhite"] = Color::FloralWhite;
	_G["util"]["Color"]["Ivory"] = Color::Ivory;
	_G["util"]["Color"]["AntiqueWhite"] = Color::AntiqueWhite;
	_G["util"]["Color"]["Linen"] = Color::Linen;
	_G["util"]["Color"]["LavenderBlush"] = Color::LavenderBlush;
	_G["util"]["Color"]["MistyRose"] = Color::MistyRose;

	// Grey and Black Colors
	_G["util"]["Color"]["Gainsboro"] = Color::Gainsboro;
	_G["util"]["Color"]["LightGrey"] = Color::LightGrey;
	_G["util"]["Color"]["Silver"] = Color::Silver;
	_G["util"]["Color"]["DarkGray"] = Color::DarkGray;
	_G["util"]["Color"]["Gray"] = Color::Gray;
	_G["util"]["Color"]["DimGray"] = Color::DimGray;
	_G["util"]["Color"]["LightSlateGray"] = Color::LightSlateGray;
	_G["util"]["Color"]["SlateGray"] = Color::SlateGray;
	_G["util"]["Color"]["DarkSlateGray"] = Color::DarkSlateGray;
	_G["util"]["Color"]["Black"] = Color::Black;

	auto dataStreamClassDef = luabind::class_<DataStream>("DataStream");
	Lua::DataStream::register_class(dataStreamClassDef);
	dataStreamClassDef.def(luabind::constructor<uint32_t>());
	utilMod[dataStreamClassDef];
}

namespace Lua::doc
{
	void register_library(Lua::Interface &lua);
};

void Game::RegisterLuaLibraries()
{
	NetworkState::RegisterSharedLuaLibraries(GetLuaInterface());

	auto *l = GetLuaState();
	Lua::RegisterLibrary(l,"import",{
		{"import_wrci",Lua::import::import_wrci},
		{"import_wad",Lua::import::import_wad},
		{"import_wrmi",Lua::import::import_wrmi},
		{"import_smd",Lua::import::import_smd},
		{"import_obj",Lua::import::import_obj},
		{"import_model_asset",Lua::import::import_model_asset},
		{"export_model_asset",Lua::import::export_model_asset}
	});

	auto activityEnums = ::Animation::GetActivityEnumRegister().GetEnums();
	auto _G = luabind::globals(l);
	for(auto i=decltype(activityEnums.size()){0};i<activityEnums.size();++i)
		_G["Animation"][activityEnums.at(i)] = i;
	auto eventEnums = ::Animation::GetEventEnumRegister().GetEnums();
	for(auto i=decltype(eventEnums.size()){0};i<eventEnums.size();++i)
		_G["Animation"][eventEnums.at(i)] = i;

	auto *nw = GetNetworkState();
	auto fAddEnum = [l](std::reference_wrapper<const std::string> name,uint32_t id) {
		auto _G = luabind::globals(l);
		_G["Animation"][name.get()] = id;
	};
	auto cbAct = ::Animation::GetActivityEnumRegister().CallOnRegister(fAddEnum);
	auto cbEv = ::Animation::GetEventEnumRegister().CallOnRegister(fAddEnum);
	nw->GetLuaEnumRegisterCallbacks().push_back(cbAct);
	nw->GetLuaEnumRegisterCallbacks().push_back(cbEv);
	AddCallback("OnLuaReleased",FunctionCallback<void>::Create([cbAct,cbEv]() mutable {
		if(cbAct.IsValid())
			cbAct.Remove();
		if(cbEv.IsValid())
			cbEv.Remove();
	}));

	/*static const luaL_Reg funcs_recast[] = {
		{"Test",Lua_recast_Test},
		{NULL,NULL}
	};
	luaL_newlib(GetLuaState(),funcs_recast);
	lua_setglobal(GetLuaState(),"recast");*/

	Lua::nav::register_library(GetLuaInterface());

	auto fileMod = luabind::module(GetLuaState(),"file");
	fileMod[
		luabind::def("open",Lua::file::Open),
		luabind::def("create_directory",Lua::file::CreateDir),
		luabind::def("create_path",Lua::file::CreatePath),
		luabind::def("exists",static_cast<bool(*)(std::string,fsys::SearchFlags)>([](std::string path,fsys::SearchFlags searchFlags) {
			return FileManager::Exists(path,searchFlags);
		})),
		luabind::def("exists",static_cast<bool(*)(std::string)>([](std::string path) {
			return FileManager::Exists(path);
		})),
		luabind::def("delete",Lua::file::Delete),
		luabind::def("find_external_game_asset_files",Lua::file::find_external_game_resource_files,luabind::meta::join<luabind::pure_out_value<3>,luabind::pure_out_value<4>>::type{}),
		luabind::def("open_external_asset_file",Lua::file::open_external_asset_file),
		luabind::def("is_directory",FileManager::IsDir),
		luabind::def("is_directory",static_cast<bool(*)(std::string)>([](std::string path) {return FileManager::IsDir(path);})),
		luabind::def("find",Lua::file::Find,luabind::meta::join<luabind::pure_out_value<4>,luabind::pure_out_value<5>>::type{}),
		luabind::def("find",static_cast<void(*)(lua_State*,const std::string&,luabind::object&,luabind::object&)>([](lua_State *l,const std::string &path,luabind::object &outFiles,luabind::object &outDirs) {
			Lua::file::Find(l,path,fsys::SearchFlags::All,outFiles,outDirs);
		}),luabind::meta::join<luabind::pure_out_value<3>,luabind::pure_out_value<4>>::type{}),
		luabind::def("find_lua_files",Lua::file::FindLuaFiles),
		luabind::def("get_attributes",FileManager::GetFileAttributes),
		luabind::def("get_flags",static_cast<uint64_t(*)(std::string,fsys::SearchFlags)>([](std::string path,fsys::SearchFlags searchFlags) {
			return FileManager::GetFileFlags(path,searchFlags);
		})),
		luabind::def("get_flags",static_cast<uint64_t(*)(std::string)>([](std::string path) {
			return FileManager::GetFileFlags(path);
		})),
		luabind::def("read",Lua::file::Read),
		luabind::def("write",Lua::file::Write),
		luabind::def("get_canonicalized_path",Lua::file::GetCanonicalizedPath),
		luabind::def("get_file_path",ufile::get_path_from_filename),
		luabind::def("get_file_name",ufile::get_file_from_filename),
		luabind::def("get_file_extension",Lua::file::GetFileExtension),
		luabind::def("get_size",FileManager::GetFileSize),
		luabind::def("get_size",static_cast<uint64_t(*)(std::string)>([](std::string path) {
			return FileManager::GetFileSize(path);
		})),
		luabind::def("compare_path",Lua::file::ComparePath),
		luabind::def("remove_file_extension",Lua::file::RemoveFileExtension)
	];

	auto classDefFile = luabind::class_<LFile>("File");
	classDefFile.def("Close",&Lua_LFile_Close);
	classDefFile.def("Size",&Lua_LFile_Size);
	classDefFile.def("ReadLine",&Lua_LFile_ReadLine);
	classDefFile.def("ReadInt32",&Lua_LFile_ReadInt32);
	classDefFile.def("ReadUInt32",&Lua_LFile_ReadUInt32);
	classDefFile.def("ReadInt16",&Lua_LFile_ReadInt16);
	classDefFile.def("ReadUInt16",&Lua_LFile_ReadUInt16);
	classDefFile.def("ReadInt8",&Lua_LFile_ReadInt8);
	classDefFile.def("ReadUInt8",&Lua_LFile_ReadUInt8);
	classDefFile.def("ReadInt64",&Lua_LFile_ReadInt64);
	classDefFile.def("ReadUInt64",&Lua_LFile_ReadUInt64);
	classDefFile.def("ReadBool",&Lua_LFile_ReadBool);
	classDefFile.def("ReadChar",&Lua_LFile_ReadChar);
	classDefFile.def("ReadFloat",&Lua_LFile_ReadFloat);
	classDefFile.def("ReadDouble",&Lua_LFile_ReadDouble);
	classDefFile.def("ReadLongDouble",&Lua_LFile_ReadLongDouble);
	classDefFile.def("ReadVector",&Lua_LFile_ReadVector);
	classDefFile.def("ReadVector2",&Lua_LFile_ReadVector2);
	classDefFile.def("ReadVector4",&Lua_LFile_ReadVector4);
	classDefFile.def("ReadAngles",&Lua_LFile_ReadAngles);
	classDefFile.def("ReadQuaternion",static_cast<void(*)(lua_State*,LFile&)>([](lua_State *l,LFile &f) {
		Lua::Push<Quat>(l,f.Read<Quat>());
	}));
	classDefFile.def("ReadColor",static_cast<Color(*)(lua_State*,LFile&)>([](lua_State *l,LFile &f) -> Color {
		return f.Read<Color>();
	}));
	classDefFile.def("ReadString",static_cast<void(*)(lua_State*,LFile&,uint32_t)>(&Lua_LFile_ReadString));
	classDefFile.def("ReadString",static_cast<void(*)(lua_State*,LFile&)>(&Lua_LFile_ReadString));
	classDefFile.def("WriteInt32",&Lua_LFile_WriteInt32);
	classDefFile.def("WriteUInt32",&Lua_LFile_WriteUInt32);
	classDefFile.def("WriteInt16",&Lua_LFile_WriteInt16);
	classDefFile.def("WriteUInt16",&Lua_LFile_WriteUInt16);
	classDefFile.def("WriteInt8",&Lua_LFile_WriteInt8);
	classDefFile.def("WriteUInt8",&Lua_LFile_WriteUInt8);
	classDefFile.def("WriteInt64",&Lua_LFile_WriteInt64);
	classDefFile.def("WriteUInt64",&Lua_LFile_WriteUInt64);
	classDefFile.def("WriteBool",&Lua_LFile_WriteBool);
	classDefFile.def("WriteChar",&Lua_LFile_WriteChar);
	classDefFile.def("WriteFloat",&Lua_LFile_WriteFloat);
	classDefFile.def("WriteDouble",&Lua_LFile_WriteDouble);
	classDefFile.def("WriteLongDouble",&Lua_LFile_WriteDouble);
	classDefFile.def("WriteVector",&Lua_LFile_WriteVector);
	classDefFile.def("WriteVector2",&Lua_LFile_WriteVector2);
	classDefFile.def("WriteVector4",&Lua_LFile_WriteVector4);
	classDefFile.def("WriteAngles",&Lua_LFile_WriteAngles);
	classDefFile.def("WriteQuaternion",static_cast<void(*)(lua_State*,LFile&,const Quat&)>([](lua_State *l,LFile &f,const Quat &rot) {
		f.Write<Quat>(rot);
	}));
	classDefFile.def("WriteColor",static_cast<void(*)(lua_State*,LFile&,const Color&)>([](lua_State *l,LFile &f,const Color &col) {
		f.Write<Color>(col);
	}));
	classDefFile.def("WriteString",static_cast<void(*)(lua_State*,LFile&,std::string,bool)>(&Lua_LFile_WriteString));
	classDefFile.def("WriteString",static_cast<void(*)(lua_State*,LFile&,std::string)>(&Lua_LFile_WriteString));
	classDefFile.def("Seek",&Lua_LFile_Seek);
	classDefFile.def("Tell",&Lua_LFile_Tell);
	classDefFile.def("Eof",&Lua_LFile_Eof);
	classDefFile.def("IgnoreComments",static_cast<void(*)(lua_State*,LFile&)>(&Lua_LFile_IgnoreComments));
	classDefFile.def("IgnoreComments",static_cast<void(*)(lua_State*,LFile&,std::string)>(&Lua_LFile_IgnoreComments));
	classDefFile.def("IgnoreComments",static_cast<void(*)(lua_State*,LFile&,std::string,std::string)>(&Lua_LFile_IgnoreComments));
	classDefFile.def("Read",static_cast<void(*)(lua_State*,LFile&,uint32_t)>(&Lua_LFile_Read));
	classDefFile.def("Read",static_cast<void(*)(lua_State*,LFile&,::DataStream &ds,uint32_t)>(&Lua_LFile_Read));
	classDefFile.def("Write",static_cast<void(*)(lua_State*,LFile&,::DataStream &ds)>(&Lua_LFile_Write));
	classDefFile.def("Write",static_cast<void(*)(lua_State*,LFile&,::DataStream &ds,uint32_t)>(&Lua_LFile_Write));
	classDefFile.def("GetPath",&Lua_LFile_GetPath);
	fileMod[classDefFile];

	auto timeMod = luabind::module(GetLuaState(),"time");
	timeMod[
		luabind::def("create_timer",Lua::time::create_timer),
		luabind::def("create_simple_timer",Lua::time::create_simple_timer),
		luabind::def("cur_time",Lua::time::cur_time),
		luabind::def("real_time",Lua::time::real_time),
		luabind::def("delta_time",Lua::time::delta_time),
		luabind::def("time_since_epoch",Lua::time::time_since_epoch),
		luabind::def("convert_duration",Lua::time::convert_duration)
	];

	auto classDefTimer = luabind::class_<TimerHandle>("Timer");
	classDefTimer.def("Start",&Lua_Timer_Start);
	classDefTimer.def("Stop",&Lua_Timer_Stop);
	classDefTimer.def("Pause",&Lua_Timer_Pause);
	classDefTimer.def("Remove",&Lua_Timer_Remove);
	classDefTimer.def("IsValid",&Lua_Timer_IsValid);
	classDefTimer.def("GetTimeLeft",&Lua_Timer_GetTimeLeft);
	classDefTimer.def("GetInterval",&Lua_Timer_GetTimeInterval);
	classDefTimer.def("SetInterval",&Lua_Timer_SetTimeInterval);
	classDefTimer.def("GetRepetitionsLeft",&Lua_Timer_GetRepetitionsLeft);
	classDefTimer.def("SetRepetitions",&Lua_Timer_SetRepetitions);
	classDefTimer.def("IsRunning",&Lua_Timer_IsRunning);
	classDefTimer.def("IsPaused",&Lua_Timer_IsPaused);
	classDefTimer.def("Call",&Lua_Timer_Call);
	classDefTimer.def("SetCall",&Lua_Timer_SetCall);
	timeMod[classDefTimer];

	auto boundingVolMod = luabind::module(GetLuaState(),"boundingvolume");
	boundingVolMod[
		luabind::def("get_rotated_aabb",Lua::boundingvolume::GetRotatedAABB,luabind::meta::join<luabind::pure_out_value<4>,luabind::pure_out_value<5>>::type{})
	];

	auto intersectMod = luabind::module(GetLuaState(),"intersect");
	intersectMod[
		luabind::def("aabb_with_aabb",static_cast<int(*)(const Vector3&,const Vector3&,const Vector3&,const Vector3&)>(Intersection::AABBAABB)),
		luabind::def("sphere_with_sphere",Intersection::SphereSphere),
		luabind::def("aabb_with_sphere",Intersection::AABBSphere),
		luabind::def("line_with_aabb",Lua::intersect::line_aabb,luabind::meta::join<luabind::pure_out_value<6>,luabind::pure_out_value<7>,luabind::pure_out_value<8>>::type{}),
		luabind::def("line_with_obb",Lua::intersect::line_obb),
		luabind::def("line_with_obb",static_cast<luabind::object(*)(lua_State*,const Vector3&,const Vector3&,const Vector3&,const Vector3&,bool)>([](lua_State *l,const Vector3 &rayStart,const Vector3 &rayDir,const Vector3 &min,const Vector3 &max,bool precise) {
			return Lua::intersect::line_obb(l,rayStart,rayDir,min,max,precise);
		})),
		luabind::def("line_with_obb",static_cast<luabind::object(*)(lua_State*,const Vector3&,const Vector3&,const Vector3&,const Vector3&)>([](lua_State *l,const Vector3 &rayStart,const Vector3 &rayDir,const Vector3 &min,const Vector3 &max) {
			return Lua::intersect::line_obb(l,rayStart,rayDir,min,max);
		})),
		luabind::def("line_with_mesh",static_cast<void(*)(lua_State*,const Vector3&,const Vector3&,ModelSubMesh&,luabind::object&,luabind::object&,bool,const umath::Transform&)>(Lua::intersect::line_mesh),luabind::meta::join<luabind::pure_out_value<5>,luabind::pure_out_value<6>>::type{}),
		luabind::def("line_with_mesh",static_cast<void(*)(lua_State*,const Vector3&,const Vector3&,ModelSubMesh&,luabind::object&,luabind::object&,bool)>([](lua_State *l,const Vector3 &rayStart,const Vector3 &rayDir,ModelSubMesh &mesh,luabind::object &r0,luabind::object &r1,bool precise) {
			return Lua::intersect::line_mesh(l,rayStart,rayDir,mesh,r0,r1,precise);
		}),luabind::meta::join<luabind::pure_out_value<5>,luabind::pure_out_value<6>>::type{}),
		luabind::def("line_with_mesh",static_cast<void(*)(lua_State*,const Vector3&,const Vector3&,ModelSubMesh&,luabind::object&,luabind::object&)>([](lua_State *l,const Vector3 &rayStart,const Vector3 &rayDir,ModelSubMesh &mesh,luabind::object &r0,luabind::object &r1) {
			return Lua::intersect::line_mesh(l,rayStart,rayDir,mesh,r0,r1);
		}),luabind::meta::join<luabind::pure_out_value<5>,luabind::pure_out_value<6>>::type{}),
		luabind::def("line_with_mesh",static_cast<void(*)(lua_State*,const Vector3&,const Vector3&,ModelMesh&,luabind::object&,luabind::object&,bool,const umath::Transform&)>(Lua::intersect::line_mesh),luabind::meta::join<luabind::pure_out_value<5>,luabind::pure_out_value<6>>::type{}),
		luabind::def("line_with_mesh",static_cast<void(*)(lua_State*,const Vector3&,const Vector3&,ModelMesh&,luabind::object&,luabind::object&,bool)>([](lua_State *l,const Vector3 &rayStart,const Vector3 &rayDir,ModelMesh &mesh,luabind::object &r0,luabind::object &r1,bool precise) {
			return Lua::intersect::line_mesh(l,rayStart,rayDir,mesh,r0,r1,precise);
		}),luabind::meta::join<luabind::pure_out_value<5>,luabind::pure_out_value<6>>::type{}),
		luabind::def("line_with_mesh",static_cast<void(*)(lua_State*,const Vector3&,const Vector3&,ModelMesh&,luabind::object&,luabind::object&)>([](lua_State *l,const Vector3 &rayStart,const Vector3 &rayDir,ModelMesh &mesh,luabind::object &r0,luabind::object &r1) {
			return Lua::intersect::line_mesh(l,rayStart,rayDir,mesh,r0,r1);
		}),luabind::meta::join<luabind::pure_out_value<5>,luabind::pure_out_value<6>>::type{}),
		luabind::def("line_with_mesh",static_cast<void(*)(lua_State*,const Vector3&,const Vector3&,Model&,uint32_t,luabind::object&,luabind::object&,bool,const umath::Transform&)>(Lua::intersect::line_mesh),luabind::meta::join<luabind::pure_out_value<6>,luabind::pure_out_value<7>>::type{}),
		luabind::def("line_with_mesh",static_cast<void(*)(lua_State*,const Vector3&,const Vector3&,Model&,uint32_t,luabind::object&,luabind::object&,bool)>([](lua_State *l,const Vector3 &rayStart,const Vector3 &rayDir,Model &mdl,uint32_t lod,luabind::object &r0,luabind::object &r1,bool precise) {
			return Lua::intersect::line_mesh(l,rayStart,rayDir,mdl,lod,r0,r1,precise);
		}),luabind::meta::join<luabind::pure_out_value<6>,luabind::pure_out_value<7>>::type{}),
		luabind::def("line_with_mesh",static_cast<void(*)(lua_State*,const Vector3&,const Vector3&,Model&,uint32_t,luabind::object&,luabind::object&)>([](lua_State *l,const Vector3 &rayStart,const Vector3 &rayDir,Model &mdl,uint32_t lod,luabind::object &r0,luabind::object &r1) {
			return Lua::intersect::line_mesh(l,rayStart,rayDir,mdl,lod,r0,r1);
		}),luabind::meta::join<luabind::pure_out_value<6>,luabind::pure_out_value<7>>::type{}),
		luabind::def("line_with_mesh",static_cast<void(*)(lua_State*,const Vector3&,const Vector3&,Model&,luabind::table<>,luabind::object&,luabind::object&,bool,const umath::Transform&)>(Lua::intersect::line_mesh),luabind::meta::join<luabind::pure_out_value<6>,luabind::pure_out_value<7>>::type{}),
		luabind::def("line_with_mesh",static_cast<void(*)(lua_State*,const Vector3&,const Vector3&,Model&,luabind::table<>,luabind::object&,luabind::object&,bool)>([](lua_State *l,const Vector3 &rayStart,const Vector3 &rayDir,Model &mdl,luabind::table<> bodyGroups,luabind::object &r0,luabind::object &r1,bool precise) {
			return Lua::intersect::line_mesh(l,rayStart,rayDir,mdl,bodyGroups,r0,r1,precise);
		}),luabind::meta::join<luabind::pure_out_value<6>,luabind::pure_out_value<7>>::type{}),
		luabind::def("line_with_mesh",static_cast<void(*)(lua_State*,const Vector3&,const Vector3&,Model&,luabind::table<>,luabind::object&,luabind::object&)>([](lua_State *l,const Vector3 &rayStart,const Vector3 &rayDir,Model &mdl,luabind::table<> bodyGroups,luabind::object &r0,luabind::object &r1) {
			return Lua::intersect::line_mesh(l,rayStart,rayDir,mdl,bodyGroups,r0,r1);
		}),luabind::meta::join<luabind::pure_out_value<6>,luabind::pure_out_value<7>>::type{}),
		luabind::def("line_with_plane",Lua::intersect::line_plane),
		luabind::def("point_in_aabb",static_cast<bool(*)(const Vector3&,const Vector3&,const Vector3&)>([](const Vector3 &vec,const Vector3 &min,const Vector3 &max) {
			return Intersection::VectorInBounds(vec,min,max);
		})),
		luabind::def("point_in_plane_mesh",Lua::intersect::point_in_plane_mesh),
		luabind::def("sphere_in_plane_mesh",Lua::intersect::sphere_in_plane_mesh),
		luabind::def("aabb_in_plane_mesh",Lua::intersect::aabb_in_plane_mesh),
		luabind::def("sphere_with_cone",Intersection::SphereCone),
		luabind::def("line_with_triangle",Lua::intersect::line_triangle,luabind::meta::join<luabind::pure_out_value<7>,luabind::pure_out_value<8>>::type{}),
		luabind::def("line_with_triangle",static_cast<void(*)(lua_State*,const Vector3&,const Vector3&,const Vector3&,const Vector3&,const Vector3&,luabind::object&,luabind::object&)>([](lua_State *l,const Vector3 &lineOrigin,const Vector3 &lineDir,const Vector3 &v0,const Vector3 &v1,const Vector3 &v2,luabind::object &outT,luabind::object &outUv) {
			Lua::intersect::line_triangle(l,lineOrigin,lineDir,v0,v1,v2,outT,outUv);
		}),luabind::meta::join<luabind::pure_out_value<7>,luabind::pure_out_value<8>>::type{}),
		luabind::def("aabb_with_plane",Intersection::AABBPlane),
		luabind::def("aabb_with_triangle",Intersection::AABBTriangle),
		luabind::def("obb_with_plane",Intersection::OBBPlane),
		luabind::def("sphere_with_plane",Intersection::SpherePlane)
	];

	auto modGeometry = luabind::module_(GetLuaState(),"geometry");
	modGeometry[
		luabind::def("closest_point_on_aabb_to_point",Lua::geometry::closest_point_on_aabb_to_point),
		luabind::def("closest_points_between_lines",Lua::geometry::closest_points_between_lines,luabind::meta::join<luabind::pure_out_value<5>,luabind::pure_out_value<6>,luabind::pure_out_value<7>>::type{}),
		luabind::def("closest_point_on_plane_to_point",Lua::geometry::closest_point_on_plane_to_point),
		luabind::def("closest_point_on_triangle_to_point",Lua::geometry::closest_point_on_triangle_to_point),
		luabind::def("smallest_enclosing_sphere",Lua::geometry::smallest_enclosing_sphere,luabind::meta::join<luabind::pure_out_value<3>,luabind::pure_out_value<4>>::type{}),
		luabind::def("closest_point_on_line_to_point",Geometry::ClosestPointOnLineToPoint),
		luabind::def("closest_point_on_line_to_point",static_cast<Vector3(*)(const Vector3&,const Vector3&,const Vector3&)>([](const Vector3 &start,const Vector3 &end,const Vector3 &p) {
			return Geometry::ClosestPointOnLineToPoint(start,end,p);
		})),
		luabind::def("closest_point_on_sphere_to_line",Geometry::ClosestPointOnSphereToLine),
		luabind::def("closest_point_on_sphere_to_line",static_cast<Vector3(*)(const Vector3&,float,const Vector3&,const Vector3&)>([](const Vector3 &origin,float radius,const Vector3 &start,const Vector3 &end) {
			return Geometry::ClosestPointOnSphereToLine(origin,radius,start,end);
		})),
		luabind::def("get_triangle_winding_order",static_cast<WindingOrder(*)(const Vector3&,const Vector3&,const Vector3&,const Vector3&)>(::Geometry::get_triangle_winding_order)),
		luabind::def("get_triangle_winding_order",static_cast<WindingOrder(*)(const Vector2&,const Vector2&,const Vector2&)>(::Geometry::get_triangle_winding_order)),
		luabind::def("generate_truncated_cone_mesh",Lua::geometry::generate_truncated_cone_mesh,luabind::meta::join<luabind::pure_out_value<7>,luabind::pure_out_value<8>,luabind::pure_out_value<9>>::type{}),
		luabind::def("generate_truncated_cone_mesh",static_cast<void(*)(lua_State*,const Vector3&,float,const Vector3&,float,float,luabind::object&,luabind::object&,luabind::object&,uint32_t,bool,bool)>(
			[](
				lua_State *l,const Vector3 &origin,float startRadius,const Vector3 &dir,float dist,float endRadius,
				luabind::object &outVerts,luabind::object &outTris,luabind::object &outNormals,
				uint32_t segmentCount,bool caps,bool generateTriangles
			) {
			Lua::geometry::generate_truncated_cone_mesh(l,origin,startRadius,dir,dist,endRadius,outVerts,outTris,outNormals,segmentCount,caps,generateTriangles);
		}),luabind::meta::join<luabind::pure_out_value<7>,luabind::pure_out_value<8>,luabind::pure_out_value<9>>::type{}),
		luabind::def("generate_truncated_cone_mesh",static_cast<void(*)(lua_State*,const Vector3&,float,const Vector3&,float,float,luabind::object&,luabind::object&,luabind::object&,uint32_t,bool)>(
			[](
				lua_State *l,const Vector3 &origin,float startRadius,const Vector3 &dir,float dist,float endRadius,
				luabind::object &outVerts,luabind::object &outTris,luabind::object &outNormals,
				uint32_t segmentCount,bool caps
			) {
			Lua::geometry::generate_truncated_cone_mesh(l,origin,startRadius,dir,dist,endRadius,outVerts,outTris,outNormals,segmentCount,caps);
		}),luabind::meta::join<luabind::pure_out_value<7>,luabind::pure_out_value<8>,luabind::pure_out_value<9>>::type{}),
		luabind::def("generate_truncated_cone_mesh",static_cast<void(*)(lua_State*,const Vector3&,float,const Vector3&,float,float,luabind::object&,luabind::object&,luabind::object&,uint32_t)>(
			[](
				lua_State *l,const Vector3 &origin,float startRadius,const Vector3 &dir,float dist,float endRadius,
				luabind::object &outVerts,luabind::object &outTris,luabind::object &outNormals,
				uint32_t segmentCount
			) {
			Lua::geometry::generate_truncated_cone_mesh(l,origin,startRadius,dir,dist,endRadius,outVerts,outTris,outNormals,segmentCount);
		}),luabind::meta::join<luabind::pure_out_value<7>,luabind::pure_out_value<8>,luabind::pure_out_value<9>>::type{}),
		luabind::def("generate_truncated_cone_mesh",static_cast<void(*)(lua_State*,const Vector3&,float,const Vector3&,float,float,luabind::object&,luabind::object&,luabind::object&)>(
			[](
				lua_State *l,const Vector3 &origin,float startRadius,const Vector3 &dir,float dist,float endRadius,
				luabind::object &outVerts,luabind::object &outTris,luabind::object &outNormals
			) {
			Lua::geometry::generate_truncated_cone_mesh(l,origin,startRadius,dir,dist,endRadius,outVerts,outTris,outNormals);
		}),luabind::meta::join<luabind::pure_out_value<7>,luabind::pure_out_value<8>,luabind::pure_out_value<9>>::type{}),
		luabind::def("calc_face_normal",Geometry::CalcFaceNormal),
		luabind::def("calc_volume_of_triangle",::Geometry::calc_volume_of_triangle),
		luabind::def("calc_volume_of_polyhedron",Lua::geometry::calc_volume_of_polyhedron),
		luabind::def("calc_center_of_mass",Lua::geometry::calc_center_of_mass,luabind::meta::join<luabind::pure_out_value<4>,luabind::pure_out_value<5>>::type{}),
		luabind::def("calc_triangle_area",static_cast<float(*)(const Vector2&,const Vector2&,const Vector2&,bool)>(Geometry::calc_triangle_area)),
			luabind::def("calc_triangle_area",static_cast<float(*)(const Vector2&,const Vector2&,const Vector2&)>([](const Vector2 &p0,const Vector2 &p1,const Vector2 &p2) {
			return Geometry::calc_triangle_area(p0,p1,p2,false);
		})),
		luabind::def("calc_barycentric_coordinates",static_cast<::Vector2(*)(const Vector3&,const Vector2&,const Vector3&,const Vector2&,const Vector3&,const Vector2&,const Vector3&)>(Lua::geometry::calc_barycentric_coordinates)),
		luabind::def("calc_barycentric_coordinates",static_cast<::Vector2(*)(const Vector3&,const Vector3&,const Vector3&,const Vector3&)>(Lua::geometry::calc_barycentric_coordinates)),
		luabind::def("calc_rotation_between_planes",::Geometry::calc_rotation_between_planes),
		luabind::def("get_side_of_point_to_line",::Geometry::get_side_of_point_to_line),
		luabind::def("get_side_of_point_to_plane",::Geometry::get_side_of_point_to_plane),
		luabind::def("get_outline_vertices",Lua::geometry::get_outline_vertices),
		//luabind::def("triangulate_point_cloud",Lua::geometry::triangulate_point_cloud),
		luabind::def("triangulate",Lua::geometry::triangulate)
	];
	Lua::RegisterLibraryEnums(GetLuaState(),"geometry",{
		{"WINDING_ORDER_CLOCKWISE",umath::to_integral(WindingOrder::Clockwise)},
		{"WINDING_ORDER_COUNTER_CLOCKWISE",umath::to_integral(WindingOrder::CounterClockwise)},
		
		{"LINE_SIDE_LEFT",umath::to_integral(Geometry::LineSide::Left)},
		{"LINE_SIDE_RIGHT",umath::to_integral(Geometry::LineSide::Right)},
		{"LINE_SIDE_ON_LINE",umath::to_integral(Geometry::LineSide::OnLine)},

		{"PLANE_SIDE_FRONT",umath::to_integral(Geometry::PlaneSide::Front)},
		{"PLANE_SIDE_BACK",umath::to_integral(Geometry::PlaneSide::Back)},
		{"PLANE_SIDE_ON_PLANE",umath::to_integral(Geometry::PlaneSide::OnPlane)}
	});

	auto modSweep = luabind::module_(GetLuaState(),"sweep");
	modSweep[
		luabind::def("aabb_with_aabb",Lua::sweep::AABBWithAABB,luabind::meta::join<luabind::pure_out_value<7>,luabind::pure_out_value<8>,luabind::pure_out_value<9>,luabind::pure_out_value<10>>::type{}),
		luabind::def("aabb_with_plane",Lua::sweep::AABBWithPlane,luabind::meta::join<luabind::pure_out_value<6>,luabind::pure_out_value<7>>::type{})
	];

	auto modMat = luabind::module_(GetLuaState(),"matrix");
	modMat[
		luabind::def("create_from_axis_angle",umat::create_from_axis_angle),
		luabind::def("create_from_axes",umat::create_from_axes),
		luabind::def("create_orthogonal_matrix",Lua::matrix::create_orthogonal_matrix),
		luabind::def("create_perspective_matrix",Lua::matrix::create_perspective_matrix),
		luabind::def("create_look_at_matrix",glm::lookAtRH<float,glm::packed_highp>),
		luabind::def("calc_covariance_matrix",static_cast<::Mat3(*)(lua_State*,luabind::table<>,const Vector3&)>(Lua::matrix::calc_covariance_matrix)),
		luabind::def("calc_covariance_matrix",static_cast<::Mat3(*)(lua_State*,luabind::table<>)>(Lua::matrix::calc_covariance_matrix))
	];

	Lua::RegisterLibrary(GetLuaState(),"mesh",{
		{"generate_convex_hull",Lua::mesh::generate_convex_hull},
		{"calc_smallest_enclosing_bbox",Lua::mesh::calc_smallest_enclosing_bbox}
	});

	Lua::RegisterLibrary(GetLuaState(),"regex",{
		{"match",Lua::regex::match},
		{"search",Lua::regex::search},
		{"replace",Lua::regex::replace}
	});

	auto regexMod = luabind::module(GetLuaState(),"regex");
	auto classDefRegexResult = luabind::class_<std::match_results<const char*>>("Result");
	classDefRegexResult.def(luabind::constructor<>());
	classDefRegexResult.def(luabind::tostring(luabind::self));
	classDefRegexResult.def("HasMatch",&Lua::regex::RegexResult::HasMatch);
	classDefRegexResult.def("GetMatchCount",&Lua::regex::RegexResult::GetMatchCount);
	classDefRegexResult.def("GetLength",&Lua::regex::RegexResult::GetLength);
	classDefRegexResult.def("GetPosition",&Lua::regex::RegexResult::GetPosition);
	classDefRegexResult.def("GetString",&Lua::regex::RegexResult::GetString);
	classDefRegexResult.def("SetFormat",&Lua::regex::RegexResult::SetFormat);
	regexMod[classDefRegexResult];

	Lua::RegisterLibraryEnums(GetLuaState(),"regex",{
		{"MATCH_DEFAULT",umath::to_integral(std::regex_constants::match_default)},
		{"MATCH_NOT_BOL",umath::to_integral(std::regex_constants::match_not_bol)},
		{"MATCH_NOT_EOL",umath::to_integral(std::regex_constants::match_not_eol)},
		{"MATCH_NOT_BOW",umath::to_integral(std::regex_constants::match_not_bow)},
		{"MATCH_NOT_EOW",umath::to_integral(std::regex_constants::match_not_eow)},
		{"MATCH_ANY",umath::to_integral(std::regex_constants::match_any)},
		{"MATCH_NOT_NULL",umath::to_integral(std::regex_constants::match_not_null)},
		{"MATCH_CONTINUOUS",umath::to_integral(std::regex_constants::match_continuous)},
		{"MATCH_PREV_AVAIL",umath::to_integral(std::regex_constants::match_prev_avail)},

		{"FORMAT_DEFAULT",umath::to_integral(std::regex_constants::format_default)},
		{"FORMAT_SED",umath::to_integral(std::regex_constants::format_sed)},
		{"FORMAT_NO_COPY",umath::to_integral(std::regex_constants::format_no_copy)},
		{"FORMAT_FIRST_ONLY",umath::to_integral(std::regex_constants::format_first_only)}
	});
	Lua::physenv::register_library(GetLuaInterface());
	Lua::doc::register_library(GetLuaInterface());
}