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

void
orbit_cbe_op_write_proto(FILE *of,
			 IDL_tree op,
			 const char *nom_prefix,
			 gboolean for_epv)
{
  IDL_tree sub, ttmp;
  char *id;

  orbit_cbe_write_typespec(of, IDL_OP_DCL(op).op_type_spec);

  ttmp = IDL_NODE_UP(IDL_OP_DCL(op).op_type_spec);
  if(IDL_NODE_TYPE(ttmp) == IDLN_TYPE_ARRAY)
    fprintf(of, "_slice*");

  id = IDL_ns_ident_to_qstring(IDL_IDENT_TO_NS(IDL_INTERFACE(IDL_get_parent_node(op, IDLN_INTERFACE, NULL)).ident), "_", 0);

  if(for_epv) {
    fprintf(of, "(*%s%s)", nom_prefix?nom_prefix:"",
	    IDL_IDENT(IDL_OP_DCL(op).ident).str);
  } else {
    fprintf(of, "%s%s_%s", nom_prefix?nom_prefix:"",
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

    ttmp = IDL_NODE_UP(IDL_OP_DCL(op).op_type_spec);
    if(IDL_NODE_TYPE(ttmp) == IDLN_TYPE_ARRAY)
      fprintf(of, "_slice*");

    g_warning("XXX param ptr printing is broken");

    fprintf(of, " %s, ", IDL_IDENT(IDL_PARAM_DCL(parm).simple_declarator).str);
  }
  fprintf(of, "CORBA_Environment *ev)");
}
