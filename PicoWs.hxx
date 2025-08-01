/*
 * PicoWs.hxx
 *
 * Copyright (C) 2025, Charles Chiou
 */

#ifndef PICOWS_HXX
#define PICOWS_HXX

#include <Bme280.hxx>
#include <PicoPlatform.hxx>
#include <PicoShell.hxx>

struct SniffIRState {
    uint32_t count;
    uint32_t max;
    uint32_t min;
};

class PicoWs : public PicoPlatform {

public:

    static shared_ptr<PicoWs> get(void);

    void startSniffIR(void);
    void stopSniffIR(void);
    void getSniffIRState(struct SniffIRState &state);

private:

    static void irxInterruptHandler(uint gpio, uint32_t events);

    uint32_t _irxPin;
    struct SniffIRState _irState;
    uint64_t _irxT0;

private:

    friend shared_ptr<PicoWs> make_shared<PicoWs>();

    PicoWs();
    ~PicoWs();

    shared_ptr<Bme280> _bme280;

};

class PicoWsShell : public PicoShell {

public:

    PicoWsShell(enum PicoShellDevice device);
    ~PicoWsShell();

protected:

    virtual int irsniff(int argc, char **argv);
    virtual int bme280(int argc, char **argv);
    virtual int unknown_command(int argc, char **argv);

};

#endif

/*
 * Local variables:
 * mode: C++
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
