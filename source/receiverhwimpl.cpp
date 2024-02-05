#include "receiver/receiverhwimpl.h"

#include "base/complex.h"
#include "dsp/fft.h"
#include "receiver/receiverfactory.h"
#include "rtl-sdr.h"

#include <iostream>
#include <thread>
using namespace Base;

struct ReceiverHWImpl::Pimpl {
    void set(const ReceiverSettings& settings);
    void open();
    int n_read{0};
    int dev_index{0};
    int dev_given{0};
    uint32_t samCount;
    rtlsdr_dev_t* dev{nullptr};
    int do_exit{0};

    int setCenterFreq(uint32_t freq);
    int setSampleRate(uint32_t samp_rate);
    int setAutoGain();
    int nearestGain(int gain);
    int setGain(int gain);
    int setDirectSampling(int on);
    int setPpm(int ppm_error);
    void setAgcMode(int on);
    void setOffsetTuningOn();
    int resetBuffer();

    static uint32_t roundPowerTwo(uint32_t& size);
};

ReceiverHWImpl::ReceiverHWImpl(SettingTransaction settingTransaction, uint32_t numberDev) :
    IReceiver(settingTransaction), m_d(std::make_unique<Pimpl>()) {
    m_d->dev_index = numberDev;
    m_d->open();
}

ReceiverHWImpl::~ReceiverHWImpl() {
    m_d->resetBuffer();
    auto r = rtlsdr_close(m_d->dev);
    if(r < 0) {
        std::cerr << "FAIL close: " << r;
    } else {
        std::cerr << "close device " << m_d->dev_index << "\n";
    }
}

/**
 * @brief Метод для открытия свистка. Сначала вызывается метод
 * deviceSearch(), чтобы определить что это за свисток.
 */
void ReceiverHWImpl::Pimpl::open() {
    int r = rtlsdr_open(&dev, (uint32_t)dev_index);
    if(r < 0) {
        std::cerr << "Failed to open rtlsdr device" << dev_index << "\n";
        exit(1);
    }
}

/**
 * @brief Метод для установки настроек приемника.
 * Данному методу на вход подается структура с настройками.
 * Здесь можно установить:
 * - центральную частоту
 * - частоту дискретизации
 * - усиление
 * - откорректровать центральную частоту
 * Также возможно включить режим автоматического контроля
 * усиления.
 */

void ReceiverHWImpl::setSettings(BaseSettings* sett) {
    ReceiverSettings* realset = dynamic_cast<ReceiverSettings*>(sett);

    m_d->set(*realset);
    m_d->samCount = realset->sampleCount;
}

void ReceiverHWImpl::Pimpl::set(const ReceiverSettings& settings) {
    if(n_read == 0) {
        n_read = settings.n_read;

        if(settings.direct_sampling)
            setDirectSampling(settings.direct_sampling);

        setSampleRate(settings.rfSettings.sampleFreq);

        setCenterFreq(settings.rfSettings.centralFreq);

        if(0 == settings.rfSettings.gain) {
            setAutoGain();
        } else {
            auto gain = nearestGain(settings.rfSettings.gain);
            setGain(gain);
        }

        setPpm(settings.rfSettings.ppm_error);

        setAgcMode(settings.rfSettings.agcMode);
    }
}

/**
 * @brief Метод для получения отсчетов I и Q. На вход методу подаются настройки приемника
 * и буфер, в который будут считанны I и Q.
 * Считывание идет следующим образом сначала идет I потом Q и так далее. I и Q представлют
 * собой uint8_t.
 * Минимальная величина буфера может быть 512. Максимальная 256 * 16384. В rtl-sdr.h
 * написано, что длина буфера должна быть кратна 16384 (URB size). Беря во внимание, что
 * минимальная длина буфера 512, то можно сказать, что длина буфера должна быть кратна 512.
 * ( С этим нужно разобраться подробнее)
 * Также важно заметить, что буфером является вектор, сотсоящий из элементов класса Complex.
 * Он содержит re и im. I считывается в re, а Q считывается в im. Из это понятно, что число
 * считанных байт будет в два раза больше, чем число считанных отсчетов. (bytesToRead = 2 * sampleCount)
 */

bool ReceiverHWImpl::getComplex(const BaseSettings* settings, ReceiverHWImpl::Buffer& out) {
    const ReceiverSettings* realset = dynamic_cast<const ReceiverSettings*>(settings);
    m_d->set(*realset);
    auto size = realset->sampleCount; //
    size      = m_d->roundPowerTwo(size);
    out.resize(size);
    auto bytesToRead = 2 * realset->sampleCount; //
    bytesToRead      = m_d->roundPowerTwo(bytesToRead);
    m_d->resetBuffer();
    auto result = rtlsdr_read_sync(m_d->dev, out.data(), bytesToRead, &(m_d->n_read));

    if(result < 0)
        std::cerr << "WARNING: sync read failed." << std::endl;

    return result;
}

bool ReceiverHWImpl::getComplex(Buffer& out) {
    auto size = m_d->samCount; //
    size      = m_d->roundPowerTwo(size);
    out.resize(size);
    auto bytesToRead = 2 * m_d->samCount; //
    bytesToRead      = m_d->roundPowerTwo(bytesToRead);
    m_d->resetBuffer();
    auto result = rtlsdr_read_sync(m_d->dev, out.data(), bytesToRead, &(m_d->n_read));

    if(result < 0)
        std::cerr << "WARNING: sync read failed." << std::endl;

    return result;
}
bool ReceiverHWImpl::getComplex(Complex<int8_t>* complexBuff, uint32_t sizeOfBuff) {
    m_d->resetBuffer();

    auto result = rtlsdr_read_sync(m_d->dev, reinterpret_cast<char*>(complexBuff), 2 * sizeOfBuff, &(m_d->n_read));

    for(uint32_t i = 0; i < sizeOfBuff; ++i) {
        complexBuff[i] = {static_cast<int8_t>(static_cast<int16_t>(complexBuff[i].re)),
                          static_cast<int8_t>(static_cast<int16_t>(complexBuff[i].im))};
    }

    if(result < 0)
        std::cerr << "WARNING: sync read failed." << std::endl;

    return result;
}
void ReceiverHWImpl::getSpectrum(SpectBuff& out) {
    IReceiver::Buffer signal;
    auto size = m_d->samCount;
    size      = m_d->roundPowerTwo(size);
    signal.resize(size);
    getComplex(signal);

    std::vector<Complex<float> > signalFloat;
    signalFloat.resize(size);
    for(size_t i = 0; i < signalFloat.size(); i++) {
        signalFloat[i].re = (static_cast<float>(signal[i].re) - 127.5f);
        signalFloat[i].im = (static_cast<float>(signal[i].im) - 127.5f);
    }

    IReceiver::SpectBuff signalDouble(signalFloat.begin(), signalFloat.end());

    for(size_t i = 0; i < signalDouble.size(); i++) {
        if(i % 2 == 0)
            signalDouble[i] = -signalDouble[i];
    }
    out.resize(size);

    fft(signalDouble, out, signalDouble.size());
}

/**
 * @brief Метод для получения результата FFT. На вход методу подаются настройки приемника
 * и буфер, в который будут считанны значения, получившегося спектра.
 * В нем мы используем метод getComplex() для получения I и Q, чтобы в дальнейшем вычислить FFT.
 */

void ReceiverHWImpl::getSpectrum(const BaseSettings* settings, SpectBuff& out) {
    const ReceiverSettings* realset = dynamic_cast<const ReceiverSettings*>(settings);
    m_d->set(*realset);
    IReceiver::Buffer signal;
    auto size = realset->sampleCount;
    size      = m_d->roundPowerTwo(size);
    signal.resize(size);
    getComplex(settings, signal);

    std::vector<Complex<float> > signalFloat;
    signalFloat.resize(size);
    for(size_t i = 0; i < signalFloat.size(); i++) {
        signalFloat[i].re = (static_cast<float>(signal[i].re) - 127.5f);
        signalFloat[i].im = (static_cast<float>(signal[i].im) - 127.5f);
    }

    IReceiver::SpectBuff signalDouble(signalFloat.begin(), signalFloat.end());

    for(size_t i = 0; i < signalDouble.size(); i++) {
        if(i % 2 == 0)
            signalDouble[i] = -signalDouble[i];
    }
    out.resize(size);

    fft(signalDouble, out, signalDouble.size());
}

/**
 * @brief Метод для установки центральной частоты.
 */
int ReceiverHWImpl::Pimpl::setCenterFreq(uint32_t freq) {
    auto r = rtlsdr_set_center_freq(dev, freq);
    if(r < 0)
        std::cerr << "WARNING: Failed to set center freq.\n";
    else
        std::cerr << "Tuned to " << freq << " Hz.\n";
    return r;
}

/**
 * @brief Метод для установки частоты дискретизации приемника.
 */
int ReceiverHWImpl::Pimpl::setSampleRate(uint32_t samp_rate) {
    auto r = rtlsdr_set_sample_rate(dev, samp_rate);
    if(r < 0)
        std::cerr << "WARNING: Failed to set sample rate.\n";
    else
        std::cerr << "Sampling at " << samp_rate << "\n";
    return r;
}

/**
 * @brief Метод, который устанавливает автоматическое усисление
 * приемника.
 */
int ReceiverHWImpl::Pimpl::setAutoGain() {
    auto r = rtlsdr_set_tuner_gain_mode(dev, 0);
    if(r != 0)
        std::cerr << "WARNING: Failed to set tuner gain.\n";
    else
        std::cerr << "Tuner gain set to automatic.\n";
    return r;
}

/**
 * @brief Метод для поиска ближайшего усиления, которое
 * возможно установить для данного приемника.
 */

int ReceiverHWImpl::Pimpl::nearestGain(int gain) {
    std::vector<int> gains;

    auto r = rtlsdr_set_tuner_gain_mode(dev, 1);
    if(r < 0) {
        std::cerr << "WARNING: Failed to enable manual gain.\n";
        return r;
    }

    auto count = rtlsdr_get_tuner_gains(dev, nullptr);
    if(count <= 0)
        return 0;

    gains.resize(count);

    count = rtlsdr_get_tuner_gains(dev, gains.data());

    auto nearest = gains[0];
    for(auto i = 0; i < count; i++) {
        auto err1 = abs(gain - nearest);
        auto err2 = abs(gain - gains[i]);
        if(err2 < err1)
            nearest = gains[i];
    }

    return nearest;
}

/**
 * @brief Метод для установки усиления.
 */
int ReceiverHWImpl::Pimpl::setGain(int gain) {
    int r;
    r = rtlsdr_set_tuner_gain_mode(dev, 1);
    if(r < 0) {
        std::cerr << "WARNING: Failed to enable manual gain.\n";
        return r;
    }

    r = rtlsdr_set_tuner_gain(dev, gain);
    if(r != 0)
        std::cerr << "WARNING: Failed to set tuner gain.\n";
    else
        std::cerr << "Tuner gain set to" << gain / 10.0 << "dB.\n";
    return r;
}

/**
 * @brief Метод для установки режима Direct Sampling
 * 0 - Откючение режима прямой выборки
 * 1 - Включение режима прямой выборки для вывода I
 * 2 - Включение режима прямой выборки для вывода Q
 * 3 - Включен режим прямой выборки
 */
int ReceiverHWImpl::Pimpl::setDirectSampling(int on) {
    int r;
    r = rtlsdr_set_direct_sampling(dev, on);
    if(r != 0) {
        std::cerr << "WARNING: Failed to set direct sampling mode.\n";
        return r;
    }

    switch(on) {
        case(0):
            std::cerr << "Direct sampling mode disabled.\n";
            break;
        case(1):
            std::cerr << "Enabled direct sampling mode, input 1/I.\n";
            break;
        case(2):
            std::cerr << "Enabled direct sampling mode, input 2/Q.\n";
            break;
        case(3):
            std::cerr << "Enabled no-mod direct sampling mode.\n";
            break;
    }

    return r;
}

int ReceiverHWImpl::Pimpl::setPpm(int ppm_error) {
    int r;
    if(ppm_error == 0)
        return 0;
    r = rtlsdr_set_freq_correction(dev, ppm_error);
    if(r < 0)
        std::cerr << "WARNING: Failed to set ppm error.\n";
    else
        std::cerr << "Tuner error set to " << ppm_error << "ppm.\n";
    return r;
}

void ReceiverHWImpl::Pimpl::setOffsetTuningOn() {
    rtlsdr_set_offset_tuning(dev, 1);
}

/**
 * @brief Функция для включения/выключения автоматического контроля усиления.
 * Он необходим для того, чтобы при большой мощности сигнала приемник
 * не ограничивал сигнал и можно было видеть его без искажений.
 * Например, без включения этого метода синусоиды при большой мощности
 * генератора приемник отсекал часть синусоиды сверху и снизу из-за этого часть
 * данных была неправильной.
 */
void ReceiverHWImpl::Pimpl::setAgcMode(int on) {
    rtlsdr_set_agc_mode(dev, on);
    if(on)
        std::cerr << "AGC mode on\n";
    else
        std::cerr << "AGC mode off\n";
}

/**
 * @brief Метод для очиски буфера приемника. Необходимо вызывать каждый раз
 * перед чтением данных (асинхронным или синхронным)
 */

int ReceiverHWImpl::Pimpl::resetBuffer() {
    int r;
    r = rtlsdr_reset_buffer(dev);
    if(r < 0)
        std::cerr << "WARNING: Failed to reset buffers.\n";
    return r;
}

/**
 * @brief Метод для округление до ближайшей степни двойки вверх.
 * Он необходим для того, чтобы можно было правильно считать данные.
 */

uint32_t ReceiverHWImpl::Pimpl::roundPowerTwo(uint32_t& size) {
    auto result = size - 1;
    for(unsigned k = 0; k <= 4; ++k)
        result |= result >> (1 << k);
    ++result;
    return result;
}

void ReceiverHWImpl::setCallBack(std::function<void(Complex<int8_t>*, uint32_t)> function) {
    process = function;
}

void ReceiverHWImpl::start() {
    needProcessing = true;

    auto Fs = rtlsdr_get_sample_rate(m_d->dev);
    if(Fs <= 0) {
        std::cerr << "WARNING: Failed sample_rate.\n";
    }
    std::cout << "sample_rate = " << double(Fs) / 10e6 << " МГц" << std::endl;

    if(rtlsdr_set_testmode(m_d->dev, settingTransaction.testMode)) {
        std::cerr << "WARNING: Failed to set testMode.\n";
    }

    if(settingTransaction.typeTransaction == TypeTransaction::single) {
        startSingle();
    } else if(settingTransaction.typeTransaction == TypeTransaction::loop) {
        startLoop();
    }
}

void ReceiverHWImpl::startLoop() {
    m_d->resetBuffer(); // должен быть обязательно!!

    // auto e = rtlsdr_set_tuner_bandwidth(m_d->dev, 0.5e6);//не понятно насклолько влияет этот метод, но замечано, что
    // он ломает счетчик if(e < 0) {
    //     throw "FAIL set_tuner_bandwidth: ";
    // };
    callback = [](uint8_t* buf, uint32_t size, void* ctx) {
        ReceiverHWImpl* d
            = reinterpret_cast<ReceiverHWImpl*>(ctx); // контекст которые мы передали, кастим к объекту класса

        d->process((Base::Complex<signed char>*)buf, size);
    };

    thread = std::make_unique<std::thread>([this]() {
        auto r = rtlsdr_read_async(m_d->dev, callback, this, settingTransaction.ircSize * 2,
                                   settingTransaction.bufferSize
                                       / settingTransaction.ircSize); // this в данном случае является контекстом

        if(r < 0) {
            std::cout << "FAIL read_async: " << r;
        };
    });
}

void ReceiverHWImpl::startSingle() {
    while(isNeedProcessing()) {
        getComplex(complexBuff.data(), settingTransaction.bufferSize);
        process(complexBuff.data(), settingTransaction.bufferSize);
    }
}
void ReceiverHWImpl::stop() {
    {
        if(settingTransaction.typeTransaction == TypeTransaction::loop && needProcessing) {
            auto r = rtlsdr_cancel_async(m_d->dev);
            if(r < 0) {
                std::cerr << "FAIL STOP\n" << r;
            }
            thread->join();
        }
        needProcessing = false;
    }
}
