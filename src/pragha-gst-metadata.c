/* GStreamer
 * Copyright (C) 2003 Thomas Vander Stichele <thomas@apestaart.org>
 *               2003 Benjamin Otte <in7y118@public.uni-hamburg.de>
 *               2005 Andy Wingo <wingo@pobox.com>
 *               2005 Jan Schmidt <thaytan@mad.scientist.com>
 *
 * gst-metadata.c: Use GStreamer to display metadata within files.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "pragha-gst-metadata.h"
#include "pragha-musicobject.h"

#include <string.h>
#include <stdlib.h>
#include <locale.h>
#include <gst/gst.h>
#include <glib.h>

static gboolean
message_loop (GstElement * element, GstTagList ** tags)
{
	GstBus *bus;
	gboolean done = FALSE;

	bus = gst_element_get_bus (element);
	g_return_val_if_fail (bus != NULL, FALSE);
 	g_return_val_if_fail (tags != NULL, FALSE);

	while (!done) {
		GstMessage *message;

		message = gst_bus_pop (bus);
		if (message == NULL)
			/* All messages read, we're done */
			break;

		switch (GST_MESSAGE_TYPE (message)) {
			case GST_MESSAGE_ERROR:
			case GST_MESSAGE_EOS:
				gst_message_unref (message);
				return TRUE;
			case GST_MESSAGE_TAG: {
				GstTagList *new_tags, *old_tags;

				gst_message_parse_tag (message, &new_tags);
				if (*tags) {
					old_tags = *tags;
					*tags = gst_tag_list_merge (old_tags, new_tags, GST_TAG_MERGE_KEEP);
					#if GST_CHECK_VERSION (1, 0, 0)
					gst_tag_list_unref (old_tags);
					#else
					gst_tag_list_free (old_tags);
					#endif
			 	}
			 	else
					 *tags = new_tags;
				break;
			}
			default:
				break;
		}
		gst_message_unref (message);
	}
	gst_object_unref (bus);

	return TRUE;
}

static void
pragha_metadata_parser_mobj_set_tag (const GstTagList *list, const gchar *tag_name, gpointer data)
{
	PraghaMusicobject *mobj = data;
	gchar *tag = NULL;
	guint uitag = 0;
	guint64 ui64tag = 0;

	if(g_ascii_strcasecmp(GST_TAG_TITLE, tag_name) == 0) {
		gst_tag_list_get_string (list, tag_name, &tag);
		pragha_musicobject_set_title(mobj, tag);
		g_free(tag);
	}
	else if(g_ascii_strcasecmp(GST_TAG_ARTIST, tag_name) == 0) {
		gst_tag_list_get_string (list, tag_name, &tag);
		pragha_musicobject_set_artist(mobj, tag);
		g_free(tag);
	}
	else if(g_ascii_strcasecmp(GST_TAG_ALBUM, tag_name) == 0) {
		gst_tag_list_get_string (list, tag_name, &tag);
		pragha_musicobject_set_album(mobj, tag);
		g_free(tag);
	}
	else if(g_ascii_strcasecmp(GST_TAG_GENRE, tag_name) == 0) {
		gst_tag_list_get_string (list, tag_name, &tag);
		pragha_musicobject_set_genre(mobj, tag);
		g_free(tag);
	}
	else if(g_ascii_strcasecmp(GST_TAG_COMMENT, tag_name) == 0) {
		gst_tag_list_get_string (list, tag_name, &tag);
		pragha_musicobject_set_comment(mobj, tag);
		g_free(tag);
	}
	else if(g_ascii_strcasecmp(GST_TAG_TRACK_NUMBER, tag_name) == 0) {
		gst_tag_list_get_uint (list, GST_TAG_DURATION, &uitag);
		pragha_musicobject_set_track_no(mobj, uitag);
		g_free(tag);
	}
	else if(g_ascii_strcasecmp(GST_TAG_DURATION, tag_name) == 0) {
		gst_tag_list_get_uint64 (list, GST_TAG_DURATION, &ui64tag);
		pragha_musicobject_set_length(mobj, GST_TIME_AS_SECONDS (ui64tag));
		g_free(tag);
	}
	else if(g_ascii_strcasecmp(GST_TAG_BITRATE, tag_name) == 0) {
		gst_tag_list_get_uint64 (list, GST_TAG_DURATION, &ui64tag);
		pragha_musicobject_set_bitrate(mobj, GST_TIME_AS_SECONDS (ui64tag));
		g_free(tag);
	}
	/*
	 *TODO: Channels and saplerate must get from caps.. And year ¿?
	 */
}

PraghaMusicobject *
pragha_metadata_parser_get_mobj (PraghaGstMetadataParser *parser, const gchar *filename)
{
	GstStateChangeReturn sret;
	GstState state;
	GstTagList *tags = NULL;

	PraghaMusicobject *mobj = NULL;

	g_object_set (parser->source, "location", filename, NULL);

	GST_DEBUG ("Starting reading for %s", filename);

	/* Decodebin will only commit to PAUSED if it actually finds a type;
	 * otherwise the state change fails */
	sret = gst_element_set_state (GST_ELEMENT (parser->pipeline), GST_STATE_PAUSED);

	if (GST_STATE_CHANGE_ASYNC == sret) {
		if (GST_STATE_CHANGE_SUCCESS !=
			gst_element_get_state (GST_ELEMENT (parser->pipeline), &state, NULL,
				5 * GST_SECOND)) {
			g_print ("State change failed for %s. Aborting\n", filename);
			goto bad;
		}
	}
	else if (sret != GST_STATE_CHANGE_SUCCESS) {
		g_print ("%s - Could not read file\n", filename);
		return NULL;
	}

	if (!message_loop (GST_ELEMENT (parser->pipeline), &tags)) {
		g_print ("Failed in message reading for %s\n", filename);
	}

	if (tags) {
		g_print ("Metadata for %s:\n", filename);
		mobj = g_object_new (PRAGHA_TYPE_MUSICOBJECT,
		                     "file", filename,
		                     NULL);
		gst_tag_list_foreach (tags, pragha_metadata_parser_mobj_set_tag, mobj);
		#if GST_CHECK_VERSION (1, 0, 0)
		gst_tag_list_unref (tags);
		#else
		gst_tag_list_free (tags);
		#endif
		tags = NULL;
	}
	else
		g_print ("No metadata found for %s\n", filename);

bad:
	sret = gst_element_set_state (GST_ELEMENT (parser->pipeline), GST_STATE_NULL);
	if (GST_STATE_CHANGE_ASYNC == sret) {
		if (GST_STATE_CHANGE_FAILURE ==
			gst_element_get_state (GST_ELEMENT (parser->pipeline),
				                   &state, NULL,
				                   GST_CLOCK_TIME_NONE)) {
			g_print ("State change failed. Aborting");
			return NULL;
		}
	}
	else if (sret != GST_STATE_CHANGE_SUCCESS) {
		g_print ("State change failed. Aborting\n");
		return NULL;
	}
	return mobj;
}

void
pragha_metadata_parser_free(PraghaGstMetadataParser *parser)
{
	gst_object_unref (parser->pipeline);
	g_slice_free(PraghaGstMetadataParser, parser);
}

PraghaGstMetadataParser *
pragha_metadata_parser_new(void)
{
	GstElement *decodebin;
	GstElement *pipeline = NULL;
	GstElement *source = NULL;

	PraghaGstMetadataParser *parser;

	parser = g_slice_new0(PraghaGstMetadataParser);

	pipeline = gst_pipeline_new (NULL);
	source = gst_element_factory_make ("filesrc", "source");
	g_assert (GST_IS_ELEMENT (source));
	decodebin = gst_element_factory_make ("decodebin", "decodebin");
	g_assert (GST_IS_ELEMENT (decodebin));

	gst_bin_add_many (GST_BIN (pipeline), source, decodebin, NULL);
	gst_element_link (source, decodebin);

	parser->pipeline = pipeline;
	parser->source = source;

	return parser;
}

