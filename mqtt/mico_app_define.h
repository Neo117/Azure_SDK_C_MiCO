/**
 * @file mqtt_config.h
 * @brief MQTT specific configuration file
 */

#ifndef MICO_APP_DEFINE_H_
#define MICO_APP_DEFINE_H_

#define MQTT_USE_SSL

#ifdef MQTT_USE_SSL
#define MQTT_HOST                   "6618fdda2a4f11e7a554fa163e876164.mqtt.iot.gz.baidubce.com" ///< Customer specific MQTT HOST. The same will be used for Thing Shadow
#define MQTT_PORT                   1884 ///< default port for MQTT/S
#define MQTT_ROOT_CA_FILENAME       NULL ///< Root CA file name
#define MQTT_CERTIFICATE_FILENAME   NULL ///< device signed certificate file name
#define MQTT_PRIVATE_KEY_FILENAME   NULL ///< Device private key filename
#define MQTT_USERNAME               "6618fdda2a4f11e7a554fa163e876164/77a0853e3a1a11e7a554fa163e876164"
#define MQTT_PASSWORD               "ibV/zzpOyHKDUVH4EEXK7RoZtJHp6GTj6fazxst2+k4="
#else
#define MQTT_HOST                   "6618fdda2a4f11e7a554fa163e876164.mqtt.iot.gz.baidubce.com" ///< Customer specific MQTT HOST. The same will be used for Thing Shadow
#define MQTT_PORT                   1883 ///< default port for MQTT/S
#define MQTT_USERNAME               "6618fdda2a4f11e7a554fa163e876164/77a0853e3a1a11e7a554fa163e876164"
#define MQTT_PASSWORD               "ibV/zzpOyHKDUVH4EEXK7RoZtJHp6GTj6fazxst2+k4="
#endif

#define MQTT_SUB_NAME "6618fdda2a4f11e7a554fa163e876164/df358c1a348611e7a554fa163e876164/77a0853e3a1a11e7a554fa163e876164/status/json"

#define MQTT_CLIENT_ID "77a0853e3a1a11e7a554fa163e876164"

#endif /* SRC_SHADOW_IOT_SHADOW_CONFIG_H_ */
