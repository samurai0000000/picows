/*
 * picows.cxx
 *
 * Copyright (C) 2025, Charles Chiou
 */

#include <time.h>
#include <stdio.h>
#include <pico/stdlib.h>
#include <pico/bootrom.h>
#include <pico/cyw43_arch.h>
#include <hardware/watchdog.h>
#include <tusb.h>
#include <bsp/board_api.h>
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <string>
#include <pico-plat.h>
#include <PicoWs.hxx>
#include "version.h"

using namespace std;

#define WATCHDOG_TASK_STACK_SIZE      1024
#define WATCHDOG_TASK_PRIORITY        30
#define LED_TASK_STACK_SIZE           1024
#define LED_TASK_PRIORITY             25
#define USB_TASK_STACK_SIZE           2048
#define USB_TASK_PRIORITY             20
#define SHELL0_TASK_STACK_SIZE        4096
#define SHELL0_TASK_PRIORITY          10
#define SHELL1_TASK_STACK_SIZE        4096
#define SHELL1_TASK_PRIORITY          10

static string banner = "PICO-WorkStation";
static string version = string("Version: ") + string(MYPROJECT_VERSION_STRING);
static string built = string("Built: ") +
    string(MYPROJECT_WHOAMI) + string("@") +
    string(MYPROJECT_HOSTNAME) + string(" ") + string(MYPROJECT_DATE);
static string copyright = string("Copyright (C) 2025, Charles Chiou");

static void watchdog_task(__unused void *params)
{
    watchdog_enable(5000, true);
    watchdog_enable_caused_reboot();

    for (;;) {
        watchdog_update();
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

static void led_task(__unused void *params)
{
    for (;;) {
        PicoWs::get()->flipOnboardLed();
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

static void usb_task(__unused void *params)
{
    for (;;) {
        usbcdc_task();
        taskYIELD();
    }
}

static void shell0_task(__unused void *params)
{
    shared_ptr<PicoWsShell> shell0;

    shell0 = make_shared<PicoWsShell>(PICO_SHELL_USB_CDC);
    shell0->setBanner(banner);
    shell0->setVersion(version);
    shell0->setBuilt(built);
    shell0->setCopyright(copyright);

    vTaskDelay(pdMS_TO_TICKS(1500));
    shell0->showWelcome();

    for (;;) {
        int ret = 0;
        do {
            ret = shell0->process();
        } while (ret > 0);
        xSemaphoreTake(cdc_sem, pdMS_TO_TICKS(1000));
    }
}

static void shell1_task(__unused void *params)
{
    shared_ptr<PicoWsShell> shell1;

    shell1 = make_shared<PicoWsShell>(PICO_SHELL_SERIAL0);
    shell1->setBanner(banner);
    shell1->setVersion(version);
    shell1->setBuilt(built);
    shell1->setCopyright(copyright);

    shell1->showWelcome();

    for (;;) {
        int ret = 0;
        do {
            ret = shell1->process();
        } while (ret > 0);
        xSemaphoreTake(uart0_sem, pdMS_TO_TICKS(1000));
    }
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    (void)(xTask);
    (void)(pcTaskName);

    for (;;);
}

void vApplicationIdleHook(void)
{

}

int main(void)
{
    TaskHandle_t watchdogTask;
    TaskHandle_t ledTask;
    TaskHandle_t usbTask;
    TaskHandle_t shell0Task;
    TaskHandle_t shell1Task;

    board_init();
    tusb_init();
    stdio_init_all();
    if (board_init_after_tusb) {
        board_init_after_tusb();
    }
    usbcdc_init();
    serial_init();
    cyw43_arch_init();

    xTaskCreate(watchdog_task,
                "Watchdog",
                WATCHDOG_TASK_STACK_SIZE,
                NULL,
                WATCHDOG_TASK_PRIORITY,
                &watchdogTask);

    xTaskCreate(led_task,
                "Led",
                LED_TASK_STACK_SIZE,
                NULL,
                LED_TASK_PRIORITY,
                &ledTask);

    xTaskCreate(usb_task,
                "USB",
                USB_TASK_STACK_SIZE,
                NULL,
                USB_TASK_PRIORITY,
                &usbTask);

    xTaskCreate(shell0_task,
                "Shell0",
                SHELL0_TASK_STACK_SIZE,
                NULL,
                SHELL0_TASK_PRIORITY,
                &shell0Task);

    xTaskCreate(shell1_task,
                "Shell1",
                SHELL1_TASK_STACK_SIZE,
                NULL,
                SHELL1_TASK_PRIORITY,
                &shell1Task);

#if defined(configUSE_CORE_AFFINITY) && (configNUMBER_OF_CORES > 1)
    vTaskCoreAffinitySet(watchdogTask, 0x1);
    vTaskCoreAffinitySet(ledTask, 0x1);
    vTaskCoreAffinitySet(usbTask, 0x1);
    vTaskCoreAffinitySet(shell0Task, 0x2);
    vTaskCoreAffinitySet(shell1Task, 0x2);
#endif

    vTaskStartScheduler();

    return 0;
}

/*
 * Local variables:
 * mode: C++
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
