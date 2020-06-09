/* 
 *
 * Software for wifi controlled fan
 * 
 *
*/

#include <string.h>
#include <sys/param.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "protocol_examples_common.h"
#include "nvs.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

#include "driver/gpio.h"

#define LOW 1
#define HIGH 0

#define PORT CONFIG_EXAMPLE_PORT
#define GPIO_OUTPUT_IO_power    16
#define GPIO_OUTPUT_IO_speed    15
#define GPIO_OUTPUT_IO_rot      14
#define GPIO_OUTPUT_IO_windMode 13
#define GPIO_OUTPUT_IO_timer    12

#define GPIO_OUTPUT_PIN_SEL  ((1ULL<<GPIO_OUTPUT_IO_power) | (1ULL<<GPIO_OUTPUT_IO_speed) | ((1ULL<<GPIO_OUTPUT_IO_rot) | (1ULL<<GPIO_OUTPUT_IO_windMode)) | (1ULL<<GPIO_OUTPUT_IO_timer))

static const char *TAG = "example";

void gpio_Init() {
    gpio_config_t io_conf;
    //disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set,e.g.GPIO15/16
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    //disable pull-down mode
    io_conf.pull_down_en = 0;
    //disable pull-up mode
    io_conf.pull_up_en = 0;
    //configure GPIO with the given settings
    gpio_config(&io_conf);

    gpio_set_level(GPIO_OUTPUT_IO_power, LOW);
    gpio_set_level(GPIO_OUTPUT_IO_speed, LOW);
    gpio_set_level(GPIO_OUTPUT_IO_rot, LOW);
    gpio_set_level(GPIO_OUTPUT_IO_windMode, LOW);
    gpio_set_level(GPIO_OUTPUT_IO_timer, LOW);

    ESP_LOGI(TAG, "GPIO Initilized");
}

static void udp_server_task(void *pvParameters){
    char rx_buffer[128];
    char addr_str[128];
    int addr_family;
    int ip_protocol;

    while (1) {
        struct sockaddr_in destAddr;
        destAddr.sin_addr.s_addr = htonl(INADDR_ANY);
        destAddr.sin_family = AF_INET;
        destAddr.sin_port = htons(PORT);
        addr_family = AF_INET;
        ip_protocol = IPPROTO_IP;
        inet_ntoa_r(destAddr.sin_addr, addr_str, sizeof(addr_str) - 1);

        int sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
        if (sock < 0) {
            ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
            break;
        }
        ESP_LOGI(TAG, "Socket created");

        int err = bind(sock, (struct sockaddr *)&destAddr, sizeof(destAddr));
        if (err < 0) {
            ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
        }
        ESP_LOGI(TAG, "Socket binded");

        while (1) {
            ESP_LOGI(TAG, "Waiting for data");
            struct sockaddr_in sourceAddr;

            socklen_t socklen = sizeof(sourceAddr);
            int len = recvfrom(sock, rx_buffer, sizeof(rx_buffer) - 1, 0, (struct sockaddr *)&sourceAddr, &socklen);

            // Error occured during receiving
            if (len < 0) {
                ESP_LOGE(TAG, "recvfrom failed: errno %d", errno);
                break;
            }
            // Data received
            else {
                // Get the sender's ip address as string
                inet_ntoa_r(((struct sockaddr_in *)&sourceAddr)->sin_addr.s_addr, addr_str, sizeof(addr_str) - 1);
                
                rx_buffer[len] = 0; // Null-terminate whatever we received and treat like a string...
                ESP_LOGI(TAG, "Received %d bytes from %s:", len, addr_str);
                ESP_LOGI(TAG, "%s", rx_buffer);

                // Determine what button was pressed. TODO: may need to investigate output logic
                if(strcmp(rx_buffer, "power") == 0) {
                    gpio_set_level(GPIO_OUTPUT_IO_power, HIGH);
                    vTaskDelay(1000 / portTICK_RATE_MS);
                    gpio_set_level(GPIO_OUTPUT_IO_power, LOW);
                    ESP_LOGI(TAG, "power button pressed");
                } else if(strcmp(rx_buffer, "speed") == 0) {
                    gpio_set_level(GPIO_OUTPUT_IO_speed, HIGH);
                    vTaskDelay(1000 / portTICK_RATE_MS);
                    gpio_set_level(GPIO_OUTPUT_IO_speed, LOW);
                    ESP_LOGI(TAG, "speed button pressed");
                } else if(strcmp(rx_buffer, "rot") == 0) {
                    gpio_set_level(GPIO_OUTPUT_IO_rot, HIGH);
                    vTaskDelay(1000 / portTICK_RATE_MS);
                    gpio_set_level(GPIO_OUTPUT_IO_rot, LOW);
                    ESP_LOGI(TAG, "rot button pressed");
                } else if(strcmp(rx_buffer, "wind") == 0) {
                    gpio_set_level(GPIO_OUTPUT_IO_windMode, HIGH);
                    vTaskDelay(1000 / portTICK_RATE_MS);
                    gpio_set_level(GPIO_OUTPUT_IO_windMode, LOW);
                    ESP_LOGI(TAG, "wind button pressed");
                } else if(strcmp(rx_buffer, "timer") == 0) {
                    gpio_set_level(GPIO_OUTPUT_IO_timer, HIGH);
                    vTaskDelay(1000 / portTICK_RATE_MS);
                    gpio_set_level(GPIO_OUTPUT_IO_timer, LOW);
                    ESP_LOGI(TAG, "timer button pressed");
                } else {
                    ESP_LOGI(TAG, "Invalid button entry");
                }

                int err = sendto(sock, rx_buffer, len, 0, (struct sockaddr *)&sourceAddr, sizeof(sourceAddr));
                if (err < 0) {
                    ESP_LOGE(TAG, "Error occured during sending: errno %d", errno);
                    break;
                }
            }
        }

        if (sock != -1) {
            ESP_LOGE(TAG, "Shutting down socket and restarting...");
            shutdown(sock, 0);
            close(sock);
        }
    }

    vTaskDelete(NULL);
}

void app_main() {
    gpio_Init();

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    ESP_ERROR_CHECK(example_connect());

    xTaskCreate(udp_server_task, "udp_server", 4096, NULL, 5, NULL);
}
