/*
Copyright © 2018 Atlas Engineer LLC.
Use of this file is governed by the license that can be found in LICENSE.
*/
#pragma once

#include <stdarg.h>
#include <glib.h>
#include <libsoup/soup.h>

#include "window.h"

typedef GVariant * (*ServerCallback) (SoupXMLRPCParams *);

// The params variant type string is "av", and the embedded "v"'s type string
// can be anything.
// To ease params manipulation, we "unwrap" the variant array to return a tuple
// of all element types.
GVariant *server_unwrap_params(SoupXMLRPCParams *params) {
	GError *error = NULL;
	GVariant *variant = soup_xmlrpc_params_parse(params, NULL, &error);
	if (error) {
		g_warning("Malformed method parameters: %s", error->message);
		g_error_free(error);
		return NULL;
	}
	if (!g_variant_check_format_string(variant, "av", FALSE)) {
		g_warning("Malformed parameter value: %s", g_variant_get_type_string(variant));
		g_error_free(error);
		return NULL;
	}

	GVariantBuilder builder;
	g_variant_builder_init(&builder, G_VARIANT_TYPE_TUPLE);
	GVariantIter iter;
	g_variant_iter_init(&iter, variant);
	GVariant *child;
	while (g_variant_iter_loop(&iter, "v", &child)) {
		g_variant_builder_add_value(&builder, child);
	}

	return g_variant_builder_end(&builder);
}

static GVariant *server_window_make(SoupXMLRPCParams *_params) {
	Window *window = window_init();
	window->identifier = strdup(akd_insert_element(state.windows, window));
	window->minibuffer->parent_window_identifier = strdup(window->identifier);
	return g_variant_new_string(window->identifier);
}

static GVariant *server_window_delete(SoupXMLRPCParams *params) {
	GVariant *unwrapped_params = server_unwrap_params(params);
	if (!unwrapped_params) {
		return g_variant_new_boolean(FALSE);
	}
	const char *a_key = NULL;
	g_variant_get(unwrapped_params, "(s)", &a_key);
	g_debug("Method parameter: %s", a_key);

	akd_remove_object_for_key(state.windows, a_key);
	return g_variant_new_boolean(TRUE);
}

static GVariant *server_window_active(SoupXMLRPCParams *_params) {
	// TODO: If we run a GTK application, then we could call
	// gtk_application_get_active_window() and get the identifier from there.
	// We could also lookup the active window in gtk_window_list_toplevels().
	GHashTableIter iter;
	gpointer key, value;

	g_hash_table_iter_init(&iter, state.windows->dict);
	while (g_hash_table_iter_next(&iter, &key, &value)) {
		Window *window = (Window *)value;
		if (gtk_window_is_active(GTK_WINDOW(window->base))) {
			g_debug("Active window identifier: %s", window->identifier);
			return g_variant_new_string(window->identifier);
		}
	}

	// TODO: Is "-1" a good name for a window that does not exist?
	g_debug("No active window");
	return g_variant_new_string("-1");
}

static GVariant *server_window_set_active_buffer(SoupXMLRPCParams *params) {
	GVariant *unwrapped_params = server_unwrap_params(params);
	if (!unwrapped_params) {
		return g_variant_new_boolean(FALSE);
	}
	const char *window_id = NULL;
	const char *buffer_id = NULL;
	g_variant_get(unwrapped_params, "(ss)", &window_id, &buffer_id);
	g_debug("Method parameter: window_id %s, buffer_id %s", window_id, buffer_id);

	Window *window = akd_object_for_key(state.windows, window_id);
	Buffer *buffer = akd_object_for_key(state.buffers, buffer_id);
	window_set_active_buffer(window, buffer);
	return g_variant_new_boolean(TRUE);
}

static GVariant *server_buffer_make(SoupXMLRPCParams *_params) {
	Buffer *buffer = buffer_init();
	buffer->identifier = strdup(akd_insert_element(state.buffers, buffer));
	return g_variant_new_string(buffer->identifier);
}

static GVariant *server_buffer_delete(SoupXMLRPCParams *params) {
	GVariant *unwrapped_params = server_unwrap_params(params);
	if (!unwrapped_params) {
		return g_variant_new_boolean(FALSE);
	}
	const char *a_key = NULL;
	g_variant_get(unwrapped_params, "(s)", &a_key);
	g_debug("Method parameter: %s", a_key);

	akd_remove_object_for_key(state.buffers, a_key);
	return g_variant_new_boolean(TRUE);
}

static GVariant *server_buffer_evaluate(SoupXMLRPCParams *params) {
	GVariant *unwrapped_params = server_unwrap_params(params);
	if (!unwrapped_params) {
		return g_variant_new_boolean(FALSE);
	}
	const char *buffer_id = NULL;
	const char *javascript = NULL;
	g_variant_get(unwrapped_params, "(ss)", &buffer_id, &javascript);
	g_debug("Method parameter: buffer_id %s, javascript %s", buffer_id, javascript);

	Buffer *buffer = akd_object_for_key(state.buffers, buffer_id);
	char *result = buffer_evaluate(buffer, javascript);
	return g_variant_new_string(result);
}

static GVariant *server_window_set_minibuffer_height(SoupXMLRPCParams *params) {
	GVariant *unwrapped_params = server_unwrap_params(params);
	if (!unwrapped_params) {
		return g_variant_new_boolean(FALSE);
	}
	const char *window_id = NULL;
	int minibuffer_height = 0;
	g_variant_get(unwrapped_params, "(si)", &window_id,
		&minibuffer_height);
	g_debug("Method parameter: window_id %s, minibuffer_height %i", window_id,
		minibuffer_height);

	Window *window = akd_object_for_key(state.windows, window_id);
	gint64 result = window_set_minibuffer_height(window, minibuffer_height);
	return g_variant_new_int64(result);
}

static GVariant *server_minibuffer_evaluate(SoupXMLRPCParams *params) {
	GVariant *unwrapped_params = server_unwrap_params(params);
	if (!unwrapped_params) {
		return g_variant_new_boolean(FALSE);
	}
	const char *window_id = NULL;
	const char *javascript = NULL;
	g_variant_get(unwrapped_params, "(ss)", &window_id, &javascript);
	g_debug("Method parameters: window_id %s, javascript %s", window_id, javascript);

	Window *window = akd_object_for_key(state.windows, window_id);
	Minibuffer *minibuffer = window->minibuffer;
	char *result = minibuffer_evaluate(minibuffer, javascript);
	return g_variant_new_string(result);
}

static void server_handler(SoupServer *_server, SoupMessage *msg,
	const char *path, GHashTable *_query,
	SoupClientContext *_context, gpointer _data) {
	// Log of the request.
	{
		const char *name, *value;
		SoupMessageHeadersIter iter;
		GString *pretty_message = g_string_new("HTTP request:\n");
		g_string_append_printf(pretty_message, "%s %s HTTP/1.%d\n", msg->method, path,
			soup_message_get_http_version(msg));
		soup_message_headers_iter_init(&iter, msg->request_headers);
		while (soup_message_headers_iter_next(&iter, &name, &value)) {
			g_string_append_printf(pretty_message, "%s: %s\n", name, value);
		}
		if (msg->request_body->length == 0) {
			g_warning("Empty HTTP request");
			return;
		}
		g_string_append_printf(pretty_message, "%s", msg->request_body->data);
		g_debug("%s", pretty_message->str);
		g_string_free(pretty_message, TRUE);
	}

	SoupXMLRPCParams *params = NULL;
	GError *error = NULL;
	char *method_name = soup_xmlrpc_parse_request(msg->request_body->data,
			msg->request_body->length,
			&params,
			&error);
	if (error) {
		g_warning("Malformed XML-RPC request: %s", error->message);
		g_error_free(error);
		return;
	}

	ServerCallback callback = NULL;
	gboolean found = g_hash_table_lookup_extended(state.server_callbacks, method_name,
			NULL, (gpointer *)&callback);
	if (!found) {
		g_warning("Unknown method: %s", method_name);
		return;
	}
	g_debug("Method name: %s", method_name);

	// TODO: Make floating result so that they are consumed in soup_xmlrpc_message_set_response?
	GVariant *result = callback(params);

	soup_xmlrpc_params_free(params);

	soup_xmlrpc_message_set_response(msg, result, &error);
	if (error) {
		g_warning("Failed to set XML-RPC response: %s", error->message);
		g_error_free(error);
	}

	g_debug("Response: %d %s", msg->status_code, msg->reason_phrase);
}

void start_server() {
	// TODO: Server logging?
	// TODO: libsoup's examples don't unref the server.  Should we?
	SoupServer *server = soup_server_new(
		SOUP_SERVER_SERVER_HEADER, APPNAME,
		NULL);

	GError *error = NULL;
	soup_server_listen_all(server, 8082, 0, &error);
	if (error) {
		g_printerr("Unable to create server: %s\n", error->message);
		g_error_free(error);
		exit(1);
	}
	g_debug("Starting XMLRPC server");
	soup_server_add_handler(server, NULL, server_handler, NULL, NULL);

	// Initialize global state.
	state.server_callbacks = g_hash_table_new(g_str_hash, g_str_equal);
	state.windows = akd_init((GDestroyNotify)&window_delete);
	state.buffers = akd_init((GDestroyNotify)&buffer_delete);

	// Register callbacks.
	g_hash_table_insert(state.server_callbacks, "window.make", &server_window_make);
	g_hash_table_insert(state.server_callbacks, "window.delete", &server_window_delete);
	g_hash_table_insert(state.server_callbacks, "window.active", &server_window_active);
	g_hash_table_insert(state.server_callbacks, "window.set.active.buffer", &server_window_set_active_buffer);
	g_hash_table_insert(state.server_callbacks, "buffer.make", &server_buffer_make);
	g_hash_table_insert(state.server_callbacks, "buffer.delete", &server_buffer_delete);
	g_hash_table_insert(state.server_callbacks, "buffer.evaluate.javascript", &server_buffer_evaluate);
	g_hash_table_insert(state.server_callbacks, "window.set.minibuffer.height", &server_window_set_minibuffer_height);
	g_hash_table_insert(state.server_callbacks, "minibuffer.evaluate.javascript", &server_minibuffer_evaluate);
}

void stop_server() {
	akd_free(state.windows);
	akd_free(state.buffers);
	g_hash_table_unref(state.server_callbacks);
}
