#include "lxml.h"
#include <luasystem.h>
#include <luainterface.hpp>
#include <fsys/filesystem.h>
#include <mathutil/umath.h>
#include <rapidxml.hpp>
#include <rapidxml_utils.hpp>
#include <rapidxml_print.hpp>
#include <sstream>
#include <pragma/ishared.hpp>

namespace Lua
{
	namespace xml
	{
		static std::string node_to_string(const rapidxml::xml_node<> &node);
		static const char *allocate_string(rapidxml::xml_document<> &doc,const std::string &str);
		class XMLBase
		{
		protected:
			std::shared_ptr<rapidxml::xml_document<>> m_doc;
			rapidxml::xml_base<char> *m_object;
			XMLBase(const std::shared_ptr<rapidxml::xml_document<>> &doc,rapidxml::xml_base<char> *o);
		public:
			std::shared_ptr<rapidxml::xml_document<>> &GetDocument();
			virtual rapidxml::xml_base<char> *operator->();
			virtual rapidxml::xml_base<char> *get();
		};
		class XMLNode
			: public XMLBase
		{
		public:
			XMLNode(const std::shared_ptr<rapidxml::xml_document<>> &doc,rapidxml::xml_node<> *node);
			virtual rapidxml::xml_node<> *operator->() override;
			virtual rapidxml::xml_node<> *get() override;
		};
		class XMLAttribute
			: public XMLBase
		{
		private:
			XMLNode &m_node;
		public:
			XMLAttribute(XMLNode &node,rapidxml::xml_attribute<char> *attr);
			XMLNode &GetNode();
			virtual rapidxml::xml_attribute<char> *operator->() override;
			virtual rapidxml::xml_attribute<char> *get() override;
		};
		namespace Base
		{
			static void GetName(lua_State *l,XMLBase &base);
			static void GetValue(lua_State *l,XMLBase &base);
			static void SetName(lua_State *l,XMLBase &base,const std::string &name);
			static void SetValue(lua_State *l,XMLBase &base,const std::string &val);
			static void GetParent(lua_State *l,XMLBase &base);
		};
		namespace Node
		{
			static void AppendAttribute(lua_State *l,XMLNode &node,const std::string &name,const std::string &value);
			static void AppendAttribute(lua_State *l,XMLNode &node,XMLAttribute &childAttr);
			static void PrependAttribute(lua_State *l,XMLNode &node,const std::string &name,const std::string &value);
			static void PrependAttribute(lua_State *l,XMLNode &node,XMLAttribute &childAttr);
			static void AppendNode(lua_State *l,XMLNode &node,uint32_t type,const std::string &name,const std::string &value);
			static void AppendNode(lua_State *l,XMLNode &node,XMLNode &childNode);
			static void PrependNode(lua_State *l,XMLNode &node,uint32_t type,const std::string &name,const std::string &value);
			static void PrependNode(lua_State *l,XMLNode &node,XMLNode &childNode);
			static void GetFirstNode(lua_State *l,XMLNode &node,const std::string &name,bool bMatchCase);
			static void GetFirstNode(lua_State *l,XMLNode &node,const std::string &name);
			static void GetFirstNode(lua_State *l,XMLNode &node);
			static void GetLastNode(lua_State *l,XMLNode &node,const std::string &name,bool bMatchCase);
			static void GetLastNode(lua_State *l,XMLNode &node,const std::string &name);
			static void GetLastNode(lua_State *l,XMLNode &node);
			static void GetFirstAttribute(lua_State *l,XMLNode &node,const std::string &name,bool bMatchCase);
			static void GetFirstAttribute(lua_State *l,XMLNode &node,const std::string &name);
			static void GetFirstAttribute(lua_State *l,XMLNode &node);
			static void GetLastAttribute(lua_State *l,XMLNode &node,const std::string &name,bool bMatchCase);
			static void GetLastAttribute(lua_State *l,XMLNode &node,const std::string &name);
			static void GetLastAttribute(lua_State *l,XMLNode &node);
			static void GetNextSibling(lua_State *l,XMLNode &node,const std::string &name,bool bMatchCase);
			static void GetNextSibling(lua_State *l,XMLNode &node,const std::string &name);
			static void GetNextSibling(lua_State *l,XMLNode &node);
			static void GetPreviousSibling(lua_State *l,XMLNode &node,const std::string &name,bool bMatchCase);
			static void GetPreviousSibling(lua_State *l,XMLNode &node,const std::string &name);
			static void GetPreviousSibling(lua_State *l,XMLNode &node);
			static void GetType(lua_State *l,XMLNode &node);
			static void InsertAttribute(lua_State *l,XMLNode &node,XMLAttribute &attrWhere,const std::string &name,const std::string &value);
			static void InsertAttribute(lua_State *l,XMLNode &node,XMLAttribute &attrWhere,XMLAttribute &childAttr);
			static void InsertNode(lua_State *l,XMLNode &node,XMLNode &nodeWhere,uint32_t type,const std::string &name,const std::string &value);
			static void InsertNode(lua_State *l,XMLNode &node,XMLNode &nodeWhere,XMLNode &childNode);
			static void RemoveAllAttributes(lua_State *l,XMLNode &node);
			static void RemoveAllNodes(lua_State *l,XMLNode &node);
			static void RemoveAttribute(lua_State *l,XMLNode &node,XMLAttribute &attr);
			static void RemoveFirstAttribute(lua_State *l,XMLNode &node);
			static void RemoveFirstNode(lua_State *l,XMLNode &node);
			static void RemoveLastAttribute(lua_State *l,XMLNode &node);
			static void RemoveLastNode(lua_State *l,XMLNode &node);
			static void RemoveNode(lua_State *l,XMLNode &node,XMLNode &nodeOther);
			static void ToString(lua_State *l,XMLNode &node);
			static void Save(lua_State *l,XMLNode &node,const std::string &fname);
		};
		namespace Attribute
		{
			static void GetNextAttribute(lua_State *l,XMLAttribute &attr,const std::string &name,bool bMatchCase);
			static void GetNextAttribute(lua_State *l,XMLAttribute &attr,const std::string &name);
			static void GetNextAttribute(lua_State *l,XMLAttribute &attr);
			static void GetPreviousAttribute(lua_State *l,XMLAttribute &attr,const std::string &name,bool bMatchCase);
			static void GetPreviousAttribute(lua_State *l,XMLAttribute &attr,const std::string &name);
			static void GetPreviousAttribute(lua_State *l,XMLAttribute &attr);
		};
	};
};

std::string Lua::xml::node_to_string(const rapidxml::xml_node<> &node)
{
	std::stringstream stream;
	std::ostream_iterator<char> iter(stream);
	rapidxml::print(iter,node,0);
	return stream.str();
}

const char *Lua::xml::allocate_string(rapidxml::xml_document<> &doc,const std::string &str)
{
	return doc.allocate_string(str.c_str(),str.length() +1); // +1 = null-terminator
}

Lua::xml::XMLBase::XMLBase(const std::shared_ptr<rapidxml::xml_document<>> &doc,rapidxml::xml_base<char> *o)
	: m_doc(doc),m_object(o)
{}

Lua::xml::XMLNode::XMLNode(const std::shared_ptr<rapidxml::xml_document<>> &doc,rapidxml::xml_node<> *node)
	: XMLBase(doc,node)
{}

Lua::xml::XMLAttribute::XMLAttribute(XMLNode &node,rapidxml::xml_attribute<char> *attr)
	: XMLBase(node.GetDocument(),attr),m_node(node)
{}

std::shared_ptr<rapidxml::xml_document<>> &Lua::xml::XMLBase::GetDocument() {return m_doc;}

rapidxml::xml_base<char> *Lua::xml::XMLBase::operator->() {return get();}
rapidxml::xml_base<char> *Lua::xml::XMLBase::get() {return m_object;}

rapidxml::xml_node<> *Lua::xml::XMLNode::operator->() {return get();}
rapidxml::xml_node<> *Lua::xml::XMLNode::get() {return static_cast<rapidxml::xml_node<>*>(m_object);}

Lua::xml::XMLNode &Lua::xml::XMLAttribute::GetNode() {return m_node;}
rapidxml::xml_attribute<char> *Lua::xml::XMLAttribute::operator->() {return get();}
rapidxml::xml_attribute<char> *Lua::xml::XMLAttribute::get() {return static_cast<rapidxml::xml_attribute<char>*>(m_object);}

//////////////////////

void Lua::xml::register_lua_library(Lua::Interface &l)
{
	ishared::load_documentation_file("doc/module_xml.wdd");

	Lua::RegisterLibrary(l.GetState(),"xml",{
		{"parse",Lua::xml::lib::parse},
		{"load",Lua::xml::lib::load},
		{"create",Lua::xml::lib::create}
	});

	auto &modXML = l.RegisterLibrary("xml");
	auto classDefBase = luabind::class_<XMLBase>("Base");
	classDefBase.def("GetName",Base::GetName);
	classDefBase.def("GetValue",Base::GetValue);
	classDefBase.def("SetName",Base::SetName);
	classDefBase.def("SetValue",Base::SetValue);
	classDefBase.def("GetParent",Base::GetParent);
	modXML[classDefBase];

	auto classDefNode = luabind::class_<XMLNode,XMLBase>("Node");
	classDefNode.def("AppendAttribute",static_cast<void(*)(lua_State*,XMLNode&,const std::string&,const std::string&)>(&Node::AppendAttribute));
	classDefNode.def("AppendAttribute",static_cast<void(*)(lua_State*,XMLNode&,XMLAttribute&)>(&Node::AppendAttribute));
	classDefNode.def("PrependAttribute",static_cast<void(*)(lua_State*,XMLNode&,const std::string&,const std::string&)>(&Node::PrependAttribute));
	classDefNode.def("PrependAttribute",static_cast<void(*)(lua_State*,XMLNode&,XMLAttribute&)>(&Node::PrependAttribute));
	classDefNode.def("AppendNode",static_cast<void(*)(lua_State*,XMLNode&,uint32_t,const std::string&,const std::string&)>(&Node::AppendNode));
	classDefNode.def("AppendNode",static_cast<void(*)(lua_State*,XMLNode&,XMLNode&)>(&Node::AppendNode));
	classDefNode.def("PrependNode",static_cast<void(*)(lua_State*,XMLNode&,uint32_t,const std::string&,const std::string&)>(&Node::PrependNode));
	classDefNode.def("PrependNode",static_cast<void(*)(lua_State*,XMLNode&,XMLNode&)>(&Node::PrependNode));
	classDefNode.def("GetFirstNode",(void(*)(lua_State*,XMLNode&,const std::string&,bool))&Node::GetFirstNode);
	classDefNode.def("GetFirstNode",(void(*)(lua_State*,XMLNode&,const std::string&))&Node::GetFirstNode);
	classDefNode.def("GetFirstNode",(void(*)(lua_State*,XMLNode&))&Node::GetFirstNode);
	classDefNode.def("GetLastNode",(void(*)(lua_State*,XMLNode&,const std::string&,bool))&Node::GetLastNode);
	classDefNode.def("GetLastNode",(void(*)(lua_State*,XMLNode&,const std::string&))&Node::GetLastNode);
	classDefNode.def("GetLastNode",(void(*)(lua_State*,XMLNode&))&Node::GetLastNode);
	classDefNode.def("GetFirstAttribute",(void(*)(lua_State*,XMLNode&,const std::string&,bool))&Node::GetFirstAttribute);
	classDefNode.def("GetFirstAttribute",(void(*)(lua_State*,XMLNode&,const std::string&))&Node::GetFirstAttribute);
	classDefNode.def("GetFirstAttribute",(void(*)(lua_State*,XMLNode&))&Node::GetFirstAttribute);
	classDefNode.def("GetLastAttribute",(void(*)(lua_State*,XMLNode&,const std::string&,bool))&Node::GetLastAttribute);
	classDefNode.def("GetLastAttribute",(void(*)(lua_State*,XMLNode&,const std::string&))&Node::GetLastAttribute);
	classDefNode.def("GetLastAttribute",(void(*)(lua_State*,XMLNode&))&Node::GetLastAttribute);
	classDefNode.def("GetNextSibling",(void(*)(lua_State*,XMLNode&,const std::string&,bool))&Node::GetNextSibling);
	classDefNode.def("GetNextSibling",(void(*)(lua_State*,XMLNode&,const std::string&))&Node::GetNextSibling);
	classDefNode.def("GetNextSibling",(void(*)(lua_State*,XMLNode&))&Node::GetNextSibling);
	classDefNode.def("GetPreviousSibling",(void(*)(lua_State*,XMLNode&,const std::string&,bool))&Node::GetPreviousSibling);
	classDefNode.def("GetPreviousSibling",(void(*)(lua_State*,XMLNode&,const std::string&))&Node::GetPreviousSibling);
	classDefNode.def("GetPreviousSibling",(void(*)(lua_State*,XMLNode&))&Node::GetPreviousSibling);
	classDefNode.def("GetType",Node::GetType);
	classDefNode.def("InsertAttribute",static_cast<void(*)(lua_State*,XMLNode&,XMLAttribute&,const std::string&,const std::string&)>(&Node::InsertAttribute));
	classDefNode.def("InsertAttribute",static_cast<void(*)(lua_State*,XMLNode&,XMLAttribute&,XMLAttribute&)>(&Node::InsertAttribute));
	classDefNode.def("InsertNode",static_cast<void(*)(lua_State*,XMLNode&,XMLNode&,uint32_t,const std::string&,const std::string&)>(&Node::InsertNode));
	classDefNode.def("InsertNode",static_cast<void(*)(lua_State*,XMLNode&,XMLNode&,XMLNode&)>(&Node::InsertNode));
	classDefNode.def("RemoveAllAttributes",Node::RemoveAllAttributes);
	classDefNode.def("RemoveAllNodes",Node::RemoveAllNodes);
	classDefNode.def("RemoveAttribute",Node::RemoveAttribute);
	classDefNode.def("RemoveFirstAttribute",Node::RemoveFirstAttribute);
	classDefNode.def("RemoveFirstNode",Node::RemoveFirstNode);
	classDefNode.def("RemoveLastAttribute",Node::RemoveLastAttribute);
	classDefNode.def("RemoveLastNode",Node::RemoveLastNode);
	classDefNode.def("RemoveNode",Node::RemoveNode);
	classDefNode.def("ToString",Node::ToString);
	classDefNode.def("Save",Node::Save);
	classDefNode.add_static_constant("TYPE_DOCUMENT",umath::to_integral(rapidxml::node_type::node_document));
	classDefNode.add_static_constant("TYPE_ELEMENT",umath::to_integral(rapidxml::node_type::node_element));
	classDefNode.add_static_constant("TYPE_DATA",umath::to_integral(rapidxml::node_type::node_data));
	classDefNode.add_static_constant("TYPE_CDATA",umath::to_integral(rapidxml::node_type::node_cdata));
	classDefNode.add_static_constant("TYPE_COMMENT",umath::to_integral(rapidxml::node_type::node_comment));
	classDefNode.add_static_constant("TYPE_DECLARATION",umath::to_integral(rapidxml::node_type::node_declaration));
	classDefNode.add_static_constant("TYPE_DOCTYPE",umath::to_integral(rapidxml::node_type::node_doctype));
	classDefNode.add_static_constant("TYPE_PI",umath::to_integral(rapidxml::node_type::node_pi));
	modXML[classDefNode];

	auto classDefAttribute = luabind::class_<XMLAttribute,XMLBase>("Attribute");
	classDefAttribute.def("GetNextAttribute",(void(*)(lua_State*,XMLAttribute&,const std::string&,bool))&Attribute::GetNextAttribute);
	classDefAttribute.def("GetNextAttribute",(void(*)(lua_State*,XMLAttribute&,const std::string&))&Attribute::GetNextAttribute);
	classDefAttribute.def("GetNextAttribute",(void(*)(lua_State*,XMLAttribute&))&Attribute::GetNextAttribute);
	classDefAttribute.def("GetPreviousAttribute",(void(*)(lua_State*,XMLAttribute&,const std::string&,bool))&Attribute::GetPreviousAttribute);
	classDefAttribute.def("GetPreviousAttribute",(void(*)(lua_State*,XMLAttribute&,const std::string&))&Attribute::GetPreviousAttribute);
	classDefAttribute.def("GetPreviousAttribute",(void(*)(lua_State*,XMLAttribute&))&Attribute::GetPreviousAttribute);
	modXML[classDefAttribute];
}
static int32_t parse_xml(lua_State *l,const std::shared_ptr<rapidxml::xml_document<>> &xmlDoc,char *pcontent)
{
	try
	{
		xmlDoc->parse<0>(pcontent);
		Lua::PushBool(l,true);
		Lua::Push<Lua::xml::XMLNode>(l,Lua::xml::XMLNode(xmlDoc,xmlDoc.get()));
		return 2;

	}
	catch(const rapidxml::parse_error &err)
	{
		Lua::PushBool(l,false);
		Lua::PushString(l,err.what());
		auto *pWhere = err.where<char>();
		auto pos = pWhere -pcontent;
		decltype(pos) line = 0;
		decltype(pos) lastLineStart = 0;
		for(auto i=decltype(pos){0};i<pos;++i)
		{
			if(pcontent[i] == '\n')
			{
				++line;
				lastLineStart = i;
			}
		}
		Lua::PushInt(l,line +1);
		Lua::PushInt(l,pos -lastLineStart);
		return 4;
	}
}
int Lua::xml::lib::create(lua_State *l)
{
	auto xmlDoc = std::make_shared<rapidxml::xml_document<>>();
	Lua::Push<XMLNode>(l,XMLNode(xmlDoc,xmlDoc.get()));
	return 1;
}
int Lua::xml::lib::parse(lua_State *l)
{
	auto *str = Lua::CheckString(l,1);
	auto xmlDoc = std::make_shared<rapidxml::xml_document<>>();
	auto *pcontent = xmlDoc->allocate_string(str,strlen(str) +1); // +1 = null-terminator
	if(pcontent == nullptr)
	{
		Lua::PushBool(l,false);
		return 1;
	}
	return parse_xml(l,xmlDoc,pcontent);
}
int Lua::xml::lib::load(lua_State *l)
{
	auto *fname = Lua::CheckString(l,1);

	auto f = FileManager::OpenFile(fname,"r");
	if(f == nullptr)
	{
		Lua::PushBool(l,false);
		Lua::PushString(l,"Unable to open file!");
		Lua::PushInt(l,-1);
		Lua::PushInt(l,-1);
		return 4;
	}
	auto content = f->ReadString();

	auto xmlDoc = std::make_shared<rapidxml::xml_document<>>();
	auto *pcontent = xmlDoc->allocate_string(content.c_str(),content.length() +1); // +1 = null-terminator
	if(pcontent == nullptr)
	{
		Lua::PushBool(l,false);
		Lua::PushString(l,"Unable to allocate string for file contents!");
		Lua::PushInt(l,-1);
		Lua::PushInt(l,-1);
		return 4;
	}
	return parse_xml(l,xmlDoc,pcontent);
}

////////////////////////////

void Lua::xml::Base::GetName(lua_State *l,XMLBase &base)
{
	Lua::PushString(l,std::string(base->name(),base->name_size()));
}
void Lua::xml::Base::GetValue(lua_State *l,XMLBase &base)
{
	Lua::PushString(l,std::string(base->value(),base->value_size()));
}
void Lua::xml::Base::SetName(lua_State *l,XMLBase &base,const std::string &name)
{
	auto &doc = base.GetDocument();
	base->name(allocate_string(*doc,name));
}
void Lua::xml::Base::SetValue(lua_State *l,XMLBase &base,const std::string &val)
{
	auto &doc = base.GetDocument();
	base->value(allocate_string(*doc,val));
}
void Lua::xml::Base::GetParent(lua_State *l,XMLBase &base)
{
	auto *parent = base->parent();
	if(parent == nullptr)
		return;
	Lua::Push<XMLNode>(l,XMLNode(base.GetDocument(),parent));
}

////////////////////////////

void Lua::xml::Node::AppendAttribute(lua_State *l,XMLNode &node,const std::string &name,const std::string &value)
{
	auto &doc = node.GetDocument();

	auto *attr = doc->allocate_attribute(allocate_string(*doc,name),allocate_string(*doc,value));
	if(attr == nullptr)
		return;
	node->append_attribute(attr);
	Lua::Push<XMLAttribute>(l,XMLAttribute(node,attr));
}
void Lua::xml::Node::AppendAttribute(lua_State *l,XMLNode &node,XMLAttribute &childAttr)
{
	node->append_attribute(childAttr.get());
}
void Lua::xml::Node::PrependAttribute(lua_State *l,XMLNode &node,const std::string &name,const std::string &value)
{
	auto &doc = node.GetDocument();
	auto *attr = doc->allocate_attribute(allocate_string(*doc,name),allocate_string(*doc,value));
	if(attr == nullptr)
		return;
	node->prepend_attribute(attr);
	Lua::Push<XMLAttribute>(l,XMLAttribute(node,attr));
}
void Lua::xml::Node::PrependAttribute(lua_State *l,XMLNode &node,XMLAttribute &childAttr)
{
	node->prepend_attribute(childAttr.get());
}
void Lua::xml::Node::AppendNode(lua_State *l,XMLNode &node,uint32_t type,const std::string &name,const std::string &value)
{
	auto &doc = node.GetDocument();
	auto *nnode = doc->allocate_node(static_cast<rapidxml::node_type>(type),allocate_string(*doc,name),allocate_string(*doc,value));
	if(nnode == nullptr)
		return;
	node->append_node(nnode);
	Lua::Push<XMLNode>(l,XMLNode(node.GetDocument(),nnode));
}
void Lua::xml::Node::AppendNode(lua_State *l,XMLNode &node,XMLNode &childNode)
{
	node->append_node(childNode.get());
}
void Lua::xml::Node::PrependNode(lua_State *l,XMLNode &node,uint32_t type,const std::string &name,const std::string &value)
{
	auto &doc = node.GetDocument();
	auto *nnode = doc->allocate_node(static_cast<rapidxml::node_type>(type),allocate_string(*doc,name),allocate_string(*doc,value));
	if(nnode == nullptr)
		return;
	node->prepend_node(nnode);
	Lua::Push<XMLNode>(l,XMLNode(node.GetDocument(),nnode));
}
void Lua::xml::Node::PrependNode(lua_State *l,XMLNode &node,XMLNode &childNode)
{
	node->prepend_node(childNode.get());
}
void Lua::xml::Node::GetFirstNode(lua_State *l,XMLNode &node,const std::string &name,bool bMatchCase)
{
	auto *child = node->first_node(name.c_str(),name.length(),bMatchCase);
	if(child == nullptr)
		return;
	Lua::Push<XMLNode>(l,XMLNode(node.GetDocument(),child));
}
void Lua::xml::Node::GetFirstNode(lua_State *l,XMLNode &node,const std::string &name)
{
	auto *child = node->first_node(name.c_str(),name.length());
	if(child == nullptr)
		return;
	Lua::Push<XMLNode>(l,XMLNode(node.GetDocument(),child));
}
void Lua::xml::Node::GetFirstNode(lua_State *l,XMLNode &node)
{
	auto *child = node->first_node();
	if(child == nullptr)
		return;
	Lua::Push<XMLNode>(l,XMLNode(node.GetDocument(),child));
}

void Lua::xml::Node::GetLastNode(lua_State *l,XMLNode &node,const std::string &name,bool bMatchCase)
{
	if(node->first_node() == nullptr)
		return;
	auto *child = node->last_node(name.c_str(),name.length(),bMatchCase);
	if(child == nullptr)
		return;
	Lua::Push<XMLNode>(l,XMLNode(node.GetDocument(),child));
}
void Lua::xml::Node::GetLastNode(lua_State *l,XMLNode &node,const std::string &name)
{
	if(node->first_node() == nullptr)
		return;
	auto *child = node->last_node(name.c_str(),name.length());
	if(child == nullptr)
		return;
	Lua::Push<XMLNode>(l,XMLNode(node.GetDocument(),child));
}
void Lua::xml::Node::GetLastNode(lua_State *l,XMLNode &node)
{
	if(node->first_node() == nullptr)
		return;
	auto *child = node->last_node();
	if(child == nullptr)
		return;
	Lua::Push<XMLNode>(l,XMLNode(node.GetDocument(),child));
}

void Lua::xml::Node::GetFirstAttribute(lua_State *l,XMLNode &node,const std::string &name,bool bMatchCase)
{
	auto *attr = node->first_attribute(name.c_str(),name.length(),bMatchCase);
	if(attr == nullptr)
		return;
	Lua::Push<XMLAttribute>(l,XMLAttribute(node,attr));
}
void Lua::xml::Node::GetFirstAttribute(lua_State *l,XMLNode &node,const std::string &name)
{
	auto *attr = node->first_attribute(name.c_str(),name.length());
	if(attr == nullptr)
		return;
	Lua::Push<XMLAttribute>(l,XMLAttribute(node,attr));
}
void Lua::xml::Node::GetFirstAttribute(lua_State *l,XMLNode &node)
{
	auto *attr = node->first_attribute();
	if(attr == nullptr)
		return;
	Lua::Push<XMLAttribute>(l,XMLAttribute(node,attr));
}
void Lua::xml::Node::GetLastAttribute(lua_State *l,XMLNode &node,const std::string &name,bool bMatchCase)
{
	auto *attr = node->last_attribute(name.c_str(),name.length(),bMatchCase);
	if(attr == nullptr)
		return;
	Lua::Push<XMLAttribute>(l,XMLAttribute(node,attr));
}
void Lua::xml::Node::GetLastAttribute(lua_State *l,XMLNode &node,const std::string &name)
{
	auto *attr = node->last_attribute(name.c_str(),name.length());
	if(attr == nullptr)
		return;
	Lua::Push<XMLAttribute>(l,XMLAttribute(node,attr));
}
void Lua::xml::Node::GetLastAttribute(lua_State *l,XMLNode &node)
{
	auto *attr = node->last_attribute();
	if(attr == nullptr)
		return;
	Lua::Push<XMLAttribute>(l,XMLAttribute(node,attr));
}

void Lua::xml::Node::GetNextSibling(lua_State *l,XMLNode &node,const std::string &name,bool bMatchCase)
{
	if(node->parent() == nullptr)
		return;
	auto *sibling = node->next_sibling(name.c_str(),name.length(),bMatchCase);
	if(sibling == nullptr)
		return;
	Lua::Push<XMLNode>(l,XMLNode(node.GetDocument(),sibling));
}
void Lua::xml::Node::GetNextSibling(lua_State *l,XMLNode &node,const std::string &name)
{
	if(node->parent() == nullptr)
		return;
	auto *sibling = node->next_sibling(name.c_str(),name.length());
	if(sibling == nullptr)
		return;
	Lua::Push<XMLNode>(l,XMLNode(node.GetDocument(),sibling));
}
void Lua::xml::Node::GetNextSibling(lua_State *l,XMLNode &node)
{
	if(node->parent() == nullptr)
		return;
	auto *sibling = node->next_sibling();
	if(sibling == nullptr)
		return;
	Lua::Push<XMLNode>(l,XMLNode(node.GetDocument(),sibling));
}
void Lua::xml::Node::GetPreviousSibling(lua_State *l,XMLNode &node,const std::string &name,bool bMatchCase)
{
	if(node->parent() == nullptr)
		return;
	auto *sibling = node->previous_sibling(name.c_str(),name.length(),bMatchCase);
	if(sibling == nullptr)
		return;
	Lua::Push<XMLNode>(l,XMLNode(node.GetDocument(),sibling));
}
void Lua::xml::Node::GetPreviousSibling(lua_State *l,XMLNode &node,const std::string &name)
{
	if(node->parent() == nullptr)
		return;
	auto *sibling = node->previous_sibling(name.c_str(),name.length());
	if(sibling == nullptr)
		return;
	Lua::Push<XMLNode>(l,XMLNode(node.GetDocument(),sibling));
}
void Lua::xml::Node::GetPreviousSibling(lua_State *l,XMLNode &node)
{
	if(node->parent() == nullptr)
		return;
	auto *sibling = node->previous_sibling();
	if(sibling == nullptr)
		return;
	Lua::Push<XMLNode>(l,XMLNode(node.GetDocument(),sibling));
}
void Lua::xml::Node::GetType(lua_State *l,XMLNode &node)
{
	Lua::PushInt(l,umath::to_integral(node->type()));
}
void Lua::xml::Node::InsertAttribute(lua_State *l,XMLNode &node,XMLAttribute &attrWhere,const std::string &name,const std::string &value)
{
	auto &doc = node.GetDocument();
	auto *attr = doc->allocate_attribute(allocate_string(*doc,name),allocate_string(*doc,value));
	if(attr == nullptr)
		return;
	node->insert_attribute(attrWhere.get(),attr);
	Lua::Push<XMLAttribute>(l,XMLAttribute(node,attr));
}
void Lua::xml::Node::InsertAttribute(lua_State *l,XMLNode &node,XMLAttribute &attrWhere,XMLAttribute &childAttr)
{
	node->insert_attribute(attrWhere.get(),childAttr.get());
}
void Lua::xml::Node::InsertNode(lua_State *l,XMLNode &node,XMLNode &nodeWhere,uint32_t type,const std::string &name,const std::string &value)
{
	auto &doc = node.GetDocument();
	auto *nnode = doc->allocate_node(static_cast<rapidxml::node_type>(type),allocate_string(*doc,name),allocate_string(*doc,value));
	if(nnode == nullptr)
		return;
	node->insert_node(nodeWhere.get(),nnode);
	Lua::Push<XMLNode>(l,XMLNode(node.GetDocument(),nnode));
}
void Lua::xml::Node::InsertNode(lua_State *l,XMLNode &node,XMLNode &nodeWhere,XMLNode &childNode)
{
	node->insert_node(nodeWhere.get(),childNode.get());
}
void Lua::xml::Node::RemoveAllAttributes(lua_State *l,XMLNode &node)
{
	node->remove_all_attributes();
}
void Lua::xml::Node::RemoveAllNodes(lua_State *l,XMLNode &node)
{
	node->remove_all_nodes();
}
void Lua::xml::Node::RemoveAttribute(lua_State *l,XMLNode &node,XMLAttribute &attr)
{
	node->remove_attribute(attr.get());
}
void Lua::xml::Node::RemoveFirstAttribute(lua_State *l,XMLNode &node)
{
	if(node->first_attribute() == nullptr)
		return;
	node->remove_first_attribute();
}
void Lua::xml::Node::RemoveFirstNode(lua_State *l,XMLNode &node)
{
	if(node->first_node() == nullptr)
		return;
	node->remove_first_node();
}
void Lua::xml::Node::RemoveLastAttribute(lua_State *l,XMLNode &node)
{
	if(node->first_attribute() == nullptr)
		return;
	node->remove_last_attribute();
}
void Lua::xml::Node::RemoveLastNode(lua_State *l,XMLNode &node)
{
	if(node->first_node() == nullptr)
		return;
	node->remove_last_node();
}
void Lua::xml::Node::RemoveNode(lua_State *l,XMLNode &node,XMLNode &nodeOther)
{
	node->remove_node(nodeOther.get());
}
void Lua::xml::Node::ToString(lua_State *l,XMLNode &node)
{
	Lua::PushString(l,node_to_string(*node.get()));
}
void Lua::xml::Node::Save(lua_State *l,XMLNode &node,const std::string &fname)
{
	auto f = FileManager::OpenFile<VFilePtrReal>(fname.c_str(),"w");
	if(f == nullptr)
	{
		Lua::PushBool(l,false);
		return;
	}
	f->WriteString(node_to_string(*node.get()));
	Lua::PushBool(l,true);
}

////////////////////////////

void Lua::xml::Attribute::GetNextAttribute(lua_State *l,XMLAttribute &attr,const std::string &name,bool bMatchCase)
{
	auto *nattr = attr->next_attribute(name.c_str(),name.length(),bMatchCase);
	if(nattr == nullptr)
		return;
	Lua::Push<XMLAttribute>(l,XMLAttribute(attr.GetNode(),nattr));
}
void Lua::xml::Attribute::GetNextAttribute(lua_State *l,XMLAttribute &attr,const std::string &name)
{
	auto *nattr = attr->next_attribute(name.c_str(),name.length());
	if(nattr == nullptr)
		return;
	Lua::Push<XMLAttribute>(l,XMLAttribute(attr.GetNode(),nattr));
}
void Lua::xml::Attribute::GetNextAttribute(lua_State *l,XMLAttribute &attr)
{
	auto *nattr = attr->next_attribute();
	if(nattr == nullptr)
		return;
	Lua::Push<XMLAttribute>(l,XMLAttribute(attr.GetNode(),nattr));
}
void Lua::xml::Attribute::GetPreviousAttribute(lua_State *l,XMLAttribute &attr,const std::string &name,bool bMatchCase)
{
	auto *nattr = attr->previous_attribute(name.c_str(),name.length(),bMatchCase);
	if(nattr == nullptr)
		return;
	Lua::Push<XMLAttribute>(l,XMLAttribute(attr.GetNode(),nattr));
}
void Lua::xml::Attribute::GetPreviousAttribute(lua_State *l,XMLAttribute &attr,const std::string &name)
{
	auto *nattr = attr->previous_attribute(name.c_str(),name.length());
	if(nattr == nullptr)
		return;
	Lua::Push<XMLAttribute>(l,XMLAttribute(attr.GetNode(),nattr));
}
void Lua::xml::Attribute::GetPreviousAttribute(lua_State *l,XMLAttribute &attr)
{
	auto *nattr = attr->previous_attribute();
	if(nattr == nullptr)
		return;
	Lua::Push<XMLAttribute>(l,XMLAttribute(attr.GetNode(),nattr));
}
