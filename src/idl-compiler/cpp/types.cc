/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  ORBit-C++: C++ bindings for ORBit.
 *
 *  Copyright (C) 2000 Andreas Kloeckner
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free
 *  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  Author:	Andreas Kloeckner <ak@ixion.net>
 *          Phil Dawes <philipd@users.sourceforge.net>
 *			John K. Luebs <jkluebs@marikarpress.com>
 *
 *  Purpose: IDL compiler type representation
 *
 */




#include "types.hh"
#include "pass_xlate.hh"
#include <strstream>

static IDLVoid idlVoid;
static IDLString idlString;




#define ORBITCPP_MAKE_SIMPLE_TYPE(name,cname)			\
	static class IDL##name : public IDLSimpleType {		\
	protected:						\
		string get_fixed_cpp_typename () const {	\
			return IDL_CORBA_NS "::" #name;		\
		} 						\
		string get_fixed_c_typename () const {		\
			return #cname;				\
		}						\
	} idl##name;


#define ORBITCPP_MAKE_SIMPLE_INT_TYPE(name,cname) \
	static class IDL##name : public IDLSimpleType, public IDLUnionDiscriminator {	\
	protected:									\
		string get_fixed_cpp_typename () const {				\
			return IDL_CORBA_NS "::" #name;					\
		} 									\
		string get_fixed_c_typename () const {					\
			return #cname;							\
		}									\
	public:										\
		string getDefaultValue(set<string> const &labels) const { 		\
			short val=0;							\
			string valstr;							\
		  	do {								\
				strstream ss; ss << val++ << ends; valstr = ss.str();	\
		  	} while( labels.find(valstr) != labels.end() ); 		\
			return valstr; 							\
		}									\
											\
		string discr_get_c_typename () const {					\
			return get_fixed_c_typename ();					\
		}									\
											\
		string discr_get_cpp_typename () const {				\
			return get_fixed_cpp_typename ();				\
		}									\
	} idl##name;

#define ORBITCPP_MAKE_SIMPLE_CHAR_TYPE(name,cname)					\
	static class IDL##name : public IDLSimpleType, public IDLUnionDiscriminator {	\
	protected:									\
		string get_fixed_cpp_typename () const {				\
			return IDL_CORBA_NS "::" #name;					\
		}									\
		string get_fixed_c_typename () const {					\
			return #cname;							\
		}									\
	public:										\
		string getDefaultValue(set<string> const &labels) const {		\
			return "\'\\0\'"; 						\
		}									\
											\
		string discr_get_c_typename () const {					\
			return get_fixed_c_typename ();					\
		}									\
											\
		string discr_get_cpp_typename () const {				\
			return get_fixed_cpp_typename ();				\
		}									\
	} idl##name;


static IDLBoolean idlBoolean;


ORBITCPP_MAKE_SIMPLE_CHAR_TYPE(Char, CORBA_char)
ORBITCPP_MAKE_SIMPLE_CHAR_TYPE(WChar, CORBA_wchar)
ORBITCPP_MAKE_SIMPLE_TYPE(Octet, CORBA_octet)

ORBITCPP_MAKE_SIMPLE_INT_TYPE(Short, CORBA_short)
ORBITCPP_MAKE_SIMPLE_INT_TYPE(UShort, CORBA_unsigned_short)
ORBITCPP_MAKE_SIMPLE_INT_TYPE(Long, CORBA_long)
ORBITCPP_MAKE_SIMPLE_INT_TYPE(ULong, CORBA_unsigned_long)
ORBITCPP_MAKE_SIMPLE_INT_TYPE(LongLong, CORBA_long_long)
ORBITCPP_MAKE_SIMPLE_INT_TYPE(ULongLong, CORBA_unsigned_long_long)

ORBITCPP_MAKE_SIMPLE_TYPE(Float, CORBA_float)
ORBITCPP_MAKE_SIMPLE_TYPE(Double, CORBA_double)
ORBITCPP_MAKE_SIMPLE_TYPE(LongDouble, CORBA_long_double)


static IDLAny idlAny;
static IDLObject idlObject;
static IDLTypeCode idlTypeCode;



// IDLTypeParser -------------------------------------------------------------
IDLTypeParser::~IDLTypeParser() {
	vector<IDLType *>::iterator
	first = m_anonymous_types.begin(),last = m_anonymous_types.end();

	while (first != last) delete *first++;
}




IDLType *
IDLTypeParser::parseTypeSpec(IDLScope &scope,IDL_tree typespec) {

	// I'm starting to think that this should be 2 methods:
	//  - one to parse the type, and one to parse the dcl into an id - PD
	// I noticed the same making some mods to the gather pass - JKL
	
	IDLType *type = NULL;

	if (typespec == NULL) type = &idlVoid;
	else
		switch (IDL_NODE_TYPE(typespec)) {
		case IDLN_TYPE_BOOLEAN:
			type = &idlBoolean;
			break;
		case IDLN_TYPE_CHAR:
			type = &idlChar;
			break;
		case IDLN_TYPE_WIDE_CHAR:
			type = &idlWChar;
			break;
		case IDLN_TYPE_OCTET:
			type = &idlOctet;
			break;
		case IDLN_TYPE_STRING:
			type = &idlString;
			break;

		case IDLN_TYPE_ANY:
			type = &idlAny;
			break;

		case IDLN_TYPE_OBJECT:
			type = &idlObject;
			break;
		case IDLN_TYPE_TYPECODE:
			type = &idlTypeCode;
			break;
				
		case IDLN_TYPE_INTEGER:
			if (IDL_TYPE_INTEGER(typespec).f_signed) {
				switch (IDL_TYPE_INTEGER(typespec).f_type) {
				case IDL_INTEGER_TYPE_SHORT:
					type = &idlShort;
					break;
				case IDL_INTEGER_TYPE_LONG:
					type = &idlLong;
					break;
				case IDL_INTEGER_TYPE_LONGLONG:
					type = &idlLongLong;
					break;
				}
			} else {
				switch (IDL_TYPE_INTEGER(typespec).f_type) {
				case IDL_INTEGER_TYPE_SHORT:
					type = &idlUShort;
					break;
				case IDL_INTEGER_TYPE_LONG:
					type = &idlULong;
					break;
				case IDL_INTEGER_TYPE_LONGLONG:
					type = &idlULongLong;
					break;
				}
			}
			break;

		case IDLN_TYPE_FLOAT:
			switch (IDL_TYPE_FLOAT(typespec).f_type) {
			case IDL_FLOAT_TYPE_FLOAT:
				type = &idlFloat;
				break;
			case IDL_FLOAT_TYPE_DOUBLE:
				type = &idlDouble;
				break;
			case IDL_FLOAT_TYPE_LONGDOUBLE:
				type = &idlLongDouble;
				break;
			}
			break;

		case IDLN_IDENT:
			{
				IDLElement *item = scope.lookup(idlGetQualIdentifier(typespec));
				if (!item) throw IDLExUnknownIdentifier(typespec,idlGetQualIdentifier(typespec));
				if (!item->isType()) throw IDLExTypeIdentifierExpected(typespec,IDL_IDENT(typespec).str);
				type = dynamic_cast<IDLType *>(item);
				break;
			}
		case IDLN_TYPE_SEQUENCE:
			{
				// parse the sequence element type
				IDLType *type_seq = parseTypeSpec(scope,IDL_TYPE_SEQUENCE(typespec).simple_type_spec);
				IDLSequence* seq = 0;
				if (IDL_TYPE_SEQUENCE(typespec).positive_int_const == NULL){
					seq = new IDLSequence(*type_seq,0);
				} else {
					string len_str =
						idlTranslateConstant(IDL_TYPE_SEQUENCE(typespec).positive_int_const,scope);
					int length = atoi(len_str.c_str());
					seq = new IDLSequence(*type_seq,length);
				}
				m_anonymous_types.push_back(seq);
				type = seq;
				break;
			}
		case IDLN_TYPE_ARRAY:
		    {
				cout << "Array!";
				break;
			}
		ORBITCPP_DEFAULT_CASE(typespec)
		}

	if (!type) ORBITCPP_NYI(idlGetNodeTypeString(typespec))

	return type;
}

IDLType*
IDLTypeParser::parseDcl(IDL_tree dcl, IDLType *typespec, string &id)
{
	IDLType *ret_type = typespec;
	
	if (IDL_NODE_TYPE(dcl) == IDLN_IDENT){
		id = IDL_IDENT(dcl).str;
	} else if (IDL_NODE_TYPE(dcl) == IDLN_TYPE_ARRAY) {
		ret_type = new IDLArray(*typespec,
								  IDL_IDENT(IDL_TYPE_ARRAY(dcl).ident).str,
								  dcl);
		// this anon_types name is obviously an "artifact" from MICO, but the behavior
		// is what we want (the memory is freed upon destruction of the IDLTypeParser)
		m_anonymous_types.push_back(ret_type);
		id = IDL_IDENT(IDL_TYPE_ARRAY(dcl).ident).str;
	} else 
		ORBITCPP_NYI(" declarators:"+idlGetNodeTypeString(dcl));
	return ret_type;
}

