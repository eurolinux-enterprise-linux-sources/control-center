/*
 * Generated by gdbus-codegen 2.48.0. DO NOT EDIT.
 *
 * The license of this code is the same as for the source it was derived from.
 */

#ifndef __ORG_GNOME_SETTINGSDAEMON_SHARING_H__
#define __ORG_GNOME_SETTINGSDAEMON_SHARING_H__

#include <gio/gio.h>

G_BEGIN_DECLS


/* ------------------------------------------------------------------------ */
/* Declarations for org.gnome.SettingsDaemon.Sharing */

#define GSD_TYPE_SHARING (gsd_sharing_get_type ())
#define GSD_SHARING(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), GSD_TYPE_SHARING, GsdSharing))
#define GSD_IS_SHARING(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), GSD_TYPE_SHARING))
#define GSD_SHARING_GET_IFACE(o) (G_TYPE_INSTANCE_GET_INTERFACE ((o), GSD_TYPE_SHARING, GsdSharingIface))

struct _GsdSharing;
typedef struct _GsdSharing GsdSharing;
typedef struct _GsdSharingIface GsdSharingIface;

struct _GsdSharingIface
{
  GTypeInterface parent_iface;


  gboolean (*handle_disable_service) (
    GsdSharing *object,
    GDBusMethodInvocation *invocation,
    const gchar *arg_service_name,
    const gchar *arg_network);

  gboolean (*handle_enable_service) (
    GsdSharing *object,
    GDBusMethodInvocation *invocation,
    const gchar *arg_service_name);

  gboolean (*handle_list_networks) (
    GsdSharing *object,
    GDBusMethodInvocation *invocation,
    const gchar *arg_service_name);

  const gchar * (*get_carrier_type) (GsdSharing *object);

  const gchar * (*get_current_network) (GsdSharing *object);

  const gchar * (*get_current_network_name) (GsdSharing *object);

  guint  (*get_sharing_status) (GsdSharing *object);

};

GType gsd_sharing_get_type (void) G_GNUC_CONST;

GDBusInterfaceInfo *gsd_sharing_interface_info (void);
guint gsd_sharing_override_properties (GObjectClass *klass, guint property_id_begin);


/* D-Bus method call completion functions: */
void gsd_sharing_complete_enable_service (
    GsdSharing *object,
    GDBusMethodInvocation *invocation);

void gsd_sharing_complete_disable_service (
    GsdSharing *object,
    GDBusMethodInvocation *invocation);

void gsd_sharing_complete_list_networks (
    GsdSharing *object,
    GDBusMethodInvocation *invocation,
    GVariant *networks);



/* D-Bus method calls: */
void gsd_sharing_call_enable_service (
    GsdSharing *proxy,
    const gchar *arg_service_name,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean gsd_sharing_call_enable_service_finish (
    GsdSharing *proxy,
    GAsyncResult *res,
    GError **error);

gboolean gsd_sharing_call_enable_service_sync (
    GsdSharing *proxy,
    const gchar *arg_service_name,
    GCancellable *cancellable,
    GError **error);

void gsd_sharing_call_disable_service (
    GsdSharing *proxy,
    const gchar *arg_service_name,
    const gchar *arg_network,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean gsd_sharing_call_disable_service_finish (
    GsdSharing *proxy,
    GAsyncResult *res,
    GError **error);

gboolean gsd_sharing_call_disable_service_sync (
    GsdSharing *proxy,
    const gchar *arg_service_name,
    const gchar *arg_network,
    GCancellable *cancellable,
    GError **error);

void gsd_sharing_call_list_networks (
    GsdSharing *proxy,
    const gchar *arg_service_name,
    GCancellable *cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data);

gboolean gsd_sharing_call_list_networks_finish (
    GsdSharing *proxy,
    GVariant **out_networks,
    GAsyncResult *res,
    GError **error);

gboolean gsd_sharing_call_list_networks_sync (
    GsdSharing *proxy,
    const gchar *arg_service_name,
    GVariant **out_networks,
    GCancellable *cancellable,
    GError **error);



/* D-Bus property accessors: */
const gchar *gsd_sharing_get_current_network_name (GsdSharing *object);
gchar *gsd_sharing_dup_current_network_name (GsdSharing *object);
void gsd_sharing_set_current_network_name (GsdSharing *object, const gchar *value);

const gchar *gsd_sharing_get_current_network (GsdSharing *object);
gchar *gsd_sharing_dup_current_network (GsdSharing *object);
void gsd_sharing_set_current_network (GsdSharing *object, const gchar *value);

const gchar *gsd_sharing_get_carrier_type (GsdSharing *object);
gchar *gsd_sharing_dup_carrier_type (GsdSharing *object);
void gsd_sharing_set_carrier_type (GsdSharing *object, const gchar *value);

guint gsd_sharing_get_sharing_status (GsdSharing *object);
void gsd_sharing_set_sharing_status (GsdSharing *object, guint value);


/* ---- */

#define GSD_TYPE_SHARING_PROXY (gsd_sharing_proxy_get_type ())
#define GSD_SHARING_PROXY(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), GSD_TYPE_SHARING_PROXY, GsdSharingProxy))
#define GSD_SHARING_PROXY_CLASS(k) (G_TYPE_CHECK_CLASS_CAST ((k), GSD_TYPE_SHARING_PROXY, GsdSharingProxyClass))
#define GSD_SHARING_PROXY_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), GSD_TYPE_SHARING_PROXY, GsdSharingProxyClass))
#define GSD_IS_SHARING_PROXY(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), GSD_TYPE_SHARING_PROXY))
#define GSD_IS_SHARING_PROXY_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), GSD_TYPE_SHARING_PROXY))

typedef struct _GsdSharingProxy GsdSharingProxy;
typedef struct _GsdSharingProxyClass GsdSharingProxyClass;
typedef struct _GsdSharingProxyPrivate GsdSharingProxyPrivate;

struct _GsdSharingProxy
{
  /*< private >*/
  GDBusProxy parent_instance;
  GsdSharingProxyPrivate *priv;
};

struct _GsdSharingProxyClass
{
  GDBusProxyClass parent_class;
};

GType gsd_sharing_proxy_get_type (void) G_GNUC_CONST;

#if GLIB_CHECK_VERSION(2, 44, 0)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (GsdSharingProxy, g_object_unref)
#endif

void gsd_sharing_proxy_new (
    GDBusConnection     *connection,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GAsyncReadyCallback  callback,
    gpointer             user_data);
GsdSharing *gsd_sharing_proxy_new_finish (
    GAsyncResult        *res,
    GError             **error);
GsdSharing *gsd_sharing_proxy_new_sync (
    GDBusConnection     *connection,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GError             **error);

void gsd_sharing_proxy_new_for_bus (
    GBusType             bus_type,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GAsyncReadyCallback  callback,
    gpointer             user_data);
GsdSharing *gsd_sharing_proxy_new_for_bus_finish (
    GAsyncResult        *res,
    GError             **error);
GsdSharing *gsd_sharing_proxy_new_for_bus_sync (
    GBusType             bus_type,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GError             **error);


/* ---- */

#define GSD_TYPE_SHARING_SKELETON (gsd_sharing_skeleton_get_type ())
#define GSD_SHARING_SKELETON(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), GSD_TYPE_SHARING_SKELETON, GsdSharingSkeleton))
#define GSD_SHARING_SKELETON_CLASS(k) (G_TYPE_CHECK_CLASS_CAST ((k), GSD_TYPE_SHARING_SKELETON, GsdSharingSkeletonClass))
#define GSD_SHARING_SKELETON_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), GSD_TYPE_SHARING_SKELETON, GsdSharingSkeletonClass))
#define GSD_IS_SHARING_SKELETON(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), GSD_TYPE_SHARING_SKELETON))
#define GSD_IS_SHARING_SKELETON_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), GSD_TYPE_SHARING_SKELETON))

typedef struct _GsdSharingSkeleton GsdSharingSkeleton;
typedef struct _GsdSharingSkeletonClass GsdSharingSkeletonClass;
typedef struct _GsdSharingSkeletonPrivate GsdSharingSkeletonPrivate;

struct _GsdSharingSkeleton
{
  /*< private >*/
  GDBusInterfaceSkeleton parent_instance;
  GsdSharingSkeletonPrivate *priv;
};

struct _GsdSharingSkeletonClass
{
  GDBusInterfaceSkeletonClass parent_class;
};

GType gsd_sharing_skeleton_get_type (void) G_GNUC_CONST;

#if GLIB_CHECK_VERSION(2, 44, 0)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (GsdSharingSkeleton, g_object_unref)
#endif

GsdSharing *gsd_sharing_skeleton_new (void);


G_END_DECLS

#endif /* __ORG_GNOME_SETTINGSDAEMON_SHARING_H__ */
