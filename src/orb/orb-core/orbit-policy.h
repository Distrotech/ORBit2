/**
 * orbit-policy.h: re-enterancy policy object for client invocations
 *
 * Author:
 *   Michael Meeks (michael@ximian.com)
 *
 * Copyright 2003 Ximian, Inc.
 */
#ifndef _ORBIT_POLICY_H_
#define _ORBIT_POLICY_H_

#include <glib-object.h>
#include <orbit/orbit.h>

G_BEGIN_DECLS

#define ORBIT_TYPE_POLICY_EX        (ORBit_policy_ex_get_type ())
#define ORBIT_POLICY_EX(o)          (G_TYPE_CHECK_INSTANCE_CAST ((o), ORBIT_TYPE_POLICY, ORBitPolicy))
#define ORBIT_POLICY_EX_CLASS(k)    (G_TYPE_CHECK_CLASS_CAST((k), ORBIT_TYPE_POLICY, ORBitPolicyClass))
#define ORBIT_IS_POLICY_EX(o)       (G_TYPE_CHECK_INSTANCE_TYPE ((o), ORBIT_TYPE_POLICY))
#define ORBIT_IS_POLICY_EX_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), ORBIT_TYPE_POLICY))

struct _ORBitPolicy {
	GObject parent;

	PortableServer_POA poa_only;
};

typedef struct {
	GObjectClass parent_class;

	/* FIXME: virtualize 'verify policy' */
} ORBitPolicyClass;

gboolean ORBit_policy_validate (ORBitPolicy *policy);

G_END_DECLS

#endif /* _ORBIT_POLICY_H_ */
