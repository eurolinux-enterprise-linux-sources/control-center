/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*-
 *
 * Copyright (C) 2012 Richard Hughes <richard@hughsie.com>
 *
 * Licensed under the GNU General Public License Version 2
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "config.h"

#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "cc-color-common.h"
#include "cc-color-profile.h"

struct _CcColorProfilePrivate
{
  GtkWidget   *box;
  CdDevice    *device;
  CdProfile   *profile;
  gboolean     is_default;
  gchar       *sortable;
  GtkWidget   *widget_description;
  GtkWidget   *widget_image;
  GtkWidget   *widget_info;
  GSettings   *settings;
  guint        device_changed_id;
  guint        profile_changed_id;
};

#define GCM_SETTINGS_RECALIBRATE_PRINTER_THRESHOLD      "recalibrate-printer-threshold"
#define GCM_SETTINGS_RECALIBRATE_DISPLAY_THRESHOLD      "recalibrate-display-threshold"


#define IMAGE_WIDGET_PADDING 12

G_DEFINE_TYPE (CcColorProfile, cc_color_profile, GTK_TYPE_LIST_BOX_ROW)

enum
{
  PROP_0,
  PROP_DEVICE,
  PROP_PROFILE,
  PROP_IS_DEFAULT,
  PROP_LAST
};

static gchar *
cc_color_profile_get_profile_date (CdProfile *profile)
{
  gint64 created;
  gchar *string = NULL;
  GDateTime *dt = NULL;

  /* get profile age */
  created = cd_profile_get_created (profile);
  if (created == 0)
    goto out;
  dt = g_date_time_new_from_unix_utc (created);
  string = g_date_time_format (dt, "%x");
out:
  if (dt != NULL)
    g_date_time_unref (dt);
  return string;
}

static gchar *
gcm_prefs_get_profile_title (CdProfile *profile)
{
  CdColorspace colorspace;
  const gchar *tmp;
  GString *str;

  str = g_string_new ("");

  /* add date only if it's a calibration profile or the profile has
   * not been tagged with this data */
  tmp = cd_profile_get_metadata_item (profile, CD_PROFILE_METADATA_DATA_SOURCE);
  if (tmp == NULL || g_strcmp0 (tmp, CD_PROFILE_METADATA_DATA_SOURCE_CALIB) == 0)
    {
      tmp = cc_color_profile_get_profile_date (profile);
      if (tmp != NULL)
        g_string_append_printf (str, "%s - ", tmp);
    }
  else if (g_strcmp0 (tmp, CD_PROFILE_METADATA_DATA_SOURCE_STANDARD) == 0)
    {
      /* TRANSLATORS: standard spaces are well known colorspaces like
       * sRGB, AdobeRGB and ProPhotoRGB */
      g_string_append_printf (str, "%s - ", _("Standard Space"));
    }
  else if (g_strcmp0 (tmp, CD_PROFILE_METADATA_DATA_SOURCE_TEST) == 0)
    {
      /* TRANSLATORS: test profiles do things like changing the screen
       * a different color, or swap the red and green channels */
      g_string_append_printf (str, "%s - ", _("Test Profile"));
    }
  else if (g_strcmp0 (tmp, CD_PROFILE_METADATA_DATA_SOURCE_EDID) == 0)
    {
      /* TRANSLATORS: automatic profiles are generated automatically
       * by the color management system based on manufacturing data,
       * for instance the default monitor profile is created from the
       * primaries specified in the monitor EDID */
      g_string_append_printf (str, "%s - ", C_("Automatically generated profile", "Automatic"));
    }

  /* add quality if it exists */
  tmp = cd_profile_get_metadata_item (profile, CD_PROFILE_METADATA_QUALITY);
  if (g_strcmp0 (tmp, CD_PROFILE_METADATA_QUALITY_LOW) == 0)
    {
      /* TRANSLATORS: the profile quality - low quality profiles take
       * much less time to generate but may be a poor reflection of the
       * device capability */
      g_string_append_printf (str, "%s - ", C_("Profile quality", "Low Quality"));
    }
  else if (g_strcmp0 (tmp, CD_PROFILE_METADATA_QUALITY_MEDIUM) == 0)
    {
      /* TRANSLATORS: the profile quality */
      g_string_append_printf (str, "%s - ", C_("Profile quality", "Medium Quality"));
    }
  else if (g_strcmp0 (tmp, CD_PROFILE_METADATA_QUALITY_HIGH) == 0)
    {
      /* TRANSLATORS: the profile quality - high quality profiles take
       * a *long* time, and have the best calibration and
       * characterisation data. */
      g_string_append_printf (str, "%s - ", C_("Profile quality", "High Quality"));
    }

  /* add profile description */
  tmp = cd_profile_get_title (profile);
  if (tmp != NULL)
    {
      g_string_append (str, tmp);
      goto out;
    }

  /* some meta profiles do not have ICC profiles */
  colorspace = cd_profile_get_colorspace (profile);
  if (colorspace == CD_COLORSPACE_RGB)
    {
      /* TRANSLATORS: this default RGB space is used for printers that
       * do not have additional printer profiles specified in the PPD */
      g_string_append (str, C_("Colorspace fallback", "Default RGB"));
      goto out;
    }
  if (colorspace == CD_COLORSPACE_CMYK)
    {
      /* TRANSLATORS: this default CMYK space is used for printers that
       * do not have additional printer profiles specified in the PPD */
      g_string_append (str, C_("Colorspace fallback", "Default CMYK"));
      goto out;
    }
  if (colorspace == CD_COLORSPACE_GRAY)
    {
      /* TRANSLATORS: this default gray space is used for printers that
       * do not have additional printer profiles specified in the PPD */
      g_string_append (str, C_("Colorspace fallback", "Default Gray"));
      goto out;
    }

  /* fall back to ID, ick */
  tmp = g_strdup (cd_profile_get_id (profile));
  g_string_append (str, tmp);
out:
  return g_string_free (str, FALSE);
}

static const gchar *
cc_color_profile_get_warnings (CcColorProfile *color_profile)
{
  CcColorProfilePrivate *priv = color_profile->priv;
  const gchar *tooltip = NULL;
  const guint seconds_in_one_day = 60 * 60 * 24;
  gint num_days;
  guint threshold_days = 0;

  /* autogenerated printer defaults */
  if (cd_device_get_kind (priv->device) == CD_DEVICE_KIND_PRINTER &&
      cd_profile_get_filename (priv->profile) == NULL)
    {
      tooltip = _("Vendor supplied factory calibration data");
      goto out;
    }

  /* autogenerated profiles are crap */
  if (cd_device_get_kind (priv->device) == CD_DEVICE_KIND_DISPLAY &&
      cd_profile_get_kind (priv->profile) == CD_PROFILE_KIND_DISPLAY_DEVICE &&
      !cd_profile_get_has_vcgt (priv->profile))
    {
      tooltip = _("Full-screen display correction not possible with this profile");
      goto out;
    }

  /* greater than the calibration threshold for the device type */
  num_days = cd_profile_get_age (priv->profile) / seconds_in_one_day;
  if (cd_device_get_kind (priv->device) == CD_DEVICE_KIND_DISPLAY)
    {
      g_settings_get (priv->settings,
                      GCM_SETTINGS_RECALIBRATE_DISPLAY_THRESHOLD,
                      "u",
                      &threshold_days);
    }
  else if (cd_device_get_kind (priv->device) == CD_DEVICE_KIND_DISPLAY)
    {
      g_settings_get (priv->settings,
                      GCM_SETTINGS_RECALIBRATE_PRINTER_THRESHOLD,
                      "u",
                      &threshold_days);
    }
  if (threshold_days > 0 && num_days > threshold_days)
    {
      tooltip = _("This profile may no longer be accurate");
      goto out;
    }
out:
  return tooltip;
}

static void
cc_color_profile_refresh (CcColorProfile *color_profile)
{
  CcColorProfilePrivate *priv = color_profile->priv;
  const gchar *warnings;
  gchar *title = NULL;

  /* show the image if the profile is default */
  gtk_widget_set_visible (priv->widget_image, priv->is_default);
  gtk_widget_set_margin_start (priv->widget_description,
                              priv->is_default ? 0 : IMAGE_WIDGET_PADDING * 4);

  /* set the title */
  title = gcm_prefs_get_profile_title (priv->profile);
  gtk_label_set_markup (GTK_LABEL (priv->widget_description), title);
  g_free (title);

  /* show any information */
  warnings = cc_color_profile_get_warnings (color_profile);
  gtk_widget_set_visible (priv->widget_info, warnings != NULL);
  gtk_widget_set_tooltip_text (priv->widget_info, warnings);
}

CdDevice *
cc_color_profile_get_device (CcColorProfile *color_profile)
{
  g_return_val_if_fail (CC_IS_COLOR_PROFILE (color_profile), NULL);
  return color_profile->priv->device;
}

CdProfile *
cc_color_profile_get_profile (CcColorProfile *color_profile)
{
  g_return_val_if_fail (CC_IS_COLOR_PROFILE (color_profile), NULL);
  return color_profile->priv->profile;
}

const gchar *
cc_color_profile_get_sortable (CcColorProfile *color_profile)
{
  g_return_val_if_fail (CC_IS_COLOR_PROFILE (color_profile), NULL);
  return color_profile->priv->sortable;
}

gboolean
cc_color_profile_get_is_default (CcColorProfile *color_profile)
{
  g_return_val_if_fail (CC_IS_COLOR_PROFILE (color_profile), 0);
  return color_profile->priv->is_default;
}

void
cc_color_profile_set_is_default (CcColorProfile *color_profile, gboolean is_default)
{
  g_return_if_fail (CC_IS_COLOR_PROFILE (color_profile));
  color_profile->priv->is_default = is_default;
  cc_color_profile_refresh (color_profile);
}

static void
cc_color_profile_get_property (GObject *object, guint param_id,
                               GValue *value, GParamSpec *pspec)
{
  CcColorProfile *color_profile = CC_COLOR_PROFILE (object);
  switch (param_id)
    {
      case PROP_DEVICE:
        g_value_set_object (value, color_profile->priv->device);
        break;
      case PROP_PROFILE:
        g_value_set_object (value, color_profile->priv->profile);
        break;
      case PROP_IS_DEFAULT:
        g_value_set_boolean (value, color_profile->priv->is_default);
        break;
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
        break;
    }
}

static void
cc_color_profile_set_property (GObject *object, guint param_id,
                               const GValue *value, GParamSpec *pspec)
{
  CcColorProfile *color_profile = CC_COLOR_PROFILE (object);

  switch (param_id)
    {
      case PROP_DEVICE:
        color_profile->priv->device = g_value_dup_object (value);
        break;
      case PROP_PROFILE:
        color_profile->priv->profile = g_value_dup_object (value);
        break;
      case PROP_IS_DEFAULT:
        color_profile->priv->is_default = g_value_get_boolean (value);
        break;
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
        break;
    }
}

static void
cc_color_profile_finalize (GObject *object)
{
  CcColorProfile *color_profile = CC_COLOR_PROFILE (object);
  CcColorProfilePrivate *priv = color_profile->priv;

  if (priv->device_changed_id > 0)
    g_signal_handler_disconnect (priv->device, priv->device_changed_id);
  if (priv->profile_changed_id > 0)
    g_signal_handler_disconnect (priv->profile, priv->profile_changed_id);

  g_free (priv->sortable);
  g_object_unref (priv->device);
  g_object_unref (priv->profile);
  g_object_unref (priv->settings);

  G_OBJECT_CLASS (cc_color_profile_parent_class)->finalize (object);
}

static void
cc_color_profile_changed_cb (CdDevice *device,
                             CcColorProfile *color_profile)
{
  CdProfile *profile;
  CcColorProfilePrivate *priv = color_profile->priv;

  /* check to see if the default has changed */
  profile = cd_device_get_default_profile (device);
  if (profile != NULL)
    {
      priv->is_default = g_strcmp0 (cd_profile_get_object_path (profile),
                                    cd_profile_get_object_path (priv->profile)) == 0;
      g_object_unref (profile);
    }
  cc_color_profile_refresh (color_profile);
}

static const gchar *
cc_color_profile_get_profile_sort_data_source (CdProfile *profile)
{
  const gchar *tmp;
  tmp = cd_profile_get_metadata_item (profile, CD_PROFILE_METADATA_DATA_SOURCE);
  if (tmp == NULL)
    return "9";
  if (g_strcmp0 (tmp, CD_PROFILE_METADATA_DATA_SOURCE_CALIB) == 0)
    return "3";
  if (g_strcmp0 (tmp, CD_PROFILE_METADATA_DATA_SOURCE_STANDARD) == 0)
    return "2";
  if (g_strcmp0 (tmp, CD_PROFILE_METADATA_DATA_SOURCE_TEST) == 0)
    return "1";
  return "0";
}

static void
cc_color_profile_constructed (GObject *object)
{
  CcColorProfile *color_profile = CC_COLOR_PROFILE (object);
  CcColorProfilePrivate *priv = color_profile->priv;
  const gchar *sortable_data_source;
  gchar *sortable_device;
  gchar *title;

  /* watch to see if the default changes */
  priv->device_changed_id =
    g_signal_connect (priv->device, "changed",
                      G_CALLBACK (cc_color_profile_changed_cb), color_profile);
  priv->profile_changed_id =
    g_signal_connect (priv->profile, "changed",
                      G_CALLBACK (cc_color_profile_changed_cb), color_profile);

  /* sort the profiles in the list by:
   * 1. thier device (required)
   * 2. the data source (so calibration profiles are listed before autogenerated ones)
   * 3. the date the profiles were created (newest first)
   * 4. the alpha sorting of the filename
   */
  title = gcm_prefs_get_profile_title (priv->profile);
  sortable_device = cc_color_device_get_sortable_base (priv->device);
  sortable_data_source = cc_color_profile_get_profile_sort_data_source (priv->profile);
  priv->sortable = g_strdup_printf ("%s-%s-%012" G_GINT64_FORMAT "-%s",
                                    sortable_device,
                                    sortable_data_source,
                                    cd_profile_get_created (priv->profile),
                                    title);
  g_free (title);
  g_free (sortable_device);

  cc_color_profile_refresh (color_profile);
}

static void
cc_color_profile_class_init (CcColorProfileClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  object_class->get_property = cc_color_profile_get_property;
  object_class->set_property = cc_color_profile_set_property;
  object_class->constructed = cc_color_profile_constructed;
  object_class->finalize = cc_color_profile_finalize;

  g_object_class_install_property (object_class, PROP_DEVICE,
                                   g_param_spec_object ("device", NULL,
                                                        NULL,
                                                        CD_TYPE_DEVICE,
                                                        G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
  g_object_class_install_property (object_class, PROP_PROFILE,
                                   g_param_spec_object ("profile", NULL,
                                                        NULL,
                                                        CD_TYPE_PROFILE,
                                                        G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
  g_object_class_install_property (object_class, PROP_IS_DEFAULT,
                                   g_param_spec_boolean ("is-default", NULL,
                                                         NULL,
                                                         FALSE,
                                                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));

  g_type_class_add_private (klass, sizeof (CcColorProfilePrivate));
}

static void
cc_color_profile_init (CcColorProfile *color_profile)
{
  CcColorProfilePrivate *priv;
  GtkWidget *box;

  color_profile->priv = G_TYPE_INSTANCE_GET_PRIVATE (color_profile,
                                                     CC_TYPE_COLOR_PROFILE,
                                                     CcColorProfilePrivate);
  priv = color_profile->priv;
  priv->settings = g_settings_new (GCM_SETTINGS_SCHEMA);

  box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 9);

  /* default tick */
  priv->widget_image = gtk_image_new_from_icon_name ("object-select-symbolic", GTK_ICON_SIZE_MENU);
  gtk_widget_set_margin_start (priv->widget_image, IMAGE_WIDGET_PADDING);
  gtk_widget_set_margin_end (priv->widget_image, IMAGE_WIDGET_PADDING);
  gtk_box_pack_start (GTK_BOX (box), priv->widget_image, FALSE, FALSE, 0);

  /* description */
  priv->widget_description = gtk_label_new ("");
  gtk_widget_set_margin_top (priv->widget_description, 9);
  gtk_widget_set_margin_bottom (priv->widget_description, 9);
  gtk_misc_set_alignment (GTK_MISC (priv->widget_description), 0.0f, 0.5f);
  gtk_box_pack_start (GTK_BOX (box), priv->widget_description, TRUE, TRUE, 0);
  gtk_widget_show (priv->widget_description);

  /* profile warnings/info */
  priv->widget_info = gtk_image_new_from_icon_name ("dialog-information-symbolic", GTK_ICON_SIZE_MENU);
  gtk_widget_set_margin_start (priv->widget_info, IMAGE_WIDGET_PADDING);
  gtk_widget_set_margin_end (priv->widget_info, IMAGE_WIDGET_PADDING);
  gtk_box_pack_start (GTK_BOX (box), priv->widget_info, FALSE, FALSE, 0);

  /* refresh */
  gtk_container_add (GTK_CONTAINER (color_profile), box);
  gtk_widget_set_visible (box, TRUE);
}

GtkWidget *
cc_color_profile_new (CdDevice *device,
                      CdProfile *profile,
                      gboolean is_default)
{
  return g_object_new (CC_TYPE_COLOR_PROFILE,
                       "device", device,
                       "profile", profile,
                       "is-default", is_default,
                       NULL);
}

