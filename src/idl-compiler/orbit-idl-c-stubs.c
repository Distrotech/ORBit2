#include "config.h"

#include "orbit-idl-c-backend.h"

#include <string.h>

static void
cs_output_remote_marshal_stub (IDL_tree    tree,
			       FILE       *of,
			       const char *iface_id,
			       gboolean    has_retval,
			       gboolean    has_args,
			       int         idx)
{
	if (has_args)
		orbit_cbe_flatten_args (tree, of, "_args");

	fprintf (of, "ORBit_small_invoke_stub_n (_obj, "
		 "&%s__iinterface.methods, %d, ", iface_id, idx);

	if (has_retval)
		fprintf (of, "&_ORBIT_retval, ");
	else
		fprintf (of, "NULL, ");

	if (has_args)
		fprintf (of, "_args, ");
	else
		fprintf (of, "NULL, ");

	if (IDL_OP_DCL (tree).context_expr)
		fprintf (of, "_ctx, ");
	else
		fprintf (of, "NULL, ");
		
	fprintf (of, "ev);\n\n");
}

static void
cs_output_inproc_poa_stub (IDL_tree    tree,
			   FILE       *of,
			   const char *iface_id)
{
	IDL_tree node;

	fprintf (of, "POA_%s__epv *%s;\n", iface_id, ORBIT_EPV_VAR_NAME);

	/* in-proc part */
	fprintf (of, "if (ORBit_small_flags & ORBIT_SMALL_FAST_LOCALS && \n");
	fprintf (of, "    ORBIT_STUB_IsBypass (_obj, %s__classid) && \n", iface_id);
	fprintf (of, "    (%s = ORBIT_STUB_GetEpv (_obj, %s__classid))->%s) {\n",
		 ORBIT_EPV_VAR_NAME, iface_id, IDL_IDENT (IDL_OP_DCL (tree).ident).str);

	fprintf (of, "ORBIT_STUB_PreCall (_obj);\n");

	fprintf (of, "%s%s->%s (ORBIT_STUB_GetServant (_obj), ",
		 IDL_OP_DCL (tree).op_type_spec? ORBIT_RETVAL_VAR_NAME " = ":"",
		 ORBIT_EPV_VAR_NAME,
		 IDL_IDENT (IDL_OP_DCL (tree).ident).str);

	for (node = IDL_OP_DCL (tree).parameter_dcls; node; node = IDL_LIST (node).next)
		fprintf (of, "%s, ",
			 IDL_IDENT (IDL_PARAM_DCL (IDL_LIST (node).data).simple_declarator).str);

	if (IDL_OP_DCL (tree).context_expr)
		fprintf (of, "_ctx, ");

	fprintf (of, "ev);\n");

	fprintf (of, "ORBIT_STUB_PostCall (_obj);\n");
	fprintf (of, "}\n");
}

static void
cs_output_stub (IDL_tree       tree,
		OIDL_Run_Info *rinfo,
		OIDL_C_Info   *ci,
		int           *idx)
{
	FILE     *of = ci->fh;
	char     *iface_id;
	gboolean  has_retval, has_args;

	g_return_if_fail (idx != NULL);

	iface_id = IDL_ns_ident_to_qstring (
			IDL_IDENT_TO_NS (IDL_INTERFACE (
				IDL_get_parent_node (tree, IDLN_INTERFACE, NULL)
					).ident), "_", 0);

	has_retval = IDL_OP_DCL (tree).op_type_spec != NULL;
	has_args   = IDL_OP_DCL (tree).parameter_dcls != NULL;

	orbit_cbe_op_write_proto (of, tree, "", NULL, FALSE);

	fprintf (of, "{\n");

	if (!rinfo->target_poa) {
		/* FIXME: need to sort out the in-proc case for the GOA */
		cs_output_remote_marshal_stub (
			tree, of, iface_id, has_retval, has_args, *idx);

		fprintf (of, "}\n");
		g_free (iface_id);
		(*idx)++;

		return;
	}

	if (has_retval) {
		orbit_cbe_write_param_typespec (of, tree);
		fprintf (of, " " ORBIT_RETVAL_VAR_NAME ";\n");
	}

	cs_output_inproc_poa_stub (tree, of, iface_id);

	fprintf (of, "else { /* remote marshal */\n");

	cs_output_remote_marshal_stub (
			tree, of, iface_id, has_retval, has_args, *idx);

	fprintf (of, "}\n");

	if (has_retval)
		fprintf (of, "return " ORBIT_RETVAL_VAR_NAME ";\n");

	fprintf (of, "}\n");

	g_free (iface_id);

	(*idx)++;
}

static void
cs_output_stubs (IDL_tree       tree,
		 OIDL_Run_Info *rinfo,
		 OIDL_C_Info   *ci,
		 int           *idx)
{
	if (!tree)
		return;

	switch (IDL_NODE_TYPE (tree)) {
	case IDLN_MODULE:
		cs_output_stubs (IDL_MODULE (tree).definition_list, rinfo, ci, idx);
		break;
	case IDLN_LIST: {
		IDL_tree sub;

		for (sub = tree; sub; sub = IDL_LIST (sub).next)
			cs_output_stubs (IDL_LIST (sub).data, rinfo, ci, idx);
		break;
		}
	case IDLN_ATTR_DCL: {
		IDL_tree node;
      
		for (node = IDL_ATTR_DCL (tree).simple_declarations; node; node = IDL_LIST (node).next) {
			OIDL_Attr_Info *ai;

			ai = IDL_LIST (node).data->data;
	
			cs_output_stubs (ai->op1, rinfo, ci, idx);

			if (ai->op2)
				cs_output_stubs (ai->op2, rinfo, ci, idx);
		}
		break;
		}
	case IDLN_INTERFACE: {
		int real_idx = 0;

		cs_output_stubs (IDL_INTERFACE (tree).body, rinfo, ci, &real_idx);
		break;
		}
	case IDLN_OP_DCL:
		cs_output_stub (tree, rinfo, ci, idx);
		break;
	default:
		break;
	}
}

void
orbit_idl_output_c_stubs (IDL_tree       tree,
			  OIDL_Run_Info *rinfo,
			  OIDL_C_Info   *ci)
{
	fprintf (ci->fh, OIDL_C_WARNING);
	fprintf (ci->fh, "#include <string.h>\n");
	fprintf (ci->fh, "#define ORBIT2_STUBS_API\n");
	fprintf (ci->fh, "#include \"%s.h\"\n\n", ci->base_name);

	cs_output_stubs (tree, rinfo, ci, NULL);
}
