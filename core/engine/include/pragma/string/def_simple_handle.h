/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __DEF_SIMPLE_HANDLE_H__
#define __DEF_SIMPLE_HANDLE_H__

#define DECLARE_SIMPLE_BASE_HANDLE(classdef,classname,localname) \
	class classdef localname \
	{ \
	protected: \
		classname *m_target; \
	public: \
		localname##(classname *t); \
		bool operator==(const localname *other) const; \
		bool operator!=(const localname *other) const; \
		virtual classname *operator->() override; \
		virtual classname *get() const override; \
	};

#define DECLARE_SIMPLE_DERIVED_HANDLE(classdef,baseclass,classname,localname) \
	class classdef localname \
		: public baseclass \
	{ \
	protected: \
		classname *m_target; \
	public: \
		localname##(classname *t); \
		virtual classname *operator->() override; \
		virtual classname *get() const override; \
	};

#define DEFINE_SIMPLE_BASE_HANDLE(classdef,classname,localname) \
	classdef localname##::##localname##(classname *t) \
		: m_target(t) \
	{} \
	classdef bool localname##::operator==(const localname *other) const {return (this == other) ? true : false;} \
	classdef bool localname##::operator!=(const localname *other) const {return (this != other) ? true : false;} \
	classdef classname *##localname##::operator->() {return get();} \
	classdef classname *##localname##::get() const {return m_target;}

#define DEFINE_SIMPLE_DERIVED_HANDLE(classdef,baseclass,classname,localname) \
	classdef localname##::##localname##(classname *t) \
		: baseclass##(t),m_target(t) \
	{} \
	classdef classname *##localname##::operator->() {return get();} \
	classdef classname *##localname##::get() const {return m_target;}

#endif