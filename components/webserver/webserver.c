/**
 * @file webserver.c
 * Serves HTML/CSS/JS/Icons/Fonts directly from SPIFFS filesystem.
 */

#include "webserver.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
#include "esp_log.h"
#include "esp_err.h"
#include "esp_event.h"
#include "esp_http_server.h"
#include "esp_spiffs.h"
#include "nvs_flash.h"
#include "nvs.h"

#include "bt_payload_attack.h"
#include "wifi_controller.h"
#include "attack.h"
#include "pcap_serializer.h"
#include "hccapx_serializer.h"
#include "attack_eviltwin.h"

static const char *TAG = "webserver";
ESP_EVENT_DEFINE_BASE(WEBSERVER_EVENTS);

static httpd_handle_t server = NULL;
static bool spiffs_mounted = false;

static void url_decode(char *dst, const char *src);


static esp_err_t serve_file(httpd_req_t *req, const char *filepath) {
    struct stat st;
    if (stat(filepath, &st) == -1) {
        ESP_LOGE(TAG, "File not found: %s", filepath);
        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "File not found");
        return ESP_FAIL;
    }

    FILE *f = fopen(filepath, "r");
    if (f == NULL) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to open file");
        return ESP_FAIL;
    }


    if (strstr(filepath, ".html")) httpd_resp_set_type(req, "text/html");
    else if (strstr(filepath, ".css")) httpd_resp_set_type(req, "text/css");
    else if (strstr(filepath, ".js")) httpd_resp_set_type(req, "application/javascript");
    else if (strstr(filepath, ".png")) httpd_resp_set_type(req, "image/png");
    else if (strstr(filepath, ".jpg") || strstr(filepath, ".jpeg")) httpd_resp_set_type(req, "image/jpeg");
    else if (strstr(filepath, ".ttf")) httpd_resp_set_type(req, "font/ttf");
    else if (strstr(filepath, ".pcap")) httpd_resp_set_type(req, "application/octet-stream");
    else httpd_resp_set_type(req, "text/plain");

    char buf[1024];
    size_t bytes_read;
    while ((bytes_read = fread(buf, 1, sizeof(buf), f)) > 0) {
        if (httpd_resp_send_chunk(req, buf, bytes_read) != ESP_OK) {
            fclose(f);
            return ESP_FAIL;
        }
    }

    fclose(f);
    return httpd_resp_send_chunk(req, NULL, 0);
}



static esp_err_t common_get_handler(httpd_req_t *req) {

    char filepath[256];
    const char* prefix = "/spiffs";


    if (strlen(req->uri) >= (sizeof(filepath) - strlen(prefix) - 1)) {
        ESP_LOGE(TAG, "URI too long: %s", req->uri);
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "URI too long");
        return ESP_FAIL;
    }

    strcpy(filepath, prefix);
    strcat(filepath, req->uri);

    return serve_file(req, filepath);
}

static esp_err_t uri_root_get_handler(httpd_req_t *req) {
    return serve_file(req, "/spiffs/index.html");
}

static esp_err_t uri_ap_list_get_handler(httpd_req_t *req) {
    wifictl_scan_nearby_aps();
    const wifictl_ap_records_t *ap_records = wifictl_get_ap_records();
    char resp_chunk[40];
    httpd_resp_set_type(req, HTTPD_TYPE_OCTET);
    for (unsigned i = 0; i < ap_records->count; i++) {
        memcpy(resp_chunk, ap_records->records[i].ssid, 33);
        memcpy(&resp_chunk[33], ap_records->records[i].bssid, 6);
        memcpy(&resp_chunk[39], &ap_records->records[i].rssi, 1);
        httpd_resp_send_chunk(req, resp_chunk, 40);
    }
    return httpd_resp_send_chunk(req, NULL, 0);
}

static esp_err_t uri_run_attack_post_handler(httpd_req_t *req) {
    attack_request_t attack_request;
    int received = httpd_req_recv(req, (char *)&attack_request, sizeof(attack_request_t));
    if (received != sizeof(attack_request_t)) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Bad payload");
        return ESP_FAIL;
    }
    esp_event_post(WEBSERVER_EVENTS, WEBSERVER_EVENT_ATTACK_REQUEST, &attack_request, sizeof(attack_request_t), portMAX_DELAY);
    return httpd_resp_send(req, NULL, 0);
}

static esp_err_t uri_status_get_handler(httpd_req_t *req) {
    const attack_status_t *attack_status = attack_get_status();
    httpd_resp_set_type(req, HTTPD_TYPE_OCTET);
    httpd_resp_send_chunk(req, (char *)attack_status, 4);
    if ((attack_status->state == FINISHED || attack_status->state == TIMEOUT) && attack_status->content_size > 0) {
        httpd_resp_send_chunk(req, attack_status->content, attack_status->content_size);
    }
    return httpd_resp_send_chunk(req, NULL, 0);
}

static esp_err_t uri_capture_hccapx_get_handler(httpd_req_t *req) {
    httpd_resp_set_type(req, "application/octet-stream");
    httpd_resp_set_hdr(req, "Content-Disposition", "attachment; filename=hydra_capture.hccapx");
    return httpd_resp_send(req, (char *)hccapx_serializer_get(), sizeof(hccapx_t));
}

static esp_err_t uri_bt_status_handler(httpd_req_t *req) {
    char json[256];
    snprintf(json, sizeof(json), "{\"connected\":%s,\"busy\":%s,\"name\":\"%s\",\"mac\":\"%s\"}",
             bt_payload_is_connected() ? "true" : "false", bt_payload_is_busy() ? "true" : "false",
             bt_payload_get_connected_name(), bt_payload_get_connected_mac());
    httpd_resp_set_type(req, "application/json");
    return httpd_resp_send(req, json, strlen(json));
}

static esp_err_t save_settings_post_handler(httpd_req_t *req) {
    char buf[128];
    int ret = httpd_req_recv(req, buf, sizeof(buf) - 1);
    if (ret <= 0) return ESP_FAIL;
    buf[ret] = '\0';

    char raw_ssid[64] = {0}, raw_pass[96] = {0};
    char ssid[33] = {0}, pass[65] = {0};

    if (httpd_query_key_value(buf, "ssid", raw_ssid, sizeof(raw_ssid)) == ESP_OK &&
        httpd_query_key_value(buf, "pass", raw_pass, sizeof(raw_pass)) == ESP_OK) {
        url_decode(ssid, raw_ssid);
    url_decode(pass, raw_pass);

    nvs_handle_t nvs_h;
    if (nvs_open("storage", NVS_READWRITE, &nvs_h) == ESP_OK) {
        nvs_set_str(nvs_h, "ap_ssid", ssid);
        nvs_set_str(nvs_h, "ap_pass", pass);
        nvs_commit(nvs_h);
        nvs_close(nvs_h);
    }
    httpd_resp_sendstr(req, "Settings Saved! Rebooting...");
    vTaskDelay(pdMS_TO_TICKS(1000));
    esp_restart();
    return ESP_OK;
        }
        return httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid data");
}


static httpd_uri_t uri_root = { .uri = "/", .method = HTTP_GET, .handler = uri_root_get_handler };
static httpd_uri_t uri_ap_list = { .uri = "/ap-list", .method = HTTP_GET, .handler = uri_ap_list_get_handler };
static httpd_uri_t uri_run_attack = { .uri = "/run-attack", .method = HTTP_POST, .handler = uri_run_attack_post_handler };
static httpd_uri_t uri_status = { .uri = "/status", .method = HTTP_GET, .handler = uri_status_get_handler };
static httpd_uri_t uri_hccapx = { .uri = "/capture.hccapx", .method = HTTP_GET, .handler = uri_capture_hccapx_get_handler };
static httpd_uri_t uri_bt_status_get = { .uri = "/bt-status", .method = HTTP_GET, .handler = uri_bt_status_handler };
static httpd_uri_t uri_save_settings = { .uri = "/save_settings", .method = HTTP_POST, .handler = save_settings_post_handler };

static httpd_uri_t uri_icons = { .uri = "/icons/*", .method = HTTP_GET, .handler = common_get_handler };
static httpd_uri_t uri_fonts = { .uri = "/fonts/*", .method = HTTP_GET, .handler = common_get_handler };
static httpd_uri_t uri_dtwin = { .uri = "/devil_twin/*", .method = HTTP_GET, .handler = common_get_handler };
static httpd_uri_t uri_style = { .uri = "/style.css", .method = HTTP_GET, .handler = common_get_handler };
static httpd_uri_t uri_js = { .uri = "/app.js", .method = HTTP_GET, .handler = common_get_handler };


static void init_spiffs(void) {
    if (spiffs_mounted) return;
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs", .partition_label = "storage",
        .max_files = 10, .format_if_mount_failed = false
    };
    if (esp_vfs_spiffs_register(&conf) == ESP_OK) spiffs_mounted = true;
}

void webserver_stop(void) {
    if (server == NULL) {
        ESP_LOGW(TAG, "Webserver not running.");
        return;
    }
    if (httpd_stop(server) == ESP_OK) {
        server = NULL;
        ESP_LOGI(TAG, "Webserver stopped.");
    } else {
        ESP_LOGE(TAG, "Failed to stop webserver!");
    }
}


void webserver_run(void) {
    if (server != NULL) return;
    init_spiffs();
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.max_uri_handlers = 30;
    config.uri_match_fn = httpd_uri_match_wildcard;

    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_register_uri_handler(server, &uri_root);
        httpd_register_uri_handler(server, &uri_ap_list);
        httpd_register_uri_handler(server, &uri_run_attack);
        httpd_register_uri_handler(server, &uri_status);
        httpd_register_uri_handler(server, &uri_hccapx);
        httpd_register_uri_handler(server, &uri_bt_status_get);
        httpd_register_uri_handler(server, &uri_save_settings);
        httpd_register_uri_handler(server, &uri_icons);
        httpd_register_uri_handler(server, &uri_fonts);
        httpd_register_uri_handler(server, &uri_dtwin);
        httpd_register_uri_handler(server, &uri_style);
        httpd_register_uri_handler(server, &uri_js);
        ESP_LOGI(TAG, "Webserver started.");
    }
}


static void url_decode(char *dst, const char *src) {
    char a, b;
    while (*src) {
        if ((*src == '%') && ((a = src[1]) && (b = src[2])) && isxdigit((unsigned char)a) && isxdigit((unsigned char)b)) {
            if (a >= 'a') { a -= 32; }
            a = (a >= 'A') ? a - 'A' + 10 : a - '0';

            if (b >= 'a') { b -= 32; }
            b = (b >= 'A') ? b - 'A' + 10 : b - '0';

            *dst++ = 16 * a + b;
            src += 3;
        } else if (*src == '+') {
            *dst++ = ' ';
            src++;
        } else {
            *dst++ = *src++;
        }
    }
    *dst = '\0';
}
