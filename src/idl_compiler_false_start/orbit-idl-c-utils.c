#include "config.h"

#include "orbit-idl-c-backend.h"

#include <string.h>

void
orbit_cbe_write_typespec(FILE *of, IDL_tree tree)
{
  if(!tree) {
    fprintf(of, "void");
    return;
  }

  switch(IDL_NODE_TYPE(tree)) {
  case IDLN_TYPE_FLOAT:
    switch(IDL_TYPE_FLOAT(tree).f_type) {
    case IDL_FLOAT_TYPE_FLOAT:
      fprintf(of, "CORBA_float");
      break;
    case IDL_FLOAT_TYPE_DOUBLE:
      fprintf(of, "CORBA_double");
      break;
    case IDL_FLOAT_TYPE_LONGDOUBLE:
      fprintf(of, "CORBA_long_double");
      break;
    };
    break;
  case IDLN_TYPE_BOOLEAN:
    fprintf(of, "CORBA_boolean");
    break;
  case IDLN_TYPE_FIXED:
    fprintf(of, "CORBA_fixed_%qd_%qd",
	    IDL_INTEGER(IDL_TYPE_FIXED(tree).positive_int_const).value,
	    IDL_INTEGER(IDL_TYPE_FIXED(tree).integer_lit).value);
    break;
  case IDLN_TYPE_INTEGER:
    fprintf(of, "CORBA_");
    if(!IDL_TYPE_INTEGER(tree).f_signed)
      fprintf(of, "unsigned_");
    switch(IDL_TYPE_INTEGER(tree).f_type) {
    case IDL_INTEGER_TYPE_SHORT:
      fprintf(of, "short");
      break;
    case IDL_INTEGER_TYPE_LONGLONG:
      fprintf(of, "long_");
    case IDL_INTEGER_TYPE_LONG:
      fprintf(of, "long");
      break;
    }
    break;
  case IDLN_TYPE_STRING:
    fprintf(of, "CORBA_char *");
    break;
  case IDLN_TYPE_OCTET:
    fprintf(of, "CORBA_octet");
    break;
  case IDLN_TYPE_WIDE_STRING:
    fprintf(of, "CORBA_wchar *");
    break;
  case IDLN_TYPE_CHAR:
    fprintf(of, "CORBA_char");
    break;
  case IDLN_TYPE_WIDE_CHAR:
    fprintf(of, "CORBA_wchar");
    break;
  case IDLN_TYPE_STRUCT:
  case IDLN_TYPE_ARRAY:    
  case IDLN_TYPE_UNION:
  case IDLN_TYPE_ENUM:
  case IDLN_EXCEPT_DCL:
    {
      char *id = IDL_ns_ident_to_qstring(IDL_IDENT_TO_NS(IDL_TYPE_ENUM(tree).ident), "_", 0);
      fprintf(of, "%s", id);
      g_free(id);
    }
    break;
  case IDLN_IDENT:
    {
      char *id = IDL_ns_ident_to_qstring(IDL_IDENT_TO_NS(tree), "_", 0);
      fprintf(of, "%s", id);
      g_free(id);
    }
    break;
  case IDLN_TYPE_OBJECT:
    fprintf(of, "CORBA_Object");
    break;
  case IDLN_TYPE_SEQUENCE:
    {
      char *ctmp;
      ctmp = orbit_cbe_get_typename(tree);
      fprintf(of, "%s", ctmp);
    }
    break;
  case IDLN_TYPE_ANY:
    fprintf(of, "CORBA_any");
    break;
  case IDLN_NATIVE:
    fprintf(of, "gpointer");
    break;
  case IDLN_TYPE_TYPECODE:
    fprintf(of, "CORBA_TypeCode");
    break;
  default:
    g_error("We were asked to print a typespec %s", IDL_tree_type_names[tree->_type]);
  }
}

IDL_tree
orbit_cbe_get_efftype(IDL_tree tree)
{
  switch(IDL_NODE_TYPE(tree)) {
  case IDLN_IDENT:
  case IDLN_LIST:
    return orbit_cbe_get_efftype(IDL_get_parent_node(tree, IDLN_ANY, NULL));
    break;
  case IDLN_TYPE_DCL:
    return orbit_cbe_get_efftype(IDL_TYPE_DCL(tree).type_spec);
    break;
  default:
    return tree;
    break;
  }
}

char *
orbit_cbe_get_typename(IDL_tree tree)
{
  GString *tmpstr;
  char *retval = NULL;

  if(!tree) {
    return g_strdup("void");
  }

  tmpstr = g_string_new(NULL);

  switch(IDL_NODE_TYPE(tree)) {
  case IDLN_TYPE_ANY:
    retval = "CORBA_any";
    break;
  case IDLN_TYPE_FLOAT:
    switch(IDL_TYPE_FLOAT(tree).f_type) {
    case IDL_FLOAT_TYPE_FLOAT:
      retval = "CORBA_float";
      break;
    case IDL_FLOAT_TYPE_DOUBLE:
      retval = "CORBA_double";
      break;
    case IDL_FLOAT_TYPE_LONGDOUBLE:
      retval = "CORBA_long_double";
      break;
    }
    break;
  case IDLN_TYPE_FIXED:
    g_string_sprintf(tmpstr, "CORBA_fixed_%qd_%qd",
		     IDL_INTEGER(IDL_TYPE_FIXED(tree).positive_int_const).value,
		     IDL_INTEGER(IDL_TYPE_FIXED(tree).integer_lit).value);
    break;
  case IDLN_TYPE_INTEGER:
    g_string_assign(tmpstr, "CORBA_");
    if(!IDL_TYPE_INTEGER(tree).f_signed)
      g_string_append(tmpstr, "unsigned_");

    switch(IDL_TYPE_INTEGER(tree).f_type) {
    case IDL_INTEGER_TYPE_SHORT:
      g_string_append(tmpstr, "short");
      break;
    case IDL_INTEGER_TYPE_LONGLONG:
      g_string_append(tmpstr, "long_");
    case IDL_INTEGER_TYPE_LONG:
      g_string_append(tmpstr, "long");
      break;
    }
    break;
  case IDLN_TYPE_STRING:
    retval = "CORBA_string";
    break;
  case IDLN_TYPE_OCTET:
    retval = "CORBA_octet";
    break;
  case IDLN_TYPE_WIDE_STRING:
    retval = "CORBA_wstring";
    break;
  case IDLN_TYPE_CHAR:
    retval = "CORBA_char";
    break;
  case IDLN_TYPE_WIDE_CHAR:
    retval = "CORBA_wchar";
    break;
  case IDLN_TYPE_BOOLEAN:
    retval = "CORBA_boolean";
    break;
  case IDLN_TYPE_STRUCT:
    retval = orbit_cbe_get_typename(IDL_TYPE_STRUCT(tree).ident);
    break;
  case IDLN_EXCEPT_DCL:
    retval = orbit_cbe_get_typename(IDL_EXCEPT_DCL(tree).ident);
    break;
  case IDLN_TYPE_ARRAY:
    retval = orbit_cbe_get_typename(IDL_TYPE_ARRAY(tree).ident);
    break;
  case IDLN_TYPE_UNION:
    retval = orbit_cbe_get_typename(IDL_TYPE_UNION(tree).ident);
    break;
  case IDLN_TYPE_ENUM:
    retval = orbit_cbe_get_typename(IDL_TYPE_ENUM(tree).ident);
    break;
  case IDLN_IDENT:
    {
      char *id = IDL_ns_ident_to_qstring(IDL_IDENT_TO_NS(tree), "_", 0);
      g_string_assign(tmpstr, id);
      g_free(id);
    }
    break;
  case IDLN_PARAM_DCL:
    retval = orbit_cbe_get_typename(IDL_PARAM_DCL(tree).param_type_spec);
    g_string_assign(tmpstr, retval); g_free(retval); retval = NULL;
    break;
  case IDLN_TYPE_SEQUENCE:
    {
      char *ctmp;

      ctmp = orbit_cbe_get_typename(orbit_cbe_get_efftype(IDL_TYPE_SEQUENCE(tree).simple_type_spec));

      if(!strncmp(ctmp, "CORBA_", strlen("CORBA_")))
	memmove(ctmp, ctmp + strlen("CORBA_"), strlen(ctmp + strlen("CORBA_")) + 1);

      g_string_sprintf(tmpstr, "CORBA_sequence_%s", ctmp);
      g_free(ctmp);
    }
    break;
  case IDLN_INTERFACE:
    retval = orbit_cbe_get_typename(IDL_INTERFACE(tree).ident);
    g_string_assign(tmpstr, retval); g_free(retval); retval = NULL;
    break;
  case IDLN_NATIVE:
    retval = "gpointer";
    break;
  case IDLN_TYPE_OBJECT:
    retval = "CORBA_Object";
    break;
  case IDLN_TYPE_TYPECODE:
    retval = "CORBA_TypeCode";
    break;
  default:
    g_error("We were asked to get a typename for a %s",
	    IDL_tree_type_names[IDL_NODE_TYPE(tree)]);
  }

  if(retval)
    return g_strdup(retval);
  else {
    retval = tmpstr->str;
    g_string_free(tmpstr, FALSE);
    return retval;
  }
}

/* This is fixed length as far as memory allocation & CORBA goes, not as far as "can we bulk-marshal it?" goes.
 *   Memory allocation fixed == nothing to free in this node
 */
gboolean
orbit_cbe_type_is_fixed_length(IDL_tree ts)
{
  gboolean is_fixed = TRUE;
  IDL_tree curitem;

  ts = orbit_cbe_get_typespec(ts);
  switch(IDL_NODE_TYPE(ts)) {
  case IDLN_TYPE_FLOAT:
  case IDLN_TYPE_INTEGER:
  case IDLN_TYPE_ENUM:
  case IDLN_TYPE_CHAR:
  case IDLN_TYPE_WIDE_CHAR:
  case IDLN_TYPE_OCTET:
  case IDLN_TYPE_BOOLEAN:
    return TRUE;
    break;
  case IDLN_TYPE_SEQUENCE:
  case IDLN_TYPE_STRING:
  case IDLN_TYPE_WIDE_STRING:
  case IDLN_TYPE_OBJECT:
  case IDLN_FORWARD_DCL:
  case IDLN_INTERFACE:
  case IDLN_TYPE_ANY:
  case IDLN_NATIVE:
  case IDLN_TYPE_TYPECODE:
    return FALSE;
    break;
  case IDLN_TYPE_UNION:
    for(curitem = IDL_TYPE_UNION(ts).switch_body; curitem;
	curitem = IDL_LIST(curitem).next) {
      is_fixed &= orbit_cbe_type_is_fixed_length(IDL_LIST(IDL_CASE_STMT(IDL_LIST(curitem).data).element_spec).data);
    }
    return is_fixed;
    break;
  case IDLN_EXCEPT_DCL:
  case IDLN_TYPE_STRUCT:
    for(curitem = IDL_TYPE_STRUCT(ts).member_list; curitem;
	curitem = IDL_LIST(curitem).next) {
      is_fixed &= orbit_cbe_type_is_fixed_length(IDL_LIST(curitem).data);
    }
    return is_fixed;
    break;
  case IDLN_TYPE_ARRAY:
    return orbit_cbe_type_is_fixed_length(IDL_TYPE_DCL(IDL_get_parent_node(ts, IDLN_TYPE_DCL, NULL)).type_spec);
    break;
  case IDLN_TYPE_DCL:
    return orbit_cbe_type_is_fixed_length(IDL_TYPE_DCL(ts).type_spec);
    break;
  case IDLN_IDENT:
  case IDLN_LIST:
    return orbit_cbe_type_is_fixed_length(IDL_NODE_UP(ts));
    break;
  case IDLN_MEMBER:
    return orbit_cbe_type_is_fixed_length(IDL_MEMBER(ts).type_spec);
    break;
  default:
    g_warning("I'm not sure if type %s is fixed-length", IDL_tree_type_names[IDL_NODE_TYPE(ts)]);
    return FALSE;
  }
}

IDL_tree
orbit_cbe_get_typespec(IDL_tree node)
{
  if(node == NULL)
    return NULL;

  switch(IDL_NODE_TYPE(node)) {
  case IDLN_TYPE_INTEGER:
  case IDLN_TYPE_FLOAT:
  case IDLN_TYPE_FIXED:
  case IDLN_TYPE_CHAR:
  case IDLN_TYPE_WIDE_CHAR:
  case IDLN_TYPE_STRING:
  case IDLN_TYPE_WIDE_STRING:
  case IDLN_TYPE_BOOLEAN:
  case IDLN_TYPE_OCTET:
  case IDLN_TYPE_ANY:
  case IDLN_TYPE_OBJECT:
  case IDLN_TYPE_ENUM:
  case IDLN_TYPE_SEQUENCE:
  case IDLN_TYPE_ARRAY:
  case IDLN_TYPE_STRUCT:
  case IDLN_TYPE_UNION:
  case IDLN_EXCEPT_DCL:
  case IDLN_FORWARD_DCL:
  case IDLN_INTERFACE:
  case IDLN_NATIVE:
  case IDLN_TYPE_TYPECODE:
    return node;
    break;
  case IDLN_TYPE_DCL:
    return orbit_cbe_get_typespec(IDL_TYPE_DCL(node).type_spec);
    break;
  case IDLN_PARAM_DCL:
    return orbit_cbe_get_typespec(IDL_PARAM_DCL(node).param_type_spec);
    break;
  case IDLN_MEMBER:
    return orbit_cbe_get_typespec(IDL_MEMBER(node).type_spec);
    break;
  case IDLN_LIST:
  case IDLN_IDENT:
    return orbit_cbe_get_typespec(IDL_get_parent_node(node, IDLN_ANY, NULL));
    break;
  default:
    g_warning("Unhandled node type %s!", IDL_tree_type_names[IDL_NODE_TYPE(node)]);
    return NULL;
  }
}

/* For use by below function */
static const int * const
orbit_cbe_get_typeoffsets_table (void)
{
  static int typeoffsets[IDLN_LAST];
  static gboolean initialized = FALSE;
  
  if (!initialized) {
    int i;

    for (i = 0; i < IDLN_LAST; ++i)
      typeoffsets[i] = -1;

    typeoffsets[IDLN_FORWARD_DCL] = 8; /* (same as objref) */
    typeoffsets[IDLN_TYPE_INTEGER] = 0;
    typeoffsets[IDLN_TYPE_FLOAT] = 0;
    typeoffsets[IDLN_TYPE_FIXED] = 3;
    typeoffsets[IDLN_TYPE_CHAR] = 5;
    typeoffsets[IDLN_TYPE_WIDE_CHAR] = 6;
    typeoffsets[IDLN_TYPE_STRING] = 12;
    typeoffsets[IDLN_TYPE_WIDE_STRING] = 13;
    typeoffsets[IDLN_TYPE_BOOLEAN] = 4;
    typeoffsets[IDLN_TYPE_OCTET] = 7;
    typeoffsets[IDLN_TYPE_ANY] = 16;
    typeoffsets[IDLN_TYPE_OBJECT] = 9;
    typeoffsets[IDLN_TYPE_TYPECODE] = 9;
    typeoffsets[IDLN_TYPE_ENUM] = 8;
    typeoffsets[IDLN_TYPE_SEQUENCE] = 14;
    typeoffsets[IDLN_TYPE_ARRAY] = 15;
    typeoffsets[IDLN_TYPE_STRUCT] = 10;
    typeoffsets[IDLN_TYPE_UNION] = 11;
    typeoffsets[IDLN_NATIVE] = 15; /* no pointers ever, same as fixed array */
    typeoffsets[IDLN_INTERFACE] = 9; /* (same as objref) */
    
    initialized = TRUE;
  }
  
  return typeoffsets;
}

/*******

	This is a rather hairy function. Its purpose is to output the
	required number of *'s that indicate the amount of indirection
	for input, output, & input-output parameters, and return
	values.  We do this by having a table of the number of *'s for
	each type and purpose (nptrrefs_required), taken from 19.20
	of the CORBA 2.2 spec, and then having a table that translates
	from the IDLN_* enums into an index into nptrrefs_required (typeoffsets)

 *******/
static gint
orbit_cbe_param_numptrs(IDL_tree param, IDL_ParamRole role)
{
  const int * const typeoffsets = orbit_cbe_get_typeoffsets_table ();
  const int nptrrefs_required[][4] = {
    {0,1,1,0} /* float */,
    {0,1,1,0} /* double */,
    {0,1,1,0} /* long double */,
    {1,1,1,0} /* fixed_d_s 3 */, 
    {0,1,1,0} /* boolean */,
    {0,1,1,0} /* char */,
    {0,1,1,0} /* wchar */,
    {0,1,1,0} /* octet */,
    {0,1,1,0} /* enum */,
    {0,1,1,0} /* objref */,
    {1,1,1,0} /* fixed struct 10 */,
    {1,1,1,0} /* fixed union */,
    {0,1,1,0} /* string */,
    {0,1,1,0} /* wstring */,
    {1,1,2,1} /* sequence */,
    {0,0,0,0} /* fixed array */,
    {1,1,2,1} /* any 16 */
  };
  int retval = 0;

  if(!param) /* void */
    return 0;

  /* Now, how do we use this table? :) */
  param = orbit_cbe_get_typespec(param);

  g_assert(param);

  switch(IDL_NODE_TYPE(param))
    {
    case IDLN_TYPE_STRUCT:
    case IDLN_TYPE_UNION:
      if(((role == DATA_RETURN) || (role == DATA_OUT))
	 && !orbit_cbe_type_is_fixed_length(param))
	retval++;

      break;
    case IDLN_TYPE_ARRAY:
      if(!orbit_cbe_type_is_fixed_length(param) && role == DATA_OUT)
	retval++;
      break;
    default:
      break;
    }

  g_assert(typeoffsets[IDL_NODE_TYPE(param)] >= 0);

  switch(role) {
  case DATA_IN: role = 0; break;
  case DATA_INOUT: role = 1; break;
  case DATA_OUT: role = 2; break;
  case DATA_RETURN: role = 3; break;
  }

  retval+=nptrrefs_required[typeoffsets[IDL_NODE_TYPE(param)]][role];

  return retval;
}

static void
orbit_cbe_param_printptrs(FILE *of, IDL_tree param, IDL_ParamRole role)
{
  int i, n;

  if(param == NULL)
    return;

  param = orbit_cbe_get_typespec(param);
  n = orbit_cbe_param_numptrs(param, role);

#if 0
  if(IDL_NODE_TYPE(param) == IDLN_TYPE_ARRAY
     && ((role == DATA_OUT) || (role == DATA_RETURN))) {
    if(role == DATA_RETURN)
      fprintf(of, "_slice*");
    else if(!cbe_type_is_fixed_length(param)) /* && role == DATA_OUT */
      fprintf(of, "_slice*");
  }
#endif

  for(i = 0; i < n; i++)
    fprintf(of, "*");
}

void
orbit_cbe_op_write_proto(FILE *of,
			 IDL_tree op,
			 const char *nom_prefix,
			 gboolean for_epv)
{
  IDL_tree sub, ttmp;
  char *id;

  if(IDL_OP_DCL(op).op_type_spec) {
    orbit_cbe_write_typespec(of, IDL_OP_DCL(op).op_type_spec);
    
    ttmp = IDL_NODE_UP(IDL_OP_DCL(op).op_type_spec);
    if(IDL_NODE_TYPE(ttmp) == IDLN_TYPE_ARRAY)
      fprintf(of, "_slice*");
  } else
    fprintf(of, "void");

  orbit_cbe_param_printptrs(of, IDL_OP_DCL(op).op_type_spec, DATA_RETURN);

  id = IDL_ns_ident_to_qstring(IDL_IDENT_TO_NS(IDL_INTERFACE(IDL_get_parent_node(op, IDLN_INTERFACE, NULL)).ident), "_", 0);

  if(for_epv) {
    fprintf(of, " (*%s%s)", nom_prefix?nom_prefix:"",
	    IDL_IDENT(IDL_OP_DCL(op).ident).str);
  } else {
    fprintf(of, " %s%s_%s", nom_prefix?nom_prefix:"",
	    id,
	    IDL_IDENT(IDL_OP_DCL(op).ident).str);
  }

  fprintf(of, "(");

  if(for_epv)
    fprintf(of, "PortableServer_Servant _servant, ");
  else
    fprintf(of, "%s, ", id);

  g_free(id);

  for(sub = IDL_OP_DCL(op).parameter_dcls; sub; sub = IDL_LIST(sub).next) {
    IDL_tree parm;

    parm = IDL_LIST(sub).data;

    orbit_cbe_write_typespec(of, IDL_PARAM_DCL(parm).param_type_spec);

    ttmp = IDL_NODE_UP(IDL_PARAM_DCL(parm).param_type_spec);
    if((IDL_NODE_TYPE(ttmp) == IDLN_TYPE_ARRAY)
       && (IDL_PARAM_DCL(parm).attr == IDL_PARAM_OUT)
       && !orbit_cbe_type_is_fixed_length(ttmp))
      fprintf(of, "_slice*");

    {
      IDL_ParamRole r;

      switch(IDL_PARAM_DCL(parm).attr) {
      case IDL_PARAM_IN: r = DATA_IN; break;
      case IDL_PARAM_INOUT: r = DATA_INOUT; break;
      case IDL_PARAM_OUT: r = DATA_OUT; break;
      default:
	g_error("Unknown IDL_PARAM type");
      }

      orbit_cbe_param_printptrs(of, IDL_OP_DCL(op).op_type_spec, r);
    }

    fprintf(of, " %s, ", IDL_IDENT(IDL_PARAM_DCL(parm).simple_declarator).str);
  }
  fprintf(of, "CORBA_Environment *ev)");
}

/* Writes the value of the constant in 'tree' to file handle 'of' */
void
orbit_cbe_write_const(FILE *of, IDL_tree tree)
{
  char *opc = NULL;
  switch(IDL_NODE_TYPE(tree)) {
  case IDLN_BOOLEAN:
    fprintf(of, "%s", IDL_BOOLEAN(tree).value?"CORBA_TRUE":"CORBA_FALSE");
    break;
  case IDLN_CHAR:
    fprintf(of, "'%s'", IDL_CHAR(tree).value);
    break;
  case IDLN_FLOAT:
    fprintf(of, "%f", IDL_FLOAT(tree).value);
    break;
  case IDLN_INTEGER:
    fprintf(of, "%" IDL_LL "d", IDL_INTEGER(tree).value);
    break;
  case IDLN_STRING:
    fprintf(of, "\"%s\"", IDL_STRING(tree).value);
    break;
  case IDLN_WIDE_CHAR:
    fprintf(of, "L'%ls'", IDL_WIDE_CHAR(tree).value);
    break;
  case IDLN_WIDE_STRING:
    fprintf(of, "L\"%ls\"", IDL_WIDE_STRING(tree).value);
    break;
  case IDLN_BINOP:
    fprintf(of, "(");
    orbit_cbe_write_const(of, IDL_BINOP(tree).left);
    switch(IDL_BINOP(tree).op) {
    case IDL_BINOP_OR:
      opc = "|";
      break;
    case IDL_BINOP_XOR:
      opc = "!!!";
      break;
    case IDL_BINOP_AND:
      opc = "&";
      break;
    case IDL_BINOP_SHR:
      opc = ">>";
      break;
    case IDL_BINOP_SHL:
      opc = "<<";
      break;
    case IDL_BINOP_ADD:
      opc = "+";
      break;
    case IDL_BINOP_SUB:
      opc = "-";
      break;
    case IDL_BINOP_MULT:
      opc = "*";
      break;
    case IDL_BINOP_DIV:
      opc = "/";
      break;
    case IDL_BINOP_MOD:
      opc = "%";
      break;
    }
    fprintf(of, " %s ", opc);
    orbit_cbe_write_const(of, IDL_BINOP(tree).right);
    fprintf(of, ")");
    break;
  case IDLN_UNARYOP:
    switch(IDL_UNARYOP(tree).op) {
    case IDL_UNARYOP_PLUS: opc = "+"; break;
    case IDL_UNARYOP_MINUS: opc = "-"; break;
    case IDL_UNARYOP_COMPLEMENT: opc = "~"; break;
    }
    fprintf(of, "%s", opc);
    orbit_cbe_write_const(of, IDL_UNARYOP(tree).operand);
    break;
  case IDLN_IDENT:
    {
      /* XXX fixme, brokenness */
      char *id;
      id = IDL_ns_ident_to_qstring(IDL_IDENT_TO_NS(tree), "_", 0);
      fprintf(of, "%s", id);
      g_free(id);
    }
    break;
  default:
    g_error("We were asked to print a constant for %s", IDL_tree_type_names[tree->_type]);
  }
}
