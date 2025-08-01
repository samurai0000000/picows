/*
 * PicoWs.cxx
 *
 * Copyright (C) 2025, Charles Chiou
 */

#include <cstring>
#include <FreeRTOS.h>
#include <task.h>
#include <time.h>
#include <pico/stdlib.h>
#include <hardware/gpio.h>
#include <pico-plat.h>
#include <PicoWs.hxx>

shared_ptr<PicoWs> PicoWs::get(void)
{
    if (PicoPlatform::pp == NULL) {
        shared_ptr<PicoWs> ws =
            shared_ptr<PicoWs>(new PicoWs(), [](PicoWs *p) {
                delete p;
            });
        PicoPlatform::pp = static_pointer_cast<PicoPlatform>(ws);
    }

    return static_pointer_cast<PicoWs>(PicoPlatform::pp);
}

PicoWs::PicoWs()
{
    _irxPin = 17;
    stopSniffIR();
}

PicoWs::~PicoWs()
{

}

void PicoWs::startSniffIR(void)
{
    _irxT0 = 0;
    _irState.count = 0;
    _irState.min = UINT32_MAX;
    _irState.max = 0;

    gpio_init(_irxPin);
    gpio_set_dir(_irxPin, GPIO_IN);
    gpio_pull_up(_irxPin);
    gpio_set_irq_enabled_with_callback(
        _irxPin,
        GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL,
        true,
        PicoWs::irxInterruptHandler);
}

void PicoWs::stopSniffIR(void)
{
    gpio_set_irq_enabled(
        _irxPin,
        GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL,
        false);
    bzero(&_irState, sizeof(struct SniffIRState));
}

void PicoWs::getSniffIRState(struct SniffIRState &state)
{
    memcpy(&state, &_irState, sizeof(struct SniffIRState));
    if (state.min == UINT32_MAX) {
        state.min = 0;
    }
}

void PicoWs::irxInterruptHandler(uint gpio, uint32_t events)
{
    shared_ptr<PicoWs> ws = PicoWs::get();
    uint64_t ts = time_us_64();

    (void)(gpio);

    if (events & GPIO_IRQ_EDGE_FALL) {
        ws->_irxT0 = ts;
    } else if (events & GPIO_IRQ_EDGE_RISE) {
        if (ws->_irxT0 != 0) {
            ws->_irxT0 = ts - ws->_irxT0;
            if (ws->_irxT0 < ws->_irState.min) {
                ws->_irState.min = (uint32_t) ws->_irxT0;
            }
            if (ws->_irxT0 > ws->_irState.max) {
                ws->_irState.max = (uint32_t) ws->_irxT0;
            }
            ws->_irxT0 = 0;
        }
    }

    ws->_irState.count++;
}

/* ------------------------------------------------------------------------ */

PicoWsShell::PicoWsShell(enum PicoShellDevice device)
    : PicoShell(device)
{
    _help_list.push_back("irsniff");
    _help_list.push_back("bme280");
}

PicoWsShell::~PicoWsShell()
{

}


int PicoWsShell::irsniff(int argc, char **argv)
{

    (void)(argc);
    (void)(argv);

    PicoWs::get()->startSniffIR();

    this->printf("IRX Sniffer\n\n");
    for (;;) {
        struct SniffIRState state;

        PicoWs::get()->getSniffIRState(state);
        this->printf("\033[A");
        this->printf("count=%u min=%lu, max=%lu\033[K\n",
                     state.count, state.min, state.max);
        if (catch_ctr_c(false)) {
            break;
        } else {
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }

    PicoWs::get()->stopSniffIR();

    return 0;
}

int PicoWsShell::bme280(int argc, char **argv)
{
    (void)(argc);
    (void)(argv);

    return 0;
}

int PicoWsShell::unknown_command(int argc, char **argv)
{
    int ret = 0;

    if (strcmp(argv[0], "irsniff") == 0) {
        ret = this->irsniff(argc, argv);
    } else if (strcmp(argv[0], "bme280") == 0) {
    } else {
        this->printf("Unknown command '%s'!\n", argv[0]);
        ret = -1;
    }

    return ret;
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
