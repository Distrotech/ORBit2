#ifndef CORBA_ORB_H
#define CORBA_ORB_H 1

#include <orbit/orb-core/orb-types.h>
#include <orbit/orb-core/corba-nvlist-type.h>
#include <orbit/orb-core/corba-object-type.h>
#include <orbit/orb-core/corba-orb-type.h>
#include <orbit/orb-core/corba-typecode-type.h>
#include <orbit/orb-core/corba-ir.h>

CORBA_ORB CORBA_ORB_init(int *argc, char **argv, CORBA_ORBid orb_identifier, CORBA_Environment *ev);

CORBA_char *CORBA_ORB_object_to_string(CORBA_ORB orb,
				       const CORBA_Object obj,
				       CORBA_Environment * ev);
CORBA_Object CORBA_ORB_string_to_object(CORBA_ORB orb,
					const CORBA_char * str,
					CORBA_Environment * ev);
void CORBA_ORB_create_list(CORBA_ORB orb, const CORBA_long count,
			   CORBA_NVList * new_list,
			   CORBA_Environment * ev);
void CORBA_ORB_create_operation_list(CORBA_ORB orb,
				     const CORBA_OperationDef oper,
				     CORBA_NVList *new_list,
				     CORBA_Environment * ev);
void CORBA_ORB_get_default_context(CORBA_ORB orb, CORBA_Context * ctx,
				   CORBA_Environment * ev);
void CORBA_ORB_send_multiple_requests_oneway(CORBA_ORB orb,
					     const CORBA_RequestSeq * req,
					     CORBA_Environment * ev);
void CORBA_ORB_send_multiple_requests_deferred(CORBA_ORB orb,
					       const CORBA_RequestSeq *req,
					       CORBA_Environment * ev);
CORBA_boolean CORBA_ORB_poll_next_response(CORBA_ORB orb,
					   CORBA_Environment * ev);
void CORBA_ORB_get_next_response(CORBA_ORB orb, CORBA_Request * req,
				 CORBA_Environment * ev);
CORBA_boolean CORBA_ORB_get_service_information(CORBA_ORB orb,
						const CORBA_ServiceType service_type,
						CORBA_ServiceInformation **service_information,
						CORBA_Environment * ev);
CORBA_ORB_ObjectIdList *CORBA_ORB_list_initial_services(CORBA_ORB orb,
							CORBA_Environment *ev);
CORBA_Object CORBA_ORB_resolve_initial_references(CORBA_ORB orb,
						   const CORBA_ORB_ObjectId identifier,
						   CORBA_Environment * ev);
CORBA_TypeCode CORBA_ORB_create_struct_tc(CORBA_ORB orb,
					  const CORBA_RepositoryId id,
					  const CORBA_Identifier name,
					  const CORBA_StructMemberSeq *members, CORBA_Environment * ev);
CORBA_TypeCode CORBA_ORB_create_union_tc(CORBA_ORB orb,
					 const CORBA_RepositoryId id,
					 const CORBA_Identifier name,
					 const CORBA_TypeCode discriminator_type,
					 const CORBA_UnionMemberSeq *members, CORBA_Environment * ev);
CORBA_TypeCode CORBA_ORB_create_enum_tc(CORBA_ORB orb,
					const CORBA_RepositoryId id,
					const CORBA_Identifier name,
					const CORBA_EnumMemberSeq *members, CORBA_Environment * ev);
CORBA_TypeCode CORBA_ORB_create_alias_tc(CORBA_ORB orb,
					 const CORBA_RepositoryId id,
					 const CORBA_Identifier name,
					 const CORBA_TypeCode original_type,
					 CORBA_Environment * ev);
CORBA_TypeCode CORBA_ORB_create_exception_tc(CORBA_ORB orb,
					     const CORBA_RepositoryId id,
					     const CORBA_Identifier name,
					     const CORBA_StructMemberSeq *members,
					     CORBA_Environment * ev);
CORBA_TypeCode CORBA_ORB_create_interface_tc(CORBA_ORB orb,
					     const CORBA_RepositoryId id,
					     const CORBA_Identifier name,
					     CORBA_Environment * ev);
CORBA_TypeCode CORBA_ORB_create_string_tc(CORBA_ORB orb,
					  const CORBA_unsigned_long bound,
					  CORBA_Environment * ev);
CORBA_TypeCode CORBA_ORB_create_wstring_tc(CORBA_ORB orb,
					   const CORBA_unsigned_long bound,
					   CORBA_Environment * ev);
CORBA_TypeCode CORBA_ORB_create_fixed_tc(CORBA_ORB orb,
					 const CORBA_unsigned_short digits,
					 const CORBA_short scale,
					 CORBA_Environment * ev);
CORBA_TypeCode CORBA_ORB_create_sequence_tc(CORBA_ORB orb,
					    const CORBA_unsigned_long bound,
					    const CORBA_TypeCode element_type,
					    CORBA_Environment * ev);
CORBA_TypeCode CORBA_ORB_create_recursive_sequence_tc(CORBA_ORB orb,
						      const CORBA_unsigned_long bound,
						      const CORBA_unsigned_long offset,
						      CORBA_Environment *ev);
CORBA_TypeCode CORBA_ORB_create_array_tc(CORBA_ORB orb,
					 const CORBA_unsigned_long length,
					 const CORBA_TypeCode element_type,
					 CORBA_Environment * ev);
CORBA_TypeCode CORBA_ORB_create_value_tc(CORBA_ORB orb,
					 const CORBA_RepositoryId id,
					 const CORBA_Identifier name,
					 const CORBA_ValueModifier type_modifier,
					 const CORBA_TypeCode concrete_base,
					 const CORBA_ValueMemberSeq *members,
					 CORBA_Environment * ev);
CORBA_TypeCode CORBA_ORB_create_value_box_tc(CORBA_ORB orb,
					     const CORBA_RepositoryId id,
					     const CORBA_Identifier name,
					     const CORBA_TypeCode boxed_type,
					     CORBA_Environment * ev);
CORBA_TypeCode CORBA_ORB_create_native_tc(CORBA_ORB orb,
					  const CORBA_RepositoryId id,
					  const CORBA_Identifier name,
					  CORBA_Environment * ev);
CORBA_TypeCode CORBA_ORB_create_recursive_tc(CORBA_ORB orb,
					     const CORBA_RepositoryId id,
					     CORBA_Environment * ev);
CORBA_TypeCode CORBA_ORB_create_abstract_interface_tc(CORBA_ORB orb,
						      const CORBA_RepositoryId id,
						      const CORBA_Identifier name,
						      CORBA_Environment *ev);
CORBA_boolean CORBA_ORB_work_pending(CORBA_ORB orb,
				     CORBA_Environment * ev);
void CORBA_ORB_perform_work(CORBA_ORB orb, CORBA_Environment * ev);
void CORBA_ORB_run(CORBA_ORB orb, CORBA_Environment * ev);
void CORBA_ORB_shutdown(CORBA_ORB orb,
			const CORBA_boolean wait_for_completion,
			CORBA_Environment * ev);
void CORBA_ORB_destroy(CORBA_ORB orb, CORBA_Environment * ev);
CORBA_Policy CORBA_ORB_create_policy(CORBA_ORB orb,
				     const CORBA_PolicyType type,
				     const CORBA_any * val,
				     CORBA_Environment * ev);
CORBA_ValueFactory CORBA_ORB_register_value_factory(CORBA_ORB orb,
						    const CORBA_RepositoryId id,
						    const CORBA_ValueFactory factory,
						    CORBA_Environment *ev);
void CORBA_ORB_unregister_value_factory(CORBA_ORB orb,
					const CORBA_RepositoryId id,
					CORBA_Environment * ev);
CORBA_ValueFactory CORBA_ORB_lookup_value_factory(CORBA_ORB orb,
						  const CORBA_RepositoryId id,
						  CORBA_Environment * ev);

/* ORBit extension */
void CORBA_ORB_set_initial_reference(CORBA_ORB orb, CORBA_ORB_ObjectId identifier, CORBA_Object obj, CORBA_Environment *ev);
#endif
