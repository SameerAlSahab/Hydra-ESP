#pragma once

#ifdef __cplusplus
extern "C" {
    #endif

    void bt_payload_attack_init(void);
    void bt_payload_attack_start(int payload);
    void bt_payload_attack_stop(void);
    void bt_payload_attack_set_payload(int payload);
    void bt_payload_attack_run_now(void);

    // New status getters for the Web UI
    bool bt_payload_is_busy(void);
    bool bt_payload_is_connected(void);

    const char* bt_payload_get_connected_name(void);
    const char* bt_payload_get_connected_mac(void);

    #ifdef __cplusplus
}
#endif
