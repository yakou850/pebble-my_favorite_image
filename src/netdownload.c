#include "netdownload.h"

#define MIN(a,b)  ((a) < (b) ? (a) : (b))

NetDownloadContext* netdownload_create_context(
	NetDownloadCallback callback, 
	NetDownloadCallback2 callback_ready, 
	NetDownloadCallback2 callback_error, 
	NetDownloadCallback3 callback_set_image_url, 
	NetDownloadCallback4 callback_set_update_interval) {
	NetDownloadContext *ctx = malloc(sizeof(NetDownloadContext));

	ctx->length = 0;
	ctx->index = 0;
	ctx->data = NULL;
	ctx->callback = callback;
	ctx->callback_ready = callback_ready;
	ctx->callback_error = callback_error;
	ctx->callback_set_image_url = callback_set_image_url;
	ctx->callback_set_update_interval = callback_set_update_interval;

	return ctx;
}

void netdownload_destroy_context(NetDownloadContext *ctx) {
	if (ctx->data) {
		free(ctx->data);
	}
	free(ctx);
}

void netdownload_initialize(
	NetDownloadCallback callback, 
	NetDownloadCallback2 callback_ready, 
	NetDownloadCallback2 callback_error, 
	NetDownloadCallback3 callback_set_image_url, 
	NetDownloadCallback4 callback_set_update_interval) {
	NetDownloadContext *ctx = netdownload_create_context(callback, callback_ready, callback_error, callback_set_image_url, callback_set_update_interval);
	APP_LOG(APP_LOG_LEVEL_DEBUG, "NetDownloadContext = %p", ctx);
	app_message_set_context(ctx);

	app_message_register_inbox_received(netdownload_receive);
	app_message_register_inbox_dropped(netdownload_dropped);
	app_message_register_outbox_sent(netdownload_out_success);
	app_message_register_outbox_failed(netdownload_out_failed);
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Max buffer sizes are %li / %li",
			app_message_inbox_size_maximum(),
			app_message_outbox_size_maximum());
	app_message_open(app_message_inbox_size_maximum(), MIN(512, app_message_outbox_size_maximum()));
}

void netdownload_deinitialize() {
	netdownload_destroy_context(app_message_get_context());
	app_message_set_context(NULL);
}

void netdownload_request(char *url) {
	//APP_LOG(APP_LOG_LEVEL_DEBUG, "download = %s", url);
	if (!url || strlen(url) == 0) {
		APP_LOG(APP_LOG_LEVEL_DEBUG, "skip");
		return;
	}

	DictionaryIterator *outbox;
	app_message_outbox_begin(&outbox);
	// Tell the javascript how big we want each chunk of data:
	// max possible size - dictionary overhead with one zero-byte Tuple in it (since rest of tuple is our data).
	uint32_t chunk_size = app_message_inbox_size_maximum() - dict_calc_buffer_size(1, 0);
	dict_write_int(outbox, NETDL_CHUNK_SIZE, &chunk_size, sizeof(uint32_t), false);
	// Send the URL
	dict_write_cstring(outbox, NETDL_URL, url);

	app_message_outbox_send();
}

void netdownload_destroy(NetDownload *image) {
	// We malloc'd that memory before creating the GBitmap
	// We are responsible for freeing it.
	if (image) {
		free(image->data);
		free(image);
	}
}

void netdownload_receive(DictionaryIterator *iter, void *context) {
	NetDownloadContext *ctx = (NetDownloadContext*) context;

	Tuple *url1 = dict_find(iter, KEY_IMAGE_URL_1);
	Tuple *url2 = dict_find(iter, KEY_IMAGE_URL_2);
	Tuple *url3 = dict_find(iter, KEY_IMAGE_URL_3);
	Tuple *url4 = dict_find(iter, KEY_IMAGE_URL_4);
	Tuple *url5 = dict_find(iter, KEY_IMAGE_URL_5);
	Tuple *update_interval = dict_find(iter, KEY_UPDATE_INTERVAL);
	Tuple *error_url_remove = dict_find(iter, KEY_CONFIG_ERROR_URL_REMOVE);
	if (url1 || url2 || url3 || url4 || url5 || update_interval || error_url_remove) {
		if (url1) {
			ctx->callback_set_image_url(url1->value->cstring, 0);
		}
		if (url2) {
			ctx->callback_set_image_url(url2->value->cstring, 1);
		}
		if (url3) {
			ctx->callback_set_image_url(url3->value->cstring, 2);
		}
		if (url4) {
			ctx->callback_set_image_url(url4->value->cstring, 3);
		}
		if (url5) {
			ctx->callback_set_image_url(url5->value->cstring, 4);
		}
		if (update_interval && error_url_remove) {
			ctx->callback_set_update_interval(atoi(update_interval->value->cstring), (error_url_remove->value->int8 > 0));
		} else if (update_interval) {
			ctx->callback_set_update_interval(atoi(update_interval->value->cstring), true);
		}
		return;
	}

	Tuple *tuple = dict_read_first(iter);
	if (!tuple) {
		APP_LOG(APP_LOG_LEVEL_ERROR, "Got a message with no first key! Size of message: %li", (uint32_t)iter->end - (uint32_t)iter->dictionary);
		return;
	}
	switch (tuple->key) {
		case NETDL_DATA:
		if (ctx->index + tuple->length <= ctx->length) {
			memcpy(ctx->data + ctx->index, tuple->value->data, tuple->length);
			ctx->index += tuple->length;
		}
		else {
			APP_LOG(APP_LOG_LEVEL_WARNING, "Not overriding rx buffer. Bufsize=%li BufIndex=%li DataLen=%i",
					ctx->length, ctx->index, tuple->length);
		}
		break;
		case NETDL_BEGIN:
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Start transmission. Size=%lu", tuple->value->uint32);
		if (ctx->data != NULL) {
			free(ctx->data);
		}
		ctx->data = malloc(tuple->value->uint32);
		if (ctx->data != NULL) {
			ctx->length = tuple->value->uint32;
			ctx->index = 0;
		}
		else {
			APP_LOG(APP_LOG_LEVEL_WARNING, "Unable to allocate memory to receive image.");
			ctx->length = 0;
			ctx->index = 0;
		}
		break;
		case NETDL_END:
		if (ctx->data && ctx->length > 0 && ctx->index > 0) {
			NetDownload *image = malloc(sizeof(NetDownload));
			image->data = ctx->data;
			image->length = ctx->length;

			ctx->callback(image);

			// We have transfered ownership of this memory to the app. Make sure we dont free it.
			// (see netdownload_destroy for cleanup)
			ctx->data = NULL;
			ctx->index = ctx->length = 0;
		}
		else {
			APP_LOG(APP_LOG_LEVEL_DEBUG, "Got End message but we have no image...");
			ctx->callback_error();
		}
		break;
		case NETDL_READY:
		ctx->callback_ready();
		break;
		default:
		APP_LOG(APP_LOG_LEVEL_WARNING, "Unknown key in dict: %lu", tuple->key);
		break;
	}
}

char *translate_error(AppMessageResult result) {
	switch (result) {
		case APP_MSG_OK: return "APP_MSG_OK";
		case APP_MSG_SEND_TIMEOUT: return "APP_MSG_SEND_TIMEOUT";
		case APP_MSG_SEND_REJECTED: return "APP_MSG_SEND_REJECTED";
		case APP_MSG_NOT_CONNECTED: return "APP_MSG_NOT_CONNECTED";
		case APP_MSG_APP_NOT_RUNNING: return "APP_MSG_APP_NOT_RUNNING";
		case APP_MSG_INVALID_ARGS: return "APP_MSG_INVALID_ARGS";
		case APP_MSG_BUSY: return "APP_MSG_BUSY";
		case APP_MSG_BUFFER_OVERFLOW: return "APP_MSG_BUFFER_OVERFLOW";
		case APP_MSG_ALREADY_RELEASED: return "APP_MSG_ALREADY_RELEASED";
		case APP_MSG_CALLBACK_ALREADY_REGISTERED: return "APP_MSG_CALLBACK_ALREADY_REGISTERED";
		case APP_MSG_CALLBACK_NOT_REGISTERED: return "APP_MSG_CALLBACK_NOT_REGISTERED";
		case APP_MSG_OUT_OF_MEMORY: return "APP_MSG_OUT_OF_MEMORY";
		case APP_MSG_CLOSED: return "APP_MSG_CLOSED";
		case APP_MSG_INTERNAL_ERROR: return "APP_MSG_INTERNAL_ERROR";
		default: return "UNKNOWN ERROR";
	}
}

void netdownload_dropped(AppMessageResult reason, void *context) {
	APP_LOG(APP_LOG_LEVEL_ERROR, "Dropped message! Reason given: %s", translate_error(reason));
}

void netdownload_out_success(DictionaryIterator *iter, void *context) {
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Message sent.");
}

void netdownload_out_failed(DictionaryIterator *iter, AppMessageResult reason, void *context) {
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Failed to send message. Reason = %s", translate_error(reason));
}


